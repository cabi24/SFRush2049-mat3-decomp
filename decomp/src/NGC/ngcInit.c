/* ngcInit.c - GameCube video, framebuffer, heap and GX initialization. Matched.
Original: D:\rush_ngc\Rush_NGC\NGC\ngcInit.c;
assembly: build/2049LE/asm/NGC/ngcInit.s */
#include "rush/rush_protos.h"
void *OSGetArenaLo(void);
void *OSGetArenaHi(void);
void OSSetArenaLo(void *);
void *OSInitAlloc(void *, void *, int);
int OSCreateHeap(void *, void *);
int OSSetCurrentHeap(int);
anon_pad_80 *GXInit(void *, unsigned long);
void GXAdjustForOverscan(_GXRenderModeObj *, _GXRenderModeObj *, unsigned short, unsigned short);
float GXGetYScaleFactor(unsigned short, unsigned short);
unsigned long GXSetDispCopyYScale(float);
void GXSetViewportJitter(float, float, float, float, float, float, unsigned int);
void GXSetCopyClear(_GXColor, unsigned long);
void GXSetDispCopyDst(unsigned short, unsigned short);

static void __ngcInitRenderMode(_GXRenderModeObj *mode);
static void __ngcInitMem(void);
static void __ngcInitGX(void);
static void __ngcInitVI(void);

int g_bRenderToTex  =  1;
static unsigned char ngcFirstFrame  =  1;
void * ngcFrameBuffer1;
void * ngcFrameBuffer2;
void * ngcCurrentBuffer;
unsigned char *g_TempBuf;
static void * DefaultFifo;
static anon_pad_80 *DefaultFifoObj;
static _GXRenderModeObj *rmode;
static int CurrentHeapHandle;
static int GPHangWorkaround;
static int TotalHeapSize;
static unsigned long allocatedFrameBufferSize;
int bSaveImage;
static _GXRenderModeObj rmodeobj;

/* 0x8009ceb8, size 0xe4 */
void ngcInit(_GXRenderModeObj *mode)
{
    DVDSetAutoFatalMessaging(1);
    OSInit();
    DVDInit();
    VIInit();
    PADInit();
    __ngcInitRenderMode(mode);
    __ngcInitMem();
    InitMemory(500, 250);
    VIConfigure(rmode);
    CARDInit();
    DefaultFifo = AllocateMemoryHeap(0, 0x40000);
    DefaultFifoObj = GXInit(DefaultFifo, 0x40000);
    __ngcInitGX();
    __ngcInitVI();
    ngcInitAudio();
    ngcInitTimeBaseRegister();
    appVBlankHandler(0);
    ngcAddInit();
    PS2InitTexWave();
    PS2InitGeoWave();
    leIniFrameCopy();
    leInitialize(0, 16, 4096);
    init_cs_read();
    InitFont();
    ForceDVDStateTest();
    ReadRegion();
    g_TempBuf = AllocateMemoryHeap(0, 1024);
    DVDCheckThreadStart();
    LoadMenuTexData();
    ngcLoadSelectEffect();
}

/* 0x8009cf9c, size 0x154 */
static void __ngcInitRenderMode(_GXRenderModeObj *mode)
{
    if (mode != NULL) {
        rmodeobj  =  *mode;
        rmode  =  &rmodeobj;
        return;
    }

    switch (VIGetTvFormat()) {
        case 0:
        rmode  =  &GXNtsc480IntDf;
        break;
        case 1:
        rmode  =  &GXPal528IntDf;
        break;
        case 5:
        rmode  =  &GXEurgb60Hz480IntDf;
        break;
        case 2:
        rmode  =  &GXMpal480IntDf;
        break;
        default:
        OSPanic("ngcInit.c", 320, "ngcInit: invalid TV format\n");
    }

    GXAdjustForOverscan(rmode, &rmodeobj, 0, 16);
    rmode  =  &rmodeobj;
}

