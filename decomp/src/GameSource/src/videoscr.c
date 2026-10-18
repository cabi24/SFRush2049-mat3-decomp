/* videoscr.c - video menu and display offsets. Status: Matching.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\videoscr.c; assembly: build/2049LE/asm/GameSource/src/videoscr.s */
#include "rush/rush_protos.h"
#include "msl_math_inline.h"

/* Inline centering helper: `160 - w - 8` written directly folds to one subfic 152, while the original has subfic 160 then subi 8. */
static inline int centeredLeft(int halfWidth) { return 160 - halfWidth; }

static void initScreen(void);
static void _TitlePane(void);
static void uninitScreen(void);
static int animPanes(void);
static int animText(void);
static void animButtonText(void);
static void drawColorBars(void);
static signed char validOption(enum_kVideoMenuCount item);
static void calcMenuRect(void);
static void initButtons(void);
static void uninitButtons(void);
static void updateButtons(void);

static char *buttonNames[9] = {
    "BUTTON_S3G1", "BUTTON_S1G1", "BUTTON_S2G1", "CURSORG1", "AB_BUTTONG1", "ARROW_BGG1",
    "ARROW_FGG1", "BAR_BGG1", "BAR_FGG1"
};
static const anon_texname_dulx_24 frontEndBlit[2] = {
    0, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animPanes, 0, (void *)0xffffffff, 0, 0, 0, 0, -1,
    -1, -1, -1, 0, 0, (void *)&animText, 0
};
static const anon_texname_dulx_24 pauseBlit[2] = {
    0, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animPanes, 0, (void *)0xffffffff, 0, 0, 0, 0, -1,
    -1, -1, -1, 0, 0, (void *)&animText, 0
};
static anon_buttonType_objIdx_40 buttonInfo[12] = {
    { 0, -1, 0.0f, { 0 }, 0 },
    { 0, -1, 0.0f, { 0 }, 0 },
    { 0, -1, 0.0f, { 0 }, 0 },
    { 0, -1, 0.0f, { 0 }, 0 },
    { 1, -1, 0.0f, { 0 }, 0 },
    { 1, -1, 0.0f, { 0 }, 0 },
    { 1, -1, 0.0f, { 0 }, 0 },
    { 1, -1, 0.0f, { 0 }, 0 },
    { 3, -1, 0.0f, { 0 }, 0 },
    { 4, -1, 0.0f, { 0 }, 0 },
    { 6, -1, 0.0f, { 0 }, 0 },
    { 2, -1, 0.0f, { 0 }, 0 },
};
static anon_x_w_10 menuRect = { 0 }; /* explicit `= { 0 }`: the target keeps this object in initialised data (.data/.sdata), not .bss */
static float gButtonScale = 1.0f;
static signed char optionValid[4] = { 1, 1, 1, 1 };
static float cursorSpeed;
static signed char gControllerRemoved;
static anon_used_perm_20 *gTitlePane;
static anon_used_perm_20 *gConfirmPane;
static anon_used_perm_20 *gTestPane;
static signed char gYesFrequency;
static signed char gConfirmFrequency;
static signed char gTestFrequency;
static int gHorzCenter;
static int gVertCenter;
static int gVideoMenuSelect;
static enum_kNumMenuDir exitVideoScreen;
static signed char panesCreated;
static _tBlit_2 *blit;
static signed char inited;
static int buttonsJustInited;
static int buttonsInited;
int g_vy;
int g_vx;

