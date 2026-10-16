/* stree.c - spatial tree. Partial: functions not listed here are still unmatched. */
/* Local ABI correction: original call sites pass an unused surface in r3.
 * Keep the recovered shared header immutable. */
#define fPathInterp fPathInterp_dwarf_declaration
#define fHalfPipeInterp fHalfPipeInterp_dwarf_declaration
#include "rush/rush_protos.h"
#undef fHalfPipeInterp
#include "msl_math_inline.h"
#undef fPathInterp

streetyp *stree;
surftyp *surf;
sverttyp *svert;
pathtyp *path;
unsigned char *slist;
unsigned char *vlist;
savetyp *save;
unsigned short savenum;
/* Preserve the original local helper binding. */
static signed short PointInSurf(surftyp *, float *, float *, float *, float *, unsigned short *, float (*)[3], float);

void *gRoadDat;
stheadertyp *gRoadHead;

/* 0x80043904 */
void InitSTree(void)
{
    stheadertyp *hp;
    int i;
    pathtyp *swapPATHTYP;
    streetyp *swapTREE;
    gRoadHead = hp = (stheadertyp *)gRoadDat;
    if (g_bIfReload) _Disenable_swap();
    else _Enable_swap();
    _swap_x16(&gRoadHead->pnum);
    _swap_x16(&gRoadHead->tnum);
    _swap_x16(&gRoadHead->snum);
    _swap_x16(&gRoadHead->vnum);
    _swap_x16(&gRoadHead->savenum);
    _swap_x16(&gRoadHead->vlnum);
    _swap_x32(&gRoadHead->slnum);
    path = (pathtyp *)(hp + 1);
    for (i = 0, swapPATHTYP = path; i < hp->pnum; swapPATHTYP++, i++) {
        _swap_x32(&swapPATHTYP->pos[0]);
        _swap_x32(&swapPATHTYP->pos[1]);
        _swap_x32(&swapPATHTYP->pos[2]);
        _swap_x32(&swapPATHTYP->uvs[0][0]);
        _swap_x32(&swapPATHTYP->uvs[0][1]);
        _swap_x32(&swapPATHTYP->uvs[0][2]);
        _swap_x32(&swapPATHTYP->uvs[1][0]);
        _swap_x32(&swapPATHTYP->uvs[1][1]);
        _swap_x32(&swapPATHTYP->uvs[1][2]);
        _swap_x32(&swapPATHTYP->uvs[2][0]);
        _swap_x32(&swapPATHTYP->uvs[2][1]);
        _swap_x32(&swapPATHTYP->uvs[2][2]);
        _swap_x32(&swapPATHTYP->ve[0]);
        _swap_x32(&swapPATHTYP->ve[1]);
        _swap_x32(&swapPATHTYP->ve[2]);
        _swap_x32(&swapPATHTYP->vl[0]);
        _swap_x32(&swapPATHTYP->vl[1]);
        _swap_x32(&swapPATHTYP->vl[2]);
        _swap_x32(&swapPATHTYP->vr[0]);
        _swap_x32(&swapPATHTYP->vr[1]);
        _swap_x32(&swapPATHTYP->vr[2]);
        _swap_x32(&swapPATHTYP->width);
        _swap_x32(&swapPATHTYP->w);
        _swap_x32(&swapPATHTYP->m0);
        _swap_x32(&swapPATHTYP->m1);
        _swap_x32(&swapPATHTYP->wl);
        _swap_x32(&swapPATHTYP->ml);
        _swap_x32(&swapPATHTYP->wr);
        _swap_x32(&swapPATHTYP->mr);
        _swap_x32(&swapPATHTYP->m0l);
        _swap_x32(&swapPATHTYP->m0r);
        _swap_x32(&swapPATHTYP->m1l);
        _swap_x32(&swapPATHTYP->m1r);
   }
    stree = (streetyp *)(path + hp->pnum);
    for (i = 0, swapTREE = stree; i < hp->tnum; swapTREE++, i++) {
        _swap_x16(&swapTREE->parent);
        _swap_x16(&swapTREE->minh);
        _swap_x16(&swapTREE->maxh);
        _swap_x16(&swapTREE->minv);
        _swap_x16(&swapTREE->maxv);
        _swap_x16(&swapTREE->child[0]);
        _swap_x16(&swapTREE->child[1]);
        _swap_x16(&swapTREE->child[2]);
        _swap_x16(&swapTREE->child[3]);
   }
    surf = (surftyp *)(stree + hp->tnum);
    _swap_x16_array((unsigned char *)surf, hp->snum * sizeof(surftyp));
    svert = (sverttyp *)(surf + hp->snum);
    _swap_x16_array((unsigned char *)svert, hp->vnum * sizeof(sverttyp));
    save = (savetyp *)(svert + hp->vnum);
    _swap_x16_array((unsigned char *)save, hp->savenum * sizeof(savetyp));
    savenum = hp->savenum;
    vlist = (unsigned char *)(save + hp->savenum);
    slist = vlist + hp->vlnum;
}

/* Retain recovered local helpers while their callers remain unmatched. */
#pragma force_active on

/* 0x80043BD4 */
static void mats2fcopy(signed short (*sp)[3], float (*fp)[3])
{
    fp[0][0] = sp[0][0] * (1.0f / 16384.0f);
    fp[0][1] = sp[0][1] * (1.0f / 16384.0f);
    fp[0][2] = sp[0][2] * (1.0f / 16384.0f);
    fp[1][0] = sp[1][0] * (1.0f / 16384.0f);
    fp[1][1] = sp[1][1] * (1.0f / 16384.0f);
    fp[1][2] = sp[1][2] * (1.0f / 16384.0f);
    fp[2][0] = sp[2][0] * (1.0f / 16384.0f);
    fp[2][1] = sp[2][1] * (1.0f / 16384.0f);
    fp[2][2] = sp[2][2] * (1.0f / 16384.0f);
}

/* 0x80043D0C */
static void matf2scopy(float (*fp)[3], signed short (*sp)[3])
{
    sp[0][0] = fp[0][0] * 16384.0f;
    sp[0][1] = fp[0][1] * 16384.0f;
    sp[0][2] = fp[0][2] * 16384.0f;
    sp[1][0] = fp[1][0] * 16384.0f;
    sp[1][1] = fp[1][1] * 16384.0f;
    sp[1][2] = fp[1][2] * 16384.0f;
    sp[2][0] = fp[2][0] * 16384.0f;
    sp[2][1] = fp[2][1] * 16384.0f;
    sp[2][2] = fp[2][2] * 16384.0f;
}

