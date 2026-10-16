/* ngcMovie.c - logo movie playback and a full-screen black quad.
   Original: D:\rush_ngc\Rush_NGC\NGC\ngcMovie.c; assembly: build/2049LE/asm/NGC/ngcMovie.s

   Only ngcPlayMovie and ngcBlackBackground survive in the DOL. The other functions were stripped by
   the linker, but their string literals and array initialisers stayed in .rodata, so they are kept
   here. Their bodies are placeholders: only the rodata they emit, and its order, can be checked. */
#include "rush/rush_protos.h"

#define ATTRIBUTE_ALIGN(n) __attribute__((aligned(n)))

static unsigned long g_bMovieLoaded;
static void *g_MoviePal;
static unsigned long g_nMovieFrames;
static unsigned long g_nMovieFramesFps;
static unsigned long g_nMovieFramesCur;

/* stripped */
static int ngcLoadMovie(char * strFileName, unsigned long nFrames, unsigned long fps)
{
    char name[256];
    int fd;

    sprintf(name, "rush2049_data/LOGO_MOVIE/%s.tpl", strFileName);
    fd = appFileOpen(name, 0);
    g_nMovieFrames = nFrames;
    g_nMovieFramesFps = fps;
    return fd;
}

/* stripped */
static int ngcReleaseMovie(void)
{
    g_MoviePal = 0;
    return 1;
}

/* 0x800ae2a4, size 0x40 */
int ngcPlayMovie(void)
{
    sndPlayerMusicPrepare("rush2049_data/LOGO_MOVIE/RUSH2049_long.bik", 0, 0, 0);
    PlayMovie();
    sndPlayerMusicStop();
    return 1;
}

/* stripped */
static void ReleaseMovieMem(void)
{
}

/* stripped */
static int ngcPlayMovieNext(void)
{
    g_nMovieFramesCur++;
    return g_nMovieFramesCur < g_nMovieFrames;
}

/* stripped */
static int ngcPlayMovieNum(unsigned long num)
{
    int i;
    _GXTexObj pTexObj;
    float Vert_f32[12] = { 0.0f, 480.0f, 0.0f, 640.0f, 480.0f, 0.0f, 640.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    float TexCoords_f32[8] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
    unsigned long Colors_u32[4] = { 0xff, 0xff, 0xff, 0xff };

    GXBegin(0x80, 0, 4);
    for (i = 0; i < 4; i++) {
        GXPosition3f32(Vert_f32[i * 3], Vert_f32[i * 3 + 1], Vert_f32[i * 3 + 2]);
        GXColor1u32(Colors_u32[i]);
        GXTexCoord2f32(TexCoords_f32[i * 2], TexCoords_f32[i * 2 + 1]);
    }
    GXEnd();
    return g_MoviePal != 0 && num;
}

/* 0x800ae2e4, size 0x2e8 */
int ngcBlackBackground(void)
{
    int i; /* r0 */
    float Vert_f32[12] ATTRIBUTE_ALIGN(32) = { 0.0f, 480.0f, 0.0f, 640.0f, 480.0f, 0.0f, 640.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    unsigned long Colors_u32[4] ATTRIBUTE_ALIGN(32) = { 0xff, 0xff, 0xff, 0xff };
    float TexCoords_f32[8] ATTRIBUTE_ALIGN(32) = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
    float vp[6] ATTRIBUTE_ALIGN(32); /* stack */

    ngcBlitInit();
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
    GXSetZMode(1, 7, 1);
    GXSetTevOp(0, 4);
    GXBegin(0x80, 0, 4);
    for (i = 0; i < 4; i++) {
        GXPosition3f32(Vert_f32[i * 3], Vert_f32[i * 3 + 1], Vert_f32[i * 3 + 2]);
        GXColor1u32(Colors_u32[i]);
        GXTexCoord2f32(TexCoords_f32[i * 2], TexCoords_f32[i * 2 + 1]);
    }
    GXEnd();
}

/* stripped */
static int ngcPlayMovieSound(char * filename)
{
    char name[256];
    int fd;

    sprintf(name, "rush2049_data/LOGO_MOVIE/%s.adp", filename);
    fd = appFileOpen(name, 0);
    return fd;
}

/* stripped */
static int ngcEndMovieSound(void)
{
    return 1;
}

/* stripped */
static int ngcPlayMovieRushRush(void)
{
    g_bMovieLoaded = ngcLoadMovie("rushrush", 0, 0);
    return g_bMovieLoaded;
}
