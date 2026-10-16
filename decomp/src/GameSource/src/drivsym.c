/* drivsym.c - vehicle simulation controls and integration. Status: complete C reconstruction.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\drivsym.c; assembly: build/2049LE/asm/GameSource/src/drivsym.s */
/* The GC caller passes an eighth floating argument to dotireforce. Its DWARF
   declaration describes only the seven used by the callee. Preserve this TU's
   observed caller ABI without changing the shared recovered declaration. */
#define dotireforce dotireforce_dwarf_decl
#include "rush/rush_protos.h"
#undef dotireforce
extern void dotireforce(tModelData *, float *, float (*)[3], anon_tradius_springK_5c *,
    float, float *, float, float, float, float, float, float, int, float);
#define MSL_NO_SQRTF
#include "msl_math_inline.h"

static void _InitModelControls(tModelData *m);
static void _ModelControls(tModelData *m);

float gWingDrag[3] = { 1.0f, 1.5f, 1.0f };
float gWingLift[3] = { 1.0f, 0.8f, 0.5f };
float gWingAngVDrag[2] = { 1e-05f, 0.0005f };
signed char gWind;
float gGravity;
signed char gSuperSpeed;
signed char gInvincible;
signed char gSpecialTires;

/* 0x8000f3f4, size 0x2c */
static void _InitModelControls(tModelData *m)
{
    m->mainin.wheel = 0.0f;
    m->mainin.clutch = 0.0f;
    m->mainin.brake = 0.0f;
    m->mainin.throttle = 1.0f;
    m->mainin.gear = 1;
}

/* 0x8000f420, size 0x1dc */
static void _ModelControls(tModelData *m)
{
    float rolloff; /* f3 */
    int i; /* r0 */
    anon_tradius_springK_5c *td; /* r5 */
    tCarData *gc; /* r4 */

    gc = &gGameCar[m->slot];
    m->dt = m->fastin.dt;
    m->idt = 1.0f / m->dt;
    m->steerangle = m->steergain * m->mainin.wheel;
    m->clutch = m->mainin.clutch;
    if (gc->place_locked == 1) {
        m->throttle = 0.0f;
    } else {
        m->throttle = m->mainin.throttle;
    }
    if (gEndGame || gc->place_locked) {
        m->brake = 0.7f;
        m->throttle = 0.0f;
        m->clutch = 1.0f;
    } else if (gCoastFlag) {
        m->brake = 0.33f;
        m->throttle = 0.0f;
        m->clutch = 1.0f;
    } else {
        m->brake = m->mainin.brake;
    }
    if (m->brake > 0.99f) {
        m->brake = 1.0f;
    }
    if (!m->autotrans) {
        m->commandgear = m->gear = m->mainin.gear;
    } else {
        m->commandgear = m->mainin.gear;
    }
    if (m->fastin.modelIteration < 0) {
        m->gear = m->commandgear = 0;
        m->brake = m->clutch = 1.0f;
    }
    for (i = 0, td = m->tires; i < 4; i++, td++) {
        if (td->angvel > 0.0f) {
            if (td->angvel < 10.0f) {
                if (td->angvel < 2.0f) {
                    rolloff = 0.0f;
                } else {
                    rolloff = 0.05f * td->angvel;
                }
            } else {
                rolloff = 1.0f;
            }
        } else {
            if (td->angvel > -10.0f) {
                if (td->angvel > -2.0f) {
                    rolloff = 0.0f;
                } else {
                    rolloff = 0.05f * td->angvel;
                }
            } else {
                rolloff = -1.0f;
            }
        }
        m->torque[i] = rolloff * (-m->brakegain[i] * m->brake);
    }
}

/* 0x8000f5fc, size 0x4c */
void syminit(tModelData *m)
{
    initialize(m);
    _InitModelControls(m);
    road(m);
    positions(m);
    mcommunication(m);
}

