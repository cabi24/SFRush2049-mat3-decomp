/* ghost.c - ghost cars: record each player's inputs during a race and replay saved ghosts as extra cars.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\ghost.c; assembly: build/2049LE/asm/GameSource/src/ghost.s */
#include "rush/rush_protos.h"

tSaveHeader *gGhostToSave;
static signed char gEndingGhosts;
static signed char gInited;
static tGhostFile *gNewGhosts[4];
static tGhostFile gRealNewGhosts[4];
static tSaveHeader gRealNewGhostHeaders[4];
static tSaveHeader *gNewGhostHeaders[4];
static tPlayerProfile gRealGhostPlayers[4];
static tPlayerProfile *gGhostPlayers[4];

/* 0x80019870, size 0x594 */
void GhostStart(void)
{
    tSaveHeader *g; /* r23 */
    int i; /* r17 */
    int np; /* r16 */
    int car; /* r15 */
    int j; /* r0 */

    if (gRace != 2) {
        return;
    }
    if (!gInited) {
        for (j = 0; j < 4; j++) {
            gGhostPlayers[j] = &gRealGhostPlayers[j];
            gNewGhostHeaders[j] = &gRealNewGhostHeaders[j];
        }
        gInited = 1;
    }
    if (gRestartGame) {
        gNumPlayers = gNumCars = 1;
    }
    np = gNumPlayers;
    for (i = 0; i < gNumPlayers; i++) {
        gNewGhosts[i] = &gRealNewGhosts[i];
        gGhosts[i] = &gNewGhosts[i];
        (*gGhosts[i])->header = &gNewGhostHeaders[i];
        g = *(*gGhosts[i])->header;
        g->flag = -1;
        g->id = (*(*gPlayers[i].save)->datah)->raceStats[gTrackNum + (gBackwards ? 6 : 0)].totalGhosts;
        g->track = gTrackNum;
        g->backward = gBackwards;
        memcpy(g->name, (*gPlayers[i].save)->name, 13);
        memcpy(g->playerId, (*gPlayers[i].save)->id, 8);
        g->trans = GetPlayerCarTrans(gPlayers[i].save);
        memcpy(g->settings, (*(*gPlayers[i].save)->datah)->carSettings[gLink[i].car].settings, 10);
        g->car = gLink[i].car;
        g->retraceTime = gRetraceTime;
        if (!gRestartGame) {
            g->size = (int)(gRetraceRate * (float)(10.0 + ghostRaceTimes[gTrackNum]));
            g->start = AllocateMemoryHeap(0, g->size * 3);
        }
        g->offset = 0;
    }
    for (; i < 4; i++) {
        if (!gGhosts[i]) {
            break;
        }
        if (!ReadGhostData(gGhosts[i])) {
            for (j = i; j < 3; j++) {
                gGhosts[j] = gGhosts[j + 1];
            }
            continue;
        }
        (*(*gGhosts[i])->header)->start = *(*gGhosts[i])->datah;
        g = *(*gGhosts[i])->header;
        g->offset = 0;
        car = g->car;
        memset(&gPlayers[i], 0, sizeof(gPlayers[i]));
        gPlayers[i].save = &gGhostPlayers[i];
        memset(*gPlayers[i].save, 0, sizeof(**gPlayers[i].save));
        memcpy((*gPlayers[i].save)->name, g->name, 13);
        gPlayers[i].slot = i;
        gPlayers[i].controller = i;
        gPlayers[i].trans = g->trans;
        gCarHandling[i + 1][car] = g->settings[0];
        gCarEngines[i + 1][car] = g->settings[1];
        gCarTires[i + 1][car] = g->settings[2];
        gCarFrame[i + 1][car] = g->settings[3];
        gCarWings[i + 1][car] = g->settings[4];
        gTireRims[i + 1][car] = g->settings[6];
        gCarHorn[i + 1][car] = g->settings[7];
        gFrontTireScale[i + 1][car] = 0.75f + g->settings[8] / 100.0f;
        gRearTireScale[i + 1][car] = 0.75f + g->settings[9] / 100.0f;
        gLink[np].track = g->track;
        gLink[np].car = car;
        gLink[np].PaintJob = g->settings[5];
        gLink[np].polepos = np;
        gLink[np].flags = 0xb0;
        gLink[np].owner = 6;
        model[np].drone_type = 2;
        model[np].fastin.dt = g->retraceTime;
        gNumCars = ++np;
    }
    gNumPlayers = np;
    num_humans = np;
}

