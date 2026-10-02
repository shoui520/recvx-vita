/*
 * TSNDDRV, the game's IOP sound driver (hardware-synth MIDI and SE banks).
 *
 * RPC dev 0 takes a byte stream of commands built by ps2_snddrv.c; dev 1
 * answers state queries. Bank uploads arrive through a shared IOP buffer:
 * the EE DMAs a piece there, then sends a "set" command naming the bank.
 * The driver keeps every bank (HD headers, BD samples, SQ sequences) so the
 * synth can play them; the HD checksum it reports back is what the EE
 * waits for before sending the samples.
 */
#include <stdlib.h>
#include <string.h>
#include "recvx_platform.h"
#include "iop.h"
#include "hsyn.h"
#include "hseq.h"

typedef struct {
    uint16_t se_info[6];
    uint16_t midi_info;
    int16_t port_info[8];
    int16_t midi_sum[4];
    int16_t se_sum[5];
    uint16_t dummy[9];
} SndStatus;   /* SND_STATUS, 0x42 bytes */

#define DATA_BUF_SIZE (204800 + 49152)   /* two BD transfer slots */
#define HD_AREA_SIZE  29952               /* sum of IOP_hd_size[] */
#define SQ_AREA_SIZE  11776               /* sum of IOP_tq_size[] */

typedef struct { uint8_t *data; uint32_t size; } Blob;

static uint32_t status_addr, table_addr, data_addr;
static Blob midi_hd[4], midi_bd[4], se_hd[5], sq[8], bd[128];
static int last_bd_slot = -1;

/* ------------------------------------------------------------------ SE (TQ)
 * Each SE port has 16 channels; a request keys program `se` on the SE bank's
 * synth port at the sample's base note and runs a 240 Hz timer for one-shots.
 * The EE sees a channel as busy while its bit is set in status->se_info[port].
 */
#define SE_PORTS 6
#define SE_SYNTH(port) (10 + (port))
enum { TQ_ACTIVE = 1, TQ_CANCEL = 2, TQ_START = 4, TQ_VOL = 0x20, TQ_PAN = 0x40, TQ_PITCH = 0x80 };

typedef struct {
    uint32_t flags;
    uint16_t timer;            /* 0xffff: until cancelled */
    uint8_t se, note, vol, pan;
    uint16_t pitch;
} TqSlot;

typedef struct {
    uint16_t dur;              /* 240 Hz ticks, 0xffff for looping samples */
    uint8_t note, valid;
} SeEntry;

static TqSlot tq[SE_PORTS][16];
static uint16_t tq_busy[SE_PORTS];
static SeEntry *se_tbl[SE_PORTS];
static int se_max[SE_PORTS] = { -1, -1, -1, -1, -1, -1 };

static void se_build(int port)
{
    const uint8_t *hd = hsyn_bank_hd(SE_SYNTH(port));
    free(se_tbl[port]);
    se_tbl[port] = NULL;
    se_max[port] = hsyn_hd_max_program(hd);
    if (se_max[port] < 0) return;
    se_tbl[port] = calloc(se_max[port] + 1, sizeof(SeEntry));
    for (int i = 0; i <= se_max[port]; i++) {
        HsynSampleInfo si;
        SeEntry *e = &se_tbl[port][i];
        if (!hsyn_hd_program_sample(hd, 0xffffffff, i, &si) || !si.rate) continue;
        e->valid = 1;
        e->note = si.base_note;
        if (si.loop) e->dur = 0xffff;
        else {
            uint32_t d = 420u * si.bd_size / si.rate + 7;   /* 28 samples per 16 bytes, 240 Hz */
            e->dur = d > 0xfffe ? 0xfffe : d;
        }
    }
}

