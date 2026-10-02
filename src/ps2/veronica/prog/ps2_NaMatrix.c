#include "../../../ps2/veronica/prog/ps2_NaMatrix.h"
#include "../../../ps2/veronica/prog/ps2_NaFog.h"
#include "../../../ps2/veronica/prog/ps2_NaView.h"
#include "../../../ps2/veronica/prog/main.h"
#include "../../../ps2/veronica/prog/ps2_NaMath.h"

int lNaMatIsUnitMatrix;
int lNaMatMatrixStuckMax;
int lNaMatMatrixStuckCnt;
NJS_MATRIX* pNaMatMatrixStuckPtr;
NJS_MATRIX* pNaMatMatrixStuckTop;
/* unused below */
/*NJS_MATRIX TempMatrix0;
NJS_MATRIX TempMatrix1;*/

extern void VU0_INIT_CALC_PROCESS() __attribute__((section(".vutext")));

// 98.85% matching
void	njInitMatrix(NJS_MATRIX *m, Sint32 n, Int flag)
{
    register float pi;

    pNaMatMatrixStuckTop = m;
    pNaMatMatrixStuckPtr = m; 
    
    lNaMatMatrixStuckCnt = 0; 
    lNaMatMatrixStuckMax = n; 
    
    lNaMatIsUnitMatrix = flag; 
    
    pi = 3.141592f;
    
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     { */
    /* |     .set noreorder */
    /* |      */
    /* |         vaddw.xyz  vf1, vf0, vf0w    */
    /* |          */
    /* |         vmul.w     vf1, vf0, vf0                      */
    /* |  */
    /* |         sub        t5, t5, t5      */
    /* |          */
    /* |         addi       t0, t5, 0x80                        */
    /* |         addi       t1, t5, 0x100           */
    /* |          */
    /* |         mult       zero, t1, t1         */
    /* |          */
    /* |         mflo       t2                */
    /* |          */
    /* |         mfc1       t3, pi            */
    /* |          */
    /* |         qmtc2      t0, vf3                           */
    /* |         qmtc2      t1, vf9                                */
    /* |         qmtc2      t2, vf10                              */
    /* |         qmtc2      t3, vf11                               */
    /* |  */
    /* |         vitof0.xyzw vf3, vf3                         */
    /* |         vitof0.xyzw vf9, vf9                  */
    /* |         vitof0.xyzw vf10, vf10          */
    /* |          */
    /* |         vaddx.y    vf3, vf0, vf9x                      */
    /* |         vaddx.z    vf3, vf0, vf10x        */
    /* |          */
    /* |         vmulx.w    vf3, vf0, vf11x         */
    /* |          */
    /* |         vaddw.xyzw vf2, vf0, vf0w         */
    /* |          */
    /* |         vdiv       Q, vf0w, vf2w         */
    /* |          */
    /* |         vaddw.xyzw vf2, vf2, vf0w                     */
    /* |         vaddw.xyzw vf2, vf2, vf0w                      */
    /* |         vaddw.yzw  vf2, vf2, vf0w                      */
    /* |         vaddw.zw   vf2, vf2, vf0w    */
    /* |          */
    /* |         vwaitq                      */
    /* |          */
    /* |         vmulq.w    vf2, vf0, Q                         */
    /* |          */
    /* |         addi       t0, zero, 0xFFF       */
    /* |          */
    /* |         qmtc2      t0, vf4                                */
    /* |          */
    /* |         vitof0.xyz vf4, vf4       */
    /* |          */
    /* |         vaddx.z    vf16, vf0, vf4x           */
    /* |          */
    /* |         vcallms    VU0_INIT_CALC_PROCESS                           */
    /* |          */
    /* |         nop        */
    /* |      */
    /* |     .set reorder */
    /* |     } */
        ee_gpr r8 = {{0}}, r9 = {{0}}, r10 = {{0}}, r11 = {{0}}, r13 = {{0}};
        uint64_t ee_lo = 0, ee_hi = 0;
        /* note: reads before write: r13 */
        vu_add_bc(VF(1), VF(0), VF(0)[3], 14);
        vu_mul(VF(1), VF(0), VF(0), 1);
        r13.d[0] = EE_SEXT32((uint32_t)(r13.d[0]) - (uint32_t)(r13.d[0]));
        r8.d[0] = EE_SEXT32((uint32_t)(r13.d[0]) + (uint32_t)(0x80));
        r9.d[0] = EE_SEXT32((uint32_t)(r13.d[0]) + (uint32_t)(0x100));
        { int64_t p_ = (int64_t)(int32_t)(r9.d[0]) * (int64_t)(int32_t)(r9.d[0]); ee_lo = EE_SEXT32((uint32_t)p_); ee_hi = EE_SEXT32((uint32_t)((uint64_t)p_ >> 32)); } 
        r10.d[0] = ee_lo;
        r11.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(pi)));
        vu_qmtc2(3, r8);
        vu_qmtc2(9, r9);
        vu_qmtc2(10, r10);
        vu_qmtc2(11, r11);
        vu_itof(VF(3), VF(3), 15, (1.0f / 1.0f));
        vu_itof(VF(9), VF(9), 15, (1.0f / 1.0f));
        vu_itof(VF(10), VF(10), 15, (1.0f / 1.0f));
        vu_add_bc(VF(3), VF(0), VF(9)[0], 4);
        vu_add_bc(VF(3), VF(0), VF(10)[0], 2);
        vu_mul_bc(VF(3), VF(0), VF(11)[0], 1);
        vu_add_bc(VF(2), VF(0), VF(0)[3], 15);
        VQ = vu_div(VF(0)[3], VF(2)[3]);
        vu_add_bc(VF(2), VF(2), VF(0)[3], 15);
        vu_add_bc(VF(2), VF(2), VF(0)[3], 15);
        vu_add_bc(VF(2), VF(2), VF(0)[3], 7);
        vu_add_bc(VF(2), VF(2), VF(0)[3], 3);
        vu_mul_bc(VF(2), VF(0), VQ, 1);
        r8.d[0] = EE_SEXT32((uint32_t)(0) + (uint32_t)(0xFFF));
        vu_qmtc2(4, r8);
        vu_itof(VF(4), VF(4), 14, (1.0f / 1.0f));
        vu_add_bc(VF(16), VF(0), VF(4)[0], 2);
        VU0_CALLMS(VU0_INIT_CALC_PROCESS);
    }
}

// 100% matching
void	njCalcPoints(NJS_MATRIX *m, NJS_POINT3 *ps, NJS_POINT3 *pd, Int num)
{
	if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         lw          a4,   0(%3) */
    /* |          */
    /* |         ldl         a6, 0x7(%1) */
    /* |         ldr         a6,   0(%1) */
    /* |  */
    /* |         lw          a7, NJS_VECTOR.z(%1)  */
    /* |  */
    /* |         pcpyld      a6, a7, a6 */
    /* |  */
    /* |         qmtc2.ni    a6, vf4 */
    /* |  */
    /* |         lqc2        vf5,    0(%0) */
    /* |         lqc2        vf6, 0x10(%0) */
    /* |         lqc2        vf7, 0x20(%0) */
    /* |         lqc2        vf8, 0x30(%0) */
    /* |  */
    /* |         l_002D67C4: */
    /* |         addi        %1, %1, 12 */
    /* |  */
    /* |         vmulax.xyz  ACC,  vf5, vf4 */
    /* |          */
    /* |         vmadday.xyz ACC,  vf6, vf4 */
    /* |         vmaddaz.xyz ACC,  vf7, vf4 */
    /* |         vmaddw.xyz  vf18, vf8, vf0 */
    /* |  */
    /* |         addi        a4, a4, -1 */
    /* |  */
    /* |         ldl         a6, 0x7(%1) */
    /* |         ldr         a6,   0(%1) */
    /* |  */
    /* |         lw          a7, NJS_VECTOR.z(%1)  */
    /* |  */
    /* |         pcpyld      a6, a7, a6 */
    /* |  */
    /* |         qmtc2.ni    a6, vf4 */
    /* |         qmfc2.ni    t5, vf18 */
    /* |  */
    /* |         pcpyud      t6, t5, t5 */
    /* |  */
    /* |         sdl         t5, 0x7(%2) */
    /* |         sdr         t5,   0(%2) */
    /* |  */
    /* |         sw          t6, NJS_VECTOR.z(%2)  */
    /* |  */
    /* |         addi        %2, %2, 12 */
    /* |  */
    /* |         bgtz        a4, l_002D67C4 */
    /* |         vnop */
    /* |     .set reorder */
    /* |     " : : "r"(m), "r"(ps), "r"(pd), "r"(&num) :  */
    /* |     ); */
        ee_gpr r8 = {{0}}, r10 = {{0}}, r11 = {{0}}, r13 = {{0}}, r14 = {{0}};
        __typeof__((&num) + 0) op0 = (&num);
        __typeof__((ps) + 0) op1 = (ps);
        __typeof__((m) + 0) op2 = (m);
        __typeof__((pd) + 0) op3 = (pd);
        r8.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0))));
        r10.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x7)) - 7);
        r10.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
        r11.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + ((int)offsetof(NJS_VECTOR, z)))));
        r10 = ee_pcpyld(r11, r10);
        vu_qmtc2(4, r10);
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0)));
        vu_lqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x10)));
        vu_lqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x20)));
        vu_lqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x30)));
        L_njCalcPoints_l_002D67C4:;
        EE_CVAR_SET(op1, EE_SEXT32((uint32_t)(EE_CVAR_GET(op1)) + (uint32_t)(12)));
        vu_mul_bc(VACC, VF(5), VF(4)[0], 14);
        vu_madd_bc(VACC, VF(6), VF(4)[1], 14);
        vu_madd_bc(VACC, VF(7), VF(4)[2], 14);
        vu_madd_bc(VF(18), VF(8), VF(0)[3], 14);
        r8.d[0] = EE_SEXT32((uint32_t)(r8.d[0]) + (uint32_t)(-1));
        r10.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x7)) - 7);
        r10.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
        r11.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + ((int)offsetof(NJS_VECTOR, z)))));
        r10 = ee_pcpyld(r11, r10);
        vu_qmtc2(4, r10);
        r13 = vu_qmfc2(18);
        r14 = ee_pcpyud(r13, r13);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x7)) - 7, r13.d[0]);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0)), r13.d[0]);
        *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + ((int)offsetof(NJS_VECTOR, z)))) = (uint32_t)(r14.d[0]);
        EE_CVAR_SET(op3, EE_SEXT32((uint32_t)(EE_CVAR_GET(op3)) + (uint32_t)(12)));
        if ((int64_t)(r8.d[0]) > 0) goto L_njCalcPoints_l_002D67C4;
    }
}

// 100% matching! 
void    njGetTranslation(NJS_MATRIX *m, NJS_POINT3 *p)
{
    if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         lw t0, 0x30(%0) */
    /* |         lw t1, 0x34(%0) */
    /* |         lw t2, 0x38(%0) */
    /* |          */
    /* |         sw t0, NJS_POINT3.x(%1) */
    /* |         sw t1, NJS_POINT3.y(%1) */
    /* |         sw t2, NJS_POINT3.z(%1) */
    /* |     .set reorder */
    /* |     " : : "r"(m), "r"(p) :  */
    /* |     ); */
        ee_gpr r8 = {{0}}, r9 = {{0}}, r10 = {{0}};
        __typeof__((m) + 0) op0 = (m);
        __typeof__((p) + 0) op1 = (p);
        r8.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x30))));
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x34))));
        r10.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x38))));
        *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + ((int)offsetof(NJS_POINT3, x)))) = (uint32_t)(r8.d[0]);
        *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + ((int)offsetof(NJS_POINT3, y)))) = (uint32_t)(r9.d[0]);
        *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + ((int)offsetof(NJS_POINT3, z)))) = (uint32_t)(r10.d[0]);
    }
}

// 100% matching!
void    njUnitTransPortion(NJS_MATRIX *m)
{
	if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         sqc2 vf0, 0x30(%0) */
    /* |     .set reorder */
    /* |     " : : "r"(m) :  */
    /* |     ); */
        __typeof__((m) + 0) op0 = (m);
        vu_sqc2(0, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x30)));
    }
}

// 100% matching! 
void    njUnitRotPortion(NJS_MATRIX *m)
{
    if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         vmulw.xyzw vf4, vf0, vf0w */
    /* |          */
    /* |         vmr32.xyzw vf5, vf4 */
    /* |         vmr32.xyzw vf6, vf5 */
    /* |         vmr32.xyzw vf7, vf6 */
    /* |          */
    /* |         sqc2       vf5, 0x20(%0) */
    /* |         sqc2       vf6, 0x10(%0) */
    /* |         sqc2       vf7,  0x0(%0) */
    /* |     .set reorder */
    /* |     " : : "r"(m) :  */
    /* |     ); */
        __typeof__((m) + 0) op0 = (m);
        vu_mul_bc(VF(4), VF(0), VF(0)[3], 15);
        vu_mr32(VF(5), VF(4), 15);
        vu_mr32(VF(6), VF(5), 15);
        vu_mr32(VF(7), VF(6), 15);
        vu_sqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20)));
        vu_sqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x10)));
        vu_sqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x0)));
    }
}

