#include "../../../ps2/veronica/prog/ps2_NaMath.h"
#include "../../../ps2/veronica/prog/main.h"

static float SinTable[16384];

// 100% matching! 
void _Make_SinTable()
{ 
    int i;

    for (i = 0; i < 16384; i++) 
    { 
        SinTable[i] = sinf(0.0000958738f * i); 
    } 
}  

// 100% matching!
Float	njSin(Angle n)
{
    float ret;
    
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         move  v0, %0 */
    /* |          */
    /* |         lw    t0, 0(%1) */
    /* |          */
    /* |         addi  t2, zero, 16383 */
    /* |      */
    /* |         andi  t3, t0, 0x4000 */
    /* |          */
    /* |         beqz  t3, l_002D7D08 */
    /* |          */
    /* |         and   t1, t0, t2 */
    /* |      */
    /* |         sub   t2, t2, t1 */
    /* |      */
    /* |         muli  t2, t2, 4 */
    /* |      */
    /* |         add   t2, t2, %2 */
    /* |          */
    /* |         lwc1  f4, 0(t2) */
    /* |          */
    /* |         neg.s f6, f4 */
    /* |          */
    /* |         mfc1  %2, f6 */
    /* |          */
    /* |         andi  t0, t0, 0x8000 */
    /* |      */
    /* |         mfc1  t6, f4 */
    /* |          */
    /* |         b     l_002D7D30 */
    /* |      */
    /* |         movz  %2, t6, t0 */
    /* |  */
    /* |         l_002D7D08: */
    /* |         muli  t1, t1, 4 */
    /* |          */
    /* |         add   t1, t1, %2 */
    /* |          */
    /* |         lwc1  f4, 0(t1) */
    /* |          */
    /* |         neg.s f6, f4 */
    /* |          */
    /* |         mfc1  %2, f4 */
    /* |         mfc1  t6, f6 */
    /* |      */
    /* |         andi  t0, t0, 0x8000 */
    /* |          */
    /* |         movn  %2, t6, t0 */
    /* |  */
    /* |         l_002D7D30: */
    /* |         mtc1  %2, f0 */
    /* |     .set reorder */
    /* |     " : "=r"(ret) : "r"(&n), "r"(SinTable) :  */
    /* |     ); */
        ee_gpr r2 = {{0}}, r8 = {{0}}, r9 = {{0}}, r10 = {{0}}, r11 = {{0}}, r14 = {{0}};
        float f0 = 0, f4 = 0, f6 = 0;
        __typeof__((&n) + 0) op0 = (&n);
        __typeof__((SinTable) + 0) op1 = (SinTable);
        r2.d[0] = EE_CVAR_GET(ret);
        r8.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0))));
        r10.d[0] = EE_SEXT32((uint32_t)(0) + (uint32_t)(16383));
        r11.d[0] = (r8.d[0]) & (uint64_t)(uint16_t)(0x4000);
        { int c_ = ((int64_t)(r11.d[0]) == 0); r9.d[0] = (r8.d[0]) & r10.d[0]; if (c_) goto L_njSin_l_002D7D08; }
        r10.d[0] = EE_SEXT32((uint32_t)(r10.d[0]) - (uint32_t)(r9.d[0]));
        r10.d[0] = EE_SEXT32((uint32_t)((int32_t)(r10.d[0]) * (int32_t)(4)));
        r10.d[0] = EE_SEXT32((uint32_t)(r10.d[0]) + (uint32_t)(EE_CVAR_GET(op1)));
        f4 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(r10.d[0]) + (0))));
        f6 = -f4;
        EE_CVAR_SET(op1, EE_SEXT32(ee_fbits(f6)));
        r8.d[0] = (r8.d[0]) & (uint64_t)(uint16_t)(0x8000);
        r14.d[0] = EE_SEXT32(ee_fbits(f4));
        if ((r8.d[0]) == 0) { EE_CVAR_SET(op1, r14.d[0]); } goto L_njSin_l_002D7D30;
        L_njSin_l_002D7D08:;
        r9.d[0] = EE_SEXT32((uint32_t)((int32_t)(r9.d[0]) * (int32_t)(4)));
        r9.d[0] = EE_SEXT32((uint32_t)(r9.d[0]) + (uint32_t)(EE_CVAR_GET(op1)));
        f4 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(r9.d[0]) + (0))));
        f6 = -f4;
        EE_CVAR_SET(op1, EE_SEXT32(ee_fbits(f4)));
        r14.d[0] = EE_SEXT32(ee_fbits(f6));
        r8.d[0] = (r8.d[0]) & (uint64_t)(uint16_t)(0x8000);
        if ((r8.d[0]) != 0) { EE_CVAR_SET(op1, r14.d[0]); }
        L_njSin_l_002D7D30:;
        f0 = ee_bitsf((uint32_t)(EE_CVAR_GET(op1)));
        L1_ret:;
        return f0;
    }
}