static void tq_req(int port, int se, int ch, int vol, int pan, int pitch)
{
    if (port >= SE_PORTS || ch > 15 || se > se_max[port] || !se_tbl[port] || !se_tbl[port][se].valid) return;
    TqSlot *t = &tq[port][ch];
    t->se = se;
    t->note = se_tbl[port][se].note;
    t->vol = vol;
    t->pan = pan;
    t->pitch = pitch;
    t->timer = se_tbl[port][se].dur;
    /* a busy channel is cut first (TQ_CANCEL), then restarted */
    t->flags = (t->flags & TQ_ACTIVE ? TQ_CANCEL : 0) | TQ_ACTIVE | TQ_START | TQ_VOL | TQ_PAN | TQ_PITCH;
    tq_busy[port] |= 1 << ch;
}

static void tq_chg(int port, int se, int ch, int vol, int pan, int pitch)
{
    if (port >= SE_PORTS) return;
    for (int c = 15; c >= 0; c--) {
        TqSlot *t = &tq[port][c];
        if (!(t->flags & TQ_ACTIVE) || t->se != se || c != ch) continue;
        if (vol >= 0) { t->vol = vol; t->flags |= TQ_VOL; }
        if (pan >= 0) { t->pan = pan; t->flags |= TQ_PAN; }
        if (pitch >= 0) { t->pitch = pitch; t->flags |= TQ_PITCH; }
        return;
    }
}

static void tq_cancel(int port, int ch)
{
    if (port < SE_PORTS && ch < 16 && (tq[port][ch].flags & TQ_ACTIVE)) tq[port][ch].flags |= TQ_CANCEL;
}

static void tq_port_stop(int port)
{
    for (int c = 0; c < 16; c++) {
        hsyn_sound_off(SE_SYNTH(port), c);
        tq[port][c].flags = 0;
    }
    tq_busy[port] = 0;
}

static void tq_end(int port, int ch)
{
    hsyn_sound_off(SE_SYNTH(port), ch);
    tq[port][ch].flags = 0;
    tq_busy[port] &= ~(1 << ch);
}

/* TsndTQ_ATick */
static void tq_tick(void)
{
    for (int p = 0; p < SE_PORTS; p++) {
        int sp = SE_SYNTH(p);
        for (int c = 0; c < 16; c++) {
            TqSlot *t = &tq[p][c];
            if (!(tq_busy[p] & 1 << c)) continue;
            if (t->flags & TQ_CANCEL) {
                hsyn_sound_off(sp, c);
                t->flags &= ~TQ_CANCEL;
                if (!(t->flags & TQ_START)) { tq_end(p, c); continue; }
            }
            if (t->flags & TQ_START) {
                hsyn_volume(sp, c, 127);
                hsyn_program(sp, c, t->se);
                hsyn_note_on(sp, c, t->note, 127);
                t->flags &= ~TQ_START;
            }
            if (t->timer != 0xffff) {
                int dec = t->pitch >> 9;
                t->timer = t->timer > dec ? t->timer - dec : 0;
                if (!t->timer) { tq_end(p, c); continue; }
            }
            if (t->flags & TQ_VOL) {
                int v = t->vol * 127 / 127;
                hsyn_volume(sp, c, v > 127 ? 127 : v);
            }
            if (t->flags & TQ_PAN) {
                int v = t->pan - 64 + 64;
                hsyn_pan(sp, c, v < 0 ? 0 : v > 127 ? 127 : v);
            }
            if (t->flags & TQ_PITCH) {
                int v = 0x2000 + t->pitch - 8192;
                hsyn_bend(sp, c, v < 0 ? 0 : v > 16383 ? 16383 : v);
            }
            t->flags &= ~(TQ_VOL | TQ_PAN | TQ_PITCH);
        }
    }
}

static SndStatus *status(void) { return iop_ptr(status_addr); }

static void blob_set(Blob *b, const void *src, uint32_t off, uint32_t len)
{
    if (off + len > b->size) {
        b->data = realloc(b->data, off + len);
        memset(b->data + b->size, 0, off + len - b->size);
        b->size = off + len;
    }
    memcpy(b->data + off, src, len);
}

