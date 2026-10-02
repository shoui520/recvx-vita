/*
 * libmc on files: memory card slot 1 is RECVX_DATA_DIR/mc0, slot 2 is absent.
 *
 * Every request completes when it is issued; the following sceMcSync()
 * reports it finished once, then reports idle.
 */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "libmc.h"
#include "sifdev.h"
#include "recvx_platform.h"

#define MC_ROOT RECVX_DATA_DIR "/mc0"
#define MC_KB_TOTAL 8000

static char cwd[256] = "/";
static int pending, pending_cmd, pending_result;
static int fds[8];

static int finish(int cmd, int result)
{
    pending = 1;
    pending_cmd = cmd;
    pending_result = result;
    return 0;
}

int sceMcSync(int mode, int *cmd, int *result)
{
    (void)mode;
    if (!pending) return -1;   /* sceMcExecIdle */
    pending = 0;
    if (cmd) *cmd = pending_cmd;
    if (result) *result = pending_result;
    return 1;                  /* sceMcExecFinish */
}

int sceMcInit(void)
{
    mkdir(RECVX_DATA_DIR, 0777);
    mkdir(MC_ROOT, 0777);
    for (int i = 0; i < 8; i++) fds[i] = -1;
    return sceMcIniSucceed;
}

/* card path ("/BASLUS-20184", "file", "../x") -> Vita path */
static void host_path(const char *name, char *out, size_t n)
{
    char tmp[256];
    if (name[0] == '/') snprintf(tmp, sizeof(tmp), "%s", name);
    else snprintf(tmp, sizeof(tmp), "%s%s%s", cwd, cwd[strlen(cwd) - 1] == '/' ? "" : "/", name);
    snprintf(out, n, MC_ROOT "%s", tmp);
}

static int dir_kb_used(const char *path)
{
    int kb = 0;
    DIR *d = opendir(path);
    if (!d) return 0;
    struct dirent *e;
    while ((e = readdir(d))) {
        if (e->d_name[0] == '.') continue;
        char p[512];
        struct stat st;
        snprintf(p, sizeof(p), "%s/%s", path, e->d_name);
        if (stat(p, &st)) continue;
        kb += S_ISDIR(st.st_mode) ? 1 + dir_kb_used(p) : (int)((st.st_size + 1023) / 1024);
    }
    closedir(d);
    return kb;
}

int sceMcGetInfo(int port, int slot, int *type, int *free, int *format)
{
    (void)slot;
    if (port != 0) {
        if (type) *type = 0;
        return finish(sceMcFuncNoCardInfo, sceMcResNoEntry - 6);   /* -10: no card */
    }
    if (type) *type = 2;       /* PS2 memory card */
    if (free) *free = MC_KB_TOTAL - dir_kb_used(MC_ROOT);
    if (format) *format = 1;
    return finish(sceMcFuncNoCardInfo, sceMcResSucceed);
}

int sceMcFormat(int port, int slot) { (void)port; (void)slot; return finish(sceMcFuncNoFormat, sceMcResSucceed); }

int sceMcChdir(int port, int slot, const char *newDir, char *currentDir)
{
    (void)port; (void)slot;
    if (currentDir) strcpy(currentDir, cwd);
    if (!newDir || !*newDir) return finish(sceMcFuncNoChDir, sceMcResSucceed);
    char p[256], next[256];
    if (newDir[0] == '/') snprintf(next, sizeof(next), "%s", newDir);
    else if (!strcmp(newDir, "..")) {
        snprintf(next, sizeof(next), "%s", cwd);
        char *s = strrchr(next, '/');
        if (s && s != next) *s = 0; else strcpy(next, "/");
    } else snprintf(next, sizeof(next), "%s%s%s", cwd, cwd[strlen(cwd) - 1] == '/' ? "" : "/", newDir);
    snprintf(p, sizeof(p), MC_ROOT "%s", next);
    struct stat st;
    if (stat(p, &st) || !S_ISDIR(st.st_mode)) return finish(sceMcFuncNoChDir, sceMcResNoEntry);
    snprintf(cwd, sizeof(cwd), "%s", next);
    return finish(sceMcFuncNoChDir, sceMcResSucceed);
}

int sceMcMkdir(int port, int slot, const char *name)
{
    (void)port; (void)slot;
    char p[256];
    host_path(name, p, sizeof(p));
    if (mkdir(p, 0777) == 0) return finish(sceMcFuncNoMkdir, sceMcResSucceed);
    return finish(sceMcFuncNoMkdir, errno == EEXIST ? sceMcResNoEntry : sceMcResFullDevice);
}

int sceMcDelete(int port, int slot, const char *name)
{
    (void)port; (void)slot;
    char p[256];
    host_path(name, p, sizeof(p));
    if (!remove(p)) return finish(sceMcFuncNoDelete, sceMcResSucceed);
    return finish(sceMcFuncNoDelete, errno == ENOTEMPTY ? sceMcResNotEmpty : sceMcResNoEntry);
}

