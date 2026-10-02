#ifndef _PS2_DUMMY_H_
#define _PS2_DUMMY_H_

#include "types.h"

typedef struct PS2_OT_LIST
{
    // total size: 0x0
    int pad; // this isn't really here, but blocks an error
} PS2_OT_LIST;

typedef struct PS2_OT
{
    // total size: 0x30
	NJS_TEXMEMLIST* tp;       // offset 0x0, size 0x4
	unsigned char mode_bk[2]; // offset 0x4, size 0x2
    unsigned char set_last;   // offset 0x6, size 0x1
    unsigned char dc_alpha;   // offset 0x7, size 0x1
    PS2_OT_LIST* op;          // offset 0x8, size 0x4
    void* p;                  // offset 0xC, size 0x4
    u_long ALPHA;      // offset 0x10, size 0x4
    u_long TEX0;       // offset 0x18, size 0x4
    u_long TEX0_NEXT;  // offset 0x20, size 0x4
    unsigned int bank;        // offset 0x28, size 0x4
    unsigned int pad;         // offset 0x2C, size 0x4
} PS2_OT;

typedef struct PS2_TP_TAG
{
    // total size: 0x40
	u_long GIF_TAG[2];    // offset 0x0, size 0x8
    u_long TEX0;          // offset 0x10, size 0x4
    u_long TEX0_TAG;      // offset 0x18, size 0x4
    u_long TEX0_NEXT;     // offset 0x20, size 0x4
    u_long TEX0_NEXT_TAG; // offset 0x28, size 0x4
    u_long CLAMP;         // offset 0x30, size 0x4
    u_long CLAMP_TAG;     // offset 0x38, size 0x4
} PS2_TP_TAG;

typedef struct PS2_TP_CACHE
{
    // total size: 0x10
	void* tp;              // offset 0x0, size 0x4
    unsigned int bank;     // offset 0x4, size 0x4
    unsigned int tex_addr; // offset 0x8, size 0x4
    unsigned int clt_addr; // offset 0xC, size 0x4
} PS2_TP_CACHE;

// modified struct
typedef struct PS2_GS_SAVE
{
	u_long SC_TAG[2];          // offset 0x0, size 0x8
    u_long GIF_TAG[2];         // offset 0x10, size 0x8
    u_long TEX0;               // offset 0x20, size 0x4
    u_long TEX0_TAG;           // offset 0x28, size 0x4
    u_long TEX0_NEXT;          // offset 0x30, size 0x4
    u_long TEX0_NEXT_TAG;      // offset 0x38, size 0x4
    u_long TEX1;               // offset 0x40, size 0x4
    u_long TEX1_TAG;           // offset 0x48, size 0x4
    u_long CLAMP;              // offset 0x50, size 0x4
    u_long CLAMP_TAG;          // offset 0x58, size 0x4
    u_long TEST;               // offset 0x60, size 0x4
    u_long TEST_TAG;           // offset 0x68, size 0x4
    u_long FOGCOL;             // offset 0x70, size 0x4
    u_long FOGCOL_TAG;         // offset 0x78, size 0x4
    u_long ALPHA;              // offset 0x80, size 0x4
    u_long ALPHA_TAG;          // offset 0x88, size 0x4
    u_long FBA;                // offset 0x90, size 0x4
    u_long FBA_TAG;            // offset 0x98, size 0x4
    u_long SCISSOR;            // offset 0xA0, size 0x4
    u_long SCISSOR_TAG;        // offset 0xA8, size 0x4
    union 
    {
        struct 
        {
            unsigned char mode_bk[2]; // offset 0xB0, size 0x2
            unsigned char set_last;   // offset 0xB2, size 0x1
            unsigned char dc_alpha;   // offset 0xB3, size 0x1
            unsigned int pad32;       // offset 0xB4, size 0x4
        };
		
        u_long dmy;            // offset 0xB8, size 0x4
    };

    u_long pad64;              // offset 0xB8, size 0x4
} PS2_GS_SAVE;

