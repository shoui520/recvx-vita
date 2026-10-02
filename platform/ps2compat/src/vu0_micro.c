/*
 * VU0 micro-mode programs (vsm/ps2_vu0.vsm in the decomp) rewritten in C.
 *
 * The EE code starts these with vcallms and then keeps using VU0 registers in
 * macro mode, so each routine works directly on the shared VU0 register file
 * and data memory in recvx_vu0. The code is written in program order; the
 * places where the VU pipeline makes the result differ from a plain sequential
 * reading (Q latency, MAC flag delay) are called out where they occur.
 */
#include <math.h>
#include <string.h>
#include "ee_asm.h"

recvx_vu0_state recvx_vu0;

#define X VU_X
#define Y VU_Y
#define Z VU_Z
#define W VU_W
#define XYZ  (X | Y | Z)
#define XYZW (X | Y | Z | W)

static inline float *vumem(int addr) { return (float *)recvx_vu0.mem[addr & 0xff]; }

static inline void sq(int vf, int addr, int m)
{
    float *d = vumem(addr);
    for (int n = 0; n < 4; n++) if (VU_FIELD(m, n)) d[n] = VF(vf)[n];
}

static inline void lq(int vf, int addr, int m) { vu_store(VF(vf), vumem(addr), m); }

/* ILW.x / ISW.x: 16-bit integer in the x word of a data-memory quadword */
static inline int16_t ilw_x(int addr) { return (int16_t)recvx_vu0.mem[addr & 0xff][0]; }
static inline void isw(int addr, int field, int16_t v) { recvx_vu0.mem[addr & 0xff][field] = (uint16_t)v; }

/* MAC flag sign bits (x=0x80, y=0x40, z=0x20, w=0x10) of fs - ft */
static inline int mac_sign_sub(const float *s, const float *t)
{
    int f = 0;
    for (int n = 0; n < 4; n++) if (s[n] - t[n] < 0.0f) f |= 0x80 >> n;
    return f;
}
static inline int mac_sign(const float *v)
{
    int f = 0;
    for (int n = 0; n < 4; n++) if (v[n] < 0.0f) f |= 0x80 >> n;
    return f;
}

/* FCOR: 1 when every clip-flag bit not set in imm is set */
static inline int fcor(uint32_t imm) { return ((recvx_vu0.clip | imm) & 0xffffff) == 0xffffff; }

void recvx_vu0_reset(void)
{
    memset(&recvx_vu0, 0, sizeof(recvx_vu0));
    VF(0)[3] = 1.0f;
}

/* ------------------------------------------------------------------ misc */

VU0_MICRO(VU0_MINMAX)
{
    vu_mini(VF(4), VF(4), VF(6), X | Y);
    vu_max(VF(4), VF(4), VF(5), X | Y);
}

VU0_MICRO(VU0_WAVE_INIT)
{
    vu_sub(VF(4), VF(0), VF(0), XYZW);
    vu_sub(VF(5), VF(0), VF(0), XYZW);
    vu_sub_bc(VF(4), VF(0), VF(0)[3], Y);
    vu_sub_bc(VF(5), VF(0), VF(0)[3], Y);
}

VU0_MICRO(VU0_WAVE_CALC)
{
    vu_add(VF(4), VF(0), VF(7), X);
    vu_sub(VF(5), VF(0), VF(9), X);
    vu_sub_bc(VF(4), VF(0), VF(6)[0], Z);
    vu_add_bc(VF(5), VF(0), VF(8)[0], Z);
    vu_add(VF(10), VF(4), VF(5), XYZ);
    vu_abs(VF(11), VF(10), XYZ);
    vu_add_bc(VF(11), VF(11), VF(11)[1], X);
    vu_add_bc(VF(11), VF(11), VF(11)[2], X);
    VQ = vu_div(VF(0)[3], VF(11)[0]);
    vu_sub(VF(10), VF(0), VF(10), X | Z);
}

