/* mdrive.c - the per-frame car simulation driver ("model drive"). Status: Matching.
 *
 * Owns the six car slots' simulation state (model[], tModelData) and publishes a render-side copy
 * of it to gGameCar[] (tCarData) every frame.
 *
 * Frame flow:
 *   model_iteration()   from model.c ModelThread, which despite the (arcade-era) name runs inline:
 *                       UpdateRushControls -> UpdateControls -> ModelThread, once per main-loop
 *                       pass, while counting down, racing or in the demo; each call advances
 *                       gModelTime by one gRetraceTime:
 *     _UpdatePlayerModel   every human car, every tick (ghost races may replay two ticks)
 *     _UpdateDroneModels   only some AI cars per tick, round-robin (drone_sched below)
 *     _UpdateModel(slot)   one car: crash/respawn bookkeeping, sym() physics, rumble, effects
 *   Update_MDrive()     once per game frame (game.c, demo.c): update_game_data() copies model -> gGameCar
 *                       and works out the visual-effect bits (appearance, rush_flags.h)
 *
 * Set-up: init_cars() at race start, Init_MDrive()/End_MDrive() per car around each (re)spawn,
 * modelgo()/modelstop() for pause.
 *
 * PORT: the simulation steps a fixed gRetraceTime per call (1/60 s NTSC, 1/50 s PAL, Init.c), and
 *   the dt clamp in _UpdateModel assumes it. Keep model_iteration on a fixed-rate tick, decoupled
 *   from the renderer, so a faster display does not speed up the cars.
 * PORT: PS_SetRumblePercent/PS_RumbleGo/PS_RumbleStop are the controller layer (NGC/ngcPad.c).
 * PORT: `**player->save` and gGhosts[] are double-indirect memory handles (memsys.c).
 * PORT: two call sites cast SetResurrectionData/update_reckon_base to a different signature to
 *   reproduce the original's calls; give the prototypes the real parameters instead.
 */

/* The generated extern drops this read-only table's qualifier. */

#define drone_sched mdrive_unqualified_drone_sched

#include "rush/rush_protos.h"

#undef drone_sched

#include "msl_math_inline.h"
#include "rush_flags.h"

/* How many AI cars get a physics tick on each retrace, so all of them can't land on the same
 * frame. Row = sched_to_use = number of locally simulated drones (0-6), +7 with two or more humans;
 * each row is a repeating cycle that ends at -1. A drone's dt is the time since its last tick,
 * so skipped frames are made up in one larger step. */
const signed short drone_sched[12][3] = {
    {0, -1, -1}, {1, -1, -1}, {2, -1, -1}, {3, -1, -1},
    {2, 2, -1}, {3, 2, -1}, {3, 3, -1}, {0, -1, -1},
    {1, -1, -1}, {2, -1, -1}, {2, 1, -1}, {2, 2, -1}
};

signed short humans[MAX_CARS];

signed short drones[MAX_CARS];

signed short our_drones[MAX_CARS];

/* uninitialised globals: deferred to the end of .sbss in reverse declaration order */
int gModelIteration;
float gModelTime;
signed short num_humans;
signed short num_drones;
signed short gNumCars;
signed char gNumActiveCars;
signed short cur_drone;
signed short sched_index;
signed short sched_to_use;
signed short new_sched_to_use;
signed short num_our_drones;
float gDeathWaitEnd;












static void _UpdateLinkCars(void);

static void _UpdatePlayerModel(void);

static void _UpdateDroneModels(void);

static void _UpdateModel(int slot);

static void multiinit(signed short mode, signed short droneIndex);

/* inline helper (not in the DWARF): the scalarised v[3] and the parameter temporary reproduce the original FP register use of the bodyForce sums */
static inline float model_magnitude(float *vec)
{
    float v[3];
    v[0] = vec[0] * vec[0];
    v[1] = vec[1] * vec[1];
    v[2] = vec[2] * vec[2];
    return sqrtf(v[0] + v[1] + v[2]);
}

/* inline helper used for every partial sum of update_game_data's side-force chain; with the self-copy below it reproduces the original FP register allocation (5 words off without it) */
static inline float add_force(float a, float b)
{
    float r;
    r = a + b;
    r = r; /* self-copy: a no-op second definition of the sum that keeps the compound scheduling while the result stays a fresh value */
    return r;
}

/* prototypes: the functions are defined in their original order below */
void munge_gLink_data(void);
void init_cars(void);
void fake_init(signed short trk);
void Init_MDrive(signed short mode, signed short droneIndex);
void Update_MDrive(void);
static void multiinit(signed short mode, signed short droneIndex);
void SetClutch(tModelData *m);
void init_model_task(int frames);
void model_iteration(void);
static void _UpdatePlayerModel(void);
static void _UpdateDroneModels(void);
static void _UpdateModel(int slot);
static void _UpdateLinkCars(void);
void modelstop(void);
void modelgo(void);
void multigo(signed short droneIndex);
void update_game_data(void);
static void _UpdateTireAppearance(signed short droneIndex);
void End_MDrive(signed short droneIndex);
signed short are_all_cars_stopped(void);

/* Rebuild the in-race slot lists (humans[], drones[], our_drones[]) from gLink without
 * resetting the cars. */
