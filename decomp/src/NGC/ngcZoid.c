/* ngcZoid.c - GameCube ZOID renderer back end: scene-graph traversal, face/blit submission, frame end. Status: NonMatching, 17/19 (Blits and Blits_First are register-allocation near misses).
   Original: D:\rush_ngc\Rush_NGC\NGC\ngcZoid.c; assembly: build/2049LE/asm/NGC/ngcZoid.s */
/* The shared header declares blit.c's array `Blits[200]`; this unit has a static function of that
 * name and never uses the array. BlitColors is defined with one parameter in blit_pc.c but called
 * here with four (address, format, size, 0): the original declaration was unprototyped. */
#define Blits Blits_header_decl
#define BlitColors BlitColors_DWARF_decl
#include "rush/rush_protos.h"
#undef Blits
#undef BlitColors
#include "msl_math_inline.h"

void BlitColors();
extern double cos(double);

/* SDK prototypes not yet in dolphin.h (as in ngcBlit.c) */
void GXSetCullMode(int mode);
void GXSetAlphaCompare(int comp0, unsigned char ref0, int op, int comp1, unsigned char ref1);
void GXSetBlendMode(int type, int src_factor, int dst_factor, int op);
void GXLoadTexObj(_GXTexObj *obj, int id);
void GXSetTevOrder(int stage, int coord, int map, int color);
void GXSetTevSwapMode(int stage, int ras_sel, int tex_sel);
void GXSetTevSwapModeTable(int table, int red, int green, int blue, int alpha);

#define ATTRIBUTE_ALIGN(n) __attribute__((aligned(n)))
#define OCR_F(off) (*(float *)&OCRamData[off])

static void Blits_First(void);
static void Blits(void);

static int frontFace[4] = { 0, 1, 3, 2 };
static int backFace[4] = { 0, 3, 1, 2 };
static unsigned int ps2_screen_center_x = 0x140;
static unsigned int ps2_screen_center_y = 0xe0;
int drawWindowBorders = 1;
static tRGBA gWhite = { 255, 255, 255, 255 };
static tRGBA gYellow = { 255, 255, 0, 255 };
static int g_DrawFace = 1;
int objsDrawn;
int stripsDrawn;
int trisDrawn;
int vertsDrawn;
unsigned int transformTime;
unsigned int transformTimeMax;
static int *pdwCurrentPtr;
static int ListType;
int numLights;
static signed char gLastMirrorWaves;
/* explicit `= 0`, in this order: emitted at the declaration point, as the target .sbss needs */
static int use_z_value = 0;
int ms_idx = 0;
float gCameraFacing[4][3];
anon_uvs_30 matrix_stack[10];
float gFrontFacing[4][3];
anon_a_l_24 bodyLights;
float camNormals[2][3];

/* 0x800a5ca0, size 0x4 */
void CheckVertexBufferSizes()
{
}

/* 0x800a5ca4, size 0xc */
void DisableDrawFace(void)
{
{
    g_DrawFace = 0;
}
}

/* 0x800a5cb0, size 0xc */
void EnableDrawFace(void)
{
{
    g_DrawFace = 1;
}
}

/* 0x800a5cbc, size 0x790 */
int *DrawFace(tTexture *tex, int *pdw, anon_x_y_28 *v, int nverts, int *face)
{
    int i; /* r16 */
    tTextureEx *pTexDef; /* r30 */
    float Vert_f32[12]; /* stack */
    unsigned long Colors_u32[4]; /* stack */
    float TexCoords_f32[8]; /* stack */
    float vp[6]; /* stack */

{
    int i; /* r16 */
    tTextureEx *pTexDef; /* r30 */
    float Vert_f32[12] ATTRIBUTE_ALIGN(32) = { 0.0f, 480.0f, 0.0f, 640.0f, 480.0f, 0.0f, 640.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }; /* stack */
    unsigned long Colors_u32[4] ATTRIBUTE_ALIGN(32) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff }; /* stack */
    float TexCoords_f32[8] ATTRIBUTE_ALIGN(32) = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f }; /* stack */
    float vp[6] ATTRIBUTE_ALIGN(32); /* stack */

    pTexDef = 0;
    if (tex)
        pTexDef = (tTextureEx *)tex->strip;
    if (!tex)
        return 0;
    if (strcmp(tex->name, "SKID04") == 0)
        return;
    ngcBlitInit();
    for (i = 0; i < nverts; i++) {
        Vert_f32[i * 3] = v[i].sx;
        if (Vert_f32[i * 3] > 640.0f)
            Vert_f32[i * 3] = 640.0f;
        if (Vert_f32[i * 3] < 0.0f)
            Vert_f32[i * 3] = 0.0f;
        Vert_f32[i * 3 + 1] = v[i].sy;
        if (Vert_f32[i * 3 + 1] > 480.0f)
            Vert_f32[i * 3 + 1] = 480.0f;
        if (Vert_f32[i * 3 + 1] < 0.0f)
            Vert_f32[i * 3 + 1] = 0.0f;
        if (strstr(tex->name, "CNTDWN"))
            Vert_f32[i * 3 + 2] = 0.0f;
        else if (strstr(tex->name, "ARROW"))
            Vert_f32[i * 3 + 2] = 0.0f;
        else
            Vert_f32[i * 3 + 2] = -3999.99f;
        Vert_f32[i * 3 + 1] = 448.0f - Vert_f32[i * 3 + 1];
    }
    for (i = 0; i < nverts; i++) {
        TexCoords_f32[i * 2] = v[i].U;
        TexCoords_f32[i * 2 + 1] = v[i].V;
    }
    for (i = 0; i < nverts; i++) {
        if (strcmp(tex->name, "BACKGROUND_SFX") == 0)
            Colors_u32[i] = 0xffffffff;
        else
            Colors_u32[i] = (v[i].clr.byte.bRed << 24) + (v[i].clr.byte.bGreen << 16) + (v[i].clr.byte.bBlue << 8) + v[i].clr.byte.bAlpha;
    }
    GXGetViewportv(vp);
    {
        float ox = vp[0];
        float oy = vp[1];
        float ow = vp[2];
        float oh = vp[3];
        float on = vp[4];
        float of = vp[5];

        vp[0] = 0.0f;
        vp[1] = 0.0f;
        vp[2] = 640.0f;
        vp[3] = 448.0f;
        vp[4] = 0.0f;
        vp[5] = 1.0f;
        GXSetViewport(vp[0], vp[1], vp[2], vp[3], vp[4], vp[5]);
        GXSetNumTevStages(1);
        GXSetNumChans(1);
        GXClearVtxDesc();
        GXSetVtxDesc(9, 1);
        GXSetVtxDesc(11, 1);
        GXSetVtxDesc(13, 1);
        GXSetVtxAttrFmt(0, 9, 1, 4, 0);
        GXSetVtxAttrFmt(0, 11, 1, 5, 0);
        GXSetVtxAttrFmt(0, 13, 1, 4, 0);
        GXSetZMode(1, 3, 0);
        GXSetCullMode(0);
        GXSetAlphaCompare(7, 0, 0, 7, 0);
        GXSetTevOp(0, 0);
        GXSetBlendMode(1, 4, 5, 0);
        if (pTexDef) {
            GXLoadTexObj((_GXTexObj *)pTexDef->texBase, 0);
            if (pTexDef->ps2flags & 1) {
                GXSetTevOp(0, 0);
                GXSetBlendMode(1, 4, 5, 0);
            } else {
                GXSetTevOp(0, 0);
                GXSetBlendMode(0, 4, 5, 0);
            }
            if (pTexDef->ps2flags & 4) {
                GXSetAlphaCompare(4, 0x3c, 0, 3, 0xff);
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
        } else {
            GXSetTevOp(0, 4);
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
        GXSetViewport(ox, oy, ow, oh, on, of);
    }
}
}

