/* gameoverscr.c - post-race menu and score screens. In progress, NonMatching.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\gameoverscr.c */
#include "rush/rush_protos.h"

#define ABS_VALUE(value) ((value) >= 0 ? (value) : -(value))

static void initScreen(void);
static void uninitScreen(void);
static void setGameOverState(enum_kGameOverStateCount newState);
static int validOption(enum_kGameOverMainMenuCount option);
static void controlMain(void);
static void controlScore(void);
static void drawMain(void);
static void drawRaceStandings(void);
static void drawStuntStandings(void);
static void drawBattleStandings(void);
static void drawRaceScore(void);
static void drawStuntScore(void);
static void drawBattleScore(void);
static int animText(void);
static void drawRect(int x, int y, int w, int h, unsigned char *color);

static const anon_texname_dulx_24 multiblit[1] = {
    (void *)-1, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animText, 0
};
static unsigned char titlePos[2] = { 160, 10 };
static enum_kGameOverStateCount gGameOverState;
static enum_kGameOverMainMenuCount gMainSelect;
static _tBlit_2 *blit;
static signed char inited;

void updateGameOverScreen(void)
{
    if (!inited) initScreen();
    BlinkResurrectingCars();
    if (gSoftReset) {
        AudKillSong();
        UnloadTrack(gTrackNum);
        RemoveEnvirons();
        gNumWindows = 1;
        ZOID_SplitScreen(1);
        RemoveMultiplayerHUD();
        ClearScreen(1);
        gSkipToGameState = 1;
        uninitScreen();
        setGameState(4);
    } else {
        switch (gGameOverState) {
        case kGameOverStateMain: controlMain(); break;
        case kGameOverStateScore: controlScore(); break;
        }
    }
}

static void initScreen(void)
{
    setGameOverState(kGameOverStateMain);
    gMainSelect = 0;
    while (!validOption(gMainSelect)) {
        if (++gMainSelect >= kGameOverMainMenuCount) {
            gMainSelect = 0;
            break;
        }
    }
    blit = (void *)NewMultiBlit(0, 0, multiblit, 1);
    inited = 1;
}

static void uninitScreen(void)
{
    setGameOverState(kGameOverStateNone);
    RemovePanes(1);
    if (blit) {
        RemoveBlit((void *)blit);
        blit = 0;
    }
    inited = 0;
}

static void setGameOverState(enum_kGameOverStateCount newState)
{
    int i, x, y, max, width, menuSize;
    if (gGameOverState == newState) return;
    switch (gGameOverState) {
    case kGameOverStateMain: RemovePanes(0); break;
    }
    gGameOverState = newState;
    switch (newState) {
    case kGameOverStateMain:
        HAL_SetFont(13);
        width = (int)HAL_GetTextWidth(gText.single[67], -1);
        x = titlePos[0];
        y = titlePos[1];
        InitPane(x - width / 2 - 8, y - 4, x + width / 2 + 8,
                 y + TB_GetFontLineHeight() + 4, 176, 0, 0, 0);
        HAL_SetFont(10);
        max = 0;
        menuSize = 0;
        for (i = 0; i < kGameOverMainMenuCount; i++) {
            if (validOption(i) == 1) {
                width = HAL_GetTextWidth(gText.taba[gText.tab[46] + i], -1);
                if (max < width) max = width;
                menuSize++;
            }
        }
        width = max / 2;
        InitPane(x - width - 8, 116, x + width + 8,
                 menuSize * TB_GetFontLineHeight() + 124, 176, 0, 0, 0);
        break;
    }
}

static int validOption(enum_kGameOverMainMenuCount option)
{
    int valid = 1;
    if (gRace == kCircuit) {
        if ((option == kGameOverMainMenuRaceAgain || option == kGameOverMainMenuNewMode ||
             option == kGameOverMainMenuNewTrack) ||
            option == kGameOverMainMenuNewCar) {
            valid = 0;
        }
        if (option == kGameOverMainMenuNextRace &&
            !gGameCar[gCircuitPlayer].place_locked) valid = 0;
    } else if (gRace == kPractice) {
        if ((option == kGameOverMainMenuScore || option == kGameOverMainMenuRaceAgain ||
             option == kGameOverMainMenuNewMode) ||
            option == kGameOverMainMenuNextRace) {
            valid = 0;
        }
    } else if (gRace == kGhost) {
        if (option == kGameOverMainMenuNewMode || option == kGameOverMainMenuNextRace)
            valid = 0;
    } else {
        if (option == kGameOverMainMenuNextRace || option == kGameOverMainMenuNewMode)
            valid = 0;
    }
    return valid;
}