#pragma force_active reset

/* 0x80043DF4 */
void FVecInterp(float *v1, float *v2, float t, float *vr)
{
    float tp;

    tp = 1.0f - t;
    vr[0] = v1[0] * tp + v2[0] * t;
    vr[1] = v1[1] * tp + v2[1] * t;
    vr[2] = v1[2] * tp + v2[2] * t;
}

/* 0x80043E3C */
streetyp *downleaf(streetyp *tp, signed short posh, signed short posv, signed short *quad)
{
    int midh, midv;
    signed short q;
    for (;;) {
        midh = tp->minh + tp->maxh;
        if (midh < 0) midh++;
        midv = tp->minv + tp->maxv;
        if (midv < 0) midv++;
        midh >>= 1;
        midv >>= 1;
        q = posh >= midh;
        if (posv < midv) q += 2;
        if (tp->flags & (1 << q)) {
            tp = &stree[tp->child[q]];
            if (tp == stree) return 0;
        } else {
            *quad = q;
            return tp;
       }
   }
}

/* 0x80043EE0 */
streetyp *updownleaf(streetyp *tp, signed short posh, signed short posv, signed short *quad)
{
    if (!tp) tp = stree;
    for (;;) {
        if ((posh >= tp->maxh || posh < tp->minh || posv >= tp->maxv || posv < tp->minv) != 0) {
            if (tp->parent == -1) return 0;
            tp = &stree[tp->parent];
        } else return downleaf(tp, posh, posv, quad);
   }
}

/* 0x80043F78 */
void DecodeSlist(unsigned char *slp, int snum, unsigned short *slnum, unsigned int sli)
{
    unsigned short *slnum2 = slnum + snum - 1;
    int cnt;
    unsigned int next;
    while (snum > 0) {
        next = (slp[0] << 8) + slp[1];
        slp += 2;
        cnt = next >> 13;
        snum -= cnt + 1;
        next &= 0x1fff;
        do {
            if (next == sli) *slnum++ = next++;
            else *slnum2-- = next++;
        } while (--cnt >= 0);
   }
}

/* 0x80043FE4 */
unsigned short DecodeVlist(unsigned char *vlp, int vnum, unsigned short *vlnum)
{
    int cnt;
    unsigned int next;
    while (vnum > 0) {
        next = (vlp[0] << 8) + vlp[1];
        vlp += 2;
        if (vnum > 1 && vlp[0] >= 0xe0) {
            cnt = *vlp++ & 0x1f;
        } else cnt = 0;
        vnum -= cnt + 1;
        do {
            *vlnum++ = next++;
        } while (--cnt >= 0);
   }
    return (vlp[0] << 8) + vlp[1];
}

/* Retain recovered geometric helpers while their callers remain unmatched. */
#pragma force_active on

/* 0x8004405C */
static void surfpos(float *pos, float (*uvs)[3], float *pt, float *rpt)
{
    float temp[3];
    temp[0] = pt[0] - pos[0];
    temp[1] = pt[1] - pos[1];
    temp[2] = pt[2] - pos[2];
    BodyVector(temp, rpt, uvs);
}

/* 0x800440BC */
static int VectorToSurfEdge(float *tpos, float *cpos, float *epos, float radiussq)
{
    float t;
    t = (cpos[0] * tpos[0] + cpos[2] * tpos[2]) / (tpos[0] * tpos[0] + tpos[2] * tpos[2]);
    if (t < 0.0f) {
        epos[0] = unitmatrix[3][0] - cpos[0];
        epos[1] = 0.0f;
        epos[2] = unitmatrix[3][2] - cpos[2];
    } else if (t > 1.0f) {
        epos[0] = tpos[0] - cpos[0];
        epos[1] = 0.0f;
        epos[2] = tpos[2] - cpos[2];
    } else {
        epos[0] = tpos[0] * t - cpos[0];
        epos[1] = 0.0f;
        epos[2] = tpos[2] * t - cpos[2];
    }
    epos[1] = 0.0f;
    return !(epos[0] * epos[0] + epos[2] * epos[2] >= radiussq);
}

#pragma force_active reset

