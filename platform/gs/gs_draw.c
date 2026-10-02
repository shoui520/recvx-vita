/*
 * vitaGL backend for the GS model.
 *
 * Every GS frame buffer the game draws to (FRAME.FBP) becomes a GL render
 * target, and stays authoritative for its part of GS memory: uploads into it
 * are blitted in, reads from it are read back, and textures whose TBP0 lands
 * on it sample it directly (the motion-blur swap and shadow passes rely on
 * this). Everything else is decoded from gs_mem into a texture cache.
 *
 * Colour conventions: vertex/texture colours are GS values (0x80 = 1.0 for
 * modulation); frame buffer alpha holds A/128 so GL's SRC/DST_ALPHA blend
 * factors match the GS's As/Ad.
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vitaGL.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include "gs/gs.h"
#include "libgraph.h"
#include "recvx_platform.h"

#define SCREEN_W 960
#define SCREEN_H 544
#define RT_HEIGHT 512

static float rt_scale = 1.0f;  /* internal resolution multiplier */

/* ---------------------------------------------------------- render targets */

typedef struct {
    u_int fbp, fbw;
    int w, h;              /* in GS pixels */
    int drawn_h;           /* rows drawn so far (scissor bottom); uploads below it leave it alone */
    int stale;             /* an upload overwrote it: GS memory holds the truth until it is drawn again */
    uint32_t dirty;        /* 32-row bands drawn since GS memory was last refreshed from them */
    GLuint tex, fbo, depth;
    int used;
} RenderTarget;

#define MAX_RT 16
static RenderTarget rts[MAX_RT];

/* per-frame profile, reported with the fps line */
static SceUInt64 prof_readback_us, prof_texdecode_us, prof_vu1_us, prof_frame_us;
static u_int prof_readbacks, prof_texmiss;

static RenderTarget *rt_find(u_int fbp)
{
    for (int i = 0; i < MAX_RT; i++)
        if (rts[i].used && rts[i].fbp == fbp) return &rts[i];
    return NULL;
}

static RenderTarget *bound_rt;

static void rt_destroy(RenderTarget *rt)
{
    /* never leave a deleted framebuffer bound in vitaGL */
    if (bound_rt == rt) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        bound_rt = NULL;
    }
    glDeleteFramebuffers(1, &rt->fbo);
    glDeleteTextures(1, &rt->tex);
    glDeleteRenderbuffers(1, &rt->depth);
    memset(rt, 0, sizeof(*rt));
}

