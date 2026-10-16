/* fake.c - matrix helpers. */
extern float appGetTime(void);
extern void *memcpy(void *, const void *, unsigned long);

float MTX[4][4];
float ObjectMTX[4][4];
extern float MatrixStack[3600];
extern float MatrixStack2[3600];

char OCRamData[1024];
float MatrixStack[3600];
float MatrixStack2[3600];
float *CurrentPoint = MatrixStack;
float *CurrentPoint2 = MatrixStack2;

unsigned int MyGetTime(void)
{
    float ms = appGetTime() * 1000.0f;
    return (int)ms;
}

unsigned int MyCountToMicro(unsigned int t)
{
    return t;
}

void MtxTranspose3x3(void)
{
    float m[3][3];
    int i;
    float *p = &MTX[0][0];
    for (i = 0; i != 3; i++, p++) {
        m[i][0] = p[0];
        m[i][1] = p[4];
        m[i][2] = p[8];
    }
    for (i = 0; i != 3; i++) {
        MTX[i][0] = m[i][0];
        MTX[i][1] = m[i][1];
        MTX[i][2] = m[i][2];
    }
}

/* 0x800128E8 */
void MtxMulVec3(float *in, float *out)
{
    out[0] = in[0] * MTX[0][0] + in[1] * MTX[1][0] + in[2] * MTX[2][0];
    out[1] = in[0] * MTX[0][1] + in[1] * MTX[1][1] + in[2] * MTX[2][1];
    out[2] = in[0] * MTX[0][2] + in[1] * MTX[1][2] + in[2] * MTX[2][2];
}

void MtxMulVec4(float *in, float *out)
{
    out[0] = in[0] * MTX[0][0] + in[1] * MTX[1][0] + in[2] * MTX[2][0] + MTX[3][0];
    out[1] = in[0] * MTX[0][1] + in[1] * MTX[1][1] + in[2] * MTX[2][1] + MTX[3][1];
    out[2] = in[0] * MTX[0][2] + in[1] * MTX[1][2] + in[2] * MTX[2][2] + MTX[3][2];
}

void MyMtxMulVec4(float mtx[3][3], float *in, float *out)
{
    out[0] = in[0] * mtx[0][0] + in[1] * mtx[0][1] + in[2] * mtx[0][2];
    out[1] = in[0] * mtx[1][0] + in[1] * mtx[1][1] + in[2] * mtx[1][2];
    out[2] = in[0] * mtx[2][0] + in[1] * mtx[2][1] + in[2] * mtx[2][2];
}

void LocalToWorld(float *in, float *out)
{
    out[0] = in[0] * ObjectMTX[0][0] + in[1] * ObjectMTX[1][0] + in[2] * ObjectMTX[2][0] + ObjectMTX[3][0];
    out[1] = in[0] * ObjectMTX[0][1] + in[1] * ObjectMTX[1][1] + in[2] * ObjectMTX[2][1] + ObjectMTX[3][1];
    out[2] = in[0] * ObjectMTX[0][2] + in[1] * ObjectMTX[1][2] + in[2] * ObjectMTX[2][2] + ObjectMTX[3][2];
}