// 100% matching!
void    njClearMatrix()
{
    lNaMatMatrixStuckCnt = 0;

    pNaMatMatrixStuckPtr = pNaMatMatrixStuckTop;
    
    njSetMatrix(NULL, &NaViwViewMatrix);
}

// 100% matching!
Bool	njPushMatrix(NJS_MATRIX *m)
{
	NJS_MATRIX* fpSrc;
    NJS_MATRIX* fpDst;

    if (lNaMatMatrixStuckMax <= lNaMatMatrixStuckCnt) 
    {
        return FALSE;
    }
    
    lNaMatMatrixStuckCnt++;

    if (m == NULL) 
    {
        fpSrc = pNaMatMatrixStuckPtr;
    }
    else 
    {
        fpSrc = m;
    }

    pNaMatMatrixStuckPtr++;
    
    fpDst = pNaMatMatrixStuckPtr;
    
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         lqc2 vf4, 0(%0)  */
    /* |         lqc2 vf5, 0x10(%0)  */
    /* |         lqc2 vf6, 0x20(%0)  */
    /* |         lqc2 vf7, 0x30(%0)  */
    /* |      */
    /* |         sqc2 vf4, 0(%1)  */
    /* |         sqc2 vf5, 0x10(%1)  */
    /* |         sqc2 vf6, 0x20(%1)  */
    /* |         sqc2 vf7, 0x30(%1)  */
    /* |     .set reorder */
    /* |     " : : "r"(fpSrc), "r"(fpDst) :  */
    /* |     ); */
        __typeof__((fpSrc) + 0) op0 = (fpSrc);
        __typeof__((fpDst) + 0) op1 = (fpDst);
        vu_lqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x10)));
        vu_lqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20)));
        vu_lqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x30)));
        vu_sqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
        vu_sqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x10)));
        vu_sqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x20)));
        vu_sqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x30)));
    }

    return TRUE;
}

// 100% matching!
Bool	njPopMatrix(Uint32 n)
{
    int lNumber;

    lNumber = lNaMatMatrixStuckCnt - n;
    
    if (lNumber < 0) 
    {
        return FALSE;
    }
    
    lNaMatMatrixStuckCnt = lNumber;
    
    pNaMatMatrixStuckPtr -= n;
    
    return TRUE;
}

// 100% matching!
void	njUnitMatrix(register NJS_MATRIX *m)
{
    if (m == NULL) 
    {
        m = pNaMatMatrixStuckPtr;
    }
    
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     { */
    /* |     .set noreorder */
    /* |  */
    /* |         vmulw.xyzw $vf4xyzw, $vf0xyzw, $vf0w */
    /* |  */
    /* |         vmr32.xyzw $vf5xyzw, $vf4xyzw */
    /* |         vmr32.xyzw $vf6xyzw, $vf5xyzw */
    /* |         vmr32.xyzw $vf7xyzw, $vf6xyzw */
    /* |      */
    /* |         sqc2       $vf4, 0x30(m) */
    /* |         sqc2       $vf5, 0x20(m) */
    /* |         sqc2       $vf6, 0x10(m)  */
    /* |         sqc2       $vf7,  0x0(m) */
    /* |          */
    /* |     .set reorder */
    /* |     } */
        vu_mul_bc(VF(4), VF(0), VF(0)[3], 15);
        vu_mr32(VF(5), VF(4), 15);
        vu_mr32(VF(6), VF(5), 15);
        vu_mr32(VF(7), VF(6), 15);
        vu_sqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(m)) + (0x30)));
        vu_sqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(m)) + (0x20)));
        vu_sqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(m)) + (0x10)));
        vu_sqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(m)) + (0x0)));
    }
}

// 100% matching!
void	njSetMatrix(NJS_MATRIX *md, NJS_MATRIX *ms)
{
    register NJS_MATRIX* fpSrc; 
    register NJS_MATRIX* fpDst; 

    fpSrc = ms;
    fpDst = md ? md : pNaMatMatrixStuckPtr;

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     { */
    /* |     .set noreorder */
    /* |          */
    /* |         lqc2       $vf4, 0x0(fpSrc)  */
    /* |         lqc2       $vf5, 0x10(fpSrc)  */
    /* |         lqc2       $vf6, 0x20(fpSrc)  */
    /* |         lqc2       $vf7, 0x30(fpSrc)  */
    /* |      */
    /* |         sqc2       $vf4, 0x0(fpDst)  */
    /* |         sqc2       $vf5, 0x10(fpDst)  */
    /* |         sqc2       $vf6, 0x20(fpDst)  */
    /* |         sqc2       $vf7, 0x30(fpDst)  */
    /* |          */
    /* |     .set reorder */
    /* |     } */
        vu_lqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpSrc)) + (0x0)));
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpSrc)) + (0x10)));
        vu_lqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpSrc)) + (0x20)));
        vu_lqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpSrc)) + (0x30)));
        vu_sqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpDst)) + (0x0)));
        vu_sqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpDst)) + (0x10)));
        vu_sqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpDst)) + (0x20)));
        vu_sqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpDst)) + (0x30)));
    }
}

// 100% matching!
void njSetMatrixCN(NJS_MATRIX* pMat)
{
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         lqc2 vf28,  0x0(%0)  */
    /* |         lqc2 vf29, 0x10(%0)  */
    /* |         lqc2 vf30, 0x20(%0)  */
    /* |         lqc2 vf31, 0x30(%0)  */
    /* |     .set reorder  */
    /* |     " : : "r"(pMat) :  */
    /* |     ); */
        __typeof__((pMat) + 0) op0 = (pMat);
        vu_lqc2(28, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x0)));
        vu_lqc2(29, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x10)));
        vu_lqc2(30, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20)));
        vu_lqc2(31, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x30)));
    }
}

// 100% matching!
void	njGetMatrix(NJS_MATRIX *m)
{
    register NJS_MATRIX* fpSrc; 
    register NJS_MATRIX* fpDst; 

    fpSrc = pNaMatMatrixStuckPtr;
    fpDst = m; 
    
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     { */
    /* |     .set noreorder */
    /* |          */
    /* |         lqc2       $vf4, 0x0(fpSrc) */
    /* |         lqc2       $vf5, 0x10(fpSrc) */
    /* |         lqc2       $vf6, 0x20(fpSrc) */
    /* |         lqc2       $vf7, 0x30(fpSrc) */
    /* |      */
    /* |         sqc2       $vf4, 0x0(fpDst) */
    /* |         sqc2       $vf5, 0x10(fpDst) */
    /* |         sqc2       $vf6, 0x20(fpDst) */
    /* |         sqc2       $vf7, 0x30(fpDst) */
    /* |          */
    /* |     .set reorder */
    /* |     } */
        vu_lqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpSrc)) + (0x0)));
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpSrc)) + (0x10)));
        vu_lqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpSrc)) + (0x20)));
        vu_lqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpSrc)) + (0x30)));
        vu_sqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpDst)) + (0x0)));
        vu_sqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpDst)) + (0x10)));
        vu_sqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpDst)) + (0x20)));
        vu_sqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(fpDst)) + (0x30)));
    }
}

// 100% matching!
void	njMultiMatrix(NJS_MATRIX *md, NJS_MATRIX *ms)
{
    if (md == NULL)
    {
        md = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         lqc2        vf4,  0(%0) */
    /* |         lqc2        vf5,  0x10(%0) */
    /* |         lqc2        vf6,  0x20(%0) */
    /* |         lqc2        vf7,  0x30(%0) */
    /* |         lqc2        vf8,  0(%1) */
    /* |         lqc2        vf9,  0x10(%1) */
    /* |         lqc2        vf10, 0x20(%1) */
    /* |         lqc2        vf11, 0x30(%1) */
    /* |  */
    /* |         vmulax.xyz  ACC,  vf4, vf8 */
    /* |          */
    /* |         vmadday.xyz ACC,  vf5, vf8 */
    /* |         vmaddz.xyz  vf8,  vf6, vf8 */
    /* |          */
    /* |         vmulax.xyz  ACC,  vf4, vf9 */
    /* |  */
    /* |         vmadday.xyz ACC,  vf5, vf9 */
    /* |         vmaddz.xyz  vf9,  vf6, vf9 */
    /* |          */
    /* |         vmulax.xyz  ACC,  vf4, vf10 */
    /* |  */
    /* |         vmadday.xyz ACC,  vf5, vf10 */
    /* |         vmaddz.xyz  vf10, vf6, vf10 */
    /* |          */
    /* |         vmulax.xyz  ACC,  vf4, vf11 */
    /* |  */
    /* |         vmadday.xyz ACC,  vf5, vf11 */
    /* |         vmaddaz.xyz ACC,  vf6, vf11 */
    /* |         vmaddw.xyz  vf11, vf7, vf0 */
    /* |  */
    /* |         sqc2        vf8,  0x0(%0) */
    /* |         sqc2        vf9,  0x10(%0) */
    /* |         sqc2        vf10, 0x20(%0) */
    /* |         sqc2        vf11, 0x30(%0) */
    /* |     .set reorder */
    /* |     " : : "r"(md), "r"(ms) :  */
    /* |     ); */
        __typeof__((md) + 0) op0 = (md);
        __typeof__((ms) + 0) op1 = (ms);
        vu_lqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x10)));
        vu_lqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20)));
        vu_lqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x30)));
        vu_lqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
        vu_lqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x10)));
        vu_lqc2(10, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x20)));
        vu_lqc2(11, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x30)));
        vu_mul_bc(VACC, VF(4), VF(8)[0], 14);
        vu_madd_bc(VACC, VF(5), VF(8)[1], 14);
        vu_madd_bc(VF(8), VF(6), VF(8)[2], 14);
        vu_mul_bc(VACC, VF(4), VF(9)[0], 14);
        vu_madd_bc(VACC, VF(5), VF(9)[1], 14);
        vu_madd_bc(VF(9), VF(6), VF(9)[2], 14);
        vu_mul_bc(VACC, VF(4), VF(10)[0], 14);
        vu_madd_bc(VACC, VF(5), VF(10)[1], 14);
        vu_madd_bc(VF(10), VF(6), VF(10)[2], 14);
        vu_mul_bc(VACC, VF(4), VF(11)[0], 14);
        vu_madd_bc(VACC, VF(5), VF(11)[1], 14);
        vu_madd_bc(VACC, VF(6), VF(11)[2], 14);
        vu_madd_bc(VF(11), VF(7), VF(0)[3], 14);
        vu_sqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x0)));
        vu_sqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x10)));
        vu_sqc2(10, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20)));
        vu_sqc2(11, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x30)));
    }
}

// 100% matching!
void	njTranslate(NJS_MATRIX *m, Float x, Float y, Float z)
{
    if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         mfc1        t0, %1  */
    /* |         mfc1        t1, %2 */
    /* |         mfc1        t2, %3 */
    /* |          */
    /* |         qmtc2       t0, vf4 */
    /* |         qmtc2       t1, vf5 */
    /* |         qmtc2       t2, vf6 */
    /* |      */
    /* |         lqc2        vf7,  0(%0) */
    /* |         lqc2        vf8,  0x10(%0) */
    /* |         lqc2        vf9,  0x20(%0) */
    /* |         lqc2        vf10, 0x30(%0) */
    /* |  */
    /* |         vmulax.xyz  ACC,  vf7,  vf4 */
    /* |          */
    /* |         vmaddax.xyz ACC,  vf8,  vf5 */
    /* |         vmaddax.xyz ACC,  vf9,  vf6 */
    /* |         vmaddw.xyz  vf11, vf10, vf0 */
    /* |      */
    /* |         sqc2        vf11, 0x30(%0) */
    /* |     .set reorder */
    /* |     " : : "r"(m), "f"(x), "f"(y), "f"(z) :  */
    /* |     ); */
        ee_gpr r8 = {{0}}, r9 = {{0}}, r10 = {{0}};
        __typeof__((x) + 0) op0 = (x);
        __typeof__((y) + 0) op1 = (y);
        __typeof__((z) + 0) op2 = (z);
        __typeof__((m) + 0) op3 = (m);
        r8.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op0)));
        r9.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op1)));
        r10.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op2)));
        vu_qmtc2(4, r8);
        vu_qmtc2(5, r9);
        vu_qmtc2(6, r10);
        vu_lqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0)));
        vu_lqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x10)));
        vu_lqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x20)));
        vu_lqc2(10, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x30)));
        vu_mul_bc(VACC, VF(7), VF(4)[0], 14);
        vu_madd_bc(VACC, VF(8), VF(5)[0], 14);
        vu_madd_bc(VACC, VF(9), VF(6)[0], 14);
        vu_madd_bc(VF(11), VF(10), VF(0)[3], 14);
        vu_sqc2(11, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x30)));
    }
}