/* 0x800a644c, size 0x20 */
void DC_SendBlit(tTexture *tex, int nNumVertex, KMVERTICES *Vertices, unsigned int color)
{
{
    NGCDC_SendBlit(tex, nNumVertex, Vertices, color);
}
}

/* 0x800a646c, size 0x504 */
void DC_SendFillRect(int ulx, int uly, int lrx, int lry, int red, int green, int blue, int alpha, float z) /* z: unused 9th arg that callers pass (overrides.json) */
{
    int i; /* r0 */
    float Vert_f32[12]; /* stack */
    unsigned long Colors_u32[4]; /* stack */
    float TexCoords_f32[8]; /* stack */
    float vp[6]; /* stack */

{
    int i; /* r0 */
    float Vert_f32[12] ATTRIBUTE_ALIGN(32) = { 0.0f, 480.0f, 0.0f, 640.0f, 480.0f, 0.0f, 640.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }; /* stack */
    unsigned long Colors_u32[4] ATTRIBUTE_ALIGN(32) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff }; /* stack */
    float TexCoords_f32[8] ATTRIBUTE_ALIGN(32) = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f }; /* stack */
    float vp[6] ATTRIBUTE_ALIGN(32); /* stack */

    ngcBlitInit();
    Vert_f32[0] = ulx;
    Vert_f32[1] = 448 - uly;
    Vert_f32[2] = 0.0f;
    Vert_f32[3] = ulx;
    Vert_f32[4] = 448 - lry;
    Vert_f32[5] = 0.0f;
    Vert_f32[6] = lrx;
    Vert_f32[7] = 448 - lry;
    Vert_f32[8] = 0.0f;
    Vert_f32[9] = lrx;
    Vert_f32[10] = 448 - uly;
    Vert_f32[11] = 0.0f;
    for (i = 0; i < 4; i++) {
        Colors_u32[i] = (red << 24) + (green << 16) + (blue << 8) + alpha;
    }
    GXGetViewportv(vp);
    {
        float ox = vp[0];
        float oy = vp[1];
        float ow = vp[2];
        float oh = vp[3];
        float on = vp[4];
        float of = vp[5];

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
        GXSetCullMode(0);
        GXSetAlphaCompare(7, 0, 0, 7, 0);
        GXSetTevOp(0, 4);
        GXSetBlendMode(1, 4, 5, 0);
        GXBegin(0x80, 0, 4);
        for (i = 0; i < 4; i++) {
            GXPosition3f32(Vert_f32[i * 3], Vert_f32[i * 3 + 1], Vert_f32[i * 3 + 2]);
            GXColor1u32(Colors_u32[i]);
            GXTexCoord2f32(TexCoords_f32[i * 2], TexCoords_f32[i * 2 + 1]);
        }
        GXEnd();
        GXSetViewport(ox, oy, ow, oh, on, of);
        GXSetScissor(ox, oy, ow, oh);
    }
}
}

/* 0x800a6970, size 0x4 */
void LoadLights()
{
}

/* 0x800a6974, size 0x36c */

