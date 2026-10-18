/* maxpath.c - maximum-path control and path lookup.
   Status: Matching. */
#include "rush/rush_protos.h"
#include "msl_math_inline.h"

static int FindResPtRecurse(int inPath, int inIdx, int *outPath, int *outIdx, int honorAltRes, int depth);
static void IntervalPosChange(tModelData *m, tPathPoint *cp, int change_flag);
static void IntervalPos(tModelData *m, tPathPoint *cp);
static signed short FindClosestDronePathPtEfficiently(signed short *, signed short, signed short, signed short, signed short);
static void FindInterval(tModelData *m, tPathPoint *cp);
static void TargetSpeed(tPathPoint *cp);
static void TargetSteerPos(tModelData *m, tPathPoint *cp);
static void AdjustSpeed(tModelData *m, float tspd);
static void AdjustSteer(tModelData *m, float *pos);
static void AvoidAreas(signed short drone_index);


unsigned short gNumResPathPts;
unsigned short gNumPathPts[4];
float gFirstLapDistance;
float gLapDistance;
signed short gPathMin[3];
signed short gPathMax[3];
signed short (*gResPathPtList)[3];
anon_resurrect_firstPath_10 *gAltPaths;
char *gPathWave;
signed char gGravityScale;
static signed short current_drone;
static float gLastPathSwitchTime;
anon_pos_mph_8 *gPathPtList[4];
static int gPathSearch[15];
static float last_save_dir[6][3];
static float last_dir[6][3];
anon_numPts_pts_10 gResPath;
anon_numPts_pts_8 gDronePaths[4];
static float lastthrottle[6];
static float lastbrake[6];
static float lastwheel[6];

/* 0x8002562c, 0x98 bytes */
void InitPathData(void)
{
    signed short i;
    ReadPathData();
    for (i = 0; i < 6; i++) {
        lastwheel[i] = 0.0f;
        lastbrake[i] = 0.0f;
        lastthrottle[i] = 0.0f;
    }
    gLastPathSwitchTime = gModelTime;
    current_drone = -1;
}

/* 0x800256c4, 0x94 bytes */
void InitCarPath(tModelData *m, tCarData *gc)
{
    static int p;
    gc->mpctl.idx = 0;
    gc->mpctl.xrel = 0.0f;
    gc->mpctl.yrel = 0.0f;
    gc->mpctl.interval_time = gModelTime;
    gc->mpctl.newIdx = -1;
    if (m->drone_type == 1) {
        gc->mpctl.path = p;
        gc->mpctl.basePath = p;
        if (++p >= 4)
            p = 0;
    } else {
        gc->mpctl.path = 0;
    }
    if (gCheckpoints.startPt == 0)
        m->started = 1;
    gc->lastResPath = -1;
    gc->lastResIdx = 0;
    gc->resIdx = 0;
}

/* 0x80025758, 0x50 bytes */
void SyncMaxpathToLastCheckpoint(signed short node)
{
    gGameCar[node].mpctl.newIdx = gCheckpoints.points[model[node].last_cp_index].pathPts[1 + gGameCar[node].mpctl.path];
}

/* 0x800257a8, 0x13c bytes */
void MaxPathControls(tModelData *m)
{
    static float gravitySpeedIncrease[4] = {0.8f, 1.0f, 1.25f, 1.5f};
    static float gravitySpeedMin[4] = {75.0f, 90.0f, 135.0f, 180.0f};
    float tspd;
    tPathPoint *cp = &gGameCar[m->slot].mpctl;
    FindInterval(m, cp);
    TargetSpeed(cp);
    TargetSteerPos(m, cp);
    AvoidAreas(m->slot);
    tspd = cp->tgtspd;
    if (m->rwv[1] < 0.0f)
        tspd = gravitySpeedIncrease[gGravityScale] * tspd;
    else if (tspd < gravitySpeedMin[gGravityScale])
        tspd = gravitySpeedMin[gGravityScale];
    tspd *= m->drone_scale;
    AdjustSpeed(m, tspd);
    AdjustSteer(m, cp->tgtpos);
    m->please_abort_me = (gModelTime - cp->interval_time > 1.0f && cp->idx > 10);
    if (fabs(cp->xrel) > 100.0 || fabs(cp->yrel) > 100.0)
        m->please_abort_me = 1;
}

/* 0x800258e4, 0xa8 bytes */
static void FindInterval(tModelData *m, tPathPoint *cp)
{
    IntervalPosChange(m, cp, 1);
    if (cp->xrel >= cp->len) {
        cp->idx = NextPathPt(cp->idx, cp->path);
        cp->interval_time = gModelTime;
        IntervalPosChange(m, cp, 0);
    } else if (cp->xrel < 0.0f) {
        cp->idx = PrevPathPt(cp->idx, cp->path);
        IntervalPosChange(m, cp, 0);
    }
}