// 100% matching!
Float	njCos(Angle n)
{
	float ret;

	{ /* translated from EE asm by agent mips2c; original kept below */
	/* |  */
	/* |     (" */
	/* |     .set noreorder */
	/* |         move  v0, %0 */
	/* |          */
	/* |         lw    t0, 0(%1) */
	/* |          */
	/* |         addi  t2, zero, 16383 */
	/* |      */
	/* |         andi  t3, t0, 0x4000 */
	/* |          */
	/* |         beqz  t3, l_002D7D94 */
	/* |          */
	/* |         and   t1, t0, t2 */
	/* |      */
	/* |         muli  t1, t1, 4 */
	/* |      */
	/* |         add   t1, t1, %2 */
	/* |          */
	/* |         lwc1  f5, 0(t1) */
	/* |          */
	/* |         neg.s f7, f5 */
	/* |          */
	/* |         mfc1  %2, f5 */
	/* | 		mfc1  t7, f7 */
	/* |          */
	/* |         andi  t0, t0, 0x8000 */
	/* |  */
	/* |         b     l_002D7DC0 */
	/* |  */
	/* |         movz  %2, t7, t0 */
	/* |  */
	/* |         l_002D7D94: */
	/* | 		sub   t2, t2, t1 */
	/* |  */
	/* |         muli  t2, t2, 4 */
	/* |          */
	/* |         add   t2, t2, %2 */
	/* |          */
	/* |         lwc1  f5, 0(t2) */
	/* |          */
	/* |         neg.s f7, f5 */
	/* |          */
	/* |         mfc1  %2, f5 */
	/* |         mfc1  t7, f7 */
	/* |      */
	/* |         andi  t0, t0, 0x8000 */
	/* |          */
	/* |         movn  %2, t7, t0 */
	/* |  */
	/* |         l_002D7DC0: */
	/* |         mtc1  %2, f0 */
	/* |     .set reorder */
	/* |     " : "=r"(ret) : "r"(&n), "r"(SinTable) :  */
	/* |     ); */
	    ee_gpr r2 = {{0}}, r8 = {{0}}, r9 = {{0}}, r10 = {{0}}, r11 = {{0}}, r15 = {{0}};
	    float f0 = 0, f5 = 0, f7 = 0;
	    __typeof__((&n) + 0) op0 = (&n);
	    __typeof__((SinTable) + 0) op1 = (SinTable);
	    r2.d[0] = EE_CVAR_GET(ret);
	    r8.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0))));
	    r10.d[0] = EE_SEXT32((uint32_t)(0) + (uint32_t)(16383));
	    r11.d[0] = (r8.d[0]) & (uint64_t)(uint16_t)(0x4000);
	    { int c_ = ((int64_t)(r11.d[0]) == 0); r9.d[0] = (r8.d[0]) & r10.d[0]; if (c_) goto L_njCos_l_002D7D94; }
	    r9.d[0] = EE_SEXT32((uint32_t)((int32_t)(r9.d[0]) * (int32_t)(4)));
	    r9.d[0] = EE_SEXT32((uint32_t)(r9.d[0]) + (uint32_t)(EE_CVAR_GET(op1)));
	    f5 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(r9.d[0]) + (0))));
	    f7 = -f5;
	    EE_CVAR_SET(op1, EE_SEXT32(ee_fbits(f5)));
	    r15.d[0] = EE_SEXT32(ee_fbits(f7));
	    r8.d[0] = (r8.d[0]) & (uint64_t)(uint16_t)(0x8000);
	    if ((r8.d[0]) == 0) { EE_CVAR_SET(op1, r15.d[0]); } goto L_njCos_l_002D7DC0;
	    L_njCos_l_002D7D94:;
	    r10.d[0] = EE_SEXT32((uint32_t)(r10.d[0]) - (uint32_t)(r9.d[0]));
	    r10.d[0] = EE_SEXT32((uint32_t)((int32_t)(r10.d[0]) * (int32_t)(4)));
	    r10.d[0] = EE_SEXT32((uint32_t)(r10.d[0]) + (uint32_t)(EE_CVAR_GET(op1)));
	    f5 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(r10.d[0]) + (0))));
	    f7 = -f5;
	    EE_CVAR_SET(op1, EE_SEXT32(ee_fbits(f5)));
	    r15.d[0] = EE_SEXT32(ee_fbits(f7));
	    r8.d[0] = (r8.d[0]) & (uint64_t)(uint16_t)(0x8000);
	    if ((r8.d[0]) != 0) { EE_CVAR_SET(op1, r15.d[0]); }
	    L_njCos_l_002D7DC0:;
	    f0 = ee_bitsf((uint32_t)(EE_CVAR_GET(op1)));
	    L2_ret:;
	    return f0;
	}
}

