/* camera.c - complete camera reset, tracking and object-update recovery.
   Status: Matching (all twenty-four functions).
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\camera.c; assembly: build/2049LE/asm/GameSource/src/camera.s */
#include "rush/rush_protos.h"
#define MSL_NO_SQRTF
#include "msl_math_inline.h"

/* camera.c/tires.c sqrtf variant: automatic 0.5/3.0 constants, so the unit gets anonymous .sdata2 doubles instead of the header's _half/_three statics */
inline float sqrtf(float x)
{
    const double _half = .5;
    const double _three = 3.0;

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

extern double cos(double);
extern double sin(double);

static void _UpdateDroneObjects(void);
static void _UpdateRearCamera(tCarData *car, float behind, float side, float *camoff);

/* explicit `= { 0 }`: the target keeps these objects in initialised data (.data/.sdata), not .bss */
float finish_cam_stime[6] = { 0 };
static float lastSwitch[4] = { 0 };
static float nextSwitch[4] = { 0 };
signed char finish_cam[6] = { 0 };
signed short gNumWindows;
signed char gUpsideDownTrack;
static signed char view3_switch[4];
static signed char gTireCnt[4];
static float gCameraMatrix[4][4];
static float elastic_factor[4];
static float elasticity[4];
static float last_yvec[4][3];
static float last_zvec[4][3];
static float yf[4];
static float zf[4];
static float gSusAvg[4];
static float cur_acc[4][3];
static float rear_z[4];
static float rear_y[4];
anon_mat_n64_98 gCamera[4];

/* 0x80005f30, size 0x20 */
void InitCameras(void)
{
    ZeroCameras();
}

/* 0x80005f50, size 0x70 */
void ZeroCameras(void)
{
    int i;
    for (i = 0; i < 4; i++)
        ZeroCamera(i == 0 ? 1 : i == 1 ? 2 : i == 2 ? 4 : 8);
    UpdateCameras();
}

/* 0x80005fc0, size 0x98 */
void ZeroCamera(signed short camera)
{
    int i = camera & 1 ? 0 : camera & 2 ? 1 : camera & 4 ? 2 : 3;
    gCamera[i].slot = i;
    gCamera[i].mat.mat3.pos[0] = 0.0f;
    gCamera[i].mat.mat3.pos[1] = 0.0f;
    gCamera[i].mat.mat3.pos[2] = 0.0f;
    CopyUV(unitmatrix, gCamera[i].mat.mat3.uvs);
    gCamera[i].view2spring = 0.0f;
}

/* 0x80006058, size 0xe0 */
void init_views(void)
{
    tCarData *car;
    int i;
    for (i = 0; i < gNumWindows; i++) {
        car = &gGameCar[gCamera[i].slot];
        car->camera = i;
        init_view3(car);
        if (gGameState & 8) {
            car->view = 6;
            init_camera_on_track(car);
            lastSwitch[i] = gModelTime;
            nextSwitch[i] = 3.0f;
        } else {
            car->view = 7;
            init_steady_move_cam(car);
        }
    }
    for (i = 0; i < 6; i++) finish_cam[i] = 0;
}

/* 0x80006138, size 0x7c */
void init_view3(tCarData *car)
{
    int camera = car->camera;
    car->saveview = 2;
    car->view = 2;
    if (gGameState & 8) {
        rear_y[camera] = 7.0f;
        rear_z[camera] = 20.0f;
    } else {
        rear_y[camera] = 3.0f;
        rear_z[camera] = 16.0f;
    }
    view3_switch[camera] = 1;
}

/* 0x800061b4, size 0xd0 */
void UpdateCameras(void)
{
    int i;
    for (i = 0; i < gNumWindows; i++) {
        gCamera[i].n64.mat.mat3.pos[0] = gCamera[i].mat.mat3.pos[0];
        gCamera[i].n64.mat.mat3.pos[1] = gCamera[i].mat.mat3.pos[1];
        gCamera[i].n64.mat.mat3.pos[2] = gCamera[i].mat.mat3.pos[2];
        CopyUV(gCamera[i].mat.mat3.uvs, gCamera[i].n64.mat.mat3.uvs);
        gCamera[i].n64.mat.mat3.uvs[0][1] = -gCamera[i].n64.mat.mat3.uvs[0][1];
        gCamera[i].n64.mat.mat3.uvs[0][2] = -gCamera[i].n64.mat.mat3.uvs[0][2];
        gCamera[i].n64.mat.mat3.uvs[1][0] = -gCamera[i].n64.mat.mat3.uvs[1][0];
        gCamera[i].n64.mat.mat3.uvs[2][0] = -gCamera[i].n64.mat.mat3.uvs[2][0];
        if (!gPauseState) SkyUpdate(i);
    }
    UpdateHUDPostCamera();
    ShowCountdown(gCountDown > 0);
}

/* 0x80006284, size 0x28 */
void SetCameraView(signed short camera)
{
    cur_acc[camera][0] = 0.0f;
    cur_acc[camera][1] = 0.0f;
    cur_acc[camera][2] = 0.0f;
}

/* 0x800062ac, size 0x4b4 */
void CheckCameraView(void)
{
    anon_slot_controller_50 *player;
    tCarData *car;
    tModelData *m;
    int view;
    int camera;
    int i;
    int newview;
    int win;
    int slot;
    if (gRace == 2 && (gContLevels[1] & 2) && (gContEdges[1] & 4)) {
        slot = gCamera[0].slot;
        gGameCar[slot].camera = -1;
        do {
            if (++slot >= (*(volatile signed short *)&gNumCars)) slot = 0; /* volatile read: the target reloads gNumCars on every pass (header keeps it plain) */
        } while (!model[slot].in_game && gGameCar[slot].camera < 0);
        gGameCar[slot].camera = 0;
        gCamera[0].slot = slot;
    }
    player = gPlayers;
    for (win = 0; win < gNumWindows; win++, player++) {
        car = &gGameCar[gCamera[win].slot];
        m = &model[gCamera[win].slot];
        view = car->view;
        camera = car->camera;
        if (gGameState & 8) {
            if ((gModelTime - lastSwitch[camera] > nextSwitch[camera] || gModelTime < lastSwitch[camera]) && view != 4 && GetCountdownTimeRemaining() > 3.0f) {
                lastSwitch[camera] = gModelTime;
                nextSwitch[camera] = 2.5 + Random(1.0f);
                newview = Random(11.0f);
                switch (newview) {
                case 0: view = 0; break;
                case 1:
                    view = 1;
                    cur_acc[camera][0] = 0.0f;
                    cur_acc[camera][1] = 0.0f;
                    cur_acc[camera][2] = 0.0f;
                    break;
                case 2: if (view != 3) view = 2; break;
                case 3: if (view != 2) view = 3; break;
                case 4: init_view3(car); view = 10; break;
                default:
                    view = 6;
                    init_camera_on_track(car);
                    nextSwitch[camera] = 3.0f;
                    break;
                }
            }
        } else {
            if (finish_cam[player->slot] && view != 4) view = 5;
            else if (view != 4 && (player->edges & player->controls[8])) {
                view++;
                if (view >= 4) view = 0;
                if ((*player->save)->datah) SavePlayerCarView(player->save, view);
                if (view == 1) {
                    for (i = 0; i < 3; i++) cur_acc[car->camera][i] = 0.0f;
                } else if (view == 2) {
                    init_view3(car);
                    gCamera[car->camera].fcam.mat.mat3.pos[0] = car->dr_pos[0];
                    gCamera[car->camera].fcam.mat.mat3.pos[1] = 1.0f + car->dr_pos[1];
                    gCamera[car->camera].fcam.mat.mat3.pos[2] = car->dr_pos[2];
                } else if (view == 3) init_view3(car);
            }
        }
        if (view != car->view) car->view = car->saveview = view;
        update_camera(car);
        gTireCnt[camera] = 0;
        gSusAvg[camera] = 0.0f;
        for (i = 0; i < 4; i++) {
            if (m->airdist[i] < 4.0f) {
                gSusAvg[camera] += m->suscomp[i];
                gTireCnt[camera]++;
            }
        }
        gSusAvg[camera] = gTireCnt[camera] >= 3 ? gSusAvg[camera] / gTireCnt[camera] : 0.0f;
    }
    BlinkResurrectingCars();
    _UpdateDroneObjects();
}


/* 0x80006760, size 0x30 */
float view1_suscomp_offset(tModelData *m)
{
    float moveup;
    moveup = (float)(0.5f * (m->reckon.suscomp[0] + m->reckon.suscomp[1])) - 0.3f;
    return moveup > 0.0f ? moveup : 0.0f;
}


/* vector helpers written as inline functions (no DWARF entries): like game_reckon_all's VecScale/VecAdd, their parameter temporaries give the original operand order and registers */
static inline void camera_copy(float *src, float *dst)
{
    dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2];
}
static inline void camera_scale(float *src, float scale, float *dst)
{
    dst[0] = src[0] * scale; dst[1] = src[1] * scale; dst[2] = src[2] * scale;
}
static inline void camera_add(float *a, float *b, float *dst)
{
    dst[0] = a[0] + b[0]; dst[1] = a[1] + b[1]; dst[2] = a[2] + b[2];
}
static inline void camera_scaled_add(float *src, float scale, float *dst)
{
    dst[0] += src[0] * scale; dst[1] += src[1] * scale; dst[2] += src[2] * scale;
}
static inline void camera_cross(float *a, float *b, float *dst)
{
    dst[0] = a[1]*b[2] - a[2]*b[1];
    dst[1] = a[2]*b[0] - a[0]*b[2];
    dst[2] = a[0]*b[1] - a[1]*b[0];
}


