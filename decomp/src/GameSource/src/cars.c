/* cars.c - car definition tables (physics, transmissions, tyres, suspension, cosmetics). Matches.
   Generated with tools/unit2c.py from rush_d_dwarf.c and the original data. */

// total size: 0xB4
struct _tCar {
    // Members
    float i[3]; // offset 0x0, size 0xC
    float invi[3]; // offset 0xC, size 0xC
    float springrate[4]; // offset 0x18, size 0x10
    float farspringrate; // offset 0x28, size 0x4
    float rarspringrate; // offset 0x2C, size 0x4
    float cdamping[4]; // offset 0x30, size 0x10
    float rdamping[4]; // offset 0x40, size 0x10
    float steerratio; // offset 0x50, size 0x4
    float swtpg; // offset 0x54, size 0x4
    float srefpcybo2; // offset 0x58, size 0x4
    float rollresist; // offset 0x5C, size 0x4
    // total size: 0x18
    struct {
        // Members
        float tradius; // offset 0x0, size 0x4
        float springK; // offset 0x4, size 0x4
        float rubdamp; // offset 0x8, size 0x4
        float cstiff; // offset 0xC, size 0x4
        float cfmax; // offset 0x10, size 0x4
        float invmi; // offset 0x14, size 0x4
    } * tires[4]; // offset 0x60, size 0x10
    float tireR[4][3]; // offset 0x70, size 0x30
    float enginvmi; // offset 0xA0, size 0x4
    float dwratio; // offset 0xA4, size 0x4
    float clutchmaxt; // offset 0xA8, size 0x4
    float upshiftangvel; // offset 0xAC, size 0x4
    float downshiftangvel; // offset 0xB0, size 0x4
};

// total size: 0x24
struct _tSusp {
    // Members
    float fwdforcemin; // offset 0x0, size 0x4
    float fwdforcerange; // offset 0x4, size 0x4
    float fwdangle; // offset 0x8, size 0x4
    float bckforcemin; // offset 0xC, size 0x4
    float bckforcerange; // offset 0x10, size 0x4
    float bckangle; // offset 0x14, size 0x4
    float sideforcemin; // offset 0x18, size 0x4
    float sideforcerange; // offset 0x1C, size 0x4
    float sideangle; // offset 0x20, size 0x4
};

const signed short stdtorquecurve[10][12] = {
    30, -20, -26, -32, -40, -51, -64, -80, -80, -80, -80, -80, 40, 10, -20, -26, -32, -40, -51,
    -64, -80, -80, -80, -80, 50, 34, 35, 33, 25, -32, -40, -51, -64, -80, -80, -80, 70, 65, 65, 70,
    65, 46, 14, -27, -34, -52, -80, -80, 90, 92, 96, 107, 104, 83, 45, -6, -29, -50, -63, -80, 110,
    120, 127, 143, 143, 120, 76, 14, -23, -48, -62, -80, 140, 150, 160, 180, 182, 158, 107, 36,
    -17, -46, -61, -80, 170, 190, 200, 216, 221, 195, 138, 57, -11, -44, -61, -80, 200, 220, 230,
    253, 260, 232, 169, 78, -5, -42, -60, -80, 230, 250, 270, 290, 300, 270, 200, 100, 0, -40, -60,
    -80
};

// total size: 0x18
const struct {
    // Members
    float tradius; // offset 0x0, size 0x4
    float springK; // offset 0x4, size 0x4
    float rubdamp; // offset 0x8, size 0x4
    float cstiff; // offset 0xC, size 0x4
    float cfmax; // offset 0x10, size 0x4
    float invmi; // offset 0x14, size 0x4
} frontTire = { 1.0f, 4080.0f, 200.0f, 16000.0f, 2.6f, 0.1f };

// total size: 0x18
const struct {
    // Members
    float tradius; // offset 0x0, size 0x4
    float springK; // offset 0x4, size 0x4
    float rubdamp; // offset 0x8, size 0x4
    float cstiff; // offset 0xC, size 0x4
    float cfmax; // offset 0x10, size 0x4
    float invmi; // offset 0x14, size 0x4
} backTire = { 1.0f, 4080.0f, 200.0f, 32000.0f, 2.8f, 0.1f };

