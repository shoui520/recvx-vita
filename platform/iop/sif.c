/*
 * SIF: module loading, IOP heap, DMA and RPC against the native IOP.
 */
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/kernel/threadmgr.h>
#include "sif.h"
#include "sifrpc.h"
#include "sifdev.h"
#include "recvx_platform.h"
#include "iop.h"

uint8_t *iop_ram;
static uint32_t heap_top;
static SceKernelLwMutexWork lock;

/* low IOP memory belongs to the IOP kernel and modules; nothing hands out 0 */
#define HEAP_BASE 0x10000

void iop_lock(void) { sceKernelLockLwMutex(&lock, 1, NULL); }
void iop_unlock(void) { sceKernelUnlockLwMutex(&lock, 1); }

uint32_t iop_alloc(uint32_t size)
{
    uint32_t a = (heap_top + 63) & ~63u;
    if (a + size > IOP_RAM_SIZE) {
        RECVX_LOG("iop: out of memory allocating %u bytes", size);
        return 0;
    }
    heap_top = a + size;
    return a;
}

static void iop_boot(void)
{
    if (iop_ram) return;
    iop_ram = memalign(64, IOP_RAM_SIZE);
    memset(iop_ram, 0, IOP_RAM_SIZE);
    heap_top = HEAP_BASE;
    sceKernelCreateLwMutex(&lock, "iop", SCE_KERNEL_MUTEX_ATTR_RECURSIVE, 0, NULL);
    tsnddrv_init();
    cri_iop_init();
}

/* ---------------------------------------------------------------- modules */

int sceSifRebootIop(const char *img) { (void)img; iop_boot(); return 1; }
int sceSifSyncIop(void) { return 1; }
int sceSifLoadModule(const char *filename, int args, const char *argp) { (void)filename; (void)args; (void)argp; return 0; }

int sceSifInitIopHeap(void) { iop_boot(); return 0; }
void *sceSifAllocIopHeap(u_int size) { iop_boot(); return (void *)(uintptr_t)iop_alloc(size); }
int sceSifFreeIopHeap(void *addr) { (void)addr; return 0; }   /* allocations are made once at boot */

/* -------------------------------------------------------------------- DMA */

static u_int dma_xfer(sceSifDmaData *sdd, int len)
{
    static u_int id;
    iop_lock();
    for (int i = 0; i < len; i++) {
        memcpy(iop_ptr(sdd[i].addr), (const void *)(uintptr_t)sdd[i].data, sdd[i].size);
        cri_iop_dma(sdd[i].addr, sdd[i].size);
    }
    iop_unlock();
    if (++id == 0) id = 1;
    return id;
}

u_int sceSifSetDma(sceSifDmaData *sdd, int len) { return dma_xfer(sdd, len); }
u_int isceSifSetDma(sceSifDmaData *sdd, int len) { return dma_xfer(sdd, len); }
int sceSifDmaStat(u_int id) { (void)id; return -1; }   /* already complete */

int sceSifGetOtherData(sceSifReceiveData *rd, void *src, void *dest, int size, u_int mode)
{
    (void)rd; (void)mode;
    iop_lock();
    memcpy(dest, iop_ptr((uint32_t)(uintptr_t)src), size);
    iop_unlock();
    return 0;
}

/* -------------------------------------------------------------------- RPC */

#define MAX_SERVERS 8
static struct {
    unsigned id;
    iop_rpc_fn fn;
    sceSifServeData sd;
} servers[MAX_SERVERS];
static int nservers;

void iop_register_rpc(unsigned id, iop_rpc_fn fn)
{
    servers[nservers].id = id;
    servers[nservers].fn = fn;
    servers[nservers].sd.command = id;
    nservers++;
}

void sceSifInitRpc(u_int mode) { (void)mode; iop_boot(); }

int sceSifBindRpc(sceSifClientData *bd, u_int request, u_int mode)
{
    (void)mode;
    iop_boot();
    bd->command = request;
    bd->serve = NULL;
    for (int i = 0; i < nservers; i++)
        if (servers[i].id == request) bd->serve = &servers[i].sd;
    if (!bd->serve) RECVX_LOG("sif: no IOP server for rpc 0x%08x", request);
    return 0;
}

int sceSifCallRpc(sceSifClientData *bd, u_int fno, u_int mode, void *send, int ssize,
                  void *receive, int rsize, sceSifEndFunc end_func, void *end_para)
{
    (void)mode;
    iop_rpc_fn fn = NULL;
    for (int i = 0; i < nservers; i++)
        if (&servers[i].sd == bd->serve) fn = servers[i].fn;
    if (!fn) return -1;

    /* the server works on its own receive buffer, as on the IOP */
    static uint32_t buf[2048 / 4];
    iop_lock();
    if (ssize > (int)sizeof(buf)) ssize = sizeof(buf);
    if (send && ssize > 0) memcpy(buf, send, ssize);
    void *ret = fn(fno, buf, ssize);
    if (receive && rsize > 0 && ret) memcpy(receive, ret, rsize);
    iop_unlock();

    if (end_func) end_func(end_para);
    return 0;
}

int sceSifCheckStatRpc(sceSifRpcData *cd) { (void)cd; return 0; }

/* EE-side servers (DTX registers one for IOP->EE calls the native IOP never makes) */
void sceSifSetRpcQueue(sceSifQueueData *q, int thread_id) { (void)thread_id; memset(q, 0, sizeof(*q)); }

void sceSifRegisterRpc(sceSifServeData *serve, u_int command, sceSifRpcFunc func, void *buff,
                       sceSifRpcFunc cfunc, void *cbuff, sceSifQueueData *qd)
{
    (void)qd;
    serve->command = command;
    serve->func = func;
    serve->buff = buff;
    serve->cfunc = cfunc;
    serve->cbuff = cbuff;
}

void sceSifRpcLoop(sceSifQueueData *qd)
{
    (void)qd;
    for (;;) sceKernelDelayThread(1000000);
}
