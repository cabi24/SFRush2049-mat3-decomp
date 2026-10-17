/* drivetra.c - drivetrain: engine torque curves, clutch, gearbox and automatic shifting.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\drivetra.c; assembly: build/2049LE/asm/GameSource/src/drivetra.s */
#include "rush/rush_protos.h"

/* 0x8000ebb0, size 0x150 */
void drivetrain(tModelData *m)
{
    float rearload; /* f4 */

    if (m->autotrans) {
        autoshift(m);
    }
    engine(m);
    transmission(m);
    m->dwangvel = 0.5f * (m->tires[2].angvel + m->tires[3].angvel);
    whatslips(m);
    rearload = m->tireForce[2][1] + m->tireForce[3][1];
    if (!m->magicdiff || rearload < 500.0f) {
        m->torque[2] += 0.5f * m->dwtorque;
        m->torque[3] += 0.5f * m->dwtorque;
    } else if (m->tireForce[2][1] <= 0.0f) {
        m->torque[2] = 0.0f;
        m->torque[3] += m->dwtorque;
    } else if (m->tireForce[3][1] <= 0.0f) {
        m->torque[2] += m->dwtorque;
        m->torque[3] = 0.0f;
    } else {
        m->torque[2] += m->dwtorque * m->tireForce[2][1] / rearload;
        m->torque[3] += m->dwtorque * m->tireForce[3][1] / rearload;
    }
    m->tires[2].invmi = 2.0f * m->efdwinvmi;
    m->tires[3].invmi = 2.0f * m->efdwinvmi;
}

/* 0x8000ed00, size 0x174 */
void whatslips(tModelData *m)
{
    float totratsq; /* f4 */
    float curclmaxt; /* f2 */
    float dv; /* f1 */
    int foo; /* r5 */

    totratsq = m->totalratio * m->totalratio;
    if (m->engangvel < 104.719666f) {
        m->engangvel = 104.719666f;
    }
    if (m->clutch > 0.8f) {
        curclmaxt = 0.0f;
    } else {
        curclmaxt = 1.25f * (0.8f - m->clutch) * m->car->clutchmaxt;
    }
    if (m->gear == 0 || curclmaxt == 0.0f) {
        m->clutchtorque = 0.0f;
        m->dwtorque = 0.0f;
        m->clutchangvel = m->engangvel;
        m->engangvel += m->dt * (m->engtorque * m->car->enginvmi);
        m->efdwinvmi = m->dwinvmi;
        return;
    }
    curclmaxt *= m->tune->clutchmaxtk;
    m->clutchangvel = m->dwangvel * m->totalratio;
    foo = m->engangvel > m->clutchangvel;
    m->clutchtorque = foo ? curclmaxt : -curclmaxt;
    dv = m->dt * (m->car->enginvmi * (m->engtorque - m->clutchtorque));
    if (foo ^ (m->engangvel + dv > m->clutchangvel)) {
        m->engangvel = m->clutchangvel;
        m->clutchtorque = m->engtorque;
    } else {
        m->engangvel += dv;
    }
    m->efdwinvmi = 1.0f / (1.0f / m->dwinvmi + totratsq / m->car->enginvmi);
    m->dwtorque = m->clutchtorque * m->totalratio;
}

/* 0x8000ee74, size 0xe8 */
void autoshift(tModelData *m)
{
    float modupshiftangvel; /* f31 */
    float moddownshiftangvel; /* f30 */
    float fact; /* f2 */

    fact = 0.25f * (3.0f + m->throttle);
    modupshiftangvel = fact * (m->tune->shiftk * m->car->upshiftangvel);
    moddownshiftangvel = fact * (m->tune->shiftk * m->car->downshiftangvel);
    if (m->commandgear == 0 || m->commandgear == -1) {
        m->gear = m->commandgear;
        return;
    }
    if (m->gear == 0 || m->gear == -1) {
        find_best_gear(m, modupshiftangvel, moddownshiftangvel);
    }
    if (m->engangvel > modupshiftangvel) {
        upshift(m);
    }
    if (m->engangvel < moddownshiftangvel) {
        downshift(m);
    }
}

