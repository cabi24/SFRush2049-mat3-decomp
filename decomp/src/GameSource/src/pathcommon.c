/* pathcommon.c - checkpoint and path loading, traversal, and distance queries.
   Status: NonMatching. All eighteen complete function bodies reproduce the GC ELF.
   Original-placement instruction, relocation and literal validation is recorded separately.
   A fresh private GC/2.7 link matches all unit ranges and the trusted DOL exception.
   Matching promotion is proposed separately; canonical files are unchanged.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\pathcommon.c; assembly: build/2049LE/asm/GameSource/src/pathcommon.s */
#include "rush/rush_protos.h"
#include "msl_math_inline.h"

#define SWAP16(v) ((((v) & 0xff) << 8) + (((v) >> 8) & 0xff))
#define SWAP32(v) (((v) << 24) + ((v) >> 24) + (((v) << 8) & 0xff0000) + (((v) >> 8) & 0xff00))

static signed short FindAltPathIntersect(signed short cp, signed short path);
static void CalcCheckpointTimes(void);
static void FindPathDistances(void);
static int TraceToCheckpoint(int inPath, int inIdx, int *cp, int depth);
static void FindClosestPt(int alt, int first, signed short *pos, int *path, int *index);

static int gPathTrace[15];

static inline float PathDistance(float x, float y, float z)
{
    return sqrtf(x*x + y*y + z*z);
}

/* 0x80031360, size 0x2f0 */
void ReadCheckpointData(void)
{
    signed short i;
    signed short j;
    signed short p;
    anon_startTime_loopPt_32c *tTrack = (anon_startTime_loopPt_32c *)gPathWave;
    if (g_bIfReload)
        _Disenable_swap();
    else
        _Enable_swap();
    _swap_x16(&tTrack->startTime);
    _swap_x16(&tTrack->loopPt);
    _swap_x16(&tTrack->finishPt);
    _swap_x16(&tTrack->startPt);
    _swap_x16(&tTrack->numPoints);
    for (i = 0; i < 10; ++i) {
        _swap_x32(&tTrack->points[i].pos[0]);
        _swap_x32(&tTrack->points[i].pos[1]);
        _swap_x32(&tTrack->points[i].pos[2]);
        _swap_x32(&tTrack->points[i].normal[0]);
        _swap_x32(&tTrack->points[i].normal[1]);
        _swap_x32(&tTrack->points[i].normal[2]);
        _swap_x32(&tTrack->points[i].radius);
        _swap_x16(&tTrack->points[i].flags);
        _swap_x16(&tTrack->points[i].time[0]);
        _swap_x16(&tTrack->points[i].time[1]);
        _swap_x16_array((unsigned char *)tTrack->points[i].pathPts, 40);
        _swap_x32(&gCheckpoints.points[i].distToNext);
    }
    memcpy(&gCheckpoints, gPathWave, sizeof(gCheckpoints));
    gCheckpoints.finishPt = gCheckpoints.loopPt = gCheckpoints.startPt = -1;
    gCheckpoints.startTime = 90;
    gCheckpoints.numPoints = SWAP16(gCheckpoints.numPoints);
    for (i = 0; i < gCheckpoints.numPoints; ++i) {
        for (j = 0; j < 3; ++j) {
            *(unsigned int *)&gCheckpoints.points[i].pos[j] = SWAP32(*(unsigned int *)&gCheckpoints.points[i].pos[j]);
            *(unsigned int *)&gCheckpoints.points[i].normal[j] = SWAP32(*(unsigned int *)&gCheckpoints.points[i].normal[j]);
        }
        *(unsigned int *)&gCheckpoints.points[i].radius = SWAP32(*(unsigned int *)&gCheckpoints.points[i].radius);
        gCheckpoints.points[i].flags = SWAP16(gCheckpoints.points[i].flags);
        if (gCheckpoints.finishPt < 0 && (gCheckpoints.points[i].flags & 1))
            gCheckpoints.finishPt = i;
        if (gCheckpoints.loopPt < 0 && (gCheckpoints.points[i].flags & 4))
            gCheckpoints.loopPt = i;
        if (gCheckpoints.startPt < 0 && (gCheckpoints.points[i].flags & 2))
            gCheckpoints.startPt = i;
        for (p = 0; p <= 19; ++p)
            gCheckpoints.points[i].pathPts[p] = 0;
        gCheckpoints.points[i].time[0] = gCheckpoints.points[i].time[1] = 45;
    }
    if (gCheckpoints.finishPt < 0)
        gCheckpoints.finishPt = 0;
    if (gCheckpoints.loopPt < 0)
        gCheckpoints.loopPt = 0;
    if (gCheckpoints.startPt < 0)
        gCheckpoints.startPt = 0;
}

