/* smoke.c - car smoke and water spray.
   Partial reconstruction from GC ELF/DWARF. Keep NonMatching. */
#include "rush/rush_protos.h"
#define MSL_NO_SQRTF
#include "msl_math_inline.h"

static float gCarMiddles[13] = {
    1.4f, 1.5f, 1.3f, 1.5f, 1.5f, 1.7f, 1.5f,
    1.5f, 1.6f, 1.5f, 2.5f, 1.4f, 1.3f
};
static float gCameraMatrix[4][4];
static float gCarWingSmokePos[6][2][3];
static unsigned short gFireTexs[6];
static unsigned int gCarSmokeDir[6];
static anon_ll_mat_58 gSmokeObjs[100];
anon_dbl_numElements_18 gSmokeList;
static unsigned char gCarBlastSmokeClr[6];

void InitSmoke(void)
{
    static char * const FireTexNames[6] = {
        "FIREGEN05", "FIREGEN03", "FIREGEN01",
        "FIREGEN07", "FIREGEN10", "FIREGEN10"
    };
    int i;
    InitStatLL(&gSmokeList, gSmokeObjs, sizeof(gSmokeObjs[0]), 100, 0);
    for (i = 0; i < 6; i++)
        gCarSmokeDir[i] = 0;
    for (i = 0; i < 6; i++)
        MBOX_FindTexture_WORK(FireTexNames[i], &gFireTexs[i], 0,
                             gNumModels - 1, 1);
}

void InitSmokeBlast(signed short slot)
{
    gCarSmokeDir[slot] &= ~0x20u;
}

void InitWingSmoke(signed short slot, signed short side)
{
    if (side)
        gCarSmokeDir[slot] &= ~0x8000u;
    else
        gCarSmokeDir[slot] &= ~0x4000u;
}

void StartWater(signed short slot, unsigned int tire)
{
    tCarData *car = &gGameCar[slot];
    float pos2[3];
    float offset;
    float offset2;
    float fmph;
    signed short mph = car->rwmph >> 2;
    signed short mph2 = fabs(mph);
    int j;
    anon_ll_mat_58 *smk;
    anon_mat3_mat4_30 mat;

    switch (tire) {
    case 0:
        if (gCarSmokeDir[slot] & 0x100u) return;
        gCarSmokeDir[slot] |= 0x100u;
        break;
    case 1:
        if (gCarSmokeDir[slot] & 0x200u) return;
        gCarSmokeDir[slot] |= 0x200u;
        break;
    case 2:
        if (gCarSmokeDir[slot] & 0x400u) return;
        gCarSmokeDir[slot] |= 0x400u;
        break;
    case 3:
        if (gCarSmokeDir[slot] & 0x800u) return;
        gCarSmokeDir[slot] |= 0x800u;
        break;
    }
    smk = GetStatLLE(&gSmokeList);
    if (!smk) return;
    smk->swTime = 1.0f / 30.0f;
    smk->data = tire << 17;
    smk->slot = slot;
    smk->data |= 0x10u;
    pos2[0] = car->dr_vel[0];
    pos2[1] = car->dr_vel[1];
    pos2[2] = car->dr_vel[2];
    NormalVector(pos2);
    LookInDir(pos2, &mat);
    CopyUV(mat.uvs, smk->mat.uvs);
    if (mph2 < 90) {
        fmph = mph2;
        if (mph < 0)
            offset2 = 0.25f + 0.75f * (fmph / 70.0f);
        else
            offset2 = 0.15f + 0.85f * (fmph / 90.0f);
        for (j = 0; j < 9; j++)
            ((float *)smk->mat.uvs)[j] *= offset2;
    }
    smk->mat.mat3.pos[0] = car->dr_tirepos[tire][0];
    smk->mat.mat3.pos[1] = car->dr_tirepos[tire][1];
    smk->mat.mat3.pos[2] = car->dr_tirepos[tire][2];
    smk->vel[0] = car->dr_uvs[0][0];
    smk->vel[1] = car->dr_uvs[0][1];
    smk->vel[2] = car->dr_uvs[0][2];
    offset = Random(1.0f) - 0.5f;
    smk->vel[0] *= offset;
    smk->vel[1] *= offset;
    smk->vel[2] *= offset;
    smk->mat.mat3.pos[0] += smk->vel[0];
    smk->mat.mat3.pos[1] += smk->vel[1];
    smk->mat.mat3.pos[2] += smk->vel[2];
    smk->mat.mat3.pos[1] += 1.25f;
    smk->tIdx = 0;
    smk->mbIdx = MBOX_NewObject(gObjList[516], &smk->mat, -1, 0x40000u);

    smk = GetStatLLE(&gSmokeList);
    if (!smk) return;
    smk->swTime = 1.0f / 30.0f;
    smk->data = tire << 17;
    smk->data |= 0x10u;
    smk->slot = slot;
    CopyUV(mat.uvs, smk->mat.uvs);
    if (mph2 < 90) {
        for (j = 0; j < 9; j++)
            ((float *)smk->mat.uvs)[j] *= offset2;
    }
    smk->mat.mat3.pos[0] = car->dr_tirepos[tire][0];
    smk->mat.mat3.pos[1] = car->dr_tirepos[tire][1];
    smk->mat.mat3.pos[2] = car->dr_tirepos[tire][2];
    smk->vel[0] = car->dr_uvs[0][0];
    smk->vel[1] = car->dr_uvs[0][1];
    smk->vel[2] = car->dr_uvs[0][2];
    offset = Random(1.0f) - 0.5f;
    smk->vel[0] *= offset;
    smk->vel[1] *= offset;
    smk->vel[2] *= offset;
    smk->mat.mat3.pos[0] += smk->vel[0];
    smk->mat.mat3.pos[1] += smk->vel[1];
    smk->mat.mat3.pos[2] += smk->vel[2];
    smk->mat.mat3.pos[1] += 1.25f;
    smk->tIdx = 4;
    smk->mbIdx = MBOX_NewObject(gObjList[516 + smk->tIdx], &smk->mat, -1, 0x40000u);
}