static int16_t hd_sum(const int8_t *p, uint32_t n)
{
    int16_t s = 0;
    for (uint32_t i = 0; i < n; i++) s += p[i];
    return s;
}

/* length of one queued command, mirroring sending_req() in ps2_snddrv.c */
static int cmd_len(const uint8_t *p)
{
    int cd = p[0];
    switch (cd & 0xf0) {
    case 0x00: return 4 + (cd & 1) + (cd >> 1 & 1) + (cd & 4 ? 2 : 0);
    case 0x10: return cd == 0x11 ? 3 : 1;
    case 0x20:
        if (cd >= 0x22 && cd <= 0x25) return 3;
        if (cd == 0x26) return 4;
        if (cd == 0x20) return 5;
        if (cd == 0x27 || cd == 0x28 || cd == 0x29 || cd == 0x2c || cd == 0x2d) return 8;
        return 2;
    case 0x40:
        if ((cd >= 0x47 && cd <= 0x4a) || cd == 0x41 || cd == 0x42) return 2;
        if (cd == 0x4b) return 3;
        if (cd == 0x45 || cd == 0x4c) return 4;
        if (cd == 0x44 || cd == 0x4f) return 6;
        if (cd == 0x4d || cd == 0x4e) return 3;
        return 1;
    case 0x50:
    case 0x60:
        return (cd >= 0x51 && cd <= 0x54) ? 8 : 2;
    }
    return 0;
}

static uint32_t le32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }

static uint8_t *blob_dup(const Blob *b)
{
    uint8_t *p = malloc(b->size ? b->size : 1);
    if (b->size) memcpy(p, b->data, b->size);
    return p;
}

/* ------------------------------------------------------------------ BGM
 * BgmReq loads MIDI bank `bank` (HD, BD, SQ) on synth port `port` and plays
 * one MIDI block of its SQ. status->midi_info keeps the port's bit until the
 * block ends or is stopped.
 */
typedef struct {
    int bank;                  /* MIDI bank loaded on the port, -1 none */
    int on, paused;
} Bgm;

static Bgm bgm[HSEQ_PORTS] = { [0 ... HSEQ_PORTS - 1] = { -1, 0, 0 } };

static void bgm_stop(int port)
{
    hseq_stop(port);
    bgm[port].on = 0;
    bgm[port].paused = 0;
}

/* a MIDI bank is about to change: nothing may keep pointing into it */
static void bgm_detach(int bank)
{
    for (int p = 0; p < HSEQ_PORTS; p++) {
        if (bgm[p].bank != bank) continue;
        bgm_stop(p);
        hsyn_bank_ref(p, NULL, 0, NULL, 0);
        bgm[p].bank = -1;
    }
}

static void bgm_req(int port, int bank, int vol, int block)
{
    if (port >= HSEQ_PORTS || bank >= 4 || !midi_hd[bank].size || !midi_bd[bank].size) return;
    bgm_stop(port);
    if (bgm[port].bank != bank) {
        hsyn_bank_ref(port, midi_hd[bank].data, midi_hd[bank].size, midi_bd[bank].data, midi_bd[bank].size);
        bgm[port].bank = bank;
    }
    hsyn_port_volume(port, vol);
    hsyn_port_pan(port, 64);
    hsyn_port_bend(port, 0x2000);
    if (!hseq_start(port, sq[bank].data, sq[bank].size, block)) {
        RECVX_LOG_ONCE("tsnddrv: BGM bank %d has no MIDI block %d", bank, block);
        return;
    }
    bgm[port].on = 1;
}

/* SdrBDDataSet/SdrBDDataSet2: the samples uploaded last belong to this bank */
static void bank_bind(int synth_port, const Blob *hd)
{
    if (last_bd_slot < 0 || !hd->size) return;
    Blob *b = &bd[last_bd_slot];
    hsyn_bank_set(synth_port, blob_dup(hd), hd->size, b->data, b->size);
    b->data = NULL;
    b->size = 0;
    last_bd_slot = -1;
}

