/* EE kernel API (threads, semaphores, interrupts, cache), implemented by the port layer. */
#pragma once
#include "eetypes.h"

struct ThreadParam {
    int   status;
    void (*entry)(void *);
    void *stack;
    int   stackSize;
    void *gpReg;
    int   initPriority;
    int   currentPriority;
    u_int attr;
    u_int option;
    int   waitType;
    int   waitId;
    int   wakeupCount;
};

struct SemaParam {
    int   currentCount;
    int   maxCount;
    int   initCount;
    int   numWaitThreads;
    u_int attr;
    u_int option;
};

#define THS_RUN     0x01
#define THS_READY   0x02
#define THS_WAIT    0x04
#define THS_SUSPEND 0x08
#define THS_DORMANT 0x10

#define INTC_GS     0
#define INTC_SBUS   1
#define INTC_VBLANK_S 2
#define INTC_VBLANK_E 3
#define INTC_VIF0   4
#define INTC_VIF1   5
#define INTC_VU0    6
#define INTC_VU1    7
#define INTC_IPU    8
#define INTC_TIM0   9
#define INTC_TIM1   10

#define WRITEBACK_DCACHE 0
#define INVALIDATE_DCACHE 1
#define INVALIDATE_ICACHE 2
#define INVALIDATE_CACHE 4

extern char _gp; /* EE global pointer; only stored in ThreadParam.gpReg */
int  CreateThread(struct ThreadParam *);
int  DeleteThread(int);
int  StartThread(int, void *arg);
void ExitThread(void);
void ExitDeleteThread(void);
int  TerminateThread(int);
int  GetThreadId(void);
int  ReferThreadStatus(int, struct ThreadParam *);
int  ChangeThreadPriority(int, int);
int  RotateThreadReadyQueue(int);
int  SleepThread(void);
int  WakeupThread(int);
int  iWakeupThread(int);
int  SuspendThread(int);
int  ResumeThread(int);

int  CreateSema(struct SemaParam *);
int  DeleteSema(int);
int  SignalSema(int);
int  iSignalSema(int);
int  WaitSema(int);
int  PollSema(int);
int  iPollSema(int);

int  SetAlarm(u_short time, void (*cb)(int, u_short, void *), void *arg);
int  iSetAlarm(u_short time, void (*cb)(int, u_short, void *), void *arg);
int  ReleaseAlarm(int);

int  AddIntcHandler(int cause, int (*handler)(int), int next);
int  RemoveIntcHandler(int cause, int id);
int  EnableIntc(int cause);
int  DisableIntc(int cause);
int  AddDmacHandler(int ch, int (*handler)(int), int next);
int  EnableDmac(int ch);
int  DisableDmac(int ch);
void ExitHandler(void);

void FlushCache(int op);
void InvalidDCache(void *start, void *end);
void SyncDCache(void *start, void *end);
void iFlushCache(int op);

void ResetEE(int);
void SetGsCrt(short interlace, short mode, short field);
u_long GsPutIMR(u_long);

int  DIntr(void);
int  EIntr(void);

int  scePrintf(const char *fmt, ...);

/* The EE's uncached/accelerated mirrors of main RAM are plain RAM on the Vita. */
#define UNCACHED_SEG(x)  ((void *)(x))
