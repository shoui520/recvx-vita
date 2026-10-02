/*
 * Hardware-synth replacement: HD program lookup, VAG ADPCM voices with the
 * SPU2 envelope, mixed at 48 kHz.
 *
 * HD layout (all little endian, offsets relative to the chunk that holds them):
 *   Head  +20 prog chunk, +24 sample-set chunk, +28 sample chunk, +32 vag-info chunk
 *   chunk +12 max index, +16 u32 offsets[max+1] (0xffffffff = none)
 *   prog  +0 split offset (from the program), +4 nsplit, +5 split size,
 *         +6 volume, +7 pan, +8 transpose, +9 detune
 *   split +0 sample set, +2 key low, +4 key high, +6/+8 bend range (1/128 semitone),
 *         +16 volume, +17 pan, +18 transpose, +19 detune
 *   sset  +3 count, +4 u16 sample index[]
 *   smpl  +0 vag index, +2/+4 velocity range, +11 base note, +12 detune, +13 pan,
 *         +16 volume, +18 ADSR1, +20 ADSR2
 *   vagi  +0 BD offset, +4 sample rate, +6 loop
 */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "hsyn.h"

#define MAX_VOICES 48

typedef struct {
    uint8_t *hd, *bd;
    uint32_t hdsize, bdsize;
    int owned;
} Bank;

typedef struct {
    uint8_t prog, vol, pan, expr;
    uint16_t bend;
} Chan;

typedef struct {
    int master;                /* sceHSyn_SetVolume, 0-0x3fff */
    uint8_t vol, pan;          /* BGM port volume, pan offset (centre 64) */
    uint16_t bend;             /* added to every channel's bend, 0x2000 centre */
} Port;

enum { ENV_OFF, ENV_ATTACK, ENV_DECAY, ENV_SUSTAIN, ENV_RELEASE };

typedef struct {
    uint8_t active, port, ch, note;
    uint32_t serial;
    const uint8_t *bd;
    uint32_t bdsize;
    /* ADPCM stream */
    uint32_t block, loop;      /* byte offsets in bd */
    int16_t pcm[28];
    int idx;                   /* next sample in pcm[] */
    int32_t h1, h2;
    int ended;                 /* block with "end, no repeat" played */
    int16_t s0, s1;            /* interpolation pair */
    uint32_t frac;             /* 16.16 */
    uint32_t step;
    /* pitch */
    float rate, semis, bend_range;
    /* envelope */
    uint16_t adsr1, adsr2;
    int phase;
    int32_t env;
    int32_t env_wait;
    /* static gain parts */
    int32_t gain;              /* 0..32767 from prog/split/sample/velocity */
    int pan;                   /* -64..63 before channel pan */
    int32_t vol_l, vol_r;      /* current 0..32767 */
} Voice;

static Bank banks[HSYN_PORTS];
static Chan chans[HSYN_PORTS][16];
static Port ports[HSYN_PORTS];
static Voice voices[MAX_VOICES];
static uint32_t serial;
static int master = 0x3fff;
static int chans_ready;

static uint32_t rd32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }
static uint16_t rd16(const uint8_t *p) { return p[0] | p[1] << 8; }

static void chans_init(void)
{
    for (int p = 0; p < HSYN_PORTS; p++)
        for (int c = 0; c < 16; c++)
            chans[p][c] = (Chan){ 0, 127, 64, 127, 0x2000 };
    for (int p = 0; p < HSYN_PORTS; p++)
        ports[p] = (Port){ 0x3fff, 127, 64, 0x2000 };
    chans_ready = 1;
}

/* ------------------------------------------------------------- HD lookup */

static const uint8_t *chunk(const uint8_t *hd, int which)
{
    return hd + rd32(hd + 0x10 + which);
}

static const uint8_t *entry(const uint8_t *ck, int idx)
{
    if (idx < 0 || (uint32_t)idx > rd32(ck + 12)) return NULL;
    uint32_t off = rd32(ck + 16 + 4 * idx);
    return off == 0xffffffff ? NULL : ck + off;
}

int hsyn_hd_max_program(const uint8_t *hd)
{
    return hd ? (int)rd32(chunk(hd, 20) + 12) : -1;
}

static int vag_info(const uint8_t *hd, uint32_t bdsize, int vi, HsynSampleInfo *o)
{
    const uint8_t *vagi = chunk(hd, 32);
    const uint8_t *v = entry(vagi, vi);
    if (!v) return 0;
    o->bd_offset = rd32(v);
    o->rate = rd16(v + 4);
    o->loop = v[6];
    const uint8_t *next = entry(vagi, vi + 1);
    uint32_t end = next ? rd32(next) : bdsize;
    o->bd_size = end > o->bd_offset ? end - o->bd_offset : 0;
    return o->bd_offset < bdsize;
}