/* ------------------------------------------------------- scissor work area */

VU0_MICRO(VU0_SET_NODE_ARRAY)
{
    VI(3) = 80;
    sq(19, 80 + 0, XYZW);  sq(18, 80 + 4, XYZW);  sq(15, 80 + 8, XYZW);  sq(19, 80 + 12, XYZW);
    sq(4,  80 + 1, XYZW);  sq(5,  80 + 5, XYZW);  sq(6,  80 + 9, XYZW);  sq(4,  80 + 13, XYZW);
    sq(7,  80 + 2, XYZW);  sq(8,  80 + 6, XYZW);  sq(9,  80 + 10, XYZW); sq(7,  80 + 14, XYZW);
    sq(10, 80 + 3, XYZW);  sq(11, 80 + 7, XYZW);  sq(12, 80 + 11, XYZW); sq(10, 80 + 15, XYZW);
}

VU0_MICRO(VU0_LOAD_SCISSOR_WORK)
{
    int a = VI(3);
    lq(8, a + 0, XYZW); lq(9, a + 1, XYZW); lq(10, a + 2, XYZW); lq(11, a + 3, XYZW);
}

VU0_MICRO(VU0_LOAD_SCISSOR_WORKi)
{
    lq(8, VI(3)++, XYZW); lq(9, VI(3)++, XYZW); lq(10, VI(3)++, XYZW); lq(11, VI(3)++, XYZW);
}

VU0_MICRO(VU0_LOAD_SCISSOR_WORKb)
{
    int a = VI(5);
    lq(4, a + 0, XYZW); lq(5, a + 1, XYZW); lq(6, a + 2, XYZW); lq(7, a + 3, XYZW);
}

VU0_MICRO(VU0_STORE_SCISSOR_WORK)
{
    sq(4, VI(4)++, XYZW); sq(5, VI(4)++, XYZW); sq(6, VI(4)++, XYZW); sq(7, VI(4)++, XYZW);
}

/* ------------------------------------------------------ vertex colour table */

/* VU addresses the original loads into the dispatch table (data memory 176..186). */
enum {
    VA_GetVertexColor           = 0x19c,
    VA_GetVertexColorCM         = 0x19f,
    VA_GetVertexColorIgnore     = 0x19a,
    VA_GetVertexColorDif        = 0x1a1,
    VA_GetVertexColorDifAmb     = 0x195,
    VA_GetVertexColorDifSpe1    = 0x1a5,
    VA_GetVertexColorDifSpe2    = 0x1ae,
    VA_GetVertexColorDifSpe3    = 0x1b8,
    VA_GetVertexColorDifSpe1Amb = 0x1c6,
    VA_GetVertexColorDifSpe2Amb = 0x1d0,
    VA_GetVertexColorDifSpe3Amb = 0x1da,
};

/* Every colour routine ends with "MUL.w vf04, vf00, vf23" in the JR delay slot. */
static void color_alpha(void) { vu_mul(VF(4), VF(0), VF(23), W); }

static void clamp01_xyz(int vf)
{
    vu_mini_bc(VF(vf), VF(vf), VF(0)[3], XYZ);
    vu_max_bc(VF(vf), VF(vf), VF(0)[0], XYZ);
}

static void vu0GetVertexColor(void)
{
    VI_ = 255.0f;                                   /* LOI 255 */
    vu_mul_bc(VF(4), VF(4), VF(3)[0], XYZ);
    vu_mul_bc(VF(4), VF(4), VF(3)[1], W);
}

static void vu0GetVertexColorCM(void)
{
    vu_add(VF(4), VF(0), VF(20), XYZ);
    color_alpha();
}

static void vu0GetVertexColorIgnore(void)
{
    vu_add_bc(VF(4), VF(0), VF(3)[0], XYZ);
    color_alpha();
}

