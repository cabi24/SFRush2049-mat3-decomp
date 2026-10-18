/* sky.c - per-window sky dome objects that follow the camera. */
#include "rush/rush_protos.h"

tUVMatrix *skyMatrices[4];

/* 0x80040d80 */
void SkyInit(void)
{
    int winidx;
    int mbobj;

    if ((unsigned char)(gTrackNum - 7) <= 6 || gTrackNum == 17 || gTrackNum == 18) {
        SkyClear();
        return;
    }
    for (winidx = 0; winidx < gNumWindows; winidx++) {
        mbobj = MBOX_FindObject_WORK("SKYSKY", 0, gNumModels - 1, 1);
        skyMatrices[winidx] = ZOID_NewMatrix();
        skyMatrices[winidx]->mat3.uvs[0][0] = 2.5f;
        skyMatrices[winidx]->mat3.uvs[1][1] = 2.5f;
        skyMatrices[winidx]->mat3.uvs[2][2] = 2.5f;
        MBOX_NewObjectCam(mbobj, skyMatrices[winidx], 1 << winidx, -1, 0);
    }
}

/* 0x80040e7c */
void SkyClear(void)
{
    int winidx;

    for (winidx = 0; winidx < gNumWindows; winidx++)
        skyMatrices[winidx] = NULL;
}

/* 0x80040f1c */
void SkyUpdate(int winidx)
{
    if (skyMatrices[winidx]) {
        skyMatrices[winidx]->mat3.pos[0] = gCamera[winidx].mat.mat3.pos[0];
        skyMatrices[winidx]->mat3.pos[2] = gCamera[winidx].mat.mat3.pos[2];
    }
}
