/* zoidwindow.c - viewport setup, fog, and screen projection. Status: matched.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\zoidwindow.c; assembly: build/2049LE/asm/GameSource/src/zoidwindow.s */
#include "rush/rush_protos.h"
/* inline angle-field initialiser (no DWARF entry): its argument temporaries preserve the original float registers in ZOID_SetWindow */
static inline void _WindowSetAngles(anon_vp_s_48 *winfo, float hang, float vang) {
    winfo->whviewang = hang;
    winfo->wvviewang = vang;
    winfo->whvangtan = ftan(winfo->whviewang / 2.0f);
    winfo->wvvangtan = ftan(winfo->wvviewang / 2.0f);
    winfo->wivvangtan = 0.5f / winfo->wvvangtan;
    winfo->wihvangtan = 0.5f / winfo->whvangtan;
}


static anon_vp_20 vp0 = { 1280.0f, 896.0f, 0.0f, 0.0f, 1280.0f, 896.0f, 0.0f, 0.0f };
static anon_vp_20 vp1 = { 960.0f, 448.0f, 0.0f, 0.0f, 960.0f, 448.0f, 0.0f, 0.0f };
static anon_vp_20 vp2 = { 960.0f, 448.0f, 0.0f, 0.0f, 1600.0f, 1344.0f, 0.0f, 0.0f };
static anon_vp_20 vpul = { 640.0f, 448.0f, 0.0f, 0.0f, 640.0f, 448.0f, 0.0f, 0.0f };
static anon_vp_20 vpur = { 640.0f, 448.0f, 0.0f, 0.0f, 1920.0f, 448.0f, 0.0f, 0.0f };
static anon_vp_20 vpll = { 640.0f, 448.0f, 0.0f, 0.0f, 640.0f, 1344.0f, 0.0f, 0.0f };
static anon_vp_20 vplr = { 640.0f, 448.0f, 0.0f, 0.0f, 1920.0f, 1344.0f, 0.0f, 0.0f };
float HFOV = 90.0f;
unsigned int gBgColor;
float gViewableDistance;
float gGlobalCameraFacingScale;
unsigned char gNumZoidWindows;
signed char gSplitScreen;
static anon_ulx_uly_8 scissor0;
static anon_ulx_uly_8 scissor1;
static anon_ulx_uly_8 scissor2;
static anon_ulx_uly_8 scissorul;
static anon_ulx_uly_8 scissorur;
static anon_ulx_uly_8 scissorll;
static anon_ulx_uly_8 scissorlr;
anon_vp_s_48 ZoidWinInfo[4];

/* 0x80062f34, size 0x84 */
void ZOID_ResetWindows(void)
{
    gNumZoidWindows = 1;
    gSplitScreen = 0;
    ZOID_InitializeWindow(0, &vp0, &scissor0, HFOV, 0.0f, 640.0f, 448.0f, 320.0f, 224.0f);
    ZOID_SetScissor(&scissor0, 0, 0, 640, 448);
    ZOID_SetViewport(0, &vp0, &scissor0);
}

/* 0x80062fb8, size 0xdc */
anon_vp_s_48 *ZOID_InitializeWindow(int idx, anon_vp_20 *vp, anon_ulx_uly_8 *s, float hang, float vang, float width, float height, float hcenter, float vcenter)
{
    anon_vp_s_48 *winfo; /* r31 */
    winfo = &ZoidWinInfo[idx];
    winfo->vp = vp;
    winfo->s = s;
    winfo->frontclip = 0.01f;
    winfo->backclip = 4000.0f;
    gViewableDistance = 4000.0f;
    vp0.vp.vscale[0] = 0.5 * width;
    vp0.vp.vtrans[0] = 0.5 * hcenter;
    vp0.vp.vscale[1] = 0.5 * height;
    vp0.vp.vtrans[1] = 0.5 * vcenter;
    ZOID_SetWindow(idx, hang, vang, width, height, hcenter, vcenter);
    winfo->fogmin = 0;
    winfo->fogmax = 0;
    winfo->fogcolor[0] = 0;
    winfo->fogcolor[1] = 0;
    winfo->fogcolor[2] = 0;
    winfo->fogcolor[3] = 255;
    ZOID_SetBGColor((unsigned short)(((winfo->fogcolor[0] << 8) & 0xf800) | ((winfo->fogcolor[1] << 3) & 0x7c0) | ((winfo->fogcolor[2] >> 2) & 0x3e)) | 1);
    return winfo;
}

/* 0x80063094, size 0x150 */
anon_vp_s_48 *ZOID_SetWindow(unsigned int winidx, float hangle, float vangle, float width, float height, float hcenter, float vcenter)
{
    anon_vp_s_48 *winfo; /* r31 */
    float hang; /* f31 */
    float vang; /* f1 */
    winfo = &ZoidWinInfo[winidx];
    winfo->wwidth = width;
    winfo->wheight = height;
    winfo->whcenter = hcenter;
    winfo->wvcenter = vcenter;
    winfo->aspect = width / height;
    if (hangle > 0.0f) {
        hang = 0.017453292f * hangle;
        vang = vangle > 0.0f ? 0.017453292f * vangle : 2.0f * fatan(height * ftan(hang / 2.0f) / width);
        winfo->flags = 0;
    } else {
        hang = 1.5707964f;
        vang = 2.0f * fatan(height * ftan(0.7853982f) / width);
        winfo->flags = 1;
    }
    _WindowSetAngles(winfo, hang, vang);
    return winfo;
}

