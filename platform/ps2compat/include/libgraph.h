/*
 * libgraph: GS register layouts and packet-building macros.
 *
 * Bit layouts follow the GS hardware so that game code which assembles GS
 * packets keeps producing the same 64-bit register values; the port's render
 * backend decodes them.
 */
#pragma once
#include "eetypes.h"

#define GS_U(x) ((u_long)(x))

/* --- GS general-purpose register addresses --- */
#define SCE_GS_PRIM       0x00
#define SCE_GS_RGBAQ      0x01
#define SCE_GS_ST         0x02
#define SCE_GS_UV         0x03
#define SCE_GS_XYZF2      0x04
#define SCE_GS_XYZ2       0x05
#define SCE_GS_TEX0_1     0x06
#define SCE_GS_TEX0_2     0x07
#define SCE_GS_CLAMP_1    0x08
#define SCE_GS_CLAMP_2    0x09
#define SCE_GS_FOG        0x0a
#define SCE_GS_XYZF3      0x0c
#define SCE_GS_XYZ3       0x0d
#define SCE_GS_TEX1_1     0x14
#define SCE_GS_TEX1_2     0x15
#define SCE_GS_TEX2_1     0x16
#define SCE_GS_TEX2_2     0x17
#define SCE_GS_XYOFFSET_1 0x18
#define SCE_GS_XYOFFSET_2 0x19
#define SCE_GS_PRMODECONT 0x1a
#define SCE_GS_PRMODE     0x1b
#define SCE_GS_TEXCLUT    0x1c
#define SCE_GS_SCANMSK    0x22
#define SCE_GS_MIPTBP1_1  0x34
#define SCE_GS_MIPTBP1_2  0x35
#define SCE_GS_MIPTBP2_1  0x36
#define SCE_GS_MIPTBP2_2  0x37
#define SCE_GS_TEXA       0x3b
#define SCE_GS_FOGCOL     0x3d
#define SCE_GS_TEXFLUSH   0x3f
#define SCE_GS_SCISSOR_1  0x40
#define SCE_GS_SCISSOR_2  0x41
#define SCE_GS_ALPHA_1    0x42
#define SCE_GS_ALPHA_2    0x43
#define SCE_GS_DIMX       0x44
#define SCE_GS_DTHE       0x45
#define SCE_GS_COLCLAMP   0x46
#define SCE_GS_TEST_1     0x47
#define SCE_GS_TEST_2     0x48
#define SCE_GS_PABE       0x49
#define SCE_GS_FBA_1      0x4a
#define SCE_GS_FBA_2      0x4b
#define SCE_GS_FRAME_1    0x4c
#define SCE_GS_FRAME_2    0x4d
#define SCE_GS_ZBUF_1     0x4e
#define SCE_GS_ZBUF_2     0x4f
#define SCE_GS_BITBLTBUF  0x50
#define SCE_GS_TRXPOS     0x51
#define SCE_GS_TRXREG     0x52
#define SCE_GS_TRXDIR     0x53
#define SCE_GS_HWREG      0x54
#define SCE_GS_SIGNAL     0x60
#define SCE_GS_FINISH     0x61
#define SCE_GS_LABEL      0x62

/* --- constants --- */
#define SCE_GS_FALSE 0
#define SCE_GS_TRUE  1

#define SCE_GS_PRIM_POINT    0
#define SCE_GS_PRIM_LINE     1
#define SCE_GS_PRIM_LINESTRIP 2
#define SCE_GS_PRIM_TRI      3
#define SCE_GS_PRIM_TRISTRIP 4
#define SCE_GS_PRIM_TRIFAN   5
#define SCE_GS_PRIM_SPRITE   6

#define SCE_GS_PSMCT32  0x00
#define SCE_GS_PSMCT24  0x01
#define SCE_GS_PSMCT16  0x02
#define SCE_GS_PSMCT16S 0x0a
#define SCE_GS_PSMT8    0x13
#define SCE_GS_PSMT4    0x14
#define SCE_GS_PSMT8H   0x1b
#define SCE_GS_PSMT4HL  0x24
#define SCE_GS_PSMT4HH  0x2c
#define SCE_GS_PSMZ32   0x30
#define SCE_GS_PSMZ24   0x31
#define SCE_GS_PSMZ16   0x32
#define SCE_GS_PSMZ16S  0x3a

