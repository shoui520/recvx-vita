/*
 * EE kernel services on the Vita kernel.
 *
 * EE threads become Vita threads pinned to one core, so the EE's strict
 * priority scheduling (a thread runs until it blocks or something of higher
 * priority wakes) carries over. Interrupt handlers and alarms run on threads of
 * the highest user priority on that same core: like an EE interrupt they
 * preempt game code and run to completion before it resumes.
 */
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>

#include "eekernel.h"
#include "libgraph.h"
#include "recvx_platform.h"

char _gp;

#define GAME_CPU      SCE_KERNEL_CPU_MASK_USER_0
#define IRQ_PRIORITY  64   /* SCE_KERNEL_HIGHEST_PRIORITY_USER */
#define EE_PRIO_BASE  65   /* EE priority 0 */
#define MIN_STACK     (128 * 1024)

static int to_vita_prio(int ee) { int p = EE_PRIO_BASE + ee; return p > 190 ? 190 : p; }
static int to_ee_prio(int vita) { return vita - EE_PRIO_BASE; }

/* ------------------------------------------------------------------ threads */

#define MAX_THREADS 32
static struct { SceUID uid, wake; void (*entry)(void *); void *arg; } threads[MAX_THREADS];

static int thread_slot(SceUID uid)
{
    for (int i = 0; i < MAX_THREADS; i++)
        if (threads[i].uid == uid) return i;
    return -1;
}

static int thread_tramp(SceSize argc, void *argv)
{
    int slot = *(int *)argv;
    (void)argc;
    threads[slot].entry(threads[slot].arg);
    return 0;
}

int CreateThread(struct ThreadParam *p)
{
    int slot = thread_slot(0);
    if (slot < 0) return -1;
    int stack = p->stackSize < MIN_STACK ? MIN_STACK : p->stackSize;
    SceUID uid = sceKernelCreateThread("ee_thread", thread_tramp, to_vita_prio(p->initPriority),
                                       stack, 0, GAME_CPU, NULL);
    if (uid < 0) return -1;
    threads[slot].uid = uid;
    threads[slot].entry = p->entry;
    return uid;
}

int StartThread(int id, void *arg)
{
    int slot = thread_slot(id);
    if (slot < 0) return -1;
    threads[slot].arg = arg;
    return sceKernelStartThread(id, sizeof(slot), &slot) < 0 ? -1 : id;
}

int DeleteThread(int id)
{
    int slot = thread_slot(id);
    if (slot < 0) return -1;
    /* only ever called on suspended threads at shutdown; a running Vita
     * thread can't be deleted, so just forget it */
    if (threads[slot].wake > 0) sceKernelDeleteSema(threads[slot].wake);
    threads[slot].uid = threads[slot].wake = 0;
    return id;
}

int GetThreadId(void) { return sceKernelGetThreadId(); }

int ReferThreadStatus(int id, struct ThreadParam *p)
{
    SceKernelThreadInfo info;
    info.size = sizeof(info);
    if (sceKernelGetThreadInfo(id, &info) < 0) return -1;
    memset(p, 0, sizeof(*p));
    p->status = THS_RUN;
    p->stackSize = info.stackSize;
    p->initPriority = to_ee_prio(info.initPriority);
    p->currentPriority = to_ee_prio(info.currentPriority);
    return id;
}

int ChangeThreadPriority(int id, int prio)
{
    int old;
    SceKernelThreadInfo info;
    info.size = sizeof(info);
    old = sceKernelGetThreadInfo(id, &info) < 0 ? 0 : to_ee_prio(info.currentPriority);
    sceKernelChangeThreadPriority(id, to_vita_prio(prio));
    return old;
}

/*
 * SleepThread/WakeupThread: the EE counts wakeups that arrive before the
 * sleep, which is exactly a semaphore per thread. Created on first use so
 * ee_main and Vita-created threads get one too.
 */
static SceUID wake_sema(SceUID tid)
{
    static SceKernelLwMutexWork lock_work;
    static int lock_init;
    if (!lock_init) { sceKernelCreateLwMutex(&lock_work, "ee_wake", 0, 0, NULL); lock_init = 1; }

    sceKernelLockLwMutex(&lock_work, 1, NULL);
    int slot = thread_slot(tid);
    if (slot < 0) {
        slot = thread_slot(0);
        if (slot >= 0) threads[slot].uid = tid;
    }
    SceUID s = -1;
    if (slot >= 0) {
        if (threads[slot].wake <= 0)
            threads[slot].wake = sceKernelCreateSema("ee_sleep", 0, 0, 0x7fffffff, NULL);
        s = threads[slot].wake;
    }
    sceKernelUnlockLwMutex(&lock_work, 1);
    return s;
}