/* 0x80031650, size 0x180 */
static signed short FindAltPathIntersect(signed short cp, signed short path)
{
    signed short p;
    signed short sign;
    signed short newSign;
    float nx,nz,dx,dz;
    int first = 1;
    if (path >= gResPath.numAlt)
        return -1;
    if (gResPath.alt[path].resurrect == 2) {
        if (cp == 0)
            return 0;
        else
            return -1;
    }
    nx = gCheckpoints.points[cp].normal[0];
    nz = gCheckpoints.points[cp].normal[2];
    for (p = 0; p < gResPath.alt[path].numPts; ++p) {
        dx = gResPath.alt[path].pts[p][0] - gCheckpoints.points[cp].pos[0];
        dz = gResPath.alt[path].pts[p][2] - gCheckpoints.points[cp].pos[2];
        if (dx *
            dx +
            dz *
            dz <=
            gCheckpoints.points[cp].radius) {
            newSign = (dx *
                       nx +
                       dz *
                       nz) < 0.0f ? -1 : 1;
            if (first) {
                sign = newSign;
                first = 0;
            } else if (sign != newSign)
                return p;
        }
    }
    return -1;
}

/* 0x800317d0, size 0x190 */
signed short FindResPathIntersect(signed short cp)
{
    float nx,nz,dx,dz;
    float pDist;
    float ppDist;
    float minDist = 1.0e20f;
    signed short p;
    signed short pp;
    signed short pSign;
    signed short ppSign;
    signed short closest;
    nx = gCheckpoints.points[cp].normal[0];
    nz = gCheckpoints.points[cp].normal[2];
    for (pp = -1, p = 0; pp < gResPath.numPts; ++p, ++pp) {
        if (p == gResPath.numPts)
            p = 0;
        dx = gResPath.pts[p][0] - gCheckpoints.points[cp].pos[0];
        dz = gResPath.pts[p][2] - gCheckpoints.points[cp].pos[2];
        pDist = dx *
                dx +
                dz *
                dz;
        pSign = (dx *
                 nx +
                 dz *
                 nz) < 0.0f ? -1 : 1;
        if (pDist < minDist) {
            minDist = pDist;
            closest = p;
        }
        if (pp >= 0 && pDist <= gCheckpoints.points[cp].radius &&
            ppDist <= gCheckpoints.points[cp].radius && pSign != ppSign)
            break;
        ppDist = pDist;
        ppSign = pSign;
    }
    if (pp == gResPath.numPts)
        return closest;
    else
        return p;
}

/* 0x80031960, size 0x1b0 */
signed short FindDronePathIntersect(signed short cp, signed short path)
{
    float nx,nz,dx,dz;
    float pDist;
    float ppDist;
    float minDist = 1.0e20f;
    signed short p;
    signed short pp;
    signed short pSign;
    signed short ppSign;
    signed short closest;
    nx = gCheckpoints.points[cp].normal[0];
    nz = gCheckpoints.points[cp].normal[2];
    for (pp = -1, p = 0; pp < gDronePaths[path].numPts; ++p, ++pp) {
        if (p == gDronePaths[path].numPts)
            p = 0;
        dx = gDronePaths[path].pts[p].pos[0] - gCheckpoints.points[cp].pos[0];
        dz = gDronePaths[path].pts[p].pos[2] - gCheckpoints.points[cp].pos[2];
        pDist = dx *
                dx +
                dz *
                dz;
        pSign = (dx *
                 nx +
                 dz *
                 nz) < 0.0f ? -1 : 1;
        if (pDist < minDist) {
            minDist = pDist;
            closest = p;
        }
        if (pp >= 0 && pDist <= gCheckpoints.points[cp].radius &&
            ppDist <= gCheckpoints.points[cp].radius && pSign != ppSign)
            break;
        ppDist = pDist;
        ppSign = pSign;
    }
    if (pp == gDronePaths[path].numPts)
        return closest;
    else
        return p;
}