/* 0x8000ef5c, size 0x50 */
void transmission(tModelData *m)
{
    m->transratio = m->tune->trans_scale * carTransGearRatio[m->trans][m->gear + 1];
    m->totalratio = m->transratio * m->car->dwratio;
}

/* 0x8000efac, size 0x74 */
/* callers pass the downshift speed too (see overrides.json); it is unused */
void find_best_gear(tModelData *m, float usang, float dsang)
{
    for (m->gear = 1; m->gear < 4; m->gear++) {
        if (m->dwangvel * (m->tune->trans_scale * m->car->dwratio * carTransGearRatio[m->trans][m->gear + 1]) < usang) {
            return;
        }
    }
}

/* 0x8000f020, size 0x38 */
void upshift(tModelData *m)
{
    if (m->gear >= 4) {
        return;
    }
    if (m->fastin.modelTime > m->shifttime) {
        m->shifttime = 1.0f + m->fastin.modelTime;
        m->gear++;
    }
}

/* 0x8000f058, size 0x38 */
void downshift(tModelData *m)
{
    if (m->gear <= 1) {
        return;
    }
    if (m->fastin.modelTime > m->shifttime) {
        m->shifttime = 1.0f + m->fastin.modelTime;
        m->gear--;
    }
}

/* 0x8000f090, size 0x1a4 */
signed short enginetorque(tModelData *m, signed short rpm, signed short throttle, const signed short *torquecurve)
{
    int promoted[2];
    signed short rindex; /* r7 */
    signed short tindex; /* r3 */
    signed short rrem; /* r8 */
    signed short trem; /* r26 */
    signed short left; /* r0 */
    signed short right; /* r0 */
    signed short *low_ptr; /* r0 */

    rindex = rpm / 1150;
    rrem = rpm % 1150;
    if (rindex < 0) {
        rindex = 0;
        rrem = 0;
    }
    if (rindex >= 11) {
        rindex = 10;
        rrem = 1149;
    }
    tindex = throttle / 14;
    trem = throttle % 14;
    if (tindex >= 9) {
        tindex = 8;
        trem = 13;
    }
    promoted[0] = rindex;
    promoted[1] = rrem;
    low_ptr = (signed short *)(torquecurve + tindex * 12 + promoted[0]);
    left = interp(low_ptr[0], low_ptr[1], promoted[1], 1149);
    low_ptr = (signed short *)(torquecurve + (tindex + 1) * 12 + promoted[0]);
    right = interp(low_ptr[0], low_ptr[1], promoted[1], 1149);
    left = interp(left, right, trem, 14);
    left *= m->tune->torque_scale * carEngineTorque[m->engine_type][m->handling];
    return left;
}

/* 0x8000f234, size 0x18 */
signed short interp(int a, int b, int rem, int total)
{
    return a + rem * (b - a) / total;
}

/* 0x8000f24c, size 0x1a8 */
void engine(tModelData *m)
{
    if (m->crashflag) {
        m->engtorque = enginetorque(m, 9.549305f * m->engangvel, 0, m->torquecurve);
    } else if (m->gear < 0) {
        m->engtorque = enginetorque(m, 9.549305f * m->engangvel, 128.0f * m->throttle, m->torquecurve);
    } else {
        m->engtorque = enginetorque(m, 9.549305f * m->engangvel, 128.0f * m->throttle, m->torquecurve);
    }
    if ((m->roadcode[2] == 1 || m->roadcode[2] == 2) && (m->roadcode[3] == 1 || m->roadcode[3] == 2)) {
        m->engtorque *= 1.5f;
    }
    if (m->autotrans && m->gear != 4) {
        m->engtorque *= 0.925f;
    }
}