static void Blits_First(void)
{
    anon_palette_texture_20 *b; /* r26 */
    tTexture *tex; /* r25 */
    int h; /* r25 */
    int w; /* r27 */
    int hh; /* r0 */
    int ww; /* r0 */
    int x; /* r28 */
    int y; /* r30 */
    int vflip; /* r0 */
    int i; /* r24 */

{
    anon_palette_texture_20 *b;
    tTexture *tex;
    int h;
    int w;
    int hh;
    int ww;
    int x;
    int y;
    int vflip;
    int i;
    int d[2];

    for (i = 0, b = gBlitInstanceList; i < gNumBlitInsts; i++, b++)
    {
        tex = b->texture;
        if (tex && !(b->flags & 2)) continue;
        if (b->flags & 0x10)
            continue;
        if (b->flags & 0x100)
            continue;
        if (b->hide)
            continue;
        if (b->flags & 0x80)
            continue;
        if (b->z)
            BlitZDist(b->z);
        else
            BlitClearAttribute(0x10);
        if (b->flags & 4)
            BlitSetAttribute(4);
        else
            BlitClearAttribute(4);
        if (!b->width || !b->height) continue;
        ww = b->width;
        hh = b->height;
        if (!tex)
        {
            if (b->alpha == 0xff)
                BlitClearAttribute(0x20);
            else
            {
                BlitXLU(b->alpha);
                BlitSetAttribute(0x20);
            }
            x = b->x * 2;
            y = b->y * 2.0f;
            w = (b->x + b->width) * 2;
            h = (b->y + b->height) * 2.0f;
            BlitScissor(x, y, w - 1, h - 1);
            BlitRect(x, y, w, h, (unsigned char *)b->palette);
            continue;
        }
        x = b->x * 2;
        y = b->y * 2.0f;
        BlitScissor(x, y, x + ww - 1, y + hh - 1);
        ww = b->right - b->left;
        hh = b->bot - b->top;
        w = b->right - b->left;
        h = b->bot - b->top;
        if (b->alpha == 0xff)
            BlitClearAttribute(0x20);
        else
            BlitXLU(b->alpha);
        if (b->palette)
        {
            BlitColors(b->palette->palBase, tex->format, tex->size, 0);
        }
        else if (tex->palIndex >= 0)
        {
            BlitColors(gPalDefPtrs[b->texIndex >> 10].palDefs[tex->palIndex].palBase, tex->format, tex->size, 0);
        }
        else
        {
            gWhite.c[3] = b->alpha;
            BlitColors(&gWhite, -1, -1, 0);
        }
        if (b->bot >= tex->height)
            b->bot = tex->height - 1;

        if (b->flags & 8)
        {
            BlitSetAttribute(8);
            vflip = 1;
        }
        else
        {
            BlitClearAttribute(8);
            vflip = 0;
        }

        if (vflip)
        {
            d[1] = hh; d[0] = ww; if (!vflip) { d[0] = w; d[1] = h; } BlitDraw(b->texture, x, y, x + d[0], y + d[1], b->left, tex->height - b->bot - 1);
        }
        else
        {
            d[1] = hh; d[0] = ww; if (!vflip) { d[0] = w; d[1] = h; } BlitDraw(b->texture, x, y, x + d[0], y + d[1], b->left, b->top);
        }
    }
}
}

/* 0x800a6ce0, size 0x3a8 */

static void Blits(void)
{
    anon_palette_texture_20 *b; /* r25 */
    tTexture *tex; /* r0 */
    int h; /* r26 */
    int w; /* r27 */
    int hh; /* r0 */
    int ww; /* r0 */
    int x; /* r28 */
    int y; /* r30 */
    int vflip; /* r23 */
    int i; /* r24 */

{
    anon_palette_texture_20 *b;
    tTexture *tex;
    int h;
    int w;
    int hh;
    int ww;
    int x;
    int y;
    int vflip;
    int i;
    int d[2];

    for (i = 0, b = gBlitInstanceList; i < gNumBlitInsts; i++, b++)
    {
        tex = b->texture;
        if (!tex && !(b->flags & 0x10)) continue; tex = b->texture;
        if (b->flags & 0x100)
            continue;
        if (b->flags & 2)
            continue;
        if (b->hide)
            continue;
        if (b->flags & 0x80)
        {
            if (tex)
                ((void (*)(anon_name_start_18 *))tex)(b->palette);
            continue;
        }
        if (b->z)
            BlitZDist(b->z);
        else
            BlitClearAttribute(0x10);
        if (b->flags & 4)
            BlitSetAttribute(4);
        else
            BlitClearAttribute(4);
        if (!b->width || !b->height) continue;
        ww = b->width;
        hh = b->height;
        if (!tex)
        {
            if (b->alpha == 0xff)
                BlitClearAttribute(0x20);
            else
            {
                BlitXLU(b->alpha);
                BlitSetAttribute(0x20);
            }
            x = b->x * 2;
            y = b->y * 2.0f;
            w = (b->x + b->width) * 2;
            h = (b->y + b->height) * 2.0f;
            BlitScissor(x, y, w - 1, h - 1);
            BlitRect(x, y, w, h, (unsigned char *)b->palette);
            continue;
        }
        x = b->x * 2;
        y = b->y * 2.0f;
        BlitScissor(x, y, x + ww - 1, y + hh - 1);
        ww = b->right - b->left;
        hh = b->bot - b->top;
        w = b->right - b->left;
        h = b->bot - b->top;
        if (b->alpha == 0xff)
            BlitClearAttribute(0x20);
        else
            BlitXLU(b->alpha);
        if (b->palette)
        {
            BlitColors(b->palette->palBase, tex->format, tex->size, 0);
        }
        else if (tex->palIndex >= 0)
        {
            BlitColors(gPalDefPtrs[b->texIndex >> 10].palDefs[tex->palIndex].palBase, tex->format, tex->size, 0);
        }
        else
        {
            gWhite.c[3] = b->alpha;
            BlitColors(&gWhite, -1, -1, 0);
        }
        if (b->bot >= tex->height)
            b->bot = tex->height - 1;

        if (b->flags & 8)
        {
            BlitSetAttribute(8);
            vflip = 1;
        }
        else
        {
            BlitClearAttribute(8);
            vflip = 0;
        }
        if (b->flags & 0x4000)
            BlitColors(&gYellow, -1, -1, 0);

        if (vflip)
        {
            d[1] = hh; d[0] = ww; if (!vflip) { d[0] = w; d[1] = h; } BlitDraw(b->texture, x, y, x + d[0], y + d[1], b->left, tex->height - b->bot - 1);
        }
        else
        {
            d[1] = hh; d[0] = ww; if (!vflip) { d[0] = w; d[1] = h; } BlitDraw(b->texture, x, y, x + d[0], y + d[1], b->left, b->top);
        }
    }
}
}

