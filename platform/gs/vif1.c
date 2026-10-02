/*
 * VIF1: decodes the command stream the game DMAs to VU1. Data lands in a
 * VU1 data-memory image; MSCAL hands the batch to vu1_hle.c, which feeds the
 * GPU directly instead of running the original microcode. DIRECT/DIRECTHL go
 * to GIF PATH2.
 */
#include <string.h>
#include "gs/gs.h"
#include "recvx_platform.h"

float vu1_mem[1024][4] __attribute__((aligned(16)));

void vu1_hle_run(u_int start_addr);

static struct {
    u_int cl, wl, mode;
    u_int mask;
    u_int row[4], col[4];
    u_int base, ofst, tops, top, itops, itop, dbf;
    u_int mark;
} v;

/* in-progress multi-word command */
static struct {
    u_int cmd;              /* 0 = idle */
    u_int left;             /* words still expected */
    /* UNPACK */
    u_int addr, num, fmt, usn, msk, cycle, done;
    /* STROW/STCOL */
    u_int idx;
    /* DIRECT: qword assembly */
    uint32_t q[4];
    u_int qn;
} c;

void vif1_reset(void)
{
    memset(&v, 0, sizeof(v));
    memset(&c, 0, sizeof(c));
    v.cl = v.wl = 1;
}

u_int vu1_top(void) { return v.top; }
u_int vu1_itop(void) { return v.itop; }

/* ----------------------------------------------------------------- UNPACK */

/* words consumed per unpacked element, in 1/4 word units */
static u_int unpack_quarter_words(u_int fmt)
{
    u_int vn = (fmt >> 2) & 3, vl = fmt & 3;
    if (fmt == 0xf) return 2;                       /* V4-5: 16 bits */
    static const u_int bits[4] = { 32, 16, 8, 5 };
    return (vn + 1) * bits[vl] / 8;                 /* bytes */
}

static void unpack_write(u_int addr, const uint32_t in[4], int ncomp)
{
    uint32_t *dst = VU1_MEMI(addr);
    u_int cyc = c.cycle < 4 ? c.cycle : 3;
    for (int i = 0; i < 4; i++) {
        uint32_t val = i < ncomp ? in[i] : 0;
        u_int m = c.msk ? (v.mask >> (cyc * 8 + i * 2)) & 3 : 0;
        switch (m) {
        case 0:
            switch (v.mode) {
            case 1: val += v.row[i]; break;
            case 2: v.row[i] += val; val = v.row[i]; break;
            }
            dst[i] = val;
            break;
        case 1: dst[i] = v.row[i]; break;
        case 2: dst[i] = v.col[cyc]; break;
        case 3: break;   /* write-protected */
        }
    }
}

static void unpack_element(const uint8_t *src)
{
    uint32_t in[4] = { 0, 0, 0, 0 };
    u_int vn = (c.fmt >> 2) & 3, vl = c.fmt & 3;
    int n = vn + 1;

    if (c.fmt == 0xf) {
        uint16_t p = src[0] | src[1] << 8;
        in[0] = (p & 0x1f) << 3;
        in[1] = ((p >> 5) & 0x1f) << 3;
        in[2] = ((p >> 10) & 0x1f) << 3;
        in[3] = ((p >> 15) & 1) << 7;
        n = 4;
    } else {
        for (int i = 0; i < n; i++) {
            switch (vl) {
            case 0: memcpy(&in[i], src + i * 4, 4); break;
            case 1: {
                uint16_t h = src[i * 2] | src[i * 2 + 1] << 8;
                in[i] = c.usn ? h : (uint32_t)(int32_t)(int16_t)h;
                break;
            }
            case 2: in[i] = c.usn ? src[i] : (uint32_t)(int32_t)(int8_t)src[i]; break;
            }
        }
        if (vn == 0) { in[1] = in[2] = in[3] = in[0]; n = 4; }  /* S: broadcast */
        else if (vn == 1) { in[2] = in[0]; in[3] = in[1]; n = 4; } /* V2: xyxy (undefined zw, matches hw) */
        else if (vn == 2) { in[3] = 0; n = 4; }   /* V3: w is garbage on hw; game never relies on it */
    }
    (void)n;
    c.done++;

    /* cycle handling: CL < WL fills from the stream, CL >= WL skips */
    unpack_write(c.addr, in, 4);
    c.cycle++;
    c.addr++;
    if (v.cl >= v.wl) {
        if (c.cycle == v.wl) { c.addr += v.cl - v.wl; c.cycle = 0; }
    } else if (c.cycle == v.wl) {
        c.cycle = 0;
    }
}

/* ------------------------------------------------------------- byte stream */

static uint8_t unpack_buf[16];
static u_int unpack_have;

