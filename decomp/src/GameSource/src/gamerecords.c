/* gamerecords.c - Game records and player unlock queries. Status: Matching.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\gamerecords.c; assembly: build/2049LE/asm/GameSource/src/gamerecords.s */
#include "rush/rush_protos.h"

signed char gRankedPlayer[4];
int ties;
signed char tied[4];
unsigned char gLap[4];
signed char gForceUnlockedCircuits[4];
signed char gTeamPoints[4];
signed char gUnlockedCircuits[4][4];
signed char gUnlockedTracks[4][19];
signed char gUnlockedFrames[4][6];
signed char gUnlockedTires[4][5];
signed char gUnlockedEngines[4][9];
signed char gUnlockedTrans[4][8];
signed char gUnlockedCars[4][13];
float gBestLapTime[4];
signed char gBattlePoints[4][4];
signed char gNewTotalScore[3][5];
float gLapTimes[4][8];
anon_checksum_numCircuits_1c gTotalCircuitStats[4];
anon_checksum_bestTime_18 gTotalObstacleStats[1];
anon_checksum_totalMatches_c gTotalBattleStats[8];
anon_checksum_bestStunt_40 gTotalStuntStats[4];
anon_checksum_scores_60 gTotalRaceStats[12];
unsigned short gTempNameAges[20];
unsigned char gTempNames[20][13];
tPlayerProfile **gTotalNames[12][3][5];

/* Only reconstructed functions are defined; the remaining bodies are still unmatched. */

/* 0x80015c48, size 0x16c */
void InitRecords(void)
{
    int i;
    int j;
    ClearTotalStats();
    for (i = 0; i < 12; i++) {
        for (j = 0; j < 15; j++)
            ((tPlayerProfile ***)gTotalNames[i])[j] = (tPlayerProfile **)-1;
    }
    memset(gTempNames, 0, sizeof(gTempNames));
    memset(gTempNameAges, 0, sizeof(gTempNameAges));
}

/* 0x80015db4, size 0x7c */
void ClearTotalStats(void)
{
    memset(gTotalRaceStats, 0, sizeof(gTotalRaceStats));
    memset(gTotalStuntStats, 0, sizeof(gTotalStuntStats));
    memset(gTotalBattleStats, 0, sizeof(gTotalBattleStats));
    memset(gTotalObstacleStats, 0, sizeof(gTotalObstacleStats));
    memset(gTotalCircuitStats, 0, sizeof(gTotalCircuitStats));
}

/* 0x80015e30, size 0x53c */
void FindWinner(void)
{
    int i;
    int j;
    ties = 0;
    for (i = 0; i < 4; i++) tied[i] = 0;
    for (i = 0; i < gNumWindows; i++) gRankedPlayer[i] = i;
    if (gRace == kStunt) {
        for (i = 0; i < gNumPlayers - 1; i++) {
            for (j = 0; j < gNumPlayers - 1; j++) {
                if (gStuntPlayer[model[gRankedPlayer[j]].slot].totalPoints < gStuntPlayer[model[gRankedPlayer[j+1]].slot].totalPoints) {
                    signed char temp = gRankedPlayer[j+1];
                    gRankedPlayer[j+1] = gRankedPlayer[j];
                    gRankedPlayer[j] = temp;
                }
            }
        }
        for (i = 1; i < gNumPlayers; i++) {
            if (gStuntPlayer[model[gRankedPlayer[0]].slot].totalPoints == gStuntPlayer[model[gRankedPlayer[i]].slot].totalPoints) {
                ties++;
                tied[i] = 1;
            }
        }
    } else if (gRace == kBattle) {
        for (i = 0; i < gNumPlayers - 1; i++) {
            for (j = 0; j < gNumPlayers - 1; j++) {
                if (gGameCar[model[gRankedPlayer[j]].slot].battle.Points < gGameCar[model[gRankedPlayer[j+1]].slot].battle.Points) {
                    signed char temp = gRankedPlayer[j+1];
                    gRankedPlayer[j+1] = gRankedPlayer[j];
                    gRankedPlayer[j] = temp;
                }
            }
        }
        for (i = 0; i < gNumPlayers - 1; i++) {
            for (j = 0; j < gNumPlayers - 1; j++) {
                if (gTeamPoints[gCarTeam[model[gRankedPlayer[j]].slot]] < gTeamPoints[gCarTeam[model[gRankedPlayer[j+1]].slot]]) {
                    signed char temp = gRankedPlayer[j+1];
                    gRankedPlayer[j+1] = gRankedPlayer[j];
                    gRankedPlayer[j] = temp;
                }
            }
        }
        for (i = 1; i < gNumPlayers; i++) {
            if (gGameCar[model[gRankedPlayer[0]].slot].battle.Points == gGameCar[model[gRankedPlayer[i]].slot].battle.Points) {
                ties++;
                tied[i] = 1;
            }
        }
    } else if (gRace != kPractice) {
        for (i = 0; i < gNumPlayers - 1; i++) {
            for (j = 0; j < gNumPlayers - 1; j++) {
                if (gGameCar[model[gRankedPlayer[j]].slot].place > gGameCar[model[gRankedPlayer[j+1]].slot].place) {
                    signed char temp = gRankedPlayer[j+1];
                    gRankedPlayer[j+1] = gRankedPlayer[j];
                    gRankedPlayer[j] = temp;
                }
            }
        }
        for (i = 1; i < gNumPlayers; i++) {
            if (gGameCar[model[gRankedPlayer[0]].slot].place == gGameCar[model[gRankedPlayer[i]].slot].place) {
                ties++;
                tied[i] = 1;
            }
        }
    }
}

