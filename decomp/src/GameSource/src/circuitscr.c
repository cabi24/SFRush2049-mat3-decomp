/* circuitscr.c - circuit menu helpers. Status: Matching.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\circuitscr.c; assembly: build/2049LE/asm/GameSource/src/circuitscr.s */
#include "rush/rush_protos.h"
#define MSL_NO_SQRTF
#include "msl_math_inline.h"

static void initScreen(void);
static void uninitScreen(void);
static void animButtonText(void);
static int animText(void);
static void drawReview(void);
static void controlShowTrophy(void);
static void controlCircuitCode(void);
static void resetEnabled(void);
static int menuItemValid(int item);
static void getFinishedStr(unsigned char *str);
static void getToGoStr(unsigned char *str);
static void getPlayerStr(int p, unsigned char *str);
static void getPlaceStr(int p, unsigned char *str);
static void getPointsStr(int p, unsigned char *str);
static void getFinishPlaceStr(int p, unsigned char *str);
static void calcProgressSize(void);
static void initButtons(void);
static void uninitButtons(void);
static void updateButtons(void);

static char *buttonNames[9] = {
    "BUTTON_S1G1", "BUTTON_S1G1", "BUTTON_S2G1", "CURSORG1", "AB_BUTTONG1", "ARROW_BGG1",
    "ARROW_FGG1", "BAR_BGG1", "BAR_FGG1"
};
static tMenuButton buttonInfo[12] = { {0,-1}, {0,-1}, {0,-1}, {0,-1}, {1,-1}, {1,-1}, {2,-1}, {3,-1}, {4,-1}, {6,-1}, {4,-1}, {6,-1} };
static const tBlitDef mblit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animText, 0 };
static char *gTrophyNames[3] = { "TGOLDL2_LOD1", "TSILVERL2_LOD1", "TBRONZEL2_LOD1" };
static char *gTrophySheenNames[3] = { "TGOLDL3_LOD1", "TSILVERL3_LOD1", "TBRONZEL3_LOD1" };
static float gButtonScale = 1.0f;
static float gButtonSpace = 40.0f;
static float maxY = 120.0f;
static float buttonZ = 300.0f;
static float cursorZ = 225.0f;
static int trophyObjIdx = -1;
static int trophySheenObjIdx = -1;
static tRGBA trophySheenColor = { 255, 255, 255, 255 };
static tBlitSet *pane;
static enum_kCircuitMenuCount gCircuitMenuSelect;
static enum_kNumCircuitTypes_2 newCircuitType;
static tCircuit *gCircuit;
static signed char gYes;
static signed short gCodeSelect;
static signed short gCodeLen;
static signed short gReviewTrack;
static signed short gProgressWidth;
static signed short gProgressHeight;
static signed short gProgressTop;
static signed short gMenuBottom;
static int buttonsInited = 0;
static int buttonsJustInited = 0;
static float buttonX = 0;
static float minY = 0;
static _tBlit *blit = 0;
static signed char inited = 0;
static enum_kNumMenuDir_4 gCircuitScreenExit = 0;
static signed char gEnterCircuitCode = 0;
static signed char gConfirmNewCircuit = 0;
static signed char gInvalidCode = 0;
static signed char gWrongNumPlayers = 0;
static signed char gContinueEnabled = 0;
static signed char gStatsEnabled = 0;
static signed char gShowTrophy = 0;
static tUVMatrix gTrophyMat;
static unsigned char gCircuitCode[32];

/* 0x80068024, size 1316 */
void updateCircuitScreen(void)
{
    int i;
    gCircuit = &(*(*gPlayers[gCircuitPlayer].save)->datah)->circuit;
    if (gSkipToGameState != 1) {
        if (gSkipToGameState != gGameState) {
            setGameState(0x80);
            return;
        }
        gSkipToGameState = 1;
    }
    if (!inited)
        initScreen();
    while (!menuItemValid(gCircuitMenuSelect)) {
        gCircuitMenuSelect++;
        if (gCircuitMenuSelect > 3)
            gCircuitMenuSelect = 0;
    }
    if (gShowTrophy)
        controlShowTrophy();
    else if (gEnterCircuitCode)
        controlCircuitCode();
    else if (gActiveEdges & 1) {
        AudPlayEffect(7, 0, 1, 0);
        if (gConfirmNewCircuit) {
            gConfirmNewCircuit = 0;
            gYes = 0;
            buttonsJustInited = 1;
        } else if (!menuItemValid(0)) {
            gCircuitMenuSelect = 2;
            goto selectMenu;
        } else {
            ContinueCircuit();
            gCircuitScreenExit = 1;
        }
    } else if (gActiveEdges & 2) {
        AudPlayEffect(0, 0, 1, 0);
selectMenu:
        switch (gCircuitMenuSelect) {
        case 0:
            ContinueCircuit();
            gCircuitScreenExit = 1;
            break;
        case 3:
            gEnterCircuitCode = 1;
            gCodeLen = 0;
            gCodeSelect = 0;
            TB_Strcpy(gCircuitCode, "....................");
            gInvalidCode = 0;
            gWrongNumPlayers = 0;
            break;
        case 2:
            if (gYes || !gContinueEnabled) {
                NewCircuit(newCircuitType);
                ContinueCircuit();
                gCircuitScreenExit = 1;
            } else if (gConfirmNewCircuit) {
                gConfirmNewCircuit = 0;
                gYes = 0;
                buttonsJustInited = 1;
            } else {
                gYes = 0;
                gConfirmNewCircuit = 1;
            }
            break;
        }
    } else if (gActiveEdges & 4) {
        AudPlayEffect(1, 0, 1, 0);
        if (gConfirmNewCircuit) {
            gConfirmNewCircuit = 0;
            gYes = 0;
            buttonsJustInited = 1;
        } else
            gCircuitScreenExit = 2;
    } else if (gActiveEdges & 0x400) {
        if (!gConfirmNewCircuit) {
            PlayUDSound(gActiveEdges);
            do {
                gCircuitMenuSelect--;
                if (gCircuitMenuSelect < 0)
                    gCircuitMenuSelect = 3;
            } while (!menuItemValid(gCircuitMenuSelect));
        }
    } else if (gActiveEdges & 0x800) {
        if (!gConfirmNewCircuit) {
            PlayUDSound(gActiveEdges);
            do {
                gCircuitMenuSelect++;
                if (gCircuitMenuSelect > 3)
                    gCircuitMenuSelect = 0;
            } while (!menuItemValid(gCircuitMenuSelect));
        }
    } else if (gActiveEdges & 0x3000) {
        if (gConfirmNewCircuit) {
            PlayLRSound(gActiveEdges);
            gYes = !gYes;
        } else if (gCircuitMenuSelect == 2) {
            PlayLRSound(gActiveEdges);
            do {
                if (gActiveEdges & 0x1000)
                    newCircuitType--;
                else if (gActiveEdges & 0x2000)
                    newCircuitType++;
                if (newCircuitType < 0)
                    newCircuitType = 3;
                if (newCircuitType > 3)
                    newCircuitType = 0;
            } while (!IsCircuitUnlocked(newCircuitType));
        } else if (gCircuitMenuSelect == 1) {
            PlayLRSound(gActiveEdges);
            if (gActiveEdges & 0x1000)
                gReviewTrack--;
            else if (gActiveEdges & 0x2000)
                gReviewTrack++;
            if (gReviewTrack < 0)
                gReviewTrack = gCircuit->track;
            if (gReviewTrack > gCircuit->track)
                gReviewTrack = 0;
        }
    }
    for (i = 0; i < gNumPlayers; i++) {
    }
    if (gSoftReset) {
        uninitScreen();
        setGameState(4);
    } else if (gCircuitScreenExit) {
        uninitScreen();
        if (gCircuitScreenExit == 1) {
            switch (gCircuitMenuSelect) {
            case 0:
                setGameState(0x80);
                break;
            case 1:
                break;
            case 2:
                setGameState(0x80);
                break;
            }
        } else if (gCircuitScreenExit == 2)
            setGameState(0x40);
        gCircuitScreenExit = 0;
    }
    updateButtons();
}