/* scale-first variants of the helpers above: the operand order of each multiply follows the parameter order */
static inline void camera_lscale(float scale, float *src, float *dst)
{
    dst[0] = scale * src[0]; dst[1] = scale * src[1]; dst[2] = scale * src[2];
}
static inline void camera_lscaled_add(float scale, float *src, float *dst)
{
    dst[0] += scale * src[0]; dst[1] += scale * src[1]; dst[2] += scale * src[2];
}

/* 0x80006790, size 0xa54 */
void View3Cam(tCarData *car, float *pos)
{
    int camera = car->camera;
    float rear_dist = rear_z[camera];
    float up_dist = rear_y[camera];
    float magvel;
    float rad;
    float len;
    float bvec[3];
    float fvec[3];
    float zvec[3];
    float yvec[3];
    float camoff[3];
    float elast;
    int i;
    update_car_object(car, pos, car->dr_car_uvs);
    if (car->view == 3) { rear_dist -= 6.0f; up_dist -= 2.0f; }
    magvel = magnitude(car->dr_vel);
    len = magvel > 100.0f ? 0.25f : 0.0025f * magvel;
    rad = sqrtf(rear_dist * rear_dist + up_dist * up_dist);
    rad *= 1.0f + len;
    rear_dist *= 1.0f + len;
    if (view3_switch[camera] || gRace == 6) elast = 0.0f;
    else elast = magvel < 140.0f ? 0.8f - 0.6f * (magvel / 140.0f) : 0.2f;
    view3_switch[camera] = 0;
    if (gTireCnt[camera] >= 3 && gSusAvg[camera] > 0.5f) zf[camera] += gFrameSeconds;
    else if (magvel < 30.0f) zf[camera] += 10.0f * gFrameSeconds;
    else zf[camera] -= gFrameSeconds;
    if (zf[camera] < 0.0f) zf[camera] = 0.0f;
    else if (zf[camera] > 1.0f) zf[camera] = 1.0f;
    if (gTireCnt[camera] < 3) yf[camera] -= gFrameSeconds;
    else if (gSusAvg[camera] > 1.5f) yf[camera] += 10.0f * gFrameSeconds;
    else if (gSusAvg[camera] > 0.5f) yf[camera] += gFrameSeconds / (0.1f + 0.9f * (1.5f - gSusAvg[camera]));
    else yf[camera] += gFrameSeconds;
    if (yf[camera] < 0.0f) yf[camera] = 0.0f;
    else if (yf[camera] > 1.0f) yf[camera] = 1.0f;
    for (i = 0; i < 3; i++) bvec[i] = -car->dr_uvs[2][i];
    if (zf[camera] < 1.0f) {
        if (magvel < 1.0f) { camera_copy(bvec, fvec); }
        else if (car->v[2] < 0.0f && gTireCnt[camera] >= 3) {
            len = 1.0f / magvel;
            camera_scale(car->dr_vel, len, fvec);
        } else {
            camera_scale(car->dr_vel, -1.0f / magvel, fvec);
        }
    }
    if (zf[camera] == 0.0f) { camera_copy(fvec, zvec); }
    else if (zf[camera] == 1.0f) { camera_copy(bvec, zvec); }
    else {
        Interpolate(fvec, bvec, 3, 0.0f, 1.0f, zf[camera], zvec);
        len = magnitude(zvec);
        if (len < 0.1f) { camera_copy(bvec, zvec); }
        else { len = 1.0f / len; camera_scale(zvec, len, zvec); }
    }
    if (elast > 0.0f) {
        camera_scale(zvec, 1.0f - elast, zvec);
        camera_lscaled_add(elast, last_zvec[camera], zvec);
        len = magnitude(zvec);
        if (len < 0.1f) { camera_copy(bvec, zvec); }
        else { len = 1.0f / len; camera_scale(zvec, len, zvec); }
    }
    camera_copy(zvec, last_zvec[camera]);
    camera_copy(car->dr_uvs[1], bvec);
    if (yf[camera] < 1.0f) { camera_copy(uvector, fvec); }
    if (yf[camera] == 0.0f) { camera_copy(fvec, yvec); }
    else if (yf[camera] == 1.0f) { camera_copy(bvec, yvec); }
    else {
        Interpolate(fvec, bvec, 3, 0.0f, 1.0f, yf[camera], yvec);
        len = magnitude(yvec);
        if (len < 0.1f) { camera_copy(bvec, yvec); }
        else { len = 1.0f / len; camera_scale(yvec, len, yvec); }
    }
    if (elast > 0.0f) {
        camera_scale(yvec, 1.0f - elast, yvec);
        camera_lscaled_add(elast, last_yvec[camera], yvec);
        len = magnitude(yvec);
        if (len < 0.1f) { camera_copy(bvec, yvec); }
        else { len = 1.0f / len; camera_scale(yvec, len, yvec); }
    }
    camera_copy(yvec, last_yvec[camera]);
    camera_scale(zvec, rear_dist, camoff);
    camera_scaled_add(yvec, up_dist, camoff);
    direction(camoff, gCamera[camera].fcam.mat.mat3.uvs[2]);
    camera_lscale(rad, gCamera[camera].fcam.mat.mat3.uvs[2], camoff);
    if (car->view == 2) { camera_lscaled_add(4.0f, yvec, camoff); }
    else if (car->view == 3) { camera_lscaled_add(4.0f, yvec, camoff); }
    camera_add(pos, camoff, gCamera[camera].fcam.mat.mat3.pos);
    camera_lscale(-1.0f, gCamera[camera].fcam.mat.mat3.uvs[2], gCamera[camera].fcam.mat.mat3.uvs[2]);
    camera_cross(yvec, gCamera[camera].fcam.mat.mat3.uvs[2], gCamera[camera].fcam.mat.mat3.uvs[0]);
    len = magnitude(gCamera[camera].fcam.mat.mat3.uvs[0]);
    if (len < 0.001f) { camera_copy(rvector, gCamera[camera].fcam.mat.mat3.uvs[0]); }
    else { len = 1.0f / len; camera_lscale(len, gCamera[camera].fcam.mat.mat3.uvs[0], gCamera[camera].fcam.mat.mat3.uvs[0]); }
    camera_cross(gCamera[camera].fcam.mat.mat3.uvs[2], gCamera[camera].fcam.mat.mat3.uvs[0], gCamera[camera].fcam.mat.mat3.uvs[1]);
}

