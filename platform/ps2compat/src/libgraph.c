/*
 * libgraph on the GS model.
 *
 * Packet layouts follow the game's own copy of libgraph (sceGsSetDefDrawEnv,
 * sceGsSetDefClear, sceGsSetDefDBuffDc in SLUS_201.84): the draw
 * environments are real A+D GIF packets that go through gs_gif.c, and
 * "putting" a display environment presents its DISPFB.
 */
#include <string.h>
#include <psp2/kernel/threadmgr.h>
#include "libgraph.h"
#include "recvx_platform.h"
#include "gs/gs.h"

#define AD(reg_) ((u_long)(reg_))

static short g_inter, g_omode, g_ffmode;
static short disp_w = 640, disp_h = 480;

int sceGsResetGraph(short mode, short inter, short omode, short ffmode)
{
    (void)mode;
    g_inter = inter; g_omode = omode; g_ffmode = ffmode;
    gs_reset();
    vif1_reset();
    return 0;
}

void sceGsResetPath(void) { vif1_reset(); }

int sceGsSyncV(int mode)
{
    (void)mode;
    u_int v = recvx_vblank_count();
    while (recvx_vblank_count() == v) sceKernelDelayThread(500);
    return (v + 1) & 1;   /* field: 0 even, 1 odd */
}

int sceGsSyncPath(int mode, u_short timeout)
{
    (void)mode; (void)timeout;
    return 0;   /* every path completes synchronously */
}

/* --------------------------------------------------------- draw environment */

/* sceGszbufaddr: Z buffer goes after two PSMCT32 colour buffers of w x h */
static u_int zbuf_addr(short psm, short w, short h)
{
    (void)psm;
    u_int pages = ((w + 63) / 64) * ((h + 31) / 32);
    return pages * 2;
}

static void set_def_draw_env(u_long *p, short psm, short w, short h, short ztest, short zpsm, int ctx2)
{
    u_long *r = p;
    r[0] = SCE_GS_SET_FRAME(0, (w + 63) / 64, psm & 0xf, 0);
    r[1] = AD(ctx2 ? SCE_GS_FRAME_2 : SCE_GS_FRAME_1);
    r[2] = SCE_GS_SET_ZBUF(zbuf_addr(psm, w, h), zpsm & 0xf, ztest ? 0 : 1);
    r[3] = AD(ctx2 ? SCE_GS_ZBUF_2 : SCE_GS_ZBUF_1);
    r[4] = SCE_GS_SET_XYOFFSET((2048 - w / 2) << 4, (2048 - h / 2) << 4);
    r[5] = AD(ctx2 ? SCE_GS_XYOFFSET_2 : SCE_GS_XYOFFSET_1);
    r[6] = SCE_GS_SET_SCISSOR(0, w - 1, 0, h - 1);
    r[7] = AD(ctx2 ? SCE_GS_SCISSOR_2 : SCE_GS_SCISSOR_1);
    r[8] = 1;  r[9] = AD(SCE_GS_PRMODECONT);
    r[10] = 1; r[11] = AD(SCE_GS_COLCLAMP);
    r[12] = (psm & 2) ? 1 : 0; r[13] = AD(SCE_GS_DTHE);
    r[14] = ztest ? SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, ztest & 3) : SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1);
    r[15] = AD(ctx2 ? SCE_GS_TEST_2 : SCE_GS_TEST_1);
}

static void set_def_clear(u_long *p, short ztest, short x, short y, short w, short h)
{
    p[0] = SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1); p[1] = AD(SCE_GS_TEST_1);
    p[2] = SCE_GS_PRIM_SPRITE;                     p[3] = AD(SCE_GS_PRIM);
    p[4] = SCE_GS_SET_RGBAQ(0, 0, 0, 0, 0x3f800000); p[5] = AD(SCE_GS_RGBAQ);
    p[6] = SCE_GS_SET_XYZ(x << 4, y << 4, 0);      p[7] = AD(SCE_GS_XYZ2);
    p[8] = SCE_GS_SET_XYZ((x + w) << 4, (y + h) << 4, 0); p[9] = AD(SCE_GS_XYZ2);
    p[10] = ztest ? SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, ztest & 3) : SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1);
    p[11] = AD(SCE_GS_TEST_1);
}

static void set_def_disp_env(sceGsDispEnv *d, short psm, short w, short h)
{
    memset(d, 0, sizeof(*d));
    d->pmode.EN1 = 1;
    d->pmode.CRTMD = 1;
    d->pmode.MMOD = 1;
    d->pmode.ALP = 0xff;
    d->smode2.INT = g_inter;
    d->smode2.FFMD = g_ffmode;
    d->dispfb.FBW = (w + 63) / 64;
    d->dispfb.PSM = psm;
    d->display.MAGH = 2560 / w - 1;
    d->display.DW = 2560 - 1;
    d->display.DH = h - 1;
}

