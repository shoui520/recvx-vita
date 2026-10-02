/* libmc: memory card access, backed by save files on ux0: by the port. */
#pragma once
#include "eetypes.h"

typedef struct {
    struct { u_char Resv2, Sec, Min, Hour, Day, Month; u_short Year; } _Create;
    struct { u_char Resv2, Sec, Min, Hour, Day, Month; u_short Year; } _Modify;
    u_int  FileSizeByte;
    u_short AttrFile;
    u_short Reserve1;
    u_int  Reserve2;
    u_int  PdaAplNo;
    u_char EntryName[32];
} sceMcTblGetDir __attribute__((aligned(64)));

#define sceMcFuncNoCardInfo 1
#define sceMcFuncNoOpen     2
#define sceMcFuncNoClose    3
#define sceMcFuncNoSeek     4
#define sceMcFuncNoRead     5
#define sceMcFuncNoWrite    6
#define sceMcFuncNoFlush    10
#define sceMcFuncNoMkdir    11
#define sceMcFuncNoChDir    12
#define sceMcFuncNoGetDir   13
#define sceMcFuncNoFileInfo 14
#define sceMcFuncNoDelete   15
#define sceMcFuncNoFormat   16
#define sceMcFuncNoUnformat 17
#define sceMcFuncNoEntSpace 18
#define sceMcFuncNoRename   19
#define sceMcFuncChgPrior   20

#define SCE_MC_FILE_ATTR_READABLE  0x0001
#define SCE_MC_FILE_ATTR_WRITEABLE 0x0002
#define SCE_MC_FILE_ATTR_EXECUTABLE 0x0004
#define SCE_MC_FILE_ATTR_SUBDIR    0x0020

#define SCE_RDONLY_MC 1

#define sceMcResSucceed    0
#define sceMcResChangedCard -1
#define sceMcResNoFormat   -2
#define sceMcResFullDevice -3
#define sceMcResNoEntry    -4
#define sceMcResDeniedPermit -5
#define sceMcResNotEmpty   -6
#define sceMcResUpLimitHandle -7
#define sceMcResFailReplace -8

#define sceMcExecRun  0
#define sceMcExecIdle -1
#define sceMcExecFinish 1

#define sceMcIniSucceed    0
#define sceMcIniErrKernel  -101
#define sceMcIniOldMcserv  -120
#define sceMcIniOldMcman   -121

int sceMcInit(void);
int sceMcGetInfo(int port, int slot, int *type, int *free, int *format);
int sceMcOpen(int port, int slot, const char *name, int mode);
int sceMcClose(int fd);
int sceMcSeek(int fd, int offset, int mode);
int sceMcRead(int fd, void *buff, int size);
int sceMcWrite(int fd, const void *buff, int size);
int sceMcMkdir(int port, int slot, const char *name);
int sceMcChdir(int port, int slot, const char *newDir, char *currentDir);
int sceMcGetDir(int port, int slot, const char *name, unsigned mode, int maxent, sceMcTblGetDir *table);
int sceMcFormat(int port, int slot);
int sceMcDelete(int port, int slot, const char *name);
int sceMcSync(int mode, int *cmd, int *result);
