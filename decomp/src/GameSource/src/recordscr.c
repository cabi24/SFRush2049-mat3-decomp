/* recordscr.c - Record screen. Status: Matching.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\recordscr.c; assembly: build/2049LE/asm/GameSource/src/recordscr.s */
#include "rush/rush_protos.h"
inline double fabs(double x) { return __fabs(x); }

static void initRecordScreen(void);
static void uninitRecordScreen(void);
static void controlSelect(void);
static void controlView(void);
static void drawSelect(void);
static void drawView(void);
static void drawRaceStats(void);
static void drawScores(int scoreTable);
static void drawBattleStats(void);
static void drawStuntStats(void);
static void drawStuntTotals(void);
static void drawObstacleStats(void);
static void drawCircuitStats(void);
static void drawTotalStats(void);
static void drawUnlockedTracks(void);
static void drawUnlockedCars(void);
static void drawUnlockedParts(void);
static int animText(void);
static void setRecordState(enum_kRecordStateCount newState);
static void updateSelectInfo(void);
static void updateViewInfo(void);
static void updatePakInfo(void);
static void getScoreStr(float score, unsigned char *str, int unused);
static void initButtons(void);
static void uninitButtons(void);
static void updateButtons(void);
static void updateSelectButtons(void);
static void updateViewButtons(void);

static char *buttonNames[14] = {
    "BUTTON_S2G1", "BUTTON_S2G1", "BUTTON_S2G1", "CURSORG1", "AB_BUTTONG1", "ARROW_BGG1",
    "ARROW_FGG1", "BUTTON_S1G1", "BUTTON_S3G1", "BUTTON_S4G1", "BUTTON_S0G1", "BUTTON_S8G1",
    "BAR_BGG1", "BAR_FGG1"
};
/* declared after buttonNames on purpose: its 14 string literals occupy the first 144 .rodata bytes, before mblit */
static const anon_texname_dulx_24 mblit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animText, 0 };
static anon_buttonType_objIdx_40 buttonInfo[23] = {
    { 0, -1, 0.0f, { 0 }, 0 },
    { 9, -1, 0.0f, { 0 }, 0 },
    { 11, -1, 0.0f, { 0 }, 0 },
    { 0, -1, 0.0f, { 0 }, 0 },
    { 0, -1, 0.0f, { 0 }, 0 },
    { 0, -1, 0.0f, { 0 }, 0 },
    { 0, -1, 0.0f, { 0 }, 0 },
    { 0, -1, 0.0f, { 0 }, 0 },
    { 0, -1, 0.0f, { 0 }, 0 },
    { 0, -1, 0.0f, { 0 }, 0 },
    { 0, -1, 0.0f, { 0 }, 0 },
    { 7, -1, 0.0f, { 0 }, 0 },
    { 7, -1, 0.0f, { 0 }, 0 },
    { 1, -1, 0.0f, { 0 }, 0 },
    { 10, -1, 0.0f, { 0 }, 0 },
    { 8, -1, 0.0f, { 0 }, 0 },
    { 8, -1, 0.0f, { 0 }, 0 },
    { 2, -1, 0.0f, { 0 }, 0 },
    { 3, -1, 0.0f, { 0 }, 0 },
    { 4, -1, 0.0f, { 0 }, 0 },
    { 6, -1, 0.0f, { 0 }, 0 },
    { 4, -1, 0.0f, { 0 }, 0 },
    { 6, -1, 0.0f, { 0 }, 0 }
};
static anon_x_y_c gScreenRect = { 40, 20, 240, 200, 160, 120 };
static signed char gViewTrackValid[30] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
static signed char gViewStatValid[6][13] = {
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1
};
static float gButtonScale = 1.0f;
static float gButtonSpace = 40.0f;
static float maxY = 120.0f;
static float minY = -120.0f;
static float buttonZ = 300.0f;
static float cursorZ = 225.0f;
static int buttonsInited;
static int buttonsJustInited;
static float cursorSpeed;
static float scrollSpeed;
static signed char gViewMenuValid[2];
static signed char gViewMenuVisible[2];
anon_ll_fileh_4c **gStatPlayer;
static enum_kRecordStateCount gRecordState;
static enum_kSelectMenuCount_2 gSelectMenuSelect;
static signed char gSelectMenuClear;
static enum_kViewMenuCount gViewMenuSelect;
static signed char gDeleteConfirm;
static signed char gYes;
static signed char gDeleteNotify;
static signed char gRecordTrack;
static signed char gRecordCircuit;
static enum_kTrackTypeCount gTrackType;
static float gDefTime;
static signed char gSelectPakIdx;
static anon_used_perm_20 *viewPane;
static _tBlit *blit;
static signed char inited;
static float buttonX;
static float gCameraMatrix[4][4];
static unsigned char gDefName[3][5];
static float gDefaultRandoms[6];
static signed char gSelectMenuVisible[11];

static int gNumPakPlayers[4];
static signed char gSelectMenuValid[11];
static unsigned char gDeleteName[13];
static enum_kViewStatTypeCount gViewStatType[6];

/* 0x800806a4, size 0xd8 */
void updateRecordScreen(void)
{
    if (!inited) initRecordScreen();
    updatePakInfo();
    updateSelectInfo();
    if (gNumPakPlayers[gSelectPakIdx] <= 0) {
        if (gStatPlayer && *gStatPlayer) {
            PS_LockFile((*gStatPlayer)->fileh, 0);
            PurgePlayerData(gStatPlayer);
            setRecordState(kRecordStateSelect);
        }
        gStatPlayer = 0;
    }
    if (gSoftReset) {
        uninitRecordScreen();
        setGameState(16);
    } else {
        switch (gRecordState) {
        case kRecordStateSelect: controlSelect(); break;
        case kRecordStateView: controlView(); break;
        }
    }
    updateButtons();
}

/* 0x8008077c, size 0x9c */
static void initRecordScreen(void)
{
    gSelectMenuSelect = 0;
    gStatPlayer = 0;
    gSelectPakIdx = 0;
    setRecordState(kRecordStateSelect);
    createBG();
    initButtons();
    viewPane = InitPane(-20, 0, -10, 10, 192, 0, 0, 0);
    blit = NewMultiBlit(0, 0, mblit, 1);
    gDeleteConfirm = 0;
    gDeleteNotify = 0;
    gYes = 0;
    gSelectMenuClear = 0;
    inited = 1;
}

/* 0x80080818, size 0x5c */
static void uninitRecordScreen(void)
{
    uninitButtons();
    if (viewPane) { RemovePane(viewPane); viewPane = 0; }
    if (blit) { RemoveBlit(blit); blit = 0; }
    removeBG();
    inited = 0;
}

/* 0x80080874, size 0x788 */
static void controlSelect(void)
{
    int p, i;
    while (!gSelectMenuValid[gSelectMenuSelect]) {
        gSelectMenuClear = 0;
        gSelectMenuSelect--;
        if (gSelectMenuSelect < 0) gSelectMenuSelect = 10;
        if (gSelectMenuSelect >= 3 && gSelectMenuSelect < 11 && gSelectMenuValid[gSelectMenuSelect] == 1) {
            if (gStatPlayer) {
                do { gStatPlayer = (*gStatPlayer)->ll.prev; }
                while (gStatPlayer && (*(*gStatPlayer)->fileh)->pak != gSelectPakIdx);
            }
            if (!gStatPlayer) gStatPlayer = gPlayerFiles.tail;
        } else gStatPlayer = 0;
    }
    if (gDeleteNotify == 1) {
    } else if (gDeleteConfirm == 1) {
        if (gActiveAutoEdges & 0x3000) {
            PlayLRSound(gActiveAutoEdges);
            gYes = !gYes;
        }
    } else if (gSelectMenuSelect == 1 && (gActiveLevels & 0x3000)) {
        if (gActiveAutoEdges & 0x1000) {
            PlayLRSound(gActiveAutoEdges);
            gSelectPakIdx--;
            if (gSelectPakIdx < 0) {
                if (MtapConnected) gSelectPakIdx = 3;
                else gSelectPakIdx = 1;
            }
        } else if (gActiveAutoEdges & 0x2000) {
            PlayLRSound(gActiveAutoEdges);
            gSelectPakIdx++;
            if (MtapConnected) {
                if (gSelectPakIdx > 3) gSelectPakIdx = 0;
            } else if (gSelectPakIdx > 1) gSelectPakIdx = 0;
        }
        updateSelectInfo();
        buttonsJustInited = 1;
    } else if (gSelectMenuSelect >= 3 && gSelectMenuSelect <= 11 && (gActiveLevels & 0x3000)) {
        if (gActiveEdges & 0x3000) {
            PlayLRSound(gActiveAutoEdges);
            gSelectMenuClear = !gSelectMenuClear;
        }
    } else if (gActiveAutoEdges & 0x400) {
        PlayUDSound(gActiveAutoEdges);
        gSelectMenuClear = 0;
        do {
            if (gSelectMenuSelect == 3 && gSelectMenuValid[gSelectMenuSelect] == 1) {
                if (gStatPlayer) {
                    do { gStatPlayer = (*gStatPlayer)->ll.prev; }
                    while (gStatPlayer && (*(*gStatPlayer)->fileh)->pak != gSelectPakIdx);
                }
                if (gStatPlayer) break;
            }
            gSelectMenuSelect--;
            if (gSelectMenuSelect < 0) gSelectMenuSelect = 10;
            if (gSelectMenuSelect >= 3 && gSelectMenuSelect < 11 && gSelectMenuValid[gSelectMenuSelect] == 1) {
                if (gStatPlayer) {
                    do { gStatPlayer = (*gStatPlayer)->ll.prev; }
                    while (gStatPlayer && (*(*gStatPlayer)->fileh)->pak != gSelectPakIdx);
                }
                if (!gStatPlayer) {
                    gStatPlayer = gPlayerFiles.tail;
                    while (gStatPlayer && (*(*gStatPlayer)->fileh)->pak != gSelectPakIdx) gStatPlayer = (*gStatPlayer)->ll.prev;
                }
            } else gStatPlayer = 0;
        } while (!gSelectMenuValid[gSelectMenuSelect]);
    } else if (gActiveAutoEdges & 0x800) {
        PlayUDSound(gActiveAutoEdges);
        gSelectMenuClear = 0;
        do {
            if (gSelectMenuSelect == 10 && gSelectMenuValid[gSelectMenuSelect] == 1) {
                if (gStatPlayer) {
                    do { gStatPlayer = (*gStatPlayer)->ll.next; }
                    while (gStatPlayer && (*(*gStatPlayer)->fileh)->pak != gSelectPakIdx);
                }
                if (gStatPlayer) break;
            }
            gSelectMenuSelect++;
            if (gSelectMenuSelect > 10) gSelectMenuSelect = 0;
            if (gSelectMenuSelect >= 3 && gSelectMenuSelect < 11 && gSelectMenuValid[gSelectMenuSelect] == 1) {
                if (gStatPlayer) {
                    do { gStatPlayer = (*gStatPlayer)->ll.next; }
                    while (gStatPlayer && (*(*gStatPlayer)->fileh)->pak != gSelectPakIdx);
                }
                if (!gStatPlayer) {
                    gStatPlayer = gPlayerFiles.head;
                    while (gStatPlayer && (*(*gStatPlayer)->fileh)->pak != gSelectPakIdx) gStatPlayer = (*gStatPlayer)->ll.next;
                }
            } else gStatPlayer = 0;
        } while (!gSelectMenuValid[gSelectMenuSelect]);
    }
    if (gActiveEdges & 2) {
        if (gSelectMenuClear == 1) {
            PlayStartABSound(gActiveEdges);
            if (gDeleteNotify == 1) gDeleteNotify = 0;
            else if (gDeleteConfirm == 1) {
                if (gYes == 1) {
                    if (gStatPlayer) {
                        if (ClearPlayerRecords(gStatPlayer)) gDeleteNotify = 1;
                    } else {
                        for (p = 0; p < 4; p++) ClearPlayerRecords(&gJustPlayers[p]);
                        gDeleteNotify = 1;
                    }
                }
                gDeleteConfirm = 0;
            } else {
                if (gStatPlayer) TB_Strcpy(gDeleteName, (*gStatPlayer)->name);
                else TB_Strcpy(gDeleteName, gText.single[187]);
                gDeleteConfirm = 1;
                gYes = 0;
            }
        } else if (gSelectMenuSelect == 0 || (gSelectMenuSelect >= 3 && gSelectMenuSelect < 11)) {
            PlayStartABSound(gActiveEdges);
            if (gStatPlayer) {
                if (ReadPlayerData(gStatPlayer)) {
                    PS_LockFile((*gStatPlayer)->fileh, 1);
                    setRecordState(kRecordStateView);
                }
            } else {
                anon_ll_fileh_4c **p;
                ClearTotalStats();
                for (i = 0; i < 4; i++) {
                    AddPlayerStats(&gJustPlayers[i]);
                    AddPlayerScores(&gJustPlayers[i]);
                }
                for (p = gPlayerFiles.head; p; p = (*p)->ll.next) {
                    if ((*p)->datah) {
                        AddPlayerStats(p); AddPlayerScores(p);
                    } else {
                        if (!ReadPlayerData(p)) break;
                        AddPlayerStats(p); AddPlayerScores(p);
                        PurgePlayerData(p);
                    }
                }
                if (!p) setRecordState(kRecordStateView);
            }
        }
    } else if (gActiveEdges & 5) {
        PlayStartABSound(gActiveEdges);
        if (gDeleteNotify == 1) gDeleteNotify = 0;
        else if (gDeleteConfirm == 1) gDeleteConfirm = 0;
        else { uninitRecordScreen(); setGameState(16); }
    }
}