/* 0x800071e4, size 0x698 */
void update_camera(tCarData *car)
{
    int i;
    int j;
    float pos_in[3];
    float pos_out[3];
    tModelData *m = &model[car->slot];
    int camera = car->camera;
    signed char rear = !(gGameState & 8) && (gPlayers[car->slot].levels & gPlayers[car->slot].controls[9]);
    int view;
    camera_copy(car->dr_pos, gCamera[camera].mat.mat3.pos);
    gCamera[camera].mat.mat3.pos[1] += 0.25f;
    switch (car->view) {
    case 0:
        update_car_object(car, gCamera[camera].mat.mat3.pos, car->dr_car_uvs);
        pos_in[0] = 0.0f;
        pos_in[1] = 1.2f + view1_suscomp_offset(m);
        pos_in[2] = rear ? -4.0f : 4.0f;
        CopyUV(car->dr_uvs, gCamera[camera].mat.mat3.uvs);
        if (rear) YawUV(3.1415927410125732f, gCamera[camera].mat.mat3.uvs);
        WorldVector(pos_in, pos_out, gCamera[camera].mat.mat3.uvs);
        camera_add(pos_out, gCamera[camera].mat.mat3.pos, gCamera[camera].mat.mat3.pos);
        break;
    case 1:
        for (i = 0; i < 3; i += 2) {
            if (m->peak_center_force[0][i] > 100000.0f) m->peak_center_force[0][i] = 100000.0f;
            if (m->peak_center_force[1][i] < -100000.0f) m->peak_center_force[1][i] = -100000.0f;
            if (m->peak_body_force[0][i] > 100000.0f) m->peak_body_force[0][i] = 100000.0f;
            if (m->peak_body_force[1][i] < -100000.0f) m->peak_body_force[1][i] = -100000.0f;
            pos_in[i] = 0.00002f * m->peak_center_force[0][i];
            pos_in[i] += 0.00002f * m->peak_center_force[1][i];
            m->peak_center_force[0][i] = 0.0f;
            m->peak_center_force[1][i] = 0.0f;
            pos_in[i] += 0.00002f * m->peak_body_force[0][i];
            pos_in[i] += 0.00002f * m->peak_body_force[1][i];
            m->peak_body_force[0][i] = 0.0f;
            m->peak_body_force[1][i] = 0.0f;
        }
        for (i = 0; i < 3; i += 2) cur_acc[camera][i] = (1.0f - 0.85f) * pos_in[i] + 0.85f * cur_acc[camera][i];
        cur_acc[camera][1] = 0.35f * cur_acc[camera][1] - gCamera[camera].view2spring;
        pos_in[0] = -0.55f - cur_acc[camera][0];
        pos_in[1] = 3.2f - cur_acc[camera][1];
        pos_in[2] = -cur_acc[camera][2];
        CopyUV(car->dr_car_uvs, gCamera[camera].mat.mat3.uvs);
        if (rear) YawUV(3.1415927410125732f, gCamera[camera].mat.mat3.uvs);
        WorldVector(pos_in, pos_out, gCamera[camera].mat.mat3.uvs);
        if (pos_out[1] < 0.0f) gCamera[camera].mat.mat3.pos[1] -= 0.7f * pos_out[1];
        update_car_object(car, gCamera[camera].mat.mat3.pos, gCamera[camera].mat.mat3.uvs);
        camera_add(pos_out, gCamera[camera].mat.mat3.pos, gCamera[camera].mat.mat3.pos);
        break;
    case 2:
    case 3:
        camera_copy(gCamera[camera].mat.mat3.pos, pos_in);
        if (rear) {
            view = car->view;
            car->view = 10;
            UpdateCarObj(car, gCamera[camera].mat.mat3.pos, car->dr_car_uvs, pos_in);
            car->view = view;
        } else View3Cam(car, gCamera[camera].mat.mat3.pos);
        CopyUV(gCamera[camera].fcam.mat.mat3.uvs, gCamera[camera].mat.mat3.uvs);
        camera_copy(gCamera[camera].fcam.mat.mat3.pos, gCamera[camera].mat.mat3.pos);
        if (m->resurrect.moving_state < 0) CameraCollision(gCamera[camera].mat.mat3.pos, pos_in);
        break;
    case 10:
        camera_copy(gCamera[camera].mat.mat3.pos, pos_in);
        UpdateCarObj(car, gCamera[camera].mat.mat3.pos, car->dr_car_uvs, pos_in);
        CopyUV(gCamera[camera].fcam.mat.mat3.uvs, gCamera[camera].mat.mat3.uvs);
        camera_copy(gCamera[camera].fcam.mat.mat3.pos, gCamera[camera].mat.mat3.pos);
        CameraCollision(gCamera[camera].mat.mat3.pos, pos_in);
        break;
    case 8:
        camera_copy(gCamera[camera].mat.mat3.pos, pos_in);
        UpdateCarObj(car, gCamera[camera].mat.mat3.pos, car->dr_car_uvs, pos_in);
        CopyUV(gCamera[camera].fcam.mat.mat3.uvs, gCamera[camera].mat.mat3.uvs);
        camera_copy(gCamera[camera].fcam.mat.mat3.pos, gCamera[camera].mat.mat3.pos);
        break;
    case 4:
        DeathCam(car, gCamera[camera].mat.mat3.pos, car->dr_car_uvs);
        camera_copy(gCamera[camera].fcam.mat.mat3.pos, gCamera[camera].mat.mat3.pos);
        CopyUV(gCamera[camera].fcam.mat.mat3.uvs, gCamera[camera].mat.mat3.uvs);
        CameraCollision(gCamera[camera].mat.mat3.pos, car->dr_pos);
        break;
    case 5:
        circle_camera_around_car(car, gCamera[camera].mat.mat3.pos, car->dr_car_uvs);
        camera_copy(gCamera[camera].fcam.mat.mat3.pos, gCamera[camera].mat.mat3.pos);
        CopyUV(gCamera[camera].fcam.mat.mat3.uvs, gCamera[camera].mat.mat3.uvs);
        break;
    case 6:
        fix_camera_in_space(2, car, gCamera[camera].mat.mat3.pos, car->dr_car_uvs);
        camera_copy(gCamera[camera].fcam.mat.mat3.pos, gCamera[camera].mat.mat3.pos);
        CopyUV(gCamera[camera].fcam.mat.mat3.uvs, gCamera[camera].mat.mat3.uvs);
        break;
    case 7:
        steady_move_cam(2, car, gCamera[camera].mat.mat3.pos, car->dr_car_uvs);
        camera_copy(gCamera[camera].fcam.mat.mat3.pos, gCamera[camera].mat.mat3.pos);
        CopyUV(gCamera[camera].fcam.mat.mat3.uvs, gCamera[camera].mat.mat3.uvs);
        break;
    case 9:
        maxpath_cam(2, car, gCamera[camera].mat.mat3.pos, car->dr_car_uvs);
        camera_copy(gCamera[camera].fcam.mat.mat3.pos, gCamera[camera].mat.mat3.pos);
        CopyUV(gCamera[camera].fcam.mat.mat3.uvs, gCamera[camera].mat.mat3.uvs);
        break;
    }
}