/* 0x80044198 */
static signed short PointInSurf(surftyp *sp, float *ipos, float *ipos2, float *spos, float *dist, unsigned short *path, float (*uvs)[3], float maxSurfHop)
{
    int i;
    float tpos[3], vpos[3], epos[3], cpos[3];
    unsigned int vnum;
    unsigned short vlnum[16];
    sverttyp *vp;
    float ny;
    int state = 1;
    vnum = sp->snd_flags & 15;
    *path = DecodeVlist(vlist + sp->vli, vnum, vlnum);
    mats2fcopy(sp->uvs, uvs);
    vp = &svert[vlnum[0]];
    vpos[0] = ((vp->pos[0] << 5) + ((vp->overflow & 0x7c00) >> 10)) * (1.0f / 32.0f);
    vpos[1] = ((vp->pos[1] << 5) + ((vp->overflow & 0x3e0) >> 5)) * (1.0f / 32.0f);
    vpos[2] = ((vp->pos[2] << 5) + (vp->overflow & 31)) * (1.0f / 32.0f);
    tpos[0] = ipos[0] - vpos[0];
    tpos[1] = ipos[1] - vpos[1];
    tpos[2] = ipos[2] - vpos[2];
    BodyVector(tpos, spos, uvs);
    if (spos[1] <= maxSurfHop || spos[1] > *dist) return 0;
    vp = &svert[vlnum[vnum - 1]];
    vpos[0] = ((vp->pos[0] << 5) + ((vp->overflow & 0x7c00) >> 10)) * (1.0f / 32.0f);
    vpos[1] = ((vp->pos[1] << 5) + ((vp->overflow & 0x3e0) >> 5)) * (1.0f / 32.0f);
    vpos[2] = ((vp->pos[2] << 5) + (vp->overflow & 31)) * (1.0f / 32.0f);
    ny = vpos[0] * (vpos[2] - spos[2]) - vpos[2] * (vpos[0] - spos[0]);
    if (ny < 0.0f) {
        if (!VectorToSurfEdge(vpos, spos, epos, 0.01f)) return 0;
        state = -1;
        goto done;
    }
    vp = &svert[vlnum[1]];
    vpos[0] = ((vp->pos[0] << 5) + ((vp->overflow & 0x7c00) >> 10)) * (1.0f / 32.0f);
    vpos[1] = ((vp->pos[1] << 5) + ((vp->overflow & 0x3e0) >> 5)) * (1.0f / 32.0f);
    vpos[2] = ((vp->pos[2] << 5) + (vp->overflow & 31)) * (1.0f / 32.0f);
    ny = spos[2] * vpos[0] - spos[0] * vpos[2];
    if (ny < 0.0f) {
        if (!VectorToSurfEdge(vpos, spos, epos, 0.01f)) return 0;
        state = -1;
        goto done;
    }
    for (i = 2; i < vnum; i++) {
        float oldx = vpos[0], oldz = vpos[2];
        vp = &svert[vlnum[i]];
        vpos[0] = ((vp->pos[0] << 5) + ((vp->overflow & 0x7c00) >> 10)) * (1.0f / 32.0f);
    vpos[1] = ((vp->pos[1] << 5) + ((vp->overflow & 0x3e0) >> 5)) * (1.0f / 32.0f);
    vpos[2] = ((vp->pos[2] << 5) + (vp->overflow & 31)) * (1.0f / 32.0f);
        tpos[0] = vpos[0] - oldx;
        tpos[1] = 0.0f;
        tpos[2] = vpos[2] - oldz;
        cpos[0] = spos[0] - oldx;
        cpos[1] = 0.0f;
        cpos[2] = spos[2] - oldz;
        ny = cpos[2] * tpos[0] - cpos[0] * tpos[2];
        if (ny < 0.0f) {
            if (!VectorToSurfEdge(tpos, cpos, epos, 0.01f)) return 0;
            state = -1;
            break;
        }
    }
done:
    *dist = spos[1];
    if (ipos2 && ((sp->flags & 15) == 5 || (sp->flags & 15) == 6)) {
        vp = &svert[vlnum[0]];
        vpos[0] = ((vp->pos[0] << 5) + ((vp->overflow & 0x7c00) >> 10)) * (1.0f / 32.0f);
    vpos[1] = ((vp->pos[1] << 5) + ((vp->overflow & 0x3e0) >> 5)) * (1.0f / 32.0f);
    vpos[2] = ((vp->pos[2] << 5) + (vp->overflow & 31)) * (1.0f / 32.0f);
        tpos[0] = ipos2[0] - vpos[0];
        tpos[1] = ipos2[1] - vpos[1];
        tpos[2] = ipos2[2] - vpos[2];
        BodyVector(tpos, vpos, uvs);
        if (vpos[1] <= *dist || vpos[1] < -0.050000011920928955f) return 0;
    }
    return state;
}


/* 0x80044738 */
#pragma force_active on
static signed short CylinderInSurf(surftyp *sp, float *ipos, float *ipos2, float *opos, float *epos, float (*uvs)[3], float radiussq, int laststate, unsigned short *path)
{
    unsigned int vnum;
    unsigned short vlnum[16];
    sverttyp *vp;
    float spos[3], spos2[3], tpos[3], vpos[3];
    float t, ny;
    int i;
    int state = 1;
    float cpos[3];
    float x, y, z;
    mats2fcopy(sp->uvs, uvs);
    vnum = sp->snd_flags & 15;
    *path = DecodeVlist(vlist + sp->vli, vnum, vlnum);
    vp = &svert[vlnum[0]];
    x = ((vp->pos[0] << 5) + ((vp->overflow & 0x7c00) >> 10)) * (1.0f / 32.0f);
    y = ((vp->pos[1] << 5) + ((vp->overflow & 0x3e0) >> 5)) * (1.0f / 32.0f);
    z = ((vp->pos[2] << 5) + (vp->overflow & 31)) * (1.0f / 32.0f);
    tpos[0] = ipos[0] - x;
    tpos[1] = ipos[1] - y;
    tpos[2] = ipos[2] - z;
    BodyVector(tpos, spos, uvs);
    if (spos[1] < 0.0f) return 0;
    tpos[0] = ipos2[0] - x;
    tpos[1] = ipos2[1] - y;
    tpos[2] = ipos2[2] - z;
    BodyVector(tpos, spos2, uvs);
    if (spos2[1] > 0.0f) return 0;
    tpos[0] = spos2[0] - spos[0];
    tpos[1] = spos2[1] - spos[1];
    tpos[2] = spos2[2] - spos[2];
    if (tpos[1] > 0.0f) return 0;
    if (tpos[1] < 0.0f) {
        t = spos[1] / tpos[1];
        spos[0] -= tpos[0] * t;
        spos[1] -= tpos[1] * t;
        spos[2] -= tpos[2] * t;
    }
    vp = &svert[vlnum[vnum - 1]];
    vpos[0] = ((vp->pos[0] << 5) + ((vp->overflow & 0x7c00) >> 10)) * (1.0f / 32.0f);
        vpos[1] = ((vp->pos[1] << 5) + ((vp->overflow & 0x3e0) >> 5)) * (1.0f / 32.0f);
        vpos[2] = ((vp->pos[2] << 5) + (vp->overflow & 31)) * (1.0f / 32.0f);
    ny = vpos[0] * (vpos[2] - spos[2]) - vpos[2] * (vpos[0] - spos[0]);
    if (ny < 0.0f) {
        if (laststate > 0) return 0;
        if (!VectorToSurfEdge(vpos, spos, epos, radiussq)) return 0;
        state = -1;
        goto done;
    }
    vp = &svert[vlnum[1]];
    vpos[0] = ((vp->pos[0] << 5) + ((vp->overflow & 0x7c00) >> 10)) * (1.0f / 32.0f);
        vpos[1] = ((vp->pos[1] << 5) + ((vp->overflow & 0x3e0) >> 5)) * (1.0f / 32.0f);
        vpos[2] = ((vp->pos[2] << 5) + (vp->overflow & 31)) * (1.0f / 32.0f);
    ny = spos[2] * vpos[0] - spos[0] * vpos[2];
    if (ny < 0.0f) {
        if (laststate > 0) return 0;
        if (!VectorToSurfEdge(vpos, spos, epos, radiussq)) return 0;
        state = -1;
        goto done;
    }
    for (i = 2; i < vnum; i++) {
        float oldx = vpos[0], oldz = vpos[2];
        vp = &svert[vlnum[i]];
        vpos[0] = ((vp->pos[0] << 5) + ((vp->overflow & 0x7c00) >> 10)) * (1.0f / 32.0f);
        vpos[1] = ((vp->pos[1] << 5) + ((vp->overflow & 0x3e0) >> 5)) * (1.0f / 32.0f);
        vpos[2] = ((vp->pos[2] << 5) + (vp->overflow & 31)) * (1.0f / 32.0f);
        tpos[0] = vpos[0] - oldx;
        tpos[1] = 0.0f;
        tpos[2] = vpos[2] - oldz;
        cpos[0] = spos[0] - oldx;
        cpos[1] = 0.0f;
        cpos[2] = spos[2] - oldz;
        ny = cpos[2] * tpos[0] - cpos[0] * tpos[2];
        if (ny < 0.0f) {
            if (laststate > 0) return 0;
            if (!VectorToSurfEdge(tpos, cpos, epos, radiussq)) return 0;
            state = -1;
            break;
        }
    }
done:
    WorldVector(spos, opos, uvs);
    opos[0] += x;
    opos[1] += y;
    opos[2] += z;
    return state;
}
#pragma force_active reset