/* 0x80068548, size 628 */
static void initScreen(void)
{
    int i;
    signed char place;
    unsigned short texIdx;
    for (i = 0; i < gNumPlayers; i++)
        SavePlayerRaceType(gPlayers[i].save, gRace);
    newCircuitType = 0;
    if (gCircuit->active && !gCircuit->track)
        gCircuit->active = 0;
    gReviewTrack = gCircuit->track;
    if (gCircuit->active) {
        ContinueCircuit();
        if (gCircuit->track >= gCircuitSize) {
            gShowTrophy = 1;
            place = gCircuitCars[0].totalPlace;
            if (place < 3) {
                trophyObjIdx = MBOX_NewObject(MBOX_FindObject_WORK(gTrophyNames[place], 0, gNumModels - 1, 1), &gTrophyMat, -1, 0);
                trophySheenObjIdx = MBOX_NewObject(MBOX_FindObject_WORK(gTrophySheenNames[place], 0, gNumModels - 1, 1), (tUVMatrix *)unitmatrix, trophyObjIdx, 0x2c6084);
                ZOID_SetTexture(trophySheenObjIdx, MBOX_FindTexture_WORK("SHEEN0", &texIdx, 0, gNumModels - 1, 1), -1);
                ZOID_SetColor(trophySheenObjIdx, &trophySheenColor);
                ZOID_SetEnvColor(trophySheenObjIdx, &trophySheenColor);
                MBOX_ShowObject(trophyObjIdx, 0, 15);
                CopyUV(unitmatrix, gTrophyMat.uvs);
                gTrophyMat.uvs[3][0] = 0.0f;
                gTrophyMat.uvs[3][1] = -13.0f;
                gTrophyMat.uvs[3][2] = 65.0f;
                ZeroCameras();
            }
        }
    }
    resetEnabled();
    gCircuitMenuSelect = 0;
    gEnterCircuitCode = 0;
    gConfirmNewCircuit = 0;
    gYes = 0;
    calcProgressSize();
    createBG();
    initButtons();
    pane = InitPane(-20, 0, -10, 10, 192, 0, 0, 0);
    blit = NewMultiBlit(0, 0, mblit, 1);
    inited = 1;
}

/* 0x800687bc, size 148 */
static void uninitScreen(void)
{
    uninitButtons();
    if (blit) {
        RemoveBlit(blit);
        blit = 0;
    }
    if (pane) {
        RemovePane(pane);
        pane = 0;
    }
    if (trophyObjIdx != -1) {
        ZOID_RemoveObject(trophyObjIdx);
        trophyObjIdx = -1;
    }
    if (trophySheenObjIdx != -1) {
        ZOID_RemoveObject(trophySheenObjIdx);
        trophySheenObjIdx = -1;
    }
    removeBG();
    inited = 0;
}

