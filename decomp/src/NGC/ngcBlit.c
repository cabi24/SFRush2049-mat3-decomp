/* ngcBlit.c - 2D blit helpers: orthographic camera setup, GX immediate-mode quad/strip blits, screen-size (viewport offset) adjustment. Matches.
   Original: D:\rush_ngc\Rush_NGC\NGC\ngcBlit.c; assembly: build/2049LE/asm/NGC/ngcBlit.s */
#include "rush/rush_protos.h"

/* SDK prototypes not yet in dolphin.h */
void C_MTXOrtho(float m[4][4], float t, float b, float l, float r, float n, float f);
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

#define ATTRIBUTE_ALIGN(n) __attribute__((aligned(n)))

static void CameraInit_swap(float (*v)[4]);
static void CameraInit(float (*v)[4]);

static int DownY_ori;
static int LeftX_ori;
static int DownY;
static int LeftX;

/* 0x800a9574, size 0x30 */
void ngcBlitInit(void)
{
    float v[3][4]; /* stack */

    CameraInit(v);
    GXLoadPosMtxImm(v, 0);
}

/* 0x800a95a4, size 0x30 */
void ngcBlitInit_swap(void)
{
    float v[3][4]; /* stack */

    CameraInit_swap(v);
    GXLoadPosMtxImm(v, 0);
}

/* 0x800a95d4, size 0x10c */
static void CameraInit_swap(float (*v)[4])
{
    float p[4][4]; /* stack */
    tVec3 camPt = { 0.0f, 0.0f, 0.0f }; /* stack */
    tVec3 at = { 0.0f, 0.0f, -100.0f }; /* stack */
    tVec3 up = { 0.0f, 1.0f, 0.0f }; /* stack */

    camPt.x += g_MoveCamX;
    camPt.y += g_MoveCamY;
    camPt.z += g_MoveCamZ;
    at.x += g_MoveCamX;
    at.y += g_MoveCamY;
    at.z += g_MoveCamZ;
    C_MTXOrtho(p, 0.0f, 480.0f, 0.0f, 640.0f, -4000.0f, 4000.0f);
    GXSetProjection(p, 1);
    C_MTXLookAt(v, &camPt, &up, &at);
}

/* 0x800a96e0, size 0xb8 */
static void CameraInit(float (*v)[4])
{
    float p[4][4]; /* stack */
    tVec3 camPt = { 0.0f, 0.0f, 0.0f }; /* stack */
    tVec3 at = { 0.0f, 0.0f, -100.0f }; /* stack */
    tVec3 up = { 0.0f, 1.0f, 0.0f }; /* stack */

    C_MTXOrtho(p, 448.0f, 0.0f, 0.0f, 640.0f, -4000.0f, 4000.0f);
    GXSetProjection(p, 1);
    C_MTXLookAt(v, &camPt, &up, &at);
}

