/*
 * GS local memory (4 MB) with the hardware's swizzled layouts, image
 * transfers and the CLUT buffer. Textures are decoded from here; render
 * targets live on the Vita GPU and are kept coherent by gs_draw.c.
 */
#include <string.h>
#include "gs/gs.h"
#include "libgraph.h"
#include "recvx_platform.h"

uint32_t gs_mem[GS_MEM_WORDS] __attribute__((aligned(64)));

/* ---------------------------------------------------------------- layouts */

static const uint8_t block32[4][8] = {
    {  0,  1,  4,  5, 16, 17, 20, 21 },
    {  2,  3,  6,  7, 18, 19, 22, 23 },
    {  8,  9, 12, 13, 24, 25, 28, 29 },
    { 10, 11, 14, 15, 26, 27, 30, 31 },
};
static const uint8_t block16[8][4] = {
    {  0,  2,  8, 10 }, {  1,  3,  9, 11 }, {  4,  6, 12, 14 }, {  5,  7, 13, 15 },
    { 16, 18, 24, 26 }, { 17, 19, 25, 27 }, { 20, 22, 28, 30 }, { 21, 23, 29, 31 },
};
static const uint8_t block16s[8][4] = {
    {  0,  2, 16, 18 }, {  1,  3, 17, 19 }, {  8, 10, 24, 26 }, {  9, 11, 25, 27 },
    {  4,  6, 20, 22 }, {  5,  7, 21, 23 }, { 12, 14, 28, 30 }, { 13, 15, 29, 31 },
};
/* block8 == block32, block4 == block16 */