void munge_gLink_data(void)
{
    int i;
    int k;
    tModelData *m;
    tCarData *car;
    gNumActiveCars = 0;
    /* volatile read: the target reloads gNumCars on every loop test; the shared header keeps it plain, so the qualifier sits at this use */
    for (i = 0; i < *(volatile signed short *)&gNumCars; i++) {
        m = &model[i];
        car = &gGameCar[i];
        m->we_control = gLink[i].owner == LINK_OWNER_DRONE;
        m->drone_type = DRONE_TYPE_NONE;
        m->in_game = (gLink[i].flags >> 7) & 1; /* LINK_FLAG_IN_GAME */
        if (m->in_game) {
            car->slot = i;
            model[gNumActiveCars++].slot = i;
            car->place = gLink[i].polepos;
            m->drone_type = gLink[i].owner < LINK_OWNER_HUMAN ? DRONE_TYPE_AI : DRONE_TYPE_HUMAN;
        }
    }
    for (i = gNumCars; i < MAX_CARS; i++) {
        m = &model[i];
        m->we_control = gLink[i].owner == LINK_OWNER_DRONE;
        m->drone_type = DRONE_TYPE_NONE;
        m->in_game = 0;
    }
    /* Keep the native count-clear ordering before building the slot lists. */
    *(volatile signed short *)&num_our_drones = 0;
    *(volatile signed short *)&num_drones = 0;
    num_humans = 0;
    for (i = 0; i < gNumActiveCars; i++) {
        k = model[i].slot;
        if (model[k].drone_type == DRONE_TYPE_HUMAN) {
            humans[num_humans++] = k;
        } else {
            drones[num_drones++] = k;
            if (model[k].we_control) {
                our_drones[num_our_drones++] = k;
            }
        }
    }
    gModelIteration = 0;
}

/* Race start: clear every car slot, give each racer its car, engine, tires and paint from the
 * player or the CPU tables, and size the collision box of cars simulated elsewhere. */
void init_cars(void)
{
    int i;
    int j;
    anon_colfront_colrear_10 *sizep;
    float dt;
    tModelData *m;

    gNumActiveCars = 0;
    /* volatile read: the target reloads gNumCars on every loop test (header keeps it plain) */
    for (i = 0; i < *(volatile signed short *)&gNumCars; i++) {
        dt = model[i].fastin.dt;
        for (j = 0; j < sizeof(tModelData); j++) {
            ((char *)&model[i])[j] = 0;
        }
        model[i].fastin.dt = dt;
        for (j = 0; j < sizeof(tCarData); j++) {
            ((char *)&gGameCar[i])[j] = 0;
        }
        gGameCar[i].camera = -1;
        model[i].car = gCarModels[0];
        model[i].we_control = gLink[i].owner == LINK_OWNER_DRONE;
        gGameCar[i].view = gGameCar[i].saveview = 2;
        j = i - gNumPlayers + 1;
        if (gLink[i].owner == LINK_OWNER_HUMAN) {
            j = 6 - i;
        }
        gGameCar[i].distance = -j;
        model[i].solid = model[i].collidable = 1;
        gGameCar[i].solid = 1;
        gGameCar[i].collide_state = -1; /* written right after the model stores: storing to another object first creates the -1 constant before 0 and binds it to r4, as in the original */
        model[i].drone_target = 0;
        model[i].catchup = model[i].time_boost = model[i].drone_scale = 1.0f;
        model[i].obResPt = 0;
        model[i].resurrect.moving_state = RES_IDLE;
        model[i].wingSnd = -1;
        gGameCar[i].last_distance = -100.0f;
        gGameCar[i].crash_state = CRASH_NONE;
        InitCarVisuals(&gGameCar[i]);
        model[i].in_game = (gLink[i].flags >> 7) & 1; /* LINK_FLAG_IN_GAME */
        if (model[i].in_game) {
            gGameCar[i].slot = i;
            model[gNumActiveCars++].slot = i;
            gGameCar[i].place = gLink[i].polepos;
            model[i].drone_type = gLink[i].owner < LINK_OWNER_HUMAN ? DRONE_TYPE_AI : DRONE_TYPE_HUMAN;
            if (model[i].drone_type == DRONE_TYPE_HUMAN) {
                gGameCar[i].player = &gPlayers[i];
                switch (gDriveHulk[i]) {
                case 1:
                    model[i].appearance = gGameCar[i].appearance = APPEAR_CRASHED;
                    break;
                case 2:
                    model[i].appearance = gGameCar[i].appearance = APPEAR_HULK;
                    break;
                }
            }
            if (gGameCar[i].player) {
                model[i].body_type = gLink[i].car;
                model[i].trans = gGameCar[i].player->trans;
                model[i].autotrans = (model[i].trans & 1) ^ 1;
                gGameCar[i].data_valid = 1;
                model[i].handling = gCarHandling[model[i].slot + 1][model[i].body_type];
                model[i].engine_type = gCarEngines[model[i].slot + 1][model[i].body_type];
                model[i].tire_type = gCarTires[model[i].slot + 1][model[i].body_type];
                model[i].frame = gCarFrame[model[i].slot + 1][model[i].body_type];
                model[i].tireRims = gTireRims[model[i].slot + 1][model[i].body_type];
                model[i].PaintJob = gLink[i].PaintJob;
                model[i].data_valid = 1;
            } else if (model[i].we_control) {
                model[i].body_type = gLink[i].car;
                model[i].trans = 0;
                model[i].autotrans = 1;
                gGameCar[i].data_valid = 1;
                model[i].handling = gCarHandling[0][model[i].body_type];
                model[i].engine_type = model[i].slot & 3;
                model[i].tire_type = gCarTires[0][model[i].body_type];
                model[i].frame = gCarFrame[0][model[i].body_type];
                model[i].tireRims = gTireRims[0][model[i].body_type];
                model[i].PaintJob = gLink[i].PaintJob;
                model[i].data_valid = 1;
            }
            model[i].last_cp_index = 0;
            model[i].next_cp_index = GetNextCheckpoint(model[i].last_cp_index);
            InitCarPath(&model[i], &gGameCar[i]);
            if (!gGameCar[i].player && !model[i].we_control) {
                sizep = &collsize[model[i].body_type];
                model[i].bodyR[0][0] = model[i].bodyR[2][0] = sizep->colside;
                model[i].bodyR[1][0] = model[i].bodyR[3][0] = -sizep->colside;
                model[i].bodyR[2][1] = model[i].bodyR[3][1] =
                    model[i].bodyR[0][1] = model[i].bodyR[1][1] = sizep->colheight;
                model[i].bodyR[0][2] = model[i].bodyR[1][2] = sizep->colfront;
                model[i].bodyR[2][2] = model[i].bodyR[3][2] = -sizep->colrear;
                dt = sizep->colfront > sizep->colrear ? sizep->colfront : sizep->colrear;
                model[i].colrad = sqrtf(dt * dt + sizep->colside * sizep->colside +
                    sizep->colheight * sizep->colheight);
            }
        }
    }
    for (i = gNumCars; i < MAX_CARS; i++) {
        m = &model[i];
        m->we_control = gLink[i].owner == LINK_OWNER_DRONE;
        m->drone_type = DRONE_TYPE_NONE;
        m->in_game = 0;
    }
    if (gGameState & GS_DEMO) {
        model[0].trans = 0;
        model[0].autotrans = 1;
    }
    gOneMoreCP = 0;
    sched_to_use = num_our_drones;
    if (num_humans > 1) {
        sched_to_use += 7; /* drone_sched rows 7-11: the lighter split-screen schedules */
    }
    new_sched_to_use = sched_to_use;
    sched_index = -1;
    cur_drone = 0;
}

