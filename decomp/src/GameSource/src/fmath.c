/* fmath.c - vector, matrix and quaternion math. Status: matched. */
#include "rush/rush_protos.h"
#include "rush/libc.h"
#include "msl_math_inline.h"

extern double sin(double);
extern double cos(double);

float unitmatrix[4][3] = {1,0,0,0,1,0,0,0,1,0,0,0};
float uvector[3] = {0,1,0};
float rvector[3] = {1,0,0};

float invsqr(float val)
{
    if (val < 0.0001f) val = 0.0001f;
    return 1.0f / sqrtf(val);
}
float magnitude(float *vec)
{
    float v[3];
    v[0] = vec[0] * vec[0];
    v[1] = vec[1] * vec[1];
    v[2] = vec[2] * vec[2];
    return sqrtf(v[0] + v[1] + v[2]);
}
float invmagnitude(float *vec)
{
    float x = vec[0] * vec[0];
    float y = vec[1] * vec[1];
    float z = vec[2] * vec[2];
    return invsqr(x + y + z);
}
float Distance(float x, float y, float z) { return sqrtf(x*x + y*y + z*z); }
float Distance2D(float x, float z) { return sqrtf(x*x + z*z); }
void direction(float *vec, float *dir)
{
    float invmag = invmagnitude(vec);
    dir[0] = vec[0] * invmag;
    dir[1] = vec[1] * invmag;
    dir[2] = vec[2] * invmag;
}
void CopyUV(float (*src)[3], float (*dst)[3])
{
    dst[0][0] = src[0][0]; dst[0][1] = src[0][1]; dst[0][2] = src[0][2];
    dst[1][0] = src[1][0]; dst[1][1] = src[1][1]; dst[1][2] = src[1][2];
    dst[2][0] = src[2][0]; dst[2][1] = src[2][1]; dst[2][2] = src[2][2];
}

void TransposeUV(float (*uv)[3])
{
    float tmp;
    tmp=uv[0][1]; uv[0][1]=uv[1][0]; uv[1][0]=tmp;
    tmp=uv[0][2]; uv[0][2]=uv[2][0]; uv[2][0]=tmp;
    tmp=uv[1][2]; uv[1][2]=uv[2][1]; uv[2][1]=tmp;
}
void ScaleUVXYZ(float (*a)[3], float (*b)[3], float sx, float sy, float sz)
{
    b[0][0] = sx * a[0][0];
    b[1][0] = a[1][0] * sy;
    b[2][0] = a[2][0] * sz;
    b[0][1] = sx * a[0][1];
    b[1][1] = a[1][1] * sy;
    b[2][1] = a[2][1] * sz;
    b[0][2] = sx * a[0][2];
    b[1][2] = a[1][2] * sy;
    b[2][2] = a[2][2] * sz;
}
void ScaleUV(float (*a)[3], float (*b)[3], float sfactor)
{
    b[0][0] = sfactor * a[0][0];
    b[0][1] = sfactor * a[0][1];
    b[0][2] = sfactor * a[0][2];
    b[1][0] = sfactor * a[1][0];
    b[1][1] = sfactor * a[1][1];
    b[1][2] = sfactor * a[1][2];
    b[2][0] = sfactor * a[2][0];
    b[2][1] = sfactor * a[2][1];
    b[2][2] = sfactor * a[2][2];
}
void WYawUV(float angle, float (*uv)[3])
{
    float ut;
    float sc[2];
    if (angle < -0.001f || angle > 0.001f) {
        sc[0] = sin(angle);
        sc[1] = cos(angle);
    ut = sc[1] * uv[0][0] + sc[0] * uv[0][2];
    uv[0][2] = sc[1] * uv[0][2] - sc[0] * uv[0][0];
    uv[0][0] = ut;
    ut = sc[1] * uv[1][0] + sc[0] * uv[1][2];
    uv[1][2] = sc[1] * uv[1][2] - sc[0] * uv[1][0];
    uv[1][0] = ut;
    ut = sc[1] * uv[2][0] + sc[0] * uv[2][2];
    uv[2][2] = sc[1] * uv[2][2] - sc[0] * uv[2][0];
    uv[2][0] = ut;
    }
}
void WPitchUV(float angle, float (*uv)[3])
{
    float ut;
    float sc[2];
    if (angle < -0.001f || angle > 0.001f) {
        sc[0] = sin(angle);
        sc[1] = cos(angle);
    ut = sc[1] * uv[0][1] - sc[0] * uv[0][2];
    uv[0][2] = sc[1] * uv[0][2] + sc[0] * uv[0][1];
    uv[0][1] = ut;
    ut = sc[1] * uv[1][1] - sc[0] * uv[1][2];
    uv[1][2] = sc[1] * uv[1][2] + sc[0] * uv[1][1];
    uv[1][1] = ut;
    ut = sc[1] * uv[2][1] - sc[0] * uv[2][2];
    uv[2][2] = sc[1] * uv[2][2] + sc[0] * uv[2][1];
    uv[2][1] = ut;
    }
}
void WRollUV(float angle, float (*uv)[3])
{
    float ut;
    float sc[2];
    if (angle < -0.001f || angle > 0.001f) {
        sc[0] = sin(angle);
        sc[1] = cos(angle);
    ut = sc[1] * uv[0][0] - sc[0] * uv[0][1];
    uv[0][1] = sc[1] * uv[0][1] + sc[0] * uv[0][0];
    uv[0][0] = ut;
    ut = sc[1] * uv[1][0] - sc[0] * uv[1][1];
    uv[1][1] = sc[1] * uv[1][1] + sc[0] * uv[1][0];
    uv[1][0] = ut;
    ut = sc[1] * uv[2][0] - sc[0] * uv[2][1];
    uv[2][1] = sc[1] * uv[2][1] + sc[0] * uv[2][0];
    uv[2][0] = ut;
    }
}
void RotateUV(float *pyr, float (*uvs)[3])
{ YawUV(pyr[1], uvs); PitchUV(pyr[0], uvs); RollUV(pyr[2], uvs); }