/* 0x80063ea8, size 0x3b4 */
void updateVideoScreen(void)
{
    unsigned int edges;
    unsigned int autoEdges;
    unsigned int levels;
    if (gGameState & 0x7c03fffe) {
        edges = gActiveEdges;
        autoEdges = gActiveAutoEdges;
        levels = gActiveLevels;
    } else {
        edges = gContEdges[playerInPause];
        autoEdges = gAutoEdges[playerInPause];
        levels = gContLevels[playerInPause];
    }
    if (!inited) initScreen();
    if (gTestFrequency == 1) {
        if (CountdownTimeOut()) {
            gTestFrequency = 0;
            gConfirmFrequency = 1;
            gYesFrequency = 0;
            Pal60(0);
            VideoSetOffset(gHorzCenter, gVertCenter);
        }
    } else if (gConfirmFrequency == 1) {
        if (edges & 7) {
            PlayStartABSound(edges);
            if (edges & 4) gYesFrequency = 0;
            if (gYesFrequency == 1) {
                Pal60(1);
                VideoSetOffset(gHorzCenter, gVertCenter);
            }
            gConfirmFrequency = 0;
        } else if (edges & 0x3000) {
            PlayLRSound(edges);
            gYesFrequency = !gYesFrequency;
        }
    } else if (edges & 5) {
        PlayStartABSound(edges);
        exitVideoScreen = kMenuBackward;
    } else if (levels & 0x3000) {
        if (autoEdges & 0x3000) {
            autoEdges = autoEdges & 0x1000 ? -1 : 1;
            PlayLRSound(edges);
            switch (gVideoMenuSelect) {
            case 0:
                gVertCenter += autoEdges;
                if (gVertCenter < -10) gVertCenter = -10;
                else if (gVertCenter > 10) gVertCenter = 10;
                gOptions[20] = gVertCenter;
                VideoSetOffset(gHorzCenter, gVertCenter);
                break;
            case 1:
                gHorzCenter += autoEdges;
                if (gHorzCenter < -10) gHorzCenter = -10;
                else if (gHorzCenter > 10) gHorzCenter = 10;
                gOptions[19] = gHorzCenter;
                VideoSetOffset(gHorzCenter, gVertCenter);
                break;
            case 2:
                if (osTvType == 1) Pal60(0);
                else {
                    SetCountdownTimer(3.0f);
                    gTestFrequency = 1;
                    Pal60(1);
                }
                VideoSetOffset(gHorzCenter, gVertCenter);
                break;
            }
        }
    } else if (edges & 0xc00) {
        autoEdges = edges & 0x400 ? -1 : 1;
        PlayUDSound(edges);
        do {
            gVideoMenuSelect += autoEdges;
            if (gVideoMenuSelect >= 4) gVideoMenuSelect = 0;
            else if (gVideoMenuSelect < 0) gVideoMenuSelect = 3;
        } while (!validOption(gVideoMenuSelect));
    } else if ((edges & 2) && gVideoMenuSelect == 3) {
        gHorzCenter = 0;
        gVertCenter = 0;
        gOptions[19] = 0;
        gOptions[20] = 0;
        VideoSetOffset(0,0);
    }
    updateButtons();
    if (gSoftReset) {
        uninitScreen();
        if (gGameState & 0x7c03fffe) setGameState(4);
        else setPauseState(1);
    } else if (exitVideoScreen) {
        uninitScreen();
        if (gGameState & 0x7c03fffe) setGameState(16);
        else setPauseState(1);
    }
}

/* 0x8006425c, size 0xf0 */
static void initScreen(void)
{
    SaveGlobals();
    calcMenuRect();
    if (gGameState & 0x7c03fffe) {
        createBG();
        initButtons();
        blit = (_tBlit_2 *)NewMultiBlit(0, 0, frontEndBlit, 2);
    } else {
        blit = (_tBlit_2 *)NewMultiBlit(0, 0, pauseBlit, 2);
    }
    gVertCenter = gOptions[20];
    gHorzCenter = gOptions[19];
    ClearRushControls();
    while (!validOption(gVideoMenuSelect)) {
        if (++gVideoMenuSelect >= 4) {
            gVideoMenuSelect = 0;
            break;
        }
    }
    gTestFrequency = 0;
    gConfirmFrequency = 0;
    gYesFrequency = 0;
    exitVideoScreen = kMenuStay;
    inited = 1;
}

/* 0x8006434c, size 0x10c */
static void _TitlePane(void)
{
    int width;
    unsigned char str1[64];
    HAL_SetFont(13);
    if (gControllerRemoved) {
        TB_Sprintf(str1, "CONTROLLER %d %s", playerInPause + 1, gText.single[242]);
        width = HAL_GetTextWidth(str1, -1);
    } else {
        width = HAL_GetTextWidth(gText.single[53], -1);
    }
    if (gTitlePane) {
        width /= 2;
        ChangePaneSize(gTitlePane, centeredLeft(width) - 8, 6,
                       160 + width + 8, 14 + TB_GetFontLineHeight());
    } else {
        width /= 2;
        gTitlePane = InitPane(centeredLeft(width) - 8, 6, 160 + width + 8,
                              14 + TB_GetFontLineHeight(), 176, 0, 0, 0);
    }
}

