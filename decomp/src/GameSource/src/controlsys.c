/* controlsys.c - controller polling: pad state to button bits, edges, auto-repeat and analog stick ranges.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\controlsys.c; assembly: build/2049LE/asm/GameSource/src/controlsys.s */
#include "rush/rush_protos.h"

static float _PotRange(int pot, float range, float max);
static void _InitSemaphore(void);
static void _GetSemaphore(void);
static void _ReleaseSemaphore(void);

const unsigned int gControlTable[19] = {
    1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000,
    0x10000, 0x20000, 0x40000
};
static signed char gResetLastKeyTime = 1;
unsigned int gActiveEdges;
unsigned int gActiveLevels;
unsigned int gActiveAutoEdges;
signed char gSoftReset;
static volatile signed char gContStickRange[4][2];
static volatile signed char gNewStick[4][2];
static volatile signed char gNewStick2[4][2];
static signed char gActiveControllers[4];
static signed char gStickEdges[4];
static signed char gHardStickEdges[4];
static signed char gInitControls = 0;
static unsigned int gLastKeyTime = 0;
static unsigned char gIgnoreCount = 0;
float gRealStick2[4][2];
float gRealStick[4][2];
unsigned int gRealLevels[4];
unsigned int gRealEdges[4];
anon_support_on_14 g_Pad[4];
static int gAutoEdgeTimes[4][32];
static volatile unsigned int gNewLevels[4];
static volatile unsigned int gNewEdges[4];
unsigned int gAutoEdges[4];
unsigned int gContLevels[4];
unsigned int gContEdges[4];
float gContStick[4][2];
tPadInfo gControllerData[4];

/* 0x8000da18, size 0xa4 */
void InitControlSys(void)
{
    if (!gInitControls) {
        SetControllerActive(-1, 1);
        SetControllerStickEdges(-1, 1);
        memset(gControllerData, 0, sizeof(gControllerData));
        gHardStickEdges[0] = 0;
        gHardStickEdges[1] = 0;
        gHardStickEdges[2] = 0;
        gHardStickEdges[3] = 0;
    }
    gInitControls = 1;
    gContStickRange[0][0] = gContStickRange[0][1] = 110;
    gContStickRange[1][0] = gContStickRange[1][1] = 110;
    gContStickRange[2][0] = gContStickRange[2][1] = 110;
    gContStickRange[3][0] = gContStickRange[3][1] = 110;
    _InitSemaphore();
    UpdateControls();
}

/* 0x8000dabc, size 0xbc */
void ClearControls(void)
{
    int i; /* register only: not in the DWARF */

    _GetSemaphore();
    gActiveEdges = gActiveAutoEdges = gActiveLevels = 0;
    for (i = 0; i < 4; i++) {
        gContStick[i][0] = gContStick[i][1] = 0.0f;
        gContEdges[i] = gContLevels[i] = gAutoEdges[i] = gNewEdges[i] = 0;
    }
    _ReleaseSemaphore();
}

/* 0x8000db78, size 0x368 */
void UpdateControls(void)
{
    unsigned int i; /* r19 */
    unsigned int j; /* r3 */
    float x; /* f0 */
    float y; /* f0 */

    _GetSemaphore();
    if (gResetLastKeyTime) {
        gResetLastKeyTime = 0;
        gLastKeyTime = sc.frameCount;
    }
    gActiveEdges = gActiveAutoEdges = gActiveLevels = 0;
    if (gIgnoreCount) {
        gIgnoreCount--;
        for (i = 0; i < 4; i++) {
            gNewEdges[i] = 0;
            gContStick[i][0] = gContStick[i][1] = 0.0f;
            gContEdges[i] = gContLevels[i] = gAutoEdges[i] = 0;
        }
        _ReleaseSemaphore();
        return;
    }
    for (i = 0; i < 4; i++) {
        x = _PotRange(gNewStick[i][0], gContStickRange[i][0], 1.0f);
        y = _PotRange(gNewStick[i][1], gContStickRange[i][1], 1.0f);
        gContStick[i][0] = x;
        gContStick[i][1] = y;
        gContLevels[i] = gNewLevels[i];
        gContEdges[i] = gNewEdges[i];
        gNewEdges[i] = 0;
        gAutoEdges[i] = 0;
        if (gContLevels[i]) {
            gLastKeyTime = sc.frameCount;
        }
        if (gActiveControllers[i]) {
            gActiveEdges |= gContEdges[i];
            gActiveLevels |= gContLevels[i];
        }
        for (j = 0; j < 32; j++) {
            if (j) {
                if (gContLevels[i] & (1 << j)) {
                    if (!gAutoEdgeTimes[i][j] || sc.frameCount - gAutoEdgeTimes[i][j] > (int)(gRetraceRate * 0.2f)) {
                        gAutoEdgeTimes[i][j] = sc.frameCount;
                        gAutoEdges[i] |= 1 << j;
                    }
                } else {
                    gAutoEdgeTimes[i][j] = 0;
                }
            }
        }
        if (gActiveControllers[i]) {
            gActiveAutoEdges |= gAutoEdges[i];
        }
    }
    _ReleaseSemaphore();
    ModelThread(0);
}

/* 0x8000dee0, size 0x74 */
int GetControllerActive(int which)
{
    int i; /* r0 */

    if (which == -1) {
        for (i = 0; i < 4; i++) {
            if (!gActiveControllers[i]) {
                return 0;
            }
        }
        return 1;
    }
    return gActiveControllers[which];
}