void MtxMulMat(float *mat)
{
    int i;
    float m[4][4];
    m[0][0] = mat[0] * MTX[0][0] + mat[1] * MTX[1][0] + mat[2] * MTX[2][0];
    m[0][1] = mat[0] * MTX[0][1] + mat[1] * MTX[1][1] + mat[2] * MTX[2][1];
    m[0][2] = mat[0] * MTX[0][2] + mat[1] * MTX[1][2] + mat[2] * MTX[2][2];
    m[0][3] = 0.0f;
    m[1][0] = mat[3] * MTX[0][0] + mat[4] * MTX[1][0] + mat[5] * MTX[2][0];
    m[1][1] = mat[3] * MTX[0][1] + mat[4] * MTX[1][1] + mat[5] * MTX[2][1];
    m[1][2] = mat[3] * MTX[0][2] + mat[4] * MTX[1][2] + mat[5] * MTX[2][2];
    m[1][3] = 0.0f;
    m[2][0] = mat[6] * MTX[0][0] + mat[7] * MTX[1][0] + mat[8] * MTX[2][0];
    m[2][1] = mat[6] * MTX[0][1] + mat[7] * MTX[1][1] + mat[8] * MTX[2][1];
    m[2][2] = mat[6] * MTX[0][2] + mat[7] * MTX[1][2] + mat[8] * MTX[2][2];
    m[2][3] = 0.0f;
    m[3][0] = mat[9] * MTX[0][0] + mat[10] * MTX[1][0] + mat[11] * MTX[2][0] + MTX[3][0];
    m[3][1] = mat[9] * MTX[0][1] + mat[10] * MTX[1][1] + mat[11] * MTX[2][1] + MTX[3][1];
    m[3][2] = mat[9] * MTX[0][2] + mat[10] * MTX[1][2] + mat[11] * MTX[2][2] + MTX[3][2];
    m[3][3] = 1.0f;
    for (i = 0; i != 4; i++) {
        MTX[i][0] = m[i][0];
        MTX[i][1] = m[i][1];
        MTX[i][2] = m[i][2];
        MTX[i][3] = m[i][3];
    }
    m[0][0] = mat[0] * ObjectMTX[0][0] + mat[1] * ObjectMTX[1][0] + mat[2] * ObjectMTX[2][0];
    m[0][1] = mat[0] * ObjectMTX[0][1] + mat[1] * ObjectMTX[1][1] + mat[2] * ObjectMTX[2][1];
    m[0][2] = mat[0] * ObjectMTX[0][2] + mat[1] * ObjectMTX[1][2] + mat[2] * ObjectMTX[2][2];
    m[0][3] = 0.0f;
    m[1][0] = mat[3] * ObjectMTX[0][0] + mat[4] * ObjectMTX[1][0] + mat[5] * ObjectMTX[2][0];
    m[1][1] = mat[3] * ObjectMTX[0][1] + mat[4] * ObjectMTX[1][1] + mat[5] * ObjectMTX[2][1];
    m[1][2] = mat[3] * ObjectMTX[0][2] + mat[4] * ObjectMTX[1][2] + mat[5] * ObjectMTX[2][2];
    m[1][3] = 0.0f;
    m[2][0] = mat[6] * ObjectMTX[0][0] + mat[7] * ObjectMTX[1][0] + mat[8] * ObjectMTX[2][0];
    m[2][1] = mat[6] * ObjectMTX[0][1] + mat[7] * ObjectMTX[1][1] + mat[8] * ObjectMTX[2][1];
    m[2][2] = mat[6] * ObjectMTX[0][2] + mat[7] * ObjectMTX[1][2] + mat[8] * ObjectMTX[2][2];
    m[2][3] = 0.0f;
    m[3][0] = mat[9] * ObjectMTX[0][0] + mat[10] * ObjectMTX[1][0] + mat[11] * ObjectMTX[2][0] + ObjectMTX[3][0];
    m[3][1] = mat[9] * ObjectMTX[0][1] + mat[10] * ObjectMTX[1][1] + mat[11] * ObjectMTX[2][1] + ObjectMTX[3][1];
    m[3][2] = mat[9] * ObjectMTX[0][2] + mat[10] * ObjectMTX[1][2] + mat[11] * ObjectMTX[2][2] + ObjectMTX[3][2];
    m[3][3] = 1.0f;
    for (i = 0; i != 4; i++) {
        ObjectMTX[i][0] = m[i][0];
        ObjectMTX[i][1] = m[i][1];
        ObjectMTX[i][2] = m[i][2];
        ObjectMTX[i][3] = m[i][3];
    }
}

