/*
 * GS high-level emulation: the game's GIF/VIF packets drive a GS register
 * model whose primitives are drawn with vitaGL.
 *
 *   hwreg.c (DMA) -> vif1.c (VIF1 + VU1 microprograms) -> gs_gif.c (GIF paths)
 *   gs_gif.c -> gs_mem.c (local memory, transfers) / gs_draw.c (vitaGL)
 */
#pragma once
#include <stdint.h>
#include "eetypes.h"

enum { GIF_PATH1, GIF_PATH2, GIF_PATH3 };

/* ---------------------------------------------------------------- vif1.c */
void vif1_transfer(const void *words, u_int nwords);
void vif1_reset(void);
extern float vu1_mem[1024][4];
#define VU1_MEMI(a) ((u_int *)vu1_mem[(a) & 0x3ff])
u_int vu1_top(void);
u_int vu1_itop(void);

/* one VU1 strip handed from vu1_hle.c to the GPU */
typedef struct {
    u_int prim;                 /* GS PRIM from the strip's GIF tag */
    u_int func;                 /* colour function 0..10 */
    float consts[15][4];        /* VU1 data memory 884..898 */
    const float (*verts)[4];    /* n * 4 qwords (VU1_STRIP_BUF) */
    u_int n;
} Vu1Batch;

/* ---------------------------------------------------------------- gs_gif.c */
void gif_transfer(int path, const void *qwords, u_int qwc);
void gs_write_reg(u_int reg, uint64_t value);
void gs_reset(void);

typedef struct {
    uint64_t tex0, tex1, clamp, xyoffset, miptbp1, miptbp2;
    uint64_t scissor, alpha, test, fba, frame, zbuf;
} GsContext;

typedef struct {
    uint64_t prim, prmode, prmodecont, rgbaq, st, uv, fog;
    float q;
    GsContext ctx[2];
    uint64_t texclut, texa, fogcol, dimx, dthe, colclamp, pabe, scanmsk;
    uint64_t bitbltbuf, trxpos, trxreg, trxdir;
    uint32_t cbp0, cbp1;
} GsRegs;

extern GsRegs gs;

/* bit-field helpers */
#define GS_BITS(v, lo, n) ((uint32_t)(((uint64_t)(v) >> (lo)) & ((1ull << (n)) - 1)))

/* ---------------------------------------------------------------- gs_mem.c */
#define GS_MEM_WORDS (1024 * 1024)
extern uint32_t gs_mem[GS_MEM_WORDS];

void gs_mem_transfer_begin(void);              /* TRXDIR written */
void gs_mem_transfer_data(const void *qwords, u_int qwc);
void gs_mem_store(void *dst, u_int qwc);       /* local -> host */
uint32_t gs_mem_read_pixel(u_int psm, u_int bp, u_int bw, u_int x, u_int y);
void gs_mem_write_pixel(u_int psm, u_int bp, u_int bw, u_int x, u_int y, uint32_t v);
void gs_clut_load(uint64_t tex0);
uint32_t gs_clut_hash(uint64_t tex0);
/* decode a TEX0 rectangle to RGBA8 (GS alpha scale, 0x80 = 1.0) */
void gs_decode_texture(uint64_t tex0, uint64_t texa, u_int w, u_int h, uint32_t *out);
/* blocks (64-word units) touched by a w*h image at bp/bw/psm */
void gs_mem_block_range(u_int psm, u_int bp, u_int bw, u_int w, u_int h, u_int *first, u_int *last);
int gs_psm_bpp(u_int psm);

/* ---------------------------------------------------------------- gs_draw.c */
typedef struct {
    float x, y, z;           /* GS pixel coordinates, z normalised 0..1 */
    float s, t, q;
    uint8_t r, g, b, a;
    float fog;               /* 0 = fog colour, 1 = no fog */
} GsVertex;

void gs_draw_init(void);
unsigned gs_frame_count(void);

/* GS work can arrive from any EE thread; this serializes it (recursive) */
void gs_lock(void);
void gs_unlock(void);
void gs_draw_prim(int type, const GsVertex *v, int n);  /* triangles / sprite pairs / lines / points */
void gs_draw_flush(void);
void gs_draw_state_changed(void);
/* host->local transfer touched these blocks */
void gs_draw_mem_written(u_int psm, u_int bp, u_int bw, u_int x, u_int y, u_int w, u_int h);
/* local->host transfer will read this rectangle */
void gs_draw_mem_read(u_int psm, u_int bp, u_int bw, u_int x, u_int y, u_int w, u_int h);
void gs_draw_vu1(const Vu1Batch *b);
void gs_draw_present(u_int fbp, u_int fbw, u_int psm, u_int w, u_int h);

/* primitive types handed to gs_draw_prim */
enum { GS_DRAW_POINTS, GS_DRAW_LINES, GS_DRAW_TRIS, GS_DRAW_SPRITES };
