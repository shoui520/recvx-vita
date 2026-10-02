/* libcdvd: disc access. The port maps sectors/files onto the prepared data directory. */
#pragma once
#include "eetypes.h"

typedef struct { u_char stat, second, minute, hour, pad, day, month, year; } sceCdCLOCK;
typedef struct { u_int lsn; u_int size; char name[16]; u_char date[8]; } sceCdlFILE;
typedef struct { u_char trycount, spindlctrl, datapattern, pad; } sceCdRMode;

#define SCECdINIT 0
#define SCECdINON 1
#define SCECdINIT_DMA 2
#define SCECdEXIT 5

#define SCECdCD  1
#define SCECdDVD 2

#define SCECdSpinMax 0
#define SCECdSpinNom 1
#define SCECdSpinStm 0
#define SCECdSecS2048 0

#define SCECdErFAIL   -1
#define SCECdErNO     0x00
#define SCECdErEOM    0x32
#define SCECdErTRMOPN 0x31
#define SCECdErREAD   0x30
#define SCECdErPRM    0x22
#define SCECdErILI    0x21
#define SCECdErIPI    0x20
#define SCECdErCUD    0x14
#define SCECdErNORDY  0x13
#define SCECdErNODISC 0x12
#define SCECdErOPENS  0x11
#define SCECdErCMD    0x10
#define SCECdErABRT   0x01

int sceCdInit(int mode);
int sceCdMmode(int media);
int sceCdDiskReady(int mode);
int sceCdSync(int mode);
int sceCdSearchFile(sceCdlFILE *fp, const char *name);
int sceCdRead(u_int lsn, u_int sectors, void *buf, sceCdRMode *mode);
int sceCdGetError(void);
int sceCdBreak(void);
int sceCdPause(void);
int sceCdReadClock(sceCdCLOCK *rtc);
int sceCdStInit(u_int bufmax, u_int bankmax, u_int iop_bufaddr);
int sceCdStStart(u_int lsn, sceCdRMode *mode);
int sceCdStRead(u_int sectors, u_int *buf, u_int mode, u_int *err);
int sceCdStStop(void);
