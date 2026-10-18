/* shots.c - battle damage, projectile collision and shot lifecycle, recovered C.
 * Status: Matching.
 */
#include "rush/rush_protos.h"
#include "msl_math_inline.h"

static const float CaseOffsets[1][3] = { 0.6f, 0.65f, -0.69f };
static const float ShotOffsets[10][3] = {
    0.16f, 0.49f, 0.57f, 0.01f, 0.65f, 1.59f, 0.0f, 0.9f, 1.1f, 0.0f, -0.3f, -0.7f,
    0.01f, 2.04f, 0.28f, 0.0f, 0.0f, 0.0f, -0.04f, 0.61f, 0.23f, 0.0f, 1.25f, 0.0f,
    0.0f, 1.5f, 2.0f, 0.0f, 0.0f, 0.0f
};
static int sWeaponFx[10] = { 28, 25, 24, 26, 27, 29, 30, 31, 20, -1 };
static float gShieldSizes[13] = {
    0.85f, 0.8f, 0.75f, 0.77f, 0.83f, 0.85f, 0.75f, 0.75f, 0.65f, 0.85f, 1.1f, 0.8f, 0.73f
};
static anon_c_rgba_4 hitColor = { 255, 255, 255, 255 };
static unsigned short wallTidx;
static float gCameraMatrix[4][4];
static tShieldStruct gCarShield[4];
unsigned short gRipTextures[5];
unsigned short gMineTextures[5];
int gWfxObjList[7];
int gWprObjList[10];
static anon_ll_mbIdx_48 gShotCase[30];
static anon_ll_mbIdx_3c gShotObjs[24];
anon_dbl_numElements_18 gShotObjList;
static anon_ll_poly_8 gHitPolys[24];
anon_dbl_numElements_18 gHitPolyList;
static anon_ll_slot_68 gShots[24];
anon_dbl_numElements_18 gShotList;
anon_dbl_numElements_18 gShotCaseList;

/* 0x8003c5c0, 248 bytes */
static void AnimateCase(void)
{
    anon_ll_mbIdx_48 *casing;
    anon_ll_mbIdx_48 *next;

    for (casing = gShotCaseList.head; casing; casing = next) {
        next = (anon_ll_mbIdx_48 *)casing->ll.next;
        casing->time -= gFrameSeconds;
        if (casing->time <= 0.0f) {
            ZOID_RemoveObject(casing->mbIdx);
            FreeStatLLE(&gShotCaseList, casing);
        } else {
            casing->vel[1] -= 2.5f * (gGravity * gFrameSeconds);
            casing->mat.mat3.pos[0] += casing->vel[0] * gFrameSeconds;
            casing->mat.mat3.pos[1] += casing->vel[1] * gFrameSeconds;
            casing->mat.mat3.pos[2] += casing->vel[2] * gFrameSeconds;
        }
    }
}

/* 0x8003c6b8, 260 bytes */
void BattleDamage(tCarData *car, tCarData *hit, int damage)
{
    if (!model[hit->slot].crashflag && hit->slot != car->slot &&
        gCarTeam[hit->slot] != gCarTeam[car->slot]) {
        if (hit->battle.defense.active & 4)
            damage *= 0.2f;
        hit->battle.health -= damage;
        hit->battle.damageThisFrame = damage;
        if (hit->battle.health <= 0) {
            hit->battle.health = 0;
            AddBattlePoint(car->slot, hit->slot, 1);
            model[hit->slot].crashflag = 1;
        }
    }
}

/* 0x8003c7bc, 424 bytes */
void BattleDamage2(tCarData *car1, tCarData *car2, int damage1, int damage2)
{
    if (!model[car1->slot].crashflag && !model[car2->slot].crashflag &&
        gCarTeam[car1->slot] != gCarTeam[car2->slot]) {
        if (car2->battle.defense.active & 4)
            damage1 *= 0.2f;
        if (car1->battle.defense.active & 4)
            damage2 *= 0.2f;
        car1->battle.health -= damage2;
        car1->battle.damageThisFrame = damage2;
        if (car1->battle.health <= 0) {
            car1->battle.health = 0;
            AddBattlePoint(car2->slot, car1->slot, 1);
            model[car1->slot].crashflag = 1;
        }
        car2->battle.health -= damage1;
        car2->battle.damageThisFrame = damage1;
        if (car2->battle.health <= 0) {
            car2->battle.health = 0;
            AddBattlePoint(car1->slot, car2->slot, 1);
            model[car2->slot].crashflag = 1;
        }
    }
}

/* 0x8003c964, 544 bytes */
void ProximityDamage(float *pos, float *old, int owner, float radiussq, int damage)
{
    tCarData *hit;
    float pos2[3];
    float pos3[3];
    float distsq;
    float y;

    for (hit = gGameCar; hit < &gGameCar[gNumCars]; hit++) {
        if (model[hit->slot].crashflag || !hit->collidable || hit->dead)
            continue;
        {
            float delta[3];
            delta[0] = pos[0] - hit->dr_pos[0];
            delta[1] = pos[1] - hit->dr_pos[1];
            delta[2] = pos[2] - hit->dr_pos[2];
            distsq = delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2];
        }
        if (distsq < radiussq) {
            y = (radiussq - distsq) / radiussq;
            if (hit->battle.weapon.type == 5) {
                pos2[0] = old[0] - pos[0];
                pos2[1] = old[1] - pos[1];
                pos2[2] = old[2] - pos[2];
                BodyVector(pos2, pos3, hit->dr_uvs);
                if (fabs(fatan2(pos3[0], pos3[2])) < 1.35f)
                    BattleDamage(&gGameCar[owner], hit, 0.35f * (damage * (y * y)));
                else
                    BattleDamage(&gGameCar[owner], hit, damage * (y * y));
            } else {
                BattleDamage(&gGameCar[owner], hit, damage * (y * y));
            }
        }
    }
}


