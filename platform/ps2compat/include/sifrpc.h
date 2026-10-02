/* SIF RPC: EE<->IOP remote calls, served in-process by the port's IOP replacements. */
#pragma once
#include "eetypes.h"
#include "sif.h"

typedef void (*sceSifEndFunc)(void *);
typedef void *(*sceSifRpcFunc)(u_int fno, void *data, int size);

typedef struct { u_int psize:8, dsize:24; u_int daddr; u_int fid; u_int fcode; } sceSifCmdHdr;

typedef struct _sif_rpc_data {
    void *paddr; u_int pid; int tid; u_int mode;
} sceSifRpcData;

typedef struct _sif_serve_data sceSifServeData;
typedef struct _sif_client_data {
    sceSifRpcData rpcd;
    u_int command;
    void *buff;
    void *cbuff;
    sceSifEndFunc func;
    void *para;
    sceSifServeData *serve;
} sceSifClientData;

typedef struct _sif_receive_data {
    sceSifRpcData rpcd;
    void *src; void *dest; int size;
} sceSifReceiveData;

typedef struct _sif_queue_data sceSifQueueData;
struct _sif_serve_data {
    u_int command;
    sceSifRpcFunc func;
    void *buff;
    int size;
    sceSifRpcFunc cfunc;
    void *cbuff;
    int csize;
    sceSifClientData *client;
    void *paddr;
    u_int fno;
    void *receive;
    int rsize;
    int rmode;
    u_int rid;
    sceSifServeData *link;
    sceSifServeData *next;
    sceSifQueueData *base;
};
struct _sif_queue_data {
    int key; int active;
    sceSifServeData *link, *start, *end;
    sceSifQueueData *next;
};

#define SIF_RPCM_NOWAIT 0x01
#define SIF_RPCM_NOWBDC 0x02

void sceSifInitRpc(u_int mode);
int  sceSifBindRpc(sceSifClientData *bd, u_int request, u_int mode);
int  sceSifCallRpc(sceSifClientData *bd, u_int fno, u_int mode, void *send, int ssize,
                   void *receive, int rsize, sceSifEndFunc end_func, void *end_para);
int  sceSifCheckStatRpc(sceSifRpcData *cd);
void sceSifSetRpcQueue(sceSifQueueData *q, int thread_id);
void sceSifRegisterRpc(sceSifServeData *serve, u_int command, sceSifRpcFunc func, void *buff,
                       sceSifRpcFunc cfunc, void *cbuff, sceSifQueueData *qd);
void sceSifRpcLoop(sceSifQueueData *qd);
int  sceSifGetOtherData(sceSifReceiveData *rd, void *src, void *dest, int size, u_int mode);