/* TODO: remaining original bodies. These stubs retain NonMatching status. */
void StartSmoke(signed short slot, unsigned int tire, int water, float *smkoffset) { }
void UpdateSmoke(void)
{
    signed short tire;
    int j;
    float t;
    float pos2[3];
    tCarData *gc;
    anon_ll_mat_58 *smk;
    anon_ll_mat_58 *next;
    signed short mph;
    signed short mph2;
    float fmph;
    anon_mat3_mat4_30 mat;

    for (smk = gSmokeList.head; smk; smk = next) {
        signed short slot = smk->slot;
        tire = (smk->data >> 17) & 15;
        next = smk->ll.next;
        gc = &gGameCar[slot];
        if (smk->data & 0x10u) {
            mph = gc->rwmph >> 2;
            mph2 = fabs(mph);
            if (gc->view < 2)
                MBOX_HideObject(smk->mbIdx, 1, 1 << gc->camera);
            else
                MBOX_ShowObject(smk->mbIdx, 0, 1 << gc->camera);
            if (model[slot].sviscode[tire] != 2 || mph2 < 10) {
                switch (tire) {
                case 0: gCarSmokeDir[slot] &= ~0x100u; break;
                case 1: gCarSmokeDir[slot] &= ~0x200u; break;
                case 2: gCarSmokeDir[slot] &= ~0x400u; break;
                case 3: gCarSmokeDir[slot] &= ~0x800u; break;
                }
                goto release;
            }
            pos2[0] = gc->dr_vel[0];
            pos2[1] = gc->dr_vel[1];
            pos2[2] = gc->dr_vel[2];
            NormalVector(pos2);
            LookInDir(pos2, &mat);
            CopyUV(mat.uvs, smk->mat.uvs);
            if (mph2 < 90) {
                fmph = mph2;
                if (mph < 0)
                    t = 0.25f + 0.75f * (fmph / 70.0f);
                else
                    t = 0.15f + 0.85f * (fmph / 90.0f);
                for (j = 0; j < 3; j++) {
                    smk->mat.uvs[j][0] *= t;
                    smk->mat.uvs[j][1] *= t;
                    smk->mat.uvs[j][2] *= t;
                }
            }
            smk->mat.mat3.pos[0] = gc->dr_tirepos[tire][0];
            smk->mat.mat3.pos[1] = gc->dr_tirepos[tire][1];
            smk->mat.mat3.pos[2] = gc->dr_tirepos[tire][2];
            smk->mat.mat3.pos[0] += smk->vel[0];
            smk->mat.mat3.pos[1] += smk->vel[1];
            smk->mat.mat3.pos[2] += smk->vel[2];
            smk->mat.mat3.pos[1] += 1.25f;
            smk->swTime -= gFrameSeconds;
            if (smk->swTime <= 0.0f) {
                ZOID_SetObjectDef(smk->mbIdx, gObjList[516 + smk->tIdx]);
                smk->tIdx++;
                if (smk->tIdx >= 8) smk->tIdx = 0;
                smk->swTime = 0.0333333f;
            }
        } else if (tire == 5) {
            if (!model[slot].crashflag) goto release;
            if (smk->tIdx == -1) {
                if (ObjInstanceList[smk->mbIdx].scale < smk->lifeTime)
                    ObjInstanceList[smk->mbIdx].scale += 0.04f;
                else
                    smk->tIdx = 0;
            } else {
                if (smk->tIdx <= 7)
                    ObjInstanceList[smk->mbIdx].scale -= 0.03f;
                else
                    ObjInstanceList[smk->mbIdx].scale += 0.03f;
                if (smk->tIdx >= 14)
                    smk->tIdx = 0;
                else
                    smk->tIdx++;
            }
            {
                float x = gc->dr_pos[0];
                float y = gc->dr_pos[1];
                float z = gc->dr_pos[2];
                float vx;
                float vz;
                float middle;
                x += gc->dr_uvs[0][0] * (vx = smk->vel[0]);
                y += gc->dr_uvs[0][1] * vx;
                z += gc->dr_uvs[0][2] * vx;
                x += gc->dr_uvs[2][0] * (vz = smk->vel[2]);
                y += gc->dr_uvs[2][1] * vz;
                z += gc->dr_uvs[2][2] * vz;
                x += gc->dr_uvs[1][0] * (middle = gCarMiddles[model[slot].body_type]);
                y += gc->dr_uvs[1][1] * middle;
                z += gc->dr_uvs[1][2] * middle;
                smk->mat.mat3.pos[0] = x;
                smk->mat.mat3.pos[1] = y;
                smk->mat.mat3.pos[2] = z;
            }
        } else {
            smk->lifeTime -= gFrameSeconds;
            if (smk->lifeTime <= 0.0f) {
release:
                ZOID_RemoveObject(smk->mbIdx);
                FreeStatLLE(&gSmokeList, smk);
                continue;
            }
            t = gFrameSeconds;
            if (smk->lifeTime < 1.0f) {
                if (ObjInstanceList[smk->mbIdx].color.c[3] >= 9)
                    ObjInstanceList[smk->mbIdx].color.c[3] -= 9;
            }
            if (tire == 4) {
                smk->mat.mat3.pos[0] += Random(0.3f) - 0.15f;
                smk->mat.mat3.pos[1] += Random(0.25f);
                smk->mat.mat3.pos[2] += Random(0.15f) - 0.075f;
                ObjInstanceList[smk->mbIdx].scale += 0.05f;
            }
            smk->swTime -= gFrameSeconds;
            if (smk->swTime <= 0.0f) {
                ZOID_SetObjectDef(smk->mbIdx, gObjList[507 + smk->tIdx]);
                if (smk->data & 1u) {
                    smk->tIdx++;
                    if (smk->tIdx >= 8) smk->tIdx = 0;
                } else {
                    smk->tIdx--;
                    if (smk->tIdx < 0) smk->tIdx = 7;
                }
                smk->swTime = 0.0333333f;
            }
            if (tire > 5)
                ObjInstanceList[smk->mbIdx].scale += 0.03f;
            else
                ObjInstanceList[smk->mbIdx].scale += 0.05f;
            smk->mat.mat3.pos[0] += smk->vel[0] * gFrameSeconds;
            smk->mat.mat3.pos[1] += smk->vel[1] * gFrameSeconds;
            smk->mat.mat3.pos[2] += smk->vel[2] * gFrameSeconds;
        }
    }
}