/* 0x80080ffc, size 0x270 */
static void controlView(void)
{
    if (gActiveLevels & 0x3000) {
        if (gViewMenuSelect == kViewMenuTrack) {
            signed char add;
            add = (gActiveAutoEdges & 0x1000) ? -1 : ((gActiveAutoEdges & 0x2000) ? 1 : 0);
            if (add) {
                PlayLRSound(gActiveAutoEdges);
                do {
                    gRecordTrack += add;
                    if (gRecordTrack < 0) gRecordTrack = 28 + (gStatPlayer != 0);
                    if (gRecordTrack > 28 + (gStatPlayer != 0)) gRecordTrack = 0;
                } while (!gViewTrackValid[gRecordTrack]);
                updateViewInfo();
            }
        } else {
            signed char add;
            add = (gActiveAutoEdges & 0x1000) ? -1 : ((gActiveAutoEdges & 0x2000) ? 1 : 0);
            if (add) {
                PlayLRSound(gActiveAutoEdges);
                do {
                    gViewStatType[gTrackType] += add;
                    if (gViewStatType[gTrackType] < 0) gViewStatType[gTrackType] = 12;
                    if (gViewStatType[gTrackType] > 12) gViewStatType[gTrackType] = 0;
                } while (!gViewStatValid[gTrackType][gViewStatType[gTrackType]]);
                updateViewInfo();
            }
        }
    } else if (gActiveAutoEdges & 0x400) {
        PlayUDSound(gActiveAutoEdges);
        do {
            gViewMenuSelect--;
            if (gViewMenuSelect < 0) gViewMenuSelect = 1;
        } while (!gViewMenuValid[gViewMenuSelect]);
    } else if (gActiveAutoEdges & 0x800) {
        PlayUDSound(gActiveAutoEdges);
        do {
            gViewMenuSelect++;
            if (gViewMenuSelect > 1) gViewMenuSelect = 0;
        } while (!gViewMenuValid[gViewMenuSelect]);
    }
    while (!gViewStatValid[gTrackType][gViewStatType[gTrackType]]) {
        gViewStatType[gTrackType]++;
        if (gViewStatType[gTrackType] > 12) gViewStatType[gTrackType] = 0;
    }
    while (!gViewMenuValid[gViewMenuSelect]) {
        gViewMenuSelect--;
        if (gViewMenuSelect < 0) gViewMenuSelect = 1;
    }
    if (gActiveEdges & 5) {
        PlayStartABSound(gActiveEdges);
        setRecordState(kRecordStateSelect);
        if (gStatPlayer) {
            PS_LockFile((*gStatPlayer)->fileh, 0);
            PurgePlayerData(gStatPlayer);
        }
    }
}

