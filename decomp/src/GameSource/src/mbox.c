/* mbox.c - model box: object and texture lookup by name, and object hiding.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\mbox.c; assembly: build/2049LE/asm/GameSource/src/mbox.s */
#include "rush/rush_protos.h"

anon_romObjDefs_count_8 gRomObjDefPtrs[64] = { 0 };
unsigned char gNumModels;
anon_palDefs_count_8 gPalDefPtrs[64];
anon_texDefs_count_8 gTexDefPtrs[64];

static int objcmp(const anon_name_radius_48 *obj1, const anon_name_radius_48 *obj2);
static int texcmp(const anon_name_width_30 *ta, const anon_name_width_30 *tb);

/* 0x80028b40, size 0x24 */
signed short MBOX_NewObject(int obj, anon_mat3_mat4_30 *mat, int parent, unsigned int flags)
{
    signed short dlidx; /* r0 */
    dlidx = ZOID_AddObject(obj, mat, parent, flags);
    return dlidx;
}

/* 0x80028b64, size 0x38 */
signed short MBOX_NewObjectCam(int obj, anon_mat3_mat4_30 *mat, signed short camlink, int parent, unsigned int flags)
{
    signed short dlidx; /* r0 */
    dlidx = ZOID_AddObject(obj, mat, parent, flags | ((camlink << 8) ^ 0xf00));
    return dlidx;
}

/* 0x80028b9c, size 0x18 */
int MBOX_GetChild(int dlidx)
{
    return ObjInstanceList[dlidx].child;
}

/* 0x80028bb4, size 0x18 */
int MBOX_GetNext(int dlidx)
{
    return ObjInstanceList[dlidx].next;
}

/* 0x80028bcc, size 0x114 */
void MBOX_HideObject(int dlidx, int mode, int camlink)
{
    unsigned int flags; /* r4 */
    int tmpidx; /* r0 */
    if (mode == 2) {
        flags = ZOID_ObjectFlags(dlidx);
        flags |= camlink << 8;
        ZOID_SetObjectFlags(dlidx, flags);
        tmpidx = MBOX_GetChild(dlidx);
        if (tmpidx != -1) {
            MBOX_HideObject(tmpidx, 3, camlink);
        }
    } else if (mode == 0) {
        flags = ZOID_ObjectFlags(dlidx);
        ZOID_SetObjectFlags(dlidx, flags | 0x80000000);
    } else if (mode == 1) {
        flags = ZOID_ObjectFlags(dlidx);
        flags |= camlink << 8;
        ZOID_SetObjectFlags(dlidx, flags);
    } else if (mode == 3) {
        do {
            flags = ZOID_ObjectFlags(dlidx);
            flags |= camlink << 8;
            ZOID_SetObjectFlags(dlidx, flags);
            tmpidx = MBOX_GetChild(dlidx);
            if (tmpidx != -1) {
                MBOX_HideObject(tmpidx, 3, camlink);
            }
            dlidx = MBOX_GetNext(dlidx);
        } while (dlidx != -1);
    }
}

/* 0x80028ce0, size 0x124 */
void MBOX_ShowObject(int dlidx, int mode, int camlink)
{
    unsigned int flags; /* r4 */
    int tmpidx; /* r0 */
    if (mode == 1) {
        flags = ZOID_ObjectFlags(dlidx);
        flags &= ~((camlink << 8) | 0x80000000);
        ZOID_SetObjectFlags(dlidx, flags);
        tmpidx = MBOX_GetChild(dlidx);
        if (tmpidx != -1) {
            MBOX_ShowObject(tmpidx, 2, camlink);
        }
    } else if (mode == 0) {
        flags = ZOID_ObjectFlags(dlidx);
        flags &= ~((camlink << 8) | 0x80000000);
        ZOID_SetObjectFlags(dlidx, flags);
    } else if (mode == 0) {
        /* unreachable: tests mode == 0 a second time */
        flags = ZOID_ObjectFlags(dlidx);
        flags &= ~0x80000000;
        flags |= camlink << 8;
        ZOID_SetObjectFlags(dlidx, flags);
    } else if (mode == 2) {
        do {
            flags = ZOID_ObjectFlags(dlidx);
            flags &= ~((camlink << 8) | 0x80000000);
            ZOID_SetObjectFlags(dlidx, flags);
            tmpidx = MBOX_GetChild(dlidx);
            if (tmpidx != -1) {
                MBOX_ShowObject(tmpidx, 2, camlink);
            }
            dlidx = MBOX_GetNext(dlidx);
        } while (dlidx != -1);
    }
}

