/* HAL_time.c - once-per-frame callback table. */
#include "rush/rush_protos.h"

static signed char gInited;
static void (*gOPPFuncs[10])(void);

static void HAL_InitTime(void);

/* 0x8001b7ac */
static void HAL_InitTime(void)
{
    int i;

    if (gInited)
        return;
    gInited = 1;
    for (i = 0; i < 10; i++)
        gOPPFuncs[i] = NULL;
}

/* 0x8001b7f4 */
void HAL_InstallOncePerFrameFunc(void (*const func)(void))
{
    int i;

    if (!gInited)
        HAL_InitTime();
    for (i = 0; i < 10; i++) {
        if (gOPPFuncs[i] == NULL) {
            gOPPFuncs[i] = func;
            break;
        }
    }
}

/* 0x8001b86c */
void HAL_CallOncePerFrameFuncs(void)
{
    int i;

    if (!gInited)
        HAL_InitTime();
    for (i = 0; i < 10; i++) {
        if (gOPPFuncs[i])
            gOPPFuncs[i]();
    }
}
