/* ngcHelper.c - timing, command-line settings, and relaunching into another game (the Midway Arcade Treasures menu).
   Original: D:\rush_ngc\Rush_NGC\NGC\ngcHelper.c; assembly: build/2049LE/asm/NGC/ngcHelper.s */
#include "rush/rush_protos.h"

static double s_gBeginTime;
static unsigned long long s_cpuTick;
volatile unsigned long GVSyncCount;
_tGlobalSettings GSettings;

/* 0x80097adc, size 0x28 */
void ngcInitTimeBaseRegister(void)
{
    s_cpuTick = OSGetTime();
}

/* 0x80097b04, size 0x58 */
int appVBlankHandler(void)
{
    unsigned long long tick; /* r0 */

    tick = OSTicksToMilliseconds((unsigned long long)OSGetTime());
    s_gBeginTime = tick / 1000.0;
    return 0;
}

/* 0x80097b5c, size 0x5c */
float appGetTime(void)
{
    unsigned long long tick;
    double time3;

    tick = OSTicksToMilliseconds((unsigned long long)OSGetTime());
    time3 = tick / 1000.0;
    {
        double delta;
        /* Retain the native subtraction temporary before narrowing to float. */
        do {
            delta = time3 - s_gBeginTime;
            if (delta) break;
        } while (0);
        return delta;
    }
}

/* 0x80097bb8, size 0x74 */
void appSleep(float s)
{
    float StartTime; /* f0 */
    float ElapsTime; /* f30 */

    ElapsTime = 0.0f;
    StartTime = appGetTime();
    while (ElapsTime < s) {
        ElapsTime = appGetTime() - StartTime;
        ResetCheck();
    }
}

/* 0x80097c2c, size 0x8 */
unsigned long appGetCycle(void)
{
    return 0;
}

/* 0x80097c34, size 0x8 */
float appCycleToSeconds(void)
{
    return 0.0f;
}

/* stripped; kept for the int-to-float constant it creates before appParseInt's literals */
static float appGetFps(void)
{
    return (int)GVSyncCount;
}

/* 0x80097c3c, size 0xac */
int appParseInt(const char *s, const char *val_name, int default_val)
{
    char *p; /* r0 */
    char *q; /* r30 */
    char *r; /* r0 */

    p = strstr(s, val_name);
    if (p) {
        p = strchr(p, '=');
        if (p) {
            q = p + 1;
            r = strchr(q, ' ');
            if (r) {
                *r = 0;
            }
            default_val = strtol(q, 0, strstr(q, "0x") == q ? 16 : 10);
            if (r) {
                *r = ' ';
            }
        }
    }
    return default_val;
}

/* 0x80097ce8, size 0x298 */
void appParseCmdArgs(int argc, char **argv)
{
    char cmd_line[256]; /* stack */
    int i; /* r30 */

    memset(&GSettings, 0, sizeof(GSettings));
    GSettings.nArgc = argc;
    GSettings.pArgv = argv;
    if (argc > 1) {
        cmd_line[0] = 0;
        for (i = 1; i < argc; i++) {
            strcat(cmd_line, argv[i]);
            strcat(cmd_line, " ");
        }
    } else {
        strcpy(cmd_line, "-heap=28");
    }
    GSettings.bCdMode = strstr(cmd_line, "-hdd") == 0;
    GSettings.bUseDiskImage = strstr(cmd_line, "-nodi") == 0;
    GSettings.bNoSound = strstr(cmd_line, "-nosound") != 0;
    GSettings.bPalMode = strstr(cmd_line, "-pal") != 0;
    GSettings.bDIProfile = strstr(cmd_line, "-diprof") != 0;
    GSettings.bNoStream = strstr(cmd_line, "-nostream") != 0;
    GSettings.bRumble = strstr(cmd_line, "-vib") != 0;
    GSettings.bConfig = strstr(cmd_line, "-alt") != 0;
    GSettings.bAutoSave = strstr(cmd_line, "-as") != 0;
    GSettings.nScreenW = GSettings.bPalMode ? 512 : 640;
    GSettings.nScreenH = GSettings.bPalMode ? 512 : 448;
    GSettings.fHeapSize = appParseInt(cmd_line, "-heap=", 28);
    GSettings.nFrameDepth = 32;
    GSettings.bUseD2D = 1;
    if (!GSettings.bUseDiskImage) {
        GSettings.bNoStream = 1;
    }
}

/* 0x80097f80, size 0x28 */
void * sndMalloc(int size)
{
    return AllocateMemoryHeap(0, size);
}

/* 0x80097fa8, size 0x20 */
void appFree(void * p)
{
    DeallocateMemory(p);
}

/* 0x80097fc8, size 0x4 */
void appCheatCodeBind(void)
{

}

/* stripped; kept for its message literal */
static unsigned short appAsciiFix(unsigned char ascii_code)
{
    OSReport("bad ASCII code 0x%x\n", ascii_code);
    return 0;
}

/* 0x80097fcc, size 0x1d4 */
void Run(const char *executableName, ...)
{
    char ngcName[256]; /* stack */
    va_list varg; /* stack */
    char *ngcArgs[30]; /* stack */
    char *p; /* r8 */
    int i; /* r31 */
    char *arg; /* r4 */
    char temp[100][256]; /* stack */
    long resSave; /* r0 */

    i = 0;
    if (g_AutoSave) {
        g_bMCInGameState = 1;
        appCheckSlot();
        SetLoopMount(0);
        for (;;) {
            if (!MenuMCMount(0)) {
                break;
            }
            g_bResetEnable = 0;
            resSave = ngcSaveToMemcard(CurSlot);
            g_bResetEnable = 1;
            if (resSave != -1) {
                break;
            }
            SetLoopMount(1);
        }
    }
    DVDCheckThreadEnd();
    ForceDVDStateTest();
    va_start(varg, executableName);
    VISetBlack(1);
    VIFlush();
    VIWaitForRetrace();
    VIFlush();
    VIWaitForRetrace();
    ngcName[0] = '/';
    p = ngcName + 1;
    while (*executableName) {
        *p++ = *executableName++;
    }
    p[0] = '.';
    p[1] = 'd';
    p[2] = 'o';
    p[3] = 'l';
    p[4] = 0;
    memset(ngcArgs, sizeof(ngcArgs), 0); /* sic: arguments swapped, clears nothing */
    while ((arg = va_arg_int(varg, char *)) != 0) {
        ngcArgs[i] = temp[i];
        strncpy(ngcArgs[i++], arg, 255);
    }
    ngcArgs[i] = 0;
    WriteRegion();
    OSExecv(ngcName, ngcArgs);
}

