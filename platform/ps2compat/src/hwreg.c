/*
 * EE hardware registers, scratchpad and DMA.
 *
 * Register names point into a shadow of the 0x10000000 register page. DMA is
 * serviced synchronously on the CPU: kicking a channel walks its source chain
 * right away and feeds VIF1/GIF, so every "wait for DMA" loop in the game sees
 * an idle channel.
 */
#include "eeregs.h"
#include "libdma.h"
#include "recvx_platform.h"
#include "gs/gs.h"

volatile u_int recvx_hwreg_shadow[RECVX_HWREG_SIZE / 4];
u_char recvx_spr[0x4000] __attribute__((aligned(64)));

/* VU microcode images (ps2_vu*.vsm); the port runs C versions instead, so
 * these are only recognised as "program upload" chains and skipped. */
int ps2_vu0sub0, ps2_vu1sub0, ps2_vu1sub1;

#define CHCR_DIR (1u << 0)
#define CHCR_MOD(x) (((x) >> 2) & 3)
#define CHCR_TTE (1u << 6)
#define CHCR_STR (1u << 8)

enum { CH_VIF0, CH_VIF1, CH_GIF };

/*
 * EE DMA addresses -> Vita pointers. Scratchpad addresses appear as the
 * 0x70000000 CPU mapping or as SPR-flagged (bit 31) offsets; anything else is
 * a whole Vita pointer.
 */
void *recvx_dma_ptr(u_int addr)
{
    if ((addr & 0x7fffc000u) == 0x70000000u || (addr & 0xffffc000u) == 0x80000000u)
        return recvx_spr + (addr & 0x3fff);
    if (addr < 0x10000000u)
        RECVX_LOG_ONCE("dma: truncated EE address %08x", addr);
    return (void *)addr;
}

static void dma_send(int ch, u_int addr, u_int qwc)
{
    if (!qwc) return;
    const void *p = recvx_dma_ptr(addr);
    switch (ch) {
    case CH_VIF1: vif1_transfer(p, qwc * 4); break;
    case CH_GIF:  gif_transfer(GIF_PATH3, p, qwc); break;
    default: break;
    }
}

static void dma_chain(int ch, u_int tadr, int tte)
{
    u_int asr[2];
    int sp = 0;

    for (int n = 0; n < 1 << 20; n++) {
        const u_int *tag = recvx_dma_ptr(tadr);
        u_int qwc = tag[0] & 0xffff, id = (tag[0] >> 28) & 7, addr = tag[1];
        u_int body = tadr + 16;

        if (tte && ch == CH_VIF1)
            vif1_transfer(tag + 2, 2);

        switch (id) {
        case 0: /* refe */ dma_send(ch, addr, qwc); return;
        case 1: /* cnt  */ dma_send(ch, body, qwc); tadr = body + qwc * 16; break;
        case 2: /* next */ dma_send(ch, body, qwc); tadr = addr; break;
        case 3: /* ref  */
        case 4: /* refs */ dma_send(ch, addr, qwc); tadr = body; break;
        case 5: /* call */
            dma_send(ch, body, qwc);
            if (sp == 2) { RECVX_LOG("dma: call stack overflow"); return; }
            asr[sp++] = body + qwc * 16;
            tadr = addr;
            break;
        case 6: /* ret  */
            dma_send(ch, body, qwc);
            if (!sp) return;
            tadr = asr[--sp];
            break;
        case 7: /* end  */ dma_send(ch, body, qwc); return;
        }
    }
    RECVX_LOG("dma: runaway chain on channel %d", ch);
}

static int channel_of(u_int hwaddr)
{
    switch (hwaddr & 0xffffff00u) {
    case 0x10008000u: return CH_VIF0;
    case 0x10009000u: return CH_VIF1;
    case 0x1000a000u: return CH_GIF;
    }
    return -1;
}

static void dma_kick(u_int chcr_addr)
{
    int ch = channel_of(chcr_addr);
    volatile u_int *r = RECVX_HWREG(chcr_addr);
    u_int chcr = r[0], madr = r[4], qwc = r[8], tadr = r[12];

    if (ch == CH_VIF0) {
        RECVX_LOG_ONCE("dma: VIF0 transfer ignored (VU0 microcode runs natively)");
    } else if (ch >= 0) {
        if (CHCR_MOD(chcr) == 1)
            dma_chain(ch, tadr, chcr & CHCR_TTE);
        else
            dma_send(ch, madr, qwc);
    }
    r[0] = chcr & ~CHCR_STR;
}

u_int recvx_hwreg_read(u_int addr)
{
    switch (addr) {
    case RECVX_GIF_STAT:
    case RECVX_VIF1_STAT:
    case RECVX_IPU_CTRL:
        return 0; /* idle */
    case RECVX_T0_COUNT:
        return recvx_vblank_count() * 262; /* H-blank clock */
    }
    return *RECVX_HWREG(addr);
}

void recvx_hwreg_write(u_int addr, u_int value)
{
    *RECVX_HWREG(addr) = value;
    if ((addr & 0xff) == 0 && channel_of(addr) >= 0 && (value & CHCR_STR))
        dma_kick(addr);
}

/* ------------------------------------------------------------------ libdma */

int sceDmaReset(int mode)
{
    (void)mode;
    for (u_int a = 0x8000; a < 0xf000; a += 4) recvx_hwreg_shadow[a / 4] = 0;
    return 0;
}

static u_int chan_hwaddr(sceDmaChan *d)
{
    u_int a = (u_int)d;
    u_int base = (u_int)recvx_hwreg_shadow;
    if (a >= base && a < base + RECVX_HWREG_SIZE)
        return RECVX_HWREG_BASE + (a - base);
    return a; /* literal register address */
}

void sceDmaSend(sceDmaChan *d, void *tag)
{
    u_int hw = chan_hwaddr(d);
    int ch = channel_of(hw);

    if (tag == &ps2_vu0sub0 || tag == &ps2_vu1sub0 || tag == &ps2_vu1sub1)
        return;
    if (ch == CH_VIF0) {
        RECVX_LOG_ONCE("dma: VIF0 transfer ignored (VU0 microcode runs natively)");
        return;
    }
    if (ch < 0) {
        RECVX_LOG("sceDmaSend: unknown channel %08x", hw);
        return;
    }
    dma_chain(ch, (u_int)tag, ch == CH_VIF1);
}

int sceDmaSync(sceDmaChan *d, int mode, int timeout)
{
    (void)d; (void)mode; (void)timeout;
    return 0;
}
