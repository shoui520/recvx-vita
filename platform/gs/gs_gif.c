/*
 * GIF tag processing and the GS register file: PACKED/REGLIST/IMAGE data
 * from the three paths becomes register writes, vertex kicks and image
 * transfers.
 */
#include <string.h>
#include "gs/gs.h"
#include "libgraph.h"
#include "recvx_platform.h"

GsRegs gs;

/* ------------------------------------------------------------ vertex queue */

typedef struct {
    uint64_t xyz, rgbaq, st, uv, fog;
    float q;
} RawVertex;

static RawVertex vq[3];
static int vq_count;
static int fan_first_valid;
static RawVertex fan_first;

static inline uint64_t cur_prim_attrs(void)
{
    /* PRMODECONT.AC = 1: attributes come from PRIM, else from PRMODE */
    return GS_BITS(gs.prmodecont, 0, 1) ? gs.prim : gs.prmode;
}

static void to_vertex(const RawVertex *rv, GsVertex *v, const GsContext *c, uint64_t attrs)
{
    u_int ofx = GS_BITS(c->xyoffset, 0, 16), ofy = GS_BITS(c->xyoffset, 32, 16);
    u_int zpsm = GS_BITS(c->zbuf, 24, 4) | 0x30;
    uint32_t z = (uint32_t)(rv->xyz >> 32);

    v->x = ((float)GS_BITS(rv->xyz, 0, 16) - (float)ofx) * (1.0f / 16.0f);
    v->y = ((float)GS_BITS(rv->xyz, 16, 16) - (float)ofy) * (1.0f / 16.0f);
    switch (zpsm) {
    case SCE_GS_PSMZ16: case SCE_GS_PSMZ16S: v->z = (float)(z & 0xffff) * (1.0f / 65536.0f); break;
    case SCE_GS_PSMZ24: v->z = (float)(z & 0xffffff) * (1.0f / 16777216.0f); break;
    default: v->z = (float)z * (1.0f / 4294967296.0f); break;
    }

    if (GS_BITS(attrs, 8, 1)) { /* FST: UV in texels (10.4) */
        u_int tw = GS_BITS(c->tex0, 26, 4), th = GS_BITS(c->tex0, 30, 4);
        v->s = (float)GS_BITS(rv->uv, 0, 14) * (1.0f / 16.0f) / (float)(1u << tw);
        v->t = (float)GS_BITS(rv->uv, 16, 14) * (1.0f / 16.0f) / (float)(1u << th);
        v->q = 1.0f;
    } else {
        union { uint32_t u; float f; } s, t;
        s.u = (uint32_t)rv->st;
        t.u = (uint32_t)(rv->st >> 32);
        v->s = s.f;
        v->t = t.f;
        v->q = rv->q;
    }
    v->r = GS_BITS(rv->rgbaq, 0, 8);
    v->g = GS_BITS(rv->rgbaq, 8, 8);
    v->b = GS_BITS(rv->rgbaq, 16, 8);
    v->a = GS_BITS(rv->rgbaq, 24, 8);
    v->fog = (float)GS_BITS(rv->fog, 56, 8) * (1.0f / 255.0f);
}

static void emit(int type, RawVertex *rv, int n)
{
    uint64_t attrs = cur_prim_attrs();
    const GsContext *c = &gs.ctx[GS_BITS(attrs, 9, 1)];
    GsVertex v[3];

    for (int i = 0; i < n; i++) to_vertex(&rv[i], &v[i], c, attrs);
    if (!GS_BITS(attrs, 3, 1)) { /* flat shading: last vertex's colour */
        for (int i = 0; i < n - 1; i++) {
            v[i].r = v[n - 1].r; v[i].g = v[n - 1].g; v[i].b = v[n - 1].b; v[i].a = v[n - 1].a;
        }
    }
    gs_draw_prim(type, v, n);
}