/* 0x8003cb84, 696 bytes */
void ElevateGun(tCarData *car)
{
    tCarData *hit;
    float vec[3];
    float pos[3];
    float best = 2000.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;
    float h;

    for (hit = gGameCar; hit < &gGameCar[gNumCars]; hit++) {
        if (car->slot == hit->slot || gCarTeam[car->slot] == gCarTeam[hit->slot] ||
            model[hit->slot].crashflag || !hit->collidable || hit->dead)
            continue;
        pos[0] = hit->dr_pos[0] - car->dr_pos[0];
        pos[1] = hit->dr_pos[1] - car->dr_pos[1];
        pos[2] = hit->dr_pos[2] - car->dr_pos[2];
        if (car->battle.weapon.type >= 8)
            h = -1.0f * ShotOffsets[car->battle.weapon.type][1];
        else
            h = -1.0f * (ShotOffsets[car->battle.weapon.type][1] +
                  weaponOffsets[car->battle.weapon.type][model[car->slot].body_type][1]);
        pos[0] += car->dr_uvs[1][0] * h;
        pos[1] += car->dr_uvs[1][1] * h;
        pos[2] += car->dr_uvs[1][2] * h;
        BodyVector(pos, vec, car->dr_uvs);
        if (vec[2] < 0.0f || vec[0] < -0.5f * vec[2] || vec[0] > 0.5f * vec[2] ||
            vec[1] < -vec[2] || vec[1] > vec[2] || vec[2] > best)
            continue;
        best = vec[2];
        yaw = fatan2(vec[0], vec[2]);
        pitch = fatan2(vec[1], vec[2]);
    }
    if (yaw < car->battle.yaw)
        car->battle.yaw -= 0.01f;
    else if (yaw > car->battle.yaw)
        car->battle.yaw += 0.01f;
    if (pitch < car->battle.pitch)
        car->battle.pitch -= 0.01f;
    else if (pitch > car->battle.pitch)
        car->battle.pitch += 0.01f;
}

/* 0x8003ce3c, 716 bytes */
void MissileLock(anon_ll_slot_68 *shot)
{
    tCarData *hit;
    float vec[3];
    float pos[3];
    float yaw, pitch;
    float best = 2000.0f;
    float dist;

    for (hit = gGameCar; hit < &gGameCar[gNumCars]; hit++) {
        if (shot->slot == hit->slot || gCarTeam[shot->slot] == gCarTeam[hit->slot] ||
            model[hit->slot].crashflag || !hit->collidable || hit->dead)
            continue;
        pos[0] = hit->dr_pos[0] - shot->pos[0];
        pos[1] = hit->dr_pos[1] - shot->pos[1];
        pos[2] = hit->dr_pos[2] - shot->pos[2];
        BodyVector(pos, vec, shot->uvs);
        if (vec[2] < 0.0f || vec[0] < -vec[2] || vec[0] > vec[2] ||
            vec[1] < -vec[2] || vec[1] > vec[2])
            continue;
        dist = sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
        if (dist >= best)
            continue;
        best = dist;
        yaw = vec[0];
        pitch = vec[1];
    }
    if (best < 2000.0f) {
        if (yaw > 0.0f)
            YawUV(-0.04f, shot->uvs);
        else
            YawUV(0.04f, shot->uvs);
        if (pitch > 0.0f)
            PitchUV(0.03f, shot->uvs);
        else
            PitchUV(-0.03f, shot->uvs);
    }
}

/* 0x8003d108, 956 bytes */
static tCarData *ShotToCarCollision(anon_ll_slot_68 *shot, float *old)
{
    tCarData *car;
    tCarData *hit;
    float pos[3];
    float pos2[3];
    float dir[3];
    float normal[3];
    float projected[3];
    float delta[3];
    float t;
    float d0;
    float t0;
    float y;
    int i;

    if (shot->type == 3)
        return 0;
    dir[0] = shot->pos[0] - old[0];
    dir[1] = shot->pos[1] - old[1];
    dir[2] = shot->pos[2] - old[2];
    normal[0] = -dir[0];
    normal[1] = 0.0f;
    normal[2] = -dir[2];
    d0 = dir[0] * normal[0] + dir[1] * normal[1] + dir[2] * normal[2];
    if (fabs(d0) < 0.001f)
        return 0;
    t0 = 1.0f;
    hit = 0;
    for (i = 0, car = gGameCar; i < gNumCars; i++, car++) {
        if (shot->slot == car->slot || model[i].resurrect.moving_state != -1 ||
            !car->collidable || model[i].crashflag)
            continue;
        delta[0] = car->dr_pos[0] - old[0];
        delta[1] = car->dr_pos[1] - old[1];
        delta[2] = car->dr_pos[2] - old[2];
        t = (delta[0] * normal[0] + delta[1] * normal[1] + delta[2] * normal[2]) / d0;
        if (t < 0.0f || t > t0)
            continue;
        projected[0] = dir[0] * t + old[0];
        projected[1] = dir[1] * t + old[1];
        projected[2] = dir[2] * t + old[2];
        pos[0] = projected[0] - car->dr_pos[0];
        pos[1] = projected[1] - car->dr_pos[1];
        pos[2] = projected[2] - car->dr_pos[2];
        if (pos[1] < -shot->radius || pos[1] > 3.5f + shot->radius)
            continue;
        y = sqrtf(pos[0] * pos[0] + pos[2] * pos[2]);
        if (y > 6.75f + shot->radius - 0.5f)
            continue;
        if (car->battle.weapon.type == 5) {
            pos2[0] = old[0] - shot->pos[0];
            pos2[1] = old[1] - shot->pos[1];
            pos2[2] = old[2] - shot->pos[2];
            BodyVector(pos2, pos, car->dr_uvs);
            if (fabs(fatan2(pos[0], pos[2])) < 1.35f)
                shot->flags |= 1;
        }
        shot->pos[0] = projected[0];
        t0 = t;
        hit = car;
        shot->pos[1] = projected[1];
        shot->pos[2] = projected[2];
    }
    return hit;
}