static const uint8_t column32[8][8] = {
    {  0,  1,  4,  5,  8,  9, 12, 13 }, {  2,  3,  6,  7, 10, 11, 14, 15 },
    { 16, 17, 20, 21, 24, 25, 28, 29 }, { 18, 19, 22, 23, 26, 27, 30, 31 },
    { 32, 33, 36, 37, 40, 41, 44, 45 }, { 34, 35, 38, 39, 42, 43, 46, 47 },
    { 48, 49, 52, 53, 56, 57, 60, 61 }, { 50, 51, 54, 55, 58, 59, 62, 63 },
};
static const uint8_t column16[8][16] = {
    {   0,   2,   8,  10,  16,  18,  24,  26,   1,   3,   9,  11,  17,  19,  25,  27 },
    {   4,   6,  12,  14,  20,  22,  28,  30,   5,   7,  13,  15,  21,  23,  29,  31 },
    {  32,  34,  40,  42,  48,  50,  56,  58,  33,  35,  41,  43,  49,  51,  57,  59 },
    {  36,  38,  44,  46,  52,  54,  60,  62,  37,  39,  45,  47,  53,  55,  61,  63 },
    {  64,  66,  72,  74,  80,  82,  88,  90,  65,  67,  73,  75,  81,  83,  89,  91 },
    {  68,  70,  76,  78,  84,  86,  92,  94,  69,  71,  77,  79,  85,  87,  93,  95 },
    {  96,  98, 104, 106, 112, 114, 120, 122,  97,  99, 105, 107, 113, 115, 121, 123 },
    { 100, 102, 108, 110, 116, 118, 124, 126, 101, 103, 109, 111, 117, 119, 125, 127 },
};
static const uint8_t column8[16][16] = {
    {   0,   4,  16,  20,  32,  36,  48,  52,   2,   6,  18,  22,  34,  38,  50,  54 },
    {   8,  12,  24,  28,  40,  44,  56,  60,  10,  14,  26,  30,  42,  46,  58,  62 },
    {  33,  37,  49,  53,   1,   5,  17,  21,  35,  39,  51,  55,   3,   7,  19,  23 },
    {  41,  45,  57,  61,   9,  13,  25,  29,  43,  47,  59,  63,  11,  15,  27,  31 },
    {  96, 100, 112, 116,  64,  68,  80,  84,  98, 102, 114, 118,  66,  70,  82,  86 },
    { 104, 108, 120, 124,  72,  76,  88,  92, 106, 110, 122, 126,  74,  78,  90,  94 },
    {  65,  69,  81,  85,  97, 101, 113, 117,  67,  71,  83,  87,  99, 103, 115, 119 },
    {  73,  77,  89,  93, 105, 109, 121, 125,  75,  79,  91,  95, 107, 111, 123, 127 },
    { 128, 132, 144, 148, 160, 164, 176, 180, 130, 134, 146, 150, 162, 166, 178, 182 },
    { 136, 140, 152, 156, 168, 172, 184, 188, 138, 142, 154, 158, 170, 174, 186, 190 },
    { 161, 165, 177, 181, 129, 133, 145, 149, 163, 167, 179, 183, 131, 135, 147, 151 },
    { 169, 173, 185, 189, 137, 141, 153, 157, 171, 175, 187, 191, 139, 143, 155, 159 },
    { 224, 228, 240, 244, 192, 196, 208, 212, 226, 230, 242, 246, 194, 198, 210, 214 },
    { 232, 236, 248, 252, 200, 204, 216, 220, 234, 238, 250, 254, 202, 206, 218, 222 },
    { 193, 197, 209, 213, 225, 229, 241, 245, 195, 199, 211, 215, 227, 231, 243, 247 },
    { 201, 205, 217, 221, 233, 237, 249, 253, 203, 207, 219, 223, 235, 239, 251, 255 },
};
/* PSMT4 columns; rows 8-15 repeat rows 0-7 one column pair (256 nibbles) on */
static const uint16_t column4_lo[8][32] = {
    {   0,   8,  32,  40,  64,  72,  96, 104,   2,  10,  34,  42,  66,  74,  98, 106,
        4,  12,  36,  44,  68,  76, 100, 108,   6,  14,  38,  46,  70,  78, 102, 110 },
    {  16,  24,  48,  56,  80,  88, 112, 120,  18,  26,  50,  58,  82,  90, 114, 122,
       20,  28,  52,  60,  84,  92, 116, 124,  22,  30,  54,  62,  86,  94, 118, 126 },
    {  65,  73,  97, 105,   1,   9,  33,  41,  67,  75,  99, 107,   3,  11,  35,  43,
       69,  77, 101, 109,   5,  13,  37,  45,  71,  79, 103, 111,   7,  15,  39,  47 },
    {  81,  89, 113, 121,  17,  25,  49,  57,  83,  91, 115, 123,  19,  27,  51,  59,
       85,  93, 117, 125,  21,  29,  53,  61,  87,  95, 119, 127,  23,  31,  55,  63 },
    { 192, 200, 224, 232, 128, 136, 160, 168, 194, 202, 226, 234, 130, 138, 162, 170,
      196, 204, 228, 236, 132, 140, 164, 172, 198, 206, 230, 238, 134, 142, 166, 174 },
    { 208, 216, 240, 248, 144, 152, 176, 184, 210, 218, 242, 250, 146, 154, 178, 186,
      212, 220, 244, 252, 148, 156, 180, 188, 214, 222, 246, 254, 150, 158, 182, 190 },
    { 129, 137, 161, 169, 193, 201, 225, 233, 131, 139, 163, 171, 195, 203, 227, 235,
      133, 141, 165, 173, 197, 205, 229, 237, 135, 143, 167, 175, 199, 207, 231, 239 },
    { 145, 153, 177, 185, 209, 217, 241, 249, 147, 155, 179, 187, 211, 219, 243, 251,
      149, 157, 181, 189, 213, 221, 245, 253, 151, 159, 183, 191, 215, 223, 247, 255 },
};

#define WMASK (GS_MEM_WORDS - 1)

static inline int is_z(u_int psm) { return (psm & 0x30) == 0x30; }

/* word address of a 32-bit pixel (PSMCT32/24, PSMT8H/4HL/4HH, PSMZ32/24) */
static inline u_int addr32(u_int psm, u_int bp, u_int bw, u_int x, u_int y)
{
    u_int page = (x >> 6) + (y >> 5) * bw;
    u_int blk = block32[(y >> 3) & 3][(x >> 3) & 7] ^ (is_z(psm) ? 0x18 : 0);
    return ((bp + page * 32 + blk) * 64 + column32[y & 7][x & 7]) & WMASK;
}

/* halfword address (PSMCT16/16S, PSMZ16/16S) */
static inline u_int addr16(u_int psm, u_int bp, u_int bw, u_int x, u_int y)
{
    u_int page = (x >> 6) + (y >> 6) * bw;
    u_int blk = (psm == SCE_GS_PSMCT16S || psm == SCE_GS_PSMZ16S)
        ? block16s[(y >> 3) & 7][(x >> 4) & 3] : block16[(y >> 3) & 7][(x >> 4) & 3];
    if (is_z(psm)) blk ^= 0x18;
    return ((bp + page * 32 + blk) * 128 + column16[y & 7][x & 15]) & (WMASK * 2 + 1);
}