// 100% matching!
void	njTranslateV(NJS_MATRIX *m, NJS_VECTOR *v)
{
	if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         ldl         a4, 0x7(%1) */
    /* |         ldr         a4,   0(%1) */
    /* |  */
    /* |         lw          a5, NJS_VECTOR.z(%1)  */
    /* |  */
    /* |         pcpyld      a4, a5, a4 */
    /* |  */
    /* |         qmtc2.ni    a4, vf4 */
    /* |  */
    /* |         lqc2        vf5, 0(%0) */
    /* |         lqc2        vf6, 0x10(%0) */
    /* |         lqc2        vf7, 0x20(%0) */
    /* |         lqc2        vf8, 0x30(%0) */
    /* |  */
    /* |         vmulax.xyz  ACC,  vf5, vf4x */
    /* |          */
    /* |         vmadday.xyz ACC,  vf6, vf4y */
    /* |         vmaddaz.xyz ACC,  vf7, vf4z */
    /* |         vmaddw.xyz  vf9,  vf8, vf0w */
    /* |      */
    /* |         sqc2        vf9,  0x30(%0) */
    /* |     .set reorder */
    /* |     " : : "r"(m), "r"(v) :  */
    /* |     ); */
        ee_gpr r8 = {{0}}, r9 = {{0}};
        __typeof__((v) + 0) op0 = (v);
        __typeof__((m) + 0) op1 = (m);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x7)) - 7);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, z)))));
        r8 = ee_pcpyld(r9, r8);
        vu_qmtc2(4, r8);
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
        vu_lqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x10)));
        vu_lqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x20)));
        vu_lqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x30)));
        vu_mul_bc(VACC, VF(5), VF(4)[0], 14);
        vu_madd_bc(VACC, VF(6), VF(4)[1], 14);
        vu_madd_bc(VACC, VF(7), VF(4)[2], 14);
        vu_madd_bc(VF(9), VF(8), VF(0)[3], 14);
        vu_sqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x30)));
    }
}

// 100% matching!
void	njRotateX(NJS_MATRIX *m, Angle ang)
{
    float fSin;
    float fCos;

    ang &= 0xFFFF;
    
    njSinCos(ang, &fSin, &fCos);
    
    if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         mfc1      t6, %1  */
    /* |         mfc1      t7, %2 */
    /* |          */
    /* |         qmtc2     t6, vf11 */
    /* |         qmtc2     t7, vf12 */
    /* |      */
    /* |         lqc2      vf4, 0x10(%0) */
    /* |         lqc2      vf5, 0x20(%0) */
    /* |  */
    /* |         vsub.x    vf10, vf0, vf11 */
    /* |          */
    /* |         vmulx.xyz vf6, vf4, vf12 */
    /* |         vmulx.xyz vf7, vf5, vf11 */
    /* |         vmulx.xyz vf4, vf4, vf10 */
    /* |         vmulx.xyz vf5, vf5, vf12 */
    /* |  */
    /* |         vadd.xyz  vf8, vf6, vf7 */
    /* |         vadd.xyz  vf9, vf4, vf5 */
    /* |  */
    /* |         sqc2      vf8, 0x10(%0) */
    /* |         sqc2      vf9, 0x20(%0) */
    /* |     .set reorder */
    /* |     " : : "r"(m), "f"(fSin), "f"(fCos) :  */
    /* |     ); */
        ee_gpr r14 = {{0}}, r15 = {{0}};
        __typeof__((fSin) + 0) op0 = (fSin);
        __typeof__((fCos) + 0) op1 = (fCos);
        __typeof__((m) + 0) op2 = (m);
        r14.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op0)));
        r15.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op1)));
        vu_qmtc2(11, r14);
        vu_qmtc2(12, r15);
        vu_lqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x10)));
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x20)));
        vu_sub(VF(10), VF(0), VF(11), 8);
        vu_mul_bc(VF(6), VF(4), VF(12)[0], 14);
        vu_mul_bc(VF(7), VF(5), VF(11)[0], 14);
        vu_mul_bc(VF(4), VF(4), VF(10)[0], 14);
        vu_mul_bc(VF(5), VF(5), VF(12)[0], 14);
        vu_add(VF(8), VF(6), VF(7), 14);
        vu_add(VF(9), VF(4), VF(5), 14);
        vu_sqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x10)));
        vu_sqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x20)));
    }
}

// 100% matching!
void	njRotateY(NJS_MATRIX *m, Angle ang)
{
    float fSin;
    float fCos;

    ang &= 0xFFFF;
    
    njSinCos(ang, &fSin, &fCos);
    
    if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         mfc1      t6, %1  */
    /* |         mfc1      t7, %2 */
    /* |          */
    /* |         qmtc2     t6, vf11 */
    /* |         qmtc2     t7, vf12 */
    /* |      */
    /* |         lqc2      vf4, 0x0(%0) */
    /* |         lqc2      vf5, 0x20(%0) */
    /* |  */
    /* |         vsub.x    vf10, vf0, vf11 */
    /* |          */
    /* |         vmulx.xyz vf6, vf4, vf12 */
    /* |         vmulx.xyz vf7, vf5, vf10 */
    /* |         vmulx.xyz vf4, vf4, vf11 */
    /* |         vmulx.xyz vf5, vf5, vf12 */
    /* |  */
    /* |         vadd.xyz  vf8, vf6, vf7 */
    /* |         vadd.xyz  vf9, vf4, vf5 */
    /* |  */
    /* |         sqc2      vf8, 0x0(%0) */
    /* |         sqc2      vf9, 0x20(%0) */
    /* |     .set reorder */
    /* |     " : : "r"(m), "f"(fSin), "f"(fCos) :  */
    /* |     ); */
        ee_gpr r14 = {{0}}, r15 = {{0}};
        __typeof__((fSin) + 0) op0 = (fSin);
        __typeof__((fCos) + 0) op1 = (fCos);
        __typeof__((m) + 0) op2 = (m);
        r14.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op0)));
        r15.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op1)));
        vu_qmtc2(11, r14);
        vu_qmtc2(12, r15);
        vu_lqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x0)));
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x20)));
        vu_sub(VF(10), VF(0), VF(11), 8);
        vu_mul_bc(VF(6), VF(4), VF(12)[0], 14);
        vu_mul_bc(VF(7), VF(5), VF(10)[0], 14);
        vu_mul_bc(VF(4), VF(4), VF(11)[0], 14);
        vu_mul_bc(VF(5), VF(5), VF(12)[0], 14);
        vu_add(VF(8), VF(6), VF(7), 14);
        vu_add(VF(9), VF(4), VF(5), 14);
        vu_sqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x0)));
        vu_sqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x20)));
    }
}

// 100% matching!
void	njRotateZ(NJS_MATRIX *m, Angle ang)
{
    float fSin;
    float fCos;

    ang &= 0xFFFF;
    
    njSinCos(ang, &fSin, &fCos);
    
    if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         mfc1      t6, %1  */
    /* |         mfc1      t7, %2 */
    /* |          */
    /* |         qmtc2     t6, vf11 */
    /* |         qmtc2     t7, vf12 */
    /* |      */
    /* |         lqc2      vf4, 0x0(%0) */
    /* |         lqc2      vf5, 0x10(%0) */
    /* |  */
    /* |         vsub.x    vf10, vf0, vf11 */
    /* |          */
    /* |         vmulx.xyz vf6, vf4, vf12 */
    /* |         vmulx.xyz vf7, vf5, vf11 */
    /* |         vmulx.xyz vf4, vf4, vf10 */
    /* |         vmulx.xyz vf5, vf5, vf12 */
    /* |  */
    /* |         vadd.xyz  vf8, vf6, vf7 */
    /* |         vadd.xyz  vf9, vf4, vf5 */
    /* |  */
    /* |         sqc2      vf8, 0x0(%0) */
    /* |         sqc2      vf9, 0x10(%0) */
    /* |     .set reorder */
    /* |     " : : "r"(m), "f"(fSin), "f"(fCos) :  */
    /* |     ); */
        ee_gpr r14 = {{0}}, r15 = {{0}};
        __typeof__((fSin) + 0) op0 = (fSin);
        __typeof__((fCos) + 0) op1 = (fCos);
        __typeof__((m) + 0) op2 = (m);
        r14.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op0)));
        r15.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op1)));
        vu_qmtc2(11, r14);
        vu_qmtc2(12, r15);
        vu_lqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x0)));
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x10)));
        vu_sub(VF(10), VF(0), VF(11), 8);
        vu_mul_bc(VF(6), VF(4), VF(12)[0], 14);
        vu_mul_bc(VF(7), VF(5), VF(11)[0], 14);
        vu_mul_bc(VF(4), VF(4), VF(10)[0], 14);
        vu_mul_bc(VF(5), VF(5), VF(12)[0], 14);
        vu_add(VF(8), VF(6), VF(7), 14);
        vu_add(VF(9), VF(4), VF(5), 14);
        vu_sqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x0)));
        vu_sqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x10)));
    }
}

// 100% matching!
void	njRotateXYZ(NJS_MATRIX *m, Angle angx, Angle angy, Angle angz)
{
    if (m == NULL) 
    {
        m = pNaMatMatrixStuckPtr;
    }
    
    njRotateZ(m, angz);
    njRotateY(m, angy);
    njRotateX(m, angx);
}

// 100% matching!
void njRotXYZ(NJS_MATRIX* pMatrix, int lAngleX, int lAngleY, int lAngleZ)
{
    float fSin;
    float fCos;

    lAngleX &= 0xFFFF;
    
    njSinCos(lAngleX, &fSin, &fCos);
    
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         lqc2      vf4,  0x0(%0) */
    /* |         lqc2      vf5, 0x10(%0) */
    /* |         lqc2      vf6, 0x20(%0) */
    /* |  */
    /* |         vsub.x    vf10, vf0, vf11 */
    /* |          */
    /* |         vmulx.xyz vf7, vf5, vf12 */
    /* |         vmulx.xyz vf8, vf6, vf11 */
    /* |         vmulx.xyz vf5, vf5, vf10 */
    /* |         vmulx.xyz vf6, vf6, vf12 */
    /* |  */
    /* |         vadd.xyz  vf6, vf5, vf6 */
    /* |         vadd.xyz  vf5, vf7, vf8 */
    /* |     .set reorder */
    /* |     " : : "r"(pMatrix) :   */
    /* |     ); */
        __typeof__((pMatrix) + 0) op0 = (pMatrix);
        vu_lqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x0)));
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x10)));
        vu_lqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20)));
        vu_sub(VF(10), VF(0), VF(11), 8);
        vu_mul_bc(VF(7), VF(5), VF(12)[0], 14);
        vu_mul_bc(VF(8), VF(6), VF(11)[0], 14);
        vu_mul_bc(VF(5), VF(5), VF(10)[0], 14);
        vu_mul_bc(VF(6), VF(6), VF(12)[0], 14);
        vu_add(VF(6), VF(5), VF(6), 14);
        vu_add(VF(5), VF(7), VF(8), 14);
    }

    lAngleY &= 0xFFFF;
    
    njSinCos(lAngleY, &fSin, &fCos);
    
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         vsub.x    vf10, vf0, vf11 */
    /* |          */
    /* |         vmulx.xyz vf7, vf4, vf12 */
    /* |         vmulx.xyz vf8, vf6, vf10 */
    /* |         vmulx.xyz vf4, vf4, vf11 */
    /* |         vmulx.xyz vf6, vf6, vf12 */
    /* |  */
    /* |         vadd.xyz  vf6, vf4, vf6 */
    /* |         vadd.xyz  vf4, vf7, vf8 */
    /* |     .set reorder */
    /* |     " : : "r"(pMatrix) :   */
    /* |     ); */
        vu_sub(VF(10), VF(0), VF(11), 8);
        vu_mul_bc(VF(7), VF(4), VF(12)[0], 14);
        vu_mul_bc(VF(8), VF(6), VF(10)[0], 14);
        vu_mul_bc(VF(4), VF(4), VF(11)[0], 14);
        vu_mul_bc(VF(6), VF(6), VF(12)[0], 14);
        vu_add(VF(6), VF(4), VF(6), 14);
        vu_add(VF(4), VF(7), VF(8), 14);
    }

    lAngleZ &= 0xFFFF;
    
    njSinCos(lAngleZ, &fSin, &fCos);
    
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         vsub.x    vf10, vf0, vf11 */
    /* |          */
    /* |         vmulx.xyz vf7, vf4, vf12 */
    /* |         vmulx.xyz vf8, vf5, vf11 */
    /* |         vmulx.xyz vf4, vf4, vf10 */
    /* |         vmulx.xyz vf5, vf5, vf12 */
    /* |  */
    /* |         vadd.xyz  vf8, vf7, vf8 */
    /* |         vadd.xyz  vf9, vf4, vf5 */
    /* |  */
    /* |         sqc2      vf8,  0x0(%0) */
    /* |         sqc2      vf9, 0x10(%0) */
    /* |         sqc2      vf6, 0x20(%0) */
    /* |     .set reorder */
    /* |     " : : "r"(pMatrix) :  */
    /* |     ); */
        __typeof__((pMatrix) + 0) op0 = (pMatrix);
        vu_sub(VF(10), VF(0), VF(11), 8);
        vu_mul_bc(VF(7), VF(4), VF(12)[0], 14);
        vu_mul_bc(VF(8), VF(5), VF(11)[0], 14);
        vu_mul_bc(VF(4), VF(4), VF(10)[0], 14);
        vu_mul_bc(VF(5), VF(5), VF(12)[0], 14);
        vu_add(VF(8), VF(7), VF(8), 14);
        vu_add(VF(9), VF(4), VF(5), 14);
        vu_sqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x0)));
        vu_sqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x10)));
        vu_sqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20)));
    }
}