/* 0x800a9798, size 0x71c */
void NGCDC_SendBlit(void * tex, int nNumVertex, void * Vertices, unsigned int color)
{
    tTextureEx *pTexDef = (tTextureEx *)((tTexture *)tex)->strip; /* r30 */
    KMVERTICES *pVertices = Vertices; /* r29 */
    int i; /* r16 */
    int j; /* r0 */
    float tfa; /* f6 */
    float tfb; /* f7 */
    float Vert_f32[12] ATTRIBUTE_ALIGN(32) = { 0.0f, 480.0f, 0.0f, 640.0f, 480.0f, 0.0f, 640.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }; /* stack */
    unsigned long Colors_u32[4] ATTRIBUTE_ALIGN(32) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff }; /* stack */
    float TexCoords_f32[8] ATTRIBUTE_ALIGN(32) = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f }; /* stack */
    float vp[6] ATTRIBUTE_ALIGN(32); /* stack */

    if (pTexDef == 0) {
        return;
    }
    if (strstr(pTexDef->name, "B2")) {
        return;
    }
    ngcBlitInit();
    for (i = 0; i < nNumVertex; i++) {
        Vert_f32[i * 3] = pVertices[i].fX;
        Vert_f32[i * 3 + 1] = 448.0f - pVertices[i].fY;
        if (strcmp(pTexDef->name, "MIDWAY") == 0 || strcmp(pTexDef->name, "BACKGROUND") == 0) {
            Vert_f32[i * 3 + 2] = -3999.99f;
        } else {
            Vert_f32[i * 3 + 2] = 0.0f;
        }
    }
    for (i = 0; i < nNumVertex; i++) {
        j = i + 1;
        if (j == 4) {
            j = 0;
        }
        TexCoords_f32[i * 2] = pVertices[j].fU;
        TexCoords_f32[i * 2 + 1] = pVertices[j].fV;
    }
    tfa = TexCoords_f32[0];
    tfb = TexCoords_f32[1];
    TexCoords_f32[0] = TexCoords_f32[6];
    TexCoords_f32[1] = TexCoords_f32[7];
    TexCoords_f32[6] = TexCoords_f32[4];
    TexCoords_f32[7] = TexCoords_f32[5];
    TexCoords_f32[4] = TexCoords_f32[2];
    TexCoords_f32[5] = TexCoords_f32[3];
    TexCoords_f32[2] = tfa;
    TexCoords_f32[3] = tfb;
    for (i = 0; i < nNumVertex; i++) {
        Colors_u32[i] = ((color & 0xff00) << 16) + (color & 0xff0000) + ((color >> 24) << 8) + (color & 0xff);
    }
    if (strcmp(pTexDef->name, "MIDWAY") == 0) {
        Vert_f32[0] = 0.0f;
        Vert_f32[1] = 0.0f;
        Vert_f32[2] = -3999.99f;
        Vert_f32[3] = 640.0f;
        Vert_f32[4] = 0.0f;
        Vert_f32[5] = -3999.99f;
        Vert_f32[6] = 640.0f;
        Vert_f32[7] = 448.0f;
        Vert_f32[8] = -3999.99f;
        Vert_f32[9] = 0.0f;
        Vert_f32[10] = 448.0f;
        Vert_f32[11] = -3999.99f;
    }
    if (strstr(pTexDef->name, "MAP_LG")) {
        pTexDef->psm = 0xffff;
    }
    GXGetViewportv(vp);
    vp[0] = 0.0f;
    vp[1] = 0.0f;
    vp[2] = 640.0f;
    vp[3] = 448.0f;
    vp[4] = 0.0f;
    vp[5] = 1.0f;
    GXSetViewport(vp[0], vp[1], vp[2], vp[3], vp[4], vp[5]);
    GXSetScissor(vp[0], vp[1], vp[2], vp[3]);
    GXSetNumTevStages(1);
    GXSetNumChans(1);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxDesc(11, 1);
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetVtxAttrFmt(0, 11, 1, 5, 0);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    GXSetZCompLoc(0);
    GXSetCullMode(0);
    GXSetAlphaCompare(7, 0, 0, 7, 0);
    GXLoadTexObj((_GXTexObj *)pTexDef->texBase, 0);
    GXSetTevOp(0, 0);
    GXSetBlendMode(1, 4, 5, 0);
    if (pTexDef->ps2flags & 1) {
        GXSetTevOp(0, 0);
        GXSetBlendMode(1, 4, 5, 0);
    } else {
        GXSetTevOp(0, 0);
        GXSetBlendMode(0, 4, 5, 0);
    }
    if (pTexDef->ps2flags & 4) {
        GXSetAlphaCompare(4, 0x78, 0, 3, 0xff);
    } else {
        GXSetAlphaCompare(7, 0, 0, 7, 0);
    }
    if (pTexDef->ps2flags & 8) {
        GXSetZMode(1, 3, 0);
    } else if (pTexDef->ps2flags & 0x40) {
        GXSetZMode(0, 7, 0);
    } else {
        GXSetZMode(1, 3, 1);
    }
    if (pTexDef->ps2flags & 0x10) {
        GXSetCullMode(1);
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
    GXBegin(0x80, 0, 4);
    for (i = 0; i < 4; i++) {
        GXPosition3f32(Vert_f32[i * 3], Vert_f32[i * 3 + 1], Vert_f32[i * 3 + 2]);
        GXColor1u32(Colors_u32[i]);
        GXTexCoord2f32(TexCoords_f32[i * 2], TexCoords_f32[i * 2 + 1]);
    }
    GXEnd();
}