static void vertex_kick(uint64_t xyz, int draw)
{
    RawVertex *rv = &vq[vq_count < 3 ? vq_count : 2];
    rv->xyz = xyz;
    rv->rgbaq = gs.rgbaq;
    rv->q = gs.q;
    rv->st = gs.st;
    rv->uv = gs.uv;
    rv->fog = gs.fog;
    vq_count++;

    u_int prim = GS_BITS(gs.prim, 0, 3);
    switch (prim) {
    case SCE_GS_PRIM_POINT:
        if (draw) emit(GS_DRAW_POINTS, vq, 1);
        vq_count = 0;
        break;
    case SCE_GS_PRIM_LINE:
        if (vq_count == 2) { if (draw) emit(GS_DRAW_LINES, vq, 2); vq_count = 0; }
        break;
    case SCE_GS_PRIM_LINESTRIP:
        if (vq_count == 2) { if (draw) emit(GS_DRAW_LINES, vq, 2); vq[0] = vq[1]; vq_count = 1; }
        break;
    case SCE_GS_PRIM_TRI:
        if (vq_count == 3) { if (draw) emit(GS_DRAW_TRIS, vq, 3); vq_count = 0; }
        break;
    case SCE_GS_PRIM_TRISTRIP:
        if (vq_count == 3) { if (draw) emit(GS_DRAW_TRIS, vq, 3); vq[0] = vq[1]; vq[1] = vq[2]; vq_count = 2; }
        break;
    case SCE_GS_PRIM_TRIFAN:
        if (!fan_first_valid) { fan_first = vq[0]; fan_first_valid = 1; }
        if (vq_count == 3) {
            if (draw) emit(GS_DRAW_TRIS, vq, 3);
            vq[0] = fan_first; vq[1] = vq[2]; vq_count = 2;
        }
        break;
    case SCE_GS_PRIM_SPRITE:
        if (vq_count == 2) { if (draw) emit(GS_DRAW_SPRITES, vq, 2); vq_count = 0; }
        break;
    default:
        vq_count = 0;
        break;
    }
}

/* -------------------------------------------------------------- registers */

static void set_ctx_reg(int ctx, u_int reg, uint64_t v)
{
    GsContext *c = &gs.ctx[ctx];
    switch (reg) {
    case SCE_GS_TEX0_1:
        c->tex0 = v;
        gs_clut_load(v);
        break;
    case SCE_GS_CLAMP_1: c->clamp = v; break;
    case SCE_GS_TEX1_1: c->tex1 = v; break;
    case SCE_GS_TEX2_1: {
        /* TEX2 replaces PSM and the CLUT fields of TEX0 */
        const uint64_t m = (0x3full << 20) | 0xffffffe000000000ull;
        c->tex0 = (c->tex0 & ~m) | (v & m);
        gs_clut_load(c->tex0);
        break;
    }
    case SCE_GS_XYOFFSET_1: c->xyoffset = v; break;
    case SCE_GS_MIPTBP1_1: c->miptbp1 = v; break;
    case SCE_GS_MIPTBP2_1: c->miptbp2 = v; break;
    case SCE_GS_SCISSOR_1: c->scissor = v; break;
    case SCE_GS_ALPHA_1: c->alpha = v; break;
    case SCE_GS_TEST_1: c->test = v; break;
    case SCE_GS_FBA_1: c->fba = v; break;
    case SCE_GS_FRAME_1: c->frame = v; break;
    case SCE_GS_ZBUF_1: c->zbuf = v; break;
    }
}

