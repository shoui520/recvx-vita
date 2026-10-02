/*
 * Vita entry point: bring up the GS renderer, then run the game's main() on
 * an EE thread. The Vita main thread is done after that.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/power.h>
#include "recvx_platform.h"
#include "gs/gs.h"

int _newlib_heap_size_user = 160 * 1024 * 1024;

int recvx_game_main(void);

static SceUID log_fd = -1;
static SceKernelLwMutexWork log_lock;

void recvx_log(const char *fmt, ...)
{
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
    va_end(ap);
    if (n < 0) return;
    if (n > (int)sizeof(buf) - 2) n = sizeof(buf) - 2;
    buf[n++] = '\n';
    sceKernelLockLwMutex(&log_lock, 1, NULL);
    sceClibPrintf("%.*s", n, buf);
    if (log_fd >= 0) sceIoWrite(log_fd, buf, n);
    sceKernelUnlockLwMutex(&log_lock, 1);
}

/*
 * Test hooks in RECVX_DATA_DIR, all off unless their file exists:
 *   watchdog.txt  when no frame was presented for 15 s, log the EE threads and
 *                 crash so the core dump holds every thread's stack
 *   dumpat.txt    frame number at which to take a core dump on purpose
 *   boot.txt      "newgame": skip the memory card check and title menu
 *   input.txt     scripted pad input (see ps2compat/src/pad.c)
 */
static int file_exists(const char *path)
{
    SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
    if (fd < 0) return 0;
    sceIoClose(fd);
    return 1;
}

static int watchdog(SceSize argc, void *argv)
{
    (void)argc; (void)argv;
    int hang_check = file_exists(RECVX_DATA_DIR "/watchdog.txt");
    /* dumpat.txt: frame number at which to take a core dump on purpose */
    unsigned dump_at = 0;
    char buf[16] = { 0 };
    SceUID fd = sceIoOpen(RECVX_DATA_DIR "/dumpat.txt", SCE_O_RDONLY, 0);
    if (fd >= 0) {
        sceIoRead(fd, buf, sizeof(buf) - 1);
        sceIoClose(fd);
        dump_at = strtoul(buf, NULL, 10);
    }
    if (!hang_check && !dump_at) return 0;
    unsigned last = gs_frame_count(), still = 0;
    for (;;) {
        sceKernelDelayThread(1000 * 1000);
        unsigned now = gs_frame_count();
        if (dump_at && now >= dump_at) {
            recvx_log("watchdog: dump requested at frame %u", now);
            recvx_kernel_dump_threads();
            sceKernelDelayThread(500 * 1000);
            *(volatile int *)0 = 0x44554d50;   /* "DUMP" */
        }
        if (!hang_check || now != last || now == 0) { last = now; still = 0; continue; }
        if (++still < 15) continue;
        recvx_log("watchdog: no frame since %u for %u s, vblank %u", now, still, recvx_vblank_count());
        recvx_kernel_dump_threads();
        sceKernelDelayThread(500 * 1000);
        *(volatile int *)0 = 0x57444f47;   /* "WDOG" */
    }
    return 0;
}

int recvx_boot_newgame(void)
{
    static int mode = -1;
    if (mode >= 0) return mode;
    mode = 0;
    char buf[32] = { 0 };
    SceUID fd = sceIoOpen(RECVX_DATA_DIR "/boot.txt", SCE_O_RDONLY, 0);
    if (fd < 0) return 0;
    sceIoRead(fd, buf, sizeof(buf) - 1);
    sceIoClose(fd);
    mode = !strncmp(buf, "newgame", 7);
    if (mode) recvx_log("boot: unattended new game (no memory card check, no title menu)");
    return mode;
}

static void game_entry(void)
{
    recvx_game_main();
}

int main(void)
{
    sceKernelCreateLwMutex(&log_lock, "recvx_log", 0, 0, NULL);
    sceIoMkdir(RECVX_DATA_DIR, 0777);
    log_fd = sceIoOpen(RECVX_DATA_DIR "/log.txt", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
    recvx_log("recvx: starting");

    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);

    gs_draw_init();
    SceUID wd = sceKernelCreateThread("watchdog", watchdog, 64, 16 * 1024, 0, SCE_KERNEL_CPU_MASK_USER_2, NULL);
    if (wd >= 0) sceKernelStartThread(wd, 0, NULL);
    recvx_kernel_start(game_entry);
    sceKernelExitDeleteThread(0);
    return 0;
}
