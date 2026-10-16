/* headlights.c - headlight projections and surface animation. Status: matched (pending link verification).
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\headlights.c; assembly: build/2049LE/asm/GameSource/src/headlights.s */
#include "rush/rush_protos.h"
#include "msl_math_inline.h"

static void AnimateForwardLights(int idx, int slot, float *backpos, float *frontpos);
static void AnimateDownwardLights(int idx, int slot, float *backpos, float *frontpos);
static void AnimateHeadLights(tVisual *v, signed short op);

static float gHeadlightOffsets[13][3] = {
    0.0f, 0.47f, 7.15f, 1.824f, 1.38f, 5.732f, 1.16f, 0.68f, 5.58f, 2.09f, 0.56f, 6.03f, 1.41f,
    1.15f, 5.67f, 2.15f, 1.34f, 6.86f, 1.61f, 0.82f, 5.92f, 1.72f, 1.35f, 5.4f, 1.42f, 1.32f,
    4.56f, 2.08f, 0.59f, 7.43f, 2.28f, 2.17f, 7.23f, 2.26f, 1.18f, 5.27f, 2.21f, 1.3f, 5.1f
};

/* 0x8001b8d8, size 0x208 */
void StartHeadLights(signed short slot)
{
    tVisual *v = &gGameCar[slot].visuals[1]; /* r6 */
    anon_c_rgba_4 clr = {255,255,255,255}; /* stack */
    anon_mat3_mat4_30 *mat; /* r4 */

    if (gTrackNum == 3 || gTrackNum == 5) {
        v->func = AnimateHeadLights;
        v->slot = slot;
        mat = ZOID_NewMatrix();
        gCarParts[slot].headlight[0] = MBOX_NewObject(gObjList[528], mat, -1, 0x42000);
        ZOID_SetColor(gCarParts[slot].headlight[0], &clr);
        mat = ZOID_NewMatrix();
        gCarParts[slot].headlight[1] = MBOX_NewObject(gObjList[528], mat, -1, 0x42000);
        ZOID_SetColor(gCarParts[slot].headlight[1], &clr);
        ObjInstanceList[gCarParts[slot].headlight[0]].w = 1.35f;
        ObjInstanceList[gCarParts[slot].headlight[1]].w = 1.35f;
        mat = ZOID_NewMatrix();
        gCarParts[slot].headlight[2] = MBOX_NewObject(gObjList[529], mat, -1, 0x42000);
        ZOID_SetColor(gCarParts[slot].headlight[2], &clr);
        mat = ZOID_NewMatrix();
        gCarParts[slot].headlight[3] = MBOX_NewObject(gObjList[529], mat, -1, 0x42000);
        ZOID_SetColor(gCarParts[slot].headlight[3], &clr);
        ObjInstanceList[gCarParts[slot].headlight[2]].w = 1.25f;
        ObjInstanceList[gCarParts[slot].headlight[3]].w = 1.25f;
    } else {
        v->func = 0;
        gCarParts[slot].headlight[0] = 0;
        gCarParts[slot].headlight[1] = 0;
        gCarParts[slot].headlight[2] = 0;
        gCarParts[slot].headlight[3] = 0;
    }

}