static RenderTarget *rt_get(u_int fbp, u_int fbw)
{
    RenderTarget *rt = rt_find(fbp);
    if (rt && rt->fbw == fbw) return rt;
    if (rt) rt_destroy(rt);
    for (int i = 0; i < MAX_RT && !rt; i++)
        if (!rts[i].used) rt = &rts[i];
    if (!rt) { RECVX_LOG("gs: out of render targets"); rt = &rts[0]; rt_destroy(rt); }

    rt->used = 1;
    rt->fbp = fbp;
    rt->fbw = fbw ? fbw : 1;
    rt->w = rt->fbw * 64;
    rt->h = RT_HEIGHT;
    int pw = (int)(rt->w * rt_scale), ph = (int)(rt->h * rt_scale);

    glGenTextures(1, &rt->tex);
    glBindTexture(GL_TEXTURE_2D, rt->tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pw, ph, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenRenderbuffers(1, &rt->depth);
    glBindRenderbuffer(GL_RENDERBUFFER, rt->depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, pw, ph);
    glGenFramebuffers(1, &rt->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, rt->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rt->tex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rt->depth);
    glViewport(0, 0, pw, ph);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glClearColor(0, 0, 0, 0);
    glClearDepthf(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RECVX_LOG("gs: render target fbp=%u fbw=%u", fbp, rt->fbw);
    return rt;
}

/* GS blocks covered by a render target (PSMCT32 layout) */
static void rt_blocks(const RenderTarget *rt, u_int *first, u_int *last)
{
    *first = rt->fbp * 32;
    *last = *first + rt->fbw * (rt->h / 32) * 32 - 1;
}

/* the blocks it has actually drawn: the game keeps textures right after a short target */
static int rt_drawn_blocks(const RenderTarget *rt, u_int *first, u_int *last)
{
    if (rt->drawn_h <= 0) return 0;
    *first = rt->fbp * 32;
    *last = *first + rt->fbw * ((rt->drawn_h + 31) / 32) * 32 - 1;
    return 1;
}

/* ----------------------------------------------------------- texture cache */

typedef struct {
    uint64_t tex0, texa;
    uint32_t clut_hash;
    u_int first, last;
    GLuint tex;
    u_int age;
    uint32_t data_hash;    /* GS memory blocks first..last when decoded */
    int suspect;           /* an upload touched the range: verify data_hash before reuse */
} CachedTex;

#define MAX_TEX 384
static CachedTex texcache[MAX_TEX];
static u_int tex_clock;
static uint32_t *decode_buf;


static int psm_paletted(u_int psm)
{
    return psm == SCE_GS_PSMT8 || psm == SCE_GS_PSMT8H || psm == SCE_GS_PSMT4 ||
           psm == SCE_GS_PSMT4HL || psm == SCE_GS_PSMT4HH;
}


/*
 * The game streams textures into GS memory every frame (Ps2TexLoad), mostly
 * re-sending identical data; hashing the range is far cheaper than decoding.
 */
static uint32_t blocks_hash(u_int first, u_int last)
{
    uint32_t h = 2166136261u;
    u_int n = (last - first + 1) * 64, base = first * 64;
    for (u_int i = 0; i < n; i++)
        h = (h ^ gs_mem[(base + i) & (GS_MEM_WORDS - 1)]) * 16777619u;
    return h;
}

static void tex_invalidate(u_int first, u_int last)
{
    for (int i = 0; i < MAX_TEX; i++) {
        CachedTex *t = &texcache[i];
        if (t->tex && t->first <= last && first <= t->last)
            t->suspect = 1;
    }
}

static GLuint tex_lookup(uint64_t tex0)
{
    const uint64_t key_mask = 0x00000007ffffffffull | (psm_paletted(GS_BITS(tex0, 20, 6)) ? 0x1ff8000000000000ull & ~(7ull << 61) : 0);
    uint64_t key = tex0 & key_mask;
    u_int psm = GS_BITS(tex0, 20, 6);
    uint64_t texa = (gs_psm_bpp(psm) == 16 || gs_psm_bpp(psm) == 24 || psm_paletted(psm)) ? gs.texa : 0;
    uint32_t ch = psm_paletted(psm) ? gs_clut_hash(tex0) : 0;
    CachedTex *slot = NULL;

    tex_clock++;
    for (int i = 0; i < MAX_TEX; i++) {
        CachedTex *t = &texcache[i];
        if (t->tex && t->tex0 == key && t->texa == texa && t->clut_hash == ch) {
            if (t->suspect) {
                if (blocks_hash(t->first, t->last) != t->data_hash) {
                    slot = t;      /* same key, new data: decode into this slot */
                    break;
                }
                t->suspect = 0;
            }
            t->age = tex_clock;
            return t->tex;
        }
        if (!t->tex) { if (!slot) slot = t; }
        else if (!slot || (slot->tex && t->age < slot->age)) slot = t;
    }
    if (slot->tex) glDeleteTextures(1, &slot->tex);

    u_int tw = GS_BITS(tex0, 26, 4), th = GS_BITS(tex0, 30, 4);
    u_int w = 1u << (tw > 10 ? 10 : tw), h = 1u << (th > 10 ? 10 : th);
    SceUInt64 t0 = sceKernelGetProcessTimeWide();
    gs_decode_texture(tex0, gs.texa, w, h, decode_buf);

    slot->tex0 = key;
    slot->texa = texa;
    slot->clut_hash = ch;
    slot->age = tex_clock;
    gs_mem_block_range(psm, GS_BITS(tex0, 0, 14), GS_BITS(tex0, 14, 6) ? GS_BITS(tex0, 14, 6) : 1,
                       w, h, &slot->first, &slot->last);
    slot->data_hash = blocks_hash(slot->first, slot->last);
    slot->suspect = 0;
    glGenTextures(1, &slot->tex);
    glBindTexture(GL_TEXTURE_2D, slot->tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, decode_buf);
    prof_texdecode_us += sceKernelGetProcessTimeWide() - t0;
    prof_texmiss++;
    return slot->tex;
}

/* ------------------------------------------------------------------ shaders */

/* fragment variant bits */
#define V_TME   (1u << 0)
#define V_TFX(x) ((x) << 1)    /* 2 bits */
#define V_TCC   (1u << 3)
#define V_FGE   (1u << 4)
#define V_ATST(x) ((x) << 5)   /* 3 bits: GS ATST */
#define V_FIXA  (1u << 8)
#define V_COUNT (1u << 9)

/* vertex paths: 0 = GS vertices, 1 + f = VU1 strip with colour function f */
#define VS_GS 0
#define VS_COUNT 12

typedef struct {
    GLuint prog;
    GLint u_xform, u_uv, u_params, u_fogcol;
    GLint u_proj, u_z, u_dif, u_amb, u_spe;    /* VU1 path */
} Shader;

static Shader *shaders[VS_COUNT];
static GLuint vshaders[VS_COUNT];
static GLuint fshaders[V_COUNT];

static const char *vs_gs_src =
    "void main(float3 aPos, float3 aTex, float4 aCol, float aFog,\n"
    "          uniform float4 uXform,\n"
    "          float4 out vPos : POSITION, float3 out vTex : TEXCOORD0,\n"
    "          float4 out vCol : TEXCOORD1, float out vFog : TEXCOORD2)\n"
    "{\n"
    "    vPos = float4(aPos.x * uXform.x + uXform.z, aPos.y * uXform.y + uXform.w, aPos.z * 2.0 - 1.0, 1.0);\n"
    "    vTex = aTex;\n"
    "    vCol = aCol;\n"
    "    vFog = aFog;\n"
    "}\n";

/*
 * The VU1 microprogram's work for one vertex. Inputs are VU1_STRIP_BUF
 * qwords 0-2. Projection follows DrawScissorPolygon: x = Vx*AW*proj/Vz + OX,
 * z = K0 + K1/Vz clamped to [1, 65534], all kept homogeneous with W = Vz so
 * the GPU clips and interpolates perspective-correctly.
 *   uProj = AW*proj, AH*proj, OX - XYOFFSET.x, OY - XYOFFSET.y
 *   uZ    = K0, K1, Z normalisation, alpha ratio
 */
static const char *vs_vu1_head =
    "float3 sat3(float3 x) { return clamp(x, 0.0, 1.0); }\n"
    "void main(float4 aUV, float4 aI, float4 aView,\n"
    "          uniform float4 uXform, uniform float4 uProj, uniform float4 uZ,\n"
    "          uniform float4 uDif, uniform float4 uAmb, uniform float4 uSpe,\n"
    "          float4 out vPos : POSITION, float3 out vTex : TEXCOORD0,\n"
    "          float4 out vCol : TEXCOORD1, float out vFog : TEXCOORD2)\n"
    "{\n"
    "    float W = aView.z;\n"
    "    float xw = aView.x * uProj.x + uProj.z * W;\n"
    "    float yw = aView.y * uProj.y + uProj.w * W;\n"
    "    float z = floor(clamp(uZ.x + uZ.y / W, 1.0, 65534.0)) * uZ.z;\n"
    "    vPos = float4(xw * uXform.x + uXform.z * W, yw * uXform.y + uXform.w * W, (z * 2.0 - 1.0) * W, W);\n"
    "    vTex = float3(aUV.xy, 1.0);\n"
    "    vFog = fmod(floor(aView.w), 256.0) / 255.0;\n"
    "    float3 I = aI.xyz, c, s;\n"
    "    float a = uZ.w;\n";

static const char *vs_vu1_func[11] = {
    /* vu1GetVertexColor */      "    c = I * 128.0; a = aI.w * 255.0;\n",
    /* vu1GetVertexColorCM */    "    c = uDif.xyz;\n",
    /* vu1GetVertexColorIgnore */"    c = float3(128.0);\n",
    /* vu1GetVertexColorDif */   "    c = sat3(I) * uDif.xyz;\n",
    /* vu1GetVertexColorDifAmb */"    c = sat3(I + uAmb.xyz) * uDif.xyz;\n",
    /* DifSpe1 */    "    s = sat3(I - 1.0); c = sat3(I) * uDif.xyz + s * uSpe.xyz;\n",
    /* DifSpe2 */    "    s = sat3(I + uAmb.xyz - 1.0); c = sat3(I) * uDif.xyz + s * uSpe.xyz;\n",
    /* DifSpe3 */    "    s = I * I; s = s * s; s = s * s; s = s * s * I;\n"
                     "    c = sat3(I) * uDif.xyz + sat3(s) * uSpe.xyz;\n",
    /* DifSpe1Amb */ "    s = sat3(I - 1.0); c = sat3(I + uAmb.xyz) * uDif.xyz + s * uSpe.xyz;\n",
    /* DifSpe2Amb */ "    I = I + uAmb.xyz; s = sat3(I - 1.0); c = sat3(I) * uDif.xyz + s * uSpe.xyz;\n",
    /* DifSpe3Amb: the microcode seeds the power loop with I+1, not 1 */
                     "    I = I + uAmb.xyz; s = I * I; s = s * s; s = s * s; s = s * s * I * (I + 1.0);\n"
                     "    c = sat3(I) * uDif.xyz + sat3(s) * uSpe.xyz;\n",
};

static const char *vs_vu1_tail =
    /* FTOI0, then the GS keeps the low 8 bits of each RGBAQ field */
    "    float4 o = floor(max(float4(c, a), 0.0));\n"
    "    vCol = (o - 256.0 * floor(o / 256.0)) / 255.0;\n"
    "}\n";

static GLuint compile(GLenum type, const char *src)
{
    /* the sources are Cg; plain GL_*_SHADER would send them through vitaGL's GLSL translator */
    GLuint s = glCreateShader(type == GL_VERTEX_SHADER ? GL_CG_VERTEX_SHADER_EXT : GL_CG_FRAGMENT_SHADER_EXT);
    GLint len = strlen(src);
    glShaderSource(s, 1, &src, &len);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024] = "";
        glGetShaderInfoLog(s, sizeof(log), NULL, log);
        RECVX_LOG("gs: shader compile failed: %s\n%s", log, src);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

static GLuint vertex_shader(u_int vs)
{
    if (vshaders[vs]) return vshaders[vs];
    if (vs == VS_GS) return vshaders[vs] = compile(GL_VERTEX_SHADER, vs_gs_src);
    char src[4096];
    snprintf(src, sizeof(src), "%s%s%s", vs_vu1_head, vs_vu1_func[vs - 1], vs_vu1_tail);
    return vshaders[vs] = compile(GL_VERTEX_SHADER, src);
}

static GLuint fragment_shader(u_int v)
{
    if (fshaders[v]) return fshaders[v];

    static const char *ops[8] = { "false", "true", "<", "<=", "==", ">=", ">", "!=" };
    char fs[2048];
    int n = 0;
    u_int tfx = (v >> 1) & 3, atst = (v >> 5) & 7;

    n += sprintf(fs + n,
        "float4 main(float3 vTex : TEXCOORD0, float4 vCol : TEXCOORD1, float vFog : TEXCOORD2,\n"
        "            uniform sampler2D uTex, uniform float4 uUV, uniform float4 uParams,\n"
        "            uniform float3 uFogCol) : COLOR\n"
        "{\n"
        "    float4 c = vCol * 255.0;\n");
    if (v & V_TME) {
        n += sprintf(fs + n,
            "    float2 uv = vTex.xy / vTex.z * uUV.xy + uUV.zw;\n"
            "    float4 t = tex2D(uTex, uv) * 255.0;\n"
            "    t.a *= uParams.x;\n");
        switch (tfx) {
        case 0: n += sprintf(fs + n, "    c.rgb = c.rgb * t.rgb / 128.0;\n"); break;
        case 1: n += sprintf(fs + n, "    c.rgb = t.rgb;\n"); break;
        default: n += sprintf(fs + n, "    c.rgb = c.rgb * t.rgb / 128.0 + c.a;\n"); break;
        }
        if (v & V_TCC) {
            switch (tfx) {
            case 0: n += sprintf(fs + n, "    c.a = c.a * t.a / 128.0;\n"); break;
            case 2: n += sprintf(fs + n, "    c.a = c.a + t.a;\n"); break;
            default: n += sprintf(fs + n, "    c.a = t.a;\n"); break;
            }
        }
        n += sprintf(fs + n, "    c = min(c, 255.0);\n");
    }
    if (atst != 1)
        n += sprintf(fs + n, "    if (!(floor(c.a) %s uParams.y)) discard;\n", ops[atst]);
    if (v & V_FGE)
        n += sprintf(fs + n, "    c.rgb = lerp(uFogCol, c.rgb, vFog);\n");
    n += sprintf(fs + n, "    return float4(c.rgb / 255.0, %s / 128.0);\n}\n",
                 (v & V_FIXA) ? "uParams.z" : "c.a");
    if (atst == 0)  /* NEVER: everything fails */
        n = sprintf(fs, "float4 main() : COLOR { discard; return float4(0.0); }\n");

    return fshaders[v] = compile(GL_FRAGMENT_SHADER, fs);
}

static Shader *shader_get(u_int vs, u_int v)
{
    if (!shaders[vs]) shaders[vs] = calloc(V_COUNT, sizeof(Shader));
    Shader *sh = &shaders[vs][v];
    if (sh->prog) return sh;

    GLuint vsh = vertex_shader(vs), fsh = fragment_shader(v);
    if (!vsh || !fsh) {
        RECVX_LOG("gs: no program for vs %u variant 0x%x", vs, v);
        return NULL;
    }
    sh->prog = glCreateProgram();
    glAttachShader(sh->prog, vsh);
    glAttachShader(sh->prog, fsh);
    if (vs == VS_GS) {
        glBindAttribLocation(sh->prog, 0, "aPos");
        glBindAttribLocation(sh->prog, 1, "aTex");
        glBindAttribLocation(sh->prog, 2, "aCol");
        glBindAttribLocation(sh->prog, 3, "aFog");
    } else {
        glBindAttribLocation(sh->prog, 0, "aUV");
        glBindAttribLocation(sh->prog, 1, "aI");
        glBindAttribLocation(sh->prog, 2, "aView");
    }
    glLinkProgram(sh->prog);
    sh->u_xform = glGetUniformLocation(sh->prog, "uXform");
    sh->u_uv = glGetUniformLocation(sh->prog, "uUV");
    sh->u_params = glGetUniformLocation(sh->prog, "uParams");
    sh->u_fogcol = glGetUniformLocation(sh->prog, "uFogCol");
    if (vs != VS_GS) {
        sh->u_proj = glGetUniformLocation(sh->prog, "uProj");
        sh->u_z = glGetUniformLocation(sh->prog, "uZ");
        sh->u_dif = glGetUniformLocation(sh->prog, "uDif");
        sh->u_amb = glGetUniformLocation(sh->prog, "uAmb");
        sh->u_spe = glGetUniformLocation(sh->prog, "uSpe");
    }
    return sh;
}

/* ------------------------------------------------------------- draw state */

typedef struct {
    RenderTarget *rt;
    GLuint tex;
    float uv[4];
    float tex_ascale;
    GLenum tex_filter, wrap_s, wrap_t;
    u_int variant;
    float aref, fixa;
    float fogcol[3];
    GLenum mode;                       /* GL_TRIANGLES / GL_LINES / GL_POINTS */
    uint8_t blend, color_mask, depth_write, afail;
    GLenum bsrc, bdst, beq, depth_func;
    int scissor[4];
} DrawState;

typedef struct {
    float x, y, z;
    float s, t, q;
    uint8_t rgba[4];
    float fog;
} BatchVertex;

#define MAX_BATCH 12288
static BatchVertex batch[MAX_BATCH];
static int batch_n;
static DrawState cur, pending;
static int state_dirty = 1, have_state;

static void blend_factors(uint64_t alpha, DrawState *s)
{
    u_int a = GS_BITS(alpha, 0, 2), b = GS_BITS(alpha, 2, 2), c = GS_BITS(alpha, 4, 2), d = GS_BITS(alpha, 6, 2);
    GLenum C, IC;

    if (c == 1) { C = GL_DST_ALPHA; IC = GL_ONE_MINUS_DST_ALPHA; }
    else { C = GL_SRC_ALPHA; IC = GL_ONE_MINUS_SRC_ALPHA; }   /* As, or FIX via shader alpha */

    s->beq = GL_FUNC_ADD;
    if (a == b) {
        s->bsrc = d == 0 ? GL_ONE : GL_ZERO;
        s->bdst = d == 1 ? GL_ONE : GL_ZERO;
        return;
    }
    /* (A - B) * C + D */
    switch (a * 9 + b * 3 + d) {
    case 0 * 9 + 1 * 3 + 1: s->bsrc = C;    s->bdst = IC;   break;              /* Cs*C + Cd*(1-C) */
    case 0 * 9 + 1 * 3 + 2: s->bsrc = C;    s->bdst = C;    s->beq = GL_FUNC_SUBTRACT; break;
    case 0 * 9 + 2 * 3 + 1: s->bsrc = C;    s->bdst = GL_ONE; break;           /* Cs*C + Cd */
    case 0 * 9 + 2 * 3 + 2: s->bsrc = C;    s->bdst = GL_ZERO; break;
    case 1 * 9 + 0 * 3 + 0: s->bsrc = IC;   s->bdst = C;    break;              /* Cd*C + Cs*(1-C) */
    case 1 * 9 + 0 * 3 + 2: s->bsrc = C;    s->bdst = C;    s->beq = GL_FUNC_REVERSE_SUBTRACT; break;
    case 1 * 9 + 2 * 3 + 0: s->bsrc = GL_ONE; s->bdst = C;  break;              /* Cd*C + Cs */
    case 1 * 9 + 2 * 3 + 2: s->bsrc = GL_ZERO; s->bdst = C; break;
    case 2 * 9 + 0 * 3 + 1: s->bsrc = C;    s->bdst = GL_ONE; s->beq = GL_FUNC_REVERSE_SUBTRACT; break; /* Cd - Cs*C */
    case 2 * 9 + 0 * 3 + 0: s->bsrc = IC;   s->bdst = GL_ZERO; break;           /* Cs*(1-C) */
    case 2 * 9 + 1 * 3 + 0: s->bsrc = GL_ONE; s->bdst = C;  s->beq = GL_FUNC_SUBTRACT; break; /* Cs - Cd*C */
    case 2 * 9 + 1 * 3 + 1: s->bsrc = GL_ZERO; s->bdst = IC; break;             /* Cd*(1-C) */
    case 2 * 9 + 0 * 3 + 2:
    case 2 * 9 + 1 * 3 + 2: s->bsrc = GL_ZERO; s->bdst = GL_ZERO; break;
    default:
        RECVX_LOG_ONCE("gs: unsupported blend %u%u%u%u", a, b, c, d);
        s->bsrc = C; s->bdst = IC;
        break;
    }
}

static int rt_texture_for(uint64_t tex0, DrawState *s)
{
    u_int tbp = GS_BITS(tex0, 0, 14), tbw = GS_BITS(tex0, 14, 6), psm = GS_BITS(tex0, 20, 6);
    u_int tw = GS_BITS(tex0, 26, 4), th = GS_BITS(tex0, 30, 4);
    if (psm != SCE_GS_PSMCT32 && psm != SCE_GS_PSMCT24) return 0;

    for (int i = 0; i < MAX_RT; i++) {
        RenderTarget *rt = &rts[i];
        u_int first, last;
        if (!rt->used || rt->stale || rt->fbw != tbw) continue;
        rt_blocks(rt, &first, &last);
        if (tbp < first || tbp > last) continue;
        u_int off = tbp - first, row = rt->fbw * 32;
        if (off % 32) { RECVX_LOG_ONCE("gs: unaligned render-target texture"); return 0; }
        float x0 = (float)((off % row) / 32 * 64), y0 = (float)(off / row * 32);
        s->tex = rt->tex;
        s->uv[0] = (float)(1u << tw) / rt->w;
        s->uv[1] = (float)(1u << th) / rt->h;
        s->uv[2] = x0 / rt->w;
        s->uv[3] = y0 / rt->h;
        s->tex_ascale = 128.0f / 255.0f;
        return 1;
    }
    return 0;
}

static void compute_state(DrawState *s, int type)
{
    uint64_t attrs = GS_BITS(gs.prmodecont, 0, 1) ? gs.prim : gs.prmode;
    const GsContext *c = &gs.ctx[GS_BITS(attrs, 9, 1)];
    u_int fbp = GS_BITS(c->frame, 0, 9), fbw = GS_BITS(c->frame, 16, 6);
    uint32_t fbmsk = (uint32_t)(c->frame >> 32);
    uint64_t test = c->test;

    memset(s, 0, sizeof(*s));
    s->rt = rt_get(fbp, fbw);
    s->mode = type == GS_DRAW_LINES ? GL_LINES : type == GS_DRAW_POINTS ? GL_POINTS : GL_TRIANGLES;

    u_int v = 0;
    if (GS_BITS(attrs, 4, 1)) {
        uint64_t tex0 = c->tex0;
        u_int tex1 = (u_int)c->tex1;
        v |= V_TME | V_TFX(GS_BITS(tex0, 35, 2));
        if (GS_BITS(tex0, 34, 1)) v |= V_TCC;
        if (!rt_texture_for(tex0, s)) {
            s->tex = tex_lookup(tex0);
            s->uv[0] = s->uv[1] = 1.0f;
            s->tex_ascale = 1.0f;
        }
        s->tex_filter = GS_BITS(tex1, 5, 1) ? GL_LINEAR : GL_NEAREST;
        s->wrap_s = GS_BITS(c->clamp, 0, 2) == SCE_GS_REPEAT ? GL_REPEAT : GL_CLAMP_TO_EDGE;
        s->wrap_t = GS_BITS(c->clamp, 2, 2) == SCE_GS_REPEAT ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    }
    if (GS_BITS(attrs, 5, 1)) {
        v |= V_FGE;
        s->fogcol[0] = GS_BITS(gs.fogcol, 0, 8);
        s->fogcol[1] = GS_BITS(gs.fogcol, 8, 8);
        s->fogcol[2] = GS_BITS(gs.fogcol, 16, 8);
    }

    u_int ate = GS_BITS(test, 0, 1), atst = ate ? GS_BITS(test, 1, 3) : 1;
    s->aref = GS_BITS(test, 4, 8);
    s->afail = (atst != 1) ? GS_BITS(test, 12, 2) : 0;
    v |= V_ATST(atst);
    if (GS_BITS(test, 14, 1)) RECVX_LOG_ONCE("gs: destination alpha test not supported");

    s->blend = GS_BITS(attrs, 6, 1);
    if (s->blend) {
        blend_factors(c->alpha, s);
        if (GS_BITS(c->alpha, 4, 2) == 2) { v |= V_FIXA; s->fixa = GS_BITS(c->alpha, 32, 8); }
    }

    if (GS_BITS(test, 16, 1)) {
        static const GLenum zf[4] = { GL_NEVER, GL_ALWAYS, GL_GEQUAL, GL_GREATER };
        s->depth_func = zf[GS_BITS(test, 17, 2)];
        s->depth_write = !GS_BITS(c->zbuf, 32, 1);
    } else {
        s->depth_func = GL_ALWAYS;
        s->depth_write = 0;
    }

    s->color_mask = ((fbmsk & 0xff) != 0xff) | ((fbmsk & 0xff00) != 0xff00) << 1 |
                    ((fbmsk & 0xff0000) != 0xff0000) << 2 | ((fbmsk >> 24) != 0xff) << 3;
    if (fbmsk && fbmsk != 0xff000000 && fbmsk != 0xffffffff)
        RECVX_LOG_ONCE("gs: partial FBMSK %08x", fbmsk);

    s->scissor[0] = GS_BITS(c->scissor, 0, 11);
    s->scissor[1] = GS_BITS(c->scissor, 32, 11);
    s->scissor[2] = GS_BITS(c->scissor, 16, 11) - s->scissor[0] + 1;
    s->scissor[3] = GS_BITS(c->scissor, 48, 11) - s->scissor[1] + 1;
    s->variant = v;
}

/* --------------------------------------------------------------- GL apply */

static u_int stat_gs_draws, stat_vu1_draws;

static Shader *apply_state(const DrawState *s, u_int vs, u_int variant, int color_mask, int depth_write)
{
    Shader *sh = shader_get(vs, variant);
    RenderTarget *rt = s->rt;
    float sc = rt_scale;
    if (!sh) return NULL;

    int bottom = s->scissor[1] + s->scissor[3];
    if (bottom > rt->drawn_h) rt->drawn_h = bottom < rt->h ? bottom : rt->h;
    rt->stale = 0;
    {
        int b0 = s->scissor[1] / 32, b1 = (bottom - 1) / 32;
        if (b1 > 31) b1 = 31;
        for (int b = b0 < 0 ? 0 : b0; b <= b1; b++) rt->dirty |= 1u << b;
    }
    if (bound_rt != rt) {
        glBindFramebuffer(GL_FRAMEBUFFER, rt->fbo);
        glViewport(0, 0, (int)(rt->w * sc), (int)(rt->h * sc));
        bound_rt = rt;
    }
    glUseProgram(sh->prog);
    glUniform4f(sh->u_xform, 2.0f / rt->w, 2.0f / rt->h, -1.0f, -1.0f);
    if (variant & V_TME) {
        glBindTexture(GL_TEXTURE_2D, s->tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, s->tex_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, s->tex_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s->wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, s->wrap_t);
        glUniform4fv(sh->u_uv, 1, s->uv);
    }
    glUniform4f(sh->u_params, s->tex_ascale, s->aref, s->fixa, 0);
    if (variant & V_FGE) glUniform3f(sh->u_fogcol, s->fogcol[0], s->fogcol[1], s->fogcol[2]);

    if (s->blend) {
        glEnable(GL_BLEND);
        glBlendFuncSeparate(s->bsrc, s->bdst, GL_ONE, GL_ZERO);
        glBlendEquationSeparate(s->beq, GL_FUNC_ADD);
    } else {
        glDisable(GL_BLEND);
    }
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(s->depth_func);
    glDepthMask(depth_write ? GL_TRUE : GL_FALSE);
    glColorMask(color_mask & 1, (color_mask >> 1) & 1, (color_mask >> 2) & 1, (color_mask >> 3) & 1);
    glEnable(GL_SCISSOR_TEST);
    glScissor((int)(s->scissor[0] * sc), (int)(s->scissor[1] * sc),
              (int)(s->scissor[2] * sc), (int)(s->scissor[3] * sc));
    return sh;
}

static void draw_gs_batch(const DrawState *s, u_int variant, int color_mask, int depth_write)
{
    if (!apply_state(s, VS_GS, variant, color_mask, depth_write)) return;
    stat_gs_draws++;
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), &batch[0].x);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), &batch[0].s);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BatchVertex), &batch[0].rgba);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), &batch[0].fog);
    glDrawArrays(s->mode, 0, batch_n);
}

