/* weapons.c - Battle weapons and muzzle-flash animation. Status: Matching.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\weapons.c; assembly: build/2049LE/asm/GameSource/src/weapons.s */
#include "rush/rush_protos.h"

static void _RemoveGattMFlash(signed short slot);
static void _RemoveCannMFlash(signed short slot);
static void AnimateWeapon(tVisual *v, signed short op);

float weaponOffsets[8][13][3] = {
    0.0f, 2.5f, 0.3f, 0.0f, 3.0f, 0.3f, 0.0f, 2.8f, 0.3f, 0.0f, 3.0f, 0.1f, 0.0f, 3.0f, 0.1f, 0.0f,
    3.4f, -0.4f, 0.0f, 3.1f, 0.3f, 0.0f, 3.3f, 1.0f, 0.0f, 3.39f, -1.82f, 0.0f, 3.05f, 0.65f, 0.0f,
    5.04f, 1.33f, 0.0f, 2.95f, -0.9f, 0.0f, 2.79f, 0.52f, 0.0f, 2.7f, 2.1f, 0.0f, 3.1f, 1.1f, 0.0f,
    2.7f, 0.1f, 0.0f, 2.9f, -0.1f, 0.0f, 2.9f, 0.4f, 0.0f, 3.5f, 0.1f, 0.0f, 3.0f, 0.3f, 0.0f,
    3.3f, 1.2f, 0.0f, 3.35f, -0.69f, 0.0f, 3.04f, 0.97f, 0.0f, 5.05f, 2.02f, 0.0f, 2.98f, -0.31f,
    0.0f, 2.79f, 0.92f, 0.0f, 2.5f, 1.3f, 0.0f, 3.0f, -0.5f, 0.0f, 2.6f, -0.6f, 0.0f, 2.7f, -1.1f,
    0.0f, 2.7f, -1.1f, 0.0f, 3.3f, -1.6f, 0.0f, 2.9f, -0.6f, 0.0f, 3.0f, 0.1f, 0.0f, 3.13f, -1.73f,
    0.0f, 2.89f, -0.41f, 0.0f, 4.84f, 0.37f, 0.0f, 2.75f, -1.67f, 0.0f, 2.66f, -0.35f, 0.0f, 1.9f,
    -5.3f, 0.0f, 1.7f, -5.7f, 0.0f, 1.9f, -5.6f, 0.0f, 2.0f, -5.9f, 0.0f, 1.4f, -6.1f, 0.0f, 1.4f,
    -7.0f, 0.0f, 1.9f, -5.7f, 0.0f, 1.8f, -6.0f, 0.0f, 1.3f, -5.71f, 0.0f, 1.6f, -6.73f, 0.0f,
    1.71f, -9.28f, 0.0f, 1.74f, -6.95f, 0.0f, 1.28f, -5.54f, 0.0f, 2.2f, 0.6f, 0.0f, 2.6f, -0.7f,
    0.0f, 2.4f, -0.8f, 0.0f, 2.4f, -1.1f, 0.0f, 2.6f, -0.5f, 0.0f, 3.0f, -1.7f, 0.0f, 2.6f, -0.5f,
    0.0f, 2.7f, -0.5f, 0.0f, 3.16f, -2.52f, 0.0f, 2.67f, -0.39f, 0.0f, 4.8f, 0.44f, 0.0f, 2.52f,
    -2.15f, 0.0f, 2.37f, -0.44f, 0.0f, 0.0f, 6.4f, 0.0f, 0.0f, 6.7f, 0.0f, 0.0f, 6.0f, 0.0f, 0.0f,
    6.0f, 0.0f, 0.2f, 6.4f, 0.0f, 0.2f, 6.8f, 0.0f, 0.1f, 5.9f, 0.0f, 0.2f, 5.8f, 0.0f, 0.25f,
    4.86f, 0.0f, 0.44f, 7.38f, 0.0f, -0.18f, 8.07f, 0.0f, 0.14f, 5.89f, 0.0f, 0.14f, 5.89f, 0.0f,
    2.9f, 0.3f, 0.0f, 3.3f, -0.7f, 0.0f, 3.1f, -0.9f, 0.0f, 3.2f, -0.9f, 0.0f, 3.2f, -0.9f, 0.0f,
    3.6f, -1.4f, 0.0f, 3.3f, -0.7f, 0.0f, 3.4f, -0.3f, 0.0f, 3.65f, -2.06f, 0.0f, 3.17f, -0.44f,
    0.0f, 5.28f, 0.66f, 0.0f, 3.15f, -1.52f, 0.0f, 3.08f, -0.6f, 0.0f, 2.6f, 0.4f, 0.0f, 3.0f,
    -0.8f, 0.0f, 2.7f, -0.8f, 0.0f, 3.1f, -0.4f, 0.0f, 3.1f, -0.2f, 0.0f, 3.4f, -1.8f, 0.0f, 3.1f,
    -0.9f, 0.0f, 3.1f, -0.9f, 0.0f, 3.52f, -2.0f, 0.0f, 3.4f, -0.33f, 0.0f, 4.38f, -4.39f, 0.0f,
    3.25f, -2.05f, 0.0f, 3.12f, -1.11f
};
static signed char gCurWeaponType[4];
static tMuzzleObj gGattMuzzleObjs[4];
static tCMuzzleObj gCannMuzzleObjs[4];

