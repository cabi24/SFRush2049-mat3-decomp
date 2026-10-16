/* ngcMtx.c - PS2 VU0 matrix calls (column-major) reimplemented in C and on top of the Dolphin PSMTX44 functions.
   Original: D:\rush_ngc\Rush_NGC\NGC\ngcMtx.c; assembly: build/2049LE/asm/NGC/ngcMtx.s */
#include "rush/rush_protos.h"

float Mtx4Identity[4][4] = {
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
};

/* 0x800ac158, size 0x1c0 */
void sceVu0MulMatrix(float (*mr)[4], float (*m1)[4], float (*m2)[4])
{
    int r; /* r6 */
    int c; /* r0 */

    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            mr[c][r] = m1[0][r] * m2[c][0] + m1[1][r] * m2[c][1] + m1[2][r] * m2[c][2] + m1[3][r] * m2[c][3];
        }
    }
}

/* 0x800ac318, size 0x88 */
void sceVu0UnitMatrix(float (*m)[4])
{
    int r; /* r0 */

    for (r = 0; r < 4; r++) {
        m[0][r] = Mtx4Identity[0][r];
        m[1][r] = Mtx4Identity[1][r];
        m[2][r] = Mtx4Identity[2][r];
        m[3][r] = Mtx4Identity[3][r];
    }
}

/* 0x800ac3a0, size 0x84 */
void sceVu0CopyMatrix(float (*mdest)[4], float (*msrc)[4])
{
    int r; /* r0 */

    for (r = 0; r < 4; r++) {
        mdest[0][r] = msrc[r][0];
        mdest[1][r] = msrc[r][1];
        mdest[2][r] = msrc[r][2];
        mdest[3][r] = msrc[r][3];
    }
}

/* 0x800ac424, size 0x2c8 */
void sceVu0InversMatrix(float (*d)[4], float (*s)[4])
{
    float temp; /* f11 */
    float scalar; /* f2 */

    temp = s[0][0] * s[1][1] * s[2][2];
    temp -= s[0][0] * s[1][2] * s[2][1];
    temp -= s[0][1] * s[1][0] * s[2][2];
    temp += s[0][1] * s[1][2] * s[2][0];
    temp += s[0][2] * s[1][0] * s[2][1];
    temp -= s[0][2] * s[1][1] * s[2][0];
    scalar = 1.0f / temp;
    d[0][0] = scalar * (s[1][1] * s[2][2] - s[1][2] * s[2][1]);
    d[0][1] = scalar * (-s[0][1] * s[2][2] + s[0][2] * s[2][1]);
    d[0][2] = scalar * (s[0][1] * s[1][2] - s[0][2] * s[1][1]);
    d[0][3] = 0.0f;
    d[1][0] = scalar * (-s[1][0] * s[2][2] + s[1][2] * s[2][0]);
    d[1][1] = scalar * (s[0][0] * s[2][2] - s[0][2] * s[2][0]);
    d[1][2] = scalar * (-s[0][0] * s[1][2] + s[0][2] * s[1][0]);
    d[1][3] = 0.0f;
    d[2][0] = scalar * (s[1][0] * s[2][1] - s[1][1] * s[2][0]);
    d[2][1] = scalar * (-s[0][0] * s[2][1] + s[0][1] * s[2][0]);
    d[2][2] = scalar * (s[0][0] * s[1][1] - s[0][1] * s[1][0]);
    d[2][3] = 0.0f;
    d[3][0] = -s[1][0] * s[2][1] * s[3][2] + s[1][0] * s[3][1] * s[2][2] + s[1][1] * s[2][0] * s[3][2] -
              s[1][1] * s[3][0] * s[2][2] - s[1][2] * s[2][0] * s[3][1] + s[1][2] * s[3][0] * s[2][1];
    d[3][0] *= scalar;
    d[3][1] = s[0][0] * s[2][1] * s[3][2] - s[0][0] * s[3][1] * s[2][2] - s[0][1] * s[2][0] * s[3][2] +
              s[0][1] * s[3][0] * s[2][2] + s[0][2] * s[2][0] * s[3][1] - s[0][2] * s[3][0] * s[2][1];
    d[3][1] *= scalar;
    d[3][2] = -s[0][0] * s[1][1] * s[3][2] + s[0][0] * s[3][1] * s[1][2] + s[0][1] * s[1][0] * s[3][2] -
              s[0][1] * s[3][0] * s[1][2] - s[0][2] * s[1][0] * s[3][1] + s[0][2] * s[3][0] * s[1][1];
    d[3][2] *= scalar;
    d[3][3] = 1.0f;
}

