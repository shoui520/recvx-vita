/* libdma: DMA channel API. The port's render backend consumes the chains. */
#pragma once
#include "eetypes.h"

#define SCE_DMA_VIF0    0
#define SCE_DMA_VIF1    1
#define SCE_DMA_GIF     2
#define SCE_DMA_fromIPU 3
#define SCE_DMA_toIPU   4
#define SCE_DMA_fromSPR 8
#define SCE_DMA_toSPR   9

typedef struct {
    u_int chcr; u_int p0[3];
    void *madr; u_int p1[3];
    u_int qwc;  u_int p2[3];
    void *tadr; u_int p3[3];
    void *as0;  u_int p4[3];
    void *as1;  u_int p5[3];
    u_int p6[4];
    u_int p7[4];
    void *sadr; u_int p8[3];
} sceDmaChan;

typedef struct { u_short qwc; u_char mark; u_char id; void *next; u_int p[2]; } sceDmaTag;

int  sceDmaReset(int mode);
sceDmaChan *sceDmaGetChan(int id);
void sceDmaSend(sceDmaChan *d, void *tag);
void sceDmaSendN(sceDmaChan *d, void *addr, int size);
int  sceDmaSync(sceDmaChan *d, int mode, int timeout);
