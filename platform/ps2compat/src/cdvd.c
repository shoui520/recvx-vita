/*
 * Disc access (libcdvd) and IOP file I/O (sifdev) on the Vita.
 *
 * The disc is the original ISO image copied to RECVX_DATA_DIR/recvx.iso, so
 * every LSN the game computes (gdfs, AFS offsets, ADX streaming) addresses
 * the same sector it did on the PS2. Reads complete synchronously; the
 * game's busy-wait loops on sceCdSync() see an idle drive immediately.
 */
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/rtc.h>

#include "libcdvd.h"
#include "sifdev.h"
#include "recvx_platform.h"

#define SECTOR 2048

static int iso_fd = -1;
static SceKernelLwMutexWork iso_lock;
static int cd_error;

static int iso_open(void)
{
    if (iso_fd >= 0) return 1;
    iso_fd = open(RECVX_DATA_DIR "/recvx.iso", O_RDONLY);
    if (iso_fd < 0) {
        RECVX_LOG("cdvd: can't open " RECVX_DATA_DIR "/recvx.iso");
        return 0;
    }
    sceKernelCreateLwMutex(&iso_lock, "cdvd", 0, 0, NULL);
    return 1;
}

static int iso_read(uint64_t off, void *buf, u_int len)
{
    if (!iso_open()) return 0;
    sceKernelLockLwMutex(&iso_lock, 1, NULL);
    int ok = lseek(iso_fd, (off_t)off, SEEK_SET) >= 0 && read(iso_fd, buf, len) == (ssize_t)len;
    sceKernelUnlockLwMutex(&iso_lock, 1);
    return ok;
}

/* ------------------------------------------------------------- ISO 9660 */

static uint32_t le32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }

/* compare a path component with an ISO name, ignoring case and ";version" */
static int name_eq(const char *a, int alen, const uint8_t *b, int blen)
{
    for (int i = 0; i < blen; i++) if (b[i] == ';') { blen = i; break; }
    for (int i = 0; i < alen; i++) if (a[i] == ';') { alen = i; break; }
    if (blen && b[blen - 1] == '.') blen--;   /* "NAME." for extensionless files */
    if (alen != blen) return 0;
    for (int i = 0; i < alen; i++)
        if (toupper((unsigned char)a[i]) != toupper(b[i])) return 0;
    return 1;
}

static int iso_lookup(const char *path, u_int *lsn, u_int *size, uint8_t date[7])
{
    uint8_t sec[SECTOR];
    if (!iso_read(16ull * SECTOR, sec, SECTOR) || memcmp(sec + 1, "CD001", 5)) return 0;
    const uint8_t *root = sec + 156;
    u_int dir_lsn = le32(root + 2), dir_size = le32(root + 10);

    while (*path == '\\' || *path == '/') path++;
    while (*path) {
        const char *end = path;
        while (*end && *end != '\\' && *end != '/') end++;
        int found = 0;
        for (u_int off = 0; off < dir_size && !found; off += SECTOR) {
            if (!iso_read((uint64_t)(dir_lsn + off / SECTOR) * SECTOR, sec, SECTOR)) return 0;
            for (u_int p = 0; p < SECTOR && sec[p];) {
                const uint8_t *r = sec + p;
                if (name_eq(path, (int)(end - path), r + 33, r[32])) {
                    dir_lsn = le32(r + 2);
                    dir_size = le32(r + 10);
                    if (date) memcpy(date, r + 18, 7);
                    found = 1;
                    break;
                }
                p += r[0];
            }
        }
        if (!found) return 0;
        path = end;
        while (*path == '\\' || *path == '/') path++;
    }
    *lsn = dir_lsn;
    *size = dir_size;
    return 1;
}

/* ---------------------------------------------------------------- libcdvd */

int sceCdInit(int mode) { if (mode != SCECdEXIT) iso_open(); return 1; }
int sceCdMmode(int media) { (void)media; return 1; }
int sceCdDiskReady(int mode) { (void)mode; return 2; /* SCECdComplete */ }
int sceCdSync(int mode) { (void)mode; return 0; }
int sceCdGetError(void) { return cd_error; }
int sceCdBreak(void) { return 1; }
int sceCdPause(void) { return 1; }

int sceCdSearchFile(sceCdlFILE *fp, const char *name)
{
    uint8_t date[7];
    if (!iso_lookup(name, &fp->lsn, &fp->size, date)) {
        RECVX_LOG("cdvd: %s not found", name);
        return 0;
    }
    const char *base = strrchr(name, '\\');
    base = base ? base + 1 : name;
    snprintf(fp->name, sizeof(fp->name), "%s", base);
    /* sec, min, hour, day, month, year (2 bytes) as libcdvd stores them */
    fp->date[0] = 0;
    fp->date[1] = date[5];
    fp->date[2] = date[4];
    fp->date[3] = date[3];
    fp->date[4] = date[2];
    fp->date[5] = date[1];
    u_int year = 1900 + date[0];
    fp->date[6] = year & 0xff;
    fp->date[7] = year >> 8;
    return 1;
}