/* 0x8000787c, size 0xd0 */
static void _UpdateDroneObjects(void)
{
    anon_mat3_mat4_30 *mat;
    signed short i;
    for (i = 0; i < 6; i++) {
        if (model[i].in_game && gGameCar[i].objnum >= 0 && model[i].drone_type != 2 && !model[i].hide_car) {
            mat = ZOID_FindMatrix(gGameCar[i].objnum);
            mat->mat3.pos[0] = gGameCar[i].dr_pos[0];
            mat->mat3.pos[1] = gGameCar[i].dr_pos[1];
            mat->mat3.pos[2] = gGameCar[i].dr_pos[2];
            mat->mat3.pos[1] += 0.25f;
            CopyUV(gGameCar[i].dr_car_uvs, mat->mat3.uvs);
        }
    }
}

float FSCLS[3] = { 0.6f, 1.2f, 1.5f };

/* 0x8000794c, size 0xa4 */
void update_car_object(tCarData *car, float *pos, float (*uvs)[3])
{
    float newuvs[3][3];
    CopyUV(uvs, newuvs);
    if (gFrameScale) {
        newuvs[0][1] *= FSCLS[gFrameScale-1];
        newuvs[1][1] *= FSCLS[gFrameScale-1];
        newuvs[2][1] *= FSCLS[gFrameScale-1];
    }
    if (car->objnum >= 0) ZOID_UpdateObject(car->objnum, pos, newuvs);
}