/* 0x80019e04, size 0x3dc */
int GhostUpdate(int i, float *time)
{
    tSaveHeader *g; /* r5 */

    *time = gRetraceTime;
    if (gEndingGhosts) {
        return 0;
    }
    if (!gGhosts[i]) {
        return 0;
    }
    g = *(*gGhosts[i])->header;
    if (!g->flag) {
        return 0;
    }
    if (g->flag > 0) {
        *time = g->retraceTime;
        if (g->retraceTime != gRetraceTime && 0.02f == g->retraceTime &&
            (gModelIteration >= 0 ? gModelIteration : -gModelIteration) % 6 == 2) {
            return -1;
        }
        if (g->offset == g->size) {
            model[i].mainin.abort = 0;
            model[i].mainin.wing = 0;
            model[i].mainin.wheel = 0.0f;
            model[i].mainin.throttle = 0.0f;
            model[i].mainin.brake = 0.0f;
        } else {
            model[i].mainin.gear = (g->start[g->offset] & 7) - 1;
            model[i].mainin.abort = (g->start[g->offset] & 8) >> 3;
            model[i].mainin.wing = (g->start[g->offset] & 0x10) >> 4;
            model[i].mainin.wheel = (signed char)g->start[g->offset + g->size] / 127.0f;
            model[i].mainin.throttle = (g->start[g->offset + g->size + g->size] >> 4) / 15.0f;
            model[i].mainin.brake = (g->start[g->offset + g->size + g->size] & 0xf) / 15.0f;
            g->offset++;
        }
        if (g->retraceTime != gRetraceTime && 0.016666668f == g->retraceTime &&
            (gModelIteration >= 0 ? gModelIteration : -gModelIteration) % 5 == 2) {
            return 1;
        }
        return 0;
    }
    if (gGameCar[i].place_locked) {
        return 0;
    }
    if (g->offset >= g->size) {
        return 0;
    }
    g->start[g->offset] = (model[i].mainin.wing << 4) | ((model[i].mainin.gear + 1) | (model[i].mainin.abort << 3));
    g->start[g->offset + g->size] = 127.0f * model[i].mainin.wheel < 0.0f ? 127.0f * model[i].mainin.wheel - 0.5f : 127.0f * model[i].mainin.wheel + 0.5f;
    g->start[g->offset + g->size + g->size] = ((unsigned char)(int)(15.0f * model[i].mainin.throttle + 0.5f) << 4) | (int)(15.0f * model[i].mainin.brake + 0.5f);
    g->offset++;
    return 0;
}

/* 0x8001a1e0, size 0x104 */
void GhostEnd(void)
{
    tSaveHeader *g; /* r27 */
    int i; /* r31 */

    gGhostToSave = 0;
    if (gRace != 2) {
        return;
    }
    gEndingGhosts = 1;
    for (i = 0; i < gNumPlayers; i++) {
        if (gGhosts[i]) {
            tGhostFile **ghost = gGhosts[i];

            gGhosts[i] = 0;
            model[i].fastin.modelrun = 0;
            g = *(*ghost)->header;
            if (g->flag >= 0) {
                if (g->flag > 0) {
                    PurgeGhostData(ghost);
                }
            } else {
                g->flag = 0;
                if (gGameCar[i].place_locked) {
                    g->finish = gGameCar[i].score;
                    g->size = g->offset;
                    gGhostToSave = g;
                } else {
                    DeallocateMemory(g->start);
                    g->start = 0;
                }
            }
        }
    }
    gNumPlayers = 1;
    gEndingGhosts = 0;
}