// 100% matching!
void	njRotate(NJS_MATRIX *m, NJS_VECTOR *v, Angle ang)
{
    float fSin;
	float fCos;

    if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    ang &= 0xFFFF;
	ang /= 2;
    
    njSinCos(ang, &fSin, &fCos);

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         ldl       a4, 0x7(%1) */
    /* |         ldr       a4,   0(%1) */
    /* |  */
    /* |         lw        a5, NJS_VECTOR.z(%1) */
    /* |  */
    /* |         pcpyld    a4, a5, a4 */
    /* |  */
    /* |         qmtc2.ni  a4, vf4 */
    /* |  */
    /* |         vmul.xyz  vf5, vf4, vf4 */
    /* |  */
    /* |         vaddy.x   vf5, vf5, vf5 */
    /* |         vaddz.x   vf5, vf5, vf5 */
    /* |  */
    /* |         vrsqrt    Q, vf0w, vf5 */
    /* |  */
    /* |         mfc1      t4, %2  */
    /* |         mfc1      t5, %3 */
    /* |      */
    /* |         qmtc2     t4, vf11 */
    /* |         qmtc2     t5, vf12 */
    /* |  */
    /* |         vwaitq */
    /* |  */
    /* |         vmulq.x   vf11, vf11, Q */
    /* |         vmulx.xyz vf4,   vf4, vf11 */
    /* |         vmul.x    vf5,  vf12, vf12 */
    /* |  */
    /* |         vaddx.yz  vf5,  vf0,  vf5 */
    /* |  */
    /* |         vmulx.xyz vf6,  vf4,  vf4 */
    /* |         vmuly.xyz vf7,  vf4,  vf4 */
    /* |         vmulz.xyz vf8,  vf4,  vf4 */
    /* |         vmulx.xyz vf9,  vf4,  vf12 */
    /* |  */
    /* |         vadd.x    vf11, vf5,  vf6 */
    /* |         vsuby.x   vf11, vf11, vf7 */
    /* |         vsubz.x   vf11, vf11, vf8 */
    /* |         vsubz.x   vf12, vf7,  vf9 */
    /* |         vadd.x    vf12, vf12, vf12 */
    /* |         vaddy.x   vf13, vf8,  vf9 */
    /* |         vadd.x    vf13, vf13, vf13 */
    /* |  */
    /* |         vaddz.y   vf11, vf6,  vf9 */
    /* |         vadd.y    vf11, vf11, vf11 */
    /* |         vsubx.y   vf12, vf5,  vf6 */
    /* |         vadd.y    vf12, vf12, vf7 */
    /* |         vsubz.y   vf12, vf12, vf8 */
    /* |         vsubx.y   vf13, vf8,  vf9 */
    /* |         vadd.y    vf13, vf13, vf13 */
    /* |  */
    /* |         vsuby.z   vf11, vf6,  vf9 */
    /* |         vadd.z    vf11, vf11, vf11 */
    /* |         vaddx.z   vf12, vf7,  vf9 */
    /* |         vadd.z    vf12, vf12, vf12 */
    /* |         vsubx.z   vf13, vf5,  vf6 */
    /* |         vsuby.z   vf13, vf13, vf7 */
    /* |         vadd.z    vf13, vf13, vf8 */
    /* |  */
    /* |         lqc2      vf4,    0(%0) */
    /* |         lqc2      vf5, 0x10(%0) */
    /* |         lqc2      vf6, 0x20(%0) */
    /* |  */
    /* |         vmulx.xyz vf7,  vf4, vf11 */
    /* |         vmuly.xyz vf10, vf5, vf11 */
    /* |         vmulz.xyz vf11, vf6, vf11 */
    /* |  */
    /* |         vadd.xyz  vf7,  vf7, vf10 */
    /* |         vadd.xyz  vf7,  vf7, vf11 */
    /* |  */
    /* |         vmulx.xyz vf8,  vf4, vf12 */
    /* |         vmuly.xyz vf10, vf5, vf12 */
    /* |         vmulz.xyz vf12, vf6, vf12 */
    /* |  */
    /* |         vadd.xyz  vf8,  vf8, vf10 */
    /* |         vadd.xyz  vf8,  vf8, vf12 */
    /* |      */
    /* |         vmulx.xyz vf9,  vf4, vf13 */
    /* |         vmuly.xyz vf10, vf5, vf13 */
    /* |         vmulz.xyz vf13, vf6, vf13 */
    /* |  */
    /* |         vadd.xyz  vf9,  vf9, vf10 */
    /* |         vadd.xyz  vf9,  vf9, vf13 */
    /* |          */
    /* |         sqc2      vf7,    0(%0) */
    /* |         sqc2      vf8, 0x10(%0) */
    /* |         sqc2      vf9, 0x20(%0) */
    /* |     .set reorder */
    /* |     " : : "r"(m), "r"(v), "f"(fSin), "f"(fCos) :  */
    /* |     ); */
        ee_gpr r8 = {{0}}, r9 = {{0}}, r12 = {{0}}, r13 = {{0}};
        __typeof__((v) + 0) op0 = (v);
        __typeof__((fSin) + 0) op1 = (fSin);
        __typeof__((fCos) + 0) op2 = (fCos);
        __typeof__((m) + 0) op3 = (m);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x7)) - 7);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, z)))));
        r8 = ee_pcpyld(r9, r8);
        vu_qmtc2(4, r8);
        vu_mul(VF(5), VF(4), VF(4), 14);
        vu_add_bc(VF(5), VF(5), VF(5)[1], 8);
        vu_add_bc(VF(5), VF(5), VF(5)[2], 8);
        VQ = vu_rsqrt(VF(0)[3], VF(5)[0]);
        r12.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op1)));
        r13.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op2)));
        vu_qmtc2(11, r12);
        vu_qmtc2(12, r13);
        vu_mul_bc(VF(11), VF(11), VQ, 8);
        vu_mul_bc(VF(4), VF(4), VF(11)[0], 14);
        vu_mul(VF(5), VF(12), VF(12), 8);
        vu_add_bc(VF(5), VF(0), VF(5)[0], 6);
        vu_mul_bc(VF(6), VF(4), VF(4)[0], 14);
        vu_mul_bc(VF(7), VF(4), VF(4)[1], 14);
        vu_mul_bc(VF(8), VF(4), VF(4)[2], 14);
        vu_mul_bc(VF(9), VF(4), VF(12)[0], 14);
        vu_add(VF(11), VF(5), VF(6), 8);
        vu_sub_bc(VF(11), VF(11), VF(7)[1], 8);
        vu_sub_bc(VF(11), VF(11), VF(8)[2], 8);
        vu_sub_bc(VF(12), VF(7), VF(9)[2], 8);
        vu_add(VF(12), VF(12), VF(12), 8);
        vu_add_bc(VF(13), VF(8), VF(9)[1], 8);
        vu_add(VF(13), VF(13), VF(13), 8);
        vu_add_bc(VF(11), VF(6), VF(9)[2], 4);
        vu_add(VF(11), VF(11), VF(11), 4);
        vu_sub_bc(VF(12), VF(5), VF(6)[0], 4);
        vu_add(VF(12), VF(12), VF(7), 4);
        vu_sub_bc(VF(12), VF(12), VF(8)[2], 4);
        vu_sub_bc(VF(13), VF(8), VF(9)[0], 4);
        vu_add(VF(13), VF(13), VF(13), 4);
        vu_sub_bc(VF(11), VF(6), VF(9)[1], 2);
        vu_add(VF(11), VF(11), VF(11), 2);
        vu_add_bc(VF(12), VF(7), VF(9)[0], 2);
        vu_add(VF(12), VF(12), VF(12), 2);
        vu_sub_bc(VF(13), VF(5), VF(6)[0], 2);
        vu_sub_bc(VF(13), VF(13), VF(7)[1], 2);
        vu_add(VF(13), VF(13), VF(8), 2);
        vu_lqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0)));
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x10)));
        vu_lqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x20)));
        vu_mul_bc(VF(7), VF(4), VF(11)[0], 14);
        vu_mul_bc(VF(10), VF(5), VF(11)[1], 14);
        vu_mul_bc(VF(11), VF(6), VF(11)[2], 14);
        vu_add(VF(7), VF(7), VF(10), 14);
        vu_add(VF(7), VF(7), VF(11), 14);
        vu_mul_bc(VF(8), VF(4), VF(12)[0], 14);
        vu_mul_bc(VF(10), VF(5), VF(12)[1], 14);
        vu_mul_bc(VF(12), VF(6), VF(12)[2], 14);
        vu_add(VF(8), VF(8), VF(10), 14);
        vu_add(VF(8), VF(8), VF(12), 14);
        vu_mul_bc(VF(9), VF(4), VF(13)[0], 14);
        vu_mul_bc(VF(10), VF(5), VF(13)[1], 14);
        vu_mul_bc(VF(13), VF(6), VF(13)[2], 14);
        vu_add(VF(9), VF(9), VF(10), 14);
        vu_add(VF(9), VF(9), VF(13), 14);
        vu_sqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0)));
        vu_sqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x10)));
        vu_sqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x20)));
    }
}

// 100% matching!
void	njScale(NJS_MATRIX *m, Float sx, Float sy, Float sz)
{
    if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         mfc1      t2, %1  */
    /* |         mfc1      t3, %2 */
    /* |         mfc1      t4, %3 */
    /* |          */
    /* |         qmtc2     t2, vf4 */
    /* |         qmtc2     t3, vf5 */
    /* |         qmtc2     t4, vf6 */
    /* |      */
    /* |         lqc2      vf7,  0x0(%0) */
    /* |         lqc2      vf8, 0x10(%0) */
    /* |         lqc2      vf9, 0x20(%0) */
    /* |  */
    /* |         vmulx.xyz vf7, vf7, vf4 */
    /* |         vmulx.xyz vf8, vf8, vf5 */
    /* |         vmulx.xyz vf9, vf9, vf6 */
    /* |      */
    /* |         sqc2      vf7,  0x0(%0) */
    /* |         sqc2      vf8, 0x10(%0) */
    /* |         sqc2      vf9, 0x20(%0) */
    /* |     .set reorder */
    /* |     " : : "r"(m), "f"(sx), "f"(sy), "f"(sz) :  */
    /* |     ); */
        ee_gpr r10 = {{0}}, r11 = {{0}}, r12 = {{0}};
        __typeof__((sx) + 0) op0 = (sx);
        __typeof__((sy) + 0) op1 = (sy);
        __typeof__((sz) + 0) op2 = (sz);
        __typeof__((m) + 0) op3 = (m);
        r10.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op0)));
        r11.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op1)));
        r12.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op2)));
        vu_qmtc2(4, r10);
        vu_qmtc2(5, r11);
        vu_qmtc2(6, r12);
        vu_lqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x0)));
        vu_lqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x10)));
        vu_lqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x20)));
        vu_mul_bc(VF(7), VF(7), VF(4)[0], 14);
        vu_mul_bc(VF(8), VF(8), VF(5)[0], 14);
        vu_mul_bc(VF(9), VF(9), VF(6)[0], 14);
        vu_sqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x0)));
        vu_sqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x10)));
        vu_sqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x20)));
    }
}

// 100% matching!
void	njScaleV(NJS_MATRIX *m, NJS_VECTOR *v)
{
	if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         ldl       a4, 0x7(%1) */
    /* |         ldr       a4,   0(%1) */
    /* |  */
    /* |         lw        a5, NJS_VECTOR.z(%1)  */
    /* |  */
    /* |         pcpyld    a4, a5, a4 */
    /* |  */
    /* |         qmtc2.ni  a4, vf4 */
    /* |  */
    /* |         lqc2      vf5,    0(%0) */
    /* |         lqc2      vf6, 0x10(%0) */
    /* |         lqc2      vf7, 0x20(%0) */
    /* |  */
    /* |         vmulx.xyz vf5, vf5, vf4 */
    /* |         vmuly.xyz vf6, vf6, vf4 */
    /* |         vmulz.xyz vf7, vf7, vf4 */
    /* |      */
    /* |         sqc2      vf5,     0(%0) */
    /* |         sqc2      vf6,  0x10(%0) */
    /* |         sqc2      vf7,  0x20(%0) */
    /* |     .set reorder */
    /* |     " : : "r"(m), "r"(v) :  */
    /* |     ); */
        ee_gpr r8 = {{0}}, r9 = {{0}};
        __typeof__((v) + 0) op0 = (v);
        __typeof__((m) + 0) op1 = (m);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x7)) - 7);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, z)))));
        r8 = ee_pcpyld(r9, r8);
        vu_qmtc2(4, r8);
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
        vu_lqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x10)));
        vu_lqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x20)));
        vu_mul_bc(VF(5), VF(5), VF(4)[0], 14);
        vu_mul_bc(VF(6), VF(6), VF(4)[1], 14);
        vu_mul_bc(VF(7), VF(7), VF(4)[2], 14);
        vu_sqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
        vu_sqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x10)));
        vu_sqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x20)));
    }
}