/* ------------------------------------------------------------ VU1 batches */

typedef struct {
    u_int func;
    float proj[4], z[4], dif[4], amb[4], spe[4];
} Vu1Uniforms;

#define MAX_VU1_V 8192
#define MAX_VU1_I 24576
static float vu1_v[MAX_VU1_V][12];      /* VU1_STRIP_BUF qwords 0-2 */
static uint16_t vu1_i[MAX_VU1_I];
static int vu1_nv, vu1_ni;
static Vu1Uniforms vu1_u;

static void draw_vu1_batch(const DrawState *s, u_int variant, int color_mask, int depth_write)
{
    Shader *sh = apply_state(s, 1 + vu1_u.func, variant, color_mask, depth_write);
    if (!sh) return;
    stat_vu1_draws++;
    glUniform4fv(sh->u_proj, 1, vu1_u.proj);
    glUniform4fv(sh->u_z, 1, vu1_u.z);
    glUniform4fv(sh->u_dif, 1, vu1_u.dif);
    glUniform4fv(sh->u_amb, 1, vu1_u.amb);
    glUniform4fv(sh->u_spe, 1, vu1_u.spe);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vu1_v[0]), &vu1_v[0][0]);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vu1_v[0]), &vu1_v[0][4]);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vu1_v[0]), &vu1_v[0][8]);
    glDrawElements(GL_TRIANGLES, vu1_ni, GL_UNSIGNED_SHORT, vu1_i);
}

