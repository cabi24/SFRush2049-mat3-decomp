/* cheatscr.c - cheat menu: scrolling list of unlocked cheats, toggled or cycled with left/right.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\cheatscr.c; assembly: build/2049LE/asm/GameSource/src/cheatscr.s */
#include "rush/rush_protos.h"

static void initScreen(void);
static void uninitScreen(void);
static void printOption(int x, int y, int item, const unsigned char *name, const unsigned char *value);
static int animText(void);

static const anon_texname_dulx_24 multiblit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animText, 0 };
static enum_kNumMenuDir_3 gCheatMenuExit;
static int gSelectedOption;
static _tBlit *blit;
static signed char inited;

/* 0x800675b4, size 0x5cc */
void updateCheatScreen(void)
{
    if (!inited) {
        initScreen();
    }
    if (gActiveEdges & 5) {
        AudPlayEffect((gActiveEdges & 1) ? 7 : 1, 0, 1, 0);
        gCheatMenuExit = 2;
    } else if (gActiveAutoEdges & 0xc00) {
        if (gActiveAutoEdges & 0x400) {
            AudPlayEffect(8, 0, 1, 0);
            if (--gSelectedOption < 0) {
                gSelectedOption = 22;
            }
        } else {
            AudPlayEffect(2, 0, 1, 0);
            if (++gSelectedOption >= 23) {
                gSelectedOption = 0;
            }
        }
        InitEgg(gSelectedOption);
    }
    if ((gActiveEdges & 0x3000) && gCheatActive[gSelectedOption]) {
        int dir = (gActiveEdges & 0x1000) ? -1 : 1;

        PlayLRSound(gActiveEdges);
        switch (gSelectedOption) {
        case 0:
            gNoCarCollisions ^= 1;
            gCheatState[0] = gNoCarCollisions;
            break;
        case 1:
            gMineCones ^= 1;
            gCheatState[1] = gMineCones;
            break;
        case 2:
            gCarMines ^= 1;
            gCheatState[2] = gCarMines;
            break;
        case 3:
            gCheatState[3] ^= 1;
            break;
        case 4:
            gNoAutoAbort ^= 1;
            gCheatState[4] = gNoAutoAbort;
            break;
        case 5:
            gSuperSpeed ^= 1;
            gCheatState[5] = gSuperSpeed;
            break;
        case 6:
            gInvincible ^= 1;
            gCheatState[6] = gInvincible;
            break;
        case 7:
            gInvisibleCar ^= 1;
            gCheatState[7] = gInvisibleCar;
            break;
        case 8:
            gInvisibleTrack ^= 1;
            gCheatState[8] = gInvisibleTrack;
            break;
        case 9:
            gNoBrakes ^= 1;
            gCheatState[9] = gNoBrakes;
            break;
        case 10:
            gSpecialTires = !gSpecialTires ? 2 : 0;
            gCheatState[10] = gSpecialTires == 2;
            break;
        case 11:
            gMassiveMass += dir;
            if (gMassiveMass > 2) {
                gMassiveMass = 0;
            } else if (gMassiveMass < 0) {
                gMassiveMass = 2;
            }
            gCheatState[11] = gMassiveMass;
            break;
        case 12:
            gSuicideMode += dir;
            if (gSuicideMode > 2) {
                gSuicideMode = 0;
            } else if (gSuicideMode < 0) {
                gSuicideMode = 2;
            }
            gCheatState[12] = gSuicideMode;
            break;
        case 13:
            gCheatState[13] ^= 1;
            break;
        case 14:
            gDemolitionBattle ^= 1;
            gCheatState[14] = gDemolitionBattle;
            break;
        case 15:
            gRandomWeapons += dir;
            if (gRandomWeapons > 2) {
                gRandomWeapons = 0;
            } else if (gRandomWeapons < 0) {
                gRandomWeapons = 2;
            }
            gCheatState[15] = gRandomWeapons;
            break;
        case 16:
            gCheatState[16] ^= 1;
            break;
        case 17:
            gCheatState[17] ^= 1;
            break;
        case 18:
            gCheatState[18] ^= 1;
            break;
        case 19:
            gResurrectInPlace ^= 1;
            gCheatState[19] = gResurrectInPlace;
            break;
        case 20:
            gFrameScale += dir;
            if (gFrameScale > 3) {
                gFrameScale = 0;
            } else if (gFrameScale < 0) {
                gFrameScale = 3;
            }
            gCheatState[20] = gFrameScale;
            break;
        case 21:
            gEnableTireScaling ^= 1;
            gCheatState[21] = gEnableTireScaling;
            break;
        case 22:
            if (gFogColor < 19) {
                gFogColor = 0;
            } else {
                gFogColor -= 18;
            }
            gFogColor += dir;
            if (gFogColor > 8) {
                gFogColor = 0;
            } else if (gFogColor < 0) {
                gFogColor = 8;
            }
            gCheatState[22] = gFogColor;
            if (gFogColor) {
                gFogColor += 18;
            }
            break;
        }
    }
    if (!gCheatActive[gSelectedOption]) {
        gCheatActive[gSelectedOption] = CheckForEgg();
    }
    if (gSoftReset) {
        uninitScreen();
        setGameState(4);
    } else if (gCheatMenuExit) {
        uninitScreen();
        setGameState(16);
    }
}

