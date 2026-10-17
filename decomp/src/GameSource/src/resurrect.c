/* Partial recovery of resurrect.c. This translation unit remains NonMatching. */

#include "rush/rush_types.h"

#define MSL_NO_SQRTF

#include "msl_math_inline.h"

extern tCarData gGameCar[6];

extern tModelData model[6];

extern float unitmatrix[4][3];

extern double sin(double);

extern double cos(double);

extern void ExtractYaw(float (*mat)[3], float *yaw);

extern surftyp *RoadPosition(float *ipos, float *opos, float (*uvs)[3]);

extern void CopyUV(float (*src)[3], float (*dst)[3]);

extern void fyaw(float sint, float cost, float (*uv)[3]);

extern void syminit(tModelData *m);

extern void multigo(signed short droneIndex);

extern signed char gDeathMode;

extern float gModelTime;

extern enum_kNumRaceTypes gRace;

extern void CrashedStunt(int player);

extern void InitBattlePlayer(signed short slot);

extern anon_numPts_pts_8 gDronePaths[4];

extern anon_startTime_loopPt_32c gCheckpoints;

extern anon_numPts_pts_10 gResPath;

extern void SetNewPathIndex(tModelData *m, signed short idx);

extern signed char gTrackNum, gBackwards;


extern anon_track_car_a gLink[13];

extern signed char gSuperSpeed;

extern float magnitude(float *v);

extern void make_super_speed(tModelData *m);

static float resurrectTime = 0.0f; /* explicitly initialised: first in .sbss */
/* defined here (deferred .sbss, reverse order: gResurrectInPlace, gNoAutoAbort, gDemoPathPt) */
signed short gDemoPathPt;
signed char gNoAutoAbort;
signed char gResurrectInPlace;

/* The recovered call sites pass an abort flag in r5, absent from the generated header. */

extern void SetResurrectionData(signed short mode, tModelData *m, int abort);

inline float sinf(float x) { return (float)sin(x); }

inline float cosf(float x) { return (float)cos(x); }

#pragma force_active on

#pragma force_active on

#pragma force_active on

extern unsigned int gGameState;

extern signed char gEndGame,gCoastFlag,gNumActiveCars;

extern signed short gNumPlayers;

extern anon_slot_controller_50 gPlayers[4];

extern void NextResPathPt(int path,int idx,int *outPath,int *outIdx);

extern void BodyVector(float *v,float *result,float (*uvs)[3]);

extern float fatan2(float x,float y);

extern void TB_Vector3Normalize(float *v);

extern float fasin(float v);

/* sqrtf is needed here; the earlier bodies intentionally suppress its unused constants. */

#pragma force_active on

extern signed short GetNextCheckpoint(signed short idx);

extern void SetDroneResPathIndex(tModelData *m,signed short path,signed short idx);

extern void SyncMaxpathToLastCheckpoint(signed short slot);

extern void NormalVector(float *v);

extern signed short gNumPlayers;

extern float Random(float max);

extern int FindResPt(int inPath, int inIdx, int *outPath, int *outIdx, int honorAltRes);

#pragma force_active on


extern signed short gNumLaps;

extern signed short FindClosestPathPt(signed short *pos, signed short path);

extern int NumPtsToCheckpoint(int res,int inPath,int inIdx,int cp);

extern void TracePathPtForward(int res,int inPath,int inIdx,int numPts,int *outPath,int *outIdx);

extern int CountdownTimeOut(void);

extern void PassedCP(tModelData *m,float time);

static float res_last_time[4] = {0.0f,0.0f,0.0f,0.0f};

static const float pole_pos_offset[12][3] = {
    {0,0,0},{17,0,0},{-8.5f,0,-20},{8.5f,0,-20},{0,0,-40},{17,0,-40},
    {-3.4f,0,0},{3.4f,0,0},{-23.8f,0,0},{-17,0,0},{-10.2f,0,0},{10.2f,0,0}
};

inline float surface_sqrt(float x)
{
    if (x > 0.0f) {
        double guess = __frsqrte((double)x);
        guess = 0.5 * guess * (3.0 - guess * guess * x);
        guess = 0.5 * guess * (3.0 - guess * guess * x);
        guess = 0.5 * guess * (3.0 - guess * guess * x);
        return (float)(x * guess);
    } else if (x < 0.0) return NAN;
    else if (fpclassify(x) == FP_NAN) return NAN;
    return x;
}

static inline float project_axis(short first,short next,float position)
{
    float direction=(float)next-first;
    float projection=direction*(position-first);
    projection=projection; /* self-copy keeps the original product register allocation */
    return projection;
}

/* prototypes: the functions are defined in their original order below */
void Resurrect(tModelData *m);
void GetResPt(void);
static void FindHumanResPt(tModelData *m, int *outPath, int *outIdx);
static void FindDroneResPt(tModelData *m, int *outPath, int *outIdx);
static void FindSurfaceUvsFromResPoint(int res, int path, int idx, float (*uvs)[3]);
static int FindSurfaceUvsInPlace(float *inPos, float (*inUvs)[3], float (*uvs)[3]);
void MakeUvsFromQuat(float *q, float (*uvs)[3]);
void MakeQuatFromUvs(float (*uvs)[3], float *q);
static void FindBestQuat(float *q1, float *q2);
void Interpolate(float *pointa, float *pointb, signed short p_size, float vala, float valb, float inab, float *pointab);
void SetResurrectionData(signed short mode,tModelData *m,int abort);
void InitDemoPathPoint(void);
void CheckCrash(signed short drone_index);
void FinishResurrecting(tModelData *m);
void UpdateResurrectingCar(tModelData *m);
void BlinkResurrectingCars(void);
void AbortCar(signed short drone_index, int death);
void DetectDroneAbort(void);
int AreWeSolid(signed short node);

