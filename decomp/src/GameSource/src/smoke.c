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
    smk->swTime = 0.0333333f; /* the original literal: 0x3D088880, not 1/30 (0x3D088889) */
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
            (&smk->mat.uvs[0][0])[j] *= offset2;
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
    smk->swTime = 0.0333333f; /* the original literal: 0x3D088880, not 1/30 (0x3D088889) */
    smk->data = tire << 17;
    smk->data |= 0x10u;
    smk->slot = slot;
    CopyUV(mat.uvs, smk->mat.uvs);
    if (mph2 < 90) {
        for (j = 0; j < 9; j++)
            (&smk->mat.uvs[0][0])[j] *= offset2;
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
void StartSmoke(signed short slot, unsigned int tire, int water, float *smkoffset)
{
    tCarData *car = &gGameCar[slot];
    tModelData *m = &model[slot];
    float pos2[3];
    float gap;
    float offset;
    float offset2;
    signed short surface;
    signed short first;
    signed short mph2 = fabs((signed short)(car->rwmph >> 2));
    int howmany = 1;
    int howmanydone = 0;
    anon_ll_mat_58 *smk;
    anon_c_rgba_4 wht = { {255,255,255,255} };
    anon_c_rgba_4 wg1 = { {180,255,180,160} };
    anon_c_rgba_4 wg2 = { {185,180,255,160} };
    anon_c_rgba_4 wg3 = { {255,192,187,160} };
    anon_c_rgba_4 rdClr = { {117,125,133,255} };
    anon_c_rgba_4 dtClr = { {157,108,71,255} };
    float pos[3];
    float prevPos[3];

    surface = model[slot].sviscode[tire];
    if ((surface == 8 && tire < 5) || (mph2 < 10 && tire < 4))
        return;
    first = 1;
    if (tire > 5) gap=0.75f; else gap=2.25f;
    if (water) {
        StartWater(slot,tire);
        return;
    }
    if (tire == 5) {
        howmany = 5;
    } else if (tire < 4 || tire > 5) {
        if (mph2 >= 115) howmany = 4;
        else if (mph2 >= 100) howmany = 3;
        else if (mph2 >= 85) howmany = 2;
        if (tire > 5) {
            if (tire == 6 && !(gCarSmokeDir[slot] & 0x4000u)) howmany = 1;
            else if (tire == 7 && !(gCarSmokeDir[slot] & 0x8000u)) howmany = 1;
            else howmany += 5;
            if (mph2 >= 150 && howmany != 1) howmany += 3;
        }
    } else {
        if (gCarSmokeDir[slot] & 0x40u) {
            gCarSmokeDir[slot] &= ~0x40u;
            gCarSmokeDir[slot] |= 0x80u;
            return;
        }
        if (gCarSmokeDir[slot] & 0x80u) {
            gCarSmokeDir[slot] &= ~0x80u;
            return;
        }
    }
    do {
        smk = GetStatLLE(&gSmokeList);
        if (!smk) return;
        smk->swTime = 0.0333333f;
        smk->data = tire << 17;
        smk->slot = slot;
        if (tire == 5) {
            smk->tIdx = -1;
            smk->mbIdx = MBOX_NewObject(gObjList[515], &smk->mat, -1, 0x52000u);
            ObjInstanceList[smk->mbIdx].w = 1.01f;
        } else {
            smk->tIdx = (int)Random(8.0f);
            smk->mbIdx = MBOX_NewObject(gObjList[507 + smk->tIdx], &smk->mat, -1, 0x52000u);
        }
        if (tire > 5) smk->lifeTime = 0.08f + Random(0.07f);
        else if (howmany == 1) smk->lifeTime = 0.5f + Random(1.5f);
        else if (tire == 5) smk->lifeTime = 0.5f + Random(2.25f);
        else smk->lifeTime = 0.5f + Random(0.25f);
        if (tire < 5) {
            smk->vel[0] = car->dr_vel[0];
            smk->vel[1] = car->dr_vel[1];
            smk->vel[2] = car->dr_vel[2];
            smk->vel[0] *= 0.125f;
            smk->vel[1] *= 0.125f;
            smk->vel[2] *= 0.125f;
            if (tire == 4) {
                smk->vel[0] *= 0.5f;
                smk->vel[1] *= 0.5f;
                smk->vel[2] *= 0.5f;
            }
        } else if (tire > 5) {
            smk->vel[0] = car->dr_vel[0];
            smk->vel[1] = car->dr_vel[1];
            smk->vel[2] = car->dr_vel[2];
            smk->vel[0] *= 0.5f;
            smk->vel[1] *= 0.5f;
            smk->vel[2] *= 0.5f;
        }
        switch (tire) {
        case 0:
            if (gCarSmokeDir[slot] & 1u) gCarSmokeDir[slot] &= ~1u;
            else { gCarSmokeDir[slot] |= 1u; smk->data |= 1u; }
            break;
        case 1:
            if (gCarSmokeDir[slot] & 2u) gCarSmokeDir[slot] &= ~2u;
            else { gCarSmokeDir[slot] |= 2u; smk->data |= 1u; }
            break;
        case 2:
            if (gCarSmokeDir[slot] & 4u) gCarSmokeDir[slot] &= ~4u;
            else { gCarSmokeDir[slot] |= 4u; smk->data |= 1u; }
            break;
        case 3:
            if (gCarSmokeDir[slot] & 8u) gCarSmokeDir[slot] &= ~8u;
            else { gCarSmokeDir[slot] |= 8u; smk->data |= 1u; }
            break;
        case 4:
            gCarSmokeDir[slot] |= 0x40u;
            smk->lifeTime += 1.0f;
            if (!(gCarSmokeDir[slot] & 0x20u)) {
                gCarSmokeDir[slot] |= 0x20u;
                gCarBlastSmokeClr[slot] = 175;
            }
            if (gCarSmokeDir[slot] & 0x10u) gCarSmokeDir[slot] &= ~0x10u;
            else { gCarSmokeDir[slot] |= 0x10u; smk->data |= 1u; }
            pos[0] = car->dr_pos[0]; pos[1] = car->dr_pos[1]; pos[2] = car->dr_pos[2];
            pos[0] += car->dr_uvs[0][0] * (Random(5.0f) - 2.5f);
            pos[1] += car->dr_uvs[0][1] * (Random(5.0f) - 2.5f);
            pos[2] += car->dr_uvs[0][2] * (Random(5.0f) - 2.5f);
            pos[0] += car->dr_uvs[2][0] * (Random(7.0f) - 3.5f);
            pos[1] += car->dr_uvs[2][1] * (Random(7.0f) - 3.5f);
            pos[2] += car->dr_uvs[2][2] * (Random(7.0f) - 3.5f);
            pos[0] += car->dr_uvs[1][0] * gCarMiddles[model[slot].body_type];
            pos[1] += car->dr_uvs[1][1] * gCarMiddles[model[slot].body_type];
            pos[2] += car->dr_uvs[1][2] * gCarMiddles[model[slot].body_type];
            pos[1] += 0.75f;
            ObjInstanceList[smk->mbIdx].scale = 0.75f + Random(0.5f);
            break;
        case 5:
            ObjInstanceList[smk->mbIdx].scale = 0.2f;
            ZOID_SetTexture(smk->mbIdx, &gTexDefPtrs[gFireTexs[howmanydone] >> 10].texDefs[gFireTexs[howmanydone] & 1023], -1);
            pos[0] = car->dr_pos[0]; pos[1] = car->dr_pos[1]; pos[2] = car->dr_pos[2];
            smk->vel[0] = Random(4.0f) - 2.0f;
            smk->vel[2] = Random(8.0f) - 4.0f;
            pos[0] += car->dr_uvs[0][0] * smk->vel[0];
            pos[1] += car->dr_uvs[0][1] * smk->vel[0];
            pos[2] += car->dr_uvs[0][2] * smk->vel[0];
            pos[0] += car->dr_uvs[2][0] * smk->vel[2];
            pos[1] += car->dr_uvs[2][1] * smk->vel[2];
            pos[2] += car->dr_uvs[2][2] * smk->vel[2];
            pos[0] += car->dr_uvs[1][0] * gCarMiddles[model[slot].body_type];
            pos[1] += car->dr_uvs[1][1] * gCarMiddles[model[slot].body_type];
            pos[2] += car->dr_uvs[1][2] * gCarMiddles[model[slot].body_type];
            break;
        case 6:
            if (gCarSmokeDir[slot] & 0x1000u) gCarSmokeDir[slot] &= ~0x1000u;
            else { gCarSmokeDir[slot] |= 0x1000u; smk->data |= 1u; }
            break;
        case 7:
            if (gCarSmokeDir[slot] & 0x2000u) gCarSmokeDir[slot] &= ~0x2000u;
            else { gCarSmokeDir[slot] |= 0x2000u; smk->data |= 1u; }
            break;
        }
        if (tire < 4) {
            offset2 = gap * howmanydone;
            pos[0] = car->dr_uvs[2][0] * (-2.0f - offset2);
            pos[1] = car->dr_uvs[2][1] * (-2.0f - offset2);
            pos[2] = car->dr_uvs[2][2] * (-2.0f - offset2);
            pos[0] += car->dr_tirepos[tire][0];
            pos[1] += car->dr_tirepos[tire][1];
            pos[2] += car->dr_tirepos[tire][2];
            pos[0] += car->dr_uvs[0][0] * (Random(2.0f) - 1.0f);
            pos[1] += car->dr_uvs[0][1] * (Random(2.0f) - 1.0f);
            pos[2] += car->dr_uvs[0][2] * (Random(2.0f) - 1.0f);
            pos[1] += m->reckon.suscomp[tire];
            ObjInstanceList[smk->mbIdx].scale = 0.25f + Random(0.75f);
        }
        if (tire > 5) {
            pos[0] = car->dr_pos[0]; pos[1] = car->dr_pos[1]; pos[2] = car->dr_pos[2];
            pos[0] += car->dr_uvs[1][0] * smkoffset[1];
            pos[1] += car->dr_uvs[1][1] * smkoffset[1];
            pos[2] += car->dr_uvs[1][2] * smkoffset[1];
            pos[0] += car->dr_uvs[2][0] * smkoffset[2];
            pos[1] += car->dr_uvs[2][1] * smkoffset[2];
            pos[2] += car->dr_uvs[2][2] * smkoffset[2];
            pos[0] += -1.15f * car->dr_uvs[2][0];
            pos[1] += -1.15f * car->dr_uvs[2][1];
            pos[2] += -1.15f * car->dr_uvs[2][2];
            if (tire == 6) {
            pos[0] += car->dr_uvs[0][0] * smkoffset[0];
            pos[1] += car->dr_uvs[0][1] * smkoffset[0];
            pos[2] += car->dr_uvs[0][2] * smkoffset[0];
                pos[0] += -2.6f * car->dr_uvs[0][0];
                pos[1] += -2.6f * car->dr_uvs[0][1];
                pos[2] += -2.6f * car->dr_uvs[0][2];
            } else {
            pos[0] += car->dr_uvs[0][0] * smkoffset[0];
            pos[1] += car->dr_uvs[0][1] * smkoffset[0];
            pos[2] += car->dr_uvs[0][2] * smkoffset[0];
                pos[0] += 2.6f * car->dr_uvs[0][0];
                pos[1] += 2.6f * car->dr_uvs[0][1];
                pos[2] += 2.6f * car->dr_uvs[0][2];
            }
            pos[0] += 0.5f * car->dr_uvs[1][0];
            pos[1] += 0.5f * car->dr_uvs[1][1];
            pos[2] += 0.5f * car->dr_uvs[1][2];
            if (first) {
                if (tire == 6) {
                    if (!(gCarSmokeDir[slot] & 0x4000u)) gCarSmokeDir[slot] |= 0x4000u;
                    else { prevPos[0]=gCarWingSmokePos[slot][0][0]; prevPos[1]=gCarWingSmokePos[slot][0][1]; prevPos[2]=gCarWingSmokePos[slot][0][2]; }
                    gCarWingSmokePos[slot][0][0]=pos[0]; gCarWingSmokePos[slot][0][1]=pos[1]; gCarWingSmokePos[slot][0][2]=pos[2];
                } else if (tire == 7) {
                    if (!(gCarSmokeDir[slot] & 0x8000u)) gCarSmokeDir[slot] |= 0x8000u;
                    else { prevPos[0]=gCarWingSmokePos[slot][1][0]; prevPos[1]=gCarWingSmokePos[slot][1][1]; prevPos[2]=gCarWingSmokePos[slot][1][2]; }
                    gCarWingSmokePos[slot][1][0]=pos[0]; gCarWingSmokePos[slot][1][1]=pos[1]; gCarWingSmokePos[slot][1][2]=pos[2];
                }
            } else {
                pos2[0]=prevPos[0]-pos[0]; pos2[1]=prevPos[1]-pos[1]; pos2[2]=prevPos[2]-pos[2];
                offset=NormalVector(pos2);
                if (offset) {
                    offset2=howmanydone*(offset/howmany);
                    pos[0]+=pos2[0]*offset2; pos[1]+=pos2[1]*offset2; pos[2]+=pos2[2]*offset2;
                }
            }
            ObjInstanceList[smk->mbIdx].scale = 0.075f + Random(0.1f);
        }
        MBOX_ShowObject(smk->mbIdx,0,15);
        if (car->camera >= 0 && (car->view == 0 || car->view == 1))
            MBOX_HideObject(smk->mbIdx,0,1<<car->camera);
        if (tire > 5) {
            switch(gCarWings[slot+1][m->body_type]) {
            case 0: ZOID_SetColor(smk->mbIdx,&wg1); break;
            case 1: ZOID_SetColor(smk->mbIdx,&wg2); break;
            case 2: ZOID_SetColor(smk->mbIdx,&wg3); break;
            }
        } else if (tire == 4) {
            ObjInstanceList[smk->mbIdx].color.c[0]=gCarBlastSmokeClr[slot];
            ObjInstanceList[smk->mbIdx].color.c[1]=gCarBlastSmokeClr[slot];
            ObjInstanceList[smk->mbIdx].color.c[2]=gCarBlastSmokeClr[slot];
            ObjInstanceList[smk->mbIdx].color.c[3]=255;
            gCarBlastSmokeClr[slot]-=3;
            if(gCarBlastSmokeClr[slot]<53) gCarBlastSmokeClr[slot]=53;
        } else if (tire == 5) ZOID_SetColor(smk->mbIdx,&wht);
        else if (surface == 1) ZOID_SetColor(smk->mbIdx,&dtClr);
        else ZOID_SetColor(smk->mbIdx,&rdClr);
        smk->mat.mat3.pos[0]=pos[0];
        smk->mat.mat3.pos[1]=pos[1];
        smk->mat.mat3.pos[2]=pos[2];
        howmanydone++;
        first=0;
    } while(howmany != howmanydone);
}
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