int hsyn_hd_program_sample(const uint8_t *hd, uint32_t bdsize, int prog, HsynSampleInfo *o)
{
    if (!hd) return 0;
    const uint8_t *p = entry(chunk(hd, 20), prog);
    if (!p || p[4] == 0) return 0;
    const uint8_t *split = p + rd32(p);
    const uint8_t *ss = entry(chunk(hd, 24), rd16(split));
    if (!ss || ss[3] == 0) return 0;
    const uint8_t *sm = entry(chunk(hd, 28), rd16(ss + 4));
    if (!sm) return 0;
    o->base_note = sm[11];
    return vag_info(hd, bdsize, rd16(sm), o);
}

/* ---------------------------------------------------------------- banks */

static void bank_put(int port, uint8_t *hd, uint32_t hdsize, uint8_t *bd, uint32_t bdsize, int owned)
{
    for (int i = 0; i < MAX_VOICES; i++)
        if (voices[i].active && voices[i].port == port) voices[i].active = 0;
    if (banks[port].owned) {
        free(banks[port].hd);
        free(banks[port].bd);
    }
    banks[port] = (Bank){ hd, bd, hdsize, bdsize, owned };
}

void hsyn_bank_set(int port, uint8_t *hd, uint32_t hdsize, uint8_t *bd, uint32_t bdsize)
{
    if (port < 0 || port >= HSYN_PORTS) { free(hd); free(bd); return; }
    bank_put(port, hd, hdsize, bd, bdsize, 1);
}

void hsyn_bank_ref(int port, const uint8_t *hd, uint32_t hdsize, const uint8_t *bd, uint32_t bdsize)
{
    if (port < 0 || port >= HSYN_PORTS) return;
    bank_put(port, (uint8_t *)hd, hdsize, (uint8_t *)bd, bdsize, 0);
}

const uint8_t *hsyn_bank_hd(int port)
{
    return port >= 0 && port < HSYN_PORTS ? banks[port].hd : NULL;
}

/* ----------------------------------------------------------------- ADPCM */

static const int8_t vag_f[5][2] = { { 0, 0 }, { 60, 0 }, { 115, -52 }, { 98, -55 }, { 122, -60 } };

static void decode_block(Voice *v)
{
    if (v->ended || v->block + 16 > v->bdsize) {
        memset(v->pcm, 0, sizeof(v->pcm));
        v->ended = 1;
        v->idx = 0;
        return;
    }
    const uint8_t *b = v->bd + v->block;
    int shift = b[0] & 15, filt = (b[0] >> 4) & 7, flags = b[1];
    if (filt > 4) filt = 0;
    if (shift > 12) shift = 9;
    if (flags & 4) v->loop = v->block;
    for (int i = 0; i < 28; i++) {
        int nib = (b[2 + i / 2] >> ((i & 1) * 4)) & 15;
        int32_t s = (int16_t)(nib << 12) >> shift;
        s += (v->h1 * vag_f[filt][0] + v->h2 * vag_f[filt][1] + 32) >> 6;
        if (s > 32767) s = 32767;
        if (s < -32768) s = -32768;
        v->h2 = v->h1;
        v->h1 = s;
        v->pcm[i] = s;
    }
    v->idx = 0;
    if (flags & 1) {
        if (flags & 2) v->block = v->loop;
        else v->ended = 1;           /* plays out this block, then stops */
    } else {
        v->block += 16;
    }
}

static int16_t next_sample(Voice *v)
{
    if (v->idx >= 28) decode_block(v);
    return v->pcm[v->idx++];
}

/* -------------------------------------------------------------- envelope */

/* one SPU envelope step: shift/step as in the ADSR registers */
static void env_step(Voice *v, int exp_mode, int decreasing, int shift, int step)
{
    if (v->env_wait > 0) { v->env_wait--; return; }
    int32_t cycles = 1 << (shift > 11 ? shift - 11 : 0);
    int32_t inc = step << (shift < 11 ? 11 - shift : 0);
    if (exp_mode && !decreasing && v->env > 0x6000) cycles *= 4;
    if (exp_mode && decreasing) inc = inc * v->env >> 15;
    v->env += inc;
    if (v->env > 0x7fff) v->env = 0x7fff;
    if (v->env < 0) v->env = 0;
    v->env_wait = cycles - 1;
}

