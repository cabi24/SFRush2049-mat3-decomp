/* skids.c - skid allocation, tire trails, packet updates and fading.
   Status: 10/10 bodies and a fresh private full link verified; pending parent review.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\skids.c; assembly: build/2049LE/asm/GameSource/src/skids.s */
#include "rush/rush_protos.h"
#include "msl_math_inline.h"

static void _ReleaseSkid(tSkid *s);
static void _UpdateSkid(tNewSkid *ns, signed short slot, unsigned int tire, tRGBA *color);
static tSkid *_GetSkid(tRGBA *color);
static void _StartSkid(tNewSkid *ns, signed short slot, unsigned int tire, tRGBA *color);
static void _StopSkid(tNewSkid *ns);
static void _ContinueSkid(tNewSkid *ns, signed short slot, unsigned int tire, tRGBA *color);

tRGBA gDirtColor = { 80, 56, 24, 128 };
static tTexture *gSkidTexDef;
static int inited;
tCarData *thecar;
static tNewSkid gNewSkid[6][4];
static tSkid gSkidArray[100];
static tStatList gSkidList;

/* 0x8003f9a0, size 0x40 */
static void _ReleaseSkid(tSkid *s)
{
    ZOID_RemovePoly(s->poly);
    FreeStatLLE(&gSkidList, s);
}

/* 0x8003f9e0, size 0x3e8 */
static void _UpdateSkid(tNewSkid *ns, signed short slot, unsigned int tire, tRGBA *color)
{
    tCarData *car = &gGameCar[slot];
    tModelData *m = &model[slot];
    tSkid *s = ns->skid;
    signed short i;
    float orth[3];
    float dir[3];
    float wid;
    anon_ob_tc_18 *vv;
    float xof;
    float yof;
    float zof;
    GetWorkingBufIdx();
    orth[0] = car->dr_tirepos[tire | 1][0] - car->dr_tirepos[tire & 2][0];
    orth[1] = car->dr_tirepos[tire | 1][1] - car->dr_tirepos[tire & 2][1];
    orth[2] = car->dr_tirepos[tire | 1][2] - car->dr_tirepos[tire & 2][2];
    orth[1] += m->reckon.suscomp[tire | 1] - m->reckon.suscomp[tire & 2];
    direction(orth, dir);
    for (i = 0; i < 3; i++) {
        wid = 0.75f * dir[i];
        ns->end[i] = car->dr_tirepos[tire][i];
        ns->vert[2][i] = ns->end[i] - wid;
        ns->vert[3][i] = ns->end[i] + wid;
    }
    ns->vert[2][1] += 0.5f;
    ns->vert[3][1] += 0.5f;
    s->lastTime = gSeconds;
    s->color.c[0] = color->c[0];
    s->color.c[1] = color->c[1];
    s->color.c[2] = color->c[2];
    s->color.c[3] = 128;
    ZOID_UpdatePoly(s->poly, 4, ns->vert, 0, &s->color, 0, 0);
    ZOID_SetPolyFlags(s->poly, 0x800);
    if (!thecar) thecar = car;
    vv = s->poly->verts;
    xof = (vv[0].ob[0] + vv[1].ob[0] + vv[2].ob[0] + vv[3].ob[0]) / 4.0f;
    yof = (vv[0].ob[1] + vv[1].ob[1] + vv[2].ob[1] + vv[3].ob[1]) / 4.0f;
    zof = (vv[0].ob[2] + vv[1].ob[2] + vv[2].ob[2] + vv[3].ob[2]) / 4.0f;
    if (xof - vv[0].ob[0] > 5.0f || xof - vv[0].ob[0] < -5.0f) return;
    if (yof - vv[0].ob[1] > 5.0f || yof - vv[0].ob[1] < -5.0f) return;
    if (zof - vv[0].ob[2] > 5.0f || zof - vv[0].ob[2] < -5.0f) return;
    for (i = 0; i < 3; i++) {
        s->polys[0].strips->vertexs[i].pos[0] = vv[i].ob[0];
        s->polys[0].strips->vertexs[i].pos[1] = vv[i].ob[1];
        s->polys[0].strips->vertexs[i].pos[2] = vv[i].ob[2];
        s->polys[0].strips->vertexs[i].color = color->rgba;
    }
    for (i = 3; i < 6; i++) {
        s->polys[0].strips->vertexs[i].pos[0] = vv[6 - (i == 5 ? 6 : i)].ob[0];
        s->polys[0].strips->vertexs[i].pos[1] = vv[6 - (i == 5 ? 6 : i)].ob[1];
        s->polys[0].strips->vertexs[i].pos[2] = vv[6 - (i == 5 ? 6 : i)].ob[2];
        s->polys[0].strips->vertexs[i].color = color->rgba;
    }
    currentz = 5000.0f;
    PS2DrawObject(s->polys, 0, -1);
}

