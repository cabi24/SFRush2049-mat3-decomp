/* Stunt tracking and scoring. Reconstructed from GC ELF/DWARF. */
#include "rush/rush_protos.h"
#define MSL_NO_SQRTF
#include "msl_math_inline.h"

static void ResetStunts(void);
static void _AddStunt(int player, int type);
static void _CheckAirTime(int p);
static void _CheckRoll(int p);
static void _CheckSpin(int p);
static void _CheckFlip(int p);
static void _CheckMulti(int p);
static void _Check2WheelSide(int p);
static void _Check2WheelFront(int p);
static void _Check2WheelBack(int p);

static int chW0, chW1, chW2, j, l, r, b, f, okToStart;
static float eTime;
static float GCabsW[3];
static tStuntInfo gStuntInfo[4];

void InitStunt(void)
{
    memset(gStuntPlayer, 0, sizeof(gStuntPlayer));
    ResetStunts();
    gStuntsDone = 0;
    eTime = 0.0f;
}

static void ResetStunts(void)
{
    int playerIdx;
    for (playerIdx = 0; playerIdx < 4; playerIdx++) {
        memset(&gStuntInfo[playerIdx], 0, sizeof(tStuntInfo));
        gStuntInfo[playerIdx].active |= 0x15000000;
    }
}

void CrashedStunt(int player)
{
    gStuntPlayer[player].crash = 1;
    if (gStuntPlayer[player].numstunts)
        _AddStunt(player, 11);
    memset(&gStuntInfo[player], 0, sizeof(tStuntInfo));
    gStuntInfo[player].active |= 0x15000000;
}

static void _AddStunt(int player, int type)
{
    int p;
    int pt;
    tStuntPlayer *plyr = &gStuntPlayer[player];
    plyr->done = 0;
    plyr->show = 1;
    plyr->time = gModelTime;
    if (type != 11) {
        if (!plyr->stunts[type] && plyr->numstunts > 0) {
            plyr->bonus++;
            plyr->showBonus++;
        }
        plyr->stunts[type]++;
    }
    if (type != plyr->lastStuntType) {
        plyr->lastStuntType = type;
        plyr->numstunts++;
    }
    /* The original has no default; its callers use the enumerated stunt types. */
    switch (type) {
    case 0: case 1: case 2: case 3: case 4: case 7: case 8: pt = 5; break;
    case 5: case 6: pt = 3; break;
    case 9: pt = 5; break;
    case 11: plyr->done = 1; pt = 0; break;
    }
    plyr->points += pt;
    plyr->bonusHud = plyr->bonus;
    if (type == 11) {
        if (!plyr->crash) {
            p = 0;
            for (pt = 0; pt < 10; pt++) {
                if (plyr->stunts[pt] > 0) p++;
                plyr->totalStunts[pt] += plyr->stunts[pt];
            }
            if ((plyr->stunts[0] || plyr->stunts[1] || plyr->stunts[2] ||
                 plyr->stunts[3] || plyr->stunts[4]) && plyr->wingTime <= 1.0f &&
                (p == 1 || (p == 2 && plyr->stunts[9]))) {
                if (plyr->stunts[4]) {
                    plyr->bonus += 15;
                    plyr->doneBonus = 11;
                } else if (plyr->stunts[0] || plyr->stunts[1]) {
                    plyr->bonus += 25;
                    plyr->doneBonus = 13;
                } else {
                    plyr->bonus += 25;
                    plyr->doneBonus = 12;
                }
            } else if (plyr->stunts[9] || plyr->stunts[5] || plyr->stunts[0] ||
                       plyr->stunts[1] || plyr->stunts[2] || plyr->stunts[3] ||
                       plyr->stunts[4]) {
                if (plyr->wingTime == 0.0f) {
                    plyr->bonus += 7;
                    plyr->wingBonus = 1;
                } else if (plyr->wingTime <= 3.0f) {
                    plyr->bonus += 2;
                    plyr->wingBonus = 2;
                }
            }
            if (p >= 3) {
                plyr->doneBonus = p;
                switch (p) {
                case 3: plyr->bonus += 10; break;
                case 4: plyr->bonus += 20; break;
                case 5: plyr->bonus += 40; break;
                case 6: plyr->bonus += 60; break;
                case 7: plyr->bonus += 80; break;
                case 8: plyr->bonus += 100; break;
                case 9: plyr->bonus += 500; break;
                case 10: plyr->bonus += 1000; break;
                }
            }
            if (plyr->bonus > 1) plyr->points *= plyr->bonus;
            plyr->totalPoints += plyr->points;
            if (plyr->maxStuntPoints < plyr->points)
                plyr->maxStuntPoints = plyr->points;
        }
        plyr->doneBonusHud = plyr->doneBonus;
        plyr->pointsHud = plyr->points;
        plyr->wingBonusHud = plyr->wingBonus;
        for (pt = 0; pt < 10; pt++) {
            plyr->stuntsHud[pt] = plyr->stunts[pt];
            plyr->stunts[pt] = 0;
        }
        plyr->wingTime = 0.0f;
        plyr->doneBonus = 0;
        plyr->wingBonus = 0;
        plyr->bonus = 1;
        plyr->showBonus = 0;
        plyr->numstunts = 0;
        plyr->points = 0;
        memset(&gStuntInfo[player], 0, sizeof(tStuntInfo));
        gStuntInfo[player].active |= 0x15000000;
    }
}

