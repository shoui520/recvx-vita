/*
 * libpad: a DualShock 2 in analog + pressure mode, fed from SceCtrl.
 *
 * The Vita has no L2/R2/L3/R3: L2/R2 are the left/right halves of the rear
 * touchpad, L3/R3 the left/right halves of the front touchscreen.
 */
#include <string.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include "libpad.h"
#include "recvx_platform.h"

/* DualShock 2 button bits (active low on the wire) */
enum {
    DS_SELECT = 0x0001, DS_L3 = 0x0002, DS_R3 = 0x0004, DS_START = 0x0008,
    DS_UP = 0x0010, DS_RIGHT = 0x0020, DS_DOWN = 0x0040, DS_LEFT = 0x0080,
    DS_L2 = 0x0100, DS_R2 = 0x0200, DS_L1 = 0x0400, DS_R1 = 0x0800,
    DS_TRIANGLE = 0x1000, DS_CIRCLE = 0x2000, DS_CROSS = 0x4000, DS_SQUARE = 0x8000,
};

static int press_mode;

/*
 * Scripted input for unattended runs: RECVX_DATA_DIR/input.txt holds lines
 * "FRAME BUTTON[+BUTTON...] [HOLD]" (frames presented; HOLD defaults to 6).
 */
unsigned gs_frame_count(void);

#define MAX_SCRIPT 256
static struct { u_int frame, hold, buttons; } script[MAX_SCRIPT];
static int nscript = -1;

static u_int button_bit(const char *name)
{
    static const struct { const char *n; u_int b; } names[] = {
        { "select", DS_SELECT }, { "l3", DS_L3 }, { "r3", DS_R3 }, { "start", DS_START },
        { "up", DS_UP }, { "right", DS_RIGHT }, { "down", DS_DOWN }, { "left", DS_LEFT },
        { "l2", DS_L2 }, { "r2", DS_R2 }, { "l1", DS_L1 }, { "r1", DS_R1 },
        { "triangle", DS_TRIANGLE }, { "circle", DS_CIRCLE }, { "cross", DS_CROSS }, { "square", DS_SQUARE },
    };
    for (u_int i = 0; i < sizeof(names) / sizeof(names[0]); i++)
        if (!strcmp(name, names[i].n)) return names[i].b;
    return 0;
}

static void script_load(void)
{
    nscript = 0;
    FILE *f = fopen(RECVX_DATA_DIR "/input.txt", "r");
    if (!f) return;
    char line[128];
    while (nscript < MAX_SCRIPT && fgets(line, sizeof(line), f)) {
        char names[96];
        u_int frame, hold = 6;
        if (sscanf(line, "%u %95s %u", &frame, names, &hold) < 2) continue;
        u_int b = 0;
        for (char *t = strtok(names, "+"); t; t = strtok(NULL, "+")) b |= button_bit(t);
        script[nscript].frame = frame;
        script[nscript].hold = hold;
        script[nscript].buttons = b;
        nscript++;
    }
    fclose(f);
    RECVX_LOG("pad: %d scripted inputs", nscript);
}

static u_int script_buttons(void)
{
    if (nscript < 0) script_load();
    u_int now = gs_frame_count(), b = 0;
    for (int i = 0; i < nscript; i++)
        if (now >= script[i].frame && now < script[i].frame + script[i].hold) b |= script[i].buttons;
    return b;
}

int scePadInit(int mode)
{
    (void)mode;
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);
    return 1;
}

int scePadEnd(void) { return 1; }
int scePadPortOpen(int port, int slot, u_long128 *addr) { (void)slot; (void)addr; return port == 0; }
int scePadPortClose(int port, int slot) { (void)port; (void)slot; return 1; }

int scePadGetState(int port, int slot)
{
    (void)slot;
    return port == 0 ? scePadStateStable : scePadStateDiscon;
}

int scePadGetReqState(int port, int slot) { (void)port; (void)slot; return scePadReqStateComplete; }