/* 0x8003fdc8, size 0x3c8 */
static tSkid *_GetSkid(tRGBA *color)
{
    int i;
    tSkid *s;
    tSkid *fars;
    float d;
    float dsq;
    float fardsq;
    anon_ob_tc_18 *vv;
    float xof;
    float yof;
    float zof;
    s = GetStatLLE(&gSkidList);
    if (!s) {
        fars = gSkidList.head;
        fardsq = -1.0f;
        for (s = gSkidList.head; s->ll.next; s = s->ll.next) {
            dsq = 0.0f;
            for (i = 0; i < 3; i++) {
                d = s->pos[i] - gCamera[0].mat.mat3.pos[i];
                dsq += d * d;
            }
            if (dsq > fardsq) {
                fardsq = dsq;
                fars = s;
            }
        }
        _ReleaseSkid(fars);
        s = GetStatLLE(&gSkidList);
    }
    s->active = 0;
    s->color.c[0] = color->c[2];
    s->color.c[1] = color->c[1];
    s->color.c[2] = color->c[0];
    s->color.c[3] = 128;
    s->poly = ZOID_NewPoly(4, gDummyVtx, gTexList[10], color,
                          0x1200 | ((gGameState & 0x100) ? 1 : 15), 1);
    vv = s->poly->verts;
    xof = (vv[0].ob[0] + vv[1].ob[0] + vv[2].ob[0] + vv[3].ob[0]) / 4.0f;
    yof = (vv[0].ob[1] + vv[1].ob[1] + vv[2].ob[1] + vv[3].ob[1]) / 4.0f;
    zof = (vv[0].ob[2] + vv[1].ob[2] + vv[2].ob[2] + vv[3].ob[2]) / 4.0f;
    /* The GC's three range exits leave the return register untouched. Keep the
       original valueless returns; returning s here adds three register moves. */
    if (xof - vv[0].ob[0] > 5.0f || xof - vv[0].ob[0] < -5.0f) return;
    if (yof - vv[0].ob[1] > 5.0f || yof - vv[0].ob[1] < -5.0f) return;
    if (zof - vv[0].ob[2] > 5.0f || zof - vv[0].ob[2] < -5.0f) return;
    for (i = 0; i < 3; i++) {
        s->polys[0].strips->vertexs[i].pos[0] = vv[i].ob[0];
        s->polys[0].strips->vertexs[i].pos[1] = vv[i].ob[1];
        s->polys[0].strips->vertexs[i].pos[2] = vv[i].ob[2];
        s->polys[0].strips->vertexs[i].color = color->rgba;
        s->polys[0].strips->vertexs[i].uv[0] = vv[i].tc[0];
        s->polys[0].strips->vertexs[i].uv[1] = vv[i].tc[1];
    }
    for (i = 3; i < 6; i++) {
        s->polys[0].strips->vertexs[i].pos[0] = vv[6 - (i == 5 ? 6 : i)].ob[0];
        s->polys[0].strips->vertexs[i].pos[1] = vv[6 - (i == 5 ? 6 : i)].ob[1];
        s->polys[0].strips->vertexs[i].pos[2] = vv[6 - (i == 5 ? 6 : i)].ob[2];
        s->polys[0].strips->vertexs[i].color = color->rgba;
        s->polys[0].strips->vertexs[i].uv[0] = vv[6 - (i == 5 ? 6 : i)].tc[0];
        s->polys[0].strips->vertexs[i].uv[1] = vv[6 - (i == 5 ? 6 : i)].tc[1];
    }
    /* The original checks allocation failure only after filling the vertices. */
    if (!s->poly) {
        FreeStatLLE(&gSkidList, s);
        return 0;
    }
    return s;
}

/* 0x80040190, size 0xcc */
static void _StartSkid(tNewSkid *ns, signed short slot, unsigned int tire, tRGBA *color)
{
    tSkid *s;
    if ((s = _GetSkid(color)) != 0) {
        ns->skid = s;
        ns->lensq = 0.0f;
        _UpdateSkid(ns, slot, tire, color);
        ns->start[0] = ns->end[0];
        ns->start[1] = ns->end[1];
        ns->start[2] = ns->end[2];
        ns->vert[0][0] = ns->vert[3][0];
        ns->vert[0][1] = ns->vert[3][1];
        ns->vert[0][2] = ns->vert[3][2];
        ns->vert[1][0] = ns->vert[2][0];
        ns->vert[1][1] = ns->vert[2][1];
        ns->vert[1][2] = ns->vert[2][2];
        ZOID_UpdatePoly(s->poly, 4, ns->vert, 0, 0, 0, 0);
    }
}