static void env_tick(Voice *v)
{
    uint16_t a1 = v->adsr1, a2 = v->adsr2;
    switch (v->phase) {
    case ENV_ATTACK:
        env_step(v, a1 >> 15, 0, (a1 >> 10) & 31, 7 - ((a1 >> 8) & 3));
        if (v->env >= 0x7fff) { v->phase = ENV_DECAY; v->env_wait = 0; }
        break;
    case ENV_DECAY: {
        int32_t sl = ((a1 & 15) + 1) * 0x800;
        env_step(v, 1, 1, (a1 >> 4) & 15, -8);
        if (v->env <= sl) { v->env = sl > 0x7fff ? 0x7fff : sl; v->phase = ENV_SUSTAIN; v->env_wait = 0; }
        break;
    }
    case ENV_SUSTAIN: {
        int dec = (a2 >> 14) & 1, s = (a2 >> 6) & 3;
        env_step(v, a2 >> 15, dec, (a2 >> 8) & 31, dec ? -8 + s : 7 - s);
        break;
    }
    case ENV_RELEASE:
        env_step(v, (a2 >> 5) & 1, 1, a2 & 31, -8);
        if (v->env <= 0) { v->phase = ENV_OFF; v->active = 0; }
        break;
    }
}

/* ---------------------------------------------------------------- voices */

static void voice_pitch(Voice *v)
{
    Chan *c = &chans[v->port][v->ch];
    float bend = (c->bend + ports[v->port].bend - 2 * 8192) / 8192.0f * v->bend_range;
    float hz = v->rate * exp2f((v->semis + bend) / 12.0f);
    v->step = (uint32_t)(hz / HSYN_RATE * 65536.0f);
}

static void voice_volume(Voice *v)
{
    Chan *c = &chans[v->port][v->ch];
    Port *pt = &ports[v->port];
    int pan = v->pan + c->pan + pt->pan - 64;   /* centre 64 */
    if (pan < 0) pan = 0;
    if (pan > 127) pan = 127;
    int64_t g = (int64_t)v->gain * c->vol * c->expr * pt->vol / (127 * 127 * 127);
    g = g * pt->master / 0x3fff * master / 0x3fff;
    v->vol_l = g * (pan <= 64 ? 64 : 127 - pan) / 64;
    v->vol_r = g * (pan >= 64 ? 64 : pan) / 64;
}

static Voice *voice_alloc(void)
{
    Voice *best = NULL;
    for (int i = 0; i < MAX_VOICES; i++) {
        Voice *v = &voices[i];
        if (!v->active) return v;
        /* steal the oldest releasing voice, else the oldest */
        if (!best || (v->phase == ENV_RELEASE) > (best->phase == ENV_RELEASE) ||
            ((v->phase == ENV_RELEASE) == (best->phase == ENV_RELEASE) && v->serial < best->serial))
            best = v;
    }
    return best;
}

static int pan_rel(uint8_t p)   /* HD pans: 0-127 centre 64, 0x80+ reversed */
{
    return p > 127 ? 64 - (p - 127) - 64 : p - 64;
}

void hsyn_note_on(int port, int ch, int note, int vel)
{
    if (!chans_ready) chans_init();
    Bank *b = &banks[port];
    if (!b->hd || !b->bd) return;
    Chan *c = &chans[port][ch];
    const uint8_t *hd = b->hd;
    const uint8_t *p = entry(chunk(hd, 20), c->prog);
    if (!p) return;
    for (int s = 0; s < p[4]; s++) {
        const uint8_t *split = p + rd32(p) + s * p[5];
        int lo = split[2], hi = split[4];
        if (hi < lo) hi = 127;
        if (note < lo || note > hi) continue;
        const uint8_t *ss = entry(chunk(hd, 24), rd16(split));
        if (!ss) continue;
        for (int k = 0; k < ss[3]; k++) {
            const uint8_t *sm = entry(chunk(hd, 28), rd16(ss + 4 + 2 * k));
            if (!sm || vel < sm[2] || vel > sm[4]) continue;
            HsynSampleInfo vi;
            if (!vag_info(hd, b->bdsize, rd16(sm), &vi)) continue;
            Voice *v = voice_alloc();
            memset(v, 0, sizeof(*v));
            v->active = 1;
            v->port = port;
            v->ch = ch;
            v->note = note;
            v->serial = ++serial;
            v->bd = b->bd;
            v->bdsize = b->bdsize;
            v->block = v->loop = vi.bd_offset;
            v->idx = 28;
            v->rate = vi.rate;
            v->semis = note - sm[11] + (int8_t)p[8] + (int8_t)split[18] +
                       ((int8_t)p[9] + (int8_t)split[19] + (int8_t)sm[12]) / 128.0f;
            uint16_t br = rd16(split + 6);
            v->bend_range = br ? br / 128.0f : 2.0f;
            v->adsr1 = rd16(sm + 18);
            v->adsr2 = rd16(sm + 20);
            v->phase = ENV_ATTACK;
            v->gain = (int32_t)((int64_t)32767 * p[6] * split[16] * sm[16] * vel / (127LL * 127 * 127 * 127));
            v->pan = pan_rel(p[7]) + pan_rel(split[17]) + pan_rel(sm[13]);
            v->s0 = next_sample(v);
            v->s1 = next_sample(v);
            voice_pitch(v);
            voice_volume(v);
        }
    }
}