static void controlMain(void)
{
    if (gActiveEdges & 3) {
        PlayStartABSound(gActiveEdges);
        switch (gMainSelect) {
        case kGameOverMainMenuScore:
            setGameOverState(kGameOverStateScore);
            break;
        case kGameOverMainMenuRaceAgain:
            if (gRace == kGhost) {
                AudKillSong();
                UnloadTrack(gTrackNum);
                RemoveEnvirons();
                gNumWindows = 1;
                ZOID_SplitScreen(1);
                RemoveMultiplayerHUD();
                uninitScreen();
                gSkipToGameState = 0x40000;
                setGameState(4);
            } else {
                uninitScreen();
                gRestartGame = 1;
                AudStopAllEffects();
                AbortGame();
            }
            break;
        case kGameOverMainMenuNewMode:
        case kGameOverMainMenuNewTrack:
        case kGameOverMainMenuNewCar:
        case kGameOverMainMenuNextRace:
        case kGameOverMainMenuQuit:
            AudKillSong();
            UnloadTrack(gTrackNum);
            RemoveEnvirons();
            gNumWindows = 1;
            ZOID_SplitScreen(1);
            RemoveMultiplayerHUD();
            if (gMainSelect == kGameOverMainMenuNewMode) gSkipToGameState = 0x40;
            else if (gMainSelect == kGameOverMainMenuNewTrack) gSkipToGameState = 0x80;
            else if (gMainSelect == kGameOverMainMenuNewCar) gSkipToGameState = 0x100;
            else if (gMainSelect == kGameOverMainMenuNextRace) gSkipToGameState = 0x4000;
            uninitScreen();
            setGameState(4);
            break;
        }
    } else if (gActiveAutoEdges & 0x400) {
        PlayUDSound(gActiveAutoEdges);
        do {
            if (--gMainSelect < 0) gMainSelect = kGameOverMainMenuCount - 1;
        } while (!validOption(gMainSelect));
    } else if (gActiveAutoEdges & 0x800) {
        PlayUDSound(gActiveAutoEdges);
        do {
            if (++gMainSelect > kGameOverMainMenuCount - 1) gMainSelect = 0;
        } while (!validOption(gMainSelect));
    }
}

static void controlScore(void)
{
    if (gActiveEdges & 7) {
        PlayStartABSound(4);
        setGameOverState(kGameOverStateMain);
    }
}

static void drawMain(void)
{
    int i;
    signed short x, y;
    unsigned char *str;
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 3);
    HAL_SetFont(13);
    TB_SetTextColor(1);
    str = gText.single[67];
    x = titlePos[0];
    y = titlePos[1];
    TB_DrawText(x, y, str);
    y = 120;
    HAL_SetFont(10);
    for (i = 0; i < kGameOverMainMenuCount; i++) {
        if (validOption(i)) {
            str = gText.taba[gText.tab[46] + i];
            if (i == kGameOverMainMenuScore && gRace != kStunt && gRace != kBattle)
                str = gText.single[0];
            TB_SetTextColor(i == gMainSelect ? 22 : 1);
            TB_DrawText(x, y, str);
            y += TB_GetFontLineHeight();
        }
    }
    TB_SetJust(0, 3);
    HAL_SetTextDepth(-1.0f);
    switch (gRace) {
    case kStunt: drawStuntStandings(); break;
    case kBattle: drawBattleStandings(); break;
    case kOneRace: case kGhost: case kCircuit: case kObstacle:
        drawRaceStandings(); break;
    }
}

