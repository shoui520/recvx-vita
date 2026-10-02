/*
 * Force-included into every game translation unit.
 *
 * The game was written for the PS2's EE, where `long` is 64 bits and the
 * compiler has a native 128-bit integer. On ARMv7 `long` is 32 bits and there
 * is no 128-bit integer, so pin the SDK's fixed-width typedefs here before any
 * game or middleware header can define them with the wrong size.
 */
#ifndef RECVX_PRELUDE_H
#define RECVX_PRELUDE_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * newlib's <sys/types.h> declares u_long as a 32-bit unsigned long. PS2 code
 * means 64 bits by it, so take the name over once newlib's typedef is done.
 */
typedef uint64_t recvx_u_long;
#define u_long recvx_u_long
/* EE `long` (64-bit); game sources spell it ee_long since ARM long is 32-bit */
typedef int64_t ee_long;
/* EE code masks pointers to physical addresses for DMA; Vita pointers stay whole */
#define EE_PHYS_MASK 0xFFFFFFFFu

/* 128-bit "quadword": only ever copied around (GS/VIF packets, VU vectors). */
typedef unsigned int recvx_qword __attribute__((vector_size(16), aligned(16)));

#define _TYPEDEF_Uint64
#define _TYPEDEF_Sint64
#define _TYPEDEF_Uint128
#define _TYPEDEF_Sint128
typedef uint64_t    Uint64;
typedef int64_t     Sint64;
typedef recvx_qword Uint128;
typedef recvx_qword Sint128;

/*
 * MWCC-era code declares many `register` locals that the translated inline
 * assembly takes the address of. The keyword has no effect on GCC's codegen.
 */
#define register

#endif