#define SCE_GS_MODULATE 0
#define SCE_GS_DECAL    1
#define SCE_GS_HIGHLIGHT 2
#define SCE_GS_HIGHLIGHT2 3

#define SCE_GS_NEAREST  0
#define SCE_GS_LINEAR   1
#define SCE_GS_NEAREST_MIPMAP_NEAREST 2
#define SCE_GS_NEAREST_MIPMAP_LINEAR  3
#define SCE_GS_LINEAR_MIPMAP_NEAREST  4
#define SCE_GS_LINEAR_MIPMAP_LINEAR   5

#define SCE_GS_REPEAT 0
#define SCE_GS_CLAMP  1
#define SCE_GS_REGION_CLAMP  2
#define SCE_GS_REGION_REPEAT 3

#define SCE_GS_ALPHA_NEVER    0
#define SCE_GS_ALPHA_ALWAYS   1
#define SCE_GS_ALPHA_LESS     2
#define SCE_GS_ALPHA_LEQUAL   3
#define SCE_GS_ALPHA_EQUAL    4
#define SCE_GS_ALPHA_GEQUAL   5
#define SCE_GS_ALPHA_GREATER  6
#define SCE_GS_ALPHA_NOTEQUAL 7

#define SCE_GS_AFAIL_KEEP    0
#define SCE_GS_AFAIL_FB_ONLY 1
#define SCE_GS_AFAIL_ZB_ONLY 2
#define SCE_GS_AFAIL_RGB_ONLY 3

#define SCE_GS_DEPTH_NEVER   0
#define SCE_GS_DEPTH_ALWAYS  1
#define SCE_GS_DEPTH_GEQUAL  2
#define SCE_GS_DEPTH_GREATER 3
#define SCE_GS_ZNEVER   0
#define SCE_GS_ZALWAYS  1
#define SCE_GS_ZGEQUAL  2
#define SCE_GS_ZGREATER 3

#define SCE_GS_ALPHA_CS  0
#define SCE_GS_ALPHA_CD  1
#define SCE_GS_ALPHA_ZERO 2
#define SCE_GS_ALPHA_AS  0
#define SCE_GS_ALPHA_AD  1
#define SCE_GS_ALPHA_FIX 2

#define SCE_GS_NOINTERLACE 0
#define SCE_GS_INTERLACE   1
#define SCE_GS_FIELD 0
#define SCE_GS_FRAME 1
#define SCE_GS_NTSC  2
#define SCE_GS_PAL   3

/* --- GIF tags --- */
#define SCE_GIF_PACKED  0
#define SCE_GIF_REGLIST 1
#define SCE_GIF_IMAGE   2
#define SCE_GIF_PACKED_AD 0x0e

#define SCE_GIF_SET_TAG(nloop, eop, pre, prim, flg, nreg) \
    (GS_U(nloop) | GS_U(eop) << 15 | GS_U(pre) << 46 | GS_U(prim) << 47 | GS_U(flg) << 58 | GS_U(nreg) << 60)

/* --- register value builders --- */
#define SCE_GS_SET_PRIM(prim, iip, tme, fge, abe, aa1, fst, ctxt, fix) \
    (GS_U(prim) | GS_U(iip) << 3 | GS_U(tme) << 4 | GS_U(fge) << 5 | GS_U(abe) << 6 | \
     GS_U(aa1) << 7 | GS_U(fst) << 8 | GS_U(ctxt) << 9 | GS_U(fix) << 10)
#define SCE_GS_SET_RGBAQ(r, g, b, a, q) \
    (GS_U(r) | GS_U(g) << 8 | GS_U(b) << 16 | GS_U(a) << 24 | GS_U(q) << 32)
