/* zoidpoly.c - flat polygon instance list for the zoid renderer.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\zoidpoly.c; assembly: build/2049LE/asm/GameSource/src/zoidpoly.s */
#include "rush/rush_protos.h"

int gNumPolyInsts;
int gMaxPolyInsts;
tPoly PolyInstanceList[200];

/* 0x80062a50, size 0xfc */
tPoly *ZOID_NewPoly(int nverts, float (*xyz)[3], unsigned short texidx, tRGBA *color, unsigned short flags, int useTidx)
{
    int i; /* r4 */
    tPoly *poly; /* r31 */
    for (i = 0; i < gNumPolyInsts; i++) {
        if (PolyInstanceList[i].nverts == 0) {
            break;
        }
    }
    if (i >= 200) {
        return 0;
    }
    if (i >= gNumPolyInsts) {
        gNumPolyInsts = i + 1;
    }
    if (gMaxPolyInsts < gNumPolyInsts) {
        gMaxPolyInsts = gNumPolyInsts;
    }
    poly = &PolyInstanceList[i];
    poly->flags = flags;
    if (useTidx) {
        poly->flags |= 0x400;
        poly->texidx = texidx;
    }
    ZOID_UpdatePoly(poly, nverts, xyz, texidx, color, 0, useTidx);
    return poly;
}

/* 0x80062b4c, size 0x21c */
void ZOID_UpdatePoly(tPoly *poly, int nverts, float (*xyz)[3], unsigned short texidx, tRGBA *color, unsigned short flags, int useTidx)
{
    tTexture *tex; /* r10 */
    int i; /* r11 */
    int ii; /* r12 */
    if (nverts == 0) {
        nverts = poly->nverts;
    } else {
        poly->nverts = nverts;
    }
    if (flags) {
        poly->flags = flags;
    }
    if (useTidx) {
        poly->flags |= 0x400;
        poly->texidx = texidx;
    }
    if (poly->flags & 0x400) {
        tex = &gTexDefPtrs[poly->texidx >> 10].texDefs[poly->texidx & 0x3ff];
    } else {
        tex = 0;
    }
    for (i = 0; i < nverts; i++) {
        if (tex) {
            ii = i;
            if (gMirrorWaves && (poly->flags & 0x80)) {
                ii = i + 4;
            }
            switch (ii) {
            case 0:
                poly->verts[i].tc[0] = 0.99f;
                poly->verts[i].tc[1] = 0.01f;
                break;
            case 1:
                poly->verts[i].tc[0] = 0.01f;
                poly->verts[i].tc[1] = 0.01f;
                break;
            case 2:
                poly->verts[i].tc[0] = 0.01f;
                poly->verts[i].tc[1] = 0.99f;
                break;
            case 3:
                poly->verts[i].tc[0] = 0.99f;
                poly->verts[i].tc[1] = 0.99f;
                break;
            case 4:
                poly->verts[i].tc[0] = 0.01f;
                poly->verts[i].tc[1] = 0.01f;
                break;
            case 5:
                poly->verts[i].tc[0] = 0.99f;
                poly->verts[i].tc[1] = 0.01f;
                break;
            case 6:
                poly->verts[i].tc[0] = 0.99f;
                poly->verts[i].tc[1] = 0.99f;
                break;
            case 7:
                poly->verts[i].tc[0] = 0.01f;
                poly->verts[i].tc[1] = 0.99f;
                break;
            case 8:
                poly->verts[i].tc[0] = 0.01f;
                poly->verts[i].tc[1] = 0.99f;
                break;
            case 9:
                poly->verts[i].tc[0] = 0.01f;
                poly->verts[i].tc[1] = 0.01f;
                break;
            case 10:
                poly->verts[i].tc[0] = 0.99f;
                poly->verts[i].tc[1] = 0.01f;
                break;
            case 11:
                poly->verts[i].tc[0] = 0.99f;
                poly->verts[i].tc[1] = 0.99f;
                break;
            }
        }
        if (xyz) {
            if (poly->flags & 0x4000) {
                poly->verts[i].ob[0] = xyz[i][0];
                poly->verts[i].ob[1] = xyz[i][1];
            } else {
                poly->verts[i].ob[0] = xyz[i][0];
                poly->verts[i].ob[1] = xyz[i][1];
                poly->verts[i].ob[2] = xyz[i][2];
            }
        }
        if (color) {
            poly->verts[i].cn[0] = color->c[2];
            poly->verts[i].cn[1] = color->c[1];
            poly->verts[i].cn[2] = color->c[0];
            poly->verts[i].cn[3] = color->c[3];
        }
    }
}

/* 0x80062d68, size 0x8 */
void ZOID_SetPolyZ(tPoly *poly, signed short z)
{
    poly->z = z;
}

/* 0x80062d70, size 0x10 */
void ZOID_HidePoly(tPoly *poly)
{
    poly->flags |= 0x8000;
}

/* 0x80062d80, size 0x10 */
void ZOID_ShowPoly(tPoly *poly)
{
    poly->flags &= ~0x8000;
}

/* 0x80062d90, size 0x10 */
void ZOID_ClearPolyFlags(tPoly *poly, unsigned short flags)
{
    poly->flags &= ~flags;
}

/* 0x80062da0, size 0x10 */
void ZOID_SetPolyFlags(tPoly *poly, unsigned short flags)
{
    poly->flags |= flags;
}

/* 0x80062db0, size 0x4c */
void ZOID_RemovePoly(tPoly *poly)
{
    poly->nverts = 0;
    if (poly == &PolyInstanceList[gNumPolyInsts - 1]) {
        do {
            gNumPolyInsts--;
            poly--;
        } while (poly->nverts == 0 && gNumPolyInsts > 0);
    }
}