/* 0x8003d4c4, 936 bytes */
static void SonicCarCollision(anon_ll_slot_68 *shot)
{
    tCarData *car;
    tCarData *our;
    tModelData *m;
    int i;
    int dam;
    float vec[3];
    float vec2[3];
    float dist;
    float rad;
    float radsq;
    float mul;
    float scale;
    anon_ll_mbIdx_3c *o = shot->obj;
    float pos[3];

    our = &gGameCar[shot->slot];
    scale = ObjInstanceList[o->mbIdx].scale;
    rad = 4.0f * scale;
    radsq = rad * rad;
    if (scale <= 8.0f) {
        mul = 1.0f;
        dam = 800;
    } else if (scale <= 20.0f) {
        mul = 0.6f;
        dam = 400;
    } else {
        dam = 200;
        mul = 0.5f;
    }
    pos[0] = o->mat.mat3.pos[0];
    pos[1] = o->mat.mat3.pos[1];
    pos[2] = o->mat.mat3.pos[2];
    for (i = 0, car = gGameCar; i < gNumCars; i++, car++) {
        if (shot->slot == car->slot || model[i].resurrect.moving_state != -1 ||
            !car->collidable || (shot->slotDamaged & (1 << car->slot)))
            continue;
        vec[0] = car->dr_pos[0] - pos[0];
        vec[1] = car->dr_pos[1] - pos[1];
        vec[2] = car->dr_pos[2] - pos[2];
        dist = vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2];
        if (dist <= radsq) {
            shot->slotDamaged |= 1 << car->slot;
            BattleDamage(our, car, dam);
            m = &model[car->slot];
            if (!m->isCollideWithWall) {
                rad = sqrtf(dist);
                vec2[2] = mul * (330000.0f * (vec[2] / rad));
                vec2[1] = mul * (330000.0f * (vec[1] / rad) + 66000.0f);
                vec2[0] = mul * (330000.0f * (vec[0] / rad));
                BodyVector(vec2, vec, car->dr_uvs);
                m->centerForce[0] += vec[0];
                m->centerForce[1] += vec[1];
                m->centerForce[2] += vec[2];
                vec[1] = 0.0f;
                m->centerMoment[0] += vec[0];
                m->centerMoment[1] += vec[1];
                m->centerMoment[2] += vec[2];
                m->isCollideWithWall = 50;
            }
        }
    }
}


/* 0x8003d86c, 440 bytes */
void ShotInit(void)
{
    int i;
    static const char *const WprObjNames[10] = {
        "WPR_CASEINGG1", "WFX_TRACERG1", "WPR_CANNG1", "WPR_GRENG1", "WPR_MINEBASE",
        "WPR_MINECAP", "WPR_MINEGLOW", "WPR_MISSG1", "WPR_ROCKG1", "WPR_SONCG1"
    };
    static const char *const WfxObjNames[7] = {
        "WFX_MFLSHG1", "WFX_MFLSHG11", "WFX_MFLSHG13", "WFX_MFLSHG17",
        "WFX_MFLSHG3", "WFX_SHIELDG1", "WFX_RIPSMOKE"
    };
    static const char *const MineTexNames[5] = {
        "MINE_CAP_B", "MINE_CAP_R", "MINE_CAP_Y", "MINE_CAP_G", "MINE_CAP"
    };
    static const char *const RipTexNames[5] = {
        "WFX_SHLDRIP01", "WFX_SHLDRIP02", "WFX_SHLDRIP03", "WFX_SHLDRIP04", "WFX_SHLDRIP05"
    };

    wallTidx = 0;
    InitStatLL(&gShotList, gShots, sizeof(gShots[0]), 24, 0);
    InitStatLL(&gHitPolyList, gHitPolys, sizeof(gHitPolys[0]), 24, 0);
    InitStatLL(&gShotObjList, gShotObjs, sizeof(gShotObjs[0]), 24, 0);
    InitStatLL(&gShotCaseList, gShotCase, sizeof(gShotCase[0]), 30, 0);
    for (i = 0; i < 10; i++)
        gWprObjList[i] = MBOX_FindObject_WORK(WprObjNames[i], 0, gNumModels - 1, 0);
    for (i = 0; i < 7; i++)
        gWfxObjList[i] = MBOX_FindObject_WORK(WfxObjNames[i], 0, gNumModels - 1, 0);
    for (i = 0; i < 5; i++)
        MBOX_FindTexture_WORK(MineTexNames[i], &gMineTextures[i], 0, gNumModels - 1, 1);
    for (i = 0; i < 5; i++)
        MBOX_FindTexture_WORK(RipTexNames[i], &gRipTextures[i], 0, gNumModels - 1, 1);
    for (i = 0; i < 4; i++)
        gCarShield[i].dlIdx = -1;
    InitStreaks();
}

/* 0x8003da24, 172 bytes */
static void ShotDone(anon_ll_slot_68 *shot)
{
    anon_ll_mbIdx_3c *o;
    switch (shot->type) {
    case 3:
        o = shot->obj2;
        ZOID_RemoveObject(o->mbIdx);
        FreeStatLLE(&gShotObjList, o);
    case 0:
    case 1:
    case 2:
    case 4:
    case 6:
    case 7:
    case 8:
        o = shot->obj;
        ZOID_RemoveObject(o->mbIdx);
        FreeStatLLE(&gShotObjList, o);
        break;
    }
}

/* 0x8003dad0, 176 bytes */
static anon_ll_mbIdx_3c *_CreateShotObject(int idx, int parent, unsigned int flags, int o2)
{
    anon_ll_mbIdx_3c *o;
    anon_mat3_mat4_30 *mat = (anon_mat3_mat4_30 *)unitmatrix;
    o = GetStatLLE(&gShotObjList);
    if (o2)
        o->mbIdx = MBOX_NewObject(gWprObjList[idx], mat, parent, flags);
    else
        o->mbIdx = MBOX_NewObject(gWprObjList[idx], &o->mat, parent, flags);
    return o;
}

