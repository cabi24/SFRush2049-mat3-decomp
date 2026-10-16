/* tires.c - Tire coordinate frames, suspension forces and friction-circle grip.
   Status: all four functions exact; privately verified native link and layout.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\tires.c; assembly: build/2049LE/asm/GameSource/src/tires.s */
#include "rush/rush_protos.h"
#define MSL_NO_SQRTF
#include "msl_math_inline.h"

/* Automatic constants avoid emitting unused per-translation-unit statics. */
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

/* 0x80050e70, size 0x19c */
void calctireuv(float *v, float *w, float *r, float steer, float (*caruvs)[3], float (*roaduvs)[3], float (*tireuvs)[3], float *tirev)
{
    float cartirev[3]; /* stack */
    cartirev[0] = r[1]*w[2] - r[2]*w[1];
    cartirev[1] = r[2]*w[0] - r[0]*w[2];
    cartirev[2] = r[0]*w[1] - r[1]*w[0];
    cartirev[0] += v[0];
    cartirev[1] += v[1];
    cartirev[2] += v[2];
    BodyVector(roaduvs[1], tireuvs[1], caruvs);
    tireuvs[0][0] = cos(steer);
    tireuvs[0][1] = 0.0f;
    tireuvs[0][2] = -(float)sin(steer);
    tireuvs[2][0] = tireuvs[0][1]*tireuvs[1][2] - tireuvs[0][2]*tireuvs[1][1];
    tireuvs[2][1] = tireuvs[0][2]*tireuvs[1][0] - tireuvs[0][0]*tireuvs[1][2];
    tireuvs[2][2] = tireuvs[0][0]*tireuvs[1][1] - tireuvs[0][1]*tireuvs[1][0];
    tireuvs[0][0] = tireuvs[1][1]*tireuvs[2][2] - tireuvs[1][2]*tireuvs[2][1];
    tireuvs[0][1] = tireuvs[1][2]*tireuvs[2][0] - tireuvs[1][0]*tireuvs[2][2];
    tireuvs[0][2] = tireuvs[1][0]*tireuvs[2][1] - tireuvs[1][1]*tireuvs[2][0];
    BodyVector(cartirev,tirev,tireuvs);
}

/* 0x8005100c, size 0x278 */
void dotireforce(tModelData *m, float *tirev, float (*tireuvs)[3], anon_tradius_springK_5c *tire, float torque, float *forcevec, float suscomp, float otsuscomp, float springrate, float arspringrate, float cdamping, float rdamping, int poortract)
{
    float normal; /* f1 */
    float damping; /* f6 */
    float sideforce; /* stack */
    float traction; /* stack */
    float arforce; /* f5 */
    float tireforcevec[3]; /* stack */
    if(suscomp>0.0f && otsuscomp>0.0f) arforce=arspringrate*(suscomp-otsuscomp);
    else arforce=0.0f;
    damping=tirev[1]<0.0f?cdamping:rdamping;
    if(suscomp>10.0f) {
        if(tirev[1]<1.0f) tireforcevec[1] = m->idt*(-0.25f*((1.0f-tirev[1])*m->mass));
        else tireforcevec[1]=suscomp*springrate+arforce-damping*tirev[1];
    } else if(suscomp>0.0f) tireforcevec[1]=suscomp*springrate+arforce-damping*tirev[1];
    else tireforcevec[1]=0.0f;
    tireforcevec[1] *= m->idt*(0.000047744f*m->idt-0.017552f);
    normal=tireforcevec[1];
    if(normal<0.0f) tireforcevec[1]=normal=0.0f;
    else if(normal>m->weight) normal=m->weight;
    frictioncircle(m,tirev,normal,torque,tire,&sideforce,&traction);
    if(tire==&m->tires[2] || tire==&m->tires[3]) {
        traction *= carEngineFwdForce[m->engine_type][m->handling];
        traction *= 1.0+(carTransTraction[m->trans][m->gear+1]-1.0f)*(1.0-fabs(m->mainin.wheel));
        traction *= m->tune->fwdfk;
        sideforce *= m->sidefk*(1.0f-0.1f*m->tune->sidefk);
        if(poortract) sideforce *= gSpecialTires==2?0.7f:0.1f*m->offroadtires+0.9f;
    }
    tireforcevec[2]=traction;
    tireforcevec[0]=sideforce;
    WorldVector(tireforcevec,forcevec,tireuvs);
    tire->sideforce=sideforce;
    tire->traction=traction;
}