int SleepThread(void)
{
    sceKernelWaitSema(wake_sema(sceKernelGetThreadId()), 1, NULL);
    return GetThreadId();
}
int WakeupThread(int id) { return sceKernelSignalSema(wake_sema(id), 1) < 0 ? -1 : id; }
int iWakeupThread(int id) { return WakeupThread(id); }

/* log every EE thread's state (hang watchdog) */
void recvx_kernel_dump_threads(void)
{
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i].uid <= 0) continue;
        SceKernelThreadInfo info;
        info.size = sizeof(info);
        if (sceKernelGetThreadInfo(threads[i].uid, &info) < 0) continue;
        RECVX_LOG("thread %08x %s entry=%p status=%x prio=%d wait=%x/%08x pc=%p",
                  threads[i].uid, info.name, (void *)threads[i].entry, info.status,
                  to_ee_prio(info.currentPriority), info.waitType, info.waitId, info.entry);
    }
}

/* Only the ADX "safe" spinner was suspended, and the port doesn't create it. */
int SuspendThread(int id) { RECVX_LOG_ONCE("SuspendThread(%x) ignored", id); return id; }
int ResumeThread(int id) { RECVX_LOG_ONCE("ResumeThread(%x) ignored", id); return id; }

/* --------------------------------------------------------------- semaphores */

int CreateSema(struct SemaParam *p)
{
    int max = p->maxCount > 0 ? p->maxCount : 1;
    SceUID id = sceKernelCreateSema("ee_sema", 0, p->initCount, max, NULL);
    return id < 0 ? -1 : id;
}

int DeleteSema(int id) { return sceKernelDeleteSema(id) < 0 ? -1 : id; }
int SignalSema(int id) { return sceKernelSignalSema(id, 1) < 0 ? -1 : id; }
int iSignalSema(int id) { return SignalSema(id); }
int WaitSema(int id) { return sceKernelWaitSema(id, 1, NULL) < 0 ? -1 : id; }
int PollSema(int id) { return sceKernelPollSema(id, 1) < 0 ? -1 : id; }
int iPollSema(int id) { return PollSema(id); }

/* --------------------------------------------------------------- interrupts */

#define MAX_HANDLERS 4
static int (*intc_handlers[16][MAX_HANDLERS])(int);
static volatile u_int intc_mask;
static int (*vsync_callback)(int);

int AddIntcHandler(int cause, int (*handler)(int), int next)
{
    (void)next;
    for (int i = 0; i < MAX_HANDLERS; i++)
        if (!intc_handlers[cause][i]) { intc_handlers[cause][i] = handler; return i + 1; }
    return -1;
}

int RemoveIntcHandler(int cause, int id)
{
    if (id < 1 || id > MAX_HANDLERS) return -1;
    intc_handlers[cause][id - 1] = NULL;
    return 0;
}

int EnableIntc(int cause)  { int was = (intc_mask >> cause) & 1; intc_mask |= 1u << cause; return !was; }
int DisableIntc(int cause) { int was = (intc_mask >> cause) & 1; intc_mask &= ~(1u << cause); return was; }
void ExitHandler(void) {}

int (*sceGsSyncVCallback(int (*func)(int)))(int)
{
    int (*old)(int) = vsync_callback;
    vsync_callback = func;
    return old;
}

static void raise_intc(int cause)
{
    if (!(intc_mask & (1u << cause))) return;
    for (int i = 0; i < MAX_HANDLERS; i++)
        if (intc_handlers[cause][i]) intc_handlers[cause][i](cause);
}

static volatile u_int vblank_count;

static int vblank_thread(SceSize argc, void *argv)
{
    (void)argc; (void)argv;
    for (;;) {
        sceDisplayWaitVblankStart();
        vblank_count++;
        raise_intc(INTC_VBLANK_S);
        if (vsync_callback) vsync_callback(INTC_VBLANK_S);
        raise_intc(INTC_VBLANK_E);
    }
    return 0;
}

u_int recvx_vblank_count(void) { return vblank_count; }

/* ------------------------------------------------------------------- alarms */

/* SetAlarm counts H-blanks: 15734 Hz on NTSC */
#define HSYNC_US(n) ((SceUInt64)(n) * 63556 / 1000)
#define MAX_ALARMS 16