/* 0x80068850, size 3500 */
static void animButtonText(void)
{
    int i;
    int x;
    int y;
    unsigned char str1[64];
    unsigned char *str;
    signed short screenPos[2];

    HAL_SetTextDepth(0.0f);
    TB_SetJust(0, 1);
    ZOID_ProjectWindow(0, buttonInfo[6].mat.mat3.pos, &gCamera[0].mat, 0, screenPos);
    x = screenPos[0] / 2;
    y = screenPos[1] / 2.0f;
    HAL_SetFont(13);
    TB_SetTextColor(1);
    str = gText.single[42];
    TB_DrawText(TB_CenterText(str, x), y, str);
    if (gEnterCircuitCode) {
        static const unsigned char *const del = (const unsigned char *)"DEL";
        TB_SetJust(0, 3);
        HAL_SetFont(11);
        y = 60;
        x = 115;
        TB_SetTextColor(1);
        TB_DrawText(TB_CenterText(gText.single[44], 160), y, gText.single[44]);
        y += 16;
        if (gInvalidCode || gWrongNumPlayers) {
            TB_SetTextColor(5);
            TB_DrawText(TB_CenterText(gText.single[144], 160), y, gText.single[144]);
        }
        y += 16;
        TB_SetTextColor(12);
        str1[1] = 0;
        for (i = 0; i < 20; i++) {
            str1[0] = gCircuitCode[i];
            TB_DrawText(TB_CenterText(str1, x + 4), y, str1);
            if ((i + 1) % 10 == 0) {
                x = 115;
                y += 9;
            } else
                x += 9;
        }
        TB_SetTextColor(1);
        x = 94;
        y = 128;
        for (i = 0; i < 32; i++) {
            str1[0] = gPasswordAlphabet[i];
            TB_SetTextColor(gCodeSelect == i ? 22 : 1);
            TB_DrawText(TB_CenterText(str1, x + 5), y, str1);
            if ((i + 1) % 12 == 0) {
                x = 94;
                y += 11;
            } else
                x += 11;
        }
        TB_SetTextColor(gCodeSelect == -2 ? 22 : 1);
        TB_DrawText(TB_CenterText((void *)del, x + 17), y, (void *)del);
        if (pane)
            ChangePaneSize(pane, 80, 56, 240, 168);
    } else if (gConfirmNewCircuit) {
        TB_SetJust(1, 3);
        HAL_SetFont(11);
        TB_SetTextColor(1);
        TB_DrawTextScrollBox(160, 60, 140, 78, -1, 0, gText.single[104]);
        y = 60 + 5 * TB_GetFontLineHeight();
        TB_SetTextColor(gYes ? 22 : 1);
        TB_DrawText(145, y, gText.taba[gText.tab[25]]);
        TB_SetTextColor(gYes ? 1 : 22);
        TB_DrawText(175, y, gText.taba[gText.tab[25] + 1]);
        TB_SetJust(0, 3);
        if (pane)
            ChangePaneSize(pane, 60, 56, 260, 132);
    } else {
        HAL_SetFont(11);
        for (i = 0; i < 4; i++) {
            if (buttonInfo[i].angle > 0.698131701f && buttonInfo[i].angle < 2.443460953f)
                continue;
            if (!buttonInfo[i].alpha)
                continue;
            HAL_SetTextTranslucency(buttonInfo[i].alpha);
            ZOID_ProjectWindow(0, buttonInfo[i].mat.mat3.pos, &gCamera[0].mat, 0, screenPos);
            x = screenPos[0] / 2 - 35.0f * gButtonScale;
            y = screenPos[1] / 2.0f;
            if (buttonInfo[i].angle < 1.570796327f) {
                tRGBA c1, c2;
                c2 = flashColors[1];
                c1 = flashColors[0];
                TB_FlashTextColors(&c1, &c2);
                TB_SetTextColor(1);
            } else {
                tRGBA c1, c2;
                c2 = flashColors[1];
                c1 = flashColors[0];
                TB_FlashTextColors(&c1, &c2);
                TB_SetTextColor(22);
            }
            TB_DrawText(x, y, gText.taba[gText.tab[44] + i]);
            if (i == 1) {
                ZOID_ProjectWindow(0, buttonInfo[4].mat.mat3.pos, &gCamera[0].mat, 0, screenPos);
                x = screenPos[0] / 2 - 35.0f * gButtonScale;
                y = screenPos[1] / 2.0f;
                if (gReviewTrack == gCircuit->track)
                    str = gText.single[186];
                else {
                    TB_Sprintf(str1, "%s%d", gText.single[86], gReviewTrack + 1);
                    str = str1;
                }
                TB_DrawText(x, y, str);
            }
            if (i == 2) {
                ZOID_ProjectWindow(0, buttonInfo[5].mat.mat3.pos, &gCamera[0].mat, 0, screenPos);
                x = screenPos[0] / 2 - 35.0f * gButtonScale;
                y = screenPos[1] / 2.0f;
                TB_DrawText(x, y, gText.taba[gText.tab[45] + newCircuitType]);
            }
        }
        HAL_SetTextTranslucency(-1.0f);
        if (gShowTrophy) {
            signed short width;
            str = str1;
            TB_SetJust(0, 3);
            HAL_SetFont(11);
            TB_SetTextColor(1);
            TB_Sprintf(str, "%s %s", gText.taba[gText.tab[45] + gCircuit->type], gText.single[80]);
            width = HAL_GetTextWidth(str, -1) < 20 ? 20 : HAL_GetTextWidth(str, -1);
            TB_DrawText(TB_CenterText(str, 160), 156, str);
            y = 156 + TB_GetFontLineHeight();
            y += TB_GetFontLineHeight();
            getFinishPlaceStr(0, str);
            width = width > HAL_GetTextWidth(str, -1) ? width : HAL_GetTextWidth(str, -1);
            TB_DrawText(TB_CenterText(str, 160), y, str);
            y += TB_GetFontLineHeight();
            getPointsStr(0, str);
            width = width > HAL_GetTextWidth(str, -1) ? width : HAL_GetTextWidth(str, -1);
            TB_DrawText(TB_CenterText(str, 160), y, str);
            y += TB_GetFontLineHeight();
            ChangePaneSize(pane, 160 - width / 2 - 4, 152, 160 + width / 2 + 4, y + 4);
        } else if (gCircuitMenuSelect == 2) {
            int numCircuitTracks;
            signed short width;
            str = str1;
            TB_SetJust(0, 3);
            HAL_SetFont(11);
            TB_SetTextColor(1);
            TB_Sprintf(str, "%s %s", gText.taba[gText.tab[45] + newCircuitType], gText.single[42]);
            width = HAL_GetTextWidth(str, -1) < 20 ? 20 : HAL_GetTextWidth(str, -1);
            TB_DrawText(TB_CenterText(str, 160), 132, str);
            y = 132 + TB_GetFontLineHeight();
            numCircuitTracks = gNumCircuitTracks[newCircuitType];
            TB_Sprintf(str, "%d %s", numCircuitTracks * (newCircuitType == 3 ? 4 : 2), gText.single[75]);
            width = width > HAL_GetTextWidth(str, -1) ? width : HAL_GetTextWidth(str, -1);
            TB_DrawText(TB_CenterText(str, 160), y, str);
            y += TB_GetFontLineHeight();
            ChangePaneSize(pane, 160 - width / 2 - 4, 128, 160 + width / 2 + 4, y + 4);
        } else if (gStatsEnabled) {
            str = str1;
            TB_SetJust(0, 3);
            HAL_SetFont(11);
            TB_SetTextColor(1);
            if (gCircuitMenuSelect != 1) {
                signed short width;
                if (gContinueEnabled) {
                    TB_Sprintf(str, "%s %s", gText.taba[gText.tab[45] + gCircuit->type], gText.single[71]);
                    width = HAL_GetTextWidth(str, -1) < 20 ? 20 : HAL_GetTextWidth(str, -1);
                    TB_DrawText(TB_CenterText(str, 160), 132, str);
                    y = 132 + TB_GetFontLineHeight();
                    getFinishedStr(str);
                    width = width > HAL_GetTextWidth(str, -1) ? width : HAL_GetTextWidth(str, -1);
                    TB_DrawText(TB_CenterText(str, 160), y, str);
                    y += TB_GetFontLineHeight();
                    getToGoStr(str);
                    width = width > HAL_GetTextWidth(str, -1) ? width : HAL_GetTextWidth(str, -1);
                    TB_DrawText(TB_CenterText(str, 160), y, str);
                    y += TB_GetFontLineHeight();
                } else {
                    TB_Sprintf(str, "%s %s", gText.taba[gText.tab[45] + gCircuit->type], gText.single[80]);
                    width = HAL_GetTextWidth(str, -1) < 20 ? 20 : HAL_GetTextWidth(str, -1);
                    TB_DrawText(TB_CenterText(str, 160), 132, str);
                    y = 132 + TB_GetFontLineHeight();
                }
                y += TB_GetFontLineHeight();
                getPlaceStr(0, str);
                width = width > HAL_GetTextWidth(str, -1) ? width : HAL_GetTextWidth(str, -1);
                TB_DrawText(TB_CenterText(str, 160), y, str);
                y += TB_GetFontLineHeight();
                getPointsStr(0, str);
                width = width > HAL_GetTextWidth(str, -1) ? width : HAL_GetTextWidth(str, -1);
                TB_DrawText(TB_CenterText(str, 160), y, str);
                y += TB_GetFontLineHeight();
                ChangePaneSize(pane, 160 - width / 2 - 4, 128, 160 + width / 2 + 4, y + 4);
            } else {
                TB_SetJust(0, 3);
                drawReview();
                if (pane)
                    ChangePaneSize(pane, 12, 128, 308, 208);
            }
        } else
            ChangePaneSize(pane, -20, 102, -10, 122);
    }
    TB_SetJust(0, 3);
    HAL_SetTextDepth(-1.0f);
}

