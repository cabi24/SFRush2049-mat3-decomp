/* checkpoint.c - checkpoint crossing, laps, and race placement. Status: in progress.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\checkpoint.c; assembly: build/2049LE/asm/GameSource/src/checkpoint.s */
#include "rush/rush_protos.h"

static void PlaceUnlockedCars(void);

float ghostRaceTimes[6] = { 225.0f, 285.0f, 300.0f, 390.0f, 330.0f, 450.0f };
signed char gNumLapsLeft;
signed char gOneMoreCP;
static float gFirstPlaceTime;
static signed char gInFirstPlace;
float lastDotProd[6];
anon_startTime_loopPt_32c gCheckpoints;
static float gPrevPos[6][3];

/* 0x800095d0, size 0x30 */
void InitCheckpointData(void)
{
    finish_cam[1] = finish_cam[0] = 0;
    ReadCheckpointData();
}

/* 0x80009600, size 0x14 */
void InitCPS(void)
{
    gEndGame = 0;
    gNumLapsLeft = gNumLaps;
}

/* 0x80009614, size 0x1a8 */
void InitCPTime(void)
{
    int i; /* r8 */
    gInFirstPlace = 0;
    gFirstPlaceTime = 0.0f;
    for (i = 0; i < gNumActiveCars; i++) {
        gGameCar[i].mileage = 0.0f;
        gPrevPos[i][0] = gGameCar[i].dr_pos[0];
        gPrevPos[i][1] = gGameCar[i].dr_pos[1];
        gPrevPos[i][2] = gGameCar[i].dr_pos[2];
    }
}

/* 0x800097bc, size 0x354 */
static void PlaceUnlockedCars(void)
{
    signed short i; /* r8 */
    signed short j; /* r22 */
    signed short num_left; /* r12 */
    signed short num_dead; /* r31 */
    signed short place[6]; /* stack */
    int distance[6]; /* stack */
    int highIndex; /* r23 */
    num_left = 0;
    num_dead = 0;
    for (i = 0; i < gNumActiveCars; i++) {
        if (gGameCar[model[i].slot].dead > 0 && model[i].laps < gNumLaps) num_dead++;
        else if (gGameCar[model[i].slot].place_locked != 1) place[num_left++] = model[i].slot;
    }
    if (num_left == 0) return;
    for (i = 0; i < num_left; i++) distance[place[i]] = gGameCar[place[i]].distance;
    for (i = 0; i < num_left; i++) {
        highIndex = i;
        for (j = i + 1; j < num_left; j++) {
            if (distance[place[j]] > distance[place[highIndex]]) highIndex = j;
        }
        gGameCar[place[highIndex]].place = gNumActiveCars + i - num_left - num_dead;
        place[highIndex] ^= place[i];
        place[i] ^= place[highIndex];
        place[highIndex] ^= place[i];
    }
}

/* 0x80009b10, size 0x2b4 */
void CheckCPs(void)
{
    signed short index; /* r26 */
    anon_pos_normal_50 *chkpnt; /* r25 */
    float distVec[3]; /* stack */
    float dot; /* f31 */
    float time; /* f2 */
    chkpnt = gCheckpoints.points;
    for (index = 0; index < gNumCars; index++) {
        tModelData *m = &model[index];
        tCarData *gc = &gGameCar[index];
        float *prev = gPrevPos[index];
        anon_track_car_a *link = &gLink[index];
        distVec[0] = gc->dr_pos[0] - prev[0];
        distVec[1] = gc->dr_pos[1] - prev[1];
        distVec[2] = gc->dr_pos[2] - prev[2];
        gc->mileage += magnitude(distVec);
        prev[0] = gc->dr_pos[0];
        prev[1] = gc->dr_pos[1];
        prev[2] = gc->dr_pos[2];
        if ((link->owner == 0 || link->owner == 6) && gc->crash_state == 0 && m->resurrect.moving_state == -1) {
            float dx = gc->dr_pos[0] - chkpnt[m->next_cp_index].pos[0];
            float dy = gc->dr_pos[1] - chkpnt[m->next_cp_index].pos[1];
            float dz = gc->dr_pos[2] - chkpnt[m->next_cp_index].pos[2];
            int sign;
            dot = dx * chkpnt[m->next_cp_index].normal[0] + dy * chkpnt[m->next_cp_index].normal[1] + dz * chkpnt[m->next_cp_index].normal[2];
            sign = dot < 0.0f ? -1 : 1;
            if (sign != m->cpSign) {
                m->cpSign = sign;
                if (dx * dx + dz * dz < chkpnt[m->next_cp_index].radius) {
                    time = dot / (dot - lastDotProd[index]);
                    time = gFrameSeconds * time;
                    time = gc->modelTime - time;
                    PassedCP(&model[index], time);
                }
            }
            lastDotProd[index] = dot;
        }
    }
    if (gRace != 1) {
        FindMaxpathIntervals();
        PlaceUnlockedCars();
        if (gFirstPlaceTime && gSeconds - gFirstPlaceTime > 3.0f) gFirstPlaceTime = 0.0f;
        if (num_humans == 1 && gGameCar[gPlayers[0].slot].place == 0 && !(gGameState & 8)) {
            if (!gInFirstPlace && gFirstPlaceTime == 0.0f) {
                gFirstPlaceTime = gSeconds;
                PlayGameSound(0x8e, gPlayers[0].slot, 1, 1);
            }
            gInFirstPlace = 1;
        } else gInFirstPlace = 0;
    }
}