/* 0x80031b10, size 0x138 */
void SyncCheckpointsToPaths(signed short path)
{
    int i;
    int p;
    for (i = 0; i < gCheckpoints.numPoints; ++i) {
        if (path < 0) {
            for (p = 0; p <= 19; ++p) {
                if (p == 0)
                    gCheckpoints.points[i].pathPts[p] = FindResPathIntersect(i);
                else if (p > 4)
                    gCheckpoints.points[i].pathPts[p] = FindAltPathIntersect(i, p - 5);
                else
                    gCheckpoints.points[i].pathPts[p] = FindDronePathIntersect(i, p - 1);
            }
        } else if (path == 0) {
            gCheckpoints.points[i].pathPts[path] = FindResPathIntersect(i);
            for (p = 0; p < gResPath.numAlt; ++p)
                gCheckpoints.points[i].pathPts[p + 5] = FindAltPathIntersect(i, p);
        } else
            gCheckpoints.points[i].pathPts[path] = FindDronePathIntersect(i, path - 1);
    }
    FindPathDistances();
}

/* 0x80031c48, size 0x59c */
static void CalcCheckpointTimes(void)
{
    anon_numPts_pts_8 *dronePath;
    int lap;
    int cp;
    int ncp;
    int endPt;
    int i;
    float time;
    int started;
    float x, y, z;
    for (lap = 0; lap < 2; ++lap) {
        started = gCheckpoints.startPt == 0 || lap != 0;
        cp = lap == 0 ? gCheckpoints.startPt : gCheckpoints.finishPt;
        for (;;) {
            dronePath = gDronePaths;
            ncp = GetNextCheckpoint(cp);
            if (ncp < cp)
                endPt = dronePath->numPts - 1;
            else
                endPt = gCheckpoints.points[ncp].pathPts[1] - 1;
            time = 0.0f;
            for (i = gCheckpoints.points[cp].pathPts[1]; i < endPt; ++i) {
                x = dronePath->pts[i+1].pos[0] - dronePath->pts[i].pos[0];
                y = dronePath->pts[i+1].pos[1] - dronePath->pts[i].pos[1];
                z = dronePath->pts[i+1].pos[2] - dronePath->pts[i].pos[2];
                time += PathDistance(x, y, z) / (dronePath->pts[i].mph * 1.4666667f);
            }
            x = dronePath->pts[NextPathPt(i, 0)].pos[0] - dronePath->pts[i].pos[0];
            y = dronePath->pts[NextPathPt(i, 0)].pos[1] - dronePath->pts[i].pos[1];
            z = dronePath->pts[NextPathPt(i, 0)].pos[2] - dronePath->pts[i].pos[2];
            time += PathDistance(x, y, z) / (dronePath->pts[i].mph * 1.4666667f);
            gCheckpoints.points[cp].time[lap] = time;
            cp = ncp;
            if (cp == gCheckpoints.finishPt) {
                if (started)
                    break;
                started = 1;
            }
        }
    }
    time = 0.0f;
    gCheckpoints.startTime = gCheckpoints.points[0].time[0] + 10;
    cp = gCheckpoints.finishPt;
    do {
        time += gCheckpoints.points[cp].time[1];
        cp = GetNextCheckpoint(cp);
    } while (cp != gCheckpoints.finishPt);
    gCheckpoints.startTime += time / 30.0f;
    gCheckpoints.points[gCheckpoints.finishPt].time[1] += time / 30.0f;
}

/* 0x800321e4, size 0x28 */
unsigned int GetNextCheckpoint(unsigned int cp)
{
    if (cp == gCheckpoints.numPoints - 1)
        return gCheckpoints.loopPt;
    return cp + 1;
}