/* 0x800a7088, size 0xbc */
void WindowBorders(void)
{
    if (drawWindowBorders) {
        if (gSplitScreen == 1) {
            DC_SendFillRect(0, 0xdf, 0x280, 0xe1, 0, 0, 0, 0xff, 1.0f);
        } else if (gSplitScreen == 2) {
            DC_SendFillRect(0, 0xdf, 0x280, 0xe1, 0, 0, 0, 0xff, 1.0f);
            DC_SendFillRect(0x13f, 0, 0x141, 0x1c0, 0, 0, 0, 0xff, 1.0f);
        }
    }
}

/* The original has 26 unreferenced static functions here (Clip, ClipTri0_1..ClipTri1_3,
 * DrawObject; DWARF `// Erased`). CodeWarrior compiles them (the linker strips the code) and
 * numbers their float literals before PS2DrawObjectWithEnvMap's, so the .sdata2 pool holds the
 * doubles 1.0 and 0.5 and the unsigned int->float constant before 1/(2*pi) although live code
 * first uses them in Polys and Traverse. This placeholder reproduces only those three literal
 * references, in that order. */
static void Clip(void)
{
    cos(1.0);
    cos(0.5);
    HAL_SetTextTranslucency((float)gFrames);
}

/* 0x800a7144, size 0x2a8 */
void PS2DrawObjectWithEnvMap(unsigned int *dlist, int normals, int texgen, float (*mat)[3], float *cam, float *north, float ouls, float oult)
{
    float texs; /* f29 */
    float text; /* f28 */
    float texgenuvs[3][3]; /* stack */
    tPackedObject *obj; /* r0 */
    int i; /* r6 */
    int j; /* r7 */
    float vec[3]; /* stack */
    float new[3]; /* stack */
    obj = (tPackedObject *)dlist;
    if (normals && numLights > 0) {
        MtxTranspose3x3();
        MtxMulVec3(camNormals[0], bodyLights.l[0].dir);
        if (numLights > 1) {
            MtxMulVec3(camNormals[1], bodyLights.l[1].dir);
        }
        MtxTranspose3x3();
    }
    if (texgen) {
        float dx = mat[3][0] - cam[0];
        float dy = mat[3][1] - cam[1];
        float dz = mat[3][2] - cam[2];
        vec[0] = dx * north[2] - dz * north[0];
        vec[1] = dy;
        vec[2] = dz * north[2] + dx * north[0];
        BodyVector(vec, new, mat);
        NormalVector(vec);
        LookInDir(new, (tUVMatrix *)texgenuvs);
        {
            float zero = 0.0f; /* a local zero: `0.0f - x*k` folds to a negate, the target's fnmsub needs a freshly loaded 0.0f */
            texs = zero - fatan2(vec[0], vec[2]) * 0.15915493866300567;
            text = zero - fasin(vec[1]) * 0.31830987732601135;
        }
    }
    Fake_EnvMap_Normal1[0] = texgenuvs[0][0];
    Fake_EnvMap_Normal1[1] = texgenuvs[0][1];
    Fake_EnvMap_Normal1[2] = texgenuvs[0][2];
    Fake_EnvMap_Normal1[3] = 0.0f;
    Fake_EnvMap_Normal2[0] = texgenuvs[1][0];
    Fake_EnvMap_Normal2[1] = texgenuvs[1][1];
    Fake_EnvMap_Normal2[2] = texgenuvs[1][2];
    Fake_EnvMap_Normal2[3] = 0.0f;
    Fake_EnvMap_uu = 0.5f + texs + ouls;
    Fake_EnvMap_vv = 0.5f + text + oult;
    for (i = 0; i < obj->nStrips; i++) {
        for (j = 0; j < obj->strips[i].nVertex; j++) {
            {
                float n0 = obj->strips[i].vertexs[j].nom[0];
                float n1 = obj->strips[i].vertexs[j].nom[1];
                float n2 = obj->strips[i].vertexs[j].nom[2];
                obj->strips[i].vertexs[j].uv[0] = Fake_EnvMap_uu + 0.25f * (n0 * texgenuvs[0][0] + n1 * texgenuvs[0][1] + n2 * texgenuvs[0][2]);
            }
            {
                float n0 = obj->strips[i].vertexs[j].nom[0];
                float n1 = obj->strips[i].vertexs[j].nom[1];
                float n2 = obj->strips[i].vertexs[j].nom[2];
                obj->strips[i].vertexs[j].uv[1] = Fake_EnvMap_vv + 0.25f * (n0 * texgenuvs[1][0] + n1 * texgenuvs[1][1] + n2 * texgenuvs[1][2]);
            }
        }
    }
}