/* 0x80009dc4, size 0x33c */
void PassedCP(tModelData *m, float time)
{
    int slot; /* r29 */
    tCarData *gc; /* r28 */
    int finish; /* r27 */
    int prevLapsLeft; /* r0 */
    slot = m->slot;
    gc = &gGameCar[slot];
    finish = 0;
    m->last_cp_index = m->next_cp_index;
    m->next_cp_index = GetNextCheckpoint(m->last_cp_index);
    if (m->last_cp_index == gCheckpoints.finishPt && m->started && gRace != 1 && !(gGameState & 8)) {
        finish = 1;
        m->laps++;
    }
    if (!m->started && m->last_cp_index == gCheckpoints.startPt) m->started = 1;
    if (gLink[slot].owner == 6) {
        if (m->last_cp_index == gCheckpoints.finishPt && !(gGameState & 8)) ShowCheckpointTime(slot, time);
        gOneMoreCP = m->next_cp_index == gCheckpoints.finishPt && m->laps == gNumLaps - 1;
    }
    if (finish) {
        RecordLap(m->slot, time);
        if (m->laps == gNumLaps && !gc->place_locked) CarReportsGameOver(m->slot, time);
        if (gLink[slot].owner == 6) {
            if (gRace == 2) {
                switch (slot) {
                case 0: break;
                default: goto done_finish;
                }
            }
            prevLapsLeft = gNumLapsLeft;
            gNumLapsLeft = gNumLapsLeft < gNumLaps - m->laps ? gNumLapsLeft : gNumLaps - m->laps;
            if (m->laps == gNumLaps && gc->place_locked != -1) {
                finish_cam[slot] = 1;
                finish_cam_stime[slot] = gSeconds;
                switch (gc->place) {
                case 0: PlayGameSound(0x89, slot, 1, 1); break;
                case 1: PlayGameSound(0x93, slot, 1, 1); break;
                case 2: PlayGameSound(0x96, slot, 1, 1); break;
                default: PlayGameSound(0x94, slot, 1, 1); break;
                }
                gc->collidable = 1;
                gc->solid = 1;
                gc->collide_time = 0.0f;
                gc->collide_state |= 1;
                m->collidable = 1;
                m->solid = 1;
                m->appearance &= ~8;
            } else if (gNumLapsLeft != prevLapsLeft) {
                if (gNumLapsLeft == 1) PlayGameSound(0x88, slot, 1, 1);
                else ((int (*)(int, float, int, int, int, ...))AudPlaySequence)(0, 2.0f, 1, 0, 2, gNumLapsLeft + 0x5b, 0x8d);
            }
        }
    }
done_finish: ;
}

/* 0x8000a100, size 0x2b8 */
void CarReportsGameOver(int slot, float score)
{
    signed short i; /* r8 */
    signed short j; /* r10 */
    signed short index; /* r27 */
    signed short temp; /* r11 */
    signed short num_locked; /* r31 */
    signed short place[6]; /* stack */
    signed short num_humans_locked; /* r9 */
    float scores[6]; /* stack */
    float ftemp; /* f0 */
    gGameCar[slot].score = score;
    ftemp = 5999.999f + (int)(9.0f * gLapDistance);
    gGameCar[slot].place_locked = 1;
    num_humans_locked = 0;
    i = 0;
    num_locked = -1;
    for (; i < gNumActiveCars; i++) {
        index = model[i].slot;
        {
            if (gGameCar[index].place_locked == 1) {
                num_locked++;
                scores[num_locked] = gGameCar[index].score;
                place[(signed short)num_locked] = index;
                if (model[index].drone_type == 2 && (gRace != 2 || index == 0)) num_humans_locked++;
                if (gGameCar[index].dead > 0 && model[index].laps < gNumLaps) scores[num_locked] = ftemp - gGameCar[index].distance;
            }
        }
    }
    if (model[slot].drone_type == 2 && (gNumPlayers == num_humans_locked || (gRace == 2 && num_humans_locked == 1)) && !gEndGame) {
        gEndGame = 1;
        AddCountdownTime(1.0f - GetCountdownTimeRemaining());
    }
    for (i = 0; i < num_locked; i++) {
        for (j = 0; j < num_locked - i; j++) {
            if (scores[j] > scores[j + 1]) {
                temp = place[j];
                place[j] = place[j + 1];
                place[j + 1] = temp;
                ftemp = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = ftemp;
            }
        }
    }
    /* The original reads gNumCars in each dead-car assignment. */
    for (i = 0; i < num_locked + 1; i++) {
        if (gGameCar[place[i]].dead > 0 && model[place[i]].laps < gNumLaps) gGameCar[place[i]].place = *(volatile signed short *)&gNumCars - num_locked - 1 + i;
        else gGameCar[place[i]].place = i;
    }
    PlaceUnlockedCars();
}

/* 0x8000a3b8, size 0x40 */
float CalcRaceTime(void)
{
    float time; /* f1 */
    if (gRace == 2 && gTrackNum >= 0 && gTrackNum <= 5) return ghostRaceTimes[gTrackNum];
    return 600.0f;
}

