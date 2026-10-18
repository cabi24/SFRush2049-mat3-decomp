/* front.c - front-end screen dispatch and the scrolling menu background.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\front.c; assembly: build/2049LE/asm/GameSource/src/front.s */
#include "rush/rush_protos.h"

static int animBG(void);

#define BG_DIST 16777000.0f

static const tBlitDef frontBack[1] = { "BACKGROUND", 0, 0, 640, 448, 16, 463, 0, 639, 0x400, 0xff, (void *)&animBG, 0 };
static const tBlitDef trackBack[1] = {
    "BACKGROUND", 0, 0, 640, 448, 560, 1007, 0, 639, 0x7fff94, 0xff, (void *)&animBG, 0
};
static anon_minX_maxX_38 gBackRects[4][4] = {
    -1.0f, 0.0f, -1.0f, 0.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    0.0f, -1.0f, 0.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
    0.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -2.0f, 0.0f, -1.0f, 1.0f,
    0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, -1.0f, 1.0f, 0.2f,
    0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 1.0f, -1.0f, 0.2f, 0.2f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -2.0f, 0.0f, 1.0f, -1.0f, 0.2f, 0.2f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, -1.0f, 1.0f, 0.2f, 0.2f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 1.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, -1.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, -1.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, -1.0f, 1.0f, -1.0f, 1.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 1.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, -1.0f, 1.0f, -1.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, 1.0f, -1.0f, 0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};
static int gBackCamera[4][4] = { 1, 1, 1, 1, 2, 2, 1, 1, 4, 8, 2, 1, 4, 8, 2, 1 };
static tPoly *backPolys[4] = { 0, 0, 0, 0 };
static int offsetsValid[4] = { 0, 0, 0, 0 };
int inmenu;
static _tBlit_2 *backBlit;

static int animBG(void);

/* 0x80013aa4, size 0x15c */
void updateFrontEnd(void)
{
    inmenu = 0;
    if (gGameState & 0x2) {
        updateLicenseScreen();
    } else if (gGameState & 0x40000000) {
        updateWarningScreen();
    } else if (gGameState & 0x4) {
        updateTitleScreen();
    } else if (gGameState & 0x8) {
        updateDemoGame();
    } else if (gGameState & 0x10) {
        gMirrorWaves = 0;
        MirrorWaveInstance(FindWaveInstance(40));
        updateStartScreen();
    } else if (gGameState & 0x2000) {
        updateRecordScreen();
    } else if (gGameState & 0x20) {
        updatePlayerScreen();
    } else if (gGameState & 0x40) {
        updateModeScreen();
    } else if (gGameState & 0x4000) {
        updateCircuitScreen();
    } else if (gGameState & 0x80) {
        updateTrackScreen();
    } else if (gGameState & 0x100) {
        inmenu = 1;
        updateCarScreen();
    } else if (gGameState & 0x200) {
        updateOptionScreen();
    } else if (gGameState & 0x400) {
        updateControlScreen();
    } else if (gGameState & 0x800) {
        updateAudioScreen();
    } else if (gGameState & 0x1000) {
        updateVideoScreen();
    } else if (gGameState & 0x4000000) {
        updatePakScreen();
    } else if (gGameState & 0x8000000) {
        updateCheatScreen();
    }
    updateBG();
    SaveServer(0, 0);
}