/* 0x800079f0, size 0xc0 */
void UpdateCarObj(tCarData *car, float *pos, float (*uvs)[3], float *carpos)
{
    float res[3];
    int camera = car->camera;
    update_car_object(car, pos, uvs);
    if (car->view == 10) _UpdateRearCamera(car, -rear_z[camera], rear_y[camera], res);
    else _UpdateRearCamera(car, rear_z[camera], rear_y[camera], res);
    point_at_car(car, carpos, 0, res);
}

/* 0x80007ab0, size 0x114 */
void DeathCam(tCarData *car, float *pos, float (*uvs)[3])
{
    int i;
    float rpos[3];
    float res[3];
    int camera = car->camera;
    ZOID_UpdateObject(car->objnum, pos, uvs);
    elasticity[camera] = 0.13f;
    _UpdateRearCamera(car, 30.0f, 16.0f, res);
    for(i=0;i<3;i++) { gCamera[camera].fcam.mat.mat3.pos[i] = gCamera[camera].fcam.mat.mat3.pos[i] * (1.0f - elasticity[camera]) + elasticity[camera] * (pos[i] + res[i]); }
    for(i=0;i<3;i++) { rpos[i] = pos[i] - gCamera[camera].fcam.mat.mat3.pos[i]; }
    LookInDir(rpos, &gCamera[camera].fcam.mat);
}

/* 0x80007bc4, size 0x42c */
static void _UpdateRearCamera(tCarData *car, float behind, float side, float *camoff)
{
    float vec[3];
    float res[3];
    float temp[3];
    float a;
    float fact;
    float magvel;
    int camera = car->camera;
    magvel = car->view == 8 ? 0.0f : Distance2D(car->dr_vel[0], car->dr_vel[2]);
    if (magvel <= 100.0f) {
        vec[0] = 0.0f;
        vec[1] = side;
        vec[2] = -behind;
        if (car->view == 8) {
            temp[0] = vec[0];
            temp[1] = vec[1];
            temp[2] = vec[2];
        } else WorldVector(vec, temp, car->dr_uvs);
        temp[1] = fabs(temp[1]);
    }
    if (magvel > 1.0f) {
        if (car->view == 4) {
            vec[0] = car->v[2];
            vec[1] = car->v[1];
            vec[2] = 0.0f;
            WorldVector(vec, res, car->dr_uvs);
            res[0] = -res[0];
            res[2] = -res[2];
        } else {
            res[0] = car->dr_vel[0];
            res[1] = car->dr_vel[1];
            res[2] = car->dr_vel[2];
            if (behind < 0.0f) res[1] = -res[1];
            BodyVector(res, vec, car->dr_uvs);
            vec[2] = fabs(vec[2]);
            WorldVector(vec, res, car->dr_uvs);
            res[0] = -res[0];
            res[2] = -res[2];
        }
        fact = behind / magvel;
        res[0] *= fact;
        res[1] *= fact;
        res[2] *= fact;
        a = -res[1];
        fact = sqrtf(res[0] * res[0] + res[2] * res[2]);
        if (fact > 0.001f) {
            float scale = (fact - a * side / behind) / fact;
            res[0] *= scale;
            res[2] *= scale;
        }
        res[1] = fabs(-res[1] + fact * side / behind);
        if (magvel < 100.0f)
            Interpolate(temp, res, 3, 1.0f, 100.0f, magvel, res);
    } else {
        res[0] = temp[0];
        res[1] = temp[1];
        res[2] = temp[2];
    }
    if (car->view == 8) elastic_factor[camera] = 0.0f;
    else if (car->view == 4) elastic_factor[camera] = 1.0f - magvel / 100.0f;
    else if (magvel < 100.0f) elastic_factor[camera] = 1.0f - 0.01f * magvel;
    else elastic_factor[camera] = 0.0f;
    camoff[0] = res[0];
    camoff[1] = res[1];
    camoff[2] = res[2];
}

/* 0x80007ff0, size 0x1e0 */
void circle_camera_around_car(tCarData *car, float *pos, float (*uvs)[3])
{
    float res[3];
    float ang;
    int camera = car->camera;
    if (finish_cam[car->slot] == 1 && finish_cam_stime[car->slot] + 10.0f < gModelTime) {
        finish_cam_stime[car->slot] = gModelTime;
        finish_cam[car->slot] = 2;
    }
    if (finish_cam[car->slot] == 2)
        ang = 6.2831854820251465f * ((1.0f / 6000.0f) * ((unsigned int)(1000.0f * finish_cam_stime[car->slot]) % 6000));
    else
        ang = 6.2831854820251465f * ((1.0f / 6000.0f) * ((unsigned int)(1000.0f * gModelTime) % 6000));
    res[0] = 15.0f * (float)cos(ang);
    res[1] = 10.0f;
    res[2] = 15.0f * (float)sin(ang);
    elastic_factor[camera] = 0.0f;
    point_at_car(car, pos, uvs, res);
}