static void vu0GetVertexColorDif(void)
{
    vu_mini_bc(VF(4), VF(4), VF(0)[3], XYZ);
    vu_max_bc(VF(4), VF(4), VF(0)[0], XYZ);
    vu_mul(VF(4), VF(4), VF(20), XYZ);
    color_alpha();
}

static void vu0GetVertexColorDifAmb(void)
{
    vu_add(VF(4), VF(4), VF(22), XYZ);
    vu_max_bc(VF(4), VF(4), VF(0)[0], XYZ);
    vu_mini_bc(VF(4), VF(4), VF(0)[3], XYZ);
    vu_mul(VF(4), VF(4), VF(20), XYZ);
    color_alpha();
}

static void spe_finish(void)
{
    clamp01_xyz(4);
    clamp01_xyz(5);
    vu_mul(VF(4), VF(4), VF(20), XYZ);
    vu_mul(VF(5), VF(5), VF(21), XYZ);
    vu_add(VF(4), VF(4), VF(5), XYZ);
    color_alpha();
}

static void vu0GetVertexColorDifSpe1(void)
{
    vu_sub_bc(VF(5), VF(4), VF(0)[3], XYZ);
    spe_finish();
}

static void vu0GetVertexColorDifSpe2(void)
{
    vu_add(VF(5), VF(4), VF(22), XYZ);
    vu_sub_bc(VF(5), VF(5), VF(0)[3], XYZ);
    spe_finish();
}

static void vu0GetVertexColorDifSpe3(void)
{
    vu_add_bc(VF(5), VF(0), VF(0)[3], XYZ);
    for (int i = 0; i < 17; i++)
        vu_mul(VF(5), VF(5), VF(1), XYZ);
    VI(1) = 0;
    spe_finish();
}

static void vu0GetVertexColorDifSpe1Amb(void)
{
    vu_sub_bc(VF(5), VF(4), VF(0)[3], XYZ);
    clamp01_xyz(5);
    vu_add(VF(4), VF(4), VF(18), XYZ);
    clamp01_xyz(4);
    vu_mul(VF(4), VF(4), VF(20), XYZ);
    vu_mul(VF(5), VF(5), VF(21), XYZ);
    vu_add(VF(4), VF(4), VF(5), XYZ);
    color_alpha();
}

static void vu0GetVertexColorDifSpe2Amb(void)
{
    vu_add(VF(4), VF(4), VF(22), XYZ);
    vu_sub_bc(VF(5), VF(4), VF(0)[3], XYZ);
    spe_finish();
}

static void vu0GetVertexColorDifSpe3Amb(void)
{
    vu_add(VF(4), VF(4), VF(22), XYZ);
    vu_add_bc(VF(5), VF(4), VF(0)[3], XYZ);
    for (int i = 0; i < 17; i++)
        vu_mul(VF(5), VF(5), VF(4), XYZ);
    VI(1) = 0;
    spe_finish();
}

/* JALR vi15, vi01 into the colour routine selected by VU0_INIT_CALC_PROCESS */
static void call_color(int vu_addr)
{
    switch (vu_addr) {
    case VA_GetVertexColor:           vu0GetVertexColor(); break;
    case VA_GetVertexColorCM:         vu0GetVertexColorCM(); break;
    case VA_GetVertexColorIgnore:     vu0GetVertexColorIgnore(); break;
    case VA_GetVertexColorDif:        vu0GetVertexColorDif(); break;
    case VA_GetVertexColorDifAmb:     vu0GetVertexColorDifAmb(); break;
    case VA_GetVertexColorDifSpe1:    vu0GetVertexColorDifSpe1(); break;
    case VA_GetVertexColorDifSpe2:    vu0GetVertexColorDifSpe2(); break;
    case VA_GetVertexColorDifSpe3:    vu0GetVertexColorDifSpe3(); break;
    case VA_GetVertexColorDifSpe1Amb: vu0GetVertexColorDifSpe1Amb(); break;
    case VA_GetVertexColorDifSpe2Amb: vu0GetVertexColorDifSpe2Amb(); break;
    case VA_GetVertexColorDifSpe3Amb: vu0GetVertexColorDifSpe3Amb(); break;
    default: break;
    }
}

