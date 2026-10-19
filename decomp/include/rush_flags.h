/* rush_flags.h - names for the magic numbers in the decompiled game code.
 *
 * Hand-written, not generated. The original #defines did not survive into the DWARF, so every
 * name here is ours, read off how the value is used. Each group cites its evidence; a "?" in a
 * comment marks a meaning that is a best guess.
 *
 * Matching rule: each macro expands to a constant of the same type and value as the literal it
 * replaces (int stays int, floats keep their `f` suffix), so swapping one in never changes code
 * generation. Keep them macros: an enum constant in place of an int literal can change
 * CodeWarrior's select shape (writeup "highscorescr matched").
 */
#ifndef RUSH_FLAGS_H
#define RUSH_FLAGS_H

/* ---- Car slots ---------------------------------------------------------------------------- */

/* model[], gGameCar[], gLink[0..5]: the race always has six car slots (humans first, then
 * drones); unused slots are flagged out of the race rather than removed. */
#define MAX_CARS 6

/* ---- gGameState: one-hot top-level state -------------------------------------------------- */
/* Requested with setGameState() and applied at the top of the next frame (state.c
 * updateGameState, which runs uninitGameState/initGameState). Front-end states are dispatched by
 * updateFrontEnd (front.c), in-game states by _UpdateGame (game.c). */

#define GS_BOOT          0x1        /* one-time init, then -> GS_LICENSE */
#define GS_LICENSE       0x2        /* updateLicenseScreen */
#define GS_TITLE         0x4        /* updateTitleScreen */
#define GS_DEMO          0x8        /* updateDemoGame: attract-mode race, every car is a drone */
#define GS_START         0x10       /* updateStartScreen */
#define GS_PLAYER        0x20       /* updatePlayerScreen */
#define GS_MODE          0x40       /* updateModeScreen */
#define GS_TRACK         0x80       /* updateTrackScreen */
#define GS_CAR           0x100      /* updateCarScreen */
#define GS_OPTIONS       0x200      /* updateOptionScreen */
#define GS_CONTROLS      0x400      /* updateControlScreen */
#define GS_AUDIO         0x800      /* updateAudioScreen */
#define GS_VIDEO         0x1000     /* updateVideoScreen */
#define GS_RECORDS       0x2000     /* updateRecordScreen */
#define GS_CIRCUIT       0x4000     /* updateCircuitScreen */
/* 0x8000, 0x10000, 0x20000: never set */
#define GS_GAME_INIT     0x40000    /* options, pole positions; state.c loads the race sound banks */
#define GS_PREPLAY       0x80000    /* PrePlay() streams the track in, then -> GS_COUNTDOWN */
#define GS_GO            0x100000   /* countdown hit zero: race clock, music, then -> GS_RACING */
#define GS_COUNTDOWN     0x200000   /* cars and physics live, 3.5 s start countdown */
#define GS_RACING        0x400000   /* the race proper */
#define GS_GAME_OVER     0x800000   /* updateGameOverScreen */
#define GS_END_GAME      0x1000000  /* _EndGame: tear down the race */
#define GS_HIGH_SCORE    0x2000000  /* updateHighScoreScreen */
#define GS_PAK           0x4000000  /* updatePakScreen: memory-card check */
#define GS_CHEAT         0x8000000  /* updateCheatScreen */
#define GS_PAK_EXIT      0x10000000 /* ? set by pakscr on A/Start; only effect: uninit unloads fonts */
#define GS_UNKNOWN_20000000 0x20000000 /* ? never set here; initGameState would load wave 41 */
#define GS_WARNING       0x40000000 /* updateWarningScreen */

/* Every state updateFrontEnd handles (the menus, title, demo, pak and warning screens). */
#define GS_FRONTEND_MASK 0x7c03fffe
/* Every state _UpdateGame handles: GS_GAME_INIT through GS_HIGH_SCORE. */
#define GS_IN_GAME_MASK  0x3fc0000
/* Race set-up through racing: GS_GAME_INIT | GS_PREPLAY | GS_GO | GS_COUNTDOWN | GS_RACING. */
#define GS_RACE_MASK     0x7c0000

/* ---- gLink[slot]: who owns each car slot -------------------------------------------------- */
/* An arcade leftover: on linked cabinets `owner` named the machine that simulated the car. On
 * GameCube only these values occur (game.c _CalcPolePositions, ghost.c, mdrive.c fake_init). */

#define LINK_OWNER_DRONE 0          /* CPU car simulated here (model.we_control = owner == 0) */
#define LINK_OWNER_HUMAN 6          /* local human player */
#define LINK_OWNER_EMPTY 7          /* slot not in the race (fake_init writes slot + 6 instead) */

#define LINK_FLAG_IN_GAME 0x80      /* model.in_game = (flags >> 7) & 1 */
#define LINK_FLAGS_RACER  0xb0      /* what every racing slot gets; the 0x30 bits are not read here */