static void _CheckAirTime(int p)
{
    tCarData *gc = &gGameCar[p];
    tStuntInfo *plyrInfo = &gStuntInfo[p];
    float tmp;
    if (plyrInfo->active & 0x100) {
        if (j >= 1 || (gc->appearance & 0x100000)) {
            plyrInfo->active &= ~0x100;
            plyrInfo->time[0].endTime = gModelTime;
            tmp = plyrInfo->time[0].endTime - plyrInfo->time[0].startTime;
            while (tmp > 5.0f) {
                _AddStunt(p, 9);
                tmp -= 1.0f;
            }
        }
    } else if (j == 0) {
        plyrInfo->active |= 0x100;
        plyrInfo->time[0].startTime = gModelTime;
        plyrInfo->time[0].endTime = -1.0f;
    }
}

/* The float narrowing after fabs and the sum grouping are intentional. */

static void _CheckRoll(int p)
{
    tCarData *gc = &gGameCar[p];
    tStuntInfo *plyrInfo = &gStuntInfo[p];
    if (plyrInfo->active & 0x20) {
        if (chW2 || j >= 3)
            plyrInfo->active &= ~0x20;
        else {
            plyrInfo->stuntW[0][0] += gc->stuntW[0];
            plyrInfo->stuntW[0][1] += gc->stuntW[1];
            plyrInfo->stuntW[0][2] += gc->stuntW[2];
        }
        {
            float x = (float)fabs(plyrInfo->stuntW[0][0]);
            float y = (float)fabs(plyrInfo->stuntW[0][1]);
            float z = (float)fabs(plyrInfo->stuntW[0][2]);
            if (x > 1.2566371f || y > 0.4712389f) {
                plyrInfo->active &= ~0x20;
                return;
            }
            if ((z > 2.5132742f && z < 3.7699113f) ||
                ((chW2 || j >= 1 || (gc->appearance & 0x100000)) && z > 1.727876f)) {
                plyrInfo->active &= ~0x80;
                if (plyrInfo->stuntW[0][2] > 0.0f) {
                    _AddStunt(p, 3);
                    plyrInfo->stuntW[0][2] += -3.1415927f;
                } else {
                    _AddStunt(p, 2);
                    plyrInfo->stuntW[0][2] += 3.1415927f;
                }
                plyrInfo->stuntW[0][0] = plyrInfo->stuntW[0][1] = 0.0f;
            }
        }
    } else if (okToStart && GCabsW[2] > GCabsW[1] &&
               GCabsW[2] > GCabsW[0]) {
        plyrInfo->active |= 0x20;
        plyrInfo->stuntW[0][0] = gc->stuntW[0];
        plyrInfo->stuntW[0][1] = gc->stuntW[1];
        plyrInfo->stuntW[0][2] = gc->stuntW[2];
    }
}

