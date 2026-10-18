#ifndef RUSH_DOLPHIN_H
#define RUSH_DOLPHIN_H

/* Nintendo GameCube SDK ("Dolphin") functions the game calls. The SDK libraries have no debug info,
   so these are not in rush_protos.h; signatures follow the public SDK headers. Add as needed. */

void GXSetViewport(float left, float top, float wd, float ht, float nearz, float farz);
void GXSetScissor(unsigned long left, unsigned long top, unsigned long wd, unsigned long ht);
unsigned long VIGetTvFormat(void);
void OSPanic(const char *file, int line, const char *msg, ...);
void OSReport(const char *msg, ...);

void *OSAllocFromHeap(int heap, unsigned long size);
void OSFreeToHeap(int heap, void *ptr);
void DCStoreRange(void *addr, unsigned long nBytes);
void DCFlushRange(void *addr, unsigned long nBytes);
void DCInvalidateRange(void *addr, unsigned long nBytes);

unsigned long GXGetTexBufferSize(unsigned short width, unsigned short height, unsigned long format,
                                 unsigned char mipmap, unsigned char max_lod);
void GXInitTexObj(_GXTexObj *obj, void *image_ptr, unsigned short width, unsigned short height,
                  int format, int wrap_s, int wrap_t, unsigned char mipmap);
void GXInitTexObjLOD(_GXTexObj *obj, int min_filt, int mag_filt, float min_lod, float max_lod,
                     float lod_bias, unsigned char bias_clamp, unsigned char do_edge_lod, int max_aniso);

long long OSGetTime(void);
int OSExecv(const char *dolfile, char *const argv[]);
#define OS_BUS_CLOCK (*(unsigned long *)0x800000F8)
#define OS_TIMER_CLOCK (OS_BUS_CLOCK / 4)
#define OSTicksToMilliseconds(ticks) ((ticks) / (OS_TIMER_CLOCK / 1000))

void OSSetSaveRegion(void *start, void *end);
void OSGetSavedRegion(void **start, void **end);
unsigned long OSGetResetCode(void);
int OSGetResetButtonState(void);
void OSResetSystem(int reset, unsigned long resetCode, int forceMenu);
int PADRecalibrate(unsigned long mask);
unsigned long PADRead(PADStatus *status);
int PADReset(unsigned long mask);
void PADControlMotor(long chan, unsigned long command);
void VISetBlack(int black);
void VIFlush(void);
void VIWaitForRetrace(void);

void GXGetViewportv(float *vp);
void GXSetNumTevStages(unsigned char nStages);
void GXSetNumChans(unsigned char nChans);
void GXClearVtxDesc(void);
void GXSetVtxDesc(int attr, int type);
void GXSetVtxAttrFmt(int vtxfmt, int attr, int cnt, int type, unsigned char frac);
void GXSetZMode(unsigned char compare_enable, int func, unsigned char update_enable);
void GXSetTevOp(int id, int mode);
void GXBegin(int type, int vtxfmt, unsigned short nverts);

int DVDOpen(char *fileName, DVDFileInfo *fileInfo);
long DVDReadPrio(DVDFileInfo *fileInfo, void *addr, long length, long offset, long prio);
int DVDClose(DVDFileInfo *fileInfo);

long DVDGetDriveStatus(void);
int DVDReadAsyncPrio(DVDFileInfo *fileInfo, void *addr, long length, long offset, void *callback, long prio);
int DVDSeekAsyncPrio(DVDFileInfo *fileInfo, long offset, void *callback, long prio);
int DVDCancelAll(void);

int OSCreateThread(OSThread *thread, void *(*func)(void *), void *param, void *stack, unsigned long stackSize,
                   long priority, unsigned short attr);
long OSResumeThread(OSThread *thread);
long OSSuspendThread(OSThread *thread);
void OSCancelThread(OSThread *thread);
OSThread *OSGetCurrentThread(void);
void OSInitSemaphore(OSSemaphore *sem, long count);
long OSSignalSemaphore(OSSemaphore *sem);
long OSTryWaitSemaphore(OSSemaphore *sem);

void C_MTXLookAt(float m[3][4], tVec3 *camPos, tVec3 *camUp, tVec3 *target);
void PSMTX44Trans(float m[4][4], float xT, float yT, float zT);
void PSMTX44RotRad(float m[4][4], char axis, float rad);

/* charPipeline display cache */
void *DSGetCacheObj(anon_PurgeFlag_CacheNodeList_10 *cache, char *name);
void DSAddCacheNode(anon_PurgeFlag_CacheNodeList_10 *cache, char *name, void *data, void *freeFunc);
void DSReleaseCacheObj(anon_PurgeFlag_CacheNodeList_10 *cache, void *data);

/* GX write-gather FIFO and the SDK's inline vertex functions */
typedef union {
    unsigned char u8;
    signed char s8;
    unsigned short u16;
    signed short s16;
    unsigned long u32;
    signed long s32;
    float f32;
} PPCWGPipe;
/* CodeWarrior's fixed-address declaration, as in the SDK. Dereferencing a pointer constant instead
   gives different FP register allocation in the inline vertex functions (ngcMovie.c). */
volatile PPCWGPipe GXWGFifo : 0xCC008000;

static inline void GXPosition3f32(float x, float y, float z)
{
    GXWGFifo.f32 = x;
    GXWGFifo.f32 = y;
    GXWGFifo.f32 = z;
}
static inline void GXColor1u32(unsigned long clr) { GXWGFifo.u32 = clr; }
static inline void GXTexCoord2f32(float s, float t)
{
    GXWGFifo.f32 = s;
    GXWGFifo.f32 = t;
}
static inline void GXEnd(void) {}

#endif
