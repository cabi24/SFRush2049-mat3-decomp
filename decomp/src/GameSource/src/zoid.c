/* zoid.c - zoid object instance tree (parent/child/sibling links) and matrices.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\zoid.c; assembly: build/2049LE/asm/GameSource/src/zoid.s */
#include "rush/rush_protos.h"

static void _InsertObject(signed short objnum, signed short parent); /* first parameter unnamed in the original */
static void _RemoveObjectTree(signed short objnum, int removeChildren, int removeNext);

int gNumObjects;
int gMaxObjects;
int gNumMatrices;
int gMaxMatrices;
signed short ObjHead;
static anon_mat3_mat4_30 gMatrices[300];
anon_flags_culling_60 ObjInstanceList[600];

/* 0x80061ec0, size 0x4 */
/* callers pass the bounds and a display-list index (see overrides.json); the body is gone */
void ZOID_BuildCullingList(float *min, float *max, signed short dlindex)
{

}

/* 0x80061ec4, size 0x48 */
void ZOID_Reset(void)
{
    HideControllerWarning();
    gNumObjects = 0;
    gNumBlitInsts = 0;
    gNumMatrices = 0;
    gNumPolyInsts = 0;
    ObjHead = -1;
    ZOID_ResetWindows();
    gGlobalCameraFacingScale = 1.0f;
}

/* 0x80061f0c, size 0x20 */
void ZOID_Init(void)
{
    ZOID_Reset();
}

/* 0x80061f2c, size 0x1c */
anon_mat3_mat4_30 *ZOID_FindMatrix(signed short objnum)
{
    return ObjInstanceList[objnum].mat;
}

/* 0x80061f48, size 0x74 */
anon_mat3_mat4_30 *ZOID_NewMatrix(void)
{
    anon_mat3_mat4_30 *mat; /* r31 */

    {
        int n = ++gNumMatrices; /* a register temporary (not in the DWARF) */

        if (n > gMaxMatrices) {
            gMaxMatrices = n;
        }
        mat = &gMatrices[n];
    }
    mat->uvs[3][0] = 0.0f;
    mat->uvs[3][1] = 0.0f;
    mat->uvs[3][2] = 0.0f;
    CopyUV(unitmatrix, mat->uvs);
    return mat;
}

/* 0x80061fbc, size 0x68 */
void ZOID_UpdateObject(signed short objnum, float *pos, float (*uvs)[3])
{
    anon_mat3_mat4_30 *mat; /* r6 */

    mat = ObjInstanceList[objnum].mat;
    if (pos) {
        mat->uvs[3][0] = pos[0];
        mat->uvs[3][1] = pos[1];
        mat->uvs[3][2] = pos[2];
    }
    if (uvs) {
        CopyUV(uvs, mat->uvs);
    }
}

/* 0x80062024, size 0xa8 */
signed short ZOID_FindObjInstance(char *name)
{
    int i; /* r30 */
    anon_flags_culling_60 *oip; /* r29 */

    for (i = 0, oip = ObjInstanceList; i < gNumObjects; i++, oip++) {
        if (!strcmp(name, gRomObjDefPtrs[oip->index >> 10].romObjDefs[oip->index & 0x3ff].name)) {
            return i;
        }
    }
    return -1;
}

/* 0x800620cc, size 0x48 */
signed short ZOID_ObjParent(signed short objnum)
{
    int i; /* r5 */

    for (i = 0; i < gNumObjects; i++) {
        if (ObjInstanceList[i].child == objnum) {
            return i;
        }
    }
    return -1;
}

/* 0x80062114, size 0x74 */
signed short ZOID_TrueObjParent(signed short objnum)
{
    int i; /* r7 */
    short target = (signed short)objnum;
    int l; /* r0 */

    for (i = 0; i < gNumObjects; i++) {
        l = ObjInstanceList[i].child;
        if ((int)target == l) {
            return i;
        }
        while (l != -1) {
            l = ObjInstanceList[l].next;
            if ((int)target == l) {
                return i;
            }
        }
    }
    return -1;
}

/* 0x80062188, size 0x48 */
signed short ZOID_ObjPrevObj(signed short objnum)
{
    int i; /* r5 */

    for (i = 0; i < gNumObjects; i++) {
        if (ObjInstanceList[i].next == objnum) {
            return i;
        }
    }
    return -1;
}

/* 0x800621d0, size 0x38 */
signed short ZOID_LastSibling(signed short objnum)
{
    signed short curobj; /* r6 */
    signed short nextobj; /* r3 */

    nextobj = objnum;
    do {
        curobj = nextobj;
        nextobj = ObjInstanceList[curobj].next;
    } while (nextobj >= 0 && nextobj != objnum);
    return curobj;
}

/* 0x80062208, size 0xc */
static void _InsertObject(signed short objnum, signed short parent)
{
    /* the rest of the function is gone; only the test survives */
    if (parent == -1) {
        return;
    }
}

/* 0x80062214, size 0xbc */
void ZOID_InsertObject(signed short objnum, signed short parent)
{
    signed short child; /* r3 */
    signed short sibling; /* r0 */

    _InsertObject(objnum, parent);
    if (parent < 0) {
        if (ObjHead < 0) {
            ObjHead = objnum;
        } else {
            sibling = ZOID_LastSibling(ObjHead);
            ObjInstanceList[sibling].next = objnum;
        }
    } else {
        child = ObjInstanceList[parent].child;
        if (child == -1) {
            ObjInstanceList[parent].child = objnum;
        } else {
            sibling = ZOID_LastSibling(child);
            ObjInstanceList[sibling].next = objnum;
        }
    }
}