/* 0x8002598c, 0x774 bytes */
static void IntervalPosChange(tModelData *m, tPathPoint *cp, int change_flag)
{
    signed short j;
    signed short highIndex;
    signed short highPath;
    int path;
    int idx;
    int nidx;
    float x, y, z;
    float score, highScore;
    float delta_spd, mspd;
    float xpos, zpos;
    float x1, z1, x2, z2;
    float dx, dz, b1, c1, dist;
    float speed, xz_speed;
    float vec0, vec1;
    float time;
    signed short node;
    signed short pos[3];
    node = m->slot;
    if (cp->newIdx >= 0) {
        cp->interval_time = gModelTime;
        cp->newIdx = -1;
    }
    time = gModelTime;
    if (change_flag && time - gLastPathSwitchTime > 0.5f &&
        (current_drone == -1 || node == drones[current_drone])) {
        /* Observe the shared drone count on each pass. */
        for (j = 0; j < (*(volatile signed short *)&num_drones); j++) {
            current_drone++;
            if (current_drone >= (*(volatile signed short *)&num_drones))
                current_drone = 0;
            if (model[drones[current_drone]].we_control)
                break;
        }
        gLastPathSwitchTime = time;
        highPath = -1;
        highIndex = 0;
        x = m->rwv[0];
        y = m->rwv[1];
        z = m->rwv[2];
        xpos = m->reckon.rwr[0];
        zpos = m->reckon.rwr[2];
        xz_speed = x * x + z * z;
        speed = xz_speed + y * y;
        highScore = 0.0f;
        if (xz_speed < 2500.0f) {
            xz_speed = 0.0f;
            vec1 = 0.0f;
            vec0 = 0.0f;
        } else {
            xz_speed = 1.0f / sqrtf(xz_speed);
            vec0 = z * xz_speed;
            vec1 = x * xz_speed;
        }
        for (path = 0; path < 4; path++) {
            score = 0.0f;
            pos[0] = m->reckon.rwr[0];
            pos[1] = m->reckon.rwr[1];
            pos[2] = m->reckon.rwr[2];
            idx = FindClosestDronePathPtEfficiently(pos, cp->path, cp->idx, m->last_cp_index, path);
            z1 = gDronePaths[path].pts[idx].pos[2];
            x1 = gDronePaths[path].pts[idx].pos[0];
            nidx = NextPathPt(idx, path);
            z2 = gDronePaths[path].pts[nidx].pos[2];
            x2 = gDronePaths[path].pts[nidx].pos[0];
            dx = x1 - xpos;
            dz = z1 - zpos;
            if (dz * dz + dx * dx < 400.0f) {
                dz = z1 - z2;
                dx = x1 - x2;
                b1 = -dz;
                c1 = x1 * dz - z1 * dx;
                dist = sqrtf(dx * dx + b1 * b1);
                if (dist > 0.01f) {
                    dist = fabs(c1 + (dx * zpos + b1 * xpos)) / dist;
                    if (dist < 10.0f)
                        score = 1.0f - 0.1f * dist;
                }
            }
            mspd = 1.4666667f * gDronePaths[path].pts[idx].mph;
            delta_spd = fabs(speed - mspd * mspd);
            if (delta_spd < 73.333336f)
                score += 0.5f * (1.0f - delta_spd / 73.333336f);
            if (path == cp->basePath)
                score += 1.0f;
            if (xz_speed == 0.0f) {
                score += 1.0f;
            } else {
                x2 = x2 - x1;
                dz = z2 - z1;
                dist = 1.0f / sqrtf(dz * dz + x2 * x2);
                x2 *= dist;
                dz *= dist;
                dist = 0.5f * (1.0f + (vec0 * dz + vec1 * x2));
                score += dist;
            }
            if (score > highScore) {
                highScore = score;
                highIndex = idx;
                highPath = path;
            }
        }
        if (highPath != -1 && highPath != cp->path) {
            cp->idx = highIndex;
            cp->interval_time = time;
            cp->newIdx = -1;
            cp->path = highPath;
        }
    }
    IntervalPos(m, cp);
}


/* 0x80026100, 0x1c0 bytes */
static void IntervalPos(tModelData *m, tPathPoint *cp)
{
    anon_pos_mph_8 *pt = &gDronePaths[cp->path].pts[cp->idx];
    anon_pos_mph_8 *npt;
    anon_pos_mph_8 *nnpt;
    signed short nextIdx = NextPathPt(cp->idx, cp->path);
    float x, z;
    float n[2];
    npt = &gDronePaths[cp->path].pts[nextIdx];
    n[0] = npt->pos[0] - pt->pos[0];
    n[1] = npt->pos[2] - pt->pos[2];
    cp->len = TB_Vector2Normalize(n);
    x = m->rwr[0] - pt->pos[0];
    z = m->rwr[2] - pt->pos[2];
    cp->xrel = z * n[1] + x * n[0];
    cp->yrel = x * n[1] - z * n[0];
    nextIdx = NextPathPt(nextIdx, cp->path);
    nnpt = &gDronePaths[cp->path].pts[nextIdx];
    cp->cyrel = n[1] * (nnpt->pos[0] - pt->pos[0]) - n[0] * (nnpt->pos[2] - pt->pos[2]);
}

