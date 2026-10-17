/* drones.c - AI "drone" cars. */
#include "rush/rush_protos.h"
#define MSL_NO_SQRTF
#include "msl_math_inline.h"

float old_drone_scale[6];
float old_boost[6];

static void _AssignDrones(void);
static float _LinearInterp(float in_bound1, float in_bound2, float out_bound1, float out_bound2, float input);

/* 0x800114A4, lines 56-60 */
void InitDrones(void)
{
    int i;

    for (i = 0; i < 6; i++) {
        old_drone_scale[i] = 1.0f;
        old_boost[i] = 1.0f;
    }
}

/* 0x800114E4, lines 73-78 */
void DoDrones(void)
{
    SetCatchup();
    _AssignDrones();
}

/* 0x80011508, lines 91-499 */
static void _AssignDrones(void)
{
    signed short i; /* r11 */
    signed short k; /* r9 */
    signed short j; /* r12 */
    signed short indx; /* r24 */
    signed short index; /* r24 */
    signed short index2; /* r30 */
    signed short index3; /* r9 */
    signed short humans[6]; /* stack */
    signed short drones[6]; /* stack */
    signed short car_in_conflict[6]; /* stack */
    signed short cars_in_order[6]; /* stack */
    signed short my_place; /* r25 */
    signed short places[6]; /* stack */
    signed short num_humans; /* r10 */
    signed short num_drones; /* r11 */
    float scale; /* f2 */
    float delta_dist; /* f0 */
    float target_dist; /* f6 */
    float delta[3]; /* stack */
    float time_scale; /* f1 */
    float delta_dist2; /* f7 */
    float car_distance[6][6]; /* stack */
    float max_brake; /* f0 */
    float min_brake; /* f31 */
    float max_boost; /* f4 */
    float min_boost; /* f30 */
    float temp1; /* f3 */
    tCarData *gc; /* r12 */
    float place_scale; /* f0 */
    float diff_scale; /* f9 */

    for (i = 0; i < gNumActiveCars; i++)
        places[model[i].slot] = gGameCar[model[i].slot].place;

    num_drones = 0;
    num_humans = 0;
    for (k = 0; k < gNumActiveCars; k++) {
        indx = 0;
        for (j = 0; j < gNumActiveCars; j++) {
            indx = model[j].slot;
            if (k == places[indx])
                break;
        }
        cars_in_order[k] = indx;
        if (model[indx].drone_type == 2)
            humans[num_humans++] = indx;
        else
            drones[num_drones++] = indx;
    }
    for (i = 0; i < num_drones; i++)
        gGameCar[drones[i]].weight_index = i;

    if (gGameState & 8) {
        for (i = 0; i < gNumActiveCars; i++) {
            index = model[i].slot;
            if (model[index].drone_type == 1)
                model[index].drone_scale = 1.0f;
        }
        return;
    }
    if (num_drones == 0)
        return;
    if (gModelTime < 5.0f) {
        for (i = 0; i < num_drones; i++) {
            indx = num_drones - i - 1;
            model[drones[i]].time_boost = .98f + .02f * indx;
            model[drones[i]].drone_scale = 1.0f;
        }
        return;
    }

    for (i = 0; i < gNumActiveCars; i++) {
        index = model[i].slot;
        car_in_conflict[index] = 0;
        gc = &gGameCar[index];
        for (j = i; j < gNumActiveCars; j++) {
            index2 = model[j].slot;
            if (index == index2) {
                car_distance[index][index2] = 100000000.0f;
            } else {
                for (k = 0; k < 3; k++)
                    delta[k] = gc->dr_pos[k] - gGameCar[index2].dr_pos[k];
                car_distance[index][index2] = car_distance[index2][index] =
                    delta[2] * delta[2] + (delta[0] * delta[0] + delta[1] * delta[1]);
            }
        }
    }
    for (i = 0; i < num_drones; i++)
        model[drones[i]].drone_target = drones[i];

    for (i = 0; i < num_humans; i++) {
        for (j = i + 1; j < num_humans; j++) {
            index = humans[i];
            index2 = humans[j];
            if (car_distance[index][index2] < 4900.0f) {
                car_in_conflict[index] += 10;
                car_in_conflict[index2] += 10;
            }
        }
    }
    for (i = 0; i < num_humans; i++) {
        if (car_in_conflict[humans[i]] < 10) {
            index = humans[i];
            my_place = places[index];
            index3 = -1;
            index2 = -1;
            if (my_place > 0) {
                indx = cars_in_order[my_place - 1];
                if (model[indx].drone_type == 1 && car_in_conflict[indx] == 0)
                    index2 = indx;
            }
            if (my_place < gNumActiveCars - 1) {
                indx = cars_in_order[my_place + 1];
                if (model[indx].drone_type == 1 && car_in_conflict[indx] == 0)
                    index3 = indx;
            }
            if (index2 != -1 && index3 != -1) {
                if (car_distance[index3][index] < car_distance[index2][index])
                    index2 = index3;
            } else if (index2 == -1) {
                index2 = index3;
            }
            if (index2 != -1) {
                car_in_conflict[index]++;
                car_in_conflict[index2]++;
                model[index2].drone_target = index;
            }
        }
    }

    {
    short i;
    for (i = 0; i < gNumActiveCars; i++) {
        index = model[i].slot;
        if (model[index].we_control) {
            my_place = places[index];
            index2 = model[index].drone_target;
            if (gDifficulty == 5)
                place_scale = 0.0f;
            else if (my_place == 0)
                place_scale = .5f / (gNumActiveCars - 1);
            else
                place_scale = (float)my_place / (gNumActiveCars - 1);
            diff_scale = 1.0f - gDifficulty / 5.0f;
            if (gRace == kCircuit)
                diff_scale *= .75f + gCircuitCars[model[index].slot].skill[gTrackNum + (gBackwards ? 6 : 0)] / 8.0f;

            max_boost = 1.1f + (1.0f - 1.1f) * diff_scale
                - place_scale * (.05f + (.15f - .05f) * diff_scale);
            min_boost = 1.0f - place_scale * (.02f + .04f * diff_scale);
            max_brake = .9f - place_scale * (.1f + .5f * diff_scale);
            min_brake = (1.0f + (.96f - 1.0f) * diff_scale)
                - place_scale * (.02f + (.05f - .02f) * diff_scale);

            if (index2 != index) {
                delta_dist = gGameCar[index].distance - gGameCar[index2].distance;
                if (delta_dist < -20.0f) {
                    time_scale = _LinearInterp(-300.0f, -60.0f, max_boost, min_boost, delta_dist);
                    scale = min_brake;
                } else if (car_in_conflict[index2] <= 1) {
                    scale = _LinearInterp(200.0f, 60.0f, max_brake, min_brake, delta_dist);
                    time_scale = min_boost;
                } else {
                    scale = min_brake;
                    time_scale = min_boost;
                }
            } else if (my_place == 0) {
                delta_dist = gGameCar[index].distance - gGameCar[cars_in_order[1]].distance;
                if (delta_dist < 200.0f) {
                    time_scale = _LinearInterp(200.0f, 0.0f, min_boost, max_boost, delta_dist);
                    scale = min_brake;
                } else {
                    scale = _LinearInterp(500.0f, 200.0f, max_brake, min_brake, delta_dist);
                    time_scale = min_boost;
                }
            } else if (my_place == gNumActiveCars - 1) {
                delta_dist = gGameCar[index].distance - gGameCar[cars_in_order[gNumActiveCars - 2]].distance;
                if (delta_dist < -150.0f) {
                    time_scale = _LinearInterp(-150.0f, -300.0f, min_boost, max_boost, delta_dist);
                    scale = min_brake;
                } else {
                    scale = _LinearInterp(-150.0f, 0.0f, min_brake, max_brake, delta_dist);
                    time_scale = min_boost;
                }
            } else {
                indx = cars_in_order[my_place - 1];
                delta_dist = gGameCar[index].distance - gGameCar[indx].distance;
                indx = cars_in_order[my_place + 1];
                delta_dist2 = gGameCar[index].distance - gGameCar[indx].distance;
                if (delta_dist > -150.0f && delta_dist2 < 150.0f) {
                    target_dist = (delta_dist + delta_dist2) / 2.0f;
                    if (target_dist < 0.0f) {
                        time_scale = _LinearInterp(delta_dist - target_dist, -200.0f, min_boost, max_boost, delta_dist);
                        scale = min_brake;
                    } else {
                        scale = _LinearInterp(delta_dist2 - target_dist, 200.0f, min_brake, max_brake, delta_dist2);
                        time_scale = min_boost;
                    }
                } else {
                    for (j = 0, k = 0; j < my_place; j++)
                        if (model[cars_in_order[j]].drone_type == 2)
                            k++;
                    index3 = 0;
                    for (j = my_place; j < gNumActiveCars; j++)
                        if (model[cars_in_order[j]].drone_type == 2)
                            index3++;
                    if (k >= index3) {
                        if (delta_dist < -150.0f) {
                            time_scale = _LinearInterp(-150.0f, -300.0f, min_boost, max_boost, delta_dist);
                            scale = min_brake;
                        } else {
                            scale = _LinearInterp(-150.0f, 0.0f, min_brake, max_brake, delta_dist);
                            time_scale = min_boost;
                        }
                    } else {
                        if (delta_dist2 < 150.0f) {
                            time_scale = _LinearInterp(150.0f, 0.0f, min_boost, max_boost, delta_dist2);
                            scale = min_brake;
                        } else {
                            scale = _LinearInterp(500.0f, 150.0f, max_brake, min_brake, delta_dist2);
                            time_scale = min_boost;
                        }
                    }
                }
            }
            if (gCoastFlag || gEndGame)
                time_scale = min_boost;
            if (gOneMoreCP && gGameCar[humans[num_humans - 1]].place > my_place) {
                scale = min_brake;
                time_scale = min_boost;
            }

            temp1 = model[index].drone_scale;
            if (temp1 > scale) {
                temp1 -= .01f;
                if (temp1 < scale)
                    temp1 = scale;
            } else {
                temp1 += .01f;
                if (temp1 > scale)
                    temp1 = scale;
            }
            model[index].drone_scale = temp1;
            temp1 = model[index].time_boost;
            if (temp1 > time_scale) {
                temp1 -= .01f;
                if (temp1 < time_scale)
                    temp1 = time_scale;
            } else {
                temp1 += .01f;
                if (temp1 > time_scale)
                    temp1 = time_scale;
            }
            model[index].time_boost = temp1;
        }
    }
    }
}

/* 0x8001269C, lines 522-544 */
static float _LinearInterp(float in_bound1, float in_bound2, float out_bound1, float out_bound2, float input)
{
    float slope;

    if (in_bound1 < in_bound2) {
        if (input < in_bound1)
            return out_bound1;
        if (input > in_bound2)
            return out_bound2;
    } else {
        if (input < in_bound2)
            return out_bound2;
        if (input > in_bound1)
            return out_bound1;
    }

    if (fabs(in_bound1 - in_bound2) < 0.001f)
        return (out_bound1 + out_bound2) * 0.5f;

    slope = (out_bound1 - out_bound2) / (in_bound1 - in_bound2);
    return slope * input + (out_bound1 - slope * in_bound1);
}