void Resurrect(tModelData *m)
{
    int inPlace;
    signed short node = m->slot;
    int i;
    int startPath,endPath,startIdx,endIdx;
    int jumpPts,numPts;
    float time_scale,crash_time;
    tCarData *gc = &gGameCar[node];
    int human;
    if (gDeathMode && m->resurrect.crash_time != 0.0f && !m->resurrect.autoAbort) return;
    m->resurrect.autoAbort = 0;
    human = gLink[node].owner == 6;
    if (human && m->fastin.modelTime < res_last_time[m->slot]) res_last_time[m->slot] = 0.0f;
    inPlace = 0;
    if (human && !m->resurrect.waterAbort && gResurrectInPlace &&
        2.0f + res_last_time[m->slot] < m->fastin.modelTime && !gDeathMode) {
        inPlace = 1;
        res_last_time[m->slot] = m->fastin.modelTime;
    }
    m->resurrect.waterAbort = 0;
    if (m->resurrect.crash_time == 0.0f) {
        m->initin.pos[0] = m->resurrect.save_pos[0];
        m->initin.pos[1] = m->resurrect.save_pos[1];
        m->initin.pos[2] = m->resurrect.save_pos[2];
        CopyUV(m->resurrect.uvs,m->initin.uvs);
        if ((unsigned int)(gRace-kStunt) <= 2 || m->initin.polepos >= 6) {
            m->initin.offset[0] = 0.0f;
            m->initin.offset[1] = 0.0f;
            m->initin.offset[2] = 0.0f;
        } else {
            m->initin.offset[0] = pole_pos_offset[m->initin.polepos][0];
            m->initin.offset[1] = pole_pos_offset[m->initin.polepos][1];
            m->initin.offset[2] = pole_pos_offset[m->initin.polepos][2];
        }
        m->resurrect.moving_state = -1;
        syminit(m);
        if (gGameState & 8) {
            m->resurrect.velocity = (1.4666667f * gDronePaths[gc->mpctl.path].pts[gCheckpoints.points[m->last_cp_index].pathPts[gc->mpctl.path+1]].mph) / 2.0f;
            m->v[2] = m->resurrect.velocity;
            for (i = 0;i < 4;i++) {
                m->tireV[i][2] = m->resurrect.velocity;
                m->tires[i].angvel = m->tireV[i][2] / m->tires[i].tradius;
            }
        }
        multigo(node);
        return;
    }
    if (inPlace) {
        m->resurrect.pos[0] = m->rwr[0];
        m->resurrect.pos[1] = m->rwr[1];
        m->resurrect.pos[2] = m->rwr[2];
        m->resurrect.velocity = 0;
        if (!FindSurfaceUvsInPlace(m->resurrect.pos,m->reckon.uvs,m->resurrect.uvs)) {
            m->resurrect.pos[1] += 10.0f;
            m->appearance &= ~0x10;
            m->crashflag = 0;
            goto finish;
        }
    }
    time_scale = gc->solid ? 1.1f : 0.8f;
    gc->collidable = m->collidable = 0;
    gc->solid = m->solid = 0;
    gc->collide_time = m->fastin.modelTime;
    gc->collide_count = gc->collide_state = -1;
    m->hide_car = 1;
    if (human) {
        FindHumanResPt(m,&startPath,&startIdx);
        if (startPath >= 0 && gResPath.alt[startPath].resurrect == 2) {
            m->resurrect.pos[0] = gResPath.alt[startPath].pts[startIdx][0];
            m->resurrect.pos[1] = 3.0f + gResPath.alt[startPath].pts[startIdx][1];
            m->resurrect.pos[2] = gResPath.alt[startPath].pts[startIdx][2];
            m->resurrect.velocity = 0;
            FindSurfaceUvsFromResPoint(1,startPath,startIdx,m->resurrect.uvs);
            m->appearance &= ~0x10;
            m->crashflag = 0;
            goto finish;
        }
    } else FindDroneResPt(m,&startPath,&startIdx);
    m->resurrect.resurrect_time = m->fastin.modelTime;
    if (gRace == kObstacle || (unsigned int)gRace <= 1 || gRace == kCircuit || gRace == kGhost)
        resurrectTime = 0.01f;
    else resurrectTime = 1.5f;
    crash_time = resurrectTime + (m->resurrect.resurrect_time - m->resurrect.crash_time);
    if (gRace == kPractice || (unsigned int)(gRace-kStunt) <= 2 || gResurrectInPlace) {
        m->resurrect.velocity = 0;
    } else if (m->resurrect.velocity < 0) {
        m->resurrect.velocity = 1;
    } else if (human) {
        i = FindClosestPathPt(startPath >= 0 ? gResPath.alt[startPath].pts[startIdx] : gResPath.pts[startIdx],0);
        m->resurrect.velocity = 1.4666667f * gDronePaths[0].pts[i].mph;
    } else {
        m->resurrect.velocity = 1.4666667f * gDronePaths[startPath].pts[startIdx].mph;
    }
    if (m->resurrect.velocity > 146.66667f) m->resurrect.velocity = 146.66667f;
    jumpPts = crash_time * (m->resurrect.velocity * time_scale) / 20.0f;
    if (m->next_cp_index == gCheckpoints.finishPt && m->laps == gNumLaps - 1 &&
        gRace != kPractice && gRace != kStunt && gRace != kBattle && gRace != kObstacle) {
        numPts = NumPtsToCheckpoint(human,startPath,startIdx,m->next_cp_index) / 2;
        if (jumpPts > numPts) jumpPts = numPts;
    }
    TracePathPtForward(human,startPath,startIdx,jumpPts,&endPath,&endIdx);
    if (endPath >= 0) {
        if (human) {
            m->resurrect.pos[0] = gResPath.alt[endPath].pts[endIdx][0];
            m->resurrect.pos[1] = 3.0f + gResPath.alt[endPath].pts[endIdx][1];
            m->resurrect.pos[2] = gResPath.alt[endPath].pts[endIdx][2];
            if (gRace != kPractice && gRace != kBattle && gRace != kStunt &&
                gCheckpoints.points[m->next_cp_index].pathPts[endPath+5] >= 0 &&
                gCheckpoints.points[m->next_cp_index].pathPts[endPath+5] <= endIdx && !CountdownTimeOut())
                PassedCP(m,m->fastin.modelTime);
        } else {
            m->resurrect.pos[0] = gDronePaths[endPath].pts[endIdx].pos[0];
            m->resurrect.pos[1] = 3.0f + gDronePaths[endPath].pts[endIdx].pos[1];
            m->resurrect.pos[2] = gDronePaths[endPath].pts[endIdx].pos[2];
            if (((m->next_cp_index < m->last_cp_index && gCheckpoints.points[m->last_cp_index].pathPts[endPath+1] > endIdx) ||
                 (m->next_cp_index > m->last_cp_index && gCheckpoints.points[m->next_cp_index].pathPts[endPath+1] <= endIdx)) &&
                 !CountdownTimeOut()) PassedCP(m,m->fastin.modelTime);
        }
    } else {
        m->resurrect.pos[0] = gResPath.pts[endIdx][0];
        m->resurrect.pos[1] = 3.0f + gResPath.pts[endIdx][1];
        m->resurrect.pos[2] = gResPath.pts[endIdx][2];
        if (gRace != kPractice && gRace != kBattle && gRace != kStunt &&
            ((m->next_cp_index < m->last_cp_index && gCheckpoints.points[m->last_cp_index].pathPts[0] > endIdx) ||
             (m->next_cp_index > m->last_cp_index && gCheckpoints.points[m->next_cp_index].pathPts[0] <= endIdx)) &&
             !CountdownTimeOut()) PassedCP(m,m->fastin.modelTime);
    }
    if (gRace != kPractice && gRace != kBattle && gRace != kStunt && gRace != kObstacle)
        m->resurrect.velocity = 58;
    FindSurfaceUvsFromResPoint(human,endPath,endIdx,m->resurrect.uvs);
finish:
    m->initin.offset[0] = 0.0f;
    m->initin.offset[1] = 0.0f;
    m->initin.offset[2] = 0.0f;
    MakeQuatFromUvs(m->uvs,m->resurrect.quat_start);
    MakeQuatFromUvs(m->resurrect.uvs,m->resurrect.quat_end);
    FindBestQuat(m->resurrect.quat_start,m->resurrect.quat_end);
    m->resurrect.moving_state = 0;
    CopyUV(m->reckon.uvs,m->initin.uvs);
    m->initin.pos[0] = m->reckon.rwr[0];
    m->initin.pos[1] = m->reckon.rwr[1];
    m->initin.pos[2] = m->reckon.rwr[2];
    {
        float vx=m->rwv[0],vy=m->rwv[1],vz=m->rwv[2];
        syminit(m);
        for (i=0;i<4;i++) {
            m->suscomp[i] = 0.0f;
            m->bodyForce[i][0] = 0.0f;
            gc->sound_flags[i] = m->sound_flags[i] = 0;
        }
        m->topscrape = 0;
        m->thumpflag = 0;
        m->mthumpflag = 0;
        m->rwv[0]=vx;m->rwv[1]=vy;m->rwv[2]=vz;
    }
    gc->restart = 1;
}