/* 0x800a73ec, size 0x57c */
void Polys(int winidx, int delay)
{
    char text[64]; /* stack */
    anon_x_y_28 gVlistp[4]; /* stack */
    tPoly *poly; /* r25 */
    int *pdw; /* r21 */
    int polyMatrix; /* r24 */
    int i; /* r23 */
    int j; /* r17 */
    tTexture *tex; /* r20 */
    float x; /* stack */
    float y; /* stack */
    float z; /* stack */
    int needclip; /* r16 */
    float vec[3]; /* stack */
    float invW; /* f5 */
    poly = PolyInstanceList;
    polyMatrix = 0;
    OCR_F(0x6c) = 1.0f;
    for (i = 0; i < gNumPolyInsts; i++, poly++) {
        if (delay == 0) {
            if ((poly->flags & 0x40) || (poly->flags & 0x800)) {
                continue;
            }
        } else if (delay == 2) {
            if (!(poly->flags & 0x800)) {
                continue;
            }
        } else {
            if (!(poly->flags & 0x40)) {
                continue;
            }
        }
        if (poly->flags & 0x8000) {
            continue;
        }
        if (!(poly->flags & (1 << winidx))) {
            continue;
        }
        if (poly->nverts == 0) {
            continue;
        }
        if (poly->flags & 0x4000) {
            if (poly->flags & 0x2000) {
                DC_SendFillRect(poly->verts[0].ob[0], poly->verts[0].ob[1], poly->verts[1].ob[0], poly->verts[1].ob[1],
                                poly->verts[0].cn[0], poly->verts[0].cn[1], poly->verts[0].cn[2], poly->verts[0].cn[3],
                                poly->z * 0.0001f);
            } else {
                DC_SendFillRect(poly->verts[0].ob[0], poly->verts[0].ob[1], poly->verts[1].ob[0], poly->verts[1].ob[1],
                                poly->verts[0].cn[0], poly->verts[0].cn[1], poly->verts[0].cn[2], poly->verts[0].cn[3],
                                1.0f / poly->verts[0].ob[2]);
            }
            continue;
        }
        if (poly->flags & 0x8000) {
            if (polyMatrix == 0) {
                push();
            } else if (polyMatrix > 0) {
                poppush();
            }
            polyMatrix = -1;
            MtxMulMatTrans(gFrontFacing[0], gCamera[winidx].mat.mat3.pos);
        } else if (polyMatrix) {
            polyMatrix = 0;
            pop();
        }
        if (poly->flags & 0x400) {
            tex = &gTexDefPtrs[poly->texidx >> 10].texDefs[poly->texidx & 0x3ff];
        } else {
            tex = NULL;
        }
        stripsDrawn++;
        trisDrawn += poly->nverts - 2;
        vertsDrawn += poly->nverts;
        use_z_value = 0; /* written here, after the counters, though stored first: Polys only matches with this placement */
        needclip = 0;
        for (j = 0; j < poly->nverts; j++) {
            if (!(poly->flags & 0x10)) {
                vec[0] = poly->verts[j].ob[0] - gCamera[winidx].mat.mat3.pos[0];
                vec[1] = poly->verts[j].ob[1] - gCamera[winidx].mat.mat3.pos[1];
                vec[2] = poly->verts[j].ob[2] - gCamera[winidx].mat.mat3.pos[2];
                MtxMulVec4(vec, &gVlistp[j].x);
                use_z_value = 1;
                WorldToScreen(poly->verts[j].ob, &x, &y, &z);
                if (z < 0.0f) {
                    needclip = 1;
                }
            } else {
                gVlistp[j].x = poly->verts[j].ob[0];
                gVlistp[j].y = poly->verts[j].ob[1];
                gVlistp[j].z = poly->verts[j].ob[2];
                if (gUpsideDownTrack && gMirrorWaves) {
                    gVlistp[j].y = -gVlistp[j].y;
                } else if (gUpsideDownTrack) {
                    gVlistp[j].y = -gVlistp[j].y;
                    gVlistp[j].x = -gVlistp[j].x;
                } else if (gMirrorWaves) {
                    gVlistp[j].x = -gVlistp[j].x;
                }
            }
            if (gVlistp[j].z < OCR_F(0xc)) {
                gVlistp[j].clip = 4;
                needclip = 1;
            } else {
                invW = 1.0 / gVlistp[j].z;
                if (poly->flags & 0x2000) {
                    gVlistp[j].sz = poly->z * 0.0001;
                } else {
                    gVlistp[j].sz = invW;
                }
                gVlistp[j].sx = OCR_F(0) * (invW * gVlistp[j].x) + OCR_F(4);
                gVlistp[j].sy = OCR_F(0) * (-invW * gVlistp[j].y) + OCR_F(8);
                gVlistp[j].clip = 0;
                if (use_z_value) {
                    gVlistp[j].sx = x + ZoidWinInfo[winidx].whcenter;
                    gVlistp[j].sy = y + ZoidWinInfo[winidx].wvcenter;
                    if (poly->flags & 0x100) {
                        gVlistp[j].sz = g_ZMax2D;
                    } else {
                        gVlistp[j].sz = z;
                    }
                }
            }
            gVlistp[j].U = poly->verts[j].tc[0];
            gVlistp[j].V = poly->verts[j].tc[1];
            gVlistp[j].clr.dwPacked = *(unsigned int *)poly->verts[j].cn;
        }
        if (!needclip) {
            if (poly->flags & 0x1000) {
                pdw = DrawFace(tex, pdw, gVlistp, poly->nverts, frontFace);
                pdw = DrawFace(tex, pdw, gVlistp, poly->nverts, backFace);
            } else {
                pdw = DrawFace(tex, pdw, gVlistp, poly->nverts, frontFace);
            }
        }
    }
    if (polyMatrix) {
        pop();
    }
    if (gNumPolyInsts > 0) {
        sprintf(text, "Polys called: inst:%d\n", gNumPolyInsts);
    }
}

/* 0x800a7968, size 0x44 */
void push(void)
{
    if (ms_idx < 10) {
        MtxSave(&matrix_stack[ms_idx++]);
    }
}