/* 0x8001bae0, size 0x428 */
static void AnimateForwardLights(int idx, int slot, float *backpos, float *frontpos)
{
    signed short roadflag; /* r0 */
    tCarData *car; /* r26 */
    anon_mat3_mat4_30 *mat; /* r30 */
    float dotx; /* f31 */
    float dx; /* f4 */
    float dx3; /* f3 */
    float uvs[3][3]; /* stack */
    surftyp *surf; /* r0 */
    anon_c_rgba_4 clr = {255,255,255,243}; /* stack */

    car = &gGameCar[slot];
    surf = ShotToWorldCollision(backpos, frontpos, uvs, 0.5f, 1, 65535);
    if (surf) {
        mat = ZOID_FindMatrix(gCarParts[slot].headlight[idx]);
        mat->uvs[3][0] = frontpos[0];
        mat->uvs[3][1] = frontpos[1];
        mat->uvs[3][2] = frontpos[2];
        dotx = fabs(car->dr_uvs[2][0]*uvs[1][0] + car->dr_uvs[2][1]*uvs[1][1] + car->dr_uvs[2][2]*uvs[1][2]);
        roadflag = surf->flags & 15;
        if (dotx < 0.25f && roadflag != 5 && roadflag != 6) {
            MBOX_HideObject(gCarParts[slot].headlight[idx], 1, 15);
            return;
        }
        if (dotx != 0.0f) {
        
            mat->uvs[1][0] = uvs[1][0];
            mat->uvs[1][1] = uvs[1][1];
            mat->uvs[1][2] = uvs[1][2];
            mat->uvs[0][0] = uvs[1][1] * car->dr_uvs[2][2] - uvs[1][2] * car->dr_uvs[2][1];
            mat->uvs[0][1] = uvs[1][2] * car->dr_uvs[2][0] - uvs[1][0] * car->dr_uvs[2][2];
            mat->uvs[0][2] = uvs[1][0] * car->dr_uvs[2][1] - uvs[1][1] * car->dr_uvs[2][0];
            NormalVector(mat->uvs[0]);
            mat->uvs[2][0] = mat->uvs[0][1] * mat->uvs[1][2] - mat->uvs[0][2] * mat->uvs[1][1];
            dx3 = 1.0f + 3.0f * (1.0f-dotx);
            mat->uvs[2][1] = mat->uvs[0][2] * mat->uvs[1][0] - mat->uvs[0][0] * mat->uvs[1][2];
            mat->uvs[2][2] = mat->uvs[0][0] * mat->uvs[1][1] - mat->uvs[0][1] * mat->uvs[1][0];
            mat->uvs[2][0] *= dx3; mat->uvs[2][1] *= dx3; mat->uvs[2][2] *= dx3;

        } else CopyUV(uvs, mat->mat3.uvs);
        {
            float x = backpos[0]-frontpos[0];
            float y = backpos[1]-frontpos[1];
            float z = backpos[2]-frontpos[2];
            dx = sqrtf(x*x+y*y+z*z);
        }
        dx3 = 1.0f + dx/7.5f;
        mat->uvs[2][0] *= dx3; mat->uvs[2][1] *= dx3; mat->uvs[2][2] *= dx3;
        mat->uvs[0][0] *= dx3; mat->uvs[0][1] *= dx3; mat->uvs[0][2] *= dx3;
        dx = 1.0f - (dx-45.0f)/45.0f;
        if (dx < 0.0f) dx = 0.0f;
        else if (dx > 1.0f) dx = 1.0f;
        clr.c[3] = 233.0f * dx;
        ZOID_SetColor(gCarParts[slot].headlight[idx], &clr);
    } else MBOX_HideObject(gCarParts[slot].headlight[idx], 1, 15);

}