/* 0x8003220c, size 0x3b0 */
void ReadPathData(void)
{
    signed short i;
    signed short p;
    anon_numPts_pts_10 *tResPath;
    anon_numPts_pts_8 *tDronePath;
    char *ptr;
    signed short j;
    if (gRestartGame) {
        CalcCheckpointTimes();
        SyncCheckpointsToPaths(-1);
        SyncAltPaths();
        CalcResPathBounds();
        CalcCheckpointTimes();
        return;
    }
    ptr = gPathWave + sizeof(gCheckpoints);
    tResPath = (anon_numPts_pts_10 *)ptr;
    _Enable_swap();
    _swap_x16(&tResPath->numPts);
    _swap_x32(&tResPath->pts);
    _swap_x32(&tResPath->alt);
    memcpy(&gResPath, ptr, sizeof(gResPath));
    ptr += sizeof(gResPath);
    gAltPaths = (anon_resurrect_firstPath_10 *)ptr;
    gResPath.numPts = SWAP16(*(short *)&gResPath.numPts);
    ptr += gResPath.numAlt * sizeof(*gAltPaths);
    for (i = 0; i < gResPath.numAlt; ++i) {
        _swap_x16(&gAltPaths[i].firstIdx);
        _swap_x16(&gAltPaths[i].lastIdx);
        _swap_x16(&gAltPaths[i].numPts);
        _swap_x32(&gAltPaths[i].pts);
        *(short *)&gAltPaths[i].firstIdx = SWAP16(*(short *)&gAltPaths[i].firstIdx);
        *(short *)&gAltPaths[i].lastIdx = SWAP16(*(short *)&gAltPaths[i].lastIdx);
        *(short *)&gAltPaths[i].numPts = SWAP16(*(short *)&gAltPaths[i].numPts);
    }
    _swap_x16(ptr);
    memcpy(&gNumResPathPts, ptr, sizeof(gNumResPathPts));
    ptr += sizeof(gNumResPathPts);
    gNumResPathPts = SWAP16(*(short *)&gNumResPathPts);
    gResPathPtList = (signed short (*)[3])ptr;
    ptr += gResPath.numPts * sizeof(*gResPathPtList);
    for (i = 0; i < gNumResPathPts; ++i)
        for (j = 0; j < 3; ++j) {
            _swap_x16(&gResPathPtList[i][j]);
            *(short *)&gResPathPtList[i][j] = SWAP16(*(short *)&gResPathPtList[i][j]);
        }
    gResPath.pts = gResPathPtList;
    if (gResPath.numAlt)
        gResPath.alt = gAltPaths;
    else
        gResPath.alt = gAltPaths = 0;
    for (i = 0; i < gResPath.numAlt; ++i) {
        gAltPaths[i].pts = (signed short (*)[3])ptr;
        ptr += gAltPaths[i].numPts * sizeof(*gResPathPtList);
    }
    for (p = 0; p < 4; ++p) {
        tDronePath = (anon_numPts_pts_8 *)ptr;
        _swap_x16(&tDronePath->numPts);
        _swap_x32(&tDronePath->pts);
        memcpy(&gDronePaths[p], ptr, sizeof(gDronePaths[p]));
        ptr += sizeof(gDronePaths[p]);
        *(short *)&gDronePaths[p].numPts = SWAP16(*(short *)&gDronePaths[p].numPts);
        gNumPathPts[p] = gDronePaths[p].numPts;
        gPathPtList[p] = (anon_pos_mph_8 *)ptr;
        gDronePaths[p].pts = (anon_pos_mph_8 *)ptr;
        ptr += gDronePaths[p].numPts * sizeof(*gPathPtList[p]);
        for (i = 0; i < gDronePaths[p].numPts; ++i)
            for (j = 0; j < 3; ++j) {
                _swap_x16(&gDronePaths[p].pts[i].pos[j]);
                *(short *)&gDronePaths[p].pts[i].pos[j] = SWAP16(*(short *)&gDronePaths[p].pts[i].pos[j]);
            }
    }
    SyncCheckpointsToPaths(-1);
    SyncAltPaths();
    CalcResPathBounds();
    CalcCheckpointTimes();
}

/* 0x800325bc, size 0xe4 */
static void FindPathDistances(void)
{
    int i;
    int n;
    int finishReached = 0;
    gLapDistance = gFirstLapDistance = 0.0f;
    for (i = 0; i < gCheckpoints.numPoints; ++i) {
        n = GetNextCheckpoint(i);
        gCheckpoints.points[i].distToNext = GetPathDistance(
            gCheckpoints.points[i].pathPts[0], gCheckpoints.points[n].pathPts[0]);
        if (i > 0 && gCheckpoints.finishPt == i)
            finishReached = 1;
        if (!finishReached)
            gFirstLapDistance += gCheckpoints.points[i].distToNext;
        if (i >= gCheckpoints.loopPt)
            gLapDistance += gCheckpoints.points[i].distToNext;
    }
}