/* 0x800a79ac, size 0x44 */
void pop(void)
{
    if (ms_idx) {
        MtxLoad(&matrix_stack[--ms_idx]);
    }
}

/* 0x800a79f0, size 0x2c */
void poppush(void)
{
    pop();
    ms_idx++;
}

/* 0x800a7a1c, size 0xa08 */
void Traverse(anon_flags_culling_60 *parent, anon_flags_culling_60 *hier, int winidx, int lvl, int useZDist, float gZDist)
{
    float dx, dy, dz;
    tModelDef *ptr; /* r15 */
    float pos[3]; /* stack */
    float newmtx[3]; /* stack */
    float *mat; /* r28 */
    float r; /* f0 */
    float r1; /* f3 */
    float sx; /* f1 */
    float sy; /* f5 */
    float z; /* f5 */
    float rat; /* f5 */
    int cull; /* r31 */
    int lod; /* r30 */
    int pushed; /* r29 */
    float newMat[4][3]; /* stack */
    float objz; /* f28 */
    float uls; /* f27 */
    float ult; /* f26 */
    tTexture *texid; /* r28 */
    int modelindex; /* r28 */
    int wave; /* r0 */
    int count; /* r27 */
    int i; /* r0 */
    static float fardis = 6000.0f;
    static int debug = 0;
    static int choosen = 0;
    count = 0;
    pushed = 0;
    while (hier) {
        modelindex = hier->index >> 10;
        ptr = &gRomObjDefPtrs[modelindex].romObjDefs[hier->index & 0x3ff];
        if (strstr(ptr->name, "TRACK2L25705")) {
            hier->mat->mat3.pos[0] = 3600.0f;
            hier->mat->mat3.pos[1] = 256.0f;
            hier->mat->mat3.pos[2] = -3392.0f;
            ptr->radius = 432.817f;
        }
        if (strstr(ptr->name, "TRACK2L25703")) {
            hier->mat->mat3.pos[0] = 3648.0f;
            hier->mat->mat3.pos[1] = 288.0f;
            hier->mat->mat3.pos[2] = -3472.0f;
            ptr->radius = 447.805f;
        }
        strstr(ptr->name, "CAR3");
        if (hier->flags & 0x80000000) {
            goto next;
        }
        if (hier->mat == NULL) {
            goto next;
        }
        if ((hier->flags & (0x100 << winidx)) && hier->child == -1) {
            goto next;
        }
        if (lvl == 0) {
            pos[0] = hier->mat->mat3.pos[0] - gCamera[winidx].mat.mat3.pos[0];
            pos[1] = hier->mat->mat3.pos[1] - gCamera[winidx].mat.mat3.pos[1];
            pos[2] = hier->mat->mat3.pos[2] - gCamera[winidx].mat.mat3.pos[2];
        } else {
            pos[0] = hier->mat->mat3.pos[0];
            pos[1] = hier->mat->mat3.pos[1];
            pos[2] = hier->mat->mat3.pos[2];
        }
        MtxMulVec4(pos, newmtx);
        if (lvl == 0) {
            newmtx[0] = hier->mat->mat3.pos[0];
            newmtx[1] = hier->mat->mat3.pos[1];
            newmtx[2] = hier->mat->mat3.pos[2];
        } else {
            LocalToWorld(hier->mat->mat3.pos, newmtx);
        }
        pos[0] = newmtx[0] - gCamera[winidx].mat.mat3.pos[0];
        pos[1] = newmtx[1] - gCamera[winidx].mat.mat3.pos[1];
        pos[2] = newmtx[2] - gCamera[winidx].mat.mat3.pos[2];
        MyMtxMulVec4(gCamera[winidx].mat.uvs, pos, newmtx);
        r = ptr->radius * hier->scale;
        objz = newmtx[2];
        if (objz + r <= OCR_F(0xc)) {
            goto next;
        }
        if (objz - r > OCR_F(0x20)) {
            goto next;
        }
        if (objz - r >= 500.0f && objz + r <= fardis) {
            rat = OCR_F(0) / objz;
            r1 = rat * r;
            sx = rat * newmtx[0];
            sy = rat * newmtx[1];
            if (sx - r1 > -1024.0f && sx + r1 < 1024.0f && sy - r1 > -1024.0f && sy + r1 < 1024.0f) {
                cull = 0;
                count++;
                if (debug && choosen == count) {
                    goto next;
                }
            } else {
                cull = -1;
            }
        } else {
            cull = -1;
        }
        r1 = OCR_F(0x24) * r;
        z = objz * OCR_F(0x2c);
        if (newmtx[0] + r1 <= -z) {
            goto next;
        }
        if (newmtx[0] - r1 >= z) {
            goto next;
        }
        r1 = OCR_F(0x28) * r;
        z = objz * OCR_F(0x30);
        if (newmtx[1] + r1 <= -z) {
            goto next;
        }
        if (newmtx[1] - r1 >= z) {
            goto next;
        }
        uls = ult = 0.0f;
        wave = GetWaveByIndex(modelindex);
        isdrawingtrack = 0;
        if (wave >= 0xe9) {
            isdrawingtrack = 1;
            objz += 20000.0f;
        }
        lod = ptr->lodCount - 1;
        if (hier->flags & 4) {
            lod = hier->flags & 3;
            if (lod == 0) {
                gZDist = 0.0f;
            } else {
                gZDist = ptr->lod[lod - 1].lodRad * hier->scale;
            }
        } else if ((ptr->lod[lod].lodRad != 0.0f && !useZDist) || (hier->flags & 8)) {
            if (lvl <= 1) {
                if (lvl > 0) {
                    dx = parent->mat->mat3.pos[0] + hier->mat->mat3.pos[0];
                    dy = parent->mat->mat3.pos[1] + hier->mat->mat3.pos[1];
                    dz = parent->mat->mat3.pos[2] + hier->mat->mat3.pos[2];
                    dx -= gCamera[winidx].mat.mat3.pos[0];
                    dy -= gCamera[winidx].mat.mat3.pos[1];
                    dz -= gCamera[winidx].mat.mat3.pos[2];
                } else {
                    dx = hier->mat->mat3.pos[0] - gCamera[winidx].mat.mat3.pos[0];
                    dy = hier->mat->mat3.pos[1] - gCamera[winidx].mat.mat3.pos[1];
                    dz = hier->mat->mat3.pos[2] - gCamera[winidx].mat.mat3.pos[2];
                }
            }
            gZDist = sqrtf(dx * dx + dy * dy + dz * dz);
        }
        if (ptr->lod[lod].lodRad != 0.0f && gZDist > ptr->lod[lod].lodRad * hier->scale) {
            goto next;
        }
        if (lod > 0) {
            while (lod > 0 && gZDist < ptr->lod[lod - 1].lodRad * hier->scale) {
                lod--;
            }
        }
        if (hier->numLights >= 0) {
            LoadLights(hier->lights, hier->numLights);
        }
        if (hier->flags & 0x10000) {
            if (hier->scale != 1.0f) {
                mat = newMat[0];
                for (i = 0; i < 9; i++) {
                    newMat[0][i] = hier->scale * gCameraFacing[0][i];
                }
                newMat[3][0] = hier->mat->mat3.pos[0];
                newMat[3][1] = hier->mat->mat3.pos[1];
                newMat[3][2] = hier->mat->mat3.pos[2];
            } else {
                gCameraFacing[3][0] = hier->mat->mat3.pos[0];
                gCameraFacing[3][1] = hier->mat->mat3.pos[1];
                gCameraFacing[3][2] = hier->mat->mat3.pos[2];
                mat = gCameraFacing[0];
            }
        } else if (hier->flags & 0x8000) {
            gFrontFacing[3][0] = hier->mat->mat3.pos[0];
            gFrontFacing[3][1] = hier->mat->mat3.pos[1];
            gFrontFacing[3][2] = hier->mat->mat3.pos[2];
            mat = gFrontFacing[0];
        } else {
            mat = (float *)hier->mat;
        }
        if (!(hier->flags & 0x80)) {
            push();
            pushed = 1;
            if (lvl == 0) {
                MtxMulMatTrans(mat, pos);
            } else {
                MtxMulMat(mat);
            }
        }
        if (!(hier->flags & (0x100 << winidx))) {
            MySetObjectPosition(objz);
            MySetAmbientColor(hier->color);
            texid = NULL;
            if (ptr->lod[lod].flags & 1) {
                if (hier->texture[lod] != NULL) {
                    if (hier->texture[lod]->xflags & 0x20000) {
                        uls = hier->ouls;
                        ult = hier->oult;
                    }
                    texid = hier->texture[lod];
                }
            }
            OCR_F(0x6c) = hier->w;
            if (hier->flags & 0x2000) {
                *(unsigned int *)&OCRamData[0x70] = hier->color.rgba;
            } else {
                *(unsigned int *)&OCRamData[0x70] = -1;
            }
            if (hier->flags & 0x200000) {
                if (gZDist > 24.0f) {
                    if (gZDist > 80.0f) {
                        ((unsigned char *)OCRamData)[0x73] = 0;
                    } else {
                        ((unsigned char *)OCRamData)[0x73] = ((unsigned char *)OCRamData)[0x73] * (80.0f - gZDist) / 56.0f;
                    }
                }
            }
            if (ptr->lod[lod].flags & 0x10) {
                PS2DrawObjectWithEnvMap(ptr->lod[lod].displayList, ptr->lod[lod].flags & 0x10, hier->flags & 0x80000,
                                        parent->mat->uvs, gCamera[winidx].mat.uvs[3], hier->look, uls, ult); /* spelled .mat.uvs[3], not .mat3.pos: only this spelling stops the three hoisted camera registers rotating */
                MyDrawObjectWithEnvmap(ptr->lod[lod].displayList, texid, -1);
            } else if (cull < 0) {
                MyDrawObject(ptr->lod[lod].displayList, texid, cull, hier->color.rgba);
            } else {
                MyDrawObject(ptr->lod[lod].displayList, texid, cull, hier->color.rgba);
                DrawNoClip(ptr->lod[lod].displayList, 0, pdwCurrentPtr, ListType);
            }
            objsDrawn++;
        }
        if (hier->child != -1) {
            if (!pushed) {
                push();
            }
            Traverse(hier, &ObjInstanceList[hier->child], winidx, lvl + 1, (hier->flags & 8) | useZDist, gZDist);
            pushed = 0;
            pop();
        } else if (pushed) {
            pushed = 0;
            pop();
        }
    next:
        if (hier->next == -1) {
            break;
        }
        hier = &ObjInstanceList[hier->next];
        if (lvl == 0) {
            gZDist = 0.0f;
        }
    }
}

