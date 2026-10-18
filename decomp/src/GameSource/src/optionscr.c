/* optionscr.c - option menu, pause panes and animated buttons. Status: Matching.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\optionscr.c; assembly: build/2049LE/asm/GameSource/src/optionscr.s */
#include "rush/rush_protos.h"
#include "rush/libc.h"
#include "msl_math_inline.h"

/* inline add (no DWARF entry): its parameter temporaries keep the original grouping of the pane-height sum in the InitPane call */
static inline int addPane(int a, int b) { return a + b; }
static void initScreen(void);
static void _TitlePane(void);
static void uninitScreen(void);
static void printOption(int x, int y, int item, const unsigned char *name, const unsigned char *value);
static int animText(void);
static void animButtonText(void);
static void calcOptionMenuMaxSize(void);
static void initButtons(void);
static void uninitButtons(void);
static void updateButtons(void);

unsigned char *cStrLanguageNames[6] = {
    (unsigned char *)"ENGLISH", (unsigned char *)"ESPA\x84" "OL", (unsigned char *)"ITALIANO",
    (unsigned char *)"DEUTSCH", (unsigned char *)"NEDERLANDS", (unsigned char *)"FRAN\x82" "AIS"
};
static char *buttonNames[9] = {
    "BUTTON_S3G1", "BUTTON_S1G1", "BUTTON_S2G1", "CURSORG1", "AB_BUTTONG1", "ARROW_BGG1",
    "ARROW_FGG1", "BAR_BGG1", "BAR_FGG1"
};
static const anon_texname_dulx_24 frontEndBlit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animText, 0 };
static const anon_texname_dulx_24 pauseBlit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animText, 0 };
static anon_buttonType_objIdx_40 buttonInfo[32] = {
    { 0, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 0, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 0, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 0, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 0, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 0, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 0, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 0, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 0, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 0, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 0, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 0, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 0, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 0, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 3, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 4, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 6, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 },
    { 2, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0 }
};
static int validOption[14] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
static int buttonsInited = 0;
static int buttonsJustInited = 0;
static signed char inited = 0;
static _tBlit_2 *blit = 0;
static signed short gSelectedOption = 0;
static enum_kNumMenuDir_7 exitOptionScreen = 0;
static anon_used_perm_20 *gTitlePane = 0;
static signed char gControllerRemoved = 0;
static signed short gOptionMenuSize;
static signed short gMenuCount;
static signed short gFirstOption;

/* 0x80074a04, size 0x3e4 */
void updateOptionScreen(void)
{
    unsigned int edges;
    unsigned int autoEdges;
    if (gGameState & 0x7c03fffe) {
        edges = gActiveEdges; autoEdges = gActiveAutoEdges;
    } else {
        edges = gContEdges[playerInPause]; autoEdges = gAutoEdges[playerInPause];
    }
    if (!inited) initScreen();
    if (edges & 5) { PlayStartABSound(edges); exitOptionScreen = 2; }
    else if (autoEdges & 0x400) {
        PlayUDSound(autoEdges);
        do { if (--gSelectedOption < 0) gSelectedOption = 13; } while (!validOption[gSelectedOption]);
    } else if (autoEdges & 0x800) {
        PlayUDSound(autoEdges);
        do { if (++gSelectedOption >= 14) gSelectedOption = 0; } while (!validOption[gSelectedOption]);
    }
    if (gFirstOption < gSelectedOption - gOptionMenuSize / 2) gFirstOption = gSelectedOption - gOptionMenuSize / 2;
    if (gFirstOption > gSelectedOption - gOptionMenuSize / 2) gFirstOption = gSelectedOption - gOptionMenuSize / 2;
    if (gFirstOption > 14 - gOptionMenuSize) gFirstOption = 14 - gOptionMenuSize;
    if (gFirstOption < 0) gFirstOption = 0;
    if (edges & 0x3000) {
        int dir = 1;
        if (edges & 0x1000) dir = -1;
        PlayLRSound(edges);
        switch (gSelectedOption) {
        case 1: gOptions[10] = !gOptions[10]; break;
        case 2: gOptions[0] ^= 1; break;
        case 3: gOptions[1] ^= 1; break;
        case 4: gOptions[2] ^= 1; break;
        case 5: gOptions[3] ^= 1; break;
        case 6: gOptions[4] ^= 1; break;
        case 7: gOptions[5] ^= 1; break;
        case 8: gOptions[6] ^= 1; break;
        case 9: gOptions[7] ^= 1; break;
        case 10: gOptions[8] ^= 1; break;
        case 11: gOptions[9] ^= 1; break;
        case 12: gOptions[18] ^= 1; break;
        case 13: gOptions[11] ^= 1; break;
        case 0:
            gLang += dir;
            if (gLang < 0) gLang = 5;
            else if (gLang >= 6) gLang = 0;
            gOptions[16] = gLang;
            gOptions[17] = 1;
            LoadTextWave();
            break;
        }
        if (!(gGameState & 0x7c03fffe)) SetOptions();
    }
    updateButtons();
    if (gSoftReset) {
        uninitScreen();
        if (gGameState & 0x7c03fffe) setGameState(4); else setPauseState(1);
    } else if (exitOptionScreen) {
        uninitScreen();
        if (gGameState & 0x7c03fffe) setGameState(16); else setPauseState(1);
    }
}