/* The original call sites reserve r3 for the surface argument. The recovered
 * DWARF declaration omits this unused argument. */
/* 0x80044CD0 */
void fPathInterp(surftyp *sp, unsigned short pathIndex, float *ipos, float *opos, float (*uvs)[3])
{
    pathtyp *p0p;
    pathtyp *p1p;
    float xi, yi;
    float vz0[3], vz1[3], vx[3], vy[3], vz[3];
    float pos0[3], pos1[3], pos[3];
    float m0, m1, length, i2, offset;
    float fgpos[3];
    int i;
    p1p = &path[pathIndex + 1 < gRoadHead->pnum ? pathIndex + 1 : 0];
    p0p = &path[pathIndex];
    surfpos(p0p->pos, p0p->uvs, ipos, fgpos);
    length = fgpos[0] * (p0p->m1 - p0p->m0) + p0p->w;
    xi = (fgpos[2] - fgpos[0] * p0p->m0) / length;
    if (fgpos[0] > 0.0f)
        yi = 0.5f * (fgpos[0] / (fgpos[2] * p0p->mr + p0p->wr)) + 0.5f;
    else
        yi = -0.5f * (fgpos[0] / (fgpos[2] * p0p->ml + p0p->wl)) + 0.5f;
    FVecInterp(p0p->vl, p0p->vr, yi, vz0);
    FVecInterp(p1p->vl, p1p->vr, yi, vz1);
    FVecInterp(vz0, vz1, xi, vz);
    FVecInterp(p0p->ve, p1p->ve, xi, vx);
    vy[0] = vz[1] * vx[2] - vz[2] * vx[1];
    vy[1] = vz[2] * vx[0] - vz[0] * vx[2];
    vy[2] = vz[0] * vx[1] - vz[1] * vx[0];
    for (i=0; i<3; i++) {
        uvs[0][i] = vx[i]; uvs[1][i] = vy[i]; uvs[2][i] = vz[i];
    }
    for (i=0; i<3; i++) pos0[i] = p0p->ve[i] * ((yi - 0.5f) * p0p->width) + p0p->pos[i];
    for (i=0; i<3; i++) pos1[i] = p1p->ve[i] * ((yi - 0.5f) * p1p->width) + p1p->pos[i];
    FVecInterp(pos0, pos1, xi, pos);
    m0 = yi * (p0p->m0r - p0p->m0l) + p0p->m0l;
    m1 = yi * (p1p->m0r - p1p->m0l) + p1p->m0l;
    i2 = xi * xi;
    m1 = -m1;
    offset = (xi * i2 * (m0 + m1) - i2 * (m1 + (m0 + m0)) + xi * m0);
    offset *= length;
    pos[0] -= offset * uvs[1][0];
    pos[1] -= offset * uvs[1][1];
    pos[2] -= offset * uvs[1][2];
    surfpos(pos, uvs, ipos, opos);
}

/* 0x80044FBC */
void fHalfPipeInterp(surftyp *sp, unsigned short pathIndex, float *ipos, float *opos, float (*uvs)[3], float dist)
{
    pathtyp *p0p;
    pathtyp *p1p;
    float r, r2, w, x, y, z, l;
    float fpos[3];
    p0p = &path[pathIndex];
    p1p = pathIndex + 1 < gRoadHead->pnum ? p0p + 1 : path;
    CopyUV(p0p->uvs, uvs);
    surfpos(p0p->pos, p0p->uvs, ipos, fpos);
    z = fpos[2];
    l = p0p->m1;
    if (z < 0.0f) z = 0.0f;
    else if (z > l) z = l;
    l = z / l;
    w = l * (p1p->w - p0p->w) + p0p->w;
    r = l * (p1p->width - p0p->width) + p0p->width;
    r -= w;
    r -= 2.5f;
    x = fpos[0];
    x += p0p->m0;
    y = fpos[1];
    if (uvs[1][1] < 0.0f) {
        froll(0.0f, -1.0f, uvs);
        x = -x;
        y = 5.0f + (-y + r);
    } else y -= r;
    if (x < -w) x += w;
    else if (x > w) x -= w;
    else {
        if (y > 0.0f) {
            opos[1] = r - y;
            froll(0.0f, -1.0f, uvs);
        } else opos[1] = r + y;
        return;
    }
    w = sqrtf(x * x + y * y);
    if (w < 0.01f) {
        opos[1] += r;
        return;
    }
    if ((opos[1] = r - w) < dist) {
        r2 = p0p->w + (p1p->width - p1p->w - p0p->width);
        r = p1p->w - p0p->w;
        if (fabs(r2) > 1.0) {
            l = sqrtf(r2 * r2 + p0p->m1 * p0p->m1);
            z = l;
            fpitch(-r2 / l, p0p->m1 / l, uvs);
        }
        froll(-x / w, -y / w, uvs);
        if (fabs(r) > 1.0) {
            l = r * fabs(x) / w;
            {
                float length = sqrtf(l * l + p0p->m1 * p0p->m1);
                fpitch(-l / length, p0p->m1 / length, uvs);
            }
        }
        if (fabs(r2) > 1.0) fpitch(r2 / z, p0p->m1 / z, uvs);
    }
}

static inline float floorpos(float f)
{
    if ((f < 0.0f && (int)f != f) != 0) f -= 1.0f;
    return f;
}