static void _CheckSpin(int p)
{
    tCarData *gc = &gGameCar[p];
    tStuntInfo *plyrInfo = &gStuntInfo[p];
    if (plyrInfo->active & 0x10) {
        if (chW1 || j >= 3)
            plyrInfo->active &= ~0x10;
        else {
            plyrInfo->stuntW[2][0] += gc->stuntW[0];
            plyrInfo->stuntW[2][1] += gc->stuntW[1];
            plyrInfo->stuntW[2][2] += gc->stuntW[2];
        }
        {
            float x = (float)fabs(plyrInfo->stuntW[2][0]);
            float y = (float)fabs(plyrInfo->stuntW[2][1]);
            float z = (float)fabs(plyrInfo->stuntW[2][2]);
            if (x > 0.4712389f || z > 0.4712389f) {
                plyrInfo->active &= ~0x10;
                return;
            }
            if ((y > 2.5132742f && y < 3.7699113f) ||
                ((chW1 || j >= 1 || (gc->appearance & 0x100000)) && y > 1.727876f)) {
                plyrInfo->active &= ~0x80;
                _AddStunt(p, 4);
                plyrInfo->stuntW[2][1] += plyrInfo->stuntW[2][1] > 0.0f ? -3.1415927f : 3.1415927f;
                plyrInfo->stuntW[2][0] = plyrInfo->stuntW[2][2] = 0.0f;
            }
        }
    } else if (okToStart && GCabsW[1] > GCabsW[0] &&
               GCabsW[1] > GCabsW[2]) {
        plyrInfo->active |= 0x10;
        plyrInfo->stuntW[2][0] = gc->stuntW[0];
        plyrInfo->stuntW[2][1] = gc->stuntW[1];
        plyrInfo->stuntW[2][2] = gc->stuntW[2];
    }
}

static void _CheckFlip(int p)
{
    tCarData *gc = &gGameCar[p];
    tStuntInfo *plyrInfo = &gStuntInfo[p];
    if (plyrInfo->active & 0x40) {
        if (chW0 || j >= 3)
            plyrInfo->active &= ~0x40;
        else {
            plyrInfo->stuntW[1][0] += gc->stuntW[0];
            plyrInfo->stuntW[1][1] += gc->stuntW[1];
            plyrInfo->stuntW[1][2] += gc->stuntW[2];
        }
        {
            float y = (float)fabs(plyrInfo->stuntW[1][0]);
            float x = (float)fabs(plyrInfo->stuntW[1][1]);
            float z = (float)fabs(plyrInfo->stuntW[1][2]);
            if (x > 0.4712389f || z > 0.4712389f) {
                plyrInfo->active &= ~0x40;
                return;
            }
            if ((y > 2.5132742f && y < 3.7699113f) ||
                ((chW0 || j >= 1 || (gc->appearance & 0x100000)) && y > 1.727876f)) {
                plyrInfo->active &= ~0x80;
                if (plyrInfo->stuntW[1][0] > 0.0f) {
                    _AddStunt(p, 0);
                    plyrInfo->stuntW[1][0] += -3.1415927f;
                } else {
                    _AddStunt(p, 1);
                    plyrInfo->stuntW[1][0] += 3.1415927f;
                }
                plyrInfo->stuntW[1][1] = plyrInfo->stuntW[1][2] = 0.0f;
            }
        }
    } else if (okToStart && GCabsW[0] > GCabsW[1] &&
               GCabsW[0] > GCabsW[2]) {
        plyrInfo->active |= 0x40;
        plyrInfo->stuntW[1][0] = gc->stuntW[0];
        plyrInfo->stuntW[1][1] = gc->stuntW[1];
        plyrInfo->stuntW[1][2] = gc->stuntW[2];
    }
}

static void _CheckMulti(int p)
{
    tCarData *gc = &gGameCar[p];
    tStuntInfo *plyrInfo = &gStuntInfo[p];
    int c;
    if (plyrInfo->active & 0x80) {
        if (j >= 3) plyrInfo->active &= ~0x80;
        else if (j == 0) {
            plyrInfo->stuntW[3][0] += fabs(gc->stuntW[0]);
            plyrInfo->stuntW[3][1] += fabs(gc->stuntW[1]);
            plyrInfo->stuntW[3][2] += fabs(gc->stuntW[2]);
        }
        c = 0;
        if (plyrInfo->stuntW[3][0] > 0.4712389f) c++;
        if (plyrInfo->stuntW[3][1] > 0.4712389f) c++;
        if (plyrInfo->stuntW[3][2] > 0.4712389f) c++;
        if (c >= 2 && (plyrInfo->stuntW[3][2] +
                      (plyrInfo->stuntW[3][0] + plyrInfo->stuntW[3][1])) >= 3.1415927f) {
            plyrInfo->active &= ~0x70;
            _AddStunt(p, 5);
            plyrInfo->stuntW[3][0] = 0.0f;
            plyrInfo->stuntW[3][1] = 0.0f;
            plyrInfo->stuntW[3][2] = 0.0f;
        }
    } else if (okToStart) {
        plyrInfo->active |= 0x80;
        plyrInfo->stuntW[3][0] = fabs(gc->stuntW[0]);
        plyrInfo->stuntW[3][1] = fabs(gc->stuntW[1]);
        plyrInfo->stuntW[3][2] = fabs(gc->stuntW[2]);
    }
}