/* 0x8000f648, size 0x20c */
void sym(tModelData *m)
{
    int i; /* r0 */
    int j; /* r0 */

    _ModelControls(m);
    if (m->crashflag || gGameCar[m->slot].dead) {
        m->throttle = m->brake = m->clutch = m->steerangle = 0.0f;
    }
    drivetrain(m);
    regular(m);
    checkok(m);
    mcommunication(m);
    if (!m->autotrans && m->throttle > 0.5f &&
        (m->rpm >= 0 ? m->rpm : -m->rpm) < 600) {
        if (m->bogState == 0) {
            m->bogState = 1;
        }
    } else {
        if (m->bogState != 0) {
            m->bogState = 3;
        }
    }
    if (m->drone_type == 2) {
        for (i = 0; i < 3; i++) {
            if (m->centerForce[i] > m->peak_center_force[0][i]) {
                m->peak_center_force[0][i] = m->centerForce[i];
            }
            if (m->centerForce[i] < m->peak_center_force[1][i]) {
                m->peak_center_force[1][i] = m->centerForce[i];
            }
            for (j = 0; j < 4; j++) {
                if (m->bodyForce[j][i] > m->peak_body_force[0][i]) {
                    m->peak_body_force[0][i] = m->bodyForce[j][i];
                }
                if (m->bodyForce[j][i] < m->peak_body_force[1][i]) {
                    m->peak_body_force[1][i] = m->bodyForce[j][i];
                }
            }
        }
    }
    m->centerForceLast[0] = m->centerForce[0];
    m->centerForceLast[1] = m->centerForce[1];
    m->centerForceLast[2] = m->centerForce[2];
    m->centerForce[2] = m->centerForce[1] = m->centerForce[0] = 0.0f;
    m->centerMoment[2] = m->centerMoment[1] = m->centerMoment[0] = 0.0f;
}

/* 0x8000f854, size 0x5c */
void regular(tModelData *m)
{
    road(m);
    forces(m);
    antispin(m);
    torques(m);
    accelerations(m);
    velocities(m);
    positions(m);
}

/* 0x8000f8b0, size 0x114 */
void antispin(tModelData *m)
{
    float drift; /* f3 */

    if (m->appearance & 0x10) {
        return;
    }
    if (m->roadcode[0] != 8 || m->roadcode[1] != 8) {
        drift = m->drift;
        drift += 0.5f * m->brake;
        if (drift > 1.0f) {
            drift = 1.0f;
        }
        if (m->magvel > 100.0f) {
            m->centerMoment[1] -= (12000.0f * m->mainin.wheel) * drift;
        } else {
            m->centerMoment[1] -= drift * (120.0f * (m->mainin.wheel * m->magvel));
        }
    }
    if (m->v[2] < 0.0f) {
        return;
    }
    if (m->we_control) {
        if (m->w[1] * m->v[0] > 0.0f) {
            m->centerMoment[1] -= 100.0f * m->v[0];
        }
    } else {
        if (m->w[1] * m->v[0] > 0.0f && m->brake < 0.5f) {
            m->centerMoment[1] -= (100.0f * m->v[0]) * m->tune->antispin;
        }
    }
}