/* Attract mode: six CPU cars on track trk; the camera follows a different slot each time. */
void fake_init(signed short trk)
{
    signed short i;
    signed short place;
    static signed short attractSlot;

    gTrackNum = trk;
    gNumCars = MAX_CARS;
    for (i = 0, place = 0; i < MAX_CARS; i++) {
        if (i >= gNumCars) {
            gLink[i].flags = 0;
            gLink[i].owner = i + 6; /* unused slot; unreachable with gNumCars = MAX_CARS */
        } else {
            gLink[i].owner = LINK_OWNER_DRONE;
            gLink[i].polepos = place++;
            gLink[i].flags = LINK_FLAGS_RACER;
            gLink[i].track = gTrackNum;
        }
    }
    if (++attractSlot >= MAX_CARS) {
        attractSlot = 0;
    }
    gCamera[0].slot = gPlayers[0].slot = attractSlot;
    gUseCatchup = 2;
}

/* Put one car on the track: at its grid slot (INIT_MODE_START) or at its respawn point after a
 * crash or abort (INIT_MODE_CRASH). In death mode a crash ends the car's race instead. */
void Init_MDrive(signed short mode, signed short droneIndex)
{
    tCarData *car = &gGameCar[droneIndex];
    tModelData *m = &model[droneIndex];
    if (gDeathMode && !car->dead && mode == INIT_MODE_CRASH) {
        /* death mode: a wrecked car is out of the race instead of respawning */
        car->dead = CAR_DYING;
        m->crashflag = 0;
        CarReportsGameOver(droneIndex, car->modelTime);
        m->fastin.modelrun = 1;
    } else {
        if (car->dead < CAR_ALIVE) {
            car->dead = CAR_ALIVE;
        }
        m->fastin.modelrun = 0;
        multiinit(mode, droneIndex);
        if (gLink[droneIndex].owner == LINK_OWNER_HUMAN) {
            if (mode) {
                if (m->resurrect.velocity >= 44.0f) { /* 30 mph: pull away in 2nd gear */
                    m->mainin.gear = 2;
                } else {
                    m->mainin.gear = 1;
                }
            }
            car->strayAngle = 0.0f;
        }
        modelrun = 1;
    }
}

void Update_MDrive(void)
{
    int i;
    update_game_data();
    if (gRace == kGhost) {
        for (i = 1; i < gNumPlayers; i++) {
            ZOID_UpdateObject(gGameCar[i].objnum, gGameCar[i].dr_pos, gGameCar[i].dr_car_uvs);
        }
    }
    CheckCameraView();
}

/* Common (re)spawn work for Init_MDrive: respawn data, Resurrect(), clear the tire state, start
 * the engine sounds (only car 0's in a ghost race). */