/* ---------------------------------------------------------------- batching */

enum { B_NONE, B_GS, B_VU1 };
static int batch_kind;

static u_int invert_atst(u_int v)
{
    static const u_int inv[8] = { 1, 0, 5, 6, 7, 2, 3, 4 };
    return (v & ~V_ATST(7)) | V_ATST(inv[(v >> 5) & 7]);
}

void gs_draw_flush(void)
{
    int empty = batch_kind == B_GS ? !batch_n : batch_kind == B_VU1 ? !vu1_ni : 1;
    if (empty || !have_state) { batch_n = vu1_nv = vu1_ni = 0; return; }

    void (*draw)(const DrawState *, u_int, int, int) = batch_kind == B_GS ? draw_gs_batch : draw_vu1_batch;
    const DrawState *s = &cur;
    u_int atst = (s->variant >> 5) & 7;

    if (atst != 0)
        draw(s, s->variant, s->color_mask, s->depth_write);
    if (s->afail) {
        /* pixels failing the alpha test still update part of the frame */
        int cm = s->color_mask, dw = 0;
        switch (s->afail) {
        case 1: break;                              /* FB_ONLY */
        case 2: cm = 0; dw = s->depth_write; break; /* ZB_ONLY */
        case 3: cm &= 7; break;                     /* RGB_ONLY */
        }
        draw(s, invert_atst(s->variant), cm, dw);
    }
    batch_n = vu1_nv = vu1_ni = 0;
}