/* 0x8008126c, size 0xa0c */
static void drawSelect(void)
{
    static unsigned char strPlayer[32];
    int i, x, y, cx;
    unsigned char *str;
    unsigned char str1[64];
    signed short screenPos[2];
    TB_SetJust(0, 1);
    ZOID_ProjectWindow(0, buttonInfo[17].mat.uvs[3], &gCamera[0].mat, 0, screenPos);
    cx = screenPos[0] / 2;
    y = screenPos[1] / 2.0f;
    HAL_SetMenuTextDepth(buttonInfo[17].mat.uvs[3][2] - 10.0f);
    TB_SetTextColor(1);
    HAL_SetFont(13);
    TB_SetTextColor(1);
    str = gText.single[47];
    TB_DrawText(TB_CenterText(str, cx), y, str);
    y += TB_GetFontLineHeight() * 2;
    HAL_SetFont(10);
    if (gDeleteConfirm == 1) {
        HAL_SetTextDepth(0.0f);
        y += TB_GetFontLineHeight();
        if (gStatPlayer) {
            TB_Sprintf(str1, gText.single[153], gDeleteName);
            str = str1;
        } else str = gText.single[154];
        ChangePaneSize(viewPane, cx - gScreenRect.w / 2 - 4, y - 4,
                       cx + gScreenRect.w / 2 + 4, 4 + y + ((TB_GetTextNumLines(gScreenRect.w, str) + 2) * TB_GetFontLineHeight()));
        TB_SetTextColor(1);
        TB_SetJust(1, 3);
        TB_DrawTextScrollBox(cx, y, gScreenRect.w, gScreenRect.h, -1, 0, str);
        TB_SetJust(0, 3);
        y += TB_GetFontLineHeight() * (TB_GetTextNumLines(gScreenRect.w, str) + 1);
        str = gText.taba[gText.tab[25]];
        TB_SetTextColor(gYes == 1 ? 22 : 1);
        TB_DrawText(TB_RightText(str, cx - 5), y, str);
        str = gText.taba[gText.tab[25] + 1];
        TB_SetTextColor(gYes == 0 ? 22 : 1);
        TB_DrawText(cx + 5, y, str);
    } else if (gDeleteNotify == 1) {
        HAL_SetTextDepth(0.0f);
        y += TB_GetFontLineHeight();
        if (gStatPlayer) {
            TB_Sprintf(str1, gText.single[156], gDeleteName);
            str = str1;
        } else str = gText.single[157];
        ChangePaneSize(viewPane, cx - gScreenRect.w / 2 - 4, y - 4,
                       cx + gScreenRect.w / 2 + 4, 4 + y + (TB_GetTextNumLines(gScreenRect.w, str) * TB_GetFontLineHeight()));
        TB_SetTextColor(1);
        TB_SetJust(1, 3);
        TB_DrawTextScrollBox(cx, y, gScreenRect.w, gScreenRect.h, -1, 0, str);
        TB_SetJust(0, 3);
    } else {
    for (i = 0; i < 11; i++) {
        ChangePaneSize(viewPane, -20, 0, -10, 10);
        HAL_SetMenuTextDepth(buttonInfo[i].mat.uvs[3][2] - 10.0f);
        ZOID_ProjectWindow(0, buttonInfo[i].mat.uvs[3], &gCamera[0].mat, 0, screenPos);
        if (buttonInfo[i].buttonType == 11) x = screenPos[0] / 2;
        else if (buttonInfo[i].buttonType == 9) x = screenPos[0] / 2 - 300.0f * (65.0f * gButtonScale) / buttonZ;
        else x = screenPos[0] / 2 - 300.0f * (45.0f * gButtonScale) / buttonZ;
        y = screenPos[1] / 2.0f;
        if (i == 1) {
            ZOID_ProjectWindow(0, buttonInfo[14].mat.uvs[3], &gCamera[0].mat, 0, screenPos);
            cx = screenPos[0] / 2 - 300.0f * (25.0f * gButtonScale) / buttonZ;
        } else {
            ZOID_ProjectWindow(0, buttonInfo[13].mat.uvs[3], &gCamera[0].mat, 0, screenPos);
            cx = screenPos[0] / 2 - 300.0f * (45.0f * gButtonScale) / buttonZ;
        }
        if (i >= 3 && i < 11 && i == gSelectMenuSelect &&
            (buttonInfo[13].angle < 0.6981317f || buttonInfo[13].angle > 2.443461f) && buttonInfo[13].alpha) {
            HAL_SetTextTranslucency(buttonInfo[13].alpha);
            if (buttonInfo[13].angle < 1.5707964f) {
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
            str = gText.taba[gText.tab[52] + 2];
            HAL_SetFont(11);
            TB_DrawText(cx, y, str);
            HAL_SetFont(10);
        }
        if (buttonInfo[i].angle > 0.6981317f && buttonInfo[i].angle < 2.443461f) continue;
        if (!buttonInfo[i].alpha) continue;
        HAL_SetTextTranslucency(buttonInfo[i].alpha);
        if (buttonInfo[i].angle < 1.5707964f) {
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
        if (i == 0) {
            str = gText.taba[gText.tab[52]];
            TB_DrawText(x, y, str);
        } else if (i == 1) {
            TB_DrawText(x, y, "CHOOSE PLAYER");
            sprintf((char *)strPlayer, "PLAYER %d", gSelectPakIdx + 1);
            str = strPlayer;
            TB_DrawText(cx - 6, y, str);
        } else if (i == 2) {
            TB_SetTextColor(0);
            str = gText.single[158];
            TB_DrawText(TB_CenterText(str, x), y, str);
        } else if (i >= 3 && i < 11) {
            anon_ll_fileh_4c **player;
            int playerIdx;
            if (!gStatPlayer) {
                for (playerIdx = 0, player = gPlayerFiles.head; player; player = (*player)->ll.next) {
                    if ((*(*player)->fileh)->pak == gSelectPakIdx) {
                        if (playerIdx == i - 3) str = (*player)->name;
                        playerIdx++;
                    }
                }
            } else {
                for (playerIdx = 0, player = gStatPlayer; player && *player;
                     player = i < gSelectMenuSelect ? (*player)->ll.prev : (*player)->ll.next) {
                    if ((*(*player)->fileh)->pak == gSelectPakIdx) {
                        if (playerIdx == (i - gSelectMenuSelect >= 0 ? i - gSelectMenuSelect : -(i - gSelectMenuSelect))) str = (*player)->name;
                        playerIdx++;
                    }
                }
            }
            if (HAL_GetTextWidth(str, -1) > 140) HAL_SetFontSpacing(0);
            TB_DrawEllipseText(x, y, 98, str);
            HAL_SetFontSpacing(-1);
        }
        TB_GetFontLineHeight();
    }
    }
    TB_SetJust(0, 3);
}

/* 0x80081c78, size 0x568 */
static void drawView(void)
{
    int i, x, y, cx;
    unsigned char *str;
    unsigned char str1[64];
    signed short screenPos[2];
    y = 1;
    TB_SetJust(y, y);
    ZOID_ProjectWindow(0, buttonInfo[17].mat.uvs[3], &gCamera[0].mat, 0, screenPos);
    cx = screenPos[0] / 2;
    y = screenPos[1] / 2.0f;
    HAL_SetMenuTextDepth(buttonInfo[17].mat.uvs[3][2] - 10.0f);
    TB_SetTextColor(1);
    HAL_SetFont(13);
    TB_SetTextColor(1);
    TB_DrawEllipseText(cx, y, 140, gStatPlayer ? (*gStatPlayer)->name : gText.single[186]);
    TB_SetJust(0, 1);
    TB_GetFontLineHeight();
    HAL_SetFont(11);
    for (i = 0; i < 2; i++) {
        if (buttonInfo[i + 11].angle > 0.6981317f && buttonInfo[i + 11].angle < 2.443461f) continue;
        if (!buttonInfo[i + 11].alpha) continue;
        HAL_SetTextTranslucency(buttonInfo[i + 11].alpha);
        if (buttonInfo[i + 11].angle < 1.5707964f) {
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
        HAL_SetMenuTextDepth(buttonInfo[i + 11].mat.uvs[3][2] - 10.0f);
        ZOID_ProjectWindow(0, buttonInfo[i + 11].mat.uvs[3], &gCamera[0].mat, 0, screenPos);
        x = screenPos[0] / 2 - 300.0f * (35.0f * gButtonScale) / buttonZ;
        y = screenPos[1] / 2.0f;
        ZOID_ProjectWindow(0, buttonInfo[15].mat.uvs[3], &gCamera[0].mat, 0, screenPos);
        cx = screenPos[0] / 2 - 300.0f * (55.0f * gButtonScale) / buttonZ;
        TB_DrawText(x, y, gText.taba[gText.tab[53] + i]);
        x = cx;
        if (i == 0) {
            if (gTrackType == kTrackTypeCircuit) {
                TB_Sprintf(str1, "%s %s", gText.taba[gText.tab[45] + gRecordCircuit], gText.single[42]);
                str = str1;
            } else if (gTrackType == kTrackTypeTotals) str = gText.single[186];
            else if (gRecordTrack < 12 && (gRecordTrack & 1)) {
                TB_Sprintf(str1, "%s %s", gTrackTitles[gRecordTrack > 12 ? gRecordTrack - 6 : gRecordTrack / 2], gText.single[6]);
                str = str1;
            } else str = gTrackTitles[gRecordTrack > 12 ? gRecordTrack - 6 : gRecordTrack / 2];
            TB_DrawText(x, y, str);
        } else if (i == 1) TB_DrawText(x, y, gText.taba[gText.tab[54] + gViewStatType[gTrackType]]);
    }
    TB_SetJust(0, 3);
    HAL_SetTextTranslucency(-1.0f);
    HAL_SetTextDepth(0.0f);
    switch (gViewStatType[gTrackType]) {
    case kViewRaceStats: drawRaceStats(); break;
    case kViewLapTimes: drawScores(0); break;
    case kViewAverageTimes: drawScores(1); break;
    case kViewGhostTimes: drawScores(2); break;
    case kViewBattleStats: drawBattleStats(); break;
    case kViewStuntStats: drawStuntStats(); break;
    case kViewStuntTotals: drawStuntTotals(); break;
    case kViewObstacleStats: drawObstacleStats(); break;
    case kViewCircuitStats: drawCircuitStats(); break;
    case kViewTotalStats: drawTotalStats(); break;
    case kViewUnlockedTracks: drawUnlockedTracks(); break;
    case kViewUnlockedCars: drawUnlockedCars(); break;
    case kViewUnlockedParts: drawUnlockedParts(); break;
    }
    TB_SetJust(0, 3);
}

/* 0x800821e0, size 0x40c */
static void drawRaceStats(void)
{
    int whichTrack;
    int statIdx;
    int x;
    int y;
    anon_checksum_scores_60 *stats;
    unsigned int totalMiles;
    unsigned char *str;
    unsigned char str1[40];
    float avgTime;
    TB_SetJust(0, 3);
    whichTrack = gRecordTrack / 2;
    if (gRecordTrack & 1) whichTrack += 6;
    if (gStatPlayer) stats = &(*(*gStatPlayer)->datah)->raceStats[whichTrack];
    else stats = &gTotalRaceStats[whichTrack];
    totalMiles = stats->totalMiles;
    if (gOptions[9]) { totalMiles *= 10; totalMiles /= 6; }
    HAL_SetFont(11);
    ChangePaneSize(viewPane, 12, 88, 308, 96 + TB_GetFontLineHeight() * 8);
    TB_SetTextColor(1);
    for (statIdx = 0; statIdx < 14; statIdx++) {
        if (statIdx == 0) { x = 100; y = 92; }
        else if (statIdx == 3) { x = 110; y += TB_GetFontLineHeight(); }
        else if (statIdx == 7) { x = 270; y = 92 + TB_GetFontLineHeight() * 4; }
        else if (statIdx == 10) { x = 260; y = 92; continue; }
        else if (statIdx == 12 || statIdx == 13) { if (!gStatPlayer) continue; }
        str = gText.taba[gText.tab[29] + statIdx];
        if (statIdx == 11 && gOptions[9]) str = gText.taba[gText.tab[29] + 14];
        TB_DrawText(TB_RightText(str, x - 4), y, str);
        switch (statIdx) {
        case 0: getScoreStr(stats->totalTime, str1, 0); break;
        case 1: TB_Sprintf(str1, "%d", stats->totalLaps); break;
        case 2:
            avgTime = 0.0f;
            if (stats->totalLaps) avgTime = stats->totalTime / stats->totalLaps;
            getScoreStr(avgTime, str1, 0);
            break;
        case 3: TB_Sprintf(str1, "%d", stats->totalRaces); break;
        case 4: TB_Sprintf(str1, "%d", stats->total1st); break;
        case 5: TB_Sprintf(str1, "%d", stats->total2nd); break;
        case 6: TB_Sprintf(str1, "%d", stats->total3rd); break;
        case 7: TB_Sprintf(str1, "%d", stats->totalCrashes); break;
        case 8: TB_Sprintf(str1, "%d", stats->totalDeathAttempted); break;
        case 9: TB_Sprintf(str1, "%d", stats->totalDeathCompleted); break;
        case 10: TB_Sprintf(str1, "%d", stats->totalGhosts); break;
    case 11: TB_Sprintf(str1, "%01d.%01d", totalMiles / 10, totalMiles % 10); break;
        case 12: { int totalCoins = CountBits(stats->coinsFound & 0xff, 16); TB_Sprintf(str1, "%d/8", totalCoins); } break;
        case 13: { int totalCoins = CountBits(stats->coinsFound & 0xff00, 16); TB_Sprintf(str1, "%d/8", totalCoins); } break;
        }
        str = str1;
        if (statIdx == 0 || statIdx == 2) HAL_SetFont(2);
        TB_DrawText(x + 4, y, str);
        if (statIdx == 0 || statIdx == 2) HAL_SetFont(11);
        y += TB_GetFontLineHeight();
    }
}

/* 0x800825ec, size 0x160 */
static void drawScores(int scoreTable)
{
    int whichTrack;
    int scoreIdx;
    int y;
    anon_checksum_scores_60 *stats;
    unsigned char str1[40];
    TB_SetJust(1, 3);
    whichTrack = gRecordTrack / 2;
    if (gRecordTrack & 1) whichTrack += 6;
    if (gStatPlayer) stats = &(*(*gStatPlayer)->datah)->raceStats[whichTrack];
    else stats = &gTotalRaceStats[whichTrack];
    HAL_SetFont(11);
    TB_SetTextColor(1);
    TB_DrawText(160, 92, gText.taba[gText.tab[28] + scoreTable]);
    y = 92; y += TB_GetFontLineHeight();
    HAL_SetFont(2);
    for (scoreIdx = 0; scoreIdx < 5; scoreIdx++) {
        getScoreStr(stats->scores[scoreTable][scoreIdx], str1, 0);
        TB_DrawText(160, y, str1);
        y += TB_GetFontLineHeight();
    }
    ChangePaneSize(viewPane, 100, 88, 220, 96 + TB_GetFontLineHeight() * 6);
}

/* 0x8008274c, size 0x190 */
static void drawBattleStats(void)
{
    int statIdx;
    int x;
    int y;
    anon_checksum_totalMatches_c *stats;
    unsigned char *str;
    unsigned char str1[40];
    if (gStatPlayer) stats = &(*(*gStatPlayer)->datah)->battleStats[gRecordTrack - 12];
    else stats = &gTotalBattleStats[gRecordTrack - 12];
    HAL_SetFont(11);
    TB_SetTextColor(1);
    for (statIdx = 0; statIdx < 3; statIdx++) {
        if (statIdx == 0) y = 92;
        str = gText.taba[gText.tab[30] + statIdx];
    TB_DrawText(TB_RightText(str, 186), y, str);
        switch (statIdx) {
        case 0: TB_Sprintf(str1, "%d", stats->totalMatches); break;
        case 1: TB_Sprintf(str1, "%d", stats->totalWins); break;
        case 2: TB_Sprintf(str1, "%d", stats->totalKills); break;
        case 3: TB_Sprintf(str1, "%d", stats->totalDeaths); break;
        }
        TB_DrawText(194, y, str1);
        y += TB_GetFontLineHeight();
    }
    ChangePaneSize(viewPane, 80, 88, 240, y + 4);
}

/* 0x800828dc, size 0x254 */
static void drawStuntStats(void)
{
    int statIdx;
    int x;
    int y;
    anon_checksum_bestStunt_40 *stats;
    int statsDrawn;
    unsigned char *str;
    unsigned char str1[40];
    if (gStatPlayer) stats = &(*(*gStatPlayer)->datah)->stuntStats[gRecordTrack - 20];
    else stats = &gTotalStuntStats[gRecordTrack - 20];
    HAL_SetFont(11);
    TB_SetTextColor(1);
    statsDrawn = 0;
    for (statIdx = 0; statIdx < 8; statIdx++) {
        if (statIdx == 0) y = 92;
        else if (statIdx == 4 || statIdx == 5) continue;
        else if (statIdx == 6 || statIdx == 7) { if (!gStatPlayer) continue; }
        str = gText.taba[gText.tab[31] + statIdx];
        TB_DrawText(TB_RightText(str, 186), y, str);
        switch (statIdx) {
        case 0: TB_Sprintf(str1, "%d", stats->bestStunt); break;
        case 1: TB_Sprintf(str1, "%d", stats->bestAvg); break;
        case 2: TB_Sprintf(str1, "%d", stats->totalPoints); break;
        case 3: TB_Sprintf(str1, "%d", stats->totalTime); break;
        case 4: TB_Sprintf(str1, "%d", stats->totalCrashes); break;
        case 5: TB_Sprintf(str1, "%d", stats->totalStunts[0]); break;
        case 6: { int totalCoins = CountBits(stats->coinsFound & 0xff, 16); TB_Sprintf(str1, "%d/8", totalCoins); } break;
        case 7: { int totalCoins = CountBits(stats->coinsFound & 0xff00, 16); TB_Sprintf(str1, "%d/8", totalCoins); } break;
        }
        TB_DrawText(194, y, str1);
        y += TB_GetFontLineHeight();
        statsDrawn++;
    }
    ChangePaneSize(viewPane, 80, 88, 240, 96 + statsDrawn * TB_GetFontLineHeight());
}

/* 0x80082b30, size 0x164 */
static void drawStuntTotals(void)
{
    int statIdx;
    int x;
    int y;
    anon_checksum_bestStunt_40 *stats;
    unsigned char str[40];
    if (gStatPlayer) stats = &(*(*gStatPlayer)->datah)->stuntStats[gRecordTrack - 20];
    else stats = &gTotalStuntStats[gRecordTrack - 20];
    HAL_SetFont(11);
    TB_SetTextColor(1);
    for (statIdx = 0; statIdx < 10; statIdx++) {
        if (statIdx == 0) { x = 110; y = 92; }
        else if (statIdx == 5) { x = 250; y = 92; }
        TB_Sprintf(str, "%s:", gText.taba[gText.tab[38] + statIdx]);
        TB_DrawText(TB_RightText(str, x - 4), y, str);
        TB_Sprintf(str, "%d", stats->totalStunts[statIdx]);
        TB_DrawText(x + 4, y, str);
        y += TB_GetFontLineHeight();
    }
    ChangePaneSize(viewPane, 20, 88, 300, 96 + TB_GetFontLineHeight() * 5);
}

/* 0x80082c94, size 0x214 */
static void drawObstacleStats(void)
{
    int statIdx;
    int x;
    int y;
    anon_checksum_bestTime_18 *stats;
    unsigned char *str;
    unsigned char str1[40];
    if (gStatPlayer) stats = &(*(*gStatPlayer)->datah)->obstacleStats[gRecordTrack - 24];
    else stats = &gTotalObstacleStats[gRecordTrack - 24];
    HAL_SetFont(11);
    TB_SetTextColor(1);
    for (statIdx = 0; statIdx < 4; statIdx++) {
        if (statIdx == 0) y = 92;
        else if (statIdx == 4) continue;
        str = gText.taba[gText.tab[32] + statIdx];
    TB_DrawText(TB_RightText(str, 186), y, str);
        switch (statIdx) {
        case 0: getScoreStr(stats->bestTime, str1, 0); break;
        case 1: getScoreStr(stats->totalTime, str1, 0); break;
        case 2: TB_Sprintf(str1, "%d", stats->totalRaces); break;
        case 3: TB_Sprintf(str1, "%d", stats->totalFinished); break;
        case 4: TB_Sprintf(str1, "%d", stats->totalCrashes); break;
        case 5: TB_Sprintf(str1, "%d", stats->numDeathAttempted); break;
        case 6: TB_Sprintf(str1, "%d", stats->numDeathCompleted); break;
        }
        if (statIdx == 0 || statIdx == 1) HAL_SetFont(2);
        TB_DrawText(194, y, str1);
        if (statIdx == 0 || statIdx == 1) HAL_SetFont(11);
        y += TB_GetFontLineHeight();
    }
    ChangePaneSize(viewPane, 40, 88, 280, 96 + TB_GetFontLineHeight() * 4);
}

/* 0x80082ea8, size 0x218 */
static void drawCircuitStats(void)
{
    int statIdx;
    int x;
    int y;
    anon_checksum_numCircuits_1c *stats;
    unsigned char *str;
    unsigned char str1[40];
    if (gStatPlayer) stats = &(*(*gStatPlayer)->datah)->circuitStats[gRecordCircuit];
    else stats = &gTotalCircuitStats[gRecordCircuit];
    HAL_SetFont(11);
    TB_SetTextColor(1);
    for (statIdx = 0; statIdx < 8; statIdx++) {
        if (statIdx == 0) y = 92;
        str = gText.taba[gText.tab[33] + statIdx];
    TB_DrawText(TB_RightText(str, 176), y, str);
        switch (statIdx) {
        case 0: TB_Sprintf(str1, "%d", stats->numCircuits); break;
        case 1: TB_Sprintf(str1, "%d", stats->num1st); break;
        case 2: TB_Sprintf(str1, "%d", stats->num2nd); break;
        case 3: TB_Sprintf(str1, "%d", stats->num3rd); break;
        case 4: TB_Sprintf(str1, "%d", stats->numDeathAttempted); break;
        case 5: TB_Sprintf(str1, "%d", stats->numDeathCompleted); break;
        case 6: TB_Sprintf(str1, "%d", stats->bestPts); break;
        case 7: getScoreStr(stats->bestTime, str1, 0); break;
        case 8: TB_Sprintf(str1, "%d", stats->totalPts); break;
        }
        if (statIdx == 7) HAL_SetFont(2);
        TB_DrawText(184, y, str1);
        if (statIdx == 7) HAL_SetFont(11);
        y += TB_GetFontLineHeight();
    }
    ChangePaneSize(viewPane, 50, 88, 270, y + 4);
}

/* 0x800830c0, size 0x3c8 */
static void drawTotalStats(void)
{
    signed short statIdx, i, x, y;
    unsigned int totalMiles = 0;
    unsigned int totalStuntPoints = 0;
    unsigned short totalBattlePoints;
    unsigned short silverRaceCoins = 0;
    unsigned short goldRaceCoins = 0;
    unsigned short silverStuntCoins = 0;
    unsigned short goldStuntCoins = 0;
    unsigned char *str;
    unsigned char str1[40];
    if (gStatPlayer) {
        TB_SetJust(0, 3);
        for (i = 0; i < 12; i++) {
            anon_checksum_scores_60 *stats = &(*(*gStatPlayer)->datah)->raceStats[i];
            totalMiles += stats->totalMiles;
            if (i < 6) {
                silverRaceCoins += CountBits(stats->coinsFound & 0xff, 16);
                goldRaceCoins += CountBits(stats->coinsFound & 0xff00, 16);
            }
        }
        for (i = 0; i < 4; i++) {
            anon_checksum_bestStunt_40 *stats = &(*(*gStatPlayer)->datah)->stuntStats[i];
            totalStuntPoints += stats->totalPoints;
            silverStuntCoins += CountBits(stats->coinsFound & 0xff, 16);
            goldStuntCoins += CountBits(stats->coinsFound & 0xff00, 16);
        }
        totalBattlePoints = 0;
        totalBattlePoints += (*(*gStatPlayer)->datah)->battleStats[0].totalKills;
        totalBattlePoints += (*(*gStatPlayer)->datah)->battleStats[1].totalKills;
        totalBattlePoints += (*(*gStatPlayer)->datah)->battleStats[2].totalKills;
        totalBattlePoints += (*(*gStatPlayer)->datah)->battleStats[3].totalKills;
        totalBattlePoints += (*(*gStatPlayer)->datah)->battleStats[4].totalKills;
        totalBattlePoints += (*(*gStatPlayer)->datah)->battleStats[5].totalKills;
        totalBattlePoints += (*(*gStatPlayer)->datah)->battleStats[6].totalKills;
        totalBattlePoints += (*(*gStatPlayer)->datah)->battleStats[7].totalKills;
        HAL_SetFont(11);
        TB_SetTextColor(1);
        for (statIdx = 0; statIdx < 7; statIdx++) {
            if (statIdx == 0) y = 92;
            str = gText.taba[gText.tab[34] + statIdx];
            if (statIdx == 0 && gOptions[9]) str = gText.taba[gText.tab[34] + 7];
            TB_DrawText(TB_RightText(str, 176), y, str);
            switch (statIdx) {
            case 0:
                if (gOptions[9]) TB_Sprintf(str1, "%01d.%01d", totalMiles * 10 / 6 / 10, totalMiles * 10 / 6 % 10);
                else TB_Sprintf(str1, "%01d.%01d", totalMiles / 10, totalMiles % 10);
                break;
            case 1: TB_Sprintf(str1, "%d", totalStuntPoints); break;
            case 2: TB_Sprintf(str1, "%d", totalBattlePoints); break;
            case 3: TB_Sprintf(str1, "%d/%d", silverRaceCoins, 48); break;
            case 4: TB_Sprintf(str1, "%d/%d", goldRaceCoins, 48); break;
            case 5: TB_Sprintf(str1, "%d/%d", silverStuntCoins, 32); break;
            case 6: TB_Sprintf(str1, "%d/%d", goldStuntCoins, 32); break;
            }
            TB_DrawText(184, y, str1);
            y += TB_GetFontLineHeight();
        }
        ChangePaneSize(viewPane, 12, 88, 308, 96 + TB_GetFontLineHeight() * 7);
    }
}

/* 0x80083488, size 0x9b8 */
static void drawUnlockedTracks(void)
{
    anon_ll_fileh_4c **tempPlayer;
    int tempNumPlayers;
    signed short i, x, y, w, b;
    int noRaceTracksUnlocked = 1;
    int noStuntTracksUnlocked = 1;
    int noBattleTracksUnlocked = 1;
    int noCircuitsUnlocked = 1;
    int noneUnlocked;
    unsigned char *str;
    w = 20;
    if (gStatPlayer) {
    TB_SetJust(0, 3);
    tempNumPlayers = gNumPlayers;
    tempPlayer = gPlayers[0].save;
    gPlayers[0].save = gStatPlayer;
    gNumPlayers = 1;
    CalcUnlockedStuff();
    gPlayers[0].save = tempPlayer;
    gNumPlayers = tempNumPlayers;
    TB_SetTextColor(1);
    x = 80; y = 92;
    HAL_SetFont(11);
    str = gText.single[202];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    HAL_SetFont(12);
    noneUnlocked = 1;
    for (i = 4; i <= 5; i++) {
    if (gUnlockedTracks[0][i]) {
    noneUnlocked = 0;
    str = gTrackTitles[i];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    }}
    if (noneUnlocked == 1) {
    str = gText.taba[gText.tab[16]];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    } else noRaceTracksUnlocked = 0;
    y += TB_GetFontLineHeight();
    HAL_SetFont(11);
    str = gText.taba[gText.tab[23] + 6];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    HAL_SetFont(12);
    noneUnlocked = 1;
    for (i = 10; i <= 13; i++) {
    if (gUnlockedTracks[0][i]) {
    noneUnlocked = 0;
    str = gTrackTitles[i];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    }}
    if (noneUnlocked == 1) {
    str = gText.taba[gText.tab[16]];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    } else noBattleTracksUnlocked = 0;
    b = y; x = 240; y = 92;
    HAL_SetFont(11);
    str = gText.single[204];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    HAL_SetFont(12);
    noneUnlocked = 1;
    for (i = 1; i < 4; i++) {
    if (gUnlockedCircuits[0][i]) {
    noneUnlocked = 0;
    str = gText.taba[gText.tab[45] + i];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    }}
    if (noneUnlocked == 1) {
    str = gText.taba[gText.tab[16]];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    } else noCircuitsUnlocked = 0;
    y += TB_GetFontLineHeight();
    HAL_SetFont(11);
    str = gText.taba[gText.tab[23] + 4];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    HAL_SetFont(12);
    noneUnlocked = 1;
    for (i = 15; i <= 17; i++) {
    if (gUnlockedTracks[0][i]) {
    noneUnlocked = 0;
    str = gTrackTitles[i];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    }}
    if (noneUnlocked == 1) {
    str = gText.taba[gText.tab[16]];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    } else noStuntTracksUnlocked = 0;
    w += 160;
    y = b > y ? b : y;
    if (gUnlockedTracks[0][18]) {
    x = 160;
    HAL_SetFont(11);
    str = gText.taba[gText.tab[23] + 5];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    HAL_SetFont(12);
    str = gTrackTitles[18];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    }
    if (noRaceTracksUnlocked || noStuntTracksUnlocked || noBattleTracksUnlocked || noCircuitsUnlocked) { x = 160; y += TB_GetFontLineHeight(); }
    if (noRaceTracksUnlocked) {
    str = gText.single[61];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    }
    if (noCircuitsUnlocked) {
    str = gText.single[64];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    }
    if (noBattleTracksUnlocked) {
    str = gText.single[63];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    }
    if (noStuntTracksUnlocked) {
    str = gText.single[62];
    w = w > HAL_GetTextWidth(str, -1) ? w : HAL_GetTextWidth(str, -1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    }
    y = b > y ? b : y;
    ChangePaneSize(viewPane, 152 - w / 2, 88, 168 + w / 2, y + 4);
    }
}

/* 0x80083e40, size 0x1d0 */
static void drawUnlockedCars(void)
{
    anon_ll_fileh_4c **tempPlayer;
    int tempNumPlayers;
    signed short i;
    signed short y;
    signed short w;
    w = 20;
    if (gStatPlayer) {
        TB_SetJust(0, 3);
        tempNumPlayers = gNumPlayers;
        tempPlayer = gPlayers[0].save;
        gPlayers[0].save = gStatPlayer;
    gNumPlayers = 1;
        CalcUnlockedStuff();
        gPlayers[0].save = tempPlayer;
        gNumPlayers = tempNumPlayers;
        HAL_SetFont(12);
        TB_SetTextColor(1);
        y = 92;
        for (i = 6; i < 13; i++) {
        if ((unsigned short)i <= 4 || i == 5 || !gUnlockedCars[0][i]) continue;
            w = w > HAL_GetTextWidth(CarTitles[i], -1) ? w : HAL_GetTextWidth(CarTitles[i], -1);
            TB_DrawText(TB_CenterText(CarTitles[i], 160), y, CarTitles[i]);
            y += TB_GetFontLineHeight();
        }
        if (y == 92) {
            w = w > HAL_GetTextWidth(gText.single[65], -1) ? w : HAL_GetTextWidth(gText.single[65], -1);
            TB_DrawText(TB_CenterText(gText.single[65], 160), y, gText.single[65]);
            y += TB_GetFontLineHeight();
        }
        ChangePaneSize(viewPane, 152 - w / 2, 88, 168 + w / 2, y + 4);
    }
}

/* 0x80084010, size 0x50c */
static void drawUnlockedParts(void)
{
    anon_ll_fileh_4c **tempPlayer;
    int tempNumPlayers;
    signed short i, y, b;
    int noneUnlocked;
    int noPartsUnlocked = 1;
    unsigned char *str;
    if (gStatPlayer) {
    TB_SetJust(0, 3);
    tempNumPlayers = gNumPlayers;
    tempPlayer = gPlayers[0].save;
    gPlayers[0].save = gStatPlayer;
    gNumPlayers = 1;
    CalcUnlockedStuff();
    gPlayers[0].save = tempPlayer;
    gNumPlayers = tempNumPlayers;
    TB_SetTextColor(1);
    HAL_SetFont(11);
    str = gText.taba[gText.tab[35] + 0];
    TB_DrawText(TB_CenterText(str, 80), 92, str);
    y = 92; y += TB_GetFontLineHeight();
    HAL_SetFont(12);
    noneUnlocked = 1;
    for (i = 3; i < 9; i++) {
    if (gUnlockedEngines[0][i]) {
    noneUnlocked = 0;
    str = gText.taba[gText.tab[7] + i];
    TB_DrawText(TB_CenterText(str, 80), y, str);
    y += TB_GetFontLineHeight();
    }}
    if (noneUnlocked == 1) {
    str = gText.taba[gText.tab[16]];
    TB_DrawText(TB_CenterText(str, 80), y, str);
    y += TB_GetFontLineHeight();
    } else noPartsUnlocked = 0;
    y += TB_GetFontLineHeight();
    HAL_SetFont(11);
    str = gText.taba[gText.tab[35] + 2];
    TB_DrawText(TB_CenterText(str, 80), y, str);
    y += TB_GetFontLineHeight();
    HAL_SetFont(12);
    noneUnlocked = 1;
    for (i = 1; i < 4; i++) {
    if (gUnlockedTrans[0][i*2]) {
    noneUnlocked = 0;
    str = gText.taba[gText.tab[5] + i];
    TB_DrawText(TB_CenterText(str, 80), y, str);
    y += TB_GetFontLineHeight();
    }}
    if (noneUnlocked == 1) {
    str = gText.taba[gText.tab[16]];
    TB_DrawText(TB_CenterText(str, 80), y, str);
    y += TB_GetFontLineHeight();
    } else noPartsUnlocked = 0;
    b = y + 4;
    HAL_SetFont(11);
    str = gText.taba[gText.tab[35] + 1];
    TB_DrawText(TB_CenterText(str, 240), 92, str);
    y = 92; y += TB_GetFontLineHeight();
    HAL_SetFont(12);
    noneUnlocked = 1;
    for (i = 3; i < 6; i++) {
    if (gUnlockedFrames[0][i]) {
    noneUnlocked = 0;
    str = gText.taba[gText.tab[9] + i];
    TB_DrawText(TB_CenterText(str, 240), y, str);
    y += TB_GetFontLineHeight();
    }}
    if (noneUnlocked == 1) {
    str = gText.taba[gText.tab[16]];
    TB_DrawText(TB_CenterText(str, 240), y, str);
    y += TB_GetFontLineHeight();
    } else noPartsUnlocked = 0;
    y += TB_GetFontLineHeight();
    HAL_SetFont(11);
    str = gText.taba[gText.tab[35] + 3];
    TB_DrawText(TB_CenterText(str, 240), y, str);
    y += TB_GetFontLineHeight();
    HAL_SetFont(12);
    noneUnlocked = 1;
    for (i = 1; i < 5; i++) {
    if (gUnlockedTires[0][i]) {
    noneUnlocked = 0;
    str = gText.taba[gText.tab[8] + i];
    TB_DrawText(TB_CenterText(str, 240), y, str);
    y += TB_GetFontLineHeight();
    }}
    if (noneUnlocked == 1) {
    str = gText.taba[gText.tab[16]];
    TB_DrawText(TB_CenterText(str, 240), y, str);
    y += TB_GetFontLineHeight();
    } else noPartsUnlocked = 0;
    if (noPartsUnlocked) {
    y += TB_GetFontLineHeight();
    str = gText.single[66];
    TB_DrawText(TB_CenterText(str, 160), y, str);
    y += TB_GetFontLineHeight();
    }
    ChangePaneSize(viewPane, 12, 88, 308, (signed short)(b > y + 4 ? b : y + 4));
    }
}

/* 0x8008451c, size 0x58 */
static int animText(void)
{
    HAL_SetTextDepth(0.0f);
    switch (gRecordState) {
    case kRecordStateSelect: drawSelect(); break;
    case kRecordStateView: drawView(); break;
    }
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x80084574, size 0xd8 */
static void setRecordState(enum_kRecordStateCount newState)
{
    gRecordState = newState;
    switch (newState) {
    case kRecordStateSelect:
        updatePakInfo();
        updateSelectInfo();
        gDeleteConfirm = 0; gDeleteNotify = 0; gYes = 0; gSelectMenuClear = 0;
        break;
    case kRecordStateView:
        if (!gStatPlayer && gRecordTrack > 28) {
            gRecordTrack = 0;
            while (!gViewStatValid[gTrackType][gViewStatType[gTrackType]]) {
                gViewStatType[gTrackType]++;
                if (gViewStatType[gTrackType] > 12) gViewStatType[gTrackType] = 0;
            }
        }
        updateViewInfo();
        break;
    }
    buttonsJustInited = 1;
}

/* 0x8008464c, size 0x1a4 */
static void updateSelectInfo(void)
{
    gSelectMenuValid[0] = 1;
        gSelectMenuVisible[0] = 1;
        gSelectMenuValid[1] = 1;
        gSelectMenuVisible[1] = 1;
        gSelectMenuValid[2] = 0;
        gSelectMenuVisible[2] = gNumPakPlayers[gSelectPakIdx] <= 0;
        gSelectMenuValid[3] = gNumPakPlayers[gSelectPakIdx] > 0;
        gSelectMenuVisible[3] = gNumPakPlayers[gSelectPakIdx] > 0;
        gSelectMenuValid[4] = gNumPakPlayers[gSelectPakIdx] > 1;
        gSelectMenuVisible[4] = gNumPakPlayers[gSelectPakIdx] > 1;
        gSelectMenuValid[5] = gNumPakPlayers[gSelectPakIdx] > 2;
        gSelectMenuVisible[5] = gNumPakPlayers[gSelectPakIdx] > 2;
        gSelectMenuValid[6] = gNumPakPlayers[gSelectPakIdx] > 3;
        gSelectMenuVisible[6] = gNumPakPlayers[gSelectPakIdx] > 3;
        gSelectMenuValid[7] = gNumPakPlayers[gSelectPakIdx] > 4;
        gSelectMenuVisible[7] = gNumPakPlayers[gSelectPakIdx] > 4;
        gSelectMenuValid[8] = gNumPakPlayers[gSelectPakIdx] > 5;
        gSelectMenuVisible[8] = gNumPakPlayers[gSelectPakIdx] > 5;
        gSelectMenuValid[9] = gNumPakPlayers[gSelectPakIdx] > 6;
        gSelectMenuVisible[9] = gNumPakPlayers[gSelectPakIdx] > 6;
        gSelectMenuValid[10] = gNumPakPlayers[gSelectPakIdx] > 7;
        gSelectMenuVisible[10] = gNumPakPlayers[gSelectPakIdx] > 7;
}

/* 0x800847f0, size 0x9c */
static void updateViewInfo(void)
{
    if (gRecordTrack < 12) gTrackType = 0;
    else if (gRecordTrack < 20) gTrackType = 1;
    else if (gRecordTrack < 24) gTrackType = 2;
    else if (gRecordTrack < 25) gTrackType = 3;
    else if (gRecordTrack < 29) {
        gTrackType = 4;
        gRecordCircuit = gRecordTrack - 25;
    } else gTrackType = 5;
    gViewMenuValid[0] = 1; gViewMenuVisible[0] = 1;
    gViewMenuValid[1] = 1; gViewMenuVisible[1] = 1;
}

/* 0x8008488c, size 0xe0 */
static void updatePakInfo(void)
{
    int pak;
    anon_ll_fileh_4c **player;
    for (pak = 0; pak < 4; pak++) {
        gNumPakPlayers[pak] = 0;
        for (player = gPlayerFiles.head; player; player = (*player)->ll.next) {
            if ((*(*player)->fileh)->pak == pak) gNumPakPlayers[pak]++;
        }
        if (gNumPakPlayers[pak] == 0) gNumPakPlayers[pak] = 0;
    }
}

/* 0x8008496c, size 0xf0 */
static void getScoreStr(float score, unsigned char *str, int unused)
{
    if (score == 0.0f) TB_Strcpy(str, "--:--.---");
    else {
        gDefTime = score;
        TB_Sprintf(str, "%.2d:%.2d.%.3d", (unsigned int)(score / 60.0f) % 100,
                   (unsigned int)score % 60, (unsigned int)(score * 1000.0f) % 1000);
    }
}

/* 0x80084a5c, size 0x418 */
static void initButtons(void)
{
    anon_c_rgba_4 buttonColor = { { 255, 255, 255, 255 } };
    anon_name_width_30 *tex;
    unsigned short texIdx;
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 23; buttonIdx++) {
        if (buttonInfo[buttonIdx].objIdx == -1) {
            int flags;
            memcpy(buttonInfo[buttonIdx].mat.uvs, unitmatrix, 36);
            flags = buttonInfo[buttonIdx].buttonType == 0 || buttonInfo[buttonIdx].buttonType == 1 || buttonInfo[buttonIdx].buttonType == 2 || buttonInfo[buttonIdx].buttonType == 7 || buttonInfo[buttonIdx].buttonType == 8 || buttonInfo[buttonIdx].buttonType == 9 || buttonInfo[buttonIdx].buttonType == 10 || buttonInfo[buttonIdx].buttonType == 11 ? 0x42000 : 0;
            buttonInfo[buttonIdx].objIdx = MBOX_NewObject(MBOX_FindObject_WORK(buttonNames[buttonInfo[buttonIdx].buttonType], 0, gNumModels - 1, 1), &buttonInfo[buttonIdx].mat, -1, flags);
            ZOID_SetColor(buttonInfo[buttonIdx].objIdx, &buttonColor);
            MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 1, 15);
            MBOX_ShowObject(buttonInfo[buttonIdx].objIdx, 0, 15);
        }
    }
    tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[17].objIdx, tex, -1);
    memcpy(buttonInfo[17].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[17].mat.uvs);
    buttonInfo[17].mat.uvs[3][0] = 0.0f;
    buttonInfo[17].mat.uvs[3][1] = 115.0f;
    buttonInfo[17].mat.uvs[3][2] = 200.0f;
    buttonInfo[17].mat.uvs[3][0] = 0.0f;
    buttonInfo[17].mat.uvs[3][1] = 115.0f;
    buttonInfo[17].mat.uvs[3][2] = 200.0f;
    tex = MBOX_FindTexture_WORK("CURSOR", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[18].objIdx, tex, -1);
    tex = MBOX_FindTexture_WORK("A1", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[19].objIdx, tex, -1);
    memcpy(buttonInfo[19].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[19].mat.uvs);
    buttonInfo[19].mat.uvs[3][0] = 75.1f;
    buttonInfo[19].mat.uvs[3][1] = 59.2f;
    buttonInfo[19].mat.uvs[3][2] = 100.0f;
    ScaleUV(buttonInfo[19].mat.uvs, buttonInfo[19].mat.uvs, 0.5f);
    memcpy(buttonInfo[20].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[20].mat.uvs);
    WRollUV(3.1415927f, buttonInfo[20].mat.uvs);
    buttonInfo[20].mat.uvs[3][0] = 87.1f;
    buttonInfo[20].mat.uvs[3][1] = 59.2f;
    buttonInfo[20].mat.uvs[3][2] = 100.0f;
    tex = MBOX_FindTexture_WORK("B1", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[21].objIdx, tex, -1);
    memcpy(buttonInfo[21].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[21].mat.uvs);
    buttonInfo[21].mat.uvs[3][0] = -75.1f;
    buttonInfo[21].mat.uvs[3][1] = 59.2f;
    buttonInfo[21].mat.uvs[3][2] = 100.0f;
    buttonInfo[21].mat.uvs[3][0] = -75.1f;
    buttonInfo[21].mat.uvs[3][1] = 59.2f;
    buttonInfo[21].mat.uvs[3][2] = 100.0f;
    ScaleUV(buttonInfo[21].mat.uvs, buttonInfo[21].mat.uvs, 0.5f);
    memcpy(buttonInfo[22].mat.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[22].mat.uvs);
    buttonInfo[22].mat.uvs[3][0] = -87.1f;
    buttonInfo[22].mat.uvs[3][1] = 59.2f;
    buttonInfo[22].mat.uvs[3][2] = 100.0f;
    buttonInfo[22].mat.uvs[3][0] = -87.1f;
    buttonInfo[22].mat.uvs[3][1] = 59.2f;
    buttonInfo[22].mat.uvs[3][2] = 100.0f;
    buttonsInited = 1;
    buttonsJustInited = 1;
    ZeroCameras();
}

/* 0x80084e74, size 0x74 */
static void uninitButtons(void)
{
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 23; buttonIdx++) {
        if (buttonInfo[buttonIdx].objIdx != -1) {
            ZOID_RemoveObject(buttonInfo[buttonIdx].objIdx);
            buttonInfo[buttonIdx].objIdx = -1;
        }
    }
    buttonsInited = 0;
}

/* 0x80084ee8, size 0x50 */
static void updateButtons(void)
{
    if (buttonsInited) {
        switch (gRecordState) {
        case kRecordStateSelect: updateSelectButtons(); break;
        case kRecordStateView: updateViewButtons(); break;
        }
    }
}

/* 0x80084f38, size 0xa60 */
static void updateSelectButtons(void)
{
    int buttonIdx, i;
        float prevPos[3];
        signed char clearScrollSpeed = 1;
        int gSelectMenuCount;
        int whichItem;
        float buttonY, selectedPos;
        anon_name_width_30 *tex;
        unsigned short texIdx;
        anon_c_rgba_4 buttonColor = { { 255, 255, 255, 255 } };
    
        if (gDeleteConfirm == 1 || gDeleteNotify == 1) {
            for (buttonIdx = 0; buttonIdx < 23; buttonIdx++) {
                if (buttonIdx == 17 || buttonIdx == 19 || buttonIdx == 21 || buttonIdx == 22)
                    MBOX_ShowObject(buttonInfo[buttonIdx].objIdx, 0, 15);
                else MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 1, 15);
            }
        } else {
            for (buttonIdx = 0; buttonIdx < 23; buttonIdx++) {
                if ((buttonIdx > 11 && buttonIdx < 13) || buttonIdx == 15 || buttonIdx == 16)
                    MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 1, 15);
                else MBOX_ShowObject(buttonInfo[buttonIdx].objIdx, 0, 15);
            }
            gSelectMenuCount = 0;
            whichItem = 0;
        for (i = 0; i < 11; i++) {
            if (gSelectMenuVisible[i]) {
                gSelectMenuCount++;
                if (i < gSelectMenuSelect) whichItem++;
            }
        }
        if (gButtonScale * ((gSelectMenuCount - 1) * gButtonSpace) > maxY - minY)
            selectedPos = maxY - whichItem * (maxY - minY) / (gSelectMenuCount - 1);
        else selectedPos = maxY - gButtonScale * (whichItem * gButtonSpace);
        memcpy(prevPos, buttonInfo[18].mat.uvs[3], 12);
        buttonInfo[18].angle += 6.2831855f * gFrameSeconds;
        if (buttonInfo[18].angle > 6.2831855f) buttonInfo[18].angle -= 6.2831855f;
        memcpy(buttonInfo[18].mat.uvs, unitmatrix, 36);
        buttonInfo[18].mat.uvs[3][0] = buttonX * cursorZ / buttonZ + -165.0f * gButtonScale;
        buttonInfo[18].mat.uvs[3][1] = selectedPos * cursorZ / buttonZ;
        buttonInfo[18].mat.uvs[3][2] = cursorZ;
        if (!buttonsJustInited) {
            if (gButtonScale * (fabs(buttonInfo[18].mat.uvs[3][1] - prevPos[1]) * 60.0) / 10.0 > cursorSpeed) {
                cursorSpeed = gButtonScale * (fabs(buttonInfo[18].mat.uvs[3][1] - prevPos[1]) * 60.0) / 10.0;
                cursorSpeed = cursorSpeed < 540.0f * gButtonScale ? cursorSpeed : 540.0f * gButtonScale;
                cursorSpeed = cursorSpeed > 180.0f * gButtonScale ? cursorSpeed : 180.0f * gButtonScale;
            }
            if (buttonInfo[18].mat.uvs[3][1] > prevPos[1] + cursorSpeed * gFrameSeconds)
                buttonInfo[18].mat.uvs[3][1] = prevPos[1] + cursorSpeed * gFrameSeconds;
            else if (buttonInfo[18].mat.uvs[3][1] < prevPos[1] - cursorSpeed * gFrameSeconds)
                buttonInfo[18].mat.uvs[3][1] = prevPos[1] - cursorSpeed * gFrameSeconds;
            else cursorSpeed = 180.0f * gButtonScale;
        }
        WRollUV(buttonInfo[18].angle, buttonInfo[18].mat.uvs);
        WPitchUV(1.5707964f, buttonInfo[18].mat.uvs);
        ScaleUV(buttonInfo[18].mat.uvs, buttonInfo[18].mat.uvs, 0.4f * gButtonScale);
        buttonY = selectedPos + gButtonScale * (whichItem * gButtonSpace);
        for (buttonIdx = 0; buttonIdx < 11; buttonIdx++) {
            if (buttonIdx == gSelectMenuSelect && !gSelectMenuClear) {
                if (buttonInfo[buttonIdx].angle < 3.1415927f) buttonInfo[buttonIdx].angle += 12.566371f * gFrameSeconds;
                if (buttonInfo[buttonIdx].angle > 3.1415927f) buttonInfo[buttonIdx].angle = 3.1415927f;
                if (buttonsJustInited) buttonInfo[buttonIdx].angle = 3.1415927f;
            } else {
                if (buttonInfo[buttonIdx].angle > 0.0f) buttonInfo[buttonIdx].angle -= 12.566371f * gFrameSeconds;
                if (buttonInfo[buttonIdx].angle < 0.0f) buttonInfo[buttonIdx].angle = 0.0f;
                if (buttonsJustInited) buttonInfo[buttonIdx].angle = 0.0f;
            }
            memcpy(prevPos, buttonInfo[buttonIdx].mat.uvs[3], 12);
            if (buttonInfo[buttonIdx].buttonType == 11) buttonInfo[buttonIdx].mat.uvs[3][0] = buttonX;
    else if (buttonInfo[buttonIdx].buttonType == 9) buttonInfo[buttonIdx].mat.uvs[3][0] = buttonX + -80.0f * gButtonScale;
    else buttonInfo[buttonIdx].mat.uvs[3][0] = buttonX + -120.0f * gButtonScale;
            buttonInfo[buttonIdx].mat.uvs[3][1] = buttonY;
            buttonInfo[buttonIdx].mat.uvs[3][2] = buttonZ;
            if (!buttonsJustInited) {
                if (fabs(buttonInfo[buttonIdx].mat.uvs[3][1] - prevPos[1]) * 60.0 / 10.0 > scrollSpeed) {
                    scrollSpeed = fabs(buttonInfo[buttonIdx].mat.uvs[3][1] - prevPos[1]) * 60.0 / 10.0;
                    scrollSpeed = scrollSpeed < 540.0f * gButtonScale ? scrollSpeed : 540.0f * gButtonScale;
                    scrollSpeed = scrollSpeed > 180.0f * gButtonScale ? scrollSpeed : 180.0f * gButtonScale;
                }
                if (buttonInfo[buttonIdx].mat.uvs[3][1] > prevPos[1] + scrollSpeed * gFrameSeconds) {
                    buttonInfo[buttonIdx].mat.uvs[3][1] = prevPos[1] + scrollSpeed * gFrameSeconds;
                    clearScrollSpeed = 0;
                } else if (buttonInfo[buttonIdx].mat.uvs[3][1] < prevPos[1] - scrollSpeed * gFrameSeconds) {
                    buttonInfo[buttonIdx].mat.uvs[3][1] = prevPos[1] - scrollSpeed * gFrameSeconds;
                    clearScrollSpeed = 0;
                }
            }
            ScaleUV(unitmatrix, buttonInfo[buttonIdx].mat.uvs, gButtonScale);
            WPitchUV(buttonInfo[buttonIdx].angle - 1.5707964f, buttonInfo[buttonIdx].mat.uvs);
            if (buttonInfo[buttonIdx].angle > 1.5707964f) tex = MBOX_FindTexture_WORK("BUTTON_SELECT", &texIdx, 0, gNumModels - 1, 1);
            else tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels - 1, 1);
            ZOID_SetTexture(buttonInfo[buttonIdx].objIdx, tex, -1);
            if (buttonInfo[buttonIdx].mat.uvs[3][1] > maxY + gButtonSpace * gButtonScale || buttonInfo[buttonIdx].mat.uvs[3][1] < minY - gButtonSpace * gButtonScale) buttonInfo[buttonIdx].alpha = 0;
            else if (buttonInfo[buttonIdx].mat.uvs[3][1] > maxY) buttonInfo[buttonIdx].alpha = 80;
            else if (buttonInfo[buttonIdx].mat.uvs[3][1] < minY) buttonInfo[buttonIdx].alpha = 80;
            else buttonInfo[buttonIdx].alpha = 255;
            if (!gSelectMenuVisible[buttonIdx]) buttonInfo[buttonIdx].alpha = 0;
            buttonColor.c[3] = buttonInfo[buttonIdx].alpha;
            ZOID_SetColor(buttonInfo[buttonIdx].objIdx, &buttonColor);
            if (!gSelectMenuVisible[buttonIdx] || !buttonInfo[buttonIdx].alpha) MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 1, 15);
            else MBOX_ShowObject(buttonInfo[buttonIdx].objIdx, 0, 15);
    if (buttonIdx == 1) {
    
    buttonInfo[14].mat.uvs[3][0] = buttonX + 160.0f * gButtonScale;
    buttonInfo[14].mat.uvs[3][1] = buttonInfo[buttonIdx].mat.uvs[3][1];
    buttonInfo[14].mat.uvs[3][2] = buttonInfo[buttonIdx].mat.uvs[3][2];
    buttonInfo[14].angle = buttonInfo[buttonIdx].angle;
    
    ScaleUV(unitmatrix, buttonInfo[14].mat.uvs, gButtonScale);
    WPitchUV(buttonInfo[14].angle - 1.5707964f, buttonInfo[14].mat.uvs);
    ZOID_SetTexture(buttonInfo[14].objIdx, tex, -1);
    
    buttonInfo[14].alpha = buttonInfo[buttonIdx].alpha;
    ZOID_SetColor(buttonInfo[14].objIdx, &buttonColor);
    if (!gSelectMenuVisible[buttonIdx] || !buttonInfo[buttonIdx].alpha) MBOX_HideObject(buttonInfo[14].objIdx, 1, 15);
    else MBOX_ShowObject(buttonInfo[14].objIdx, 0, 15);
    } else if (gSelectMenuSelect >= 3 && gSelectMenuSelect < 11) {
    if (buttonIdx == gSelectMenuSelect) {
    
    buttonInfo[13].mat.uvs[3][0] = buttonX + 120.0f * gButtonScale;
    buttonInfo[13].mat.uvs[3][1] = buttonInfo[buttonIdx].mat.uvs[3][1];
    buttonInfo[13].mat.uvs[3][2] = buttonInfo[buttonIdx].mat.uvs[3][2];
    
    if (gSelectMenuClear) {
    if (buttonInfo[13].angle < 3.1415927f) buttonInfo[13].angle += 12.566371f * gFrameSeconds;
    if (buttonInfo[13].angle > 3.1415927f) buttonInfo[13].angle = 3.1415927f;
    if (buttonsJustInited) buttonInfo[13].angle = 3.1415927f;
    } else {
    if (buttonInfo[13].angle > 0.0f) buttonInfo[13].angle -= 12.566371f * gFrameSeconds;
    if (buttonInfo[13].angle < 0.0f) buttonInfo[13].angle = 0.0f;
    if (buttonsJustInited) buttonInfo[13].angle = 0.0f;
    }
    
    ScaleUV(unitmatrix, buttonInfo[13].mat.uvs, gButtonScale);
    WPitchUV(buttonInfo[13].angle - 1.5707964f, buttonInfo[13].mat.uvs);
    
    if (buttonInfo[13].angle > 1.5707964f) tex = MBOX_FindTexture_WORK("BUTTON_SELECT", &texIdx, 0, gNumModels - 1, 1);
    else tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[13].objIdx, tex, -1);
    
    buttonInfo[13].alpha = buttonInfo[buttonIdx].alpha;
    ZOID_SetColor(buttonInfo[13].objIdx, &buttonColor);
    if (!gSelectMenuVisible[buttonIdx] || !buttonInfo[13].alpha) MBOX_HideObject(buttonInfo[13].objIdx, 1, 15);
    else MBOX_ShowObject(buttonInfo[13].objIdx, 0, 15);
    }
    } else {
    buttonInfo[13].angle = 0.0f;
    MBOX_HideObject(buttonInfo[13].objIdx, 1, 15);
    }
            if (gSelectMenuVisible[buttonIdx] == 1) buttonY -= gButtonSpace * gButtonScale;
        }
        for (; buttonIdx < 13; buttonIdx++) MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 1, 15);
        if (clearScrollSpeed == 1) scrollSpeed = 0.0f;
        buttonsJustInited = 0;
        }
}

