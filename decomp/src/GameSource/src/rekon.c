/* rekon.c - dead reckoning: between physics updates, cars are drawn from a snapshot extrapolated
   with their velocities. */
#include "rush/rush_protos.h"

inline void VecScale(float *d, float *v, float k)
{
    d[0] = v[0] * k;
    d[1] = v[1] * k;
    d[2] = v[2] * k;
}

inline void VecAdd(float *d, float *a, float *b)
{
    d[0] = a[0] + b[0];
    d[1] = a[1] + b[1];
    d[2] = a[2] + b[2];
}

/* 0x80033cbc */
void dead_reckon_all(void)
{
    int i;

    for (i = 0; i < 6; i++) {
        if (model[i].in_game && gGameCar[i].dead < 2)
            dead_reckon_posuv(&model[i]);
    }
}

/* 0x80033d3c */
void game_reckon_all(float time)
{
    int i;
    float deltime;
    float temp[3];

    for (i = 0; i < 6; i++) {
        if (model[i].in_game && gGameCar[i].dead < 2) {
            deltime = model[i].time_fudge * (time - model[i].fastin.modelTime);
            VecScale(temp, model[i].rwv, deltime);
            VecAdd(model[i].reckon.rwr, model[i].rwr, temp);
            CopyUV(model[i].uvs, model[i].reckon.uvs);
            VecScale(temp, model[i].w, deltime);
            RotateUV(temp, model[i].reckon.uvs);
        }
    }
}

/* 0x80033e6c */
void dead_reckon_posuv(tModelData *m)
{
    CopyUV(m->uvs, m->reckon.uvs);
    m->reckon.rwmph = 2.7272727f * (0.5f * (m->tires[2].angvel * m->tires[2].tradius
                                            + m->tires[3].angvel * m->tires[3].tradius));
    m->reckon.suscomp[0] = m->suscomp[0];
    m->reckon.airdist[0] = m->airdist[0];
    m->reckon.suscomp[1] = m->suscomp[1];
    m->reckon.airdist[1] = m->airdist[1];
    m->reckon.suscomp[2] = m->suscomp[2];
    m->reckon.airdist[2] = m->airdist[2];
    m->reckon.suscomp[3] = m->suscomp[3];
    m->reckon.airdist[3] = m->airdist[3];
    m->reckon.rwa[0] = m->reckon.base_rwa[0];
    m->reckon.rwa[1] = m->reckon.base_rwa[1];
    m->reckon.rwa[2] = m->reckon.base_rwa[2];
    m->reckon.rwv[0] = m->reckon.base_rwv[0];
    m->reckon.rwv[1] = m->reckon.base_rwv[1];
    m->reckon.rwv[2] = m->reckon.base_rwv[2];
    m->reckon.rwr[0] = m->reckon.base_rwr[0];
    m->reckon.rwr[1] = m->reckon.base_rwr[1];
    m->reckon.rwr[2] = m->reckon.base_rwr[2];
}

/* 0x80033f60 */
void update_reckon_base(int slot)
{
    tModelData *m = &model[slot];

    m->reckon.base_rwa[0] = m->rwa[0];
    m->reckon.base_rwa[1] = m->rwa[1];
    m->reckon.base_rwa[2] = m->rwa[2];
    m->reckon.base_rwv[0] = m->rwv[0] * m->time_fudge;
    m->reckon.base_rwv[1] = m->rwv[1] * m->time_fudge;
    m->reckon.base_rwv[2] = m->rwv[2] * m->time_fudge;
    m->reckon.base_rwr[0] = m->rwr[0];
    m->reckon.base_rwr[1] = m->rwr[1];
    m->reckon.base_rwr[2] = m->rwr[2];
}