static void multiinit(signed short mode, signed short droneIndex)
{
    tModelData *m = &model[droneIndex];
    if (mode == INIT_MODE_START) {
        if (gRace == kGhost) {
            m->initin.polepos = 0;
        } else {
            m->initin.polepos = gLink[droneIndex].polepos;
        }
        /* Preserve the original native call's third integer argument. */
        ((void (*)(signed short, tModelData *, int))SetResurrectionData)(INIT_MODE_START, m, 0);
    } else {
        if (gRace == kGhost) {
            /* polepos >= 6: no grid offset (resurrect.c pole_pos_offset) */
            m->initin.polepos = 6;
        } else {
            m->initin.polepos = droneIndex + 6;
        }
    }
    Resurrect(m);
    m->skidIntensity[0] = 0.0f;
    m->tires[0].sideforce = 0.0f;
    m->tires[0].traction = 0.0f;
    m->skidIntensity[1] = 0.0f;
    m->tires[1].sideforce = 0.0f;
    m->tires[1].traction = 0.0f;
    m->skidIntensity[2] = 0.0f;
    m->tires[2].sideforce = 0.0f;
    m->tires[2].traction = 0.0f;
    m->skidIntensity[3] = 0.0f;
    m->tires[3].sideforce = 0.0f;
    gGameCar[droneIndex].crash_state = CRASH_NONE;
    m->tires[3].traction = 0.0f;
    if (gRace != kGhost || m->slot == 0) {
        StartCarSounds(m);
    }
}

/* Automatic clutch: engaged when idling, slipping between 4000 and 5000 rpm under hard braking,
 * released otherwise. */
void SetClutch(tModelData *m)
{
    signed short rpm = m->rpm;
    int revs = rpm >= 0 ? rpm : -rpm;
    if (revs < 1000 && m->mainin.throttle < 0.1f) {
        m->mainin.clutch = 1.0f;
    } else if (m->mainin.brake > 0.8f) {
        if (revs < 4000) {
            m->mainin.clutch = 1.0f;
        } else if (revs >= 5000) {
            m->mainin.clutch = 0.0f;
        } else {
            m->mainin.clutch = 0.001f * (5000 - revs);
        }
    } else {
        m->mainin.clutch = 0.0f;
    }
}

/* Rewind the model clock so it reaches 0 after `frames` ticks (the countdown passes
 * 3.5 s worth). Ghost cars keep their own recorded tick rate. */
void init_model_task(int frames)
{
    int i;
    gModelIteration = -frames;
    gModelTime = gModelIteration * gRetraceTime;
    for (i = 0; i < MAX_CARS; i++) {
        if (gRace != kGhost || i == 0 || model[i].fastin.dt == 0.0f) {
            model[i].fastin.modelIteration = gModelIteration;
            model[i].fastin.modelTime = gModelTime;
            model[i].fastin.dt = gRetraceTime;
        } else {
            float iteration = gModelTime / model[i].fastin.dt;
            model[i].fastin.modelIteration = (int)(iteration < 0.0f ? iteration - 0.5f : iteration + 0.5f);
            model[i].fastin.modelTime = gModelTime;
        }
    }
}

/* One fixed physics tick for every car; see the file comment. */
void model_iteration(void)
{
    if ((gGameState & (GS_COUNTDOWN | GS_RACING | GS_DEMO)) && modelrun) {
        gModelIteration++;
        gModelTime = gModelIteration * gRetraceTime;
        _UpdateLinkCars();
        _UpdatePlayerModel();
        _UpdateDroneModels();
        if (cur_drone == 0) {
            dead_reckon_all();
        }
        UpdateListeners();
    }
}

/* Physics for the human cars. In a ghost race the tick length comes from GhostUpdate (the
 * recorded run's rate) and one call may run two ticks. The abort control respawns the car
 * when allowed. */
static void _UpdatePlayerModel(void)
{
    tPadState *player;
    tModelData *m;
    int i;
    int pass;
    int status;
    float rate;
    UpdateModelControls();
    for (i = 0; i < num_humans; i++) {
        player = &gPlayers[i];
        m = &model[player->slot];
        for (pass = 0; ; pass = 1) {
            if (gRace == kGhost) {
                status = GhostUpdate(i, &rate);
                if (status < 0) {
                    break;
                }
                m->fastin.dt = rate;
                m->fastin.modelIteration++;
                m->fastin.modelTime = m->fastin.modelIteration * rate;
            } else {
                m->fastin.dt = gRetraceTime * (gModelIteration - m->fastin.modelIteration);
                m->fastin.modelIteration = gModelIteration;
                m->fastin.modelTime = gModelTime;
            }
            SetClutch(m);
            if (m->mainin.abort &&
                (gRace != kBattle || m->crashflag) &&
                (gGameState & GS_RACING) &&
                m->resurrect.moving_state == RES_IDLE &&
                !gGameCar[player->slot].place_locked &&
                !gGameCar[player->slot].dead && !gCoastFlag &&
                (!gDeathMode || (m->magvel < 10.0f && !gGameCar[player->slot].crash_state))) {
                m->resurrect.autoAbort = 1;
                AbortCar(player->slot, 0);
            }
            if (gGameCar[player->slot].dead < CAR_DEAD) {
                _UpdateModel(player->slot);
            } else {
                collision(m);
            }
            if (gRace != kGhost || pass || status <= 0) {
                break;
            }
        }
    }
}

/* Physics for this frame's share of AI cars (drone_sched), round-robin over our_drones[]. */
static void _UpdateDroneModels(void)
{
    signed short i;
    signed short num_drones_to_do;
    tModelData *m;
    if (drone_sched[sched_to_use][++sched_index] == -1) {
        sched_index = 0;
        sched_to_use = new_sched_to_use;
    }
    num_drones_to_do = drone_sched[sched_to_use][sched_index];
    if (num_drones_to_do) {
        for (i = 0; i < num_drones_to_do; i++) {
            m = &model[our_drones[cur_drone]];
            m->fastin.dt = gRetraceTime * (gModelIteration - m->fastin.modelIteration);
            m->fastin.modelIteration = gModelIteration;
            m->fastin.modelTime = gModelTime;
            _UpdateModel(m->slot);
            if (++cur_drone >= num_our_drones) {
                cur_drone = 0;
            }
        }
    }
}

