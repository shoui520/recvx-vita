#include "../../../ps2/veronica/prog/expand.h"

EXPAND_CTRL_BUF ExpandCtrlBuf;

// 100% matching!
void Init_Expand() 
{ 
    ExpandCtrlBuf.flag.abort = 0; 
}

// 99.63% matching
int Expand(register char* s, register unsigned char* d) 
{
    register int T;

    T = 0;

    { /* translated from EE asm by agent mips2c; original kept below */
    /* |  */
    /* |     { */
    /* |      */
    /* |         lui     t9, 0xFFFF  */
    /* |         addiu   t9, t9, 0x7F08 */
    /* |         addiu   t9, t9, 0x7FF8 */
    /* |              */
    /* |         lui     t8, 0xFFFF  */
    /* |         addiu   t8, t8, 0x6008 */
    /* |         addiu   t8, t8, 0x7FF8  */
    /* |              */
    /* |         lbu     t6, 0(s)  */
    /* |          */
    /* |         addi    s,  s,  1  */
    /* |          */
    /* |         sub     t7, t7, t7  */
    /* |         addiu   t7, t7, 9  */
    /* |         sub     t3, t3, t3  */
    /* |         addu    t3, t3, d */
    /* |          */
    /* |         j       l_002CAEE4  */
    /* |          */
    /* |         l_002CAED4: */
    /* |         lbu     t1, 0(s)  */
    /* |          */
    /* |         addi    s, s, 1  */
    /* |          */
    /* |         sb      t1, 0(d)  */
    /* |              */
    /* |         addi    d, d, 1  */
    /* |          */
    /* |         l_002CAEE4: */
    /* |         addiu   t7, t7, -1  */
    /* |          */
    /* |         andi    t4, t6, 0x1  */
    /* |         srl     t6, t6, 1 */
    /* |          */
    /* |         bgtz    t7, l_002CAF10          */
    /* |         vnop    */
    /* |          */
    /* |         lbu     t6, 0(s)  */
    /* |          */
    /* |         addi    s,  s,  1  */
    /* |          */
    /* |         sub     t7, t7, t7  */
    /* |         addiu   t7, t7, 8  */
    /* |          */
    /* |         andi    t4, t6, 0x1  */
    /* |              */
    /* |         srl     t6, t6, 1  */
    /* |              */
    /* |         l_002CAF10: */
    /* |         bgtz    t4, l_002CAED4  */
    /* |      */
    /* |         addiu   t7, t7, -1  */
    /* |              */
    /* |         andi    t4, t6, 0x1  */
    /* |         srl     t6, t6, 1  */
    /* |          */
    /* |         bgtz    t7, l_002CAF44  */
    /* |         vnop */
    /* |          */
    /* |         lbu     t6, 0(s) */
    /* |              */
    /* |         addi    s,  s,  1  */
    /* |          */
    /* |         sub     t7, t7, t7  */
    /* |         addiu   t7, t7, 8  */
    /* |          */
    /* |         andi    t4, t6, 0x1  */
    /* |              */
    /* |         srl     t6, t6, 1  */
    /* |              */
    /* |         l_002CAF44: */
    /* |         bgtz    t4, l_002CAFF0  */
    /* |          */
    /* |         sub     t0, t0, t0  */
    /* |          */
    /* |         addiu   t7, t7, -1  */
    /* |              */
    /* |         andi    t4, t6, 0x1  */
    /* |         srl     t6, t6, 1 */
    /* |              */
    /* |         bgtz    t7, l_002CAF7C  */
    /* |         vnop     */
    /* |          */
    /* |         lbu     t6, 0(s)  */
    /* |              */
    /* |         addi    s,  s,  1  */
    /* |          */
    /* |         sub     t7, t7, t7  */
    /* |         addiu   t7, t7, 8  */
    /* |         andi    t4, t6, 0x1  */
    /* |         srl     t6, t6, 1  */
    /* |              */
    /* |         l_002CAF7C: */
    /* |         sll     t0, t0, 1  */
    /* |         or      t0, t0, t4  */
    /* |              */
    /* |         addiu   t7, t7, -1  */
    /* |              */
    /* |         andi    t4, t6, 0x1  */
    /* |         srl     t6, t6, 1 */
    /* |              */
    /* |         bgtz    t7, l_002CAFB0  */
    /* |         vnop      */
    /* |          */
    /* |         lbu     t6, 0(s) */
    /* |              */
    /* |         addi    s,  s,  1  */
    /* |          */
    /* |         sub     t7, t7, t7  */
    /* |         addiu   t7, t7, 8  */
    /* |         andi    t4, t6, 0x1  */
    /* |         srl     t6, t6, 1  */
    /* |              */
    /* |         l_002CAFB0: */
    /* |         lbu     t2, 0(s) */
    /* |          */
    /* |         addi    s,  s,  1  */
    /* |          */
    /* |         sll     t0, t0, 1  */
    /* |         or      t0, t0, t4  */
    /* |         or      t2, t2, t9  */
    /* |      */
    /* |         l_002CAFC4: */
    /* |         addiu   t0, t0, 2  */
    /* |         addu    t2, d,  t2  */
    /* |              */
    /* |         l_002CAFCC: */
    /* |         lbu     t1, 0(t2)  */
    /* |              */
    /* |         addiu   t2, t2, 1  */
    /* |          */
    /* |         addiu   t0, t0, -1  */
    /* |              */
    /* |         sb      t1, 0(d)  */
    /* |              */
    /* |         addi    d, d, 1  */
    /* |         nop */
    /* |          */
    /* |         bgtz    t0, l_002CAFCC  */
    /* |         vnop     */
    /* |          */
    /* |         j       l_002CAEE4  */
    /* |         vnop     */
    /* |      */
    /* |         l_002CAFF0: */
    /* |         lbu     t0, 0(s)  */
    /* |         lbu     t1, 1(s) */
    /* |              */
    /* |         addi    s,  s,  2  */
    /* |          */
    /* |         andi    t2, t0, 0xFF  */
    /* |         sll     t1, t1, 8  */
    /* |         or      t2, t2, t1  */
    /* |              */
    /* |         sub     t4, t4, t4  */
    /* |          */
    /* |         beq     t4, t2, l_002CB040  */
    /* |          */
    /* |         srl     t2, t2, 3  */
    /* |         andi    t0, t0, 0x7  */
    /* |              */
    /* |         and     t4, t4, t4  */
    /* |              */
    /* |         or      t2, t2, t8 */
    /* |          */
    /* |         bne     t4, t0, l_002CAFC4  */
    /* |          */
    /* |         lbu     t0, 0(s)  */
    /* |              */
    /* |         addi    s,  s,  1  */
    /* |          */
    /* |         addu    t2, d,  t2  */
    /* |         andi    t0, t0, 0xFF  */
    /* |         addiu   t0, t0, 1 */
    /* |              */
    /* |         j       l_002CAFCC  */
    /* |         nop */
    /* |          */
    /* |         l_002CB040: */
    /* |         sub     T, d, t3  */
    /* |         nop */
    /* |          */
    /* |     }	 */
        ee_gpr r8 = {{0}}, r9 = {{0}}, r10 = {{0}}, r11 = {{0}}, r12 = {{0}}, r14 = {{0}}, r15 = {{0}}, r24 = {{0}}, r25 = {{0}};
        /* note: reads before write: r15 */
        r25.d[0] = EE_SEXT32((uint32_t)(0xFFFF) << 16);
        r25.d[0] = EE_SEXT32((uint32_t)(r25.d[0]) + (uint32_t)(0x7F08));
        r25.d[0] = EE_SEXT32((uint32_t)(r25.d[0]) + (uint32_t)(0x7FF8));
        r24.d[0] = EE_SEXT32((uint32_t)(0xFFFF) << 16);
        r24.d[0] = EE_SEXT32((uint32_t)(r24.d[0]) + (uint32_t)(0x6008));
        r24.d[0] = EE_SEXT32((uint32_t)(r24.d[0]) + (uint32_t)(0x7FF8));
        r14.d[0] = (uint64_t)*(uint8_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(s)) + (0)));
        EE_CVAR_SET(s, EE_SEXT32((uint32_t)(EE_CVAR_GET(s)) + (uint32_t)(1)));
        r15.d[0] = EE_SEXT32((uint32_t)(r15.d[0]) - (uint32_t)(r15.d[0]));
        r15.d[0] = EE_SEXT32((uint32_t)(r15.d[0]) + (uint32_t)(9));
        r11.d[0] = EE_SEXT32((uint32_t)(r11.d[0]) - (uint32_t)(r11.d[0]));
        r11.d[0] = EE_SEXT32((uint32_t)(r11.d[0]) + (uint32_t)(EE_CVAR_GET(d)));
        goto L_Expand_l_002CAEE4;
        L_Expand_l_002CAED4:;
        r9.d[0] = (uint64_t)*(uint8_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(s)) + (0)));
        EE_CVAR_SET(s, EE_SEXT32((uint32_t)(EE_CVAR_GET(s)) + (uint32_t)(1)));
        *(uint8_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(d)) + (0))) = (uint8_t)(r9.d[0]);
        EE_CVAR_SET(d, EE_SEXT32((uint32_t)(EE_CVAR_GET(d)) + (uint32_t)(1)));
        L_Expand_l_002CAEE4:;
        r15.d[0] = EE_SEXT32((uint32_t)(r15.d[0]) + (uint32_t)(-1));
        r12.d[0] = (r14.d[0]) & (uint64_t)(uint16_t)(0x1);
        r14.d[0] = EE_SEXT32((uint32_t)(r14.d[0]) >> 1);
        if ((int64_t)(r15.d[0]) > 0) goto L_Expand_l_002CAF10;
        r14.d[0] = (uint64_t)*(uint8_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(s)) + (0)));
        EE_CVAR_SET(s, EE_SEXT32((uint32_t)(EE_CVAR_GET(s)) + (uint32_t)(1)));
        r15.d[0] = EE_SEXT32((uint32_t)(r15.d[0]) - (uint32_t)(r15.d[0]));
        r15.d[0] = EE_SEXT32((uint32_t)(r15.d[0]) + (uint32_t)(8));
        r12.d[0] = (r14.d[0]) & (uint64_t)(uint16_t)(0x1);
        r14.d[0] = EE_SEXT32((uint32_t)(r14.d[0]) >> 1);
        L_Expand_l_002CAF10:;
        if ((int64_t)(r12.d[0]) > 0) goto L_Expand_l_002CAED4;
        r15.d[0] = EE_SEXT32((uint32_t)(r15.d[0]) + (uint32_t)(-1));
        r12.d[0] = (r14.d[0]) & (uint64_t)(uint16_t)(0x1);
        r14.d[0] = EE_SEXT32((uint32_t)(r14.d[0]) >> 1);
        if ((int64_t)(r15.d[0]) > 0) goto L_Expand_l_002CAF44;
        r14.d[0] = (uint64_t)*(uint8_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(s)) + (0)));
        EE_CVAR_SET(s, EE_SEXT32((uint32_t)(EE_CVAR_GET(s)) + (uint32_t)(1)));
        r15.d[0] = EE_SEXT32((uint32_t)(r15.d[0]) - (uint32_t)(r15.d[0]));
        r15.d[0] = EE_SEXT32((uint32_t)(r15.d[0]) + (uint32_t)(8));
        r12.d[0] = (r14.d[0]) & (uint64_t)(uint16_t)(0x1);
        r14.d[0] = EE_SEXT32((uint32_t)(r14.d[0]) >> 1);
        L_Expand_l_002CAF44:;
        if ((int64_t)(r12.d[0]) > 0) goto L_Expand_l_002CAFF0;
        r8.d[0] = EE_SEXT32((uint32_t)(r8.d[0]) - (uint32_t)(r8.d[0]));
        r15.d[0] = EE_SEXT32((uint32_t)(r15.d[0]) + (uint32_t)(-1));
        r12.d[0] = (r14.d[0]) & (uint64_t)(uint16_t)(0x1);
        r14.d[0] = EE_SEXT32((uint32_t)(r14.d[0]) >> 1);
        if ((int64_t)(r15.d[0]) > 0) goto L_Expand_l_002CAF7C;
        r14.d[0] = (uint64_t)*(uint8_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(s)) + (0)));
        EE_CVAR_SET(s, EE_SEXT32((uint32_t)(EE_CVAR_GET(s)) + (uint32_t)(1)));
        r15.d[0] = EE_SEXT32((uint32_t)(r15.d[0]) - (uint32_t)(r15.d[0]));
        r15.d[0] = EE_SEXT32((uint32_t)(r15.d[0]) + (uint32_t)(8));
        r12.d[0] = (r14.d[0]) & (uint64_t)(uint16_t)(0x1);
        r14.d[0] = EE_SEXT32((uint32_t)(r14.d[0]) >> 1);
        L_Expand_l_002CAF7C:;
        r8.d[0] = EE_SEXT32((uint32_t)(r8.d[0]) << 1);
        r8.d[0] = (r8.d[0]) | r12.d[0];
        r15.d[0] = EE_SEXT32((uint32_t)(r15.d[0]) + (uint32_t)(-1));
        r12.d[0] = (r14.d[0]) & (uint64_t)(uint16_t)(0x1);
        r14.d[0] = EE_SEXT32((uint32_t)(r14.d[0]) >> 1);
        if ((int64_t)(r15.d[0]) > 0) goto L_Expand_l_002CAFB0;
        r14.d[0] = (uint64_t)*(uint8_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(s)) + (0)));
        EE_CVAR_SET(s, EE_SEXT32((uint32_t)(EE_CVAR_GET(s)) + (uint32_t)(1)));
        r15.d[0] = EE_SEXT32((uint32_t)(r15.d[0]) - (uint32_t)(r15.d[0]));
        r15.d[0] = EE_SEXT32((uint32_t)(r15.d[0]) + (uint32_t)(8));
        r12.d[0] = (r14.d[0]) & (uint64_t)(uint16_t)(0x1);
        r14.d[0] = EE_SEXT32((uint32_t)(r14.d[0]) >> 1);
        L_Expand_l_002CAFB0:;
        r10.d[0] = (uint64_t)*(uint8_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(s)) + (0)));
        EE_CVAR_SET(s, EE_SEXT32((uint32_t)(EE_CVAR_GET(s)) + (uint32_t)(1)));
        r8.d[0] = EE_SEXT32((uint32_t)(r8.d[0]) << 1);
        r8.d[0] = (r8.d[0]) | r12.d[0];
        r10.d[0] = (r10.d[0]) | r25.d[0];
        L_Expand_l_002CAFC4:;
        r8.d[0] = EE_SEXT32((uint32_t)(r8.d[0]) + (uint32_t)(2));
        r10.d[0] = EE_SEXT32((uint32_t)(EE_CVAR_GET(d)) + (uint32_t)(r10.d[0]));
        L_Expand_l_002CAFCC:;
        r9.d[0] = (uint64_t)*(uint8_t *)(((uintptr_t)(uint32_t)(r10.d[0]) + (0)));
        r10.d[0] = EE_SEXT32((uint32_t)(r10.d[0]) + (uint32_t)(1));
        r8.d[0] = EE_SEXT32((uint32_t)(r8.d[0]) + (uint32_t)(-1));
        *(uint8_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(d)) + (0))) = (uint8_t)(r9.d[0]);
        EE_CVAR_SET(d, EE_SEXT32((uint32_t)(EE_CVAR_GET(d)) + (uint32_t)(1)));
        if ((int64_t)(r8.d[0]) > 0) goto L_Expand_l_002CAFCC;
        goto L_Expand_l_002CAEE4;
        L_Expand_l_002CAFF0:;
        r8.d[0] = (uint64_t)*(uint8_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(s)) + (0)));
        r9.d[0] = (uint64_t)*(uint8_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(s)) + (1)));
        EE_CVAR_SET(s, EE_SEXT32((uint32_t)(EE_CVAR_GET(s)) + (uint32_t)(2)));
        r10.d[0] = (r8.d[0]) & (uint64_t)(uint16_t)(0xFF);
        r9.d[0] = EE_SEXT32((uint32_t)(r9.d[0]) << 8);
        r10.d[0] = (r10.d[0]) | r9.d[0];
        r12.d[0] = EE_SEXT32((uint32_t)(r12.d[0]) - (uint32_t)(r12.d[0]));
        if ((int64_t)(r12.d[0]) == (int64_t)(r10.d[0])) goto L_Expand_l_002CB040;
        r10.d[0] = EE_SEXT32((uint32_t)(r10.d[0]) >> 3);
        r8.d[0] = (r8.d[0]) & (uint64_t)(uint16_t)(0x7);
        r12.d[0] = (r12.d[0]) & r12.d[0];
        r10.d[0] = (r10.d[0]) | r24.d[0];
        if ((int64_t)(r12.d[0]) != (int64_t)(r8.d[0])) goto L_Expand_l_002CAFC4;
        r8.d[0] = (uint64_t)*(uint8_t *)(((uintptr_t)(uint32_t)(EE_CVAR_GET(s)) + (0)));
        EE_CVAR_SET(s, EE_SEXT32((uint32_t)(EE_CVAR_GET(s)) + (uint32_t)(1)));
        r10.d[0] = EE_SEXT32((uint32_t)(EE_CVAR_GET(d)) + (uint32_t)(r10.d[0]));
        r8.d[0] = (r8.d[0]) & (uint64_t)(uint16_t)(0xFF);
        r8.d[0] = EE_SEXT32((uint32_t)(r8.d[0]) + (uint32_t)(1));
        goto L_Expand_l_002CAFCC;
        L_Expand_l_002CB040:;
        EE_CVAR_SET(T, EE_SEXT32((uint32_t)(EE_CVAR_GET(d)) - (uint32_t)(r11.d[0])));
    }

    return T;
} 