// 100% matching!
void njSinCos(int lAngle, float* sin, float* cos)
{
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |     .set noreorder */
    /* |         move  v1, %3  */
    /* |          */
    /* |         lw    t0, 0(%0)  */
    /* |          */
    /* |         addi  t2, zero, 16383  */
    /* |      */
    /* |         andi  t3, t0, 0x4000  */
    /* |          */
    /* |         beqz  t3, l_002D7E4C  */
    /* |          */
    /* |         and   t1, t0, t2 */
    /* |      */
    /* |         sub   t2, t2, t1  */
    /* |      */
    /* |         muli  t1, t1, 4  */
    /* |         muli  t2, t2, 4  */
    /* |          */
    /* |         add   t1, t1, v1  */
    /* |         add   t2, t2, v1  */
    /* |      */
    /* |         lwc1  f4, 0(t2)  */
    /* |         lwc1  f5, 0(t1)  */
    /* |          */
    /* |         neg.s f6, f4  */
    /* |         neg.s f7, f5 */
    /* |          */
    /* |         mfc1  t4, f6 */
    /* |         mfc1  t5, f5 */
    /* |         mfc1  t6, f4 */
    /* |         mfc1  t7, f7 */
    /* |      */
    /* |         andi  t0, t0, 0x8000 */
    /* |      */
    /* |         movz  t4, t6, t0 */
    /* |          */
    /* |         b     l_002D7E9C */
    /* |      */
    /* |         movz  t5, t7, t0 */
    /* |      */
    /* |         l_002D7E4C: */
    /* |         sub   t2, t2, t1  */
    /* |          */
    /* |         muli  t1, t1, 4 */
    /* |         muli  t2, t2, 4 */
    /* |          */
    /* |         add   t1, t1, v1  */
    /* |         add   t2, t2, v1  */
    /* |      */
    /* |         lwc1  f4, 0(t1) */
    /* |         lwc1  f5, 0(t2) */
    /* |          */
    /* |         neg.s f6, f4 */
    /* |         neg.s f7, f5 */
    /* |          */
    /* |         mfc1  t4, f4 */
    /* |         mfc1  t5, f5 */
    /* |         mfc1  t6, f6 */
    /* |         mfc1  t7, f7 */
    /* |      */
    /* |         andi  t0, t0, 0x8000 */
    /* |      */
    /* |         movn  t4, t6, t0 */
    /* |         movn  t5, t7, t0 */
    /* |      */
    /* |         l_002D7E9C: */
    /* |         sw    t4, 0(%1) */
    /* |         sw    t5, 0(%2) */
    /* |          */
    /* |         qmtc2 t4, vf11 */
    /* |         qmtc2 t5, vf12 */
    /* |     .set reorder */
    /* |     " : : "r"(&lAngle), "r"(sin), "r"(cos), "r"(SinTable) :  */
    /* |     ); */
        ee_gpr r3 = {{0}}, r8 = {{0}}, r9 = {{0}}, r10 = {{0}}, r11 = {{0}}, r12 = {{0}}, r13 = {{0}}, r14 = {{0}}, r15 = {{0}};
        float f4 = 0, f5 = 0, f6 = 0, f7 = 0;
        __typeof__((SinTable) + 0) op0 = (SinTable);
        __typeof__((&lAngle) + 0) op1 = (&lAngle);
        __typeof__((sin) + 0) op2 = (sin);
        __typeof__((cos) + 0) op3 = (cos);
        r3.d[0] = EE_CVAR_GET(op0);
        r8.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op1)) + (0))));
        r10.d[0] = EE_SEXT32((uint32_t)(0) + (uint32_t)(16383));
        r11.d[0] = (r8.d[0]) & (uint64_t)(uint16_t)(0x4000);
        { int c_ = ((int64_t)(r11.d[0]) == 0); r9.d[0] = (r8.d[0]) & r10.d[0]; if (c_) goto L_njSinCos_l_002D7E4C; }
        r10.d[0] = EE_SEXT32((uint32_t)(r10.d[0]) - (uint32_t)(r9.d[0]));
        r9.d[0] = EE_SEXT32((uint32_t)((int32_t)(r9.d[0]) * (int32_t)(4)));
        r10.d[0] = EE_SEXT32((uint32_t)((int32_t)(r10.d[0]) * (int32_t)(4)));
        r9.d[0] = EE_SEXT32((uint32_t)(r9.d[0]) + (uint32_t)(r3.d[0]));
        r10.d[0] = EE_SEXT32((uint32_t)(r10.d[0]) + (uint32_t)(r3.d[0]));
        f4 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(r10.d[0]) + (0))));
        f5 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(r9.d[0]) + (0))));
        f6 = -f4;
        f7 = -f5;
        r12.d[0] = EE_SEXT32(ee_fbits(f6));
        r13.d[0] = EE_SEXT32(ee_fbits(f5));
        r14.d[0] = EE_SEXT32(ee_fbits(f4));
        r15.d[0] = EE_SEXT32(ee_fbits(f7));
        r8.d[0] = (r8.d[0]) & (uint64_t)(uint16_t)(0x8000);
        if ((r8.d[0]) == 0) { r12.d[0] = r14.d[0]; }
        if ((r8.d[0]) == 0) { r13.d[0] = r15.d[0]; } goto L_njSinCos_l_002D7E9C;
        L_njSinCos_l_002D7E4C:;
        r10.d[0] = EE_SEXT32((uint32_t)(r10.d[0]) - (uint32_t)(r9.d[0]));
        r9.d[0] = EE_SEXT32((uint32_t)((int32_t)(r9.d[0]) * (int32_t)(4)));
        r10.d[0] = EE_SEXT32((uint32_t)((int32_t)(r10.d[0]) * (int32_t)(4)));
        r9.d[0] = EE_SEXT32((uint32_t)(r9.d[0]) + (uint32_t)(r3.d[0]));
        r10.d[0] = EE_SEXT32((uint32_t)(r10.d[0]) + (uint32_t)(r3.d[0]));
        f4 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(r9.d[0]) + (0))));
        f5 = ee_bitsf(*(uint32_t *)(((uintptr_t)(uint32_t)(r10.d[0]) + (0))));
        f6 = -f4;
        f7 = -f5;
        r12.d[0] = EE_SEXT32(ee_fbits(f4));
        r13.d[0] = EE_SEXT32(ee_fbits(f5));
        r14.d[0] = EE_SEXT32(ee_fbits(f6));
        r15.d[0] = EE_SEXT32(ee_fbits(f7));
        r8.d[0] = (r8.d[0]) & (uint64_t)(uint16_t)(0x8000);
        if ((r8.d[0]) != 0) { r12.d[0] = r14.d[0]; }
        if ((r8.d[0]) != 0) { r13.d[0] = r15.d[0]; }
        L_njSinCos_l_002D7E9C:;
        *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0))) = (uint32_t)(r12.d[0]);
        *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0))) = (uint32_t)(r13.d[0]);
        vu_qmtc2(11, r12);
        vu_qmtc2(12, r13);
    }
}

