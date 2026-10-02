/* VIF code builders (Sony eestruct.h subset used by the game). */
#pragma once
#include "eetypes.h"

#define SCE_VIF_CODE(cmd, num, imm, irq) \
    ((u_int)(imm) | ((u_int)(num) << 16) | ((u_int)(cmd) << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_NOP(irq)                   SCE_VIF_CODE(0x00, 0, 0, irq)
#define SCE_VIF1_SET_STCYCL(wl, cl, irq)        SCE_VIF_CODE(0x01, 0, (cl) | ((wl) << 8), irq)
#define SCE_VIF1_SET_OFFSET(offset, irq)        SCE_VIF_CODE(0x02, 0, offset, irq)
#define SCE_VIF1_SET_BASE(base, irq)            SCE_VIF_CODE(0x03, 0, base, irq)
#define SCE_VIF1_SET_ITOP(addr, irq)            SCE_VIF_CODE(0x04, 0, addr, irq)
#define SCE_VIF1_SET_STMOD(mode, irq)           SCE_VIF_CODE(0x05, 0, mode, irq)
#define SCE_VIF1_SET_MSKPATH3(mask, irq)        SCE_VIF_CODE(0x06, 0, (mask) << 15, irq)
#define SCE_VIF1_SET_MARK(mark, irq)            SCE_VIF_CODE(0x07, 0, mark, irq)
#define SCE_VIF1_SET_FLUSHE(irq)                SCE_VIF_CODE(0x10, 0, 0, irq)
#define SCE_VIF1_SET_FLUSH(irq)                 SCE_VIF_CODE(0x11, 0, 0, irq)
#define SCE_VIF1_SET_FLUSHA(irq)                SCE_VIF_CODE(0x13, 0, 0, irq)
#define SCE_VIF1_SET_MSCAL(vuaddr, irq)         SCE_VIF_CODE(0x14, 0, vuaddr, irq)
#define SCE_VIF1_SET_MSCALF(vuaddr, irq)        SCE_VIF_CODE(0x15, 0, vuaddr, irq)
#define SCE_VIF1_SET_MSCNT(irq)                 SCE_VIF_CODE(0x17, 0, 0, irq)
#define SCE_VIF1_SET_STMASK(irq)                SCE_VIF_CODE(0x20, 0, 0, irq)
#define SCE_VIF1_SET_STROW(irq)                 SCE_VIF_CODE(0x30, 0, 0, irq)
#define SCE_VIF1_SET_STCOL(irq)                 SCE_VIF_CODE(0x31, 0, 0, irq)
#define SCE_VIF1_SET_MPG(vuaddr, num, irq)      SCE_VIF_CODE(0x4a, num, vuaddr, irq)
#define SCE_VIF1_SET_DIRECT(count, irq)         SCE_VIF_CODE(0x50, 0, count, irq)
#define SCE_VIF1_SET_DIRECTHL(count, irq)       SCE_VIF_CODE(0x51, 0, count, irq)
#define SCE_VIF1_SET_UNPACK(vuaddr, num, cmd, irq) SCE_VIF_CODE(0x60 | (cmd), num, vuaddr, irq)