void GetResPt(void)
{
    int p,idx;
    float min,x,y,z,dx,dy,dz,dist;
    tModelData *m=&model[0];
    tCarData *gc=&gGameCar[0];
    x=gc->dr_pos[0];
    y=gc->dr_pos[1];
    z=gc->dr_pos[2];
    min=1.0e20f;
    for(p=0;p<gResPath.numPts;p++) {
        dx=gResPath.pts[p][0]-x;
        dy=gResPath.pts[p][1]-y;
        dz=gResPath.pts[p][2]-z;
        dist=dx*dx+dy*dy+dz*dz;
        if(dist<min) { min=dist;idx=p; }
    }
    if(idx&1) {
        --idx;
        if(idx>m->obResPt) m->obResPt=idx;
    } else {
        z=project_axis(gResPath.pts[idx][2],gResPath.pts[idx+1][2],gc->dr_pos[2]);
        x=project_axis(gResPath.pts[idx][0],gResPath.pts[idx+1][0],gc->dr_pos[0]);
        y=project_axis(gResPath.pts[idx][1],gResPath.pts[idx+1][1],gc->dr_pos[1]);
        dist=z+(x+y);
        if(dist<0.0f && idx>0) idx-=2;
        if(idx>m->obResPt) m->obResPt=idx;
    }
}