static void drawRaceStandings(void)
{
    unsigned char *str;
    unsigned char str1[40];
    int rows, row, col;
    unsigned char c[4];
    int cols;

    rows = gNumPlayers + 1;
    cols = (gNumCars == 1 || (gRace == kCircuit && !gGameCar[gCircuitPlayer].place_locked)) ? 3 : 4;
    for (row = 0; row < rows; row++) {
        int bgY = 36 + row * 14;
        for (col = 0; col < cols; col++) {
            int x = 160 - cols * 60 / 2 + col * 60;
            if (row == 0 && col == 0) continue;
            if (col > 0 && row > 0) {
                c[0] = 0; c[1] = 0; c[2] = 0; c[3] = 192;
                switch (gRankedPlayer[row - 1]) {
                case 0: c[2] = 128; break;
                case 1: c[0] = 128; break;
                case 2: c[0] = 128; c[1] = 128; break;
                case 3: c[1] = 128; break;
                }
            } else {
                c[0] = 0; c[1] = 0; c[2] = 0; c[3] = 192;
            }
            drawRect(x, bgY, 60, 14, c);
        }
    }
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 1);
    HAL_SetFont(11);
    for (row = 0; row < rows; row++) {
        for (col = 0; col < cols; col++) {
            int x = 160 - cols * 60 / 2 + col * 60;
            int y = 36 + row * 14;
            if (row == 0 && col == 0) continue;
            if (col == 0) {
                HAL_SetFont(11);
                if (gRankedPlayer[row - 1] < gNumPlayers) {
                    TB_SetTextColor(playerTextColor[gRankedPlayer[row - 1]]);
                    if (gRace == kGhost) str = gGhostPlayer->name;
                    else str = (*gPlayers[gRankedPlayer[row - 1]].save)->name;
                } else {
                    TB_SetTextColor(4);
                    str = (unsigned char *)"DRONE";
                }
                TB_DrawEllipseText(x + 30, y + 7, 60, str);
            } else if (row == 0) {
                HAL_SetFont(11);
                TB_SetTextColor(1);
                if (col == 1) str = gText.single[219];
                else if (col == 2) str = gText.single[221];
                else if (col == 3) str = gText.single[22];
            } else {
                if (col == 1) {
                    if (gLap[gRankedPlayer[row - 1]] == gNumLaps) {
                        float time = gGameCar[gRankedPlayer[row - 1]].score;
                        HAL_SetFont(3);
                        TB_SetTextColor(1);
                        TB_Sprintf(str1, "%d:%02d.%03d", (unsigned int)(time / 60.0f),
                                   (unsigned int)time % 60, (unsigned int)(1000.0f * time) % 1000);
                        str = str1;
                    } else {
                        HAL_SetFont(11);
                        TB_SetTextColor(1);
                        str = gText.single[226];
                    }
                } else if (col == 2) {
                    if (gLap[gRankedPlayer[row - 1]] != 0) {
                        float time = gBestLapTime[gRankedPlayer[row - 1]];
                        HAL_SetFont(3);
                        TB_SetTextColor(1);
                        TB_Sprintf(str1, "%d:%02d.%03d", (unsigned int)(time / 60.0f),
                                   (unsigned int)time % 60, (unsigned int)(1000.0f * time) % 1000);
                        str = str1;
                    } else {
                        HAL_SetFont(11);
                        TB_SetTextColor(1);
                        str = gText.single[226];
                    }
                } else if (col == 3) {
                    HAL_SetFont(11);
                    TB_SetTextColor(1);
                    str = gText.taba[gText.tab[56] + gGameCar[model[gRankedPlayer[row - 1]].slot].place];
                }
            }
            if (col != 0) TB_DrawText(x + 30, y + 7, str);
        }
    }
    HAL_SetTextDepth(-1.0f);
    TB_SetJust(0, 3);
}