static struct {
    SceUInt64 due;
    void (*cb)(int, u_short, void *);
    void *arg;
    u_short time;
} alarms[MAX_ALARMS];
static SceUID alarm_sema, alarm_lock;

static int alarm_thread(SceSize argc, void *argv)
{
    (void)argc; (void)argv;
    for (;;) {
        SceUInt64 now = sceKernelGetProcessTimeWide(), next = ~0ull;
        sceKernelLockMutex(alarm_lock, 1, NULL);
        for (int i = 0; i < MAX_ALARMS; i++) {
            if (!alarms[i].cb) continue;
            if (alarms[i].due <= now) {
                void (*cb)(int, u_short, void *) = alarms[i].cb;
                void *arg = alarms[i].arg;
                u_short time = alarms[i].time;
                alarms[i].cb = NULL;
                sceKernelUnlockMutex(alarm_lock, 1);
                cb(i + 1, time, arg);
                sceKernelLockMutex(alarm_lock, 1, NULL);
            } else if (alarms[i].due < next) {
                next = alarms[i].due;
            }
        }
        sceKernelUnlockMutex(alarm_lock, 1);
        SceUInt timeout = next == ~0ull ? 1000000 : (SceUInt)(next - now);
        sceKernelWaitSema(alarm_sema, 1, &timeout);
    }
    return 0;
}

int SetAlarm(u_short time, void (*cb)(int, u_short, void *), void *arg)
{
    int id = -1;
    sceKernelLockMutex(alarm_lock, 1, NULL);
    for (int i = 0; i < MAX_ALARMS; i++) {
        if (alarms[i].cb) continue;
        alarms[i].due = sceKernelGetProcessTimeWide() + HSYNC_US(time);
        alarms[i].arg = arg;
        alarms[i].time = time;
        alarms[i].cb = cb;
        id = i + 1;
        break;
    }
    sceKernelUnlockMutex(alarm_lock, 1);
    sceKernelSignalSema(alarm_sema, 1);
    return id;
}

int iSetAlarm(u_short time, void (*cb)(int, u_short, void *), void *arg) { return SetAlarm(time, cb, arg); }

int ReleaseAlarm(int id)
{
    if (id < 1 || id > MAX_ALARMS) return -1;
    alarms[id - 1].cb = NULL;
    return id;
}

/* -------------------------------------------------------------------- cache */

/* DMA is serviced synchronously by the CPU, so there is nothing to keep coherent. */
void FlushCache(int op) { (void)op; }
void iFlushCache(int op) { (void)op; }
void SyncDCache(void *start, void *end) { (void)start; (void)end; }
void InvalidDCache(void *start, void *end) { (void)start; (void)end; }

/* ------------------------------------------------------------ ADXPS2 locking */

static SceKernelLwMutexWork adx_mutex;

void recvx_adx_lock(void) { sceKernelLockLwMutex(&adx_mutex, 1, NULL); }
void recvx_adx_unlock(void) { sceKernelUnlockLwMutex(&adx_mutex, 1); }

/* ------------------------------------------------------------------- start */

static void (*ee_main_entry)(void);

static int ee_main_tramp(SceSize argc, void *argv)
{
    (void)argc; (void)argv;
    ee_main_entry();
    sceKernelExitProcess(0);
    return 0;
}

void recvx_kernel_start(void (*entry)(void))
{
    sceKernelCreateLwMutex(&adx_mutex, "adxps2_lock", SCE_KERNEL_MUTEX_ATTR_RECURSIVE, 0, NULL);
    alarm_sema = sceKernelCreateSema("ee_alarm", 0, 0, 1, NULL);
    alarm_lock = sceKernelCreateMutex("ee_alarm_lock", 0, 0, NULL);

    SceUID t = sceKernelCreateThread("ee_vblank", vblank_thread, IRQ_PRIORITY, 0x4000, 0, GAME_CPU, NULL);
    sceKernelStartThread(t, 0, NULL);
    t = sceKernelCreateThread("ee_alarm", alarm_thread, IRQ_PRIORITY, 0x4000, 0, GAME_CPU, NULL);
    sceKernelStartThread(t, 0, NULL);

    /* the EE main thread starts at priority 1 */
    ee_main_entry = entry;
    t = sceKernelCreateThread("ee_main", ee_main_tramp, to_vita_prio(1), 1024 * 1024, 0, GAME_CPU, NULL);
    sceKernelStartThread(t, 0, NULL);
}
