/* ngcCamera.c - camera and projection matrices (PS2 libvu0-style code kept for the NGC port).
   Original: D:\rush_ngc\Rush_NGC\NGC\ngcCamera.c; assembly: build/2049LE/asm/NGC/ngcCamera.s */
#include "rush/rush_protos.h"

/* a matrix inside PS2UpdateCameraMatrix's VU1 environment block */
#define VU1MTX(off) ((float (*)[4])((char *)vu1env + (off)))

float camera_p[4] = { 0 };
float camera_zd[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
float camera_yd[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
float camera_rot[4] = { 0 };
float clip_volume[4] = { 500.0f, 400.0f, 16.0f, 16777215.0f };
float light0[4] = { -0.5f, 0.5f, 0.5f, 0.0f };
float light1[4] = { -0.5f, -1.5f, -1.0f, 0.0f };
float light2[4] = { 1.0f, -0.5f, 0.5f, 0.0f };
float ambient[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float color0[4] = { 0 };
float color1[4] = { 0 };
float color2[4] = { 0 };
int free_camera;
int _CameraUsed;
static int _curIdx;
float g_NGCMTXWorldView[3][4];

/* 0x800a54a0, size 0xb4 */
/* the first parameter is unnamed in the original */
void sceVu0RotCameraMatrix(float (*m)[4], float *p, float *zd, float *yd, float *rot)
{
    float work[4][4]; /* stack */
    float direction[4]; /* stack */
    float vertical[4]; /* stack */
    float position[4]; /* stack */

    sceVu0UnitMatrix(work);
    sceVu0RotMatrixX(work, work, rot[0]);
    sceVu0RotMatrixY(work, work, rot[1]);
    sceVu0RotMatrixZ(work, work, rot[2]);
    sceVu0ApplyMatrix(direction, work, zd);
    sceVu0ApplyMatrix(vertical, work, yd);
    sceVu0ApplyMatrix(position, work, p);
}

/* 0x800a5554, size 0xbc */
void WorldToScreen(float *v, float *x, float *y, float *z)
{
    float inv; /* f12 */

    {
        float sx = v[0] * g_MTXWorldScreen[0][0] + v[1] * g_MTXWorldScreen[1][0] + v[2] * g_MTXWorldScreen[2][0] +
                   g_MTXWorldScreen[3][0];
        float sy = v[0] * g_MTXWorldScreen[0][1] + v[1] * g_MTXWorldScreen[1][1] + v[2] * g_MTXWorldScreen[2][1] +
                   g_MTXWorldScreen[3][1];
        float sz = v[0] * g_MTXWorldScreen[0][2] + v[1] * g_MTXWorldScreen[1][2] + v[2] * g_MTXWorldScreen[2][2] +
                   g_MTXWorldScreen[3][2];
        float sw = v[0] * g_MTXWorldScreen[0][3] + v[1] * g_MTXWorldScreen[1][3] + v[2] * g_MTXWorldScreen[2][3] +
                   g_MTXWorldScreen[3][3];

        inv = 1.0f / sw;
        *x = sx * inv - 2048.0f;
        *y = sy * inv - 2048.0f;
        *z = sz * inv;
    }
}

/* 0x800a5610, size 0x27c */
void SetViewScreenClipMatrix(float (*m)[4], float (*mc)[4], float (*mcs)[4], float scrz, float ax, float ay, float cx, float cy, float zmin, float zmax, float nearz, float farz)
{
    float gsx; /* f29 */
    float gsy; /* f28 */
    float mt[4][4]; /* stack */

    gsx = nearz * clip_volume[0] / scrz;
    gsy = nearz * clip_volume[1] / scrz;
    sceVu0UnitMatrix(m);
    m[0][0] = scrz;
    m[1][1] = scrz;
    m[2][2] = 0.0f;
    m[3][3] = 0.0f;
    m[3][2] = 1.0f;
    m[2][3] = 1.0f;
    sceVu0UnitMatrix(mt);
    mt[0][0] = ax;
    mt[1][1] = ay;
    mt[3][0] = cx;
    mt[3][1] = cy;
    mt[2][2] = farz * nearz * (-zmin + zmax) / (-nearz + farz);
    mt[3][2] = (-zmax * nearz + zmin * farz) / (-nearz + farz);
    sceVu0MulMatrix(m, mt, m);
    sceVu0UnitMatrix(mc);
    mc[0][0] = 2.0f * nearz / (gsx + gsx);
    mc[1][1] = 2.0f * nearz / (gsy + gsy);
    mc[2][2] = (farz + nearz) / (farz - nearz);
    mc[3][2] = -2.0f * (farz * nearz) / (farz - nearz);
    mc[2][3] = 1.0f;
    mc[3][3] = 0.0f;
    sceVu0UnitMatrix(mcs);
    mcs[0][0] = gsx * (scrz * ax) / nearz;
    mcs[1][1] = gsy * (scrz * ay) / nearz;
    mcs[2][2] = (-zmax + zmin) / 2.0f;
    mcs[3][2] = (zmax + zmin) / 2.0f;
    mcs[3][0] = cx;
    mcs[3][1] = cy;
    mcs[3][3] = 1.0f;
}

/* 0x800a588c, size 0x2bc */
void MySetCameraPos(float (*mtx)[3])
{
    float rot[4][4]; /* stack */
    anon_x_y_c_2 camPos; /* stack */
    anon_x_y_c_2 camTar; /* stack */
    anon_x_y_c_2 camUp; /* stack */

    static int update = 1;

    if (free_camera) {
        if (update) {
            sceVu0RotCameraMatrix(g_MTXWorldView, camera_p, camera_zd, camera_yd, camera_rot);
        }
    } else {
        rot[0][0] = mtx[0][0];
        rot[0][1] = mtx[0][1];
        rot[0][2] = mtx[0][2];
        rot[0][3] = 0.0f;
        rot[1][0] = mtx[1][0];
        rot[1][1] = mtx[1][1];
        rot[1][2] = mtx[1][2];
        rot[1][3] = 0.0f;
        rot[2][0] = mtx[2][0];
        rot[2][1] = mtx[2][1];
        rot[2][2] = mtx[2][2];
        rot[2][3] = 0.0f;
        rot[3][2] = 0.0f;
        rot[3][1] = 0.0f;
        rot[3][0] = 0.0f;
        rot[3][3] = 1.0f;
        camera_p[0] = mtx[3][0];
        camera_p[1] = mtx[3][1];
        camera_p[2] = mtx[3][2];
        sceVu0TransMatrix(rot, rot, camera_p);
        sceVu0InversMatrix(g_MTXWorldView, rot);
        if (gMirrorWaves && gUpsideDownTrack) {
            g_MTXWorldView[0][0] = -g_MTXWorldView[0][0];
            g_MTXWorldView[1][0] = -g_MTXWorldView[1][0];
            g_MTXWorldView[2][0] = -g_MTXWorldView[2][0];
            g_MTXWorldView[3][0] = -g_MTXWorldView[3][0];
        } else if (!gUpsideDownTrack) {
            if (gMirrorWaves) {
                g_MTXWorldView[0][0] = -g_MTXWorldView[0][0];
                g_MTXWorldView[1][0] = -g_MTXWorldView[1][0];
                g_MTXWorldView[2][0] = -g_MTXWorldView[2][0];
                g_MTXWorldView[3][0] = -g_MTXWorldView[3][0];
                g_MTXWorldView[0][1] = -g_MTXWorldView[0][1];
                g_MTXWorldView[1][1] = -g_MTXWorldView[1][1];
                g_MTXWorldView[2][1] = -g_MTXWorldView[2][1];
                g_MTXWorldView[3][1] = -g_MTXWorldView[3][1];
            } else {
                g_MTXWorldView[0][1] = -g_MTXWorldView[0][1];
                g_MTXWorldView[1][1] = -g_MTXWorldView[1][1];
                g_MTXWorldView[2][1] = -g_MTXWorldView[2][1];
                g_MTXWorldView[3][1] = -g_MTXWorldView[3][1];
            }
        }
    }
    camPos.x = camera_p[0];
    camPos.y = camera_p[1];
    camPos.z = camera_p[2];
    camTar.x = camPos.x + rot[2][0];
    camTar.y = camPos.y + rot[2][1];
    camTar.z = camPos.z + rot[2][2];
    camUp.x = rot[1][0];
    camUp.y = rot[1][1];
    camUp.z = rot[1][2];
    C_MTXLookAt(g_NGCMTXWorldView, &camPos, &camUp, &camTar);
    if (gMirrorWaves) {
        g_NGCMTXWorldView[0][0] = -g_NGCMTXWorldView[0][0];
        g_NGCMTXWorldView[0][1] = -g_NGCMTXWorldView[0][1];
        g_NGCMTXWorldView[0][2] = -g_NGCMTXWorldView[0][2];
        g_NGCMTXWorldView[0][3] = -g_NGCMTXWorldView[0][3];
    }
}

/* 0x800a5b48, size 0x28 */
void PS2CameraStart(void)
{
    _curIdx++;
    if (_curIdx >= 2) {
        _curIdx = 0;
    }
    _CameraUsed = 0;
}

/* 0x800a5b70, size 0x130 */
void PS2UpdateCameraMatrix(anon_vp_s_48 *win)
{
    static float vu1env[0x1d0 / 4];
    static float nearz = 0.4f;

    if (win->backclip == 4000.0f) {
        g_FogFarZ = 10.0f * win->backclip;
        g_FogNearZ = win->backclip - 1.0f;
    } else {
        g_FogFarZ = win->backclip;
        g_FogNearZ = 0.0f;
    }
    FogColorR = win->fogcolor[0];
    FogColorG = win->fogcolor[1];
    FogColorB = win->fogcolor[2];
    SetViewScreenClipMatrix(g_MTXViewScreen, g_MTXViewClip, VU1MTX(0x180), win->wwidth / 2.0f / win->whvangtan,
                            1.0f, 0.93333f, 2048.0f, 2048.0f, 1.0f, 16777000.0f, nearz, 65536.0f);
    sceVu0MulMatrix(g_MTXWorldScreen, g_MTXViewScreen, g_MTXWorldView);
    sceVu0CopyMatrix(VU1MTX(0x100), g_MTXWorldScreen);
    sceVu0MulMatrix(VU1MTX(0x140), g_MTXViewClip, g_MTXWorldView);
    _CameraUsed++;
    /* the compare survives in the code but its body was optimised away; this reproduces it */
    if (_CameraUsed >= 4) {
        nearz = nearz;
    }
}