/* 0x800a9eb4, size 0x720 */
void NGCDC_SendBlit_MENU(void * tex, void * tex_a, int nNumVertex, void * Vertices, unsigned int color)
{
    KMVERTICES *pVertices; /* r31 */
    int i; /* r6 */
    float tfa; /* f2 */
    float tfb; /* f3 */
    float Vert_f32[12] ATTRIBUTE_ALIGN(32) = { 0.0f, 480.0f, 0.0f, 640.0f, 480.0f, 0.0f, 640.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }; /* stack */
    unsigned long Colors_u32[4] ATTRIBUTE_ALIGN(32) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff }; /* stack */
    float TexCoords_f32[8] ATTRIBUTE_ALIGN(32) = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f }; /* stack */
    float vp[6] ATTRIBUTE_ALIGN(32); /* stack */
    float v[3][4]; /* stack */
    float p[4][4]; /* stack */
    tVec3 camPt = { 0.0f, 0.0f, 0.0f }; /* stack */
    tVec3 at = { 0.0f, 0.0f, -100.0f }; /* stack */
    tVec3 up = { 0.0f, 1.0f, 0.0f }; /* stack */

    pVertices = Vertices;
    CameraInit(v);
    GXLoadPosMtxImm(v, 0);
    C_MTXOrtho(p, 480.0f, 0.0f, 0.0f, 640.0f, -4000.0f, 4000.0f);
    GXSetProjection(p, 1);
    C_MTXLookAt(v, &camPt, &up, &at);
    for (i = 0; i < nNumVertex; i++) {
        Vert_f32[i * 3] = pVertices[i].fX + 60.0f;
        Vert_f32[i * 3 + 1] = 448.0f - pVertices[i].fY - 10.0f;
        Vert_f32[i * 3 + 2] = 0.0f;
    }
    tfa = Vert_f32[6];
    tfb = Vert_f32[7];
    Vert_f32[6] = Vert_f32[9];
    Vert_f32[7] = Vert_f32[10];
    Vert_f32[9] = tfa;
    Vert_f32[10] = tfb;
    TexCoords_f32[0] = pVertices[0].fU;
    TexCoords_f32[1] = pVertices[0].fV;
    TexCoords_f32[2] = pVertices[1].fU;
    TexCoords_f32[3] = pVertices[1].fV;
    TexCoords_f32[4] = pVertices[3].fU;
    TexCoords_f32[5] = pVertices[3].fV;
    TexCoords_f32[6] = pVertices[2].fU;
    TexCoords_f32[7] = pVertices[2].fV;
    for (i = 0; i < nNumVertex; i++) {
        Colors_u32[i] = color;
    }
    GXGetViewportv(vp);
    vp[0] = 0.0f;
    vp[1] = 0.0f;
    vp[2] = 640.0f;
    vp[3] = 448.0f;
    vp[4] = 0.0f;
    vp[5] = 1.0f;
    GXSetViewport(vp[0], vp[1], vp[2], vp[3], vp[4], vp[5]);
    GXSetScissor(vp[0], vp[1], vp[2], vp[3]);
    GXSetNumTevStages(1);
    GXSetNumChans(1);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxDesc(11, 1);
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetVtxAttrFmt(0, 11, 1, 5, 0);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    GXSetZCompLoc(0);
    GXSetCullMode(0);
    GXSetAlphaCompare(7, 0, 0, 7, 0);
    GXLoadTexObj(tex, 0);
    GXSetTevOp(0, 0);
    GXSetBlendMode(1, 4, 5, 0);
    GXSetZMode(1, 3, 1);
    GXSetCullMode(0);
    GXSetNumTevStages(2);
    GXLoadTexObj(tex_a, 1);
    GXSetTevOrder(1, 0, 1, 0xff);
    GXSetTevOp(1, 0);
    GXSetTevSwapMode(1, 0, 1);
    GXSetTevSwapModeTable(1, 0, 0, 0, 1);
    GXBegin(0x80, 0, 4);
    for (i = 0; i < 4; i++) {
        GXPosition3f32(Vert_f32[i * 3], Vert_f32[i * 3 + 1], Vert_f32[i * 3 + 2]);
        GXColor1u32(Colors_u32[i]);
        GXTexCoord2f32(TexCoords_f32[i * 2], TexCoords_f32[i * 2 + 1]);
    }
    GXEnd();
}