/* Following is a C version of Expand() provided by Clownacy */
/*typedef struct Expand_State
{
    const unsigned char *source;
    unsigned char descriptor_field;
    unsigned char descriptor_bits_remaining;
} Expand_State;
 
static void Expand_RefreshDescriptorField(Expand_State* const state)
{
    state->descriptor_field = *state->source++;
    state->descriptor_bits_remaining = 8;
}
 
static bool Expand_GetDescriptorBit(Expand_State* const state)
{
    bool bit;
 
    if (--state->descriptor_bits_remaining == 0)
        Expand_RefreshDescriptorField(state);
 
    bit = state->descriptor_field & 1;
    state->descriptor_field >>= 1;
 
    return bit;
}
 
int Expand(register char* s, register unsigned char* d)
{
    unsigned char *destination = d;
    Expand_State state;
 
    state.source = (unsigned char*)s;
    Expand_RefreshDescriptorField(&state);
 
    // This seems like a bug, but the game's data will not decompress correctly without it.
    ++state.descriptor_bits_remaining;
 
    for (;;)
    {
        if (Expand_GetDescriptorBit(&state))
        {
            *destination++ = *state.source++;
        }
        else
        {
            unsigned int length = 0;
            int offset;
 
            if (!Expand_GetDescriptorBit(&state))
            {
                if (Expand_GetDescriptorBit(&state))
                    length += 2;
 
                if (Expand_GetDescriptorBit(&state))
                    ++length;
 
                ++length;
 
                offset = -0x100 + *state.source++;
            }
            else
            {
                const unsigned int lower = *state.source++;
                const unsigned int upper = *state.source++;
                const unsigned int whole = lower | (upper << 8);
 
                if (whole == 0)
                    break;
 
                offset = -0x2000 + (whole >> 3);
                length = whole & 7;
 
                if (length != 0)
                    ++length;
                else
                    length = *state.source++;
            }
 
            do
            {
                *destination = destination[offset];
                ++destination;
            } while (length-- != 0);
        }
    }
 
    return destination - d;
}*/