/* One car's tick: finish a death-mode coast-out, crash -> wreck -> respawn, then either the
 * respawn flight or the driving physics (sym), rumble, engine sound, wreck effects, and the
 * timer that makes a respawned car solid again. */
static void _UpdateModel(int slot)
{
    tModelData *m = &model[slot];
    tPadState *player = gGameCar[slot].player;
    if (gGameCar[slot].dead == CAR_DYING && (m->magvel < 5.0f || !m->fastin.modelrun)) {
        int i;
        gGameCar[slot].dead = CAR_DEAD;
        m->crashflag = 0;
        m->magvel = 0.0f;
        gGameCar[slot].crash_state = CRASH_NONE;
        m->fastin.modelrun = 0;
        if (player) {
            if (new_sched_to_use >= 7) new_sched_to_use -= 7; /* a human is out: back to the single-player schedules */
            PS_SetRumblePercent(player->controller, 0);
        } else {
            for (i = cur_drone; i < num_our_drones - 1; i++) our_drones[i] = our_drones[i + 1];
            num_our_drones--;
            cur_drone--;
            new_sched_to_use--; /* one drone fewer to schedule */
        }
        return;
    }
    CheckCrash(slot);
    FinishResurrecting(m);
    if (gGameCar[slot].crash_state == CRASH_RESPAWN && gGameCar[slot].dead <= CAR_ALIVE) {
        End_MDrive(slot);
        Init_MDrive(INIT_MODE_CRASH, slot);
    } else if (m->crashflag && gGameCar[slot].dead >= CAR_DEAD) {
        model[slot].crashflag = 0;
    }
    /* time scale for this car: speed-boost pads, rubber-band catch-up (model.c SetCatchup) and
     * time_boost; the car's dt is stretched by it, so the physics itself runs faster */
    m->time_fudge = (1.0f + 0.02f * m->roadboost[0]) * (m->catchup * m->time_boost);
    /* a car that missed more than ~2 ticks (an AI car waiting on drone_sched) catches up by two
     * ticks at most. PORT: tuned for gRetraceTime = 1/60 or 1/50 s. */
    if (m->fastin.dt > 0.045f) m->fastin.dt = 2.0f * gRetraceTime;
    m->fastin.dt *= m->time_fudge;
    if (m->resurrect.moving_state > RES_IDLE) {
        if (player) {
            UpdateResurrectingCar(m);
            if (gGameState & GS_RACING) PS_SetRumblePercent(player->controller, 0);
        } else {
            UpdateResurrectingCar(m);
        }
        ((void (*)(int, float))update_reckon_base)(slot, m->fastin.modelTime);
    } else if (m->fastin.modelrun) {
        if (player) {
            sym(m);
            if ((gGameState & GS_RACING) && !(gRace == kGhost && gGhosts[m->slot] && (**(**gGhosts[m->slot]).header).flag >= 0)) {
                /* rumble: summed body/tire/collision force -> a 0-35 strength step (the first entry
                 * it is below); the player's rumble setting 1 knocks 4 steps off, 0 turns it off */
                static int percentTable[36] = {
                    4000,5000,6500,8000,10000,12000,15000,20000,25000,
                    30000,40000,50000,60000,70000,80000,90000,100000,125000,
                    150000,175000,200000,225000,250000,275000,300000,325000,350000,
                    375000,400000,425000,450000,475000,500000,525000,550000,600000
                };
                int i;
                int force = 0;
                int percent;
                for (i = 0; i < 4; i++) force += 4.0f * model_magnitude(m->bodyForce[i]);
                force += 4.0f * model_magnitude(m->centerForceLast);
                force += m->tireForce[0][1];
                force += m->tireForce[1][1];
                force += m->tireForce[2][1];
                force += m->tireForce[3][1];
                if (gRace == kBattle) {
                    force += (gGameCar[slot].battle.damageThisFrame * 500000) / 800;
                    gGameCar[slot].battle.damageThisFrame = 0;
                }
                if (gGameCar[slot].crash_state == CRASH_WRECKED) {
                    float delta_time = m->fastin.modelTime - gGameCar[slot].crashtime;
                    if (delta_time < 1.0f) force += 100000.0f * (1.0f - delta_time);
                }
                for (percent = 0; percent < 36; percent++) if (force < percentTable[percent]) break;
                if (!(**player->save).rumble) percent = 0;
                else if ((**player->save).rumble == 1) {
                    percent -= 4;
                    if (percent < 0) percent = 0;
                }
                PS_SetRumblePercent(player->controller, percent);
            }
        } else {
            MaxPathControls(m);
            sym(m);
        }
        ((void (*)(int, float))update_reckon_base)(slot, m->fastin.modelTime);
    }
    if (gRace != kGhost || !m->slot) UpdateCarSounds(m);
    if (m->crashflag) m->appearance |= APPEAR_CRASHED;
    else if (!(m->appearance & APPEAR_HULK)) m->appearance &= ~APPEAR_WRECK_SMOKE;
    if (!m->solid) {
        float delta_time;
        if (m->resurrect.moving_state > RES_IDLE) {
            gGameCar[slot].collide_time = m->fastin.modelTime;
            delta_time = 0.0f;
        } else {
            delta_time = m->fastin.modelTime - gGameCar[slot].collide_time;
        }
        /* ORIGINAL BUG: `gRace != kBattle || gRace != kObstacle` is always true, so every mode turns
         * solid again after 5 s; battle/obstacle also do after 0.6 s. Kept for matching; a port that
         * wants the original behaviour must keep it too. */
        if (gCoastFlag || ((gRace != kBattle || gRace != kObstacle) && delta_time >= 5.0f) || ((gRace == kBattle || gRace == kObstacle) && delta_time > 0.6f)) {
            if (gLink[m->slot].owner == LINK_OWNER_DRONE || gLink[m->slot].owner == LINK_OWNER_HUMAN) {
                m->collidable = 1;
                m->solid = 1;
                gGameCar[slot].collide_time = 0.0f;
            }
        }
    }
}