/* 0x80045520 */
void TireSurf(tModelData *m, float *ipos, float *opos, int *roadcode, float (*uvs)[3], int whl)
{
    float tuvs[3][3];
    unsigned int i;
    unsigned short path, savepath;
    signed short quad;
    unsigned int sli;
    float dist, maxdist = 250.0f;
    float spos[3];
    unsigned short snum, slnum[35];
    unsigned char *slp;
    streetyp *ltp;
    surftyp *sp, *savesp = 0;
    float x, z;
    z = floorpos(ipos[2]);
    x = floorpos(ipos[0]);
    ltp = updownleaf(m->lasttp[whl], x, z, &quad);
    if (!ltp) goto failed;
    m->lasttp[whl] = ltp;
    sli = ltp->child[quad];
    if (ltp->flags & (16 << quad)) sli |= 0x10000;
    if (!sli) goto failed;
    slp = slist + sli;
    snum = *slp++;
    DecodeSlist(slp, snum, slnum, m->tiresli[whl]);
    for (i=0; i<snum; i++) {
        sp = &surf[slnum[i]];
        if ((sp->flags & 15) == 5 || (sp->flags & 15) == 6 || (sp->flags & 15) == 15) continue;
        dist = maxdist;
        if (PointInSurf(sp, ipos, 0, spos, &dist, &path, tuvs, -5.0f) && dist > -5.0f && dist < maxdist) {
            savepath = path;
            maxdist = dist;
            savesp = sp;
            opos[0] = spos[0]; opos[1] = spos[1]; opos[2] = spos[2];
            CopyUV(tuvs, uvs);
            m->tiresli[whl] = slnum[i];
        }
    }
    if (maxdist == 250.0f) goto failed;
    if (opos[1] < 1.0f && (savesp->snd_flags & 0x30)) m->movingSurface = savesp;
    if (savesp->flags & 0x2000) {
        fHalfPipeInterp(savesp, savepath, ipos, opos, uvs, 250.0f);
        if (opos[1] < -5.0f) goto failed;
    } else if (savesp->flags & 0x1000) fPathInterp(savesp, savepath, ipos, opos, uvs);
    *roadcode = savesp->flags & 15;
    m->sviscode[whl] = (savesp->flags >> 4) & 15;
    m->roadboost[whl] = (savesp->flags >> 8) & 15;
    if (savesp->flags & 0x8000) m->roadboost[whl] = -m->roadboost[whl];
    m->sound_flags[whl] = savesp->snd_flags;
    if (savesp->snd_flags & 0x30) m->sound_flags[whl] &= 0x7ff;
    if ((*roadcode == 3 || *roadcode == 4 || *roadcode == 7) && opos[1] < 0.25f &&
        !gGameCar[m->slot].dead && !gGameCar[m->slot].crash_state && m->resurrect.moving_state == -1) {
        if (*roadcode == 4) {
            AddBattlePoint(m->slot, m->slot, -1);
            m->crashflag = 1;
        } else if (*roadcode == 7) {
            m->resurrect.waterAbort = 1;
            AbortCar(m->slot, 1);
        } else {
            m->resurrect.waterAbort = 1;
            AbortCar(m->slot, 1);
            PlayGameSound(0x56, 0, 1, 2);
        }
    }
    return;
failed:
    *roadcode = 0;
    m->sviscode[whl] = 0;
    m->roadboost[whl] = 0;
    m->sound_flags[whl] = 0;
    opos[0] = ipos[0];
    opos[1] = ipos[1] - -200.0f;
    opos[2] = ipos[2];
    CopyUV(unitmatrix, uvs);
}

/* 0x80045990 */
surftyp *RoadPosition(float *ipos, float *opos, float (*uvs)[3])
{
    float tuvs[3][3];
    unsigned int i;
    unsigned short path, savepath;
    signed short quad;
    unsigned int sli;
    float dist, maxdist, savedist;
    float spos[3];
    unsigned short snum, slnum[35];
    unsigned char *slp;
    streetyp *ltp;
    surftyp *sp, *savesp = 0;
    int tries = 0;
    float x, z;
retry:
    z = floorpos(ipos[2]);
    x = floorpos(ipos[0]);
    ltp = updownleaf(stree, x, z, &quad);
    if (!ltp) goto failed;
    sli = ltp->child[quad];
    if (ltp->flags & (16 << quad)) sli |= 0x10000;
    if (!sli) goto failed;
    slp = slist + sli;
    snum = *slp++;
    DecodeSlist(slp, snum, slnum, -1);
    maxdist = savedist = 250.0f;
    for (i = 0; i < snum; i++) {
        sp = &surf[slnum[i]];
        if ((sp->flags & 15) == 5 || (sp->flags & 15) == 6 || (sp->flags & 15) == 15) continue;
        dist = maxdist;
        if (PointInSurf(sp, ipos, 0, spos, &dist, &path, tuvs, -20.0f) && fabs(dist) < maxdist) {
            savepath = path;
            savedist = dist;
            maxdist = fabs(dist);
            savesp = sp;
            opos[0] = spos[0]; opos[1] = spos[1]; opos[2] = spos[2];
            CopyUV(tuvs, uvs);
        }
    }
    if (maxdist == 250.0f) goto failed;
    if (savesp->flags & 0x2000) {
        fHalfPipeInterp(savesp, savepath, ipos, opos, uvs, 250.0f);
        if (opos[1] < -5.0f) goto failed;
        spos[0] = uvs[1][0] * -opos[1];
        spos[1] = uvs[1][1] * -opos[1];
        spos[2] = uvs[1][2] * -opos[1];
    } else {
        if (savesp->flags & 0x1000) {
            fPathInterp(savesp, savepath, ipos, opos, uvs);
            NormalVector(uvs[0]); NormalVector(uvs[1]); NormalVector(uvs[2]);
        }
        spos[0] = uvs[1][0] * -savedist;
        spos[1] = uvs[1][1] * -savedist;
        spos[2] = uvs[1][2] * -savedist;
    }
    opos[0] = ipos[0] + spos[0];
    opos[1] = ipos[1] + spos[1];
    opos[2] = ipos[2] + spos[2];
    return savesp;
failed:
    if (tries == 0) { ipos[0] += 1.0f; tries = 1; goto retry; }
    else if (tries == 1) { ipos[0] -= 1.0f; ipos[2] += 1.0f; tries = 2; goto retry; }
    else if (tries == 2) { ipos[2] -= 2.0f; tries = 3; goto retry; }
    return 0;
}