/* 0x800262c0, 0x990 bytes */
void FindMaxpathIntervals(void)
{
    signed short slot;
    float min;
    float dx, dy, dz, dist;
    float feet;
    int p, i, a, path, idx, nextIdx;
    signed short end;
    float dirNext[3];
    signed short node;
    tModelData *m;
    tCarData *gc;
    for (slot = 0; slot < gNumActiveCars; slot++) {
        node = model[slot].slot;
        m = &model[node];
        gc = &gGameCar[node];
        if (!AreWeSolid(node) || gc->place_locked == 1)
            continue;
        {
        signed short pos[3];
        pos[0] = m->rwr[0];
        pos[1] = m->rwr[1];
        pos[2] = m->rwr[2];
        min = 1.0e20f;
        idx = 0;
        path = -1;
        i = gc->resIdx;
        for (p = 0; p < 5; p++) {
            dx = gResPath.pts[i][0] - pos[0];
            dy = gResPath.pts[i][1] - pos[1];
            dz = gResPath.pts[i][2] - pos[2];
            dist = dx * dx + dy * dy + dz * dz;
            if (dist < min) {
                min = dist;
                idx = i;
            }
            NextResPathPt(-1, i, 0, &i);
        }
        if (min > 10000.0f) {
            if (gc->lastResPath >= 0) {
                end = gResPath.alt[gc->lastResPath].numPts - gc->lastResIdx > 5 ? gc->lastResIdx + 5 : gResPath.alt[gc->lastResPath].numPts;
                for (p = gc->lastResIdx; p < end; p++) {
                    dx = gResPath.alt[gc->lastResPath].pts[p][0] - pos[0];
                    dy = gResPath.alt[gc->lastResPath].pts[p][1] - pos[1];
                    dz = gResPath.alt[gc->lastResPath].pts[p][2] - pos[2];
                    dist = dx * dx + dy * dy + dz * dz;
                    if (dist < min) {
                        min = dist;
                        idx = p;
                        path = gc->lastResPath;
                    }
                }
            } else if (gc->resIdx != gc->lastResIdx) {
                i = gc->lastResIdx;
                for (p = 0; p < 5; p++) {
                    dx = gResPath.pts[i][0] - pos[0];
                    dy = gResPath.pts[i][1] - pos[1];
                    dz = gResPath.pts[i][2] - pos[2];
                    dist = dx * dx + dy * dy + dz * dz;
                    if (dist < min) {
                        min = dist;
                        idx = i;
                        path = -1;
                    }
                    NextResPathPt(-1, i, 0, &i);
                }
            }
        }
        if (min > 40000.0f) {
            end = m->next_cp_index < m->last_cp_index ? gResPath.numPts : gCheckpoints.points[m->next_cp_index].pathPts[0];
            for (p = gCheckpoints.points[m->last_cp_index].pathPts[0]; p < end; p++) {
                dx = gResPath.pts[p][0] - pos[0];
                dy = gResPath.pts[p][1] - pos[1];
                dz = gResPath.pts[p][2] - pos[2];
                dist = dx * dx + dy * dy + dz * dz;
                if (dist < min) {
                    min = dist;
                    idx = p;
                    path = -1;
                    if (dist <= 10000.0f)
                        break;
                }
            }
            if (p == end) {
                for (a = 0; a < gResPath.numAlt; a++) {
                    if (gResPath.alt[a].resurrect == 2)
                        continue;
                    for (p = 0; p < gResPath.alt[a].numPts; p++) {
                        dx = gResPath.alt[a].pts[p][0] - pos[0];
                        dy = gResPath.alt[a].pts[p][1] - pos[1];
                        dz = gResPath.alt[a].pts[p][2] - pos[2];
                        dist = dx * dx + dy * dy + dz * dz;
                        if (dist < min) {
                            min = dist;
                            idx = p;
                            path = a;
                            if (dist <= 2500.0f)
                                break;
                        }
                    }
                    if (p < gResPath.alt[a].numPts)
                        break;
                }
                if (a == gResPath.numAlt) {
                    p = gc->resIdx;
                    do {
                        dx = gResPath.pts[p][0] - pos[0];
                        dy = gResPath.pts[p][1] - pos[1];
                        dz = gResPath.pts[p][2] - pos[2];
                        dist = dx * dx + dy * dy + dz * dz;
                        if (dist < min) {
                            min = dist;
                            idx = p;
                            path = -1;
                            if (dist <= 10000.0f)
                                break;
                        }
                        if (++p >= gResPath.numPts)
                            p = 0;
                    } while (p != gc->resIdx);
                }
            }
        }
        }
        gc->lastResPath = path;
        gc->lastResIdx = idx;
        if (min > 40000.0f)
            continue;
        if (path >= 0)
            FindResPt(path, idx, &path, &idx, 0);
        if (idx < gCheckpoints.points[m->last_cp_index].pathPts[0] ||
            idx >= (m->next_cp_index < m->last_cp_index ? gResPath.numPts : gCheckpoints.points[m->next_cp_index].pathPts[0]))
            continue;
        gc->resIdx = idx;
        if (gc->lastResPath >= 0) {
            feet = 0.0f;
        } else {
            NextResPathPt(-1, idx, 0, &nextIdx);
            for (p = 0; p < 3; p++)
                dirNext[p] = gResPath.pts[nextIdx][p] - gResPath.pts[idx][p];
            dirNext[1] = 0.0f;
            TB_Vector3Normalize(dirNext);
            dx = m->rwr[0] - gResPath.pts[idx][0];
            dz = m->rwr[2] - gResPath.pts[idx][2];
            feet = dx * dirNext[0] + dz * dirNext[2];
        }
        gc->distance = 0.0f;
        if (m->laps > 0) {
            gc->distance += gFirstLapDistance + (m->laps - 1) * gLapDistance;
            if (m->last_cp_index < gCheckpoints.finishPt)
                end = gCheckpoints.numPoints;
            else
                end = m->last_cp_index;
            for (i = gCheckpoints.finishPt; i < end; i++)
                gc->distance += gCheckpoints.points[i].distToNext;
            if (end != m->last_cp_index)
                for (i = gCheckpoints.loopPt; i < m->last_cp_index; i++)
                    gc->distance += gCheckpoints.points[i].distToNext;
        } else {
            for (i = 0; i < m->last_cp_index; i++)
                gc->distance += gCheckpoints.points[i].distToNext;
        }
        gc->distance += GetPathDistance(gCheckpoints.points[m->last_cp_index].pathPts[0], idx);
        gc->distance += feet;
    }
}