/* Speed of the cars simulated elsewhere, from their dead-reckoned velocity. */
static void _UpdateLinkCars(void)
{
    int i;
    for (i = 0; i < MAX_CARS; i++) {
        tModelData *m = &model[i];
        if (!gGameCar[i].player && !m->we_control) {
            m->magvel = sqrtf(m->reckon.rwv[0] * m->reckon.rwv[0] +
                m->reckon.rwv[1] * m->reckon.rwv[1] +
                m->reckon.rwv[2] * m->reckon.rwv[2]);
        }
    }
}

void modelstop(void)
{
    int i;
    modelrun = 0;
    for (i = 0; i < gNumActiveCars; i++) {
        StopCarSounds(&model[i]);
    }
    if (!(gGameState & GS_DEMO)) {
        PS_RumbleStop();
    }
}

void modelgo(void)
{
    int i;
    if ((gGameState & GS_DEMO) || (gGameState & GS_RACE_MASK)) {
        for (i = 0; i < gNumActiveCars; i++) {
            StartCarSounds(&model[i]);
        }
    }
    if (!(gGameState & GS_DEMO)) {
        PS_RumbleGo();
    }
    modelrun = 1;
}

void multigo(signed short droneIndex)
{
    model[droneIndex].fastin.modelrun = 1;
}

/* Once per game frame: copy each car's simulation state into gGameCar for the renderer, sound
 * and HUD, and derive the visual bits (lean, brake lights, sparks, skids, smoke). */