/* 0x80045D5C */
surftyp *CameraCollision(float *ipos, float *lookat)
{
    surftyp *sp;
    float surfuvs[3][3];
    float temp[3];
    sp = ShotToWorldCollision(lookat, ipos, surfuvs, 0.5f, 1, 6);
    if (sp) {
        temp[0] = lookat[0] - ipos[0];
        temp[1] = lookat[1] - ipos[1];
        temp[2] = lookat[2] - ipos[2];
        NormalVector(temp);
        ipos[0] += 0.25f * temp[0];
        ipos[1] += 0.25f * temp[1];
        ipos[2] += 0.25f * temp[2];
   }
    return 0;
}


/* 0x800469EC */
void MoveCollisionObject(int num, float *trans)
{
    unsigned int i;
    sverttyp *vp;
    savetyp *s;
    for (i = 0, s = save; i < savenum; s++, i++) {
        if (s->object == num) {
            float x, y, z;
            int ix, iy, iz;
            vp = &svert[s->vnum];
            x = ((vp->pos[0] << 5) + ((vp->overflow & 0x7c00) >> 10)) * (1.0f / 32.0f);
            y = ((vp->pos[1] << 5) + ((vp->overflow & 0x3e0) >> 5)) * (1.0f / 32.0f);
            z = ((vp->pos[2] << 5) + (vp->overflow & 31)) * (1.0f / 32.0f);
            x += trans[0];
            y += trans[1];
            z += trans[2];
            {
                float scaled = x * 32.0f;
                ix = scaled < 0.0f ? scaled - 0.5f : scaled + 0.5f;
            }
            {
                float scaled = y * 32.0f;
                iy = scaled < 0.0f ? scaled - 0.5f : scaled + 0.5f;
            }
            {
                float scaled = z * 32.0f;
                iz = scaled < 0.0f ? scaled - 0.5f : scaled + 0.5f;
            }
            vp->pos[0] = ix >> 5;
            vp->pos[1] = iy >> 5;
            vp->pos[2] = iz >> 5;
            vp->overflow = ((ix & 31) << 10) | ((iy & 31) << 5) | (iz & 31);
        }
    }
}

/* 0x80046B98 */
void RotateCollisionObject(int num, float *ipos, float (*orig)[3], float (*uvs)[3])
{
    unsigned int i;
    float p0[3];
    float vec[3];
    float surfuvs[3][3];
    float newuvs[3][3];
    sverttyp *vp;
    savetyp *s;
    for (i = 0, s = save; i < savenum; s++, i++) {
        if (s->object == num) {
            int ix, iy, iz;
            mats2fcopy(s->uvs, surfuvs);
            MulUVS(orig, surfuvs, newuvs);
            MulUVS(uvs, newuvs, surfuvs);
            matf2scopy(surfuvs, surf[s->snum].uvs);
            p0[0] = ((s->pos[0] << 5) + ((s->overflow & 0x7c00) >> 10)) * (1.0f / 32.0f);
            p0[1] = ((s->pos[1] << 5) + ((s->overflow & 0x3e0) >> 5)) * (1.0f / 32.0f);
            p0[2] = ((s->pos[2] << 5) + (s->overflow & 31)) * (1.0f / 32.0f);
            vec[0] = p0[0] - ipos[0];
            vec[1] = p0[1] - ipos[1];
            vec[2] = p0[2] - ipos[2];
            WorldVector(vec, p0, orig);
            WorldVector(p0, vec, uvs);
            p0[0] = vec[0] + ipos[0];
            p0[1] = vec[1] + ipos[1];
            p0[2] = vec[2] + ipos[2];
            ix = p0[0] * 32.0f;
            iy = p0[1] * 32.0f;
            iz = p0[2] * 32.0f;
            vp = &svert[s->vnum];
            vp->overflow = ((ix & 31) << 10) + ((iy & 31) << 5) + (iz & 31);
            vp->pos[0] = ix >> 5;
            vp->pos[1] = iy >> 5;
            vp->pos[2] = iz >> 5;
        }
    }
}

/* 0x80046DBC */
void ResetCollisionObject(int num)
{
    savetyp *s;
    int i;
    for (s = save, i = 0; i < savenum; i++, s++) {
        if (s->object == num) {
            memcpy(surf[s->snum].uvs, s->uvs, 18);
            memcpy(&svert[s->vnum], s->pos, 8);
       }
   }
}

/* 0x80046E5C */
void RemoveCollisionObject(int num)
{
    savetyp *s;
    int i;
    for (s = save, i = 0; i < savenum; i++, s++) {
        if (s->object == num) surf[s->snum].flags = 15;
   }
}