/* 0x8003db80, 308 bytes */
static void _UpdateShotObject(anon_ll_slot_68 *shot, int unt)
{
    anon_ll_mbIdx_3c *o = shot->obj;
    o->mat.mat3.pos[0] = shot->pos[0];
    o->mat.mat3.pos[1] = shot->pos[1];
    o->mat.mat3.pos[2] = shot->pos[2];
    if (unt == 1)
        CopyUV(unitmatrix, o->mat.mat3.uvs);
    else if (unt != 2)
        CopyUV(shot->uvs, o->mat.mat3.uvs);

    if (shot->flags & 0x70) {
        if ((shot->flags & 0x80) && (shot->flags & 0x10) &&
            (shot->type == 1 || shot->type == 8)) {
reduceScale:
            o->mat.mat3.uvs[2][0] *= 0.2f;
            o->mat.mat3.uvs[2][1] *= 0.2f;
            o->mat.mat3.uvs[2][2] *= 0.2f;
        } else if (shot->type == 0) {
            if (shot->flags & 0x80) {
                goto reduceScale;
            } else {
                o->mat.mat3.uvs[2][0] *= 0.4f;
                o->mat.mat3.uvs[2][1] *= 0.4f;
                o->mat.mat3.uvs[2][2] *= 0.4f;
            }
        }
    }
    if (shot->flags & 2)
        PitchUV(0.18f, o->mat.mat3.uvs);
}

/* 0x8003dcb4, 152 bytes */
static void _InitNewShot(anon_ll_slot_68 *shot, tCarData *car)
{
    shot->slotDamaged = 0;
    shot->obj = 0;
    shot->obj2 = 0;
    shot->flags = 0x10;
    shot->type = car->battle.weapon.type;
    shot->slot = car->slot;
    shot->incTime = 0.0f;
    shot->vel[0] = 0.0f;
    shot->vel[1] = 0.0f;
    shot->vel[2] = 0.0f;
    shot->pos[0] = car->dr_pos[0];
    shot->pos[1] = car->dr_pos[1];
    shot->pos[2] = car->dr_pos[2];
    shot->old[0] = car->dr_pos[0];
    shot->old[1] = car->dr_pos[1];
    shot->old[2] = car->dr_pos[2];
    CopyUV(car->dr_uvs, shot->uvs);
}

/* 0x8003dd4c, 812 bytes */
static void UpdateDefense(tCarData *car, tModelData *m, anon_active_invsTime_10 *defense, int i)
{
    anon_c_rgba_4 clr = { 255, 255, 255, 255 };
    int j;
    tShieldStruct *shield;
    anon_mat3_mat4_30 *mat;

    if ((defense->active & 1) && car->battle.fadeState != 2) {
        defense->invsTime -= gFrameSeconds;
        if (defense->invsTime <= 0.0f || m->crashflag) {
            defense->invsTime = 0.0f;
            car->battle.fadeState = 2;
            car->battle.fadeTime = 0.666667f;
            car->battle.fadeAlpha = 16;
        }
    }
    if (defense->active & 2) {
        if (m->crashflag)
            goto removeShield;
        shield = &gCarShield[i];
        mat = &shield->mat;
        shield->mat.mat3.pos[0] = car->dr_pos[0];
        shield->mat.mat3.pos[1] = car->dr_pos[1];
        shield->mat.mat3.pos[2] = car->dr_pos[2];
        CopyUV(car->dr_uvs, mat->mat3.uvs);
        defense->shldTime -= gFrameSeconds;
        if (defense->shldTime <= 0.0f) {
            if (defense->active & 8) {
                if (defense->shldScale >= gShieldSizes[m->body_type]) {
                    defense->shldScale = gShieldSizes[m->body_type];
                    defense->active &= ~8;
                    defense->active |= 4;
                    defense->shldTime = 30.0f;
                } else {
                    defense->shldScale += 0.05f;
                    if (defense->shldScale > gShieldSizes[m->body_type])
                        defense->shldScale = gShieldSizes[m->body_type];
                    defense->shldTime = 0.0333333f;
                }
            } else if (defense->active & 16) {
                if (defense->shldScale <= 0.06f) {
removeShield:
                    defense->shldTime = 0.0f;
                    defense->active &= ~0x1e;
                    if (gCarShield[i].dlIdx != -1) {
                        ZOID_RemoveObject(gCarShield[i].dlIdx);
                        gCarShield[i].dlIdx = -1;
                    }
                    return;
                } else {
                    defense->shldScale -= 0.05f;
                    defense->shldTime = 0.0333333f;
                }
            } else {
                defense->active &= ~4;
                defense->active |= 16;
                defense->shldTime = 0.0333333f;
            }
        }
        for (j = 0; j < 3; j++) {
            shield->mat.mat3.uvs[j][0] *= defense->shldScale;
            shield->mat.mat3.uvs[j][1] *= defense->shldScale;
            shield->mat.mat3.uvs[j][2] *= defense->shldScale;
        }
        if (shield->dlIdx == -1)
            shield->dlIdx = MBOX_NewObject(gWfxObjList[5], mat, -1, 0x802000);
        if (car->battle.defense.active & 1) {
            clr.c[3] = car->battle.fadeAlpha;
            if (clr.c[3] < 48)
                clr.c[3] = 48;
        }
        ZOID_SetColor(shield->dlIdx, &clr);
    }
}

