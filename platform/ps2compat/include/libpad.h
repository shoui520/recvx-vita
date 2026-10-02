/* libpad: DualShock 2 input, backed by SceCtrl on the Vita. */
#pragma once
#include "eetypes.h"

#define scePadDmaBufferMax 16

#define scePadStateDiscon   0
#define scePadStateFindPad  1
#define scePadStateFindCTP1 2
#define scePadStateExecCmd  5
#define scePadStateStable   6
#define scePadStateError    7

#define scePadReqStateComplete 0
#define scePadReqStateFaild    1
#define scePadReqStateBusy     2

#define InfoModeCurID     1
#define InfoModeCurExID   2
#define InfoModeCurExOffs 3
#define InfoModeIdTable   4
#define InfoActFunc 1
#define InfoActSub  2
#define InfoActSize 3
#define InfoActCurr 4

int scePadInit(int mode);
int scePadEnd(void);
int scePadPortOpen(int port, int slot, u_long128 *addr);
int scePadPortClose(int port, int slot);
int scePadRead(int port, int slot, u_char *rdata);
int scePadGetState(int port, int slot);
int scePadGetReqState(int port, int slot);
int scePadInfoMode(int port, int slot, int term, int offs);
int scePadSetMainMode(int port, int slot, int offs, int lock);
int scePadInfoAct(int port, int slot, int actno, int term);
int scePadSetActAlign(int port, int slot, const u_char *data);
int scePadSetActDirect(int port, int slot, const u_char *data);
int scePadInfoPressMode(int port, int slot);
int scePadEnterPressMode(int port, int slot);
int scePadExitPressMode(int port, int slot);