/* 0x80026c50, 0x144 bytes */
static void TargetSpeed(tPathPoint *cp)
{
    signed short nextIdx = NextPathPt(cp->idx, cp->path);
    float interp = cp->xrel / cp->len;
    float fps;
    float offsc;
    interp = interp < 0.0f ? 0.0f : interp > 1.0f ? 1.0f : interp;
    fps = 1.4666667f * gDronePaths[cp->path].pts[cp->idx].mph;
    cp->tgtspd = fps + interp * (1.4666667f * gDronePaths[cp->path].pts[nextIdx].mph - fps);
    offsc = cp->yrel > 0.0f ? cp->yrel : -cp->yrel;
    if (offsc < 0.1f)
        offsc = 1.0f;
    else if (offsc > 80.0f)
        offsc = 0.2f;
    else
        offsc = 1.0f - 0.00003125f * (offsc * offsc);
    cp->tgtspd *= offsc;
    if (cp->tgtspd < 28.0f)
        cp->tgtspd = 28.0f;
}

/* 0x80026d94, 0x350 bytes */
static void TargetSteerPos(tModelData *m, tPathPoint *cp)
{
    anon_pos_mph_8 *pt = &gDronePaths[cp->path].pts[cp->idx];
    anon_pos_mph_8 *npt;
    signed short nextIdx = NextPathPt(cp->idx, cp->path);
    float interp;
    float x, y, z;
    float dist = cp->len;
    float tgtdist = 80.0f + cp->xrel;
    tgtdist -= dist;
    npt = &gDronePaths[cp->path].pts[nextIdx];
    while (tgtdist > 0.0f) {
        pt = npt;
        nextIdx = NextPathPt(nextIdx, cp->path);
        npt = &gDronePaths[cp->path].pts[nextIdx];
        x = npt->pos[0] - pt->pos[0];
        y = npt->pos[1] - pt->pos[1];
        z = npt->pos[2] - pt->pos[2];
        dist = sqrtf(x * x + y * y + z * z);
        tgtdist -= dist;
    }
    if (dist < 0.00001f) {
        TB_DrawTextC(80, 160, 5, "Corrupt Maxpath Data");
        interp = 1.0f;
    } else {
        interp = (dist + tgtdist) / dist;
    }
    interp = interp < 0.0f ? 0.0f : interp > 1.0f ? 1.0f : interp;
    cp->tgtpos[0] = pt->pos[0] + interp * (npt->pos[0] - pt->pos[0]);
    cp->tgtpos[1] = pt->pos[1] + interp * (npt->pos[1] - pt->pos[1]);
    cp->tgtpos[2] = pt->pos[2] + interp * (npt->pos[2] - pt->pos[2]);
}

/* 0x800270e4, 0x240 bytes */
static void AdjustSpeed(tModelData *m, float tspd)
{
    int node = m->slot;
    float x, y, z, delta_speed;
    tspd *= 1.05f;
    if (gGravityScale <= 1 && (m->tires[0].slipflag == 40 || m->tires[1].slipflag == 40)) {
        lastbrake[node] = 0.0f;
        lastthrottle[node] = 0.0f;
    } else {
        x = m->rwv[0];
        y = m->rwv[1];
        z = m->rwv[2];
        delta_speed = tspd - sqrtf(x * x + y * y + z * z);
        if (delta_speed < 0.0f)
            delta_speed *= 0.5f;
        lastthrottle[node] += 0.183f * delta_speed;
        if (lastthrottle[node] < -0.75f)
            lastbrake[node] -= (0.75f + lastthrottle[node]) / 4.0f;
        else
            lastbrake[node] = 0.0f;
    }
    if (lastthrottle[node] < 0.0f)
        lastthrottle[node] = 0.0f;
    else if (lastthrottle[node] > 1.0f)
        lastthrottle[node] = 1.0f;
    if (lastbrake[node] < 0.0f)
        lastbrake[node] = 0.0f;
    else if (lastbrake[node] > 1.0f)
        lastbrake[node] = 1.0f;
    m->mainin.throttle = lastthrottle[node];
    m->mainin.brake = lastbrake[node];
}