// 100% matching! 
Float	njFraction  (Float n) 
{ 
    return n - floorf(n); 
} 

// 100% matching! 
Float	njSqrt(Float n)
{
	float ret;

	ret = 0;
    
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |         mfc1    t0, f12 */
    /* |  */
    /* |         qmtc2   t0, vf8 */
    /* |      */
    /* |         vsqrt   Q,  vf8 */
    /* |  */
    /* |         vwaitq  */
    /* |  */
    /* |         vaddq.x vf8, vf0, Q */
    /* |          */
    /* |         qmfc2   v0,  vf8 */
    /* |      */
    /* |         mtc1    v0, %0 */
    /* |     " : "=f"(ret) : :  */
    /* |     ); */
        ee_gpr r2 = {{0}}, r8 = {{0}};
        float f12 = 0;
        f12 = n;
        r8.d[0] = EE_SEXT32(ee_fbits(f12));
        vu_qmtc2(8, r8);
        VQ = sqrtf(fabsf(VF(8)[0]));
        vu_add_bc(VF(8), VF(0), VQ, 8);
        r2 = vu_qmfc2(8);
        EE_CVAR_SETF(ret, ee_bitsf((uint32_t)(r2.d[0])));
    }

    return ret;
}

// 100% matching! 
Float	njInvertSqrt(Float n)
{
    float ret;

    ret = 0;
    
    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     (" */
    /* |         mfc1    t0, f12 */
    /* |  */
    /* |         qmtc2   t0, vf8 */
    /* |      */
    /* |         vrsqrt  Q,  vf0w, vf8 */
    /* |  */
    /* |         vwaitq  */
    /* |  */
    /* |         vaddq.x vf8, vf0, Q */
    /* |          */
    /* |         qmfc2   v0,  vf8 */
    /* |      */
    /* |         mtc1    v0, %0 */
    /* |     " : "=f"(ret) : :  */
    /* |     ); */
        ee_gpr r2 = {{0}}, r8 = {{0}};
        float f12 = 0;
        f12 = n;
        r8.d[0] = EE_SEXT32(ee_fbits(f12));
        vu_qmtc2(8, r8);
        VQ = vu_rsqrt(VF(0)[3], VF(8)[0]);
        vu_add_bc(VF(8), VF(0), VQ, 8);
        r2 = vu_qmfc2(8);
        EE_CVAR_SETF(ret, ee_bitsf((uint32_t)(r2.d[0])));
    }

    return ret;
}

