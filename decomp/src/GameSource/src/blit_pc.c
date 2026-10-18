/* blit_pc.c - 2D blitting state and quad submission (PC version of the blitter, used on NGC).
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\blit_pc.c; assembly: build/2049LE/asm/GameSource/src/blit_pc.s */
#include "rush/rush_protos.h"

anon_rgba_rgbaU32_4 blitPrimColor = { 255, 255, 255, 255 };
anon_rgba_rgbaU32_4 blitColor = { 255, 255, 255, 255 };
unsigned int blitAttribute;
int blitXmin;
int blitXmax;
int blitYmin;
int blitYmax;
unsigned short blitZDist;
int blitLUT;
int blitDLnum;
int blitMode;
void * blitColors;
static float gZValue;

/* 0x8000575c, size 0x68 */
void BlitInit(void)
{
    blitAttribute = 0;
    blitXmin = 0;
    blitYmin = 0;
    blitXmax = 639;
    blitYmax = 447;
    blitZDist = 0;
    blitPrimColor.rgba[3] = 0xff;
    blitDLnum++;
    if (blitDLnum >= 2) {
        blitDLnum = 0;
    }
    blitMode = -1;
    blitLUT = -1;
    blitColors = 0;
    gZValue = g_ZMax2D;
}

/* 0x800057c4, size 0x4 */
void BlitFinish(void)
{

}

/* 0x800057c8, size 0x1c */
void BlitSetAttribute(unsigned int attribute)
{
    if ((attribute & blitAttribute) != attribute) {
        blitAttribute |= attribute;
    }
}

/* 0x800057e4, size 0x18 */
void BlitClearAttribute(unsigned int attribute)
{
    if (blitAttribute & attribute) {
        blitAttribute &= ~attribute;
    }
}

/* 0x800057fc, size 0x28 */
void BlitZDist(unsigned short zDist)
{
    blitZDist = zDist;
    BlitSetAttribute(0x10);
}

/* 0x80005824, size 0x34 */
void BlitXLU(unsigned short xlu)
{
    blitPrimColor.rgba[3] = xlu;
    BlitSetAttribute(0x20);
    blitColors = 0;
}

/* 0x80005858, size 0x44 */
void BlitScissor(int ulx, int uly, int lrx, int lry)
{
    blitXmin = ulx < 0 ? 0 : ulx;
    blitYmin = uly < 0 ? 0 : uly;
    blitXmax = lrx > 639 ? 639 : lrx;
    blitYmax = lry > 447 ? 447 : lry;
}

/* 0x8000589c, size 0x24 */
void BlitClearColors(void)
{
    blitColors = 0;
    blitColor.rgba[0] = 0xff;
    blitColor.rgba[1] = 0xff;
    blitColor.rgba[2] = 0xff;
    blitColor.rgba[3] = 0xff;
}

/* 0x800058c0, size 0x28 */
void BlitColors(void * address)
{
    blitColor.rgba[0] = ((unsigned char *)address)[2];
    blitColor.rgba[1] = ((unsigned char *)address)[1];
    blitColor.rgba[2] = ((unsigned char *)address)[0];
    blitColor.rgba[3] = ((unsigned char *)address)[3];
}

/* 0x800058e8, size 0x428 */
void BlitDraw(tTexture *tex, int x0, int y0, int x1, int y1, int s, int t)
{
    KMVERTICES verts[4]; /* stack */
    float sf; /* f0 */
    float tf; /* f1 */
    float sf1; /* f11 */
    float tf1; /* f12 */
    if (x0 < blitXmin) {
        if (!(blitAttribute & 4)) {
            s += blitXmin - x0;
        }
        x0 = blitXmin;
    }
    if (y0 < blitYmin) {
        if (!(blitAttribute & 8)) {
            t += blitYmin - y0;
        }
        y0 = blitYmin;
    }
    if (x1 > blitXmax) {
        if (blitAttribute & 4) {
            s -= blitXmax - x1;
        }
        x1 = blitXmax;
    }
    if (y1 > blitYmax) {
        if (blitAttribute & 8) {
            t -= blitYmax - y1;
        }
        y1 = blitYmax;
    }
    if (x0 > x1 || y0 > y1) {
        return;
    }
    if (tex->flags & 0x20) {
        sf = (float)s / tex->width;
        tf = (float)t / tex->height;
        sf1 = (float)(s + x1 - x0) / tex->width;
        tf1 = (float)(t + y1 - y0) / tex->height;
    } else {
        sf = (0.5f + s) / tex->width;
        tf = (0.5f + t) / tex->height;
        sf1 = (0.5f + (s + x1 - x0)) / tex->width;
        tf1 = (0.5f + (t + y1 - y0)) / tex->height;
    }
    verts[3].fX = x0;
    verts[3].fY = y0;
    verts[2].fX = x1;
    verts[2].fY = y0;
    verts[1].fX = x1;
    verts[1].fY = y1;
    verts[0].fX = x0;
    verts[0].fY = y1;
    if (blitAttribute & 0x10) {
        verts[0].fZ = verts[1].fZ = verts[2].fZ = verts[3].fZ = 1.0f / blitZDist;
    } else {
        verts[0].fZ = verts[1].fZ = verts[2].fZ = verts[3].fZ = gZValue;
    }
    if (blitAttribute & 0x20) {
        blitColor.rgba[3] = blitPrimColor.rgba[3];
    }
    if (blitAttribute & 4) {
        verts[3].fU = sf1;
        verts[2].fU = sf;
        verts[1].fU = sf;
        verts[0].fU = sf1;
    } else {
        verts[3].fU = sf;
        verts[2].fU = sf1;
        verts[1].fU = sf1;
        verts[0].fU = sf;
    }
    if (blitAttribute & 8) {
        verts[3].fV = tf1;
        verts[2].fV = tf1;
        verts[1].fV = tf;
        verts[0].fV = tf;
    } else {
        verts[3].fV = tf;
        verts[2].fV = tf;
        verts[1].fV = tf1;
        verts[0].fV = tf1;
    }
    DC_SendBlit(tex, 4, verts, blitColor.rgbaU32);
    gZValue += 1.0f;
    if (gZValue > g_ZMax2D) {
        gZValue = g_ZMax2D;
    }
}

/* 0x80005d10, size 0xe8 */
void BlitRect(int x0, int y0, int x1, int y1, unsigned char *color)
{
    float z; /* f1 */
    z = gZValue;
    /* the test survives in the code; the branch was probably a 1.0f / blitZDist once */
    if (blitAttribute & 0x10) {
        z = gZValue;
    }
    if (x0 < blitXmin) {
        x0 = blitXmin;
    }
    if (y0 < blitYmin) {
        y0 = blitYmin;
    }
    if (x1 > blitXmax) {
        x1 = blitXmax;
    }
    if (y1 > blitYmax) {
        y1 = blitYmax;
    }
    if (x0 > x1 || y0 > y1) {
        return;
    }
    if (blitAttribute & 0x20) {
        DC_SendFillRect(x0, y0, x1 + 1, y1 + 1, color[0], color[1], color[2], blitPrimColor.rgba[3], z);
    } else {
        DC_SendFillRect(x0, y0, x1 + 1, y1 + 1, color[0], color[1], color[2], color[3], z);
    }
    gZValue += 1.0f;
    if (gZValue > g_ZMax2D) {
        gZValue = g_ZMax2D;
    }
}