void gs_draw_state_changed(void) { state_dirty = 1; }

static inline void push(const GsVertex *v, int k)
{
    BatchVertex *b = &batch[batch_n++];
    b->x = v[k].x; b->y = v[k].y; b->z = v[k].z;
    b->s = v[k].s; b->t = v[k].t; b->q = v[k].q;
    b->rgba[0] = v[k].r; b->rgba[1] = v[k].g; b->rgba[2] = v[k].b; b->rgba[3] = v[k].a;
    b->fog = v[k].fog;
}

/* recompute the draw state if anything changed; flush when it differs */
static void update_state(int kind, int type)
{
    static int last_type = -1;
    if (batch_kind != kind) {
        gs_draw_flush();
        batch_kind = kind;
        state_dirty = 1;
    }
    if (state_dirty || type != last_type) {
        compute_state(&pending, type);
        state_dirty = 0;
        last_type = type;
        if (!have_state || memcmp(&pending, &cur, sizeof(cur))) {
            gs_draw_flush();
            cur = pending;
            have_state = 1;
        }
    }
}

void gs_draw_prim(int type, const GsVertex *v, int n)
{
    update_state(B_GS, type);
    if (batch_n + 6 > MAX_BATCH) gs_draw_flush();

    if (type == GS_DRAW_SPRITES) {
        /* two corners -> two triangles; colour, z and fog from the second */
        GsVertex q[4];
        q[0] = v[1]; q[0].x = v[0].x; q[0].y = v[0].y; q[0].s = v[0].s; q[0].t = v[0].t; q[0].q = v[0].q;
        q[1] = v[1]; q[1].y = v[0].y; q[1].t = v[0].t;
        q[2] = v[1]; q[2].x = v[0].x; q[2].s = v[0].s;
        q[3] = v[1];
        push(q, 0); push(q, 1); push(q, 2);
        push(q, 1); push(q, 3); push(q, 2);
        return;
    }
    for (int i = 0; i < n; i++) push(v, i);
}