static void do_cmd(const uint8_t *p)
{
    const uint8_t *buf = iop_ptr(data_addr);
    int port = p[1];
    switch (p[0]) {
    case 0x28:   /* SdrHDDataSet: MIDI program bank header */
    case 0x29: { /* SdrHDDataSet2: SE bank header */
        uint32_t size = le32(p + 4);
        if (size > DATA_BUF_SIZE) break;
        int16_t sum = hd_sum((const int8_t *)buf, size);
        if (p[0] == 0x28 && port < 4) {
            bgm_detach(port);
            blob_set(&midi_hd[port], buf, 0, size);
            midi_hd[port].size = size;
            status()->midi_sum[port] = sum;
        } else if (p[0] == 0x29 && port < 5) {
            blob_set(&se_hd[port], buf, 0, size);
            se_hd[port].size = size;
            status()->se_sum[port] = sum;
        }
        break;
    }
    case 0x2a:   /* SdrBDDataSet: MIDI bank complete */
        if (port < 4 && last_bd_slot >= 0) {
            bgm_detach(port);
            free(midi_bd[port].data);
            midi_bd[port] = bd[last_bd_slot];
            bd[last_bd_slot] = (Blob){ 0 };
            last_bd_slot = -1;
        }
        break;
    case 0x2b:   /* SdrBDDataSet2: SE bank complete */
        if (port < 5) {
            tq_port_stop(port);
            bank_bind(SE_SYNTH(port), &se_hd[port]);
            se_build(port);
        }
        break;
    case 0x2c: { /* SdrBDDataTrans: a piece of a bank's sample data */
        int flag = p[1] >> 7;
        port = p[1] & 0x7f;
        uint32_t adrs = p[2] | p[3] << 8 | p[4] << 16;
        uint32_t size = p[5] | p[6] << 8 | p[7] << 16;
        if (adrs == 0) bd[port].size = 0;   /* a new bank starts over */
        if (size <= 49152) blob_set(&bd[port], buf + (flag ? 204800 : 0), adrs, size);
        last_bd_slot = port;
        break;
    }
    case 0x2d: { /* SdrSQDataSet */
        uint32_t size = le32(p + 4);
        if (port < 8 && size <= DATA_BUF_SIZE) {
            blob_set(&sq[port], buf, 0, size);
            sq[port].size = size;
        }
        break;
    }
    case 0x10:   /* SdrSeAllStop */
        for (int i = 0; i < SE_PORTS; i++) tq_port_stop(i);
        break;
    case 0x11:   /* SE master volume */
        for (int i = 0; i < SE_PORTS; i++) hsyn_port_master(SE_SYNTH(i), p[1] << 8 | p[2]);
        break;
    case 0x20:   /* SdrBgmReq: port bank vol block */
        bgm_req(port, p[2], p[3], p[4]);
        break;
    case 0x21:   /* SdrBgmStop */
        if (port < HSEQ_PORTS) bgm_stop(port);
        break;
    case 0x22:   /* BGM volume */
        if (port < HSEQ_PORTS && bgm[port].on) hsyn_port_volume(port, p[2]);
        break;
    case 0x23:   /* BGM master volume */
        for (int i = 0; i < HSEQ_PORTS; i++) hsyn_port_master(i, p[1] << 8 | p[2]);
        break;
    case 0x24:   /* BGM pause: 1 pause, 2 resume, 0 toggle */
        if (port < HSEQ_PORTS && bgm[port].on) {
            int pause = p[2] == 0 ? !bgm[port].paused : p[2] == 1;
            bgm[port].paused = pause;
            hseq_pause(port, pause);
        }
        break;
    case 0x25:   /* BGM pan */
        if (port < HSEQ_PORTS && bgm[port].on) hsyn_port_pan(port, p[2]);
        break;
    case 0x26:   /* BGM pitch: lo hi, 7 bits each */
        if (port < HSEQ_PORTS && bgm[port].on) hsyn_port_bend(port, p[3] << 7 | p[2]);
        break;
    case 0x27:   /* SdrBgmChg: port - - vol pan lo hi */
        if (port < HSEQ_PORTS && bgm[port].on) {
            hsyn_port_volume(port, p[4]);
            hsyn_port_pan(port, p[5]);
            hsyn_port_bend(port, p[7] << 7 | p[6]);
        }
        break;
    case 0x4b:   /* SdrMasterVol */
        hsyn_master_volume((p[1] << 8 | p[2]) >> 1);
        break;
    default:
        if (p[0] < 0x10) {   /* SE request/change/cancel: cd port se ch [vol] [pan] [pitch.be] */
            int cd = p[0], se = p[2], ch = p[3] & 0x7f, i = 4;
            port = p[1] & 0x7f;
            int vol = -1, pan = -1, pitch = -1;
            if (cd & 1) vol = p[i++];
            if (cd & 2) pan = p[i++];
            if (cd & 4) { pitch = p[i] << 8 | p[i + 1]; i += 2; }
            if (cd == 8) tq_cancel(port, ch);
            else if (cd & 8) tq_chg(port, se, ch, vol, pan, pitch);
            else tq_req(port, se, ch, vol < 0 ? 127 : vol, pan < 0 ? 64 : pan, pitch < 0 ? 8192 : pitch);
        }
        /* reverb and the rest: not wired up yet */
        break;
    }
}

