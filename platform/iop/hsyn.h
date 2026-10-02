/*
 * Software stand-in for the IOP hardware synthesizer (modhsyn on the SPU2):
 * plays Sony HD/BD banks through MIDI-style channel messages.
 *
 * Ports 0-9 carry MIDI (BGM) banks, ports 10-15 the SE banks, as in TSNDDRV.
 * Every call must be made with iop_lock() held; the audio thread mixes under it.
 */
#pragma once
#include <stdint.h>

#define HSYN_PORTS 16
#define HSYN_RATE  48000

/* hd/bd are malloc'd and owned by the synth from here on; NULL unloads */
void hsyn_bank_set(int port, uint8_t *hd, uint32_t hdsize, uint8_t *bd, uint32_t bdsize);
/* the same bank shared by reference; the caller keeps it alive while loaded */
void hsyn_bank_ref(int port, const uint8_t *hd, uint32_t hdsize, const uint8_t *bd, uint32_t bdsize);
const uint8_t *hsyn_bank_hd(int port);

void hsyn_program(int port, int ch, int prog);
void hsyn_note_on(int port, int ch, int note, int vel);
void hsyn_note_off(int port, int ch, int note);
void hsyn_sound_off(int port, int ch);          /* CC 120: silence at once */
void hsyn_volume(int port, int ch, int vol);     /* CC 7, 0-127 */
void hsyn_pan(int port, int ch, int pan);        /* CC 10, 0-127 */
void hsyn_bend(int port, int ch, int bend);      /* 14 bit, 0x2000 centre */
void hsyn_expression(int port, int ch, int expr);/* CC 11, 0-127 */
void hsyn_note_release_all(int port);
void hsyn_reset_channels(int port);
void hsyn_port_master(int port, int vol);        /* sceHSyn_SetVolume, 0-0x3fff */
void hsyn_port_volume(int port, int vol);        /* BGM volume, 0-127 */
void hsyn_port_pan(int port, int pan);           /* 0-127, centre 64 */
void hsyn_port_bend(int port, int bend);         /* 14 bit, 0x2000 centre */
void hsyn_master_volume(int vol);                /* 0-0x3fff */

void hsyn_mix(int32_t *out, int n);              /* adds n stereo frames */

/* HD helpers for the driver's SE table */
typedef struct {
    uint32_t bd_offset, bd_size;
    uint16_t rate;
    uint8_t loop, base_note;
} HsynSampleInfo;
int hsyn_hd_max_program(const uint8_t *hd);
int hsyn_hd_program_sample(const uint8_t *hd, uint32_t bdsize, int prog, HsynSampleInfo *out);
