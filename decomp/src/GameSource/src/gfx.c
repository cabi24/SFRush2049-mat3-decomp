/* gfx.c - top-level graphics init/update. */
#include "rush/rush_protos.h"

signed char gRenderInProgress;

/* 0x8001979c */
void InitGraphics(void)
{
    _InitGfx();
    ZOID_Init();
    InitBlits();
    InitCameras();
}

/* 0x800197c8 */
void UpdateGraphics(int drawBlits, int updateEnvirons)
{
    statsUnclock(6);
    statsClock(5);
    if (drawBlits)
        DrawBlits();
    if (updateEnvirons)
        UpdateEnvirons();
    UpdateCameras();
    statsClock(10);
    ZOID_EndFrame(0);
    statsUnclock(10);
    statsUnclock(5);
    statsClock(6);
    if (!DRAW_TRC_DVD() && !DRAW_TRC_CONTROLLER())
        updatePauseMenuxb();
}

/* 0x8001986c */
void ShowSystemInfo(void)
{
}