static void drawStuntStandings(void)
{
    unsigned char *str;
    unsigned char str1[40];
    int rows, row, col;
    unsigned char c[4];

    rows = gNumActiveCars + 1;
    for (row = 0; row < rows; row++) {
        int bgY = 36 + row * 14;
        for (col = 0; col < 3; col++) {
            if (row == 0 && col == 0) continue;
            if (col > 0 && row > 0) {
                c[0] = 0; c[1] = 0; c[2] = 0;
                c[3] = 192;
                switch (gRankedPlayer[row - 1]) {
                case 0: c[2] = 128; break;
                case 1: c[0] = 128; break;
                case 2: c[0] = 128; c[1] = 128; break;
                case 3: c[1] = 128; break;
                }
            } else {
                c[0] = 0; c[1] = 0; c[2] = 0;
                c[3] = 192;
            }
            drawRect(70 + col * 60, bgY, 60, 14, c);
        }
    }
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 1);
    HAL_SetFont(11);
    for (row = 0; row < rows; row++) {
        for (col = 0; col < 3; col++) {
            int x = 70 + col * 60;
            int y = 36 + row * 14;
            if (row == 0 && col == 0) continue;
            if (col == 0) {
                HAL_SetFont(11);
                if (gRankedPlayer[row - 1] < gNumPlayers) {
                    TB_SetTextColor(playerTextColor[gRankedPlayer[row - 1]]);
                    str = (*gPlayers[gRankedPlayer[row - 1]].save)->name;
                } else {
                    TB_SetTextColor(4);
                    str = (unsigned char *)"DRONE";
                }
                TB_DrawEllipseText(x + 30, y + 7, 60, str);
            } else if (row == 0) {
                HAL_SetFont(12);
                if (col == 1) {
                    TB_SetTextColor(1);
                    str = gText.single[140];
                } else if (col == 2) {
                    TB_SetTextColor(1);
                    str = gText.single[224];
                }
            } else {
                if (col == 1) {
                    TB_SetTextColor(1);
                    TB_Sprintf(str1, "%d", gStuntPlayer[gRankedPlayer[row - 1]].totalPoints);
                    str = str1;
                } else if (col == 2) {
                    TB_SetTextColor(1);
                    TB_Sprintf(str1, "%d", gStuntPlayer[gRankedPlayer[row - 1]].maxStuntPoints);
                    str = str1;
                }
            }
            if (col != 0) TB_DrawText(x + 30, y + 7, str);
            HAL_SetFont(11);
        }
    }
    HAL_SetTextDepth(-1.0f);
    TB_SetJust(0, 3);
}

static void drawBattleStandings(void)
{
    unsigned char *str;
    unsigned char str1[40];
    int rows, row, col;
    int playerIdx;
    unsigned char c[4];

    rows = gNumActiveCars + 1;
    for (row = 0; row < rows; row++) {
        int bgY = 36 + row * 14;
        for (col = 0; col < 3; col++) {
            if (row == 0 && col == 0) continue;
            if (col > 0 && row > 0) {
                c[0] = 0; c[1] = 0; c[2] = 0;
                c[3] = 192;
                switch (gCarTeam[gRankedPlayer[row - 1]]) {
                case 0: c[2] = 128; break;
                case 1: c[0] = 128; break;
                case 2: c[0] = 128; c[1] = 128; break;
                case 3: c[1] = 128; break;
                }
            } else {
                c[0] = 0; c[1] = 0; c[2] = 0;
                c[3] = 192;
            }
            drawRect(70 + col * 60, bgY, 60, 14, c);
        }
    }
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 1);
    HAL_SetFont(11);
    for (row = 0; row < rows; row++) {
        for (col = 0; col < 3; col++) {
            int x = 70 + col * 60;
            int y = 36 + row * 14;
            if (row == 0 && col == 0) continue;
            if (col == 0) {
                HAL_SetFont(11);
                if (gRankedPlayer[row - 1] < gNumPlayers) {
                    TB_SetTextColor(playerTextColor[gCarTeam[gRankedPlayer[row - 1]]]);
                    str = (*gPlayers[gRankedPlayer[row - 1]].save)->name;
                } else {
                    TB_SetTextColor(4);
                    str = (unsigned char *)"DRONE";
                }
                TB_DrawEllipseText(x + 30, y + 7, 60, str);
            } else if (row == 0) {
                if (col == 1) {
                    TB_SetTextColor(1);
                    str = gText.single[140];
                } else if (col == 2) {
                    TB_SetTextColor(1);
                    str = gText.single[12];
                }
            } else {
                int total = 0;
                if (col == 1) {
                    total = gGameCar[gPlayers[gRankedPlayer[row - 1]].slot].battle.Points;
                } else if (col == 2) {
                    for (playerIdx = 0; playerIdx < gNumPlayers; playerIdx++) {
                        total += ABS_VALUE(gBattlePoints[playerIdx][gRankedPlayer[row - 1]]);
                    }
                }
                TB_SetTextColor(1);
                TB_Sprintf(str1, "%d", total);
                str = str1;
            }
            if (col != 0) TB_DrawText(x + 30, y + 7, str);
        }
    }
    HAL_SetTextDepth(-1.0f);
    TB_SetJust(0, 3);
}