// 100% matching!
Bool	njInvertMatrix(NJS_MATRIX *m)
{
	if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         lq          t0,     0(%0) */
    /* |         lq          t1,  0x10(%0) */
    /* |         lq          t2,  0x20(%0) */
    /* |         lqc2        vf4, 0x30(%0) */
    /* |  */
    /* |         vmove       vf5, vf4 */
    /* |  */
    /* |         vsub.xyz    vf4, vf4, vf4 */
    /* |  */
    /* |         vmove       vf10, vf4 */
    /* |  */
    /* |         qmfc2.ni    t3, vf4 */
    /* |  */
    /* |         pextlw      t4, t1, t0 */
    /* |         pextuw      t5, t1, t0 */
    /* |   */
    /* |         pextlw      t6, t3, t2 */
    /* |         pextuw      t7, t3, t2 */
    /* |   */
    /* |         pcpyld      t0, t6, t4 */
    /* |         pcpyud      t1, t4, t6 */
    /* |         pcpyld      t2, t7, t5 */
    /* |   */
    /* |         qmtc2.ni    t0, vf7 */
    /* |         qmtc2.ni    t1, vf8 */
    /* |         qmtc2.ni    t2, vf9 */
    /* |  */
    /* |         vmulax.xyz  ACC, vf7, vf5 */
    /* |  */
    /* |         vmadday.xyz ACC, vf8, vf5 */
    /* |         vmaddz.xyz  vf4, vf9, vf5 */
    /* |  */
    /* |         vsub.xyz    vf4, vf10, vf4 */
    /* |  */
    /* |         sq          t0,     0(%0) */
    /* |         sq          t1,  0x10(%0) */
    /* |         sq          t2,  0x20(%0) */
    /* |         sqc2        vf4, 0x30(%0) */
    /* |     .set reorder */
    /* |     " : : "r"(m) :  */
    /* |     ); */
        ee_gpr r8 = {{0}}, r9 = {{0}}, r10 = {{0}}, r11 = {{0}}, r12 = {{0}}, r13 = {{0}}, r14 = {{0}}, r15 = {{0}};
        __typeof__((m) + 0) op0 = (m);
        r8 = (*(ee_gpr *)((((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0))) & ~(uintptr_t)15));
        r9 = (*(ee_gpr *)((((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x10))) & ~(uintptr_t)15));
        r10 = (*(ee_gpr *)((((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20))) & ~(uintptr_t)15));
        vu_lqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x30)));
        vu_move(VF(5), VF(4), 15);
        vu_sub(VF(4), VF(4), VF(4), 14);
        vu_move(VF(10), VF(4), 15);
        r11 = vu_qmfc2(4);
        r12 = ee_pextlw(r9, r8);
        r13 = ee_pextuw(r9, r8);
        r14 = ee_pextlw(r11, r10);
        r15 = ee_pextuw(r11, r10);
        r8 = ee_pcpyld(r14, r12);
        r9 = ee_pcpyud(r12, r14);
        r10 = ee_pcpyld(r15, r13);
        vu_qmtc2(7, r8);
        vu_qmtc2(8, r9);
        vu_qmtc2(9, r10);
        vu_mul_bc(VACC, VF(7), VF(5)[0], 14);
        vu_madd_bc(VACC, VF(8), VF(5)[1], 14);
        vu_madd_bc(VF(4), VF(9), VF(5)[2], 14);
        vu_sub(VF(4), VF(10), VF(4), 14);
        *(ee_gpr *)((((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0))) & ~(uintptr_t)15) = r8;
        *(ee_gpr *)((((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x10))) & ~(uintptr_t)15) = r9;
        *(ee_gpr *)((((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20))) & ~(uintptr_t)15) = r10;
        vu_sqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x30)));
    }

    return TRUE;
}

// 100% matching!
void	njTransposeMatrix(NJS_MATRIX *m)
{
	if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         lqc2    vf4,    0(%0) */
    /* |         lqc2    vf5, 0x10(%0) */
    /* |         lqc2    vf6, 0x20(%0) */
    /* |  */
    /* |         vaddx.x vf7, vf0, vf4 */
    /* |         vaddx.y vf7, vf0, vf5 */
    /* |         vaddx.z vf7, vf0, vf6 */
    /* |  */
    /* |         vaddy.x vf8, vf0, vf4 */
    /* |         vaddy.y vf8, vf0, vf5 */
    /* |         vaddy.z vf8, vf0, vf6 */
    /* |  */
    /* |         vaddz.x vf9, vf0, vf4 */
    /* |         vaddz.y vf9, vf0, vf5 */
    /* |         vaddz.z vf9, vf0, vf6 */
    /* |  */
    /* |         sqc2    vf7,    0(%0) */
    /* |         sqc2    vf8, 0x10(%0) */
    /* |         sqc2    vf9, 0x20(%0) */
    /* |     .set reorder */
    /* |     " : : "r"(m) :  */
    /* |     ); */
        __typeof__((m) + 0) op0 = (m);
        vu_lqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x10)));
        vu_lqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20)));
        vu_add_bc(VF(7), VF(0), VF(4)[0], 8);
        vu_add_bc(VF(7), VF(0), VF(5)[0], 4);
        vu_add_bc(VF(7), VF(0), VF(6)[0], 2);
        vu_add_bc(VF(8), VF(0), VF(4)[1], 8);
        vu_add_bc(VF(8), VF(0), VF(5)[1], 4);
        vu_add_bc(VF(8), VF(0), VF(6)[1], 2);
        vu_add_bc(VF(9), VF(0), VF(4)[2], 8);
        vu_add_bc(VF(9), VF(0), VF(5)[2], 4);
        vu_add_bc(VF(9), VF(0), VF(6)[2], 2);
        vu_sqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        vu_sqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x10)));
        vu_sqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20)));
    }
}

// 100% matching!
static float njAtan2b(float a, float b)
{
    return atan2(a, b);
}

// 100% matching!
/* Hand-written replacement for the EE asm version.
 * The original builds the mirror as Rx*Rz*Scale(1,-1,1)*Rz^-1*Rx^-1 with a translation
 * row of 2*(p.n^)*n, then njMultiMatrix(m, &mat). That is the Householder reflection
 * I - 2*n^*n^T; it is computed directly here. The original's degenerate paths (plane
 * through the origin, normal on the X axis) read uninitialised registers or return
 * through the inline asm; here they fall out of the same formula. */
void	njMirror(NJS_MATRIX *m,NJS_PLANE *pl)
{
    NJS_MATRIX mat;
    float nx, ny, nz, len, inv, d;
    int i, j;

    if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    nx = pl->vx;
    ny = pl->vy;
    nz = pl->vz;

    len = nx * nx + ny * ny + nz * nz;
    inv = (len > 0.0f) ? 1.0f / sqrtf(len) : 0.0f;

    {
        float n[3];

        n[0] = nx * inv;
        n[1] = ny * inv;
        n[2] = nz * inv;

        d = n[0] * pl->px + n[1] * pl->py + n[2] * pl->pz;

        for (i = 0; i < 3; i++)
        {
            for (j = 0; j < 3; j++)
            {
                mat[i * 4 + j] = ((i == j) ? 1.0f : 0.0f) - 2.0f * n[i] * n[j];
            }

            mat[i * 4 + 3] = 0.0f;
        }

        /* translation uses the unnormalised normal, as the original does */
        mat[12] = 2.0f * d * nx;
        mat[13] = 2.0f * d * ny;
        mat[14] = 2.0f * d * nz;
        mat[15] = 1.0f;
    }

    njMultiMatrix(m, &mat);
}

// 100% matching!
void	njCalcPoint(NJS_MATRIX *m, NJS_POINT3 *ps, NJS_POINT3 *pd)
{
    if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         ldl         t0, 7(%1) */
    /* |         ldr         t0, 0(%1) */
    /* |          */
    /* |         lw          t1, 8(%1) */
    /* |          */
    /* |         pcpyld      t0, t1, t0 */
    /* |      */
    /* |         qmtc2       t0, vf4 */
    /* |      */
    /* |         lqc2        vf5, 0(%0) */
    /* |         lqc2        vf6, 0x10(%0) */
    /* |         lqc2        vf7, 0x20(%0) */
    /* |         lqc2        vf8, 0x30(%0) */
    /* |  */
    /* |         vmulax.xyz  ACC,  vf5, vf4 */
    /* |          */
    /* |         vmadday.xyz ACC,  vf6, vf4 */
    /* |         vmaddaz.xyz ACC,  vf7, vf4 */
    /* |         vmaddw.xyz  vf18, vf8, vf0 */
    /* |  */
    /* |         qmfc2       t0, vf18 */
    /* |      */
    /* |         pcpyud      t1, t0, t0 */
    /* |      */
    /* |         sdl         t0, 7(%2) */
    /* |         sdr         t0, 0(%2) */
    /* |          */
    /* |         sw          t1, 8(%2) */
    /* |     .set reorder */
    /* |     " : : "r"(m), "r"(ps), "r"(pd) :  */
    /* |     ); */
        ee_gpr r8 = {{0}}, r9 = {{0}};
        __typeof__((ps) + 0) op0 = (ps);
        __typeof__((m) + 0) op1 = (m);
        __typeof__((pd) + 0) op2 = (pd);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (7)) - 7);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (8))));
        r8 = ee_pcpyld(r9, r8);
        vu_qmtc2(4, r8);
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
        vu_lqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x10)));
        vu_lqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x20)));
        vu_lqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x30)));
        vu_mul_bc(VACC, VF(5), VF(4)[0], 14);
        vu_madd_bc(VACC, VF(6), VF(4)[1], 14);
        vu_madd_bc(VACC, VF(7), VF(4)[2], 14);
        vu_madd_bc(VF(18), VF(8), VF(0)[3], 14);
        r8 = vu_qmfc2(18);
        r9 = ee_pcpyud(r8, r8);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (7)) - 7, r8.d[0]);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0)), r8.d[0]);
        *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (8))) = (uint32_t)(r9.d[0]);
    }
}

// 100% matching!
void njCalcPoint4(NJS_MATRIX* pMatrix, NJS_POINT4* pSrcPoint, NJS_POINT4* pDstPoint)
{
    if (pMatrix == NULL)
    {
        pMatrix = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         lqc2        vf4, 0(%1) */
    /* |         lqc2        vf5, 0(%0) */
    /* |         lqc2        vf6, 0x10(%0) */
    /* |         lqc2        vf7, 0x20(%0) */
    /* |         lqc2        vf8, 0x30(%0) */
    /* |  */
    /* |         vmulax.xyz  ACC,  vf5, vf4x */
    /* |          */
    /* |         vmadday.xyz ACC,  vf6, vf4y */
    /* |         vmaddaz.xyz ACC,  vf7, vf4z */
    /* |         vmaddw.xyz  vf18, vf8, vf0w */
    /* |      */
    /* |         sqc2        vf18, 0(%2) */
    /* |     .set reorder */
    /* |     " : : "r"(pMatrix), "r"(pSrcPoint), "r"(pDstPoint) :  */
    /* |     ); */
        __typeof__((pSrcPoint) + 0) op0 = (pSrcPoint);
        __typeof__((pMatrix) + 0) op1 = (pMatrix);
        __typeof__((pDstPoint) + 0) op2 = (pDstPoint);
        vu_lqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
        vu_lqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x10)));
        vu_lqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x20)));
        vu_lqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x30)));
        vu_mul_bc(VACC, VF(5), VF(4)[0], 14);
        vu_madd_bc(VACC, VF(6), VF(4)[1], 14);
        vu_madd_bc(VACC, VF(7), VF(4)[2], 14);
        vu_madd_bc(VF(18), VF(8), VF(0)[3], 14);
        vu_sqc2(18, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0)));
    }
}