/* 0x8000f9c4, size 0x434 */
void MovingSurfaceForce(tModelData *m)
{
    surftyp *sp; /* r29 */
    float speed; /* f3 */
    float temp[3]; /* stack */
    float temp1[3]; /* stack */
    int i; /* r7 */
    int obj; /* r28 */
    int on; /* r8 */
    int mask; /* r27 */

    sp = m->movingSurface;
    obj = (sp->snd_flags >> 11) & 31;
    if ((sp->snd_flags & 0x10) && (obj <<= 3) > 50) {
        temp[0] = 0.00006103515625f * sp->uvs[0][0];
        temp[1] = 0.00006103515625f * sp->uvs[0][1];
        temp[2] = 0.00006103515625f * sp->uvs[0][2];
        speed = temp[0] * m->rwv[0] + temp[1] * m->rwv[1] + temp[2] * m->rwv[2];
        speed = 1.4666667f * obj - speed;
        speed *= m->mass;
        temp1[0] = temp[0] * speed;
        temp1[1] = temp[1] * speed;
        temp1[2] = temp[2] * speed;
        BodyVector(temp1, temp, m->uvs);
        m->centerForce[0] = temp[0] + m->centerForce[0];
        m->centerForce[1] = temp[1] + m->centerForce[1];
        m->centerForce[2] = temp[2] + m->centerForce[2];
    } else {
        mask = on = 0;
        for (i = 0; i < 4; i++) {
            if (m->airdist[i] <= 0.0f && (surf[m->tiresli[i]].snd_flags & 0x30)) {
                on++;
                mask |= 1 << i;
            }
        }
        if (on >= 3) {
            if (sp->snd_flags & 0x20) {
                GetTargetVelocity(obj, temp);
                m->rwr[0] += temp[0] * m->dt;
                m->rwr[2] += temp[2] * m->dt;
            } else {
                speed = (1.4666667f * obj) * m->dt;
                m->rwr[0] += 0.00006103515625f * (speed * sp->uvs[0][0]);
                m->rwr[2] += 0.00006103515625f * (speed * sp->uvs[0][2]);
            }
        } else {
            for (i = 0; i < 4; i++) {
                if (mask & (1 << i)) {
                    if (sp->snd_flags & 0x20) {
                        GetTargetVelocity(obj, temp);
                        NormalVector(temp);
                    } else {
                        temp[0] = 0.00006103515625f * sp->uvs[0][0];
                        temp[1] = 0.00006103515625f * sp->uvs[0][1];
                        temp[2] = 0.00006103515625f * sp->uvs[0][2];
                    }
                    speed = 30.0f * m->mass;
                    temp[0] *= speed;
                    temp[1] *= speed;
                    temp[2] *= speed;
                    BodyVector(temp, temp1, m->uvs);
                    m->tireForce[i][0] = temp1[0] + m->tireForce[i][0];
                    m->tireForce[i][1] = temp1[1] + m->tireForce[i][1];
                    m->tireForce[i][2] = temp1[2] + m->tireForce[i][2];
                }
            }
        }
    }
}

/* 0x8000fdf8, size 0x34 */
void forces(tModelData *m)
{
    forces1(m);
    forces2(m);
}

