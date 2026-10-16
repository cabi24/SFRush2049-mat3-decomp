/* controlscr.c - partial control-menu recovery, wave19.
 * NonMatching: four unrecovered functions remain absent; no unit promotion.
 */
#include "rush/rush_protos.h"
#pragma force_active on

static void uninitButtons(signed char p);
static int validOption(signed char p, int option);

static char *buttonNames[11] = {
    "BUTTON_S2G1", "BUTTON_S3G1", "BUTTON_S2G1", "CURSORG1", "AB_BUTTONG1", "ARROW_BGG1",
    "ARROW_FGG1", "BAR_BGG1", "BAR_FGG1", "BUTTON_S1G1", "BUTTON_S0G1"
};

static anon_buttonType_objIdx_40 buttonInfo[4][18] = {
{
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 9, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 10, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 }
},
{
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 9, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 10, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 }
},
{
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 9, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 10, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 }
},
{
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 9, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 10, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 }
}
};
static anon_x_y_18 gPlayerRects[4][4] = {
    80, 0, 160, 220, 160, 110, 0, 0, 320, 220, 160, 110, 0, 0, 320, 220, 160, 110, 0, 0, 320, 220,
    160, 110, 80, 0, 160, 110, 160, 55, 80, 110, 160, 110, 160, 165, 0, 0, 320, 220, 160, 110, 0,
    0, 320, 220, 160, 110, 0, 0, 160, 110, 80, 55, 160, 0, 160, 110, 240, 55, 0, 110, 160, 110, 80,
    165, 0, 0, 320, 220, 160, 110, 0, 0, 160, 110, 80, 55, 160, 0, 160, 110, 240, 55, 0, 110, 160,
    110, 80, 165, 160, 110, 160, 110, 240, 165
};
static signed char gControlValid[26] = { 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 };
signed char controlScrActive[4];
signed char gConflictExists[4];
static signed short gControlSelect[4];
static signed short gControlFirst[4];
static signed short gControlMenuSize;
static signed char prevControlScrActive[4];
static enum_kNumMenuDir_5 exitControlScreen;
static _tBlit_2 *blit;
static signed char inited;
static float buttonX;
static float gCameraMatrix[4][4];
static float scrollSpeed[4];
static float cursorSpeed[4];
static int buttonsJustInited[4];
static int buttonsInited[4];
signed char gConflict[4][11];


/* 0x8006bb9c, size 0x90 */
void cleanupControlScreen(void)
{
    int playerIdx;
    for (playerIdx = 0; playerIdx < 4; playerIdx++) {
        controlScrActive[playerIdx] = 0;
        prevControlScrActive[playerIdx] = 0;
        uninitButtons(playerIdx);
    }
    if (blit) {
        RemoveBlit((_tBlit *)blit);
        blit = 0;
    }
    inited = 0;
}

/* 0x8006c1d8, size 0x13c */
static int animControls2(_tBlit_2 *blt)
{
    int conflict = (blt->animID >> 4) & 15;
    int uninited = (blt->animID >> 8) & 15;
    int p = (blt->animID >> 12) & 15;
    if (Hidden(blt, gPlayers[p].controller == 5 || controlScrActive[p] == 1))
        return 1;
    if (uninited) {
        blt->x = gPlayerRects[gNumWindows - 1][p].cx + 30;
        blt->y = gPlayerRects[gNumWindows - 1][p].y + 32;
        UpdateBlit((_tBlit *)blt);
        blt->animID &= 0x00fff0ff;
        if (conflict) {
            blt->animID = (blt->animID & 0xffff) | 0x1c0000;
            SelectBlit((_tBlit *)blt, 0, blt->width, blt->width);
        }
        UpdateBlit((_tBlit *)blt);
    }
    return 1;
}

/* 0x8006cd08, size 0xac */
static void calcControlMenuMaxSize(void)
{
    int rectHeight = gPlayerRects[gNumWindows - 1][0].h;
    int i;
    int gControlMenuCount;
    HAL_SetFont(11);
    gControlMenuSize = (rectHeight - 24) / 16;
    gControlMenuCount = 2;
    for (i = 0; i < 13; i++) {
        if (validOption(0, i)) gControlMenuCount++;
    }
    if (gControlMenuSize > gControlMenuCount || (gGameState & 0x7c03fffe))
        gControlMenuSize = gControlMenuCount;
}