static void init_calc_process(void)
{
    static const int16_t table[11] = {
        VA_GetVertexColor, VA_GetVertexColorCM, VA_GetVertexColorIgnore, VA_GetVertexColorDif,
        VA_GetVertexColorDifAmb, VA_GetVertexColorDifSpe1, VA_GetVertexColorDifSpe2,
        VA_GetVertexColorDifSpe3, VA_GetVertexColorDifSpe1Amb, VA_GetVertexColorDifSpe2Amb,
        VA_GetVertexColorDifSpe3Amb,
    };

    VI(13) = 176;
    for (int i = 0; i < 11; i++) {
        VI(2 + i) = table[i];
        isw(176 + i, 0, table[i]);
    }
    VI(1) = (int16_t)(VI(1) + 176);
    vu_mul_bc(VF(15), VF(0), VF(0)[3], XYZW);
    VI(1) = ilw_x(VI(1));
    vu_mul_bc(VF(18), VF(0), VF(0)[3], XYZW);
    isw(187, 0, VI(1));
    vu_mul_bc(VF(19), VF(0), VF(0)[3], XYZW);
}

VU0_MICRO(VU0_INIT_CALC_PROCESS) { init_calc_process(); }
VU0_MICRO(VU0_INIT_CALC_COLOR)   { init_calc_process(); }

/* vf15 = clip matrix (vf24..vf27) * vf14, history kept in vf18/vf19 */
static void push_clip_vertex(void)
{
    vu_move(VF(19), VF(18), XYZW);
    vu_move(VF(18), VF(15), XYZW);
    vu_mul_bc(VACC, VF(24), VF(14)[0], XYZW);
    vu_madd_bc(VACC, VF(25), VF(14)[1], XYZW);
    vu_madd_bc(VACC, VF(26), VF(14)[2], XYZW);
    vu_madd_bc(VF(15), VF(27), VF(0)[3], XYZW);
}

VU0_MICRO(VU0_CALC_COLOR)
{
    push_clip_vertex();
    VI(1) = ilw_x(187);
    vu_clip(VF(15), VF(15)[3]);               /* JALR delay slot */
    call_color(VI(1));
}

/* Trivial reject test on the last three clip results: vi02 = 0 when all three
 * vertices are outside the same plane, otherwise 1. */
static void clip_volume_plane(void)
{
    static const uint32_t planes[6] = { 0xDF7DF, 0xEFBEF, 0x7DF7, 0xBEFB, 0xDF7D, 0xEFBE };

    VI(2) = 0;
    for (int i = 0; i < 6; i++) {
        VI(1) = (int16_t)fcor(planes[i]);
        if (VI(1) != 0)
            return;
    }
    VI(2) = 1;
}

VU0_MICRO(VU0_CLIP_VIEW_VOLUME_ALL)
{
    push_clip_vertex();
    vu_clip(VF(19), VF(19)[3]);
    vu_clip(VF(18), VF(18)[3]);
    VI(1) = ilw_x(187);
    vu_clip(VF(15), VF(15)[3]);
    call_color(VI(1));
    clip_volume_plane();
}

VU0_MICRO(VU0_CLIP_VIEW_VOLUME)
{
    push_clip_vertex();
    vu_clip(VF(19), VF(19)[3]);
    vu_clip(VF(18), VF(18)[3]);
    vu_clip(VF(15), VF(15)[3]);
}

VU0_MICRO(VU0_CLIP_VOLUME_PLANE) { clip_volume_plane(); }

/* -------------------------------------------------------- material colour */

VU0_MICRO(VU0_CALCCOLINIT)
{
    sq(4, 3, XYZW);
    sq(5, 4, XYZW);
    sq(6, 5, XYZW);
}