void _builtin_set_imask(int mask);
void Ps2Init();
void Ps2LoadModule(char* p);
void Snd_init();
void Cd_init();
void Card_init();
void PS2_jikken();
void PS2_swap();
void Ps2AddPrim(u_long prim, void* dp, unsigned int num, unsigned int clip_3d_on);
void Ps2AddPrim2D(u_long prim, void* dp, unsigned int num);
void Ps2AddPrim3D(u_long prim, void* dp, unsigned int num);
void Ps2AddPrim3DEx(u_long prim, void* dp, unsigned int num);
void Ps2AddPrim3DEx1P(u_long prim, void* dp, unsigned int num);
void Ps2AddPrim3DMod(u_long prim, void* dp, unsigned int num);
void Ps2ClearOT();
void Ps2AddOT(void* p, unsigned int num, float z, u_long prim);
void Ps2DrawOTag();
int Ps2DrawOTagSub(int start_no);
unsigned int Ps2BitCount(unsigned int value);
void Ps2InitTexCache();
int Ps2GlobalIndexTexLoad(unsigned int index);
int Ps2TexLoad(NJS_TEXMEMLIST* addr);
void Ps2SetFogColor();
void Ps2SetFogColorSys(unsigned int r, unsigned int g, unsigned int b);
void Ps2AlphaIs000(unsigned int* cp, unsigned int num);
unsigned int Ps2AlphaIsHalf(unsigned int* cp, unsigned int num);
unsigned int Ps2Alpha4to8(unsigned int* cp, unsigned int num);
int Ps2CheckTextureAlpha(void* pp);
void Ps2InitPS2_GS_SAVE();
void Ps2ScreenClear();
void Ps2DispScreenClear();
void Ps2ZbuffOff();
void Ps2ZbuffOff2();
void Ps2ZbuffOn();
void Ps2ShadowStart();
void Ps2ShadowDraw();
void Ps2ShadowMain0();
void Ps2ShadowMain1();
void Ps2ShadowEnd();
void Ps2Vu0ProgSend(unsigned int prog_no);
void Ps2Vu1ProgSend(unsigned int prog_no);
void Ps2AddPrim3DExI(u_long prim, void* dp, unsigned int num);
void PS2_Render_Tex_Sub();

extern unsigned int PS2_Render_tex_sub_flag;
extern float Ps2AddPrimPrio;
extern int ViewType;
extern unsigned char Ps2_PBUFF[1835008];
extern unsigned int Ps2_use_pt_flag;
extern sceGsDBuffDc Db;
extern PS2_GS_SAVE Ps2_gs_save;
extern unsigned int Ps2_albinoid_flag;
extern unsigned int Ps2_sys_cnt;
extern unsigned char* Ps2_MOVIE;
extern unsigned int Ps2_ice_flag;
extern unsigned int palbuf[4096];
extern float mbuf[128][16];
extern float cmmat[2][16];
extern NJS_MATRIX crmat;
extern float Ps2_shadow_fog;
extern float Ps2_shadow_vec[4];
extern int Ps2_shadow_z;
extern NJS_MATRIX lcmat[12];
extern Uint8* _BSG_END;
extern NJS_VERTEX_BUF* _nj_vertex_buf_;
extern unsigned int _nj_control_3d_flag_;
extern NJS_SCREEN _nj_screen_;
extern unsigned char Ps2_DRAW_TMP[16384];
extern unsigned int Ps2_njControl3D_flag;
extern void (* VsyncFunc)();
extern void (* EorFunc)();
extern volatile unsigned int Ps2_vcount; /* bumped by the vsync handler */
extern unsigned int Ps2_dbuff;
extern unsigned int Ps2_tex_load_tp_cancel;
extern NJS_TEXMEMLIST* Ps2_now_tex;

#endif