static void write_reg_(u_int reg, uint64_t v)
{
    switch (reg) {
    case SCE_GS_PRIM:
        gs.prim = v;
        vq_count = 0;
        fan_first_valid = 0;
        gs_draw_state_changed();
        return;
    case SCE_GS_RGBAQ: {
        union { uint32_t u; float f; } q;
        gs.rgbaq = v;
        q.u = (uint32_t)(v >> 32);
        gs.q = q.f;
        return;
    }
    case SCE_GS_ST: gs.st = v; return;
    case SCE_GS_UV: gs.uv = v; return;
    case SCE_GS_FOG: gs.fog = v; return;
    case SCE_GS_XYZF2:
        gs.fog = (uint64_t)GS_BITS(v, 56, 8) << 56;
        vertex_kick(v & 0x00ffffffffffffffull, 1);
        return;
    case SCE_GS_XYZ2: vertex_kick(v, 1); return;
    case SCE_GS_XYZF3:
        gs.fog = (uint64_t)GS_BITS(v, 56, 8) << 56;
        vertex_kick(v & 0x00ffffffffffffffull, 0);
        return;
    case SCE_GS_XYZ3: vertex_kick(v, 0); return;

    case SCE_GS_TEX0_1: case SCE_GS_CLAMP_1: case SCE_GS_TEX1_1: case SCE_GS_TEX2_1:
    case SCE_GS_XYOFFSET_1: case SCE_GS_MIPTBP1_1: case SCE_GS_MIPTBP2_1:
    case SCE_GS_SCISSOR_1: case SCE_GS_ALPHA_1: case SCE_GS_TEST_1: case SCE_GS_FBA_1:
    case SCE_GS_FRAME_1: case SCE_GS_ZBUF_1:
        set_ctx_reg(0, reg, v);
        break;
    case SCE_GS_TEX0_2: case SCE_GS_CLAMP_2: case SCE_GS_TEX1_2: case SCE_GS_TEX2_2:
    case SCE_GS_XYOFFSET_2: case SCE_GS_MIPTBP1_2: case SCE_GS_MIPTBP2_2:
    case SCE_GS_SCISSOR_2: case SCE_GS_ALPHA_2: case SCE_GS_TEST_2: case SCE_GS_FBA_2:
    case SCE_GS_FRAME_2: case SCE_GS_ZBUF_2:
        set_ctx_reg(1, reg - 1, v);
        break;

    case SCE_GS_PRMODECONT: gs.prmodecont = v; break;
    case SCE_GS_PRMODE: gs.prmode = v; break;
    case SCE_GS_TEXCLUT: gs.texclut = v; break;
    case SCE_GS_SCANMSK: gs.scanmsk = v; break;
    case SCE_GS_TEXA: gs.texa = v; break;
    case SCE_GS_FOGCOL: gs.fogcol = v; break;
    case SCE_GS_TEXFLUSH: return;
    case SCE_GS_DIMX: gs.dimx = v; break;
    case SCE_GS_DTHE: gs.dthe = v; break;
    case SCE_GS_COLCLAMP: gs.colclamp = v; break;
    case SCE_GS_PABE: gs.pabe = v; break;
    case SCE_GS_BITBLTBUF: gs.bitbltbuf = v; return;
    case SCE_GS_TRXPOS: gs.trxpos = v; return;
    case SCE_GS_TRXREG: gs.trxreg = v; return;
    case SCE_GS_TRXDIR:
        gs.trxdir = v;
        gs_draw_flush();
        gs_mem_transfer_begin();
        return;
    case SCE_GS_HWREG:
        RECVX_LOG_ONCE("gs: HWREG write ignored");
        return;
    case SCE_GS_SIGNAL: case SCE_GS_FINISH: case SCE_GS_LABEL:
        return;
    default:
        return;
    }
    gs_draw_state_changed();
}

void gs_write_reg(u_int reg, uint64_t v)
{
    gs_lock();
    write_reg_(reg, v);
    gs_unlock();
}

/* ------------------------------------------------------------------- GIF */

typedef struct {
    u_int nloop, nreg, flg, eop, reg_index;
    uint64_t regs;
    int in_tag;
} GifPath;

static GifPath paths[3];