#pragma force_active on
static void FindHumanResPt(tModelData *m, int *outPath, int *outIdx)
{
    int a;
    int p;
    int path = -1;
    int idx = 0;
    float min = 1.0e20f;
    float x, y, z;
    float dx, dy, dz, dist;
    if (gRace == kBattle) {
        float min2 = 0.0f;
        int bestfit = 0;
        int pOffset = Random(gResPath.numPts);
        int pReal;
        for (p = 0; p < gResPath.numPts; p++) {
            min = 1.0e20f;
            pReal = pOffset + p;
            if (pReal >= gResPath.numPts) pReal -= gResPath.numPts;
            for (a = 0; a < gNumPlayers; a++) {
                if (m->slot == a) continue;
                if (model[a].resurrect.moving_state >= 0) {
                    x = model[a].resurrect.pos[0];
                    z = model[a].resurrect.pos[2];
                } else {
                    x = model[a].rwr[0];
                    z = model[a].rwr[2];
                }
                dx = gResPath.pts[pReal][0] - x;
                dz = gResPath.pts[pReal][2] - z;
                dist = dx * dx + dz * dz;
                if (dist < min) min = dist;
            }
            if (min > min2) {
                bestfit = pReal;
                min2 = min;
            }
        }
        idx = bestfit;
    } else if (gRace == kObstacle) {
        idx = m->obResPt;
    } else {
        x = m->resurrect.save_pos[0];
        y = m->resurrect.save_pos[1];
        z = m->resurrect.save_pos[2];
        for (p = 0; p < gResPath.numPts; p++) {
            dx = gResPath.pts[p][0] - x;
            dy = gResPath.pts[p][1] - y;
            dz = gResPath.pts[p][2] - z;
            dist = dx * dx + dy * dy + dz * dz;
            if (dist < min) { min = dist; idx = p; }
        }
        if ((unsigned int)(gRace - kStunt) > 1) {
            if (min > 1600.0f) {
                for (a = 0; a < gResPath.numAlt; a++) {
                    if ((gResPath.alt[a].resurrect == 0 && gRace == kPractice) ||
                        (gResPath.alt[a].resurrect == 2 && gRace != kPractice)) continue;
                    for (p = 0; p < gResPath.alt[a].numPts; p++) {
                        dx = gResPath.alt[a].pts[p][0] - x;
                        dy = gResPath.alt[a].pts[p][1] - y;
                        dz = gResPath.alt[a].pts[p][2] - z;
                        dist = dx * dx + dy * dy + dz * dz;
                        if (dist < min) { min = dist; idx = p; path = a; }
                    }
                }
            }
            if (path >= 0 && gResPath.alt[path].resurrect == 0)
                FindResPt(path,idx,&path,&idx,1);
            if (!(path >= 0 && gResPath.alt[path].resurrect == 2)) {
                if (path >= 0) {
                    a = path + 5;
                    if (gCheckpoints.points[m->last_cp_index].pathPts[a] >= 0) {
                        if (gCheckpoints.points[m->next_cp_index].pathPts[a] >= 0) {
                            if (idx < gCheckpoints.points[m->last_cp_index].pathPts[a] ||
                                idx > gCheckpoints.points[m->next_cp_index].pathPts[a]) {
                                min = 1.0e20f;
                                for (p = gCheckpoints.points[m->last_cp_index].pathPts[a];
                                     p <= gCheckpoints.points[m->next_cp_index].pathPts[a];p++) {
                                    dx = gResPath.alt[path].pts[p][0] - x;
                                    dy = gResPath.alt[path].pts[p][1] - y;
                                    dz = gResPath.alt[path].pts[p][2] - z;
                                    dist = dx * dx + dy * dy + dz * dz;
                                    if (dist < min) { min = dist; idx = p; }
                                }
                            }
                        } else if (idx < gCheckpoints.points[m->last_cp_index].pathPts[a]) {
                            idx = gCheckpoints.points[m->last_cp_index].pathPts[a];
                        }
                    } else if (gCheckpoints.points[m->next_cp_index].pathPts[a] < 0 &&
                               m->last_cp_index != gResPath.alt[path].startSection) {
                        path = -1;
                        idx = -1;
                    }
                }
                if (path < 0) {
                    a = m->next_cp_index < m->last_cp_index ? gResPath.numPts :
                        gCheckpoints.points[m->next_cp_index].pathPts[0];
                    if (idx < gCheckpoints.points[m->last_cp_index].pathPts[0] || idx >= a) {
                        min = 1.0e20f;
                        for (p = gCheckpoints.points[m->last_cp_index].pathPts[0];p < a;p++) {
                            dx = gResPath.pts[p][0] - x;
                            dy = gResPath.pts[p][1] - y;
                            dz = gResPath.pts[p][2] - z;
                            dist = dx * dx + dy * dy + dz * dz;
                            if (dist < min) { min = dist; idx = p; }
                        }
                    }
                }
            }
        }
    }
    *outPath = path;
    *outIdx = idx;
}
#pragma force_active reset

#pragma force_active on
static void FindDroneResPt(tModelData *m, int *outPath, int *outIdx)
{
    int pathIndex=((tCarData *)((char *)gGameCar+m->slot*sizeof(tCarData)))->mpctl.path;
    int p, idx, end;
    float min, x, y, z, dx, dy, dz, dist;
    anon_numPts_pts_8 *path;
    path=&gDronePaths[pathIndex];
    min=1.0e20f;
    x=m->resurrect.save_pos[0];
    y=m->resurrect.save_pos[1];
    z=m->resurrect.save_pos[2];
    idx=0;
    end=m->next_cp_index < m->last_cp_index ? path->numPts : gCheckpoints.points[m->next_cp_index].pathPts[pathIndex+1];
    for (p=gCheckpoints.points[m->last_cp_index].pathPts[pathIndex+1];p<end;p++) {
        dx=path->pts[p].pos[0]-x;
        dy=path->pts[p].pos[1]-y;
        dz=path->pts[p].pos[2]-z;
        dist=dx*dx+dy*dy+dz*dz;
        if (dist<min) { min=dist;idx=p; }
    }
    *outPath=pathIndex;
    *outIdx=idx;
    SetNewPathIndex(m,idx);
}
#pragma force_active reset

