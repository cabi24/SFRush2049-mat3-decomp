/* TB_Vector.c - vector normalisation. */
#include "msl_math_inline.h"

/* 0x80050058 */
float TB_Vector2Normalize(float *v)
{
    float len = sqrtf(v[0] * v[0] + v[1] * v[1]);
    float invlen;

    if (len) {
        invlen = 1.0f / len;
        v[0] *= invlen;
        v[1] *= invlen;
    } else {
        v[0] = 1.0f;
        v[1] = 0.0f;
    }
    return len;
}

/* 0x8005018C */
float TB_Vector3Normalize(float *v)
{
    float len = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    float invlen;

    if (len) {
        invlen = 1.0f / len;
        v[0] *= invlen;
        v[1] *= invlen;
        v[2] *= invlen;
    } else {
        v[0] = 1.0f;
        v[1] = 0.0f;
        v[2] = 0.0f;
    }
    return len;
}