/* 0x800695fc, size 56 */
static int animText(void)
{
    if (buttonsInited == 1) {
        animButtonText();
        return 1;
    }
    return 1;
}

/* 0x80069634, size 532 */
static void drawReview(void)
{
    unsigned char str[64];
    unsigned char str1[32];
    int y;
    int i;
    int p;

    HAL_SetFont(10);
    TB_SetTextColor(1);
    y = HAL_GetFontAscent();
    for (i = 0; i < 6; i++) {
        if (gReviewTrack == gCircuit->track)
            p = gCircuitCars[i].totalPlace;
        else {
            p = gCircuitCars[i].places[gReviewTrack];
            if (p >= 6) {
                HAL_SetFont(10);
                TB_SetTextColor(1);
                TB_DrawText(TB_CenterText(gText.single[85], 160), 168, gText.single[85]);
                return;
            }
        }
        {
            unsigned int pos[2];
            /* Keep the wide intermediate for the original GC/2.6 register allocation. */
            pos[1] = (int)((long long)(p % 3) * 24);
            pos[0] = p < 3 ? 16 : 160;
            TB_SetTextColor(i < gNumPlayers ? 17 : 1);
            HAL_SetFont(5);
            TB_Sprintf(str, "%d", p + 1);
            TB_DrawText(TB_CenterText(str, pos[0] + 6), pos[1] + 133, str);
            HAL_SetFont(11);
            if (i < gNumPlayers)
                TB_Strcpy(str1, (*gPlayers[i].save)->name);
            else
                TB_Strcpy(str1, gDefaultNames[gCircuitCars[i].name]);
            TB_DrawEllipseText(pos[0] + 15, pos[1] + 134, 120, str1);
            TB_Sprintf(str, "%d ", gReviewTrack == gCircuit->track ? gCircuitCars[i].totalPoints : gCircuitCars[i].points[gReviewTrack]);
            TB_Strcat(str, gText.single[140]);
            TB_DrawText(pos[0] + 15, pos[1] + 143, str);
        }
    }
}