/* 0x80074de8, size 0x160 */
static void initScreen(void)
{
    signed short y;
    SaveGlobals();
    calcOptionMenuMaxSize();
    gSelectedOption = 0;
    while (!validOption[gSelectedOption]) {
        if (++gSelectedOption >= 14) gSelectedOption = 0;
    }
    gFirstOption = gSelectedOption;
    exitOptionScreen = 0;
    if (gGameState & 0x7c03fffe) {
        createBG();
        initButtons();
        blit = (_tBlit_2 *)NewMultiBlit(0, 0, frontEndBlit, 1);
    } else {
        gControllerRemoved = !gControllerData[playerInPause].attached;
        HAL_SetFont(13);
        _TitlePane();
        y = TB_GetFontLineHeight() * 2 + 10;
        HAL_SetFont(11);
        InitPane(40, y - 4, 280, addPane(y, gOptionMenuSize * TB_GetFontLineHeight() + 4), 176, 0, 0, 0);
        blit = (_tBlit_2 *)NewMultiBlit(0, 0, pauseBlit, 1);
    }
    ClearRushControls();
    inited = 1;
}

/* 0x80074f48, size 0x10c */
static void _TitlePane(void)
{
    int width;
    unsigned char str1[64];
    HAL_SetFont(13);
    if (gControllerRemoved) {
        TB_Sprintf(str1, "CONTROLLER %d %s", playerInPause + 1, gText.single[242]);
        width = HAL_GetTextWidth(str1, -1);
    } else {
        width = HAL_GetTextWidth(gText.single[50], -1);
    }
    if (gTitlePane) {
        ChangePaneSize(gTitlePane, (160 - width / 2) - (unsigned int)8, 6, 160 + width / 2 + 8, TB_GetFontLineHeight() + 14);
    } else {
        gTitlePane = InitPane((160 - width / 2) - (unsigned int)8, 6, 160 + width / 2 + 8, TB_GetFontLineHeight() + 14, 176, 0, 0, 0);
    }
}

/* 0x80075054, size 0x68 */
static void uninitScreen(void)
{
    if (!(gGameState & 0x7c03fffe)) {
        RemovePanes(0);
        gTitlePane = 0;
    }
    if (blit) {
        RemoveBlit((_tBlit *)blit);
        blit = 0;
    }
    removeBG();
    uninitButtons();
    inited = 0;
}

/* 0x800750bc, size 0x110 */
static void printOption(int x, int y, int item, const unsigned char *name, const unsigned char *value)
{
    if (buttonsInited == 1) {
        if (buttonInfo[item].angle < 1.5707964f) {
            anon_c_rgba_4 c1, c2;
            c2 = flashColors[1]; c1 = flashColors[0];
            TB_FlashTextColors(&c1, &c2);
            TB_SetTextColor(1);
        } else {
            anon_c_rgba_4 c1, c2;
            c2 = flashColors[1]; c1 = flashColors[0];
            TB_FlashTextColors(&c1, &c2);
            TB_SetTextColor(22);
        }
    } else {
        TB_SetTextColor(item == gSelectedOption ? 22 : 1);
    }
    TB_DrawText(x - 105, y, (void *)name);
    TB_DrawText(x + 40, y, (void *)value);
}

