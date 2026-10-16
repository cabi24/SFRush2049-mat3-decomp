/* initiali.c - car and tire initialization. Status: matched.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\initiali.c; assembly: build/2049LE/asm/GameSource/src/initiali.s */
#include "rush/rush_protos.h"
#include "msl_math_inline.h"

/* Separate argument temporaries preserve the original floating-point allocation. */
static inline float _MassScale(float mass, float scale)
{
    return mass * scale;
}


/* 0x800249f8, size 0x4e4 */
void carsetup(tModelData *m)
{
    anon_colfront_colrear_10 *sizep; /* r3 */
    int i; /* r25 */
    int p; /* r7 */
    float delta; /* f3 */
    _tCar *carp; /* r31 */
    p = m->drone_type == 2 ? m->slot + 1 : 0;
    carp = gCarModels[m->body_type];
    m->car = carp;
    m->tune = &gTune[m->handling];
    m->wingEffect = gWingSizes[gCarWings[p][m->body_type]];
    m->mass = CalcMass(m);
    delta = carDurability[m->frame] - 0.4f;
    m->collisionmass = 1000.0f * delta + gCollisionMass[m->body_type];
    m->crashthreshold = 100000.0f * delta + gCrashThreshold[m->body_type];
    m->weight = m->mass * gGravity;
    m->magicdiff = gCarDifferential[m->body_type] & 1;
    m->nothrusttorque = gCarDifferential[m->body_type] & 2;
    m->torquecurve = &stdtorquecurve[0][0];
    m->offroadtires = CalcOffroadTires(m);
    m->drift = CalcDrift(m);
    m->sidefk = CalcTireSideForce(m);
    m->wheelbase = carp->tireR[0][2] - carp->tireR[2][2];
    sizep = &collsize[m->body_type];
    m->bodyR[0][0] = m->bodyR[2][0] = sizep->colside;
    m->bodyR[1][0] = m->bodyR[3][0] = -sizep->colside;
    m->bodyR[0][1] = m->bodyR[1][1] = sizep->colheight;
    m->bodyR[2][1] = m->bodyR[3][1] = sizep->colheight;
    m->bodyR[0][2] = m->bodyR[1][2] = sizep->colfront;
    m->bodyR[2][2] = m->bodyR[3][2] = -sizep->colrear;
    delta = sizep->colfront > sizep->colrear ? sizep->colfront : sizep->colrear;
    m->colrad = sqrtf(delta * delta + sizep->colside * sizep->colside + sizep->colheight * sizep->colheight);
    for (i = 0; i < 4; i++) {
        copy_tire_info(carp->tires[i], &m->tires[i]);
        if (i < 2) tire_constants(m, &m->tires[i], -carp->tireR[i + 2][2]);
        else tire_constants(m, &m->tires[i], carp->tireR[i - 2][2]);
        m->tires[i].angvel = m->v[2] / m->tires[i].tradius;
        m->suscomp[i] = 0.0f;
        m->tireV[i][0] = 0.0f;
        m->tireV[i][1] = 0.0f;
        m->tireV[i][2] = 0.0f;
    }
    m->brakegain[0] = m->weight * m->tires[0].cfmax * gCarBraking[m->body_type];
    m->brakegain[1] = m->weight * m->tires[1].cfmax * gCarBraking[m->body_type];
    m->brakegain[2] = m->weight * m->tires[2].cfmax * gCarBraking[m->body_type];
    m->brakegain[3] = m->weight * m->tires[3].cfmax * gCarBraking[m->body_type];
    m->dwinvmi = m->tires[2].invmi / 2.0f;
    steer_constants(m);
    m->grw[2] = 0.0f;
    m->grw[0] = 0.0f;
    m->grw[1] = -m->weight;
    m->mainin.gear = 0;
    m->commandgear = 0;
    m->wing = 0;
    m->idt = 0.0f;
    m->d[0] = 0.0f;
    m->d[1] = 0.0f;
    m->d[2] = 0.0f;
    m->g[0] = 0.0f;
    m->g[1] = 0.0f;
    m->g[2] = 0.0f;
    m->centerMoment[0] = 0.0f;
    m->centerMoment[1] = 0.0f;
    m->centerMoment[2] = 0.0f;
    m->tireForce[0][0] = 0.0f;
    m->tireForce[0][1] = 0.0f;
    m->tireForce[0][2] = 0.0f;
    m->tireForce[1][0] = 0.0f;
    m->tireForce[1][1] = 0.0f;
    m->tireForce[1][2] = 0.0f;
    m->tireForce[2][0] = 0.0f;
    m->tireForce[2][1] = 0.0f;
    m->tireForce[2][2] = 0.0f;
    m->tireForce[3][0] = 0.0f;
    m->tireForce[3][1] = 0.0f;
    m->tireForce[3][2] = 0.0f;
    m->brake = 0.0f;
    m->engtorque = 0.0f;
    m->clutchtorque = 0.0f;
    m->clutchangvel = 0.0f;
    m->transratio = 0.0f;
    m->totalratio = 0.0f;
    m->dwtorque = 0.0f;
    m->efdwinvmi = 0.0f;
    m->dwangvel = 0.0f;

}

