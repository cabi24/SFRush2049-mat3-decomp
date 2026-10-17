/* recordscr.c - Record screen; partial reconstruction, 19 target-placed exact bodies.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\recordscr.c; assembly: build/2049LE/asm/GameSource/src/recordscr.s */
#include "rush/rush_protos.h"

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

static const anon_texname_dulx_24 mblit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animText, 0 };
static char *buttonNames[14] = {
    "BUTTON_S2G1", "BUTTON_S2G1", "BUTTON_S2G1", "CURSORG1", "AB_BUTTONG1", "ARROW_BGG1",
    "ARROW_FGG1", "BUTTON_S1G1", "BUTTON_S3G1", "BUTTON_S4G1", "BUTTON_S0G1", "BUTTON_S8G1",
    "BAR_BGG1", "BAR_FGG1"
};
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
    int p; /* r30 */
    int i; /* r30 */
    /* TODO */
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
    int i; /* r23 */
    int x; /* r19 */
    int y; /* r17 */
    int cx; /* r18 */
    unsigned char *str; /* r22 */
    unsigned char str1[64]; /* stack */
    signed short screenPos[2]; /* stack */
    anon_ll_fileh_4c **player; /* r3 */
    int playerIdx; /* r6 */
    /* TODO */
}

/* 0x80081c78, size 0x568 */
static void drawView(void)
{
    int i; /* r25 */
    int x; /* r26 */
    int y; /* r25 */
    int cx; /* r26 */
    unsigned char *str; /* r6 */
    unsigned char str1[64]; /* stack */
    signed short screenPos[2]; /* stack */
    /* TODO */
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
    y = 92 + TB_GetFontLineHeight();
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
    signed short statIdx; /* r31 */
    signed short i; /* r17 */
    signed short x; /* r0 */
    signed short y; /* r30 */
    unsigned int totalMiles; /* r29 */
    unsigned int totalStuntPoints; /* r28 */
    unsigned short totalBattlePoints; /* r17 */
    unsigned short silverRaceCoins; /* r18 */
    unsigned short goldRaceCoins; /* r19 */
    unsigned short silverStuntCoins; /* r23 */
    unsigned short goldStuntCoins; /* r31 */
    unsigned char *str; /* r16 */
    unsigned char str1[40]; /* stack */
    anon_checksum_scores_60 *stats; /* r16 */
    /* TODO */
}

/* 0x80083488, size 0x9b8 */
static void drawUnlockedTracks(void)
{
    anon_ll_fileh_4c **tempPlayer; /* r17 */
    int tempNumPlayers; /* r19 */
    signed short i; /* r18 */
    signed short x; /* r30 */
    signed short y; /* r26 */
    signed short w; /* r19 */
    signed short b; /* r28 */
    int noRaceTracksUnlocked; /* r25 */
    int noStuntTracksUnlocked; /* r22 */
    int noBattleTracksUnlocked; /* r23 */
    int noCircuitsUnlocked; /* r24 */
    int noneUnlocked; /* r20 */
    unsigned char *str; /* r17 */
    /* TODO */
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
    anon_ll_fileh_4c **tempPlayer; /* r24 */
    int tempNumPlayers; /* r25 */
    signed short i; /* r24 */
    signed short y; /* r30 */
    signed short b; /* r29 */
    int noneUnlocked; /* r25 */
    int noPartsUnlocked; /* r28 */
    unsigned char *str; /* r24 */
    /* TODO */
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
    /* TODO: original body not recovered. */
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
    anon_c_rgba_4 buttonColor; /* stack */
    anon_name_width_30 *tex; /* r3 */
    unsigned short texIdx; /* stack */
    int buttonIdx; /* r30 */
    /* TODO */
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
    int buttonIdx; /* r24 */
    int i; /* r5 */
    float prevPos[3]; /* stack */
    int gSelectMenuCount; /* r6 */
    int whichItem; /* r24 */
    float buttonY; /* f30 */
    float selectedPos; /* f30 */
    signed char clearScrollSpeed; /* r25 */
    anon_name_width_30 *tex; /* r23 */
    unsigned short texIdx; /* stack */
    anon_c_rgba_4 buttonColor; /* stack */
    /* TODO */
}

/* 0x80085998, size 0x8b4 */
static void updateViewButtons(void)
{
    int buttonIdx; /* r25 */
    int i; /* r5 */
    float prevPos[3]; /* stack */
    int gViewMenuCount; /* r6 */
    int whichItem; /* r25 */
    float buttonY; /* f29 */
    float selectedPos; /* f29 */
    signed char clearScrollSpeed; /* r24 */
    anon_name_width_30 *tex; /* r23 */
    unsigned short texIdx; /* stack */
    anon_c_rgba_4 buttonColor; /* stack */
    /* TODO */
}

