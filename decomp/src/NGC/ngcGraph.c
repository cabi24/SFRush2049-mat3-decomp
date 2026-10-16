/* ngcGraph.c - PS2 graphics entry points, mostly stubs on GameCube, plus viewport setup. */
#include "rush/rush_protos.h"

float g_ZMax2D = 16777000.0f;
unsigned char FogColorR = 255;
unsigned char FogColorG = 255;
unsigned char FogColorB = 255;
int g_HalfEye;
float g_MTXWorldView[4][4];
float g_MTXViewScreen[4][4];
float g_MTXViewClip[4][4];
float g_MTXWorldScreen[4][4];
viewWindow g_VW;

/* sets viewport and scissor from the same float rectangle (the scissor gets the float->u32 conversions) */
inline void SetViewportScissor(int x, int y, unsigned int w, unsigned int h)
{
    float fh;
    float fw;
    float fy;
    float fx;

    fx = x;
    fy = y;
    fw = w;
    fh = h;
    GXSetViewport(fx, fy, fw, fh, 0.0f, 1.0f);
    GXSetScissor(fx, fy, fw, fh);
}

/* 0x80097890 */
void ClearFrame(void)
{
}

/* 0x80097894 */
void SetFrameRate(void)
{
}

/* 0x80097898 (no return value in the original) */
_eGraphFpsMode ps2GraphGetFpsMode(void)
{
}

/* 0x8009789c */
void PS2SwapBuffer(void)
{
}

/* 0x800978a0 */
void PS2SetRenderTarget(anon_vp_s_48 *win)
{
    int w;
    int h;
    int cy;
    int x;
    int y;

    w = win->wwidth;
    h = win->wheight;
    cy = win->wvcenter;
    x = (int)win->whcenter - w / 2;
    y = cy - h / 2;
    if (w == 640 && (cy == 112 || cy == 336))
        g_HalfEye = 1;
    else
        g_HalfEye = 0;
    g_VW.x = x;
    g_VW.y = y;
    g_VW.w = w;
    g_VW.h = h;
    SetViewportScissor(x, y, g_VW.w, g_VW.h);
    NGCSetFog(win);
}

/* 0x80097a74 */
void StartGraphics(void)
{
    ClearFrame();
    BlitInit();
    HAL_StartText();
}

/* 0x80097a9c */
void EndGraphics(void)
{
    BlitFinish();
    statsClock(8);
}

/* 0x80097ac4 */
void ClearScreen(void)
{
}

/* 0x80097ac8 */
void _InitGfx(void)
{
}

/* 0x80097acc */
void SwitchFrameMode(void)
{
}

/* 0x80097ad0 (no return value in the original) */
int isInLowRes(void)
{
}

/* 0x80097ad4 */
void ps2GraphResumeSwappingThread(void)
{
}

/* 0x80097ad8 */
void WaitTillDMAEnd(void)
{
}