/* 0x8001bf08, size 0x424 */
static void AnimateDownwardLights(int idx, int slot, float *backpos, float *frontpos)
{
    tCarData *car; /* r30 */
    anon_mat3_mat4_30 *mat; /* r29 */
    float dotx; /* f31 */
    float dx; /* f1 */
    float dx3; /* f4 */
    float uvs[3][3]; /* stack */
    float scf; /* f30 */
    float scl; /* f29 */
    surftyp *surf; /* r0 */
    anon_c_rgba_4 clr = {255,255,255,243}; /* stack */

    car = &gGameCar[slot];
    surf = ShotToWorldCollision(backpos, frontpos, uvs, 1.0f, 1, 65535);
    if (surf) {
        scl = 0.0f; scf = 0.0f;
        mat = ZOID_FindMatrix(gCarParts[slot].headlight[idx]);
        mat->uvs[3][0] = frontpos[0]; mat->uvs[3][1] = frontpos[1]; mat->uvs[3][2] = frontpos[2];
        dotx = fabs(car->dr_uvs[2][0]*uvs[1][0] + car->dr_uvs[2][1]*uvs[1][1] + car->dr_uvs[2][2]*uvs[1][2]);
        if (dotx > 0.8) { MBOX_HideObject(gCarParts[slot].headlight[idx], 1, 15); return; }
        if (dotx != 0.0f) {
        
            mat->uvs[1][0] = uvs[1][0];
            mat->uvs[1][1] = uvs[1][1];
            mat->uvs[1][2] = uvs[1][2];
            mat->uvs[0][0] = uvs[1][1] * car->dr_uvs[2][2] - uvs[1][2] * car->dr_uvs[2][1];
            mat->uvs[0][1] = uvs[1][2] * car->dr_uvs[2][0] - uvs[1][0] * car->dr_uvs[2][2];
            mat->uvs[0][2] = uvs[1][0] * car->dr_uvs[2][1] - uvs[1][1] * car->dr_uvs[2][0];
            NormalVector(mat->uvs[0]);
            mat->uvs[2][0] = mat->uvs[0][1] * mat->uvs[1][2] - mat->uvs[0][2] * mat->uvs[1][1];
            scf += 1.0f + 7.0f * (1.0f-dotx);
            mat->uvs[2][1] = mat->uvs[0][2] * mat->uvs[1][0] - mat->uvs[0][0] * mat->uvs[1][2];
            mat->uvs[2][2] = mat->uvs[0][0] * mat->uvs[1][1] - mat->uvs[0][1] * mat->uvs[1][0];
            

        } else CopyUV(uvs, mat->mat3.uvs);
        {
            float x = backpos[0]-frontpos[0];
            float y = backpos[1]-frontpos[1];
            float z = backpos[2]-frontpos[2];
            dx = sqrtf(x*x+y*y+z*z);
        }
        dx3 = dx / 4.0f - 1.125f;
        if (dx3 < 1.0f) dx3 = 1.0f;
        scf += dx3; scl += dx3;
        mat->uvs[2][0] *= scf; mat->uvs[2][1] *= scf; mat->uvs[2][2] *= scf;
        mat->uvs[0][0] *= scl; mat->uvs[0][1] *= scl; mat->uvs[0][2] *= scl;
        dx = 1.0f - (dx-25.0f)/25.0f;
        if (dx < 0.0f) dx = 0.0f;
        else if (dx > 1.0f) dx = 1.0f;
        clr.c[3] = 243.0f * dx;
        ZOID_SetColor(gCarParts[slot].headlight[idx], &clr);
    } else MBOX_HideObject(gCarParts[slot].headlight[idx], 1, 15);

}