/* 0x80013c00, size 0x220 */
void createBG(void)
{
    int polyIdx; /* r23 */
    float polyVerts[4][3]; /* stack */
    unsigned short texIdx; /* stack */
    tPoly *backPoly; /* r6 */
    int vertIdx; /* r7 */
    for (polyIdx = 0; polyIdx < 4; polyIdx++) {
        tRGBA backColor = { 0x00, 0x00, 0xc0, 0xff }; /* stack */

        MBOX_FindTexture_WORK("BACKGROUND_SFX", &texIdx, 0, gNumModels - 1, 1);
        polyVerts[0][0] = BG_DIST * gBackRects[gNumWindows - 1][polyIdx].maxX;
        polyVerts[0][1] = BG_DIST * gBackRects[gNumWindows - 1][polyIdx].maxY;
        polyVerts[0][2] = BG_DIST;
        polyVerts[1][0] = BG_DIST * gBackRects[gNumWindows - 1][polyIdx].minX;
        polyVerts[1][1] = BG_DIST * gBackRects[gNumWindows - 1][polyIdx].maxY;
        polyVerts[1][2] = BG_DIST;
        polyVerts[2][0] = BG_DIST * gBackRects[gNumWindows - 1][polyIdx].minX;
        polyVerts[2][1] = BG_DIST * gBackRects[gNumWindows - 1][polyIdx].minY;
        polyVerts[2][2] = BG_DIST;
        polyVerts[3][0] = BG_DIST * gBackRects[gNumWindows - 1][polyIdx].maxX;
        polyVerts[3][1] = BG_DIST * gBackRects[gNumWindows - 1][polyIdx].minY;
        polyVerts[3][2] = BG_DIST;
        backPolys[polyIdx] = ZOID_NewPoly(4, polyVerts, texIdx, &backColor, gBackCamera[gNumWindows - 1][polyIdx] | 0x1210, 1);
        ZOID_ShowPoly(backPolys[polyIdx]);
        if (offsetsValid[gNumWindows - 1] == 1) {
            backPoly = backPolys[polyIdx];
            for (vertIdx = 0; vertIdx < backPoly->nverts; vertIdx++) {
                backPoly->verts[vertIdx].tc[0] = gBackRects[gNumWindows - 1][polyIdx].offX[vertIdx];
                backPoly->verts[vertIdx].tc[1] = gBackRects[gNumWindows - 1][polyIdx].offY[vertIdx];
            }
        }
    }
    if (gGameState & 0x80) {
        backBlit = (_tBlit_2 *)NewMultiBlit(0, 0, trackBack, 1);
    } else {
        backBlit = (_tBlit_2 *)NewMultiBlit(0, 0, frontBack, 1);
    }
    ZOID_ForceDrawFirstBlit(backBlit->blidx);
}

/* 0x80013e20, size 0x1d4 */
void updateBG(void)
{
    int polyIdx; /* r10 */
    tPoly *backPoly; /* r11 */
    int vertIdx; /* r5 */
    for (polyIdx = 0; polyIdx < 4; polyIdx++) {
        backPoly = backPolys[polyIdx];
        if (backPoly) {
            float wrapX = 0.0f;
            float wrapY = 0.0f;
            float shiftX;
            float shiftY;

            for (vertIdx = 0; vertIdx < backPoly->nverts; vertIdx++) {
                backPoly->verts[vertIdx].tc[0] += gFrameSeconds * gBackRects[gNumWindows - 1][polyIdx].delX;
                backPoly->verts[vertIdx].tc[1] += gFrameSeconds * gBackRects[gNumWindows - 1][polyIdx].delY;
            }
            for (vertIdx = 0; vertIdx < backPoly->nverts; vertIdx++) {
                if (backPoly->verts[vertIdx].tc[0] >= 1.0f) {
                    wrapX -= 1.0f;
                }
                if (backPoly->verts[vertIdx].tc[0] <= -1.0f) {
                    wrapX += 1.0f;
                }
                if (backPoly->verts[vertIdx].tc[1] >= 1.0f) {
                    wrapY -= 1.0f;
                }
                if (backPoly->verts[vertIdx].tc[1] <= -1.0f) {
                    wrapY += 1.0f;
                }
            }
            if (wrapX == -4.0f) {
                shiftX = -1.0f;
            } else if (wrapX == 4.0f) {
                shiftX = 1.0f;
            } else {
                shiftX = 0.0f;
            }
            if (wrapY == -4.0f) {
                shiftY = -1.0f;
            } else if (wrapY == 4.0f) {
                shiftY = 1.0f;
            } else {
                shiftY = 0.0f;
            }
            for (vertIdx = 0; vertIdx < backPoly->nverts; vertIdx++) {
                backPoly->verts[vertIdx].tc[0] += shiftX;
                backPoly->verts[vertIdx].tc[1] += shiftY;
                gBackRects[gNumWindows - 1][polyIdx].offX[vertIdx] = backPoly->verts[vertIdx].tc[0];
                gBackRects[gNumWindows - 1][polyIdx].offY[vertIdx] = backPoly->verts[vertIdx].tc[1];
            }
            offsetsValid[gNumWindows - 1] = 1;
        }
    }
}

/* 0x80013ff4, size 0x80 */
void removeBG(void)
{
    int polyIdx; /* r29 */
    if (backBlit) {
        RemoveBlit((_tBlit *)backBlit);
        backBlit = 0;
    }
    for (polyIdx = 0; polyIdx < 4; polyIdx++) {
        if (backPolys[polyIdx]) {
            ZOID_RemovePoly(backPolys[polyIdx]);
            backPolys[polyIdx] = 0;
        }
    }
}

/* 0x80014074, size 0x8 */
static int animBG(void)
{
    return 1;
}

