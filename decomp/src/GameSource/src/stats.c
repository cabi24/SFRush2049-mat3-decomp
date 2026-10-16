/* stats.c - profiling timers, counters and the FPS meter. */
#include "rush/rush_protos.h"

static _eStatsStage s_statsStage = 1;
static char debug_info[1024];
static unsigned long s_gpuTimer[3];
static int s_drawTypeDisabled[3];
static _tStats s_stats;

static void on_change_stats_group0(void);

/* 0x80063b3c - cheat "rr" toggles the stats display */
static void on_change_stats_group0(void)
{
    s_statsStage = s_statsStage != 1;
}

/* 0x80063b58 */
void statsClock(_eStatsTimer type)
{
    s_stats.startTime[type] = appGetCycle();
}

/* 0x80063b98 */
void statsUnclock(_eStatsTimer type)
{
    s_stats.timer[type] += appGetCycle() - s_stats.startTime[type];
}

/* 0x80063be4 */
void statsCount(_eStatsCounter counter)
{
    s_stats.counter[counter]++;
}

/* 0x80063c04 */
void statsInit(void)
{
    int i;

    for (i = 0; i < 11; i++) {
        s_stats.lastTime[i] = 0.0f;
        s_stats.timer[i] = 0;
    }
    for (i = 0; i < 6; i++) {
        s_stats.lastCount[i] = 0;
        s_stats.counter[i] = 0;
    }
    for (i = 0; i < 3; i++) {
        s_drawTypeDisabled[i] = 0;
        s_gpuTimer[i] = 0;
    }
    appCheatCodeBind("rr", on_change_stats_group0);
}

/* 0x80063d1c */
void statsBegin(void)
{
    int i;

    for (i = 0; i < 11; i++)
        s_stats.timer[i] = 0;
    for (i = 0; i < 6; i++)
        s_stats.counter[i] = 0;
}

/* 0x80063da0 */
void statsEnd(void)
{
    int i;
    float elapsed_vsync;

    for (i = 0; i < 11; i++)
        s_stats.lastTime[i] = appCycleToSeconds(s_stats.timer[i]);
    for (i = 0; i < 6; i++)
        s_stats.lastCount[i] = s_stats.counter[i];
    if (++s_stats.fpsElapsedFrames == 6) {
        elapsed_vsync = GVSyncCount - s_stats.fpsStartVSync;
        s_stats.fpsAverage = 60.0f * s_stats.fpsElapsedFrames / elapsed_vsync;
        s_stats.fpsElapsedFrames = 0;
        s_stats.fpsStartVSync = GVSyncCount;
    }
}