/* 0x800aa5d4, size 0x650 */
void NGCDC_SendBlit_MENU_RGBA(void * tex, void * tex_a, int nNumVertex, void * Vertices, unsigned int color_1, unsigned int color_2, unsigned int color_3, unsigned int color_4)
{
    KMVERTICES *pVertices; /* r31 */
    int i; /* r6 */
    float tfa; /* f2 */
    float tfb; /* f3 */
    float Vert_f32[12] ATTRIBUTE_ALIGN(32) = { 0.0f, 480.0f, 0.0f, 640.0f, 480.0f, 0.0f, 640.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }; /* stack */
    unsigned long Colors_u32[4] ATTRIBUTE_ALIGN(32) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff }; /* stack */
    float TexCoords_f32[8] ATTRIBUTE_ALIGN(32) = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f }; /* stack */
    float vp[6] ATTRIBUTE_ALIGN(32); /* stack */
    float v[3][4]; /* stack */
    float p[4][4]; /* stack */
    tVec3 camPt = { 0.0f, 0.0f, 0.0f }; /* stack */
    tVec3 at = { 0.0f, 0.0f, -100.0f }; /* stack */
    tVec3 up = { 0.0f, 1.0f, 0.0f }; /* stack */

    pVertices = Vertices;
    CameraInit(v);
    GXLoadPosMtxImm(v, 0);
    C_MTXOrtho(p, 480.0f, 0.0f, 0.0f, 640.0f, -4000.0f, 4000.0f);
    GXSetProjection(p, 1);
    C_MTXLookAt(v, &camPt, &up, &at);
    for (i = 0; i < nNumVertex; i++) {
        Vert_f32[i * 3] = pVertices[i].fX + 60.0f;
        Vert_f32[i * 3 + 1] = 448.0f - pVertices[i].fY - 10.0f;
        Vert_f32[i * 3 + 2] = 0.0f;
    }
    tfa = Vert_f32[6];
    tfb = Vert_f32[7];
    Vert_f32[6] = Vert_f32[9];
    Vert_f32[7] = Vert_f32[10];
    Vert_f32[9] = tfa;
    Vert_f32[10] = tfb;
    TexCoords_f32[0] = pVertices[0].fU;
    TexCoords_f32[1] = pVertices[0].fV;
    TexCoords_f32[2] = pVertices[1].fU;
    TexCoords_f32[3] = pVertices[1].fV;
    TexCoords_f32[4] = pVertices[3].fU;
    TexCoords_f32[5] = pVertices[3].fV;
    TexCoords_f32[6] = pVertices[2].fU;
    TexCoords_f32[7] = pVertices[2].fV;
    Colors_u32[0] = color_1;
    Colors_u32[1] = color_2;
    Colors_u32[2] = color_3;
    Colors_u32[3] = color_4;
    GXGetViewportv(vp);
    vp[0] = 0.0f;
    vp[1] = 0.0f;
    vp[2] = 640.0f;
    vp[3] = 448.0f;
    vp[4] = 0.0f;
    vp[5] = 1.0f;
    GXSetViewport(vp[0], vp[1], vp[2], vp[3], vp[4], vp[5]);
    GXSetScissor(vp[0], vp[1], vp[2], vp[3]);
    GXSetNumTevStages(1);
    GXSetNumChans(1);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxDesc(11, 1);
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetVtxAttrFmt(0, 11, 1, 5, 0);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    GXSetZCompLoc(0);
    GXSetCullMode(0);
    GXSetAlphaCompare(7, 0, 0, 7, 0);
    GXSetTevOp(0, 4);
    GXSetBlendMode(1, 4, 5, 0);
    GXSetZMode(1, 3, 1);
    GXSetCullMode(0);
    GXBegin(0x80, 0, 4);
    for (i = 0; i < 4; i++) {
        GXPosition3f32(Vert_f32[i * 3], Vert_f32[i * 3 + 1], Vert_f32[i * 3 + 2]);
        GXColor1u32(Colors_u32[i]);
        GXTexCoord2f32(TexCoords_f32[i * 2], TexCoords_f32[i * 2 + 1]);
    }
    GXEnd();
}

/* stripped by the linker; kept only because its local-array initialisers still occupy .rodata between the live functions */
static void BlitOneTpl(void)
{
    int i; /* r0 */
    float Vert_f32[12] ATTRIBUTE_ALIGN(32) = { 0.0f, 0.0f, -3999.99f, 640.0f, 0.0f, -3999.99f, 640.0f, 448.0f, -3999.99f, 0.0f, 448.0f, -3999.99f };
    unsigned long Colors_u32[4] ATTRIBUTE_ALIGN(32) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
    float TexCoords_f32[8] ATTRIBUTE_ALIGN(32) = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

    GXBegin(0x80, 0, 4);
    for (i = 0; i < 4; i++) {
        GXPosition3f32(Vert_f32[i * 3], Vert_f32[i * 3 + 1], Vert_f32[i * 3 + 2]);
        GXColor1u32(Colors_u32[i]);
        GXTexCoord2f32(TexCoords_f32[i * 2], TexCoords_f32[i * 2 + 1]);
    }
    GXEnd();
}