#define SCE_GS_SET_ST(s, t) (GS_U(s) | GS_U(t) << 32)
#define SCE_GS_SET_UV(u, v) (GS_U(u) | GS_U(v) << 16)
#define SCE_GS_SET_XYZF(x, y, z, f) (GS_U(x) | GS_U(y) << 16 | GS_U(z) << 32 | GS_U(f) << 56)
#define SCE_GS_SET_XYZF2 SCE_GS_SET_XYZF
#define SCE_GS_SET_XYZF3 SCE_GS_SET_XYZF
#define SCE_GS_SET_XYZ(x, y, z) (GS_U(x) | GS_U(y) << 16 | GS_U(z) << 32)
#define SCE_GS_SET_XYZ2 SCE_GS_SET_XYZ
#define SCE_GS_SET_XYZ3 SCE_GS_SET_XYZ
#define SCE_GS_SET_TEX0(tbp, tbw, psm, tw, th, tcc, tfx, cbp, cpsm, csm, csa, cld) \
    (GS_U(tbp) | GS_U(tbw) << 14 | GS_U(psm) << 20 | GS_U(tw) << 26 | GS_U(th) << 30 | \
     GS_U(tcc) << 34 | GS_U(tfx) << 35 | GS_U(cbp) << 37 | GS_U(cpsm) << 51 | \
     GS_U(csm) << 55 | GS_U(csa) << 56 | GS_U(cld) << 61)
#define SCE_GS_SET_TEX0_1 SCE_GS_SET_TEX0
#define SCE_GS_SET_TEX0_2 SCE_GS_SET_TEX0
#define SCE_GS_SET_TEX1(lcm, mxl, mmag, mmin, mtba, l, k) \
    (GS_U(lcm) | GS_U(mxl) << 2 | GS_U(mmag) << 5 | GS_U(mmin) << 6 | GS_U(mtba) << 9 | \
     GS_U(l) << 19 | GS_U(k) << 32)
#define SCE_GS_SET_TEX1_1 SCE_GS_SET_TEX1
#define SCE_GS_SET_TEX1_2 SCE_GS_SET_TEX1
#define SCE_GS_SET_TEX2(psm, cbp, cpsm, csm, csa, cld) \
    (GS_U(psm) << 20 | GS_U(cbp) << 37 | GS_U(cpsm) << 51 | GS_U(csm) << 55 | GS_U(csa) << 56 | GS_U(cld) << 61)
#define SCE_GS_SET_CLAMP(wms, wmt, minu, maxu, minv, maxv) \
    (GS_U(wms) | GS_U(wmt) << 2 | GS_U(minu) << 4 | GS_U(maxu) << 14 | GS_U(minv) << 24 | GS_U(maxv) << 34)
#define SCE_GS_SET_CLAMP_1 SCE_GS_SET_CLAMP
#define SCE_GS_SET_CLAMP_2 SCE_GS_SET_CLAMP
#define SCE_GS_SET_FOG(f) (GS_U(f) << 56)
#define SCE_GS_SET_FOGCOL(r, g, b) (GS_U(r) | GS_U(g) << 8 | GS_U(b) << 16)
#define SCE_GS_SET_XYOFFSET(ofx, ofy) (GS_U(ofx) | GS_U(ofy) << 32)
#define SCE_GS_SET_XYOFFSET_1 SCE_GS_SET_XYOFFSET
#define SCE_GS_SET_XYOFFSET_2 SCE_GS_SET_XYOFFSET
#define SCE_GS_SET_PRMODECONT(ac) GS_U(ac)
#define SCE_GS_SET_TEXCLUT(cbw, cou, cov) (GS_U(cbw) | GS_U(cou) << 6 | GS_U(cov) << 12)
#define SCE_GS_SET_TEXA(ta0, aem, ta1) (GS_U(ta0) | GS_U(aem) << 15 | GS_U(ta1) << 32)
#define SCE_GS_SET_TEXFLUSH() GS_U(0)
#define SCE_GS_SET_SCISSOR(x0, x1, y0, y1) (GS_U(x0) | GS_U(x1) << 16 | GS_U(y0) << 32 | GS_U(y1) << 48)
#define SCE_GS_SET_SCISSOR_1 SCE_GS_SET_SCISSOR
#define SCE_GS_SET_SCISSOR_2 SCE_GS_SET_SCISSOR
#define SCE_GS_SET_ALPHA(a, b, c, d, fix) \
    (GS_U(a) | GS_U(b) << 2 | GS_U(c) << 4 | GS_U(d) << 6 | GS_U(fix) << 32)
