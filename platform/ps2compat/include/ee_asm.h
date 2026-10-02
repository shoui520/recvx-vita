/*
 * Runtime support for C translated from the game's EE/VU0 inline assembly.
 *
 * The original code drives VU0 in macro mode, and VU0 registers keep their
 * values between functions (njInitMatrix loads constants that later matrix
 * code reads, the clip matrix lives in vf24-vf27, ...). The translation keeps
 * that model: one global VU0 register file that every translated block and
 * every VU0 microprogram (vcallms) shares.
 *
 * Instruction semantics follow the EE Core Instruction Set Manual and the VU
 * User's Manual. VU float clamping (no inf/NaN, no denormals) is not modelled.
 */
#ifndef RECVX_EE_ASM_H
#define RECVX_EE_ASM_H

#include <stdint.h>
#include <string.h>
#include <math.h>

/* ---------------------------------------------------------------- EE GPRs */

typedef union {
    uint64_t d[2];
    int64_t  sd[2];
    uint32_t w[4];
    int32_t  sw[4];
    uint16_t h[8];
    int16_t  sh[8];
    uint8_t  b[16];
    float    f[4];
} ee_gpr;

#define EE_SEXT32(x) ((uint64_t)(int64_t)(int32_t)(uint32_t)(x))
#define EE_PTR(r)    ((uintptr_t)(uint32_t)(r).d[0])

static inline uint32_t ee_fbits(float f) { uint32_t u; memcpy(&u, &f, 4); return u; }
static inline float ee_bitsf(uint32_t u) { float f; memcpy(&f, &u, 4); return f; }

static inline uint64_t ee_ld64(uintptr_t a) { uint64_t v; memcpy(&v, (void *)a, 8); return v; }
static inline void ee_sd64(uintptr_t a, uint64_t v) { memcpy((void *)a, &v, 8); }

/*
 * Read/write a C variable as a GPR (pointers, integers, and float bit patterns).
 * Like an EE register, every 32-bit value is held sign-extended - the same
 * form EE_SEXT32 gives loaded words - so a counter that goes negative still
 * compares below zero and a pointer compares equal however it was obtained.
 * 64-bit integers pass through whole.
 */
#define EE_CVAR_GET(v) _Generic((v), \
    float: (uint64_t)ee_fbits(*(float *)(void *)&(v)), \
    double: (uint64_t)ee_fbits((float)*(double *)(void *)&(v)), \
    long long: (uint64_t)(v), \
    unsigned long long: (uint64_t)(v), \
    default: EE_SEXT32((uintptr_t)(v)))
#define EE_CVAR_SET(v, x) do { \
    uint64_t ee_tmp_ = (x); \
    _Generic((v), \
        float: (void)(*(float *)(void *)&(v) = ee_bitsf((uint32_t)ee_tmp_)), \
        long long: (void)((v) = (__typeof__(v))ee_tmp_), \
        unsigned long long: (void)((v) = (__typeof__(v))ee_tmp_), \
        default: (void)((v) = (__typeof__(v))(uintptr_t)ee_tmp_)); \
} while (0)

/* Read/write a C variable as an FPR. */
#define EE_CVAR_GETF(v) _Generic((v), \
    float: *(float *)(void *)&(v), double: (float)*(double *)(void *)&(v), \
    default: ee_bitsf((uint32_t)(uintptr_t)(v)))
#define EE_CVAR_SETF(v, x) do { \
    float ee_tmpf_ = (x); \
    _Generic((v), \
        float: (void)(*(float *)(void *)&(v) = ee_tmpf_), \
        default: (void)((v) = (__typeof__(v))(uintptr_t)ee_fbits(ee_tmpf_))); \
} while (0)

/* ---------------------------------------------------------------- MMI */