/* 0x80027324, 0xa0 bytes */
static void AdjustSteer(tModelData *m, float *pos)
{
    int node = m->slot;
    float angle;
    pos[0] *= 108.43373f * m->dt;
    angle = fatan2(pos[2], pos[0]);
    lastwheel[node] = 1.0f - 0.63661975f * (angle < 0.0f ? 0.0f : angle > 3.1415927f ? 3.1415927f : angle);
    m->mainin.wheel = lastwheel[node];
}

/* 0x80027a94, 0x290 bytes */
/* AvoidAreas: private function-level match, 0x800273c4 /1,744 bytes.
 * copyAvoidVector reconstructs an inline vector-copy primitive; no separate
 * original helper symbol or whole-unit matching claim is made. */
static inline void copyAvoidVector(const float *src,float *dst)
{
    dst[0]=src[0]; dst[1]=src[1]; dst[2]=src[2];
}

static void AvoidAreas(signed short drone_index)
{
    signed char hints;
    signed short i;
    signed short index;
    signed short can_we_cheat;
    float dist;
    float scale;
    float scale1;
    float scale2;
    float rpos[3];
    float pos[3];
    tPathPoint *cp;
    tModelData *m;
    float dir[3], save_dir[3];
    float mpos0, mpos1, mpos2;
    m = &model[drone_index];
    cp = &gGameCar[drone_index].mpctl;
    {
        float c0, m1, c1, c2;
        c0 = cp->tgtpos[0];
        c1 = cp->tgtpos[1]; m1 = m->reckon.rwr[1];
        c2 = cp->tgtpos[2];
        pos[0] = c0 - m->reckon.rwr[0];
        pos[1] = c1 - m1;
        pos[2] = c2 - m->reckon.rwr[2];
    }
    BodyVector(pos, rpos, m->reckon.uvs);
    if (m->idt == 0.0f || m->magvel < 0.1f) {
        float *ld = last_dir[drone_index];
        cp->tgtspd = 100.0f;
        ld[0] = 0.0f;
        cp->tgtpos[0] = 0.0f;
        cp->tgtpos[1] = rpos[1];
        ld[1] = 0.0f;
        model[drone_index].can_we_cheat = 0;
        rpos[0] = 0.0f;
        cp->tgtpos[2] = rpos[2];
        ld[2] = 0.0f;
        return;
    }
    if (m->magvel < 0.5f * (1.4666667f * gDronePaths[cp->path].pts[cp->idx].mph)) {
        rpos[0] *= 0.2f;
        cp->tgtspd = 1.5f * (1.4666667f * gDronePaths[cp->path].pts[cp->idx].mph);
        for (i = 0; i < 3; i++) {
            last_dir[drone_index][i] = 0.0f;
            cp->tgtpos[i] = rpos[i];
        }
        model[drone_index].can_we_cheat = 0;
        return;
    }
    if (rpos[2] < fabs(2.0f * rpos[0])) {
        for (i = 0; i < 3; i++) {
            cp->tgtpos[i] = rpos[i];
            last_dir[drone_index][i] = 0.0f;
        }
        model[drone_index].can_we_cheat = 0;
        return;
    }
    scale = m->magvel * m->dt / magnitude(rpos);
    
    dir[0] = rpos[0] * scale;
    dir[1] = rpos[1] * scale;
    dir[2] = rpos[2] * scale;
    save_dir[0] = dir[0];
    save_dir[1] = dir[1];
    save_dir[2] = dir[2];
    mpos0 = model[drone_index].reckon.rwr[0];
    mpos1 = model[drone_index].reckon.rwr[1];
    mpos2 = model[drone_index].reckon.rwr[2];
    scale1 = 1.0f;
    can_we_cheat = 1;
    for (i = 0; i < gNumActiveCars; i++) {
        index = model[i].slot;
        if (index == drone_index)
            continue;
        m = &model[index];
        pos[0] = m->reckon.rwr[0] - mpos0;
        pos[1] = m->reckon.rwr[1] - mpos1;
        pos[2] = m->reckon.rwr[2] - mpos2;
        BodyVector(pos, rpos, model[drone_index].reckon.uvs);
        dist = rpos[0] * rpos[0] + rpos[2] * rpos[2];
        if (dist > 90000.0f)
            continue;
        if (m->drone_type == 2 && fabs(rpos[0]) < 60.0 && fabs(rpos[2]) < 200.0)
            can_we_cheat = 0;
        if (rpos[2] < 0.0f || fabs(rpos[0]) > 8.0)
            continue;
        scale2 = 1.0f;
        if (rpos[2] < 20.0f)
            scale2 = 0.9f;
        else if (rpos[2] < 100.0f)
            scale2 -= 0.0012500003213062882f * (100.0f - rpos[2]);
        if (scale2 < scale1)
            scale1 = scale2;
    }
    TargetPathAvoid(&model[drone_index], &scale1);
    if (4.0 * fabs(dir[0]) < fabs(dir[2])) {
        if (dir[0] < 0.0f)
            dir[0] += 8.0f * (1.0f - scale1);
        else
            dir[0] -= 8.0f * (1.0f - scale1);
    } else if (scale1 < 0.9f) {
        if (dir[0] < 0.0f)
            dir[0] = 4.0f * (1.0f - scale1);
        else
            dir[0] = 4.0f * -(1.0f - scale1);
    }
    hints = gDronePaths[cp->path].pts[cp->idx].hints;
    if (last_dir[drone_index][0] == 0.0f && last_dir[drone_index][2] == 0.0f) {
        dir[0] = 0.0f;
        cp->tgtspd *= 1.0f - 0.3f * (1.0f - scale1);
    } else if (hints == 5 || m->resurrect.moving_state > -1) {
        dir[0] *= 0.4f;
    } else if (hints == 2 || (last_dir[drone_index][0] == 0.0f && last_dir[drone_index][2] == 0.0f)) {
        dir[0] = save_dir[0];
        cp->tgtspd *= 1.0f - 0.3f * (1.0f - scale1);
    } else if (hints == 1) {
        if (last_dir[drone_index][0] == 0.0f && last_dir[drone_index][2] == 0.0f)
            dir[0] = save_dir[0];
        else
            dir[0] = 0.15f * save_dir[0] + 0.85f * last_dir[drone_index][0];
        cp->tgtspd *= 1.0f - 0.1f * (1.0f - scale1);
    } else if (hints == 3) {
        dir[0] = save_dir[0] - last_save_dir[drone_index][0];
        save_dir[0] = last_save_dir[drone_index][0];
        save_dir[1] = last_save_dir[drone_index][1];
        save_dir[2] = last_save_dir[drone_index][2];
    } else if (hints == 4) {
        dir[0] = save_dir[0] - last_save_dir[drone_index][0];
    } else {
        dir[0] = 0.5f * dir[0] + 0.5f * last_dir[drone_index][0];
    }
    
    model[drone_index].can_we_cheat = can_we_cheat;
    copyAvoidVector(dir,cp->tgtpos);
    copyAvoidVector(dir,last_dir[drone_index]);
    copyAvoidVector(save_dir,last_save_dir[drone_index]);
}

