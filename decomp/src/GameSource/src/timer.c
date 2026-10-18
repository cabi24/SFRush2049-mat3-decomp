/* timer.c - frame timer and countdown timer, driven by the scheduler's retrace count.
   Data placement: tentative definitions are emitted at end of file in reverse order;
   gTimerPaused is explicitly initialised so it is emitted before InitializeTimer's static.
   volatile (not recorded in DWARF, inferred from reloads): sc.frameCount, gSeconds, gFrameSeconds,
   gLastFrameCount, gLastFrameCounter - see include/rush/overrides.json. */
#include "rush/rush_protos.h"

float gRetraceRate = 60.0f;
float gRetraceTime = 0.016666668f;

volatile float gSeconds;
volatile float gFrameSeconds;
volatile int gLastFrameCount;
static unsigned int gCountdownEnd;
static unsigned int gCountdownStart;
static volatile unsigned int gLastFrameCounter;
static int gTimerPaused = 0; /* explicit `= 0`: emitted at its declaration point, before the function-local statics */

/* 0x80050c74 */
void InitializeTimer(void)
{
    static int sInited;

    if (!sInited) {
        sc.frameCount = 0;
        sInited = 1;
    }
    gLastFrameCounter = sc.frameCount;
    gSeconds = gFrameSeconds = gRetraceTime;
}

/* 0x80050cb8 */
void PauseTimer(void)
{
    gTimerPaused = 1;
}

/* 0x80050cc4 */
void ResumeTimer(void)
{
    gTimerPaused = 0;
}

/* 0x80050cd0 */
void UpdateFrameTimer(void)
{
    gLastFrameCount = sc.frameCount - gLastFrameCounter;
    gLastFrameCounter = sc.frameCount;
    if (gTimerPaused) {
        gCountdownStart += gLastFrameCount;
        gCountdownEnd += gLastFrameCount;
    } else {
        gSeconds = gRetraceTime * (float)(int)(sc.frameCount - gCountdownStart);
    }
    if (gLastFrameCount > 5)
        gLastFrameCount = 5;
    gFrameSeconds = (float)gLastFrameCount * gRetraceTime;
}

/* 0x80050da0 */
void SetCountdownTimer(float sec)
{
    gCountdownStart = sc.frameCount;
    gCountdownEnd = sc.frameCount + (int)(sec * gRetraceRate);
}

/* 0x80050ddc */
void AddCountdownTime(float sec)
{
    gCountdownEnd += (int)(sec * gRetraceRate);
}

/* 0x80050e08 */
float GetCountdownTimeRemaining(void)
{
    return gRetraceTime * (float)(int)(gCountdownEnd - sc.frameCount);
}

/* 0x80050e4c */
int CountdownTimeOut(void)
{
    return (int)(gCountdownEnd - sc.frameCount) <= 0;
}