/* 0x800751cc, size 0x4f0 */
static int animText(void)
{
    int i;
    int y;
    unsigned char *str;
    signed short itemsDrawn;
    char str1[40];
    if (buttonsInited == 1) { animButtonText(); return 1; }
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(13);
    TB_SetTextColor(1);
    if ((gGameState & 0x7c0000) && gControllerRemoved) {
        TB_Sprintf(str1, "CONTROLLER %d %s", playerInPause + 1, gText.single[242]);
        TB_DrawText(TB_CenterText(str1, 160), 10, str1);
    } else {
        str = gText.single[50];
        TB_DrawText(TB_CenterText(str, 160), 10, str);
    }
    y = 10 + TB_GetFontLineHeight() * 2;
    HAL_SetFont(11);
    itemsDrawn = 0;
    for (i = 0; i < 14; i++) {
        if (validOption[i] && i >= gFirstOption && itemsDrawn < gOptionMenuSize) {
            switch (i) {
        case 1: printOption(160, y, 1, gText.single[13], gText.taba[gText.tab[17] + gOptions[10]]); break;
        case 2: printOption(160, y, 2, gText.single[15], gText.taba[gText.tab[17] + gOptions[0]]); break;
        case 3: printOption(160, y, 3, gText.single[16], gText.taba[gText.tab[17] + gOptions[1]]); break;
        case 4: printOption(160, y, 4, gText.single[17], gText.taba[gText.tab[17] + gOptions[2]]); break;
        case 5: printOption(160, y, 5, gText.single[18], gText.taba[gText.tab[17] + gOptions[3]]); break;
        case 6: printOption(160, y, 6, gText.single[19], gText.taba[gText.tab[17] + gOptions[4]]); break;
        case 7: printOption(160, y, 7, gText.single[20], gText.taba[gText.tab[17] + gOptions[5]]); break;
        case 8: printOption(160, y, 8, gText.single[21], gText.taba[gText.tab[17] + gOptions[6]]); break;
        case 9: printOption(160, y, 9, gText.single[22], gText.taba[gText.tab[17] + gOptions[7]]); break;
        case 10: printOption(160, y, 10, gText.single[23], gText.taba[gText.tab[17] + gOptions[8]]); break;
        case 11: printOption(160, y, 11, gText.single[24], gText.taba[gText.tab[17] + gOptions[9]]); break;
        case 12: printOption(160, y, 12, gText.single[26], gText.taba[gText.tab[17] + gOptions[18]]); break;
        case 13: printOption(160, y, 13, gText.single[14], gText.taba[gText.tab[17] + gOptions[11]]); break;
        case 0: printOption(160, y, 0, gText.single[25], cStrLanguageNames[gLang]); break;
            }
            y += TB_GetFontLineHeight();
            itemsDrawn++;
        }
    }
    HAL_SetTextTranslucency(-1.0f);
    { anon_c_rgba_4 c1, c2; c2 = flashColors[1]; c1 = flashColors[0]; TB_FlashTextColors(&c1, &c2); }
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x800756bc, size 0x4f4 */
static void animButtonText(void)
{
    int i;
    int x;
    int y;
    unsigned char *str;
    signed short screenPos[2];
    HAL_SetTextDepth(0.0f);
    TB_SetJust(0, 1);
    ZOID_ProjectWindow(0, buttonInfo[31].mat.uvs[3], &gCamera[0].mat, 0, screenPos);
    x = screenPos[0] / 2;
    y = screenPos[1] / 2.0f;
    HAL_SetFont(13);
    TB_SetTextColor(1);
    str = gText.single[50];
    TB_DrawText(TB_CenterText(str, x), y, str);
    HAL_SetFont(11);
    for (i = 0; i < 14; i++) {
        if (buttonInfo[i].angle > 0.6981317f && buttonInfo[i].angle < 2.443461f) continue;
        if (!buttonInfo[i].alpha) continue;
        HAL_SetTextTranslucency(buttonInfo[i].alpha);
        ZOID_ProjectWindow(0, buttonInfo[i].mat.uvs[3], &gCamera[0].mat, 0, screenPos);
        x = screenPos[0] / 2 + 50;
        y = screenPos[1] / 2.0f;
        switch (i) {
        case 1: printOption(x, y, 1, gText.single[13], gText.taba[gText.tab[17] + gOptions[10]]); break;
        case 2: printOption(x, y, 2, gText.single[15], gText.taba[gText.tab[17] + gOptions[0]]); break;
        case 3: printOption(x, y, 3, gText.single[16], gText.taba[gText.tab[17] + gOptions[1]]); break;
        case 4: printOption(x, y, 4, gText.single[17], gText.taba[gText.tab[17] + gOptions[2]]); break;
        case 5: printOption(x, y, 5, gText.single[18], gText.taba[gText.tab[17] + gOptions[3]]); break;
        case 6: printOption(x, y, 6, gText.single[19], gText.taba[gText.tab[17] + gOptions[4]]); break;
        case 7: printOption(x, y, 7, gText.single[20], gText.taba[gText.tab[17] + gOptions[5]]); break;
        case 8: printOption(x, y, 8, gText.single[21], gText.taba[gText.tab[17] + gOptions[6]]); break;
        case 9: printOption(x, y, 9, gText.single[22], gText.taba[gText.tab[17] + gOptions[7]]); break;
        case 10: printOption(x, y, 10, gText.single[23], gText.taba[gText.tab[17] + gOptions[8]]); break;
        case 11: printOption(x, y, 11, gText.single[24], gText.taba[gText.tab[17] + gOptions[9]]); break;
        case 12: printOption(x, y, 12, gText.single[26], gText.taba[gText.tab[17] + gOptions[18]]); break;
        case 13: printOption(x, y, 13, gText.single[14], gText.taba[gText.tab[17] + gOptions[11]]); break;
        case 0: printOption(x, y, 0, gText.single[25], cStrLanguageNames[gLang]); break;
        }
    }
    ZeroCameras();
    TB_SetJust(0, 3);
    HAL_SetTextTranslucency(-1.0f);
    { anon_c_rgba_4 c1, c2; c2 = flashColors[1]; c1 = flashColors[0]; TB_FlashTextColors(&c1, &c2); }
    HAL_SetTextDepth(-1.0f);
}

/* 0x80075bb0, size 0x150 */
static void calcOptionMenuMaxSize(void)
{
    int height;
    int i;
    HAL_SetFont(13);
    height = 214 - TB_GetFontLineHeight() * 2;
    HAL_SetFont(11);
    height -= TB_GetFontLineHeight();
    gMenuCount = 0;
    for (i = 0; i < 14; i++) {
        if (validOption[i]) gMenuCount++;
    }
    gOptionMenuSize = gMenuCount < height / TB_GetFontLineHeight() ? gMenuCount : height / TB_GetFontLineHeight();
}

/* 0x80075d00, size 0x2bc */
static void initButtons(void)
{
    anon_c_rgba_4 buttonColor = { 255, 255, 255, 255 };
    anon_name_width_30 *tex;
    unsigned short texIdx;
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 32; buttonIdx++) {
        if (buttonInfo[buttonIdx].objIdx == -1) {
            memcpy(buttonInfo[buttonIdx].mat.uvs, unitmatrix, 36);
            buttonInfo[buttonIdx].objIdx = MBOX_NewObject(MBOX_FindObject_WORK(buttonNames[buttonInfo[buttonIdx].buttonType], 0, gNumModels - 1, 1), &buttonInfo[buttonIdx].mat, -1,
                (buttonInfo[buttonIdx].buttonType == 0 || buttonInfo[buttonIdx].buttonType == 1 || buttonInfo[buttonIdx].buttonType == 2) ? 0x42000 : 0);
            ZOID_SetColor(buttonInfo[buttonIdx].objIdx, &buttonColor);
        }
    }
    tex = MBOX_FindTexture_WORK("CURSOR", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[28].objIdx, tex, -1);
    tex = MBOX_FindTexture_WORK("B1", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[29].objIdx, tex, -1);
    memcpy(buttonInfo[29].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[29].mat.uvs);
    buttonInfo[29].mat.uvs[3][0] = -75.1f;
    buttonInfo[29].mat.uvs[3][1] = 59.2f;
    buttonInfo[29].mat.uvs[3][2] = 100.0f;
    ScaleUV(buttonInfo[29].mat.uvs, buttonInfo[29].mat.uvs, 0.5f);
    memcpy(buttonInfo[30].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[30].mat.uvs);
    buttonInfo[30].mat.uvs[3][0] = -87.1f;
    buttonInfo[30].mat.uvs[3][1] = 59.2f;
    buttonInfo[30].mat.uvs[3][2] = 100.0f;
    tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[31].objIdx, tex, -1);
    memcpy(buttonInfo[31].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[31].mat.uvs);
    buttonInfo[31].mat.uvs[3][0] = 0.0f;
    buttonInfo[31].mat.uvs[3][1] = 115.0f;
    buttonInfo[31].mat.uvs[3][2] = 200.0f;
    buttonsInited = 1;
    buttonsJustInited = 1;
    ZeroCameras();
}