static void cmd_begin(uint32_t w)
{
    u_int cmd = (w >> 24) & 0x7f, imm = w & 0xffff, num = (w >> 16) & 0xff;

    if ((cmd & 0x60) == 0x60) {               /* UNPACK */
        c.cmd = 0x60;
        c.fmt = cmd & 0xf;
        c.msk = (cmd >> 4) & 1;
        c.usn = (imm >> 14) & 1;
        c.num = num ? num : 256;
        c.addr = imm & 0x3ff;
        if (imm & 0x8000) c.addr += v.tops;
        c.cycle = 0;
        c.done = 0;
        u_int elems_from_stream = c.num;
        if (v.wl > v.cl) {
            /* only CL of every WL written elements come from the stream */
            u_int full = c.num / v.wl, rem = c.num % v.wl;
            elems_from_stream = full * v.cl + (rem < v.cl ? rem : v.cl);
        }
        u_int bytes = elems_from_stream * unpack_quarter_words(c.fmt);
        c.left = (bytes + 3) / 4;
        unpack_have = 0;
        if (!c.left) c.cmd = 0;
        return;
    }

    switch (cmd) {
    case 0x00: break;                                   /* NOP */
    case 0x01: v.cl = imm & 0xff; v.wl = (imm >> 8) & 0xff; if (!v.wl) v.wl = 256; break;
    case 0x02: v.ofst = imm & 0x3ff; v.dbf = 0; v.tops = v.base; break;   /* OFFSET */
    case 0x03: v.base = imm & 0x3ff; break;             /* BASE */
    case 0x04: v.itops = imm & 0x3ff; break;            /* ITOP */
    case 0x05: v.mode = imm & 3; break;                 /* STMOD */
    case 0x06: break;                                   /* MSKPATH3 */
    case 0x07: v.mark = imm; break;                     /* MARK */
    case 0x10: case 0x11: case 0x13: break;             /* FLUSHE/FLUSH/FLUSHA */
    case 0x14: case 0x15: case 0x17: {                  /* MSCAL / MSCALF / MSCNT */
        v.top = v.tops;
        v.itop = v.itops;
        vu1_hle_run(cmd == 0x17 ? ~0u : imm);
        v.dbf ^= 1;
        v.tops = v.dbf ? v.base + v.ofst : v.base;
        break;
    }
    case 0x20: c.cmd = 0x20; c.left = 1; break;         /* STMASK */
    case 0x30: c.cmd = 0x30; c.left = 4; c.idx = 0; break; /* STROW */
    case 0x31: c.cmd = 0x31; c.left = 4; c.idx = 0; break; /* STCOL */
    case 0x4a:                                           /* MPG: microcode, not run */
        c.cmd = 0x4a;
        c.left = (num ? num : 256) * 2;
        break;
    case 0x50: case 0x51:                                /* DIRECT / DIRECTHL */
        c.cmd = 0x50;
        c.left = (imm ? imm : 65536) * 4;
        c.qn = 0;
        break;
    default:
        RECVX_LOG_ONCE("vif1: unknown command %02x", cmd);
        break;
    }
}

static void cmd_word(uint32_t w)
{
    switch (c.cmd) {
    case 0x60:
        memcpy(unpack_buf + unpack_have, &w, 4);
        unpack_have += 4;
        while (c.done < c.num) {
            u_int need = unpack_quarter_words(c.fmt);
            if (v.wl > v.cl && c.cycle >= v.cl) {
                static const uint8_t zero[16];
                unpack_element(zero);      /* filling write: no stream data */
                continue;
            }
            if (unpack_have < need) break;
            unpack_element(unpack_buf);
            unpack_have -= need;
            memmove(unpack_buf, unpack_buf + need, unpack_have);
        }
        break;
    case 0x20: v.mask = w; break;
    case 0x30: v.row[c.idx++] = w; break;
    case 0x31: v.col[c.idx++] = w; break;
    case 0x4a: break;
    case 0x50:
        c.q[c.qn++] = w;
        if (c.qn == 4) { gif_transfer(GIF_PATH2, c.q, 1); c.qn = 0; }
        break;
    }
    if (--c.left == 0) {
        if (c.cmd == 0x60) unpack_have = 0;   /* tail padding to a word */
        c.cmd = 0;
    }
}

static void vif1_transfer_(const void *words, u_int nwords)
{
    const uint32_t *p = words;
    for (u_int i = 0; i < nwords; i++) {
        if (c.cmd) cmd_word(p[i]);
        else cmd_begin(p[i]);
    }
}

void vif1_transfer(const void *words, u_int nwords)
{
    gs_lock();
    vif1_transfer_(words, nwords);
    gs_unlock();
}
