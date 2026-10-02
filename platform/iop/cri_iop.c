/*
 * The IOP half of CRI's ADX output path, and the audio thread that plays it.
 *
 * ADX decodes on the EE. For each output channel, DTR DMAs the PCM into a
 * 16 KB IOP buffer and SJX forwards the chunk descriptors through a DTX
 * mailbox to an IOP stream joint (SJRMT). PS2RNA here plays the data lane
 * of those joints and puts consumed chunks back on the free lane, and the
 * SJX reply returns them to the EE. Chunk addresses are IOP addresses.
 *
 * DTX mailbox: the EE DMAs its whole work area (commands + footer) to the
 * IOP. The server answers in the EE work area and raises the footer's
 * ticket, which the EE's DTX_ExecHndl takes as "reply ready".
 */
#include <stdlib.h>
#include <string.h>
#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>
#include "sj_uni.h"
#include "sjx.h"
#include "ps2_rna.h"
#include "sjr_clt.h"
#include "dtx.h"
#include "recvx_platform.h"
#include "iop.h"
#include "hsyn.h"

#define DTX_ID 0x7D000000
#define OUT_RATE 48000
#define OUT_GRAIN 256

/* ------------------------------------------------------------ DTX mailbox */

typedef struct {
    int used, id;
    Sint8 *eewk;          /* EE work area: replies go here */
    uint32_t iopwk;       /* IOP work area: requests land here */
    int wklen;
} IopDtx;
static IopDtx dtx[8];

/* ------------------------------------------------------------------- SJX */

typedef struct {
    int used;
    SJ sjdst;             /* IOP joint */
    int lin;
    void *eesjx;          /* the EE's SJX object, named in replies */
    int16_t xid;
} IopSjx;
static IopSjx sjx[16];

static void sjx_receive(const SJX_DTXFMT *in, SJX_DTXFMT *out)
{
    for (int i = 0; i < in->ncmd && i < 128; i++) {
        const SJX_DTXCMD *c = &in->cmd[i];
        IopSjx *x = (IopSjx *)c->sj;
        if (c->no != SJX_CMD_PUT_CHUNK || !x || !x->used || c->xid != x->xid) continue;
        SJCK ck = c->ck;
        SJ_PutChunk(x->sjdst, c->lin, &ck);
    }
    /* return what the IOP side has finished with */
    int n = 0;
    for (int i = 0; i < 16 && n < 128; i++) {
        IopSjx *x = &sjx[i];
        if (!x->used) continue;
        while (n < 128) {
            SJCK ck;
            SJ_GetChunk(x->sjdst, SJ_LIN_FREE, SJCK_LEN_MAX, &ck);
            if (ck.len == 0) break;
            out->cmd[n].no = SJX_CMD_PUT_CHUNK;
            out->cmd[n].lin = SJ_LIN_FREE;
            out->cmd[n].xid = x->xid;
            out->cmd[n].sj = x->eesjx;
            out->cmd[n].ck = ck;
            n++;
        }
    }
    out->ncmd = n;
}

/* ------------------------------------------------------------------- RNA */

typedef struct {
    int used;
    int nch, plysw, sfreq, vol;  /* vol: 0..256 */
    SJ sj[2];
    SJCK cur[2];                 /* chunk being played, per channel */
    int pos[2];
    uint32_t phase;              /* 16.16 resampler position */
    int16_t s0[2], s1[2];
} IopRna;
static IopRna rna[8];

static void rna_drop(IopRna *r, int ch)
{
    r->cur[ch].data = NULL;
    r->cur[ch].len = 0;
    r->pos[ch] = 0;
}

static void rna_receive(const PS2RNA_DTXFMT *in, PS2RNA_DTXFMT *out)
{
    for (int i = 0; i < in->ncmd && i < 128; i++) {
        const PS2RNA_DTXCMD *c = &in->cmd[i];
        IopRna *r = (IopRna *)c->rna;
        if (!r || !r->used) continue;
        switch (c->no) {
        case IOPRNA_CMD_SETPSW: r->plysw = c->arg1; break;
        case IOPRNA_CMD_SETNCH: r->nch = c->arg1 > 2 ? 2 : c->arg1; break;
        case IOPRNA_CMD_SETSFREQ: r->sfreq = c->arg1; break;
        case IOPRNA_CMD_SETVOL: r->vol = c->arg2; break;
        }
    }
    out->ncmd = 0;
}