#pragma force_active on
static void FindSurfaceUvsFromResPoint(int res, int path, int idx, float (*uvs)[3])
{
    float sine, cosine, yaw, angle;
    float snapPos[3], pt[3], pos[3];
    surftyp *surf;
    if (path >= 0) {
        if (res) {
            pos[0] = gResPath.alt[path].pts[idx][0];
            pos[1] = gResPath.alt[path].pts[idx][1];
            pos[2] = gResPath.alt[path].pts[idx][2];
            if (idx == gResPath.alt[path].numPts - 1) {
                if (gResPath.alt[path].lastPath >= 0) {
                    pt[0] = gResPath.alt[gResPath.alt[path].lastPath].pts[gResPath.alt[path].lastIdx][0] - pos[0];
                    pt[1] = gResPath.alt[gResPath.alt[path].lastPath].pts[gResPath.alt[path].lastIdx][1] - pos[1];
                    pt[2] = gResPath.alt[gResPath.alt[path].lastPath].pts[gResPath.alt[path].lastIdx][2] - pos[2];
                } else {
                    pt[0] = gResPath.pts[gResPath.alt[path].lastIdx][0] - pos[0];
                    pt[1] = gResPath.pts[gResPath.alt[path].lastIdx][1] - pos[1];
                    pt[2] = gResPath.pts[gResPath.alt[path].lastIdx][2] - pos[2];
                }
            } else {
                pt[0] = gResPath.alt[path].pts[idx+1][0] - pos[0];
                pt[1] = gResPath.alt[path].pts[idx+1][1] - pos[1];
                pt[2] = gResPath.alt[path].pts[idx+1][2] - pos[2];
            }
        } else {
            pos[0] = gDronePaths[path].pts[idx].pos[0];
            pos[1] = gDronePaths[path].pts[idx].pos[1];
            pos[2] = gDronePaths[path].pts[idx].pos[2];
            if (idx == gDronePaths[path].numPts - 1) {
                pt[0] = gDronePaths[path].pts[gCheckpoints.points[gCheckpoints.loopPt].pathPts[path+1]].pos[0] - gDronePaths[path].pts[idx].pos[0];
                pt[1] = gDronePaths[path].pts[gCheckpoints.points[gCheckpoints.loopPt].pathPts[path+1]].pos[1] - gDronePaths[path].pts[idx].pos[1];
                pt[2] = gDronePaths[path].pts[gCheckpoints.points[gCheckpoints.loopPt].pathPts[path+1]].pos[2] - gDronePaths[path].pts[idx].pos[2];
            } else {
                pt[0] = gDronePaths[path].pts[idx+1].pos[0] - gDronePaths[path].pts[idx].pos[0];
                pt[1] = gDronePaths[path].pts[idx+1].pos[1] - gDronePaths[path].pts[idx].pos[1];
                pt[2] = gDronePaths[path].pts[idx+1].pos[2] - gDronePaths[path].pts[idx].pos[2];
            }
        }
    } else {
        pos[0] = gResPath.pts[idx][0];
        pos[1] = gResPath.pts[idx][1];
        pos[2] = gResPath.pts[idx][2];
        if (idx == gResPath.numPts - 1) {
            pt[0] = gResPath.pts[gCheckpoints.points[gCheckpoints.loopPt].pathPts[0]][0] - gResPath.pts[idx][0];
            pt[1] = gResPath.pts[gCheckpoints.points[gCheckpoints.loopPt].pathPts[0]][1] - gResPath.pts[idx][1];
            pt[2] = gResPath.pts[gCheckpoints.points[gCheckpoints.loopPt].pathPts[0]][2] - gResPath.pts[idx][2];
        } else {
            pt[0] = gResPath.pts[idx+1][0] - gResPath.pts[idx][0];
            pt[1] = gResPath.pts[idx+1][1] - gResPath.pts[idx][1];
            pt[2] = gResPath.pts[idx+1][2] - gResPath.pts[idx][2];
        }
    }
    TB_Vector3Normalize(pt);
    surf = RoadPosition(pos,snapPos,uvs);
    if (surf) {
        angle = fasin(pt[0]);
        if (pt[2] < 0.0f) angle = 3.1415927410125732f - angle;
        ExtractYaw(uvs,&yaw);
        sine = sinf(angle+yaw);
        cosine = cosf(angle+yaw);
    } else {
        sine = pt[1];
        cosine = surface_sqrt(pt[0]*pt[0] + pt[2]*pt[2]);
        if (pt[2] < 0.0f) sine = -sine;
        uvs[0][0]=1.0f;uvs[0][1]=0.0f;uvs[0][2]=0.0f;
        uvs[1][0]=0.0f;uvs[1][1]=cosine;uvs[1][2]=-sine;
        uvs[2][0]=0.0f;uvs[2][1]=sine;uvs[2][2]=cosine;
        sine=pt[0];cosine=pt[2];
    }
    fyaw(-sine,cosine,uvs);
}
#pragma force_active reset

#pragma force_active on
static int FindSurfaceUvsInPlace(float *inPos, float (*inUvs)[3], float (*uvs)[3])
{
    float sine, cosine, inYaw, outYaw;
    float snapPos[3];
    surftyp *surf;
    ExtractYaw(inUvs, &inYaw);
    inYaw=-inYaw;
    surf=RoadPosition(inPos, snapPos, uvs);
    if (surf) {
        if (surf->flags & 0x2000) return 1;
        ExtractYaw(uvs, &outYaw);
        sine=sinf(inYaw+outYaw);
        cosine=cosf(inYaw+outYaw);
    } else {
        sine=sinf(inYaw);
        cosine=cosf(inYaw);
        CopyUV(unitmatrix, uvs);
    }
    fyaw(-sine, cosine, uvs);
    return 0;
}
#pragma force_active reset

void MakeUvsFromQuat(float *q, float (*uvs)[3])
{
    float Nq;
    float q1=q[0], q2=q[1], q3=q[2], q4=q[3];
    float q11=q1*q1, q22=q2*q2, q33=q3*q3, q44=q4*q4;
    float q12, q13, q14, q23, q24, q34;
    q14=q1*q4; q13=q1*q3; q12=q1*q2;
    q24=q2*q4; q23=q2*q3; q34=q3*q4;
    Nq=q11+q22+q33+q44;
    Nq=Nq>0.0f ? 1.0f/Nq : 0.0f;
    uvs[0][0]=Nq*(q11+q22-q33-q44);
    uvs[0][1]=Nq*(2.0f*(q23-q14));
    uvs[0][2]=Nq*(2.0f*(q24+q13));
    uvs[1][0]=Nq*(2.0f*(q23+q14));
    uvs[1][1]=Nq*(q11-q22+q33-q44);
    uvs[1][2]=Nq*(2.0f*(q34-q12));
    uvs[2][0]=Nq*(2.0f*(q24-q13));
    uvs[2][1]=Nq*(2.0f*(q34+q12));
    uvs[2][2]=Nq*(q11-q22-q33+q44);
}

