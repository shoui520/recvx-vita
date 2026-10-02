#include "../../../ps2/veronica/prog/ps2_Vu1Scissor2.h"
#include "../../../ps2/veronica/prog/ps2_Vu1Strip.h"
#include "../../../ps2/veronica/prog/ps2_dummy.h"

extern void VU0_CLIP_VIEW_VOLUME() __attribute__((section(".vutext")));
extern void VU0_CLIP_VIEW_VOLUME_ALL() __attribute__((section(".vutext")));
extern void VU0_LOAD_SCISSOR_WORK() __attribute__((section(".vutext")));
extern void VU0_LOAD_SCISSOR_WORKi() __attribute__((section(".vutext")));
extern void VU0_LOAD_SCISSOR_WORKb() __attribute__((section(".vutext")));
extern void VU0_SET_NODE_ARRAY() __attribute__((section(".vutext")));
extern void VU0_STORE_SCISSOR_WORK() __attribute__((section(".vutext")));

// 98.80% matching
void DrawScissorPolygonOpaque2(int count, u_long ulType)
{
    VU1_PRIM_BUF* pPrim; 

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |         lw        t0, 0(%1) */
    /* |          */
    /* |         addi      t1, zero, 80 */
    /* |      */
    /* |         ctc2      t1, vi3 */
    /* |          */
    /* |         l_002D3B2C: */
    /* |         vcallms   VU0_LOAD_SCISSOR_WORKi */
    /* |      */
    /* |         vdiv      Q, vf0w, vf9z */
    /* |          */
    /* |         vmulx.w   vf10, vf0, vf0x */
    /* |         vmul.xy   vf9,  vf9, vf17 */
    /* |          */
    /* |         vaddw.z   vf10, vf0, vf0w */
    /* |      */
    /* |         addi      t0, t0, -1 */
    /* |         addi      %0, %0, sizeof(VU1_PRIM_BUF) */
    /* |          */
    /* |         vwaitq */
    /* |          */
    /* |         vmulq.z   vf4,  vf23, Q */
    /* |         vmulq.xyz vf10, vf10, Q */
    /* |         vmulz.xy  vf9,  vf9,  vf4z */
    /* |          */
    /* |         vadd.xy   vf9, vf9, vf16 */
    /* |          */
    /* |         sqc2      vf10, -0x30(%0) */
    /* |         sqc2      vf11, -0x20(%0) */
    /* |         sqc2      vf9,  -0x10(%0) */
    /* |  */
    /* |         bnez      t0, l_002D3B2C */
    /* |         nop */
    /* |     " : : "r"(vu1ScessorBuf), "r"(&count) : "$t0", "memory" */
    /* |     ); */
        ee_gpr r8 = {{0}}, r9 = {{0}};
        __typeof__((&count) + 0) op0 = (&count);
        __typeof__((vu1ScessorBuf) + 0) op1 = (vu1ScessorBuf);
        r8.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0))));
        r9.d[0] = EE_SEXT32((uint32_t)(0) + (uint32_t)(80));
        vu_ctc2(3, (uint32_t)(r9.d[0]));
        L_DrawScissorPolygonOpaque2_l_002D3B2C:;
        VU0_CALLMS(VU0_LOAD_SCISSOR_WORKi);
        VQ = vu_div(VF(0)[3], VF(9)[2]);
        vu_mul_bc(VF(10), VF(0), VF(0)[0], 1);
        vu_mul(VF(9), VF(9), VF(17), 12);
        vu_add_bc(VF(10), VF(0), VF(0)[3], 2);
        r8.d[0] = EE_SEXT32((uint32_t)(r8.d[0]) + (uint32_t)(-1));
        EE_CVAR_SET(op1, EE_SEXT32((uint32_t)(EE_CVAR_GET(op1)) + (uint32_t)((int)sizeof(VU1_PRIM_BUF))));
        vu_mul_bc(VF(4), VF(23), VQ, 2);
        vu_mul_bc(VF(10), VF(10), VQ, 14);
        vu_mul_bc(VF(9), VF(9), VF(4)[2], 12);
        vu_add(VF(9), VF(9), VF(16), 12);
        vu_sqc2(10, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (-0x30)));
        vu_sqc2(11, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (-0x20)));
        vu_sqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (-0x10)));
        if ((int64_t)(r8.d[0]) != 0) goto L_DrawScissorPolygonOpaque2_l_002D3B2C;
    }

    ulType = (ulType & ~0x2000000000000) | 0x6800000000000;
    
    pPrim = vu1ScessorBuf;
    
    Ps2AddPrim3DEx(ulType, pPrim, count);
}