/* 0x80024edc, size 0x60 */
float CalcDrift(tModelData *m)
{
    float drift; /* f1 */
    int susp; /* r8 */
    drift = gTireDrift[m->tire_type][m->handling];
    susp = gCarSuspensions[m->body_type];
    if (susp == 0) drift += 0.05f;
    else if (susp == 2) drift -= 0.05f;
    return drift;
}

/* 0x80024f3c, size 0x1c */
float CalcTireSideForce(tModelData *m)
{
    return gTireSideForce[m->tire_type];
}

/* 0x80024f58, size 0x50 */
float CalcOffroadTires(tModelData *m)
{
    if (m->tire_type == 4) return 0.75f;
    if (m->tire_type == 3) return 0.5f;
    if (m->tire_type == 1) return -0.25f;
    if (m->tire_type == 2) return -0.5f;
    return 0.0f;
}

/* 0x80024fa8, size 0x90 */
float CalcMass(tModelData *m)
{
    return _MassScale((31.0559f * (carDurability[m->frame] - 0.4f) + gCarMass[m->body_type] + 31.0559f * gMassiveMass), gTune[m->handling].weight_scale);
}

/* 0x80025038, size 0x18c */
void initialize(tModelData *m)
{
    m->pitchangle = 0.0f;
    m->torque[1] = 0.0f;
    m->torque[0] = 0.0f;
    m->torque[2] = 50.0f;
    m->torque[3] = 50.0f;
    m->dt = 0.0025f;
    m->brakeroll = 3.0f;
    m->shifttime = 0.0f;
    m->thumpthreshold = 7500.0f;
    m->thumpflag = 0;
    m->f[0] = 0.0f;
    m->f[1] = 0.0f;
    m->f[2] = 0.0f;
    m->m[0] = 0.0f;
    m->m[1] = 0.0f;
    m->m[2] = 0.0f;
    m->a[0] = 0.0f;
    m->a[1] = 0.0f;
    m->a[2] = 0.0f;
    m->aa[0] = 0.0f;
    m->aa[1] = 0.0f;
    m->aa[2] = 0.0f;
    m->w[0] = 0.0f;
    m->w[1] = 0.0f;
    m->w[2] = 0.0f;
    m->v[0] = 0.0f;
    m->v[1] = 0.0f;
    m->v[2] = 0.0f;
    m->engangvel = 0.0f;
    CopyUV(unitmatrix, m->uvs);
    if (gSuperSpeed) m->uvs[2][2] = m->uvs[1][1] = m->uvs[0][0] = 1.00025f;
    carsetup(m);
    init_road(m);
    init_collision(m);
    m->crashflag = 0;
    gGameCar[m->slot].battle.health = 800;
    gGameCar[m->slot].battle.shoot = 0;
    gGameCar[m->slot].battle.fadeState = 0;
    gGameCar[m->slot].battle.defense.active = 0;
    gGameCar[m->slot].battle.pitch = 0.0f;
    gGameCar[m->slot].battle.yaw = 0.0f;
    gGameCar[m->slot].crash_state = 0;
    m->bodyForces = 0.0f;
    m->bogState = 3;
}

/* 0x800251c4, size 0x4c */
void steer_constants(tModelData *m)
{
    m->steergain = 0.017453292f * (540.0f / m->car->steerratio);
    m->trail = (m->car->swtpg * m->car->steerratio) / (m->weight * (m->car->tireR[2][2] / m->wheelbase));
}

/* 0x80025210, size 0x34 */
void copy_tire_info(const anon_tradius_springK_18 *tp1, anon_tradius_springK_5c *tp2)
{
    tp2->tradius = tp1->tradius;
    tp2->springK = tp1->springK;
    tp2->rubdamp = tp1->rubdamp;
    tp2->cstiff = tp1->cstiff;
    tp2->cfmax = tp1->cfmax;
    tp2->invmi = tp1->invmi;
}

/* 0x80025244, size 0xf4 */
void tire_constants(tModelData *m, anon_tradius_springK_5c *tdes, float otw)
{
    tdes->zforce = 0.5f * (m->weight * otw / m->wheelbase);
    tdes->afmax = 3.0f * tdes->cfmax * tdes->zforce / tdes->cstiff;
    tdes->k1 = tdes->cstiff / tdes->zforce;
    tdes->k2 = tdes->k1 * tdes->k1 / (3.0f * tdes->cfmax);
    tdes->k3 = (tdes->k1 * tdes->k1 * tdes->k1) / (27.0f * tdes->cfmax * tdes->cfmax);
    tdes->l2 = 2.0f * tdes->k2;
    tdes->l3 = 3.0f * tdes->k3;
    tdes->m1 = 4.16f / tdes->cfmax;
    tdes->m2 = 0.29411766f * (tdes->m1 * tdes->m1);
    tdes->m3 = 0.021598272f * (tdes->m1 * tdes->m1 * tdes->m1);
    tdes->m4 = 18181.818f;
    tdes->patchy = 0.0f;
}