/* 0x80028e04, size 0x3c */
static int objcmp(const anon_name_radius_48 *obj1, const anon_name_radius_48 *obj2)
{
    if (!obj1 || !obj2) {
        return -1;
    }
    return strncmp(obj1->name, obj2->name, 15);
}

/* 0x80028e40, size 0xc4 */
void MBOX_GetPosition(float *pos, int dlidx)
{
    pos[0] = ObjInstanceList[dlidx].mat->uvs[3][0];
    pos[1] = ObjInstanceList[dlidx].mat->uvs[3][1];
    pos[2] = ObjInstanceList[dlidx].mat->uvs[3][2];
    while ((dlidx = ZOID_TrueObjParent(dlidx)) != -1) {
        pos[0] += ObjInstanceList[dlidx].mat->uvs[3][0];
        pos[1] += ObjInstanceList[dlidx].mat->uvs[3][1];
        pos[2] += ObjInstanceList[dlidx].mat->uvs[3][2];
    }
}

/* 0x80028f04, size 0x54 */
float MBOX_GetObjectRadius(int mbidx)
{
    int model_num; /* r4 */
    int object_num; /* r6 */
    model_num = mbidx >> 10;
    object_num = mbidx & 0x3ff;
    if (model_num >= gNumModels) {
        return 0.0f;
    }
    if (object_num >= gRomObjDefPtrs[model_num].count) {
        return 0.0f;
    }
    return gRomObjDefPtrs[model_num].romObjDefs[object_num].radius;
}

/* 0x80028f58, size 0x140 */
/* Callers pass a fourth argument (see overrides.json); the DWARF definition has three parameters. */
int MBOX_FindObject_WORK(const char *name, signed char firstModel, signed char lastModel, int flag)
{
    int i; /* r27 */
    anon_name_radius_48 odef; /* stack */
    anon_name_radius_48 *ptr; /* r31 */
    ptr = 0;
    if (!name || !name[0]) {
        strcpy(odef.name, "AAANULLOBJ");
    } else {
        strncpy(odef.name, name, 16);
    }
    if (firstModel < 0) {
        firstModel = 0;
    }
    if (lastModel < 0 || lastModel >= gNumModels) {
        lastModel = gNumModels - 1;
    }
    for (i = firstModel; i <= lastModel; i++) {
        if (ModelActive(i)) {
            ptr = bsearch(&odef, gRomObjDefPtrs[i].romObjDefs, gRomObjDefPtrs[i].count, sizeof(anon_name_radius_48),
                          (int (*)(void *, void *))objcmp);
            if (ptr) {
                break;
            }
        }
    }
    if (!ptr) {
        return -1;
    }
    return (i << 10) | (ptr - gRomObjDefPtrs[i].romObjDefs);
}

/* 0x80029098, size 0x20 */
static int texcmp(const anon_name_width_30 *ta, const anon_name_width_30 *tb)
{
    return strcmp(ta->name, tb->name);
}

/* 0x800290b8, size 0x128 */
/* Callers pass a fifth argument (see overrides.json). */
anon_name_width_30 *MBOX_FindTexture_WORK(const char *name, unsigned short *index, signed char firstModel, signed char lastModel, int flag)
{
    int i; /* r26 */
    anon_name_width_30 ti; /* stack */
    anon_name_width_30 *ptr; /* r31 */
    ptr = 0;
    strcpy(ti.name, name);
    if (firstModel < 0) {
        firstModel = 0;
    }
    if (lastModel < 0 || lastModel >= gNumModels) {
        lastModel = gNumModels - 1;
    }
    for (i = firstModel; i <= lastModel; i++) {
        if (ModelActive(i)) {
            ptr = bsearch(&ti, gTexDefPtrs[i].texDefs, gTexDefPtrs[i].count, sizeof(anon_name_width_30),
                          (int (*)(void *, void *))texcmp);
            if (ptr) {
                break;
            }
        }
    }
    if (!ptr) {
        *index = 0;
        return 0;
    }
    *index = (i << 10) | (ptr - gTexDefPtrs[i].texDefs);
    return ptr;
}