static void gs_draw_vu1_(const Vu1Batch *b);

void gs_draw_vu1(const Vu1Batch *b)
{
    SceUInt64 t0 = sceKernelGetProcessTimeWide();
    gs_draw_vu1_(b);
    prof_vu1_us += sceKernelGetProcessTimeWide() - t0;
}

static void gs_draw_vu1_(const Vu1Batch *b)
{
    u_int ptype = b->prim & 7;
    if (ptype < SCE_GS_PRIM_TRI || ptype > SCE_GS_PRIM_TRIFAN) {
        RECVX_LOG_ONCE("vu1: unexpected primitive type %u", ptype);
        return;
    }
    gs_write_reg(SCE_GS_PRIM, b->prim);
    if (!GS_BITS(b->prim, 3, 1)) RECVX_LOG_ONCE("vu1: flat-shaded strip drawn gouraud");
    update_state(B_VU1, GS_DRAW_TRIS);

    /* uniforms from the constant block, as DoubleMain/SingleMain derive them */
    const float (*k)[4] = b->consts;
    uint64_t attrs = GS_BITS(gs.prmodecont, 0, 1) ? gs.prim : gs.prmode;
    const GsContext *c = &gs.ctx[GS_BITS(attrs, 9, 1)];
    float near = k[3][0], far = k[3][1], proj = k[3][2];
    u_int zpsm = GS_BITS(c->zbuf, 24, 4) | 0x30;
    Vu1Uniforms u;
    memset(&u, 0, sizeof(u));
    u.func = b->func;
    u.proj[0] = k[9][0] * proj;
    u.proj[1] = k[9][1] * proj;
    u.proj[2] = k[8][0] - GS_BITS(c->xyoffset, 0, 16) / 16.0f;
    u.proj[3] = k[8][1] - GS_BITS(c->xyoffset, 32, 16) / 16.0f;
    u.z[0] = near * 65536.0f / (far - near);
    u.z[1] = near * 65536.0f * far / (far - near);
    u.z[2] = zpsm == SCE_GS_PSMZ24 ? 1.0f / 16777216.0f :
             zpsm == SCE_GS_PSMZ32 ? 1.0f / 4294967296.0f : 1.0f / 65536.0f;
    u.z[3] = k[3][3];
    memcpy(u.dif, k[0], 16);
    memcpy(u.amb, k[1], 16);
    memcpy(u.spe, k[2], 16);

    int ntri = ptype == SCE_GS_PRIM_TRI ? (int)b->n / 3 : (int)b->n - 2;
    if (ntri <= 0) return;
    if (vu1_ni && memcmp(&u, &vu1_u, sizeof(u))) gs_draw_flush();
    if (vu1_nv + (int)b->n > MAX_VU1_V || vu1_ni + ntri * 3 > MAX_VU1_I) gs_draw_flush();
    vu1_u = u;

    int base = vu1_nv;
    for (u_int i = 0; i < b->n; i++) memcpy(vu1_v[vu1_nv++], b->verts[i * 4], 48);
    for (int t = 0; t < ntri; t++) {
        uint16_t *ix = &vu1_i[vu1_ni];
        switch (ptype) {
        case SCE_GS_PRIM_TRI:      ix[0] = base + t * 3; ix[1] = base + t * 3 + 1; ix[2] = base + t * 3 + 2; break;
        case SCE_GS_PRIM_TRISTRIP: ix[0] = base + t; ix[1] = base + t + 1; ix[2] = base + t + 2; break;
        case SCE_GS_PRIM_TRIFAN:   ix[0] = base; ix[1] = base + t + 1; ix[2] = base + t + 2; break;
        }
        vu1_ni += 3;
    }
}