static void packed(const uint32_t *q, u_int desc)
{
    uint64_t lo = (uint64_t)q[0] | (uint64_t)q[1] << 32;
    uint64_t hi = (uint64_t)q[2] | (uint64_t)q[3] << 32;

    switch (desc) {
    case 0x0: gs_write_reg(SCE_GS_PRIM, lo & 0x7ff); break;
    case 0x1:
        gs.rgbaq = (q[0] & 0xff) | (q[1] & 0xff) << 8 | (q[2] & 0xff) << 16 |
                   (uint64_t)(q[3] & 0xff) << 24;
        {
            union { float f; uint32_t u; } qq = { gs.q };
            gs.rgbaq |= (uint64_t)qq.u << 32;
        }
        break;
    case 0x2: {
        union { uint32_t u; float f; } qq;
        gs.st = lo;
        qq.u = q[2];
        gs.q = qq.f;
        break;
    }
    case 0x3: gs.uv = (q[0] & 0x3fff) | (uint64_t)(q[1] & 0x3fff) << 16; break;
    case 0x4: { /* XYZF2: X, Y, Z (24 bits at 4), F (8 bits at 100) */
        uint64_t v = (q[0] & 0xffff) | (uint64_t)(q[1] & 0xffff) << 16 |
                     (uint64_t)((q[2] >> 4) & 0xffffff) << 32 |
                     (uint64_t)((q[3] >> 4) & 0xff) << 56;
        gs_write_reg((hi >> 47) & 1 ? SCE_GS_XYZF3 : SCE_GS_XYZF2, v);
        break;
    }
    case 0x5: {
        uint64_t v = (q[0] & 0xffff) | (uint64_t)(q[1] & 0xffff) << 16 | (uint64_t)q[2] << 32;
        gs_write_reg((hi >> 47) & 1 ? SCE_GS_XYZ3 : SCE_GS_XYZ2, v);
        break;
    }
    case 0xa: gs.fog = (uint64_t)((q[3] >> 4) & 0xff) << 56; break;
    case 0xe: gs_write_reg(q[2] & 0xff, lo); break;
    case 0xf: break;
    default: gs_write_reg(desc, lo); break;  /* TEX0/CLAMP/XYZF3/XYZ3 */
    }
}

static void set_prim_from_tag(uint64_t tag)
{
    gs_write_reg(SCE_GS_PRIM, GS_BITS(tag, 47, 11));
}

static void gif_transfer_(int path, const void *data, u_int qwc)
{
    GifPath *p = &paths[path];
    const uint32_t *q = data;

    while (qwc) {
        if (!p->in_tag) {
            uint64_t tag = (uint64_t)q[0] | (uint64_t)q[1] << 32;
            p->regs = (uint64_t)q[2] | (uint64_t)q[3] << 32;
            p->nloop = GS_BITS(tag, 0, 15);
            p->eop = GS_BITS(tag, 15, 1);
            p->flg = GS_BITS(tag, 58, 2);
            p->nreg = GS_BITS(tag, 60, 4);
            if (!p->nreg) p->nreg = 16;
            p->reg_index = 0;
            if (GS_BITS(tag, 46, 1) && p->flg == SCE_GIF_PACKED)
                set_prim_from_tag(tag);
            q += 4; qwc--;
            p->in_tag = p->nloop != 0;
            continue;
        }

        switch (p->flg) {
        case SCE_GIF_PACKED:
            packed(q, (p->regs >> (p->reg_index * 4)) & 15);
            q += 4; qwc--;
            if (++p->reg_index == p->nreg) {
                p->reg_index = 0;
                if (!--p->nloop) p->in_tag = 0;
            }
            break;
        case SCE_GIF_REGLIST:
            for (int half = 0; half < 2 && p->in_tag; half++) {
                u_int desc = (p->regs >> (p->reg_index * 4)) & 15;
                uint64_t v = (uint64_t)q[half * 2] | (uint64_t)q[half * 2 + 1] << 32;
                /* REGLIST takes register numbers directly; A+D, NOP and 0xb are skipped */
                if (desc <= 0xd && desc != 0xb)
                    gs_write_reg(desc, v);
                if (++p->reg_index == p->nreg) {
                    p->reg_index = 0;
                    if (!--p->nloop) p->in_tag = 0;
                }
            }
            q += 4; qwc--;
            break;
        default: { /* IMAGE (and the undefined 3) */
            u_int n = p->nloop < qwc ? p->nloop : qwc;
            gs_mem_transfer_data(q, n);
            q += n * 4; qwc -= n;
            p->nloop -= n;
            if (!p->nloop) p->in_tag = 0;
            break;
        }
        }
    }
}

void gif_transfer(int path, const void *data, u_int qwc)
{
    gs_lock();
    gif_transfer_(path, data, qwc);
    gs_unlock();
}

void gs_reset(void)
{
    memset(&gs, 0, sizeof(gs));
    memset(paths, 0, sizeof(paths));
    gs.prmodecont = 1;
    vq_count = 0;
}