#define SCE_GS_SET_ALPHA_1 SCE_GS_SET_ALPHA
#define SCE_GS_SET_ALPHA_2 SCE_GS_SET_ALPHA
#define SCE_GS_SET_DTHE(dthe) GS_U(dthe)
#define SCE_GS_SET_COLCLAMP(clamp) GS_U(clamp)
#define SCE_GS_SET_TEST(ate, atst, aref, afail, date, datm, zte, ztst) \
    (GS_U(ate) | GS_U(atst) << 1 | GS_U(aref) << 4 | GS_U(afail) << 12 | GS_U(date) << 14 | \
     GS_U(datm) << 15 | GS_U(zte) << 16 | GS_U(ztst) << 17)
#define SCE_GS_SET_TEST_1 SCE_GS_SET_TEST
#define SCE_GS_SET_TEST_2 SCE_GS_SET_TEST
#define SCE_GS_SET_PABE(pabe) GS_U(pabe)
#define SCE_GS_SET_FBA(fba) GS_U(fba)
#define SCE_GS_SET_FBA_1 SCE_GS_SET_FBA
#define SCE_GS_SET_FBA_2 SCE_GS_SET_FBA
#define SCE_GS_SET_FRAME(fbp, fbw, psm, fbmask) \
    (GS_U(fbp) | GS_U(fbw) << 16 | GS_U(psm) << 24 | GS_U(fbmask) << 32)
#define SCE_GS_SET_FRAME_1 SCE_GS_SET_FRAME
#define SCE_GS_SET_FRAME_2 SCE_GS_SET_FRAME
#define SCE_GS_SET_ZBUF(zbp, psm, zmsk) (GS_U(zbp) | GS_U(psm) << 24 | GS_U(zmsk) << 32)
#define SCE_GS_SET_ZBUF_1 SCE_GS_SET_ZBUF
#define SCE_GS_SET_ZBUF_2 SCE_GS_SET_ZBUF
#define SCE_GS_SET_BITBLTBUF(sbp, sbw, spsm, dbp, dbw, dpsm) \
    (GS_U(sbp) | GS_U(sbw) << 16 | GS_U(spsm) << 24 | GS_U(dbp) << 32 | GS_U(dbw) << 48 | GS_U(dpsm) << 56)
#define SCE_GS_SET_TRXPOS(ssax, ssay, dsax, dsay, dir) \
    (GS_U(ssax) | GS_U(ssay) << 16 | GS_U(dsax) << 32 | GS_U(dsay) << 48 | GS_U(dir) << 59)
#define SCE_GS_SET_TRXREG(rrw, rrh) (GS_U(rrw) | GS_U(rrh) << 32)
#define SCE_GS_SET_TRXDIR(xdr) GS_U(xdr)

