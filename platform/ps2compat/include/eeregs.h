/*
 * EE hardware registers. As in the Sony header, the register names are
 * `volatile u_int *` so game code can dereference them; here they point into a
 * shadow copy of the 0x10000000-0x1000ffff register page. Writes that must have
 * an effect (starting a DMA channel) go through DPUT_* / recvx_hwreg_write.
 */
#pragma once
#include "eetypes.h"

#define RECVX_HWREG_BASE 0x10000000u
#define RECVX_HWREG_SIZE 0x10000u

extern volatile u_int recvx_hwreg_shadow[RECVX_HWREG_SIZE / 4];

#define RECVX_HWREG(a) (&recvx_hwreg_shadow[((u_int)(a) - RECVX_HWREG_BASE) / 4])

u_int recvx_hwreg_read(u_int addr);
void  recvx_hwreg_write(u_int addr, u_int value);

/* register addresses */
#define RECVX_T0_COUNT   0x10000000u
#define RECVX_T0_MODE    0x10000010u
#define RECVX_IPU_CTRL   0x10002010u
#define RECVX_IPU_BP     0x10002020u
#define RECVX_GIF_STAT   0x10003020u
#define RECVX_VIF1_STAT  0x10003c00u
#define RECVX_D0_CHCR    0x10008000u
#define RECVX_D1_CHCR    0x10009000u
#define RECVX_D2_CHCR    0x1000a000u
#define RECVX_D2_TADR    0x1000a030u
#define RECVX_D3_CHCR    0x1000b000u
#define RECVX_D3_MADR    0x1000b010u
#define RECVX_D3_QWC     0x1000b020u
#define RECVX_D4_CHCR    0x1000b400u
#define RECVX_D4_MADR    0x1000b410u
#define RECVX_D4_QWC     0x1000b420u
#define RECVX_D4_TADR    0x1000b430u
#define RECVX_D_CTRL     0x1000e000u
#define RECVX_D_STAT     0x1000e010u
#define RECVX_D_PCR      0x1000e020u

#define T0_COUNT   RECVX_HWREG(RECVX_T0_COUNT)
#define T0_MODE    RECVX_HWREG(RECVX_T0_MODE)
#define IPU_CTRL   RECVX_HWREG(RECVX_IPU_CTRL)
#define IPU_BP     RECVX_HWREG(RECVX_IPU_BP)
#define GIF_STAT   RECVX_HWREG(RECVX_GIF_STAT)
#define VIF1_STAT  RECVX_HWREG(RECVX_VIF1_STAT)
#define D0_CHCR    RECVX_HWREG(RECVX_D0_CHCR)
#define D1_CHCR    RECVX_HWREG(RECVX_D1_CHCR)
#define D2_CHCR    RECVX_HWREG(RECVX_D2_CHCR)
#define D2_TADR    RECVX_HWREG(RECVX_D2_TADR)
#define D3_CHCR    RECVX_HWREG(RECVX_D3_CHCR)
#define D3_MADR    RECVX_HWREG(RECVX_D3_MADR)
#define D3_QWC     RECVX_HWREG(RECVX_D3_QWC)
#define D4_CHCR    RECVX_HWREG(RECVX_D4_CHCR)
#define D4_MADR    RECVX_HWREG(RECVX_D4_MADR)
#define D4_QWC     RECVX_HWREG(RECVX_D4_QWC)
#define D4_TADR    RECVX_HWREG(RECVX_D4_TADR)
#define D_CTRL     RECVX_HWREG(RECVX_D_CTRL)
#define D_STAT     RECVX_HWREG(RECVX_D_STAT)
#define D_PCR      RECVX_HWREG(RECVX_D_PCR)

#define D_ENABLER  0x1000f520u
#define D_ENABLEW  0x1000f590u

#define DGET_D_PCR()      recvx_hwreg_read(RECVX_D_PCR)
#define DPUT_D_PCR(x)     recvx_hwreg_write(RECVX_D_PCR, (x))
#define DPUT_D_STAT(x)    recvx_hwreg_write(RECVX_D_STAT, (x))
#define DPUT_D2_TADR(x)   recvx_hwreg_write(RECVX_D2_TADR, (x))
#define DPUT_D2_CHCR(x)   recvx_hwreg_write(RECVX_D2_CHCR, (x))
#define DGET_VIF1_STAT()  recvx_hwreg_read(RECVX_VIF1_STAT)
#define DGET_GIF_STAT()   recvx_hwreg_read(RECVX_GIF_STAT)
#define DGET_IPU_CTRL()   recvx_hwreg_read(RECVX_IPU_CTRL)
#define DGET_IPU_BP()     recvx_hwreg_read(RECVX_IPU_BP)