const struct _tCar basic = {
    1527.0f, 1636.0f, 1000.0f, 0.00065487885f, 0.000611247f, 0.001f, -3600.0f, -3600.0f, -4200.0f,
    -4200.0f, -1000.0f, -1000.0f, -880.0f, -880.0f, -1100.0f, -1100.0f, -480.0f, -480.0f, -550.0f,
    -550.0f, 16.0f, 4.0f, 0.0135f, 75.0f, (void *)&frontTire, (void *)&frontTire,
    (void *)&backTire, (void *)&backTire, 2.75f, -0.8f, 3.925f, -2.75f, -0.8f, 3.925f, 2.75f,
    -0.75f, -3.925f, -2.75f, -0.75f, -3.925f, 2.5f, 3.3f, 1000.0f, 662.3519f, 429.35065f
};

const struct _tCar forward = {
    1527.0f, 1636.0f, 1000.0f, 0.00065487885f, 0.000611247f, 0.001f, -3600.0f, -3600.0f, -4200.0f,
    -4200.0f, -1000.0f, -1000.0f, -880.0f, -880.0f, -1100.0f, -1100.0f, -480.0f, -480.0f, -550.0f,
    -550.0f, 16.0f, 4.0f, 0.0135f, 75.0f, (void *)&frontTire, (void *)&frontTire,
    (void *)&backTire, (void *)&backTire, 2.75f, -0.8f, 4.425f, -2.75f, -0.8f, 4.425f, 2.75f,
    -0.75f, -3.425f, -2.75f, -0.75f, -3.425f, 2.5f, 3.3f, 1000.0f, 662.3519f, 429.35065f
};

const struct _tCar backward = {
    1527.0f, 1636.0f, 1000.0f, 0.00065487885f, 0.000611247f, 0.001f, -3600.0f, -3600.0f, -4200.0f,
    -4200.0f, -1000.0f, -1000.0f, -880.0f, -880.0f, -1100.0f, -1100.0f, -480.0f, -480.0f, -550.0f,
    -550.0f, 16.0f, 4.0f, 0.0135f, 75.0f, (void *)&frontTire, (void *)&frontTire,
    (void *)&backTire, (void *)&backTire, 2.75f, -0.8f, 3.425f, -2.75f, -0.8f, 3.425f, 2.75f,
    -0.75f, -4.425f, -2.75f, -0.75f, -4.425f, 2.5f, 3.3f, 1000.0f, 662.3519f, 429.35065f
};

const struct _tCar shortie = {
    1527.0f, 1636.0f, 1000.0f, 0.00065487885f, 0.000611247f, 0.001f, -3600.0f, -3600.0f, -4200.0f,
    -4200.0f, -1000.0f, -1000.0f, -880.0f, -880.0f, -1100.0f, -1100.0f, -480.0f, -480.0f, -550.0f,
    -550.0f, 16.0f, 4.0f, 0.0135f, 75.0f, (void *)&frontTire, (void *)&frontTire,
    (void *)&backTire, (void *)&backTire, 2.5f, -0.8f, 3.0f, -2.5f, -0.8f, 3.0f, 2.5f, -0.75f,
    -3.925f, -2.5f, -0.75f, -3.925f, 2.5f, 3.3f, 1000.0f, 662.3519f, 429.35065f
};

const struct _tCar stretch = {
    1527.0f, 1636.0f, 1000.0f, 0.00065487885f, 0.000611247f, 0.001f, -3600.0f, -3600.0f, -4200.0f,
    -4200.0f, -1000.0f, -1000.0f, -880.0f, -880.0f, -1100.0f, -1100.0f, -480.0f, -480.0f, -550.0f,
    -550.0f, 16.0f, 4.0f, 0.0135f, 75.0f, (void *)&frontTire, (void *)&frontTire,
    (void *)&backTire, (void *)&backTire, 3.25f, -0.8f, 4.75f, -3.25f, -0.8f, 4.75f, 3.25f, -0.75f,
    -4.425f, -3.25f, -0.75f, -4.425f, 2.5f, 3.3f, 1000.0f, 662.3519f, 429.35065f
};

const struct _tCar truck = {
    1527.0f, 1636.0f, 1000.0f, 0.00065487885f, 0.000611247f, 0.001f, -3600.0f, -3600.0f, -4200.0f,
    -4200.0f, -1000.0f, -1000.0f, -880.0f, -880.0f, -1100.0f, -1100.0f, -480.0f, -480.0f, -550.0f,
    -550.0f, 16.0f, 4.0f, 0.0135f, 75.0f, (void *)&frontTire, (void *)&frontTire,
    (void *)&backTire, (void *)&backTire, 3.0f, -0.8f, 5.5f, -3.0f, -0.8f, 5.5f, 3.75f, -0.75f,
    -5.6f, -3.75f, -0.75f, -5.6f, 2.5f, 3.3f, 1000.0f, 662.3519f, 429.35065f
};

