/*
 * VU1 microprograms (vsm/ps2_vu1.vsm) replaced by the GPU.
 *
 * Both programs (DoubleMain at MSCAL 0, SingleMain at MSCAL 700) take one
 * triangle strip from ps2_NinjaCnk.c: a GIF tag at TOP followed by NLOOP
 * VU1_STRIP_BUF vertices (U V Q func | Ir Ig Ib A | Vx Vy Vz Fog | screen),
 * light it with one of eleven colour functions, project it, clip it against
 * the frustum and XGKICK it. Here the strip goes to the GPU untouched: the
 * vertex shader does the colour function and builds a homogeneous position
 * (W = Vz), and the hardware clipper takes the place of CLIP_INTER.
 *
 * Constant block (VU addresses):
 *   884 diffuse  885 ambient  886 specular  887 near far proj alpha
 *   888-891 ClipMatrix2  892 OffsetX OffsetY 0 4095  893 AspectW AspectH 0 65535
 *   894-897 ClipScreenMatrix  898 GIF tag for clipped polygons
 */
#include <string.h>
#include "gs/gs.h"
#include "recvx_platform.h"

void vu1_hle_run(u_int start_addr)
{
    if (start_addr != 0 && start_addr != 700) {
        RECVX_LOG_ONCE("vu1: unknown microprogram entry %u", start_addr);
        return;
    }

    u_int top = vu1_top();
    const uint32_t *tagw = VU1_MEMI(top);
    uint64_t tag = (uint64_t)tagw[0] | (uint64_t)tagw[1] << 32;
    u_int n = tagw[0] & 0x7fff;
    if (!n) return;
    if (top + 1 + n * 4 > 1024) {
        RECVX_LOG_ONCE("vu1: strip overruns VU memory (top=%u n=%u)", top, n);
        return;
    }

    Vu1Batch b;
    b.prim = GS_BITS(tag, 46, 1) ? GS_BITS(tag, 47, 11) : (u_int)gs.prim;
    b.func = VU1_MEMI(top + 1)[3] & 0xffff;    /* ILW.w of the first vertex */
    if (b.func > 10) {
        RECVX_LOG_ONCE("vu1: colour function %u out of range", b.func);
        b.func = 0;
    }
    memcpy(b.consts, vu1_mem[884], sizeof(b.consts));
    b.verts = (const float (*)[4])vu1_mem[top + 1];
    b.n = n;
    gs_draw_vu1(&b);
}