/* 0x80028580, 0x314 bytes */

void SetDroneResPathIndex(tModelData *m, signed short path, signed short idx)
{
    tPathPoint *cp;
    float min = 1.0e20f;
    float x, y, z, dx, dy, dz, dist;
    int p;
    int end;
    if (path >= 0) {
        x = gResPath.alt[path].pts[idx][0];
        y = gResPath.alt[path].pts[idx][1];
        z = gResPath.alt[path].pts[idx][2];
    } else {
        x = gResPath.pts[idx][0];
        y = gResPath.pts[idx][1];
        z = gResPath.pts[idx][2];
    }
    cp = &gGameCar[m->slot].mpctl;
    end = m->next_cp_index < m->last_cp_index ? gDronePaths[cp->path].numPts : gCheckpoints.points[m->next_cp_index].pathPts[1 + cp->path];
    idx = 0;
    for (p = gCheckpoints.points[m->last_cp_index].pathPts[1 + cp->path]; p < end; p++) {
        dx = gDronePaths[cp->path].pts[p].pos[0] - x;
        dy = gDronePaths[cp->path].pts[p].pos[1] - y;
        dz = gDronePaths[cp->path].pts[p].pos[2] - z;
        dist = dx * dx + dy * dy + dz * dz;
        if (dist < min) {
            min = dist;
            idx = p;
        }
    }
    for (p = 0; p < m->slot; p++)
        idx = NextPathPt(idx, cp->path);
    SetNewPathIndex(m, idx);
}

/* 0x80027d24, 0x40 bytes */
void SetNewPathIndex(tModelData *m, signed short idx)
{
    tPathPoint *cp = &gGameCar[m->slot].mpctl;
    cp->idx = idx;
    cp->interval_time = gModelTime;
    cp->newIdx = -1;
    cp->xrel = 20.0f;
    cp->yrel = 0.0f;
}

/* 0x80027d64, 0xdc bytes */
signed short FindClosestPathPt(signed short *pos, signed short path)
{
    float min = 1.0e20f;
    float dx, dy, dz, dist;
    int p;
    signed short idx;
    for (p = 0; p < gDronePaths[path].numPts; p++) {
        dx = gDronePaths[path].pts[p].pos[0] - pos[0];
        dy = gDronePaths[path].pts[p].pos[1] - pos[1];
        dz = gDronePaths[path].pts[p].pos[2] - pos[2];
        dist = dx * dx + dy * dy + dz * dz;
        if (dist < min) {
            min = dist;
            idx = p;
        }
    }
    return idx;
}