/* 0x80064458, size 0x6c */
static void uninitScreen(void)
{
    if (inited) {
        SaveFlush(0);
        uninitButtons();
        RemovePanes(0);
        gTitlePane = 0;
        if (blit) {
            RemoveBlit((_tBlit *)blit);
            blit = 0;
        }
        removeBG();
        panesCreated = 0;
        inited = 0;
    }
}

/* 0x800644c4, size 0xf0 */
static int animPanes(void)
{
    signed short y;
    if (!panesCreated) {
        if (!(gGameState & 0x7c03fffe)) {
            HAL_SetFont(13);
            gControllerRemoved = !gControllerData[playerInPause].attached;
            _TitlePane();
            y = TB_GetFontLineHeight()*2 + 10;
            InitPane(menuRect.x - 8, y - 4, menuRect.x + menuRect.w + 8,
                     y + menuRect.h + 4, 176, 0, 0, 0);
        }
        InitPane(20, 136, 300, 204, 176, 0, 0, 0);
        panesCreated = 1;
    }
    return 1;
}

/* 0x800645b4, size 0x30c */
static int animText(void)
{
    int i;
    int j;
    int x;
    int y;
    unsigned char *str;
    unsigned char str1[40];
    int freqX;
    anon_c_rgba_4 color1;
    anon_c_rgba_4 color2;
    drawColorBars();
    if (buttonsInited == 1) {
        animButtonText();
        return 1;
    }
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(13);
    TB_SetTextColor(1);
    if ((gGameState & 0x7c0000) && gControllerRemoved) {
        TB_Sprintf(str1, "CONTROLLER %d %s", playerInPause + 1, gText.single[242]);
        TB_DrawText(TB_CenterText(str1, 160), 10, str1);
    } else {
        str = gText.single[53];
        TB_DrawText(TB_CenterText(str, 160), 10, str);
    }
    y = TB_GetFontLineHeight()*2 + 10;
    HAL_SetFont(10);
    x = menuRect.cx + 5;
    for (i = 0; i < 4; i++) {
        if (validOption(i)) {
            TB_SetTextColor(i == gVideoMenuSelect ? 22 : 1);
            str = gText.taba[gText.tab[42] + i];
            TB_DrawText(TB_RightText(str, menuRect.cx - 5), y, str);
            switch (i) {
            case 0:
                TB_Sprintf(str1, "%s%d", gVertCenter > 0 ? "+" : "", gVertCenter);
                TB_DrawText(x, y, str1);
                break;
            case 1:
                TB_Sprintf(str1, "%s%d", gHorzCenter > 0 ? "+" : "", gHorzCenter);
                TB_DrawText(x, y, str1);
                break;
            case 2:
                freqX = x;
                for (j=0; j<2; j++) {
                    str = gText.taba[gText.tab[43] + j];
                    if ((osTvType == 1 && j == 1) || (!osTvType && j == 0))
                        TB_SetTextColor(i == gVideoMenuSelect ? 22 : 4);
                    else
                        TB_SetTextColor(i == gVideoMenuSelect ? 1 : 4);
                    TB_DrawText(freqX, y, str);
                    freqX += HAL_GetTextWidth(str, -1) + HAL_GetTextWidth("  ", -1);
                }
                break;
            }
            y += TB_GetFontLineHeight();
        }
    }
    HAL_SetTextTranslucency(-1.0f);
    color2 = flashColors[1];
    color1 = flashColors[0];
    TB_FlashTextColors(&color1, &color2);
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* Pass the flash colours by value through an inline: the copies stay out of the DWARF locals, as in the original, and give its stack and register use. */
static inline void flashText(anon_c_rgba_4 color1, anon_c_rgba_4 color2)
{
    TB_FlashTextColors(&color1, &color2);
}

/* 0x800648c0, size 0x610 */
static void animButtonText(void)
{
    int i;
    int j;
    int x;
    int y;
    unsigned char *str;
    unsigned char str1[40];
    signed short screenPos[2];
    HAL_SetTextDepth(0.0f);
    TB_SetJust(0, 1);
    HAL_SetFont(13);
    TB_SetTextColor(1);
    ZOID_ProjectWindow(0, buttonInfo[11].mat.mat3.pos, &gCamera[0].mat, 0, screenPos);
    y = screenPos[1] / 2.0f;
    str = gText.single[53];
    TB_DrawText(TB_CenterText(str, screenPos[0] / 2), y, str);
    HAL_SetFont(11);
    for (i = 0; i < 4; i++) {
        if (validOption(i)) {
            if (buttonInfo[i].angle > 0.6981317f && buttonInfo[i].angle < 2.443461f) continue;
            if (buttonInfo[i].angle > 0.34906584f && buttonInfo[i].angle < 2.7925267f)
                HAL_SetTextTranslucency(128.0f);
            else
                HAL_SetTextTranslucency(-1.0f);
            if (buttonInfo[i].angle < 1.5707964f) {
                flashText(flashColors[0], flashColors[1]);
                TB_SetTextColor(1);
            } else {
                flashText(flashColors[0], flashColors[1]);
                TB_SetTextColor(22);
            }
            ZOID_ProjectWindow(0, buttonInfo[i].mat.mat3.pos, &gCamera[0].mat, 0, screenPos);
            TB_DrawText(screenPos[0] / 2 - 55, (int)(screenPos[1] / 2.0f), gText.taba[gText.tab[42] + i]);
            ZOID_ProjectWindow(0, buttonInfo[i+4].mat.mat3.pos, &gCamera[0].mat, 0, screenPos);
            x = screenPos[0] / 2 - 35;
            y = screenPos[1] / 2.0f;
            switch (i) {
            case 0:
                TB_Sprintf(str1, "%s%d", gVertCenter > 0 ? "+" : "", gVertCenter);
                TB_DrawText(x, y, str1);
                break;
            case 1:
                TB_Sprintf(str1, "%s%d", gHorzCenter > 0 ? "+" : "", gHorzCenter);
                TB_DrawText(x, y, str1);
                break;
            case 2:
                j = osTvType == 1;
                if (gConfirmFrequency == 1) j = 1;
                TB_DrawText(x, y, gText.taba[gText.tab[43] + j]);
                break;
            }
        }
    }
    TB_SetJust(1, 3);
    HAL_SetTextTranslucency(-1.0f);
    flashText(flashColors[0], flashColors[1]);
    HAL_SetTextDepth(-1.0f);
    if (gTestFrequency == 1) {
        static signed short width = 160;
        static signed short height = 1;
        signed short lines;
        HAL_SetFont(10);
        if (!gTestPane) {
            lines = TB_GetTextNumLines(width, gText.single[228]);
            height = lines * TB_GetFontLineHeight();
            gTestPane = InitPane(centeredLeft(width / 2) - 8, 52, 160 + width / 2 + 8, height + 60, 176, 0, 0, 0);
        }
        TB_DrawTextScrollBox(160, 56, width, height, -1, 0, gText.single[228]);
    } else if (gTestPane) {
        RemovePane(gTestPane);
        gTestPane = 0;
    }
    if (gConfirmFrequency == 1) {
        static signed short width = 160;
        static signed short height = 1;
        signed short lines;
        HAL_SetFont(10);
        if (!gConfirmPane) {
            lines = TB_GetTextNumLines(width, gText.single[229]) + 1;
            height = lines * TB_GetFontLineHeight();
            gConfirmPane = InitPane(centeredLeft(width / 2) - 8, 52, 160 + width / 2 + 8, height + 60, 176, 0, 0, 0);
        }
        TB_DrawTextScrollBox(160, 56, width, height, -1, 0, gText.single[229]);
        /* yes/no tail: the x value in x, the y value in the function's j and a block-local str give the original register assignment */
        {
            unsigned char *str;
            j = height - TB_GetFontLineHeight() + 56;
            x = 160 - width / 4;
            str = gText.taba[gText.tab[25]];
            TB_SetTextColor(gYesFrequency ? 22 : 1);
            TB_DrawText(x, j, str);
            x = 160 + width / 4;
            str = gText.taba[gText.tab[25] + 1];
            TB_SetTextColor(gYesFrequency ? 1 : 22);
            TB_DrawText(x, j, str);
        }
    } else if (gConfirmPane) {
        RemovePane(gConfirmPane);
        gConfirmPane = 0;
    }
    TB_SetJust(0, 3);
}

/* Scale a 320x240 menu coordinate to the 640x480 frame buffer; an inline with a named temporary, because a macro does not reproduce the original r30/r31 order. */
static inline int S2(int v)
{
    int r = v*2;
    return r;
}

/* 0x80064ed0, size 0x29c */
static void drawColorBars(void)
{
    int x;
    int i;
    /* a one-iteration loop: the original's induction seed is a slwi of a register, not a folded 64 */
    for (x = 24; x < 32; x += 8)
        DC_SendFillRect(S2(x), S2(140), S2(32), S2(160), 255, 255, 255, 255, 65500.0f);
    for (i = 0; i < 32; x += 8, i++)
        DC_SendFillRect(S2(x), S2(140), S2(x+8), S2(160), i*8, i*8, i*8, 255, 65500.0f);
    DC_SendFillRect(S2(x), S2(140), S2(x+8), S2(160), 0, 0, 0, 255, 65500.0f);
    DC_SendFillRect(S2(24), S2(160), S2(56), S2(180), 255, 255, 255, 255, 65500.0f);
    DC_SendFillRect(S2(24), S2(180), S2(56), S2(200), 0, 0, 0, 255, 65500.0f);
    DC_SendFillRect(S2(56), S2(160), S2(104), S2(200), 255, 0, 0, 255, 65500.0f);
    DC_SendFillRect(S2(104), S2(160), S2(136), S2(180), 255, 255, 255, 255, 65500.0f);
    DC_SendFillRect(S2(104), S2(180), S2(136), S2(200), 0, 0, 0, 255, 65500.0f);
    DC_SendFillRect(S2(136), S2(160), S2(184), S2(200), 0, 255, 0, 255, 65500.0f);
    DC_SendFillRect(S2(184), S2(160), S2(216), S2(180), 255, 255, 255, 255, 65500.0f);
    DC_SendFillRect(S2(184), S2(180), S2(216), S2(200), 0, 0, 0, 255, 65500.0f);
    DC_SendFillRect(S2(216), S2(160), S2(264), S2(200), 0, 0, 255, 255, 65500.0f);
    DC_SendFillRect(S2(264), S2(160), S2(296), S2(180), 255, 255, 255, 255, 65500.0f);
    DC_SendFillRect(S2(264), S2(180), S2(296), S2(200), 0, 0, 0, 255, 65500.0f);
}

/* 0x8006516c, size 0x30 */
static signed char validOption(enum_kVideoMenuCount item)
{
    signed char valid = optionValid[item];
    if (item == 2 && ((gGameState & 0x7c0000) || osCblType != 1))
        valid = 0;
    return valid;
}

/* 0x8006519c, size 0x19c */
static void calcMenuRect(void)
{
    int menuSize;
    int optionWidth;
    int valueWidth;
    int menuIdx;
    unsigned char str1[40];
    HAL_SetFont(10);
    menuSize = 0;
    optionWidth = 0;
    valueWidth = 0;
    for (menuIdx = 0; menuIdx < 4; menuIdx++) {
        if (validOption(menuIdx) == 1) {
            menuSize++;
            optionWidth = HAL_GetTextWidth(gText.taba[gText.tab[42] + menuIdx], -1) > optionWidth ?
                HAL_GetTextWidth(gText.taba[gText.tab[42] + menuIdx], -1) : optionWidth;
            if (menuIdx == 2) {
                TB_Sprintf(str1, "%s  %s", (gText.taba + gText.tab[43])[0], (gText.taba + gText.tab[43])[1]);
                valueWidth = HAL_GetTextWidth(str1, -1) > valueWidth ?
                    HAL_GetTextWidth(str1, -1) : valueWidth;
            } else {
                valueWidth = HAL_GetTextWidth("000", -1) > valueWidth ?
                    HAL_GetTextWidth("000", -1) : valueWidth;
            }
        }
    }
    menuRect.w = optionWidth + valueWidth + 10;
    menuRect.h = menuSize * TB_GetFontLineHeight();
    menuRect.x = 160 - menuRect.w / 2;
    menuRect.cx = menuRect.x + optionWidth + 5;
}

/* 0x80065338, size 0x2bc */
static void initButtons(void)
{
    anon_c_rgba_4 buttonColor = { { 255, 255, 255, 255 } };
    anon_name_width_30 *tex;
    unsigned short texIdx;
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 12; buttonIdx++) {
        if (buttonInfo[buttonIdx].objIdx == -1) {
            unsigned int flags;
            memcpy(buttonInfo[buttonIdx].mat.mat3.uvs, unitmatrix, 36);
            flags = (buttonInfo[buttonIdx].buttonType == 0 ||
                     buttonInfo[buttonIdx].buttonType == 1 ||
                     buttonInfo[buttonIdx].buttonType == 2) ? 0x42000 : 0;
            buttonInfo[buttonIdx].objIdx = MBOX_NewObject(
                MBOX_FindObject_WORK(buttonNames[buttonInfo[buttonIdx].buttonType], 0, gNumModels-1, 1),
                &buttonInfo[buttonIdx].mat, -1, flags);
            ZOID_SetColor(buttonInfo[buttonIdx].objIdx, &buttonColor);
        }
    }
    tex = MBOX_FindTexture_WORK("CURSOR", &texIdx, 0, gNumModels-1, 1);
    ZOID_SetTexture(buttonInfo[8].objIdx, tex, -1);
    tex = MBOX_FindTexture_WORK("B1", &texIdx, 0, gNumModels-1, 1);
    ZOID_SetTexture(buttonInfo[9].objIdx, tex, -1);
    memcpy(buttonInfo[9].mat.mat3.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[9].mat.mat3.uvs);
    buttonInfo[9].mat.mat3.pos[0] = -75.1f;
    buttonInfo[9].mat.mat3.pos[1] = 59.2f;
    buttonInfo[9].mat.mat3.pos[2] = 100.0f;
    ScaleUV(buttonInfo[9].mat.mat3.uvs, buttonInfo[9].mat.mat3.uvs, 0.5f);
    memcpy(buttonInfo[10].mat.mat3.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[10].mat.mat3.uvs);
    buttonInfo[10].mat.mat3.pos[0] = -87.1f;
    buttonInfo[10].mat.mat3.pos[1] = 59.2f;
    buttonInfo[10].mat.mat3.pos[2] = 100.0f;
    tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels-1, 1);
    ZOID_SetTexture(buttonInfo[11].objIdx, tex, -1);
    memcpy(buttonInfo[11].mat.mat3.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[11].mat.mat3.uvs);
    buttonInfo[11].mat.mat3.pos[0] = 0.0f;
    buttonInfo[11].mat.mat3.pos[1] = 115.0f;
    buttonInfo[11].mat.mat3.pos[2] = 200.0f;
    buttonsInited = 1;
    buttonsJustInited = 1;
    ZeroCameras();
}

