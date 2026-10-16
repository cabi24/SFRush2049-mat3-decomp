/* Init.c - system start-up and PAL60 switching. */
#include "rush/rush_protos.h"

/* 0x800248d0 */
void Pal60(int on)
{
    if (on) {
        EndGraphics();
        StartGraphics();
        EndGraphics();
        StartGraphics();
        osTvType = 1;
        gRetraceRate = 60.0f;
        gRetraceTime = 0.016666668f;
    } else {
        EndGraphics();
        StartGraphics();
        EndGraphics();
        StartGraphics();
        osTvType = 0;
        gRetraceRate = 50.0f;
        gRetraceTime = 0.02f;
    }
}

/* 0x80024948 */
void InitSystem(void)
{
    ngcInit(NULL);
    SaveInit();
    InitGraphics();
    HAL_InitText();
    GDFS_Init();
    AudioInit(64, 64, 1, 5, 64, 1.0f, 200.0f, 1);
    InitRushControls();
    SetControllerActive(-1, 0);
    PS_InitPakSys(PS_ErrorFunc);
    PS_PakServer(1, 0);
    if (!appCheckMC(1)) {
        ngcLoadFromMemcard(0);
        g_bAutoLoaded = 1;
    }
    InitOptions();
    InitializeTimer();
    statsInit();
}

/* 0x800249f4 */
void ExitSystem(void)
{
}