/* 0x8000fe2c, size 0x520 */
void forces1(tModelData *m)
{
    float speed; /* f3 */
    float airfact; /* f2 */
    float drag; /* f1 */
    float tsc; /* f2 */
    int poortract;

    calctireuv(m->v, m->w, m->car->tireR[0], m->steerangle, m->uvs, m->roaduvs[0], m->tireuvs[0], m->tireV[0]);
    calctireuv(m->v, m->w, m->car->tireR[1], m->steerangle, m->uvs, m->roaduvs[1], m->tireuvs[1], m->tireV[1]);
    calctireuv(m->v, m->w, m->car->tireR[2], 0.0f, m->uvs, m->roaduvs[2], m->tireuvs[2], m->tireV[2]);
    calctireuv(m->v, m->w, m->car->tireR[3], 0.0f, m->uvs, m->roaduvs[3], m->tireuvs[3], m->tireV[3]);
    dotireforce(m, m->tireV[0], m->tireuvs[0], &m->tires[0], m->torque[0], m->tireForce[0],
        m->suscomp[0], m->suscomp[1], m->car->springrate[0], m->car->farspringrate, m->car->cdamping[0], m->car->rdamping[0], 0, 1.0f);
    dotireforce(m, m->tireV[1], m->tireuvs[1], &m->tires[1], m->torque[1], m->tireForce[1],
        m->suscomp[1], m->suscomp[0], m->car->springrate[1], m->car->farspringrate, m->car->cdamping[1], m->car->rdamping[1], 0, 1.0f);
    airfact = 1.0f - m->throttle;
    if (m->tireForce[0][2] < 0.0f) {
        m->tireForce[0][2] *= airfact;
    }
    if (m->tireForce[1][2] < 0.0f) {
        m->tireForce[1][2] *= airfact;
    }
    poortract = 0;
    if ((m->roadcode[2] == 1 || m->roadcode[2] == 2) &&
        (m->roadcode[3] == 1 || m->roadcode[0] == 2)) {
        poortract = 1;
    }
    dotireforce(m, m->tireV[2], m->tireuvs[2], &m->tires[2], m->torque[2], m->tireForce[2],
        m->suscomp[2], m->suscomp[3], m->car->springrate[2], m->car->farspringrate, m->car->cdamping[2], m->car->rdamping[2], poortract, 1.0f);
    dotireforce(m, m->tireV[3], m->tireuvs[3], &m->tires[3], m->torque[3], m->tireForce[3],
        m->suscomp[3], m->suscomp[2], m->car->springrate[3], m->car->farspringrate, m->car->cdamping[3], m->car->rdamping[3], poortract, 1.0f);
    if (m->movingSurface) {
        MovingSurfaceForce(m);
    }
    if (!gGameCar[m->slot].in_tunnel && m->roadcode[0] == 8 && m->roadcode[1] == 8 &&
        m->roadcode[2] == 8 && m->roadcode[3] == 8) {
        m->d[0] = 80.0f * (gWind * (m->uvs[0][0] * gWind));
        m->d[1] = 80.0f * (gWind * (m->uvs[1][0] * gWind));
    } else {
        m->d[1] = m->d[0] = 0.0f;
    }
    if (gGameCar[m->slot].in_tunnel == 0) {
        speed = m->v[2] - 20.0f * (m->uvs[2][0] * gWind);
    } else {
        speed = m->v[2];
    }
    if (m->appearance & 0x10) {
        airfact = 5000.0f;
        drag = 10.0f;
    } else {
        airfact = 30.0f;
        drag = m->car->srefpcybo2;
    }
    if (speed > 0.0f) {
        m->d[2] = -(speed * (drag * speed) + (airfact + m->car->rollresist));
    } else {
        m->d[2] = speed * (drag * speed) + (airfact + m->car->rollresist);
    }
    if (m->wing) {
        m->d[2] *= gWingDrag[gCarWings[m->drone_type == 2 ? m->slot + 1 : 0][m->body_type]];
    }
    if (m->roadcode[3] == 1 && m->roadcode[2] == 1) {
        tsc = -30.0f * (gSpecialTires == 2 ? 0.1f : 1.0f - m->offroadtires);
        m->d[2] += speed * tsc;
    } else if (m->roadcode[3] == 2 && m->roadcode[2] == 2) {
        tsc = -40.0f * (gSpecialTires == 2 ? 0.1f : 1.0f - m->offroadtires);
        m->d[2] += speed * tsc;
    } else {
        tsc = -0.7f * (gSpecialTires == 2 ? 0.0f : m->offroadtires);
        m->d[2] += speed * tsc;
    }
}