/* 0x8001636c, size 0x6c */
void RecordGameOver(void)
{
    switch (gRace) {
    case kOneRace:
    case kCircuit: RecordRace(); break;
    case kPractice: RecordPractice(); break;
    case kGhost: RecordGhost(); break;
    case kStunt: RecordStunt(); break;
    case kObstacle: RecordObstacle(); break;
    case kBattle: RecordBattle(); break;
    }
}

/* 0x800163d8, size 0x748 */
void RecordRace(void)
{
    int whichTrack;
    int saveTrack;
    int p;
    anon_checksum_scores_60 *stats;
    tCarData *car;
    int i;
    int lap;
    int scoreIdx;
    int statIdx;
    float avgTime;
    whichTrack = gTrackNum;
    saveTrack = gTrackNum;
    if (gBackwards) {
        whichTrack += 6;
        saveTrack += 19;
    }
    for (p = 0; p < gNumPlayers; p++) {
        if (!gPlayers[p].save) gPlayers[p].save = &gJustPlayers[gPlayers[p].controller];
        if (!(*gPlayers[p].save)->datah) break;
        for (statIdx = 0; statIdx < 2; statIdx++) {
            if (statIdx == 0) stats = &(*(*gPlayers[p].save)->datah)->raceStats[whichTrack];
            else stats = &gTotalRaceStats[whichTrack];
            car = &gGameCar[gPlayers[p].slot];
            for (lap = 0; lap < gLap[p]; lap++) {
                for (scoreIdx = 0; scoreIdx < 5; scoreIdx++) {
                    if (stats->scores[0][scoreIdx] == 0.0f || gLapTimes[p][lap] < stats->scores[0][scoreIdx]) {
                        for (i = 4; i > scoreIdx; i--) {
                            stats->scores[0][i] = stats->scores[0][i-1];
                            if (statIdx == 1) {
                                gNewTotalScore[0][i] = gNewTotalScore[0][i-1];
                                gTotalNames[whichTrack][0][i] = gTotalNames[whichTrack][0][i-1];
                            }
                        }
                        stats->scores[0][scoreIdx] = gLapTimes[p][lap];
                        if (statIdx == 1) {
                            gNewTotalScore[0][scoreIdx] = p;
                            if ((*gPlayers[p].save)->fileh) gTotalNames[whichTrack][0][scoreIdx] = gPlayers[p].save;
                        }
                        break;
                    }
                }
            }
            if (gLap[p]) {
                avgTime = 0.0f;
                for (lap = 0; lap < gLap[p]; lap++) avgTime += gLapTimes[p][lap];
                avgTime /= gLap[p];
                for (scoreIdx = 0; scoreIdx < 5; scoreIdx++) {
                    if (stats->scores[1][scoreIdx] == 0.0f || avgTime < stats->scores[1][scoreIdx]) {
                        for (i = 4; i > scoreIdx; i--) {
                            stats->scores[1][i] = stats->scores[1][i-1];
                            if (statIdx == 1) {
                                gNewTotalScore[1][i] = gNewTotalScore[1][i-1];
                                gTotalNames[whichTrack][1][i] = gTotalNames[whichTrack][1][i-1];
                            }
                        }
                        stats->scores[1][scoreIdx] = avgTime;
                        if (statIdx == 1) {
                            gNewTotalScore[1][i] = p;
                            if ((*gPlayers[p].save)->fileh) gTotalNames[whichTrack][1][scoreIdx] = gPlayers[p].save;
                        }
                        break;
                    }
                }
            }
            if (car->place_locked) {
                stats->totalRaces++;
                if (car->place == 0 && gNumActiveCars > 1) stats->total1st++;
                else if (car->place == 1 && gNumActiveCars > 2) stats->total2nd++;
                else if (car->place == 2 && gNumActiveCars >= 3) stats->total3rd++;
                if (gDeathMode) {
                    stats->totalDeathAttempted++;
                    if (gLap[p] == gNumLaps) stats->totalDeathCompleted++;
                }
            }
            stats->totalCrashes += gPlayers[p].numCrashes;
            stats->totalLaps += gLap[p];
            for (i = 0; i < gLap[p]; i++) stats->totalTime += gLapTimes[p][i];
            stats->totalMiles += car->mileage / 528.0f;
        }
        SavePlayerStats(gPlayers[p].save, saveTrack);
    }
}

