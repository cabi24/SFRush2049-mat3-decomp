/* model.c - model thread entry and catch-up (rubber band) speed scaling.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\model.c; assembly: build/2049LE/asm/GameSource/src/model.s */
#include "rush/rush_protos.h"

unsigned short modelrun;
signed char gCatchup;
signed char gUseCatchup;
signed short gNumPlayers;
int gStuntsDone;
tPadState gPlayers[4];
tStuntPlayer gStuntPlayer[4];
tCarData gGameCar[6];
tModelData model[6];

static void no_catchup(void);

/* Probably a helper or macro in the original: a plain `lagdist * 0.1f` puts the constant first. */
static inline float scalef(float a, float b) { return a * b; }

/* 0x8002ca78, size 0x64 */
void ModelThread(void)
{
    static signed char sInited;

    MyGetTime();
    if (!sInited) {
        sInited = 1;
        modelrun = 0;
        init_model_task(0);
    }
    if (ControllerUpdate() && modelrun) {
        model_iteration();
    }
    AudioMain();
}

/* 0x8002cadc, size 0xcc */
static void no_catchup(void)
{
    int i; /* r0 */
    for (i = 0; i < 6; i++) {
        if (gLink[i].owner == 0 || gLink[i].owner == 6) {
            model[i].catchup = 1.0f;
        }
    }
}

/* 0x8002cba8, size 0x1c8 */
void SetCatchup(void)
{
    signed short i; /* r10 */
    signed short high_index; /* r11 */
    float firstdist; /* f3 */
    float lagdist; /* f4 */
    float target_catchup; /* f4 */
    if ((gNumPlayers == 1 && gDifficulty < 5) || !gUseCatchup) {
        no_catchup();
        return;
    }
    high_index = -1;
    for (i = 0; i < 6; i++) {
        if (model[i].in_game && gGameCar[i].dead < 2 && (model[i].drone_type == 2 || gDifficulty == 5)) {
            if (high_index == -1) {
                high_index = i;
            } else if (gGameCar[i].distance > gGameCar[high_index].distance) {
                high_index = i;
            }
        }
    }
    firstdist = gGameCar[high_index].distance;
    for (i = 0; i < 6; i++) {
        if (model[i].in_game && gGameCar[i].dead < 2 && (model[i].drone_type == 2 || gDifficulty == 5)) {
            lagdist = firstdist - gGameCar[i].distance;
            if (lagdist > 1500.0f) {
                target_catchup = 1.1f;
            } else {
                target_catchup = 1.0f + scalef(lagdist, 0.1f) / 1500.0f;
            }
            if (gUseCatchup == 1) {
                target_catchup = 1.0f + 0.5f * (1.0f - target_catchup);
            }
            model[i].catchup = 0.03f * target_catchup + 0.97f * model[i].catchup;
        }
    }
}