void MakeQuatFromUvs(float (*uvs)[3], float *q)
{
    if (uvs[0][0] >= 0.0f) {
        if (uvs[1][1] + uvs[2][2] >= 0.0f) {
            q[0]=1.0f+uvs[0][0]+uvs[1][1]+uvs[2][2];
            q[1]=uvs[2][1]-uvs[1][2];
            q[2]=uvs[0][2]-uvs[2][0];
            q[3]=uvs[1][0]-uvs[0][1];
        } else {
            q[0]=uvs[2][1]-uvs[1][2];
            q[1]=1.0f+uvs[0][0]-uvs[1][1]-uvs[2][2];
            q[2]=uvs[1][0]+uvs[0][1];
            q[3]=uvs[0][2]+uvs[2][0];
        }
    } else {
        if (uvs[1][1]-uvs[2][2] >= 0.0f) {
            q[0]=uvs[0][2]-uvs[2][0];
            q[1]=uvs[1][0]+uvs[0][1];
            q[2]=1.0f-uvs[0][0]+uvs[1][1]-uvs[2][2];
            q[3]=uvs[2][1]+uvs[1][2];
        } else {
            q[0]=uvs[1][0]-uvs[0][1];
            q[1]=uvs[0][2]+uvs[2][0];
            q[2]=uvs[2][1]+uvs[1][2];
            q[3]=1.0f-uvs[0][0]-uvs[1][1]+uvs[2][2];
        }
    }
}

#pragma force_active on
static void FindBestQuat(float *q1, float *q2)
{
    signed short i;
    float dq1;
    float dq2;
    dq1 = dq2 = 0.0f;
    for (i = 0; i < 4; i++) {
        dq1 += fabs(q1[i] - q2[i]);
        dq2 += fabs(q1[i] + q2[i]);
    }
    if (dq2 < dq1) {
        q2[0] = -q2[0];
        q2[1] = -q2[1];
        q2[2] = -q2[2];
        q2[3] = -q2[3];
    }
}
#pragma force_active reset

void Interpolate(float *pointa, float *pointb, signed short p_size,
                 float vala, float valb, float inab, float *pointab)
{
    float scale = valb - vala;
    signed short i;
    if (scale != 0.0f)
        scale = (inab - vala) / scale;
    for (i = 0; i < p_size; i++)
        pointab[i] = pointa[i] + scale * (pointb[i] - pointa[i]);
}

void SetResurrectionData(signed short mode,tModelData *m,int abort)
{
    int p;
    switch(mode) {
    case 0:
        if(gGameState&8) {
            FindSurfaceUvsFromResPoint(1,-1,gDemoPathPt,m->resurrect.uvs);
            for(p=gCheckpoints.numPoints-1;p>=0;p--) {
                if(gDemoPathPt>=gCheckpoints.points[p].pathPts[0]) {
                    m->last_cp_index=p;
                    m->next_cp_index=GetNextCheckpoint(m->last_cp_index);
                    break;
                }
            }
            SetDroneResPathIndex(m,-1,gDemoPathPt);
            m->resurrect.save_pos[0]=gDronePaths[gGameCar[m->slot].mpctl.path].pts[gGameCar[m->slot].mpctl.idx].pos[0];
            m->resurrect.save_pos[1]=gDronePaths[gGameCar[m->slot].mpctl.path].pts[gGameCar[m->slot].mpctl.idx].pos[1];
            m->resurrect.save_pos[2]=gDronePaths[gGameCar[m->slot].mpctl.path].pts[gGameCar[m->slot].mpctl.idx].pos[2];
        } else if(gRace==kBattle || gRace==kStunt) {
            m->resurrect.save_pos[0]=gCheckpoints.points[(m->slot==0 ? 0 : m->slot==1 ? 2 : m->slot==2 ? 1 : 3)].pos[0];
            m->resurrect.save_pos[1]=gCheckpoints.points[(m->slot==0 ? 0 : m->slot==1 ? 2 : m->slot==2 ? 1 : 3)].pos[1];
            m->resurrect.save_pos[2]=gCheckpoints.points[(m->slot==0 ? 0 : m->slot==1 ? 2 : m->slot==2 ? 1 : 3)].pos[2];
            CopyUV(unitmatrix,m->resurrect.uvs);
            m->resurrect.uvs[2][0]=gCheckpoints.points[(m->slot==0 ? 0 : m->slot==1 ? 2 : m->slot==2 ? 1 : 3)].normal[0];
            m->resurrect.uvs[2][1]=gCheckpoints.points[(m->slot==0 ? 0 : m->slot==1 ? 2 : m->slot==2 ? 1 : 3)].normal[1];
            m->resurrect.uvs[2][2]=gCheckpoints.points[(m->slot==0 ? 0 : m->slot==1 ? 2 : m->slot==2 ? 1 : 3)].normal[2];
            m->resurrect.uvs[2][1]=0.0f;
            m->resurrect.uvs[2][0]*=-1.0f;
            m->resurrect.uvs[2][2]*=-1.0f;
            NormalVector(m->resurrect.uvs[2]);
            m->resurrect.uvs[0][0]=m->resurrect.uvs[1][1]*m->resurrect.uvs[2][2]-m->resurrect.uvs[1][2]*m->resurrect.uvs[2][1];
            m->resurrect.uvs[0][1]=m->resurrect.uvs[1][2]*m->resurrect.uvs[2][0]-m->resurrect.uvs[1][0]*m->resurrect.uvs[2][2];
            m->resurrect.uvs[0][2]=m->resurrect.uvs[1][0]*m->resurrect.uvs[2][1]-m->resurrect.uvs[1][1]*m->resurrect.uvs[2][0];
            NormalVector(m->resurrect.uvs[0]);
            SyncMaxpathToLastCheckpoint(m->slot);
        } else {
            m->resurrect.save_pos[0]=gResPath.pts[gCheckpoints.points[m->last_cp_index].pathPts[0]][0];
            m->resurrect.save_pos[1]=gResPath.pts[gCheckpoints.points[m->last_cp_index].pathPts[0]][1];
            m->resurrect.save_pos[2]=gResPath.pts[gCheckpoints.points[m->last_cp_index].pathPts[0]][2];
            FindSurfaceUvsFromResPoint(1,-1,gCheckpoints.points[m->last_cp_index].pathPts[0],m->resurrect.uvs);
            SyncMaxpathToLastCheckpoint(m->slot);
        }
        m->resurrect.crash_time=0.0f;
        break;
    case 1:
        break;
    case 2:
        m->resurrect.save_pos[0]=m->reckon.rwr[0];
        m->resurrect.save_pos[1]=m->reckon.rwr[1];
        m->resurrect.save_pos[2]=m->reckon.rwr[2];
        m->resurrect.crash_time=m->fastin.modelTime;
        break;
    }
}