/* 0x80069848, size 152 */
static void controlShowTrophy(void)
{
    YawUV(0.017453292f * (-90.0f * gFrameSeconds), gTrophyMat.uvs);
    if (gActiveEdges & 7) {
        if (trophyObjIdx != -1) {
            ZOID_RemoveObject(trophyObjIdx);
            trophyObjIdx = -1;
        }
        if (trophySheenObjIdx != -1) {
            ZOID_RemoveObject(trophySheenObjIdx);
            trophySheenObjIdx = -1;
        }
        PlayExitSound(gActiveEdges);
        gShowTrophy = 0;
        buttonsJustInited = 1;
    }
}

/* 0x800698e0, size 768 */
static void controlCircuitCode(void)
{
    int result;
    if (gActiveAutoEdges & 0x400) {
        AudPlayEffect(8, 0, 1, 0);
        if (gCodeSelect == -2) gCodeSelect = 21;
        else if (gCodeSelect < 8) gCodeSelect += 24;
        else if (gCodeSelect < 12) gCodeSelect = -2;
        else gCodeSelect -= 12;
    } else if (gActiveAutoEdges & 0x800) {
        AudPlayEffect(2, 0, 1, 0);
        if (gCodeSelect == -2) gCodeSelect = 9;
        else if (gCodeSelect < 20) gCodeSelect += 12;
        else if (gCodeSelect < 24) gCodeSelect = -2;
        else gCodeSelect -= 24;
    } else if (gActiveAutoEdges & 0x1000) {
        AudPlayEffect(3, 0, 1, 0);
        if (gCodeSelect == -2) gCodeSelect = 31;
        else if (gCodeSelect == 24) gCodeSelect = -2;
        else if (gCodeSelect % 12 == 0) gCodeSelect += 11;
        else gCodeSelect--;
    } else if (gActiveAutoEdges & 0x2000) {
        AudPlayEffect(6, 0, 1, 0);
        if (gCodeSelect == -2) gCodeSelect = 24;
        else if (gCodeSelect == 31) gCodeSelect = -2;
        else if (gCodeSelect % 12 == 11) gCodeSelect -= 11;
        else gCodeSelect++;
    }
    if (gActiveEdges & 2) {
        AudPlayEffect(0, 0, 1, 0);
        if (gCodeSelect == -2) {
            if (gCodeLen > 0) {
                gCodeLen--;
                gCircuitCode[gCodeLen] = '.';
                gInvalidCode = 0;
                gWrongNumPlayers = 0;
            }
        } else if (gCodeLen < 20) {
            gCircuitCode[gCodeLen] = gPasswordAlphabet[gCodeSelect];
            gCodeLen++;
            if (gCodeLen == 20) {
                result = SetCircuitCode(gCircuitCode);
                if (result == 0) gInvalidCode = 1;
                else if (result == -1) gWrongNumPlayers = 1;
                else {
                    gEnterCircuitCode = 0;
                    resetEnabled();
                    buttonsJustInited = 1;
                    gReviewTrack = gCircuit->track;
                }
            }
        }
    } else if (gActiveEdges & 5) {
        PlayExitSound(gActiveEdges);
        gEnterCircuitCode = 0;
        buttonsJustInited = 1;
    }
}

/* 0x80069be0, size 204 */
static void resetEnabled(void)
{
    gContinueEnabled = gCircuit->track < gCircuitSize && gCircuit->active;
    gStatsEnabled = gCircuit->active;
    if (gCircuit->type >= 1) {
        gUnlockedCircuits[0][1] = 1;
        gUnlockedTracks[0][4] = 1;
        gForceUnlockedCircuits[1] = 1;
    }
    if (gCircuit->type >= 2) {
        gUnlockedCircuits[0][2] = 1;
        gUnlockedTracks[0][5] = 1;
        gForceUnlockedCircuits[2] = 1;
    }
    if (gCircuit->type == 3) {
        gUnlockedCircuits[0][3] = 1;
        gForceUnlockedCircuits[3] = 1;
    }
}

/* 0x80069cac, size 104 */
static int menuItemValid(int item)
{
    return !(item == 3 && (*gPlayers[gCircuitPlayer].save)->fileh)
        && !(item == 0 && !gContinueEnabled)
        && !(item == 1 && !gStatsEnabled);
}

/* 0x80069d14, size 168 */
static void getFinishedStr(unsigned char *str)
{
    unsigned char str1[32];
    TB_Strcpy(str, gText.single[78]);
    TB_Sprintf(str1, "%d", gCircuit->track);
    TB_Strcat(str, str1);
    TB_Strcat(str, gCircuit->track != 1 ? gText.single[75] : gText.single[74]);
    TB_Strcat(str, gText.single[79]);
}

/* 0x80069dbc, size 196 */
static void getToGoStr(unsigned char *str)
{
    unsigned char str1[32];
    TB_Strcpy(str, gText.single[76]);
    TB_Sprintf(str1, "%d", gCircuitSize - gCircuit->track);
    TB_Strcat(str, str1);
    TB_Strcat(str, gCircuitSize - gCircuit->track != 1 ? gText.single[75] : gText.single[74]);
    TB_Strcat(str, gText.single[77]);
}

/* 0x80069e80, size 64 */
static void getPlayerStr(int p, unsigned char *str)
{
    TB_Strcpy(str, (*gPlayers[p].save)->name);
}

/* 0x80069ec0, size 172 */
static void getPlaceStr(int p, unsigned char *str)
{
    TB_Strcpy(str, gText.single[81]);
    TB_Strcat(str, gText.taba[gText.tab[56] + gCircuitCars[p].totalPlace]);
    TB_Strcat(str, " ");
    TB_Strcat(str, gText.single[139]);
}

/* 0x80069f6c, size 148 */
static void getPointsStr(int p, unsigned char *str)
{
    unsigned char str1[32];
    TB_Strcpy(str, gText.single[84]);
    TB_Sprintf(str1, "%d ", gCircuitCars[p].totalPoints);
    TB_Strcat(str, str1);
    TB_Strcat(str, gText.single[140]);
}

