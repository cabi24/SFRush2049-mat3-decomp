/* powerups.c - battle-mode power-ups (health, shield, invisibility) respawning on targets.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\powerups.c; assembly: build/2049LE/asm/GameSource/src/powerups.s */
#include "rush/rush_protos.h"

static void UpdateInvs(void);
static void UpdateBattleHealth(void);
static void UpdateBattleShield(void);
static void UpdateBattleInvs(void);

static tBattlePowerUps gBattlePowerUps;

/* 0x80033324, size 0x10 */
void PreInitBattlePowerUps(void)
{
    gBattlePowerUps.num = 0;
}

/* 0x80033334, size 0x18 */
void IncBattlePowerUps(void)
{
    gBattlePowerUps.num++;
}

/* 0x8003334c, size 0x44 */
void exitBattlePowerUps(void)
{
    if (gBattlePowerUps.tS) {
        DeallocateMemory(gBattlePowerUps.tS);
        gBattlePowerUps.tS = 0;
    }
}

/* 0x80033390, size 0x140 */
void InitBattlePowerUps(int restart)
{
    tTarget *t; /* r8 */
    float r; /* f0 */

    if (!restart) {
        gBattlePowerUps.tS = AllocateMemoryHeap(0, gBattlePowerUps.num * sizeof(tBattlePowerUpT));
    }
    {
        int n = 0; /* register temporary, not in the DWARF */

        for (t = gTargetList.head; t; t = t->ll.next) {
            if (t->Flags & 0x20) {
                gBattlePowerUps.tS[n].t = t;
                gBattlePowerUps.tS[n].last = 1;
                gBattlePowerUps.tS[n].type = 1;
                n++;
            }
        }
    }
    gBattlePowerUps.healthTLeft = 5.0f + Random(10.0f);
    gBattlePowerUps.invsTLeft = 15.0f + Random(45.0f);
    gBattlePowerUps.shieldTLeft = 15.0f + Random(45.0f);
    r = Random(1.0f);
    if (r > 0.5f) {
        gBattlePowerUps.shieldTLeft += 45.0f;
    } else {
        gBattlePowerUps.invsTLeft += 45.0f;
    }
}

/* 0x800334d0, size 0x148 */
static void UpdateInvs(void)
{
    int i; /* r0 */
    unsigned char invsColor[8] = { 0x10, 0x20, 0x40, 0x60, 0x70, 0x90, 0xb0, 0xb0 }; /* stack */

    for (i = 0; i < gNumPlayers; i++) {
        tCarData *car = &gGameCar[i];
        if (car->battle.defense.active & 1) {
            if (car->battle.fadeState == 3) {
                car->battle.fadeAlpha = invsColor[car->battle.shoot];
            } else if (car->battle.fadeState == 1) {
                car->battle.shoot = 0;
                if (car->battle.fadeTime > 0.0f) {
                    car->battle.fadeAlpha -= 8;
                    if (car->battle.fadeAlpha <= 16) {
                        car->battle.fadeAlpha = 16;
                        car->battle.fadeState = 3;
                    }
                    car->battle.fadeTime -= gFrameSeconds;
                } else {
                    car->battle.fadeState = 3;
                }
            } else {
                car->battle.shoot = 0;
                if (car->battle.fadeTime > 0.0f) {
                    if (car->battle.fadeAlpha + 8 > 255) {
                        car->battle.fadeAlpha = 255;
                    } else {
                        car->battle.fadeAlpha = car->battle.fadeAlpha + 8;
                    }
                    car->battle.fadeTime -= gFrameSeconds;
                } else {
                    car->battle.defense.active &= ~1;
                    car->battle.fadeState = 0;
                }
            }
        }
        if (car->battle.shoot > 0) {
            car->battle.shoot--;
        }
    }
}

