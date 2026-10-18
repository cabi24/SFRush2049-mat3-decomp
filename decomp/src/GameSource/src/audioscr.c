/* audioscr.c - Audio options screen. Status: Matching.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\audioscr.c; assembly: build/2049LE/asm/GameSource/src/audioscr.s */
#include "rush/rush_protos.h"
#define MSL_NO_SQRTF
#include "msl_math_inline.h"

static void initScreen(void);
static void _TitlePane(void);
static void uninitScreen(void);
static int animPanes(void);
static int animText(void);
static void animButtonText(void);
static signed char validOption();
static void calcMenuRect(void);
static void initButtons(void);
static void uninitButtons(void);
static void updateButtons(void);

static char *buttonNames[9] = {
    "BUTTON_S3G1", "BUTTON_S1G1", "BUTTON_S2G1", "CURSORG1", "AB_BUTTONG1", "ARROW_BGG1",
    "ARROW_FGG1", "BAR_BGG1", "BAR_FGG1"
};
static const tBlitDef frontEndBlit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animText, 0 };
static const tBlitDef pauseBlit[2] = {
    0, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animPanes, 0, (void *)0xffffffff, 0, 0, 0, 0, -1,
    -1, -1, -1, 0, 0, (void *)&animText, 0
};
static tMenuButton buttonInfo[12] = {
    {0, -1}, {0, -1}, {0, -1}, {0, -1},
    {1, -1}, {1, -1}, {1, -1}, {1, -1},
    {3, -1}, {4, -1}, {6, -1}, {2, -1}
};
static anon_x_w_10 menuRect = { 0 }; /* explicit `= { 0 }`: the target keeps this object in initialised data (.data/.sdata), not .bss */
static float gButtonScale = 1.0f;
static int gSong = 18;
static int gLastSong = -1;
static float cursorSpeed;
static signed char gControllerRemoved;
static tBlitSet *gTitlePane;
static enum_kNumMenuDir_2 exitAudioScreen;
static int gAudioMenuSelect;
static signed char panesCreated;
static _tBlit *blit;
static signed char inited;
static int buttonsJustInited;
static int buttonsInited;

/* 0x80065b40, size 0x4cc */
void updateAudioScreen(void)
{
    unsigned int edges;
    if (gGameState & 0x7c03fffe) edges = gActiveEdges;
    else edges = gContEdges[playerInPause];
    if (!inited) initScreen();
    if (edges & 5) {
        PlayStartABSound(edges);
        exitAudioScreen = 2;
    } else if (edges & 2) {
        switch (gAudioMenuSelect) {
        case 0: case 1: break;
        case 2:
            AudKillSong();
        AudSetMusicVol(gOptions[12] / 10.0f, 0.05f);
        if (gSong < 18) AudPlaySong(gSong, 1);
        else if (gSong == 18) {
            if (gGameState & 0x7c03fffe) AudPlaySong(19, 1);
            else StartGameMusic();
        }
            break;
        }
    } else if (edges & 0xc00) {
        int dir = 1;
        if (edges & 0x400) dir = -1;
        PlayUDSound(edges);
        do {
            gAudioMenuSelect += dir;
            if (gAudioMenuSelect >= 4) gAudioMenuSelect = 0;
            else if (gAudioMenuSelect < 0) gAudioMenuSelect = 3;
        } while (!validOption(gAudioMenuSelect));
        if (gAudioMenuSelect == 1 || gAudioMenuSelect == 2) {
        AudSetMusicVol(gOptions[12] / 10.0f, 0.05f);
        if (gSong < 18) AudPlaySong(gSong, 1);
        else if (gSong == 18) {
            if (gGameState & 0x7c03fffe) AudPlaySong(19, 1);
            else StartGameMusic();
        }
        }
    } else if (edges & 0x3000) {
        int lrdir = (edges & 0x1000) ? -1 : 1;
        PlayLRSound(edges);
        switch (gAudioMenuSelect) {
        case 0:
            gOptions[13] += lrdir;
            if (gOptions[13] > 10) gOptions[13] = 0;
            else if (gOptions[13] < 0) gOptions[13] = 10;
            AudSetFxVol(gOptions[13]/10.0f, 0.05f);
            break;
        case 1:
            gOptions[12] += lrdir;
            if (gOptions[12] > 10) gOptions[12] = 0;
            else if (gOptions[12] < 0) gOptions[12] = 10;
            AudSetMusicVol(gOptions[12]/10.0f, 0.05f);
            break;
        case 2:
            AudKillSong();
            gSong += lrdir;
            if (gSong > 19) gSong = 0;
            else if (gSong < 0) gSong = 19;
            gLastSong = gSong;
            gOptions[14] = gSong;
        AudSetMusicVol(gOptions[12] / 10.0f, 0.05f);
        if (gSong < 18) AudPlaySong(gSong, 1);
        else if (gSong == 18) {
            if (gGameState & 0x7c03fffe) AudPlaySong(19, 1);
            else StartGameMusic();
        }
            break;
        case 3:
            gOptions[15] = !gOptions[15];
            AudMono(gOptions[15]);
            break;
        }
    }
    updateButtons();
    if (gSoftReset) {
        uninitScreen();
        if (gGameState & 0x7c03fffe) setGameState(4);
        else setPauseState(1);
    } else if (exitAudioScreen) {
        uninitScreen();
        if (gGameState & 0x7c03fffe) setGameState(16);
        else setPauseState(1);
    }
}

