/* ngcVu1pack.c - GameCube port of the PS2 "VU1 packet" object renderer: per-object GX draw of
   vertex strips with lighting, alpha-sorted and texture-sorted draw lists, fog, geometry group
   loading (PS2GeoInfo.bin / NGCGeoData.bin). Status: in progress.
   Original: D:\rush_ngc\Rush_NGC\NGC\ngcVu1pack.c; assembly: build/2049LE/asm/NGC/ngcVu1pack.s */
/* Several functions take parameters the DWARF omits (unused); see overrides.json "prototypes". */
#include "rush/rush_protos.h"

/* SDK prototypes not yet in dolphin.h (as in ngcBlit.c / ngcZoid.c) */
void C_MTXPerspective(float m[4][4], float fovY, float aspect, float n, float f);
void PSMTXConcat(float a[3][4], float b[3][4], float ab[3][4]);
void GXSetProjection(float m[4][4], int type);
void GXLoadPosMtxImm(float m[3][4], unsigned long id);
void GXSetZCompLoc(unsigned char before_tex);
void GXSetCullMode(int mode);
void GXSetAlphaCompare(int comp0, unsigned char ref0, int op, int comp1, unsigned char ref1);
void GXLoadTexObj(_GXTexObj *obj, int id);
void GXSetBlendMode(int type, int src_factor, int dst_factor, int op);
void GXSetTevOrder(int stage, int coord, int map, int color);
void GXSetTevSwapMode(int stage, int ras_sel, int tex_sel);
void GXSetTevSwapModeTable(int table, int red, int green, int blue, int alpha);
void GXSetFog(int type, float startz, float endz, float nearz, float farz, _GXColor color);

static int GetAnEmptyGeoGroup(void);
static anon_dmatag_Local_world_90 *AddVu1ObjPacket_WithLight(unsigned int *obj, float (*rotMTX)[4], int unused, float scale);

