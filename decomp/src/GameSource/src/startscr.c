/* startscr.c - TODO: describe. Status: in progress.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\startscr.c; assembly: build/2049LE/asm/GameSource/src/startscr.s */
#include "rush/rush_protos.h"
#include "rush/libc.h"
#include "msl_math_inline.h"

static void initScreen(void);
static void uninitScreen(void);
static int animText(void);
static void animButtonText(void);
static signed char optionValid(enum_kStartMenuCount opt);
static void initButtons(void);
static void uninitButtons(void);
static void updateButtons(void);

static const anon_texname_dulx_24 mblit[1] = {(void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0,
                                              (void *)&animText,  0};
static signed char validOption[10] = {1, 1, 1, 1, 1, 0, 1, 0, 0, 0};
static char *buttonNames[8] = {"BUTTON_S1G1", "BUTTON_S2G1", "CURSORG1", "AB_BUTTONG1",
                               "ARROW_BGG1",  "ARROW_FGG1",  "BAR_BGG1", "BAR_FGG1"};
static anon_buttonType_objIdx_3c buttonInfo[18] = {
    0,    -1,   1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0,    -1,
    1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0,    -1,   1.5f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0,    -1,   1.5f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0,    -1,   1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0,    -1,   1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0,    -1,   1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0,    -1,   1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0,    -1,   1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0,
    -1,   1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0,    -1,   1.5f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1,    -1,   1.5f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 3,    -1,   0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5,    -1,   0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 3,    -1,   0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 5,    -1,   0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 2,    -1,   0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    2,    -1,   0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
static float gButtonScale = 1.03f;
static float gButtonSpace = 40.0f;
static float maxY = 105.0f;
static float buttonZ = 300.0f;
static float cursorZ = 225.0f;
static float cursorSpeed;
static float playerCursorSpeed;
static enum_kNumMenuDir_9 exitStartScreen;
static int gMaxPlayers;
static enum_kStartMenuCount gStartMenuSelect;
static float buttonX;
static int buttonsJustInited;
static int buttonsInited;
static _tBlit_2 *blit;
static signed char inited;

/* 0x8008624c, size 0x3ac */
void updateStartScreen(void)
{
    int ctrlIdx;
    if (!gSoftReset)
        gGameStarted = 1;
    if (gSkipToGameState != 1) {
        if (gSkipToGameState != gGameState) {
            if (!(gSkipToGameState & 0x7c0000) && gOptions[14] <= 18) {
                AudSetMusicVol(gOptions[12] / 10.0f, 0.05f);
                AudPlaySong(19, 1);
            }
            setGameState(32);
            return;
        }
        gSkipToGameState = 1;
    }
    if (!inited)
        initScreen();
    while (!optionValid(gStartMenuSelect)) {
        if (++gStartMenuSelect >= 10)
            gStartMenuSelect = 0;
    }
    gMaxPlayers = 0;
    for (ctrlIdx = 0; ctrlIdx < 4; ctrlIdx++) {
        if (gControllerData[ctrlIdx].attached != 1)
            break;
        gMaxPlayers++;
    }
    if (gNumPlayers > gMaxPlayers)
        gNumPlayers = gMaxPlayers >= 1 ? gMaxPlayers : 1;
    if (gActiveEdges & 1) {
        PlayStartABSound(gActiveEdges);
        gStartMenuSelect = 0;
        exitStartScreen = 1;
    } else if (gActiveEdges & 2) {
        PlayStartABSound(gActiveEdges);
        exitStartScreen = 1;
    } else if (gActiveEdges & 4) {
    } else if (gActiveEdges & 0x1000) {
        if (gStartMenuSelect == 0) {
            PlayLRSound(gActiveEdges);
            if (--gNumPlayers < 1)
                gNumPlayers = gMaxPlayers;
        }
    } else if (gActiveEdges & 0x2000) {
        if (gStartMenuSelect == 0) {
            PlayLRSound(gActiveEdges);
            if (++gNumPlayers > gMaxPlayers)
                gNumPlayers = 1;
        }
    } else if (gActiveEdges & 0x400) {
        PlayUDSound(gActiveEdges);
        do {
            if (--gStartMenuSelect < 0)
                gStartMenuSelect = 9;
        } while (!optionValid(gStartMenuSelect));
    } else if (gActiveEdges & 0x800) {
        PlayUDSound(gActiveEdges);
        do {
            if (++gStartMenuSelect >= 10)
                gStartMenuSelect = 0;
        } while (!optionValid(gStartMenuSelect));
    }
    updateButtons();
    if (gSoftReset) {
        uninitScreen();
        setGameState(4);
    } else if (exitStartScreen) {
        uninitScreen();
        switch (exitStartScreen) {
            case 1:
                switch (gStartMenuSelect) {
                    case 0:
                        setGameState(32);
                        break;
                    case 1:
                        setGameState(0x2000);
                        break;
                    case 2:
                        setGameState(0x200);
                        break;
                    case 3:
                        setGameState(0x800);
                        break;
                    case 4:
                        setGameState(0x1000);
                        break;
                    case 6:
                        setGameState(0x8000000);
                        break;
                    case 9:
                        AudKillSong();
                        AudStopAllEffects();
                        ExitSystem();
                        break;
                }
                break;
            case 2:
                setGameState(4);
                break;
        }
        exitStartScreen = 0;
    }
    if (!gCheatActive[23])
        gCheatActive[23] = CheckForEgg();
}

/* 0x800865f8, size 0xc8 */
static void initScreen(void)
{
    if (gOptions[14] <= 18) {
        AudSetMusicVol(gOptions[12] / 10.0f, 0.05f);
        AudPlaySong(19, 1);
    }
    createBG();
    initButtons();
    blit = (_tBlit_2 *)NewMultiBlit(0, 0, mblit, 1);
    exitStartScreen = 0;
    ClearRushControls();
    if (!gCheatActive[23])
        InitEgg(23);
    inited = 1;
}

/* 0x800866c0, size 0x44 */
static void uninitScreen(void)
{
    if (blit) {
        RemoveBlit((_tBlit *)blit);
        blit = 0;
    }
    removeBG();
    uninitButtons();
    inited = 0;
}

/* 0x80086704, size 0x2a8 */
static int animText(void)
{
    int i;
    float y;
    unsigned char str[40];
    int numX, playerNumIdx;
    if (buttonsInited == 1) {
        animButtonText();
        return 1;
    }
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 3);
    HAL_SetFont(13);
    TB_SetTextColor(1);
    y = 10.0f;
    TB_DrawText(160, y, gText.single[39]);
    TB_GetFontLineHeight();
    HAL_SetFont(10);
    y = 40.0f;
    for (i = 0; i < 10; i++)
        if (optionValid(i)) {
            if (i == gStartMenuSelect)
                TB_SetTextColor(22);
            else
                HAL_SetTextColor(224, 224, 224, 255);
            TB_DrawText(160, y, gText.taba[gText.tab[21] + i]);
            if (i == 0) {
                numX = 230;
                for (playerNumIdx = 0; playerNumIdx < 4; playerNumIdx++) {
                    TB_Sprintf(str, "%d", playerNumIdx + 1);
                    if (gStartMenuSelect == 0) {
                        if (playerNumIdx == gNumPlayers - 1)
                            TB_SetTextColor(22);
                        else
                            HAL_SetTextColor(64, 64, 64, 255);
                    } else if (playerNumIdx == gNumPlayers - 1) {
                        HAL_SetTextColor(0, 0, 0, 255);
                        TB_DrawText(numX + 1, y + 1.0f, str);
                        HAL_SetTextColor(224, 224, 224, 255);
                    } else
                        HAL_SetTextColor(64, 64, 64, 255);
                    TB_DrawText(numX, y, str);
                    numX += 16;
                }
            }
            y += 22.5f;
        }
    HAL_SetTextTranslucency(-1.0f);
    TB_SetJust(0, 3);
    {
        anon_c_rgba_4 c1, c2;
        c2 = flashColors[1];
        c1 = flashColors[0];
        TB_FlashTextColors(&c1, &c2);
    }
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x800869ac, size 0x3b0 */
static void animButtonText(void)
{
    int i, x;
    float y;
    unsigned char str[40];
    signed short screenPos[2];
    int playerNumIdx;
    HAL_SetTextDepth(0.0f);
    TB_SetJust(0, 1);
    ZOID_ProjectWindow(0, buttonInfo[11].mat.uvs[3], &gCamera[0].mat, 0, screenPos);
    x = screenPos[0] / 2;
    y = screenPos[1] / 2.0f;
    HAL_SetFont(13);
    TB_SetTextColor(1);
    TB_DrawText(TB_CenterText(gText.single[39], x), y, gText.single[39]);
    HAL_SetFont(10);
    for (i = 0; i < 10; i++)
        if (optionValid(i)) {
            if (buttonInfo[i].angle > 0.6981317f && buttonInfo[i].angle < 2.443461f)
                continue;
            if (buttonInfo[i].angle > 0.34906584f && buttonInfo[i].angle < 2.7925267f)
                HAL_SetTextTranslucency(128.0f);
            else
                HAL_SetTextTranslucency(-1.0f);
            if (buttonInfo[i].angle < 1.5707964f) {
                anon_c_rgba_4 c1, c2;
                c2 = flashColors[1];
                c1 = flashColors[0];
                TB_FlashTextColors(&c1, &c2);
                HAL_SetTextColor(224, 224, 224, 255);
            } else {
                anon_c_rgba_4 c1, c2;
                c2 = flashColors[1];
                c1 = flashColors[0];
                TB_FlashTextColors(&c1, &c2);
                TB_SetTextColor(22);
            }
            ZOID_ProjectWindow(0, buttonInfo[i].mat.uvs[3], &gCamera[0].mat, 0, screenPos);
            TB_DrawText(screenPos[0] / 2 - 35, screenPos[1] / 2.0f, gText.taba[gText.tab[21] + i]);
            if (i == 0) {
                ZOID_ProjectWindow(0, buttonInfo[10].mat.uvs[3], &gCamera[0].mat, 0, screenPos);
                x = screenPos[0] / 2 - 18;
                y = screenPos[1] / 2.0f;
                for (playerNumIdx = 0; playerNumIdx < 4; playerNumIdx++) {
                    TB_Sprintf(str, "%d", playerNumIdx + 1);
                    if (gStartMenuSelect == 0) {
                        if (playerNumIdx > gMaxPlayers - 1)
                            TB_SetTextColor(3);
                        else if (playerNumIdx == gNumPlayers - 1)
                            TB_SetTextColor(22);
                        else
                            TB_SetTextColor(0);
                    } else if (playerNumIdx > gMaxPlayers - 1)
                        TB_SetTextColor(3);
                    else
                        TB_SetTextColor(0);
                    TB_DrawText(TB_CenterText(str, x), y, str);
                    x += 16;
                }
            }
        }
    HAL_SetTextTranslucency(-1.0f);
    TB_SetJust(0, 3);
    {
        anon_c_rgba_4 c1, c2;
        c2 = flashColors[1];
        c1 = flashColors[0];
        TB_FlashTextColors(&c1, &c2);
    }
    HAL_SetTextDepth(-1.0f);
}

/* 0x80086d5c, size 0x48 */
static signed char optionValid(enum_kStartMenuCount opt)
{
    int valid = validOption[opt];
    if (opt == 7 && !(gActiveLevels & 0x100))
        valid = 0;
    if (opt == 6)
        valid = gCheatActive[23];
    return valid;
}

/* 0x80086da4, size 0x3cc */
static void initButtons(void)
{
    anon_c_rgba_4 buttonColor = {255, 255, 255, 255};
    anon_name_width_30 *tex;
    unsigned short texIdx;
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 18; buttonIdx++)
        if (buttonInfo[buttonIdx].objIdx == -1) {
            memcpy(buttonInfo[buttonIdx].mat.uvs, unitmatrix, 48);
            buttonInfo[buttonIdx].objIdx = MBOX_NewObject(
                MBOX_FindObject_WORK(buttonNames[buttonInfo[buttonIdx].buttonType], 0, gNumModels - 1, 1),
                &buttonInfo[buttonIdx].mat, -1,
                (buttonInfo[buttonIdx].buttonType == 0 || buttonInfo[buttonIdx].buttonType == 1) ? 0x42000
                                                                                                 : 0);
            ZOID_SetColor(buttonInfo[buttonIdx].objIdx, &buttonColor);
        }
    tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[11].objIdx, tex, -1);
    memcpy(buttonInfo[11].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[11].mat.uvs);
    buttonInfo[11].mat.uvs[3][0] = 0.0f;
    buttonInfo[11].mat.uvs[3][1] = 115.0f;
    buttonInfo[11].mat.uvs[3][2] = 200.0f;
    tex = MBOX_FindTexture_WORK("A1", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[12].objIdx, tex, -1);
    memcpy(buttonInfo[12].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[12].mat.uvs);
    buttonInfo[12].mat.uvs[3][0] = 75.1f;
    buttonInfo[12].mat.uvs[3][1] = 59.2f;
    buttonInfo[12].mat.uvs[3][2] = 100.0f;
    ScaleUV(buttonInfo[12].mat.uvs, buttonInfo[12].mat.uvs, 0.5f);
    memcpy(buttonInfo[13].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[13].mat.uvs);
    WRollUV(3.1415927f, buttonInfo[13].mat.uvs);
    buttonInfo[13].mat.uvs[3][0] = 87.1f;
    buttonInfo[13].mat.uvs[3][1] = 59.2f;
    buttonInfo[13].mat.uvs[3][2] = 100.0f;
    tex = MBOX_FindTexture_WORK("B1", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[14].objIdx, tex, -1);
    memcpy(buttonInfo[14].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[14].mat.uvs);
    buttonInfo[14].mat.uvs[3][0] = -75.1f;
    buttonInfo[14].mat.uvs[3][1] = 59.2f;
    buttonInfo[14].mat.uvs[3][2] = 100.0f;
    ScaleUV(buttonInfo[14].mat.uvs, buttonInfo[14].mat.uvs, 0.5f);
    memcpy(buttonInfo[15].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[15].mat.uvs);
    buttonInfo[15].mat.uvs[3][0] = -87.1f;
    buttonInfo[15].mat.uvs[3][1] = 59.2f;
    buttonInfo[15].mat.uvs[3][2] = 100.0f;
    MBOX_HideObject(buttonInfo[14].objIdx, 0, 15);
    MBOX_HideObject(buttonInfo[15].objIdx, 0, 15);
    tex = MBOX_FindTexture_WORK("CURSOR", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[16].objIdx, tex, -1);
    buttonsInited = 1;
    buttonsJustInited = 1;
    ZeroCameras();
}