/* 0x800ac6ec, size 0xdc */
void sceVu0TransMatrix(float (*unused)[4], float (*s)[4], float *vec)
{
    int i; /* r0 */
    float in[4][4]; /* stack */

    for (i = 0; i < 4; i++) {
        in[i][0] = s[0][i];
        in[i][1] = s[1][i];
        in[i][2] = s[2][i];
        in[i][3] = s[3][i];
    }
    PSMTX44Trans(in, vec[0], vec[1], vec[2]);
    s[3][0] = in[0][3];
    s[3][1] = in[1][3];
    s[3][2] = in[2][3];
    s[3][3] = in[3][3];
}

/* 0x800ac7c8, size 0xd4 */
void sceVu0ApplyMatrix(float *out, float (*s)[4], float *in)
{
    out[0] = in[0] * s[0][0] + in[1] * s[1][0] + in[2] * s[2][0] + in[3] * s[3][0];
    out[1] = in[0] * s[0][1] + in[1] * s[1][1] + in[2] * s[2][1] + in[3] * s[3][1];
    out[2] = in[0] * s[0][2] + in[1] * s[1][2] + in[2] * s[2][2] + in[3] * s[3][2];
    out[3] = in[0] * s[0][3] + in[1] * s[1][3] + in[2] * s[2][3] + in[3] * s[3][3];
}

/* 0x800ac89c, size 0x134 */
void sceVu0RotMatrixX(float (*out)[4], float (*in)[4], float angle)
{
    int i; /* r0 */
    float mat[4][4]; /* stack */

    mat[0][0] = in[0][0];
    mat[0][1] = in[1][0];
    mat[0][2] = in[2][0];
    mat[0][3] = in[3][0];
    mat[1][0] = in[0][1];
    mat[1][1] = in[1][1];
    mat[1][2] = in[2][1];
    mat[1][3] = in[3][1];
    mat[2][0] = in[0][2];
    mat[2][1] = in[1][2];
    mat[2][2] = in[2][2];
    mat[2][3] = in[3][2];
    mat[3][0] = in[0][3];
    mat[3][1] = in[1][3];
    mat[3][2] = in[2][3];
    mat[3][3] = in[3][3];
    PSMTX44RotRad(mat, 'X', angle);
    for (i = 0; i < 4; i++) {
        out[i][0] = mat[0][i];
        out[i][1] = mat[1][i];
        out[i][2] = mat[2][i];
        out[i][3] = mat[3][i];
    }
}

/* 0x800ac9d0, size 0x134 */
void sceVu0RotMatrixY(float (*out)[4], float (*in)[4], float angle)
{
    int i; /* r0 */
    float mat[4][4]; /* stack */

    mat[0][0] = in[0][0];
    mat[0][1] = in[1][0];
    mat[0][2] = in[2][0];
    mat[0][3] = in[3][0];
    mat[1][0] = in[0][1];
    mat[1][1] = in[1][1];
    mat[1][2] = in[2][1];
    mat[1][3] = in[3][1];
    mat[2][0] = in[0][2];
    mat[2][1] = in[1][2];
    mat[2][2] = in[2][2];
    mat[2][3] = in[3][2];
    mat[3][0] = in[0][3];
    mat[3][1] = in[1][3];
    mat[3][2] = in[2][3];
    mat[3][3] = in[3][3];
    PSMTX44RotRad(mat, 'Y', angle);
    for (i = 0; i < 4; i++) {
        out[i][0] = mat[0][i];
        out[i][1] = mat[1][i];
        out[i][2] = mat[2][i];
        out[i][3] = mat[3][i];
    }
}

/* 0x800acb04, size 0x134 */
void sceVu0RotMatrixZ(float (*out)[4], float (*in)[4], float angle)
{
    int i; /* r0 */
    float mat[4][4]; /* stack */

    mat[0][0] = in[0][0];
    mat[0][1] = in[1][0];
    mat[0][2] = in[2][0];
    mat[0][3] = in[3][0];
    mat[1][0] = in[0][1];
    mat[1][1] = in[1][1];
    mat[1][2] = in[2][1];
    mat[1][3] = in[3][1];
    mat[2][0] = in[0][2];
    mat[2][1] = in[1][2];
    mat[2][2] = in[2][2];
    mat[2][3] = in[3][2];
    mat[3][0] = in[0][3];
    mat[3][1] = in[1][3];
    mat[3][2] = in[2][3];
    mat[3][3] = in[3][3];
    PSMTX44RotRad(mat, 'Z', angle);
    for (i = 0; i < 4; i++) {
        out[i][0] = mat[0][i];
        out[i][1] = mat[1][i];
        out[i][2] = mat[2][i];
        out[i][3] = mat[3][i];
    }
}