// 100% matching! 
void	njLinear(Float *idata, Float *odata, NJS_SPLINE *attr, Float frame)
{
	{ /* translated from EE asm by agent mips2c; original kept below */
	/* |  */
	/* |     (" */
	/* | 	    addi      a4, %0, 12 */
	/* |  */
	/* | 		mfc1      a5, %3 */
	/* |  */
	/* |         ldl       a6, 0x7(%0) */
	/* |         ldr       a6,   0(%0) */
	/* |       */
	/* |         lw        a7,   8(%0)  */
	/* |  */
	/* |         ldl       t4, 0x7(a4) */
	/* |         ldr       t4,   0(a4) */
	/* |       */
	/* |         lw        t5,   8(a4)  */
	/* |       */
	/* |         pcpyld    a6, a7, a6 */
	/* |         pcpyld    t4, t5, t4 */
	/* |       */
	/* |         qmtc2.ni  a6, vf10 */
	/* |         qmtc2.ni  t4, vf11 */
	/* | 		qmtc2.ni  a5, vf9 */
	/* | 		 */
	/* | 		vsub.xyz  vf12, vf11, vf10 */
	/* |  */
	/* | 		vmulx.xyz vf12, vf12, vf9 */
	/* |  */
	/* | 		vadd.xyz  vf4,  vf12, vf10 */
	/* |  */
	/* | 		qmfc2.ni  a6, vf4 */
	/* |  */
	/* | 		pcpyud    a7, a6, a6 */
	/* |  */
	/* | 		sdl       a6, 0x7(%1) */
	/* |         sdr       a6,   0(%1) */
	/* |       */
	/* |         sw        a7,   8(%1)  */
	/* |     " : : "r"(idata), "r"(odata), "r"(attr), "f"(frame) :  */
	/* |     ); */
	    ee_gpr r8 = {{0}}, r9 = {{0}}, r10 = {{0}}, r11 = {{0}}, r12 = {{0}}, r13 = {{0}};
	    __typeof__((idata) + 0) op0 = (idata);
	    __typeof__((frame) + 0) op1 = (frame);
	    __typeof__((odata) + 0) op2 = (odata);
	    r8.d[0] = EE_SEXT32((uint32_t)(EE_CVAR_GET(op0)) + (uint32_t)(12));
	    r9.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op1)));
	    r10.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0x7)) - 7);
	    r10.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (0)));
	    r11.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op0)) + (8))));
	    r12.d[0] = ee_ld64(((uintptr_t)(uint32_t)(r8.d[0]) + (0x7)) - 7);
	    r12.d[0] = ee_ld64(((uintptr_t)(uint32_t)(r8.d[0]) + (0)));
	    r13.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(r8.d[0]) + (8))));
	    r10 = ee_pcpyld(r11, r10);
	    r12 = ee_pcpyld(r13, r12);
	    vu_qmtc2(10, r10);
	    vu_qmtc2(11, r12);
	    vu_qmtc2(9, r9);
	    vu_sub(VF(12), VF(11), VF(10), 14);
	    vu_mul_bc(VF(12), VF(12), VF(9)[0], 14);
	    vu_add(VF(4), VF(12), VF(10), 14);
	    r10 = vu_qmfc2(4);
	    r11 = ee_pcpyud(r10, r10);
	    ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x7)) - 7, r10.d[0]);
	    ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0)), r10.d[0]);
	    *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (8))) = (uint32_t)(r11.d[0]);
	}
}