/* 0x8001034c, size 0x320 */
void forces2(tModelData *m)
{
    float temp; /* f4 */
    float forces[3]; /* stack */

    if (m->nothrusttorque) {
        temp = 0.5f * (m->tireForce[2][2] + m->tireForce[3][2]);
        if (temp > 0.0f) {
            m->tireForce[2][2] = m->roadcode[3] == 8 ? 0.0f :
                temp * (1.0f - m->tune->diffbl) + m->tireForce[2][2] * m->tune->diffbl;
            m->tireForce[3][2] = m->roadcode[2] == 8 ? 0.0f :
                temp * (1.0f - m->tune->diffbl) + m->tireForce[3][2] * m->tune->diffbl;
        }
    }
    m->f[0] = m->tireForce[0][0] + m->tireForce[1][0];
    m->f[1] = m->tireForce[0][1] + m->tireForce[1][1];
    m->f[2] = m->tireForce[0][2] + m->tireForce[1][2];
    m->f[0] = m->tireForce[2][0] + m->f[0];
    m->f[1] = m->tireForce[2][1] + m->f[1];
    m->f[2] = m->tireForce[2][2] + m->f[2];
    m->f[0] = m->tireForce[3][0] + m->f[0];
    m->f[1] = m->tireForce[3][1] + m->f[1];
    m->f[2] = m->tireForce[3][2] + m->f[2];
    m->grw[1] = -1.0f * (m->mass * gGravity);
    if (m->wing && m->rwr[1] <= m->rwr2[1]) {
        m->grw[1] *= gWingLift[gCarWings[m->drone_type == 2 ? m->slot + 1 : 0][m->body_type]];
    }
    BodyVector(m->grw, m->g, m->uvs);
    m->f[0] = m->g[0] + m->f[0];
    m->f[1] = m->g[1] + m->f[1];
    m->f[2] = m->g[2] + m->f[2];
    m->f[0] = m->d[0] + m->f[0];
    m->f[1] = m->d[1] + m->f[1];
    m->f[2] = m->d[2] + m->f[2];
    forces[0] = m->bodyForce[0][0] + m->bodyForce[1][0];
    forces[1] = m->bodyForce[0][1] + m->bodyForce[1][1];
    forces[2] = m->bodyForce[0][2] + m->bodyForce[1][2];
    forces[0] = m->bodyForce[2][0] + forces[0];
    forces[1] = m->bodyForce[2][1] + forces[1];
    forces[2] = m->bodyForce[2][2] + forces[2];
    forces[0] = m->bodyForce[3][0] + forces[0];
    forces[1] = m->bodyForce[3][1] + forces[1];
    forces[2] = m->bodyForce[3][2] + forces[2];
    m->f[0] = forces[0] + m->f[0];
    m->f[1] = forces[1] + m->f[1];
    m->f[2] = forces[2] + m->f[2];
    m->bodyForces = magnitude(forces);
    m->f[0] = m->centerForce[0] + m->f[0];
    m->f[1] = m->centerForce[1] + m->f[1];
    m->f[2] = m->centerForce[2] + m->f[2];
}

/* 0x8001066c, size 0x304 */
void torques(tModelData *m)
{
    float *rp; /* r4 */
    float *fp; /* r6 */
    int i; /* r0 */
    tCarData *gc; /* r5 */
    /* Keep the arm and cross product live across the vector addition. */
    float arm[3];
    float moment[3];

    m->m[2] = m->m[1] = m->m[0] = 0.0f;
    rp = m->car->tireR[0];
    fp = m->tireForce[0];
    for (i = 0; i < 4; i++, rp += 3, fp += 3) {
        arm[0] = rp[0];
        arm[1] = rp[1];
        arm[1] += m->suscomp[i];
        arm[2] = rp[2];
        moment[0] = fp[1] * arm[2] - fp[2] * arm[1];
        moment[1] = fp[2] * arm[0] - fp[0] * arm[2];
        moment[2] = fp[0] * arm[1] - fp[1] * arm[0];
        m->m[0] += moment[0];
        m->m[1] += moment[1];
        m->m[2] += moment[2];
    }
    rp = m->car->tireR[0];
    fp = m->bodyForce[0];
    for (i = 0; i < 4; i++, rp += 3, fp += 3) {
        arm[0] = rp[0];
        arm[1] = 0.0f;
        arm[2] = rp[2];
        moment[0] = fp[1] * arm[2] - fp[2] * arm[1];
        moment[1] = fp[2] * arm[0] - fp[0] * arm[2];
        moment[2] = fp[0] * arm[1] - fp[1] * arm[0];
        m->m[0] += moment[0];
        m->m[1] += moment[1];
        m->m[2] += moment[2];
    }
    if (!(m->appearance & 0x10) && m->airdist[0] > 5.0f &&
        m->airdist[1] > 5.0f && m->airdist[2] > 5.0f &&
        m->airdist[3] > 5.0f && gRace != kBattle) {
        if (m->mainin.wing) {
            m->wing = 1;
        } else {
            m->wing = 0;
        }
        if (m->wing) {
            gc = &gGameCar[m->slot];
            if (gc->player) {
                m->m[0] -= m->magvel * (gc->player->analog[1] * m->wingEffect);
                m->m[2] += (m->magvel * (gc->player->analog[0] * m->wingEffect)) *
                    (gMirrorWaves ? -1 : 1);
            }
        }
    } else {
        m->wing = 0;
    }
    m->m[0] = m->centerMoment[0] + m->m[0];
    m->m[1] = m->centerMoment[1] + m->m[1];
    m->m[2] = m->centerMoment[2] + m->m[2];
}

