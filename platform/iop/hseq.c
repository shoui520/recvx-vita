/*
 * Sony SQ sequences, as modmidi plays them on the IOP.
 *
 * SQ file: "Vers" chunk, "Sequ" head chunk (+0x14: Midi chunk offset), then the
 * Midi chunk: +12 max block, +16 u32 block offsets (from the chunk).
 * Block: +0 u32 event offset (from the block), +4 u16 ticks per quarter note.
 *
 * Events are SMF-like with two quirks: note off carries no velocity, and a
 * last data byte with bit 7 set means the next delta time is zero and omitted.
 * Loops are NRPN controls: CC99 0 marks the loop start, CC99 1 the loop end
 * (the CC6/CC38 that follow carry the count, always 0 = endless on this disc).
 */
#include <stdlib.h>
#include <string.h>
#include "hseq.h"
#include "hsyn.h"

typedef struct {
    uint32_t pos;
    uint8_t run, skip;
} Cursor;

typedef struct {
    uint8_t *data;
    uint32_t size;
    Cursor cur, loop;
    int has_loop, playing, paused;
    uint32_t tempo, ppqn;      /* us per quarter note, ticks per quarter note */
    uint32_t delta;            /* ticks until the next event */
    uint64_t acc;              /* elapsed us * ppqn * 240 not yet spent */
} Seq;

static Seq seqs[HSEQ_PORTS];

static uint32_t rd32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }

static int byte(Seq *s)
{
    return s->cur.pos < s->size ? s->data[s->cur.pos++] : -1;
}

/* the last data byte of an event: bit 7 drops the next delta time */
static int data_last(Seq *s)
{
    int b = byte(s);
    if (b < 0) return 0;
    s->cur.skip = b >> 7;
    return b & 0x7f;
}

static uint32_t varlen(Seq *s)
{
    uint32_t v = 0;
    for (int i = 0; i < 4; i++) {
        int b = byte(s);
        if (b < 0) break;
        v = v << 7 | (b & 0x7f);
        if (!(b & 0x80)) break;
    }
    return v;
}

static void read_delta(Seq *s)
{
    s->delta = s->cur.skip ? 0 : varlen(s);
    s->cur.skip = 0;
}

static void event(Seq *s, int port)
{
    int st = byte(s);
    if (st < 0) { s->playing = 0; return; }
    if (st < 0x80) {
        st = s->cur.run;
        s->cur.pos--;
    } else if (st != 0xff) {
        s->cur.run = st;
    }
    int ch = st & 15;
    switch (st & 0xf0) {
    case 0x80:
        hsyn_note_off(port, ch, data_last(s));
        break;
    case 0x90: {
        int key = byte(s) & 0x7f, vel = data_last(s);
        if (vel) hsyn_note_on(port, ch, key, vel);
        else hsyn_note_off(port, ch, key);
        break;
    }
    case 0xa0:
        byte(s);
        data_last(s);
        break;
    case 0xb0: {
        int cc = byte(s) & 0x7f, v = data_last(s);
        switch (cc) {
        case 7: hsyn_volume(port, ch, v); break;
        case 10: hsyn_pan(port, ch, v); break;
        case 11: hsyn_expression(port, ch, v); break;
        case 99:
            if (v == 0) {
                s->loop = s->cur;
                s->has_loop = 1;
            } else if (v == 1 && s->has_loop) {
                s->cur = s->loop;
            }
            break;
        case 120: hsyn_sound_off(port, ch); break;
        case 123: hsyn_note_release_all(port); break;
        }
        break;
    }
    case 0xc0:
        hsyn_program(port, ch, data_last(s));
        break;
    case 0xd0:
        data_last(s);
        break;
    case 0xe0: {
        int lo = byte(s) & 0x7f, hi = data_last(s);
        hsyn_bend(port, ch, hi << 7 | lo);
        break;
    }
    default:
        if (st == 0xff && byte(s) == 0x51) {
            /* the length byte (3) rides in the top of a big-endian word */
            uint32_t t = 0;
            for (int i = 0; i < 4; i++) t = t << 8 | (byte(s) & 0xff);
            t &= 0xffffff;
            if (t) s->tempo = t;
        } else {
            s->playing = 0;   /* FF 2F end of track, or data we can't follow */
        }
        return;
    }
}

int hseq_start(int port, const uint8_t *sq, uint32_t size, int block)
{
    if (port < 0 || port >= HSEQ_PORTS || !sq || size < 0x30) return 0;
    hseq_stop(port);
    uint32_t head = rd32(sq + 8);   /* Vers chunk size */
    if (head + 0x18 > size) return 0;
    uint32_t midi = rd32(sq + head + 0x14);
    if (midi == 0xffffffff || midi + 16 > size) return 0;
    uint32_t max = rd32(sq + midi + 12);
    if ((uint32_t)block > max || midi + 16 + 4 * (block + 1) > size) return 0;
    uint32_t blk = rd32(sq + midi + 16 + 4 * block);
    if (blk == 0xffffffff || midi + blk + 6 > size) return 0;
    blk += midi;
    uint32_t evt = blk + rd32(sq + blk);
    if (evt >= size) return 0;

    Seq *s = &seqs[port];
    free(s->data);
    *s = (Seq){ 0 };
    s->data = malloc(size);
    if (!s->data) return 0;
    memcpy(s->data, sq, size);
    s->size = size;
    s->cur.pos = evt;
    s->ppqn = sq[blk + 4] | sq[blk + 5] << 8;
    if (!s->ppqn) s->ppqn = 480;
    s->tempo = 500000;
    s->playing = 1;
    hsyn_reset_channels(port);
    read_delta(s);
    return 1;
}

void hseq_stop(int port)
{
    if (port < 0 || port >= HSEQ_PORTS) return;
    Seq *s = &seqs[port];
    if (s->playing) hsyn_note_release_all(port);
    s->playing = 0;
    s->paused = 0;
}

void hseq_pause(int port, int paused)
{
    if (port < 0 || port >= HSEQ_PORTS) return;
    Seq *s = &seqs[port];
    if (paused && !s->paused && s->playing) hsyn_note_release_all(port);
    s->paused = paused;
}

int hseq_playing(int port)
{
    return port >= 0 && port < HSEQ_PORTS && seqs[port].playing;
}

void hseq_tick(void)
{
    for (int port = 0; port < HSEQ_PORTS; port++) {
        Seq *s = &seqs[port];
        if (!s->playing || s->paused) continue;
        /* one 240 Hz tick is 1e6/240 us; one sequence tick tempo/ppqn us */
        s->acc += (uint64_t)1000000 * s->ppqn;
        /* a loop with no time in it must not hang the audio thread */
        for (int guard = 0; s->playing && guard < 4096; guard++) {
            uint64_t need = (uint64_t)s->delta * s->tempo * 240;
            if (s->acc < need) break;
            s->acc -= need;
            event(s, port);
            if (s->playing) read_delta(s);
        }
    }
}