void MtxMulMatTrans(float *mat, float *pos)
{
    int i;
    float m[4][4];
    m[0][0] = mat[0] * MTX[0][0] + mat[1] * MTX[1][0] + mat[2] * MTX[2][0] + pos[0] * MTX[3][0];
    m[0][1] = mat[0] * MTX[0][1] + mat[1] * MTX[1][1] + mat[2] * MTX[2][1] + pos[0] * MTX[3][1];
    m[0][2] = mat[0] * MTX[0][2] + mat[1] * MTX[1][2] + mat[2] * MTX[2][2] + pos[0] * MTX[3][2];
    m[0][3] = 0.0f;
    m[1][0] = mat[3] * MTX[0][0] + mat[4] * MTX[1][0] + mat[5] * MTX[2][0] + pos[1] * MTX[3][0];
    m[1][1] = mat[3] * MTX[0][1] + mat[4] * MTX[1][1] + mat[5] * MTX[2][1] + pos[1] * MTX[3][1];
    m[1][2] = mat[3] * MTX[0][2] + mat[4] * MTX[1][2] + mat[5] * MTX[2][2] + pos[1] * MTX[3][2];
    m[1][3] = 0.0f;
    m[2][0] = mat[6] * MTX[0][0] + mat[7] * MTX[1][0] + mat[8] * MTX[2][0] + pos[2] * MTX[3][0];
    m[2][1] = mat[6] * MTX[0][1] + mat[7] * MTX[1][1] + mat[8] * MTX[2][1] + pos[2] * MTX[3][1];
    m[2][2] = mat[6] * MTX[0][2] + mat[7] * MTX[1][2] + mat[8] * MTX[2][2] + pos[2] * MTX[3][2];
    m[2][3] = 0.0f;
    m[3][0] = mat[9] * MTX[0][0] + mat[10] * MTX[1][0] + mat[11] * MTX[2][0] + MTX[3][0];
    m[3][1] = mat[9] * MTX[0][1] + mat[10] * MTX[1][1] + mat[11] * MTX[2][1] + MTX[3][1];
    m[3][2] = mat[9] * MTX[0][2] + mat[10] * MTX[1][2] + mat[11] * MTX[2][2] + MTX[3][2];
    m[3][3] = 1.0f;
    for (i = 0; i != 4; i++) {
        MTX[i][0] = m[i][0];
        MTX[i][1] = m[i][1];
        MTX[i][2] = m[i][2];
        MTX[i][3] = m[i][3];
    }
    m[0][0] = mat[0] * ObjectMTX[0][0] + mat[1] * ObjectMTX[1][0] + mat[2] * ObjectMTX[2][0] + pos[0] * ObjectMTX[3][0];
    m[0][1] = mat[0] * ObjectMTX[0][1] + mat[1] * ObjectMTX[1][1] + mat[2] * ObjectMTX[2][1] + pos[0] * ObjectMTX[3][1];
    m[0][2] = mat[0] * ObjectMTX[0][2] + mat[1] * ObjectMTX[1][2] + mat[2] * ObjectMTX[2][2] + pos[0] * ObjectMTX[3][2];
    m[0][3] = 0.0f;
    m[1][0] = mat[3] * ObjectMTX[0][0] + mat[4] * ObjectMTX[1][0] + mat[5] * ObjectMTX[2][0] + pos[1] * ObjectMTX[3][0];
    m[1][1] = mat[3] * ObjectMTX[0][1] + mat[4] * ObjectMTX[1][1] + mat[5] * ObjectMTX[2][1] + pos[1] * ObjectMTX[3][1];
    m[1][2] = mat[3] * ObjectMTX[0][2] + mat[4] * ObjectMTX[1][2] + mat[5] * ObjectMTX[2][2] + pos[1] * ObjectMTX[3][2];
    m[1][3] = 0.0f;
    m[2][0] = mat[6] * ObjectMTX[0][0] + mat[7] * ObjectMTX[1][0] + mat[8] * ObjectMTX[2][0] + pos[2] * ObjectMTX[3][0];
    m[2][1] = mat[6] * ObjectMTX[0][1] + mat[7] * ObjectMTX[1][1] + mat[8] * ObjectMTX[2][1] + pos[2] * ObjectMTX[3][1];
    m[2][2] = mat[6] * ObjectMTX[0][2] + mat[7] * ObjectMTX[1][2] + mat[8] * ObjectMTX[2][2] + pos[2] * ObjectMTX[3][2];
    m[2][3] = 0.0f;
    m[3][0] = mat[9] * ObjectMTX[0][0] + mat[10] * ObjectMTX[1][0] + mat[11] * ObjectMTX[2][0] + ObjectMTX[3][0];
    m[3][1] = mat[9] * ObjectMTX[0][1] + mat[10] * ObjectMTX[1][1] + mat[11] * ObjectMTX[2][1] + ObjectMTX[3][1];
    m[3][2] = mat[9] * ObjectMTX[0][2] + mat[10] * ObjectMTX[1][2] + mat[11] * ObjectMTX[2][2] + ObjectMTX[3][2];
    m[3][3] = 1.0f;
    for (i = 0; i != 4; i++) {
        ObjectMTX[i][0] = m[i][0];
        ObjectMTX[i][1] = m[i][1];
        ObjectMTX[i][2] = m[i][2];
        ObjectMTX[i][3] = m[i][3];
    }
}

