/* streaks.c - light streaks trailing behind cars (fading polygon ribbons).
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\streaks.c; assembly: build/2049LE/asm/GameSource/src/streaks.s */
#include "rush/rush_protos.h"

unsigned char gStreakColors[4][4] = { 228, 190, 59, 168, 0, 169, 47, 168, 27, 56, 229, 216, 255, 255, 255, 96 };
static tStreak gStreakPolyArray[150];
static anon_dbl_numElements_18 gStreakPolyList;
static tStreak gStreakArray[50];
static anon_dbl_numElements_18 gStreakList;

static void _AnimateStreakPolys(void);
static void _NewStreakPoly(tStreak *s);

/* 0x80043284, size 0x5c */
void InitStreaks(void)
{
    InitStatLL(&gStreakList, gStreakArray, sizeof(tStreak), 50, 0);
    InitStatLL(&gStreakPolyList, gStreakPolyArray, sizeof(tStreakPoly), 150, 0);
}

/* 0x800432e0, size 0x140 */
static void _AnimateStreakPolys(void)
{
    tStreakPoly *sp; /* r31 */
    tStreakPoly *nextsp; /* r30 */
    for (sp = gStreakPolyList.head; sp; sp = nextsp) {
        nextsp = sp->ll.next;
        sp->lifeTime -= gFrameSeconds;
        if (sp->lifeTime <= 0.0f) {
        remove:
            ZOID_RemovePoly(sp->poly);
            FreeStatLLE(&gStreakPolyList, sp);
            continue;
        }
        if (sp->s->flags & 0x1000) {
            sp->lifeTime = 0.2f;
        }
        if (sp->lifeTime <= 0.2f) {
            sp->incTime -= gFrameSeconds;
            if (sp->incTime <= 0.0f) {
                if (sp->s->flags & 0x8) {
                    sp->incTime = 0.0f;
                } else {
                    sp->incTime = 0.0333333f;
                }
                if (sp->clr.c[3] < 23) {
                    goto remove;
                }
                sp->clr.c[3] -= 23;
                ZOID_UpdatePoly(sp->poly, 0, 0, 0, &sp->clr, 0, 0);
            }
        }
    }
}

/* 0x80043420, size 0x240 */
static void _NewStreakPoly(tStreak *s)
{
    tStreakPoly *sp; /* r31 */
    float xyz[4][3]; /* stack */
    float w; /* f2 */
    sp = GetStatLLE(&gStreakPolyList);
    if (!sp) {
        return;
    }
    if (s->flags & 1) {
        w = 0.75f;
        sp->lifeTime = 0.75f;
        sp->clr.c[0] = gStreakColors[0][0];
        sp->clr.c[1] = gStreakColors[0][1];
        sp->clr.c[2] = gStreakColors[0][2];
        sp->clr.c[3] = gStreakColors[0][3];
    } else if (s->flags & 2) {
        w = 0.5f;
        sp->lifeTime = 0.5f;
        sp->clr.c[0] = gStreakColors[1][0];
        sp->clr.c[1] = gStreakColors[1][1];
        sp->clr.c[2] = gStreakColors[1][2];
        sp->clr.c[3] = gStreakColors[1][3];
    } else if (s->flags & 4) {
        w = 0.35f;
        sp->lifeTime = 0.25f;
        sp->clr.c[0] = gStreakColors[2][0];
        sp->clr.c[1] = gStreakColors[2][1];
        sp->clr.c[2] = gStreakColors[2][2];
        sp->clr.c[3] = gStreakColors[2][3];
    } else {
        w = 0.025f;
        sp->lifeTime = 0.16f;
        sp->clr.c[0] = gStreakColors[3][0];
        sp->clr.c[1] = gStreakColors[3][1];
        sp->clr.c[2] = gStreakColors[3][2];
        sp->clr.c[3] = gStreakColors[3][3];
    }
    if (s->flags & 8) {
        sp->incTime = 0.0f;
    } else {
        sp->incTime = 0.0333333f;
    }
    sp->s = s;
    xyz[0][0] = s->backVerts[0][0];
    xyz[0][1] = s->backVerts[0][1];
    xyz[0][2] = s->backVerts[0][2];
    xyz[1][0] = s->backVerts[1][0];
    xyz[1][1] = s->backVerts[1][1];
    xyz[1][2] = s->backVerts[1][2];
    xyz[2][0] = s->newpos[0];
    xyz[2][1] = s->newpos[1];
    xyz[2][2] = s->newpos[2];
    xyz[3][0] = s->newpos[0];
    xyz[3][1] = s->newpos[1];
    xyz[3][2] = s->newpos[2];
    xyz[2][1] += w;
    xyz[3][1] -= w;
    sp->poly = ZOID_NewPoly(4, xyz, 0, &sp->clr, 0x120f, 0);
    if (sp->poly) {
        s->backVerts[1][0] = xyz[2][0];
        s->backVerts[1][1] = xyz[2][1];
        s->backVerts[1][2] = xyz[2][2];
        s->backVerts[0][0] = xyz[3][0];
        s->backVerts[0][1] = xyz[3][1];
        s->backVerts[0][2] = xyz[3][2];
        s->lastpos[0] = s->newpos[0];
        s->lastpos[1] = s->newpos[1];
        s->lastpos[2] = s->newpos[2];
    } else {
        FreeStatLLE(&gStreakPolyList, sp);
    }
}