/* 0x8009d0f0, size 0xdc */
static void __ngcInitMem(void)
{
    void *arenaLo;
    void *arenaHi;
    unsigned long fbSize;
    arenaLo = OSGetArenaLo();
    arenaHi = OSGetArenaHi();
    fbSize = (unsigned short)((rmode->fbWidth+15)&~15)*rmode->xfbHeight*2;
    ngcFrameBuffer1 = (void *)(((unsigned long)arenaLo+31)&~31);
    allocatedFrameBufferSize = fbSize;
    arenaLo = (void *)(((unsigned long)ngcFrameBuffer1+fbSize+31)&~31);
    ngcFrameBuffer2 = arenaLo;
    ngcCurrentBuffer = ngcFrameBuffer2;
    arenaLo = (void *)(((unsigned long)arenaLo+fbSize+31)&~31);
    OSSetArenaLo(arenaLo);
    arenaLo = OSGetArenaLo();
    arenaHi = OSGetArenaHi();
    arenaLo = OSInitAlloc(arenaLo, arenaHi, 1);
    OSSetArenaLo(arenaLo);
    arenaLo = (void *)(((unsigned long)arenaLo+31)&~31);
    arenaHi = (void *)((unsigned long)arenaHi&~31);
    TotalHeapSize = (unsigned long)arenaHi-(unsigned long)arenaLo;
    CurrentHeapHandle = OSSetCurrentHeap(OSCreateHeap(arenaLo, arenaHi));
    OSSetArenaLo(arenaHi);
}

/* 0x8009d1cc, size 0x8 */
int ngcGetHeapSize(void)
{
    return TotalHeapSize;
}

/* 0x8009d1d4, size 0x144 */
static void __ngcInitGX(void)
{
    unsigned short xfbHeight;
    float yScale;
    _GXColor BgColor = {0, 0, 0, 0};
    GXSetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    GXSetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
    yScale = GXGetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
    xfbHeight = GXSetDispCopyYScale(yScale);
    GXSetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GXSetDispCopyDst(rmode->fbWidth, xfbHeight);
    GXSetCopyFilter(rmode->aa, rmode->sample_pattern, 1, rmode->vfilter);
    if (rmode->aa) GXSetPixelFmt(2, 0);
    else GXSetPixelFmt(0, 0);
    GXCopyDisp(ngcCurrentBuffer, 1);
    GXSetDispCopyGamma(0);
    GXSetCullMode(0);
    GXSetCopyClear(BgColor, 0xffffff);
}

/* 0x8009d318, size 0x48 */
static void __ngcInitVI(void)
{
    VISetNextFrameBuffer(ngcFrameBuffer1);
    ngcCurrentBuffer = ngcFrameBuffer2;
    VIFlush();
    VIWaitForRetrace();
    if (rmode->viTVmode&1) VIWaitForRetrace();
}

/* 0x8009d360, size 0xe8 */
void ngcBeforeRender(void)
{
    if (GPHangWorkaround) {
        GXSetDrawSync(0xfeeb);
        GXClearGPMetric();
    }
    if (rmode->field_rendering) GXSetViewportJitter(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1, VIGetNextField());
    else GXSetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    GXInvalidateVtxCache();
    GXInvalidateTexAll();
}

/* 0x8009d448, size 0xc8 */
void ngcDoneRender(void)
{
    if (bSaveImage) bSaveImage = 0;
    if (!g_bDvdIsError) {
        if (!g_bLoadingNow) leCopyFrameBufferTex(0);
        ngcBlackBackground();
        BlitOneTpl_Obj_Frame(&g_BGTexObj);
        if (!g_bRenderToTex) {
            SetViewPortForScreenSize();
            do_loading_effect();
        }
    }
    GXSetZMode(1, 3, 1);
    GXSetColorUpdate(1);
    rmode->viXOrigin = g_vx + 40;
    rmode->viYOrigin = g_vy + 16;
    VIConfigure(rmode);
    GXCopyDisp(ngcCurrentBuffer, 1);
    GXDrawDone();
    ngcSwapBuffers();
}

/* 0x8009d510, size 0x7c */
void ngcSwapBuffers(void)
{
    VISetNextFrameBuffer(ngcCurrentBuffer);
    if (ngcFirstFrame) {
        VISetBlack(0);
        ngcFirstFrame = 0;
    }
    VIFlush();
    VIWaitForRetrace();
    ngcCurrentBuffer = ngcCurrentBuffer == ngcFrameBuffer1 ? ngcFrameBuffer2 : ngcFrameBuffer1;
    sc.frameCount++;
    statsEnd();
    statsBegin();
}

/* 0x8009d58c, size 0x8 */
_GXRenderModeObj *ngcGetRenderModeObj(void)
{
    return rmode;
}

/* 0x8009d594, size 0x9c */
void ngcAddInit(void)
{
    GXSetNumTevStages(1);
    GXSetNumChans(1);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxDesc(11, 1);
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetVtxAttrFmt(0, 11, 1, 5, 0);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
}

