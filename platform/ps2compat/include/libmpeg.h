/* libmpeg: PSS demux/decode. Movies are pre-converted for SceAvPlayer, so this is declarations only. */
#pragma once
#include "eetypes.h"
#include "libipu.h"

typedef enum {
    sceMpegCbError, sceMpegCbNodata, sceMpegCbStopDMA, sceMpegCbRestartDMA,
    sceMpegCbBackground, sceMpegCbTimeStamp, sceMpegCbStr
} sceMpegCbType;

typedef enum {
    sceMpegStrM2V = 0, sceMpegStrIPU = 1, sceMpegStrPCM = 2, sceMpegStrADPCM = 3, sceMpegStrDATA = 4
} sceMpegStrType;

typedef struct {
    int width, height, frameCount;
    int64_t pts, dts;
    u_long flags;
    int64_t pts2nd, dts2nd;
    u_long flags2nd;
    void *sys;
} sceMpeg;

typedef struct { sceMpegCbType type; } sceMpegCbData;
typedef struct { sceMpegCbType type; char *errMessage; } sceMpegCbDataError;
typedef struct { sceMpegCbType type; int64_t pts, dts; int frameCount; } sceMpegCbDataTimeStamp;
typedef struct { sceMpegCbType type; u_char *header; u_char *data; u_int len; int64_t pts, dts; } sceMpegCbDataStr;

typedef int (*sceMpegCallback)(sceMpeg *mp, sceMpegCbData *cbdata, void *anyData);

int  sceMpegInit(void);
int  sceMpegCreate(sceMpeg *mp, u_char *work_area, int work_area_size);
int  sceMpegDelete(sceMpeg *mp);
int  sceMpegReset(sceMpeg *mp);
int  sceMpegIsEnd(sceMpeg *mp);
int  sceMpegGetPicture(sceMpeg *mp, sceIpuRGB32 *rgb32, int mbcount);
sceMpegCallback sceMpegAddCallback(sceMpeg *mp, sceMpegCbType type, sceMpegCallback func, void *anyData);
int  sceMpegAddStrCallback(sceMpeg *mp, sceMpegStrType strType, int ch, void *cb, void *data);
int  sceMpegDemuxPssRing(sceMpeg *mp, u_char *start, int size, u_char *bufStart, int bufSize);