// 100% matching!
void InitNodeArraySet2()
{
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     { */
    /* |         vmove.xyzw vf15, vf0 */
    /* |         vmove.xyzw vf18, vf0 */
    /* |         vmove.xyzw vf19, vf0 */
    /* |     } */
        vu_move(VF(15), VF(0), 15);
        vu_move(VF(18), VF(0), 15);
        vu_move(VF(19), VF(0), 15);
    }
}

// 92.50% matching
unsigned int _Clip_ViewVolume2(NJS_POINT4* vec) 
{
    unsigned int ret;
    
    ret = 0;
    
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         lqc2    vf14, NJS_POINT4.x(%0) */
    /* |          */
    /* |         vcallms VU0_CLIP_VIEW_VOLUME */
    /* |     .set reorder */
    /* |     " : : "r"(vec) :  */
    /* |     ); */
        __typeof__((vec) + 0) op0 = (vec);
        vu_lqc2(14, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_POINT4, x))));
        VU0_CALLMS(VU0_CLIP_VIEW_VOLUME);
    }
    
    return ret;
}

// 100% matching! 
unsigned int _Get_ClipViewVolume2()
{
    unsigned int ret;

    ret = 0;

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |         cfc2.i %0, vi18 */
    /* |          */
    /* |         addi   %0, %0, 0  */
    /* |     " : "=r"(ret) : :  */
    /* |     ); */
        EE_CVAR_SET(ret, (uint64_t)vu_cfc2(18));
        EE_CVAR_SET(ret, EE_SEXT32((uint32_t)(EE_CVAR_GET(ret)) + (uint32_t)(0)));
    }

    return ret;
}

// 100% matching! 
int _Get_ClipVolumePlane() 
{
    int ret;

    ret = 0;

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         cfc2.i %0, vi2 */
    /* |          */
    /* |         addi   %0, %0, 0  */
    /* |     .set reorder */
    /* |     " : "=r"(ret) : :  */
    /* |     ); */
        EE_CVAR_SET(ret, (uint64_t)(uint16_t)VI(2));
        EE_CVAR_SET(ret, EE_SEXT32((uint32_t)(EE_CVAR_GET(ret)) + (uint32_t)(0)));
    }

    return ret;
}

// 91.43% matching
void _Check_ClipViewAll(NJS_POINT4* vec) 
{
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         lqc2    vf14, NJS_POINT4.x(%0) */
    /* |          */
    /* |         vcallms VU0_CLIP_VIEW_VOLUME_ALL */
    /* |     .set reorder */
    /* |     " : : "r"(vec) :  */
    /* |     ); */
        __typeof__((vec) + 0) op0 = (vec);
        vu_lqc2(14, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_POINT4, x))));
        VU0_CALLMS(VU0_CLIP_VIEW_VOLUME_ALL);
    }
}

// 96% matching
void _Set_NodeArray(VU1_STRIP_BUF* pS, VU1_PRIM_BUF* pP)
{
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         lqc2    vf4, -0x60(%0) */
    /* |         lqc2    vf5, -0x20(%0) */
    /* |         lqc2    vf6, 0x20(%0) */
    /* |         lqc2    vf7, -0x80(%0) */
    /* |         lqc2    vf8, -0x40(%0) */
    /* |         lqc2    vf9, 0(%0) */
    /* |         lqc2    vf10, -0x50(%1) */
    /* |         lqc2    vf11, -0x20(%1) */
    /* |         lqc2    vf12, 0x10(%1) */
    /* |          */
    /* |         vcallms VU0_SET_NODE_ARRAY */
    /* |     .set reorder */
    /* |     " : : "r"(pS), "r"(pP) :  */
    /* |     ); */
        __typeof__((pS) + 0) op0 = (pS);
        __typeof__((pP) + 0) op1 = (pP);
        vu_lqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (-0x60)));
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (-0x20)));
        vu_lqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20)));
        vu_lqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (-0x80)));
        vu_lqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (-0x40)));
        vu_lqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        vu_lqc2(10, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (-0x50)));
        vu_lqc2(11, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (-0x20)));
        vu_lqc2(12, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x10)));
        VU0_CALLMS(VU0_SET_NODE_ARRAY);
    }
}