static inline ee_gpr ee_pcpyld(ee_gpr rs, ee_gpr rt) { ee_gpr r; r.d[0] = rt.d[0]; r.d[1] = rs.d[0]; return r; }
static inline ee_gpr ee_pcpyud(ee_gpr rs, ee_gpr rt) { ee_gpr r; r.d[0] = rs.d[1]; r.d[1] = rt.d[1]; return r; }
static inline ee_gpr ee_pextlw(ee_gpr rs, ee_gpr rt)
{ ee_gpr r; r.w[0] = rt.w[0]; r.w[1] = rs.w[0]; r.w[2] = rt.w[1]; r.w[3] = rs.w[1]; return r; }
static inline ee_gpr ee_pextuw(ee_gpr rs, ee_gpr rt)
{ ee_gpr r; r.w[0] = rt.w[2]; r.w[1] = rs.w[2]; r.w[2] = rt.w[3]; r.w[3] = rs.w[3]; return r; }
static inline ee_gpr ee_pextlh(ee_gpr rs, ee_gpr rt)
{
    ee_gpr r;
    for (int i = 0; i < 4; i++) { r.h[2 * i] = rt.h[i]; r.h[2 * i + 1] = rs.h[i]; }
    return r;
}
static inline ee_gpr ee_prot3w(ee_gpr rt)
{ ee_gpr r; r.w[0] = rt.w[1]; r.w[1] = rt.w[2]; r.w[2] = rt.w[0]; r.w[3] = rt.w[3]; return r; }
static inline ee_gpr ee_paddub(ee_gpr rs, ee_gpr rt)
{
    ee_gpr r;
    for (int i = 0; i < 16; i++) { unsigned s = rs.b[i] + rt.b[i]; r.b[i] = s > 255 ? 255 : s; }
    return r;
}
static inline ee_gpr ee_plzcw(ee_gpr rs)
{
    /* Count of leading bits equal to the sign bit, minus one, per word. */
    ee_gpr r; r.d[1] = rs.d[1];
    for (int i = 0; i < 2; i++) {
        uint32_t v = rs.w[i];
        if (v & 0x80000000u) v = ~v;
        r.w[i] = (v == 0 ? 32 : __builtin_clz(v)) - 1;
    }
    return r;
}

/* ---------------------------------------------------------------- VU0 */

typedef struct {
    float    vf[32][4];   /* vf0 is the constant (0,0,0,1) */
    float    acc[4];
    float    q, p, i, r;
    int16_t  vi[16];      /* vi0 is always 0 */
    uint32_t clip;        /* 24-bit clipping flag history */
    uint32_t status, mac;
    uint32_t mem[256][4]; /* VU0 data memory (4 KB), addressed in quadwords */
} recvx_vu0_state;

extern recvx_vu0_state recvx_vu0;
#define VF(n) recvx_vu0.vf[n]
#define VI(n) recvx_vu0.vi[n]
#define VACC  recvx_vu0.acc
#define VQ    recvx_vu0.q
#define VI_   recvx_vu0.i

/* field masks: x=8 y=4 z=2 w=1 as in the instruction encoding */
#define VU_X 8
#define VU_Y 4
#define VU_Z 2
#define VU_W 1
#define VU_FIELD(m, n) ((m) & (8 >> (n)))

static inline void vu_store(float *d, const float *s, int m)
{
    if (d == recvx_vu0.vf[0])
        return;                       /* vf0 is hard-wired to (0,0,0,1) */
    for (int n = 0; n < 4; n++) if (VU_FIELD(m, n)) d[n] = s[n];
}

/* VU division never produces Inf/NaN: x/0 saturates to +-FLT_MAX. */
static inline float vu_div(float s, float t)
{
    if (t == 0.0f) {
        uint32_t sign = (ee_fbits(s) ^ ee_fbits(t)) & 0x80000000u;
        return ee_bitsf(sign | 0x7f7fffffu);
    }
    return s / t;
}
static inline float vu_rsqrt(float s, float t)
{
    t = fabsf(t);
    if (t == 0.0f)
        return ee_bitsf((ee_fbits(s) & 0x80000000u) | 0x7f7fffffu);
    return s / sqrtf(t);
}