/* 0x800a8424, size 0x428 */
void ZOID_EndFrame(void)
{
    int winidx; /* r25 */
    tWindowInfo *win; /* r24 */
    int numwin; /* r25 */

{
    int winidx; /* r25 */
    tWindowInfo *win; /* r24 */
#define OC ((float *)OCRamData)

    transformTime = MyGetTime();
    if (gMirrorWaves != gLastMirrorWaves) {
        MirrorTextures();
        gLastMirrorWaves = gMirrorWaves;
    }
    PS2CameraStart();
    for (winidx = 0, win = (tWindowInfo *)(int)ZoidWinInfo; winidx < gNumWindows; winidx++, win++) {
        PS2SetRenderTarget(win);
        MySetCameraPos(gCamera[winidx].mat.uvs);
        PS2UpdateCameraMatrix(win);
        ps2_screen_center_x = win->whcenter;
        ps2_screen_center_y = win->wvcenter;
        WorldCullObjects(winidx, gCamera[winidx].mat.uvs[3], 0);
        OC[0] = 0.5f * win->wwidth / win->whvangtan;
        OC[1] = win->whcenter;
        OC[2] = win->wvcenter;
        OC[3] = win->frontclip;
        OC[4] = 1.0 / win->frontclip;
        OC[5] = OC[4] * OC[0];
        OC[8] = win->backclip > 2000.0f ? win->backclip : 2000.0f;
        OC[9] = 1.0 / (float)cos(0.5f * win->whviewang);
        OC[10] = 1.0 / (float)cos(0.5f * win->wvviewang);
        OC[11] = win->whvangtan;
        OC[12] = win->wvvangtan;
        CopyUV(gCamera[winidx].mat.uvs, gCameraFacing);
        gFrontFacing[2][0] = gCamera[winidx].mat.uvs[2][0];
        gFrontFacing[2][1] = gCamera[winidx].mat.uvs[2][1];
        gFrontFacing[2][2] = gCamera[winidx].mat.uvs[2][2];
        gFrontFacing[1][0] = unitmatrix[1][0];
        gFrontFacing[1][1] = unitmatrix[1][1];
        gFrontFacing[1][2] = unitmatrix[1][2];
        gFrontFacing[0][0] = gFrontFacing[1][1] * gFrontFacing[2][2] - gFrontFacing[1][2] * gFrontFacing[2][1];
        gFrontFacing[0][1] = gFrontFacing[1][2] * gFrontFacing[2][0] - gFrontFacing[1][0] * gFrontFacing[2][2];
        gFrontFacing[0][2] = gFrontFacing[1][0] * gFrontFacing[2][1] - gFrontFacing[1][1] * gFrontFacing[2][0];
        NormalVector(gFrontFacing[0]);
        gFrontFacing[2][0] = gFrontFacing[0][1] * gFrontFacing[1][2] - gFrontFacing[0][2] * gFrontFacing[1][1];
        gFrontFacing[2][1] = gFrontFacing[0][2] * gFrontFacing[1][0] - gFrontFacing[0][0] * gFrontFacing[1][2];
        gFrontFacing[2][2] = gFrontFacing[0][0] * gFrontFacing[1][1] - gFrontFacing[0][1] * gFrontFacing[1][0];
        if (gMirrorWaves && gUpsideDownTrack) {
            CamMtxLoadUpsideDownMirror((float *)gCamera[winidx].mat.uvs);
        } else if (gUpsideDownTrack) {
            CamMtxLoadUpsideDown((float *)gCamera[winidx].mat.uvs);
        } else if (gMirrorWaves) {
            CamMtxLoadMirror((float *)gCamera[winidx].mat.uvs);
        } else {
            CamMtxLoad((float *)gCamera[winidx].mat.uvs);
        }
        LoadLights(gLights, gNumLights);
        if (ObjHead != -1) {
            statsClock(7);
            SendCarShadows();
            Traverse(0, &ObjInstanceList[ObjHead], winidx, 0, 0, 0.0f);
            FlushSortedTex();
            statsUnclock(7);
        }
        PreDrawSortedTex();
        DrawSortedTex();
        Polys(winidx, 0);
        PreDrawAlphaList2();
        DrawAlphaList2();
        Polys(winidx, 2);
        PreDrawAlphaList();
        DrawAlphaList();
        ClearAlphaList();
    }
    ClearCarShadows();
    WindowBorders();
    {
        int numwin; /* r25 */
        numwin = gNumWindows;
        if (gNumWindows == 3)
            numwin = 4;
        for (winidx = 0, win = (tWindowInfo *)(int)ZoidWinInfo; winidx < numwin; winidx++, win++) {
            PS2SetRenderTarget(win);
            ps2_screen_center_x = win->whcenter;
            ps2_screen_center_y = win->wvcenter;
            UpdateHUDPostCamera2();
            Blits_First();
            Blits();
        }
    }
    {
        int numwin; /* r25 */
        numwin = gNumWindows;
        if (gNumWindows == 3)
            numwin = 4;
        for (winidx = 0; winidx < numwin; winidx++) {
            Polys(winidx, 1);
        }
    }
    ShowSystemInfo();
    HAL_FinishText(1);
    CheckVertexBufferSizes(0);
    transformTime = MyCountToMicro(0);
    if (transformTime > 40000)
        transformTime = 0;
    if (transformTimeMax < transformTime)
        transformTimeMax = transformTime;
#undef OC
}
}

/* 0x800a884c, size 0xc0 */
void Fake_EndFrame(void)
{
    int winidx; /* r30 */
    tWindowInfo *win; /* r29 */

{
    int winidx; /* r30 */
    tWindowInfo *win; /* r29 */
    transformTime = MyGetTime();
    PS2CameraStart();
    for (winidx = 0, win = ZoidWinInfo; winidx < gNumWindows; winidx++, win++) {
        PS2SetRenderTarget(win);
        MySetCameraPos(gCamera[winidx].mat.uvs);
        PS2UpdateCameraMatrix(win);
        ps2_screen_center_x = win->whcenter;
        ps2_screen_center_y = win->wvcenter;
        FlushSortedTex();
        DrawSortedTex();
        DrawAlphaList2();
        DrawAlphaList();
        ClearAlphaList();
    }
    HAL_FinishText(1);
}
}

/* 0x800a890c, size 0x4 */
void DrawNoClip()
{
}