static void drawRaceScore(void)
{
    int y;
    unsigned char *str;
    unsigned char str1[40];
    int rows, cols, row, col;
    unsigned char c[4];

    rows = gNumLaps + 3;
    cols = gNumPlayers + 1;
    for (row = 0; row < rows; row++) {
        for (col = 0; col < cols; col++) {
            int x = 160 - cols * 60 / 2 + col * 60;
            int bgY = 112 - rows * 16 / 2 + row * 16;
            if (row == 0 && col == 0) continue;
            y = bgY;
            if (row > 2) {
                if (row == 3 && col == 0) {
                    c[0] = 0; c[1] = 0; c[2] = 0; c[3] = 192;
                    drawRect(x, bgY, cols * 60, 8, c);
                }
                y += 8;
            }
            if (col > 0 && row > 0) {
                c[0] = 0; c[1] = 0; c[2] = 0; c[3] = 192;
                switch (col - 1) {
                case 0: c[2] = 128; break;
                case 1: c[0] = 128; break;
                case 2: c[0] = 128; c[1] = 128; break;
                case 3: c[1] = 128; break;
                }
            } else {
                c[0] = 0; c[1] = 0; c[2] = 0; c[3] = 192;
            }
            drawRect(x, y, 60, 16, c);
        }
    }
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 1);
    HAL_SetFont(11);
    TB_DrawText(0, 0, (unsigned char *)" ");
    for (row = 0; row < rows; row++) {
        for (col = 0; col < cols; col++) {
            int x = 160 - cols * 60 / 2 + col * 60;
            int textY = 112 - rows * 16 / 2 + row * 16;
            if (row == 0 && col == 0) continue;
            y = textY;
            if (row > 2) y += 8;
            if (row == 0) {
                HAL_SetFont(11);
                TB_SetTextColor(playerTextColor[col - 1]);
                if (gRace == kGhost) str = gGhostPlayer->name;
                else str = (*gPlayers[col - 1].save)->name;
                TB_DrawEllipseText(x + 30, y + 8, 60, str);
            } else if (col == 0) {
                HAL_SetFont(11);
                if (row == 1) {
                    TB_SetTextColor(1);
                    str = gText.single[219];
                } else if (row == 2) {
                    TB_SetTextColor(1);
                    str = gText.single[221];
                } else {
                    TB_SetTextColor(1);
                    TB_Sprintf(str1, gText.single[216], row - 2);
                    str = str1;
                }
            } else {
                if (row == 1) {
                    if (gNumLaps == gLap[gPlayers[col - 1].slot]) {
                        float time = gGameCar[gPlayers[col - 1].slot].score;
                        HAL_SetFont(3);
                        TB_SetTextColor(1);
                        TB_Sprintf(str1, "%d:%02d.%03d", (unsigned int)(time / 60.0f),
                                   (unsigned int)time % 60, (unsigned int)(1000.0f * time) % 1000);
                        str = str1;
                    } else {
                        HAL_SetFont(11);
                        TB_SetTextColor(1);
                        str = gText.single[226];
                    }
                } else if (row == 2) {
                    if (gLap[gPlayers[col - 1].slot] != 0) {
                        float time = gBestLapTime[col - 1];
                        HAL_SetFont(3);
                        TB_SetTextColor(1);
                        TB_Sprintf(str1, "%d:%02d.%03d", (unsigned int)(time / 60.0f),
                                   (unsigned int)time % 60, (unsigned int)(1000.0f * time) % 1000);
                        str = str1;
                    } else {
                        HAL_SetFont(11);
                        TB_SetTextColor(1);
                        str = gText.single[226];
                    }
                } else {
                    if (gLap[gPlayers[col - 1].slot] > row - 3) {
                        float time = gLapTimes[col - 1][row - 3];
                        HAL_SetFont(3);
                        TB_SetTextColor(1);
                        TB_Sprintf(str1, "%d:%02d.%03d", (unsigned int)(time / 60.0f),
                                   (unsigned int)time % 60, (unsigned int)(1000.0f * time) % 1000);
                        str = str1;
                    } else {
                        HAL_SetFont(11);
                        TB_SetTextColor(1);
                        str = gText.single[226];
                    }
                }
            }
            if (row != 0) TB_DrawText(x + 30, y + 8, str);
        }
    }
    HAL_SetTextDepth(-1.0f);
    TB_SetJust(0, 3);
}
static void drawStuntScore(void)
{
    int y;
    unsigned char *str;
    unsigned char str1[40];
    int cols, row, col;
    unsigned char c[4];

    cols = gNumPlayers + 1;
    for (row = 0; row < 13; row++) {
        for (col = 0; col < cols; col++) {
            int x = 160 - cols * 60 / 2 + col * 60;
            int bgY = 21 + row * 14;
            if (row == 0 && col == 0) continue;
            y = bgY;
            if (row > 2) {
                if (row == 3 && col == 0) {
                    c[0] = 0; c[1] = 0; c[2] = 0; c[3] = 192;
                    drawRect(x, bgY, cols * 60, 7, c);
                }
                y += 7;
            }
            if (col > 0 && row > 0) {
                c[0] = 0; c[1] = 0; c[2] = 0; c[3] = 192;
                switch (col - 1) {
                case 0: c[2] = 128; break;
                case 1: c[0] = 128; break;
                case 2: c[0] = 128; c[1] = 128; break;
                case 3: c[1] = 128; break;
                }
            } else {
                c[0] = 0; c[1] = 0; c[2] = 0; c[3] = 192;
            }
            drawRect(x, y, 60, 14, c);
        }
    }
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 1);
    HAL_SetFont(11);
    for (row = 0; row < 13; row++) {
        for (col = 0; col < cols; col++) {
            int x = 160 - cols * 60 / 2 + col * 60;
            int textY = 21 + row * 14;
            if (row == 0 && col == 0) continue;
            y = textY;
            if (row > 2) y += 7;
            if (row == 0) {
                TB_SetTextColor(playerTextColor[col - 1]);
                str = (*gPlayers[col - 1].save)->name;
                TB_DrawEllipseText(x + 30, y + 7, 60, str);
            } else if (col == 0) {
                HAL_SetFont(12);
                if (row == 1) {
                    TB_SetTextColor(1);
                    str = gText.single[140];
                } else if (row == 2) {
                    TB_SetTextColor(1);
                    str = gText.single[224];
                } else {
                    TB_SetTextColor(1);
                    TB_Sprintf(str1, gText.taba[gText.tab[38] + (row - 3)]);
                    str = str1;
                }
            } else {
                if (row == 1) {
                    TB_SetTextColor(1);
                    TB_Sprintf(str1, "%d", gStuntPlayer[col - 1].totalPoints);
                    str = str1;
                } else if (row == 2) {
                    TB_SetTextColor(1);
                    TB_Sprintf(str1, "%d", gStuntPlayer[col - 1].maxStuntPoints);
                    str = str1;
                } else {
                    TB_SetTextColor(1);
                    TB_Sprintf(str1, "%d", gStuntPlayer[col - 1].totalStunts[row - 3]);
                    str = str1;
                }
            }
            if (row != 0) TB_DrawText(x + 30, y + 7, str);
            HAL_SetFont(11);
        }
    }
    HAL_SetTextDepth(-1.0f);
    TB_SetJust(0, 3);
}