/* 0x8006a000, size 172 */
static void getFinishPlaceStr(int p, unsigned char *str)
{
    TB_Strcpy(str, gText.single[82]);
    TB_Strcat(str, gText.taba[gText.tab[56] + gCircuitCars[p].totalPlace]);
    TB_Strcat(str, " ");
    TB_Strcat(str, gText.single[139]);
}

/* 0x8006a0ac, size 584 */
static void calcProgressSize(void)
{
    int i;
    int p;
    int width;
    int maxWidth;
    unsigned char str[64];

    gProgressWidth = 0;
    gProgressHeight = 0;
    if (gStatsEnabled) {
        HAL_SetFont(11);
        if (gContinueEnabled) {
            gProgressHeight += 35;
            width = HAL_GetTextWidth(gText.single[71], -1);
            gProgressWidth = gProgressWidth > width ? gProgressWidth : width;
            getFinishedStr(str);
            width = HAL_GetTextWidth(str, -1);
            gProgressWidth = gProgressWidth > width ? gProgressWidth : width;
            getToGoStr(str);
            width = HAL_GetTextWidth(str, -1);
            gProgressWidth = gProgressWidth > width ? gProgressWidth : width;
        } else {
            gProgressHeight += 15;
            width = HAL_GetTextWidth(gText.single[80], -1);
            gProgressWidth = gProgressWidth > width ? gProgressWidth : width;
        }
        gProgressHeight += 40 - (10 - HAL_GetFontAscent());
        p = gCircuitPlayer;
        maxWidth = 0;
        for (i = 0; i < 2; i++, p ^= 1) {
            getPlayerStr(p, str);
            width = HAL_GetTextWidth(str, -1);
            maxWidth = maxWidth > width ? maxWidth : width;
            getPlaceStr(p, str);
            width = HAL_GetTextWidth(str, -1);
            maxWidth = maxWidth > width ? maxWidth : width;
            getPointsStr(p, str);
            width = HAL_GetTextWidth(str, -1);
            maxWidth = maxWidth > width ? maxWidth : width;
            if (gNumPlayers == 1)
                break;
        }
        if (gNumPlayers > 1)
            gProgressWidth = maxWidth + 160;
        else
            gProgressWidth = gProgressWidth > maxWidth ? gProgressWidth : maxWidth;
    }
    gProgressWidth += 20;
    gProgressTop = gMenuBottom + (224 - gMenuBottom - gProgressHeight) / 2;
}

/* 0x8006a2f4, size 940 */
static void initButtons(void)
{
    tRGBA buttonColor = {255, 255, 255, 255};
    tTexture *tex;
    unsigned short texIdx;
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 12; buttonIdx++) {
        if (buttonInfo[buttonIdx].objIdx == -1) {
            unsigned int flags;
            memcpy(buttonInfo[buttonIdx].mat.uvs, unitmatrix, 36);
            flags = buttonInfo[buttonIdx].buttonType == 0 || buttonInfo[buttonIdx].buttonType == 1 || buttonInfo[buttonIdx].buttonType == 2 ? 0x42000 : 0;
            buttonInfo[buttonIdx].objIdx = MBOX_NewObject(MBOX_FindObject_WORK(buttonNames[buttonInfo[buttonIdx].buttonType], 0, gNumModels - 1, 1), &buttonInfo[buttonIdx].mat, -1, flags);
            ZOID_SetColor(buttonInfo[buttonIdx].objIdx, &buttonColor);
        }
    }
    tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[6].objIdx, tex, -1);
    memcpy(buttonInfo[6].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.570796327f, buttonInfo[6].mat.uvs);
    buttonInfo[6].mat.uvs[3][0] = 0.0f;
    buttonInfo[6].mat.uvs[3][1] = 115.0f;
    buttonInfo[6].mat.uvs[3][2] = 200.0f;
    tex = MBOX_FindTexture_WORK("CURSOR", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[7].objIdx, tex, -1);
    tex = MBOX_FindTexture_WORK("A1", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[8].objIdx, tex, -1);
    memcpy(buttonInfo[8].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.570796327f, buttonInfo[8].mat.uvs);
    buttonInfo[8].mat.uvs[3][0] = 75.1f;
    buttonInfo[8].mat.uvs[3][1] = 59.2f;
    buttonInfo[8].mat.uvs[3][2] = 100.0f;
    ScaleUV(buttonInfo[8].mat.uvs, buttonInfo[8].mat.uvs, 0.5f);
    memcpy(buttonInfo[9].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.570796327f, buttonInfo[9].mat.uvs);
    WRollUV(3.141592654f, buttonInfo[9].mat.uvs);
    buttonInfo[9].mat.uvs[3][0] = 87.1f;
    buttonInfo[9].mat.uvs[3][1] = 59.2f;
    buttonInfo[9].mat.uvs[3][2] = 100.0f;
    tex = MBOX_FindTexture_WORK("B1", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[10].objIdx, tex, -1);
    memcpy(buttonInfo[10].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.570796327f, buttonInfo[10].mat.uvs);
    buttonInfo[10].mat.uvs[3][0] = -75.1f;
    buttonInfo[10].mat.uvs[3][1] = 59.2f;
    buttonInfo[10].mat.uvs[3][2] = 100.0f;
    ScaleUV(buttonInfo[10].mat.uvs, buttonInfo[10].mat.uvs, 0.5f);
    memcpy(buttonInfo[11].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.570796327f, buttonInfo[11].mat.uvs);
    buttonInfo[11].mat.uvs[3][0] = -87.1f;
    buttonInfo[11].mat.uvs[3][1] = 59.2f;
    buttonInfo[11].mat.uvs[3][2] = 100.0f;
    buttonsInited = 1;
    buttonsJustInited = 1;
    ZeroCameras();
}