static int wild_match(const char *pat, const char *s)
{
    if (!*pat) return !*s;
    if (*pat == '*') return wild_match(pat + 1, s) || (*s && wild_match(pat, s + 1));
    if (*s && (*pat == '?' || *pat == *s)) return wild_match(pat + 1, s + 1);
    return 0;
}

static void fill_entry(sceMcTblGetDir *t, const char *name, const struct stat *st)
{
    memset(t, 0, sizeof(*t));
    struct tm *tm = localtime(&st->st_mtime);
    if (tm) {
        t->_Create.Sec = t->_Modify.Sec = tm->tm_sec;
        t->_Create.Min = t->_Modify.Min = tm->tm_min;
        t->_Create.Hour = t->_Modify.Hour = tm->tm_hour;
        t->_Create.Day = t->_Modify.Day = tm->tm_mday;
        t->_Create.Month = t->_Modify.Month = tm->tm_mon + 1;
        t->_Create.Year = t->_Modify.Year = tm->tm_year + 1900;
    }
    t->FileSizeByte = S_ISDIR(st->st_mode) ? 0 : (u_int)st->st_size;
    t->AttrFile = 0x8000 | SCE_MC_FILE_ATTR_READABLE | SCE_MC_FILE_ATTR_WRITEABLE |
                  (S_ISDIR(st->st_mode) ? SCE_MC_FILE_ATTR_SUBDIR : 0x0010);
    snprintf((char *)t->EntryName, sizeof(t->EntryName), "%s", name);
}

int sceMcGetDir(int port, int slot, const char *name, unsigned mode, int maxent, sceMcTblGetDir *table)
{
    (void)port; (void)slot; (void)mode;
    char p[256], dir[256], pat[64];
    host_path(name, p, sizeof(p));
    char *s = strrchr(p, '/');
    snprintf(pat, sizeof(pat), "%s", s + 1);
    *s = 0;
    snprintf(dir, sizeof(dir), "%s", p);
    int is_root = !strcmp(dir, MC_ROOT);

    int n = 0;
    struct stat st;
    if (!is_root && n < maxent && !stat(dir, &st)) {
        /* subdirectories list "." and ".." like the real card */
        if (wild_match(pat, ".")) fill_entry(&table[n++], ".", &st);
        if (n < maxent && wild_match(pat, "..")) fill_entry(&table[n++], "..", &st);
    }
    DIR *d = opendir(dir);
    if (!d) return finish(sceMcFuncNoGetDir, n ? n : sceMcResNoEntry);
    struct dirent *e;
    while (n < maxent && (e = readdir(d))) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
        if (!wild_match(pat, e->d_name)) continue;
        char fp[512];
        snprintf(fp, sizeof(fp), "%s/%s", dir, e->d_name);
        if (stat(fp, &st)) continue;
        fill_entry(&table[n++], e->d_name, &st);
    }
    closedir(d);
    return finish(sceMcFuncNoGetDir, n);
}

int sceMcOpen(int port, int slot, const char *name, int mode)
{
    (void)port; (void)slot;
    char p[256];
    host_path(name, p, sizeof(p));
    int of = (mode & 3) == SCE_RDONLY ? O_RDONLY : (mode & 3) == SCE_WRONLY ? O_WRONLY : O_RDWR;
    if (mode & SCE_CREAT) of |= O_CREAT;
    int i;
    for (i = 0; i < 8 && fds[i] >= 0; i++);
    if (i == 8) return finish(sceMcFuncNoOpen, sceMcResUpLimitHandle);
    fds[i] = open(p, of, 0666);
    if (fds[i] < 0) { fds[i] = -1; return finish(sceMcFuncNoOpen, sceMcResNoEntry); }
    return finish(sceMcFuncNoOpen, i);
}

int sceMcClose(int fd)
{
    if (fd < 0 || fd >= 8 || fds[fd] < 0) return finish(sceMcFuncNoClose, sceMcResDeniedPermit);
    close(fds[fd]);
    fds[fd] = -1;
    return finish(sceMcFuncNoClose, sceMcResSucceed);
}

int sceMcSeek(int fd, int offset, int mode)
{
    if (fd < 0 || fd >= 8 || fds[fd] < 0) return finish(sceMcFuncNoSeek, sceMcResDeniedPermit);
    return finish(sceMcFuncNoSeek, (int)lseek(fds[fd], offset, mode));
}

int sceMcRead(int fd, void *buff, int size)
{
    if (fd < 0 || fd >= 8 || fds[fd] < 0) return finish(sceMcFuncNoRead, sceMcResDeniedPermit);
    return finish(sceMcFuncNoRead, (int)read(fds[fd], buff, size));
}

int sceMcWrite(int fd, const void *buff, int size)
{
    if (fd < 0 || fd >= 8 || fds[fd] < 0) return finish(sceMcFuncNoWrite, sceMcResDeniedPermit);
    return finish(sceMcFuncNoWrite, (int)write(fds[fd], buff, size));
}