static signed char tireIndex[8] = {0,1,3,2,0,1,3,2};
/* 0x80046EA0 */
#pragma force_active on
static void CarApplyCollision(tModelData *m, anon_surf_uvs_30 *c)
{
    float tmp[3];
    float coord[3];
    float massEffect, dz;
    int i, whl;
    surftyp *sp;
    whl = tireIndex[c->carEdge];
    dz = c->dist;
    sp = c->surf;
    m->isCollideWithWall = 50;
    tmp[0] = c->uvs[1][0] * dz;
    tmp[1] = c->uvs[1][1] * dz;
    tmp[2] = c->uvs[1][2] * dz;
    m->rwr[0] = tmp[0] + m->rwr[0];
    m->rwr[1] = tmp[1] + m->rwr[1];
    m->rwr[2] = tmp[2] + m->rwr[2];
    m->reckon.rwr[0] = tmp[0] + m->reckon.rwr[0];
    m->reckon.rwr[1] = tmp[1] + m->reckon.rwr[1];
    m->reckon.rwr[2] = tmp[2] + m->reckon.rwr[2];
    for (i = 0; i < 4; i++) {
        m->bodyrwr[i][0] = tmp[0] + m->bodyrwr[i][0];
        m->bodyrwr[i][1] = tmp[1] + m->bodyrwr[i][1];
        m->bodyrwr[i][2] = tmp[2] + m->bodyrwr[i][2];
        m->tireRwr[i][0] = tmp[0] + m->tireRwr[i][0];
        m->tireRwr[i][1] = tmp[1] + m->tireRwr[i][1];
        m->tireRwr[i][2] = tmp[2] + m->tireRwr[i][2];
    }
    tmp[0] = m->bodyR[whl][1] * m->w[2] - m->bodyR[whl][2] * m->w[1];
    tmp[1] = m->bodyR[whl][2] * m->w[0] - m->bodyR[whl][0] * m->w[2];
    tmp[2] = m->bodyR[whl][0] * m->w[1] - m->bodyR[whl][1] * m->w[0];
    tmp[0] += m->v[0];
    tmp[1] += m->v[1];
    tmp[2] += m->v[2];
    WorldVector(tmp, coord, m->uvs);
    BodyVector(coord, tmp, c->uvs);
    m->appearance |= 0x100000;
    massEffect = 20000.0f / m->mass;
    i = sp->flags & 15;
    if (i == 5 || (sp->flags & 0x2000) || i == 6)
        dz = 0.0009f * (massEffect * m->idt);
    else
        dz = 0.015f * (massEffect * m->idt);
    coord[0] = -tmp[0] * dz;
    coord[2] = -tmp[2] * dz;
    if (m->drone_type == 1)
        dz = 0.05f + 0.1f * fabs(m->uvs[2][0] * c->uvs[1][0] + m->uvs[2][1] * c->uvs[1][1] + m->uvs[2][2] * c->uvs[1][2]);
    else
        dz = 0.05f + 0.2f * fabs(m->uvs[2][0] * c->uvs[1][0] + m->uvs[2][1] * c->uvs[1][1] + m->uvs[2][2] * c->uvs[1][2]);
    coord[1] = dz * (m->idt * (-tmp[1] * massEffect));
    WorldVector(coord, tmp, c->uvs);
    BodyVector(tmp, coord, m->uvs);
    if ((i == 5 || i == 6) && m->roadcode[whl] == 8 && fabs(coord[2]) < 4000.0 && fabs(coord[1]) < 4000.0) {
        m->centerForce[2] += coord[2];
        m->bodyForce[whl][0] += coord[0];
    } else {
        m->bodyForce[whl][0] = coord[0] + m->bodyForce[whl][0];
        m->bodyForce[whl][1] = coord[1] + m->bodyForce[whl][1];
        m->bodyForce[whl][2] = coord[2] + m->bodyForce[whl][2];
    }
}
#pragma force_active reset

static signed char BodyVerts[16] = {0,1,2,3,0,1,2,3,1,2,3,0,4,5,7,6};
/* 0x800472A0 */
static void CollideCarBodyAgainstSurface(tModelData *m, surftyp *sp, anon_surf_uvs_30 *buf)
{
    unsigned int numVerts;
    unsigned short vertIndices[16];
    sverttyp *vert;
    float coord[3];
    float tmp[3];
    float bodyPoints[8][3];
    float surfPoints[16][2];
    float surfTrans[3][3];
    int isSurfReady = 0;
    int i, j, k, l;
    float crossY, y1, y2, dy, dx, dz, sx, sz, cand;
    int crossEdge, index;
    float a, t, x, z, sdx, sdz;
    numVerts = sp->snd_flags & 15;
    DecodeVlist(vlist + sp->vli, numVerts, vertIndices);
    vert = &svert[vertIndices[0]];
    coord[0] = ((vert->pos[0] << 5) + ((vert->overflow & 0x7c00) >> 10)) * (1.0f / 32.0f);
    coord[1] = ((vert->pos[1] << 5) + ((vert->overflow & 0x3e0) >> 5)) * (1.0f / 32.0f);
    coord[2] = ((vert->pos[2] << 5) + (vert->overflow & 31)) * (1.0f / 32.0f);
    mats2fcopy(sp->uvs, surfTrans);
    for (i = 0; i < 4; i++) {
        tmp[0] = m->bodyrwr[tireIndex[i]][0] - coord[0];
        tmp[1] = m->bodyrwr[tireIndex[i]][1] - coord[1];
        tmp[2] = m->bodyrwr[tireIndex[i]][2] - coord[2];
        BodyVector(tmp, bodyPoints[i], surfTrans);
    }
    if ((sp->flags & 15) == 5 || (sp->flags & 15) == 6) dz = -3.0f;
    else if (surfTrans[1][0] * m->uvs[1][0] + surfTrans[1][1] * m->uvs[1][1] + surfTrans[1][2] * m->uvs[1][2] < -0.707f) dz = -4.5f;
    else dz = -0.5f;
    BodyVector(m->uvs[1], tmp, surfTrans);
    for (i = 0; i < 4; i++) {
        bodyPoints[i + 4][0] = bodyPoints[i][0] + tmp[0] * dz;
        bodyPoints[i + 4][1] = bodyPoints[i][1] + tmp[1] * dz;
        bodyPoints[i + 4][2] = bodyPoints[i][2] + tmp[2] * dz;
    }
    crossY = 0.0f;
    crossEdge = -1;
    for (l = 0; l < 8; l++) {
        i = BodyVerts[l];
        j = BodyVerts[l + 8];
        y1 = bodyPoints[i][1];
        y2 = bodyPoints[j][1];
        if (!(y1 * y2 < 0.0f)) continue;
        if (!isSurfReady) {
            for (k = numVerts - 1; k > 0; k--) {
                vert = &svert[vertIndices[k]];
                surfPoints[k][0] = ((vert->pos[0] << 5) + ((vert->overflow & 0x7c00) >> 10)) * (1.0f / 32.0f);
                surfPoints[k][1] = ((vert->pos[2] << 5) + (vert->overflow & 31)) * (1.0f / 32.0f);
            }
            isSurfReady = 1;
        }
        dy = y2 - y1;
        dx = bodyPoints[j][0] - bodyPoints[i][0];
        dz = bodyPoints[j][2] - bodyPoints[i][2];
        if (!dy) continue;
        a = fabs(y1 / dy);
        x = bodyPoints[i][0] + dx * a;
        z = bodyPoints[i][2] + dz * a;
        if (dy > 0.0f) {
            cand = a;
            dx = -dx;
            dz = -dz;
        } else cand = 1.0f - a;
        sx = surfPoints[1][0];
        sz = 0.0f;
        if (-sx * z > 0.0f) continue;
        if (dz) {
            t = -z / dz;
            if (t > 0.0f && t < cand) cand = t;
        }
        for (k = 2; k < numVerts; k++) {
            sdx = surfPoints[k][0] - sx;
            sdz = surfPoints[k][1] - sz;
            a = (x - sx) * sdz - (z - sz) * sdx;
            if (a > 0.0f) goto nextEdge;
            t = sdx * dz - sdz * dx;
            if (t) {
                t = a / t;
                if (t > 0.0f && t < cand) cand = t;
            }
            sx = surfPoints[k][0];
            sz = surfPoints[k][1];
        }
        a = sz * x - sx * z;
        if (a < 0.0f) continue;
        t = sx * dz - sz * dx;
        if (t) {
            t = a / t;
            if (t > 0.0f && t < cand) cand = t;
        }
        if ((sp->flags & 15) == 4) {
            if (!gGameCar[m->slot].dead && !gGameCar[m->slot].crash_state && m->resurrect.moving_state == -1) {
                AddBattlePoint(m->slot, m->slot, -1);
                m->crashflag = 1;
            }
        } else if ((sp->flags & 15) == 7) {
            if (!gGameCar[m->slot].dead && !gGameCar[m->slot].crash_state && m->resurrect.moving_state == -1) {
                m->resurrect.waterAbort = 1;
                AbortCar(m->slot, 1);
            }
        } else if ((sp->flags & 15) == 3) {
            if (!gGameCar[m->slot].dead && !gGameCar[m->slot].crash_state && m->resurrect.moving_state == -1) {
                m->resurrect.waterAbort = 1;
                AbortCar(m->slot, 1);
                if (gLink[m->slot].owner == 6) PlayGameSound(0x56, 0, 1, 2);
            }
            return;
        }
        cand *= dy;
        if (cand < 0.0f) cand = -cand;
        if (cand > crossY) {
            if (-y1 / y2 < 1.0f) k = i;
            else k = j;
            coord[0] = m->bodyrwr[tireIndex[k]][0] - m->bodyrwr2[tireIndex[k]][0];
            coord[1] = m->bodyrwr[tireIndex[k]][1] - m->bodyrwr2[tireIndex[k]][1];
            coord[2] = m->bodyrwr[tireIndex[k]][2] - m->bodyrwr2[tireIndex[k]][2];
            BodyVector(coord, tmp, surfTrans);
            if (tmp[1] >= 0.0f) continue;
            crossY = cand;
            index = l;
            crossEdge = k;
        }
nextEdge:;
    }
    if (crossEdge < 0) return;
    {
        surftyp *old = buf->surf;
        if (old) {
            if (buf->dist < crossY) {
                CarApplyCollision(m, buf);
                buf->surf = 0;
                CollideCarBodyAgainstSurface(m, sp, buf);
            } else {
                buf->surf = sp;
                CopyUV(surfTrans, buf->uvs);
                buf->carEdge = crossEdge;
                buf->index = index;
                buf->dist = crossY;
                CarApplyCollision(m, buf);
                buf->surf = 0;
                CollideCarBodyAgainstSurface(m, old, buf);
            }
        } else {
            buf->surf = sp;
            CopyUV(surfTrans, buf->uvs);
            buf->carEdge = crossEdge;
            buf->index = index;
            buf->dist = crossY;
        }
    }
}