void YawUV(float angle, float (*uv)[3])
{
    float ut;
    float sc[2];
    if (angle < -0.0001f || angle > 0.0001f) {
        sc[0] = sin(angle);
        sc[1] = cos(angle);
    ut = sc[1] * uv[0][0] + uv[2][0] * sc[0];
    uv[2][0] = uv[2][0] * sc[1] - sc[0] * uv[0][0];
    uv[0][0] = ut;
    ut = sc[1] * uv[0][1] + uv[2][1] * sc[0];
    uv[2][1] = uv[2][1] * sc[1] - sc[0] * uv[0][1];
    uv[0][1] = ut;
    ut = sc[1] * uv[0][2] + uv[2][2] * sc[0];
    uv[2][2] = uv[2][2] * sc[1] - sc[0] * uv[0][2];
    uv[0][2] = ut;
    }
}
void PitchUV(float angle, float (*uv)[3])
{
    float ut;
    float sc[2];
    if (angle < -0.0001f || angle > 0.0001f) {
        sc[0] = sin(angle);
        sc[1] = cos(angle);
    ut = uv[1][0] * sc[1] - uv[2][0] * sc[0];
    uv[2][0] = uv[2][0] * sc[1] + uv[1][0] * sc[0];
    uv[1][0] = ut;
    ut = uv[1][1] * sc[1] - uv[2][1] * sc[0];
    uv[2][1] = uv[2][1] * sc[1] + uv[1][1] * sc[0];
    uv[1][1] = ut;
    ut = uv[1][2] * sc[1] - uv[2][2] * sc[0];
    uv[2][2] = uv[2][2] * sc[1] + uv[1][2] * sc[0];
    uv[1][2] = ut;
    }
}
void RollUV(float angle, float (*uv)[3])
{
    float ut;
    float sc[2];
    if (angle < -0.0001f || angle > 0.0001f) {
        sc[0] = sin(angle);
        sc[1] = cos(angle);
    ut = sc[1] * uv[0][0] - uv[1][0] * sc[0];
    uv[1][0] = uv[1][0] * sc[1] + sc[0] * uv[0][0];
    uv[0][0] = ut;
    ut = sc[1] * uv[0][1] - uv[1][1] * sc[0];
    uv[1][1] = uv[1][1] * sc[1] + sc[0] * uv[0][1];
    uv[0][1] = ut;
    ut = sc[1] * uv[0][2] - uv[1][2] * sc[0];
    uv[1][2] = uv[1][2] * sc[1] + sc[0] * uv[0][2];
    uv[0][2] = ut;
    }
}
void fpitch(float sint, float cost, float (*uv)[3])
{
    float ut;
    ut = uv[1][0] * cost - uv[2][0] * sint;
    uv[2][0] = uv[2][0] * cost + uv[1][0] * sint;
    uv[1][0] = ut;
    ut = uv[1][1] * cost - uv[2][1] * sint;
    uv[2][1] = uv[2][1] * cost + uv[1][1] * sint;
    uv[1][1] = ut;
    ut = uv[1][2] * cost - uv[2][2] * sint;
    uv[2][2] = uv[2][2] * cost + uv[1][2] * sint;
    uv[1][2] = ut;
}
void fyaw(float sint, float cost, float (*uv)[3])
{
    float ut;
    ut = uv[0][0] * cost + uv[2][0] * sint;
    uv[2][0] = uv[2][0] * cost - uv[0][0] * sint;
    uv[0][0] = ut;
    ut = uv[0][1] * cost + uv[2][1] * sint;
    uv[2][1] = uv[2][1] * cost - uv[0][1] * sint;
    uv[0][1] = ut;
    ut = uv[0][2] * cost + uv[2][2] * sint;
    uv[2][2] = uv[2][2] * cost - uv[0][2] * sint;
    uv[0][2] = ut;
}
void froll(float sint, float cost, float (*uv)[3])
{
    float ut;
    ut = uv[0][0] * cost - uv[1][0] * sint;
    uv[1][0] = uv[1][0] * cost + uv[0][0] * sint;
    uv[0][0] = ut;
    ut = uv[0][1] * cost - uv[1][1] * sint;
    uv[1][1] = uv[1][1] * cost + uv[0][1] * sint;
    uv[0][1] = ut;
    ut = uv[0][2] * cost - uv[1][2] * sint;
    uv[1][2] = uv[1][2] * cost + uv[0][2] * sint;
    uv[0][2] = ut;
}
void MulUVS(float (*B)[3], float (*A)[3], float (*C)[3])
{
    int i;
    for (i = 0; i < 3; i++) {
        C[i][0] = B[0][0] * A[i][0] + B[1][0] * A[i][1] + B[2][0] * A[i][2];
        C[i][1] = B[0][1] * A[i][0] + B[1][1] * A[i][1] + B[2][1] * A[i][2];
        C[i][2] = B[0][2] * A[i][0] + B[1][2] * A[i][1] + B[2][2] * A[i][2];
    }
}
void BodyVector(float *src, float *dst, float (*uvs)[3])
{
    dst[0] = src[0] * uvs[0][0] + src[1] * uvs[0][1] + src[2] * uvs[0][2];
    dst[1] = src[0] * uvs[1][0] + src[1] * uvs[1][1] + src[2] * uvs[1][2];
    dst[2] = src[0] * uvs[2][0] + src[1] * uvs[2][1] + src[2] * uvs[2][2];
}
void WorldVector(float *src, float *dst, float (*uvs)[3])
{
    dst[0] = src[0] * uvs[0][0] + src[1] * uvs[1][0] + src[2] * uvs[2][0];
    dst[1] = src[0] * uvs[0][1] + src[1] * uvs[1][1] + src[2] * uvs[2][1];
    dst[2] = src[0] * uvs[0][2] + src[1] * uvs[1][2] + src[2] * uvs[2][2];
}
void U64WorldVector(float *bodvec, float *uvec, float (*uvs)[3])
{
    uvec[0] = bodvec[0]*uvs[0][0] - bodvec[1]*uvs[1][0] - bodvec[2]*uvs[2][0];
    uvec[1] = -bodvec[0]*uvs[0][1] + bodvec[1]*uvs[1][1] + bodvec[2]*uvs[2][1];
    uvec[2] = -bodvec[0]*uvs[0][2] + bodvec[1]*uvs[1][2] + bodvec[2]*uvs[2][2];
}
float NormalVector(float *v)
{
    float mag = Distance(v[0],v[1],v[2]);
    float invmag;
    if (mag <= 0.00001f) return 0.0f;
    invmag = 1.0f / mag;
    v[0] *= invmag; v[1] *= invmag; v[2] *= invmag;
    return mag;
}