/* --- register structs (bitfields; little-endian, same layout as the GS) --- */
typedef struct {
    u_long TBP0:14, TBW:6, PSM:6, TW:4, TH:4, TCC:1, TFX:2, CBP:14, CPSM:4, CSM:1, CSA:5, CLD:3;
} sceGsTex0;
typedef struct {
    u_long LCM:1, pad01:1, MXL:3, MMAG:1, MMIN:3, MTBA:1, pad10:9, L:2, pad21:11, K:12, pad44:20;
} sceGsTex1;
typedef struct { u_long FBP:9, pad09:7, FBW:6, pad22:2, PSM:6, pad30:2, FBMSK:32; } sceGsFrame;
typedef struct { u_long ZBP:9, pad09:15, PSM:4, pad28:4, ZMSK:1, pad33:31; } sceGsZbuf;
typedef struct { u_long OFX:16, pad16:16, OFY:16, pad48:16; } sceGsXyoffset;
typedef struct { u_long SCAX0:11, pad11:5, SCAX1:11, pad27:5, SCAY0:11, pad43:5, SCAY1:11, pad59:5; } sceGsScissor;
typedef struct { u_long AC:1, pad01:63; } sceGsPrmodecont;
typedef struct { u_long CLAMP:1, pad01:63; } sceGsColclamp;
typedef struct { u_long DTHE:1, pad01:63; } sceGsDthe;
typedef struct {
    u_long ATE:1, ATST:3, AREF:8, AFAIL:2, DATE:1, DATM:1, ZTE:1, ZTST:2, pad19:45;
} sceGsTest;
typedef struct { u_long A:2, B:2, C:2, D:2, pad8:24, FIX:8, pad40:24; } sceGsAlpha;
typedef struct { u_long WMS:2, WMT:2, MINU:10, MAXU:10, MINV:10, MAXV:10, pad44:20; } sceGsClamp;
typedef struct { u_long TA0:8, pad08:7, AEM:1, pad16:16, TA1:8, pad40:24; } sceGsTexa;
typedef struct { u_long pad00:56, F:8; } sceGsFog;
typedef struct { u_long FCR:8, FCG:8, FCB:8, pad24:40; } sceGsFogcol;
typedef struct { u_long PABE:1, pad01:63; } sceGsPabe;
typedef struct { u_long FBA:1, pad01:63; } sceGsFba;
typedef struct { u_long pad; } sceGsTexflush;
typedef struct { u_long pad; } sceGsFinish;
typedef struct {
    u_long SBP:14, pad14:2, SBW:6, pad22:2, SPSM:6, pad30:2, DBP:14, pad46:2, DBW:6, pad54:2, DPSM:6, pad62:2;
} sceGsBitbltbuf;
typedef struct { u_long SSAX:11, pad11:5, SSAY:11, pad27:5, DSAX:11, pad43:5, DSAY:11, DIR:2, pad61:3; } sceGsTrxpos;
typedef struct { u_long RRW:12, pad12:20, RRH:12, pad44:20; } sceGsTrxreg;
typedef struct { u_long XDR:2, pad02:62; } sceGsTrxdir;

/* GS privileged display registers */
typedef struct { u_long EN1:1, EN2:1, CRTMD:3, MMOD:1, AMOD:1, SLBG:1, ALP:8, p0:48; } sceGsPmode;
typedef struct { u_long INT:1, FFMD:1, DPMS:2, p0:60; } sceGsSmode2;
typedef struct { u_long FBP:9, FBW:6, PSM:5, p0:12, DBX:11, DBY:11, p1:10; } sceGsDispfb;
typedef struct { u_long DX:12, DY:11, MAGH:4, MAGV:2, p0:3, DW:12, DH:11, p1:9; } sceGsDisplay;
typedef struct { u_long R:8, G:8, B:8, p0:40; } sceGsBgcolor;

typedef struct {
    sceGsPmode   pmode;
    sceGsSmode2  smode2;
    sceGsDispfb  dispfb;
    sceGsDisplay display;
    sceGsBgcolor bgcolor;
} sceGsDispEnv;

/* A+D register packets used by the draw-environment helpers */
typedef struct {
    sceGsFrame frame1;       u_long frame1addr;
    sceGsZbuf zbuf1;         u_long zbuf1addr;
    sceGsXyoffset xyoffset1; u_long xyoffset1addr;
    sceGsScissor scissor1;   u_long scissor1addr;
    sceGsPrmodecont prmodecont; u_long prmodecontaddr;
    sceGsColclamp colclamp;  u_long colclampaddr;
    sceGsDthe dthe;          u_long dtheaddr;
    sceGsTest test1;         u_long test1addr;
} sceGsDrawEnv1 __attribute__((aligned(16)));