VU0_MICRO(VU0_CALCDIFAMB)
{
    vu_sub(VF(7), VF(7), VF(7), W);
    lq(4, 5, XYZ);
    lq(5, 3, XYZW);
    vu_add(VF(7), VF(7), VF(4), XYZ);
    vu_max_bc(VF(7), VF(7), VF(0)[0], XYZ);
    vu_mini_bc(VF(7), VF(7), VF(0)[3], XYZ);
    vu_add(VF(7), VF(7), VF(5), W);
    vu_mul(VF(7), VF(7), VF(5), XYZ);
}

VU0_MICRO(VU0_CALCIGNORE)
{
    lq(5, 3, XYZW);
    VI_ = 128.0f;
    vu_add_bc(VF(7), VF(0), VI_, XYZ);
    vu_move(VF(7), VF(5), W);
}

/* ------------------------------------------------- lit point (4 lights) */

VU0_MICRO(VU0_CALCPOINT_INIT)
{
    for (int i = 4; i <= 14; i++)
        sq(i, 64 + (i - 4), XYZW);
    sq(17, 49, XYZW);
    for (int i = 24; i <= 31; i++)
        sq(i, 56 + (i - 24), XYZW);
    vu_mul_bc(VF(17), VF(17), VF(23)[2], X | Y);
    lq(24, 64, XYZW);
    lq(25, 65, XYZW);
    lq(26, 66, XYZW);
    lq(12, 68, XYZW);
    lq(13, 69, XYZW);
    lq(27, 70, XYZW);
    VI(10) = ilw_x(67);
    VI(4) = 16;
    VI(5) = 32;
    VI(6) = 64;
    VI(7) = 128;
    VI(8) = 8;
    VI(9) = 240;
    isw(71, 3, 0);
    VI(11) = 63;                              /* [E] delay slot */
}

VU0_MICRO(VU0_CALCPOINT_END)
{
    lq(17, 49, XYZW);
    lq(24, 56, XYZW);
    lq(25, 57, XYZW);
    lq(26, 58, XYZW);
    lq(27, 59, XYZW);
}

/* Per-light term for light n (x,y,z,w = lights 0..3); enable/point bits are
 * taken from vi14, which is shifted left once per light. */
static void calcpoint_light(int n, float rsq_q)
{
    int enable = VI(14) & VI(7);
    int point = VI(14) & VI(8);

    VI(1) = (int16_t)enable;
    VI(15) = (int16_t)point;

    VI(14) = (int16_t)(VI(14) + VI(14));    /* IBEQ delay slot: always executes */
    if (!enable)
        return;

    if (!point) {
        /* parallel light: N . L, clamped at 0 */
        float d = VF(24)[n] * VF(10)[0] + VF(25)[n] * VF(10)[1] + VF(26)[n] * VF(10)[2];
        VF(8)[n] = d > 0.0f ? d : 0.0f;
        return;
    }

    {
        int mask = 0x80 >> n;
        if (VI(13) & mask)                   /* back facing or beyond the far radius */
            return;
        /* "MULq vf08, vf04, Q | DIV Q, ..." sits in the IBEQ delay slot: it runs
         * whether or not the near-radius test branches, and reads the RSQRT Q. */
        VF(8)[n] = VF(4)[n] * rsq_q;
        if (!(VI(12) & mask))                /* inside the near radius: no falloff */
            return;
        VQ = vu_div(VF(12)[n], VF(5)[n]);
        VF(8)[n] = VF(8)[n] * VQ;
    }
}