/* 0x80047B88 */
void CollideCarBody(tModelData *m)
{
    streetyp *trees[4];
    signed short quads[4];
    streetyp *candidate;
    unsigned short slnum[140];
    int slcnt, numTrees = 0;
    int i, j, unique;
    signed short quadCand;
    anon_surf_uvs_30 collBuffer = {0};
    int sli;
    unsigned char *slp;
    unsigned short snum;
    for (i = 0; i < 4; i++) {
        float z = floorpos(m->bodyrwr[i][2]);
        float x = floorpos(m->bodyrwr[i][0]);
        candidate = updownleaf(m->lasttp[i], x, z, &quadCand);
        if (candidate) {
            unique = 1;
            for (j = numTrees - 1; j > -1; j--) {
                if (candidate == trees[j] && quadCand == quads[j]) {
                    unique = 0;
                    break;
                }
            }
            if (unique) {
                quads[numTrees] = quadCand;
                trees[numTrees++] = candidate;
            }
        }
    }
    slcnt = 0;
    for (i = 0; i < numTrees; i++) {
        sli = trees[i]->child[quads[i]];
        if (trees[i]->flags & (16 << quads[i])) sli |= 0x10000;
        if (sli) {
            slp = slist + sli;
            snum = *slp++;
            DecodeSlist(slp, snum, &slnum[slcnt], 0);
            for (j = slcnt; j < slcnt + snum; j++) {
                surftyp *sp = &surf[slnum[j]];
                if (collBuffer.surf == sp || (sp->flags & 0x4000) || (sp->flags & 0x2000) || (sp->flags & 15) == 15) continue;
                CollideCarBodyAgainstSurface(m, sp, &collBuffer);
                sp->flags |= 0x4000;
            }
            slcnt += snum;
        }
    }
    if (m->isCollideWithWall > 0) m->isCollideWithWall--;
    if (collBuffer.surf) CarApplyCollision(m, &collBuffer);
    for (i = 0; i < slcnt; i++) {
        surftyp *sp = &surf[slnum[i]];
        float dist;
        float opos[3], svel[3];
        unsigned short path;
        sp->flags &= ~0x4000;
        if (sp->flags & 0x2000) {
            for (j = 0; j < 4; j++) {
                dist = 250.0f;
                if (PointInSurf(sp, m->bodyrwr[tireIndex[j]], m->bodyrwr2[tireIndex[j]], opos, &dist, &path, collBuffer.uvs, -5.0f)) {
                    fHalfPipeInterp(sp, path, m->bodyrwr[j], opos, collBuffer.uvs, 0.0f);
                    dist = opos[1];
                    if (dist < -5.0f || dist >= 0.0f) continue;
                    opos[0] = m->bodyrwr[tireIndex[j]][0] - m->bodyrwr2[tireIndex[j]][0];
                    opos[1] = m->bodyrwr[tireIndex[j]][1] - m->bodyrwr2[tireIndex[j]][1];
                    opos[2] = m->bodyrwr[tireIndex[j]][2] - m->bodyrwr2[tireIndex[j]][2];
                    BodyVector(opos, svel, collBuffer.uvs);
                    if (svel[1] >= 0.0f) continue;
                    collBuffer.surf = sp;
                    collBuffer.carEdge = j;
                    collBuffer.index = j;
                    collBuffer.dist = -dist;
                    CarApplyCollision(m, &collBuffer);
                }
            }
        }
    }
    for (i = 0; i < 4; i++) {
        m->bodyrwr2[i][0] = m->bodyrwr[i][0];
        m->bodyrwr2[i][1] = m->bodyrwr[i][1];
        m->bodyrwr2[i][2] = m->bodyrwr[i][2];
    }
}