/* 0x80085998, size 0x8b4 */
static void updateViewButtons(void)
{
    int buttonIdx, i;
    float prevPos[3];
    signed char clearScrollSpeed = 1;
    int gViewMenuCount = 0;
    int whichItem = 0;
    float buttonY, selectedPos;
    anon_name_width_30 *tex;
    unsigned short texIdx;
    anon_c_rgba_4 buttonColor = { { 255, 255, 255, 255 } };
    for (i = 0; i < 2; i++) {
        if (gViewMenuVisible[i]) {
            gViewMenuCount++;
            if (i < gViewMenuSelect) whichItem++;
        }
    }
    if (gButtonScale * ((gViewMenuCount - 1) * gButtonSpace) > maxY - minY)
        selectedPos = maxY - whichItem * (maxY - minY) / (gViewMenuCount - 1);
    else selectedPos = maxY - gButtonScale * (whichItem * gButtonSpace);
    memcpy(prevPos, buttonInfo[18].mat.uvs[3], 12);
    buttonInfo[18].angle += 6.2831855f * gFrameSeconds;
    if (buttonInfo[18].angle > 6.2831855f) buttonInfo[18].angle -= 6.2831855f;
    memcpy(buttonInfo[18].mat.uvs, unitmatrix, 36);
    buttonInfo[18].mat.uvs[3][0] = buttonX * cursorZ / buttonZ + -165.0f * gButtonScale;
    buttonInfo[18].mat.uvs[3][1] = selectedPos * cursorZ / buttonZ;
    buttonInfo[18].mat.uvs[3][2] = cursorZ;
    if (!buttonsJustInited) {
        if (gButtonScale * (fabs(buttonInfo[18].mat.uvs[3][1] - prevPos[1]) * 60.0) / 10.0 > cursorSpeed) {
            cursorSpeed = gButtonScale * (fabs(buttonInfo[18].mat.uvs[3][1] - prevPos[1]) * 60.0) / 10.0;
            cursorSpeed = cursorSpeed < 540.0f * gButtonScale ? cursorSpeed : 540.0f * gButtonScale;
            cursorSpeed = cursorSpeed > 180.0f * gButtonScale ? cursorSpeed : 180.0f * gButtonScale;
        }
        if (buttonInfo[18].mat.uvs[3][1] > prevPos[1] + cursorSpeed * gFrameSeconds)
            buttonInfo[18].mat.uvs[3][1] = prevPos[1] + cursorSpeed * gFrameSeconds;
        else if (buttonInfo[18].mat.uvs[3][1] < prevPos[1] - cursorSpeed * gFrameSeconds)
            buttonInfo[18].mat.uvs[3][1] = prevPos[1] - cursorSpeed * gFrameSeconds;
        else cursorSpeed = 180.0f * gButtonScale;
    }
    WRollUV(buttonInfo[18].angle, buttonInfo[18].mat.uvs);
    WPitchUV(1.5707964f, buttonInfo[18].mat.uvs);
    ScaleUV(buttonInfo[18].mat.uvs, buttonInfo[18].mat.uvs, 0.4f * gButtonScale);
    buttonY = selectedPos + gButtonScale * (whichItem * gButtonSpace);
    for (buttonIdx = 0; buttonIdx < 11; buttonIdx++) MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 1, 15);
    for (buttonIdx = 11; buttonIdx < 13; buttonIdx++) {
        if (buttonIdx - 11 == gViewMenuSelect) {
            if (buttonInfo[buttonIdx].angle < 3.1415927f) buttonInfo[buttonIdx].angle += 12.566371f * gFrameSeconds;
            if (buttonInfo[buttonIdx].angle > 3.1415927f) buttonInfo[buttonIdx].angle = 3.1415927f;
            if (buttonsJustInited) buttonInfo[buttonIdx].angle = 3.1415927f;
        } else {
            if (buttonInfo[buttonIdx].angle > 0.0f) buttonInfo[buttonIdx].angle -= 12.566371f * gFrameSeconds;
            if (buttonInfo[buttonIdx].angle < 0.0f) buttonInfo[buttonIdx].angle = 0.0f;
            if (buttonsJustInited) buttonInfo[buttonIdx].angle = 0.0f;
        }
        memcpy(prevPos, buttonInfo[buttonIdx].mat.uvs[3], 12);
        buttonInfo[buttonIdx].mat.uvs[3][0] = buttonX + -140.0f * gButtonScale;
        buttonInfo[buttonIdx].mat.uvs[3][1] = buttonY;
        buttonInfo[buttonIdx].mat.uvs[3][2] = buttonZ;
        if (!buttonsJustInited) {
            if (fabs(buttonInfo[buttonIdx].mat.uvs[3][1] - prevPos[1]) * 60.0 / 10.0 > scrollSpeed) {
                scrollSpeed = fabs(buttonInfo[buttonIdx].mat.uvs[3][1] - prevPos[1]) * 60.0 / 10.0;
                scrollSpeed = scrollSpeed < 540.0f * gButtonScale ? scrollSpeed : 540.0f * gButtonScale;
                scrollSpeed = scrollSpeed > 180.0f * gButtonScale ? scrollSpeed : 180.0f * gButtonScale;
            }
            if (buttonInfo[buttonIdx].mat.uvs[3][1] > prevPos[1] + scrollSpeed * gFrameSeconds) {
                buttonInfo[buttonIdx].mat.uvs[3][1] = prevPos[1] + scrollSpeed * gFrameSeconds;
                clearScrollSpeed = 0;
            } else if (buttonInfo[buttonIdx].mat.uvs[3][1] < prevPos[1] - scrollSpeed * gFrameSeconds) {
                buttonInfo[buttonIdx].mat.uvs[3][1] = prevPos[1] - scrollSpeed * gFrameSeconds;
                clearScrollSpeed = 0;
            }
        }
        ScaleUV(unitmatrix, buttonInfo[buttonIdx].mat.uvs, gButtonScale);
        WPitchUV(buttonInfo[buttonIdx].angle - 1.5707964f, buttonInfo[buttonIdx].mat.uvs);
        if (buttonInfo[buttonIdx].angle > 1.5707964f) tex = MBOX_FindTexture_WORK("BUTTON_SELECT", &texIdx, 0, gNumModels - 1, 1);
        else tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels - 1, 1);
        ZOID_SetTexture(buttonInfo[buttonIdx].objIdx, tex, -1);
        if (buttonInfo[buttonIdx].mat.uvs[3][1] > maxY + gButtonSpace * gButtonScale || buttonInfo[buttonIdx].mat.uvs[3][1] < minY - gButtonSpace * gButtonScale) buttonInfo[buttonIdx].alpha = 0;
        else if (buttonInfo[buttonIdx].mat.uvs[3][1] > maxY) buttonInfo[buttonIdx].alpha = 80;
        else if (buttonInfo[buttonIdx].mat.uvs[3][1] < minY) buttonInfo[buttonIdx].alpha = 80;
        else buttonInfo[buttonIdx].alpha = 255;
        if (!gViewMenuVisible[buttonIdx - 11]) buttonInfo[buttonIdx].alpha = 0;
        buttonColor.c[3] = buttonInfo[buttonIdx].alpha;
        ZOID_SetColor(buttonInfo[buttonIdx].objIdx, &buttonColor);
        if (!gViewMenuVisible[buttonIdx - 11] || !buttonInfo[buttonIdx].alpha) MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 1, 15);
        else MBOX_ShowObject(buttonInfo[buttonIdx].objIdx, 0, 15);
        if (buttonIdx - 11 == 0) {
    buttonInfo[15].mat.uvs[3][0] = buttonX + 100.0f * gButtonScale;
    buttonInfo[15].mat.uvs[3][1] = buttonInfo[buttonIdx].mat.uvs[3][1];
    buttonInfo[15].mat.uvs[3][2] = buttonInfo[buttonIdx].mat.uvs[3][2];
    buttonInfo[15].angle = buttonInfo[buttonIdx].angle;
    ScaleUV(unitmatrix, buttonInfo[15].mat.uvs, gButtonScale);
    WPitchUV(buttonInfo[15].angle - 1.5707964f, buttonInfo[15].mat.uvs);
    ZOID_SetTexture(buttonInfo[15].objIdx, tex, -1);
    buttonInfo[15].alpha = buttonInfo[buttonIdx].alpha;
    ZOID_SetColor(buttonInfo[15].objIdx, &buttonColor);
    if (!gViewMenuVisible[buttonIdx] || !buttonInfo[buttonIdx].alpha) MBOX_HideObject(buttonInfo[15].objIdx, 1, 15);
    else MBOX_ShowObject(buttonInfo[15].objIdx, 0, 15);
    }
    else if (buttonIdx - 11 == 1) {
    buttonInfo[16].mat.uvs[3][0] = buttonX + 100.0f * gButtonScale;
    buttonInfo[16].mat.uvs[3][1] = buttonInfo[buttonIdx].mat.uvs[3][1];
    buttonInfo[16].mat.uvs[3][2] = buttonInfo[buttonIdx].mat.uvs[3][2];
    buttonInfo[16].angle = buttonInfo[buttonIdx].angle;
    ScaleUV(unitmatrix, buttonInfo[16].mat.uvs, gButtonScale);
    WPitchUV(buttonInfo[16].angle - 1.5707964f, buttonInfo[16].mat.uvs);
    ZOID_SetTexture(buttonInfo[16].objIdx, tex, -1);
    buttonInfo[16].alpha = buttonInfo[buttonIdx].alpha;
    ZOID_SetColor(buttonInfo[16].objIdx, &buttonColor);
    if (!gViewMenuVisible[buttonIdx - 11] || !buttonInfo[buttonIdx].alpha) MBOX_HideObject(buttonInfo[16].objIdx, 1, 15);
    else MBOX_ShowObject(buttonInfo[16].objIdx, 0, 15);
    }
    
        if (gViewMenuVisible[buttonIdx - 11] == 1) buttonY -= gButtonSpace * gButtonScale;
    }
    for (; buttonIdx < 23; buttonIdx++) {
        if (buttonIdx < 15) MBOX_HideObject(buttonInfo[buttonIdx].objIdx, 1, 15);
        else MBOX_ShowObject(buttonInfo[buttonIdx].objIdx, 0, 15);
    }
    if (clearScrollSpeed == 1) scrollSpeed = 0.0f;
    buttonsJustInited = 0;
    MBOX_HideObject(buttonInfo[19].objIdx, 1, 15);
    MBOX_HideObject(buttonInfo[20].objIdx, 1, 15);
}