/* 0x80051284, size 0xa88 */
void frictioncircle(tModelData *m, float *tirev, float normalforce, float torque, anon_tradius_springK_5c *tire, float *sfp, float *trp)
{
    float maxtraction; /* f1 */
    float maxtorque; /* f3 */
    float maxf; /* f29 */
    float Cfmax; /* f5 */
    float Afmax; /* f28 */
    float temp; /* f4 */
    float roadangvel; /* f4 */
    float ydot; /* f4 */
    float p; /* f8 */
    float k2; /* f27 */
    float k3; /* f26 */
    float patchvel; /* f6 */
    float realtorque; /* f31 */
    float alpha; /* f0 */
    realtorque = torque;
    temp = m->mass*tire->tradius;
    tire->slipflag=0;
    torque = (temp*torque)/(temp+1.0f/tire->invmi);
    if(normalforce<=0.0f) {
        *sfp=0.0f; *trp=0.0f; tire->slipflag+=3;
        tire->angvel+=m->dt*(realtorque*tire->invmi);
        if(m->brake>0.0f) { tire->angvel-=m->dt*(1000.0f*m->brake*tire->invmi); if(tire->angvel<0.0f)tire->angvel=0.0f; }
        return;
    }
    maxtraction=tire->cfmax*normalforce;
    maxtorque=maxtraction*tire->tradius;
    roadangvel=tirev[2]/tire->tradius;
    if(tire->angvel>roadangvel) {
        tire->sliptorque=maxtorque;
        tire->angvel+=m->dt*(tire->invmi*(realtorque-tire->sliptorque));
        if(tire->angvel<=roadangvel) {tire->slipflag=10;tire->angvel=roadangvel;}
        else {
            patchvel=tirev[2]-tire->angvel*tire->tradius;
            temp=sqrtf(patchvel*patchvel+tirev[0]*tirev[0]);
            if(temp==0.0f) {*trp=maxtraction;*sfp=0.0f;}
            else {*trp=-maxtraction*patchvel/temp;*sfp=-maxtraction*tirev[0]/temp;}
            tire->slipflag=20; return;
        }
    } else if(tire->angvel<roadangvel) {
        tire->sliptorque=-maxtorque;
        if(m->brake>0.8f && roadangvel>5.0f) tire->angvel=roadangvel-(roadangvel-tire->angvel)*fabs(m->steerangle);
        else tire->angvel+=m->dt*(tire->invmi*(realtorque-tire->sliptorque));
        if(tire->angvel>=roadangvel) {tire->angvel=roadangvel;tire->slipflag=30;}
        else {
            patchvel=tirev[2]-tire->angvel*tire->tradius;
            temp=sqrtf(patchvel*patchvel+tirev[0]*tirev[0]);
            if(temp==0.0f) {*trp=-maxtraction;*sfp=0.0f;}
            else {
                *trp=-maxtraction*patchvel/temp;
                if(m->magvel>20.0f) *sfp=-maxtraction*tirev[0]/(2.0f+sqrtf(tirev[2]*tirev[2]+tirev[0]*tirev[0]));
                else *sfp=-maxtraction*tirev[0]/temp;
            }
            tire->slipflag=40;return;
        }
    }
    tire->sliptorque=0.0f;
    if(torque!=0.0f) {
        if(torque>=maxtorque) {
            *trp=maxtraction;*sfp=0.0f;tire->slipflag+=4;tire->sliptorque=maxtorque;
            tire->angvel+=m->dt*(tire->invmi*(realtorque-tire->sliptorque));return;
        } else if(torque<=-maxtorque) {
            *trp=-maxtraction;*sfp=0.0f;tire->slipflag+=5;tire->sliptorque=-maxtorque;
            tire->angvel+=m->dt*(tire->invmi*(realtorque-tire->sliptorque));return;
        }
        *trp=torque/tire->tradius;
        temp=*trp/normalforce;
        Cfmax=sqrtf(tire->cfmax*tire->cfmax-temp*temp);
        k2=(tire->k1*tire->k1)/(3.0f*Cfmax);
        k3=(tire->k1*(tire->k1*tire->k1))/(27.0f*Cfmax*Cfmax);
        Afmax=(3.0f*Cfmax)/tire->k1;
    } else {
        *trp=0.0f;k2=tire->k2;k3=tire->k3;Cfmax=tire->cfmax;Afmax=tire->afmax;
    }
    maxf=Cfmax*normalforce;
    alpha=calcalpha(tirev);
    tire->sliptorque=0.0f;
    if(alpha>=0.0f) {
        if(alpha>=Afmax) {
            ydot=-tirev[0]; tire->patchy+=ydot*m->dt;
            *sfp=tire->springK*tire->patchy+tire->rubdamp*ydot;
            if(*sfp<-(double)maxf) {*sfp=-maxf;tire->patchy=*sfp/tire->springK;}
            tire->slipflag+=6;
            tire->sliptorque=*trp*tire->tradius;
            tire->angvel+=m->dt*(tire->invmi*(realtorque-tire->sliptorque));return;
        }
        tire->slipflag+=7;
        p=tirev[2]*tire->springK/(normalforce*(tire->k1-k2*alpha+alpha*(k3*alpha)));
        if(p<0.0f)p=-p;
        if(p*m->dt<0.5f) {
            ydot=-p*tire->patchy-tirev[0];tire->patchy+=ydot*m->dt;
            *sfp=tire->springK*tire->patchy+tire->rubdamp*ydot;tire->slipflag+=100;
        } else {
            temp=tire->k1*alpha-alpha*(k2*alpha);
            temp=-(alpha*(alpha*(k3*alpha))+temp);
            *sfp=normalforce*temp;
            tire->patchy=*sfp/tire->springK;tire->slipflag+=200;
        }
    } else {
        if(alpha<=-Afmax) {
            ydot=-tirev[0];tire->patchy+=ydot*m->dt;
            *sfp=tire->springK*tire->patchy+tire->rubdamp*ydot;
            if(*sfp>maxf){*sfp=maxf;tire->patchy=*sfp/tire->springK;}
            tire->slipflag+=8;tire->sliptorque=*trp*tire->tradius;
            tire->angvel+=m->dt*(tire->invmi*(realtorque-tire->sliptorque));return;
        }
        tire->slipflag+=9;
        p=tirev[2]*tire->springK/(normalforce*(tire->k1+k2*alpha+alpha*(k3*alpha)));
        if(p<0.0f)p=-p;
        if(p*m->dt<0.5f) {
            ydot=-p*tire->patchy-tirev[0];tire->patchy+=ydot*m->dt;
            *sfp=tire->springK*tire->patchy+tire->rubdamp*ydot;tire->slipflag+=100;
        } else {
            temp=tire->k1*alpha+alpha*(k2*alpha);
            temp=-(alpha*(alpha*(k3*alpha))+temp);
            *sfp=normalforce*temp;
            tire->patchy=*sfp/tire->springK;tire->slipflag+=200;
        }
    }
}

/* 0x80051d0c, size 0x5c */
float calcalpha(float *tirev)
{
    float alpha; /* f1 */
    alpha=tirev[0];
    if(tirev[2]>0.0f) {
        if(tirev[2]<400.0f) alpha=alpha/tirev[2];
        else alpha=0.0025f*alpha;
    } else if(tirev[2]<0.0f) {
        if(tirev[2]>-400.0f) alpha=-alpha/tirev[2];
        else alpha=0.0025f*alpha;
    }
    return alpha;
}