void InitDemoPathPoint(void)
{
    static signed short track1[]={0,242,494};
    static signed short track2[]={0,258,484,700};
    static signed short track3[]={0,250,520,740};
    static signed short track4[]={0,230,430,730};
    static signed short track5[]={0,200,430,600};
    static signed short track6[]={0,230,420,680};
    static signed short track1Back[]={0,250,480};
    static signed short track2Back[]={0,270,525,740};
    static signed short track3Back[]={0,250,520,770};
    static signed short track4Back[]={0,220,450,760};
    static signed short track5Back[]={0,260,515,750,1030};
    static signed short track6Back[]={0,260,505,760,1027};
    static signed short *demo_path_points[]={track1,track2,track3,track4,track5,track6,track1Back,track2Back,track3Back,track4Back,track5Back,track6Back};
    static signed short num_demo_ppoints[]={3,4,4,4,4,4,3,4,4,4,5,5};
    static signed short cur_demo_ppoint[12];
    int dindex;
    int track=gBackwards ? gTrackNum+6 : gTrackNum;
    dindex=cur_demo_ppoint[track]+1;
    if(dindex<0 || dindex>=num_demo_ppoints[track]) dindex=0;
    cur_demo_ppoint[track]=dindex;
    gDemoPathPt=demo_path_points[track][dindex];
}

void CheckCrash(signed short drone_index)
{
    tCarData *gc=&gGameCar[drone_index];
    tModelData *m=&model[drone_index];
    switch (gc->crash_state) {
    case 2:
        break;
    case 0:
        if (gc->dead <= 0 && m->crashflag) {
            gc->restart=-1;
            SetResurrectionData(2,m,0);
            gc->crash_state=1;
            gc->crashtime=m->fastin.modelTime;
        }
        break;
    case 1:
        if (gRace == kBattle) {
            if (m->fastin.modelTime-gc->crashtime > 2.25f)
                gc->crash_state=2;
        } else {
            if (m->fastin.modelTime-gc->crashtime > 3.5f)
                gc->crash_state=2;
        }
        break;
    }
}

void FinishResurrecting(tModelData *m)
{
    if (m->resurrect.moving_state == -2) {
        m->fastin.modelrun=0;
        m->resurrect.moving_state=-1;
        m->initin.pos[0]=m->resurrect.pos[0];
        m->initin.pos[1]=m->resurrect.pos[1];
        m->initin.pos[2]=m->resurrect.pos[2];
        CopyUV(m->resurrect.uvs,m->initin.uvs);
        syminit(m);
        if ((float)m->resurrect.velocity >= 44.0f) {
            m->gear=m->commandgear=m->mainin.gear=2;
        } else {
            m->gear=m->commandgear=m->mainin.gear=1;
        }
        m->v[2]=m->resurrect.velocity;
        m->tireV[0][2]=m->resurrect.velocity;
        m->tires[0].angvel=m->tireV[0][2]/m->tires[0].tradius;
        m->tireV[1][2]=m->resurrect.velocity;
        m->tires[1].angvel=m->tireV[1][2]/m->tires[1].tradius;
        m->tireV[2][2]=m->resurrect.velocity;
        m->tires[2].angvel=m->tireV[2][2]/m->tires[2].tradius;
        m->tireV[3][2]=m->resurrect.velocity;
        m->tires[3].angvel=m->tireV[3][2]/m->tires[3].tradius;
        multigo(m->slot);
    }
}

void UpdateResurrectingCar(tModelData *m)
{
    static float save_pos[6][3];
    float pos[3],pos2[3];
    float f32_state,temp,scale;
    float uvs[3][3],quat[4];
    signed short i,node=m->slot;
    if(m->resurrect.moving_state==0) {
        pos2[0]=m->initin.pos[0]-m->resurrect.pos[0];
        pos2[1]=m->initin.pos[1]-m->resurrect.pos[1];
        pos2[2]=m->initin.pos[2]-m->resurrect.pos[2];
        temp=magnitude(pos2);
        temp*=0.125f;
        if(temp>100.0f) temp=100.0f;
        Interpolate(m->initin.pos,m->resurrect.pos,3,0.0f,resurrectTime,resurrectTime/2.0f,save_pos[node]);
        save_pos[node][1]+=temp;
        m->appearance&=~8;
    }
    m->resurrect.moving_state++;
    f32_state=m->fastin.modelTime-m->resurrect.resurrect_time;
    if(f32_state>resurrectTime) {
        if(m->appearance&16) m->appearance&=~16;
        m->resurrect.moving_state=-2;
        if(gLink[node].owner==6 || m->we_control) m->hide_car=0;
    } else {
        if(m->appearance&16) m->appearance&=~16;
        if(f32_state<resurrectTime/2.0f) {
            scale=2.0f*f32_state*f32_state/resurrectTime;
            Interpolate(m->initin.pos,save_pos[node],3,0.0f,resurrectTime/2.0f,scale,pos);
            Interpolate(m->resurrect.quat_start,m->resurrect.quat_end,4,0.0f,resurrectTime/2.0f,scale,quat);
            MakeUvsFromQuat(quat,uvs);
            scale=1.0f-2.0f*scale/resurrectTime;
            if(gLink[node].owner==6) {
                for(i=0;i<3;i++) {
                    if(fabs(m->rwv[i]*=scale)<0.01f) m->rwv[i]=0.0f;
                }
            }
            CopyUV(uvs,m->uvs);
            if(gSuperSpeed) make_super_speed(m);
        } else {
            Interpolate(save_pos[node],m->resurrect.pos,3,resurrectTime/2.0f,resurrectTime,f32_state,pos);
        }
        m->rwr[0]=pos[0];
        m->rwr[1]=pos[1];
        m->rwr[2]=pos[2];
    }
}