/* ---------------------------------------------------- memory coherence */

static void blit_to_rt(RenderTarget *rt, u_int x, u_int y, u_int w, u_int h)
{
    /* decode the uploaded rectangle and draw it into the render target */
    uint64_t tex0 = (uint64_t)(rt->fbp * 32) | (uint64_t)rt->fbw << 14;  /* PSMCT32 */
    uint32_t *buf = malloc(w * h * 4);
    if (!buf) return;
    for (u_int j = 0; j < h; j++)
        for (u_int i = 0; i < w; i++)
            buf[j * w + i] = gs_mem_read_pixel(SCE_GS_PSMCT32, rt->fbp * 32, rt->fbw, x + i, y + j);
    (void)tex0;

    GLuint t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
    free(buf);

    DrawState s;
    memset(&s, 0, sizeof(s));
    s.rt = rt;
    s.tex = t;
    s.uv[0] = s.uv[1] = 1.0f;
    s.tex_ascale = 1.0f;
    s.tex_filter = GL_NEAREST;
    s.wrap_s = s.wrap_t = GL_CLAMP_TO_EDGE;
    s.mode = GL_TRIANGLES;
    s.depth_func = GL_ALWAYS;
    s.scissor[2] = rt->w;
    s.scissor[3] = (int)(y + h) < rt->h ? (int)(y + h) : rt->h;   /* only the uploaded rows count as drawn */
    u_int variant = V_TME | V_TFX(1) | V_TCC | V_ATST(1);

    GsVertex q[2] = {
        { (float)x, (float)y, 0, 0, 0, 1, 128, 128, 128, 128, 1 },
        { (float)(x + w), (float)(y + h), 0, 1, 1, 1, 128, 128, 128, 128, 1 },
    };
    int saved = batch_n;
    batch_n = 0;
    GsVertex c[4] = { q[0], q[1], q[0], q[1] };
    c[1].y = q[0].y; c[1].t = 0; c[2].x = q[0].x; c[2].s = 0;
    push(c, 0); push(c, 1); push(c, 2); push(c, 1); push(c, 3); push(c, 2);
    draw_gs_batch(&s, variant, 15, 0);
    batch_n = saved;
    glDeleteTextures(1, &t);
}

void gs_draw_mem_written(u_int psm, u_int bp, u_int bw, u_int x, u_int y, u_int w, u_int h)
{
    u_int first, last;
    gs_draw_flush();
    state_dirty = 1;
    gs_mem_block_range(psm, bp, bw ? bw : 1, x + w, y + h, &first, &last);
    tex_invalidate(first, last);

    for (int i = 0; i < MAX_RT; i++) {
        RenderTarget *rt = &rts[i];
        u_int rf, rl;
        if (!rt->used || rt->stale || !rt_drawn_blocks(rt, &rf, &rl)) continue;
        if (last < rf || first > rl) continue;
        if ((psm == SCE_GS_PSMCT32 || psm == SCE_GS_PSMCT24) && bw == rt->fbw && bp == rt->fbp * 32) {
            blit_to_rt(rt, x, y, w, h);
        } else {
            /* keep the GL objects: deleting them every frame churns vitaGL's GC */
            RECVX_LOG_ONCE("gs: upload psm=%x bp=%u overwrote render target fbp=%u (stale until redrawn)", psm, bp, rt->fbp);
            rt->stale = 1;
            rt->drawn_h = 0;
        }
    }
}

void gs_draw_mem_read(u_int psm, u_int bp, u_int bw, u_int x, u_int y, u_int w, u_int h)
{
    u_int first, last;
    gs_mem_block_range(psm, bp, bw ? bw : 1, x + w, y + h, &first, &last);
    for (int i = 0; i < MAX_RT; i++) {
        RenderTarget *rt = &rts[i];
        u_int rf, rl;
        /* GS memory is already current unless the target was drawn since the last readback */
        if (!rt->used || rt->stale || !rt_drawn_blocks(rt, &rf, &rl)) continue;
        if (last < rf || first > rl) continue;

        /* read back only the 32-row page bands the access touches */
        u_int band = rt->fbw * 32;
        int r0 = (int)((first > rf ? first - rf : 0) / band) * 32;
        int r1 = (int)((last - rf) / band + 1) * 32;
        if (r1 > rt->drawn_h) r1 = rt->drawn_h;
        /* skip clean bands at either end */
        while (r0 < r1 && !(rt->dirty >> (r0 / 32) & 1)) r0 += 32;
        while (r1 > r0 && !(rt->dirty >> ((r1 - 1) / 32) & 1)) r1 = (r1 - 1) / 32 * 32;
        if (r0 >= r1) continue;

        SceUInt64 t0 = sceKernelGetProcessTimeWide();
        gs_draw_flush();
        int pw = (int)(rt->w * rt_scale);
        int py0 = (int)(r0 * rt_scale), ph = (int)(r1 * rt_scale) - py0;
        uint32_t *px = malloc(pw * ph * 4);
        if (!px) return;
        glBindFramebuffer(GL_FRAMEBUFFER, rt->fbo);
        bound_rt = NULL;
        glReadPixels(0, py0, pw, ph, GL_RGBA, GL_UNSIGNED_BYTE, px);
        for (int yy = r0; yy < r1; yy++) {
            const uint32_t *line = px + ((int)(yy * rt_scale) - py0) * pw;
            for (int xx = 0; xx < rt->w; xx++) {
                uint32_t c = line[(int)(xx * rt_scale)];
                uint32_t a = (c >> 24) * 128 / 255;   /* back to GS alpha */
                gs_mem_write_pixel(SCE_GS_PSMCT32, rt->fbp * 32, rt->fbw, xx, yy, (c & 0xffffff) | a << 24);
            }
        }
        free(px);
        for (int b = r0 / 32; b * 32 < r1; b++) rt->dirty &= ~(1u << b);
        prof_readback_us += sceKernelGetProcessTimeWide() - t0;
        prof_readbacks++;
    }
}