/* byte address (PSMT8) */
static inline u_int addr8(u_int bp, u_int bw, u_int x, u_int y)
{
    u_int pw = bw >> 1 ? bw >> 1 : 1;
    u_int page = (x >> 7) + (y >> 6) * pw;
    u_int blk = block32[(y >> 4) & 3][(x >> 4) & 7];
    return ((bp + page * 32 + blk) * 256 + column8[y & 15][x & 15]) & (WMASK * 4 + 3);
}

/* nibble address (PSMT4) */
static inline u_int addr4(u_int bp, u_int bw, u_int x, u_int y)
{
    u_int pw = bw >> 1 ? bw >> 1 : 1;
    u_int page = (x >> 7) + (y >> 7) * pw;
    u_int blk = block16[(y >> 4) & 7][(x >> 5) & 3];
    u_int col = column4_lo[y & 7][x & 31] + ((y & 8) ? 256 : 0);
    return ((bp + page * 32 + blk) * 512 + col) & (WMASK * 8 + 7);
}

int gs_psm_bpp(u_int psm)
{
    switch (psm) {
    case SCE_GS_PSMCT32: case SCE_GS_PSMZ32: case SCE_GS_PSMT8H:
    case SCE_GS_PSMT4HL: case SCE_GS_PSMT4HH: return 32;
    case SCE_GS_PSMCT24: case SCE_GS_PSMZ24: return 24;
    case SCE_GS_PSMCT16: case SCE_GS_PSMCT16S: case SCE_GS_PSMZ16: case SCE_GS_PSMZ16S: return 16;
    case SCE_GS_PSMT8: return 8;
    case SCE_GS_PSMT4: return 4;
    }
    return 32;
}

uint32_t gs_mem_read_pixel(u_int psm, u_int bp, u_int bw, u_int x, u_int y)
{
    switch (psm) {
    case SCE_GS_PSMCT32: case SCE_GS_PSMZ32:
        return gs_mem[addr32(psm, bp, bw, x, y)];
    case SCE_GS_PSMCT24: case SCE_GS_PSMZ24:
        return gs_mem[addr32(psm, bp, bw, x, y)] & 0xffffff;
    case SCE_GS_PSMT8H:
        return gs_mem[addr32(psm, bp, bw, x, y)] >> 24;
    case SCE_GS_PSMT4HL:
        return (gs_mem[addr32(psm, bp, bw, x, y)] >> 24) & 15;
    case SCE_GS_PSMT4HH:
        return gs_mem[addr32(psm, bp, bw, x, y)] >> 28;
    case SCE_GS_PSMCT16: case SCE_GS_PSMCT16S: case SCE_GS_PSMZ16: case SCE_GS_PSMZ16S:
        return ((uint16_t *)gs_mem)[addr16(psm, bp, bw, x, y)];
    case SCE_GS_PSMT8:
        return ((uint8_t *)gs_mem)[addr8(bp, bw, x, y)];
    case SCE_GS_PSMT4: {
        u_int a = addr4(bp, bw, x, y);
        return (((uint8_t *)gs_mem)[a >> 1] >> ((a & 1) * 4)) & 15;
    }
    }
    return 0;
}

void gs_mem_write_pixel(u_int psm, u_int bp, u_int bw, u_int x, u_int y, uint32_t v)
{
    uint32_t *w;
    switch (psm) {
    case SCE_GS_PSMCT32: case SCE_GS_PSMZ32:
        gs_mem[addr32(psm, bp, bw, x, y)] = v;
        break;
    case SCE_GS_PSMCT24: case SCE_GS_PSMZ24:
        w = &gs_mem[addr32(psm, bp, bw, x, y)];
        *w = (*w & 0xff000000) | (v & 0xffffff);
        break;
    case SCE_GS_PSMT8H:
        w = &gs_mem[addr32(psm, bp, bw, x, y)];
        *w = (*w & 0x00ffffff) | (v << 24);
        break;
    case SCE_GS_PSMT4HL:
        w = &gs_mem[addr32(psm, bp, bw, x, y)];
        *w = (*w & 0xf0ffffff) | ((v & 15) << 24);
        break;
    case SCE_GS_PSMT4HH:
        w = &gs_mem[addr32(psm, bp, bw, x, y)];
        *w = (*w & 0x0fffffff) | (v << 28);
        break;
    case SCE_GS_PSMCT16: case SCE_GS_PSMCT16S: case SCE_GS_PSMZ16: case SCE_GS_PSMZ16S:
        ((uint16_t *)gs_mem)[addr16(psm, bp, bw, x, y)] = v;
        break;
    case SCE_GS_PSMT8:
        ((uint8_t *)gs_mem)[addr8(bp, bw, x, y)] = v;
        break;
    case SCE_GS_PSMT4: {
        u_int a = addr4(bp, bw, x, y);
        uint8_t *b = &((uint8_t *)gs_mem)[a >> 1];
        *b = (a & 1) ? (*b & 0x0f) | (v << 4) : (*b & 0xf0) | (v & 15);
        break;
    }
    }
}