/* 0x80027e40, 0x210 bytes */
static signed short FindClosestDronePathPtEfficiently(signed short *pos, signed short lastPath, signed short lastIdx, signed short lastCP, signed short path)
{
    float min;
    float dx, dy, dz, dist;
    int p;
    int i;
    signed short idx;
    signed short offset;
    signed short delta;
    offset = gCheckpoints.points[lastCP].pathPts[1 + path] - gCheckpoints.points[lastCP].pathPts[1 + lastPath];
    if (lastCP == gCheckpoints.numPoints - 1)
        delta = gDronePaths[path].numPts - gDronePaths[lastPath].numPts - offset;
    else
        delta = gCheckpoints.points[lastCP + 1].pathPts[1 + path] - gCheckpoints.points[lastCP + 1].pathPts[1 + lastPath] - offset;
    delta = delta < 0 ? -delta : delta;
    min = 1.0e20f;
    idx = 0;
    delta = delta < 5 ? 5 : delta;
    p = lastIdx + offset - delta;
    while (p < 0)
        p += gDronePaths[path].numPts;
    while (p >= gDronePaths[path].numPts)
        p -= gDronePaths[path].numPts;
    for (i = 0; i < (signed short)(2 * delta + 1); i++) {
        dx = gDronePaths[path].pts[p].pos[0] - pos[0];
        dy = gDronePaths[path].pts[p].pos[1] - pos[1];
        dz = gDronePaths[path].pts[p].pos[2] - pos[2];
        dist = dx * dx + dy * dy + dz * dz;
        if (dist < min) {
            min = dist;
            idx = p;
        }
        if (p == gDronePaths[path].numPts - 1)
            p = gCheckpoints.points[gCheckpoints.loopPt].pathPts[1 + path];
        else
            p++;
    }
    return idx;
}

/* 0x80028050, 0xb0 bytes */
static int TraceToMain(int inPath, int inIdx, int *outIdx, int *numPts, int depth)
{
    if (!TraceCheck(inPath, depth))
        return 0;
    if (depth == 0)
        *numPts = 0;
    if (inPath < 0) {
        *outIdx = inIdx;
        return 1;
    }
    *numPts += inIdx;
    return TraceToMain(gResPath.alt[inPath].firstPath, gResPath.alt[inPath].firstIdx,
                       outIdx, numPts, depth + 1);
}

/* 0x80028100, 0xf8 bytes */
static int TraceToEnd(int inPath, int inIdx, int *outPath, int *outIdx, int *numPts, int honorAltRes, int depth)
{
    if (!TraceCheck(inPath, depth))
        return 0;
    if (depth == 0)
        *numPts = 0;
    if (inPath < 0 || (honorAltRes && gResPath.alt[inPath].resurrect)) {
        *outPath = inPath;
        *outIdx = inIdx;
        return 1;
    }
    *numPts += gResPath.alt[inPath].numPts - inIdx;
    return TraceToEnd(gResPath.alt[inPath].lastPath, gResPath.alt[inPath].lastIdx,
                      outPath, outIdx, numPts, honorAltRes, depth + 1);
}

/* 0x800281f8, 0xe8 bytes */
static void TraceToIndex(int inPath, int inIdx, int toIdx, int *outPath, int *outIdx)
{
    if (inPath < 0) {
        *outPath = inPath;
        *outIdx = inIdx - toIdx;
        if (inIdx < gCheckpoints.points[gCheckpoints.loopPt].pathPts[0]) {
            if (*outIdx < 0)
                *outIdx = 0;
        } else {
            while (*outIdx < gCheckpoints.points[gCheckpoints.loopPt].pathPts[0])
                *outIdx += gResPath.numPts - gCheckpoints.points[gCheckpoints.loopPt].pathPts[0];
        }
    } else if (toIdx <= inIdx) {
        *outPath = inPath;
        *outIdx = inIdx - toIdx;
    } else {
        TraceToIndex(gResPath.alt[inPath].firstPath, gResPath.alt[inPath].firstIdx,
                     toIdx - inIdx, outPath, outIdx);
    }
}

/* 0x800282e0, 0x120 bytes */
void TracePathPtForward(int res, int inPath, int inIdx, int numPts, int *outPath, int *outIdx)
{
    if (!res || inPath < 0) {
        *outPath = inPath;
        *outIdx = inIdx + numPts;
        if (res) {
            while (*outIdx >= gResPath.numPts)
                *outIdx = gCheckpoints.points[gCheckpoints.loopPt].pathPts[0] + (*outIdx - gResPath.numPts);
        } else {
            while (*outIdx >= gDronePaths[inPath].numPts)
                *outIdx = gCheckpoints.points[gCheckpoints.loopPt].pathPts[1 + inPath] + (*outIdx - gDronePaths[inPath].numPts);
        }
    } else if (inIdx + numPts < gResPath.alt[inPath].numPts) {
        *outPath = inPath;
        *outIdx = inIdx + numPts;
    } else {
        TracePathPtForward(res, gResPath.alt[inPath].lastPath, gResPath.alt[inPath].lastIdx,
                           inIdx + (numPts - gResPath.alt[inPath].numPts), outPath, outIdx);
    }
}