/* 0x80075fbc, size 0x74 */
static void uninitButtons(void)
{
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 32; buttonIdx++) {
        if (buttonInfo[buttonIdx].objIdx != -1) {
            ZOID_RemoveObject(buttonInfo[buttonIdx].objIdx);
            buttonInfo[buttonIdx].objIdx = -1;
        }
    }
    buttonsInited = 0;
}

/* 0x80076030, size 0x704 */
static void updateButtons(void)
{
    int buttonIdx;
    int i;
    float prevPos[3];
    float selectedPos;
    signed char clearScrollSpeed = 1;
    int whichItem;
    int buttonY;
    anon_name_width_30 *tex;
    unsigned short texIdx;
    anon_c_rgba_4 buttonColor = {255, 255, 255, 255};
    static float cursorSpeed;
    static float scrollSpeed;
    if (buttonsInited) {
        whichItem = 0;
        for (i = 0; i < gSelectedOption; i++) if (validOption[i]) whichItem++;
        memcpy(prevPos, buttonInfo[28].mat.uvs[3], 12);
        buttonInfo[28].angle += 6.2831855f * gFrameSeconds;
        if (buttonInfo[28].angle > 6.2831855f) buttonInfo[28].angle -= 6.2831855f;
        memcpy(buttonInfo[28].mat.uvs, unitmatrix, 36);
        selectedPos = 75.0f - (165.0f * whichItem) / (gMenuCount - 1);
        buttonInfo[28].mat.uvs[3][0] = -160.0f;
        buttonInfo[28].mat.uvs[3][2] = 225.0f;
        buttonInfo[28].mat.uvs[3][1] = selectedPos;
        if (!buttonsJustInited) {
            if ((60.0 * fabs(selectedPos - prevPos[1])) / 10.0 > cursorSpeed) {
                cursorSpeed = (60.0 * fabs(selectedPos - prevPos[1])) / 10.0;
                cursorSpeed = cursorSpeed < 540.0f ? cursorSpeed : 540.0f;
                cursorSpeed = cursorSpeed > 180.0f ? cursorSpeed : 180.0f;
            }
            if (buttonInfo[28].mat.uvs[3][1] > prevPos[1] + cursorSpeed * gFrameSeconds)
                buttonInfo[28].mat.uvs[3][1] = prevPos[1] + cursorSpeed * gFrameSeconds;
            else if (buttonInfo[28].mat.uvs[3][1] < prevPos[1] - cursorSpeed * gFrameSeconds)
                buttonInfo[28].mat.uvs[3][1] = prevPos[1] - cursorSpeed * gFrameSeconds;
            else cursorSpeed = 180.0f;
        }
        WRollUV(buttonInfo[28].angle, buttonInfo[28].mat.uvs);
        WPitchUV(1.5707964f, buttonInfo[28].mat.uvs);
        ScaleUV(buttonInfo[28].mat.uvs, buttonInfo[28].mat.uvs, 0.4f);
        buttonY = 1.33f * selectedPos + whichItem * 40;
        for (buttonIdx = 0; buttonIdx < 14; buttonIdx++) {
            if (buttonIdx == gSelectedOption) {
                if (buttonInfo[buttonIdx].angle < 3.1415927f) buttonInfo[buttonIdx].angle += 12.566371f * gFrameSeconds;
                if (buttonInfo[buttonIdx].angle > 3.1415927f) buttonInfo[buttonIdx].angle = 3.1415927f;
                if (buttonsJustInited) buttonInfo[buttonIdx].angle = 3.1415927f;
            } else {
                if (buttonInfo[buttonIdx].angle > 0.08726646f) buttonInfo[buttonIdx].angle -= 12.566371f * gFrameSeconds;
                if (buttonInfo[buttonIdx].angle < 0.08726646f) buttonInfo[buttonIdx].angle = 0.08726646f;
                if (buttonsJustInited) buttonInfo[buttonIdx].angle = 0.08726646f;
            }
            buttonInfo[buttonIdx+14].angle = buttonInfo[buttonIdx].angle;
            memcpy(buttonInfo[buttonIdx].mat.uvs, unitmatrix, 36);
            memcpy(buttonInfo[buttonIdx+14].mat.uvs, unitmatrix, 36);
            memcpy(prevPos, buttonInfo[buttonIdx].mat.uvs[3], 12);
            buttonInfo[buttonIdx].mat.uvs[3][0] = -90.0f;
            buttonInfo[buttonIdx].mat.uvs[3][1] = buttonY;
            buttonInfo[buttonIdx].mat.uvs[3][2] = 300.0f;
            if (!buttonsJustInited) {
                if ((60.0 * fabs(buttonInfo[buttonIdx].mat.uvs[3][1] - prevPos[1])) / 10.0 > scrollSpeed) {
                    scrollSpeed = (60.0 * fabs(buttonInfo[buttonIdx].mat.uvs[3][1] - prevPos[1])) / 10.0;
                    scrollSpeed = scrollSpeed < 540.0f ? scrollSpeed : 540.0f;
                }
                if (buttonInfo[buttonIdx].mat.uvs[3][1] > prevPos[1] + scrollSpeed * gFrameSeconds) {
                    buttonInfo[buttonIdx].mat.uvs[3][1] = prevPos[1] + scrollSpeed * gFrameSeconds;
                    clearScrollSpeed = 0;
                } else if (buttonInfo[buttonIdx].mat.uvs[3][1] < prevPos[1] - scrollSpeed * gFrameSeconds) {
                    buttonInfo[buttonIdx].mat.uvs[3][1] = prevPos[1] - scrollSpeed * gFrameSeconds;
                    clearScrollSpeed = 0;
                }
            }
            buttonInfo[buttonIdx+14].mat.uvs[3][0] = 150.0f;
            buttonInfo[buttonIdx+14].mat.uvs[3][1] = 31.0f * buttonInfo[buttonIdx].mat.uvs[3][1] / 30.0f;
            buttonInfo[buttonIdx+14].mat.uvs[3][2] = 310.0f;
            WPitchUV(buttonInfo[buttonIdx].angle - 1.5707964f, buttonInfo[buttonIdx].mat.uvs);
            WPitchUV(buttonInfo[buttonIdx+14].angle - 1.5707964f, buttonInfo[buttonIdx+14].mat.uvs);
            if (buttonInfo[buttonIdx].angle > 1.5707964f) tex = MBOX_FindTexture_WORK("BUTTON_SELECT", &texIdx, 0, gNumModels-1, 1);
            else tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels-1, 1);
            ZOID_SetTexture(buttonInfo[buttonIdx].objIdx, tex, -1);
            ZOID_SetTexture(buttonInfo[buttonIdx+14].objIdx, tex, -1);
            if (buttonInfo[buttonIdx].mat.uvs[3][1] > 140.0f || buttonInfo[buttonIdx].mat.uvs[3][1] < -160.0f) buttonInfo[buttonIdx].alpha = 0;
            else if (buttonInfo[buttonIdx].mat.uvs[3][1] > 100.0f) buttonInfo[buttonIdx].alpha = 80;
            else if (buttonInfo[buttonIdx].mat.uvs[3][1] < -120.0f) buttonInfo[buttonIdx].alpha = 80;
            else buttonInfo[buttonIdx].alpha = 255;
            buttonInfo[buttonIdx+14].alpha = buttonInfo[buttonIdx].alpha;
            buttonColor.c[3] = buttonInfo[buttonIdx].alpha;
            ZOID_SetColor(buttonInfo[buttonIdx].objIdx, &buttonColor);
            ZOID_SetColor(buttonInfo[buttonIdx+14].objIdx, &buttonColor);
            if (!validOption[buttonIdx]) {
                MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 0, 15);
                MBOX_HideObject(buttonInfo[buttonIdx+14].objIdx, 0, 15);
                buttonInfo[buttonIdx].alpha = 0;
            } else {
                if (!buttonInfo[buttonIdx].alpha) {
                    MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 0, 15);
                    MBOX_HideObject(buttonInfo[buttonIdx+14].objIdx, 0, 15);
                } else {
                    MBOX_ShowObject(buttonInfo[buttonIdx].objIdx, 0, 15);
                    MBOX_ShowObject(buttonInfo[buttonIdx+14].objIdx, 0, 15);
                }
                buttonY -= 40.0f;
            }
        }
        if (clearScrollSpeed == 1) scrollSpeed = 0.0f;
        buttonsJustInited = 0;
    }
}