/* 0x800aac24, size 0x34c */
void BlitOneTpl_XYWHRGBA(float x, float y, float w, float h, unsigned long color, _GXTexObj *pTexObj, _GXTexObj *pTexObj_a)
{
    int i; /* r0 */
    float Vert_f32[12]; /* stack */
    unsigned long Colors_u32[4]; /* stack */
    float TexCoords_f32[8]; /* stack */

{
    int i; /* r0 */
    float Vert_f32[12] ATTRIBUTE_ALIGN(32) = { 0.0f, 0.0f, 0.0f, 640.0f, 0.0f, 0.0f, 640.0f, 448.0f, 0.0f, 0.0f, 448.0f, 0.0f };
    unsigned long Colors_u32[4] ATTRIBUTE_ALIGN(32) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
    float TexCoords_f32[8] ATTRIBUTE_ALIGN(32) = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

    Vert_f32[0] = x;
    Vert_f32[1] = y - h;
    Vert_f32[3] = x + w;
    Vert_f32[4] = y - h;
    Vert_f32[6] = x + w;
    Vert_f32[7] = y;
    Vert_f32[9] = x;
    Vert_f32[10] = y;
    Colors_u32[0] = color;
    Colors_u32[1] = color;
    Colors_u32[2] = color;
    Colors_u32[3] = color;
    ngcBlitInit();
    GXSetNumTevStages(1);
    GXSetNumChans(1);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxDesc(11, 1);
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetVtxAttrFmt(0, 11, 1, 5, 0);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    GXSetZCompLoc(0);
    GXSetCullMode(0);
    GXSetAlphaCompare(7, 0, 0, 7, 0);
    GXLoadTexObj(pTexObj, 0);
    GXSetTevOp(0, 0);
    GXSetBlendMode(1, 4, 5, 0);
    if (pTexObj_a) {
        GXSetNumTevStages(2);
        GXLoadTexObj(pTexObj_a, 1);
        GXSetTevOrder(1, 0, 1, 0xff);
        GXSetTevOp(1, 0);
        GXSetTevSwapMode(1, 0, 1);
        GXSetTevSwapModeTable(1, 0, 0, 0, 1);
    }
    GXBegin(0x80, 0, 4);
    for (i = 0; i < 4; i++) {
        GXPosition3f32(Vert_f32[i * 3], Vert_f32[i * 3 + 1], Vert_f32[i * 3 + 2]);
        GXColor1u32(Colors_u32[i]);
        GXTexCoord2f32(TexCoords_f32[i * 2], TexCoords_f32[i * 2 + 1]);
    }
    GXEnd();
}
}