/* 0x800631e4, size 0x6c */
anon_vp_s_48 *ZOID_SetFog(signed short min, signed short max, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha, unsigned int winidx)
{
    anon_vp_s_48 *win; /* r31 */
    win = &ZoidWinInfo[winidx];
    win->fogmin = min;
    win->fogmax = max;
    win->fogcolor[0] = red;
    win->fogcolor[1] = green;
    win->fogcolor[2] = blue;
    win->fogcolor[3] = alpha;
    ZOID_SetBGColor((unsigned short)(((red << 8) & 0xf800) | ((green << 3) & 0x7c0) | ((blue >> 2) & 0x3e)) | 1);
    return win;
}

/* 0x80063250, size 0x48 */
anon_vp_s_48 *ZOID_GetFog(signed short *min, signed short *max, unsigned char *red, unsigned char *green, unsigned char *blue, unsigned char *alpha, unsigned int winidx)
{
    anon_vp_s_48 *win; /* r9 */
    win = &ZoidWinInfo[winidx];
    *min = win->fogmin;
    *max = win->fogmax;
    *red = win->fogcolor[0];
    *green = win->fogcolor[1];
    *blue = win->fogcolor[2];
    *alpha = win->fogcolor[3];
    return win;
}

/* 0x80063298, size 0x14 */
anon_vp_s_48 *ZOID_GetWindow(int winidx)
{
    return &ZoidWinInfo[winidx];
}

/* 0x800632ac, size 0x1c */
anon_vp_s_48 *ZOID_SetViewport(int winidx, anon_vp_20 *vp, anon_ulx_uly_8 *s)
{
    anon_vp_s_48 *win; /* r3 */
    win = &ZoidWinInfo[winidx];
    win->vp = vp;
    win->s = s;
    return win;
}

/* 0x800632c8, size 0x14 */
void ZOID_SetScissor(anon_ulx_uly_8 *s, unsigned short ulx, unsigned short uly, unsigned short lrx, unsigned short lry)
{
    s->ulx = ulx;
    s->uly = uly;
    s->lrx = lrx;
    s->lry = lry;
}

/* 0x800632dc, size 0x2b0 */
int ZOID_SplitScreen(int num)
{
    if (num > 0) {
        gNumZoidWindows = num;
        if (num == 1) {
            ZOID_InitializeWindow(0, &vp0, &scissor0, HFOV, 0.0f, 640.0f, 448.0f, 320.0f, 224.0f);
            ZOID_SetScissor(&scissor0, 0, 0, 640, 448);
            ZOID_SetViewport(0, &vp0, &scissor0);
            gSplitScreen = 0;
        } else if (num == 2) {
            ZOID_InitializeWindow(0, &vp1, &scissor1, HFOV, 0.0f, 480.0f, 224.0f, 240.0f, 113.0f);
            ZOID_SetScissor(&scissor1, 0, 0, 480, 224);
            ZOID_SetViewport(0, &vp1, &scissor1);
            ZOID_InitializeWindow(1, &vp2, &scissor2, HFOV, 0.0f, 480.0f, 224.0f, 240.0f, 336.0f);
            ZOID_SetScissor(&scissor2, 0, 224, 480, 448);
            ZOID_SetViewport(1, &vp2, &scissor2);
            gSplitScreen = 1;
        } else if (num == 3 || num == 4) {
            ZOID_InitializeWindow(0, &vpul, &scissorul, HFOV, 0.0f, 320.0f, 224.0f, 160.0f, 112.0f);
            ZOID_SetScissor(&scissorul, 0, 0, 320, 224);
            ZOID_SetViewport(0, &vpul, &scissorul);
            ZOID_InitializeWindow(1, &vpur, &scissorur, HFOV, 0.0f, 320.0f, 224.0f, 480.0f, 112.0f);
            ZOID_SetScissor(&scissorur, 320, 0, 640, 224);
            ZOID_SetViewport(1, &vpur, &scissorur);
            ZOID_InitializeWindow(2, &vpll, &scissorll, HFOV, 0.0f, 320.0f, 224.0f, 160.0f, 336.0f);
            ZOID_SetScissor(&scissorll, 0, 224, 320, 448);
            ZOID_SetViewport(2, &vpll, &scissorll);
            ZOID_InitializeWindow(3, &vplr, &scissorlr, HFOV, 0.0f, 320.0f, 224.0f, 480.0f, 336.0f);
            ZOID_SetScissor(&scissorlr, 320, 224, 640, 448);
            ZOID_SetViewport(3, &vplr, &scissorlr);
            gSplitScreen = 2;
        }
    }
    return gSplitScreen;
}

