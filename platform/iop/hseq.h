/*
 * Sony SQ sequence player (modmidi stand-in) driving the hsyn ports 0-9.
 * Every call must be made with iop_lock() held.
 */
#pragma once
#include <stdint.h>

#define HSEQ_PORTS 10

/* plays MIDI block `block` of an SQ file (Vers/Head/Midi chunks); keeps a copy */
int hseq_start(int port, const uint8_t *sq, uint32_t size, int block);
void hseq_stop(int port);                 /* releases sounding notes */
void hseq_pause(int port, int paused);
int hseq_playing(int port);               /* until the end of an unlooped block */
void hseq_tick(void);                     /* 240 Hz */