/* 0x800aaf70, size 0x344 */
void BlitOneTpl_LURDRGBA_TEX(float left, float up, float right, float down, unsigned long color, _GXTexObj *obj, _GXTexObj *obj_a)
{
    int i; /* r0 */
    float Vert_f32[12]; /* stack */
    unsigned long Colors_u32[4]; /* stack */
    float TexCoords_f32[8]; /* stack */

{
    int i; /* r0 */
    float Vert_f32[12] ATTRIBUTE_ALIGN(32) = { 0.0f, 0.0f, 0.0f, 640.0f, 0.0f, 0.0f, 640.0f, 448.0f, 0.0f, 0.0f, 448.0f, 0.0f };
    unsigned long Colors_u32[4] ATTRIBUTE_ALIGN(32) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
    float TexCoords_f32[8] ATTRIBUTE_ALIGN(32) = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

    Vert_f32[0] = left;
    Vert_f32[1] = up;
    Vert_f32[3] = right;
    Vert_f32[4] = up;
    Vert_f32[6] = right;
    Vert_f32[7] = down;
    Vert_f32[9] = left;
    Vert_f32[10] = down;
    Colors_u32[0] = color;
    Colors_u32[1] = color;
    Colors_u32[2] = color;
    Colors_u32[3] = color;
    ngcBlitInit_swap();
    GXSetNumTevStages(1);
    GXSetNumChans(1);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxDesc(11, 1);
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetVtxAttrFmt(0, 11, 1, 5, 0);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    GXSetZCompLoc(0);
    GXSetCullMode(0);
    GXSetAlphaCompare(7, 0, 0, 7, 0);
    GXLoadTexObj(obj, 0);
    GXSetTevOp(0, 0);
    GXSetBlendMode(1, 4, 5, 0);
    if (obj_a) {
        GXSetNumTevStages(2);
        GXLoadTexObj(obj_a, 1);
        GXSetTevOrder(1, 0, 1, 0xff);
        GXSetTevOp(1, 0);
        GXSetTevSwapMode(1, 0, 1);
        GXSetTevSwapModeTable(1, 0, 0, 0, 1);
    }
    GXBegin(0x80, 0, 4);
    for (i = 0; i < 4; i++) {
        GXPosition3f32(Vert_f32[i * 3], Vert_f32[i * 3 + 1], Vert_f32[i * 3 + 2]);
        GXColor1u32(Colors_u32[i]);
        GXTexCoord2f32(TexCoords_f32[i * 2], TexCoords_f32[i * 2 + 1]);
    }
    GXEnd();
}
}

/* 0x800ab2b4, size 0x374 */
void BlitOneTpl_LURDRGBAUV_TEX(float left, float up, float right, float down, float leftU, float upV, float rightU, float downV, unsigned long color, _GXTexObj *obj, _GXTexObj *obj_a)
{
    int i; /* r0 */
    float Vert_f32[12]; /* stack */
    unsigned long Colors_u32[4]; /* stack */
    float TexCoords_f32[8]; /* stack */

{
    int i; /* r0 */
    float Vert_f32[12] ATTRIBUTE_ALIGN(32) = { 0.0f, 0.0f, 0.0f, 640.0f, 0.0f, 0.0f, 640.0f, 448.0f, 0.0f, 0.0f, 448.0f, 0.0f };
    unsigned long Colors_u32[4] ATTRIBUTE_ALIGN(32) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
    float TexCoords_f32[8] ATTRIBUTE_ALIGN(32) = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

    Vert_f32[0] = left;
    Vert_f32[1] = up;
    Vert_f32[3] = right;
    Vert_f32[4] = up;
    Vert_f32[6] = right;
    Vert_f32[7] = down;
    Vert_f32[9] = left;
    Vert_f32[10] = down;
    Colors_u32[0] = color;
    Colors_u32[1] = color;
    Colors_u32[2] = color;
    Colors_u32[3] = color;
    TexCoords_f32[0] = leftU;
    TexCoords_f32[1] = upV;
    TexCoords_f32[2] = rightU;
    TexCoords_f32[3] = upV;
    TexCoords_f32[4] = rightU;
    TexCoords_f32[5] = downV;
    TexCoords_f32[6] = leftU;
    TexCoords_f32[7] = downV;
    ngcBlitInit_swap();
    GXSetNumTevStages(1);
    GXSetNumChans(1);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxDesc(11, 1);
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetVtxAttrFmt(0, 11, 1, 5, 0);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    GXSetZCompLoc(0);
    GXSetZMode(1, 7, 0);
    GXSetCullMode(0);
    GXSetAlphaCompare(7, 0, 0, 7, 0);
    GXLoadTexObj(obj, 0);
    GXSetTevOp(0, 0);
    GXSetBlendMode(1, 4, 5, 0);
    if (obj_a) {
        GXSetNumTevStages(2);
        GXLoadTexObj(obj_a, 1);
        GXSetTevOrder(1, 0, 1, 0xff);
        GXSetTevOp(1, 0);
        GXSetTevSwapMode(1, 0, 1);
        GXSetTevSwapModeTable(1, 0, 0, 0, 1);
    }
    GXBegin(0x80, 0, 4);
    for (i = 0; i < 4; i++) {
        GXPosition3f32(Vert_f32[i * 3], Vert_f32[i * 3 + 1], Vert_f32[i * 3 + 2]);
        GXColor1u32(Colors_u32[i]);
        GXTexCoord2f32(TexCoords_f32[i * 2], TexCoords_f32[i * 2 + 1]);
    }
    GXEnd();
}
}