int sceCdRead(u_int lsn, u_int sectors, void *buf, sceCdRMode *mode)
{
    (void)mode;
    cd_error = iso_read((uint64_t)lsn * SECTOR, buf, sectors * SECTOR) ? SCECdErNO : SCECdErREAD;
    if (cd_error) RECVX_LOG("cdvd: read of %u sectors at lsn %u failed", sectors, lsn);
    return 1;
}

static u_char bcd(int v) { return (u_char)((v / 10) << 4 | (v % 10)); }

int sceCdReadClock(sceCdCLOCK *rtc)
{
    SceDateTime t;
    sceRtcGetCurrentClockLocalTime(&t);
    rtc->stat = 0;
    rtc->second = bcd(t.second);
    rtc->minute = bcd(t.minute);
    rtc->hour = bcd(t.hour);
    rtc->pad = 0;
    rtc->day = bcd(t.day);
    rtc->month = bcd(t.month);
    rtc->year = bcd(t.year % 100);
    return 1;
}

/* ------------------------------------------------------------------ sifdev */

#define MAX_FILES 16
static struct {
    int used;
    int fd;              /* host file, or -1 for an ISO extent */
    u_int lsn, size, pos;
} files[MAX_FILES];

int sceFsReset(void) { return 0; }

int sceOpen(const char *filename, int flag, ...)
{
    int i;
    for (i = 0; i < MAX_FILES && files[i].used; i++);
    if (i == MAX_FILES) return -1;

    if (!strncmp(filename, "cdrom0:", 7)) {
        if (!iso_lookup(filename + 7, &files[i].lsn, &files[i].size, NULL)) return -1;
        files[i].fd = -1;
    } else {
        /* host0:/host: paths (CRI dev builds) resolve under the data dir */
        const char *p = strchr(filename, ':');
        p = p ? p + 1 : filename;
        while (*p == '/' || *p == '\\') p++;
        char path[256];
        snprintf(path, sizeof(path), RECVX_DATA_DIR "/host/%s", p);
        for (char *c = path; *c; c++) if (*c == '\\') *c = '/';
        int of = (flag & 3) == SCE_RDONLY ? O_RDONLY : (flag & 3) == SCE_WRONLY ? O_WRONLY : O_RDWR;
        if (flag & SCE_CREAT) of |= O_CREAT;
        if (flag & SCE_TRUNC) of |= O_TRUNC;
        if (flag & SCE_APPEND) of |= O_APPEND;
        files[i].fd = open(path, of, 0666);
        if (files[i].fd < 0) return -1;
    }
    files[i].used = 1;
    files[i].pos = 0;
    return i;
}

int sceClose(int fd)
{
    if (fd < 0 || fd >= MAX_FILES || !files[fd].used) return -1;
    if (files[fd].fd >= 0) close(files[fd].fd);
    files[fd].used = 0;
    return 0;
}

int sceRead(int fd, void *buf, int nbyte)
{
    if (fd < 0 || fd >= MAX_FILES || !files[fd].used) return -1;
    if (files[fd].fd >= 0) return read(files[fd].fd, buf, nbyte);
    u_int left = files[fd].size - files[fd].pos;
    if ((u_int)nbyte > left) nbyte = left;
    if (!iso_read((uint64_t)files[fd].lsn * SECTOR + files[fd].pos, buf, nbyte)) return -1;
    files[fd].pos += nbyte;
    return nbyte;
}

int sceWrite(int fd, const void *buf, int nbyte)
{
    if (fd < 0 || fd >= MAX_FILES || !files[fd].used || files[fd].fd < 0) return -1;
    return write(files[fd].fd, buf, nbyte);
}

int sceLseek(int fd, int offset, int whence)
{
    if (fd < 0 || fd >= MAX_FILES || !files[fd].used) return -1;
    if (files[fd].fd >= 0) return lseek(files[fd].fd, offset, whence);
    int base = whence == SCE_SEEK_SET ? 0 : whence == SCE_SEEK_CUR ? (int)files[fd].pos : (int)files[fd].size;
    int np = base + offset;
    if (np < 0) return -1;
    files[fd].pos = np;
    return np;
}

int sceIoctl(int fd, int req, void *arg)
{
    (void)fd;
    if (req == SCE_FS_EXECUTING && arg) *(int *)arg = 0;   /* nothing is ever in flight */
    return 0;
}

/* --------------------------------------------------------------- streaming */

static u_int st_lsn;

int sceCdStInit(u_int bufmax, u_int bankmax, u_int iop_bufaddr) { (void)bufmax; (void)bankmax; (void)iop_bufaddr; return 1; }
int sceCdStStart(u_int lsn, sceCdRMode *mode) { (void)mode; st_lsn = lsn; return 1; }
int sceCdStStop(void) { return 1; }

int sceCdStRead(u_int sectors, u_int *buf, u_int mode, u_int *err)
{
    (void)mode;
    int ok = iso_read((uint64_t)st_lsn * SECTOR, buf, sectors * SECTOR);
    if (err) *err = ok ? SCECdErNO : SCECdErREAD;
    if (!ok) return 0;
    st_lsn += sectors;
    return sectors;
}