/* 0x8006cdb4, size 0x8 */
static int validOption(signed char p, int option)
{
    return 1;
}

/* 0x8006cdbc, size 0xa0 */
static int validControl(signed char p)
{
    signed short cur_action = gControlSelect[p];
    signed short cur_control = gNewControls[gPlayers[p].controller][cur_action];
    if (!gControlValid[cur_control]) return 0;
    if (cur_action != 2 && (cur_control == 19 || cur_control == 20)) return 0;
    if (cur_action == 2 && cur_control != 19 && cur_control != 20) return 0;
    return 1;
}

/* 0x8006d1d8, size 0x94 */
static void uninitButtons(signed char p)
{
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 18; buttonIdx++) {
        if (buttonInfo[p][buttonIdx].objIdx != -1) {
            ZOID_RemoveObject(buttonInfo[p][buttonIdx].objIdx);
            buttonInfo[p][buttonIdx].objIdx = -1;
        }
    }
    buttonsInited[p] = 0;
}

/* 0x8006ce5c, size 0x37c */
static void initButtons(signed char p)
{
    anon_c_rgba_4 buttonColor = {255, 255, 255, 255};
    anon_name_width_30 *tex;
    unsigned short texIdx;
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 18; buttonIdx++) {
        if (buttonInfo[p][buttonIdx].objIdx == -1) {
            CopyUV(unitmatrix, buttonInfo[p][buttonIdx].mat.uvs);
            buttonInfo[p][buttonIdx].objIdx = MBOX_NewObject(MBOX_FindObject_WORK(buttonNames[buttonInfo[p][buttonIdx].buttonType], 0, gNumModels - 1, 1), &buttonInfo[p][buttonIdx].mat, -1,
                (buttonInfo[p][buttonIdx].buttonType == 0 || buttonInfo[p][buttonIdx].buttonType == 1 || buttonInfo[p][buttonIdx].buttonType == 2 || buttonInfo[p][buttonIdx].buttonType == 9 || buttonInfo[p][buttonIdx].buttonType == 10) ? 0x42000 : 0);
            ZOID_SetColor(buttonInfo[p][buttonIdx].objIdx, &buttonColor);
            MBOX_HideObject(buttonInfo[p][buttonIdx].objIdx, 1, 15);
            MBOX_ShowObject(buttonInfo[p][buttonIdx].objIdx, 0, 1 << p);
        }
    }
    tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[p][14].objIdx, tex, -1);
    CopyUV(unitmatrix, buttonInfo[p][14].mat.uvs);
    WPitchUV(-1.5707964f, buttonInfo[p][14].mat.uvs);
    buttonInfo[p][14].mat.uvs[3][0] = 0.0f;
    buttonInfo[p][14].mat.uvs[3][1] = gNumWindows > 1 ? 105.0f : 115.0f;
    buttonInfo[p][14].mat.uvs[3][2] = 200.0f;
    buttonInfo[p][14].mat.uvs[3][0] = buttonInfo[p][14].mat.uvs[3][0] * 1.0f;
    buttonInfo[p][14].mat.uvs[3][1] = buttonInfo[p][14].mat.uvs[3][1] * 1.0f;

    tex = MBOX_FindTexture_WORK("CURSOR", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[p][15].objIdx, tex, -1);
    tex = MBOX_FindTexture_WORK("B1", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[p][16].objIdx, tex, -1);
    CopyUV(unitmatrix, buttonInfo[p][16].mat.uvs);
    WPitchUV(-1.5707964f, buttonInfo[p][16].mat.uvs);
    buttonInfo[p][16].mat.uvs[3][0] = -75.1f;
    buttonInfo[p][16].mat.uvs[3][1] = 4.0f + (gNumWindows > 1 ? 50.2f : 55.2f);
    buttonInfo[p][16].mat.uvs[3][2] = 100.0f;
    buttonInfo[p][16].mat.uvs[3][0] = buttonInfo[p][16].mat.uvs[3][0] * 1.0f;
    buttonInfo[p][16].mat.uvs[3][1] = buttonInfo[p][16].mat.uvs[3][1] * 1.0f;
    buttonInfo[p][16].mat.uvs[3][2] = buttonInfo[p][16].mat.uvs[3][2] * 1.0f;

    ScaleUV(buttonInfo[p][16].mat.uvs, buttonInfo[p][16].mat.uvs, 0.5f);
    CopyUV(unitmatrix, buttonInfo[p][17].mat.uvs);
    WPitchUV(-1.5707964f, buttonInfo[p][17].mat.uvs);
    buttonInfo[p][17].mat.uvs[3][0] = -87.1f;
    buttonInfo[p][17].mat.uvs[3][1] = 4.0f + (gNumWindows > 1 ? 50.2f : 55.2f);
    buttonInfo[p][17].mat.uvs[3][2] = 100.0f;
    buttonInfo[p][17].mat.uvs[3][0] = buttonInfo[p][17].mat.uvs[3][0] * 1.0f;
    buttonInfo[p][17].mat.uvs[3][1] = buttonInfo[p][17].mat.uvs[3][1] * 1.0f;
    buttonsInited[p] = 1;
    buttonsJustInited[p] = 1;
    ZeroCameras();
}


