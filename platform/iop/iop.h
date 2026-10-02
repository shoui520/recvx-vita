/*
 * The IOP, replaced by native code running in the EE's address space.
 *
 * IOP memory is a 2 MB array; an IOP address is an offset into it, so the
 * values the game stores and passes around stay small integers as on the
 * PS2. SIF DMA and RPC complete synchronously; the IOP-side services
 * (TSNDDRV, CRI's DTX/SJX/SJRMT/PS2RNA servers) run under one lock that
 * they share with the audio thread.
 */
#pragma once
#include <stdint.h>

#define IOP_RAM_SIZE 0x200000

extern uint8_t *iop_ram;
static inline void *iop_ptr(uint32_t addr) { return iop_ram + (addr & (IOP_RAM_SIZE - 1)); }

uint32_t iop_alloc(uint32_t size);
void iop_lock(void);
void iop_unlock(void);

typedef void *(*iop_rpc_fn)(unsigned fno, void *data, int size);
void iop_register_rpc(unsigned id, iop_rpc_fn fn);

/* tsnddrv.c */
void tsnddrv_init(void);
void tsnddrv_tick(void);

/* cri_iop.c */
void cri_iop_init(void);
void cri_iop_dma(uint32_t addr, uint32_t size);   /* after an EE->IOP DMA lands */