static void *rpc_send(unsigned fno, void *data, int size)
{
    (void)fno;
    const uint8_t *p = data, *end = p + size;
    while (p < end && *p != 0xff) {
        int n = cmd_len(p);
        if (n == 0) {
            RECVX_LOG("tsnddrv: unknown command 0x%02x", *p);
            break;
        }
        do_cmd(p);
        p += n;
    }
    return data;
}

static void *rpc_get(unsigned fno, void *data, int size)
{
    (void)size;
    int32_t *arg = data;
    switch (fno) {
    case 18: arg[0] = (int32_t)status_addr; break;
    case 19: arg[0] = (int32_t)table_addr; break;
    case 16: break;   /* sdDrvInit sends 2048 and never reads the reply */
    default:
        RECVX_LOG_ONCE("tsnddrv: unhandled state query %u", fno);
        arg[0] = 0;
        break;
    }
    return data;
}

/* 240 Hz driver tick (ATick), run from the audio thread with iop_lock held */
void tsnddrv_tick(void)
{
    tq_tick();
    hseq_tick();
    uint16_t midi = 0;
    for (int p = 0; p < HSEQ_PORTS; p++) {
        if (bgm[p].on && !bgm[p].paused && !hseq_playing(p)) bgm[p].on = 0;
        if (bgm[p].on) midi |= 1 << p;
    }
    SndStatus *st = status();
    st->midi_info = midi;
    for (int p = 0; p < SE_PORTS; p++) st->se_info[p] = tq_busy[p];
}

void tsnddrv_init(void)
{
    status_addr = iop_alloc(sizeof(SndStatus));
    table_addr = iop_alloc(64);
    data_addr = iop_alloc(DATA_BUF_SIZE);
    int32_t *t = iop_ptr(table_addr);
    t[0] = (int32_t)iop_alloc(HD_AREA_SIZE);
    t[1] = (int32_t)iop_alloc(SQ_AREA_SIZE);
    t[2] = (int32_t)data_addr;
    iop_register_rpc(0, rpc_send);
    iop_register_rpc(1, rpc_get);
}

int sceSdRemoteInit(void) { return 0; }
int sceSSyn_SetOutputMode(int mode) { (void)mode; return 0; }
