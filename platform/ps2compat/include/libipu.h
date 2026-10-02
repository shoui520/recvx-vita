/* libipu: MPEG macroblock decoder. Movies are pre-converted for SceAvPlayer, so this is declarations only. */
#pragma once
#include "eetypes.h"

typedef struct { u_int d3madr, d3qwc, d4madr, d4qwc, d4tadr, d4chcr, ipubp, ipuctrl; } sceIpuDmaEnv;
typedef struct { u_int c[16 * 16]; } sceIpuRGB32;

void sceIpuInit(void);
void sceIpuBCLR(int bp);
int  sceIpuIsBusy(void);
void sceIpuStopDMA(sceIpuDmaEnv *env);
void sceIpuRestartDMA(sceIpuDmaEnv *env);