/* 0x80043660, size 0x14c */
tStreak *NewStreak(int type, float *initpos)
{
    tStreak *s; /* r3 */
    float w; /* f31 */
    s = GetStatLLE(&gStreakList);
    if (!s) {
        return 0;
    }
    s->delay = 0.1f;
    switch (type) {
    case 0:
        s->flags = 1;
        w = 0.75f;
        break;
    case 1:
        s->flags = 2;
        w = 0.5f;
        break;
    case 2:
        s->flags = 4;
        w = 0.35f;
        break;
    case 3:
        s->delay = 0.0f;
        s->flags = 8;
        w = 0.025f;
        break;
    }
    s->newpos[0] = initpos[0];
    s->newpos[1] = initpos[1];
    s->newpos[2] = initpos[2];
    s->lastpos[0] = initpos[0];
    s->lastpos[1] = initpos[1];
    s->lastpos[2] = initpos[2];
    s->backVerts[0][0] = initpos[0];
    s->backVerts[0][1] = initpos[1];
    s->backVerts[0][2] = initpos[2];
    s->backVerts[1][0] = initpos[0];
    s->backVerts[1][1] = initpos[1];
    s->backVerts[1][2] = initpos[2];
    s->backVerts[0][1] -= w;
    s->backVerts[1][1] += w;
    return s;
}

/* 0x800437ac, size 0x158 */
void UpdateStreaks(void)
{
    tStreak *s; /* r4 */
    tStreak *nexts; /* r31 */
    float w; /* f1 */
    _AnimateStreakPolys();
    for (s = gStreakList.head; s; s = nexts) {
        nexts = s->ll.next;
        if (s->flags & 0x1000) {
            s->flags &= ~0x1000;
            FreeStatLLE(&gStreakList, s);
            continue;
        }
        if (s->delay >= 0.0f) {
            s->delay -= gFrameSeconds;
            if (!(s->flags & 0x2000) && (s->flags & 0x10)) {
                s->backVerts[0][0] = s->newpos[0];
                s->backVerts[0][1] = s->newpos[1];
                s->backVerts[0][2] = s->newpos[2];
                s->backVerts[1][0] = s->newpos[0];
                s->backVerts[1][1] = s->newpos[1];
                s->backVerts[1][2] = s->newpos[2];
                if (s->flags & 1) {
                    w = 0.75f;
                } else if (s->flags & 2) {
                    w = 0.5f;
                } else if (s->flags & 4) {
                    w = 0.35f;
                } else {
                    w = 0.025f;
                }
                s->backVerts[0][1] -= w;
                s->backVerts[1][1] += w;
            }
        } else if (s->flags & 0x10) {
            if (!(s->flags & 0x2000)) {
                s->flags |= 0x2000;
            }
            s->delay = 0.0333333f;
            s->flags &= ~0x10;
            _NewStreakPoly(s);
        }
    }
}