static void animButtonText(signed char p);

/* 0x8006c314, size 0x3b4 */
static int animText(void)
{
    int menuY;
    int i;
    int p;
    int x;
    int y;
    unsigned char *str;
    unsigned char str1[64];
    unsigned char pName[14];
    int itemsDrawn;
    HAL_SetTextDepth(0.0f);
    for (p = 0; p < gNumWindows; p++) {
        if (buttonsInited[p] == 1) {
            animButtonText(p);
        } else {
            HAL_SetFont(11);
            if (gPlayers[p].controller != 5 &&
                (controlScrActive[p] == 1 || (gGameState & 0x7c0000))) {
                y = gPlayerRects[gNumWindows - 1][p].y + 10;
                x = gPlayerRects[gNumWindows - 1][p].cx;
                TB_SetTextColor(playerTextColor[p]);
                if (!gControllerData[p].attached) {
                    TB_Sprintf(str1, "CONTROLLER %d %s", p + 1, gText.single[242]);
                } else {
                    TB_EllipseText(pName, (*gPlayers[p].save)->name, 69);
                    TB_Sprintf(str1, gText.single[37], pName);
                }
                TB_DrawText(TB_CenterText(str1, x), y, str1);
                TB_GetFontLineHeight();
                if (!controlScrActive[p]) {
                    if (gGameState & 0x7c0000) {
                        TB_SetTextColor(1);
                        TB_SetJust(1, 1);
                        TB_DrawTextScrollBox(gPlayerRects[gNumWindows - 1][p].cx,
                            gPlayerRects[gNumWindows - 1][p].cy,
                            gPlayerRects[gNumWindows - 1][p].w / 2,
                            gPlayerRects[gNumWindows - 1][p].h, -1, 0,
                            gText.single[38]);
                        TB_SetJust(0, 3);
                    }
                } else {
                    x = gPlayerRects[gNumWindows - 1][p].cx + 10;
                    menuY = gPlayerRects[gNumWindows - 1][p].y + 24;
                    itemsDrawn = 0;
                    for (i = 0; i < 13; i++) {
                        if (i >= gControlFirst[p] && validOption(p, i)) {
                            if (!PS_PakRumble(gPlayers[p].controller) && i == 11)
                                TB_SetTextColor(3);
                            else
                                TB_SetTextColor(i == gControlSelect[p] ? 22 : 1);
                            str = gText.taba[gText.tab[20] + i];
                            if (i == 12)
                                TB_DrawText(TB_CenterText(str, gPlayerRects[gNumWindows - 1][p].cx), menuY, str);
                            else
                                TB_DrawText(TB_RightText(str, x), menuY, str);
                            if (i == 11) {
                                if (PS_PakRumble(gPlayers[p].controller))
                                    str = gText.taba[gText.tab[18] + (*gPlayers[p].save)->rumble];
                                else
                                    str = gText.taba[gText.tab[17]];
                                TB_DrawText(x + 20, menuY, str);
                            }
                            menuY += 16;
                            if (++itemsDrawn >= gControlMenuSize) break;
                        }
                    }
                }
            }
        }
    }
    HAL_SetTextDepth(-1.0f);
    return 1;
}

#pragma force_active reset