anon_zoom_unused_10 track1Spots[17] = {
    1, 0, 0, 1490.0f, 560.0f, 1920.0f, 0, 0, 0, 828.0f, 533.0f, 1780.0f, 1, 0, 0, 1080.0f, 547.0f,
    1120.0f, 1, 0, 0, 1994.0f, 574.0f, 1173.0f, 1, 0, 0, 2629.0f, 598.0f, 1217.0f, 0, 0, 0,
    3704.0f, 582.0f, 1219.0f, 1, 0, 0, 4380.0f, 518.0f, 1156.0f, 1, 0, 0, 4907.0f, 521.0f, 1147.0f,
    0, 0, 0, 5200.0f, 549.0f, 384.0f, 1, 0, 0, 4668.0f, 580.0f, -149.0f, 1, 0, 0, 4312.0f, 580.0f,
    -452.0f, 0, 0, 0, 3049.0f, 631.0f, -958.0f, 0, 0, 0, 2935.0f, 714.0f, -308.0f, 1, 0, 0,
    2946.0f, 758.0f, 158.0f, 0, 0, 0, 2924.0f, 716.0f, 469.0f, 1, 0, 0, 2915.0f, 641.0f, 1694.0f,
    0, 0, 0, 0.0f, -999.0f, 0.0f
};
anon_zoom_unused_10 track2Spots[12] = {
    1, 0, 0, 2418.0f, 210.0f, -2761.0f, 0, 0, 0, 1696.0f, 160.0f, -2652.0f, 0, 0, 0, -1231.0f,
    95.0f, -2252.0f, 1, 0, 0, -1801.0f, -47.0f, -3039.0f, 1, 0, 0, -1692.0f, -32.0f, -5132.0f, 0,
    0, 0, -1179.0f, 24.0f, -5868.0f, 0, 0, 0, -1042.0f, -24.0f, -5844.0f, 0, 0, 0, 957.0f, 92.0f,
    -7121.0f, 0, 0, 0, 3297.0f, 101.0f, -5942.0f, 0, 0, 0, 3475.0f, 116.0f, -4589.0f, 0, 0, 0,
    3397.0f, 167.0f, -2971.0f, 0, 0, 0, 0.0f, -999.0f, 0.0f
};
anon_zoom_unused_10 track3Spots[15] = {
    1, 0, 0, -240.0f, 215.0f, 1376.0f, 0, 0, 0, 762.0f, 234.0f, 2411.0f, 0, 0, 0, 2152.0f, 191.0f,
    2496.0f, 0, 0, 0, 3103.0f, 204.0f, 2538.0f, 1, 0, 0, 4153.0f, 216.0f, 2441.0f, 1, 0, 0,
    4167.0f, 240.0f, 874.0f, 0, 0, 0, 2702.0f, 433.0f, 281.0f, 0, 0, 0, 2812.0f, 557.0f, -408.0f,
    1, 0, 0, 1899.0f, 575.0f, -1295.0f, 0, 0, 0, 784.0f, 416.0f, -2596.0f, 0, 0, 0, 225.0f, 370.0f,
    -2822.0f, 1, 0, 0, 26.0f, 457.0f, -1965.0f, 0, 0, 0, -70.0f, 324.0f, -330.0f, 0, 0, 0, -42.0f,
    235.0f, 474.0f, 0, 0, 0, 0.0f, -999.0f, 0.0f
};
anon_zoom_unused_10 track4Spots[12] = {
    0, 0, 0, 5540.0f, 28.0f, 273.0f, 1, 0, 0, 5474.0f, 240.0f, -473.0f, 1, 0, 0, 5474.0f, 190.0f,
    -1069.0f, 1, 0, 0, 6249.0f, 205.0f, -1048.0f, 1, 0, 0, 6244.0f, 192.0f, -1611.0f, 1, 0, 0,
    5495.0f, 273.0f, -1573.0f, 1, 0, 0, 4632.0f, 207.0f, -1564.0f, 1, 0, 0, 3964.0f, 71.0f,
    -1395.0f, 0, 0, 0, 3981.0f, 128.0f, -3232.0f, 0, 0, 0, 7562.0f, 9.0f, -1700.0f, 0, 0, 0,
    6597.0f, 6.0f, -592.0f, 0, 0, 0, 0.0f, -999.0f, 0.0f
};
anon_zoom_unused_10 track5Spots[19] = {
    0, 0, 0, 510.0f, 35.0f, 1891.0f, 1, 0, 0, 1829.0f, 20.0f, 1500.0f, 1, 0, 0, 2042.0f, -2.0f,
    955.0f, 0, 0, 0, 1413.0f, 14.0f, 880.0f, 1, 0, 0, 2306.0f, 240.0f, 30.0f, 1, 0, 0, 2977.0f,
    207.0f, -335.0f, 1, 0, 0, 2629.0f, 36.0f, 828.0f, 0, 0, 0, 3178.0f, 310.0f, -127.0f, 1, 0, 0,
    1260.0f, 45.0f, -1815.0f, 1, 0, 0, 733.0f, 15.0f, -1920.0f, 1, 0, 0, 93.0f, 15.0f, -2194.0f, 1,
    0, 0, -417.0f, 13.0f, -2133.0f, 1, 0, 0, -400.0f, 13.0f, -1364.0f, 1, 0, 0, -977.0f, 14.0f,
    -950.0f, 0, 0, 0, -685.0f, 8.0f, -510.0f, 0, 0, 0, -606.0f, 15.0f, 227.0f, 1, 0, 0, -894.0f,
    15.0f, 1128.0f, 1, 0, 0, -122.0f, 75.0f, 1982.0f, 0, 0, 0, 0.0f, -999.0f, 0.0f
};
anon_zoom_unused_10 track6Spots[21] = {
    0, 0, 0, -299.0f, 136.0f, 5750.0f, 0, 0, 0, -339.0f, 130.0f, 3352.0f, 1, 0, 0, -777.0f, 114.0f,
    1495.0f, 0, 0, 0, 85.0f, 138.0f, 1010.0f, 0, 0, 0, 102.0f, 138.0f, -74.0f, 0, 0, 0, -1138.0f,
    219.0f, -511.0f, 0, 0, 0, -1134.0f, 376.0f, -1538.0f, 0, 0, 0, -1158.0f, 188.0f, -2991.0f, 0,
    0, 0, -1376.0f, 195.0f, -3559.0f, 0, 0, 0, -41.0f, 178.0f, -3554.0f, 1, 0, 0, 1255.0f, 241.0f,
    -3066.0f, 1, 0, 0, 1850.0f, 192.0f, -3022.0f, 0, 0, 0, 1836.0f, 290.0f, -1488.0f, 0, 0, 0,
    2629.0f, 425.0f, -1096.0f, 0, 0, 0, 2927.0f, 477.0f, -439.0f, 0, 0, 0, 2882.0f, 371.0f, 524.0f,
    1, 0, 0, 2509.0f, 255.0f, 1143.0f, 1, 0, 0, 1817.0f, 211.0f, 1336.0f, 0, 0, 0, 1172.0f, 117.0f,
    1416.0f, 1, 0, 0, 358.0f, 118.0f, 2127.0f, 0, 0, 0, 0.0f, -999.0f, 0.0f
};
anon_zoom_unused_10 *trackSpots[6] = {
    (void *)&track1Spots, (void *)&track2Spots, (void *)&track3Spots, (void *)&track4Spots,
    (void *)&track5Spots, (void *)&track6Spots
};