void hsyn_note_off(int port, int ch, int note)
{
    for (int i = 0; i < MAX_VOICES; i++) {
        Voice *v = &voices[i];
        if (v->active && v->port == port && v->ch == ch && v->note == note && v->phase != ENV_RELEASE) {
            v->phase = ENV_RELEASE;
            v->env_wait = 0;
        }
    }
}

void hsyn_sound_off(int port, int ch)
{
    for (int i = 0; i < MAX_VOICES; i++)
        if (voices[i].active && voices[i].port == port && voices[i].ch == ch) voices[i].active = 0;
}

void hsyn_program(int port, int ch, int prog)
{
    if (!chans_ready) chans_init();
    chans[port][ch].prog = prog;
}

static void chan_update(int port, int ch, int pitch)
{
    for (int i = 0; i < MAX_VOICES; i++) {
        Voice *v = &voices[i];
        if (!v->active || v->port != port || v->ch != ch) continue;
        if (pitch) voice_pitch(v);
        else voice_volume(v);
    }
}

void hsyn_volume(int port, int ch, int vol)
{
    if (!chans_ready) chans_init();
    chans[port][ch].vol = vol;
    chan_update(port, ch, 0);
}

void hsyn_pan(int port, int ch, int pan)
{
    if (!chans_ready) chans_init();
    chans[port][ch].pan = pan;
    chan_update(port, ch, 0);
}

void hsyn_bend(int port, int ch, int bend)
{
    if (!chans_ready) chans_init();
    chans[port][ch].bend = bend;
    chan_update(port, ch, 1);
}

void hsyn_expression(int port, int ch, int expr)
{
    if (!chans_ready) chans_init();
    chans[port][ch].expr = expr;
    chan_update(port, ch, 0);
}

static void port_update(int port, int pitch)
{
    for (int ch = 0; ch < 16; ch++) chan_update(port, ch, pitch);
}

void hsyn_port_master(int port, int vol)
{
    if (!chans_ready) chans_init();
    ports[port].master = vol < 0 ? 0 : vol > 0x3fff ? 0x3fff : vol;
    port_update(port, 0);
}

void hsyn_port_volume(int port, int vol)
{
    if (!chans_ready) chans_init();
    ports[port].vol = vol;
    port_update(port, 0);
}

void hsyn_port_pan(int port, int pan)
{
    if (!chans_ready) chans_init();
    ports[port].pan = pan;
    port_update(port, 0);
}

void hsyn_port_bend(int port, int bend)
{
    if (!chans_ready) chans_init();
    ports[port].bend = bend;
    port_update(port, 1);
}

void hsyn_note_release_all(int port)
{
    for (int i = 0; i < MAX_VOICES; i++) {
        Voice *v = &voices[i];
        if (v->active && v->port == port && v->phase != ENV_RELEASE) {
            v->phase = ENV_RELEASE;
            v->env_wait = 0;
        }
    }
}

void hsyn_reset_channels(int port)
{
    for (int c = 0; c < 16; c++) chans[port][c] = (Chan){ 0, 127, 64, 127, 0x2000 };
}

void hsyn_master_volume(int vol)
{
    if (!chans_ready) chans_init();
    master = vol < 0 ? 0 : vol > 0x3fff ? 0x3fff : vol;
    for (int i = 0; i < MAX_VOICES; i++)
        if (voices[i].active) voice_volume(&voices[i]);
}

/* ------------------------------------------------------------------- mix */

void hsyn_mix(int32_t *out, int n)
{
    for (int i = 0; i < MAX_VOICES; i++) {
        Voice *v = &voices[i];
        if (!v->active) continue;
        for (int k = 0; k < n && v->active; k++) {
            int32_t f = v->frac >> 4;   /* 12 bit */
            int32_t s = v->s0 + ((v->s1 - v->s0) * f >> 12);
            s = s * v->env >> 15;
            out[2 * k] += s * v->vol_l >> 15;
            out[2 * k + 1] += s * v->vol_r >> 15;
            env_tick(v);
            v->frac += v->step;
            while (v->frac >= 0x10000) {
                v->frac -= 0x10000;
                v->s0 = v->s1;
                v->s1 = next_sample(v);
            }
            /* a one-shot that ran off its end is done once the last block drained */
            if (v->ended && v->idx >= 28) v->active = 0;
        }
    }
}