/* 0x80016b20, size 0x3dc */
void RecordGhost(void)
{
    int whichTrack;
    int saveTrack;
    int p;
    anon_checksum_scores_60 *stats;
    tCarData *car;
    int i;
    int statIdx;
    int scoreIdx;
    float time;
    whichTrack = gTrackNum;
    saveTrack = gTrackNum;
    if (gBackwards) {
        whichTrack += 6;
        saveTrack += 19;
    }
    for (p = 0; p < gNumPlayers; p++) {
        if (!gPlayers[p].save) gPlayers[p].save = &gJustPlayers[gPlayers[p].controller];
        if (!(*gPlayers[p].save)->datah) break;
        for (statIdx = 0; statIdx < 2; statIdx++) {
            if (statIdx == 0) stats = &(*(*gPlayers[p].save)->datah)->raceStats[whichTrack];
            else stats = &gTotalRaceStats[whichTrack];
            car = &gGameCar[gPlayers[p].slot];
            if (car->place_locked) {
                time = car->score;
                for (scoreIdx = 0; scoreIdx < 5; scoreIdx++) {
                    if (stats->scores[2][scoreIdx] == 0.0f || time < stats->scores[2][scoreIdx]) {
                        for (i = 4; i > scoreIdx; i--) {
                            stats->scores[2][i] = stats->scores[2][i-1];
                            if (statIdx == 1) {
                                gNewTotalScore[2][i] = gNewTotalScore[2][i-1];
                                gTotalNames[whichTrack][2][i] = gTotalNames[whichTrack][2][i-1];
                            }
                        }
                        stats->scores[2][scoreIdx] = time;
                        if (statIdx == 1) {
                            gNewTotalScore[2][i] = p;
                            if ((*gPlayers[p].save)->fileh) gTotalNames[whichTrack][2][scoreIdx] = gPlayers[p].save;
                        }
                        break;
                    }
                }
            }
            stats->totalRaces++;
            stats->totalGhosts++;
            stats->totalCrashes += gPlayers[p].numCrashes;
            stats->totalLaps += gLap[p];
            for (i = 0; i < gLap[p]; i++) stats->totalTime += gLapTimes[p][i];
            stats->totalMiles += car->mileage / 528.0f;
        }
        SavePlayerStats(gPlayers[p].save, saveTrack);
    }
}

/* 0x80016efc, size 0x164 */
void RecordPractice(void)
{
    int whichTrack;
    int saveTrack;
    int p;
    int statIdx;
    whichTrack = gTrackNum;
    saveTrack = gTrackNum;
    if (gBackwards) {
        whichTrack += 6;
        saveTrack += 19;
    }
    for (p = 0; p < gNumPlayers; p++) {
        if (!gPlayers[p].save) gPlayers[p].save = &gJustPlayers[gPlayers[p].controller];
        if (!(*gPlayers[p].save)->datah) break;
        for (statIdx = 0; statIdx < 2; statIdx++) {
            (statIdx == 0 ? &(*(*gPlayers[p].save)->datah)->raceStats[whichTrack] : &gTotalRaceStats[whichTrack])->totalMiles +=
                gGameCar[gPlayers[p].slot].mileage / 528.0f;
        }
        SavePlayerStats(gPlayers[p].save, saveTrack);
    }
}

/* 0x80017060, size 0x1fc */
void RecordStunt(void)
{
    int trackIdx;
    int p;
    anon_checksum_bestStunt_40 *stats;
    int i;
    int statIdx;
    tStuntPlayer *stunt;
    trackIdx = gTrackNum - 14;
    for (p = 0; p < gNumPlayers; p++) {
        if (!gPlayers[p].save) gPlayers[p].save = &gJustPlayers[gPlayers[p].controller];
        if (!(*gPlayers[p].save)->datah) break;
        for (statIdx = 0; statIdx < 2; statIdx++) {
            if (statIdx == 0) stats = &(*(*gPlayers[p].save)->datah)->stuntStats[trackIdx];
            else stats = &gTotalStuntStats[trackIdx];
            stunt = &gStuntPlayer[p];
            if (stunt->maxStuntPoints > stats->bestStunt)
                stats->bestStunt = stunt->maxStuntPoints;
            if (stunt->totalPoints / gStuntModeMins > stats->bestAvg)
                stats->bestAvg = stunt->totalPoints / gStuntModeMins;
            stats->totalPoints += stunt->totalPoints;
            stats->totalTime += gStuntModeMins;
            stats->totalCrashes += gPlayers[p].numCrashes;
            for (i = 0; i < 10; i++) stats->totalStunts[i] += stunt->totalStunts[i];
        }
        SavePlayerStats(gPlayers[p].save, gTrackNum);
    }
}

/* 0x8001725c, size 0x1c4 */
void RecordBattle(void)
{
    int p;
    anon_checksum_totalMatches_c *stats;
    int i;
    int statIdx;
    int trackIdx = gTrackNum - 6;
    for (p = 0; p < gNumPlayers; p++) {
        if (!gPlayers[p].save) gPlayers[p].save = &gJustPlayers[gPlayers[p].controller];
        if (!(*gPlayers[p].save)->datah) break;
        for (statIdx = 0; statIdx < 2; statIdx++) {
            if (statIdx == 0) stats = &(*(*gPlayers[p].save)->datah)->battleStats[trackIdx];
            else stats = &gTotalBattleStats[trackIdx];
            for (i = 0; i < gNumPlayers; i++) {
                if (i != p) stats->totalKills += gBattlePoints[p][i];
                stats->totalDeaths += gBattlePoints[i][p] >= 0 ? gBattlePoints[i][p] : -gBattlePoints[i][p];
            }
            stats->totalMatches++;
            if (p == gRankedPlayer[0] || gCarTeam[p] == gCarTeam[gRankedPlayer[0]] || tied[p] == 1)
                stats->totalWins++;
        }
        SavePlayerStats(gPlayers[p].save, gTrackNum);
    }
}

/* 0x80017420, size 0x1a0 */
void RecordObstacle(void)
{
    int trackIdx;
    int p;
    anon_checksum_bestTime_18 *stats;
    tCarData *car;
    int statIdx;
    trackIdx = gTrackNum - 18;
    for (p = 0; p < gNumPlayers; p++) {
        if (!gPlayers[p].save) gPlayers[p].save = &gJustPlayers[gPlayers[p].controller];
        if (!(*gPlayers[p].save)->datah) break;
        for (statIdx = 0; statIdx < 2; statIdx++) {
            if (statIdx == 0) stats = &(*(*gPlayers[p].save)->datah)->obstacleStats[trackIdx];
            else stats = &gTotalObstacleStats[trackIdx];
            car = &gGameCar[gPlayers[p].slot];
            stats->totalTime += car->score;
            stats->totalRaces++;
            if (gLap[p] == 1) {
                stats->totalFinished++;
                if (stats->bestTime == 0.0f || car->score < stats->bestTime)
                    stats->bestTime = car->score;
            }
            stats->totalCrashes += gPlayers[p].numCrashes;
            if (gDeathMode) {
                stats->numDeathAttempted++;
                if (gLap[p] == 1) stats->numDeathCompleted++;
            }
        }
        SavePlayerStats(gPlayers[p].save, gTrackNum);
    }
}