/* 0x8003e078, 4220 bytes */
static void UpdateShots(void)
{
    anon_ll_slot_68 *shot, *nexts;
    anon_ll_poly_8 *h;
    anon_ll_mbIdx_3c *o;
    tCarData *car, *hit;
    tModelData *m;
    float old[3];
    float vel;
    anon_c_rgba_4 clr = {255,255,255,255};
    float verts[4][3];
    float scale;
    float radius;
    float uvs[3][3];
    int damage;
    float v1[3];

    for (shot = gShotList.head; shot; shot = nexts) {
        nexts = (anon_ll_slot_68 *)shot->ll.next;
        if (shot->type == 3) {
            car = &gGameCar[shot->slot];
            m = &model[shot->slot];
            if (shot->incTime == 0.0f) {
                float x, y, z;
                shot->pos[0] = car->dr_pos[0];
                shot->pos[1] = car->dr_pos[1];
                shot->pos[2] = car->dr_pos[2];
                x = ShotOffsets[3][0] + weaponOffsets[3][m->body_type][0];
                y = ShotOffsets[3][1] + weaponOffsets[3][m->body_type][1];
                z = ShotOffsets[3][2] + weaponOffsets[3][m->body_type][2];
                shot->pos[0] += car->dr_uvs[2][0]*z;
                shot->pos[1] += car->dr_uvs[2][1]*z;
                shot->pos[2] += car->dr_uvs[2][2]*z;
                shot->pos[0] += car->dr_uvs[1][0]*y;
                shot->pos[1] += car->dr_uvs[1][1]*y;
                shot->pos[2] += car->dr_uvs[1][2]*y;
                shot->pos[0] += car->dr_uvs[0][0]*x;
                shot->pos[1] += car->dr_uvs[0][1]*x;
                shot->pos[2] += car->dr_uvs[0][2]*x;
                CopyUV(car->dr_uvs, shot->uvs);
                PitchUV(2.0734513f, shot->uvs);
                if (car->battle.loaded == 1) {
                    if (car->battle.weapon.type != 3 || m->crashflag || !car->collidable || car->dead) {
                        car->battle.loaded = 0;
                        FreeStatLLE(&gShotList, shot);
                        continue;
                    }
                    car->battle.loaded = 2;
                    if (!shot->obj) {
                        clr = car->IRclr;
                        if (car->battle.defense.active & 1) {
                            clr.c[3] = car->battle.fadeAlpha;
                            if (clr.c[3] < 48)
                                clr.c[3] = 48;
                        }
                        shot->obj = _CreateShotObject(4, -1, 0x2000, 0);
                        ZOID_SetColor(((anon_ll_mbIdx_3c *)shot->obj)->mbIdx, &clr);
                        shot->obj2 = _CreateShotObject(5, ((anon_ll_mbIdx_3c *)shot->obj)->mbIdx, 0x2080, 1);
                        h = shot->obj2;
                        ZOID_SetColor(((anon_ll_mbIdx_3c *)h)->mbIdx, &clr);
                        ZOID_SetTexture(((anon_ll_mbIdx_3c *)h)->mbIdx,
                            (anon_name_width_30 *)&gTexDefPtrs[gMineTextures[gCarTeam[shot->slot]] >> 10].texDefs[gMineTextures[gCarTeam[shot->slot]] & 1023], -1);
                    }
                    shot->radius = 0.0f;
                }
                if (car->battle.loaded == 2) {
                    if (car->battle.weapon.type != 3 || m->crashflag || !car->collidable || car->dead) {
                        car->battle.loaded = 0;
                        ShotDone(shot);
                        FreeStatLLE(&gShotList, shot);
                        continue;
                    }
                    o = shot->obj;
                    o->mat.mat3.pos[0] = shot->pos[0];
                    o->mat.mat3.pos[1] = shot->pos[1];
                    o->mat.mat3.pos[2] = shot->pos[2];
                    CopyUV(shot->uvs, o->mat.mat3.uvs);
                    clr = car->IRclr;
                    if (car->battle.defense.active & 1) {
                        clr.c[3] = car->battle.fadeAlpha;
                        if (clr.c[3] < 48)
                            clr.c[3] = 48;
                    }
                    ZOID_SetColor(((anon_ll_mbIdx_3c *)shot->obj)->mbIdx, &clr);
                    ZOID_SetColor(((anon_ll_mbIdx_3c *)shot->obj2)->mbIdx, &clr);
                    if (car->player && (car->player->edges & car->player->controls[7])) {
                        clr.c[3] = 255;
                        ZOID_SetColor(((anon_ll_mbIdx_3c *)shot->obj)->mbIdx, &clr);
                        ZOID_SetColor(((anon_ll_mbIdx_3c *)shot->obj2)->mbIdx, &clr);
                        if (car->battle.weapon.ammoCnt > 0)
                            car->battle.weapon.ammoCnt--;
                        car->battle.shoot = 6;
                        PlayGameSound(sWeaponFx[3], shot->slot, 1, 1);
                        car->battle.reloadingTime = 1.0f;
                        shot->vel[0] = car->dr_vel[0] + 15.0f*shot->uvs[1][0];
                        shot->vel[1] = car->dr_vel[1] + 15.0f*shot->uvs[1][1];
                        shot->vel[2] = car->dr_vel[2] + 15.0f*shot->uvs[1][2];
                        car->battle.loaded = 0;
                        shot->lifeTime = 5.0f;
                    } else {
                        o = shot->obj;
                        ScaleUV(o->mat.mat3.uvs, o->mat.mat3.uvs, 0.5f);
                        continue;
                    }
                }
            }
        }
        shot->lifeTime -= gFrameSeconds;
        shot->incTime += gFrameSeconds;
        if (shot->lifeTime <= 0.0f) {
            ShotDone(shot);
            FreeStatLLE(&gShotList, shot);
            continue;
        }
        car = &gGameCar[shot->slot];
        old[0] = shot->pos[0];
        old[1] = shot->pos[1];
        old[2] = shot->pos[2];
        radius = shot->radius;
        switch (shot->type) {
        case 3:
            vel = gFrameSeconds;
            damage = 960;
            scale = 10.0f;
            shot->vel[1] -= gGravity*gFrameSeconds;
            shot->vel[0] *= 0.9f;
            shot->vel[2] *= 0.9f;
            shot->pos[0] = old[0] + shot->vel[0]*gFrameSeconds;
            shot->pos[1] = old[1] + shot->vel[1]*gFrameSeconds;
            shot->pos[2] = old[2] + shot->vel[2]*gFrameSeconds;
            break;
        case 2:
            damage = 800;
            shot->vel[1] -= 2.5f*(gGravity*gFrameSeconds);
            radius = 0.5f;
            scale = 10.0f;
            shot->pos[0] = old[0] + shot->vel[0]*gFrameSeconds;
            shot->pos[1] = old[1] + shot->vel[1]*gFrameSeconds;
            shot->pos[2] = old[2] + shot->vel[2]*gFrameSeconds;
            break;
        case 4:
            scale = 12.0f;
            damage = 960;
            MissileLock(shot);
            shot->vel[2] += (100.0f - shot->vel[2])*gFrameSeconds;
            vel = shot->vel[2]*gFrameSeconds;
            goto straightShot;
        case 6:
            damage = 520;
            shot->vel[0] += shot->uvs[2][0]*(800.0f*gFrameSeconds);
            shot->vel[1] += shot->uvs[2][1]*(800.0f*gFrameSeconds);
            shot->vel[2] += shot->uvs[2][2]*(800.0f*gFrameSeconds);
            scale = 8.0f;
            shot->pos[0] = old[0] + shot->vel[0]*gFrameSeconds;
            shot->pos[1] = old[1] + shot->vel[1]*gFrameSeconds;
            shot->pos[2] = old[2] + shot->vel[2]*gFrameSeconds;
            break;
        case 0:
            damage = 400;
            vel = 1000.0f*gFrameSeconds;
            scale = 10.0f;
            goto straightShot;
        case 1:
            damage = 114;
            vel = 2000.0f*gFrameSeconds;
            scale = 3.0f;
            goto straightShot;
        case 7:
            damage = 66;
            break;
        case 8:
        default:
            damage = 80;
            vel = 2000.0f*gFrameSeconds;
            scale = 3.0f;
straightShot:
            shot->pos[0] = old[0] + shot->uvs[2][0]*vel;
            shot->pos[1] = old[1] + shot->uvs[2][1]*vel;
            shot->pos[2] = old[2] + shot->uvs[2][2]*vel;
            break;
        }
        if (shot->streak) {
            shot->streak->flags |= 0x10;
            shot->streak->newpos[0] = shot->pos[0];
            shot->streak->newpos[1] = shot->pos[1];
            shot->streak->newpos[2] = shot->pos[2];
        }
        if (shot->type != 7) {
            if (shot->type == 4 || shot->type == 6)
                hit = ShotToCarCollision(shot, shot->old);
            else
                hit = ShotToCarCollision(shot, old);
            if (hit)
                shot->flags |= 0x80;
            if (ShotToWorldCollision(old, shot->pos, uvs, radius, 0, 65535)) {
                shot->flags |= 0x80;
                switch (shot->type) {
                case 3:
                    if (shot->flags & 2) {
                        if (fabs(shot->vel[0]) < 10.0 && fabs(shot->vel[2]) < 10.0) {
                            shot->lifeTime = 0.0000001f;
                            SetupTarget("WPR_MINE", -2, shot->pos, 0, shot->slot, 1, 0);
                        } else {
                            BodyVector(shot->vel, v1, uvs);
                            v1[1] = 0.4f*-v1[1];
                            WorldVector(v1, shot->vel, uvs);
                        }
                    } else {
                        shot->flags |= 2;
                        BodyVector(shot->vel, v1, uvs);
                        v1[1] = 15.0f;
                        WorldVector(v1, shot->vel, uvs);
                    }
                    break;
                case 4: case 6:
                    ProximityDamage(shot->pos, shot->old, shot->slot, 900.0f, damage);
                    StartBlast(shot->pos, 0, 0.5f, 1);
                    shot->lifeTime = 0.0000001f;
                    if (shot->streak)
                        shot->streak->flags |= 0x1000;
                    break;
                case 2:
                    if (hit || shot->lifeTime <= gFrameSeconds)
                        goto carHit;
                    BodyVector(shot->vel, v1, uvs);
                    v1[1] = 0.9f*-v1[1];
                    WorldVector(v1, shot->vel, uvs);
                    EmitGameSound(shot->pos, unitmatrix[3], 400.0f, 0.0f, 1.0f, 0.0f, 21.0f + Random(3.0f), shot->slot, 0, 0x80);
                    break;
                default:
wallHit:
                    shot->lifeTime = 0.0000001f;
                    {
                        float side[3];
                        side[0] = shot->uvs[0][0]*scale;
                        side[1] = shot->uvs[0][1]*scale;
                        side[2] = shot->uvs[0][2]*scale;
                        v1[0] = shot->uvs[1][0]*scale;
                        v1[1] = shot->uvs[1][1]*scale;
                        v1[2] = shot->uvs[1][2]*scale;
                        verts[0][0] = shot->pos[0] - side[0];
                        verts[0][1] = shot->pos[1] - side[1];
                        verts[0][2] = shot->pos[2] - side[2];
                        verts[1][0] = shot->pos[0] + side[0];
                        verts[1][1] = shot->pos[1] + side[1];
                        verts[1][2] = shot->pos[2] + side[2];
                        verts[2][0] = shot->pos[0] + side[0];
                        verts[2][1] = shot->pos[1] + side[1];
                        verts[2][2] = shot->pos[2] + side[2];
                        verts[3][0] = shot->pos[0] - side[0];
                        verts[3][1] = shot->pos[1] - side[1];
                        verts[3][2] = shot->pos[2] - side[2];
                        verts[0][0] -= v1[0];
                        verts[0][1] -= v1[1];
                        verts[0][2] -= v1[2];
                        verts[1][0] -= v1[0];
                        verts[1][1] -= v1[1];
                        verts[1][2] -= v1[2];
                        verts[2][0] += v1[0];
                        verts[2][1] += v1[1];
                        verts[2][2] += v1[2];
                        verts[3][0] += v1[0];
                        verts[3][1] += v1[1];
                        verts[3][2] += v1[2];
                    }
                    if (!wallTidx)
                        MBOX_FindTexture_WORK("HIT_WALL", &wallTidx, 0, gNumModels-1, 1);
                    h = GetStatLLE(&gHitPolyList);
                    h->poly = ZOID_NewPoly(4, verts, wallTidx, &hitColor, 0x20f, 1);
                    if (!h->poly)
                        FreeStatLLE(&gHitPolyList, h);
                    break;
                }
            } else if (hit) {
carHit:
                shot->lifeTime = 0.0000001f;
                if (shot->type == 2 || shot->type == 4 || shot->type == 6) {
                    if (shot->streak)
                        shot->streak->flags |= 0x1000;
                    if (shot->type == 4 || shot->type == 6)
                        ProximityDamage(shot->pos, shot->old, shot->slot, 900.0f, damage);
                    else
                        ProximityDamage(shot->pos, old, shot->slot, 900.0f, damage);
                    StartBlast(shot->pos, 0, 0.5f, 1);
                } else {
                    if (shot->flags & 1)
                        damage *= 0.35f;
                    BattleDamage(car, hit, damage);
                    goto wallHit;
                }
            } else if (shot->lifeTime <= gFrameSeconds) {
                if (shot->type == 2 || shot->type == 4 || shot->type == 6) {
                    if (shot->type == 4 || shot->type == 6)
                        ProximityDamage(shot->pos, shot->old, shot->slot, 900.0f, damage);
                    else
                        ProximityDamage(shot->pos, old, shot->slot, 900.0f, damage);
                    StartBlast(shot->pos, 0, 0.5f, 1);
                    if (shot->streak)
                        shot->streak->flags |= 0x1000;
                    shot->lifeTime = 0.0000001f;
                }
            }
        } else if (shot->obj) {
            SonicCarCollision(shot);
            ((anon_ll_mbIdx_3c *)shot->obj)->mat.mat3.pos[0] = car->dr_pos[0];
            ((anon_ll_mbIdx_3c *)shot->obj)->mat.mat3.pos[1] = car->dr_pos[1];
            ((anon_ll_mbIdx_3c *)shot->obj)->mat.mat3.pos[2] = car->dr_pos[2];
            if (shot->incTime >= 0.0333333f) {
                shot->incTime -= 0.0333333f;
                ObjInstanceList[((anon_ll_mbIdx_3c *)shot->obj)->mbIdx].scale += 1.0f;
                ((anon_ll_mbIdx_3c *)shot->obj)->mat.mat3.uvs[0][0] = ObjInstanceList[((anon_ll_mbIdx_3c *)shot->obj)->mbIdx].scale;
                ((anon_ll_mbIdx_3c *)shot->obj)->mat.mat3.uvs[1][1] = ObjInstanceList[((anon_ll_mbIdx_3c *)shot->obj)->mbIdx].scale;
                ((anon_ll_mbIdx_3c *)shot->obj)->mat.mat3.uvs[2][2] = ObjInstanceList[((anon_ll_mbIdx_3c *)shot->obj)->mbIdx].scale;
            }
        }
        if (!(shot->flags & 0x70)) {
            shot->old[0] = old[0];
            shot->old[1] = old[1];
            shot->old[2] = old[2];
        }
        switch (shot->type) {
        case 0:
            if (!shot->obj)
                shot->obj = _CreateShotObject(2, -1, 0, 0);
            _UpdateShotObject(shot, 0);
            break;
        case 2:
            if (!shot->obj)
                shot->obj = _CreateShotObject(3, -1, 0, 0);
            _UpdateShotObject(shot, 0);
            break;
        case 3:
            _UpdateShotObject(shot, 2);
            break;
        case 4:
            if (!shot->obj)
                shot->obj = _CreateShotObject(7, -1, 0, 0);
            _UpdateShotObject(shot, 0);
            break;
        case 6:
            if (!shot->obj)
                shot->obj = _CreateShotObject(8, -1, 0, 0);
            _UpdateShotObject(shot, 0);
            break;
        case 7:
            if (!shot->obj) {
                shot->obj = _CreateShotObject(9, -1, 0x800000, 0);
                _UpdateShotObject(shot, 1);
            }
            break;
        case 1: case 8:
            if (!shot->obj)
                shot->obj = _CreateShotObject(1, -1, 0, 0);
            _UpdateShotObject(shot, 0);
            break;
        }
        if (shot->flags & 0x10) {
            shot->flags &= ~0x10;
            shot->flags |= 0x20;
        } else if (shot->flags & 0x20) {
            shot->flags &= ~0x20;
            shot->flags |= 0x40;
        } else if (shot->flags & 0x40) {
            shot->flags &= ~0x40;
        }
    }
}