void BlinkResurrectingCars(void)
{
    signed short i;
    float delta_time;
    for (i = 0; i < 6; i++) {
        if (model[i].in_game && gGameCar[i].data_valid) {
            if (!gGameCar[i].solid) {
                delta_time = model[i].fastin.modelTime - gGameCar[i].collide_time;
                if (--gGameCar[i].collide_count < 0) {
                    if (++gGameCar[i].collide_state & 1) {
                        gGameCar[i].collide_count = 0;
                        model[i].appearance &= ~8;
                    } else {
                        switch ((int)delta_time) {
                        case 0: case 1: gGameCar[i].collide_count = 2; break;
                        case 2: case 3: gGameCar[i].collide_count = 1; break;
                        default: gGameCar[i].collide_count = 0; break;
                        }
                        if (gGameCar[i].dead <= 0)
                            model[i].appearance |= 8;
                    }
                }
            } else {
                model[i].appearance &= ~8;
            }
        }
    }
}

void AbortCar(signed short drone_index, int death)
{
    if (gModelTime < 5.0f) return;
    if (gRace == kStunt) CrashedStunt(drone_index);
    if (gRace == kBattle) InitBattlePlayer(drone_index);
    SetResurrectionData(2,&model[drone_index],1);
    gGameCar[drone_index].crash_state=2;
    if (gDeathMode && death)
        gGameCar[drone_index].dead=0;
    else
        gGameCar[drone_index].dead=-1;
}

void DetectDroneAbort(void)
{
    signed short i,node,abort_me;
    float pos2[3],delta[3];
    tModelData *m;
    tCarData *gc;
    int path,idx;
    if(gModelTime<5.0f) {
        for(i=0;i<6;i++) {
            model[i].stuck_time=0.0f;
            gGameCar[i].fwd_progress_t=0.0f;
            gGameCar[i].strayAngle=0.0f;
        }
        return;
    }
    for(i=0;i<gNumActiveCars;i++) {
        node=model[i].slot;
        m=&model[node];
        if(m->we_control || gLink[node].owner==6) {
            gc=&gGameCar[node];
            abort_me=m->please_abort_me;
            if(gc->dr_pos[1]<-190.0f && !gc->dead && !gc->crash_state && m->resurrect.moving_state==-1) {
                m->resurrect.autoAbort=1;
                AbortCar(m->slot,0);
                return;
            }
            if((unsigned int)(gRace-kStunt)>kObstacle-kStunt && gRace!=kBattle) {
                if((gGameState&0x400008) && gRace!=kPractice && !gNoAutoAbort && !gEndGame && !gCoastFlag &&
                   gc->place_locked!=1 && AreWeSolid(m->slot) && m->drone_scale>0.1f &&
                   (m->magvel<25.0f || abort_me)) {
                    if(!abort_me) {
                        if(m->stuck_time==0.0f) m->stuck_time=m->fastin.modelTime;
                        else m->stuck_time=m->fastin.modelTime;
                    }
                    if(abort_me) {
                        SetResurrectionData(2,m,1);
                        gc->crash_state=2;
                        m->stuck_time=0.0f;
                        gc->dead=-1;
                        m->resurrect.autoAbort=1;
                    }
                } else m->stuck_time=0.0f;
            }
        }
    }
    for(node=0;node<gNumPlayers;node++) {
        m=&model[gPlayers[node].slot];
        gc=&gGameCar[gPlayers[node].slot];
        if(gc->distance>gc->last_distance) {
            gc->fwd_progress_t=0.0f;
            gc->last_distance=gc->distance;
            gc->strayAngle=0.0f;
        } else if(!AreWeSolid(m->slot)) {
            gc->fwd_progress_t=0.0f;
            gc->strayAngle=0.0f;
        } else if(gc->fwd_progress_t==0.0f) {
            gc->fwd_progress_t=m->fastin.modelTime;
        } else if(m->fastin.modelTime-gc->fwd_progress_t>2.0f) {
            path=gc->lastResPath;
            idx=gc->lastResIdx;
            for(i=0;i<5;i++) NextResPathPt(path,idx,&path,&idx);
            if(path>=0) {
                delta[0]=gResPath.alt[path].pts[idx][0]-gc->dr_pos[0];
                delta[1]=gResPath.alt[path].pts[idx][1]-gc->dr_pos[1];
                delta[2]=gResPath.alt[path].pts[idx][2]-gc->dr_pos[2];
            } else {
                delta[0]=gResPath.pts[idx][0]-gc->dr_pos[0];
                delta[1]=gResPath.pts[idx][1]-gc->dr_pos[1];
                delta[2]=gResPath.pts[idx][2]-gc->dr_pos[2];
            }
            BodyVector(delta,pos2,gc->dr_uvs);
            gc->strayAngle=fatan2(pos2[0],pos2[2]);
        }
    }
}

int AreWeSolid(signed short node)
{
    tModelData *m = &model[node];
    tCarData *gc = &gGameCar[node];
    return gc->crash_state == 0 && m->crashflag == 0 &&
           m->resurrect.moving_state == -1 && m->solid && !m->hide_car;
}