/* 0x800326a0, size 0x3a8 */
float GetPathDistance(int from, int to)
{
    int i;
    int n;
    int end;
    float dist;
    if (from > to && to < gCheckpoints.points[gCheckpoints.loopPt].pathPts[0])
        return -1.0f;
    if (from > to)
        end = gResPath.numPts;
    else
        end = to;
    dist = 0.0f;
    for (i = from; i < end; ++i) {
        float dx, dz;
        NextResPathPt(-1, i, 0, &n);
        dx = gResPath.pts[n][0] - gResPath.pts[i][0];
        dz = gResPath.pts[n][2] - gResPath.pts[i][2];
        dist += PathDistance(dx, 0.0f, dz);
    }
    if (end != to)
        for (i = gCheckpoints.points[gCheckpoints.loopPt].pathPts[0]; i < to; ++i) {
            float dx, dz;
            NextResPathPt(-1, i, 0, &n);
            dx = gResPath.pts[n][0] - gResPath.pts[i][0];
            dz = gResPath.pts[n][2] - gResPath.pts[i][2];
            dist += PathDistance(dx, 0.0f, dz);
        }
    return dist;
}

/* 0x80032a48, size 0xb0 */
void NextResPathPt(int inPath, int inIdx, int *outPath, int *outIdx)
{
    if (inPath >= 0) {
        if (inIdx == gResPath.alt[inPath].numPts - 1) {
            *outIdx = gResPath.alt[inPath].lastIdx;
            *outPath = gResPath.alt[inPath].lastPath;
        } else {
            *outIdx = inIdx + 1;
            *outPath = inPath;
        }
    } else {
        if (inIdx == gResPath.numPts - 1)
            *outIdx = gCheckpoints.points[gCheckpoints.loopPt].pathPts[0];
        else
            *outIdx = inIdx + 1;
        if (outPath)
            *outPath = -1;
    }
}

/* 0x80032af8, size 0x80 */
signed short PrevPathPt(signed short idx, signed short path)
{
    if (idx == 0 && gCheckpoints.loopPt > 0)
        return idx;
    if (idx == 0 || idx == gCheckpoints.points[gCheckpoints.loopPt].pathPts[path + 1])
        return gDronePaths[path].numPts - 1;
    return idx - 1;
}

/* 0x80032b78, size 0x54 */
signed short NextPathPt(signed short idx, signed short path)
{
    if (idx == gDronePaths[path].numPts - 1)
        return gCheckpoints.points[gCheckpoints.loopPt].pathPts[path + 1];
    return idx + 1;
}

/* 0x80032bcc, size 0xb4 */
int TraceCheck(int inPath, int depth)
{
    int i;
    if (inPath >= 0 && gResPath.alt[inPath].resurrect &&
        ((gResPath.alt[inPath].firstPath >= 0 &&
          !gResPath.alt[gResPath.alt[inPath].firstPath].resurrect) ||
         (gResPath.alt[inPath].lastPath >= 0 &&
          !gResPath.alt[gResPath.alt[inPath].lastPath].resurrect)))
        return 0;
    gPathTrace[depth] = inPath;
    for (i = 0; i < depth; ++i)
        if (gPathTrace[i] == inPath)
            return 0;
    return 1;
}

/* 0x80032c80, size 0x150 */
static int TraceToCheckpoint(int inPath, int inIdx, int *cp, int depth)
{
    int i;
    int idx;
    if (!TraceCheck(inPath, depth))
        return 0;
    if (inPath < 0) {
        for (i = 1; i < gCheckpoints.numPoints; ++i)
            if (inIdx < gCheckpoints.points[i].pathPts[0])
                break;
        *cp = i - 1;
        return 1;
    }
    idx = -1;
    for (i = 0; i < gCheckpoints.numPoints; ++i) {
        if ((short)gCheckpoints.points[i].pathPts[inPath + 5] >= 0 &&
            (idx < 0 || idx < gCheckpoints.points[i].pathPts[inPath + 5]) &&
            inIdx >= gCheckpoints.points[i].pathPts[inPath + 5]) {
            *cp = i;
            idx = gCheckpoints.points[i].pathPts[inPath + 5];
        }
    }
    if (idx >= 0)
        return 1;
    return TraceToCheckpoint(gResPath.alt[inPath].firstPath,
                             gResPath.alt[inPath].firstIdx, cp, depth + 1);
}