/* 0x80010970, size 0x58 */
void accelerations(tModelData *m)
{
    float k; /* f1 */

    k = gGravity / m->weight;
    m->a[0] = m->f[0] * k;
    m->a[1] = m->f[1] * k;
    m->a[2] = m->f[2] * k;
    calcaa(m, m->m, m->aa);
}

/* 0x800109c8, size 0x2a4 */
void velocities(tModelData *m)
{
    float velfact; /* f1 */
    float t; /* f0 */
    float dw[3];

    /* Reuse the scaled vector for linear and angular integration. */
    dw[0] = m->a[0] * m->dt;
    dw[1] = m->a[1] * m->dt;
    dw[2] = m->a[2] * m->dt;
    m->v[0] += dw[0];
    m->v[1] += dw[1];
    m->v[2] += dw[2];
    if (m->throttle < 0.1f) {
        if (fabs(m->v[0]) < 0.025f) {
            m->v[0] = 0.0f;
        }
        if (fabs(m->v[2]) < 0.025f) {
            m->v[2] = 0.0f;
        }
    }
    if (gGameCar[m->slot].dead != 2) {
        m->magvel = magnitude(m->v);
    }
    if (m->magvel > 300.0f) {
        m->v[0] *= 300.0f / m->magvel;
        m->v[1] *= 300.0f / m->magvel;
        m->v[2] *= 300.0f / m->magvel;
    } else {
        dw[0] = m->aa[0] * m->dt;
        dw[1] = m->aa[1] * m->dt;
        dw[2] = m->aa[2] * m->dt;
        if (m->bodyForces) {
            dw[0] -= 0.025f * m->w[0];
            dw[1] -= 0.025f * m->w[1];
            dw[2] -= 0.025f * m->w[2];
        }
        m->w[0] += dw[0];
        m->w[1] += dw[1];
        m->w[2] += dw[2];
        if ((m->airdist[0] > 0.0f && m->airdist[1] > 0.0f &&
             m->airdist[2] > 0.0f && m->airdist[3] > 0.0f) || m->crashflag) {
            velfact = magnitude(m->w);
            velfact *= velfact;
            velfact *= m->crashflag ? gWingAngVDrag[1] : gWingAngVDrag[0];
            m->w[0] -= m->w[0] * velfact;
            m->w[1] -= m->w[1] * velfact;
            m->w[2] -= m->w[2] * velfact;
        }
        if (m->magvel < 3.0f && (m->brake > 0.5f || m->uvs[1][1] < 0.0f)) {
            m->v[2] = m->v[0] = 0.0f;
        }
        if (m->magvel < 3.0f && m->uvs[1][1] < -0.75f && magnitude(m->w) < 0.5f) {
            m->w[2] = m->w[1] = m->w[0] = 0.0f;
        }
    }
}