static void drawBattleScore(void)
{
    int y;
    unsigned char *str;
    unsigned char str1[40];
    int rows, cols, row, col;
    unsigned char c[4];

    cols = gNumPlayers + 1;
    rows = gNumPlayers + 2;
    for (row = 0; row < rows; row++) {
        int bgY = 112 - rows * 16 / 2 + row * 16;
        for (col = 0; col < cols; col++) {
            int x = 160 - cols * 60 / 2 + col * 60;
            if (row == 0 && col == 0) continue;
            y = bgY;
            if (row > 1) {
                if (row == 2 && col == 0) {
                    c[0] = 0; c[1] = 0; c[2] = 0; c[3] = 192;
                    drawRect(x, bgY, cols * 60, 8, c);
                }
                y += 8;
            }
            if (col == 0 || row == 0) {
                c[0] = 0; c[1] = 0; c[2] = 0; c[3] = 192;
            } else if (row == col + 1) {
                c[0] = 0; c[1] = 0; c[2] = 0; c[3] = 192;
                switch (gCarTeam[col - 1]) {
                case 0: c[2] = 64; break;
                case 1: c[0] = 64; break;
                case 2: c[0] = 64; c[1] = 64; break;
                case 3: c[1] = 64; break;
                }
            } else {
                c[0] = 0; c[1] = 0; c[2] = 0; c[3] = 192;
                switch (gCarTeam[col - 1]) {
                case 0: c[2] = 128; break;
                case 1: c[0] = 128; break;
                case 2: c[0] = 128; c[1] = 128; break;
                case 3: c[1] = 128; break;
                }
            }
            drawRect(x, y, 60, 16, c);
        }
    }
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 1);
    HAL_SetFont(11);
    for (row = 0; row < rows; row++) {
        for (col = 0; col < cols; col++) {
            int x = 160 - cols * 60 / 2 + col * 60;
            int textY = 112 - rows * 16 / 2 + row * 16;
            if (row == 0 && col == 0) continue;
            y = textY;
            if (row > 1) y += 8;
            if (row == 0) {
                TB_SetTextColor(playerTextColor[gCarTeam[col - 1]]);
                str = (*gPlayers[col - 1].save)->name;
                TB_DrawEllipseText(x + 30, y + 8, 60, str);
            } else if (col == 0) {
                if (row == 1) {
                    TB_SetTextColor(1);
                    str = gText.single[140];
                } else {
                    TB_SetTextColor(playerTextColor[gCarTeam[row - 2]]);
                    str = (*gPlayers[row - 2].save)->name;
                    TB_DrawEllipseText(x + 30, y + 8, 60, str);
                }
            } else {
                if (row == 1) {
                    int total = gGameCar[gPlayers[col - 1].slot].battle.Points;
                    TB_SetTextColor(1);
                    TB_Sprintf(str1, "%d", total);
                    str = str1;
                } else {
                    if (row == col + 1) TB_SetTextColor(4);
                    else TB_SetTextColor(1);
                    TB_Sprintf(str1, "%d", gBattlePoints[col - 1][row - 2]);
                    str = str1;
                }
            }
            if (row != 0 && (col != 0 || row == 1))
                TB_DrawText(x + 30, y + 8, str);
        }
    }
    HAL_SetTextDepth(-1.0f);
    TB_SetJust(0, 3);
}

static int animText(void)
{
    switch (gGameOverState) {
    case kGameOverStateMain: drawMain(); break;
    case kGameOverStateScore:
        switch (gRace) {
        case kStunt: drawStuntScore(); break;
        case kBattle: drawBattleScore(); break;
        case kOneRace: case kGhost: case kCircuit: case kObstacle:
            drawRaceScore(); break;
        }
        break;
    }
    return 1;
}

static void drawRect(int x, int y, int w, int h, unsigned char *color)
{
    DC_SendFillRect(x * 2, (int)(2.0f * y), (x + w) * 2, (int)(2.0f * (y + h)),
                    color[0], color[1], color[2], color[3], 65400.0f);
}