void gs_mem_block_range(u_int psm, u_int bp, u_int bw, u_int w, u_int h, u_int *first, u_int *last)
{
    /* page granular: (page width, page height) per format */
    u_int pw = 64, ph = 32;
    switch (gs_psm_bpp(psm)) {
    case 16: ph = 64; break;
    case 8: pw = 128; ph = 64; break;
    case 4: pw = 128; ph = 128; break;
    }
    u_int ppr = bw * 64 / pw;
    if (!ppr) ppr = 1;
    u_int rows = (h + ph - 1) / ph, cols = (w + pw - 1) / pw;
    u_int pages = (rows - 1) * ppr + (cols > ppr ? cols : ppr);
    *first = bp & ~31u;
    *last = bp + pages * 32 - 1;
}

/* -------------------------------------------------------------- transfers */

static struct {
    u_int active, psm, bp, bw, x0, y0, w, h, x, y;
    uint32_t acc;
    int acc_bits;
} xfer;

static void xfer_put(uint32_t v)
{
    if (xfer.y >= xfer.h) return;
    gs_mem_write_pixel(xfer.psm, xfer.bp, xfer.bw, xfer.x0 + xfer.x, xfer.y0 + xfer.y, v);
    if (++xfer.x == xfer.w) { xfer.x = 0; xfer.y++; }
}

static void xfer_finish(void)
{
    if (!xfer.active) return;
    xfer.active = 0;
    gs_draw_mem_written(xfer.psm, xfer.bp, xfer.bw, xfer.x0, xfer.y0, xfer.w, xfer.h);
}

static void local_to_local(void)
{
    u_int sbp = GS_BITS(gs.bitbltbuf, 0, 14), sbw = GS_BITS(gs.bitbltbuf, 16, 6);
    u_int spsm = GS_BITS(gs.bitbltbuf, 24, 6);
    u_int dbp = GS_BITS(gs.bitbltbuf, 32, 14), dbw = GS_BITS(gs.bitbltbuf, 48, 6);
    u_int dpsm = GS_BITS(gs.bitbltbuf, 56, 6);
    u_int sx = GS_BITS(gs.trxpos, 0, 11), sy = GS_BITS(gs.trxpos, 16, 11);
    u_int dx = GS_BITS(gs.trxpos, 32, 11), dy = GS_BITS(gs.trxpos, 48, 11);
    u_int w = GS_BITS(gs.trxreg, 0, 12), h = GS_BITS(gs.trxreg, 32, 12);

    gs_draw_mem_read(spsm, sbp, sbw, sx, sy, w, h);
    for (u_int y = 0; y < h; y++)
        for (u_int x = 0; x < w; x++)
            gs_mem_write_pixel(dpsm, dbp, dbw, dx + x, dy + y,
                               gs_mem_read_pixel(spsm, sbp, sbw, sx + x, sy + y));
    gs_draw_mem_written(dpsm, dbp, dbw, dx, dy, w, h);
}