/* 0x8004025c, size 0x58 */
static void _StopSkid(tNewSkid *ns)
{
    tSkid *s = ns->skid;
    s->pos[0] = 0.5f * (ns->start[0] + ns->end[0]);
    s->pos[1] = 0.5f * (ns->start[1] + ns->end[1]);
    s->pos[2] = 0.5f * (ns->start[2] + ns->end[2]);
    s->active = 1;
    ns->skid = 0;
}

/* 0x800402b4, size 0x308 */
static void _ContinueSkid(tNewSkid *ns, signed short slot, unsigned int tire, tRGBA *color)
{
    float lensq;
    float invlen;
    _UpdateSkid(ns, slot, tire, color);
    {
        float dx = gGameCar[slot].dr_tirepos[tire][0] - ns->start[0];
        float dy = gGameCar[slot].dr_tirepos[tire][1] - ns->start[1];
        float dz = gGameCar[slot].dr_tirepos[tire][2] - ns->start[2];
        lensq = dx * dx + dy * dy + dz * dz;
        if (ns->lensq > 0.0f) {
            float len = sqrtf(lensq);
            float v[3];
            float x, y, z;
            /* Stage the products as floats before subtracting, as in the GC. */
            v[0] = ns->dir[0] * len;
            v[1] = ns->dir[1] * len;
            v[2] = ns->dir[2] * len;
            x = ns->end[0] - v[0];
            y = ns->end[1] - v[1];
            z = ns->end[2] - v[2];
            if (x * x + y * y + z * z > 1.0f || lensq < ns->lensq) {
                _StopSkid(ns);
                _StartSkid(ns, slot, tire, color);
            }
        } else if (lensq > 0.1f) {
            invlen = 1.0f / sqrtf(lensq);
            ns->dir[0] = dx * invlen;
            ns->dir[1] = dy * invlen;
            ns->dir[2] = dz * invlen;
            ns->lensq = lensq;
        }
    }
}

/* 0x800405bc, size 0x270 */
void InitSkids(void)
{
    _tdstVertext vert[4];
    int i;
    int j;
    tTextureEx *pTexDef;
    GMem = 1;
    if (inited) ClearSkids();
    inited = 1;
    while (gSkidList.head) _ReleaseSkid(gSkidList.head);
    InitStatLL(&gSkidList, gSkidArray, sizeof(tSkid), 100, 1);
    memset(gNewSkid, 0, sizeof(gNewSkid));
    gSkidTexDef = &gTexDefPtrs[gTexList[10] >> 10].texDefs[gTexList[10] & 1023];
    for (j = 0; j < 100; j++) {
        for (i = 0; i < 4; i++) {
            vert[i].x = 0.0f;
            vert[i].y = 0.0f;
            vert[i].z = 0.0f;
            vert[i].nz = 0.0f;
            vert[i].ny = 0.0f;
            vert[i].nx = 0.0f;
            vert[i].u = 0.0f;
            vert[i].v = 0.0f;
            vert[i].r = 0;
            vert[i].g = 0;
            vert[i].b = 0;
            vert[i].a = 0;
        }
        gSkidArray[j].polys[0].numPack = 1;
        gSkidArray[j].polys[0].name = 0;
        gSkidArray[j].polys[0].tex = AllocateMemoryHeap(0, 4);
        gSkidArray[j].polys[0].tex[0] = AllocateMemoryHeap(0, 0x38);
        memcpy(gSkidArray[j].polys[0].tex[0], (void *)gSkidTexDef->strip, 0x38);
        pTexDef = (tTextureEx *)gSkidArray[j].polys[0].tex[0];
        pTexDef->ps2flags = 0;
        pTexDef->ps2flags |= 1;
        gSkidArray[j].polys[0].displaylist = 0;
        gSkidArray[j].polys[0].nStrips = 1;
        gSkidArray[j].polys[0].strips = AllocateMemoryHeap(0, 0x10);
        gSkidArray[j].polys[0].strips->nVertex = 6;
        gSkidArray[j].polys[0].strips->iTex = 0;
        gSkidArray[j].polys[0].strips->iHasAlpha = 0;
        gSkidArray[j].polys[0].strips->vertexs = AllocateMemoryHeap(0, 0xd8);
    }
    GMem = 0;
}