/* 0x800ab628, size 0x2d0 */
void BlitOneTpl_LURDRGBA_4COLOR(float left, float up, float right, float down, unsigned int color_1, unsigned int color_2, unsigned int color_3, unsigned int color_4)
{
    int i; /* r0 */
    float Vert_f32[12]; /* stack */
    unsigned long Colors_u32[4]; /* stack */
    float TexCoords_f32[8]; /* stack */

{
    int i; /* r0 */
    float Vert_f32[12] ATTRIBUTE_ALIGN(32) = { 0.0f, 0.0f, 0.0f, 640.0f, 0.0f, 0.0f, 640.0f, 448.0f, 0.0f, 0.0f, 448.0f, 0.0f };
    unsigned long Colors_u32[4] ATTRIBUTE_ALIGN(32) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
    float TexCoords_f32[8] ATTRIBUTE_ALIGN(32) = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

    Vert_f32[0] = left;
    Vert_f32[1] = up;
    Vert_f32[3] = right;
    Vert_f32[4] = up;
    Vert_f32[6] = right;
    Vert_f32[7] = down;
    Vert_f32[9] = left;
    Vert_f32[10] = down;
    Colors_u32[0] = color_1;
    Colors_u32[1] = color_2;
    Colors_u32[2] = color_3;
    Colors_u32[3] = color_4;
    ngcBlitInit_swap();
    GXSetNumTevStages(1);
    GXSetNumChans(1);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxDesc(11, 1);
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetVtxAttrFmt(0, 11, 1, 5, 0);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    GXSetZCompLoc(0);
    GXSetCullMode(0);
    GXSetAlphaCompare(7, 0, 0, 7, 0);
    GXSetTevOp(0, 4);
    GXSetZMode(1, 3, 1);
    GXSetCullMode(0);
    GXBegin(0x80, 0, 4);
    for (i = 0; i < 4; i++) {
        GXPosition3f32(Vert_f32[i * 3], Vert_f32[i * 3 + 1], Vert_f32[i * 3 + 2]);
        GXColor1u32(Colors_u32[i]);
        GXTexCoord2f32(TexCoords_f32[i * 2], TexCoords_f32[i * 2 + 1]);
    }
    GXEnd();
}
}

/* stripped by the linker; kept only because its local-array initialisers still occupy .rodata between the live functions */
static void BlitOneTpl_Obj_A(void * obj, void * obj_a)
{
    int i; /* r0 */
    float Vert_f32[12] ATTRIBUTE_ALIGN(32) = { 0.0f, 0.0f, 0.0f, 640.0f, 0.0f, 0.0f, 640.0f, 448.0f, 0.0f, 0.0f, 448.0f, 0.0f };
    unsigned long Colors_u32[4] ATTRIBUTE_ALIGN(32) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
    float TexCoords_f32[8] ATTRIBUTE_ALIGN(32) = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
    float vp[6] ATTRIBUTE_ALIGN(32); /* stack */

    GXBegin(0x80, 0, 4);
    for (i = 0; i < 4; i++) {
        GXPosition3f32(Vert_f32[i * 3], Vert_f32[i * 3 + 1], Vert_f32[i * 3 + 2]);
        GXColor1u32(Colors_u32[i]);
        GXTexCoord2f32(TexCoords_f32[i * 2], TexCoords_f32[i * 2 + 1]);
    }
    GXEnd();
}

/* stripped by the linker; kept only because its local-array initialisers still occupy .rodata between the live functions */
static void BlitOneTpl_Obj(void * obj)
{
    int i; /* r0 */
    float Vert_f32[12] ATTRIBUTE_ALIGN(32) = { 0.0f, 0.0f, 0.0f, 640.0f, 0.0f, 0.0f, 640.0f, 448.0f, 0.0f, 0.0f, 448.0f, 0.0f };
    unsigned long Colors_u32[4] ATTRIBUTE_ALIGN(32) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
    float TexCoords_f32[8] ATTRIBUTE_ALIGN(32) = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
    float vp[6] ATTRIBUTE_ALIGN(32); /* stack */

    GXBegin(0x80, 0, 4);
    for (i = 0; i < 4; i++) {
        GXPosition3f32(Vert_f32[i * 3], Vert_f32[i * 3 + 1], Vert_f32[i * 3 + 2]);
        GXColor1u32(Colors_u32[i]);
        GXTexCoord2f32(TexCoords_f32[i * 2], TexCoords_f32[i * 2 + 1]);
    }
    GXEnd();
}