/* 0x8005c600, size 0x90 */
void InitBattlePlayer(signed short slot)
{
    gGameCar[slot].battle.defense.active = 0;
    gGameCar[slot].battle.fadeState = 0;
    gGameCar[slot].battle.shoot = 0;
    gGameCar[slot].battle.loaded = 0;
    gGameCar[slot].battle.weapon.type = 8;
    gGameCar[slot].battle.weapon.ammoCnt = -1;
    gCurWeaponType[slot] = 9;
    gCannMuzzleObjs[slot].ripdl = -1;
    gCannMuzzleObjs[slot].ripCnt = -1;
    gCannMuzzleObjs[slot].onTime = 0.0f;
    gGattMuzzleObjs[slot].onTime = 0.0f;
    {
        int i;
        for (i = 0; i < 5; i++)
            gGattMuzzleObjs[slot].mdls[i] = -1;
    }
}

/* 0x8005c690, size 0x78 */
static void _RemoveGattMFlash(signed short slot)
{
    int i; /* r29 */
    for (i = 0; i < 5; i++) {
        if (gGattMuzzleObjs[slot].mdls[i] != -1) {
            ZOID_RemoveObject(gGattMuzzleObjs[slot].mdls[i]);
            gGattMuzzleObjs[slot].mdls[i] = -1;
        }
    }
}

/* 0x8005c708, size 0xb8 */
static void _RemoveCannMFlash(signed short slot)
{
    int i; /* r28 */
    for (i = 0; i < 5; i++) {
        if (gCannMuzzleObjs[slot].mdls[i] != -1) {
            ZOID_RemoveObject(gCannMuzzleObjs[slot].mdls[i]);
            gCannMuzzleObjs[slot].mdls[i] = -1;
        }
    }
    if (gCannMuzzleObjs[slot].ripdl != -1) {
        ZOID_RemoveObject(gCannMuzzleObjs[slot].ripdl);
        gCannMuzzleObjs[slot].ripdl = -1;
        gCannMuzzleObjs[slot].ripTIdx = -1;
    }
}

/* 0x8005c7c0, size 0xf4 */
void StartWeapon(signed short slot)
{
    tVisual *v; /* r31 */
    v = &gGameCar[slot].visuals[21];
    if (v->objnum < 0) {
        v->index = 0;
        v->func = AnimateWeapon;
        v->timeStamp = gSeconds;
        v->slot = slot;
        v->objnum = gCarParts[slot].weap = InitCarObj(gObjList[531], 15, gCarParts[slot].frame, 0x2000, slot, -1, 1);
        ZOID_UpdateObject(v->objnum, weaponOffsets[1][model[slot].body_type], 0);
    }
}