/* 0x8006600c, size 0x13c */
static void initScreen(void)
{
    SaveGlobals();
    calcMenuRect();
    if (gGameState & 0x7c03fffe) {
        createBG();
        initButtons();
        blit = NewMultiBlit(0, 0, frontEndBlit, 1);
    } else blit = NewMultiBlit(0, 0, pauseBlit, 2);
    ClearRushControls();
    exitAudioScreen = 0;
    gSong = gOptions[14];
    if (gAudioMenuSelect == 1 || gAudioMenuSelect == 2) {
        AudKillSong();
        AudSetMusicVol(gOptions[12] / 10.0f, 0.05f);
        if (gSong < 18) AudPlaySong(gSong, 1);
        else if (gSong == 18) {
            if (gGameState & 0x7c03fffe) AudPlaySong(19, 1);
            else StartGameMusic();
        }
    }
    inited = 1;
}

/* 0x80066148, size 0x10c */
static void _TitlePane(void)
{
    int width;
    unsigned char str1[64];
    HAL_SetFont(13);
    if (gControllerRemoved) {
        TB_Sprintf(str1, "CONTROLLER %d %s", playerInPause + 1, gText.single[242]);
        width = HAL_GetTextWidth(str1, -1);
    } else width = HAL_GetTextWidth(gText.single[52], -1);
    if (gTitlePane) {
        int left, right;
        width /= 2;
        left = 160-width;
        right = 160+width;
        ChangePaneSize(gTitlePane, left-8, 6, right+8, TB_GetFontLineHeight()+14);
    } else {
        int left, right;
        width /= 2;
        left = 160-width;
        right = 160+width;
        gTitlePane = InitPane(left-8, 6, right+8, TB_GetFontLineHeight()+14, 176, 0, 0, 0);
    }
}

/* 0x80066254, size 0x7c */
static void uninitScreen(void)
{
    AudKillSong();
    if (inited) {
        uninitButtons();
        if (!(gGameState & 0x7c03fffe)) {
            RemovePanes(0);
            gTitlePane = 0;
        }
        if (blit) {
            RemoveBlit(blit);
            blit = 0;
        }
        removeBG();
        panesCreated = 0;
        inited = 0;
    }
}

/* 0x800662d0, size 0xdc */
static int animPanes(void)
{
    signed short y;
    if (!panesCreated) {
        if (!(gGameState & 0x7c03fffe)) {
            HAL_SetFont(13);
            gControllerRemoved = !gControllerData[playerInPause].attached;
            _TitlePane();
            y = 10 + 2*TB_GetFontLineHeight();
            HAL_SetFont(10);
            InitPane(menuRect.x-8, y-4, menuRect.x+menuRect.w+8, y+menuRect.h+4, 176, 0, 0, 0);
        }
        panesCreated = 1;
    }
    return 1;
}