// 100% matching!
void njCalcPointCN(NJS_POINT3* pSrcPoint, NJS_POINT3* pDstPoint)
{
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         ldl         t2, 7(%0) */
    /* |         ldr         t2, 0(%0) */
    /* |          */
    /* |         lw          t3, 8(%0) */
    /* |          */
    /* |         pcpyld      t2, t3, t2 */
    /* |      */
    /* |         qmtc2       t2, vf4 */
    /* |      */
    /* |         vmulax.xyz  ACC, vf28, vf4 */
    /* |          */
    /* |         vmadday.xyz ACC, vf29, vf4 */
    /* |         vmaddaz.xyz ACC, vf30, vf4 */
    /* |  */
    /* |         vmaddw.xyz  vf18, vf31, vf0 */
    /* |  */
    /* |         qmfc2       t0, vf18 */
    /* |      */
    /* |         pcpyud      t1, t0, t0 */
    /* |      */
    /* |         sdl         t0, 7(%1) */
    /* |         sdr         t0, 0(%1) */
    /* |          */
    /* |         sw          t1, 8(%1) */
    /* |     .set reorder */
    /* |     " : : "r"(pSrcPoint), "r"(pDstPoint) :  */
    /* |     ); */
        ee_gpr r8 = {{0}}, r9 = {{0}}, r10 = {{0}}, r11 = {{0}};
        __typeof__((pSrcPoint) + 0) op0 = (pSrcPoint);
        __typeof__((pDstPoint) + 0) op1 = (pDstPoint);
        r10.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (7)) - 7);
        r10.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        r11.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (8))));
        r10 = ee_pcpyld(r11, r10);
        vu_qmtc2(4, r10);
        vu_mul_bc(VACC, VF(28), VF(4)[0], 14);
        vu_madd_bc(VACC, VF(29), VF(4)[1], 14);
        vu_madd_bc(VACC, VF(30), VF(4)[2], 14);
        vu_madd_bc(VF(18), VF(31), VF(0)[3], 14);
        r8 = vu_qmfc2(18);
        r9 = ee_pcpyud(r8, r8);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (7)) - 7, r8.d[0]);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)), r8.d[0]);
        *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (8))) = (uint32_t)(r9.d[0]);
    }
}

// 100% matching!
void	njAddVector(NJS_VECTOR *vd, NJS_VECTOR *vs)
{
	{ /* translated from EE asm by agent mips2c; original kept below */
	/* |  */
	/* |     (" */
	/* |     .set noreorder */
	/* |         lwc1   f8, NJS_VECTOR.x(%0) */
	/* |         lwc1   f9, NJS_VECTOR.y(%0) */
	/* |         lwc1  f10, NJS_VECTOR.z(%0) */
	/* |  */
	/* |         lwc1  f11, NJS_VECTOR.x(%1) */
	/* |         lwc1  f12, NJS_VECTOR.y(%1) */
	/* |         lwc1  f13, NJS_VECTOR.z(%1) */
	/* |  */
	/* |         add.s  f8,  f8, f11 */
	/* |         add.s  f9,  f9, f12 */
	/* |         add.s f10, f10, f13 */
	/* |  */
	/* |         swc1   f8, NJS_VECTOR.x(%0) */
	/* |         swc1   f9, NJS_VECTOR.y(%0) */
	/* |         swc1  f10, NJS_VECTOR.z(%0) */
	/* |     .set reorder */
	/* |     " : : "r"(vd), "r"(vs) :  */
	/* |     ); */
	    float f8 = 0, f9 = 0, f10 = 0, f11 = 0, f12 = 0, f13 = 0;
	    __typeof__((vd) + 0) op0 = (vd);
	    __typeof__((vs) + 0) op1 = (vs);
	    f8 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, x)))));
	    f9 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, y)))));
	    f10 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, z)))));
	    f11 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + ((int)offsetof(NJS_VECTOR, x)))));
	    f12 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + ((int)offsetof(NJS_VECTOR, y)))));
	    f13 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + ((int)offsetof(NJS_VECTOR, z)))));
	    f8 = f8 + f11;
	    f9 = f9 + f12;
	    f10 = f10 + f13;
	    *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, x)))) = ee_fbits(f8);
	    *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, y)))) = ee_fbits(f9);
	    *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, z)))) = ee_fbits(f10);
	}
}

// 100% matching!
void	njSubVector(NJS_VECTOR *vd, NJS_VECTOR *vs)
{
	{ /* translated from EE asm by agent mips2c; original kept below */
	/* |  */
	/* |     (" */
	/* |     .set noreorder */
	/* |         lwc1   f8, NJS_VECTOR.x(%0) */
	/* |         lwc1   f9, NJS_VECTOR.y(%0) */
	/* |         lwc1  f10, NJS_VECTOR.z(%0) */
	/* |  */
	/* |         lwc1  f11, NJS_VECTOR.x(%1) */
	/* |         lwc1  f12, NJS_VECTOR.y(%1) */
	/* |         lwc1  f13, NJS_VECTOR.z(%1) */
	/* |  */
	/* |         sub.s  f8,  f8, f11 */
	/* |         sub.s  f9,  f9, f12 */
	/* |         sub.s f10, f10, f13 */
	/* |  */
	/* |         swc1   f8, NJS_VECTOR.x(%0) */
	/* |         swc1   f9, NJS_VECTOR.y(%0) */
	/* |         swc1  f10, NJS_VECTOR.z(%0) */
	/* |     .set reorder */
	/* |     " : : "r"(vd), "r"(vs) :  */
	/* |     ); */
	    float f8 = 0, f9 = 0, f10 = 0, f11 = 0, f12 = 0, f13 = 0;
	    __typeof__((vd) + 0) op0 = (vd);
	    __typeof__((vs) + 0) op1 = (vs);
	    f8 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, x)))));
	    f9 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, y)))));
	    f10 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, z)))));
	    f11 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + ((int)offsetof(NJS_VECTOR, x)))));
	    f12 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + ((int)offsetof(NJS_VECTOR, y)))));
	    f13 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + ((int)offsetof(NJS_VECTOR, z)))));
	    f8 = f8 - f11;
	    f9 = f9 - f12;
	    f10 = f10 - f13;
	    *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, x)))) = ee_fbits(f8);
	    *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, y)))) = ee_fbits(f9);
	    *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, z)))) = ee_fbits(f10);
	}
}

// 100% matching!
void	njCalcVector(NJS_MATRIX *m, NJS_VECTOR *vs, NJS_VECTOR *vd)
{
    if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         ldl         t0, 7(%1) */
    /* |         ldr         t0, 0(%1) */
    /* |          */
    /* |         lw          t1, 8(%1) */
    /* |          */
    /* |         pcpyld      t0, t1, t0 */
    /* |      */
    /* |         qmtc2       t0, vf4 */
    /* |      */
    /* |         lqc2        vf7, 0(%0) */
    /* |         lqc2        vf8, 0x10(%0) */
    /* |         lqc2        vf9, 0x20(%0) */
    /* |  */
    /* |         vmulax.xyz  ACC,  vf7, vf4 */
    /* |          */
    /* |         vmadday.xyz ACC,  vf8, vf4 */
    /* |         vmaddz.xyz  vf18, vf9, vf4 */
    /* |  */
    /* |         qmfc2       t0, vf18 */
    /* |      */
    /* |         pcpyud      t1, t0, t0 */
    /* |      */
    /* |         sdl         t0, 7(%2) */
    /* |         sdr         t0, 0(%2) */
    /* |          */
    /* |         sw          t1, 8(%2) */
    /* |     .set reorder */
    /* |     " : : "r"(m), "r"(vs), "r"(vd) :  */
    /* |     ); */
        ee_gpr r8 = {{0}}, r9 = {{0}};
        __typeof__((vs) + 0) op0 = (vs);
        __typeof__((m) + 0) op1 = (m);
        __typeof__((vd) + 0) op2 = (vd);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (7)) - 7);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (8))));
        r8 = ee_pcpyld(r9, r8);
        vu_qmtc2(4, r8);
        vu_lqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
        vu_lqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x10)));
        vu_lqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x20)));
        vu_mul_bc(VACC, VF(7), VF(4)[0], 14);
        vu_madd_bc(VACC, VF(8), VF(4)[1], 14);
        vu_madd_bc(VF(18), VF(9), VF(4)[2], 14);
        r8 = vu_qmfc2(18);
        r9 = ee_pcpyud(r8, r8);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (7)) - 7, r8.d[0]);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0)), r8.d[0]);
        *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (8))) = (uint32_t)(r9.d[0]);
    }
}

// 100% matching!
Float	njUnitVector(NJS_VECTOR *v)
{
	float ret;

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         ldl       a4, 0x7(%1) */
    /* |         ldr       a4,   0(%1) */
    /* |       */
    /* |         lw        a5, NJS_VECTOR.z(%1)  */
    /* |       */
    /* |         pcpyld    a4, a5, a4 */
    /* |       */
    /* |         qmtc2.ni  a4, vf4 */
    /* |       */
    /* |         vmul.xyz  vf5, vf4, vf4 */
    /* |       */
    /* |         vaddy.x   vf5, vf5, vf5 */
    /* |         vaddz.x   vf5, vf5, vf5 */
    /* |       */
    /* |         vrsqrt    Q, vf0w, vf5 */
    /* |       */
    /* |         vwaitq    */
    /* |  */
    /* |         vmulq.xyz vf6, vf4, Q */
    /* |         vmulq.x   vf7, vf5, Q */
    /* |  */
    /* |         qmfc2.ni  a6, vf6 */
    /* |  */
    /* |         pcpyud    a7, a6, a6 */
    /* |  */
    /* |         sdl       a6, 0x7(%1) */
    /* |         sdr       a6,   0(%1) */
    /* |       */
    /* |         sw        a7, NJS_VECTOR.z(%1)  */
    /* |       */
    /* |         qmfc2.ni  v0, vf7 */
    /* |  */
    /* |         mtc1      v0, %0 */
    /* |     .set reorder */
    /* |     " : "=r"(ret) : "r"(v) :  */
    /* |     ); */
        ee_gpr r2 = {{0}}, r8 = {{0}}, r9 = {{0}}, r10 = {{0}}, r11 = {{0}};
        __typeof__((v) + 0) op0 = (v);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x7)) - 7);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, z)))));
        r8 = ee_pcpyld(r9, r8);
        vu_qmtc2(4, r8);
        vu_mul(VF(5), VF(4), VF(4), 14);
        vu_add_bc(VF(5), VF(5), VF(5)[1], 8);
        vu_add_bc(VF(5), VF(5), VF(5)[2], 8);
        VQ = vu_rsqrt(VF(0)[3], VF(5)[0]);
        vu_mul_bc(VF(6), VF(4), VQ, 14);
        vu_mul_bc(VF(7), VF(5), VQ, 8);
        r10 = vu_qmfc2(6);
        r11 = ee_pcpyud(r10, r10);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x7)) - 7, r10.d[0]);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)), r10.d[0]);
        *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, z)))) = (uint32_t)(r11.d[0]);
        r2 = vu_qmfc2(7);
        EE_CVAR_SETF(ret, ee_bitsf((uint32_t)(r2.d[0])));
    }

    return ret;
}

// 100% matching!
Float	njScalor(NJS_VECTOR *v)
{
	float ret;

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         ldl      a4, 0x7(%1) */
    /* |         ldr      a4,   0(%1) */
    /* |       */
    /* |         lw       a5, NJS_VECTOR.z(%1)  */
    /* |       */
    /* |         pcpyld   a4, a5, a4 */
    /* |       */
    /* |         qmtc2.ni a4, vf4 */
    /* |       */
    /* |         vmul.xyz vf5, vf4, vf4 */
    /* |       */
    /* |         vaddy.x  vf5, vf5, vf5 */
    /* |         vaddz.x  vf5, vf5, vf5 */
    /* |       */
    /* |         vsqrt    Q, vf5 */
    /* |       */
    /* |         vwaitq    */
    /* |       */
    /* |         vaddq.x  vf6, vf0, Q */
    /* |       */
    /* |         qmfc2.ni v0, vf6 */
    /* |  */
    /* |         mtc1     v0, %0 */
    /* |     .set reorder */
    /* |     " : "=r"(ret) : "r"(v) :  */
    /* |     ); */
        ee_gpr r2 = {{0}}, r8 = {{0}}, r9 = {{0}};
        __typeof__((v) + 0) op0 = (v);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x7)) - 7);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, z)))));
        r8 = ee_pcpyld(r9, r8);
        vu_qmtc2(4, r8);
        vu_mul(VF(5), VF(4), VF(4), 14);
        vu_add_bc(VF(5), VF(5), VF(5)[1], 8);
        vu_add_bc(VF(5), VF(5), VF(5)[2], 8);
        VQ = sqrtf(fabsf(VF(5)[0]));
        vu_add_bc(VF(6), VF(0), VQ, 8);
        r2 = vu_qmfc2(6);
        EE_CVAR_SETF(ret, ee_bitsf((uint32_t)(r2.d[0])));
    }

    return ret;
}

