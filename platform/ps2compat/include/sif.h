/* SIF DMA between EE and IOP. On the Vita both sides live in one address space. */
#pragma once
#include "eetypes.h"

typedef struct { u_int data; u_int addr; u_int size; u_int mode; } sceSifDmaData;

#define SIF_DMA_INT_I 0x02
#define SIF_DMA_INT_O 0x04

u_int sceSifSetDma(sceSifDmaData *sdd, int len);
int   sceSifDmaStat(u_int id);
u_int isceSifSetDma(sceSifDmaData *sdd, int len);