/* fd = fs OP ft (ft either a vector or a broadcast scalar) */
#define VU_BINOP(name, expr) \
static inline void vu_##name(float *d, const float *s, const float *t, int m) \
{ float r[4]; for (int n = 0; n < 4; n++) { float a = s[n], b = t[n]; (void)a; (void)b; r[n] = (expr); } vu_store(d, r, m); } \
static inline void vu_##name##_bc(float *d, const float *s, float b, int m) \
{ float r[4]; for (int n = 0; n < 4; n++) { float a = s[n]; (void)a; r[n] = (expr); } vu_store(d, r, m); }

VU_BINOP(add, a + b)
VU_BINOP(sub, a - b)
VU_BINOP(mul, a * b)
VU_BINOP(max, a > b ? a : b)
VU_BINOP(mini, a < b ? a : b)

/* fd = ACC +/- fs*ft ; fd may be ACC itself (madda/msuba) */
static inline void vu_madd(float *d, const float *s, const float *t, int m)
{ float r[4]; for (int n = 0; n < 4; n++) r[n] = VACC[n] + s[n] * t[n]; vu_store(d, r, m); }
static inline void vu_madd_bc(float *d, const float *s, float b, int m)
{ float r[4]; for (int n = 0; n < 4; n++) r[n] = VACC[n] + s[n] * b; vu_store(d, r, m); }
static inline void vu_msub(float *d, const float *s, const float *t, int m)
{ float r[4]; for (int n = 0; n < 4; n++) r[n] = VACC[n] - s[n] * t[n]; vu_store(d, r, m); }
static inline void vu_msub_bc(float *d, const float *s, float b, int m)
{ float r[4]; for (int n = 0; n < 4; n++) r[n] = VACC[n] - s[n] * b; vu_store(d, r, m); }

static inline void vu_abs(float *d, const float *s, int m)
{ float r[4]; for (int n = 0; n < 4; n++) r[n] = fabsf(s[n]); vu_store(d, r, m); }
static inline void vu_move(float *d, const float *s, int m)
{ float r[4]; memcpy(r, s, 16); vu_store(d, r, m); }
static inline void vu_mr32(float *d, const float *s, int m)
{ float r[4] = { s[1], s[2], s[3], s[0] }; vu_store(d, r, m); }

/* outer product: opmula ACC.xyz = fs.yzx*ft.zxy ; opmsub fd.xyz = ACC - fs.yzx*ft.zxy */
static inline void vu_opmula(const float *s, const float *t)
{
    float r[4] = { s[1] * t[2], s[2] * t[0], s[0] * t[1], VACC[3] };
    vu_store(VACC, r, VU_X | VU_Y | VU_Z);
}
static inline void vu_opmsub(float *d, const float *s, const float *t)
{
    float r[4] = { VACC[0] - s[1] * t[2], VACC[1] - s[2] * t[0], VACC[2] - s[0] * t[1], d[3] };
    vu_store(d, r, VU_X | VU_Y | VU_Z);
}

static inline void vu_itof(float *d, const float *s, int m, float scale)
{
    float r[4];
    for (int n = 0; n < 4; n++) { int32_t v; memcpy(&v, &s[n], 4); r[n] = (float)v * scale; }
    vu_store(d, r, m);
}
static inline void vu_ftoi(float *d, const float *s, int m, float scale)
{
    float r[4];
    for (int n = 0; n < 4; n++) {
        float f = s[n] * scale;
        int32_t v = f >= 2147483647.0f ? INT32_MAX : f <= -2147483648.0f ? INT32_MIN : (int32_t)f;
        memcpy(&r[n], &v, 4);
    }
    vu_store(d, r, m);
}