void CreateYPRMatrix(float (*mat)[3], float *pyr)
{
    float sc[6];
    float temp;
    sc[0] = cos(pyr[0]);
    sc[1] = sin(pyr[0]);
    sc[2] = cos(pyr[1]);
    sc[3] = sin(pyr[1]);
    sc[4] = cos(pyr[2]);
    sc[5] = sin(pyr[2]);
    temp = sc[3] * sc[1];
    mat[0][0] = sc[2]*sc[4] - temp*sc[5];
    mat[1][0] = sc[2]*sc[5] + temp*sc[4];
    mat[2][0] = -sc[3]*sc[0];
    mat[0][1] = -sc[0]*sc[5];
    mat[1][1] = sc[0]*sc[4];
    mat[2][1] = sc[1];
    temp = sc[2]*sc[1];
    mat[0][2] = sc[3]*sc[4] + temp*sc[5];
    mat[1][2] = sc[3]*sc[5] - temp*sc[4];
    mat[2][2] = sc[2]*sc[0];
}

void CreateQuatMat(float *mat, float *q, int normal)
{
    float s, xs, ys, zs, Nq, xx, yy, zz, xy, wz, yz, wx, xz, wy;
    if (!normal) {
        float x=q[0]*q[0], y=q[1]*q[1], z=q[2]*q[2], w=q[3]*q[3];
        Nq = x+y+z+w;
        s = Nq > 0.0f ? 2.0f/Nq : 0.0f;
    } else s = 2.0f;
    xs = q[0]*s; ys=q[1]*s; zs=q[2]*s;
    xx=q[0]*xs; yy=q[1]*ys; zz=q[2]*zs;
    mat[0]=1.0f-(yy+zz); mat[4]=1.0f-(xx+zz); mat[8]=1.0f-(xx+yy);
    xy=q[0]*ys; wz=q[3]*zs;
    mat[1]=-(xy+wz); mat[3]=-(xy-wz);
    yz=q[1]*zs; wx=q[3]*xs;
    mat[5]=-(yz+wx); mat[7]=-(yz-wx);
    xz=q[0]*zs; wy=q[3]*ys;
    mat[2]=xz-wy; mat[6]=xz+wy;
}