static void _Check2WheelSide(int p)
{
    tStuntInfo *plyrInfo = &gStuntInfo[p];
    float tmp;
    if (plyrInfo->active & 1) {
        if (j >= 3 || j <= 1) {
            plyrInfo->active &= ~1;
            plyrInfo->time[1].endTime = gModelTime;
            tmp = plyrInfo->time[1].endTime - plyrInfo->time[1].startTime;
            if (tmp > 0.25f) {
                tmp -= 0.25f;
                while (tmp > 0.0f) {
                    _AddStunt(p, 6);
                    tmp -= 0.15f;
                }
            }
        }
    } else if (j == 2 && (l == 2 || r == 2)) {
        plyrInfo->active |= 1;
        plyrInfo->time[1].startTime = gModelTime;
        plyrInfo->time[1].endTime = -1.0f;
    }
}

static void _Check2WheelFront(int p)
{
    tModelData *m = &model[p];
    tStuntInfo *plyrInfo = &gStuntInfo[p];
    float tmp;
    if (plyrInfo->active & 2) {
        if (j >= 3 || b > 0 || m->airdist[0] > 0.75f || m->airdist[1] > 0.75f || m->airdist[2] < 1.0f || m->airdist[3] < 1.0f) {
            plyrInfo->active &= ~2;
            plyrInfo->time[2].endTime = gModelTime;
            tmp = plyrInfo->time[2].endTime - plyrInfo->time[2].startTime;
            if (tmp > 0.25f) {
                tmp -= 0.25f;
                while (tmp > 0.0f) {
                    _AddStunt(p, 8);
                    tmp -= 0.15f;
                }
            }
        }
    } else if ((j == 2 && f == 2) || (((m->airdist[0] < 0.75f || m->airdist[1] < 0.75f) && m->airdist[2] > 2.5f) || m->airdist[3] > 2.5f)) {
        plyrInfo->active |= 2;
        plyrInfo->time[2].startTime = gModelTime;
        plyrInfo->time[2].endTime = -1.0f;
    }
}

static void _Check2WheelBack(int p)
{
    tModelData *m = &model[p];
    tStuntInfo *plyrInfo = &gStuntInfo[p];
    float tmp;
    if (plyrInfo->active & 4) {
        if (j >= 3 || f > 0 || m->airdist[2] > 0.75f || m->airdist[3] > 0.75f || m->airdist[0] < 1.0f || m->airdist[1] < 1.0f) {
            plyrInfo->active &= ~4;
            plyrInfo->time[3].endTime = gModelTime;
            tmp = plyrInfo->time[3].endTime - plyrInfo->time[3].startTime;
            if (tmp > 0.25f) {
                tmp -= 0.25f;
                while (tmp > 0.0f) {
                    _AddStunt(p, 7);
                    tmp -= 0.15f;
                }
            }
        }
    } else if ((j == 2 && b == 2) || ((m->airdist[2] < 0.75f || m->airdist[3] < 0.75f) && m->airdist[0] > 2.5f && m->airdist[1] > 2.5f)) {
        plyrInfo->active |= 4;
        plyrInfo->time[3].startTime = gModelTime;
        plyrInfo->time[3].endTime = -1.0f;
    }
}