/* 0x800663ac, size 0x2fc */
static int animText(void)
{
    int i, x, y;
    unsigned char str[40];
    unsigned char *charPtr;
    if (buttonsInited == 1) {
        animButtonText();
        return 1;
    }
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(13);
    TB_SetTextColor(1);
    if ((gGameState & 0x7c0000) && gControllerRemoved) {
        TB_Sprintf(str, "CONTROLLER %d %s", playerInPause+1, gText.single[242]);
        TB_DrawText(TB_CenterText(str, 160), 10, str);
    } else {
        unsigned char *title = gText.single[52];
        TB_DrawText(TB_CenterText(title, 160), 10, title);
    }
    y = 10 + 2*TB_GetFontLineHeight();
    HAL_SetFont(10);
    x = menuRect.cx;
    for (i = 0; i < 4; i++) {
        if (validOption(i)) {
            unsigned char *label;
            TB_SetTextColor(i == gAudioMenuSelect ? 22 : 1);
            label = gText.taba[gText.tab[40]+i];
            TB_DrawText(TB_RightText(label, x-5), y, label);
            switch (i) {
            case 0: TB_Sprintf(str, "%d", gOptions[13]); break;
            case 1: TB_Sprintf(str, "%d", gOptions[12]); break;
            case 2:
                if (gSong == 18) TB_Strcpy(str, gText.single[34]);
                else if (gSong == 19) TB_Strcpy(str, gText.taba[gText.tab[17]]);
                else TB_Strcpy(str, gSongNames[gSong]);
                break;
            case 3: TB_Strcpy(str, gText.taba[gText.tab[41]+gOptions[15]]); break;
            }
            for (charPtr = str; *charPtr; charPtr++)
                *charPtr = (*charPtr >= 'a' && *charPtr <= 'z') ? *charPtr ^ 0x20 : *charPtr;

            TB_DrawText(x+5, y, str);
            y += TB_GetFontLineHeight();
        }
    }
    HAL_SetTextTranslucency(-1.0f);
    {
        tRGBA c1, c2;
        c2 = flashColors[1]; c1 = flashColors[0];
        TB_FlashTextColors(&c1, &c2);
    }
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x800666a8, size 0x44c */
static void animButtonText(void)
{
    int i, y;
    unsigned char str[40];
    signed short screenPos[2];
    unsigned char *charPtr;
    HAL_SetTextDepth(0.0f);
    TB_SetJust(0, 1);
    HAL_SetFont(13);
    TB_SetTextColor(1);
    ZOID_ProjectWindow(0, buttonInfo[11].mat.mat3.pos, &gCamera[0].mat, 0, screenPos);
    y = screenPos[1]/2.0f;
    TB_DrawText(TB_CenterText(gText.single[52],screenPos[0]/2), y, gText.single[52]);
    HAL_SetFont(11);
    for (i = 0; i < 4; i++) {
        if (!(buttonInfo[i].angle > 0.6981317f && buttonInfo[i].angle < 2.443461f) && validOption(i)) {
            if (buttonInfo[i].angle > 0.34906584f && buttonInfo[i].angle < 2.7925267f)
                HAL_SetTextTranslucency(128.0f);
            else HAL_SetTextTranslucency(-1.0f);
            if (buttonInfo[i].angle < 1.5707964f) {
                tRGBA c1, c2;
                c2 = flashColors[1];c1 = flashColors[0];
                TB_FlashTextColors(&c1, &c2);
                TB_SetTextColor(1);
            } else {
                tRGBA c1, c2;
                c2 = flashColors[1];c1 = flashColors[0];
                TB_FlashTextColors(&c1, &c2);
                TB_SetTextColor(22);
            }
            ZOID_ProjectWindow(0, buttonInfo[i].mat.mat3.pos, &gCamera[0].mat, 0, screenPos);
            TB_DrawText(screenPos[0]/2-55, (signed short)(int)(screenPos[1]/2.0f), gText.taba[gText.tab[40]+i]);
            switch (i) {
            case 0: TB_Sprintf(str, "%d", gOptions[13]); break;
            case 1: TB_Sprintf(str, "%d", gOptions[12]); break;
            case 2:
                if (gSong == 18) TB_Strcpy(str, gText.single[34]);
                else if (gSong == 19) TB_Strcpy(str, gText.taba[gText.tab[17]]);
                else TB_Strcpy(str, gSongNames[gSong]);
                break;
            case 3: TB_Strcpy(str, gText.taba[gText.tab[41]+gOptions[15]]); break;
            }
            for (charPtr = str; *charPtr; charPtr++)
                *charPtr = (*charPtr >= 'a' && *charPtr <= 'z') ? *charPtr ^ 0x20 : *charPtr;

            ZOID_ProjectWindow(0, buttonInfo[i+4].mat.mat3.pos, &gCamera[0].mat, 0, screenPos);
            TB_DrawText(screenPos[0]/2-35, (signed short)(int)(screenPos[1]/2.0f), str);
        }
    }
    TB_SetJust(0, 3);
    HAL_SetTextTranslucency(-1.0f);
    {
        tRGBA c1, c2;
        c2 = flashColors[1];c1 = flashColors[0];
        TB_FlashTextColors(&c1, &c2);
    }
    HAL_SetTextDepth(-1.0f);
}

/* 0x80066af4, size 0x8 */
static signed char validOption()
{
    return 1;
}

/* 0x80066afc, size 0x2dc */
static void calcMenuRect(void)
{
    int menuIdx, menuCount, optionWidth, valueWidth;
    unsigned char str[40];
    unsigned char *charPtr;
    int songIdx;
    HAL_SetFont(10);
    optionWidth = 0;
    valueWidth = 0;
    menuCount = 0;
    for (menuIdx = 0; menuIdx < 4; menuIdx++) {
        optionWidth = HAL_GetTextWidth(gText.taba[gText.tab[40]+menuIdx], -1) > optionWidth ?
            HAL_GetTextWidth(gText.taba[gText.tab[40]+menuIdx], -1) : optionWidth;
        if (validOption(menuIdx)) {
            switch (menuIdx) {
            case 0: case 1:
                valueWidth = HAL_GetTextWidth("10", -1) > valueWidth ? HAL_GetTextWidth("10", -1) : valueWidth;
                break;
            case 2:
                for (songIdx = 0; songIdx < 20; songIdx++) {
                    if (songIdx < 18) TB_Strcpy(str, gSongNames[songIdx]);
                    else if (songIdx == 18) TB_Strcpy(str, gText.single[34]);
                    else if (songIdx == 19) TB_Strcpy(str, gText.taba[gText.tab[17]]);
                    for (charPtr = str; *charPtr; charPtr++)
                        *charPtr = (*charPtr >= 'a' && *charPtr <= 'z') ? *charPtr ^ 0x20 : *charPtr;
                    valueWidth = HAL_GetTextWidth(str, -1) > valueWidth ? HAL_GetTextWidth(str, -1) : valueWidth;
                }
                break;
            case 3:
                valueWidth = HAL_GetTextWidth(gText.taba[gText.tab[41]], -1) > valueWidth ?
                    HAL_GetTextWidth(gText.taba[gText.tab[41]], -1) : valueWidth;
                valueWidth = HAL_GetTextWidth(gText.taba[gText.tab[41]+1], -1) > valueWidth ?
                    HAL_GetTextWidth(gText.taba[gText.tab[41]+1], -1) : valueWidth;
                break;
            }
            menuCount++;
        }
    }
    menuRect.w = optionWidth + valueWidth + 10;
    menuRect.h = menuCount * TB_GetFontLineHeight();
    menuRect.x = 160 - menuRect.w/2;
    menuRect.cx = menuRect.x + optionWidth + 5;
}

/* 0x80066dd8, size 0x2bc */
static void initButtons(void)
{
    tRGBA buttonColor = {{255,255,255,255}};
    tTexture *tex;
    unsigned short texIdx;
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 12; buttonIdx++) {
        if (buttonInfo[buttonIdx].objIdx == -1) {
            unsigned int flags;
            memcpy(buttonInfo[buttonIdx].mat.mat3.uvs, unitmatrix, 36);
            flags = (buttonInfo[buttonIdx].buttonType == 0 || buttonInfo[buttonIdx].buttonType == 1 ||
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

/* 0x80067094, size 0x74 */
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

/* 0x80067108, size 0x4ac */
static void updateButtons(void)
{
    int buttonIdx;
    float prevPos[3];
    float buttonY, selectY;
    tTexture *tex;
    unsigned short texIdx;
    if (!buttonsInited) return;
    buttonY = 100.0f;
    for (buttonIdx = 0; buttonIdx < 4; buttonIdx++) {
        if (buttonIdx == gAudioMenuSelect) {
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
            if (buttonInfo[buttonIdx].mat.mat3.pos[1] > prevPos[1] + 180.0f*gFrameSeconds)
                buttonInfo[buttonIdx].mat.mat3.pos[1] = prevPos[1] + 180.0f*gFrameSeconds;
            if (buttonInfo[buttonIdx].mat.mat3.pos[1] < prevPos[1] - 180.0f*gFrameSeconds)
                buttonInfo[buttonIdx].mat.mat3.pos[1] = prevPos[1] - 180.0f*gFrameSeconds;
        }
        buttonInfo[buttonIdx+4].mat.mat3.pos[0] = 150.0f;
        buttonInfo[buttonIdx+4].mat.mat3.pos[1] = buttonInfo[buttonIdx].mat.mat3.pos[1];
        buttonInfo[buttonIdx+4].mat.mat3.pos[2] = buttonInfo[buttonIdx].mat.mat3.pos[2];
        buttonInfo[buttonIdx+4].angle = buttonInfo[buttonIdx].angle;
        WPitchUV(buttonInfo[buttonIdx].angle - 1.5707964f, buttonInfo[buttonIdx].mat.mat3.uvs);
        memcpy(buttonInfo[buttonIdx+4].mat.mat3.uvs, buttonInfo[buttonIdx].mat.mat3.uvs, 36);
        if (buttonInfo[buttonIdx].angle > 1.5707964f)
            tex = MBOX_FindTexture_WORK("BUTTON_SELECT", &texIdx, 0, gNumModels-1, 1);
        else tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels-1, 1);
        ZOID_SetTexture(buttonInfo[buttonIdx].objIdx, tex, -1);
        ZOID_SetTexture(buttonInfo[buttonIdx+4].objIdx, tex, -1);
        if (validOption(buttonIdx) == 1) {
            MBOX_ShowObject(buttonInfo[buttonIdx].objIdx, 0, 15);
            MBOX_ShowObject(buttonInfo[buttonIdx+4].objIdx, 0, 15);
            buttonY -= 40.0f;
        } else {
            MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 0, 15);
            MBOX_HideObject(buttonInfo[buttonIdx+4].objIdx, 0, 15);
        }
    }
    memcpy(prevPos, buttonInfo[8].mat.mat3.pos, 12);
    buttonInfo[8].angle += 6.2831855f*gFrameSeconds;
    if (buttonInfo[8].angle > 6.2831855f) buttonInfo[8].angle -= 6.2831855f;
    memcpy(buttonInfo[8].mat.mat3.uvs, unitmatrix, 36);
    buttonInfo[8].mat.mat3.pos[0] = -160.0f;
    buttonInfo[8].mat.mat3.pos[1] = 225.0f*selectY/300.0f;
    buttonInfo[8].mat.mat3.pos[2] = 225.0f;
    if (!buttonsJustInited) {
        if (60.0 * fabs(buttonInfo[8].mat.mat3.pos[1] - prevPos[1]) / 10.0 > cursorSpeed) {
            cursorSpeed = 60.0 * fabs(buttonInfo[8].mat.mat3.pos[1] - prevPos[1]) / 10.0;
            cursorSpeed = cursorSpeed < 540.0f*gButtonScale ? cursorSpeed : 540.0f*gButtonScale;
            cursorSpeed = cursorSpeed > 180.0f*gButtonScale ? cursorSpeed : 180.0f*gButtonScale;
        }
        if (buttonInfo[8].mat.mat3.pos[1] > prevPos[1] + cursorSpeed*gFrameSeconds)
            buttonInfo[8].mat.mat3.pos[1] = prevPos[1] + cursorSpeed*gFrameSeconds;
        else if (buttonInfo[8].mat.mat3.pos[1] < prevPos[1] - cursorSpeed*gFrameSeconds)
            buttonInfo[8].mat.mat3.pos[1] = prevPos[1] - cursorSpeed*gFrameSeconds;
        else cursorSpeed = 180.0f*gButtonScale;
    }
    WRollUV(buttonInfo[8].angle, buttonInfo[8].mat.mat3.uvs);
    WPitchUV(1.5707964f, buttonInfo[8].mat.mat3.uvs);
    ScaleUV(buttonInfo[8].mat.mat3.uvs, buttonInfo[8].mat.mat3.uvs, 0.4f);
    buttonsJustInited = 0;
}