void MtxSave(void)
{
    memcpy(CurrentPoint, MTX, 64);
    CurrentPoint += 16;
    memcpy(CurrentPoint2, ObjectMTX, 64);
    CurrentPoint2 += 16;
}

void MtxLoad(void)
{
    memcpy(MTX, CurrentPoint - 16, 64);
    CurrentPoint -= 16;
    memcpy(ObjectMTX, CurrentPoint2 - 16, 64);
    CurrentPoint2 -= 16;
}

void CamMtxLoad(float *mtx)
{
    MTX[0][0] = mtx[0];
    MTX[1][0] = mtx[1];
    MTX[2][0] = mtx[2];
    MTX[0][3] = 0.0f;
    MTX[0][1] = mtx[3];
    MTX[1][1] = mtx[4];
    MTX[2][1] = mtx[5];
    MTX[1][3] = 0.0f;
    MTX[0][2] = mtx[6];
    MTX[1][2] = mtx[7];
    MTX[2][2] = mtx[8];
    MTX[2][3] = 0.0f;
    MTX[3][0] = 0.0f;
    MTX[3][1] = 0.0f;
    MTX[3][2] = 0.0f;
    MTX[3][3] = 1.0f;
    ObjectMTX[0][0] = 1.0f;
    ObjectMTX[1][0] = 0.0f;
    ObjectMTX[2][0] = 0.0f;
    ObjectMTX[0][3] = 0.0f;
    ObjectMTX[0][1] = 0.0f;
    ObjectMTX[1][1] = 1.0f;
    ObjectMTX[2][1] = 0.0f;
    ObjectMTX[1][3] = 0.0f;
    ObjectMTX[0][2] = 0.0f;
    ObjectMTX[1][2] = 0.0f;
    ObjectMTX[2][2] = 1.0f;
    ObjectMTX[2][3] = 0.0f;
    ObjectMTX[3][0] = 0.0f;
    ObjectMTX[3][1] = 0.0f;
    ObjectMTX[3][2] = 0.0f;
    ObjectMTX[3][3] = 1.0f;
}

void CamMtxLoadMirror(float *mtx)
{
    MTX[0][0] = -mtx[0];
    MTX[1][0] = -mtx[1];
    MTX[2][0] = -mtx[2];
    MTX[0][3] = 0.0f;
    MTX[0][1] = mtx[3];
    MTX[1][1] = mtx[4];
    MTX[2][1] = mtx[5];
    MTX[1][3] = 0.0f;
    MTX[0][2] = mtx[6];
    MTX[1][2] = mtx[7];
    MTX[2][2] = mtx[8];
    MTX[2][3] = 0.0f;
    MTX[3][0] = 0.0f;
    MTX[3][1] = 0.0f;
    MTX[3][2] = 0.0f;
    MTX[3][3] = 1.0f;
    ObjectMTX[0][0] = 1.0f;
    ObjectMTX[1][0] = 0.0f;
    ObjectMTX[2][0] = 0.0f;
    ObjectMTX[0][3] = 0.0f;
    ObjectMTX[0][1] = 0.0f;
    ObjectMTX[1][1] = 1.0f;
    ObjectMTX[2][1] = 0.0f;
    ObjectMTX[1][3] = 0.0f;
    ObjectMTX[0][2] = 0.0f;
    ObjectMTX[1][2] = 0.0f;
    ObjectMTX[2][2] = 1.0f;
    ObjectMTX[2][3] = 0.0f;
    ObjectMTX[3][0] = 0.0f;
    ObjectMTX[3][1] = 0.0f;
    ObjectMTX[3][2] = 0.0f;
    ObjectMTX[3][3] = 1.0f;
}

