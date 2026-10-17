/* controlscr.c - live function bodies recovered privately.
 * NonMatching: no canonical integration, unit layout promotion, or link performed.
 */
#include "rush/rush_protos.h"
#include "msl_math_inline.h"
#pragma force_active on

static void uninitButtons(signed char p);
static int validOption(signed char p, int option);

static float gButtonZoom = 1.0f;
static float gButtonScale = 1.0f;
static float gButtonSpace = 40.0f;
static float maxY = 120.0f;
static float minY = -120.0f;
static float buttonZ = 300.0f;
static float cursorZ = 225.0f;
static int animControls(_tBlit_2 *blt);
static int animControls2(_tBlit_2 *blt);
static int animText(void);
static void calcControlMenuMaxSize(void);
static void initButtons(signed char p);
static int validControl(signed char p);
static void updateButtons(signed char p);
static char *buttonNames[11] = {
    "BUTTON_S2G1", "BUTTON_S3G1", "BUTTON_S2G1", "CURSORG1", "AB_BUTTONG1", "ARROW_BGG1",
    "ARROW_FGG1", "BAR_BGG1", "BAR_FGG1", "BUTTON_S1G1", "BUTTON_S0G1"
};

static const anon_texname_dulx_24 buttonsFrontEnd[89] = {
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 257 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 258 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 259 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 260 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 261 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 262 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 263 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 264 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 265 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 266 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 267 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 273 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 274 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 275 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 276 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 277 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 278 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 279 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 280 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 281 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 282 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 283 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4353 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4354 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4355 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4356 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4357 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4358 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4359 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4360 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4361 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4362 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4363 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4369 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4370 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4371 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4372 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4373 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4374 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4375 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4376 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4377 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4378 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4379 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8449 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8450 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8451 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8452 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8453 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8454 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8455 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8456 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8457 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8458 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8459 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8465 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8466 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8467 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8468 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8469 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8470 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8471 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8472 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8473 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8474 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8475 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12545 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12546 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12547 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12548 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12549 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12550 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12551 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12552 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12553 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12554 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12555 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12561 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12562 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12563 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12564 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12565 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12566 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12567 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12568 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12569 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12570 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12571 },
    { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))animText, 0 },
};

static const anon_texname_dulx_24 buttonsPauseSingle[23] = {
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 257 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 258 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 259 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 260 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 261 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 262 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 263 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 264 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 265 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 266 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 267 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 273 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 274 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 275 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 276 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 277 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 278 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 279 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 280 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 281 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 282 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 283 },
    { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))animText, 0 },
};

static const anon_texname_dulx_24 buttonsPauseMulti[45] = {
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 257 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 258 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 259 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 260 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 261 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 273 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 274 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 275 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 276 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 277 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4353 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4354 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4355 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4356 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4357 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4369 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4370 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4371 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4372 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 4373 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8449 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8450 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8451 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8452 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8453 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8465 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8466 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8467 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8468 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 8469 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12545 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12546 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12547 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12548 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12549 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12561 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12562 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12563 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12564 },
    { "CONTROLS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls, 12565 },
    { "CONTROLS_LG", 10, 10, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls2, 278 },
    { "CONTROLS_LG", 10, 10, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls2, 4374 },
    { "CONTROLS_LG", 10, 10, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls2, 8470 },
    { "CONTROLS_LG", 10, 10, -1, -1, -1, -1, -1, -1, 0, 254, (int (*)(_tBlit *))animControls2, 12566 },
    { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))animText, 0 },
};


static anon_buttonType_objIdx_40 buttonInfo[4][18] = {
{
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 9, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 10, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 }
},
{
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 9, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 10, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 }
},
{
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 9, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 10, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 }
},
{
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 9, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 10, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
    { 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 }
}
};
static anon_x_y_18 gPlayerRects[4][4] = {
    80, 0, 160, 220, 160, 110, 0, 0, 320, 220, 160, 110, 0, 0, 320, 220, 160, 110, 0, 0, 320, 220,
    160, 110, 80, 0, 160, 110, 160, 55, 80, 110, 160, 110, 160, 165, 0, 0, 320, 220, 160, 110, 0,
    0, 320, 220, 160, 110, 0, 0, 160, 110, 80, 55, 160, 0, 160, 110, 240, 55, 0, 110, 160, 110, 80,
    165, 0, 0, 320, 220, 160, 110, 0, 0, 160, 110, 80, 55, 160, 0, 160, 110, 240, 55, 0, 110, 160,
    110, 80, 165, 160, 110, 160, 110, 240, 165
};
static signed char gControlValid[26] = { 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 };
signed char controlScrActive[4];
signed char gConflictExists[4];
static signed short gControlSelect[4];
static signed short gControlFirst[4];
static signed short gControlMenuSize;
static signed char prevControlScrActive[4];
static enum_kNumMenuDir_5 exitControlScreen;
static _tBlit_2 *blit;
static signed char inited;
static float buttonX;
static float gCameraMatrix[4][4];
static float scrollSpeed[4];
static float cursorSpeed[4];
static int buttonsJustInited[4];
static int buttonsInited[4];
signed char gConflict[4][11];



void updateControlScreen(void);
void cleanupControlScreen(void);
static int animControls(_tBlit_2 *blt);
static int animControls2(_tBlit_2 *blt);
static int animText(void);
static void animButtonText(signed char p);
static void calcControlMenuMaxSize(void);
static int validOption(signed char p, int option);
static int validControl(signed char p);
static void initButtons(signed char p);
static void uninitButtons(signed char p);
static void updateButtons(signed char p);