/* 0x800175c0, size 0x20c */
void RecordCircuit(void)
{
    int trackIdx;
    anon_checksum_numCircuits_1c *stats;
    int statIdx;
    if (!gPlayers[gCircuitPlayer].save)
        gPlayers[gCircuitPlayer].save = &gJustPlayers[gPlayers[gCircuitPlayer].controller];
    trackIdx = (*(*gPlayers[gCircuitPlayer].save)->datah)->circuit.type;
    for (statIdx = 0; statIdx < 2; statIdx++) {
        if (statIdx == 0) stats = &(*(*gPlayers[gCircuitPlayer].save)->datah)->circuitStats[trackIdx];
        else stats = &gTotalCircuitStats[trackIdx];
        stats->numCircuits++;
        if (gCircuitCars[0].totalPlace == 0) stats->num1st++;
        else if (gCircuitCars[0].totalPlace == 1) stats->num2nd++;
        else if (gCircuitCars[0].totalPlace == 2) stats->num3rd++;
        if (gDeathMode) {
            stats->numDeathAttempted++;
            if (gCircuitCars[0].totalPlace < 3) stats->numDeathCompleted++;
        }
        stats->totalPts += gCircuitCars[0].totalPoints;
        if (gCircuitCars[0].totalPoints > stats->bestPts) stats->bestPts = gCircuitCars[0].totalPoints;
        if (stats->bestTime == 0.0f || (*(*gPlayers[gCircuitPlayer].save)->datah)->circuit.totalTime < stats->bestTime)
            stats->bestTime = (*(*gPlayers[gCircuitPlayer].save)->datah)->circuit.totalTime;
    }
    SavePlayerCircuitStats(gPlayers[gCircuitPlayer].save, (*(*gPlayers[gCircuitPlayer].save)->datah)->circuit.type);
}

/* 0x800177cc, size 0x124 */
void ResetHighScores(void)
{
    int i;
    for (i = 0; i < gNumPlayers; i++) {
        gLap[i] = 0;
        gBestLapTime[i] = 5999.999f;
    }
    for (i = 0; i < 15; i++) ((signed char *)gNewTotalScore)[i] = -1;
}

/* 0x800178f0, size 0x17c */
void RecordLap(int p, float time)
{
    int i;
    if ((gGameState & 8) || gRace == kPractice) return;
    if (p >= gNumPlayers) return;
    gLapTimes[p][gLap[p]] = time;
    for (i = 0; i < gLap[p]; i++)
        gLapTimes[p][gLap[p]] -= gLapTimes[p][i];
    if (gLapTimes[p][gLap[p]] < gBestLapTime[p])
        gBestLapTime[p] = gLapTimes[p][gLap[p]];
    gLap[p]++;
}

/* 0x80017a6c, size 0x40 */
void InitBattlePoints(void)
{
    memset(gBattlePoints, 0, sizeof(gBattlePoints));
    memset(gTeamPoints, 0, sizeof(gTeamPoints));
}

/* 0x80017aac, size 0x28 */
void RecordBattlePoints(signed char to, signed char from, signed char points)
{
    gBattlePoints[to][from] += points;
}

/* 0x80017ad4, size 0x1dc */
void FoundCoin(int p, int coin)
{
    if (gGameState & 8) return;
    if (gTrackNum >= 0 && gTrackNum <= 5) {
        if (!gPlayers[p].save) {
            (*gJustPlayers[gPlayers[p].controller]->datah)->raceStats[gTrackNum].coinsFound |= 1 << coin;
            (*gJustPlayers[gPlayers[p].controller]->datah)->raceStats[gTrackNum + 6].coinsFound |= 1 << coin;
        } else if ((*gPlayers[p].save)->datah) {
            (*(*gPlayers[p].save)->datah)->raceStats[gTrackNum].coinsFound |= 1 << coin;
            (*(*gPlayers[p].save)->datah)->raceStats[gTrackNum + 6].coinsFound |= 1 << coin;
            SavePlayerCoins(gPlayers[p].save, gTrackNum);
        }
    } else if (gTrackNum >= 14 && gTrackNum <= 17) {
        if (!gPlayers[p].save)
            (*gJustPlayers[gPlayers[p].controller]->datah)->stuntStats[gTrackNum - 14].coinsFound |= 1 << coin;
        else {
            (*(*gPlayers[p].save)->datah)->stuntStats[gTrackNum - 14].coinsFound |= 1 << coin;
            SavePlayerCoins(gPlayers[p].save, gTrackNum);
        }
    }
}