/* 0x8001c32c, size 0x654 */
static void AnimateHeadLights(tVisual *v, signed short op)
{
    signed short slot; /* r26 */
    tCarData *car; /* r25 */
    tModelData *m; /* r24 */
    float tmpb[3]; /* stack */
    float tmpf[3]; /* stack */
    float b[3]; /* stack */
    float f[3]; /* stack */
    float dx; /* f31 */

    slot = v->slot;
    car = &gGameCar[slot]; m = &model[slot];
    if (!op) { v->func = 0; v->objnum = -1; return; }
    if (m->resurrect.moving_state > -1 || gInvisibleCar || (car->appearance & 16) || m->hide_car) {
        MBOX_HideObject(gCarParts[slot].headlight[0],1,15);
        MBOX_HideObject(gCarParts[slot].headlight[1],1,15);
        MBOX_HideObject(gCarParts[slot].headlight[2],1,15);
        MBOX_HideObject(gCarParts[slot].headlight[3],1,15);
        return;
    }
    MBOX_ShowObject(gCarParts[slot].headlight[0],0,15);
    MBOX_ShowObject(gCarParts[slot].headlight[1],0,15);
    MBOX_ShowObject(gCarParts[slot].headlight[2],0,15);
    MBOX_ShowObject(gCarParts[slot].headlight[3],0,15);
    if (gInvisibleCar || (car->appearance & 8)) {
        ObjInstanceList[gCarParts[slot].headlight[0]].flags ^= 0x80000000;
        ObjInstanceList[gCarParts[slot].headlight[1]].flags ^= 0x80000000;
        ObjInstanceList[gCarParts[slot].headlight[2]].flags ^= 0x80000000;
        ObjInstanceList[gCarParts[slot].headlight[3]].flags ^= 0x80000000;
    } else {
        ObjInstanceList[gCarParts[slot].headlight[1]].flags &= ~0x80000000;
        ObjInstanceList[gCarParts[slot].headlight[2]].flags &= ~0x80000000;
        ObjInstanceList[gCarParts[slot].headlight[3]].flags &= ~0x80000000;
        /* The original clears extraparts[0] here, rather than headlight[0]. */
        ObjInstanceList[gCarParts[slot].extraparts[0]].flags &= ~0x80000000;
    }
    dx = m->body_type == 0 ? 0.0f : 1.0f;
    tmpb[0] = -gHeadlightOffsets[m->body_type][0]; tmpb[1] = 5.0f; tmpb[2] = gHeadlightOffsets[m->body_type][2] - 11.0f;
    WorldVector(tmpb,b,car->dr_uvs);
    b[0] += car->dr_pos[0]; b[1] += car->dr_pos[1]; b[2] += car->dr_pos[2];
    tmpf[0] = -gHeadlightOffsets[m->body_type][0]-dx; tmpf[1] = -12.0f; tmpf[2] = 24.0f + gHeadlightOffsets[m->body_type][2];
    WorldVector(tmpf,f,car->dr_uvs);
    f[0] += car->dr_pos[0]; f[1] += car->dr_pos[1]; f[2] += car->dr_pos[2];
    AnimateDownwardLights(0,slot,b,f);
    if (m->body_type != 0) {
        tmpb[0] = gHeadlightOffsets[m->body_type][0]; WorldVector(tmpb,b,car->dr_uvs);
        b[0] += car->dr_pos[0]; b[1] += car->dr_pos[1]; b[2] += car->dr_pos[2];
        tmpf[0] = dx + gHeadlightOffsets[m->body_type][0]; WorldVector(tmpf,f,car->dr_uvs);
        f[0] += car->dr_pos[0]; f[1] += car->dr_pos[1]; f[2] += car->dr_pos[2];
        AnimateDownwardLights(1,slot,b,f);
    }
    dx = m->body_type == 0 ? 0.0f : 13.0f;
    tmpb[0] = -gHeadlightOffsets[m->body_type][0]; tmpb[1] = 0.75f + gHeadlightOffsets[m->body_type][1]; tmpb[2] = gHeadlightOffsets[m->body_type][2];
    WorldVector(tmpb,b,car->dr_uvs);
    b[0] += car->dr_pos[0]; b[1] += car->dr_pos[1]; b[2] += car->dr_pos[2];
    tmpf[0] = -gHeadlightOffsets[m->body_type][0]-dx; tmpf[1] = 2.0f + gHeadlightOffsets[m->body_type][1]; tmpf[2] = 90.0f + gHeadlightOffsets[m->body_type][2];
    WorldVector(tmpf,f,car->dr_uvs);
    f[0] += car->dr_pos[0]; f[1] += car->dr_pos[1]; f[2] += car->dr_pos[2];
    AnimateForwardLights(2,slot,b,f);
    if (m->body_type != 0) {
        tmpb[0] = gHeadlightOffsets[m->body_type][0]; WorldVector(tmpb,b,car->dr_uvs);
        b[0] += car->dr_pos[0]; b[1] += car->dr_pos[1]; b[2] += car->dr_pos[2];
        tmpf[0] = dx + gHeadlightOffsets[m->body_type][0]; WorldVector(tmpf,f,car->dr_uvs);
        f[0] += car->dr_pos[0]; f[1] += car->dr_pos[1]; f[2] += car->dr_pos[2];
        AnimateForwardLights(3,slot,b,f);
    }

}