void gs_mem_transfer_begin(void)
{
    xfer_finish();
    switch (GS_BITS(gs.trxdir, 0, 2)) {
    case 0: /* host -> local */
        xfer.active = 1;
        xfer.bp = GS_BITS(gs.bitbltbuf, 32, 14);
        xfer.bw = GS_BITS(gs.bitbltbuf, 48, 6);
        xfer.psm = GS_BITS(gs.bitbltbuf, 56, 6);
        xfer.x0 = GS_BITS(gs.trxpos, 32, 11);
        xfer.y0 = GS_BITS(gs.trxpos, 48, 11);
        xfer.w = GS_BITS(gs.trxreg, 0, 12);
        xfer.h = GS_BITS(gs.trxreg, 32, 12);
        xfer.x = xfer.y = 0;
        xfer.acc = 0;
        xfer.acc_bits = 0;
        if (GS_BITS(gs.trxpos, 59, 2))
            RECVX_LOG_ONCE("gs: TRXPOS.DIR %d ignored", GS_BITS(gs.trxpos, 59, 2));
        break;
    case 2:
        local_to_local();
        break;
    }
}

void gs_mem_transfer_data(const void *qwords, u_int qwc)
{
    if (!xfer.active) {
        RECVX_LOG_ONCE("gs: IMAGE data with no host->local transfer set up");
        return;
    }
    const uint8_t *b = qwords;
    u_int n = qwc * 16;
    int bpp = gs_psm_bpp(xfer.psm);

    switch (bpp) {
    case 32: {
        const uint32_t *w = qwords;
        for (u_int i = 0; i < qwc * 4; i++) xfer_put(w[i]);
        break;
    }
    case 16: {
        const uint16_t *h = qwords;
        for (u_int i = 0; i < qwc * 8; i++) xfer_put(h[i]);
        break;
    }
    case 8:
        for (u_int i = 0; i < n; i++) xfer_put(b[i]);
        break;
    case 4:
        for (u_int i = 0; i < n; i++) { xfer_put(b[i] & 15); xfer_put(b[i] >> 4); }
        break;
    case 24:
        for (u_int i = 0; i < n; i++) {
            xfer.acc |= (uint32_t)b[i] << xfer.acc_bits;
            if ((xfer.acc_bits += 8) == 24) { xfer_put(xfer.acc); xfer.acc = 0; xfer.acc_bits = 0; }
        }
        break;
    }
    if (xfer.y >= xfer.h) xfer_finish();
}

static void mem_store_(void *dst, u_int qwc)
{
    u_int sbp = GS_BITS(gs.bitbltbuf, 0, 14), sbw = GS_BITS(gs.bitbltbuf, 16, 6);
    u_int spsm = GS_BITS(gs.bitbltbuf, 24, 6);
    u_int sx = GS_BITS(gs.trxpos, 0, 11), sy = GS_BITS(gs.trxpos, 16, 11);
    u_int w = GS_BITS(gs.trxreg, 0, 12), h = GS_BITS(gs.trxreg, 32, 12);

    if (gs_psm_bpp(spsm) != 32) {
        RECVX_LOG("gs: local->host of psm %x not supported", spsm);
        return;
    }
    gs_draw_mem_read(spsm, sbp, sbw, sx, sy, w, h);
    uint32_t *out = dst;
    u_int n = qwc * 4;
    for (u_int y = 0; y < h; y++)
        for (u_int x = 0; x < w; x++) {
            if (!n--) return;
            *out++ = gs_mem_read_pixel(spsm, sbp, sbw, sx + x, sy + y);
        }
}

void gs_mem_store(void *dst, u_int qwc)
{
    gs_lock();
    mem_store_(dst, qwc);
    gs_unlock();
}

/* ------------------------------------------------------------------- CLUT */

static uint32_t clut[512];   /* CT32 entries, or raw CT16 values */
static u_int clut_cpsm;

