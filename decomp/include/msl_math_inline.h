#ifndef MSL_MATH_INLINE_H
#define MSL_MATH_INLINE_H

/* The pieces of MSL's GameCube math.h that the game's code inlines (sqrtf, fpclassify, fabs).
   Reconstructed to match the inlined code in TB_Vector3Normalize; see writeup.md. */

extern int __float_nan[];
#define NAN (*(float *)__float_nan)

#define FP_NAN       1
#define FP_INFINITE  2
#define FP_ZERO      3
#define FP_NORMAL    4
#define FP_SUBNORMAL 5

#define __HI(x) (*(int *)&x)

inline int __fpclassifyf(float x)
{
    switch (__HI(x) & 0x7f800000) {
    case 0x7f800000:
        if (__HI(x) & 0x007fffff)
            return FP_NAN;
        else
            return FP_INFINITE;
    case 0:
        if (__HI(x) & 0x007fffff)
            return FP_SUBNORMAL;
        else
            return FP_ZERO;
    }
    return FP_NORMAL;
}

#define fpclassify(x) __fpclassifyf(x)

/* Its static constants are emitted into every file that sees this definition, so a file that
   doesn't call sqrtf (and whose original has no such constants) defines MSL_NO_SQRTF. */
#ifndef MSL_NO_SQRTF
inline float sqrtf(float x)
{
    static const double _half = .5;
    static const double _three = 3.0;

    if (x > 0.0f) {
        double guess = __frsqrte((double)x);
        guess = _half * guess * (_three - guess * guess * x);
        guess = _half * guess * (_three - guess * guess * x);
        guess = _half * guess * (_three - guess * guess * x);
        return (float)(x * guess);
    } else if (x < 0.0) {
        return NAN;
    } else if (fpclassify(x) == FP_NAN) {
        return NAN;
    }
    return x;
}
#endif

/* an inline function, not a macro: the double temporary it creates shows in register use (asincos.c) */
inline double fabs(double x) { return __fabs(x); }

#endif