// 100% matching!
Float	njScalor2(NJS_VECTOR *v)
{
	float ret;

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         ldl      a4, 0x7(%1) */
    /* |         ldr      a4,   0(%1) */
    /* |       */
    /* |         lw       a5, NJS_VECTOR.z(%1)  */
    /* |       */
    /* |         pcpyld   a4, a5, a4 */
    /* |       */
    /* |         qmtc2.ni a4, vf4 */
    /* |       */
    /* |         vmul.xyz vf5, vf4, vf4 */
    /* |       */
    /* |         vaddy.x  vf5, vf5, vf5 */
    /* |         vaddz.x  vf6, vf5, vf5 */
    /* |       */
    /* |         qmfc2.ni v0, vf6 */
    /* |          */
    /* |         mtc1     v0, %0 */
    /* |     .set reorder */
    /* |     " : "=r"(ret) : "r"(v) :  */
    /* |     ); */
        ee_gpr r2 = {{0}}, r8 = {{0}}, r9 = {{0}};
        __typeof__((v) + 0) op0 = (v);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x7)) - 7);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, z)))));
        r8 = ee_pcpyld(r9, r8);
        vu_qmtc2(4, r8);
        vu_mul(VF(5), VF(4), VF(4), 14);
        vu_add_bc(VF(5), VF(5), VF(5)[1], 8);
        vu_add_bc(VF(6), VF(5), VF(5)[2], 8);
        r2 = vu_qmfc2(6);
        EE_CVAR_SETF(ret, ee_bitsf((uint32_t)(r2.d[0])));
    }

    return ret;
}

// 100% matching!
void	njProjectScreen(NJS_MATRIX *m, NJS_POINT3 *p3, NJS_POINT2 *p2)
{
    NJS_POINT3 Point;
    
    if (m == NULL)
    {
        m = pNaMatMatrixStuckPtr;
    }
    
    njMulMatrixCN(&NaViewScreenMatrix, m);
    
    njCalcPointCN(p3, &Point);
    
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         mfc1     t0, %2 */
    /* |          */
    /* |         qmtc2    t0, vf4 */
    /* |  */
    /* |         vdiv     Q, vf4x, vf18z */
    /* |  */
    /* |         lw       t1, fNaViwOffsetX */
    /* |         lw       t2, fNaViwOffsetY */
    /* |          */
    /* |         qmtc2    t1, vf5 */
    /* |         qmtc2    t2, vf6 */
    /* |  */
    /* |         vwaitq */
    /* |  */
    /* |         vaddq.z  vf8, vf0, Q */
    /* |  */
    /* |         vmulq.xy vf7, vf18, Q */
    /* |          */
    /* |         vaddx.x  vf8, vf7, vf5 */
    /* |         vaddx.y  vf8, vf7, vf6 */
    /* |  */
    /* |         qmfc2    t0, vf8 */
    /* |      */
    /* |         pcpyud   t1, t0, t0 */
    /* |      */
    /* |         sdl      t0, 7(%0) */
    /* |         sdr      t0, 0(%0) */
    /* |          */
    /* |         sw       t1, 8(%1) */
    /* |     .set reorder */
    /* |     " : : "r"(p2), "r"(&Point), "f"(_nj_screen_.dist) :  */
    /* |     ); */
        ee_gpr r8 = {{0}}, r9 = {{0}}, r10 = {{0}};
        __typeof__((_nj_screen_.dist) + 0) op0 = (_nj_screen_.dist);
        __typeof__((p2) + 0) op1 = (p2);
        __typeof__((&Point) + 0) op2 = (&Point);
        r8.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op0)));
        vu_qmtc2(4, r8);
        VQ = vu_div(VF(4)[0], VF(18)[2]);
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)&fNaViwOffsetX)));
        r10.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)&fNaViwOffsetY)));
        vu_qmtc2(5, r9);
        vu_qmtc2(6, r10);
        vu_add_bc(VF(8), VF(0), VQ, 2);
        vu_mul_bc(VF(7), VF(18), VQ, 12);
        vu_add_bc(VF(8), VF(7), VF(5)[0], 8);
        vu_add_bc(VF(8), VF(7), VF(6)[0], 4);
        r8 = vu_qmfc2(8);
        r9 = ee_pcpyud(r8, r8);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (7)) - 7, r8.d[0]);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)), r8.d[0]);
        *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (8))) = (uint32_t)(r9.d[0]);
    }
}

// 100% matching! 
Float	njOuterProduct(NJS_VECTOR *v1, NJS_VECTOR *v2, NJS_VECTOR *ov)
{
	float ret;

	{ /* translated from EE asm by agent mips2c; original kept below */
	/* |  */
	/* |     (" */
	/* |     .set noreorder */
	/* |         ldl      a4, 0x7(%1) */
	/* |         ldr      a4,   0(%1) */
	/* |       */
	/* |         lw       a5, NJS_VECTOR.z(%1)  */
	/* |  */
	/* |         ldl      a6, 0x7(%2) */
	/* |         ldr      a6,   0(%2) */
	/* |       */
	/* |         lw       a7, NJS_VECTOR.z(%2)  */
	/* |       */
	/* |         pcpyld   a4, a5, a4 */
	/* |         pcpyld   a6, a7, a6 */
	/* |       */
	/* |         qmtc2.ni a4, vf4 */
	/* |         qmtc2.ni a6, vf5 */
	/* |  */
	/* |         vopmula  ACC, vf4, vf5 */
	/* |         vopmsub  vf6, vf5, vf4 */
	/* |       */
	/* |         vmul.xyz vf7, vf6, vf6 */
	/* |       */
	/* |         vaddy.x  vf7, vf7, vf7 */
	/* |         vaddz.x  vf7, vf7, vf7 */
	/* |  */
	/* |         vsqrt    Q, vf7 */
	/* |  */
	/* |         qmfc2.ni a4, vf6 */
	/* |  */
	/* |         pcpyud   a5, a4, a4 */
	/* |  */
	/* |         sdl      a4, 0x7(%3) */
	/* |         sdr      a4,   0(%3) */
	/* |  */
	/* |         sw       a5, NJS_VECTOR.z(%3) */
	/* |  */
	/* |         vwaitq */
	/* |  */
	/* |         vaddq.x  vf8, vf0, Q */
	/* |       */
	/* |         qmfc2.ni v0, vf8 */
	/* |          */
	/* |         mtc1     v0, %0 */
	/* |     .set reorder */
	/* |     " : "=r"(ret) : "r"(v1), "r"(v2), "r"(ov) :  */
	/* |     ); */
	    ee_gpr r2 = {{0}}, r8 = {{0}}, r9 = {{0}}, r10 = {{0}}, r11 = {{0}};
	    __typeof__((v1) + 0) op0 = (v1);
	    __typeof__((v2) + 0) op1 = (v2);
	    __typeof__((ov) + 0) op2 = (ov);
	    r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x7)) - 7);
	    r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
	    r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, z)))));
	    r10.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x7)) - 7);
	    r10.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
	    r11.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + ((int)offsetof(NJS_VECTOR, z)))));
	    r8 = ee_pcpyld(r9, r8);
	    r10 = ee_pcpyld(r11, r10);
	    vu_qmtc2(4, r8);
	    vu_qmtc2(5, r10);
	    vu_opmula(VF(4), VF(5));
	    vu_opmsub(VF(6), VF(5), VF(4));
	    vu_mul(VF(7), VF(6), VF(6), 14);
	    vu_add_bc(VF(7), VF(7), VF(7)[1], 8);
	    vu_add_bc(VF(7), VF(7), VF(7)[2], 8);
	    VQ = sqrtf(fabsf(VF(7)[0]));
	    r8 = vu_qmfc2(6);
	    r9 = ee_pcpyud(r8, r8);
	    ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x7)) - 7, r8.d[0]);
	    ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0)), r8.d[0]);
	    *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + ((int)offsetof(NJS_VECTOR, z)))) = (uint32_t)(r9.d[0]);
	    vu_add_bc(VF(8), VF(0), VQ, 8);
	    r2 = vu_qmfc2(8);
	    EE_CVAR_SETF(ret, ee_bitsf((uint32_t)(r2.d[0])));
	}

    return ret;
}

// 100% matching! 
Float	njInnerProduct(NJS_VECTOR *v1, NJS_VECTOR *v2)
{
	float ret;

	{ /* translated from EE asm by agent mips2c; original kept below */
	/* |  */
	/* |     (" */
	/* |     .set noreorder */
	/* |         ldl      a4, 0x7(%1) */
	/* |         ldr      a4,   0(%1) */
	/* |       */
	/* |         lw       a5, NJS_VECTOR.z(%1)  */
	/* |  */
	/* |         ldl      a6, 0x7(%2) */
	/* |         ldr      a6,   0(%2) */
	/* |       */
	/* |         lw       a7, NJS_VECTOR.z(%2)  */
	/* |       */
	/* |         pcpyld   a4, a5, a4 */
	/* |         pcpyld   a6, a7, a6 */
	/* |       */
	/* |         qmtc2.ni a4, vf4 */
	/* |         qmtc2.ni a6, vf5 */
	/* |       */
	/* |         vmul.xyz vf6,  vf4, vf5 */
	/* |       */
	/* |         vaddy.x  vf6,  vf6, vf6 */
	/* |         vaddz.x  vf14, vf6, vf6 */
	/* |       */
	/* |         qmfc2.ni v0, vf14 */
	/* |          */
	/* |         mtc1     v0, %0 */
	/* |     .set reorder */
	/* |     " : "=r"(ret) : "r"(v1), "r"(v2) :  */
	/* |     ); */
	    ee_gpr r2 = {{0}}, r8 = {{0}}, r9 = {{0}}, r10 = {{0}}, r11 = {{0}};
	    __typeof__((v1) + 0) op0 = (v1);
	    __typeof__((v2) + 0) op1 = (v2);
	    r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x7)) - 7);
	    r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
	    r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, z)))));
	    r10.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x7)) - 7);
	    r10.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
	    r11.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + ((int)offsetof(NJS_VECTOR, z)))));
	    r8 = ee_pcpyld(r9, r8);
	    r10 = ee_pcpyld(r11, r10);
	    vu_qmtc2(4, r8);
	    vu_qmtc2(5, r10);
	    vu_mul(VF(6), VF(4), VF(5), 14);
	    vu_add_bc(VF(6), VF(6), VF(6)[1], 8);
	    vu_add_bc(VF(14), VF(6), VF(6)[2], 8);
	    r2 = vu_qmfc2(14);
	    EE_CVAR_SETF(ret, ee_bitsf((uint32_t)(r2.d[0])));
	}

    return ret;
}

// 100% matching! 
void njTranslateEx(NJS_VECTOR *v)
{
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         ldl         a4, 0x7(%0) */
    /* |         ldr         a4,   0(%0) */
    /* |  */
    /* |         lw          a5, NJS_VECTOR.z(%0)  */
    /* |  */
    /* |         pcpyld      a4, a5, a4 */
    /* |  */
    /* |         qmtc2.ni    a4, vf4 */
    /* |  */
    /* |         lqc2        vf28,    0(%1) */
    /* |         lqc2        vf29, 0x10(%1) */
    /* |         lqc2        vf30, 0x20(%1) */
    /* |         lqc2        vf31, 0x30(%1) */
    /* |  */
    /* |         vmulax.xyz  ACC,  vf28, vf4 */
    /* |          */
    /* |         vmadday.xyz ACC,  vf29, vf4 */
    /* |         vmaddaz.xyz ACC,  vf30, vf4 */
    /* |         vmaddw.xyz  vf31, vf31, vf0 */
    /* |      */
    /* |         sqc2        vf31, 0x30(%1) */
    /* |     .set reorder */
    /* |     " : : "r"(v), "r"(pNaMatMatrixStuckPtr) :  */
    /* |     ); */
        ee_gpr r8 = {{0}}, r9 = {{0}};
        __typeof__((v) + 0) op0 = (v);
        __typeof__((pNaMatMatrixStuckPtr) + 0) op1 = (pNaMatMatrixStuckPtr);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x7)) - 7);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + ((int)offsetof(NJS_VECTOR, z)))));
        r8 = ee_pcpyld(r9, r8);
        vu_qmtc2(4, r8);
        vu_lqc2(28, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
        vu_lqc2(29, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x10)));
        vu_lqc2(30, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x20)));
        vu_lqc2(31, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x30)));
        vu_mul_bc(VACC, VF(28), VF(4)[0], 14);
        vu_madd_bc(VACC, VF(29), VF(4)[1], 14);
        vu_madd_bc(VACC, VF(30), VF(4)[2], 14);
        vu_madd_bc(VF(31), VF(31), VF(0)[3], 14);
        vu_sqc2(31, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x30)));
    }
}

// 100% matching! 
void njRotateEx( Angle *ang, Sint32 lv )
{
	if (lv != 0) 
    {
        njRotateY(NULL, ang[1]);
        njRotateX(NULL, ang[0]);
        njRotateZ(NULL, ang[2]);
    }
    else
    {
        njRotateZ(NULL, ang[2]);
        njRotateY(NULL, ang[1]);
        njRotateX(NULL, ang[0]);
    }
}