void update_game_data(void)
{
    signed short i;
    signed short j;
    signed short k;
    signed short l;
    signed short high_index;
    signed short same_count[4];
    tModelData *m;
    tCarData *gc;
    float angle;
    float forwardPair, sidePair;
    float fwdforce;
    float sideforce;
    const _tSusp *susp;
    unsigned short tsf;

    game_reckon_all(gModelTime);
    for (i = 0; i < MAX_CARS; i++) {
        m = &model[i];
        gc = &gGameCar[i];
        if (!m->in_game || gc->dead >= CAR_DEAD) {
            continue;
        }
        gc->modelIteration = m->fastin.modelIteration;
        gc->modelTime = m->fastin.modelTime;
        gc->dr_pos[0] = m->reckon.rwr[0];
        gc->dr_pos[1] = m->reckon.rwr[1];
        gc->dr_pos[2] = m->reckon.rwr[2];
        gc->stuntW[0] = m->stuntW[0];
        gc->stuntW[1] = m->stuntW[1];
        gc->stuntW[2] = m->stuntW[2];
        m->stuntW[0] = 0.0f;
        m->stuntW[1] = 0.0f;
        m->stuntW[2] = 0.0f;
        CopyUV(m->reckon.uvs, gc->dr_uvs);
        CopyUV(m->reckon.uvs, gc->dr_car_uvs);
        susp = carSuspensions[gCarSuspensions[m->body_type]];
        forwardPair = m->tireForce[0][2] + m->tireForce[1][2];
        sidePair = add_force(m->tireForce[0][0], m->tireForce[1][0]);
        forwardPair = m->tireForce[2][2] + forwardPair;
        sidePair = add_force(m->tireForce[2][0], sidePair);
        angle = m->tireForce[3][2] + forwardPair;
        fwdforce = angle;
        sideforce = m->tireForce[3][0] + sidePair;
        if ((angle = fwdforce - susp->fwdforcemin) > 0.0f) {
            angle *= susp->fwdangle / susp->fwdforcerange;
            if (angle > susp->fwdangle) angle = susp->fwdangle;
        } else {
            if ((angle = fwdforce - susp->bckforcemin) < 0.0f) {
                angle *= -susp->bckangle / susp->bckforcerange;
                if (angle < susp->bckangle) angle = susp->bckangle;
            } else angle = 0.0f;
        }
        /* cosmetic body pitch and roll from the summed tire forces, smoothed; only the drawn car leans */
        gc->pitch = 0.6f * gc->pitch + 0.4f * angle;
        PitchUV(gc->pitch, gc->dr_car_uvs);
        if ((angle = sideforce - susp->sideforcemin) > 0.0f) {
            angle *= -susp->sideangle / susp->sideforcerange;
            if (angle > susp->sideangle) angle = susp->sideangle;
        } else {
            if ((angle = sideforce + susp->sideforcemin) < 0.0f) {
                angle *= -susp->sideangle / susp->sideforcerange;
                if (angle < -susp->sideangle) angle = -susp->sideangle;
            } else angle = 0.0f;
        }
        gc->roll = 0.7f * gc->roll + 0.3f * angle;
        RollUV(gc->roll, gc->dr_car_uvs);
        for (j = 0; j < 4; j++) {
            gc->tireR[j][0] = m->car->tireR[j][0];
            gc->tireR[j][1] = m->car->tireR[j][1];
            gc->tireR[j][2] = m->car->tireR[j][2];
            WorldVector(m->car->tireR[j], gc->dr_tirepos[j], gc->dr_uvs);
            gc->dr_tirepos[j][0] += gc->dr_pos[0];
            gc->dr_tirepos[j][1] += gc->dr_pos[1];
            gc->dr_tirepos[j][2] += gc->dr_pos[2];
        }
        gc->dr_vel[0] = m->reckon.rwv[0];
        gc->dr_vel[1] = m->reckon.rwv[1];
        gc->dr_vel[2] = m->reckon.rwv[2];
        gc->dr_acc[0] = m->reckon.rwa[0];
        gc->dr_acc[1] = m->reckon.rwa[1];
        gc->dr_acc[2] = m->reckon.rwa[2];
        if (gLink[i].owner == LINK_OWNER_DRONE || gLink[i].owner == LINK_OWNER_HUMAN) {
            high_index = 0;
            for (k = 0; k < 4; k++) {
                gc->sound_flags[k] = m->sound_flags[k];
                same_count[k] = 0;
                if (m->sviscode[k] != SURF_VIS_AIRBORNE || m->sound_flags[k]) {
                    for (l = k + 1; l < 4; l++) {
                        if (m->sound_flags[k] == m->sound_flags[l]) same_count[k]++;
                    }
                }
                if (same_count[k] >= same_count[high_index]) {
                    high_index = k;
                    tsf = m->sound_flags[k];
                }
            }
            gc->in_tunnel = (tsf >> 8) & 1; /* bit 8 of the most common wheel sound flags */
            if (m->brake) m->appearance |= APPEAR_BRAKE;
            else m->appearance &= ~APPEAR_BRAKE;
            if (m->magvel > 20.0f && m->bodyForce[0][0]) m->appearance |= APPEAR_SPARK_C0;
            else m->appearance &= ~APPEAR_SPARK_C0;
            if (m->magvel > 20.0f && m->bodyForce[2][0]) m->appearance |= APPEAR_SPARK_C2;
            else m->appearance &= ~APPEAR_SPARK_C2;
            if (m->magvel > 20.0f && m->bodyForce[1][0]) m->appearance |= APPEAR_SPARK_C1;
            else m->appearance &= ~APPEAR_SPARK_C1;
            if (m->magvel > 20.0f && m->bodyForce[3][0]) m->appearance |= APPEAR_SPARK_C3;
            else m->appearance &= ~APPEAR_SPARK_C3;
            if (m->thumpflag > 2) m->appearance |= APPEAR_THUMP;
            else if (!m->thumpflag) m->appearance &= ~APPEAR_THUMP;
            if (gc->solid && !m->solid) m->appearance &= ~APPEAR_INVISIBLE;
            else if (!gc->solid && m->solid) m->appearance |= APPEAR_INVISIBLE;
            if (gc->solid && m->solid) m->appearance &= ~APPEAR_INVISIBLE;
            _UpdateTireAppearance(i);
            gc->v[0] = m->v[0];
            gc->v[1] = m->v[1];
            gc->v[2] = m->v[2];
            gc->rwmph = m->reckon.rwmph;
        }
        gc->appearance = m->appearance;
        m->appearance &= ~APPEAR_BODY_HIT; /* one-frame pulse */
        gc->collidable = m->collidable;
        gc->solid = m->solid;
        if (!gc->solid && gc->collide_time == 0.0f) {
            gc->collide_time = m->fastin.modelTime;
            gc->collide_count = gc->collide_state = -1;
        }
        if (gc->solid == 1) gc->collide_time = 0.0f;
        gc->data_valid = m->data_valid;
    }
}

/* Skid marks and tire smoke. `side` means two things here: for sSmoke2 it is a tire number;
 * for sSkid/sSmoke/sSkidData it is a skid direction: 0 = the -x pair sliding sideways,
 * 1 = the +x pair sliding, 2 = front wheelspin/lock-up, 3 = rear. The first loop mixes the two
 * (sSkid[tire]); that is the original code. */