/* 0x80067b80, size 0x160 */
static void initScreen(void)
{
    gCheatState[0] = gNoCarCollisions;
    gCheatState[1] = gMineCones;
    gCheatState[2] = gCarMines;
    gCheatState[4] = gNoAutoAbort;
    gCheatState[5] = gSuperSpeed;
    gCheatState[6] = gInvincible;
    gCheatState[7] = gInvisibleCar;
    gCheatState[8] = gInvisibleTrack;
    gCheatState[9] = gNoBrakes;
    gCheatState[15] = gRandomWeapons;
    gCheatState[10] = gSpecialTires == 2;
    gCheatState[11] = gMassiveMass;
    gCheatState[12] = gSuicideMode;
    gCheatState[19] = gResurrectInPlace;
    gCheatState[20] = gFrameScale;
    gCheatState[21] = gEnableTireScaling;
    gCheatState[22] = gFogColor;
    if (gCheatState[22] > 8) {
        gCheatState[22] = 0;
    } else if (gCheatState[22] < 0) {
        gCheatState[22] = 8;
    }
    createBG();
    blit = NewMultiBlit(0, 0, multiblit, 1);
    gSelectedOption = 0;
    gCheatMenuExit = 0;
    InitEgg(gSelectedOption);
    inited = 1;
}

/* 0x80067ce0, size 0x40 */
static void uninitScreen(void)
{
    if (blit) {
        RemoveBlit(blit);
        blit = 0;
    }
    removeBG();
    inited = 0;
}

/* 0x80067d20, size 0x148 */
static void printOption(int x, int y, int item, const unsigned char *name, const unsigned char *value)
{
    HAL_SetFont(gSelectedOption == item ? 11 : 12);
    TB_SetTextColor(gSelectedOption == item ? 22 : 1);
    TB_DrawText(TB_RightText((void *)name, x - 4), y - (gSelectedOption == item ? 1 : 0), (void *)name);
    HAL_SetFont((gCheatActive[item] && gSelectedOption == item) ? 11 : 12);
    TB_SetTextColor(gCheatActive[item] ? (gSelectedOption == item ? 11 : 1) : 4);
    TB_DrawText(x + 12, y - ((gCheatActive[item] && gSelectedOption == item) ? 1 : 0), (void *)value);
}

/* 0x80067e68, size 0x1bc */
static int animText(void)
{
    int i; /* r30 */
    int y; /* r29 */
    int cheat; /* r28 */
    unsigned char *str; /* r7 */
    unsigned char str1[4]; /* stack */

    HAL_SetTextDepth(0.0f);
    HAL_SetFont(10);
    TB_SetTextColor(12);
    TB_DrawText(TB_CenterText(gText.single[54], 151), (18 - HAL_GetFontAscent()) / 2 + 6, gText.single[54]);
    HAL_SetFont(12);
    y = 47;
    for (i = 0; i < 13; i++) {
        cheat = i + gSelectedOption - 6;
        if (cheat < 0) {
            cheat += 23;
        } else if (cheat >= 23) {
            cheat -= 23;
        }
        if (cheat == 20) {
            TB_Sprintf(str1, "%d", gFrameScale);
            str = str1;
        } else if (cheat == 15) {
            TB_Sprintf(str1, "%d", gRandomWeapons);
            str = str1;
        } else {
            str = gText.tab2b[gCheatState[cheat] + 1 + gText.tab2a[*gText.tab2 + cheat]];
        }
        printOption(154, y, cheat, gText.tab2b[gText.tab2a[*gText.tab2 + cheat]], str);
        y += TB_GetFontLineHeight();
    }
    HAL_SetTextDepth(-1.0f);
    return 1;
}