void InterpQuats(float frac, float *q1, float *q2, float *qr)
{
    float dp, theta, isint, X1, X2;
    int i;
    if (frac < 0.001f) {
        qr[0]=q1[0]; qr[1]=q1[1]; qr[2]=q1[2]; qr[3]=q1[3];
    } else if (frac > 0.999f) {
        qr[0]=q2[0]; qr[1]=q2[1]; qr[2]=q2[2]; qr[3]=q2[3];
    } else {
        dp=0.999f*(q1[0]*q2[0]+q1[1]*q2[1]+q1[2]*q2[2]+q1[3]*q2[3]);
        if (dp < 0.0f) {
            dp=-dp;
            qr[0]=-q2[0]; qr[1]=-q2[1]; qr[2]=-q2[2]; qr[3]=-q2[3];
        } else {
            qr[0]=q2[0]; qr[1]=q2[1]; qr[2]=q2[2]; qr[3]=q2[3];
        }
        if (dp < 0.98f) {
            theta=facos(dp);
            isint=1.0f/(float)sin(theta);
            X1=isint*(float)sin((1.0f-frac)*theta);
            X2=isint*(float)sin(frac*theta);
            qr[0]=X1*q1[0]+X2*qr[0]; qr[1]=X1*q1[1]+X2*qr[1];
            qr[2]=X1*q1[2]+X2*qr[2]; qr[3]=X1*q1[3]+X2*qr[3];
        } else {
            for (i=0;i<4;i++) {
                float delta=qr[i]-q1[i];
                if (delta > 1.0f) delta-=2.0f;
                if (delta < -1.0f) delta+=2.0f;
                qr[i]=q1[i]+frac*delta;
                if (qr[i] > 1.0f) qr[i]-=2.0f;
                else if (qr[i] < -1.0f) qr[i]+=2.0f;
            }
        }
    }
}

void ExtractYaw(float (*mat)[3], float *yaw)
{
    if (__fabs(mat[2][0]) < (double)0.000001f && __fabs(mat[2][2]) < (double)0.000001f)
        *yaw=fatan2(mat[0][2],mat[0][0]);
    else *yaw=fatan2(-mat[2][0],mat[2][2]);
}

float Random(float max)
{
    return max * (rand() & 0x7fff) / 32768.0f;
}