/* 0x80033618, size 0x228 */
static void UpdateBattleHealth(void)
{
    int i; /* r8 */

    if (gBattlePowerUps.healthTLeft == -2.0f) {
        if (gBattlePowerUps.tS[gBattlePowerUps.hIdx].t->Flags & 2) {
            return;
        }
        gBattlePowerUps.healthTLeft = 5.0f + Random(10.0f);
        for (i = 0; i < gBattlePowerUps.num; i++) {
            if (gBattlePowerUps.tS[i].last == 4) {
                gBattlePowerUps.tS[i].last = 1;
            }
        }
        gBattlePowerUps.tS[gBattlePowerUps.hIdx].last = 4;
        gBattlePowerUps.tS[gBattlePowerUps.hIdx].type = 1;
    } else if ((gBattlePowerUps.healthTLeft -= gFrameSeconds) < 0.0f) {
        i = Random(gBattlePowerUps.num);
        while (gBattlePowerUps.tS[i].type != 1 || gBattlePowerUps.tS[i].last == 4) {
            if (++i >= gBattlePowerUps.num) {
                i = 0;
            }
        }
        gBattlePowerUps.hIdx = i;
        gBattlePowerUps.tS[i].last = 1;
        gBattlePowerUps.tS[i].type = 4;
        gBattlePowerUps.tS[i].t->Flags |= 2;
        gBattlePowerUps.tS[i].t->type = 0x2a1;
        ZOID_SetObjectDef(gBattlePowerUps.tS[i].t->dlIdx, gObjList[0x2a1]);
        MBOX_ShowObject(gBattlePowerUps.tS[i].t->dlIdx, 0, 15);
        gBattlePowerUps.healthTLeft = -2.0f;
    }
}

/* 0x80033840, size 0x228 */
static void UpdateBattleShield(void)
{
    int i; /* r8 */

    if (gBattlePowerUps.shieldTLeft == -2.0f) {
        if (gBattlePowerUps.tS[gBattlePowerUps.sIdx].t->Flags & 2) {
            return;
        }
        gBattlePowerUps.shieldTLeft = 90.0f + Random(60.0f);
        for (i = 0; i < gBattlePowerUps.num; i++) {
            if (gBattlePowerUps.tS[i].last == 3) {
                gBattlePowerUps.tS[i].last = 1;
            }
        }
        gBattlePowerUps.tS[gBattlePowerUps.sIdx].last = 3;
        gBattlePowerUps.tS[gBattlePowerUps.sIdx].type = 1;
    } else if ((gBattlePowerUps.shieldTLeft -= gFrameSeconds) < 0.0f) {
        i = Random(gBattlePowerUps.num);
        while (gBattlePowerUps.tS[i].type != 1 || gBattlePowerUps.tS[i].last == 3) {
            if (++i >= gBattlePowerUps.num) {
                i = 0;
            }
        }
        gBattlePowerUps.sIdx = i;
        gBattlePowerUps.tS[i].last = 1;
        gBattlePowerUps.tS[i].type = 3;
        gBattlePowerUps.tS[i].t->Flags |= 2;
        gBattlePowerUps.tS[i].t->type = 0x2a3;
        ZOID_SetObjectDef(gBattlePowerUps.tS[i].t->dlIdx, gObjList[0x2a3]);
        MBOX_ShowObject(gBattlePowerUps.tS[i].t->dlIdx, 0, 15);
        gBattlePowerUps.shieldTLeft = -2.0f;
    }
}

/* 0x80033a68, size 0x22c */
static void UpdateBattleInvs(void)
{
    int i; /* r8 */

    UpdateInvs();
    if (gBattlePowerUps.invsTLeft == -2.0f) {
        if (gBattlePowerUps.tS[gBattlePowerUps.iIdx].t->Flags & 2) {
            return;
        }
        gBattlePowerUps.invsTLeft = 90.0f + Random(60.0f);
        for (i = 0; i < gBattlePowerUps.num; i++) {
            if (gBattlePowerUps.tS[i].last == 2) {
                gBattlePowerUps.tS[i].last = 1;
            }
        }
        gBattlePowerUps.tS[gBattlePowerUps.iIdx].last = 2;
        gBattlePowerUps.tS[gBattlePowerUps.iIdx].type = 1;
    } else if ((gBattlePowerUps.invsTLeft -= gFrameSeconds) < 0.0f) {
        i = Random(gBattlePowerUps.num);
        while (gBattlePowerUps.tS[i].type != 1 || gBattlePowerUps.tS[i].last == 2) {
            if (++i >= gBattlePowerUps.num) {
                i = 0;
            }
        }
        gBattlePowerUps.iIdx = i;
        gBattlePowerUps.tS[i].last = 1;
        gBattlePowerUps.tS[i].type = 2;
        gBattlePowerUps.tS[i].t->Flags |= 2;
        gBattlePowerUps.tS[i].t->type = 0x2a2;
        ZOID_SetObjectDef(gBattlePowerUps.tS[i].t->dlIdx, gObjList[0x2a2]);
        MBOX_ShowObject(gBattlePowerUps.tS[i].t->dlIdx, 0, 15);
        gBattlePowerUps.invsTLeft = -2.0f;
    }
}

/* 0x80033c94, size 0x28 */
void UpdateBattlePowerUps(void)
{
    UpdateBattleHealth();
    UpdateBattleShield();
    UpdateBattleInvs();
}