typedef struct {
    sceGsFrame frame2;       u_long frame2addr;
    sceGsZbuf zbuf2;         u_long zbuf2addr;
    sceGsXyoffset xyoffset2; u_long xyoffset2addr;
    sceGsScissor scissor2;   u_long scissor2addr;
    sceGsPrmodecont prmodecont; u_long prmodecontaddr;
    sceGsColclamp colclamp;  u_long colclampaddr;
    sceGsDthe dthe;          u_long dtheaddr;
    sceGsTest test2;         u_long test2addr;
} sceGsDrawEnv2 __attribute__((aligned(16)));

typedef struct { u_long PRIM:3, IIP:1, TME:1, FGE:1, ABE:1, AA1:1, FST:1, CTXT:1, FIX:1, pad11:53; } sceGsPrim;
typedef struct { u_long R:8, G:8, B:8, A:8, Q:32; } sceGsRgbaq;
typedef struct { u_long X:16, Y:16, Z:32; } sceGsXyz;

typedef struct {
    sceGsTest testa;  u_long testaaddr;
    sceGsPrim prim;   u_long primaddr;
    sceGsRgbaq rgbaq; u_long rgbaqaddr;
    sceGsXyz xyz2a;   u_long xyz2aaddr;
    sceGsXyz xyz2b;   u_long xyz2baddr;
    sceGsTest testb;  u_long testbaddr;
} sceGsClear __attribute__((aligned(16)));

typedef struct {
    sceGsDispEnv disp[2];
    u_long giftag0[2];
    sceGsDrawEnv1 draw0;
    sceGsClear clear0;
    u_long giftag1[2];
    sceGsDrawEnv1 draw1;
    sceGsClear clear1;
} sceGsDBuff;

typedef struct {
    sceGsDispEnv disp[2];
    u_long giftag0[2];
    sceGsDrawEnv1 draw01;
    sceGsDrawEnv2 draw02;
    sceGsClear clear0;
    u_long giftag1[2];
    sceGsDrawEnv1 draw11;
    sceGsDrawEnv2 draw12;
    sceGsClear clear1;
} sceGsDBuffDc;

typedef struct {
    u_long giftag0[2];
    sceGsBitbltbuf bitbltbuf; u_long bitbltbufaddr;
    sceGsTrxpos trxpos;       u_long trxposaddr;
    sceGsTrxreg trxreg;       u_long trxregaddr;
    sceGsTrxdir trxdir;       u_long trxdiraddr;
    u_long giftag1[2];
} sceGsLoadImage __attribute__((aligned(16)));

typedef struct {
    u_int vifcode[4];
    u_long giftag[2];
    sceGsBitbltbuf bitbltbuf; u_long bitbltbufaddr;
    sceGsTrxpos trxpos;       u_long trxposaddr;
    sceGsTrxreg trxreg;       u_long trxregaddr;
    sceGsFinish finish;       u_long finishaddr;
    sceGsTrxdir trxdir;       u_long trxdiraddr;
} sceGsStoreImage __attribute__((aligned(16)));

int  sceGsResetGraph(short mode, short inter, short omode, short ffmode);
void sceGsResetPath(void);
int  sceGsSyncV(int mode);
int  sceGsSyncPath(int mode, u_short timeout);
int (*sceGsSyncVCallback(int (*func)(int)))(int);
void sceGsSetDefDBuffDc(sceGsDBuffDc *db, short psm, short w, short h, short ztest, short zpsm, short clear);
int  sceGsSwapDBuffDc(sceGsDBuffDc *db, int id);
int  sceGsSetDefLoadImage(sceGsLoadImage *lp, short dbp, short dbw, short dpsm, short x, short y, short w, short h);
int  sceGsExecLoadImage(sceGsLoadImage *lp, u_long128 *srcaddr);
int  sceGsSetDefStoreImage(sceGsStoreImage *sp, short sbp, short sbw, short spsm, short x, short y, short w, short h);
int  sceGsExecStoreImage(sceGsStoreImage *sp, u_long128 *dstaddr);