/* 0x80010c6c, size 0x3a4 */
void positions(tModelData *m)
{
    float temp[3]; /* stack */
    float temp1[3]; /* stack */
    float *bp; /* r6 */
    float *rwp; /* r27 */
    int i; /* r0 */
    float z; /* f0 */

    m->rwr2[0] = m->rwr[0];
    m->rwr2[1] = m->rwr[1];
    m->rwr2[2] = m->rwr[2];
    WorldVector(m->a, m->rwa, m->uvs);
    WorldVector(m->v, m->rwv, m->uvs);
    temp[0] = m->rwv[0] * m->dt;
    temp[1] = m->rwv[1] * m->dt;
    temp[2] = m->rwv[2] * m->dt;
    m->rwr[0] = temp[0] + m->rwr[0];
    m->rwr[1] = temp[1] + m->rwr[1];
    m->rwr[2] = temp[2] + m->rwr[2];
    temp[0] = m->w[0] * m->dt;
    temp[1] = m->w[1] * m->dt;
    temp[2] = m->w[2] * m->dt;
    if (gRace == kStunt) {
        m->stuntW[0] = temp[0] + m->stuntW[0];
        m->stuntW[1] = temp[1] + m->stuntW[1];
        m->stuntW[2] = temp[2] + m->stuntW[2];
    }
    RotateUV(temp, m->uvs);
    if ((m->fastin.modelIteration & 0x3ff) == 0x3ff) {
        if (gSuperSpeed) {
            make_super_speed(m);
        } else {
            z = invmagnitude(m->uvs[0]);
            m->uvs[0][0] *= z;
            m->uvs[0][1] *= z;
            m->uvs[0][2] *= z;
            z = invmagnitude(m->uvs[1]);
            m->uvs[1][0] *= z;
            m->uvs[1][1] *= z;
            m->uvs[1][2] *= z;
            z = invmagnitude(m->uvs[2]);
            m->uvs[2][0] *= z;
            m->uvs[2][1] *= z;
            m->uvs[2][2] *= z;
        }
    }
    BodyVector(m->rwv, m->v, m->uvs);
    temp[1] = 0.0f;
    for (i = 0; i < 4; i++) {
        z = m->tpcomp[i] - 0.5f;
        if (z > temp[1]) {
            temp[1] = z;
            m->tpcomp[i] = 0.5f;
        }
    }
    if (temp[1] > 0.0f) {
        temp[0] = temp[2] = 0.0f;
        WorldVector(temp, temp1, m->uvs);
        m->rwr[0] = temp1[0] + m->rwr[0];
        m->rwr[1] = temp1[1] + m->rwr[1];
        m->rwr[2] = temp1[2] + m->rwr[2];
    }
    for (i = 0; i < 4; i++) {
        rwp = m->tireRwr[i];
        bp = m->car->tireR[i];
        temp[0] = bp[0];
        temp[1] = bp[1] + m->tpcomp[i];
        temp[2] = bp[2];
        WorldVector(temp, rwp, m->uvs);
        rwp[0] = rwp[0] + m->rwr[0];
        rwp[1] = rwp[1] + m->rwr[1];
        rwp[2] = rwp[2] + m->rwr[2];
    }
    for (i = 0; i < 4; i++) {
        rwp = m->bodyrwr[i];
        WorldVector(m->bodyR[i], rwp, m->uvs);
        rwp[0] = rwp[0] + m->rwr[0];
        rwp[1] = rwp[1] + m->rwr[1];
        rwp[2] = rwp[2] + m->rwr[2];
    }
}

/* 0x80011010, size 0x40 */
void calcaa(tModelData *m, float *moment, float *angaccel)
{
    angaccel[0] = moment[0] * m->car->invi[0];
    angaccel[1] = moment[1] * m->car->invi[1];
    angaccel[2] = moment[2] * m->car->invi[2];
}