/* 0x800081d0, size 0x1fc */
void init_camera_on_track(tCarData *car)
{
    anon_zoom_unused_10 *spots;
    signed short cnt;
    float pos[3];
    float old[3];
    float uvs[3][3];
    signed short pt;
    cnt = (car->rwmph >> 6) + 5;
    pt = car->mpctl.idx;
    while (--cnt >= 0) pt = NextPathPt(pt, 0);
    old[0] = gDronePaths[car->mpctl.path].pts[pt].pos[0];
    old[1] = gDronePaths[car->mpctl.path].pts[pt].pos[1];
    old[2] = gDronePaths[car->mpctl.path].pts[pt].pos[2];
    spots = trackSpots[gTrackNum];
    if (spots) {
        while (spots->pos[1] != -999.0f) {
            pos[0] = spots->pos[0] - old[0];
            pos[1] = spots->pos[1] - old[1];
            pos[2] = spots->pos[2] - old[2];
            if (!(Distance2D(pos[0], pos[2]) > 200.0f)) {
                if (spots->zoom) fix_camera_in_space(1, car, spots->pos, uvs);
                else fix_camera_in_space(0, car, spots->pos, uvs);
                return;
            }
            spots++;
        }
    }
    pos[0] = old[0];
    pos[1] = 7.0f + old[1] + Random(5.0f);
    pos[2] = old[2];
    CameraCollision(pos, old);
    fix_camera_in_space(0, car, pos, uvs);
}

/* 0x800083cc, size 0x194 */
void fix_camera_in_space(signed short mode, tCarData *car, float *pos, float (*uvs)[3])
{
    static float cam_pos[4][3];
    static signed char cam_mode[4];
    float res[3];
    float dist;
    int camera = car->camera;
    float scale;
    if (mode == 0) {
        cam_mode[camera] = 0;
        cam_pos[camera][0] = pos[0];
        cam_pos[camera][1] = pos[1];
        cam_pos[camera][2] = pos[2];
    } else if (mode == 1) {
        cam_mode[camera] = 1;
        cam_pos[camera][0] = pos[0];
        cam_pos[camera][1] = pos[1];
        cam_pos[camera][2] = pos[2];
    } else {
        res[0] = cam_pos[camera][0] - pos[0];
        res[1] = cam_pos[camera][1] - pos[1];
        res[2] = cam_pos[camera][2] - pos[2];
        elastic_factor[camera] = 0.0f;
        if (cam_mode[camera] && (dist = magnitude(res)) != 0.0f) {
            scale = (0.15714286267757416f * (dist - 50.0f) + 20.0f) / dist;
            res[0] *= scale;
            res[1] *= scale;
            res[2] *= scale;
        }
        point_at_car(car, pos, uvs, res);
    }
}

/* 0x80008560, size 0x78 */
void init_steady_move_cam(tCarData *car)
{
    float pos[3];
    float uvs[3][3];
    pos[0] = car->dr_pos[0] + 80.0f * car->dr_uvs[2][0] + 20.0f * car->dr_uvs[0][0];
    pos[1] = car->dr_pos[1] + 5.0f;
    pos[2] = car->dr_pos[2] + 80.0f * car->dr_uvs[2][2] + 20.0f * car->dr_uvs[0][2];
    steady_move_cam(0, car, pos, uvs);
}

/* steady_move_cam helpers (no DWARF entries): the parameter temporaries keep the original operand order of the sum and quotient and the register use of the delta */
static inline float camera_steady_sum(float delta, float start)
{
    return start + delta;
}
static inline float camera_steady_ratio(float numerator, float denominator)
{
    return numerator / denominator;
}
static inline void camera_steady_delta(tCarData *car, float *start, float *dst)
{
    float x, y, z;
    x = car->dr_pos[0] - 40.0f * car->dr_uvs[2][0];
    y = car->dr_pos[1] + 5.0f;
    z = car->dr_pos[2] - 40.0f * car->dr_uvs[2][2];
    dst[0] = x - start[0];
    dst[1] = y - start[1];
    dst[2] = z - start[2];
}

/* 0x800085d8, size 0x4f8 */
void steady_move_cam(signed short mode, tCarData *car, float *pos, float (*uvs)[3])
{
    static float legDuration[2] = { 1.4f, 1.0f };
    static float startPos[4][3];
    static signed short stage[4];
    static float legTime[4];
    float delta[4][3];
    float res[3];
    int camera = car->camera;
    if (mode == 0) {
        camera_copy(pos, startPos[camera]);
        camera_copy(pos, gCamera[camera].fcam.mat.mat3.pos);
        camera_copy(car->dr_pos, res);
        pos[0] -= res[0];
        pos[1] -= res[1];
        pos[2] -= res[2];
        LookInDir(pos, &gCamera[camera].fcam.mat);
        stage[camera] = 0;
        legTime[camera] = 0.0f;
    } else {
        switch (stage[camera]) {
        case 0:
            _UpdateRearCamera(car, rear_z[camera], rear_y[camera], res);
            camera_add(pos, res, res);
            delta[camera][0] = res[0] - startPos[camera][0];
            delta[camera][1] = res[1] - startPos[camera][1];
            delta[camera][2] = res[2] - startPos[camera][2];
            camera_scale(delta[camera], camera_steady_ratio(0.1f * legTime[camera], legDuration[stage[camera]]), delta[camera]);
            camera_steady_delta(car, startPos[camera], delta[camera]);
            camera_scale(delta[camera], camera_steady_ratio(legTime[camera], legDuration[stage[camera]]), delta[camera]);
            break;
        case 1:
            _UpdateRearCamera(car, rear_z[camera], rear_y[camera], res);
            camera_add(pos, res, res);
            delta[camera][0] = res[0] - startPos[camera][0];
            delta[camera][1] = res[1] - startPos[camera][1];
            delta[camera][2] = res[2] - startPos[camera][2];
            camera_scale(delta[camera], camera_steady_ratio(legTime[camera], legDuration[stage[camera]]), delta[camera]);
            break;
        case 2:
            init_view3(car);
            if (!(gGameState & 8)) {
                car->view = car->saveview = (*car->player->save)->datah ? GetPlayerCarView(car->player->save) : 2;
                if (car->view == 1) {
                    cur_acc[car->camera][0] = 0.0f;
                    cur_acc[car->camera][1] = 0.0f;
                    cur_acc[car->camera][2] = 0.0f;
                }
            }
            break;
        }
        if (stage[camera] == 0 || stage[camera] == 1) {
            { float x, y, z;
            x = camera_steady_sum(delta[camera][0], startPos[camera][0]);
            y = camera_steady_sum(delta[camera][1], startPos[camera][1]);
            z = camera_steady_sum(delta[camera][2], startPos[camera][2]);
            res[0] = x - pos[0]; res[1] = y - pos[1]; res[2] = z - pos[2]; }
            elastic_factor[camera] = 0.0f;
            point_at_car(car, pos, uvs, res);
            gCamera[camera].fcam.mat.mat3.pos[1] += 4.0f;
            legTime[camera] += gFrameSeconds;
            if (legTime[camera] >= legDuration[stage[camera]]) {
                legTime[camera] = 0.0f;
                stage[camera]++;
                camera_copy(gCamera[camera].fcam.mat.mat3.pos, startPos[camera]);
            }
        }
    }
}