/* 0x8000df54, size 0x34 */
void SetControllerActive(int which, int active)
{
    if (which == -1) {
        gActiveControllers[0] = active;
        gActiveControllers[1] = active;
        gActiveControllers[2] = active;
        gActiveControllers[3] = active;
    } else if (which < 4) {
        gActiveControllers[which] = active;
    }
}

/* 0x8000df88, size 0x74 */
int GetControllerStickEdges(int which)
{
    int i; /* r0 */

    if (which == -1) {
        for (i = 0; i < 4; i++) {
            if (!gStickEdges[i]) {
                return 0;
            }
        }
        return 1;
    }
    return gStickEdges[which];
}

/* 0x8000dffc, size 0x34 */
void SetControllerStickEdges(int which, int active)
{
    if (which == -1) {
        gStickEdges[0] = active;
        gStickEdges[1] = active;
        gStickEdges[2] = active;
        gStickEdges[3] = active;
    } else if (which < 4) {
        gStickEdges[which] = active;
    }
}

/* 0x8000e030, size 0x1e0 */
int ControllerUpdate(void)
{
    anon_support_on_14 *per; /* r0 */
    int i; /* r7 */
    int j; /* r31 */

    for (i = 0; i < 4; i++) {
        per = &g_Pad[i];
        /* a volatile read whose value is unused; the original's statement is unknown */
        if (!gControllerData[i].attached || per->type) {
            gControllerData[i].attached;
        }
        gControllerData[i].availButtons = 0;
        for (j = 0; j < 19; j++) {
            if (per->support & gControlTable[j]) {
                gControllerData[i].availButtons |= 1 << j;
            }
        }
        gControllerData[i].incompatible = (int)(gControllerData[i].availButtons & 0x3ff87) != 0x3ff87;
        gControllerData[i].attached = per->type != 0;
        if (!gControllerData[i].attached) {
            per->on = 0;
            per->l = per->r = per->x1 = per->y1 = per->x2 = per->y2 = 0;
        } else {
            gHardStickEdges[i] = per->type == 1;
        }
        gControllerData[i].buttons = 0;
        for (j = 0; j < 19; j++) {
            if (per->on & gControlTable[j]) {
                gControllerData[i].buttons |= 1 << j;
            }
        }
        gControllerData[i].stick1x = per->x1;
        gControllerData[i].stick1y = per->y1 < -127 ? 127 : -per->y1;
        gControllerData[i].stick2x = per->x2;
        gControllerData[i].stick2y = per->y2 < -127 ? 127 : -per->y2;
        gControllerData[i].analog1 = 0;
        gControllerData[i].analog2 = 0;
    }
    ControlIRQ();
    return 1;
}

/* 0x8000e210, size 0x298 */
void ControlIRQ(void)
{
    unsigned int i; /* r20 */
    unsigned int edges; /* r4 */
    unsigned int levels; /* r0 */

    if (!gInitControls) {
        InitControlSys();
    }
    _GetSemaphore();
    for (i = 0; i < 4; i++) {
        if (!gControllerData[i].attached) {
            gNewStick[i][0] = 0;
            gNewStick[i][1] = 0;
            levels = 0;
            gNewStick2[i][0] = 0;
            gNewStick2[i][1] = 0;
            gContStickRange[i][0] = 110;
            gContStickRange[i][1] = 110;
        } else {
            levels = gControllerData[i].buttons;
            gNewStick[i][0] = gControllerData[i].stick1x;
            gNewStick[i][1] = gControllerData[i].stick1y;
            gNewStick2[i][0] = gControllerData[i].stick2x;
            gNewStick2[i][1] = gControllerData[i].stick2y;
            if ((gNewStick[i][0] > gContStickRange[i][0] || gNewStick[i][0] < -gContStickRange[i][0]) &&
                gContStickRange[i][0] < 127) {
                gContStickRange[i][0]++;
            }
            if ((gNewStick[i][1] > gContStickRange[i][1] || gNewStick[i][1] < -gContStickRange[i][1]) &&
                gContStickRange[i][1] < 127) {
                gContStickRange[i][1]++;
            }
        }
        edges = levels ^ gNewLevels[i];
        edges &= levels;
        gNewEdges[i] |= edges;
        gRealEdges[i] = edges;
        gNewLevels[i] = levels;
        gRealLevels[i] = levels;
        gRealStick[i][0] = _PotRange(gNewStick[i][0], gContStickRange[i][0], 1.0f);
        gRealStick[i][1] = _PotRange(gNewStick[i][1], gContStickRange[i][1], 1.0f);
        gRealStick2[i][0] = _PotRange(gNewStick2[i][0], gContStickRange[i][0], 1.0f);
        gRealStick2[i][1] = _PotRange(gNewStick2[i][1], gContStickRange[i][1], 1.0f);
    }
    _ReleaseSemaphore();
}

/* 0x8000e4a8, size 0x50 */
static float _PotRange(int pot, float range, float max)
{
    float res; /* f1 */

    res = pot * max / range;
    if (res < -max) {
        res = -max;
    } else if (res > max) {
        res = max;
    }
    return res;
}

/* 0x8000e4f8, size 0x18 */
static void _InitSemaphore(void)
{
    static signed char sInited;

    if (!sInited) {
        sInited = 1;
    }
}

/* 0x8000e510, size 0x4 */
static void _GetSemaphore(void)
{

}

/* 0x8000e514, size 0x4 */
static void _ReleaseSemaphore(void)
{

}