/* 0x80017cb0, size 0x150 */
int IsCoinFound(int p, int coin)
{
    if (gGameState & 8) return 1;
    if (gTrackNum >= 0 && gTrackNum <= 5) {
        if (!gPlayers[p].save)
            return (*gJustPlayers[gPlayers[p].controller]->datah)->raceStats[gTrackNum].coinsFound & (1 << coin);
        if ((*gPlayers[p].save)->datah)
            return (*(*gPlayers[p].save)->datah)->raceStats[gTrackNum].coinsFound & (1 << coin);
        return 1;
    }
    if (gTrackNum >= 14 && gTrackNum <= 17) {
        if (gPlayers[p].save)
            return (*(*gPlayers[p].save)->datah)->stuntStats[gTrackNum - 14].coinsFound & (1 << coin);
        return (*gJustPlayers[gPlayers[p].controller]->datah)->stuntStats[gTrackNum - 14].coinsFound & (1 << coin);
    }
    return 1;
}

/* Coin-address helpers preserve the original indexed field-access evaluation. */
static inline unsigned short *raceStatsCoins(tSaveData *data, int track)
{
    return &data->raceStats[track].coinsFound;
}

static inline unsigned short *stuntStatsCoins(tSaveData *data, int track)
{
    return &data->stuntStats[track].coinsFound;
}

/* 0x80017e00, size 0x144 */
unsigned char NumCoinsFound(int p, int whichCoins)
{
    unsigned int coins;
    unsigned char numFound;
    if (gTrackNum >= 0 && gTrackNum <= 5) {
        if (!gPlayers[p].save) gPlayers[p].save = &gJustPlayers[gPlayers[p].controller];
        if (!(*gPlayers[p].save)->datah) return 16;
        coins = *raceStatsCoins((*(*gPlayers[p].save)->datah), gTrackNum);
    } else if (gTrackNum >= 14 && gTrackNum <= 17) {
        if (!gPlayers[p].save) gPlayers[p].save = &gJustPlayers[gPlayers[p].controller];
        coins = *stuntStatsCoins((*(*gPlayers[p].save)->datah), gTrackNum - 14);
    } else coins = 0;
    if (whichCoins) coins &= whichCoins == 1 ? 0xff00 : 0xff;
    numFound = CountBits(coins, 16);
    return numFound;
}

/* 0x80017f44, size 0x34 */
int CountBits(unsigned int data, unsigned char numBits)
{
    int count = 0;
    while (numBits && data) {
        if (data & 1) count++;
        data >>= 1;
        numBits--;
    }
    return count;
}

/* 0x80017f78, size 0x1b4 */
void AddPlayerScores(tPlayerProfile **player)
{
    tSaveData *data;
    int track;
    int type;
    int scoreIdx;
    float *scores;
    float *totalScores;
    tPlayerProfile ***totalNames;
    int totalIdx;
    int i;
    if (!(*player)->datah) return;
    data = *(*player)->datah;
    for (track = 0; track < 12; track++) {
        for (type = 0; type < 3; type++) {
            scoreIdx = 0;
            scores = data->raceStats[track].scores[type];
            totalScores = gTotalRaceStats[track].scores[type];
            totalNames = gTotalNames[track][type];
            for (totalIdx = 0; totalIdx < 5; totalIdx++) {
                if (scores[scoreIdx] > 0.0f && (totalScores[totalIdx] == 0.0f || totalScores[totalIdx] > scores[scoreIdx])) {
                    for (i = 4; i > totalIdx; i--) {
                        totalScores[i] = totalScores[i-1];
                        totalNames[i] = totalNames[i-1];
                    }
                    totalScores[totalIdx] = scores[scoreIdx];
                    totalNames[i] = player;
                    scoreIdx++;
                }
            }
        }
    }
}