/* stripped by the linker; kept only because its local-array initialisers still occupy .rodata between the live functions */
static void BlitOneTpl_PS(void)
{
    int i; /* r0 */
    float Vert_f32[12] ATTRIBUTE_ALIGN(32) = { 220.0f, 0.0f, 0.0f, 420.0f, 0.0f, 0.0f, 420.0f, 80.0f, 0.0f, 220.0f, 80.0f, 0.0f };
    unsigned long Colors_u32[4] ATTRIBUTE_ALIGN(32) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
    float TexCoords_f32[8] ATTRIBUTE_ALIGN(32) = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

    GXBegin(0x80, 0, 4);
    for (i = 0; i < 4; i++) {
        GXPosition3f32(Vert_f32[i * 3], Vert_f32[i * 3 + 1], Vert_f32[i * 3 + 2]);
        GXColor1u32(Colors_u32[i]);
        GXTexCoord2f32(TexCoords_f32[i * 2], TexCoords_f32[i * 2 + 1]);
    }
    GXEnd();
}

/* 0x800ab8f8, size 0x114 */
void SetViewPortForScreenSize(void)
{
    float vp[6]; /* stack */

{
    float vp[6]; /* stack */

    GXGetViewportv(vp);
    vp[0] = LeftX;
    vp[1] = DownY;
    vp[2] = 640.0f - LeftX * 2;
    vp[3] = 448.0f - DownY * 2;
    vp[4] = 0.0f;
    vp[5] = 1.0f;
    GXSetViewport(vp[0], vp[1], vp[2], vp[3], vp[4], vp[5]);
    GXSetScissor(vp[0], vp[1], vp[2], vp[3]);
}
}

/* 0x800aba0c, size 0x2a0 */
void BlitOneTpl_Obj_Frame(void * obj)
{
    int i; /* r0 */
    float Vert_f32[12]; /* stack */
    unsigned long Colors_u32[4]; /* stack */
    float TexCoords_f32[8]; /* stack */

{
    int i; /* r0 */
    float Vert_f32[12] ATTRIBUTE_ALIGN(32) = { 0.0f, 0.0f, 0.0f, 640.0f, 0.0f, 0.0f, 640.0f, 448.0f, 0.0f, 0.0f, 448.0f, 0.0f };
    unsigned long Colors_u32[4] ATTRIBUTE_ALIGN(32) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
    float TexCoords_f32[8] ATTRIBUTE_ALIGN(32) = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

    SetViewPortForScreenSize();
    Vert_f32[0] = 0.0f;
    Vert_f32[1] = 0.0f;
    Vert_f32[3] = 640.0f;
    Vert_f32[4] = 0.0f;
    Vert_f32[6] = 640.0f;
    Vert_f32[7] = 448.0f;
    Vert_f32[9] = 0.0f;
    Vert_f32[10] = 448.0f;
    ngcBlitInit();
    GXSetNumTevStages(1);
    GXSetNumChans(1);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxDesc(11, 1);
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetVtxAttrFmt(0, 11, 1, 5, 0);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    GXLoadTexObj(obj, 0);
    GXSetTevOp(0, 3);
    GXBegin(0x80, 0, 4);
    for (i = 0; i < 4; i++) {
        GXPosition3f32(Vert_f32[i * 3], Vert_f32[i * 3 + 1], Vert_f32[i * 3 + 2]);
        GXColor1u32(Colors_u32[i]);
        GXTexCoord2f32(TexCoords_f32[i * 2], TexCoords_f32[i * 2 + 1]);
    }
    GXEnd();
}
}

/* 0x800abcac, size 0x74 */
void TurnScreenSize(int TurnPoint)
{
{
    LeftX += TurnPoint * 640 / 640;
    if (LeftX > 100) {
        LeftX = 100;
        return;
    }
    if (LeftX < -100) {
        LeftX = -100;
        return;
    }
    DownY += TurnPoint * 448 / 640;
}
}

/* 0x800abd20, size 0x14 */
void ConfirmScreenSize(void)
{
{
    LeftX_ori = LeftX;
    DownY_ori = DownY;
}
}

/* 0x800abd34, size 0x14 */
void CancelScreenSize(void)
{
{
    LeftX = LeftX_ori;
    DownY = DownY_ori;
}
}

