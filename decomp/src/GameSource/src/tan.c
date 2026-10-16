/* tan.c - single-precision tangent, Cody & Waite's algorithm.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\tan.c; assembly: build/2049LE/asm/GameSource/src/tan.s */
#include "rush/rush_protos.h"
#define MSL_NO_SQRTF
#include "msl_math_inline.h"

#define YMAX 6746520064.0f
#define TWOOPI 0.63661975f
#define C1 1.5708008f
#define C2 -4.454455e-6f
#define EPS 2.33e-10f
#define P1 -0.1333835f
#define P2 0.0034248878f
#define P3 -0.000017861707f
#define Q1 -0.46671683f
#define Q2 0.025663832f
#define Q3 -0.00031181533f
#define Q4 0.00000049819437f

/* MSL's inline modff (math.h) */
inline float modff(float x, float *iptr)
{
    float frac;
    int i;

    switch (fpclassify(x)) {
    case FP_NAN:
    case FP_ZERO:
        *iptr = x;
        return x;
    case FP_INFINITE:
        *iptr = x;
        return 0.0f;
    case FP_SUBNORMAL:
        *iptr = 0.0f;
        return x;
    }
    i = (int)x;
    frac = x - (float)i;
    if (__HI(frac) == 0 || (__HI(x) & 0x7f800000) >= 0x4b800000) {
        *iptr = x;
        return 0.0f;
    }
    *iptr = (float)i;
    return frac;
}

/* 0x80049868, size 0x31c */
float ftan(float x)
{
    float g; /* f1 */ /* declared before f: the other order swaps their registers */
    float f; /* f8 */
    float xn; /* f0 */
    float xnum; /* f3 */
    float xden; /* f1 */
    float temp; /* f4 */
    float tt1; /* f0 */

    if ((float)fabs(x) > YMAX) {
        return 0.0f;
    }
    temp = x * TWOOPI;
    temp = modff(temp, &xn);
    if (fabs(temp) >= 0.5) {
        xn += (x < 0.0f) ? -1.0f : 1.0f;
    }
    temp = modff(x, &tt1);
    f = ((tt1 - xn * C1) + temp) - xn * C2;
    if (fabs(f) < EPS) {
        xnum = f;
        xden = 1.0f;
    } else {
        g = f * f;
        xnum = f * (g * ((P3 * g + P2) * g + P1)) + f;
        xden = (((Q4 * g + Q3) * g + Q2) * g + Q1) * g + 1.0f;
    }
    if ((int)xn & 1) {
        return xden / -xnum;
    }
    return xnum / xden;
}