/* 0x8003f0f4, 2220 bytes */
void ShotUpdate(void)
{
    int i;
    int StartStreak;
    int streaktype;
    anon_ll_slot_68 *shot;
    anon_ll_mbIdx_48 *h;
    anon_ll_poly_8 *hp, *nexth;
    anon_active_invsTime_10 *defense;
    tCarData *car;
    tModelData *m;
    anon_colfront_colrear_10 *sizep;

    AnimateCase();
    for (i = 0, car = gGameCar; i < gNumCars; i++, car++) {
        m = &model[car->slot];
        defense = &car->battle.defense;
        if (defense->active)
            UpdateDefense(car, m, defense, car->slot);
        switch (car->battle.weapon.type) {
        case 0: case 1: case 6: case 8:
            ElevateGun(car);
            break;
        }
        if (car->player->edges & car->player->controls[6])
            goto resetWeapon;
        if (!car->battle.weapon.ammoCnt) {
            if (car->battle.weapon.type == 5) {
                float z;
                sizep = &collsize[m->body_type];
                m->bodyR[0][2] = m->bodyR[1][2] = sizep->colfront;
                z = m->bodyR[0][2] > sizep->colrear ? m->bodyR[0][2] : sizep->colrear;
                m->colrad = sqrtf(z*z + sizep->colside*sizep->colside + sizep->colheight*sizep->colheight);
            }
resetWeapon:
            car->battle.weapon.type = 8;
            car->battle.weapon.ammoCnt = -1;
            car->battle.pitch = 0.0f;
            car->battle.yaw = 0.0f;
        }
        if (car->battle.reloadingTime > 0.0f)
            car->battle.reloadingTime -= gFrameSeconds;
        if (car->battle.reloadingTime <= 0.0f && car->battle.weapon.ammoCnt) {
            if (car->battle.weapon.type == 3) {
                if (car->battle.loaded)
                    continue;
                car->battle.loaded = 1;
            } else {
                if (!car->player || car->battle.weapon.type == 5 ||
                    (!(car->player->edges & car->player->controls[7]) &&
                     (car->battle.weapon.type != 1 || !(car->player->levels & car->player->controls[7]))) ||
                    m->crashflag || !car->collidable || car->dead)
                    continue;
                car->battle.shoot = 6;
            }
            shot = GetStatLLE(&gShotList);
            if (!shot)
                continue;
            _InitNewShot(shot, car);
            if (shot->type != 3)
                PlayGameSound(sWeaponFx[car->battle.weapon.type], car->slot, 1, 1);
            StartStreak = 0;
            switch (shot->type) {
            case 2:
                StartStreak = 1;
                streaktype = 1;
                PitchUV(0.5f, shot->uvs);
                car->battle.reloadingTime = 0.1666667f;
                shot->lifeTime = 3.1666667f;
                shot->radius = 15.0f;
                shot->vel[0] = car->dr_vel[0] + 75.0f*shot->uvs[2][0];
                shot->vel[1] = car->dr_vel[1] + 75.0f*shot->uvs[2][1];
                shot->vel[2] = car->dr_vel[2] + 75.0f*shot->uvs[2][2];
                break;
            case 4:
                StartStreak = 1;
                streaktype = 0;
                car->battle.reloadingTime = 1.0f;
                shot->lifeTime = 15.0f;
                shot->radius = 2.0f;
                shot->vel[2] = 165.0f + (car->dr_vel[0]*shot->uvs[2][0] + car->dr_vel[1]*shot->uvs[2][1] + car->dr_vel[2]*shot->uvs[2][2]);
                break;
            case 6:
                StartStreak = 1;
                streaktype = 2;
                car->battle.reloadingTime = 0.1666667f;
                shot->lifeTime = 10.0f;
                shot->radius = 1.0f;
                PitchUV(car->battle.pitch, shot->uvs);
                shot->vel[0] = car->dr_vel[0] + 50.0f*shot->uvs[2][0];
                shot->vel[1] = car->dr_vel[1] + 50.0f*shot->uvs[2][1];
                shot->vel[2] = car->dr_vel[2] + 50.0f*shot->uvs[2][2];
                break;
            case 8:
                car->battle.reloadingTime = 0.1666667f;
                shot->lifeTime = 4.0f;
                shot->radius = 0.5f;
                YawUV(-car->battle.yaw, shot->uvs);
                PitchUV(car->battle.pitch, shot->uvs);
                break;
            case 0:
                car->battle.reloadingTime = 0.1666667f;
                shot->lifeTime = 4.0f;
                shot->radius = 0.5f;
                PitchUV(car->battle.pitch, shot->uvs);
                break;
            case 1:
                car->battle.reloadingTime = 0.0666667f;
                shot->lifeTime = 4.0f;
                shot->radius = 0.25f;
                YawUV(Random(0.025f) - 0.0125f - car->battle.yaw, shot->uvs);
                PitchUV(car->battle.pitch + (Random(0.025f) - 0.0125f), shot->uvs);
                h = GetStatLLE(&gShotCaseList);
                if (h) {
                    float x = weaponOffsets[1][m->body_type][0] + CaseOffsets[0][0];
                    float y = weaponOffsets[1][m->body_type][1] + CaseOffsets[0][1];
                    float z = weaponOffsets[1][m->body_type][2] + CaseOffsets[0][2];
                    h->mat.mat3.pos[0] = shot->pos[0] + car->dr_uvs[2][0]*z;
                    h->mat.mat3.pos[1] = shot->pos[1] + car->dr_uvs[2][1]*z;
                    h->mat.mat3.pos[2] = shot->pos[2] + car->dr_uvs[2][2]*z;
                    h->mat.mat3.pos[0] += car->dr_uvs[1][0]*y;
                    h->mat.mat3.pos[1] += car->dr_uvs[1][1]*y;
                    h->mat.mat3.pos[2] += car->dr_uvs[1][2]*y;
                    h->mat.mat3.pos[0] += car->dr_uvs[0][0]*x;
                    h->mat.mat3.pos[1] += car->dr_uvs[0][1]*x;
                    h->mat.mat3.pos[2] += car->dr_uvs[0][2]*x;
                    CopyUV(car->dr_uvs, h->mat.mat3.uvs);
                    RollUV(0.3f + Random(0.23f), h->mat.mat3.uvs);
                    h->mbIdx = MBOX_NewObject(gWprObjList[0], &h->mat, -1, 0);
                    h->time = 0.5f + Random(0.3f);
                    h->vel[0] = car->dr_vel[0] + 28.0f*h->mat.mat3.uvs[1][0];
                    h->vel[1] = car->dr_vel[1] + 28.0f*h->mat.mat3.uvs[1][1];
                    h->vel[2] = car->dr_vel[2] + 28.0f*h->mat.mat3.uvs[1][2];
                    h->vel[2] *= 0.88f;
                    h->vel[1] *= 0.88f;
                    h->vel[0] *= 0.88f;
                }
                break;
            case 7:
                car->battle.reloadingTime = 1.0f;
                shot->lifeTime = 1.0f;
                shot->radius = 2.0f;
                break;
            }
            {
                float x, y, z;
                if (car->battle.weapon.type >= 8) {
                    x = ShotOffsets[car->battle.weapon.type][0];
                    y = ShotOffsets[car->battle.weapon.type][1];
                    z = ShotOffsets[car->battle.weapon.type][2];
                } else {
                    x = ShotOffsets[car->battle.weapon.type][0] + weaponOffsets[car->battle.weapon.type][m->body_type][0];
                    y = ShotOffsets[car->battle.weapon.type][1] + weaponOffsets[car->battle.weapon.type][m->body_type][1];
                    z = ShotOffsets[car->battle.weapon.type][2] + weaponOffsets[car->battle.weapon.type][m->body_type][2];
                }
                shot->pos[0] += car->dr_uvs[2][0]*z;
                shot->pos[1] += car->dr_uvs[2][1]*z;
                shot->pos[2] += car->dr_uvs[2][2]*z;
                shot->pos[0] += car->dr_uvs[1][0]*y;
                shot->pos[1] += car->dr_uvs[1][1]*y;
                shot->pos[2] += car->dr_uvs[1][2]*y;
                shot->pos[0] += car->dr_uvs[0][0]*x;
                shot->pos[1] += car->dr_uvs[0][1]*x;
                shot->pos[2] += car->dr_uvs[0][2]*x;
            }
            if (StartStreak)
                /* Preserve the original call's extra position argument, unused by NewStreak. */
                shot->streak = ((tStreak *(*)(int, float *, float *))NewStreak)(streaktype, shot->pos, shot->pos);
            else
                shot->streak = 0;
            if (car->battle.weapon.ammoCnt > 0 && car->battle.weapon.type != 5 && car->battle.weapon.type != 3)
                car->battle.weapon.ammoCnt--;
        }
    }
    for (hp = gHitPolyList.head; hp; hp = nexth) {
        nexth = (anon_ll_poly_8 *)hp->ll.next;
        ZOID_RemovePoly(hp->poly);
        FreeStatLLE(&gHitPolyList, hp);
    }
    UpdateShots();
    UpdateStreaks();
}
