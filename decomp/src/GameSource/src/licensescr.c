/* licensescr.c - start-up: intro movie, then the licence screen. */
#include "rush/rush_protos.h"

static int animText(void);

static const tBlitDef blits[2] = {
    "MIDWAY", 32, 24, -1, -1, -1, -1, -1, -1, 0, 0xff, 0, 0, (void *)0xffffffff, 0, 0, 0, 0, -1,
    -1, -1, -1, 0, 0, (void *)&animText, 0
};
static signed char inited;
static _tBlit *blit;

/* 0x80073884 */
void updateLicenseScreen(void)
{
    if (!pfsInited) {
        if (!lcdInited)
            lcdInited = 1;
        pfsInited = 1;
    }
    if (gIntroMovieShown < 2) {
        gIntroMovieShown++;
        ngcPlayMovie();
        ngcBlackBackground();
        gIntroMovieShown++;
        return;
    }
    if (!inited) {
        blit = NewMultiBlit(0, 0, blits, 2);
        inited = 1;
        SetCountdownTimer(3.0f);
    }
    if (CountdownTimeOut() || (gLicenseShown && (gActiveEdges & 7))) {
        if (blit) {
            RemoveBlit(blit);
            blit = NULL;
        }
        inited = 0;
        setGameState(gLicenseShown ? 4 : 0x40000000);
        gLicenseShown = 1;
    }
}

/* 0x80073994 */
static int animText(void)
{
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(0);
    HAL_SetFontFixedWidth(0);
    HAL_SetFontSpacing(2);
    TB_SetJust(1, 1);
    TB_SetTextColor(1);
    TB_DrawTextScrollBox(160, 168, 288, 112, -1, 0, gText.single[243]);
    TB_SetJust(0, 3);
    HAL_SetFontFixedWidth(1);
    HAL_SetFontSpacing(-1);
    HAL_SetTextDepth(-1.0f);
    return 1;
}