/* 0x80032dd0, size 0x234 */
static void FindClosestPt(int alt, int first, signed short *pos, int *path, int *index)
{
    float min = 1.0e20f;
    float dx;
    float dy;
    float dz;
    float dist;
    int p;
    int a;
    *path = -1;
    for (p = 0; p < gResPath.numPts; ++p) {
        dx = gResPath.pts[p][0] - pos[0];
        dy = gResPath.pts[p][1] - pos[1];
        dz = gResPath.pts[p][2] - pos[2];
        dist = dx * dx + dy * dy + dz * dz;
        if (dist < min) {
            min = dist;
            *index = p;
        }
    }
    for (a = 0; a < gResPath.numAlt; ++a) {
        if (a == alt)
            continue;
        if (first && a < alt && gResPath.alt[a].firstPath == alt && gResPath.alt[a].firstIdx == 0)
            continue;
        if (!first && a < alt && gResPath.alt[a].lastPath == alt &&
            gResPath.alt[a].lastIdx == gResPath.alt[alt].numPts - 1)
            continue;
        for (p = 0; p < gResPath.alt[a].numPts; ++p) {
            dx = gResPath.alt[a].pts[p][0] - pos[0];
            dy = gResPath.alt[a].pts[p][1] - pos[1];
            dz = gResPath.alt[a].pts[p][2] - pos[2];
            dist = dx * dx + dy * dy + dz * dz;
            if (dist < min) {
                *index = p;
                min = dist;
                *path = a;
            }
        }
    }
}

/* 0x80033004, size 0x1b0 */
void SyncAltPaths(void)
{
    int i;
    int cp;
    int path;
    int index;
    for (i = 0; i < gResPath.numAlt; ++i) {
        if (gResPath.alt[i].resurrect == 2) {
            gResPath.alt[i].firstPath = -1;
            gResPath.alt[i].firstIdx = 0;
            gResPath.alt[i].lastPath = -1;
            gResPath.alt[i].lastIdx = 1;
        } else {
            FindClosestPt(i, 1, gResPath.alt[i].pts[0], &path, &index);
            gResPath.alt[i].firstPath = path;
            gResPath.alt[i].firstIdx = index;
            FindClosestPt(i, 0, gResPath.alt[i].pts[gResPath.alt[i].numPts - 1], &path, &index);
            gResPath.alt[i].lastPath = path;
            gResPath.alt[i].lastIdx = index;
        }
    }
    for (i = 0; i < gResPath.numAlt; ++i) {
        if (gResPath.alt[i].resurrect == 2)
            gResPath.alt[i].startSection = -1;
        else if (!TraceToCheckpoint(i, 0, &cp, 0))
            gResPath.alt[i].startSection = -1;
        else
            gResPath.alt[i].startSection = cp;
    }
}

/* 0x800331b4, size 0x170 */
void CalcResPathBounds(void)
{
    int i;
    int j;
    int validPt;
    gPathMin[0] = gPathMin[1] = gPathMin[2] = 32767;
    gPathMax[0] = gPathMax[1] = gPathMax[2] = -32767;
    for (i = 0; i < gNumResPathPts; ++i) {
        if (i < gResPath.numPts)
            validPt = 1;
        else {
            validPt = 0;
            for (j = 0; j < gResPath.numAlt; ++j)
                if (gResPathPtList[i] >= gResPath.alt[j].pts[0] &&
                    gResPathPtList[i] < gResPath.alt[j].pts[gResPath.alt[j].numPts] &&
                    gResPath.alt[j].resurrect == 1)
                    validPt = 1;
        }
        if (validPt == 1)
            for (j = 0; j < 3; ++j) {
                gPathMin[j] = gPathMin[j] < gResPathPtList[i][j] ? gPathMin[j] : gResPathPtList[i][j];
                gPathMax[j] = gPathMax[j] > gResPathPtList[i][j] ? gPathMax[j] : gResPathPtList[i][j];
            }
    }
}