static void _UpdateTireAppearance(signed short droneIndex)
{
static const unsigned int sSmoke[4] = {APPEAR_SMOKE_T3, APPEAR_SMOKE_T2,
    APPEAR_SMOKE_T0 | APPEAR_SMOKE_T1, APPEAR_SMOKE_T2 | APPEAR_SMOKE_T3};
static const unsigned int sSmoke2[4] = {APPEAR_SMOKE_T0, APPEAR_SMOKE_T1, APPEAR_SMOKE_T2, APPEAR_SMOKE_T3};
static const unsigned int sSkid[4] = {APPEAR_SKID_T1 | APPEAR_SKID_T3, APPEAR_SKID_T0 | APPEAR_SKID_T2,
    APPEAR_SKID_T0 | APPEAR_SKID_T1, APPEAR_SKID_T2 | APPEAR_SKID_T3};
static const struct tSkidData sSkidData[4] = {
    {{1500, 1700, 1900, 2200, 2600}, 0.625f, 0.977f},
    {{1500, 1700, 1900, 2200, 2600}, 0.625f, 0.977f},
    {{800, 975, 1250, 1325, 1500}, 0.5f, 0.977f},
    {{2400, 2225, 2050, 1875, 1700}, 0.469f, 0.742f}
};


    int index;
    int side;
    int force;
    int forceLow;
    tModelData *m = &model[droneIndex];
    const struct tSkidData *skidData;

    /* not in the DWARF: the original likely tested these inline */
    int airborne = m->sviscode[1] == SURF_VIS_AIRBORNE || m->sviscode[2] == SURF_VIS_AIRBORNE;
    int wet = m->sviscode[1] == SURF_VIS_WATER || m->sviscode[2] == SURF_VIS_WATER;

    for (side = 0; side < 4; side++) {
        if (m->sviscode[side] == SURF_VIS_DIRT) { /* dust cloud behind a tire on dirt */
            m->appearance |= sSmoke2[side];
        } else {
            m->appearance &= ~(sSkid[side] | sSmoke2[side]);
        }
    }
    if (airborne || wet || gCoastFlag) { /* no skids or tire smoke in the air, in water or after the finish */
        for (side = 0; side < 4; side++) {
            m->skidIntensity[side] = 0.0f;
            m->appearance &= ~(sSkid[side] | sSmoke[side]);
        }
    } else {
        for (side = 0; side < 4; side++) {
            switch (side) {
            case 0:
                force = fabs(m->tires[1].sideforce) > fabs(m->tires[3].sideforce) ?
                    fabs(m->tires[1].sideforce) : fabs(m->tires[3].sideforce);
                break;
            case 1:
                force = fabs(m->tires[0].sideforce) > fabs(m->tires[2].sideforce) ?
                    fabs(m->tires[0].sideforce) : fabs(m->tires[2].sideforce);
                break;
            case 2:
                force = fabs(m->tires[0].traction) > fabs(m->tires[1].traction) ?
                    fabs(m->tires[0].traction) : fabs(m->tires[1].traction);
                break;
            case 3:
                force = fabs(m->tires[2].traction) + fabs(m->tires[3].traction);
                break;
            }
            skidData = &sSkidData[side];
            { float scale = gRace == kBattle ? 3.0f : m->tune->skidk;
            if (force >= scale * skidData->forceTable[0]) {
                if (force > skidData->forceTable[4]) {
                    force = skidData->forceTable[4];
                }
                for (index = 0; index < 4; index++) {
                    if (force <= skidData->forceTable[index + 1]) {
                        break;
                    }
                }
                forceLow = skidData->forceTable[index];
                m->skidIntensity[side] = 0.25f * (index +
                    (float)(force - forceLow) / (skidData->forceTable[index + 1] - forceLow));
                if (m->skidIntensity[side] > skidData->skidThreshold) {
                    m->appearance |= sSkid[side];
                }
                if (m->skidIntensity[side] > skidData->smokeThreshold) {
                    m->appearance |= sSmoke[side];
                }
            } else if (m->skidIntensity[side]) {
                if (m->skidIntensity[side] > skidData->skidThreshold) {
                    m->appearance |= sSkid[side];
                }
                if (m->skidIntensity[side] > skidData->smokeThreshold) {
                    m->appearance |= sSmoke[side];
                }
                m->skidIntensity[side] -= 0.125f; /* fade out over 8 updates */
                if (m->skidIntensity[side] <= 0.0f) {
                    m->skidIntensity[side] = 0.0f;
                    m->appearance &= ~(sSkid[side] | sSmoke[side]);
                }
            }
            }
        }
    }
}

void End_MDrive(signed short droneIndex)
{
    tModelData *m = &model[droneIndex];
    m->fastin.modelrun = 0;
    StopCarSounds(m);
    m->skidIntensity[0] = 0.0f;
    m->skidIntensity[1] = 0.0f;
    m->skidIntensity[2] = 0.0f;
    m->skidIntensity[3] = 0.0f;
}

/* End of race: true once every human car has crossed the line and slowed below 5 ft/s (and all
 * stunts are scored). Death mode with several players waits 4 s after the first finisher. */
signed short are_all_cars_stopped(void)
{
    signed short i;
    signed short k;
    signed short done_flag;
    if (gDeathMode && gNumPlayers > 1 && gAPlayerReallyFinished) {
        if (gDeathWaitEnd > 0.0f) {
            gDeathWaitEnd += gFrameSeconds;
            if (gDeathWaitEnd > 4.0f) { /* 4 s after the leader finished */
                return 1;
            }
        } else {
            for (i = 0; i < gNumPlayers; i++) {
                if (gNumLaps == model[i].laps) {
                    gDeathWaitEnd = gFrameSeconds;
                }
            }
        }
        return 0;
    }
    done_flag = 1;
    k = gRace == kGhost ? 1 : gNumActiveCars;
    for (i = 0; i < k; i++) {
        if (model[model[i].slot].drone_type == DRONE_TYPE_HUMAN &&
            (!gDeathMode || gNumLaps == model[model[i].slot].laps) &&
            model[model[i].slot].magvel > 5.0f) {
            done_flag = 0;
            break;
        }
    }
    if (done_flag && gRace == kStunt) {
        done_flag = gStuntsDone;
    }
    return done_flag;
}