/* next 16-bit sample of a channel, or 0 if the joint has run dry */
static int rna_fetch(IopRna *r, int ch, int16_t *s)
{
    if (r->pos[ch] >= r->cur[ch].len) {
        if (r->cur[ch].data) SJ_PutChunk(r->sj[ch], SJ_LIN_FREE, &r->cur[ch]);
        rna_drop(r, ch);
        SJCK ck;
        SJ_GetChunk(r->sj[ch], SJ_LIN_DATA, SJCK_LEN_MAX, &ck);
        if (ck.len < 2) {
            if (ck.len) SJ_UngetChunk(r->sj[ch], SJ_LIN_DATA, &ck);
            return 0;
        }
        r->cur[ch] = ck;
    }
    *s = *(int16_t *)iop_ptr((uint32_t)(uintptr_t)r->cur[ch].data + r->pos[ch]);
    r->pos[ch] += 2;
    return 1;
}

/* mix one RNA into out[] (stereo, n frames) */
static void rna_mix(IopRna *r, int32_t *out, int n)
{
    int nch = r->nch < 1 ? 1 : r->nch;
    uint32_t step = (uint32_t)(((uint64_t)(r->sfreq > 0 ? r->sfreq : OUT_RATE) << 16) / OUT_RATE);
    for (int i = 0; i < n; i++) {
        while (r->phase >= 0x10000) {
            int16_t s[2];
            for (int ch = 0; ch < nch; ch++)
                if (!rna_fetch(r, ch, &s[ch])) return;   /* underrun: rest is silence */
            for (int ch = 0; ch < nch; ch++) {
                r->s0[ch] = r->s1[ch];
                r->s1[ch] = s[ch];
            }
            r->phase -= 0x10000;
        }
        int f = r->phase >> 1;   /* 15-bit fraction */
        int32_t l = r->s0[0] + (((r->s1[0] - r->s0[0]) * f) >> 15);
        int32_t rr = l;
        if (nch > 1) rr = r->s0[1] + (((r->s1[1] - r->s0[1]) * f) >> 15);
        out[i * 2] += l * r->vol >> 8;
        out[i * 2 + 1] += rr * r->vol >> 8;
        r->phase += step;
    }
}

static int audio_thread(SceSize args, void *argp)
{
    (void)args; (void)argp;
    int port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, OUT_GRAIN, OUT_RATE, SCE_AUDIO_OUT_MODE_STEREO);
    if (port < 0) {
        RECVX_LOG("audio: can't open output port (0x%08x)", port);
        return 0;
    }
    static int16_t pcm[2][OUT_GRAIN * 2];
    int32_t mix[OUT_GRAIN * 2];
    for (int buf = 0;; buf ^= 1) {
        memset(mix, 0, sizeof(mix));
        iop_lock();
        for (int i = 0; i < 8; i++)
            if (rna[i].used && rna[i].plysw) rna_mix(&rna[i], mix, OUT_GRAIN);
        /* the sound driver ticks at 240 Hz between synth slices */
        static int tick_pos;
        for (int done = 0; done < OUT_GRAIN;) {
            int n = OUT_RATE / 240 - tick_pos;
            if (n > OUT_GRAIN - done) n = OUT_GRAIN - done;
            hsyn_mix(mix + done * 2, n);
            done += n;
            tick_pos += n;
            if (tick_pos == OUT_RATE / 240) {
                tick_pos = 0;
                tsnddrv_tick();
            }
        }
        iop_unlock();
        for (int i = 0; i < OUT_GRAIN * 2; i++)
            pcm[buf][i] = mix[i] > 32767 ? 32767 : mix[i] < -32768 ? -32768 : mix[i];
        sceAudioOutOutput(port, pcm[buf]);
    }
    return 0;
}

/* ------------------------------------------------------------ URPC server */

