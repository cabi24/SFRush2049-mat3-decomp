/* titlescr.c - title screen ("PRESS START"). */
#include "rush/rush_protos.h"

static int animText(void);

static const tBlitDef blits[4] = {
    "RUSH_SPLASH_A", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff, 0, 0, "RUSH_SPLASH_B", 255, 0, -1, -1,
    -1, -1, -1, -1, 0, 0xff, 0, 1, "B2", 10, 175, -1, -1, -1, -1, -1, -1, 0, 0x80, 0, 2,
    (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animText, 0
};
static int movieIdx = -1;
static _tBlit *blit;
static int inited;

/* 0x80087890 */
void GoBackToLaunchPage(void)
{
}

/* 0x80087894 */
void updateTitleScreen(void)
{
    if (gSkipToGameState != 1) {
        if (gSkipToGameState != gGameState) {
            setGameState(0x10);
            return;
        }
        gSkipToGameState = 1;
    }
    if (!inited) {
        blit = NewMultiBlit(0, 0, blits, 4);
        SetCountdownTimer(30.0f);
        inited = 1;
        if (gGameStarted && gSoftReset)
            gSoftReset = 0;
        gGameStarted = 0;
        ClearRushControls();
    }
    if ((gActiveEdges & 1) || (gActiveEdges & 2)) {
        if (movieIdx != -1)
            movieIdx = -1;
        if (blit) {
            RemoveBlit(blit);
            blit = NULL;
        }
        inited = 0;
        AudPlayEffect(0x34, 0, 1, 0);
        gPlayers[0].controller = 0;
        gPlayers[1].controller = 5;
        gPlayers[2].controller = 5;
        gPlayers[3].controller = 5;
        setGameState(0x10);
    } else if (gActiveEdges & 4) {
        GoBackToLaunchPage();
    } else if (CountdownTimeOut()) {
        if (blit) {
            RemoveBlit(blit);
            blit = NULL;
        }
        inited = 0;
        HideControllerWarning();
        PauseControllerWarning();
        setGameState(8);
    }
}

/* 0x80087a14 */
static int animText(void)
{
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 1);
    HAL_SetFont(11);
    TB_SetTextColor(22);
    TB_DrawTextScrollBox(160, 188, 320, 112, -1, 0, "PRESS START");
    TB_SetTextColor(0);
    HAL_SetFont(12);
    TB_DrawText(160, 208, gText.single[244]);
    TB_SetJust(0, 3);
    HAL_SetTextDepth(-1.0f);
    return 1;
}