/* 0x8006358c, size 0x2d0 */
int ZOID_SplitMenuScreen(int num)
{
    if (num > 0) {
        gNumZoidWindows = num;
        if (num == 1) {
            ZOID_InitializeWindow(0, &vp0, &scissor0, HFOV, 0.0f, 640.0f, 448.0f, 320.0f, 224.0f);
            ZOID_SetScissor(&scissor0, 0, 0, 640, 448);
            ZOID_SetViewport(0, &vp0, &scissor0);
            gSplitScreen = 0;
        } else if (num == 2) {
            ZOID_InitializeWindow(0, &vp1, &scissor1, 127.0f * HFOV / 90.0f, 0.0f, 640.0f, 224.0f, 320.0f, 112.0f);
            ZOID_SetScissor(&scissor1, 0, 0, 640, 224);
            ZOID_SetViewport(0, &vp1, &scissor1);
            ZOID_InitializeWindow(1, &vp2, &scissor2, 127.0f * HFOV / 90.0f, 0.0f, 640.0f, 224.0f, 320.0f, 336.0f);
            ZOID_SetScissor(&scissor2, 0, 224, 640, 448);
            ZOID_SetViewport(1, &vp2, &scissor2);
            gSplitScreen = 1;
        } else if (num == 3 || num == 4) {
            ZOID_InitializeWindow(0, &vpul, &scissorul, HFOV, 0.0f, 320.0f, 224.0f, 160.0f, 112.0f);
            ZOID_SetScissor(&scissorul, 0, 0, 320, 224);
            ZOID_SetViewport(0, &vpul, &scissorul);
            ZOID_InitializeWindow(1, &vpur, &scissorur, HFOV, 0.0f, 320.0f, 224.0f, 480.0f, 112.0f);
            ZOID_SetScissor(&scissorur, 320, 0, 640, 224);
            ZOID_SetViewport(1, &vpur, &scissorur);
            ZOID_InitializeWindow(2, &vpll, &scissorll, HFOV, 0.0f, 320.0f, 224.0f, 160.0f, 336.0f);
            ZOID_SetScissor(&scissorll, 0, 224, 320, 448);
            ZOID_SetViewport(2, &vpll, &scissorll);
            ZOID_InitializeWindow(3, &vplr, &scissorlr, HFOV, 0.0f, 320.0f, 224.0f, 480.0f, 336.0f);
            ZOID_SetScissor(&scissorlr, 320, 224, 640, 448);
            ZOID_SetViewport(3, &vplr, &scissorlr);
            gSplitScreen = 2;
        }
    }
    return gSplitScreen;
}

/* 0x8006385c, size 0x184 */
void ZOID_ProjectWindow(int widx, float *point, anon_mat3_mat4_30 *mat, float *body, signed short *screen)
{
    float tmp[3]; /* stack */
    float sx, sy;
    float invz; /* f4 */
    float bdy[3]; /* stack */
    anon_vp_s_48 *wd; /* r0 */
    tmp[0] = point[0] - mat->uvs[3][0];
    tmp[1] = point[1] - mat->uvs[3][1];
    tmp[2] = point[2] - mat->uvs[3][2];
    BodyVector(tmp, bdy, mat->uvs);
    if (bdy[2] < 0.01f) bdy[2] = 0.01f;
    wd = ZOID_GetWindow(widx);
    invz = 1.0f / bdy[2];
    sx = wd->wwidth * (wd->wihvangtan * (bdy[0] * invz)) + wd->whcenter;
    sy = wd->wvcenter - 0.93333f * (wd->wheight * (wd->wivvangtan * (bdy[1] * invz)));
    if (sx < -32768.0f) screen[0] = -32768;
    else if (sx > 32767.0f) screen[0] = 32767;
    else screen[0] = sx;
    if (sy < -32768.0f) screen[1] = -32768;
    else if (sy > 32767.0f) screen[1] = 32767;
    else screen[1] = sy;
    if (body) {
        body[0] = bdy[0];
        body[1] = bdy[1];
        body[2] = bdy[2];
    }
}

/* 0x800639e0, size 0x14c */
void ZOID_WindowTo3D(int widx, signed short *screen, anon_mat3_mat4_30 *mat, float zdist, float *point)
{
    float tmp[3]; /* stack */
    float bvec[3]; /* stack */
    anon_vp_s_48 *wd; /* r0 */
    wd = ZOID_GetWindow(widx);
    bvec[0] = zdist * (screen[0] - wd->whcenter) / (wd->wihvangtan * wd->wwidth);
    bvec[1] = zdist * (wd->wvcenter - screen[1]) / (wd->wivvangtan * wd->wheight);
    bvec[2] = zdist;
    if (gMirrorWaves) bvec[0] = -bvec[0];
    if (gUpsideDownTrack) {
        bvec[0] = -bvec[0];
        bvec[1] = -bvec[1];
    }
    WorldVector(bvec, tmp, mat->uvs);
    point[0] = tmp[0] + mat->uvs[3][0];
    point[1] = tmp[1] + mat->uvs[3][1];
    point[2] = tmp[2] + mat->uvs[3][2];
}

/* 0x80063b2c, size 0x10 */
void ZOID_SetBGColor(unsigned short rgb555)
{
    gBgColor = (rgb555 << 16) | rgb555;
}