float obj_trans[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
float obj_rot[4] = { 0 }; /* explicit `= { 0 }`: the target keeps this object in initialised data (.data/.sdata), not .bss */
float g_FogFarZ = 500.0f;
static _GXColor gxambient_color;
static anon_c_rgba_4 ambient_color;
static int JustLoadedGroupIdx;
float currentz;
float Fake_EnvMap_uu;
float Fake_EnvMap_vv;
/* explicitly zero-initialised: emitted at the declaration point, in this order (.sbss) */
int g_NumAlphaObj = 0;
int g_NumAlphaObj1 = 0;
int g_NumAlphaObj2 = 0;
int g_NumTexSortObj = 0;
int g_NumTexSortPack = 0;
int g_NumTexSorted = 0;
_ps2objzsort *g_AlphaRoot = 0;
_ps2objzsort *g_AlphaRoot2 = 0;
int isdrawingtrack = 0;
float g_FogNearZ = 0.0f;
static int s_NumTexSorted = 0;
anon_loaded_group_8 g_PS2GeoWave[64];
anon_numObj__objs_c g_PS2GeoGroup[500];
_ps2objtexsort g_TexSort[10000];
float g_TexSortMatrix[700][4][4];
_ps2objzsort g_AlphaSort2[1000];
_ps2objzsort g_AlphaSort[5000];
float Fake_EnvMap_Normal2[4];
float Fake_EnvMap_Normal1[4];
anon__tex__pPack_30 g_AlphaObj[5000];
float obj_rotMTX[4][4];
int g_GeoGroupUsed[500];

/* 0x80094b9c, size 0x4c */
static int GetAnEmptyGeoGroup(void)
{
    int i; /* r5 */

    for (i = 0; i < 500; i++) {
        if (!g_GeoGroupUsed[i]) {
            g_GeoGroupUsed[i] = 1;
            return i;
        }
    }
}

/* 0x80094be8, size 0x2f8 */
static anon_dmatag_Local_world_90 *AddVu1ObjPacket_WithLight(unsigned int *obj, float (*rotMTX)[4], int unused, float scale)
{
    static anon_x_y_c_2 camPt = { 0.0f, 0.0f, 0.0f }; /* .data */
    static anon_x_y_c_2 at = { 0.0f, 0.0f, 1.0f }; /* .data */
    static anon_x_y_c_2 up = { 0.0f, 1.0f, 0.0f }; /* .data */
    int j; /* r4 */
    float *pmtx; /* r0 */
    anon_nVertex_iTex_10 *strips; /* r0 */
    float p[4][4]; /* stack */
    float v[3][4]; /* stack */
    float w[3][4]; /* stack */

    C_MTXLookAt(v, &camPt, &up, &at);
    pmtx = (float *)rotMTX;
    w[0][0] = pmtx[0];
    w[0][1] = pmtx[4];
    w[0][2] = pmtx[8];
    w[0][3] = pmtx[12];
    w[1][0] = pmtx[1];
    w[1][1] = pmtx[5];
    w[1][2] = pmtx[9];
    w[1][3] = pmtx[13];
    w[2][0] = pmtx[2];
    w[2][1] = pmtx[6];
    w[2][2] = pmtx[10];
    w[2][3] = pmtx[14];
    PSMTXConcat(g_NGCMTXWorldView, w, v);
    v[0][0] = -v[0][0];
    v[0][1] = -v[0][1];
    v[0][2] = -v[0][2];
    v[0][3] = -v[0][3];
    GXLoadPosMtxImm(v, 0);
    C_MTXPerspective(p, 75.0f, 1.3333334f, 0.01f, 8000.0f);
    GXSetProjection(p, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxDesc(11, 1);
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetVtxAttrFmt(0, 11, 1, 5, 0);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    GXSetCullMode(0);
    GXSetZMode(1, 3, 1);
    GXSetZCompLoc(0);
    GXSetAlphaCompare(7, 0, 0, 7, 0);
    GXSetTevOp(0, 4);
    GXSetBlendMode(0, 4, 5, 0);
    strips = (anon_nVertex_iTex_10 *)obj;
    GXBegin(strips->iHasAlpha == 0xFECE0EE6 ? 0x98 : 0x90, 0, strips->nVertex);
    for (j = 0; j < strips->nVertex; j++) {
        GXPosition3f32(strips->vertexs[j].pos[0], strips->vertexs[j].pos[1], strips->vertexs[j].pos[2]);
        if (strips->iHasAlpha == 0xFECE0EE5) {
            if (ambient_color.rgba) {
                GXColor1u32((ambient_color.rgba & strips->vertexs[j].color) | 0x3F3F3FFF);
            } else {
                GXColor1u32(strips->vertexs[j].color);
            }
        } else {
            GXColor1u32(strips->vertexs[j].color == 0xFFFFFFFF ? ambient_color.rgba : ambient_color.rgba | strips->vertexs[j].color);
        }
        GXTexCoord2f32(strips->vertexs[j].uv[0], 1.0f - strips->vertexs[j].uv[1]);
    }
    GXEnd();
}

/* 0x80094ee0, size 0x2b8 */
anon_dmatag_Local_world_90 *AddVu1ObjPacket_WithLight2(unsigned int *obj, float (*rotMTX)[4], int unused, anon_name_width_38 *pTexDef, float scale)
{
    static anon_x_y_c_2 camPt = { 0.0f, 0.0f, 0.0f }; /* .data */
    static anon_x_y_c_2 at = { 0.0f, 0.0f, 1.0f }; /* .data */
    static anon_x_y_c_2 up = { 0.0f, 1.0f, 0.0f }; /* .data */
    int j; /* r8 */
    float *pmtx; /* r0 */
    anon_nVertex_iTex_10 *strips; /* r0 */
    float p[4][4]; /* stack */
    float v[3][4]; /* stack */
    float w[3][4]; /* stack */

    C_MTXLookAt(v, &camPt, &up, &at);
    pmtx = (float *)rotMTX;
    w[0][0] = pmtx[0];
    w[0][1] = pmtx[4];
    w[0][2] = pmtx[8];
    w[0][3] = pmtx[12];
    w[1][0] = pmtx[1];
    w[1][1] = pmtx[5];
    w[1][2] = pmtx[9];
    w[1][3] = pmtx[13];
    w[2][0] = pmtx[2];
    w[2][1] = pmtx[6];
    w[2][2] = pmtx[10];
    w[2][3] = pmtx[14];
    PSMTXConcat(g_NGCMTXWorldView, w, v);
    v[0][0] = -v[0][0];
    v[0][1] = -v[0][1];
    v[0][2] = -v[0][2];
    v[0][3] = -v[0][3];
    GXLoadPosMtxImm(v, 0);
    C_MTXPerspective(p, 75.0f, 1.3333334f, 0.01f, 8000.0f);
    GXSetProjection(p, 0);
    GXSetNumTevStages(1);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxDesc(11, 1);
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetVtxAttrFmt(0, 11, 1, 5, 0);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    GXLoadTexObj((_GXTexObj *)pTexDef->texBase, 0);
    GXSetAlphaCompare(7, 0, 0, 7, 0);
    GXSetCullMode(0);
    GXSetZMode(1, 7, 1);
    GXSetZCompLoc(0);
    GXSetAlphaCompare(7, 0, 0, 7, 0);
    GXSetTevOp(0, 0);
    GXSetBlendMode(1, 4, 5, 0);
    strips = (anon_nVertex_iTex_10 *)obj;
    GXBegin(strips->iHasAlpha == 0xFECE0EE6 ? 0x98 : 0x90, 0, strips->nVertex);
    for (j = 0; j < strips->nVertex; j++) {
        GXPosition3f32(strips->vertexs[j].pos[0], strips->vertexs[j].pos[1], strips->vertexs[j].pos[2]);
        GXColor1u32(ambient_color.rgba);
        GXTexCoord2f32(strips->vertexs[j].uv[0], 1.0f - strips->vertexs[j].uv[1]);
    }
    GXEnd();
}

/* 0x80095198, size 0x4 */
unsigned int *Vu1PackOneMaterialNoTexture(void)
{
}

/* 0x8009519c, size 0x148 */
void PS2SetCurrentMatrix(float (*a)[4])
{
    if (free_camera)
        return;
    if (!a) {
        obj_rotMTX[0][0] = ObjectMTX[0][0];
        obj_rotMTX[0][1] = ObjectMTX[0][1];
        obj_rotMTX[0][2] = ObjectMTX[0][2];
        obj_rotMTX[0][3] = 0.0f;
        obj_rotMTX[1][0] = ObjectMTX[1][0];
        obj_rotMTX[1][1] = ObjectMTX[1][1];
        obj_rotMTX[1][2] = ObjectMTX[1][2];
        obj_rotMTX[1][3] = 0.0f;
        obj_rotMTX[2][0] = ObjectMTX[2][0];
        obj_rotMTX[2][1] = ObjectMTX[2][1];
        obj_rotMTX[2][2] = ObjectMTX[2][2];
        obj_rotMTX[2][3] = 0.0f;
        obj_rotMTX[3][0] = ObjectMTX[3][0];
        obj_rotMTX[3][1] = ObjectMTX[3][1];
        obj_rotMTX[3][2] = ObjectMTX[3][2];
        obj_rotMTX[3][3] = 1.0f;
        obj_trans[0] = ObjectMTX[3][0];
        obj_trans[1] = ObjectMTX[3][1];
        obj_trans[2] = ObjectMTX[3][2];
        obj_trans[3] = 0.0f;
    } else {
        obj_rotMTX[0][0] = a[0][0];
        obj_rotMTX[0][1] = a[0][1];
        obj_rotMTX[0][2] = a[0][2];
        obj_rotMTX[0][3] = 0.0f;
        obj_rotMTX[1][0] = a[1][0];
        obj_rotMTX[1][1] = a[1][1];
        obj_rotMTX[1][2] = a[1][2];
        obj_rotMTX[1][3] = 0.0f;
        obj_rotMTX[2][0] = a[2][0];
        obj_rotMTX[2][1] = a[2][1];
        obj_rotMTX[2][2] = a[2][2];
        obj_rotMTX[2][3] = 0.0f;
        obj_rotMTX[3][0] = a[3][0];
        obj_rotMTX[3][1] = a[3][1];
        obj_rotMTX[3][2] = a[3][2];
        obj_rotMTX[3][3] = 1.0f;
        obj_trans[0] = a[3][0];
        obj_trans[1] = a[3][1];
        obj_trans[2] = a[3][2];
        obj_trans[3] = 0.0f;
    }
}

/* 0x800952e4, size 0x34 */
void MySetObjectPosition(float z)
{
    PS2SetCurrentMatrix(0);
    currentz = z;
}

/* 0x80095318, size 0x34 */
void MySetAmbientColor(anon_c_rgba_4 c)
{
    ambient_color = c;
    gxambient_color.r = ambient_color.c[0];
    gxambient_color.g = ambient_color.c[1];
    gxambient_color.b = ambient_color.c[2];
    gxambient_color.a = ambient_color.c[3];
}

/* 0x8009534c, size 0x8 */
anon_c_rgba_4 MyGetAmbientColor(void)
{
    return ambient_color;
}

/* 0x80095354, size 0x1ec */
void AddPackToAlphaList(float **mtx, anon_name_width_38 *tex, unsigned int *pack, int cull, int AddColor)
{
    _ps2objzsort *next; /* r4 */
    _ps2objzsort *last; /* r3 */
    int added; /* r0 */

    if (g_NumAlphaObj >= 5000)
        return;
    if (cull < 0) {
        g_AlphaObj[g_NumAlphaObj]._pPack = pack;
    } else if (cull == 0x58) {
        g_AlphaObj[g_NumAlphaObj]._pPack = pack;
        g_AlphaObj[g_NumAlphaObj].fakeenvmap[0][0] = Fake_EnvMap_Normal1[0];
        g_AlphaObj[g_NumAlphaObj].fakeenvmap[0][1] = Fake_EnvMap_Normal1[1];
        g_AlphaObj[g_NumAlphaObj].fakeenvmap[0][2] = Fake_EnvMap_Normal1[2];
        g_AlphaObj[g_NumAlphaObj].fakeenvmap[1][0] = Fake_EnvMap_Normal2[0];
        g_AlphaObj[g_NumAlphaObj].fakeenvmap[1][1] = Fake_EnvMap_Normal2[1];
        g_AlphaObj[g_NumAlphaObj].fakeenvmap[1][2] = Fake_EnvMap_Normal2[2];
        g_AlphaObj[g_NumAlphaObj].fakeenv_u = Fake_EnvMap_uu;
        g_AlphaObj[g_NumAlphaObj].fakeenv_v = Fake_EnvMap_vv;
    } else {
        g_AlphaObj[g_NumAlphaObj]._pPack = pack;
    }
    g_AlphaObj[g_NumAlphaObj]._matrix = mtx;
    g_AlphaObj[g_NumAlphaObj]._tex = tex;
    g_AlphaSort[g_NumAlphaObj1]._obj = &g_AlphaObj[g_NumAlphaObj];
    g_AlphaObj[g_NumAlphaObj]._color = AddColor;
    g_AlphaSort[g_NumAlphaObj1]._next = 0;
    if (tex->ps2flags & 0x40)
        g_AlphaSort[g_NumAlphaObj1]._objz = 100000.0f + currentz;
    else
        g_AlphaSort[g_NumAlphaObj1]._objz = currentz;
    if (tex->xflags & 1)
        g_AlphaSort[g_NumAlphaObj1]._objz = currentz - 99000.0f;
    if (tex->xflags & 0x20)
        g_AlphaSort[g_NumAlphaObj1]._objz = 500.0f + currentz;
    {
        _ps2objzsort *cur = &g_AlphaSort[g_NumAlphaObj1];
        if (!g_AlphaRoot) {
            g_AlphaRoot = cur;
        } else if (cur->_objz > g_AlphaRoot->_objz) {
            cur->_next = g_AlphaRoot;
            g_AlphaRoot = cur;
        } else {
            last = g_AlphaRoot;
            next = last->_next;
            added = 0;
            while (next) {
                if (cur->_objz > next->_objz) {
                    last->_next = cur;
                    added = 1;
                    cur->_next = next;
                    break;
                }
                last = next;
                next = next->_next;
            }
            if (!added)
                last->_next = cur;
        }
    }
    g_NumAlphaObj++;
    g_NumAlphaObj1++;
}

/* 0x80095540, size 0x1f4 */
void AddPackToAlphaList2(float **mtx, anon_name_width_38 *tex, unsigned int *pack, int cull, int AddColor)
{
    _ps2objzsort *next; /* r4 */
    _ps2objzsort *last; /* r3 */
    int added; /* r0 */

    if (g_NumAlphaObj >= 5000)
        return;
    if (cull < 0) {
        g_AlphaObj[g_NumAlphaObj]._pPack = pack;
    } else if (cull == 0x58) {
        g_AlphaObj[g_NumAlphaObj]._pPack = pack;
        g_AlphaObj[g_NumAlphaObj].fakeenvmap[0][0] = Fake_EnvMap_Normal1[0];
        g_AlphaObj[g_NumAlphaObj].fakeenvmap[0][1] = Fake_EnvMap_Normal1[1];
        g_AlphaObj[g_NumAlphaObj].fakeenvmap[0][2] = Fake_EnvMap_Normal1[2];
        g_AlphaObj[g_NumAlphaObj].fakeenvmap[1][0] = Fake_EnvMap_Normal2[0];
        g_AlphaObj[g_NumAlphaObj].fakeenvmap[1][1] = Fake_EnvMap_Normal2[1];
        g_AlphaObj[g_NumAlphaObj].fakeenvmap[1][2] = Fake_EnvMap_Normal2[2];
        g_AlphaObj[g_NumAlphaObj].fakeenv_u = Fake_EnvMap_uu;
        g_AlphaObj[g_NumAlphaObj].fakeenv_v = Fake_EnvMap_vv;
    } else {
        g_AlphaObj[g_NumAlphaObj]._pPack = pack;
    }
    g_AlphaObj[g_NumAlphaObj]._matrix = mtx;
    g_AlphaObj[g_NumAlphaObj]._tex = tex;
    g_AlphaSort2[g_NumAlphaObj2]._obj = &g_AlphaObj[g_NumAlphaObj];
    g_AlphaObj[g_NumAlphaObj]._color = AddColor;
    g_AlphaSort2[g_NumAlphaObj2]._next = 0;
    if (tex->ps2flags & 0x40)
        g_AlphaSort2[g_NumAlphaObj2]._objz = 100000.0f + currentz;
    else
        g_AlphaSort2[g_NumAlphaObj2]._objz = currentz;
    if (tex->xflags & 1)
        g_AlphaSort2[g_NumAlphaObj2]._objz = currentz - 99000.0f;
    if (tex->xflags & 0x20)
        g_AlphaSort2[g_NumAlphaObj2]._objz = 500.0f + currentz;
    {
        _ps2objzsort *cur = &g_AlphaSort2[g_NumAlphaObj2];
        if (!g_AlphaRoot2) {
            g_AlphaRoot2 = cur;
        } else if (cur->_objz > g_AlphaRoot2->_objz) {
            cur->_next = g_AlphaRoot2;
            g_AlphaRoot2 = cur;
        } else {
            last = g_AlphaRoot2;
            next = last->_next;
            added = 0;
            while (next) {
                if (cur->_objz > next->_objz) {
                    last->_next = cur;
                    added = 1;
                    cur->_next = next;
                    break;
                }
                last = next;
                next = next->_next;
            }
            if (!added)
                last->_next = cur;
        }
    }
    g_NumAlphaObj++;
    g_NumAlphaObj2++;
    /* the condition really is written twice: a single test folds the branch away, leaving
     * `cmpwi; blr`; the duplicated (CSE'd) test keeps the original `bltlr; blr` */
    if (g_NumAlphaObj2 >= 1000 && g_NumAlphaObj2 >= 1000)
        return;
}

/* 0x80095734, size 0x1c */
void ClearAlphaList(void)
{
    g_NumAlphaObj = 0;
    g_NumAlphaObj1 = 0;
    g_AlphaRoot = 0;
    g_NumAlphaObj2 = 0;
    g_AlphaRoot2 = 0;
}

/* 0x80095750, size 0x130 */
void PreDrawAlphaList(void)
{
    float p[4][4]; /* stack */

    if (g_HalfEye == 0) {
        C_MTXPerspective(p, 75.0f, 1.3333334f, 0.01f, 8000.0f);
    } else {
        C_MTXPerspective(p, 75.0f, 2.6666667f, 0.01f, 8000.0f);
    }
    GXSetProjection(p, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxDesc(11, 1);
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetVtxAttrFmt(0, 11, 1, 5, 0);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    GXSetCullMode(0);
    GXSetZMode(1, 3, 1);
    GXSetZCompLoc(0);
    GXSetAlphaCompare(7, 0, 0, 7, 0);
    GXSetTevOp(0, 0);
    GXSetBlendMode(1, 4, 5, 0);
}

/* 0x80095880, size 0x35c */
void DrawAlphaList(void)
{
    static anon_x_y_c_2 camPt = { 0.0f, 0.0f, 0.0f }; /* .data */
    static anon_x_y_c_2 at = { 0.0f, 0.0f, 1.0f }; /* .data */
    static anon_x_y_c_2 up = { 0.0f, 1.0f, 0.0f }; /* .data */
    int j; /* r6 */
    float *pmtx; /* r6 */
    anon_name_width_38 *pTexDef; /* r27 */
    anon_nVertex_iTex_10 *strips; /* r26 */
    float v[3][4]; /* stack */
    float w[3][4]; /* stack */
    int iAddColor; /* r0 */
    anon__tex__pPack_30 *obj; /* r26 */
    _ps2objzsort *next; /* r25 */

    next = g_AlphaRoot;
    while (next) {
        obj = next->_obj;
        if (obj->_color & 0xff) {
            iAddColor = obj->_color | 0xFFFFFF00;
        } else {
            iAddColor = -1;
        }
        C_MTXLookAt(v, &camPt, &up, &at);
        pmtx = (float *)obj->_matrix;
        w[0][0] = pmtx[0];
        w[0][1] = pmtx[4];
        w[0][2] = pmtx[8];
        w[0][3] = pmtx[12];
        w[1][0] = pmtx[1];
        w[1][1] = pmtx[5];
        w[1][2] = pmtx[9];
        w[1][3] = pmtx[13];
        w[2][0] = pmtx[2];
        w[2][1] = pmtx[6];
        w[2][2] = pmtx[10];
        w[2][3] = pmtx[14];
        PSMTXConcat(g_NGCMTXWorldView, w, v);
        v[0][0] = -v[0][0];
        v[0][1] = -v[0][1];
        v[0][2] = -v[0][2];
        v[0][3] = -v[0][3];
        GXLoadPosMtxImm(v, 0);
        pTexDef = obj->_tex;
        GXLoadTexObj((_GXTexObj *)pTexDef->texBase, 0);
        if (pTexDef->ps2flags & 4) {
            GXSetAlphaCompare(4, 0x78, 0, 3, 0xff);
        } else {
            GXSetAlphaCompare(7, 0, 0, 7, 0);
        }
        if (pTexDef->ps2flags & 8) {
            GXSetZMode(1, 3, 0);
        } else {
            GXSetZMode(1, 3, 1);
        }
        if (pTexDef->ps2flags & 0x10) {
            if (gMirrorWaves) {
                GXSetCullMode(2);
            } else {
                GXSetCullMode(1);
            }
        } else {
            GXSetCullMode(0);
        }
        if (pTexDef->psm != 0xffff) {
            GXSetNumTevStages(2);
            GXLoadTexObj((_GXTexObj *)pTexDef->texhead, 1);
            GXSetTevOrder(1, 0, 1, 0xff);
            GXSetTevOp(1, 0);
            GXSetTevSwapMode(1, 0, 1);
            GXSetTevSwapModeTable(1, 0, 0, 0, 1);
        } else {
            GXSetNumTevStages(1);
        }
        strips = (anon_nVertex_iTex_10 *)obj->_pPack;
        GXBegin(strips->iHasAlpha == 0xFECE0EE6 ? 0x98 : 0x90, 0, strips->nVertex);
        for (j = 0; j < strips->nVertex; j++) {
            GXPosition3f32(strips->vertexs[j].pos[0], strips->vertexs[j].pos[1], strips->vertexs[j].pos[2]);
            if (iAddColor == 0xFFFFFFFF) {
                GXColor1u32(iAddColor & strips->vertexs[j].color);
            } else {
                GXColor1u32(iAddColor & (strips->vertexs[j].color | 0xff));
            }
            GXTexCoord2f32(strips->vertexs[j].uv[0] + (pTexDef ? pTexDef->uls : 0.0f),
                           1.0f - strips->vertexs[j].uv[1] - (pTexDef ? pTexDef->u.ult : 0.0f));
        }
        GXEnd();
        next = next->_next;
    }
}

/* 0x80095bdc, size 0x130 */
void PreDrawAlphaList2(void)
{
    float p[4][4]; /* stack */

    if (g_HalfEye == 0) {
        C_MTXPerspective(p, 75.0f, 1.3333334f, 0.01f, 8000.0f);
    } else {
        C_MTXPerspective(p, 75.0f, 2.6666667f, 0.01f, 8000.0f);
    }
    GXSetProjection(p, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxDesc(11, 1);
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetVtxAttrFmt(0, 11, 1, 5, 0);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    GXSetCullMode(0);
    GXSetZMode(1, 3, 1);
    GXSetZCompLoc(0);
    GXSetAlphaCompare(7, 0, 0, 7, 0);
    GXSetTevOp(0, 0);
    GXSetBlendMode(1, 4, 5, 0);
}

/* 0x80095d0c, size 0x35c */
void DrawAlphaList2(void)
{
    static anon_x_y_c_2 camPt = { 0.0f, 0.0f, 0.0f }; /* .data */
    static anon_x_y_c_2 at = { 0.0f, 0.0f, 1.0f }; /* .data */
    static anon_x_y_c_2 up = { 0.0f, 1.0f, 0.0f }; /* .data */
    int j; /* r6 */
    float *pmtx; /* r6 */
    anon_name_width_38 *pTexDef; /* r27 */
    anon_nVertex_iTex_10 *strips; /* r26 */
    float v[3][4]; /* stack */
    float w[3][4]; /* stack */
    int iAddColor; /* r0 */
    anon__tex__pPack_30 *obj; /* r26 */
    _ps2objzsort *next; /* r25 */

    next = g_AlphaRoot2;
    while (next) {
        obj = next->_obj;
        if (obj->_color & 0xff) {
            iAddColor = obj->_color | 0xFFFFFF00;
        } else {
            iAddColor = -1;
        }
        C_MTXLookAt(v, &camPt, &up, &at);
        pmtx = (float *)obj->_matrix;
        w[0][0] = pmtx[0];
        w[0][1] = pmtx[4];
        w[0][2] = pmtx[8];
        w[0][3] = pmtx[12];
        w[1][0] = pmtx[1];
        w[1][1] = pmtx[5];
        w[1][2] = pmtx[9];
        w[1][3] = pmtx[13];
        w[2][0] = pmtx[2];
        w[2][1] = pmtx[6];
        w[2][2] = pmtx[10];
        w[2][3] = pmtx[14];
        PSMTXConcat(g_NGCMTXWorldView, w, v);
        v[0][0] = -v[0][0];
        v[0][1] = -v[0][1];
        v[0][2] = -v[0][2];
        v[0][3] = -v[0][3];
        GXLoadPosMtxImm(v, 0);
        pTexDef = obj->_tex;
        GXLoadTexObj((_GXTexObj *)pTexDef->texBase, 0);
        if (pTexDef->ps2flags & 4) {
            GXSetAlphaCompare(4, 0x78, 0, 3, 0xff);
        } else {
            GXSetAlphaCompare(7, 0, 0, 7, 0);
        }
        if (pTexDef->ps2flags & 8) {
            GXSetZMode(1, 3, 0);
        } else {
            GXSetZMode(1, 3, 1);
        }
        if (pTexDef->ps2flags & 0x10) {
            if (gMirrorWaves) {
                GXSetCullMode(2);
            } else {
                GXSetCullMode(1);
            }
        } else {
            GXSetCullMode(0);
        }
        if (pTexDef->psm != 0xffff) {
            GXSetNumTevStages(2);
            GXLoadTexObj((_GXTexObj *)pTexDef->texhead, 1);
            GXSetTevOrder(1, 0, 1, 0xff);
            GXSetTevOp(1, 0);
            GXSetTevSwapMode(1, 0, 1);
            GXSetTevSwapModeTable(1, 0, 0, 0, 1);
        } else {
            GXSetNumTevStages(1);
        }
        strips = (anon_nVertex_iTex_10 *)obj->_pPack;
        GXBegin(strips->iHasAlpha == 0xFECE0EE6 ? 0x98 : 0x90, 0, strips->nVertex);
        for (j = 0; j < strips->nVertex; j++) {
            GXPosition3f32(strips->vertexs[j].pos[0], strips->vertexs[j].pos[1], strips->vertexs[j].pos[2]);
            if (iAddColor == 0xFFFFFFFF) {
                GXColor1u32(iAddColor & strips->vertexs[j].color);
            } else {
                GXColor1u32(iAddColor & (strips->vertexs[j].color | 0xff));
            }
            GXTexCoord2f32(strips->vertexs[j].uv[0] + (pTexDef ? pTexDef->uls : 0.0f),
                           1.0f - strips->vertexs[j].uv[1] - (pTexDef ? pTexDef->u.ult : 0.0f));
        }
        GXEnd();
        next = next->_next;
    }
}

/* 0x80096068, size 0x64 */
float **SaveOBJMatrix(float (*mtx)[4], int cull)
{
    if (g_NumTexSortObj < 700) {
        memcpy(g_TexSortMatrix[g_NumTexSortObj], mtx, 0x40);
        g_NumTexSortObj++;
        return (float **)(&g_TexSortMatrix[g_NumTexSortObj] - 1); /* `&arr[n] - 1` after the increment gives slwi; add; subi 0x40 as in the target (`arr[n - 1]` does not) */
    }
}

/* 0x800960cc, size 0x44 */
void AddPackToTexSortList(float **mtx, anon_name_width_38 *tex, unsigned int *pack, int cull, int AddColor)
{
    g_TexSort[s_NumTexSorted]._tex = tex;
    g_TexSort[s_NumTexSorted]._pPack = pack;
    g_TexSort[s_NumTexSorted]._matrix = mtx;
    g_TexSort[s_NumTexSorted]._next = 0;
    g_TexSort[s_NumTexSorted]._color = AddColor;
    g_NumTexSortPack++; /* written before s_NumTexSorted++ although the target stores it second: only this source order matches */
    s_NumTexSorted++;
}

/* 0x80096110, size 0xc */
void FlushSortedTex(void)
{
    s_NumTexSorted = 0;
}

/* 0x8009611c, size 0x130 */
void PreDrawSortedTex(void)
{
    float p[4][4]; /* stack */

    if (g_HalfEye == 0) {
        C_MTXPerspective(p, 75.0f, 1.3333334f, 0.01f, 8000.0f);
    } else {
        C_MTXPerspective(p, 75.0f, 2.6666667f, 0.01f, 8000.0f);
    }
    GXSetProjection(p, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxDesc(11, 1);
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetVtxAttrFmt(0, 11, 1, 5, 0);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    GXSetCullMode(0);
    GXSetZMode(1, 3, 1);
    GXSetZCompLoc(0);
    GXSetAlphaCompare(7, 0, 0, 7, 0);
    GXSetTevOp(0, 0);
    GXSetBlendMode(1, 4, 5, 0);
}

/* 0x8009624c, size 0x438 */
void DrawSortedTex(void)
{
    static anon_x_y_c_2 camPt = { 0.0f, 0.0f, 0.0f }; /* .data */
    static anon_x_y_c_2 at = { 0.0f, 0.0f, 1.0f }; /* .data */
    static anon_x_y_c_2 up = { 0.0f, 1.0f, 0.0f }; /* .data */
    static int i; /* .sbss (deferred) */
    static int j; /* .sbss (deferred) */
    static float * pmtx; /* .sbss (deferred) */
    static anon_name_width_38 *pTexDef; /* .sbss (deferred) */
    static anon_nVertex_iTex_10 *strips; /* .sbss (deferred) */
    static float v[3][4]; /* .bss (deferred) */
    static float w[3][4]; /* .bss (deferred) */
    int iAddColor; /* r0 */
    int bFullColor; /* r22 */

    for (i = 0; i < g_NumTexSortPack; i++) {
        bFullColor = 0;
        if (g_TexSort[i]._color & 0xff) {
            iAddColor = g_TexSort[i]._color | 0xFFFFFF00;
        } else {
            iAddColor = -1;
        }
        C_MTXLookAt(v, &camPt, &up, &at);
        pmtx = (float *)g_TexSort[i]._matrix;
        w[0][0] = pmtx[0];
        w[0][1] = pmtx[4];
        w[0][2] = pmtx[8];
        w[0][3] = pmtx[12];
        w[1][0] = pmtx[1];
        w[1][1] = pmtx[5];
        w[1][2] = pmtx[9];
        w[1][3] = pmtx[13];
        w[2][0] = pmtx[2];
        w[2][1] = pmtx[6];
        w[2][2] = pmtx[10];
        w[2][3] = pmtx[14];
        PSMTXConcat(g_NGCMTXWorldView, w, v);
        v[0][0] = -v[0][0];
        v[0][1] = -v[0][1];
        v[0][2] = -v[0][2];
        v[0][3] = -v[0][3];
        GXLoadPosMtxImm(v, 0);
        pTexDef = g_TexSort[i]._tex;
        GXLoadTexObj((_GXTexObj *)pTexDef->texBase, 0);
        if (strstr(pTexDef->name, "AROWGRN")) {
            bFullColor = 1;
        }
        if (iAddColor == 0xFFFFFFFF) {
            GXSetBlendMode(0, 4, 5, 0);
        } else {
            GXSetBlendMode(1, 4, 5, 0);
        }
        if (pTexDef->ps2flags & 4) {
            GXSetAlphaCompare(4, 0x78, 0, 3, 0xff);
        } else {
            GXSetAlphaCompare(7, 0, 0, 7, 0);
        }
        if (pTexDef->ps2flags & 0x10) {
            if (gMirrorWaves) {
                GXSetCullMode(2);
            } else {
                GXSetCullMode(1);
            }
        } else {
            GXSetCullMode(0);
        }
        if (pTexDef->psm != 0xffff) {
            GXSetNumTevStages(2);
            GXLoadTexObj((_GXTexObj *)pTexDef->texhead, 1);
            GXSetTevOrder(1, 0, 1, 0xff);
            GXSetTevOp(1, 0);
            GXSetTevSwapMode(1, 0, 1);
            GXSetTevSwapModeTable(1, 0, 0, 0, 1);
        } else {
            GXSetNumTevStages(1);
        }
        strips = (anon_nVertex_iTex_10 *)g_TexSort[i]._pPack;
        GXBegin(strips->iHasAlpha == 0xFECE0EE6 ? 0x98 : 0x90, 0, strips->nVertex);
        for (j = 0; j < strips->nVertex; j++) {
            GXPosition3f32(strips->vertexs[j].pos[0], strips->vertexs[j].pos[1], strips->vertexs[j].pos[2]);
            if (bFullColor) {
                GXColor1u32(0xFFFFFFFF);
            } else {
                GXColor1u32(iAddColor == 0xFFFFFFFF ? iAddColor & strips->vertexs[j].color : iAddColor & (strips->vertexs[j].color | 0xff));
            }
            GXTexCoord2f32(strips->vertexs[j].uv[0] + (pTexDef ? pTexDef->uls : 0.0f),
                           1.0f - strips->vertexs[j].uv[1] - (pTexDef ? pTexDef->u.ult : 0.0f));
        }
        GXEnd();
    }
    g_NumTexSortPack = 0;
    g_NumTexSortObj = 0;
    g_NumTexSorted = 0;
    s_NumTexSorted = 0;
}

/* 0x80096684, size 0xc0 */
void NGCSetFog(void * pInfo)
{
    anon_vp_s_48 *pWinInfo; /* r0 */
    _GXColor color; /* stack */

    pWinInfo = pInfo;
    color.r = pWinInfo->fogcolor[0];
    color.g = pWinInfo->fogcolor[1];
    color.b = pWinInfo->fogcolor[2];
    color.a = pWinInfo->fogcolor[3];
    if (pWinInfo->fogmax) {
        GXSetFog(5, 0.0f, 3400.0f - (float)pWinInfo->fogmax - 800.0f, 0.01f, 8000.0f, color);
    } else {
        GXSetFog(5, 0.0f, 0.0f, 0.01f, 8000.0f, color);
    }
}

/* 0x80096744, size 0x460 */
void PS2DrawObject(anon_numPack_name_2c *obj, anon_name_width_30 *tex, int cull)
{
    int i; /* r24 */
    anon_name_width_38 *_tex; /* r25 */
    int added = 0; /* r31 */
    float **matrix; /* r30 */

    if (!obj->numPack)
        return;
    matrix = SaveOBJMatrix(ObjectMTX, cull);
    for (i = 0; i < (obj->iHaveFWBW ? obj->nStrips / 3 : obj->nStrips); i++) {
        if (tex) {
            _tex = (anon_name_width_38 *)tex->strip;
            if (CheckAlpha(_tex)) {
                if (isdrawingtrack)
                    AddPackToAlphaList2(matrix, _tex, (unsigned int *)&obj->strips[i], cull, obj->iAddColor);
                else
                    AddPackToAlphaList(matrix, _tex, (unsigned int *)&obj->strips[i], cull, obj->iAddColor);
            } else if (!strcmp(_tex->name, "BATTLECOIN")) {
                AddVu1ObjPacket_WithLight2((unsigned int *)&obj->strips[i], (float (*)[4])matrix, 0, _tex, 1.0f);
            } else {
                AddPackToTexSortList(matrix, _tex, (unsigned int *)&obj->strips[i], cull, obj->iAddColor);
            }
            statsCount(3);
        } else if (obj->strips[i].iTex >= 0 && CheckAlpha((anon_name_width_38 *)obj->tex[obj->strips[i].iTex])) {
            if (isdrawingtrack)
                AddPackToAlphaList2(matrix, (anon_name_width_38 *)obj->tex[obj->strips[i].iTex], (unsigned int *)&obj->strips[i], cull, obj->iAddColor);
            else
                AddPackToAlphaList(matrix, (anon_name_width_38 *)obj->tex[obj->strips[i].iTex], (unsigned int *)&obj->strips[i], cull, obj->iAddColor);
            added = 1;
        } else if (obj->strips[i].iTex < 0) {
            AddVu1ObjPacket_WithLight((unsigned int *)&obj->strips[i], (float (*)[4])matrix, 0, 1.0f);
        } else {
            AddPackToTexSortList(matrix, (anon_name_width_38 *)obj->tex[obj->strips[i].iTex], (unsigned int *)&obj->strips[i], cull, obj->iAddColor);
            statsCount(3);
        }
    }
    if (obj->iHaveFWBW) {
        if (!gBackwards) {
            for (i = obj->nStrips / 3; i < obj->nStrips * 2 / 3; i++) {
                if (obj->strips[i].iTex >= 0 && CheckAlpha((anon_name_width_38 *)obj->tex[obj->strips[i].iTex])) {
                    AddPackToAlphaList2(matrix, (anon_name_width_38 *)obj->tex[obj->strips[i].iTex], (unsigned int *)&obj->strips[i], cull, obj->iAddColor);
                    added = 1;
                } else {
                    if (tex)
                        _tex = (anon_name_width_38 *)tex->strip;
                    else if (!(_tex = (anon_name_width_38 *)obj->tex[obj->strips[i].iTex]))
                        continue;
                    AddPackToTexSortList(matrix, _tex, (unsigned int *)&obj->strips[i], cull, obj->iAddColor);
                    statsCount(3);
                }
            }
        } else {
            for (i = obj->nStrips * 2 / 3; i < obj->nStrips * 3 / 3; i++) {
                if (obj->strips[i].iTex >= 0 && CheckAlpha((anon_name_width_38 *)obj->tex[obj->strips[i].iTex])) {
                    AddPackToAlphaList2(matrix, (anon_name_width_38 *)obj->tex[obj->strips[i].iTex], (unsigned int *)&obj->strips[i], cull, obj->iAddColor);
                    added = 1;
                } else {
                    if (tex)
                        _tex = (anon_name_width_38 *)tex->strip;
                    else if (!(_tex = (anon_name_width_38 *)obj->tex[obj->strips[i].iTex]))
                        continue;
                    AddPackToTexSortList(matrix, _tex, (unsigned int *)&obj->strips[i], cull, obj->iAddColor);
                    statsCount(3);
                }
            }
        }
    }
    if (added)
        statsCount(4);
    else
        statsCount(2);
}

/* 0x80096ba4, size 0x38 */
void MyDrawObject(unsigned int *dlist, anon_name_width_30 *tex, int cull, int AddColor)
{
    anon_numPack_name_2c *obj; /* r0 */

    obj = (anon_numPack_name_2c *)dlist;
    if (!free_camera && obj) {
        obj->iAddColor = AddColor;
        PS2DrawObject(obj, tex, cull);
    }
}

/* 0x80096bdc, size 0xe4 */
void ReverseArrow(anon_numPack_name_2c *obj)
{
    int i; /* r29 */
    int j; /* r5 */

    if (!obj->iHaveFWBW || !gBackwards)
        return;
    for (i = 0; i < obj->nStrips; i++) {
        if (obj->tex[obj->strips[i].iTex] && strstr(obj->tex[obj->strips[i].iTex]->name, "AL_BAR")) {
            for (j = 0; j < obj->strips[i].nVertex; j++) {
                obj->strips[i].vertexs[j].uv[1] = 1.0f - obj->strips[i].vertexs[j].uv[1];
            }
        }
    }
}

/* 0x80096cc0, size 0x194 */
anon_numPack_name_2c *RegisterObjName(char *name, unsigned int *displaylist)
{
    int i; /* r3 */
    int j; /* r27 */
    anon_numObj__objs_c *group; /* r26 */
    char nn[16]; /* stack */

    group = &g_PS2GeoGroup[JustLoadedGroupIdx];
    for (j = 0; j < group->numObj; j++) {
        if (!strcmp(name, group->_objs[j].name) && !group->_objs[j].displaylist) {
            group->_objs[j].displaylist = displaylist;
            if (!strcmp(name, "TRACK2L25619") || !strcmp(name, "TRACK2L25655") || !strcmp(name, "TRACK2L25633") ||
                !strcmp(name, "TRACK2L25645"))
                ReverseArrow(&group->_objs[j]);
            return &group->_objs[j];
        }
    }
    strcpy(nn, name);
    for (i = strlen(nn); i >= 0; i--) {
        if (nn[i] == '1' && (nn[i + 1] == '_' || nn[i + 1] == 0)) {
            nn[i] = '2';
            return RegisterObjName(nn, displaylist);
        }
    }
    printf("@@@@@@@@@@@@@@@@@object %s cannot be found!\n", name);
    return 0;
}

/* 0x80096e54, size 0x98 */
void EnlargeALittle(unsigned int *displaylist)
{
    anon_numPack_name_2c *obj; /* r0 */
    int i; /* r8 */
    int j; /* r9 */

    obj = (anon_numPack_name_2c *)displaylist;
    for (i = 0; i < obj->nStrips; i++) {
        for (j = 0; j < obj->strips[i].nVertex; j++) {
            obj->strips[i].vertexs[j].pos[0] *= 1.01f;
            obj->strips[i].vertexs[j].pos[1] *= 1.01f;
            obj->strips[i].vertexs[j].pos[2] *= 1.01f;
        }
    }
}

/* 0x80096eec, size 0x1d8 */
void ReCalculateNormal(unsigned int *displaylist)
{
    anon_numPack_name_2c *obj; /* r0 */
    int i; /* r29 */
    int j; /* r28 */
    float nor[3]; /* stack */

    obj = (anon_numPack_name_2c *)displaylist;
    for (i = 0; i < obj->nStrips; i++) {
        for (j = 0; j < obj->strips[i].nVertex; j++) {
            float zero = 0.0f; /* a local zero: `x - 0.0f` is folded away, the target keeps the fsubs with a hoisted 0.0f */
            nor[0] = obj->strips[i].vertexs[j].pos[0] - zero;
            nor[1] = obj->strips[i].vertexs[j].pos[1] - -1.0f;
            nor[2] = obj->strips[i].vertexs[j].pos[2] - zero;
            NormalVector(nor);
            nor[0] = nor[0] + obj->strips[i].vertexs[j].nom[0];
            nor[1] = nor[1] + obj->strips[i].vertexs[j].nom[1];
            nor[2] = nor[2] + obj->strips[i].vertexs[j].nom[2];
            NormalVector(nor);
            obj->strips[i].vertexs[j].nom[0] = nor[0];
            obj->strips[i].vertexs[j].nom[1] = nor[1];
            obj->strips[i].vertexs[j].nom[2] = nor[2];
            obj->strips[i].vertexs[j].pos[0] *= 1.01f;
            obj->strips[i].vertexs[j].pos[1] *= 1.01f;
            obj->strips[i].vertexs[j].pos[2] *= 1.01f;
        }
    }
}

/* 0x800970c4, size 0x9c */
void CheckNullObj(void)
{
    int j; /* r29 */
    anon_numObj__objs_c *group; /* r28 */

    group = &g_PS2GeoGroup[JustLoadedGroupIdx];
    for (j = 0; j < group->numObj; j++) {
        if (!group->_objs[j].displaylist)
            printf("obj %s has not linked!\n", group->_objs[j].name);
    }
}

/* 0x80097160, size 0x570 */
int LoadGeoBinFile(int wavenum, char *groupname, int waveNum)
{
    int fd; /* r18 */
    int i; /* r5 */
    int j; /* r10 */
    int k; /* r20 */
    int fd2; /* r20 */
    char name[256]; /* stack */
    anon_numPack_name_2c *obj; /* r3 */
    int texnum; /* stack */
    anon_name_width_30 **realidx; /* r30 */
    anon_name_width_30 **pptex; /* r21 */
    int total; /* r19 */
    int *temp_data; /* r22 */
    int *keep_data; /* r23 */
    char *temp_name; /* r19 */
    int *cur; /* r6 */
    int *texid; /* r18 */
    char *nn; /* r7 */
    int *buffer; /* r20 */
    int size_size; /* stack */
    int groupid; /* r0 */
    anon_numObj__objs_c *group; /* r29 */

    sprintf(name, "ps2geo/%s/PS2GeoInfo.bin", groupname);
    fd2 = appFileOpen(name, 0);
    if (!fd2)
        return 0;
    if (!g_PS2GeoWave[wavenum].loaded) {
        groupid = GetAnEmptyGeoGroup();
        JustLoadedGroupIdx = groupid;
        group = &g_PS2GeoGroup[groupid];
        appFileRead4(fd2, &group->numObj, 4);
        appFileRead4(fd2, &size_size, 4);
        group->_objs = AllocateMemoryHeap(0, group->numObj * sizeof(anon_numPack_name_2c));
        pptex = AllocateMemoryHeap(0, size_size * 4);
        keep_data = AllocateMemoryHeap(0, size_size * 4 + group->numObj * 16);
        temp_data = AllocateMemoryHeap(0, (group->numObj + size_size) * 4);
        appFileRead4Array(fd2, temp_data, (group->numObj + size_size) * 4);
        appFileRead(fd2, keep_data, group->numObj * 16);
        appFileRead4Array(fd2, keep_data + group->numObj * 4, size_size * 4);
        cur = temp_data;
        nn = (char *)keep_data;
        for (i = 0; i < group->numObj; i++) {
            obj = &group->_objs[i];
            obj->numPack = *cur++;
            obj->name = nn;
            nn += 16;
        }
        texid = (int *)nn;
        appFileRead4(fd2, &texnum, 4);
        temp_name = AllocateMemoryHeap(0, texnum * 16);
        realidx = AllocateMemoryHeap(0, texnum * 4);
        appFileRead(fd2, temp_name, texnum * 16);
        nn = temp_name;
        for (i = 0; i < texnum; i++) {
            nn[15] = 0;
            realidx[i] = (anon_name_width_30 *)FindPs2Texture(groupname, nn);
            if (i == 76 && !strcmp(nn, "BRI_LIGHTHOUSE_"))
                realidx[i] = (anon_name_width_30 *)FindPs2Texture2(groupname, nn);
            if (i < texnum) {
                nn[15] = 0;
                nn += 16;
            }
        }
        for (i = 0; i < group->numObj; i++) {
            obj = &group->_objs[i];
            obj->tex = pptex;
            for (j = 0; j < obj->numPack; j++) {
                if (texid[j] != -1)
                    obj->tex[j] = realidx[texid[j]];
                else
                    obj->tex[j] = 0;
            }
            texid += obj->numPack;
            pptex += obj->numPack;
        }
        for (i = 0; i < group->numObj; i++) {
            obj = &group->_objs[i];
            obj->displaylist = 0;
            obj->pPack = (unsigned int **)obj;
        }
        DeallocateMemory(temp_name);
        DeallocateMemory(temp_data);
        appFileClose(fd2);
        sprintf(name, "ps2geo/%s/NGCGeoData.bin", groupname);
        fd = appFileOpen(name, 0);
        if (!fd)
            return 0;
        total = appFileGetSize(fd);
        buffer = AllocateMemoryHeap(0, total);
        appFileRead4Array(fd, buffer, total);
        appFileClose(fd);
        group->_GeoBuf = buffer;
        cur = buffer;
        for (i = 0; i < group->numObj; i++) {
            obj = &group->_objs[i];
            obj->nStrips = cur[0];
            obj->iHaveFWBW = cur[1];
            cur += 2;
            obj->strips = AllocateMemoryHeap(0, obj->nStrips * sizeof(anon_nVertex_iTex_10));
            for (j = 0; j < obj->nStrips; j++) {
                obj->strips[j].nVertex = cur[0];
                obj->strips[j].iTex = cur[1];
                obj->strips[j].iHasAlpha = cur[2];
                cur += 3;
                if (obj->strips[j].iTex != -1) {
                    for (k = 0; k < obj->numPack; k++) {
                        if (obj->tex[k] && realidx[obj->strips[j].iTex] &&
                            !strcmp(realidx[obj->strips[j].iTex]->name, obj->tex[k]->name)) {
                            obj->strips[j].iTex = k;
                            break;
                        }
                    }
                }
                if (obj->strips[j].iTex >= 0 && !strcmp(obj->tex[obj->strips[j].iTex]->name, "WHITE")) {
                    obj->strips[j].iTex = -1;
                    obj->strips[j].iHasAlpha = 0xFECE0EE5;
                }
                obj->strips[j].vertexs = (anon_pos_nom_24 *)cur;
                cur += obj->strips[j].nVertex * sizeof(anon_pos_nom_24) / sizeof(int);
            }
        }
        DeallocateMemory(realidx);
        g_PS2GeoWave[wavenum].loaded = 1;
        g_PS2GeoWave[wavenum].group = group;
        return 1;
    }
}

/* 0x800976d0, size 0x4 */
void ReMapTexToDCDef(void)
{
}

/* 0x800976d4, size 0x44 */
void PS2InitGeoWave(void)
{
    memset(g_PS2GeoWave, 0, 0x9d8);
    memset(g_GeoGroupUsed, 0, sizeof(g_GeoGroupUsed));
}

/* 0x80097718, size 0xf8 */
void ReleaseGeoBinFile(int wavenum)
{
    anon_numObj__objs_c *group; /* r28 */
    int i; /* r27 */

    if (wavenum == 1)
        wavenum = 1;
    if (g_PS2GeoWave[wavenum].loaded) {
        group = g_PS2GeoWave[wavenum].group;
        if (group->numObj) {
            DeallocateMemory(group->_objs->name);
            DeallocateMemory(group->_objs->tex);
            DeallocateMemory(group->_GeoBuf);
            for (i = 0; i < group->numObj; i++)
                DeallocateMemory(group->_objs[i].strips);
            DeallocateMemory(group->_objs);
            group->numObj = 0;
        }
        g_PS2GeoWave[wavenum].loaded = 0;
        g_GeoGroupUsed[group - g_PS2GeoGroup] = 0;
    }
}

/* 0x80097810, size 0x20 */
void MyDrawObjectWithEnvmap(unsigned int *dlist, anon_name_width_30 *tex, int cull)
{
    PS2DrawObject((anon_numPack_name_2c *)dlist, tex, cull);
}

/* 0x80097830, size 0x58 */
unsigned int *GetThePacket(unsigned int *pack, int idx)
{
    int i; /* r6 */
    unsigned int *p; /* r3 */

    p = pack + 12;
    for (i = 0; i < idx; i++) {
        p += 0xcf0 / 4;
    }
    return p;
}