/* ---------------------------------------------------------------- present */

/* ---------------------------------------------------------- telemetry */

static u_int frame_no;
unsigned gs_frame_count(void) { return frame_no; }
static u_int capture_frames[16];
static int ncapture = -1;

/* capture.txt on the device lists frame numbers to save as frame-N.ppm */
static void capture_load(void)
{
    ncapture = 0;
    FILE *f = fopen(RECVX_DATA_DIR "/capture.txt", "r");
    if (!f) return;
    u_int n;
    while (ncapture < 16 && fscanf(f, "%u", &n) == 1) capture_frames[ncapture++] = n;
    fclose(f);
}

static void capture_frame(void)
{
    static uint8_t px[SCREEN_W * SCREEN_H * 4];
    char path[64];
    glReadPixels(0, 0, SCREEN_W, SCREEN_H, GL_RGBA, GL_UNSIGNED_BYTE, px);
    snprintf(path, sizeof(path), RECVX_DATA_DIR "/frame-%u.ppm", frame_no);
    FILE *f = fopen(path, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", SCREEN_W, SCREEN_H);
    static uint8_t row[SCREEN_W * 3];
    for (int y = SCREEN_H - 1; y >= 0; y--) {   /* GL rows are bottom-up */
        for (int x = 0; x < SCREEN_W; x++)
            memcpy(&row[x * 3], &px[(y * SCREEN_W + x) * 4], 3);
        fwrite(row, 1, sizeof(row), f);
    }
    fclose(f);
    RECVX_LOG("capture frame=%u", frame_no);
}

static void telemetry(u_int fbp)
{
    static SceUInt64 t0;
    static u_int f0, gs0, vu0;
    if (ncapture < 0) capture_load();
    for (int i = 0; i < ncapture; i++)
        if (capture_frames[i] == frame_no) capture_frame();
    SceUInt64 now = sceKernelGetProcessTimeWide();
    if (!t0) t0 = now;
    if (now - t0 >= 5000000) {
        float secs = (now - t0) / 1e6f;
        u_int nf = frame_no - f0 ? frame_no - f0 : 1;
        RECVX_LOG("frame=%u fps=%.1f fbp=%u draws/frame gs=%u vu1=%u", frame_no,
                  (frame_no - f0) / secs, fbp,
                  (stat_gs_draws - gs0) / nf, (stat_vu1_draws - vu0) / nf);
        RECVX_LOG("prof ms/frame: swap=%.1f readback=%.1f (%u) texdecode=%.1f (%u) vu1=%.1f",
                  prof_frame_us / 1000.0f / nf, prof_readback_us / 1000.0f / nf, prof_readbacks / nf,
                  prof_texdecode_us / 1000.0f / nf, prof_texmiss / nf, prof_vu1_us / 1000.0f / nf);
        prof_frame_us = prof_readback_us = prof_texdecode_us = prof_vu1_us = 0;
        prof_readbacks = prof_texmiss = 0;
        t0 = now; f0 = frame_no; gs0 = stat_gs_draws; vu0 = stat_vu1_draws;
    }
    frame_no++;
}

static void present_(u_int fbp, u_int fbw, u_int psm, u_int w, u_int h)
{
    (void)fbw; (void)psm;
    gs_draw_flush();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    bound_rt = NULL;
    glViewport(0, 0, SCREEN_W, SCREEN_H);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderTarget *rt = rt_find(fbp);
    Shader *sh = shader_get(VS_GS, V_TME | V_TFX(1) | V_TCC | V_ATST(1));
    if (rt && sh) {
        /* 4:3 pillarbox */
        float dw = SCREEN_H * 4.0f / 3.0f, x0 = (SCREEN_W - dw) / 2;
        float u1 = (float)w / rt->w, v1 = (float)h / rt->h;
        glUseProgram(sh->prog);
        /* screen space: y down, so flip */
        glUniform4f(sh->u_xform, 2.0f / SCREEN_W, -2.0f / SCREEN_H, -1.0f, 1.0f);
        glUniform4f(sh->u_uv, 1, 1, 0, 0);
        glUniform4f(sh->u_params, 0, 0, 0, 0);
        glBindTexture(GL_TEXTURE_2D, rt->tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        BatchVertex q[6];
        float xs[4] = { x0, x0 + dw, x0, x0 + dw }, ys[4] = { 0, 0, SCREEN_H, SCREEN_H };
        float us[4] = { 0, u1, 0, u1 }, vs[4] = { 0, 0, v1, v1 };
        int idx[6] = { 0, 1, 2, 1, 3, 2 };
        for (int i = 0; i < 6; i++) {
            int k = idx[i];
            q[i].x = xs[k]; q[i].y = ys[k]; q[i].z = 0.5f;
            q[i].s = us[k]; q[i].t = vs[k]; q[i].q = 1;
            q[i].rgba[0] = q[i].rgba[1] = q[i].rgba[2] = q[i].rgba[3] = 255;
            q[i].fog = 1;
        }
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), &q[0].x);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), &q[0].s);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BatchVertex), &q[0].rgba);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), &q[0].fog);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    telemetry(fbp);
    SceUInt64 ts = sceKernelGetProcessTimeWide();
    vglSwapBuffers(GL_FALSE);
    prof_frame_us += sceKernelGetProcessTimeWide() - ts;
    state_dirty = 1;
}

void gs_draw_present(u_int fbp, u_int fbw, u_int psm, u_int w, u_int h)
{
    gs_lock();
    present_(fbp, fbw, psm, w, h);
    gs_unlock();
}

/* ------------------------------------------------------------------- init */

static SceKernelLwMutexWork gs_mutex;

void gs_lock(void) { sceKernelLockLwMutex(&gs_mutex, 1, NULL); }
void gs_unlock(void) { sceKernelUnlockLwMutex(&gs_mutex, 1); }

void gs_draw_init(void)
{
    sceKernelCreateLwMutex(&gs_mutex, "gs", SCE_KERNEL_MUTEX_ATTR_RECURSIVE, 0, NULL);
    vglInitExtended(0, SCREEN_W, SCREEN_H, 16 * 1024 * 1024, SCE_GXM_MULTISAMPLE_NONE);
    vglWaitVblankStart(GL_FALSE);
    decode_buf = malloc(1024 * 1024 * 4);
    RECVX_LOG("gs: vitaGL up");
}