void sceGsSetDefDBuffDc(sceGsDBuffDc *db, short psm, short w, short h, short ztest, short zpsm, short clear)
{
    memset(db, 0, sizeof(*db));
    disp_w = w;
    disp_h = h;
    set_def_disp_env(&db->disp[0], psm, w, h);
    set_def_disp_env(&db->disp[1], psm, w, h);
    set_def_draw_env((u_long *)&db->draw01, psm, w, h, ztest, zpsm, 0);
    set_def_draw_env((u_long *)&db->draw02, psm, w, h, ztest, zpsm, 1);
    set_def_draw_env((u_long *)&db->draw11, psm, w, h, ztest, zpsm, 0);
    set_def_draw_env((u_long *)&db->draw12, psm, w, h, ztest, zpsm, 1);
    if (clear) {
        set_def_clear((u_long *)&db->clear0, ztest, 2048 - w / 2, 2048 - h / 2, w, h);
        set_def_clear((u_long *)&db->clear1, ztest, 2048 - w / 2, 2048 - h / 2, w, h);
    }
    u_int nloop = clear ? 22 : 16;
    db->giftag0[0] = db->giftag1[0] = SCE_GIF_SET_TAG(nloop, 1, 0, 0, SCE_GIF_PACKED, 1);
    db->giftag0[1] = db->giftag1[1] = SCE_GIF_PACKED_AD;

    /* buffer 1 draws at the second frame buffer (interlaced FIELD keeps both at 0) */
    u_int fbp1 = ((w + 63) / 64) * ((h + 31) / 32);
    if (!(g_inter && !g_ffmode)) {
        db->disp[1].dispfb.FBP = fbp1;
        db->draw01.frame1.FBP = fbp1;
        db->draw02.frame2.FBP = fbp1;
    }
}

static void put_draw_env(u_long *giftag)
{
    gif_transfer(GIF_PATH3, giftag, (giftag[0] & 0x7fff) + 1);
}

static void put_disp_env(const sceGsDispEnv *d)
{
    gs_draw_present(d->dispfb.FBP, d->dispfb.FBW, d->dispfb.PSM, disp_w, disp_h);
}

int sceGsSwapDBuffDc(sceGsDBuffDc *db, int id)
{
    id &= 1;
    put_disp_env(&db->disp[id]);
    put_draw_env(id ? db->giftag1 : db->giftag0);
    return 0;
}

/* --------------------------------------------------------- image transfer */

int sceGsSetDefLoadImage(sceGsLoadImage *lp, short dbp, short dbw, short dpsm, short x, short y, short w, short h)
{
    u_long *p = (u_long *)lp;
    int bpp = gs_psm_bpp(dpsm);
    p[0] = SCE_GIF_SET_TAG(4, 0, 0, 0, SCE_GIF_PACKED, 1);
    p[1] = SCE_GIF_PACKED_AD;
    p[2] = SCE_GS_SET_BITBLTBUF(0, 0, 0, dbp, dbw, dpsm); p[3] = SCE_GS_BITBLTBUF;
    p[4] = SCE_GS_SET_TRXPOS(0, 0, x, y, 0);              p[5] = SCE_GS_TRXPOS;
    p[6] = SCE_GS_SET_TRXREG(w, h);                       p[7] = SCE_GS_TRXREG;
    p[8] = SCE_GS_SET_TRXDIR(0);                          p[9] = SCE_GS_TRXDIR;
    p[10] = SCE_GIF_SET_TAG((w * h * bpp + 127) / 128, 1, 0, 0, SCE_GIF_IMAGE, 0);
    p[11] = 0;
    return 0;
}

int sceGsExecLoadImage(sceGsLoadImage *lp, u_long128 *srcaddr)
{
    u_long *p = (u_long *)lp;
    gif_transfer(GIF_PATH3, p, 6);
    gif_transfer(GIF_PATH3, srcaddr, p[10] & 0x7fff);
    return 0;
}

int sceGsSetDefStoreImage(sceGsStoreImage *sp, short sbp, short sbw, short spsm, short x, short y, short w, short h)
{
    memset(sp, 0, sizeof(*sp));
    sp->bitbltbuf.SBP = sbp; sp->bitbltbuf.SBW = sbw; sp->bitbltbuf.SPSM = spsm;
    sp->bitbltbuf.DBW = sbw; sp->bitbltbuf.DPSM = spsm;
    sp->trxpos.SSAX = x; sp->trxpos.SSAY = y;
    sp->trxreg.RRW = w; sp->trxreg.RRH = h;
    sp->trxdir.XDR = 1;
    sp->vifcode[3] = (w * h * gs_psm_bpp(spsm) + 127) / 128;   /* qwc to read back */
    return 0;
}

int sceGsExecStoreImage(sceGsStoreImage *sp, u_long128 *dstaddr)
{
    gs_write_reg(SCE_GS_BITBLTBUF, *(u_long *)&sp->bitbltbuf);
    gs_write_reg(SCE_GS_TRXPOS, *(u_long *)&sp->trxpos);
    gs_write_reg(SCE_GS_TRXREG, *(u_long *)&sp->trxreg);
    gs_write_reg(SCE_GS_TRXDIR, 1);
    gs_mem_store(dstaddr, sp->vifcode[3]);
    return 0;
}