void gs_clut_load(uint64_t tex0)
{
    u_int psm = GS_BITS(tex0, 20, 6), cbp = GS_BITS(tex0, 37, 14);
    u_int cpsm = GS_BITS(tex0, 51, 4), csm = GS_BITS(tex0, 55, 1);
    u_int csa = GS_BITS(tex0, 56, 5), cld = GS_BITS(tex0, 61, 3);
    int t8 = psm == SCE_GS_PSMT8 || psm == SCE_GS_PSMT8H;

    switch (cld) {
    case 0: return;
    case 1: break;
    case 2: gs.cbp0 = cbp; break;
    case 3: gs.cbp1 = cbp; break;
    case 4: if (gs.cbp0 == cbp) return; gs.cbp0 = cbp; break;
    case 5: if (gs.cbp1 == cbp) return; gs.cbp1 = cbp; break;
    default: return;
    }
    if (!t8 && psm != SCE_GS_PSMT4 && psm != SCE_GS_PSMT4HL && psm != SCE_GS_PSMT4HH)
        return;

    u_int count = t8 ? 256 : 16, base = t8 ? 0 : csa * 16;
    clut_cpsm = cpsm;
    gs_draw_mem_read(cpsm, cbp, 1, 0, 0, t8 ? 16 : 8, t8 ? 16 : 2);

    for (u_int e = 0; e < count; e++) {
        u_int x, y;
        if (csm == 0) {
            /* CSM1: 8x2 tiles, two tiles per 16-pixel row */
            u_int tile = e >> 4;
            x = (tile & 1) * 8 + (e & 7);
            y = (tile >> 1) * 2 + ((e >> 3) & 1);
            u_int v = gs_mem_read_pixel(cpsm, cbp, 1, x, y);
            clut[(base + e) & 511] = v;
        } else {
            /* CSM2: a linear strip at TEXCLUT (COU, COV) in a CBW-wide buffer */
            u_int cbw = GS_BITS(gs.texclut, 0, 6), cou = GS_BITS(gs.texclut, 6, 6);
            u_int cov = GS_BITS(gs.texclut, 12, 10);
            clut[(base + e) & 511] = gs_mem_read_pixel(cpsm, cbp, cbw, cou * 16 + e, cov);
        }
    }
}

/* ----------------------------------------------------------------- decode */

static inline uint32_t expand16(uint32_t v, uint64_t texa)
{
    u_int ta0 = GS_BITS(texa, 0, 8), aem = GS_BITS(texa, 15, 1), ta1 = GS_BITS(texa, 32, 8);
    uint32_t r = (v & 0x1f) << 3, g = ((v >> 5) & 0x1f) << 3, b = ((v >> 10) & 0x1f) << 3;
    uint32_t a = (v & 0x8000) ? ta1 : (aem && !(v & 0x7fff)) ? 0 : ta0;
    return r | g << 8 | b << 16 | a << 24;
}

static inline uint32_t expand24(uint32_t v, uint64_t texa)
{
    u_int ta0 = GS_BITS(texa, 0, 8), aem = GS_BITS(texa, 15, 1);
    return (v & 0xffffff) | ((aem && !(v & 0xffffff)) ? 0 : ta0) << 24;
}

void gs_decode_texture(uint64_t tex0, uint64_t texa, u_int w, u_int h, uint32_t *out)
{
    u_int tbp = GS_BITS(tex0, 0, 14), tbw = GS_BITS(tex0, 14, 6), psm = GS_BITS(tex0, 20, 6);
    u_int csa = GS_BITS(tex0, 56, 5);
    u_int cbase = (psm == SCE_GS_PSMT8 || psm == SCE_GS_PSMT8H) ? 0 : csa * 16;
    int cpsm16 = clut_cpsm != SCE_GS_PSMCT32 && clut_cpsm != SCE_GS_PSMCT24;

    if (!tbw) tbw = 1;
    for (u_int y = 0; y < h; y++)
        for (u_int x = 0; x < w; x++) {
            uint32_t v = gs_mem_read_pixel(psm, tbp, tbw, x, y), c;
            switch (psm) {
            case SCE_GS_PSMCT32: c = v; break;
            case SCE_GS_PSMCT24: c = expand24(v, texa); break;
            case SCE_GS_PSMCT16: case SCE_GS_PSMCT16S: c = expand16(v, texa); break;
            case SCE_GS_PSMT8: case SCE_GS_PSMT8H: case SCE_GS_PSMT4:
            case SCE_GS_PSMT4HL: case SCE_GS_PSMT4HH:
                c = clut[(cbase + v) & 511];
                if (cpsm16) c = expand16(c, texa);
                break;
            default: c = v; break;
            }
            *out++ = c;
        }
}

/* content hash of the CLUT entries a paletted TEX0 would use (texture cache key) */
uint32_t gs_clut_hash(uint64_t tex0)
{
    u_int psm = GS_BITS(tex0, 20, 6), csa = GS_BITS(tex0, 56, 5);
    int t8 = psm == SCE_GS_PSMT8 || psm == SCE_GS_PSMT8H;
    u_int base = t8 ? 0 : csa * 16, count = t8 ? 256 : 16;
    uint32_t h = 2166136261u ^ clut_cpsm;
    for (u_int e = 0; e < count; e++) {
        h ^= clut[(base + e) & 511];
        h *= 16777619u;
    }
    return h;
}
