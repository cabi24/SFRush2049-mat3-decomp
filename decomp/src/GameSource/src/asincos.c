/* asincos.c - single-precision arcsine/arccosine (Cody & Waite). */
#include "rush/rush_protos.h"
#include "msl_math_inline.h"

#define PIO2 1.5707964f
#define PIO4 0.7853982f
#define EPS  0.00000000023f

#define P1 -27.368494f
#define P2 57.20823f
#define P3 -39.68886f
#define P4 10.152522f
#define P5 -0.69674575f
#define Q0 -164.21097f
#define Q1 417.14432f
#define Q2 -381.86304f
#define Q3 150.95271f
#define Q4 -23.823858f

static float farcsine(float x, int flag);

/* 0x8009c8b8 */
static float farcsine(float x, int flag)
{
    static const float a[2] = { 0.0f, PIO4 };
    static const float b[2] = { PIO2, PIO4 };
    float y;
    float g;
    float r;
    int i;

    y = fabs(x);
    i = flag;
    if (y < EPS) {
        r = y;
    } else if (y >= 1.0f) {
        r = PIO2;
    } else {
        if (y > 0.5f) {
            i = 1 - flag;
            g = ((0.5f - y) + 0.5f);
            g *= 0.5f;
            r = sqrtf(g);
            y = -(r + r);
        } else {
            g = y * y;
        }
        r = y + y * ((g * ((((P5 * g + P4) * g + P3) * g + P2) * g + P1))
                     / (((((g + Q4) * g + Q3) * g + Q2) * g + Q1) * g + Q0));
    }
    if (flag) {
        if (x < 0.0f)
            r = (b[i] + r) + b[i];
        else
            r = (a[i] - r) + a[i];
    } else {
        r = (a[i] + r) + a[i];
        if (x < 0.0f)
            r = -r;
    }
    return r;
}

/* 0x8009cac8 */
float fasin(float x)
{
    return farcsine(x, 0);
}

/* 0x8009caec */
float facos(float x)
{
    return farcsine(x, 1);
}
