/* ngcDvdCheck.c - disc error polling (cover open, no disc, wrong disc, read error) on a background thread.
   Original: D:\rush_ngc\Rush_NGC\NGC\ngcDvdCheck.c; assembly: build/2049LE/asm/NGC/ngcDvdCheck.s */
#include "rush/rush_protos.h"

#define DVD_STATE_BUSY 1
#define DVD_STATE_NO_DISK 4
#define DVD_STATE_COVER_OPEN 5
#define DVD_STATE_WRONG_DISK 6
#define DVD_STATE_RETRY 11

unsigned long g_bDvdErrorCode;
int g_bpreDvdIsError;
int g_bDvdIsError;
static long s_uResult;
static int s_bDvdOpened;
static OSThread *s_pDvdCheckThread;
static unsigned char *s_pDvdThreadStack;
OSSemaphore callbackSem;
static DVDFileInfo s_fileInfo;

static void * _DVDCallback(void);
static void dvd_check_process_ex(void);
static void dvd_check_process(void);
static void * dvd_check_thread();

/* 0x800ae5cc, size 0x4 */
void init_cs_read(void)
{

}

/* 0x800ae5d0, size 0x4 */
void enter_cs_read(void)
{

}

/* 0x800ae5d4, size 0x4 */
void exit_cs_read(void)
{

}

/* 0x800ae5d8, size 0x4 */
static void * _DVDCallback(void)
{

}

/* 0x800ae5dc, size 0x78 */
void _DVDBeforeCheck(void)
{
    if (s_bDvdOpened) {
        DVDClose(&s_fileInfo);
    }
    s_bDvdOpened = DVDOpen("rush2049_data/DvdCheckFile.tmp", &s_fileInfo);
    if (s_bDvdOpened) {
        DVDReadAsyncPrio(&s_fileInfo, g_TempBuf, 0x20, 0, _DVDCallback, 2);
    }
}

/* 0x800ae654, size 0x4 */
void _DVDDoneCheck(void)
{

}

/* 0x800ae658, size 0x2c */
int _DVDCoverIsOpen(void)
{
    return DVDGetDriveStatus() == DVD_STATE_COVER_OPEN;
}

/* 0x800ae684, size 0x2c */
int _DVDIsNoDisk(void)
{
    return DVDGetDriveStatus() == DVD_STATE_NO_DISK;
}

/* 0x800ae6b0, size 0x2c */
int _DVDIsWrongDisk(void)
{
    return DVDGetDriveStatus() == DVD_STATE_WRONG_DISK;
}

/* 0x800ae6dc, size 0x2c */
int _DVDIsBrokenDisk(void)
{
    return DVDGetDriveStatus() == DVD_STATE_RETRY;
}

/* 0x800ae708, size 0x30 */
void _DVDWait(void)
{
    while (DVDGetDriveStatus() == DVD_STATE_BUSY) {
        ResetCheck();
    }
}

/* 0x800ae738, size 0xac */
unsigned long _DVDErrorCheck(void)
{
    _DVDBeforeCheck();
    _DVDWait();
    if (GetFatalError()) {
        s_uResult = 1;
    } else if (_DVDCoverIsOpen()) {
        s_uResult = 0x10000000;
    } else if (_DVDIsNoDisk()) {
        s_uResult = 0x1000000;
    } else if (_DVDIsWrongDisk()) {
        s_uResult = 0x100000;
    } else if (_DVDIsBrokenDisk()) {
        s_uResult = 0x10000;
    } else {
        s_uResult = 0;
    }
    _DVDDoneCheck();
    return s_uResult;
}

/* 0x800ae7e4, size 0xa0 */
unsigned long _DVDErrorCheckEx(void)
{
    if (GetFatalError()) {
        s_uResult = 1;
    } else if (_DVDCoverIsOpen()) {
        s_uResult = 0x10000000;
    } else if (_DVDIsNoDisk()) {
        s_uResult = 0x1000000;
    } else if (_DVDIsWrongDisk()) {
        s_uResult = 0x100000;
    } else if (_DVDIsBrokenDisk()) {
        s_uResult = 0x10000;
    } else {
        s_uResult = 0;
    }
    return s_uResult;
}

/* 0x800ae884, size 0x34 */
static void dvd_check_process_ex(void)
{
    g_bDvdErrorCode = _DVDErrorCheckEx();
    g_bDvdIsError = g_bDvdErrorCode != 0;
}

/* 0x800ae8b8, size 0x34 */
static void dvd_check_process(void)
{
    g_bDvdErrorCode = _DVDErrorCheck();
    g_bDvdIsError = g_bDvdErrorCode != 0;
}

/* 0x800ae8ec, size 0xdc */
static void * dvd_check_thread()
{
    float prevTime; /* f30 */
    float curTime; /* f29 */
    prevTime = 0.0f;
    for (;;) {
        enter_cs();
        curTime = appGetTime();
        curTime *= 1000.0f;
        if (curTime - prevTime >= 1000.0f || curTime - prevTime < 0.0f) {
            dvd_check_process();
            prevTime = curTime;
        }
        if (g_bDvdIsError && g_bLoadingNow) {
            ngcBeforeRender();
            DRAW_TRC_DVD();
            ngcDoneRender();
        } else if (g_bShowLoadingEffect) {
            ngcBeforeRender();
            g_LoadFrameCount++;
            ngcDoneRender();
        } else if (g_bLoadingNow && g_bpreDvdIsError == 1) {
            ngcBeforeRender();
            ngcDoneRender();
        }
        g_bpreDvdIsError = g_bDvdIsError;
        ResetCheck();
        exit_cs();
    }
}

/* 0x800ae9c8, size 0x7c */
void DVDCheckThreadStart(void)
{
    OSThread *_Thread; /* r31 */
    _Thread = AllocateMemoryHeap(0, sizeof(OSThread));
    s_pDvdCheckThread = _Thread;
    s_pDvdThreadStack = AllocateMemoryHeap(0, 0x1000);
    OSCreateThread(_Thread, dvd_check_thread, 0, s_pDvdThreadStack + 0x1000, 0x1000, 31, 1);
    OSResumeThread(_Thread);
}

/* 0x800aea44, size 0x3c */
void DVDCheckThreadEnd(void)
{
    OSSuspendThread(s_pDvdCheckThread);
    OSCancelThread(s_pDvdCheckThread);
    DeallocateMemory(s_pDvdCheckThread);
    DeallocateMemory(s_pDvdThreadStack);
}

/* 0x800aea80, size 0x34 */
void ResetAndDVDChecks(void)
{
    ngcBeforeRender();
    ngcBlackBackground();
    dvd_check_process_ex();
    DRAW_TRC_DVD();
    ngcDoneRender();
    ResetCheck();
}

/* 0x800aeab4, size 0x28 */
void Run_DVDCallback(void)
{
    OSSignalSemaphore(&callbackSem);
}

/* 0x800aeadc, size 0xc8 */
void ForceDVDStateTest(void)
{
    DVDFileInfo fInfo; /* stack */
    OSInitSemaphore(&callbackSem, 0);
    DVDOpen("/opening.bnr", &fInfo);
    DVDSeekAsyncPrio(&fInfo, 4, Run_DVDCallback, 2);
    while (OSTryWaitSemaphore(&callbackSem) <= 0) {
        ResetAndDVDChecks();
        if (OSGetResetButtonState()) {
            g_RegionData.Reset = 1;
        } else {
            g_RegionData.Reset = 0;
        }
    }
    DVDClose(&fInfo);
    DVDCancelAll();
}