void CamMtxLoadUpsideDown(float *mtx)
{
    MTX[0][0] = -mtx[0];
    MTX[1][0] = -mtx[1];
    MTX[2][0] = -mtx[2];
    MTX[0][3] = 0.0f;
    MTX[0][1] = -mtx[3];
    MTX[1][1] = -mtx[4];
    MTX[2][1] = -mtx[5];
    MTX[1][3] = 0.0f;
    MTX[0][2] = mtx[6];
    MTX[1][2] = mtx[7];
    MTX[2][2] = mtx[8];
    MTX[2][3] = 0.0f;
    MTX[3][0] = 0.0f;
    MTX[3][1] = 0.0f;
    MTX[3][2] = 0.0f;
    MTX[3][3] = 1.0f;
    ObjectMTX[0][0] = 1.0f;
    ObjectMTX[1][0] = 0.0f;
    ObjectMTX[2][0] = 0.0f;
    ObjectMTX[0][3] = 0.0f;
    ObjectMTX[0][1] = 0.0f;
    ObjectMTX[1][1] = 1.0f;
    ObjectMTX[2][1] = 0.0f;
    ObjectMTX[1][3] = 0.0f;
    ObjectMTX[0][2] = 0.0f;
    ObjectMTX[1][2] = 0.0f;
    ObjectMTX[2][2] = 1.0f;
    ObjectMTX[2][3] = 0.0f;
    ObjectMTX[3][0] = 0.0f;
    ObjectMTX[3][1] = 0.0f;
    ObjectMTX[3][2] = 0.0f;
    ObjectMTX[3][3] = 1.0f;
}

void CamMtxLoadUpsideDownMirror(float *mtx)
{
    MTX[0][0] = mtx[0];
    MTX[1][0] = mtx[1];
    MTX[2][0] = mtx[2];
    MTX[0][3] = 0.0f;
    MTX[0][1] = -mtx[3];
    MTX[1][1] = -mtx[4];
    MTX[2][1] = -mtx[5];
    MTX[1][3] = 0.0f;
    MTX[0][2] = mtx[6];
    MTX[1][2] = mtx[7];
    MTX[2][2] = mtx[8];
    MTX[2][3] = 0.0f;
    MTX[3][0] = 0.0f;
    MTX[3][1] = 0.0f;
    MTX[3][2] = 0.0f;
    MTX[3][3] = 1.0f;
    ObjectMTX[0][0] = 1.0f;
    ObjectMTX[1][0] = 0.0f;
    ObjectMTX[2][0] = 0.0f;
    ObjectMTX[0][3] = 0.0f;
    ObjectMTX[0][1] = 0.0f;
    ObjectMTX[1][1] = 1.0f;
    ObjectMTX[2][1] = 0.0f;
    ObjectMTX[1][3] = 0.0f;
    ObjectMTX[0][2] = 0.0f;
    ObjectMTX[1][2] = 0.0f;
    ObjectMTX[2][2] = 1.0f;
    ObjectMTX[2][3] = 0.0f;
    ObjectMTX[3][0] = 0.0f;
    ObjectMTX[3][1] = 0.0f;
    ObjectMTX[3][2] = 0.0f;
    ObjectMTX[3][3] = 1.0f;
}