/* 0x800655f4, size 0x74 */
static void uninitButtons(void)
{
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 12; buttonIdx++) {
        if (buttonInfo[buttonIdx].objIdx != -1) {
            ZOID_RemoveObject(buttonInfo[buttonIdx].objIdx);
            buttonInfo[buttonIdx].objIdx = -1;
        }
    }
    buttonsInited = 0;
}

/* 0x80065668, size 0x4c4 */
static void updateButtons(void)
{
    int buttonIdx;
    float prevPos[3];
    float buttonY;
    float selectY;
    anon_name_width_30 *tex;
    unsigned short texIdx;
    if (buttonsInited) {
        buttonY = 100.0f;
        for (buttonIdx = 0; buttonIdx < 4; buttonIdx++) {
            if (buttonIdx == gVideoMenuSelect) {
                selectY = buttonY;
                if (buttonInfo[buttonIdx].angle < 3.1415927f)
                    buttonInfo[buttonIdx].angle += 12.566371f * gFrameSeconds;
                if (buttonInfo[buttonIdx].angle > 3.1415927f || buttonsJustInited == 1)
                    buttonInfo[buttonIdx].angle = 3.1415927f;
            } else {
                if (buttonInfo[buttonIdx].angle > 0.0f)
                    buttonInfo[buttonIdx].angle -= 12.566371f * gFrameSeconds;
                if (buttonInfo[buttonIdx].angle < 0.0f || buttonsJustInited == 1)
                    buttonInfo[buttonIdx].angle = 0.0f;
            }
            memcpy(buttonInfo[buttonIdx].mat.mat3.uvs, unitmatrix, 36);
            memcpy(prevPos, buttonInfo[buttonIdx].mat.mat3.pos, 12);
            buttonInfo[buttonIdx].mat.mat3.pos[0] = -90.0f;
            buttonInfo[buttonIdx].mat.mat3.pos[1] = buttonY;
            buttonInfo[buttonIdx].mat.mat3.pos[2] = 300.0f;
            if (!buttonsJustInited) {
                if (buttonInfo[buttonIdx].mat.mat3.pos[1] > prevPos[1] + 180.0f * gFrameSeconds)
                    buttonInfo[buttonIdx].mat.mat3.pos[1] = prevPos[1] + 180.0f * gFrameSeconds;
                if (buttonInfo[buttonIdx].mat.mat3.pos[1] < prevPos[1] - 180.0f * gFrameSeconds)
                    buttonInfo[buttonIdx].mat.mat3.pos[1] = prevPos[1] - 180.0f * gFrameSeconds;
            }
            buttonInfo[buttonIdx+4].mat.mat3.pos[0] = 150.0f;
            buttonInfo[buttonIdx+4].mat.mat3.pos[1] = buttonInfo[buttonIdx].mat.mat3.pos[1];
            buttonInfo[buttonIdx+4].mat.mat3.pos[2] = buttonInfo[buttonIdx].mat.mat3.pos[2];
            buttonInfo[buttonIdx+4].angle = buttonInfo[buttonIdx].angle;
            WPitchUV(buttonInfo[buttonIdx].angle - 1.5707964f, buttonInfo[buttonIdx].mat.mat3.uvs);
            memcpy(buttonInfo[buttonIdx+4].mat.mat3.uvs, buttonInfo[buttonIdx].mat.mat3.uvs, 36);
            if (buttonInfo[buttonIdx].angle > 1.5707964f)
                tex = MBOX_FindTexture_WORK("BUTTON_SELECT", &texIdx, 0, gNumModels-1, 1);
            else
                tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels-1, 1);
            ZOID_SetTexture(buttonInfo[buttonIdx].objIdx, tex, -1);
            ZOID_SetTexture(buttonInfo[buttonIdx+4].objIdx, tex, -1);
            if (!validOption(buttonIdx)) {
                MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 1, 15);
                MBOX_HideObject(buttonInfo[buttonIdx+4].objIdx, 1, 15);
            } else {
                MBOX_ShowObject(buttonInfo[buttonIdx].objIdx, 0, 15);
                if (buttonIdx < 3)
                    MBOX_ShowObject(buttonInfo[buttonIdx+4].objIdx, 0, 15);
                else
                    MBOX_HideObject(buttonInfo[buttonIdx+4].objIdx, 1, 15);
                buttonY -= 40.0f;
            }
        }
        memcpy(prevPos, buttonInfo[8].mat.mat3.pos, 12);
        buttonInfo[8].angle += 6.2831855f * gFrameSeconds;
        if (buttonInfo[8].angle > 6.2831855f) buttonInfo[8].angle -= 6.2831855f;
        memcpy(buttonInfo[8].mat.mat3.uvs, unitmatrix, 36);
        buttonInfo[8].mat.mat3.pos[0] = -160.0f;
        buttonInfo[8].mat.mat3.pos[1] = 225.0f * selectY / 300.0f;
        buttonInfo[8].mat.mat3.pos[2] = 225.0f;
        if (!buttonsJustInited) {
            if (60.0 * fabs(buttonInfo[8].mat.mat3.pos[1] - prevPos[1]) / 10.0 > cursorSpeed) {
                cursorSpeed = 60.0 * fabs(buttonInfo[8].mat.mat3.pos[1] - prevPos[1]) / 10.0;
                cursorSpeed = cursorSpeed < 540.0f * gButtonScale ? cursorSpeed : 540.0f * gButtonScale;
                cursorSpeed = cursorSpeed > 180.0f * gButtonScale ? cursorSpeed : 180.0f * gButtonScale;
            }
            if (buttonInfo[8].mat.mat3.pos[1] > prevPos[1] + cursorSpeed * gFrameSeconds)
                buttonInfo[8].mat.mat3.pos[1] = prevPos[1] + cursorSpeed * gFrameSeconds;
            else if (buttonInfo[8].mat.mat3.pos[1] < prevPos[1] - cursorSpeed * gFrameSeconds)
                buttonInfo[8].mat.mat3.pos[1] = prevPos[1] - cursorSpeed * gFrameSeconds;
            else
                cursorSpeed = 180.0f * gButtonScale;
        }
        WRollUV(buttonInfo[8].angle, buttonInfo[8].mat.mat3.uvs);
        WPitchUV(1.5707964f, buttonInfo[8].mat.mat3.uvs);
        ScaleUV(buttonInfo[8].mat.mat3.uvs, buttonInfo[8].mat.mat3.uvs, 0.4f);
        buttonsJustInited = 0;
    }
}


/* 0x80065b2c, size 0x14 */
void VideoSetOffset(int xo, int yo)
{
    g_vx = xo * 2;
    g_vy = yo * 2;
}