/* 0x8001812c, size 0x560 */
void AddPlayerStats(tPlayerProfile **player)
{
    tSaveData *data;
    int trackIdx;
    int circuitIdx;
    int stuntIdx;
    if (!(*player)->datah) return;
    data = *(*player)->datah;
    for (trackIdx = 0; trackIdx < 12; trackIdx++) {
        gTotalRaceStats[trackIdx].totalTime += data->raceStats[trackIdx].totalTime;
        gTotalRaceStats[trackIdx].totalLaps += data->raceStats[trackIdx].totalLaps;
        gTotalRaceStats[trackIdx].totalRaces += data->raceStats[trackIdx].totalRaces;
        gTotalRaceStats[trackIdx].total1st += data->raceStats[trackIdx].total1st;
        gTotalRaceStats[trackIdx].total2nd += data->raceStats[trackIdx].total2nd;
        gTotalRaceStats[trackIdx].total3rd += data->raceStats[trackIdx].total3rd;
        gTotalRaceStats[trackIdx].totalCrashes += data->raceStats[trackIdx].totalCrashes;
        gTotalRaceStats[trackIdx].totalDeathAttempted += data->raceStats[trackIdx].totalDeathAttempted;
        gTotalRaceStats[trackIdx].totalDeathCompleted += data->raceStats[trackIdx].totalDeathCompleted;
        gTotalRaceStats[trackIdx].totalGhosts += data->raceStats[trackIdx].totalGhosts;
        gTotalRaceStats[trackIdx].totalMiles += data->raceStats[trackIdx].totalMiles;
        gTotalRaceStats[trackIdx].coinsFound |= data->raceStats[trackIdx].coinsFound;
    }
    for (trackIdx = 0; trackIdx < 4; trackIdx++) {
        if (gTotalStuntStats[trackIdx].bestStunt < data->stuntStats[trackIdx].bestStunt) gTotalStuntStats[trackIdx].bestStunt = data->stuntStats[trackIdx].bestStunt;
        if (gTotalStuntStats[trackIdx].bestAvg < data->stuntStats[trackIdx].bestAvg) gTotalStuntStats[trackIdx].bestAvg = data->stuntStats[trackIdx].bestAvg;
        gTotalStuntStats[trackIdx].totalPoints += data->stuntStats[trackIdx].totalPoints;
        gTotalStuntStats[trackIdx].totalTime += data->stuntStats[trackIdx].totalTime;
        gTotalStuntStats[trackIdx].totalCrashes += data->stuntStats[trackIdx].totalCrashes;
        for (stuntIdx = 0; stuntIdx < 10; stuntIdx++) gTotalStuntStats[trackIdx].totalStunts[stuntIdx] += data->stuntStats[trackIdx].totalStunts[stuntIdx];
        gTotalStuntStats[trackIdx].coinsFound |= data->stuntStats[trackIdx].coinsFound;
    }
    for (trackIdx = 0; trackIdx < 8; trackIdx++) {
        gTotalBattleStats[trackIdx].totalMatches += data->battleStats[trackIdx].totalMatches;
        gTotalBattleStats[trackIdx].totalWins += data->battleStats[trackIdx].totalWins;
        gTotalBattleStats[trackIdx].totalKills += data->battleStats[trackIdx].totalKills;
        gTotalBattleStats[trackIdx].totalDeaths += data->battleStats[trackIdx].totalDeaths;
    }
    for (trackIdx = 0; trackIdx < 1; trackIdx++) {
    if (data->obstacleStats[trackIdx].bestTime > 0.0f && (gTotalObstacleStats[trackIdx].bestTime == 0.0f || gTotalObstacleStats[trackIdx].bestTime > data->obstacleStats[trackIdx].bestTime)) gTotalObstacleStats[trackIdx].bestTime = data->obstacleStats[trackIdx].bestTime;
    gTotalObstacleStats[trackIdx].totalTime += data->obstacleStats[trackIdx].totalTime;
    gTotalObstacleStats[trackIdx].totalRaces += data->obstacleStats[trackIdx].totalRaces;
    gTotalObstacleStats[trackIdx].totalFinished += data->obstacleStats[trackIdx].totalFinished;
    gTotalObstacleStats[trackIdx].totalCrashes += data->obstacleStats[trackIdx].totalCrashes;
    gTotalObstacleStats[trackIdx].numDeathAttempted += data->obstacleStats[trackIdx].numDeathAttempted;
    gTotalObstacleStats[trackIdx].numDeathCompleted += data->obstacleStats[trackIdx].numDeathCompleted;
    }
    for (circuitIdx = 0; circuitIdx < 4; circuitIdx++) {
        gTotalCircuitStats[circuitIdx].numCircuits += data->circuitStats[circuitIdx].numCircuits;
        gTotalCircuitStats[circuitIdx].num1st += data->circuitStats[circuitIdx].num1st;
        gTotalCircuitStats[circuitIdx].num2nd += data->circuitStats[circuitIdx].num2nd;
        gTotalCircuitStats[circuitIdx].num3rd += data->circuitStats[circuitIdx].num3rd;
        gTotalCircuitStats[circuitIdx].numDeathAttempted += data->circuitStats[circuitIdx].numDeathAttempted;
        gTotalCircuitStats[circuitIdx].numDeathCompleted += data->circuitStats[circuitIdx].numDeathCompleted;
        if (gTotalCircuitStats[circuitIdx].bestPts < data->circuitStats[circuitIdx].bestPts) gTotalCircuitStats[circuitIdx].bestPts = data->circuitStats[circuitIdx].bestPts;
        gTotalCircuitStats[circuitIdx].totalPts += data->circuitStats[circuitIdx].totalPts;
    }
}

/* 0x8001868c, size 0x194 */
void RemovePlayerScores(tPlayerProfile **player)
{
    int track;
    int type;
    float *totalScores;
    tPlayerProfile ***totalNames;
    int totalIdx;
    int i;
    for (track = 0; track < 12; track++) {
        for (type = 0; type < 3; type++) {
            totalScores = gTotalRaceStats[track].scores[type];
            totalNames = gTotalNames[track][type];
            for (totalIdx = 0; totalIdx < 5; totalIdx++) {
                if (totalNames[totalIdx] == player) {
                    for (i = totalIdx; i < 4; i++) {
                        totalScores[i] = totalScores[i+1];
                        totalNames[i] = totalNames[i+1];
                    }
                    totalScores[i] = 0.0f;
                    totalNames[i] = (tPlayerProfile **)-1;
                    totalIdx--;
                }
            }
        }
    }
}