// total size: 0x10
const struct {
    // Members
    float colfront; // offset 0x0, size 0x4
    float colrear; // offset 0x4, size 0x4
    float colside; // offset 0x8, size 0x4
    float colheight; // offset 0xC, size 0x4
} collsize[13] = {
    6.75f, 5.25f, 3.0f, 3.5f, 6.75f, 5.75f, 3.0f, 3.5f, 6.25f, 5.75f, 3.0f, 3.5f, 6.25f, 6.0f,
    3.0f, 3.5f, 6.0f, 6.75f, 3.0f, 3.5f, 6.75f, 6.75f, 3.0f, 3.5f, 5.75f, 6.0f, 3.0f, 3.5f, 5.75f,
    5.75f, 3.0f, 3.5f, 4.5f, 5.75f, 3.0f, 3.5f, 7.0f, 7.0f, 3.25f, 3.5f, 7.25f, 9.75f, 3.5f, 3.5f,
    5.5f, 6.75f, 3.0f, 3.5f, 6.0f, 5.75f, 3.0f, 3.5f
};

const float shadow_scale[14][4] = {
    0.15f, 0.25f, 0.525f, 0.4f, 0.25f, 0.25f, 0.525f, 0.425f, 0.25f, 0.25f, 0.45f, 0.4f, 0.25f,
    0.25f, 0.45f, 0.45f, 0.25f, 0.25f, 0.45f, 0.475f, 0.25f, 0.25f, 0.475f, 0.575f, 0.25f, 0.25f,
    0.4f, 0.4f, 0.25f, 0.25f, 0.4f, 0.425f, 0.25f, 0.25f, 0.4f, 0.4f, 0.25f, 0.25f, 0.45f, 0.5f,
    0.2f, 0.2f, 0.35f, 0.55f, 0.25f, 0.25f, 0.45f, 0.525f, 0.25f, 0.25f, 0.45f, 0.5f, 0.1f, 0.1f,
    0.5f, 0.25f
};

struct _tCar * gCarModels[13] = {
    (void *)&basic, (void *)&basic, (void *)&basic, (void *)&basic, (void *)&basic,
    (void *)&forward, (void *)&basic, (void *)&basic, (void *)&shortie, (void *)&stretch,
    (void *)&truck, (void *)&backward, (void *)&basic
};

char * CarNames[13] = {
    "CAR1", "CAR2", "CAR3", "CAR4", "CAR5", "CAR6", "CAR7", "CAR8", "CAR9", "CAR10", "CAR11",
    "CAR12", "CAR13"
};

char * CarTitles[13] = {
    "FORMULA 1", "8-BALL", "ROCKET ZX", "MAGNUM", "SUPER GT", "BRUISER", "LOCUST LX", "GX-2",
    "MINI XS", "VENOM", "CRUSHER", "EURO LX", "PANTHER"
};

float gCarBraking[13] = { 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f };

float gCarMass[13] = {
    102.48447f, 102.48447f, 102.48447f, 102.48447f, 102.48447f, 102.48447f, 102.48447f, 102.48447f,
    102.48447f, 102.48447f, 102.48447f, 102.48447f, 102.48447f
};

float gCollisionMass[14] = {
    2200.0f, 2200.0f, 2200.0f, 2200.0f, 2200.0f, 2200.0f, 2200.0f, 2200.0f, 2200.0f, 2200.0f,
    2200.0f, 2200.0f, 2200.0f, 4000.0f
};

float gCrashThreshold[13] = {
    390000.0f, 390000.0f, 390000.0f, 390000.0f, 390000.0f, 390000.0f, 390000.0f, 390000.0f,
    390000.0f, 390000.0f, 390000.0f, 390000.0f, 390000.0f
};

signed char gCarTrans[5][13] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0
};

float carTransGearRatio[8][6] = {
    -3.1f, 0.0f, 3.1f, 1.78f, 1.29f, 1.0f, -3.1f, 0.0f, 3.1f, 1.78f, 1.29f, 1.0f, -3.1f, 0.0f,
    3.1f, 1.78f, 1.3f, 1.02f, -3.1f, 0.0f, 3.1f, 1.78f, 1.3f, 1.02f, -3.1f, 0.0f, 3.1f, 1.78f,
    1.25f, 0.98f, -3.1f, 0.0f, 3.1f, 1.78f, 1.25f, 0.98f, -3.1f, 0.0f, 3.1f, 1.78f, 1.4f, 1.1f,
    -3.1f, 0.0f, 3.1f, 1.78f, 1.4f, 1.1f
};

