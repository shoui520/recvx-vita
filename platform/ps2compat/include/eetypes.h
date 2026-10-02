/* PS2 EE base types, sized for ARMv7 (where `long` is only 32 bits). */
#pragma once
#include "recvx_prelude.h"

#ifndef _EETYPES_H_
#define _EETYPES_H_

typedef unsigned char  u_char;
typedef unsigned short u_short;
typedef unsigned int   u_int;
/* u_long (64-bit on the EE) comes from recvx_prelude.h. */
typedef recvx_qword    u_long128;
typedef recvx_qword    long128;

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#endif
