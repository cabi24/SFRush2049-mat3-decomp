/* ngcSaveRegion.c - keeps game settings in the OS save region across a soft reset, and handles the reset button.
   Original: D:\rush_ngc\Rush_NGC\NGC\ngcSaveRegion.c; assembly: build/2049LE/asm/NGC/ngcSaveRegion.s */
#include "rush/rush_protos.h"

#define SAVE_REGION_START ((void *)0x80700000)
#define SAVE_REGION_END ((void *)0x81200000)

int g_bResetEnable = 1;
int g_bRegionRead = 0;
int g_bForceReset = 0;
REGION_DATA g_RegionData;

static void ngcSetSaveRegion(void * p, unsigned long len);
static void * ngcGetSaveRegion(void);

/* 0x800b2b10, size 0x50 */
static void ngcSetSaveRegion(void * p, unsigned long len)
{
    OSSetSaveRegion(SAVE_REGION_START, SAVE_REGION_END);
    memcpy(SAVE_REGION_START, p, len);
}

/* 0x800b2b60, size 0x4c */
static void * ngcGetSaveRegion(void)
{
    void * start = 0; /* stack */
    void * end = 0; /* stack */

    if (OSGetResetCode() >> 31) {
        OSGetSavedRegion(&start, &end);
        return start;
    }
    return 0;
}

/* 0x800b2bac, size 0x4c */
void WriteRegion(void)
{
    g_RegionData.AutoSave = g_AutoSave;
    g_RegionData.Vibrate[1] = g_bUseRumble;
    g_RegionData.GameType = 2;
    g_RegionData.Lenth = 0x29;
    ngcSetSaveRegion(&g_RegionData, sizeof(g_RegionData));
}

/* 0x800b2bf8, size 0x78 */
void ReadRegion(void)
{
    void * pRegion; /* r4 */

    pRegion = ngcGetSaveRegion();
    if (pRegion == 0) {
        g_bRegionRead = 0;
    } else {
        g_bRegionRead = 1;
        memcpy(&g_RegionData, pRegion, sizeof(g_RegionData));
        if (gOptions[21] == 0) {
            g_bRegionRead = 0;
        }
        g_bForceReset = g_RegionData.Reset;
    }
}

/* 0x800b2c70, size 0xd0 */
void ResetCheck(void)
{
    static int resetButtonState;
    int reset; /* r0 */

    reset = OSGetResetButtonState();
    if (g_bDvdErrorCode == 1) {
        return;
    }
    if (g_bForceReset) {
        resetButtonState = 1;
        g_bForceReset = 0;
    }
    switch (resetButtonState) {
    case 0:
        if (reset) {
            resetButtonState = 1;
        }
        break;
    case 1:
        if (!reset) {
            resetButtonState = 2;
        }
        break;
    case 2:
        if (g_bResetEnable) {
            PADRecalibrate(0xf0000000);
            VISetBlack(1);
            VIFlush();
            VIWaitForRetrace();
            WriteRegion();
            OSResetSystem(0, 0, 0);
        }
        break;
    }
}