float carTransTraction[8][6] = {
    1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.2f, 0.0f, 1.1f,
    1.05f, 1.0f, 1.0f, 1.2f, 0.0f, 1.1f, 1.05f, 1.0f, 1.0f, 0.9f, 0.0f, 0.9f, 0.95f, 1.0f, 1.0f,
    0.9f, 0.0f, 0.9f, 0.95f, 1.0f, 1.0f, 1.5f, 0.0f, 1.5f, 1.2f, 1.0f, 1.0f, 1.5f, 0.0f, 1.5f,
    1.2f, 1.0f, 1.0f
};

signed char gCarHandling[5][13] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0
};

signed char gCarEngines[5][13] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0
};

float carEngineTorque[9][3] = {
    2.2f, 2.3f, 2.4f, 2.2f, 2.3f, 2.4f, 2.2f, 2.3f, 2.4f, 2.23f, 2.33f, 2.43f, 2.26f, 2.36f, 2.46f,
    2.3f, 2.4f, 2.5f, 2.33f, 2.43f, 2.53f, 2.36f, 2.46f, 2.56f, 2.4f, 2.5f, 2.6f
};

float carEngineFwdForce[9][3] = {
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.03f, 1.03f, 1.03f, 1.06f, 1.06f, 1.06f,
    1.1f, 1.1f, 1.1f, 1.13f, 1.13f, 1.13f, 1.16f, 1.16f, 1.16f, 1.2f, 1.2f, 1.2f
};

signed char gCarTires[5][13] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0
};

float gTireDrift[5][3] = { 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f };

float gTireSideForce[5] = { 1.0f, 0.9f, 0.8f, 1.0f, 1.0f };

signed char gCarFrame[5][13] = {
    2, 2, 1, 3, 2, 3, 1, 2, 1, 2, 4, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0
};

float carDurability[6] = { 0.0f, 0.2f, 0.4f, 0.6f, 0.8f, 1.0f };

signed char gCarWings[5][13] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0
};

float gWingSizes[3] = { 30.0f, 40.0f, 50.0f };

float gFrontTireScale[5][13] = {
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.4f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f
};

float gRearTireScale[5][13] = {
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.1f, 1.4f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f
};

signed char gCarDifferential[13] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };

struct _tSusp suspensionLoose = { 1500.0f, 2500.0f, 0.07f, -2000.0f, 8000.0f, -0.04f, 2000.0f, 5000.0f, 0.06f };

struct _tSusp suspensionNormal = { 1500.0f, 2500.0f, 0.04f, -2000.0f, 8000.0f, -0.02f, 2000.0f, 5000.0f, 0.03f };

struct _tSusp suspensionTight = { 10000.0f, 2500.0f, 0.0f, -10000.0f, 8000.0f, -0.0f, 10000.0f, 5000.0f, 0.0f };

struct _tSusp * carSuspensions[3] = { (void *)&suspensionLoose, (void *)&suspensionNormal, (void *)&suspensionTight };

signed char gCarSuspensions[13] = { 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 0, 2, 2 };

signed char gTireRims[5][13] = {
    17, 1, 2, 3, 20, 14, 6, 7, 8, 10, 0, 16, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0
};

signed char gCarHorn[5][13] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0
};

signed char gCarPaintJob[5][13] = {
    2, 2, 0, 4, 2, 1, 0, 3, 1, 2, 1, 7, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0
};

// total size: 0x2C
struct {
    // Members
    float diffbl; // offset 0x0, size 0x4
    float weight_scale; // offset 0x4, size 0x4
    float torque_scale; // offset 0x8, size 0x4
    float trans_scale; // offset 0xC, size 0x4
    float clutchmaxtk; // offset 0x10, size 0x4
    float shiftk; // offset 0x14, size 0x4
    float fwdfk; // offset 0x18, size 0x4
    float sidefk; // offset 0x1C, size 0x4
    float skidk; // offset 0x20, size 0x4
    float antispin; // offset 0x24, size 0x4
    float drift; // offset 0x28, size 0x4
} gTune[3] = {
    0.0f, 1.0f, 1.0f, 1.1f, 1.0f, 1.0f, 1.0f, 0.0f, 1.8f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.075f,
    1.0f, 1.0f, 1.0f, 0.0f, 1.8f, 0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.05f, 1.0f, 1.0f, 1.0f, 0.0f,
    1.8f, 0.0f, 1.0f
};

signed char gMassiveMass;