VU0_MICRO(VU0_CALCPOINT)
{
    int mac_dot, mac_near, mac_far;

    vu_mul_bc(VACC, VF(28), VF(4)[0], XYZW);
    vu_move(VF(6), VF(1), Z);
    vu_madd_bc(VACC, VF(29), VF(4)[1], XYZW);
    vu_madd_bc(VACC, VF(30), VF(4)[2], XYZW);
    vu_madd_bc(VF(18), VF(31), VF(0)[3], XYZW);
    vu_mul_bc(VACC, VF(28), VF(5)[0], XYZ);
    vu_madd_bc(VACC, VF(29), VF(5)[1], XYZ);
    vu_madd_bc(VF(19), VF(30), VF(5)[2], XYZ);
    vu_mul(VF(6), VF(18), VF(17), X | Y);
    VQ = vu_div(VF(0)[3], VF(18)[2]);
    vu_sub(VF(8), VF(8), VF(8), XYZW);
    vu_sub_bc(VF(9), VF(24), VF(18)[0], XYZW);  lq(28, 56, XYZW);
    vu_sub_bc(VF(10), VF(25), VF(18)[1], XYZW); lq(29, 57, XYZW);
    vu_sub_bc(VF(11), VF(26), VF(18)[2], XYZW); lq(30, 58, XYZW);
    lq(31, 59, XYZW);
    vu_mul_bc(VF(6), VF(0), VF(19)[2], W);
    vu_mul_bc(VF(6), VF(6), VQ, XYZ);
    vu_mul(VACC, VF(9), VF(9), XYZW);
    vu_madd(VACC, VF(10), VF(10), XYZW);
    vu_madd(VF(5), VF(11), VF(11), XYZW);       /* squared distance to each light */
    vu_mul_bc(VACC, VF(9), VF(19)[0], XYZW);
    vu_madd_bc(VACC, VF(10), VF(19)[1], XYZW);
    vu_madd_bc(VF(4), VF(11), VF(19)[2], XYZW); /* N . (light - p) per light */

    /* FMAND reads the MAC flags four instructions late: vi13 sees the dot
     * product above, vi12 the near-radius SUB and vi01 the far-radius SUB. */
    mac_dot = mac_sign(VF(4));
    mac_near = mac_sign_sub(VF(12), VF(5));
    mac_far = mac_sign_sub(VF(13), VF(5));
    recvx_vu0.mac = (uint32_t)mac_far;
    VI(13) = (int16_t)(mac_dot & VI(9));
    VI(12) = (int16_t)(mac_near & VI(9));
    VI(1) = (int16_t)(mac_far & VI(9));
    VI(13) = (int16_t)(VI(13) | VI(1));
    VI(14) = (int16_t)(VI(10) + VI(0));

    for (int n = 0; n < 4; n++) {           /* LIGHT0..LIGHT3 */
        vu_mul_bc(VF(10), VF(19), VF(27)[n], XYZ);
        VQ = vu_rsqrt(VF(0)[3], VF(5)[n]);
        calcpoint_light(n, VQ);
    }

    /* LIGHT4: clip-space position, light colour sum, original matrix back */
    lq(7, 71, XYZ);
    vu_mul_bc(VACC, VF(28), VF(18)[0], XYZW);
    lq(9, 72, XYZ);
    vu_madd_bc(VACC, VF(29), VF(18)[1], XYZW);
    lq(10, 73, XYZ);
    vu_madd_bc(VACC, VF(30), VF(18)[2], XYZW);
    lq(11, 74, XYZ);
    vu_madd_bc(VF(5), VF(31), VF(0)[3], XYZW);
    vu_mul_bc(VACC, VF(7), VF(8)[0], XYZ);   lq(28, 60, XYZW);
    vu_madd_bc(VACC, VF(9), VF(8)[1], XYZ);  lq(29, 61, XYZW);
    vu_madd_bc(VACC, VF(10), VF(8)[2], XYZ); lq(30, 62, XYZW);
    vu_clip(VF(5), VF(5)[3]);                lq(31, 63, XYZW);
    vu_madd_bc(VF(7), VF(11), VF(8)[3], XYZ);
    vu_add_bc(VF(11), VF(0), VF(6)[2], X);
    VI(1) = (int16_t)(recvx_vu0.clip & 0xfff);   /* FCGET */
    VI(11) = (int16_t)(VI(11) & VI(1));
}