/* 0x8005c8b4, size 0x14a4 */
static void AnimateWeapon(tVisual *v, signed short op)
{
    tCarData *car = &gGameCar[v->slot];
    int def;
    int i;
    int r1;
    float r2;
    anon_c_rgba_4 clr;
    anon_c_rgba_4 clr2 = { 0, 255, 0, 255 };
    anon_mat3_mat4_30 *mat;

    if (!op || model[v->slot].crashflag) {
        if (gCurWeaponType[v->slot] == 1)
            _RemoveGattMFlash(v->slot);
        else if (gCurWeaponType[v->slot] == 0)
            _RemoveCannMFlash(v->slot);
        car->battle.weapon.type = 8;
        car->battle.weapon.ammoCnt = -1;
        MBOX_HideObject(v->objnum, 0, 15);
        return;
    }
    clr.rgba = car->IRclr.rgba;
    if (car->battle.defense.active & 1) {
        clr.c[3] = car->battle.fadeAlpha;
        if (clr.c[3] < 48)
            clr.c[3] = 48;
    }
    ZOID_SetColor(gCarParts[v->slot].weap, &clr);
    mat = ZOID_FindMatrix(gCarParts[v->slot].frame);
    if (car->battle.weapon.type == 8) {
        MBOX_HideObject(v->objnum, 1, 15);
    } else if (car->camera >= 0 && car->view <= 1) {
        MBOX_ShowObject(v->objnum, 0, 15);
        MBOX_HideObject(v->objnum, 1, 1 << car->camera);
    } else {
        MBOX_ShowObject(v->objnum, 0, 15);
    }
    if (gCurWeaponType[v->slot] != car->battle.weapon.type) {
        float x = weaponOffsets[car->battle.weapon.type][model[v->slot].body_type][0];
        float y = weaponOffsets[car->battle.weapon.type][model[v->slot].body_type][1];
        float z = weaponOffsets[car->battle.weapon.type][model[v->slot].body_type][2];
        if (gCurWeaponType[v->slot] == 1)
            _RemoveGattMFlash(v->slot);
        else if (gCurWeaponType[v->slot] == 0)
            _RemoveCannMFlash(v->slot);
        gCurWeaponType[v->slot] = car->battle.weapon.type;
        switch (car->battle.weapon.type) {
        case 8:
            return;
        case 0:
            def = 530;
            x += 0.16f;
            y += 0.48f;
            z += 2.57f;
            for (i = 0; i < 5; i++) {
                CopyUV(mat->mat3.uvs, gCannMuzzleObjs[v->slot].mats[i].mat.mat3.uvs);
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0] = mat->mat3.uvs[0][0] * x + mat->mat3.pos[0];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1] = mat->mat3.uvs[0][1] * x + mat->mat3.pos[1];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2] = mat->mat3.uvs[0][2] * x + mat->mat3.pos[2];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0] = mat->mat3.uvs[1][0] * y + gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1] = mat->mat3.uvs[1][1] * y + gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2] = mat->mat3.uvs[1][2] * y + gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0] = mat->mat3.uvs[2][0] * z + gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1] = mat->mat3.uvs[2][1] * z + gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2] = mat->mat3.uvs[2][2] * z + gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2];
                gCannMuzzleObjs[v->slot].mdls[i] = MBOX_NewObject(gWfxObjList[i], &gCannMuzzleObjs[v->slot].mats[i].mat, -1, 0x42000);
                ZOID_SetColor(gCannMuzzleObjs[v->slot].mdls[i], &clr2);
                MBOX_HideObject(gCannMuzzleObjs[v->slot].mdls[i], 1, 15);
            }
            CopyUV(mat->mat3.uvs, gCannMuzzleObjs[v->slot].mat.mat3.uvs);
            gCannMuzzleObjs[v->slot].mat.mat3.pos[0] = mat->mat3.uvs[0][0] * x + mat->mat3.pos[0];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[1] = mat->mat3.uvs[0][1] * x + mat->mat3.pos[1];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[2] = mat->mat3.uvs[0][2] * x + mat->mat3.pos[2];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[0] = mat->mat3.uvs[1][0] * y + gCannMuzzleObjs[v->slot].mat.mat3.pos[0];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[1] = mat->mat3.uvs[1][1] * y + gCannMuzzleObjs[v->slot].mat.mat3.pos[1];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[2] = mat->mat3.uvs[1][2] * y + gCannMuzzleObjs[v->slot].mat.mat3.pos[2];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[0] = mat->mat3.uvs[2][0] * z + gCannMuzzleObjs[v->slot].mat.mat3.pos[0];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[1] = mat->mat3.uvs[2][1] * z + gCannMuzzleObjs[v->slot].mat.mat3.pos[1];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[2] = mat->mat3.uvs[2][2] * z + gCannMuzzleObjs[v->slot].mat.mat3.pos[2];
            gCannMuzzleObjs[v->slot].ripdl = MBOX_NewObject(gWfxObjList[6], &gCannMuzzleObjs[v->slot].mat, -1, 0x42000);
            ZOID_SetColor(gCannMuzzleObjs[v->slot].ripdl, &clr2);
            MBOX_HideObject(gCannMuzzleObjs[v->slot].ripdl, 1, 15);
            gCannMuzzleObjs[v->slot].ripCnt = -1;
            gCannMuzzleObjs[v->slot].ripTime = 0.0f;
            gCannMuzzleObjs[v->slot].ripScale = 1.5f + Random(1.5f);
            break;
        case 1:
            def = 531;
            x += 0.01f;
            y += 0.65f;
            z += 3.59f;
            for (i = 0; i < 5; i++) {
                CopyUV(mat->mat3.uvs, gGattMuzzleObjs[v->slot].mats[i].mat.mat3.uvs);
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0] = mat->mat3.uvs[0][0] * x + mat->mat3.pos[0];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1] = mat->mat3.uvs[0][1] * x + mat->mat3.pos[1];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2] = mat->mat3.uvs[0][2] * x + mat->mat3.pos[2];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0] = mat->mat3.uvs[1][0] * y + gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1] = mat->mat3.uvs[1][1] * y + gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2] = mat->mat3.uvs[1][2] * y + gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0] = mat->mat3.uvs[2][0] * z + gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1] = mat->mat3.uvs[2][1] * z + gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2] = mat->mat3.uvs[2][2] * z + gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2];
                gGattMuzzleObjs[v->slot].mdls[i] = MBOX_NewObject(gWfxObjList[i], &gGattMuzzleObjs[v->slot].mats[i].mat, -1, 0);
                MBOX_HideObject(gGattMuzzleObjs[v->slot].mdls[i], 1, 15);
            }
            break;
        case 2: def = 532; break;
        case 3: def = 533; break;
        case 4: def = 534; break;
        case 5: def = 535; break;
        case 6: def = 536; break;
        case 7: def = 537; break;
        }
        ZOID_SetObjectDef(gCarParts[v->slot].weap, gObjList[def]);
        ZOID_UpdateObject(gCarParts[v->slot].weap, weaponOffsets[car->battle.weapon.type][model[v->slot].body_type], 0);
    }
    {
        float x = weaponOffsets[car->battle.weapon.type][model[v->slot].body_type][0];
        float y = weaponOffsets[car->battle.weapon.type][model[v->slot].body_type][1];
        float z = weaponOffsets[car->battle.weapon.type][model[v->slot].body_type][2];
        if (gCurWeaponType[v->slot] == 1) {
            x += 0.01f;
            y += 0.65f;
            z += 3.59f;
            for (i = 0; i < 5; i++) {
                CopyUV(mat->mat3.uvs, gGattMuzzleObjs[v->slot].mats[i].mat.mat3.uvs);
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0] = mat->mat3.uvs[0][0] * x + mat->mat3.pos[0];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1] = mat->mat3.uvs[0][1] * x + mat->mat3.pos[1];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2] = mat->mat3.uvs[0][2] * x + mat->mat3.pos[2];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0] = mat->mat3.uvs[1][0] * y + gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1] = mat->mat3.uvs[1][1] * y + gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2] = mat->mat3.uvs[1][2] * y + gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0] = mat->mat3.uvs[2][0] * z + gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1] = mat->mat3.uvs[2][1] * z + gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1];
                gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2] = mat->mat3.uvs[2][2] * z + gGattMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2];
            }
            if (gGattMuzzleObjs[v->slot].on) {
                gGattMuzzleObjs[v->slot].onTime += gFrameSeconds;
                if (gGattMuzzleObjs[v->slot].onTime >= 0.0333333f) {
                    gGattMuzzleObjs[v->slot].on = 0;
                    for (i = 0; i < 5; i++)
                        MBOX_HideObject(gGattMuzzleObjs[v->slot].mdls[i], 1, 15);
                }
            }
            if (car->battle.shoot >= 5) {
                gGattMuzzleObjs[v->slot].on = 1;
                gGattMuzzleObjs[v->slot].onTime = 0.0f;
                MBOX_ShowObject(gGattMuzzleObjs[v->slot].mdls[0], 0, 15);
                if (car->camera >= 0 && car->view <= 1)
                    MBOX_HideObject(gGattMuzzleObjs[v->slot].mdls[0], 1, 1 << car->camera);
                r1 = (int)Random(4.0f) + 1;
                r2 = Random(0.5f);
                RollUV(r2, gGattMuzzleObjs[v->slot].mats[r1].mat.mat3.uvs);
                ScaleUV(gGattMuzzleObjs[v->slot].mats[r1].mat.mat3.uvs, gGattMuzzleObjs[v->slot].mats[r1].mat.mat3.uvs, 0.85f + Random(0.75f));
                MBOX_ShowObject(gGattMuzzleObjs[v->slot].mdls[r1], 0, 15);
                if (car->camera >= 0 && car->view <= 1)
                    MBOX_HideObject(gGattMuzzleObjs[v->slot].mdls[r1], 1, 1 << car->camera);
                r1++;
                if (r1 >= 5)
                    r1 = 1;
                RollUV(r2, gGattMuzzleObjs[v->slot].mats[r1].mat.mat3.uvs);
                ScaleUV(gGattMuzzleObjs[v->slot].mats[r1].mat.mat3.uvs, gGattMuzzleObjs[v->slot].mats[r1].mat.mat3.uvs, 0.85f + Random(0.75f));
                MBOX_ShowObject(gGattMuzzleObjs[v->slot].mdls[r1], 0, 15);
                if (car->camera >= 0 && car->view <= 1)
                    MBOX_HideObject(gGattMuzzleObjs[v->slot].mdls[r1], 1, 1 << car->camera);
            }
        }
        if (gCurWeaponType[v->slot] == 0) {
            x += 0.16f;
            y += 0.48f;
            z += 2.57f;
            for (i = 0; i < 5; i++) {
                CopyUV(mat->mat3.uvs, gCannMuzzleObjs[v->slot].mats[i].mat.mat3.uvs);
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0] = mat->mat3.uvs[0][0] * x + mat->mat3.pos[0];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1] = mat->mat3.uvs[0][1] * x + mat->mat3.pos[1];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2] = mat->mat3.uvs[0][2] * x + mat->mat3.pos[2];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0] = mat->mat3.uvs[1][0] * y + gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1] = mat->mat3.uvs[1][1] * y + gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2] = mat->mat3.uvs[1][2] * y + gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0] = mat->mat3.uvs[2][0] * z + gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[0];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1] = mat->mat3.uvs[2][1] * z + gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[1];
                gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2] = mat->mat3.uvs[2][2] * z + gCannMuzzleObjs[v->slot].mats[i].mat.mat3.pos[2];
            }
            CopyUV(mat->mat3.uvs, gCannMuzzleObjs[v->slot].mat.mat3.uvs);
            gCannMuzzleObjs[v->slot].mat.mat3.pos[0] = mat->mat3.uvs[0][0] * x + mat->mat3.pos[0];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[1] = mat->mat3.uvs[0][1] * x + mat->mat3.pos[1];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[2] = mat->mat3.uvs[0][2] * x + mat->mat3.pos[2];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[0] = mat->mat3.uvs[1][0] * y + gCannMuzzleObjs[v->slot].mat.mat3.pos[0];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[1] = mat->mat3.uvs[1][1] * y + gCannMuzzleObjs[v->slot].mat.mat3.pos[1];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[2] = mat->mat3.uvs[1][2] * y + gCannMuzzleObjs[v->slot].mat.mat3.pos[2];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[0] = mat->mat3.uvs[2][0] * z + gCannMuzzleObjs[v->slot].mat.mat3.pos[0];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[1] = mat->mat3.uvs[2][1] * z + gCannMuzzleObjs[v->slot].mat.mat3.pos[1];
            gCannMuzzleObjs[v->slot].mat.mat3.pos[2] = mat->mat3.uvs[2][2] * z + gCannMuzzleObjs[v->slot].mat.mat3.pos[2];
            if (gCannMuzzleObjs[v->slot].ripCnt != -1) {
                gCannMuzzleObjs[v->slot].ripTime -= gFrameSeconds;
                if (gCannMuzzleObjs[v->slot].ripTime <= 0.0f) {
                    gCannMuzzleObjs[v->slot].ripTime = 0.0333333f;
                    gCannMuzzleObjs[v->slot].ripCnt++;
                    if (gCannMuzzleObjs[v->slot].ripCnt >= 5) {
                        MBOX_HideObject(gCannMuzzleObjs[v->slot].ripdl, 1, 15);
                        gCannMuzzleObjs[v->slot].ripCnt = -1;
                    } else {
                        gCannMuzzleObjs[v->slot].ripTIdx++;
                        if (gCannMuzzleObjs[v->slot].ripTIdx >= 5)
                            gCannMuzzleObjs[v->slot].ripTIdx = 0;
                        ZOID_SetTexture(gCannMuzzleObjs[v->slot].ripdl, &gTexDefPtrs[gRipTextures[gCannMuzzleObjs[v->slot].ripTIdx] >> 10].texDefs[gRipTextures[gCannMuzzleObjs[v->slot].ripTIdx] & 1023], -1);
                    }
                }
            }
            if (gCannMuzzleObjs[v->slot].on) {
                gCannMuzzleObjs[v->slot].onTime += gFrameSeconds;
                if (gCannMuzzleObjs[v->slot].onTime >= 0.0666667f) {
                    gCannMuzzleObjs[v->slot].on = 0;
                    for (i = 0; i < 5; i++)
                        MBOX_HideObject(gCannMuzzleObjs[v->slot].mdls[i], 1, 15);
                }
            }
            if (car->battle.shoot >= 5) {
                gCannMuzzleObjs[v->slot].ripScale = 1.5f + Random(1.5f);
                gCannMuzzleObjs[v->slot].ripTime = 0.0333333f;
                gCannMuzzleObjs[v->slot].ripCnt = 0;
                gCannMuzzleObjs[v->slot].ripTIdx = (int)Random(5.0f);
                ZOID_SetTexture(gCannMuzzleObjs[v->slot].ripdl, &gTexDefPtrs[gRipTextures[0] >> 10].texDefs[gRipTextures[0] & 1023], -1);
                MBOX_ShowObject(gCannMuzzleObjs[v->slot].ripdl, 0, 15);
                if (car->camera >= 0 && car->view <= 1)
                    MBOX_HideObject(gCannMuzzleObjs[v->slot].ripdl, 1, 1 << car->camera);
                gCannMuzzleObjs[v->slot].on = 1;
                gCannMuzzleObjs[v->slot].onTime = 0.0f;
                MBOX_ShowObject(gCannMuzzleObjs[v->slot].mdls[0], 0, 15);
                if (car->camera >= 0 && car->view <= 1)
                    MBOX_HideObject(gCannMuzzleObjs[v->slot].mdls[0], 1, 1 << car->camera);
                for (i = 1; i < 5; i++) {
                    RollUV(Random(1.25f) - 0.5f, gCannMuzzleObjs[v->slot].mats[i].mat.mat3.uvs);
                    ScaleUV(gCannMuzzleObjs[v->slot].mats[i].mat.mat3.uvs, gCannMuzzleObjs[v->slot].mats[i].mat.mat3.uvs, 0.4f + Random(0.25f));
                    MBOX_ShowObject(gCannMuzzleObjs[v->slot].mdls[i], 0, 15);
                    if (car->camera >= 0 && car->view <= 1)
                        MBOX_HideObject(gCannMuzzleObjs[v->slot].mdls[i], 1, 1 << car->camera);
                }
            }
            ScaleUV(gCannMuzzleObjs[v->slot].mat.mat3.uvs, gCannMuzzleObjs[v->slot].mat.mat3.uvs, gCannMuzzleObjs[v->slot].ripScale);
        }
    }
}