// 100% matching! 
void njScaleEx(NJS_VECTOR *v)
{
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         lqc2     vf28,    0(%1) */
    /* |         lqc2     vf29, 0x10(%1) */
    /* |         lqc2     vf30, 0x20(%1) */
    /* |          */
    /* |         ldl      a4, 0x7(%0) */
    /* |         ldr      a4,   0(%0) */
    /* |          */
    /* |         lw       a5, NJS_VECTOR.z(%0)  */
    /* |          */
    /* |         pcpyld   a4, a5, a4 */
    /* |          */
    /* |         qmtc2.ni a4, vf4 */
    /* |          */
    /* |         vmulx    vf28, vf28, vf4 */
    /* |         vmuly    vf29, vf29, vf4 */
    /* |         vmulz    vf30, vf30, vf4 */
    /* |          */
    /* |         sqc2     vf28,    0(%1) */
    /* |         sqc2     vf29, 0x10(%1) */
    /* |         sqc2     vf30, 0x20(%1) */
    /* |     .set reorder */
    /* |     " : : "r"(v), "r"(pNaMatMatrixStuckPtr) :  */
    /* |     ); */
        ee_gpr r8 = {{0}}, r9 = {{0}};
        __typeof__((pNaMatMatrixStuckPtr) + 0) op0 = (pNaMatMatrixStuckPtr);
        __typeof__((v) + 0) op1 = (v);
        vu_lqc2(28, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        vu_lqc2(29, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x10)));
        vu_lqc2(30, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20)));
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x7)) - 7);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + ((int)offsetof(NJS_VECTOR, z)))));
        r8 = ee_pcpyld(r9, r8);
        vu_qmtc2(4, r8);
        vu_mul_bc(VF(28), VF(28), VF(4)[0], 15);
        vu_mul_bc(VF(29), VF(29), VF(4)[1], 15);
        vu_mul_bc(VF(30), VF(30), VF(4)[2], 15);
        vu_sqc2(28, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        vu_sqc2(29, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x10)));
        vu_sqc2(30, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20)));
    }
}

// 100% matching!
Bool njPushMatrixEx( void )
{
    njPushMatrix(NULL);
    return 0; /* fell off the end on the EE */
}

// 100% matching!
Bool njPopMatrixEx( void )
{
    njPopMatrix(1);
    return 0; /* fell off the end on the EE */
}

// 100% matching! 
void njRotTransPers(NJS_POINT3* pPoint, NJS_SCRVECTOR* pScreen)
{
	njMulMatrixCN(&NaViewScreenMatrix, NULL);

    njCalcPointCN(pPoint, (NJS_POINT3*)&pScreen->x);

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         vdiv     Q, vf0w, vf18z */
    /* |  */
    /* |         mfc1     t0, %2 */
    /* |  */
    /* |         lw       t1, fNaViwOffsetX */
    /* |         lw       t2, fNaViwOffsetY */
    /* |  */
    /* |         qmtc2    t0, vf4 */
    /* |         qmtc2    t1, vf5 */
    /* |         qmtc2    t2, vf6 */
    /* |  */
    /* |         vwaitq */
    /* |  */
    /* |         vmulq.x  vf8,  vf4,  Q */
    /* |  */
    /* |         vaddq.z  vf14, vf0,  Q */
    /* |  */
    /* |         vmulx.xy vf8,  vf18, vf8 */
    /* |          */
    /* |         vaddx.x  vf14, vf8,  vf5 */
    /* |         vaddx.y  vf14, vf8,  vf6 */
    /* |  */
    /* |         qmfc2    a6, vf14 */
    /* |      */
    /* |         pcpyud   a7, a6, a6 */
    /* |      */
    /* |         sdl      a6, 7(%1) */
    /* |         sdr      a6, 0(%1) */
    /* |          */
    /* |         sw       a7, NJS_SCRVECTOR.iz(%1) */
    /* |     .set reorder */
    /* |     " : : "r"(pPoint), "r"(pScreen), "f"(_nj_screen_.dist) :  */
    /* |     ); */
        ee_gpr r8 = {{0}}, r9 = {{0}}, r10 = {{0}}, r11 = {{0}};
        __typeof__((_nj_screen_.dist) + 0) op0 = (_nj_screen_.dist);
        __typeof__((pScreen) + 0) op1 = (pScreen);
        VQ = vu_div(VF(0)[3], VF(18)[2]);
        r8.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op0)));
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)&fNaViwOffsetX)));
        r10.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)&fNaViwOffsetY)));
        vu_qmtc2(4, r8);
        vu_qmtc2(5, r9);
        vu_qmtc2(6, r10);
        vu_mul_bc(VF(8), VF(4), VQ, 8);
        vu_add_bc(VF(14), VF(0), VQ, 2);
        vu_mul_bc(VF(8), VF(18), VF(8)[0], 12);
        vu_add_bc(VF(14), VF(8), VF(5)[0], 8);
        vu_add_bc(VF(14), VF(8), VF(6)[0], 4);
        r10 = vu_qmfc2(14);
        r11 = ee_pcpyud(r10, r10);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (7)) - 7, r10.d[0]);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)), r10.d[0]);
        *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + ((int)offsetof(NJS_SCRVECTOR, iz)))) = (uint32_t)(r11.d[0]);
    }

    pScreen->fog = njCalcFogPower(pScreen->z);
}

// 100% matching!
void njRotTrans(NJS_POINT3* pPoint, NJS_POINT3* pOut)
{
	njMulMatrixCN(&NaViewScreenMatrix, NULL);
    
    njCalcPointCN(pPoint, pOut);
}

// 100% matching!
void njPers(NJS_SCRVECTOR* pScreen)
{
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         ldl      t0, 7(%0) */
    /* |         ldr      t0, 0(%0) */
    /* |          */
    /* |         lw       t1, 8(%0) */
    /* |          */
    /* |         pcpyld   t0, t1, t0 */
    /* |  */
    /* |         qmtc2    t0, vf18 */
    /* |  */
    /* |         vdiv     Q, vf0w, vf18z */
    /* |  */
    /* |         mfc1     t0, %1 */
    /* |  */
    /* |         lw       t1, fNaViwOffsetX */
    /* |         lw       t2, fNaViwOffsetY */
    /* |  */
    /* |         qmtc2    t0, vf4 */
    /* |         qmtc2    t1, vf5 */
    /* |         qmtc2    t2, vf6 */
    /* |  */
    /* |         vwaitq */
    /* |  */
    /* |         vmulq.x  vf8, vf4, Q */
    /* |  */
    /* |         vaddq.z  vf14, vf0, Q */
    /* |  */
    /* |         vmulx.xy vf8, vf18, vf8 */
    /* |          */
    /* |         vaddx.x  vf14, vf8, vf5 */
    /* |         vaddx.y  vf14, vf8, vf6 */
    /* |  */
    /* |         qmfc2    t2, vf14 */
    /* |      */
    /* |         pcpyud   t3, t2, t2 */
    /* |      */
    /* |         sdl      t2, 7(%0) */
    /* |         sdr      t2, 0(%0) */
    /* |          */
    /* |         sw       t3, 12(%0) */
    /* |     .set reorder */
    /* |     " : : "r"(pScreen), "f"(_nj_screen_.dist) :  */
    /* |     ); */
        ee_gpr r8 = {{0}}, r9 = {{0}}, r10 = {{0}}, r11 = {{0}};
        __typeof__((pScreen) + 0) op0 = (pScreen);
        __typeof__((_nj_screen_.dist) + 0) op1 = (_nj_screen_.dist);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (7)) - 7);
        r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (8))));
        r8 = ee_pcpyld(r9, r8);
        vu_qmtc2(18, r8);
        VQ = vu_div(VF(0)[3], VF(18)[2]);
        r8.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op1)));
        r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)&fNaViwOffsetX)));
        r10.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)&fNaViwOffsetY)));
        vu_qmtc2(4, r8);
        vu_qmtc2(5, r9);
        vu_qmtc2(6, r10);
        vu_mul_bc(VF(8), VF(4), VQ, 8);
        vu_add_bc(VF(14), VF(0), VQ, 2);
        vu_mul_bc(VF(8), VF(18), VF(8)[0], 12);
        vu_add_bc(VF(14), VF(8), VF(5)[0], 8);
        vu_add_bc(VF(14), VF(8), VF(6)[0], 4);
        r10 = vu_qmfc2(14);
        r11 = ee_pcpyud(r10, r10);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (7)) - 7, r10.d[0]);
        ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)), r10.d[0]);
        *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (12))) = (uint32_t)(r11.d[0]);
    }
}

// 100% matching!
void njCopyMatrix(NJS_MATRIX* pDstMat, NJS_MATRIX* pSrcMat) 
{
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         lqc2 vf4, 0(%1)  */
    /* |         lqc2 vf5, 0x10(%1)  */
    /* |         lqc2 vf6, 0x20(%1)  */
    /* |         lqc2 vf7, 0x30(%1)  */
    /* |      */
    /* |         sqc2 vf4, 0(%0)  */
    /* |         sqc2 vf5, 0x10(%0)  */
    /* |         sqc2 vf6, 0x20(%0)  */
    /* |         sqc2 vf7, 0x30(%0)  */
    /* |     .set reorder */
    /* |     " : : "r"(pDstMat), "r"(pSrcMat) :  */
    /* |     ); */
        __typeof__((pSrcMat) + 0) op0 = (pSrcMat);
        __typeof__((pDstMat) + 0) op1 = (pDstMat);
        vu_lqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x10)));
        vu_lqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20)));
        vu_lqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x30)));
        vu_sqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
        vu_sqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x10)));
        vu_sqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x20)));
        vu_sqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x30)));
    }
}

// 100% matching!
void njMulMatrixCN(NJS_MATRIX* pSrcMat1, NJS_MATRIX* pSrcMat2)
{
    if (pSrcMat2 == NULL)
    {
        pSrcMat2 = pNaMatMatrixStuckPtr;
    }

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         lqc2         vf4,  0(%0) */
    /* |         lqc2         vf5,  0x10(%0) */
    /* |         lqc2         vf6,  0x20(%0) */
    /* |         lqc2         vf7,  0x30(%0) */
    /* |         lqc2         vf8,  0(%1) */
    /* |         lqc2         vf9,  0x10(%1) */
    /* |         lqc2         vf10, 0x20(%1) */
    /* |         lqc2         vf11, 0x30(%1) */
    /* |  */
    /* |         vmulax.xyzw  ACC,  vf4, vf8 */
    /* |          */
    /* |         vmadday.xyzw ACC,  vf5, vf8 */
    /* |         vmaddz.xyzw  vf28, vf6, vf8 */
    /* |          */
    /* |         vmulax.xyzw  ACC,  vf4, vf9 */
    /* |  */
    /* |         vmadday.xyzw ACC,  vf5, vf9 */
    /* |         vmaddz.xyzw  vf29, vf6, vf9 */
    /* |          */
    /* |         vmulax.xyzw  ACC,  vf4, vf10 */
    /* |  */
    /* |         vmadday.xyzw ACC,  vf5, vf10 */
    /* |         vmaddz.xyzw  vf30, vf6, vf10 */
    /* |          */
    /* |         vmulax.xyzw  ACC,  vf4, vf11 */
    /* |  */
    /* |         vmadday.xyzw ACC,  vf5, vf11 */
    /* |         vmaddaz.xyzw ACC,  vf6, vf11 */
    /* |         vmaddw.xyzw  vf31, vf7, vf0 */
    /* |     .set reorder */
    /* |     " : : "r"(pSrcMat1), "r"(pSrcMat2) :  */
    /* |     ); */
        __typeof__((pSrcMat1) + 0) op0 = (pSrcMat1);
        __typeof__((pSrcMat2) + 0) op1 = (pSrcMat2);
        vu_lqc2(4, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
        vu_lqc2(5, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x10)));
        vu_lqc2(6, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x20)));
        vu_lqc2(7, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x30)));
        vu_lqc2(8, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0)));
        vu_lqc2(9, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x10)));
        vu_lqc2(10, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x20)));
        vu_lqc2(11, ((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0x30)));
        vu_mul_bc(VACC, VF(4), VF(8)[0], 15);
        vu_madd_bc(VACC, VF(5), VF(8)[1], 15);
        vu_madd_bc(VF(28), VF(6), VF(8)[2], 15);
        vu_mul_bc(VACC, VF(4), VF(9)[0], 15);
        vu_madd_bc(VACC, VF(5), VF(9)[1], 15);
        vu_madd_bc(VF(29), VF(6), VF(9)[2], 15);
        vu_mul_bc(VACC, VF(4), VF(10)[0], 15);
        vu_madd_bc(VACC, VF(5), VF(10)[1], 15);
        vu_madd_bc(VF(30), VF(6), VF(10)[2], 15);
        vu_mul_bc(VACC, VF(4), VF(11)[0], 15);
        vu_madd_bc(VACC, VF(5), VF(11)[1], 15);
        vu_madd_bc(VACC, VF(6), VF(11)[2], 15);
        vu_madd_bc(VF(31), VF(7), VF(0)[3], 15);
    }
}