/* 0x8006a6a0, size 144 */
static void uninitButtons(void)
{
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 12; buttonIdx++) {
        if (buttonInfo[buttonIdx].objIdx != -1) {
            if (gGameState & 0x7c03fffe)
                ZOID_RemoveObject(buttonInfo[buttonIdx].objIdx);
            buttonInfo[buttonIdx].objIdx = -1;
        }
    }
    buttonsInited = 0;
}

/* 0x8006a730, size 2204 */
static void updateButtons(void)
{
    static float cursorSpeed;
    static float scrollSpeed;
    int buttonIdx;
    float prevPos[3];
    float buttonY;
    float selectedPos = 0.0f;
    signed char clearScrollSpeed = 1;
    tTexture *tex;
    unsigned short texIdx;
    tRGBA buttonColor = {255, 255, 255, 255};

    if (!buttonsInited)
        return;
    if (gShowTrophy == 1) {
        for (buttonIdx = 0; buttonIdx < 12; buttonIdx++) {
            if (buttonIdx == 6 || buttonIdx == 8 || buttonIdx == 10)
                MBOX_ShowObject(buttonInfo[buttonIdx].objIdx, 0, 15);
            else {
                buttonInfo[buttonIdx].alpha = 0;
                MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 0, 15);
            }
        }
        return;
    }
    for (buttonIdx = 0; buttonIdx < 12; buttonIdx++)
        MBOX_ShowObject(buttonInfo[buttonIdx].objIdx, 0, 15);
    buttonY = maxY;
    for (buttonIdx = 0; buttonIdx < 4; buttonIdx++) {
        if (gEnterCircuitCode || gConfirmNewCircuit) {
            MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 0, 15);
            if (buttonIdx == 1)
                MBOX_HideObject(buttonInfo[4].objIdx, 0, 15);
            else if (buttonIdx == 2)
                MBOX_HideObject(buttonInfo[5].objIdx, 0, 15);
        } else {
            if (buttonIdx == gCircuitMenuSelect) {
                selectedPos = buttonY;
                if (buttonInfo[buttonIdx].angle < 3.141592654f)
                    buttonInfo[buttonIdx].angle += 12.566370614f * gFrameSeconds;
                if (buttonInfo[buttonIdx].angle > 3.141592654f)
                    buttonInfo[buttonIdx].angle = 3.141592654f;
                if (buttonsJustInited)
                    buttonInfo[buttonIdx].angle = 3.141592654f;
            } else {
                if (buttonInfo[buttonIdx].angle > 0.087266460f)
                    buttonInfo[buttonIdx].angle -= 12.566370614f * gFrameSeconds;
                if (buttonInfo[buttonIdx].angle < 0.087266460f)
                    buttonInfo[buttonIdx].angle = 0.087266460f;
                if (buttonsJustInited)
                    buttonInfo[buttonIdx].angle = 0.087266460f;
            }
            memcpy(prevPos, buttonInfo[buttonIdx].mat.mat3.pos, 12);
            buttonInfo[buttonIdx].mat.mat3.pos[0] = buttonX + -100.0f * gButtonScale;
            buttonInfo[buttonIdx].mat.mat3.pos[1] = buttonY;
            buttonInfo[buttonIdx].mat.mat3.pos[2] = buttonZ;
            if (!buttonsJustInited) {
                if (60.0 * fabs(buttonInfo[buttonIdx].mat.mat3.pos[1] - prevPos[1]) / 10.0 > scrollSpeed) {
                    scrollSpeed = 60.0 * fabs(buttonInfo[buttonIdx].mat.mat3.pos[1] - prevPos[1]) / 10.0;
                    scrollSpeed = scrollSpeed < 540.0f * gButtonScale ? scrollSpeed : 540.0f * gButtonScale;
                    scrollSpeed = scrollSpeed > 180.0f * gButtonScale ? scrollSpeed : 180.0f * gButtonScale;
                }
                if (buttonInfo[buttonIdx].mat.mat3.pos[1] > prevPos[1] + scrollSpeed * gFrameSeconds) {
                    buttonInfo[buttonIdx].mat.mat3.pos[1] = prevPos[1] + scrollSpeed * gFrameSeconds;
                    clearScrollSpeed = 0;
                } else if (buttonInfo[buttonIdx].mat.mat3.pos[1] < prevPos[1] - scrollSpeed * gFrameSeconds) {
                    buttonInfo[buttonIdx].mat.mat3.pos[1] = prevPos[1] - scrollSpeed * gFrameSeconds;
                    clearScrollSpeed = 0;
                }
            }
            ScaleUV(unitmatrix, buttonInfo[buttonIdx].mat.uvs, gButtonScale);
            WPitchUV(buttonInfo[buttonIdx].angle - 1.570796327f, buttonInfo[buttonIdx].mat.uvs);
            if (buttonInfo[buttonIdx].angle > 1.570796327f)
                tex = MBOX_FindTexture_WORK("BUTTON_SELECT", &texIdx, 0, gNumModels - 1, 1);
            else
                tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels - 1, 1);
            ZOID_SetTexture(buttonInfo[buttonIdx].objIdx, tex, -1);
            if (buttonInfo[buttonIdx].mat.mat3.pos[1] > maxY + gButtonSpace * gButtonScale || buttonInfo[buttonIdx].mat.mat3.pos[1] < minY - gButtonSpace * gButtonScale)
                buttonInfo[buttonIdx].alpha = 0;
            else if (buttonInfo[buttonIdx].mat.mat3.pos[1] > maxY)
                buttonInfo[buttonIdx].alpha = 80;
            else if (buttonInfo[buttonIdx].mat.mat3.pos[1] < minY)
                buttonInfo[buttonIdx].alpha = 80;
            else
                buttonInfo[buttonIdx].alpha = 255;
            if (!menuItemValid(buttonIdx))
                buttonInfo[buttonIdx].alpha = 0;
            buttonColor.c[3] = buttonInfo[buttonIdx].alpha;
            ZOID_SetColor(buttonInfo[buttonIdx].objIdx, &buttonColor);
            if (buttonIdx == 1) {
                buttonInfo[4].angle = buttonInfo[buttonIdx].angle;
                buttonInfo[4].mat.mat3.pos[0] = buttonX + 100.0f * gButtonScale;
                buttonInfo[4].mat.mat3.pos[1] = buttonInfo[buttonIdx].mat.mat3.pos[1];
                buttonInfo[4].mat.mat3.pos[2] = buttonInfo[buttonIdx].mat.mat3.pos[2];
                ScaleUV(unitmatrix, buttonInfo[4].mat.uvs, gButtonScale);
                WPitchUV(buttonInfo[4].angle - 1.570796327f, buttonInfo[4].mat.uvs);
                ZOID_SetTexture(buttonInfo[4].objIdx, tex, -1);
                buttonInfo[4].alpha = buttonInfo[buttonIdx].alpha;
                ZOID_SetColor(buttonInfo[4].objIdx, &buttonColor);
                if (!menuItemValid(buttonIdx))
                    MBOX_HideObject(buttonInfo[4].objIdx, 0, 15);
                else if (!buttonInfo[buttonIdx].alpha)
                    MBOX_HideObject(buttonInfo[4].objIdx, 0, 15);
                else
                    MBOX_ShowObject(buttonInfo[4].objIdx, 0, 15);
            } else if (buttonIdx == 2) {
                buttonInfo[5].angle = buttonInfo[buttonIdx].angle;
                buttonInfo[5].mat.mat3.pos[0] = buttonX + 100.0f * gButtonScale;
                buttonInfo[5].mat.mat3.pos[1] = buttonInfo[buttonIdx].mat.mat3.pos[1];
                buttonInfo[5].mat.mat3.pos[2] = buttonInfo[buttonIdx].mat.mat3.pos[2];
                ScaleUV(unitmatrix, buttonInfo[5].mat.uvs, gButtonScale);
                WPitchUV(buttonInfo[5].angle - 1.570796327f, buttonInfo[5].mat.uvs);
                ZOID_SetTexture(buttonInfo[5].objIdx, tex, -1);
                buttonInfo[5].alpha = buttonInfo[buttonIdx].alpha;
                ZOID_SetColor(buttonInfo[5].objIdx, &buttonColor);
                if (!menuItemValid(buttonIdx))
                    MBOX_HideObject(buttonInfo[5].objIdx, 0, 15);
                else if (!buttonInfo[buttonIdx].alpha)
                    MBOX_HideObject(buttonInfo[5].objIdx, 0, 15);
                else
                    MBOX_ShowObject(buttonInfo[5].objIdx, 0, 15);
            }
            if (!menuItemValid(buttonIdx))
                MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 0, 15);
            else if (!buttonInfo[buttonIdx].alpha) {
                MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 0, 15);
                buttonY -= gButtonSpace * gButtonScale;
            } else {
                MBOX_ShowObject(buttonInfo[buttonIdx].objIdx, 0, 15);
                buttonY -= gButtonSpace * gButtonScale;
            }
        }
    }
    memcpy(prevPos, buttonInfo[7].mat.mat3.pos, 12);
    buttonInfo[7].angle += 6.283185307f * gFrameSeconds;
    if (buttonInfo[7].angle > 6.283185307f)
        buttonInfo[7].angle -= 6.283185307f;
    buttonInfo[7].mat.mat3.pos[0] = buttonX * cursorZ / buttonZ + -140.0f * gButtonScale;
    buttonInfo[7].mat.mat3.pos[1] = selectedPos * cursorZ / buttonZ;
    buttonInfo[7].mat.mat3.pos[2] = cursorZ;
    if (!buttonsJustInited) {
        if (60.0 * fabs(buttonInfo[7].mat.mat3.pos[1] - prevPos[1]) / 10.0 > cursorSpeed) {
            cursorSpeed = 60.0 * fabs(buttonInfo[7].mat.mat3.pos[1] - prevPos[1]) / 10.0;
            cursorSpeed = cursorSpeed < 540.0f * gButtonScale ? cursorSpeed : 540.0f * gButtonScale;
            cursorSpeed = cursorSpeed > 180.0f * gButtonScale ? cursorSpeed : 180.0f * gButtonScale;
        }
        if (buttonInfo[7].mat.mat3.pos[1] > prevPos[1] + cursorSpeed * gFrameSeconds)
            buttonInfo[7].mat.mat3.pos[1] = prevPos[1] + cursorSpeed * gFrameSeconds;
        else if (buttonInfo[7].mat.mat3.pos[1] < prevPos[1] - cursorSpeed * gFrameSeconds)
            buttonInfo[7].mat.mat3.pos[1] = prevPos[1] - cursorSpeed * gFrameSeconds;
        else
            cursorSpeed = 180.0f * gButtonScale;
    }
    ScaleUV(unitmatrix, buttonInfo[7].mat.uvs, 0.4f * gButtonScale);
    WRollUV(buttonInfo[7].angle, buttonInfo[7].mat.uvs);
    WPitchUV(1.570796327f, buttonInfo[7].mat.uvs);
    if (gEnterCircuitCode || gConfirmNewCircuit) {
        MBOX_HideObject(buttonInfo[7].objIdx, 0, 15);
        MBOX_HideObject(buttonInfo[9].objIdx, 0, 15);
    } else {
        MBOX_ShowObject(buttonInfo[7].objIdx, 0, 15);
        MBOX_ShowObject(buttonInfo[9].objIdx, 0, 15);
    }
    if (clearScrollSpeed == 1)
        scrollSpeed = 0.0f;
    buttonsJustInited = 0;
}