/* 0x80018820, size 0xacc */
void CalcUnlockedStuff(void)
{
    tSaveData *player;
    short carIdx;
    short trackIdx;
    short p;
    short i;
    int totalRaceSilverCoins;
    int totalRaceGoldCoins;
    int totalStuntSilverCoins;
    int totalStuntGoldCoins;
    int raceIdx;
    int stuntIdx;
    if (gCheatState[16] == 1) {
        for (p = 0; p < gNumPlayers; p++)
            for (carIdx = 0; carIdx < 13; carIdx++) gUnlockedCars[p][carIdx] = 1;
    } else {
        for (p = 0; p < gNumPlayers; p++) {
            totalRaceSilverCoins = 0;
            totalRaceGoldCoins = 0;
            totalStuntSilverCoins = 0;
            totalStuntGoldCoins = 0;
            if ((*gPlayers[p].save)->datah) {
                for (carIdx = 0; carIdx < 13; carIdx++) {
                    if (carIdx < 6) gUnlockedCars[p][carIdx] = 1;
                    else gUnlockedCars[p][carIdx] = 0;
                }
                for (raceIdx = 0; raceIdx < 6; raceIdx++) {
                    anon_checksum_scores_60 *stats = &(*(*gPlayers[p].save)->datah)->raceStats[raceIdx];
                    totalRaceSilverCoins += CountBits(stats->coinsFound & 0xff, 16);
                    totalRaceGoldCoins += CountBits(stats->coinsFound & 0xff00, 16);
                }
                for (stuntIdx = 0; stuntIdx < 4; stuntIdx++) {
                    anon_checksum_bestStunt_40 *stats = &(*(*gPlayers[p].save)->datah)->stuntStats[stuntIdx];
                    totalStuntSilverCoins += CountBits(stats->coinsFound & 0xff, 16);
                    totalStuntGoldCoins += CountBits(stats->coinsFound & 0xff00, 16);
                }
                if (totalRaceSilverCoins >= 48) gUnlockedCars[p][6] = 1;
                if (totalRaceGoldCoins >= 24) gUnlockedCars[p][7] = 1;
                if (totalRaceGoldCoins >= 36) gUnlockedCars[p][8] = 1;
                if (totalStuntSilverCoins >= 32) gUnlockedCars[p][9] = 1;
                if (totalStuntGoldCoins >= 16) gUnlockedCars[p][10] = 1;
                if (totalStuntGoldCoins >= 24) gUnlockedCars[p][11] = 1;
                if (totalRaceSilverCoins >= 48 && totalRaceGoldCoins >= 48 && totalStuntSilverCoins >= 32 && totalStuntGoldCoins >= 32)
                    gUnlockedCars[p][12] = 1;
            }
        }
    }
    if (gCheatState[18] == 1) {
        for (p = 0; p < gNumPlayers; p++) {
            if ((*gPlayers[p].save)->datah) {
                for (i = 0; i < 8; i++) {
                    if (i == 6 || i == 7) gUnlockedTrans[p][i] = 0;
                    else gUnlockedTrans[p][i] = 1;
                }
                for (i = 0; i < 9; i++) gUnlockedEngines[p][i] = 1;
                for (i = 0; i < 5; i++) gUnlockedTires[p][i] = 1;
                for (i = 0; i < 6; i++) gUnlockedFrames[p][i] = 1;
            }
        }
    } else {
        for (p = 0; p < gNumPlayers; p++) {
            if ((*gPlayers[p].save)->datah) {
                int totalMileage = 0;
                int raceIdx;
                for (raceIdx = 0; raceIdx < 12; raceIdx++) totalMileage += (*(*gPlayers[p].save)->datah)->raceStats[raceIdx].totalMiles;
                totalMileage /= 10;
                gUnlockedTrans[p][0] = 1;
                gUnlockedTrans[p][1] = 1;
                gUnlockedTrans[p][2] = totalMileage >= 200;
                gUnlockedTrans[p][3] = totalMileage >= 200;
                gUnlockedTrans[p][4] = totalMileage >= 500;
                gUnlockedTrans[p][5] = totalMileage >= 500;
                gUnlockedTrans[p][6] = 0;
                gUnlockedTrans[p][7] = 0;
                gUnlockedEngines[p][0] = 1;
                gUnlockedEngines[p][1] = 1;
                gUnlockedEngines[p][2] = 1;
                gUnlockedEngines[p][3] = totalMileage >= 250;
                gUnlockedEngines[p][4] = totalMileage >= 500;
                gUnlockedEngines[p][5] = totalMileage >= 800;
                gUnlockedEngines[p][6] = totalMileage >= 1200;
                gUnlockedEngines[p][7] = totalMileage >= 1600;
                gUnlockedEngines[p][8] = totalMileage >= 2000;
                gUnlockedTires[p][0] = 1;
                gUnlockedTires[p][1] = totalMileage >= 300;
                gUnlockedTires[p][2] = totalMileage >= 1200;
                gUnlockedTires[p][3] = totalMileage >= 100;
                gUnlockedTires[p][4] = totalMileage >= 600;
                gUnlockedFrames[p][0] = 1;
                gUnlockedFrames[p][1] = 1;
                gUnlockedFrames[p][2] = 1;
                gUnlockedFrames[p][3] = totalMileage >= 150;
                gUnlockedFrames[p][4] = totalMileage >= 400;
                gUnlockedFrames[p][5] = totalMileage >= 700;
            }
        }
    }
    if (gCheatState[17] == 1) {
        for (p = 0; p < gNumPlayers; p++)
            for (trackIdx = 0; trackIdx < 19; trackIdx++) gUnlockedTracks[p][trackIdx] = 1;
    } else {
        for (p = 0; p < gNumPlayers; p++) {
            for (trackIdx = 0; trackIdx < 19; trackIdx++) gUnlockedTracks[p][trackIdx] = 0;
            gUnlockedTracks[p][0] = 1;
            gUnlockedTracks[p][1] = 1;
            gUnlockedTracks[p][2] = 1;
            gUnlockedTracks[p][3] = 1;
            gUnlockedTracks[p][14] = 1;
            gUnlockedTracks[p][6] = 1;
            gUnlockedTracks[p][7] = 1;
            gUnlockedTracks[p][8] = 1;
            gUnlockedTracks[p][9] = 1;
            if ((*gPlayers[p].save)->datah) {
                anon_checksum_bestStunt_40 *stuntStats;
                anon_checksum_totalMatches_c *battleStats;
                int totalStuntPoints;
                int totalBattlePoints;
                stuntStats = (*(*gPlayers[p].save)->datah)->stuntStats;
                battleStats = (*(*gPlayers[p].save)->datah)->battleStats;
                totalStuntPoints = 0;
                for (i = 0; i < 4; i++) totalStuntPoints += stuntStats[i].totalPoints;
                totalBattlePoints = 0;
                for (i = 0; i < 8; i++) totalBattlePoints += battleStats[i].totalKills;
                gUnlockedTracks[p][15] = totalStuntPoints >= 100000;
                gUnlockedTracks[p][16] = totalStuntPoints >= 250000;
                gUnlockedTracks[p][17] = totalStuntPoints >= 500000;
                gUnlockedTracks[p][18] = totalStuntPoints >= 1000000;
                gUnlockedTracks[p][10] = totalBattlePoints >= 100;
                gUnlockedTracks[p][11] = totalBattlePoints >= 250;
                gUnlockedTracks[p][12] = totalBattlePoints >= 500;
                gUnlockedTracks[p][13] = totalBattlePoints >= 1000;
            }
        }
    }
    if (gCheatState[17] == 1) {
        for (p = 0; p < gNumPlayers; p++)
            for (i = 0; i < 4; i++) gUnlockedCircuits[p][i] = 1;
    } else {
        for (p = 0; p < gNumPlayers; p++) {
            if ((*gPlayers[p].save)->datah) {
                player = *(*gPlayers[p].save)->datah;
                for (i = 0; i < 4; i++) gUnlockedCircuits[p][i] = 0;
                gUnlockedCircuits[p][0] = 1;
                if (player->circuitStats[0].num1st || player->circuitStats[0].num2nd || player->circuitStats[0].num3rd || gForceUnlockedCircuits[1]) {
                    gUnlockedCircuits[p][1] = 1;
                    gUnlockedTracks[p][4] = 1;
                }
                if (player->circuitStats[1].num1st || player->circuitStats[1].num2nd || player->circuitStats[1].num3rd || gForceUnlockedCircuits[2]) {
                    gUnlockedCircuits[p][2] = 1;
                    gUnlockedTracks[p][5] = 1;
                }
                if (player->circuitStats[2].num1st || player->circuitStats[2].num2nd || player->circuitStats[2].num3rd || gForceUnlockedCircuits[3]) {
                    gUnlockedCircuits[p][3] = 1;
                }
            }
        }
    }
}