/* 0x8004082c, size 0x170 */
void UpdateSkids(int slot)
{
    static int appFlags[4] = { 4, 2, 0x400, 0x200 };
    int tire;
    int laston;
    tModelData *m = &model[slot];
    tNewSkid *ns;
    tRGBA *color;
    if (gNumPlayers > 1) return;
    ns = gNewSkid[slot];
    for (tire = 0; tire < 4; tire++, ns++) {
        int on = ((gGameCar[slot].appearance & appFlags[tire]) &&
                  m->resurrect.moving_state == -1 && m->sviscode[tire] != 8) &&
                 !(surf[m->tiresli[tire]].snd_flags & 0x30);
        laston = ns->skid != 0;
        color = m->sviscode[tire] == 0 ? &gShadowColor : &gDirtColor;
        if (!laston && on) _StartSkid(ns, slot, tire, color);
        else if (laston && on) _ContinueSkid(ns, slot, tire, color);
        else if (laston && !on) _StopSkid(ns);
    }
}

/* 0x8004099c, size 0x340 */
void AnimateSkids(void)
{
    tSkid *s;
    tSkid *nexts;
    float t;
    float fadetime;
    unsigned int xlu;
    int numfree;
    anon_ob_tc_18 *vv;
    int i;
    float xof;
    float yof;
    float zof;
    GetWorkingBufIdx();
    numfree = 0;
    for (s = gSkidList.store; s; s = s->ll.next) numfree++;
    fadetime = 2.0f + 2.0f * numfree / 100.0f;
    s = gSkidList.head;
    while (s) {
        if (!s->active) {
            s = s->ll.next;
            continue;
        }
        t = (gSeconds - s->lastTime) / fadetime;
        xlu = 255.0f * t + 127.0f;
        if (xlu < 255) {
            s->color.c[3] = 255 - xlu;
            if (s->color.c[3] > 128) s->color.c[3] = 128;
            ZOID_UpdatePoly(s->poly, 4, 0, 0, &s->color, 0, 0);
            vv = s->poly->verts;
            xof = (vv[0].ob[0] + vv[1].ob[0] + vv[2].ob[0] + vv[3].ob[0]) / 4.0f;
            yof = (vv[0].ob[1] + vv[1].ob[1] + vv[2].ob[1] + vv[3].ob[1]) / 4.0f;
            zof = (vv[0].ob[2] + vv[1].ob[2] + vv[2].ob[2] + vv[3].ob[2]) / 4.0f;
            if (xof - vv[0].ob[0] > 5.0f || xof - vv[0].ob[0] < -5.0f) return;
            if (yof - vv[0].ob[1] > 5.0f || yof - vv[0].ob[1] < -5.0f) return;
            if (zof - vv[0].ob[2] > 5.0f || zof - vv[0].ob[2] < -5.0f) return;
            for (i = 0; i < 3; i++) {
                s->polys[0].strips->vertexs[i].pos[0] = vv[i].ob[0];
                s->polys[0].strips->vertexs[i].pos[1] = vv[i].ob[1];
                s->polys[0].strips->vertexs[i].pos[2] = vv[i].ob[2];
            }
            for (i = 3; i < 6; i++) {
                s->polys[0].strips->vertexs[i].pos[0] = vv[6 - (i == 5 ? 6 : i)].ob[0];
                s->polys[0].strips->vertexs[i].pos[1] = vv[6 - (i == 5 ? 6 : i)].ob[1];
                s->polys[0].strips->vertexs[i].pos[2] = vv[6 - (i == 5 ? 6 : i)].ob[2];
            }
            currentz = 5000.0f;
            PS2DrawObject(s->polys, 0, -1);
            s = s->ll.next;
        } else {
            nexts = s->ll.next;
            _ReleaseSkid(s);
            s = nexts;
        }
    }
}

/* 0x80040cdc, size 0xa4 */
void ClearSkids(void)
{
    int j;
    if (inited) {
        WaitTillDMAEnd();
        for (j = 0; j < 100; j++) {
            DeallocateMemory(gSkidArray[j].polys[0].tex[0]);
            DeallocateMemory(gSkidArray[j].polys[0].tex);
            DeallocateMemory(gSkidArray[j].polys[0].strips->vertexs);
            DeallocateMemory(gSkidArray[j].polys[0].strips);
        }
        inited = 0;
    }
    while (gSkidList.head) _ReleaseSkid(gSkidList.head);
}