// TODO: Verify that this function hasn't got more parts written in C, for example addiu and paddub tend to be compiler-emitted instructions
// 96.57% matching 
int _ClipInter(int mask1, int mask2, int xyzflg, float sin, int work0, int work1, int count)
{
    int ret;
    int pad[4]; // not from the debugging symbols

    ret = 0;

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         addiu      s0, sp, 0x50 */
    /* |         addiu      s1, sp, 0x54 */
    /* |         addiu      s2, sp, 0x58 */
    /* |         addiu      s3, sp, 0x5C */
    /* |          */
    /* |         sw         t1, 0x5C(sp) */
    /* |         sw         a0, 0x50(sp) */
    /* |         sw         a1, 0x54(sp) */
    /* |         sw         a2, 0x58(sp) */
    /* |          */
    /* |         paddub     %0, t0, zero */
    /* |          */
    /* |         add        t0, zero, a3  */
    /* |         add        t1, zero, %0  */
    /* |          */
    /* |         ctc2.ni    t0, vi3 */
    /* |         ctc2.ni    t1, vi4 */
    /* |          */
    /* |         viadd      vi5, vi0, vi4 */
    /* |      */
    /* |         vcallms    VU0_LOAD_SCISSOR_WORKi */
    /* |      */
    /* |         mfc1       t2, f12 */
    /* |      */
    /* |         lw         t3, 0(s3) */
    /* |          */
    /* |         neg        t5, zero  */
    /* |      */
    /* |         qmtc2.ni   t2, vf12 */
    /* |          */
    /* |         l_002D3CE4: */
    /* |         lw         t1, 0(s2) */
    /* |          */
    /* |         vmove.xyzw vf4, vf8 */
    /* |         vmove.xyzw vf5, vf9 */
    /* |         vmove.xyzw vf6, vf10 */
    /* |         vmove.xyzw vf7, vf11 */
    /* |          */
    /* |         vcallms    VU0_LOAD_SCISSOR_WORKi */
    /* |      */
    /* |         vclipw.xyz vf4, vf4w                    */
    /* |         vclipw.xyz vf8, vf8w                  */
    /* |         vnop */
    /* |         vnop */
    /* |         vnop */
    /* |         vnop */
    /* |         vnop */
    /* |      */
    /* |         lw         t0, 0(s0) */
    /* |         lw         t7, 0(s1) */
    /* |          */
    /* |         cfc2.ni    %0, vi18 */
    /* |          */
    /* |         and        t0, %0, t0 */
    /* |         nop */
    /* |      */
    /* |         beqz       t0, l_002D3DCC */
    /* |         nop */
    /* |          */
    /* |         and        %0, %0, t7 */
    /* |         nop */
    /* |      */
    /* |         bnez       %0, l_002D3E74 */
    /* |         nop */
    /* |      */
    /* |         vmulx.w    vf13, vf4, vf12x */
    /* |         vmulx.w    vf14, vf8, vf12x */
    /* |          */
    /* |         vsubw.xyzw vf13, vf4, vf13w */
    /* |         vsubw.xyzw vf14, vf8, vf14w */
    /* |          */
    /* |         l_002D3D54: */
    /* |         beqz       t1, l_002D3D70 */
    /* |         nop */
    /* |          */
    /* |         vmr32.xyzw vf13, vf13 */
    /* |         vmr32.xyzw vf14, vf14 */
    /* |          */
    /* |         addi       t1, t1, -1  */
    /* |          */
    /* |         b          l_002D3D54 */
    /* |         nop */
    /* |      */
    /* |         l_002D3D70: */
    /* |         vsub.xyz   vf14, vf14, vf13 */
    /* |          */
    /* |         vdiv       Q, vf13x, vf14x */
    /* |      */
    /* |         vwaitq */
    /* |      */
    /* |         vaddq.x    vf13, vf0, Q */
    /* |          */
    /* |         vabs.x     vf13, vf13 */
    /* |          */
    /* |         vsub.xyzw  vf14, vf8, vf4 */
    /* |          */
    /* |         vmulx.xyzw vf14, vf14, vf13x */
    /* |          */
    /* |         vadd.xyzw  vf4, vf4, vf14 */
    /* |         vsub.xyzw  vf14, vf9, vf5 */
    /* |          */
    /* |         vmulx.xyzw vf14, vf14, vf13x */
    /* |          */
    /* |         vadd.xyzw  vf5, vf5, vf14 */
    /* |          */
    /* |         vmaxw.z    vf5, vf5, vf0w */
    /* |          */
    /* |         vsub.xyzw  vf14, vf10, vf6 */
    /* |          */
    /* |         vmulx.xyzw vf14, vf14, vf13x */
    /* |          */
    /* |         vadd.xyzw  vf6, vf6, vf14 */
    /* |         vsub.xyzw  vf14, vf11, vf7 */
    /* |          */
    /* |         vmulx.xyzw vf14, vf14, vf13x */
    /* |          */
    /* |         vadd.xyzw  vf7, vf7, vf14 */
    /* |          */
    /* |         vcallms    VU0_STORE_SCISSOR_WORK */
    /* |      */
    /* |         addi       t5, t5, 1  */
    /* |         nop */
    /* |      */
    /* |         b          l_002D3E74 */
    /* |         nop */
    /* |      */
    /* |         l_002D3DCC: */
    /* |         and        %0, %0, t7 */
    /* |         nop */
    /* |      */
    /* |         beqz       %0, l_002D3E6C */
    /* |         nop */
    /* |      */
    /* |         vcallms    VU0_STORE_SCISSOR_WORK */
    /* |      */
    /* |         vmulx.w    vf13, vf4, vf12x */
    /* |         vmulx.w    vf14, vf8, vf12x */
    /* |          */
    /* |         vsubw.xyzw vf13, vf4, vf13w */
    /* |         vsubw.xyzw vf14, vf8, vf14w */
    /* |          */
    /* |         l_002D3DF0: */
    /* |         beqz       t1, l_002D3E10 */
    /* |         nop */
    /* |          */
    /* |         vmr32.xyzw vf13, vf13 */
    /* |         vmr32.xyzw vf14, vf14 */
    /* |          */
    /* |         addi       t1, t1, -1  */
    /* |         nop */
    /* |          */
    /* |         b          l_002D3DF0 */
    /* |         nop */
    /* |      */
    /* |         l_002D3E10: */
    /* |         vsub.xyz   vf14, vf14, vf13 */
    /* |          */
    /* |         vdiv       Q, vf13x, vf14x */
    /* |      */
    /* |         vwaitq */
    /* |      */
    /* |         vaddq.x    vf13, vf0, Q */
    /* |          */
    /* |         vabs.x     vf13, vf13 */
    /* |          */
    /* |         vsub.xyzw  vf14, vf8, vf4 */
    /* |          */
    /* |         vmulx.xyzw vf14, vf14, vf13x */
    /* |          */
    /* |         vadd.xyzw  vf4, vf4, vf14 */
    /* |         vsub.xyzw  vf14, vf9, vf5 */
    /* |          */
    /* |         vmulx.xyzw vf14, vf14, vf13x */
    /* |          */
    /* |         vadd.xyzw  vf5, vf5, vf14 */
    /* |          */
    /* |         vmaxw.z    vf5, vf5, vf0w */
    /* |          */
    /* |         vsub.xyzw  vf14, vf10, vf6 */
    /* |          */
    /* |         vmulx.xyzw vf14, vf14, vf13x */
    /* |          */
    /* |         vadd.xyzw  vf6, vf6, vf14 */
    /* |         vsub.xyzw  vf14, vf11, vf7 */
    /* |          */
    /* |         vmulx.xyzw vf14, vf14, vf13x */
    /* |          */
    /* |         vadd.xyzw  vf7, vf7, vf14 */
    /* |          */
    /* |         vcallms    VU0_STORE_SCISSOR_WORK */
    /* |      */
    /* |         addi       t5, t5, 2  */
    /* |         nop */
    /* |      */
    /* |         b          l_002D3E74 */
    /* |         nop */
    /* |      */
    /* |         l_002D3E6C: */
    /* |         vcallms    VU0_STORE_SCISSOR_WORK */
    /* |      */
    /* |         addi       t5, t5, 1  */
    /* |      */
    /* |         l_002D3E74: */
    /* |         addi       t3, t3, -1  */
    /* |         nop */
    /* |          */
    /* |         bnez       t3, l_002D3CE4 */
    /* |         nop */
    /* |      */
    /* |         vcallms    VU0_LOAD_SCISSOR_WORKb */
    /* |         vcallms    VU0_STORE_SCISSOR_WORK */
    /* |      */
    /* |         addu       %0, t5, zero */
    /* |         addi       %0, %0, 0  */
    /* |     .set reorder */
    /* |     " : "=r"(ret) : "r"(pad) :  */
    /* |     ); */
        ee_gpr r4 = {{0}}, r5 = {{0}}, r6 = {{0}}, r7 = {{0}}, r8 = {{0}}, r9 = {{0}}, r10 = {{0}}, r11 = {{0}}, r13 = {{0}}, r15 = {{0}}, r16 = {{0}}, r17 = {{0}}, r18 = {{0}}, r19 = {{0}}, r29 = {{0}};
        float f12 = 0;
        uint8_t ee_stack[256] __attribute__((aligned(16)));
        r29.d[0] = (uintptr_t)&ee_stack[sizeof(ee_stack)];
        r4.d[0] = (uint64_t)(uintptr_t)mask1;
        r5.d[0] = (uint64_t)(uintptr_t)mask2;
        r6.d[0] = (uint64_t)(uintptr_t)xyzflg;
        f12 = sin;
        r7.d[0] = (uint64_t)(uintptr_t)work0;
        r8.d[0] = (uint64_t)(uintptr_t)work1;
        r9.d[0] = (uint64_t)(uintptr_t)count;
        r16.d[0] = EE_SEXT32((uint32_t)(r29.d[0]) + (uint32_t)(0x50));
        r17.d[0] = EE_SEXT32((uint32_t)(r29.d[0]) + (uint32_t)(0x54));
        r18.d[0] = EE_SEXT32((uint32_t)(r29.d[0]) + (uint32_t)(0x58));
        r19.d[0] = EE_SEXT32((uint32_t)(r29.d[0]) + (uint32_t)(0x5C));
        *(uint32_t *)(((uintptr_t)(uint32_t)(r29.d[0]) + (0x5C))) = (uint32_t)(r9.d[0]);
        *(uint32_t *)(((uintptr_t)(uint32_t)(r29.d[0]) + (0x50))) = (uint32_t)(r4.d[0]);
        *(uint32_t *)(((uintptr_t)(uint32_t)(r29.d[0]) + (0x54))) = (uint32_t)(r5.d[0]);
        *(uint32_t *)(((uintptr_t)(uint32_t)(r29.d[0]) + (0x58))) = (uint32_t)(r6.d[0]);
        EE_CVAR_SET(ret, (ee_paddub(r8, (ee_gpr){{0}})).d[0]);
        r8.d[0] = EE_SEXT32((uint32_t)(0) + (uint32_t)(r7.d[0]));
        r9.d[0] = EE_SEXT32((uint32_t)(0) + (uint32_t)(EE_CVAR_GET(ret)));
        vu_ctc2(3, (uint32_t)(r8.d[0]));
        vu_ctc2(4, (uint32_t)(r9.d[0]));
        VI(5) = (int16_t)(VI(0) + (VI(4)));
        VU0_CALLMS(VU0_LOAD_SCISSOR_WORKi);
        r10.d[0] = EE_SEXT32(ee_fbits(f12));
        r11.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(r19.d[0]) + (0))));
        r13.d[0] = EE_SEXT32(0u - (uint32_t)(0));
        vu_qmtc2(12, r10);
        L__ClipInter_l_002D3CE4:;
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(r18.d[0]) + (0))));
        vu_move(VF(4), VF(8), 15);
        vu_move(VF(5), VF(9), 15);
        vu_move(VF(6), VF(10), 15);
        vu_move(VF(7), VF(11), 15);
        VU0_CALLMS(VU0_LOAD_SCISSOR_WORKi);
        vu_clip(VF(4), VF(4)[3]);
        vu_clip(VF(8), VF(8)[3]);
        r8.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(r16.d[0]) + (0))));
        r15.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(r17.d[0]) + (0))));
        EE_CVAR_SET(ret, (uint64_t)vu_cfc2(18));
        r8.d[0] = (EE_CVAR_GET(ret)) & r8.d[0];
        if ((int64_t)(r8.d[0]) == 0) goto L__ClipInter_l_002D3DCC;
        EE_CVAR_SET(ret, (EE_CVAR_GET(ret)) & r15.d[0]);
        if ((int64_t)(EE_CVAR_GET(ret)) != 0) goto L__ClipInter_l_002D3E74;
        vu_mul_bc(VF(13), VF(4), VF(12)[0], 1);
        vu_mul_bc(VF(14), VF(8), VF(12)[0], 1);
        vu_sub_bc(VF(13), VF(4), VF(13)[3], 15);
        vu_sub_bc(VF(14), VF(8), VF(14)[3], 15);
        L__ClipInter_l_002D3D54:;
        if ((int64_t)(r9.d[0]) == 0) goto L__ClipInter_l_002D3D70;
        vu_mr32(VF(13), VF(13), 15);
        vu_mr32(VF(14), VF(14), 15);
        r9.d[0] = EE_SEXT32((uint32_t)(r9.d[0]) + (uint32_t)(-1));
        goto L__ClipInter_l_002D3D54;
        L__ClipInter_l_002D3D70:;
        vu_sub(VF(14), VF(14), VF(13), 14);
        VQ = vu_div(VF(13)[0], VF(14)[0]);
        vu_add_bc(VF(13), VF(0), VQ, 8);
        vu_abs(VF(13), VF(13), 8);
        vu_sub(VF(14), VF(8), VF(4), 15);
        vu_mul_bc(VF(14), VF(14), VF(13)[0], 15);
        vu_add(VF(4), VF(4), VF(14), 15);
        vu_sub(VF(14), VF(9), VF(5), 15);
        vu_mul_bc(VF(14), VF(14), VF(13)[0], 15);
        vu_add(VF(5), VF(5), VF(14), 15);
        vu_max_bc(VF(5), VF(5), VF(0)[3], 2);
        vu_sub(VF(14), VF(10), VF(6), 15);
        vu_mul_bc(VF(14), VF(14), VF(13)[0], 15);
        vu_add(VF(6), VF(6), VF(14), 15);
        vu_sub(VF(14), VF(11), VF(7), 15);
        vu_mul_bc(VF(14), VF(14), VF(13)[0], 15);
        vu_add(VF(7), VF(7), VF(14), 15);
        VU0_CALLMS(VU0_STORE_SCISSOR_WORK);
        r13.d[0] = EE_SEXT32((uint32_t)(r13.d[0]) + (uint32_t)(1));
        goto L__ClipInter_l_002D3E74;
        L__ClipInter_l_002D3DCC:;
        EE_CVAR_SET(ret, (EE_CVAR_GET(ret)) & r15.d[0]);
        if ((int64_t)(EE_CVAR_GET(ret)) == 0) goto L__ClipInter_l_002D3E6C;
        VU0_CALLMS(VU0_STORE_SCISSOR_WORK);
        vu_mul_bc(VF(13), VF(4), VF(12)[0], 1);
        vu_mul_bc(VF(14), VF(8), VF(12)[0], 1);
        vu_sub_bc(VF(13), VF(4), VF(13)[3], 15);
        vu_sub_bc(VF(14), VF(8), VF(14)[3], 15);
        L__ClipInter_l_002D3DF0:;
        if ((int64_t)(r9.d[0]) == 0) goto L__ClipInter_l_002D3E10;
        vu_mr32(VF(13), VF(13), 15);
        vu_mr32(VF(14), VF(14), 15);
        r9.d[0] = EE_SEXT32((uint32_t)(r9.d[0]) + (uint32_t)(-1));
        goto L__ClipInter_l_002D3DF0;
        L__ClipInter_l_002D3E10:;
        vu_sub(VF(14), VF(14), VF(13), 14);
        VQ = vu_div(VF(13)[0], VF(14)[0]);
        vu_add_bc(VF(13), VF(0), VQ, 8);
        vu_abs(VF(13), VF(13), 8);
        vu_sub(VF(14), VF(8), VF(4), 15);
        vu_mul_bc(VF(14), VF(14), VF(13)[0], 15);
        vu_add(VF(4), VF(4), VF(14), 15);
        vu_sub(VF(14), VF(9), VF(5), 15);
        vu_mul_bc(VF(14), VF(14), VF(13)[0], 15);
        vu_add(VF(5), VF(5), VF(14), 15);
        vu_max_bc(VF(5), VF(5), VF(0)[3], 2);
        vu_sub(VF(14), VF(10), VF(6), 15);
        vu_mul_bc(VF(14), VF(14), VF(13)[0], 15);
        vu_add(VF(6), VF(6), VF(14), 15);
        vu_sub(VF(14), VF(11), VF(7), 15);
        vu_mul_bc(VF(14), VF(14), VF(13)[0], 15);
        vu_add(VF(7), VF(7), VF(14), 15);
        VU0_CALLMS(VU0_STORE_SCISSOR_WORK);
        r13.d[0] = EE_SEXT32((uint32_t)(r13.d[0]) + (uint32_t)(2));
        goto L__ClipInter_l_002D3E74;
        L__ClipInter_l_002D3E6C:;
        VU0_CALLMS(VU0_STORE_SCISSOR_WORK);
        r13.d[0] = EE_SEXT32((uint32_t)(r13.d[0]) + (uint32_t)(1));
        L__ClipInter_l_002D3E74:;
        r11.d[0] = EE_SEXT32((uint32_t)(r11.d[0]) + (uint32_t)(-1));
        if ((int64_t)(r11.d[0]) != 0) goto L__ClipInter_l_002D3CE4;
        VU0_CALLMS(VU0_LOAD_SCISSOR_WORKb);
        VU0_CALLMS(VU0_STORE_SCISSOR_WORK);
        EE_CVAR_SET(ret, EE_SEXT32((uint32_t)(r13.d[0]) + (uint32_t)(0)));
        EE_CVAR_SET(ret, EE_SEXT32((uint32_t)(EE_CVAR_GET(ret)) + (uint32_t)(0)));
    }

    return ret;
}

// 100% matching!
int _Check_ScissorPlane()
{
    int count;

    count = _ClipInter(2048, 32, 2, -1.0f, 80, 128, 3);
    
    if (count == 0) 
    {
        return count;
    }

    count = _ClipInter(1024, 16, 2, 1.0f, 128, 80, count);

    if (count == 0) 
    {
        return count;
    }

    count = _ClipInter(128, 2, 0, -1.0f, 80, 128, count);
    
    if (count == 0) 
    {
        return count;
    }

    count = _ClipInter(64, 1, 0, 1.0f, 128, 80, count);
    
    if (count == 0) 
    {
        return count;
    }

    count = _ClipInter(512, 8, 1, -1.0f, 80, 128, count);
    
    if (count == 0) 
    {
        return count;
    }
    
    count = _ClipInter(256, 4, 1, 1.0f, 128, 80, count);

    return count;
}