/* 0x80028400, 0x180 bytes */
int NumPtsToCheckpoint(int res, int inPath, int inIdx, int cp)
{
    if (!res) {
        if (gCheckpoints.points[cp].pathPts[1 + inPath] < inIdx)
            return gDronePaths[inPath].numPts - inIdx + gCheckpoints.points[cp].pathPts[1 + inPath]
                - (cp >= gCheckpoints.loopPt ? gCheckpoints.points[gCheckpoints.loopPt].pathPts[1 + inPath] : 0);
        return gCheckpoints.points[cp].pathPts[1 + inPath] - inIdx;
    }
    if (inPath < 0) {
        if (gCheckpoints.points[cp].pathPts[0] < inIdx)
            return gResPath.numPts - inIdx + gCheckpoints.points[cp].pathPts[0]
                - (cp >= gCheckpoints.loopPt ? gCheckpoints.points[gCheckpoints.loopPt].pathPts[0] : 0);
        return gCheckpoints.points[cp].pathPts[0] - inIdx;
    }
    if (gCheckpoints.points[cp].pathPts[5 + inPath] >= 0 &&
        inIdx <= gCheckpoints.points[cp].pathPts[5 + inPath])
        return gCheckpoints.points[cp].pathPts[5 + inPath] - inIdx;
    return NumPtsToCheckpoint(res, gResPath.alt[inPath].lastPath,
                             gResPath.alt[inPath].lastIdx, cp)
        + (gResPath.alt[inPath].numPts - inIdx);
}

/* 0x80028894, 0x58 bytes */
static int FindResPtRecurse(int inPath, int inIdx, int *outPath, int *outIdx, int honorAltRes, int depth)
{
    int numPts;
    int i;
    int numPtsFromMain;
    int numPtsOnShortcut;
    int numPtsOnRes;
    int startIndex;
    int endPath;
    int endIndex;
    int endMainIndex;
    if (depth == 0) {
        *outPath = inPath;
        *outIdx = inIdx;
    }
    gPathSearch[depth] = inPath;
    for (i = 0; i < depth; i++)
        if (gPathSearch[i] == inPath)
            return 0;
    if (gResPath.alt[inPath].firstPath == gResPath.alt[inPath].lastPath) {
        if (gResPath.alt[inPath].lastIdx > gResPath.alt[inPath].firstIdx) {
            numPts = gResPath.alt[inPath].lastIdx - gResPath.alt[inPath].firstIdx + 1;
            *outIdx = gResPath.alt[inPath].firstIdx + inIdx * numPts / gResPath.alt[inPath].numPts;
        } else {
            if (gResPath.alt[inPath].firstPath != -1)
                return 0;
            i = gResPath.numPts - gResPath.alt[inPath].firstIdx;
            numPts = gResPath.alt[inPath].lastIdx - gCheckpoints.points[gCheckpoints.loopPt].pathPts[0] + 1;
            numPts = i + numPts;
            *outIdx = gResPath.alt[inPath].firstIdx + inIdx * numPts / gResPath.alt[inPath].numPts;
            if (*outIdx >= gResPath.numPts)
                *outIdx = gCheckpoints.points[gCheckpoints.loopPt].pathPts[0] + (*outIdx - gResPath.numPts);
        }
        *outPath = gResPath.alt[inPath].firstPath;
        if (*outPath < 0 || (honorAltRes && gResPath.alt[*outPath].resurrect))
            return 1;
        return FindResPtRecurse(*outPath, *outIdx, outPath, outIdx, honorAltRes, depth + 1);
    }
    if (!TraceToMain(inPath, inIdx, &startIndex, &numPtsFromMain, 0))
        return 0;
    if (!TraceToEnd(inPath, inIdx, &endPath, &endIndex, &numPtsOnShortcut, honorAltRes, 0))
        return 0;
    if (!TraceToMain(endPath, endIndex, &endMainIndex, &numPtsOnRes, 0))
        return 0;
    if (startIndex > endMainIndex) {
        numPts = gResPath.numPts - startIndex + endMainIndex - gCheckpoints.points[gCheckpoints.loopPt].pathPts[0];
        if (startIndex - endMainIndex < numPts) {
            numPtsFromMain += startIndex - endMainIndex;
            startIndex = endMainIndex;
        } else {
            numPtsOnRes += numPts;
        }
    } else {
        numPts = gResPath.numPts - endMainIndex + startIndex;
        if (endMainIndex - startIndex > numPts) {
            numPtsFromMain += numPts;
            startIndex = endMainIndex;
        } else {
            numPtsOnRes += endMainIndex - startIndex;
        }
    }
    numPtsOnShortcut += numPtsFromMain;
    TraceToIndex(endPath, endIndex, numPtsOnRes - numPtsFromMain * numPtsOnRes / numPtsOnShortcut, outPath, outIdx);
    return 1;
}

int FindResPt(int inPath, int inIdx, int *outPath, int *outIdx, int honorAltRes)
{
    if (honorAltRes && gResPath.alt[inPath].resurrect) {
        *outPath = inPath;
        *outIdx = inIdx;
        return 1;
    }
    return FindResPtRecurse(inPath, inIdx, outPath, outIdx, honorAltRes, 0);
}