void updateControlScreen(void)
{
    int forcedExit;
    int p;
    int i;
    int j;
    int menuItem;
    int maxFirstItem;
    signed char *controls;
    if (!inited) {
        if (gGameState & 0x7c03fffe) {
            signed char skipInit = 1;
            for (p = 0; p < gNumWindows; p++) {
                if (controlScrActive[p] == 1) skipInit = 0;
            }
            if (skipInit == 1) return;
        } else {
            for (p = 0; p < gNumWindows; p++) {
                InitPane(gPlayerRects[gNumWindows-1][p].x,
                    gPlayerRects[gNumWindows-1][p].y,
                    gPlayerRects[gNumWindows-1][p].x + gPlayerRects[gNumWindows-1][p].w,
                    gPlayerRects[gNumWindows-1][p].y + gPlayerRects[gNumWindows-1][p].h,
                    176,0,0,0);
            }
            for (p = gNumWindows; p < 4; p++) prevControlScrActive[p] = 0;
        }
        calcControlMenuMaxSize();
        if (gGameState & 0x7c03fffe)
            blit = (_tBlit_2 *)NewMultiBlit(0,0,buttonsFrontEnd,89);
        else if (gNumWindows == 1)
            blit = (_tBlit_2 *)NewMultiBlit(0,0,buttonsPauseSingle,23);
        else
            blit = (_tBlit_2 *)NewMultiBlit(0,0,buttonsPauseMulti,45);
        if (gNumWindows == 1) {
            gButtonZoom = 1.0f;
            gButtonScale = 0.9f;
            gButtonSpace = 40.0f;
            buttonX = 0.0f;
            maxY = 100.0f;
            minY = -120.0f;
            buttonZ = 300.0f;
            cursorZ = 225.0f;
        } else if (gNumWindows == 2) {
            gButtonZoom = 1.0f;
            gButtonScale = 1.3f;
            gButtonSpace = 40.0f;
            buttonX = 0.0f;
            maxY = 55.0f;
            minY = -120.0f;
            buttonZ = 300.0f;
            cursorZ = 225.0f;
        } else {
            gButtonZoom = 1.0f;
            gButtonScale = 1.3f;
            gButtonSpace = 40.0f;
            buttonX = -30.0f;
            maxY = 55.0f;
            minY = -120.0f;
            buttonZ = 300.0f;
            cursorZ = 225.0f;
        }
        exitControlScreen = 0;
        inited = 1;
    }
    for (p = 0; p < 4; p++) {
        if (gPlayers[p].controller == 5) continue;
        if (!controlScrActive[p]) {
            if ((gGameState & 0x7c0000) && (gPlayers[p].edges & 2)) controlScrActive[p] = 1;
            continue;
        }
        if (!prevControlScrActive[p]) {
            gControlSelect[p] = 0;
            gControlFirst[p] = 0;
            if (gPlayers[p].save)
                memcpy(gNewControls[gPlayers[p].controller],(*gPlayers[p].save)->controls,11);
            else
                memcpy(gNewControls[gPlayers[p].controller],gJustPlayers[gPlayers[p].controller]->controls,11);
            for (i = 0; i < 11; i++) { gConflict[p][i] = 0; gConflictExists[p] = 0; }
            if (gGameState & 0x7c03fffe) initButtons(p);
        }
        updateButtons(p);
        controls = gNewControls[gPlayers[p].controller];
        while ((gConflictExists[p] == 1 && !gConflict[p][gControlSelect[p]] && gControlSelect[p] != 12) ||
            !validOption(p,gControlSelect[p]) || (!PS_PakRumble(gPlayers[p].controller) && gControlSelect[p] == 11)) {
            gControlSelect[p]++;
            if (gControlSelect[p] > 12) gControlSelect[p] = 0;
        }
        if (gPlayers[p].edges & 0x400) {
            AudPlayEffect(8,0,1,0);
            do {
                gControlSelect[p]--;
                if (gControlSelect[p] < 0) gControlSelect[p] = 12;
            } while ((gConflictExists[p] == 1 && !gConflict[p][gControlSelect[p]] && gControlSelect[p] != 12) ||
                !validOption(p,gControlSelect[p]) || (!PS_PakRumble(gPlayers[p].controller) && gControlSelect[p] == 11));
        }
        if (gPlayers[p].edges & 0x800) {
            AudPlayEffect(2,0,1,0);
            do {
                gControlSelect[p]++;
            if (gControlSelect[p] > 12) gControlSelect[p] = 0;
            } while ((gConflictExists[p] == 1 && !gConflict[p][gControlSelect[p]] && gControlSelect[p] != 12) ||
                !validOption(p,gControlSelect[p]) || (!PS_PakRumble(gPlayers[p].controller) && gControlSelect[p] == 11));
        }
        maxFirstItem = 0;
        menuItem = 0;
        for (i = 12; i >= 0; i--) {
            if (validOption(p,i)) {
                menuItem++;
                if (menuItem == gControlMenuSize) {
                    maxFirstItem = i;
                    break;
                }
            }
        }
        menuItem = 0;
        for (i = 0; i < gControlSelect[p]; i++) {
            if (validOption(p,i)) menuItem++;
        }
        if (gControlFirst[p] < menuItem - (gControlMenuSize - 2)) gControlFirst[p] = menuItem - (gControlMenuSize - 2);
        if (gControlFirst[p] > menuItem - 1) gControlFirst[p] = menuItem - 1;
        if (gControlFirst[p] > maxFirstItem) gControlFirst[p] = maxFirstItem;
        if (gControlFirst[p] < 0) gControlFirst[p] = 0;
        if (gControlSelect[p] == 12) {
            if (gPlayers[p].edges & 2) {
                AudPlayEffect(5,0,1,0);
                SetDefaultControls(controls);
                (*gPlayers[p].save)->rumble = 1;
            }
        } else if (gPlayers[p].edges & 0x1000) {
            AudPlayEffect(3,0,1,0);
            if (gControlSelect[p] == 11) {
                signed char rumble;
                rumble = (*gPlayers[p].save)->rumble;
                rumble--;
                if (rumble < 0) rumble = 2;
                (*gPlayers[p].save)->rumble = rumble;
                gControlsSet[gPlayers[p].controller] = 1;
            } else {
                do {
                    controls[gControlSelect[p]]--;
                    if (controls[gControlSelect[p]] < 0) controls[gControlSelect[p]] = 25;
                } while (!validControl(p));
            }
        } else if (gPlayers[p].edges & 0x2000) {
            AudPlayEffect(6,0,1,0);
            if (gControlSelect[p] == 11) {
                signed char rumble;
                rumble = (*gPlayers[p].save)->rumble;
                rumble++;
                if (rumble > 2) rumble = 0;
                (*gPlayers[p].save)->rumble = rumble;
                gControlsSet[gPlayers[p].controller] = 1;
            } else {
                do {
                    controls[gControlSelect[p]]++;
                    if (controls[gControlSelect[p]] >= 26) controls[gControlSelect[p]] = 0;
                } while (!validControl(p));
            }
        }
        for (i = 0; i < 11; i++) { gConflict[p][i] = 0; gConflictExists[p] = 0; }
        for (i = 0; i < 10; i++) {
            for (j = i + 1; j < 11; j++) {
                if ((controls[i] == controls[j] ||
                    (controls[i] == 21 && controls[j] == 8) ||
                    (controls[i] == 8 && controls[j] == 21) ||
                    (controls[i] == 22 && controls[j] == 9) ||
                    (controls[i] == 9 && controls[j] == 22)) && controls[i] != 19 && controls[i] != 25) {
                    gConflict[p][i] = 1;
                    gConflict[p][j] = 1;
                    gConflictExists[p] = 1;
                }
            }
        }
        forcedExit = 0;
        if ((gGameState & 0x7c03fffe) && gPlayers[p].save && (*gPlayers[p].save)->fileh &&
            !PS_PakInserted((*(*gPlayers[p].save)->fileh)->pak)) forcedExit = 1;
        if (forcedExit) {
            controlScrActive[p] = 0;
            uninitButtons(p);
            exitControlScreen = 2;
        } else if (gPlayers[p].edges & 5) {
            signed char conflict = 0;
            for (i = 0; i < 11; i++) {
                if (gConflict[p][i] == 1) conflict = 1;
            }
            if (conflict == 1) AudPlayEffect(4,0,1,0);
            else {
                int playerCont = gPlayers[p].controller;
                PlayStartABSound(gPlayers[p].edges);
                if (playerCont != 5) {
                    if (gPlayers[p].save) {
                        if (memcmp(gNewControls[playerCont],(*gPlayers[p].save)->controls,11)) gControlsSet[playerCont] = 1;
                    } else {
                        if (memcmp(gNewControls[playerCont],gJustPlayers[playerCont]->controls,11)) gControlsSet[playerCont] = 1;
                    }
                    for (i = 0; i < 11; i++) {
                        if ((unsigned)i > 1 && i != 2) {
                            if (gNewControls[playerCont][i] == 21) gNewControls[playerCont][i] = 8;
                            if (gNewControls[playerCont][i] == 22) gNewControls[playerCont][i] = 9;
                        }
                    }
                }
                SetControls(0);
                controlScrActive[p] = 0;
                uninitButtons(p);
                exitControlScreen = 2;
            }
        }
        prevControlScrActive[p] = controlScrActive[p];
    }
    for (p = 0; p < gNumWindows; p++) {
        if (controlScrActive[p] == 1) exitControlScreen = 0;
    }
    if (gSoftReset || exitControlScreen) {
        cleanupControlScreen();
        if (!(gGameState & 0x7c03fffe)) {
            RemovePanes(0);
            setPauseState(1);
        }
    }
}