/* 0x800192ec, size 0x40 */
int IsTrackUnlocked(int track)
{
    int trackUnlocked = 0;
    int p;
    for (p = 0; p < gNumPlayers; p++)
        if (gUnlockedTracks[p][track]) trackUnlocked = 1;
    return trackUnlocked;
}

/* 0x8001932c, size 0x1c */
int IsCarUnlocked(int p, int car) { return gUnlockedCars[p][car]; }

/* 0x80019348, size 0x1c */
int IsEngineUnlocked(int p, int engine) { return gUnlockedEngines[p][engine]; }

/* 0x80019364, size 0x1c */
int IsTransUnlocked(int p, int trans) { return gUnlockedTrans[p][trans]; }

/* 0x80019380, size 0x1c */
int IsTiresUnlocked(int p, int tires) { return gUnlockedTires[p][tires]; }

/* 0x8001939c, size 0x1c */
int IsFrameUnlocked(int p, int frame) { return gUnlockedFrames[p][frame]; }

/* 0x800193b8, size 0x40 */
int IsCircuitUnlocked(int circuit)
{
    int circuitUnlocked = 0;
    int p;
    for (p = 0; p < gNumPlayers; p++)
        if (gUnlockedCircuits[p][circuit]) circuitUnlocked = 1;
    return circuitUnlocked;
}

/* 0x800193f8, size 0x3a4 */
int FindTempNameIdx(char *name)
{
    short i;
    short trk;
    short t;
    short count[20];
    short age;
    short oldest;
    int nameIdx;
    if (!name[0]) return 0;
    for (i = 1; i < 20; i++) gTempNameAges[i]++;
    for (i = 1; i < 20; i++) {
        if (!strcmp((char *)gTempNames[i], name)) {
            gTempNameAges[i] = 0;
            return i;
        }
    }
    for (i = 1; i < 20; i++) {
        if (!gTempNames[i][0]) {
            strcpy((char *)gTempNames[i], name);
            gTempNameAges[i] = 0;
            return i;
        }
    }
    for (i = 0; i < 20; i++) count[i] = 0;
    for (trk = 0; trk < 12; trk++) {
        for (t = 0; t < 3; t++) {
            for (i = 0; i < 5; i++) {
                nameIdx = (int)gTotalNames[trk][t][i];
                if (nameIdx >= 0 && nameIdx < 20) count[nameIdx]++;
            }
        }
    }
    for (i = 1; i < 20; i++) {
        if (count[i] == 0) {
            strcpy((char *)gTempNames[i], name);
            gTempNameAges[i] = 0;
            return i;
        }
    }
    age = 0;
    oldest = 0;
    for (i = 1; i < 20; i++) {
        if (gTempNameAges[i] > age) {
            age = gTempNameAges[i];
            oldest = i;
        }
    }
    for (trk = 0; trk < 12; trk++) {
        for (t = 0; t < 3; t++) {
            for (i = 0; i < 5; i++) {
                if (gTotalNames[trk][t][i] == (tPlayerProfile **)oldest) gTotalNames[trk][t][i] = 0;
            }
        }
    }
    if (oldest) {
        strcpy((char *)gTempNames[oldest], name);
        gTempNameAges[oldest] = 0;
    }
    return oldest;
}