static void urpc(unsigned fno, int32_t *a)
{
    switch (fno) {
    case SJX_DTXFNO_CREATE: {
        int i;
        for (i = 0; i < 16 && sjx[i].used; i++);
        if (i == 16) { a[0] = 0; break; }
        sjx[i] = (IopSjx){ .used = 1, .sjdst = (SJ)a[1], .lin = a[2], .eesjx = (void *)a[3] };
        a[0] = (int32_t)&sjx[i];
        break;
    }
    case SJX_DTXFNO_DESTROY: ((IopSjx *)a[0])->used = 0; break;
    case SJX_DTXFNO_RESET: ((IopSjx *)a[0])->xid = (int16_t)a[1]; break;

    case PS2RNA_DTXFNO_CREATE: {
        int i;
        for (i = 0; i < 8 && rna[i].used; i++);
        if (i == 8) { a[0] = 0; break; }
        IopRna *r = &rna[i];
        memset(r, 0, sizeof(*r));
        r->used = 1;
        r->nch = a[0] > 2 ? 2 : a[0];
        r->sj[0] = (SJ)a[2];
        r->sj[1] = (SJ)a[3];
        r->sfreq = OUT_RATE;
        r->vol = 256;
        a[0] = (int32_t)r;
        break;
    }
    case PS2RNA_DTXFNO_DESTROY: ((IopRna *)a[0])->used = 0; break;

    case SJRMT_UNI_CREATE:
        a[0] = (int32_t)SJUNI_Create(a[0], iop_ptr(a[1]), a[2]);
        break;
    case SJRMT_RBF_CREATE:
    case SJRMT_MEM_CREATE:
        RECVX_LOG("cri_iop: remote RBF/MEM joints are not supported");
        a[0] = 0;
        break;
    case SJRMT_DESTROY: SJ_Destroy((SJ)a[0]); break;
    case SJRMT_GET_UUID: memcpy(a, SJ_GetUuid((SJ)a[0]), 16); break;
    case SJRMT_RESET: {
        SJ sj = (SJ)a[0];
        SJ_Reset(sj);
        /* a reset joint forgets its chunks; so does whoever was playing it */
        for (int i = 0; i < 8; i++)
            for (int ch = 0; ch < 2; ch++)
                if (rna[i].used && rna[i].sj[ch] == sj) rna_drop(&rna[i], ch);
        break;
    }
    case SJRMT_GET_CHUNK: {
        SJCK ck;
        SJ_GetChunk((SJ)a[0], a[1], a[2], &ck);
        a[0] = (int32_t)ck.data;
        a[1] = ck.len;
        break;
    }
    case SJRMT_UNGET_CHUNK:
    case SJRMT_PUT_CHUNK: {
        SJCK ck = { (Sint8 *)a[2], a[3] };
        if (fno == SJRMT_PUT_CHUNK) SJ_PutChunk((SJ)a[0], a[1], &ck);
        else SJ_UngetChunk((SJ)a[0], a[1], &ck);
        break;
    }
    case SJRMT_GET_NUM_DATA: {
        SJ sj = (SJ)a[0];
        int lin = a[1];
        int32_t n = SJ_GetNumData(sj, lin);
        /* bytes the player holds still count as data in flight */
        for (int i = 0; i < 8; i++)
            for (int ch = 0; ch < 2; ch++)
                if (rna[i].used && rna[i].sj[ch] == sj && rna[i].cur[ch].data)
                    n += lin == SJ_LIN_DATA ? rna[i].cur[ch].len - rna[i].pos[ch] : 0;
        a[0] = n;
        break;
    }
    case SJRMT_IS_GET_CHUNK: {
        Sint32 rbyte = 0;
        a[0] = SJ_IsGetChunk((SJ)a[0], a[1], a[2], &rbyte);
        a[1] = rbyte;
        break;
    }
    case SJRMT_INIT:
    case SJRMT_FINISH:
        break;
    default:
        RECVX_LOG("cri_iop: unknown urpc %u", fno);
        break;
    }
}

static void *dtx_rpc(unsigned fno, void *data, int size)
{
    (void)size;
    int32_t *a = data;
    switch (fno) {
    case 0:
    case 1:
        break;
    case 2: {   /* create: id, eewk, iopwk, wklen */
        unsigned id = a[0];
        if (id >= 8) { a[0] = 0; break; }
        dtx[id] = (IopDtx){ .used = 1, .id = id, .eewk = (Sint8 *)a[1], .iopwk = a[2], .wklen = a[3] };
        a[0] = (int32_t)&dtx[id];
        break;
    }
    case 3:
        if (a[0]) ((IopDtx *)a[0])->used = 0;
        break;
    default:
        if (fno >= 1024 && fno < 1088) urpc(fno - 1024, a);
        break;
    }
    return data;
}

void cri_iop_dma(uint32_t addr, uint32_t size)
{
    for (int i = 0; i < 8; i++) {
        IopDtx *d = &dtx[i];
        if (!d->used || d->iopwk != addr || (int)size < d->wklen) continue;
        const void *in = iop_ptr(addr);
        int dtlen = d->wklen - 64;
        const DTX_FOOTER *fin = (const DTX_FOOTER *)((const Sint8 *)in + dtlen);
        if (d->id == 0) sjx_receive(in, (SJX_DTXFMT *)d->eewk);
        else if (d->id == 1) rna_receive(in, (PS2RNA_DTXFMT *)d->eewk);
        ((DTX_FOOTER *)(d->eewk + dtlen))->ticket_no = fin->ticket_no + 1;
    }
}

void cri_iop_init(void)
{
    iop_register_rpc(DTX_ID, dtx_rpc);
    SceUID th = sceKernelCreateThread("recvx_audio", audio_thread, 0x40, 0x4000, 0, SCE_KERNEL_CPU_MASK_USER_1, NULL);
    if (th >= 0) sceKernelStartThread(th, 0, NULL);
}