/* 0x80008ad0, size 0x3c0 */
void maxpath_cam(signed short mode, tCarData *car, float *pos, float (*uvs)[3])
{
    static signed short startPoint[4];
    static signed short startPath[4];
    static signed short lastPoint[4];
    static float startTime[4];
    static float lastDist[4];
    signed short point;
    signed short nextPoint;
    signed short path;
    float newPos[3];
    float numFeet;
    int camera = car->camera;
    float delta[3], dist, scale;
    if (mode == 0) {
        startPoint[camera] = lastPoint[camera] = pos[0];
        startPath[camera] = car->mpctl.path;
        startTime[camera] = gModelTime;
        lastDist[camera] = 0.0f;
    } else {
        if (gModelTime < startTime[camera]) startTime[camera] = 0.0f;
        point = lastPoint[camera];
        numFeet = (100.0f + 1.4666666984558105f * (car->rwmph >> 2)) * (gModelTime - startTime[camera]) - lastDist[camera];
        path = startPath[camera];
        for (;;) {
            nextPoint = NextPathPt(point, path);
            delta[0] = gDronePaths[path].pts[nextPoint].pos[0] - gDronePaths[path].pts[point].pos[0];
            delta[1] = gDronePaths[path].pts[nextPoint].pos[1] - gDronePaths[path].pts[point].pos[1];
            delta[2] = gDronePaths[path].pts[nextPoint].pos[2] - gDronePaths[path].pts[point].pos[2];
            dist = sqrtf(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);
            if (dist <= numFeet) {
                numFeet -= dist;
                lastDist[camera] += dist;
                lastPoint[camera] = nextPoint;
            } else {
                scale = numFeet / dist;
                newPos[0] = gDronePaths[path].pts[point].pos[0] + delta[0] * scale;
                newPos[1] = gDronePaths[path].pts[point].pos[1] + delta[1] * scale;
                newPos[2] = gDronePaths[path].pts[point].pos[2] + delta[2] * scale;
                break;
            }
            point = nextPoint;
        }
        newPos[1] += 10.0f;
        newPos[0] -= pos[0];
        newPos[1] -= pos[1];
        newPos[2] -= pos[2];
        elastic_factor[camera] = 0.0f;
        point_at_car(car, pos, uvs, newPos);
    }
}

/* 0x80008e90, size 0x1a8 */
void point_at_car(tCarData *car, float *pos, float (*uvs)[3], float *res)
{
    int i;
    float rpos[3];
    int camera = car->camera;
    if (uvs) update_car_object(car, pos, uvs);
    if (model[car->slot].resurrect.moving_state >= 0) elasticity[camera] = 0.0f;
    else {
        elasticity[camera] = 0.6f * elastic_factor[camera];
        if (gLastFrameCount == 3) elasticity[camera] *= 0.75f;
        else if (gLastFrameCount == 4) elasticity[camera] *= 0.6f;
        else if (gLastFrameCount == 5) elasticity[camera] *= 0.5f;
    }
    for (i = 0; i < 3; i++) {
        gCamera[camera].fcam.mat.mat3.pos[i] = gCamera[camera].fcam.mat.mat3.pos[i] * elasticity[camera] + (1.0f - elasticity[camera]) * (pos[i] + res[i]);
        rpos[i] = pos[i] - gCamera[camera].fcam.mat.mat3.pos[i];
    }
    LookInDir(rpos, &gCamera[camera].fcam.mat);
}

/* 0x80009038, size 0x180 */
void LookInDir(float *lookdir, anon_mat3_mat4_30 *mat)
{
    float length;
    mat->uvs[2][0] = lookdir[0];
    mat->uvs[2][1] = lookdir[1];
    mat->uvs[2][2] = lookdir[2];
    direction(mat->uvs[2], mat->uvs[2]);
    mat->uvs[0][0] = lookdir[2];
    mat->uvs[0][1] = 0.0f;
    mat->uvs[0][2] = -lookdir[0];
    length = magnitude(mat->uvs[0]);
    if (length <= 0.01f) {
        mat->uvs[0][0] = rvector[0];
        mat->uvs[0][1] = rvector[1];
        mat->uvs[0][2] = rvector[2];
    } else {
        length = 1.0f / length;
        mat->uvs[0][0] *= length;
        mat->uvs[0][1] *= length;
        mat->uvs[0][2] *= length;
    }
    mat->uvs[1][0] = mat->uvs[2][1] * mat->uvs[0][2] - mat->uvs[2][2] * mat->uvs[0][1];
    mat->uvs[1][1] = mat->uvs[2][2] * mat->uvs[0][0] - mat->uvs[2][0] * mat->uvs[0][2];
    mat->uvs[1][2] = mat->uvs[2][0] * mat->uvs[0][1] - mat->uvs[2][1] * mat->uvs[0][0];
    mat->uvs[0][0] = mat->uvs[1][1] * mat->uvs[2][2] - mat->uvs[1][2] * mat->uvs[2][1];
    mat->uvs[0][1] = mat->uvs[1][2] * mat->uvs[2][0] - mat->uvs[1][0] * mat->uvs[2][2];
    mat->uvs[0][2] = mat->uvs[1][0] * mat->uvs[2][1] - mat->uvs[1][1] * mat->uvs[2][0];
}