/* vclipw.xyz fs, ft.w : shifts 6 new judgement bits into the clip flag history */
static inline void vu_clip(const float *s, float w)
{
    float aw = fabsf(w);
    uint32_t f = 0;
    if (s[0] > +aw) f |= 0x01;
    if (s[0] < -aw) f |= 0x02;
    if (s[1] > +aw) f |= 0x04;
    if (s[1] < -aw) f |= 0x08;
    if (s[2] > +aw) f |= 0x10;
    if (s[2] < -aw) f |= 0x20;
    recvx_vu0.clip = ((recvx_vu0.clip << 6) | f) & 0xffffff;
}

/* VU0 control registers as seen through cfc2/ctc2 (vi0-vi15 are integer registers) */
static inline uint32_t vu_cfc2(int reg)
{
    switch (reg) {
    case 16: return recvx_vu0.status;
    case 17: return recvx_vu0.mac;
    case 18: return recvx_vu0.clip;
    case 20: return ee_fbits(recvx_vu0.r);
    case 21: return ee_fbits(recvx_vu0.i);
    case 22: return ee_fbits(recvx_vu0.q);
    default: return reg < 16 ? (uint32_t)(uint16_t)recvx_vu0.vi[reg] : 0;
    }
}
static inline void vu_ctc2(int reg, uint32_t v)
{
    switch (reg) {
    case 16: recvx_vu0.status = v; break;
    case 18: recvx_vu0.clip = v & 0xffffff; break;
    case 20: recvx_vu0.r = ee_bitsf(v); break;
    case 21: recvx_vu0.i = ee_bitsf(v); break;
    case 22: recvx_vu0.q = ee_bitsf(v); break;
    default: if (reg > 0 && reg < 16) recvx_vu0.vi[reg] = (int16_t)v; break;
    }
}

static inline void vu_qmtc2(int n, ee_gpr r) { if (n) memcpy(VF(n), &r, 16); }
static inline ee_gpr vu_qmfc2(int n) { ee_gpr r; memcpy(&r, VF(n), 16); return r; }
static inline void vu_lqc2(int n, uintptr_t a) { if (n) memcpy(VF(n), (void *)a, 16); }
static inline void vu_sqc2(int n, uintptr_t a) { memcpy((void *)a, VF(n), 16); }

/* VU0 microprograms started with vcallms, implemented in C (ps2_vu0_micro.c). */
#define VU0_MICRO(name) void recvx_vu0_micro_##name(void)
#define VU0_CALLMS(name) recvx_vu0_micro_##name()

VU0_MICRO(VU0_MINMAX);
VU0_MICRO(VU0_WAVE_INIT);
VU0_MICRO(VU0_WAVE_CALC);
VU0_MICRO(VU0_SET_NODE_ARRAY);
VU0_MICRO(VU0_LOAD_SCISSOR_WORK);
VU0_MICRO(VU0_LOAD_SCISSOR_WORKi);
VU0_MICRO(VU0_LOAD_SCISSOR_WORKb);
VU0_MICRO(VU0_STORE_SCISSOR_WORK);
VU0_MICRO(VU0_INIT_CALC_PROCESS);
VU0_MICRO(VU0_INIT_CALC_COLOR);
VU0_MICRO(VU0_CALC_COLOR);
VU0_MICRO(VU0_CLIP_VIEW_VOLUME_ALL);
VU0_MICRO(VU0_CLIP_VIEW_VOLUME);
VU0_MICRO(VU0_CLIP_VOLUME_PLANE);
VU0_MICRO(VU0_CALCCOLINIT);
VU0_MICRO(VU0_CALCDIFAMB);
VU0_MICRO(VU0_CALCIGNORE);
VU0_MICRO(VU0_CALCPOINT_INIT);
VU0_MICRO(VU0_CALCPOINT_END);
VU0_MICRO(VU0_CALCPOINT);

void recvx_vu0_reset(void);

#endif