int scePadInfoMode(int port, int slot, int term, int offs)
{
    (void)port; (void)slot; (void)offs;
    switch (term) {
    case InfoModeCurID: return 7;      /* analog controller */
    case InfoModeCurExID: return 7;
    case InfoModeCurExOffs: return 0;
    case InfoModeIdTable: return offs == -1 ? 2 : (offs == 0 ? 4 : 7);
    }
    return 0;
}

int scePadSetMainMode(int port, int slot, int offs, int lock) { (void)port; (void)slot; (void)offs; (void)lock; return 1; }
int scePadInfoAct(int port, int slot, int actno, int term) { (void)port; (void)slot; (void)actno; (void)term; return 0; }
int scePadSetActAlign(int port, int slot, const u_char *data) { (void)port; (void)slot; (void)data; return 1; }
int scePadSetActDirect(int port, int slot, const u_char *data) { (void)port; (void)slot; (void)data; return 1; }
int scePadInfoPressMode(int port, int slot) { (void)port; (void)slot; return 1; }
int scePadEnterPressMode(int port, int slot) { (void)port; (void)slot; press_mode = 1; return 1; }
int scePadExitPressMode(int port, int slot) { (void)port; (void)slot; press_mode = 0; return 1; }

static u_int touch_halves(SceTouchPortType port)
{
    SceTouchData t;
    u_int r = 0;
    if (sceTouchPeek(port, &t, 1) < 1) return 0;
    for (u_int i = 0; i < t.reportNum; i++)
        r |= t.report[i].x < 960 ? 1 : 2;   /* panel x is 0..1919 */
    return r;
}

int scePadRead(int port, int slot, u_char *rdata)
{
    (void)slot;
    if (port != 0) return 0;

    SceCtrlData c;
    memset(&c, 0, sizeof(c));
    sceCtrlPeekBufferPositive(0, &c, 1);

    static const struct { u_int vita, ds; } map[] = {
        { SCE_CTRL_SELECT, DS_SELECT }, { SCE_CTRL_START, DS_START },
        { SCE_CTRL_UP, DS_UP }, { SCE_CTRL_RIGHT, DS_RIGHT }, { SCE_CTRL_DOWN, DS_DOWN }, { SCE_CTRL_LEFT, DS_LEFT },
        { SCE_CTRL_LTRIGGER, DS_L1 }, { SCE_CTRL_RTRIGGER, DS_R1 },
        { SCE_CTRL_TRIANGLE, DS_TRIANGLE }, { SCE_CTRL_CIRCLE, DS_CIRCLE },
        { SCE_CTRL_CROSS, DS_CROSS }, { SCE_CTRL_SQUARE, DS_SQUARE },
    };
    u_int b = 0;
    for (u_int i = 0; i < sizeof(map) / sizeof(map[0]); i++)
        if (c.buttons & map[i].vita) b |= map[i].ds;
    u_int rear = touch_halves(SCE_TOUCH_PORT_BACK), front = touch_halves(SCE_TOUCH_PORT_FRONT);
    if (rear & 1) b |= DS_L2;
    if (rear & 2) b |= DS_R2;
    if (front & 1) b |= DS_L3;
    if (front & 2) b |= DS_R3;
    b |= script_buttons();

    memset(rdata, 0, 32);
    rdata[0] = 0;                              /* success */
    rdata[1] = press_mode ? 0x79 : 0x73;       /* analog, 9 or 3 halfwords */
    rdata[2] = ~b & 0xff;
    rdata[3] = ~b >> 8 & 0xff;
    rdata[4] = c.rx;
    rdata[5] = c.ry;
    rdata[6] = c.lx;
    rdata[7] = c.ly;
    /* pressure: right left up down triangle circle cross square L1 R1 L2 R2 */
    static const u_int pr[12] = { DS_RIGHT, DS_LEFT, DS_UP, DS_DOWN, DS_TRIANGLE, DS_CIRCLE,
                                  DS_CROSS, DS_SQUARE, DS_L1, DS_R1, DS_L2, DS_R2 };
    for (int i = 0; i < 12; i++) rdata[8 + i] = (b & pr[i]) ? 0xff : 0;
    return press_mode ? 20 : 8;
}
