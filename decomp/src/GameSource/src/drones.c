/* drones.c - AI "drone" cars. Partial: functions not listed here are still unmatched. */
#include "msl_math_inline.h"

extern void SetCatchup(void);

float old_drone_scale[6];
float old_boost[6];

static void _AssignDrones(void);

/* 0x800114A4, lines 56-60 */
void InitDrones(void)
{
    int i;

    for (i = 0; i < 6; i++) {
        old_drone_scale[i] = 1.0f;
        old_boost[i] = 1.0f;
    }
}

/* 0x800114E4, lines 73-78 */
void DoDrones(void)
{
    SetCatchup();
    _AssignDrones();
}

/* TODO: _AssignDrones (0x80011508, 0x1194 bytes, lines 91-499) */

/* 0x8001269C, lines 522-544 */
static float _LinearInterp(float in_bound1, float in_bound2, float out_bound1, float out_bound2, float input)
{
    float slope;

    if (in_bound1 < in_bound2) {
        if (input < in_bound1)
            return out_bound1;
        if (input > in_bound2)
            return out_bound2;
    } else {
        if (input < in_bound2)
            return out_bound2;
        if (input > in_bound1)
            return out_bound1;
    }

    if (fabs(in_bound1 - in_bound2) < 0.001f)
        return (out_bound1 + out_bound2) * 0.5f;

    slope = (out_bound1 - out_bound2) / (in_bound1 - in_bound2);
    return slope * input + (out_bound1 - slope * in_bound1);
}
