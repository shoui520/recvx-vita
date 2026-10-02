/* Services the platform layer gives the PS2 compatibility code. */
#pragma once
#include <stdio.h>

/* game data, as laid out by the prep tool */
#define RECVX_DATA_DIR "ux0:data/recvx"

void recvx_log(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#define RECVX_LOG(...) recvx_log(__VA_ARGS__)
#define RECVX_LOG_ONCE(...) do { static int once_; if (!once_) { once_ = 1; recvx_log(__VA_ARGS__); } } while (0)

/* kernel.c */
void recvx_kernel_start(void (*entry)(void));
unsigned int recvx_vblank_count(void);
void recvx_kernel_dump_threads(void);

/* RECVX_DATA_DIR/boot.txt "newgame": boot unattended into a new game - the
 * memory card check takes its defaults and the title menu is skipped once */
int recvx_boot_newgame(void);