void cleanupControlScreen(void)
{
    int playerIdx;
    for (playerIdx = 0; playerIdx < 4; playerIdx++) {
        controlScrActive[playerIdx] = 0;
        prevControlScrActive[playerIdx] = 0;
        uninitButtons(playerIdx);
    }
    if (blit) {
        RemoveBlit((_tBlit *)blit);
        blit = 0;
    }
    inited = 0;
}

static int animControls(_tBlit_2 *blt)
{
    int id;
    int conflict;
    int uninited;
    int p;
    int val;
    int action;
    signed short i;
    signed short j;
    signed short screenPos[2];
    float gTextPosScale;
    static int idMap[28] = { 10, 0, 1, 11, 12, 2, 3, 10, 5, 4, 6, 7, 14, 13, 10, 10, 10, 10, 10, 15, 8, 5, 4, 10, 10, 10, 9, 10 };
    id = blt->animID & 15;
    conflict = (blt->animID >> 4) & 15;
    uninited = (blt->animID >> 8) & 15;
    p = (blt->animID >> 12) & 15;
    val = ((blt->animID >> 16) & 255) - 1;
    action = gControlFirst[p] + id - 1;
    gTextPosScale = gButtonScale;
    if (gNumWindows > 1) gTextPosScale *= 0.5f;
    for (i = 0, j = 0; i < id; j++) {
        int valid = validOption(p, j + gControlFirst[p]);
        if (!valid) action++;
        if (valid) i++;
    }
    if (Hidden(blt, gPlayers[p].controller == 5 ||
        (conflict && !gConflict[p][action]) || action >= 11 || !controlScrActive[p] || id > gControlMenuSize))
        return 1;
    if (uninited) {
        if (gNumWindows > 1) {
            RenameBlit((_tBlit *)blt, "CONTROLS_SM", 0);
            blt->width = 32;
            blt->height = 512;
        }
        if (buttonsInited[p]) {
            ZOID_ProjectWindow(p, buttonInfo[p][action].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
            screenPos[0] /= 2;
            screenPos[1] *= 0.5f;
            blt->x = screenPos[0] + 300.0f * (55.0f * gTextPosScale) / buttonZ;
            blt->y = screenPos[1] - (blt->width / 2) / 2.0f;
            blt->alpha = buttonInfo[p][action].alpha;
        } else {
            blt->x = gPlayerRects[gNumWindows - 1][p].cx + 20;
            blt->y = gPlayerRects[gNumWindows - 1][p].y + 24 + (id - 1) * 16;
        }
        if (conflict) blt->x += gNumWindows == 1 ? 20 : 10;
        UpdateBlit((_tBlit *)blt);
        blt->animID &= 0x00fff0ff;
        if (conflict) {
            blt->animID = (blt->animID & 0xffff) | 0x1c0000;
            SelectBlit((_tBlit *)blt, idMap[27], blt->width, blt->width);
        }
    }
    if (buttonsInited[p]) {
        ZOID_ProjectWindow(p, buttonInfo[p][action].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
        blt->x = screenPos[0] / 2 + 300.0f * (75.0f * gTextPosScale) / buttonZ;
        blt->y = screenPos[1] / 2.0f - (blt->width / 2) / 2.0f;
        if (conflict) blt->x += gNumWindows == 1 ? 20 : 10;
        blt->alpha = buttonInfo[p][action].alpha > 254 ? 254 : buttonInfo[p][action].alpha;
        if (Hidden(blt, !blt->alpha)) return 1;
        UpdateBlit((_tBlit *)blt);
    } else {
        blt->alpha = 254;
        UpdateBlit((_tBlit *)blt);
    }
    if (conflict) return 1;
    if (val != gNewControls[gPlayers[p].controller][action]) {
        val = gNewControls[gPlayers[p].controller][action];
        blt->animID = (blt->animID & 0xffff) | (((val + 1) & 255) << 16);
        if (val == 25 && (action == 0 || action == 1)) val++;
        SelectBlit((_tBlit *)blt, idMap[val], blt->width, blt->width);
    }
    return 1;
}

static int animControls2(_tBlit_2 *blt)
{
    int conflict = (blt->animID >> 4) & 15;
    int uninited = (blt->animID >> 8) & 15;
    int p = (blt->animID >> 12) & 15;
    if (Hidden(blt, gPlayers[p].controller == 5 || controlScrActive[p] == 1))
        return 1;
    if (uninited) {
        blt->x = gPlayerRects[gNumWindows - 1][p].cx + 30;
        blt->y = gPlayerRects[gNumWindows - 1][p].y + 32;
        UpdateBlit((_tBlit *)blt);
        blt->animID &= 0x00fff0ff;
        if (conflict) {
            blt->animID = (blt->animID & 0xffff) | 0x1c0000;
            SelectBlit((_tBlit *)blt, 0, blt->width, blt->width);
        }
        UpdateBlit((_tBlit *)blt);
    }
    return 1;
}

static int animText(void)
{
    int menuY;
    int i;
    int p;
    int x;
    int y;
    unsigned char *str;
    unsigned char str1[64];
    unsigned char pName[14];
    int itemsDrawn;
    HAL_SetTextDepth(0.0f);
    for (p = 0; p < gNumWindows; p++) {
        if (buttonsInited[p] == 1) {
            animButtonText(p);
        } else {
            HAL_SetFont(11);
            if (gPlayers[p].controller != 5 &&
                (controlScrActive[p] == 1 || (gGameState & 0x7c0000))) {
                y = gPlayerRects[gNumWindows - 1][p].y + 10;
                x = gPlayerRects[gNumWindows - 1][p].cx;
                TB_SetTextColor(playerTextColor[p]);
                if (!gControllerData[p].attached) {
                    TB_Sprintf(str1, "CONTROLLER %d %s", p + 1, gText.single[242]);
                } else {
                    TB_EllipseText(pName, (*gPlayers[p].save)->name, 69);
                    TB_Sprintf(str1, gText.single[37], pName);
                }
                TB_DrawText(TB_CenterText(str1, x), y, str1);
                TB_GetFontLineHeight();
                if (!controlScrActive[p]) {
                    if (gGameState & 0x7c0000) {
                        TB_SetTextColor(1);
                        TB_SetJust(1, 1);
                        TB_DrawTextScrollBox(gPlayerRects[gNumWindows - 1][p].cx,
                            gPlayerRects[gNumWindows - 1][p].cy,
                            gPlayerRects[gNumWindows - 1][p].w / 2,
                            gPlayerRects[gNumWindows - 1][p].h, -1, 0,
                            gText.single[38]);
                        TB_SetJust(0, 3);
                    }
                } else {
                    x = gPlayerRects[gNumWindows - 1][p].cx + 10;
                    menuY = gPlayerRects[gNumWindows - 1][p].y + 24;
                    itemsDrawn = 0;
                    for (i = 0; i < 13; i++) {
                        if (i >= gControlFirst[p] && validOption(p, i)) {
                            if (!PS_PakRumble(gPlayers[p].controller) && i == 11)
                                TB_SetTextColor(3);
                            else
                                TB_SetTextColor(i == gControlSelect[p] ? 22 : 1);
                            str = gText.taba[gText.tab[20] + i];
                            if (i == 12)
                                TB_DrawText(TB_CenterText(str, gPlayerRects[gNumWindows - 1][p].cx), menuY, str);
                            else
                                TB_DrawText(TB_RightText(str, x), menuY, str);
                            if (i == 11) {
                                if (PS_PakRumble(gPlayers[p].controller))
                                    str = gText.taba[gText.tab[18] + (*gPlayers[p].save)->rumble];
                                else
                                    str = gText.taba[gText.tab[17]];
                                TB_DrawText(x + 20, menuY, str);
                            }
                            menuY += 16;
                            if (++itemsDrawn >= gControlMenuSize) break;
                        }
                    }
                }
            }
        }
    }
    HAL_SetTextDepth(-1.0f);
    return 1;
}

static void animButtonText(signed char p)
{
    int i;
    int x;
    int y;
    unsigned char *str;
    signed short screenPos[2];
    float gTextPosScale;
    gTextPosScale = gButtonScale;
    if (gNumWindows > 1) gTextPosScale *= 0.5f;
    HAL_SetFont(11);
    TB_SetJust(1, 1);
    if (gPlayers[p].controller != 5 && (controlScrActive[p] == 1 || (gGameState & 0x7c0000))) {
        ZOID_ProjectWindow(p, buttonInfo[p][14].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
        x = screenPos[0] / 2;
        y = screenPos[1] / 2.0f;
        if (gNumWindows > 1) HAL_SetFont(11);
        else HAL_SetFont(13);
        TB_SetTextColor(1);
        TB_DrawEllipseText(x, y, gNumWindows == 1 ? 150 : 69, (*gPlayers[p].save)->name);
        TB_SetJust(0, 1);
        if (!controlScrActive[p]) {
            if (gGameState & 0x7c0000) {
                if (gNumWindows == 1) HAL_SetFont(10);
                else HAL_SetFont(11);
                TB_SetTextColor(1);
                TB_SetJust(1, 1);
                TB_DrawTextScrollBox(gPlayerRects[gNumWindows-1][p].cx,
                    gPlayerRects[gNumWindows-1][p].cy,
                    gPlayerRects[gNumWindows-1][p].w,
                    gPlayerRects[gNumWindows-1][p].h, -1, 0, gText.single[38]);
                TB_SetJust(0, 3);
            }
        } else {
            if (gNumWindows == 1) HAL_SetFont(11);
            else HAL_SetFont(12);
            for (i = 0; i < 13; i++) {
                if (buttonInfo[p][i].angle > 0.6981317f && buttonInfo[p][i].angle < 2.443461f) continue;
                if (!buttonInfo[p][i].alpha) continue;
                HAL_SetTextTranslucency(buttonInfo[p][i].alpha);
                HAL_SetMenuTextDepth(buttonInfo[p][i].mat.mat3.pos[2] - 10.0f);
                ZOID_ProjectWindow(p, buttonInfo[p][i].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
                if (i == 11)
                    x = screenPos[0] / 2 - (300.0f * (35.0f * gTextPosScale)) / buttonZ;
                else if (i == 12)
                    x = screenPos[0] / 2 - (300.0f * (55.0f * gTextPosScale)) / buttonZ;
                else
                    x = screenPos[0] / 2 - (300.0f * (45.0f * gTextPosScale)) / buttonZ;
                y = screenPos[1] / 2.0f;
                if (buttonInfo[p][i].angle < 1.5707964f) {
                    anon_c_rgba_4 color1, color2;
                    color2 = flashColors[1];
                    color1 = flashColors[0];
                    TB_FlashTextColors(&color1, &color2);
                    TB_SetTextColor(1);
                    if (!PS_PakRumble(gPlayers[p].controller) && i == 11) TB_SetTextColor(3);
                } else {
                    anon_c_rgba_4 color1, color2;
                    color2 = flashColors[1];
                    color1 = flashColors[0];
                    TB_FlashTextColors(&color1, &color2);
                    TB_SetTextColor(22);
                }
                str = gText.taba[gText.tab[20] + i];
                TB_DrawText(x, y, str);
                if (i == 11) {
                    ZOID_ProjectWindow(p, buttonInfo[p][13].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
                    x = screenPos[0] / 2 - (300.0f * (25.0f * gTextPosScale)) / buttonZ;
                    y = screenPos[1] / 2.0f;
                    if (PS_PakRumble(gPlayers[p].controller))
                        str = gText.taba[gText.tab[18] + (*gPlayers[p].save)->rumble];
                    else
                        str = gText.taba[gText.tab[18]];
                    TB_DrawText(x, y, str);
                }
            }
        }
    }
    TB_SetJust(0, 3);
    HAL_SetTextTranslucency(-1.0f);
    HAL_SetTextDepth(0.0f);
}

static void calcControlMenuMaxSize(void)
{
    int rectHeight = gPlayerRects[gNumWindows - 1][0].h;
    int i;
    int gControlMenuCount;
    HAL_SetFont(11);
    gControlMenuSize = (rectHeight - 24) / 16;
    gControlMenuCount = 2;
    for (i = 0; i < 13; i++) {
        if (validOption(0, i)) gControlMenuCount++;
    }
    if (gControlMenuSize > gControlMenuCount || (gGameState & 0x7c03fffe))
        gControlMenuSize = gControlMenuCount;
}

static int validOption(signed char p, int option)
{
    return 1;
}

static int validControl(signed char p)
{
    signed short cur_action = gControlSelect[p];
    signed short cur_control = gNewControls[gPlayers[p].controller][cur_action];
    if (!gControlValid[cur_control]) return 0;
    if (cur_action != 2 && (cur_control == 19 || cur_control == 20)) return 0;
    if (cur_action == 2 && cur_control != 19 && cur_control != 20) return 0;
    return 1;
}

static void initButtons(signed char p)
{
    anon_c_rgba_4 buttonColor = {255, 255, 255, 255};
    anon_name_width_30 *tex;
    unsigned short texIdx;
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 18; buttonIdx++) {
        if (buttonInfo[p][buttonIdx].objIdx == -1) {
            CopyUV(unitmatrix, buttonInfo[p][buttonIdx].mat.uvs);
            buttonInfo[p][buttonIdx].objIdx = MBOX_NewObject(MBOX_FindObject_WORK(buttonNames[buttonInfo[p][buttonIdx].buttonType], 0, gNumModels - 1, 1), &buttonInfo[p][buttonIdx].mat, -1,
                (buttonInfo[p][buttonIdx].buttonType == 0 || buttonInfo[p][buttonIdx].buttonType == 1 || buttonInfo[p][buttonIdx].buttonType == 2 || buttonInfo[p][buttonIdx].buttonType == 9 || buttonInfo[p][buttonIdx].buttonType == 10) ? 0x42000 : 0);
            ZOID_SetColor(buttonInfo[p][buttonIdx].objIdx, &buttonColor);
            MBOX_HideObject(buttonInfo[p][buttonIdx].objIdx, 1, 15);
            MBOX_ShowObject(buttonInfo[p][buttonIdx].objIdx, 0, 1 << p);
        }
    }
    tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[p][14].objIdx, tex, -1);
    CopyUV(unitmatrix, buttonInfo[p][14].mat.uvs);
    WPitchUV(-1.5707964f, buttonInfo[p][14].mat.uvs);
    buttonInfo[p][14].mat.uvs[3][0] = 0.0f;
    buttonInfo[p][14].mat.uvs[3][1] = gNumWindows > 1 ? 105.0f : 115.0f;
    buttonInfo[p][14].mat.uvs[3][2] = 200.0f;
    buttonInfo[p][14].mat.uvs[3][0] = buttonInfo[p][14].mat.uvs[3][0] * 1.0f;
    buttonInfo[p][14].mat.uvs[3][1] = buttonInfo[p][14].mat.uvs[3][1] * 1.0f;

    tex = MBOX_FindTexture_WORK("CURSOR", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[p][15].objIdx, tex, -1);
    tex = MBOX_FindTexture_WORK("B1", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[p][16].objIdx, tex, -1);
    CopyUV(unitmatrix, buttonInfo[p][16].mat.uvs);
    WPitchUV(-1.5707964f, buttonInfo[p][16].mat.uvs);
    buttonInfo[p][16].mat.uvs[3][0] = -75.1f;
    buttonInfo[p][16].mat.uvs[3][1] = 4.0f + (gNumWindows > 1 ? 50.2f : 55.2f);
    buttonInfo[p][16].mat.uvs[3][2] = 100.0f;
    buttonInfo[p][16].mat.uvs[3][0] = buttonInfo[p][16].mat.uvs[3][0] * 1.0f;
    buttonInfo[p][16].mat.uvs[3][1] = buttonInfo[p][16].mat.uvs[3][1] * 1.0f;
    buttonInfo[p][16].mat.uvs[3][2] = buttonInfo[p][16].mat.uvs[3][2] * 1.0f;

    ScaleUV(buttonInfo[p][16].mat.uvs, buttonInfo[p][16].mat.uvs, 0.5f);
    CopyUV(unitmatrix, buttonInfo[p][17].mat.uvs);
    WPitchUV(-1.5707964f, buttonInfo[p][17].mat.uvs);
    buttonInfo[p][17].mat.uvs[3][0] = -87.1f;
    buttonInfo[p][17].mat.uvs[3][1] = 4.0f + (gNumWindows > 1 ? 50.2f : 55.2f);
    buttonInfo[p][17].mat.uvs[3][2] = 100.0f;
    buttonInfo[p][17].mat.uvs[3][0] = buttonInfo[p][17].mat.uvs[3][0] * 1.0f;
    buttonInfo[p][17].mat.uvs[3][1] = buttonInfo[p][17].mat.uvs[3][1] * 1.0f;
    buttonsInited[p] = 1;
    buttonsJustInited[p] = 1;
    ZeroCameras();
}

static void uninitButtons(signed char p)
{
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 18; buttonIdx++) {
        if (buttonInfo[p][buttonIdx].objIdx != -1) {
            ZOID_RemoveObject(buttonInfo[p][buttonIdx].objIdx);
            buttonInfo[p][buttonIdx].objIdx = -1;
        }
    }
    buttonsInited[p] = 0;
}

static void updateButtons(signed char p)
{
    signed char clearScrollSpeed = 1;
    int buttonIdx;
    int i;
    float prevPos[3];
    float buttonY;
    float selectedPos;
    int gControlMenuCount;
    int gWhichControl;
    anon_name_width_30 *tex;
    unsigned short texIdx;
    anon_c_rgba_4 buttonColor = { {255,255,255,255} };
    if (buttonsInited[p]) {
        gControlMenuCount = 0;
        gWhichControl = 0;
        for (i = 0; i < 13; i++) {
            if (validOption(p,i)) {
                if (i < gControlSelect[p]) gWhichControl++;
                gControlMenuCount++;
            }
        }
        if (gButtonScale * ((gControlMenuCount - 1) * gButtonSpace) > maxY - minY)
            selectedPos = maxY - gWhichControl * (maxY - minY) / (gControlMenuCount - 1);
        else
            selectedPos = maxY - gButtonScale * (gWhichControl * gButtonSpace);
        memcpy(prevPos,buttonInfo[p][15].mat.mat3.pos,12);
        buttonInfo[p][15].angle += 6.2831855f * gFrameSeconds;
        if (buttonInfo[p][15].angle > 6.2831855f) buttonInfo[p][15].angle -= 6.2831855f;
        buttonInfo[p][15].mat.mat3.pos[0] = buttonX * cursorZ / buttonZ + -110.0f * gButtonScale;
        buttonInfo[p][15].mat.mat3.pos[1] = selectedPos * cursorZ / buttonZ;
        buttonInfo[p][15].mat.mat3.pos[2] = cursorZ;
        if (!buttonsJustInited[p]) {
            if (gButtonScale * (60.0 * fabs(buttonInfo[p][15].mat.mat3.pos[1] - prevPos[1])) / 10.0 > cursorSpeed[p]) {
                cursorSpeed[p] = gButtonScale * (60.0 * fabs(buttonInfo[p][15].mat.mat3.pos[1] - prevPos[1])) / 10.0;
                cursorSpeed[p] = cursorSpeed[p] < 540.0f * gButtonScale ? cursorSpeed[p] : 540.0f * gButtonScale;
                cursorSpeed[p] = cursorSpeed[p] > 180.0f * gButtonScale ? cursorSpeed[p] : 180.0f * gButtonScale;
            }
            if (buttonInfo[p][15].mat.mat3.pos[1] > prevPos[1] + gFrameSeconds * cursorSpeed[p])
                buttonInfo[p][15].mat.mat3.pos[1] = prevPos[1] + gFrameSeconds * cursorSpeed[p];
            else if (buttonInfo[p][15].mat.mat3.pos[1] < prevPos[1] - gFrameSeconds * cursorSpeed[p])
                buttonInfo[p][15].mat.mat3.pos[1] = prevPos[1] - gFrameSeconds * cursorSpeed[p];
            else
                cursorSpeed[p] = 180.0f * gButtonScale;
        }
        ScaleUV(unitmatrix,buttonInfo[p][15].mat.mat3.uvs,0.4f * gButtonScale);
        WRollUV(buttonInfo[p][15].angle,buttonInfo[p][15].mat.mat3.uvs);
        WPitchUV(1.5707964f,buttonInfo[p][15].mat.mat3.uvs);
        buttonY = selectedPos + gButtonScale * (gWhichControl * gButtonSpace);
        for (buttonIdx = 0; buttonIdx < 13; buttonIdx++) {
            if (buttonIdx == gControlSelect[p]) {
                if (buttonInfo[p][buttonIdx].angle < 3.1415927f)
                    buttonInfo[p][buttonIdx].angle += 12.566371f * gFrameSeconds;
                if (buttonInfo[p][buttonIdx].angle > 3.1415927f)
                    buttonInfo[p][buttonIdx].angle = 3.1415927f;
                if (buttonsJustInited[p]) buttonInfo[p][buttonIdx].angle = 3.1415927f;
            } else {
                if (buttonInfo[p][buttonIdx].angle > 0.17453292f)
                    buttonInfo[p][buttonIdx].angle -= 12.566371f * gFrameSeconds;
                if (buttonInfo[p][buttonIdx].angle < 0.17453292f)
                    buttonInfo[p][buttonIdx].angle = 0.17453292f;
                if (buttonsJustInited[p]) buttonInfo[p][buttonIdx].angle = 0.17453292f;
            }
            memcpy(prevPos,buttonInfo[p][buttonIdx].mat.mat3.pos,12);
            buttonInfo[p][buttonIdx].mat.mat3.pos[0] = buttonX + -40.0f * gButtonScale;
            buttonInfo[p][buttonIdx].mat.mat3.pos[1] = buttonY;
            buttonInfo[p][buttonIdx].mat.mat3.pos[2] = buttonZ;
            if (buttonIdx == 11) buttonInfo[p][buttonIdx].mat.mat3.pos[0] -= 20.0f * gButtonScale;
            else if (buttonIdx == 12) buttonInfo[p][buttonIdx].mat.mat3.pos[0] += 20.0f * gButtonScale;
            if (!buttonsJustInited[p]) {
                if (60.0 * fabs(buttonInfo[p][buttonIdx].mat.mat3.pos[1] - prevPos[1]) / 10.0 > scrollSpeed[p]) {
                    scrollSpeed[p] = 60.0 * fabs(buttonInfo[p][buttonIdx].mat.mat3.pos[1] - prevPos[1]) / 10.0;
                    scrollSpeed[p] = scrollSpeed[p] < 540.0f * gButtonScale ? scrollSpeed[p] : 540.0f * gButtonScale;
                    scrollSpeed[p] = scrollSpeed[p] > 180.0f * gButtonScale ? scrollSpeed[p] : 180.0f * gButtonScale;
                }
                if (buttonInfo[p][buttonIdx].mat.mat3.pos[1] > prevPos[1] + gFrameSeconds * scrollSpeed[p]) {
                    buttonInfo[p][buttonIdx].mat.mat3.pos[1] = prevPos[1] + gFrameSeconds * scrollSpeed[p];
                    clearScrollSpeed = 0;
                } else if (buttonInfo[p][buttonIdx].mat.mat3.pos[1] < prevPos[1] - gFrameSeconds * scrollSpeed[p]) {
                    buttonInfo[p][buttonIdx].mat.mat3.pos[1] = prevPos[1] - gFrameSeconds * scrollSpeed[p];
                    clearScrollSpeed = 0;
                }
            }
            ScaleUV(unitmatrix,buttonInfo[p][buttonIdx].mat.mat3.uvs,gButtonScale);
            WPitchUV(buttonInfo[p][buttonIdx].angle - 1.5707964f,buttonInfo[p][buttonIdx].mat.mat3.uvs);
            if (buttonInfo[p][buttonIdx].angle > 1.5707964f)
                tex = MBOX_FindTexture_WORK("BUTTON_SELECT",&texIdx,0,gNumModels-1,1);
            else
                tex = MBOX_FindTexture_WORK("BUTTON",&texIdx,0,gNumModels-1,1);
            ZOID_SetTexture(buttonInfo[p][buttonIdx].objIdx,tex,-1);
            if (buttonInfo[p][buttonIdx].mat.mat3.pos[1] > maxY + gButtonSpace * gButtonScale ||
                buttonInfo[p][buttonIdx].mat.mat3.pos[1] < minY - gButtonSpace * gButtonScale)
                buttonInfo[p][buttonIdx].alpha = 0;
            else if (buttonInfo[p][buttonIdx].mat.mat3.pos[1] > maxY)
                buttonInfo[p][buttonIdx].alpha = 80;
            else if (buttonInfo[p][buttonIdx].mat.mat3.pos[1] < minY)
                buttonInfo[p][buttonIdx].alpha = 80;
            else
                buttonInfo[p][buttonIdx].alpha = 255;
            if (!validOption(p,buttonIdx)) buttonInfo[p][buttonIdx].alpha = 0;
            buttonColor.c[3] = buttonInfo[p][buttonIdx].alpha;
            ZOID_SetColor(buttonInfo[p][buttonIdx].objIdx,&buttonColor);
            if (buttonIdx == 11) {
                buttonInfo[p][13].mat.mat3.pos[0] = buttonInfo[p][buttonIdx].mat.mat3.pos[0];
                buttonInfo[p][13].mat.mat3.pos[1] = buttonInfo[p][buttonIdx].mat.mat3.pos[1];
                buttonInfo[p][13].mat.mat3.pos[2] = buttonInfo[p][buttonIdx].mat.mat3.pos[2];
                buttonInfo[p][13].mat.mat3.pos[0] += 180.0f * gButtonScale;
                ScaleUV(unitmatrix,buttonInfo[p][13].mat.mat3.uvs,gButtonScale);
                WPitchUV(buttonInfo[p][buttonIdx].angle - 1.5707964f,buttonInfo[p][13].mat.mat3.uvs);
                ZOID_SetTexture(buttonInfo[p][13].objIdx,tex,-1);
                ZOID_SetColor(buttonInfo[p][13].objIdx,&buttonColor);
            }
            if (!validOption(p,buttonIdx)) {
                MBOX_HideObject(buttonInfo[p][buttonIdx].objIdx,1,15);
                if (buttonIdx == 11) MBOX_HideObject(buttonInfo[p][13].objIdx,1,15);
            } else if (!buttonInfo[p][buttonIdx].alpha) {
                MBOX_HideObject(buttonInfo[p][buttonIdx].objIdx,1,15);
                if (buttonIdx == 11) MBOX_HideObject(buttonInfo[p][13].objIdx,1,15);
                buttonY -= gButtonSpace * gButtonScale;
            } else {
                MBOX_ShowObject(buttonInfo[p][buttonIdx].objIdx,0,1 << p);
                if (buttonIdx == 11) MBOX_ShowObject(buttonInfo[p][13].objIdx,0,1 << p);
                buttonY -= gButtonSpace * gButtonScale;
            }
        }
        if (clearScrollSpeed == 1) scrollSpeed[p] = 0.0f;
        buttonsJustInited[p] = 0;
    }
}

#pragma force_active reset