/* 0x80087170, size 0x74 */
static void uninitButtons(void)
{
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 18; buttonIdx++)
        if (buttonInfo[buttonIdx].objIdx != -1) {
            ZOID_RemoveObject(buttonInfo[buttonIdx].objIdx);
            buttonInfo[buttonIdx].objIdx = -1;
        }
    buttonsInited = 0;
}

/* 0x800871e4, size 0x6ac */
static void updateButtons(void)
{
    int buttonIdx;
    float prevPos[3];
    anon_name_width_30 *tex;
    unsigned short texIdx;
    float buttonY, selectedY;
    if (!buttonsInited)
        return;
    buttonY = maxY;
    selectedY = maxY;
    for (buttonIdx = 0; buttonIdx < 10; buttonIdx++) {
        if (!optionValid(buttonIdx)) {
            MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 0, 15);
            continue;
        }
        MBOX_ShowObject(buttonInfo[buttonIdx].objIdx, 0, 15);
        if (buttonIdx == gStartMenuSelect) {
            if (buttonInfo[buttonIdx].angle < 3.1415927f)
                buttonInfo[buttonIdx].angle += 12.566371f * gFrameSeconds;
            if (buttonInfo[buttonIdx].angle > 3.1415927f || buttonsJustInited == 1)
                buttonInfo[buttonIdx].angle = 3.1415927f;
            selectedY = buttonY;
        } else {
            if (buttonInfo[buttonIdx].angle > 0.08726646f)
                buttonInfo[buttonIdx].angle -= 12.566371f * gFrameSeconds;
            if (buttonInfo[buttonIdx].angle < 0.08726646f || buttonsJustInited == 1)
                buttonInfo[buttonIdx].angle = 0.08726646f;
        }
        buttonInfo[buttonIdx].mat.uvs[3][0] = buttonX;
        buttonInfo[buttonIdx].mat.uvs[3][1] = buttonY;
        buttonInfo[buttonIdx].mat.uvs[3][2] = buttonZ;
        ScaleUV(unitmatrix, buttonInfo[buttonIdx].mat.uvs, gButtonScale);
        WPitchUV(buttonInfo[buttonIdx].angle - 1.5707964f, buttonInfo[buttonIdx].mat.uvs);
        if (buttonInfo[buttonIdx].angle > 1.5707964f)
            tex = MBOX_FindTexture_WORK("BUTTON_SELECT", &texIdx, 0, gNumModels - 1, 1);
        else
            tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels - 1, 1);
        ZOID_SetTexture(buttonInfo[buttonIdx].objIdx, tex, -1);
        if (buttonIdx == 0) {
            buttonInfo[10].angle = buttonInfo[buttonIdx].angle;
            buttonInfo[10].mat.uvs[3][0] = buttonInfo[0].mat.uvs[3][0] + 160.0f * gButtonScale;
            buttonInfo[10].mat.uvs[3][1] = buttonInfo[0].mat.uvs[3][1];
            buttonInfo[10].mat.uvs[3][2] = buttonInfo[0].mat.uvs[3][2];
            buttonInfo[10].mat.uvs[3][0] = 1.02f * buttonInfo[10].mat.uvs[3][0];
            buttonInfo[10].mat.uvs[3][1] = 1.02f * buttonInfo[10].mat.uvs[3][1];
            buttonInfo[10].mat.uvs[3][2] = 1.02f * buttonInfo[10].mat.uvs[3][2];
            ScaleUV(unitmatrix, buttonInfo[10].mat.uvs, gButtonScale);
            WPitchUV(buttonInfo[10].angle - 1.5707964f, buttonInfo[10].mat.uvs);
            if (buttonInfo[10].angle > 1.5707964f)
                tex = MBOX_FindTexture_WORK("BUTTON_SELECT", &texIdx, 0, gNumModels - 1, 1);
            else
                tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels - 1, 1);
            ZOID_SetTexture(buttonInfo[10].objIdx, tex, -1);
        }
        buttonY -= gButtonSpace * gButtonScale;
    }
    memcpy(prevPos, buttonInfo[16].mat.uvs[3], 12);
    buttonInfo[16].angle += 6.2831855f * gFrameSeconds;
    if (buttonInfo[16].angle > 6.2831855f)
        buttonInfo[16].angle -= 6.2831855f;
    buttonInfo[16].mat.uvs[3][2] = cursorZ;
    buttonInfo[16].mat.uvs[3][0] = buttonX * cursorZ / buttonZ + (-60.0f) * gButtonScale;
    buttonInfo[16].mat.uvs[3][1] = selectedY * cursorZ / buttonZ;
    if (!buttonsJustInited) {
        if (60.0 * fabs(buttonInfo[16].mat.uvs[3][1] - prevPos[1]) / 10.0 > cursorSpeed) {
            cursorSpeed = 60.0 * fabs(buttonInfo[16].mat.uvs[3][1] - prevPos[1]) / 10.0;
            cursorSpeed = cursorSpeed < 540.0f * gButtonScale ? cursorSpeed : 540.0f * gButtonScale;
            cursorSpeed = cursorSpeed > 180.0f * gButtonScale ? cursorSpeed : 180.0f * gButtonScale;
        }
        if (buttonInfo[16].mat.uvs[3][1] > prevPos[1] + cursorSpeed * gFrameSeconds)
            buttonInfo[16].mat.uvs[3][1] = prevPos[1] + cursorSpeed * gFrameSeconds;
        else if (buttonInfo[16].mat.uvs[3][1] < prevPos[1] - cursorSpeed * gFrameSeconds)
            buttonInfo[16].mat.uvs[3][1] = prevPos[1] - cursorSpeed * gFrameSeconds;
        else
            cursorSpeed = 180.0f * gButtonScale;
    }
    ScaleUV(unitmatrix, buttonInfo[16].mat.uvs, 0.4f * gButtonScale);
    WRollUV(buttonInfo[16].angle, buttonInfo[16].mat.uvs);
    WPitchUV(1.5707964f, buttonInfo[16].mat.uvs);
    memcpy(prevPos, buttonInfo[17].mat.uvs[3], 12);
    buttonInfo[17].mat.uvs[3][0] = buttonX * cursorZ / buttonZ + (72.0f + 22.5f * gNumPlayers) * gButtonScale;
    buttonInfo[17].mat.uvs[3][2] = cursorZ;
    buttonInfo[17].mat.uvs[3][1] = maxY * cursorZ / buttonZ;
    if (!buttonsJustInited) {
        if (60.0 * fabs(buttonInfo[17].mat.uvs[3][0] - prevPos[0]) / 10.0 > playerCursorSpeed) {
            playerCursorSpeed = 60.0 * fabs(buttonInfo[17].mat.uvs[3][0] - prevPos[0]) / 10.0;
            playerCursorSpeed =
                playerCursorSpeed < 540.0f * gButtonScale ? playerCursorSpeed : 540.0f * gButtonScale;
            playerCursorSpeed =
                playerCursorSpeed > 180.0f * gButtonScale ? playerCursorSpeed : 180.0f * gButtonScale;
        }
        if (buttonInfo[17].mat.uvs[3][0] > prevPos[0] + playerCursorSpeed * gFrameSeconds)
            buttonInfo[17].mat.uvs[3][0] = prevPos[0] + playerCursorSpeed * gFrameSeconds;
        else if (buttonInfo[17].mat.uvs[3][0] < prevPos[0] - playerCursorSpeed * gFrameSeconds)
            buttonInfo[17].mat.uvs[3][0] = prevPos[0] - playerCursorSpeed * gFrameSeconds;
        else
            playerCursorSpeed = 180.0f * gButtonScale;
    }
    if (buttonInfo[0].angle > 1.5707964f)
        tex = MBOX_FindTexture_WORK("CURSOR", &texIdx, 0, gNumModels - 1, 1);
    else
        tex = MBOX_FindTexture_WORK("CURSOR_B", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[17].objIdx, tex, -1);
    ScaleUVXYZ(unitmatrix, buttonInfo[17].mat.uvs, 0.6f, 0.4f, 0.4f);
    ScaleUV(buttonInfo[17].mat.uvs, buttonInfo[17].mat.uvs, gButtonScale);
    WPitchUV(1.5707964f + buttonInfo[0].angle, buttonInfo[17].mat.uvs);
    buttonsJustInited = 0;
}