// 100% matching! 
void	njOverhauserSpline(Float *idata, Float *odata, NJS_SPLINE *attr, Float frame)
{
	float ftmp; 

	ftmp = 0.5f;

	{ /* translated from EE asm by agent mips2c; original kept below */
	/* |  */
	/* |     (" */
	/* | 		mfc1      a4, %3 */
	/* |  */
	/* | 		vaddw.x   vf12,  vf0, vf0 */
	/* | 		vaddw.x   vf12, vf12, vf0 */
	/* |  */
	/* | 		qmtc2.ni  a4, vf4 */
	/* |  */
	/* | 		vmul.x    vf5,  vf4, vf4 */
	/* | 		vmul.x    vf6,  vf5, vf4 */
	/* |  */
	/* | 		vadd.x    vf8,  vf5, vf5 */
	/* |  */
	/* | 		vsub.x    vf8,  vf8, vf6 */
	/* | 		vsub.x    vf8,  vf8, vf4 */
	/* |  */
	/* | 		vmulz.x   vf10, vf5, vf2 */
	/* | 		vmul.x    vf9,  vf6, vf2 */
	/* |  */
	/* | 		vsub.x    vf9,  vf9, vf10 */
	/* |  */
	/* | 		vadd.x    vf9,  vf9, vf12 */
	/* |  */
	/* | 		vsub.x    vf7,  vf0, vf2 */
	/* |  */
	/* | 		vmulx.x   vf7,  vf6, vf7 */
	/* | 		vmuly.x   vf10, vf5, vf2 */
	/* |  */
	/* | 		vadd.x    vf10, vf10, vf4 */
	/* | 		vadd.x    vf10, vf10, vf7 */
	/* |  */
	/* | 		vsub.x    vf11, vf6, vf5 */
	/* |  */
	/* | 		mfc1      a5, %4 */
	/* | 		nop */
	/* |  */
	/* | 		qmtc2.ni  a5, vf12 */
	/* |  */
	/* |         ldl       a4,  0x7(%0) */
	/* |         ldr       a4,    0(%0) */
	/* |       */
	/* |         lw        a5,    8(%0)  */
	/* |  */
	/* |         ldl       a6, 0x13(%0) */
	/* |         ldr       a6,  0xC(%0) */
	/* |       */
	/* |         lw        a7,   20(%0)  */
	/* |  */
	/* | 		ldl       t4, 0x1F(%0) */
	/* |         ldr       t4, 0x18(%0) */
	/* |       */
	/* |         lw        t5,   32(%0)  */
	/* |  */
	/* |         ldl       t6, 0x2B(%0) */
	/* |         ldr       t6, 0x24(%0) */
	/* |       */
	/* |         lw        t7,   44(%0)  */
	/* |       */
	/* | 		pcpyld    a4, a5, a4 */
	/* | 		pcpyld    a6, a7, a6 */
	/* |         pcpyld    t4, t5, t4 */
	/* |         pcpyld    t6, t7, t6 */
	/* |       */
	/* | 		qmtc2.ni  a4, vf4 */
	/* | 		qmtc2.ni  a6, vf5 */
	/* | 		qmtc2.ni  t4, vf6 */
	/* | 		qmtc2.ni  t6, vf7 */
	/* | 		 */
	/* | 		vmulx.xyz vf4, vf4, vf8 */
	/* | 		vmulx.xyz vf5, vf5, vf9 */
	/* | 		vmulx.xyz vf6, vf6, vf10 */
	/* | 		vmulx.xyz vf7, vf7, vf11 */
	/* |  */
	/* | 		vadd.xyz  vf4, vf4, vf5 */
	/* | 		vadd.xyz  vf4, vf4, vf6 */
	/* | 		vadd.xyz  vf4, vf4, vf7 */
	/* |  */
	/* | 		vmulx.xyz vf4, vf4, vf12 */
	/* |  */
	/* | 		qmfc2.ni  a6, vf4 */
	/* |  */
	/* | 		pcpyud    a7, a6, a6 */
	/* |  */
	/* | 		sdl       a6, 0x7(%1) */
	/* |         sdr       a6,   0(%1) */
	/* |       */
	/* |         sw        a7,   8(%1)  */
	/* |     " : : "r"(idata), "r"(odata), "r"(attr), "f"(frame), "f"(ftmp) :  */
	/* |     ); */
	    ee_gpr r8 = {{0}}, r9 = {{0}}, r10 = {{0}}, r11 = {{0}}, r12 = {{0}}, r13 = {{0}}, r14 = {{0}}, r15 = {{0}};
	    __typeof__((frame) + 0) op0 = (frame);
	    __typeof__((ftmp) + 0) op1 = (ftmp);
	    __typeof__((idata) + 0) op2 = (idata);
	    __typeof__((odata) + 0) op3 = (odata);
	    r8.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op0)));
	    vu_add_bc(VF(12), VF(0), VF(0)[3], 8);
	    vu_add_bc(VF(12), VF(12), VF(0)[3], 8);
	    vu_qmtc2(4, r8);
	    vu_mul(VF(5), VF(4), VF(4), 8);
	    vu_mul(VF(6), VF(5), VF(4), 8);
	    vu_add(VF(8), VF(5), VF(5), 8);
	    vu_sub(VF(8), VF(8), VF(6), 8);
	    vu_sub(VF(8), VF(8), VF(4), 8);
	    vu_mul_bc(VF(10), VF(5), VF(2)[2], 8);
	    vu_mul(VF(9), VF(6), VF(2), 8);
	    vu_sub(VF(9), VF(9), VF(10), 8);
	    vu_add(VF(9), VF(9), VF(12), 8);
	    vu_sub(VF(7), VF(0), VF(2), 8);
	    vu_mul_bc(VF(7), VF(6), VF(7)[0], 8);
	    vu_mul_bc(VF(10), VF(5), VF(2)[1], 8);
	    vu_add(VF(10), VF(10), VF(4), 8);
	    vu_add(VF(10), VF(10), VF(7), 8);
	    vu_sub(VF(11), VF(6), VF(5), 8);
	    r9.d[0] = EE_SEXT32(ee_fbits(EE_CVAR_GETF(op1)));
	    vu_qmtc2(12, r9);
	    r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x7)) - 7);
	    r8.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0)));
	    r9.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (8))));
	    r10.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x13)) - 7);
	    r10.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0xC)));
	    r11.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (20))));
	    r12.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x1F)) - 7);
	    r12.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x18)));
	    r13.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (32))));
	    r14.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x2B)) - 7);
	    r14.d[0] = ee_ld64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (0x24)));
	    r15.d[0] = EE_SEXT32(*(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op2)) + (44))));
	    r8 = ee_pcpyld(r9, r8);
	    r10 = ee_pcpyld(r11, r10);
	    r12 = ee_pcpyld(r13, r12);
	    r14 = ee_pcpyld(r15, r14);
	    vu_qmtc2(4, r8);
	    vu_qmtc2(5, r10);
	    vu_qmtc2(6, r12);
	    vu_qmtc2(7, r14);
	    vu_mul_bc(VF(4), VF(4), VF(8)[0], 14);
	    vu_mul_bc(VF(5), VF(5), VF(9)[0], 14);
	    vu_mul_bc(VF(6), VF(6), VF(10)[0], 14);
	    vu_mul_bc(VF(7), VF(7), VF(11)[0], 14);
	    vu_add(VF(4), VF(4), VF(5), 14);
	    vu_add(VF(4), VF(4), VF(6), 14);
	    vu_add(VF(4), VF(4), VF(7), 14);
	    vu_mul_bc(VF(4), VF(4), VF(12)[0], 14);
	    r10 = vu_qmfc2(4);
	    r11 = ee_pcpyud(r10, r10);
	    ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0x7)) - 7, r10.d[0]);
	    ee_sd64(((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (0)), r10.d[0]);
	    *(uint32_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(op3)) + (8))) = (uint32_t)(r11.d[0]);
	}
}

// 100% matching! 
void	njBezierSpline(Float *idata, Float *odata, NJS_SPLINE *attr, Float frame) 
{
    unsigned int ulCnt; 
    unsigned int ulMax; 
    float fFactMax;     
    float fResult;      
    
    odata[0] = 0;
    odata[1] = 0;
    odata[2] = 0;
    
    ulMax = *attr->iparam - 1;
    
    fResult = njFactorial(ulMax);

    for (ulCnt = 0; ulCnt <= ulMax; ulCnt++)  
    {            
        fFactMax = (powf(frame, ulCnt) * (fResult / (njFactorial(ulCnt) * njFactorial(ulMax - ulCnt)))) * powf(1.0f - frame, ulMax - ulCnt);
        
        odata[0] += fFactMax * *idata++;
        odata[1] += fFactMax * *idata++;
        odata[2] += fFactMax * *idata++; 
    }
}

// 100% matching! 
unsigned int njFactorial(unsigned int ulN) 
{ 
    unsigned int ulResult;

    ulResult = 1;
    
    for ( ; ulN != 0; ulN--)
    { 
        ulResult *= ulN; 
    }
    
    return ulResult; 
}