/* ---- tModelData.drone_type ---------------------------------------------------------------- */
#define DRONE_TYPE_NONE  0          /* slot not in the race */
#define DRONE_TYPE_AI    1          /* gLink owner < 6 */
#define DRONE_TYPE_HUMAN 2          /* gLink owner >= 6; listed in humans[] */

/* ---- tModelData.sviscode[tire]: surface under each tire ----------------------------------- */
/* audio.c road noise (DWARF locals paveVol, dirtVol, waterVol), skids.c (rubber vs dirt marks),
 * smoke.c (water spray), stunt.c (counts the wheels on the ground as sviscode != 8). */

#define SURF_VIS_PAVEMENT 0
#define SURF_VIS_DIRT     1
#define SURF_VIS_WATER    2         /* audio also treats 3 as water */
#define SURF_VIS_AIRBORNE 8         /* tire not touching anything */

/* ---- appearance: per-car visual-effect bits ----------------------------------------------- */
/* tModelData.appearance is written by the physics side (mdrive.c, resurrect.c, stree.c) and
 * copied into tCarData.appearance once a frame by update_game_data, which the render side reads
 * (visuals.c, smoke.c, skids.c, headlights.c, stunt.c).
 *
 * Tire / corner numbering, from the suspension and skid code: 0 and 1 are the front pair, 2 and 3
 * the rear; 0 and 2 sit on the +x side, 1 and 3 on the -x side. */

#define APPEAR_SKID_T1       0x2       /* skid mark under tire 1 (skids.c appFlags) */
#define APPEAR_SKID_T0       0x4
#define APPEAR_INVISIBLE     0x8       /* not drawn this frame: blink while non-solid after a respawn */
#define APPEAR_HULK          0x10      /* draw the burnt-out wreck (body type 13); also the
                                        * gDriveHulk "drive the hulk" cheat */
#define APPEAR_WRECK_SMOKE   0x60      /* wreck smoke, StartSmoke type 4; 0x20 and 0x40 always move together */
#define APPEAR_SPARK_C0      0x80      /* body scraping at corner 0: sparks (needs > 20 ft/s) */
#define APPEAR_SPARK_C1      0x100
#define APPEAR_SKID_T3       0x200
#define APPEAR_SKID_T2       0x400
#define APPEAR_THUMP         0x800     /* underbody hit: sparks under the car */
#define APPEAR_BRAKE         0x1000    /* brake lights on */
#define APPEAR_SPARK_C3      0x2000
#define APPEAR_SPARK_C2      0x4000
#define APPEAR_SMOKE_T2      0x10000   /* tire smoke, StartSmoke type = tire number */
#define APPEAR_SMOKE_T3      0x20000
#define APPEAR_SMOKE_T0      0x40000
#define APPEAR_SMOKE_T1      0x80000
#define APPEAR_BODY_HIT      0x100000  /* one-frame pulse: stree.c CarApplyCollision hit the body on
                                        * the world; stunt.c counts it; cleared after the copy */

/* A crash turns the car into a smoking hulk. */
#define APPEAR_CRASHED       (APPEAR_HULK | APPEAR_WRECK_SMOKE)

/* ---- Crash, death and respawn state ------------------------------------------------------- */

/* tCarData.crash_state (resurrect.c CheckCrash) */
#define CRASH_NONE           0
#define CRASH_WRECKED        1         /* wreck on show; after 3.5 s (2.25 s in battle) -> respawn */
#define CRASH_RESPAWN        2         /* _UpdateModel re-inits the car at its respawn point */

/* tCarData.dead. Only death mode ("last car standing") uses CAR_DYING and CAR_DEAD. */
#define CAR_ABORTED          (-1)      /* aborted / respawned outside death mode; reset to 0 on init */
#define CAR_ALIVE            0
#define CAR_DYING            1         /* out of the race, coasting to a stop */
#define CAR_DEAD             2         /* stopped: physics off, only collision() runs */

/* tModelData.resurrect.moving_state: the respawn flight. Positive values count frames in flight. */
#define RES_IDLE             (-1)      /* not respawning */
#define RES_START            0         /* Resurrect() set up a flight; first frame */
#define RES_DONE             (-2)      /* flight over; FinishResurrecting drops the car in */

/* `mode` of Init_MDrive, multiinit and SetResurrectionData */
#define INIT_MODE_START      0         /* race start: grid slot, respawn point = last checkpoint */
#define INIT_MODE_KEEP       1         /* never passed; SetResurrectionData leaves everything alone */
#define INIT_MODE_CRASH      2         /* Init_MDrive: respawn the car; SetResurrectionData: save
                                        * the crash point and time to respawn from */

/* ---- Units -------------------------------------------------------------------------------- */
/* World units are feet, speeds feet per second, Y is up. */

#define MPH_TO_FTPS          1.4666667f /* 88 ft/s = 60 mph */

#endif /* RUSH_FLAGS_H */