/* 0x800622d0, size 0x120 */
signed short ZOID_AddObject(int objidx, anon_mat3_mat4_30 *mat, signed short parent, unsigned int flags)
{
    unsigned int objnum; /* r0 */
    int i; /* r31 */

    for (i = 0; i < gNumObjects; i++) {
        if (ObjInstanceList[i].index == 0xffff) {
            break;
        }
    }
    if (i == gNumObjects) {
        gNumObjects++;
    }
    if (gMaxObjects < gNumObjects) {
        gMaxObjects = gNumObjects;
    }
    ObjInstanceList[i].flags = flags;
    ObjInstanceList[i].culling = 0;
    ObjInstanceList[i].mat = mat;
    ObjInstanceList[i].scale = 1.0f;
    ObjInstanceList[i].w = 1.0f;
    ObjInstanceList[i].look[0] = unitmatrix[2][0];
    ObjInstanceList[i].look[1] = unitmatrix[2][1];
    ObjInstanceList[i].look[2] = unitmatrix[2][2];
    ObjInstanceList[i].ouls = 0.0f;
    ObjInstanceList[i].oult = 0.0f;
    ObjInstanceList[i].index = objidx;
    ObjInstanceList[i].child = -1;
    ObjInstanceList[i].next = -1;
    ObjInstanceList[i].numLights = -1;
    ObjInstanceList[i].lights = 0;
    ObjInstanceList[i].texture[0] = 0;
    ObjInstanceList[i].texture[1] = 0;
    ObjInstanceList[i].texture[2] = 0;
    ObjInstanceList[i].texture[3] = 0;
    ObjInstanceList[i].tile = 0;
    ObjInstanceList[i].tileSize = 0;
    ObjInstanceList[i].palette = 0;
    ObjInstanceList[i].color.rgba = 0;
    ZOID_InsertObject(i, parent);
    return i;
}

/* 0x800623f0, size 0x1c0 */
static void _RemoveObjectTree(signed short objnum, int removeChildren, int removeNext)
{
    signed short idx; /* r0 */

    if (removeNext && ObjInstanceList[objnum].next >= 0) {
        _RemoveObjectTree(ObjInstanceList[objnum].next, 1, 1);
        ObjInstanceList[objnum].next = -1;
    }
    if (removeChildren && ObjInstanceList[objnum].child >= 0) {
        _RemoveObjectTree(ObjInstanceList[objnum].child, 1, 1);
        ObjInstanceList[objnum].child = -1;
    }
    idx = ZOID_ObjParent(objnum);
    if (idx >= 0) {
        ObjInstanceList[idx].child = ObjInstanceList[objnum].next;
    } else if (ObjHead == objnum) {
        ObjHead = ObjInstanceList[objnum].next;
    } else {
        idx = ZOID_ObjPrevObj(objnum);
        if (idx >= 0) {
            ObjInstanceList[idx].next = ObjInstanceList[objnum].next;
        }
    }
    ObjInstanceList[objnum].index = 0xffff;
    ObjInstanceList[objnum].child = -1;
    ObjInstanceList[objnum].next = -1;
    if (objnum == gNumObjects - 1) {
        do {
            gNumObjects--;
        } while (ObjInstanceList[gNumObjects - 1].index == 0xffff && gNumObjects > 0);
    }
}

/* 0x800625b0, size 0x24 */
void ZOID_RemoveObjectTree(signed short objnum, int removeChildren)
{
    _RemoveObjectTree(objnum, removeChildren, 0);
}

/* 0x800625d4, size 0x24 */
void ZOID_RemoveObject(signed short objnum)
{
    ZOID_RemoveObjectTree(objnum, 0);
}

/* 0x800625f8, size 0x1c */
void ZOID_SetObjectDef(signed short objnum, unsigned short def)
{
    ObjInstanceList[objnum].index = def;
}

/* 0x80062614, size 0x10c */
void ZOID_SetObjectParent(signed short objnum, signed short newparent)
{
    signed short parent; /* r0 */
    signed short idx; /* r0 */

    parent = ZOID_ObjParent(objnum);
    if (parent != -1 && parent == newparent) {
        return;
    }
    if (parent >= 0) {
        ObjInstanceList[parent].child = ObjInstanceList[objnum].next;
    } else if (ObjHead == objnum) {
        ObjHead = ObjInstanceList[objnum].next;
    } else {
        idx = ZOID_ObjPrevObj(objnum);
        if (idx >= 0) {
            ObjInstanceList[idx].next = ObjInstanceList[objnum].next;
        }
    }
    ObjInstanceList[objnum].next = -1;
    ZOID_InsertObject(objnum, newparent);
}

/* 0x80062720, size 0x18 */
unsigned int ZOID_ObjectFlags(signed short objnum)
{
    return ObjInstanceList[objnum].flags;
}

/* 0x80062738, size 0x14 */
void ZOID_SetObjectFlags(int objnum, unsigned int flags)
{
    ObjInstanceList[objnum].flags = flags;
}