/* 0x80011050, size 0x424 */
void checkok(tModelData *m)
{
    float temp; /* f3 */
    float forceavg; /* f31 */
    signed short i; /* r5 */
    signed short thump_it; /* r6 */
    signed short thump_side; /* r7 */
    float t; /* f30 */
    float tt; /* f0 */
    float tot; /* f29 */
    int cnt; /* r30 */

    if (gGameCar[m->slot].dead) {
        return;
    }
    thump_it = 0;
    thump_side = 0;
    for (i = 0; i < 4; i++) {
        if (m->tpcomp[i] > 1.2f && thump_it < 3) {
            thump_it = 3;
            thump_side |= 1 << i;
        } else if (m->tpcomp[i] > 0.9f && thump_it < 2) {
            thump_it = 2;
            thump_side |= 1 << i;
        } else if (m->tpcomp[i] > 0.6f && thump_it < 1) {
            thump_it = 1;
            thump_side |= 1 << i;
        }
    }
    m->thump_side = thump_side;
    m->thumpflag = thump_it;
    forceavg = magnitude(m->f);
    if (forceavg > m->crashthreshold) {
        t = 0.15f * m->crashthreshold;
        tot = 0.0f;
        cnt = 0;
        tt = magnitude(m->tireForce[0]);
        if (tt > t) {
            tot += tt;
            cnt++;
        }
        tt = magnitude(m->tireForce[1]);
        if (tt > t) {
            tot += tt;
            cnt++;
        }
        tt = magnitude(m->tireForce[2]);
        if (tt > t) {
            tot += tt;
            cnt++;
        }
        tt = magnitude(m->tireForce[3]);
        if (tt > t) {
            tot += tt;
            cnt++;
        }
        if (cnt > 3) {
            forceavg -= 0.4f * tot;
        } else if (cnt > 2) {
            forceavg -= 0.2f * tot;
        }
    }
    if ((gRace != kBattle && forceavg > m->crashthreshold) ||
        (gRace != kBattle && (m->magvel > 400.0f || m->w[1] > 10.0f || m->w[1] < -10.0f))) {
        if (!m->crashflag && (m->drone_type != 2 || !gInvincible)) {
            m->crashflag = 1;
        }
    }
    m->topscrape = 0;
    temp = 0.0f;
    if (m->bodyForce[2][1] < -700.0f || m->bodyForce[3][1] < -700.0f ||
        m->bodyForce[0][1] < -700.0f || m->bodyForce[1][1] < -700.0f) {
        temp = 1.0f;
        if (m->airdist[1] > 3.0f && m->airdist[1] < 5.0f &&
            m->airdist[2] > 3.0f && m->airdist[2] < 5.0f) {
            temp += temp;
        }
        if (m->airdist[0] > 3.0f && m->airdist[0] < 5.0f &&
            m->airdist[3] > 3.0f && m->airdist[3] < 5.0f) {
            temp += 1.0f;
        }
    }
    if (temp > 0.0f && m->uvs[1][1] < -0.1f && m->magvel < 5.0f) {
        if (!m->crashflag) {
            AddBattlePoint(m->slot, m->slot, -1);
            m->crashflag = 1;
        }
    } else if (temp >= 2.0f && m->magvel < 40.0f) {
        if (!m->crashflag) {
            AddBattlePoint(m->slot, m->slot, -1);
            m->crashflag = 1;
        }
    } else if (temp >= 2.0f && m->uvs[1][1] < 0.707f) {
        if (m->magvel < 100.0f && !m->crashflag) {
            AddBattlePoint(m->slot, m->slot, -1);
            m->crashflag = 1;
        }
        m->topscrape = 1;
    }
}

/* 0x80011474, size 0x30 */
void mcommunication(tModelData *m)
{
    m->rpm = 0.9f * (9.549305f * m->engangvel);
}