/* Keep the flag stores and explicit reloads in the order observed in the GC ELF. */
void CheckForStunt(void)
{
    int i;
    int x;
    if (gCoastFlag) {
        x = 1;
        for (i = 0; i < gNumPlayers; i++) {
            if (gStuntInfo[i].active & 0x1f7) {
                eTime = 0.0f;
                x = 0;
            }
        }
        if (x) {
            eTime += gFrameSeconds;
            if (eTime >= 2.69f) gStuntsDone = 1;
        }
    }
    for (i = 0; i < gNumPlayers; i++) {
        tModelData *m = &model[i];
        tCarData *gc = &gGameCar[i];
        tStuntInfo *info = &gStuntInfo[i];
        tStuntPlayer *plyr = &gStuntPlayer[i];

        if ((info->active & 0x1f0) && m->wing)
            plyr->wingTime += gFrameSeconds;
        if ((info->active & 0xc08fffff) &&
            (gc->crash_state || m->resurrect.moving_state != -1)) {
            /* volatile accesses: the target reloads info->active for every read-modify-write in this block instead of keeping it in a register */
            (*(volatile unsigned int *)&info->active) = 0x15000000;
            continue;
        }
        j = l = r = f = b = 0;
        if (m->sviscode[0] != 8) { j++; l++; f++; }
        if (m->sviscode[1] != 8) { j++; r++; f++; }
        if (m->sviscode[2] != 8) { j++; l++; b++; }
        if (m->sviscode[3] != 8) { j++; r++; b++; }
        okToStart = j < 3 && !gc->crash_state && m->resurrect.moving_state == -1;
        GCabsW[0] = fabs(gc->stuntW[0]);
        GCabsW[1] = fabs(gc->stuntW[1]);
        GCabsW[2] = fabs(gc->stuntW[2]);
        chW0 = chW1 = chW2 = 0;
        if ((info->active & 0x1000000) && gc->stuntW[0] < 0.0f) {
            (*(volatile unsigned int *)&info->active) = info->active & ~0x1000000;
            (*(volatile unsigned int *)&info->active) = info->active | 0x2000000;
            chW0 = 1;
        } else if ((info->active & 0x2000000) && gc->stuntW[0] >= 0.0f) {
            x = (*(volatile unsigned int *)&info->active);
            chW0 = 1;
            (*(volatile unsigned int *)&info->active) = x & ~0x2000000;
            (*(volatile unsigned int *)&info->active) = info->active | 0x1000000;
        }
        if ((info->active & 0x4000000) && gc->stuntW[1] < 0.0f) {
            (*(volatile unsigned int *)&info->active) = info->active & ~0x4000000;
            (*(volatile unsigned int *)&info->active) = info->active | 0x8000000;
            chW1 = 1;
        } else if ((info->active & 0x8000000) && gc->stuntW[1] >= 0.0f) {
            x = (*(volatile unsigned int *)&info->active);
            chW1 = 1;
            (*(volatile unsigned int *)&info->active) = x & ~0x8000000;
            (*(volatile unsigned int *)&info->active) = info->active | 0x4000000;
        }
        if ((info->active & 0x10000000) && gc->stuntW[2] < 0.0f) {
            (*(volatile unsigned int *)&info->active) = info->active & ~0x10000000;
            (*(volatile unsigned int *)&info->active) = info->active | 0x20000000;
            chW2 = 1;
        } else if ((info->active & 0x20000000) && gc->stuntW[2] >= 0.0f) {
            x = (*(volatile unsigned int *)&info->active);
            chW2 = 1;
            (*(volatile unsigned int *)&info->active) = x & ~0x20000000;
            (*(volatile unsigned int *)&info->active) = info->active | 0x10000000;
        }
        _CheckRoll(i);
        _CheckSpin(i);
        _CheckFlip(i);
        _CheckMulti(i);
        _CheckAirTime(i);
        _Check2WheelSide(i);
        _Check2WheelFront(i);
        _Check2WheelBack(i);
        if (info->active & 0x1f7) {
            info->endTime = -1.0f;
        } else if (j >= 3) {
            if (!plyr->numstunts) plyr->wingTime = 0.0f;
            if (info->endTime == -1.0f && plyr->numstunts)
                info->endTime = gModelTime;
        }
        if (j >= 3 && plyr->numstunts && info->time[0].endTime != -1.0f &&
            gModelTime - info->time[0].endTime > 1.0f) {
            info->time[0].endTime = -1.0f;
            _AddStunt(i, 11);
        }
        gc->appearance &= ~0x100000;
    }
}
