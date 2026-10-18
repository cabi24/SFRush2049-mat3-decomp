/* visuals.c - car visual objects (tires, shadows, sparks, blasts, environment maps). Status: NonMatching, GetCarsIRColor is the one open register-allocation near miss.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\visuals.c; assembly: build/2049LE/asm/GameSource/src/visuals.s */
#define gWheelBlur gWheelBlur_header
#define gNumCarPaintTexs gNumCarPaintTexs_header
#define gNumCarPaintJobs gNumCarPaintJobs_header
#define gCarPaintJobWaveNums gCarPaintJobWaveNums_header
#include "rush/rush_protos.h"
#undef gWheelBlur
#undef gNumCarPaintTexs
#undef gNumCarPaintJobs
#undef gCarPaintJobWaveNums
#define MSL_NO_SQRTF
#include "msl_math_inline.h"

static void InitWheelDynamicObjs(void);
static void InitDynamicTextures(void);
static void RemoveVisuals(signed short slot);
static void AnimateRocketFlame(signed short slot, int hulk);
static tRGBA GetCarsIRColor(tCarData *car, tModelData *m);
static void AnimateFrame(tVisual *v, signed short op);
static void StartWing(signed short slot, signed short side);
static void AnimateWing(tVisual *v, signed short op);
static void StartWingFlame(signed short slot, signed short side);
static void AnimateWingFlame(tVisual *v, signed short op);
static void StartNewBlast(signed short slot);
static void AnimateNewBlast(tVisual *v, signed short op);
static void StartNewBlastRing(signed short slot);
static void AnimateNewBlastRing(tVisual *v, signed short op);
static void AnimateBlast(tVisual *v, signed short op);
static void StartTire(signed short slot, unsigned int tire);
static void AnimateTire(tVisual *v, signed short op);
static void StartShadow(signed short slot);
static void AnimateShadow(tVisual *v, signed short op);
static void StartArrow(signed short slot);
static void AnimateArrow(tVisual *v);
static void StartBrakeLights(signed short slot);
static void AnimateBrakeLights(tVisual *v, signed short op);
static void AnimateLFSpark(tVisual *v, signed short op);
static void AnimateLRSpark(tVisual *v, signed short op);
static void AnimateRFSpark(tVisual *v, signed short op);
static void AnimateRRSpark(tVisual *v, signed short op);
static void AnimateBSpark(tVisual *v, signed short op);
static void HandleASpark(tVisual *v, int on, float x, float y, float z, int spark, signed short op);
static void PositionObject(int objnum, float x, float y, float z, float u, float v, float w);
static int CheckVisibleAll(tVisual *v, int on);
static int CheckVisible(tVisual *v, int on, int camera);

signed short gEnvCount;
signed short gEnvCountMax;
signed char gDriveHulk[4];
signed char gInvisibleCar;
signed char gFrameScale;
signed char gEnableTireScaling;
static tTexture *gGameCarShadowTexDef;
signed char gCarTeam[4];
signed char gCarTeamColor[4];
signed char gGoodFrameRate;
static tVisual *gEnvList;
static tVisual *gFreeVis;
static signed short gBlastMatCnt;
static int numShadowObjToDraw;
static float gCameraMatrix[4][4];
static signed char gFadePlayerArrow[4][4];
static tPackedObject *ShadowObjList[12];
static int gGameCarShadowIdx[6];
static tBlastStruct gBlastObjs[6];
static tBlastRingStruct gBlastRingObjs[6];
static tBlastMat gBlastMats[50];
static tPackedObject gGameCarShadow[18];
static unsigned int *gGameCarShadowPack[18];
signed char gCarPaintJobInited[52];
signed short gCarModelNums[52];
CarParts gCarParts[52];
static tVisual gEnvirons[100];
unsigned short gTexList[19];
tTexture *gWheelTextures[48];
unsigned short gObjList[701];

/* 0x80053c30, size 0x224 */
void InitTrackDynamicObjs(void)
{
    int i;
    static char *const WeaponNames[8] = {
        "WEP_CANNG1", "WEP_GATTG1", "WEP_GRENG1", "WEP_MINEG1", "WEP_MISSG1", "WEP_RAMG1",
        "WEP_ROCKG1", "WEP_SONCG1"
    };
    static char *const WingNames[12] = {
        "WINGSWING1L", "WINGSWING1R", "WINGSFLAME1L", "WINGSFLAME1R", "WINGSWING2L", "WINGSWING2R",
        "WINGSFLAME2L", "WINGSFLAME2R", "WINGSWING3L", "WINGSWING3R", "WINGSFLAME3L", "WINGSFLAME3R"
    };
    static char *const ObjNames[61] = {
        "HULK_DCL1_LOD1", "GEOMEXPSPHERE01", "GEOMEXPSPHERE02", "GEOMEXPSPHERE03", "GEOMEXPSPHERE04", "EXP_GLOWSMOKE",
        "EXP_RINGG3", "NEXPLOSIONG1", "NEXPLOSIONG2", "NEXPLOSIONG3", "NEXPLOSIONG4", "NEXPLOSIONG5",
        "NEXPLOSIONG6", "NEXPLOSIONG7", "NEXPLOSIONG8", "NEXPLOSIONG9", "NEXPLOSIONG10", "NEXPLOSIONG11",
        "NEXPLOSIONG12", "NEXPLOSIONG13", "NEXPLOSIONG14", "NEXPLOSIONG15", "NEXPLOSIONG16", "NEXPLOSIONG17",
        "NEXPLOSIONG18", "NEXPLOSIONG19", "NEXPLOSIONG20", "NEXPLOSIONG21", "NEXPLOSIONG22", "NEXPLOSIONG23",
        "NEXPLOSIONG24", "NEXPLOSIONG25", "NEXPLOSIONG26", "NEXPLOSIONG27", "NEXPLOSIONG28", "NEXPLOSIONG29",
        "NEXPLOSIONG30", "SPARKG1", "SMOKEG1", "SMOKEG2", "SMOKEG3", "SMOKEG4",
        "SMOKEG5", "SMOKEG6", "SMOKEG7", "SMOKEG8", "FIREFIRE", "SPLASHG12",
        "SPLASHG13", "SPLASHG14", "SPLASHG15", "SPLASHG16", "SPLASHG17", "SPLASHG18",
        "SPLASHG19", "ROKTFLAMEG1", "ROKTFLAMEG2", "ROKTFLAMEG3", "CAR4_PLCCAR", "HEADLIGHTF",
        "HEADLIGHTS"
    };
    static char *const TrkNames[163] = {
        "CONE1G1", "RATG1", "METERG2", "TREEHIT51", "TREEHIT52", "TREEHIT53",
        "TREEHIT54", "TREEHIT55", "TREEHIT56", "TREEHIT1G1", "TREEHIT2G1", "TREEHIT3G1",
        "TREEHIT4G1", "SHATPANEG2", "SHATPANEG3", "SHATPANEG4", "SHATPANEG5", "SHATPANEG6",
        "SHATPANEG7", "SHATPANEG8", "GRANDWINO1", "GRANDWINO2", "GRANDWINO3", "GRANDWINO4",
        "GRANDWINO5", "GRANDWINO6", "GRANDWINO7", "CACTUSG2", "CACTUSG3", "CACTUSG4",
        "CACTUSG5", "CACTUSG6", "CACTUSG7", "CACTUSG8", "FENCEG1", "FENCEG4",
        "FENCEG6", "FENCEG8", "FENCEG10", "FENCEG12", "FENCEG14", "FENCEG29",
        "FENCEG30", "FENCEG31", "FENCEG32", "FENCEG24", "FLAG2G13", "FLAG2G12",
        "FLAG2G11", "FLAG2G10", "FLAG2G9", "FLAG2G8", "FLAG2G6", "FLAG2G5",
        "FLAG2G4", "FLAG2G2", "T5GATEO1", "T5GATEO2", "T5GATEO3", "T5GATEO4",
        "T5GATEO5", "T5GATEO6", "T5GATEO7", "T5GATEO8", "T5GATEO9", "T5GATEO10",
        "T5GATEO11", "T5GATEO12", "GASIGNO1", "GASIGNO2", "GASIGNO3", "BUMPHITG1",
        "CURVEHITG1", "DIPHITG1", "MPH45HITG1", "MPH75HITG1", "LEFTHITG1", "RIGHTHITG1",
        "NOPASHITG1", "PCAREHITG1", "REDUCHITG1", "STOPHITG1", "SLOWHITG2", "YIELDHITG1",
        "ZONEHITG1", "GLAMPHITO1", "GLAMPHITO2", "GLAMPHITO3", "NYTREEHTO1", "THINKHITG1",
        "CHAIRHITO1", "CHAIRHITO2", "CHAIRHITO3", "CHAIRHITO4", "CHAIRHITO5", "DESKHITO1",
        "DESKHITO2", "DESKHITO3", "DESKHITO4", "DESKHITO5", "MAPSIGNO1", "MAPSIGNO2",
        "MAPSIGNO3", "RATCONEG1", "SRFBRDO1", "USFLAGO1", "USFLAGO2", "USFLAGO3",
        "UMBRELLAO1", "UMBRELLAO2", "UMBRELLAO3", "NYLGATEO1", "TRIGGERL38_LOD1", "TRIGGERL37_LOD1",
        "WEPICON_CANNL1_", "WEPICON_GATTL1_", "WEPICON_GRENL1_", "WEPICON_MINEL1_", "WEPICON_MISSL1_", "WEPICON_RAML1_L",
        "WEPICON_ROCKL1_", "WEPICON_SONCL1_", "WEPICON_QUESL1_", "WEPICON_HEALG1", "WEPICON_INVSG1", "WEPICON_SHLDG1",
        "SHARKL1_LOD1", "SHARKL2_LOD1", "SHARKL3_LOD1", "SHARKL4_LOD1", "F1FLAGL1_LOD1", "F1FLAGL2_LOD1",
        "F1FLAGL3_LOD1", "F1FLAGL4_LOD1", "F1FLAGL5_LOD1", "F1FLAGL6_LOD1", "F1FLAGL7_LOD1", "F1FLAGL8_LOD1",
        "F1FLAGL9_LOD1", "F1FLAGL10_LOD1", "F1FLAGL11_LOD1", "F1FLAGL12_LOD1", "F1FLAGL13_LOD1", "F1FLAGL14_LOD1",
        "F1FLAGL15_LOD1", "F1FLAGL16_LOD1", "F1FLAGL17_LOD1", "F1FLAGL18_LOD1", "F1FLAGL19_LOD1", "F1FLAGL20_LOD1",
        "COIN_GLOWL3_LOD"
    };
    for (i = 469; i < 530; i++)
        gObjList[i] = MBOX_FindObject_WORK(ObjNames[i - 469], 0, gNumModels - 1, 0);
    gRomObjDefPtrs[gObjList[528] >> 10].romObjDefs[gObjList[528] & 1023].radius = 100.0f;
    gRomObjDefPtrs[gObjList[529] >> 10].romObjDefs[gObjList[529] & 1023].radius = 100.0f;
    for (; i < 538; i++)
        gObjList[i] = MBOX_FindObject_WORK(WeaponNames[i - 530], 0, gNumModels - 1, 0);
    for (; i < 550; i++)
        gObjList[i] = MBOX_FindObject_WORK(WingNames[i - 538], 0, gNumModels - 1, 0);
    if (gMineCones || gCarMines)
        gObjList[i++] = MBOX_FindObject_WORK("MINEG1", 0, gNumModels - 1, 0);
    for (; i < 701; i++)
        gObjList[i] = MBOX_FindObject_WORK(TrkNames[i - 550], 0, gNumModels - 1, 0);
    SetTargetDefs();
}

const char gWheelBlur[24] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
};

float gDummyVtx[4][3] = { 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f };
tRGBA SheenColors[19][2] = {
    126, 221, 252, 255, 53, 91, 255, 255, 255, 255, 255, 255, 253, 175, 128, 255, 112, 179, 253,
    255, 113, 223, 254, 255, 255, 150, 202, 255, 255, 199, 144, 255, 255, 150, 202, 255, 158, 253,
    228, 255, 100, 166, 254, 255, 113, 240, 254, 255, 255, 255, 255, 255, 255, 246, 164, 255, 128,
    128, 128, 255, 128, 128, 128, 255, 128, 128, 128, 255, 128, 128, 128, 255, 126, 221, 252, 255,
    53, 91, 255, 255, 255, 255, 255, 255, 255, 228, 144, 255, 255, 255, 255, 255, 253, 231, 158,
    255, 255, 255, 255, 255, 255, 205, 196, 255, 255, 162, 175, 255, 255, 159, 229, 255, 128, 128,
    128, 255, 128, 128, 128, 255, 94, 142, 170, 255, 255, 207, 172, 255, 200, 200, 200, 255, 255,
    207, 159, 255, 242, 195, 103, 255, 242, 195, 103, 255, 255, 255, 255, 255, 255, 255, 255, 255
};
tRGBA Track1Colors[26] = {
    255, 255, 255, 255, 150, 150, 150, 255, 125, 125, 125, 255, 100, 100, 100, 255, 75, 75, 75,
    255, 50, 50, 50, 255, 40, 40, 40, 255, 30, 30, 30, 255, 20, 20, 20, 255, 215, 255, 252, 255,
    175, 223, 255, 255, 179, 253, 248, 255, 179, 216, 253, 255, 200, 200, 255, 255, 170, 170, 255,
    255, 128, 128, 255, 255, 254, 198, 231, 255, 254, 155, 214, 255, 254, 204, 190, 255, 255, 173,
    147, 255, 255, 245, 200, 255, 251, 254, 137, 255, 240, 255, 193, 255, 228, 255, 143, 255, 190,
    254, 190, 255, 148, 254, 154, 255
};
tRGBA Track4Colors[26] = {
    200, 200, 200, 255, 180, 180, 180, 255, 160, 160, 160, 255, 140, 140, 140, 255, 120, 120, 120,
    255, 100, 100, 100, 255, 75, 75, 75, 255, 50, 50, 50, 255, 25, 25, 25, 255, 215, 255, 252, 255,
    82, 127, 172, 255, 179, 253, 248, 255, 179, 216, 253, 255, 200, 200, 255, 255, 170, 170, 255,
    255, 128, 128, 255, 255, 254, 198, 231, 255, 254, 155, 214, 255, 210, 200, 180, 255, 255, 173,
    147, 255, 255, 245, 200, 255, 251, 254, 137, 255, 190, 200, 103, 255, 228, 255, 143, 255, 190,
    254, 190, 255, 148, 254, 154, 255
};
tRGBA Track5Colors[26] = {
    255, 255, 255, 255, 200, 200, 200, 255, 175, 175, 175, 255, 150, 150, 150, 255, 125, 125, 125,
    255, 100, 100, 100, 255, 75, 75, 75, 255, 50, 50, 50, 255, 25, 25, 25, 255, 215, 255, 252, 255,
    175, 223, 255, 255, 179, 253, 248, 255, 179, 216, 253, 255, 200, 200, 255, 255, 170, 170, 255,
    255, 128, 128, 255, 255, 254, 198, 231, 255, 254, 155, 214, 255, 254, 204, 190, 255, 255, 173,
    147, 255, 255, 245, 200, 255, 251, 254, 137, 255, 240, 255, 193, 255, 228, 255, 143, 255, 190,
    254, 190, 255, 148, 254, 154, 255
};
tRGBA *gTrackColors[19] = {
    (void *)&Track1Colors, (void *)&Track1Colors, (void *)&Track1Colors, (void *)&Track4Colors,
    (void *)&Track5Colors, (void *)&Track1Colors, (void *)&Track1Colors, (void *)&Track1Colors,
    (void *)&Track1Colors, (void *)&Track1Colors, (void *)&Track1Colors, (void *)&Track1Colors,
    (void *)&Track1Colors, (void *)&Track1Colors, (void *)&Track5Colors, (void *)&Track5Colors,
    (void *)&Track5Colors, (void *)&Track5Colors, (void *)&Track5Colors
};
static float TSCLS[3] = { 0.6f, 1.7f, 2.5f };
static char *gWheelNames[48] = {
    "RIMB1", "RIMB2", "RIMB3", "RIMB4", "RIMB5", "RIMB6", "RIMB7", "RIMB8", "RIMB9", "RIMB10",
    "RIMB11", "RIMB12", "RIMB13", "RIMB14", "RIMB15", "RIMB16", "RIMB17", "RIMB18", "RIMB19",
    "RIMB20", "RIMB21", "RIMB22", "RIMB23", "RIMB24", "RIM01", "RIM02", "RIM03", "RIM04", "RIM05",
    "RIM06", "RIM07", "RIM08", "RIM09", "RIM10", "RIM11", "RIM12", "RIM13", "RIM14", "RIM15",
    "RIM16", "RIM17", "RIM18", "RIM19", "RIM20", "RIM21", "RIM22", "RIM23", "RIM24"
};
tRGBA gShadowColor = { 0, 0, 0, 192 };


/* 0x80053e54, size 0xd4 */
static void InitWheelDynamicObjs(void)
{
    unsigned short index; /* stack */
    int i; /* r28 */
    static char *const ObjNames[1] = { "WHEEL_DCWHEEL1_" };
    for (i = 0; i < 48; i++) {
        MBOX_FindTexture_WORK(gWheelNames[i], &index, 0, gNumModels - 1, 1);
        gWheelTextures[i] = &gTexDefPtrs[index >> 10].texDefs[index & 1023];
    }
    for(i=0;i<1;i++)
        gObjList[468+i] = MBOX_FindObject_WORK(ObjNames[i], 0, gNumModels - 1, 1);
}

/* 0x80053f28, size 0x7c */
static const char *const TexNames[19] = {
    "SPARK01", "SPARK02", "SPARK03", "SPARK04", "SPARK05", "SPARK06", "SPARK07", "SPARK08",
    "SHADBLUR", "ARROW", "SKID04", "SHEEN0", "SHEENBURN", "SHEENS1", "SHEENS2", "SHEENS3",
    "SHEENT1", "SHEENT2", "SHEENT3"
};

static void InitDynamicTextures(void)
{
    int i; /* r29 */
    for (i = 0; i < 19; i++)
        MBOX_FindTexture_WORK(TexNames[i], &gTexList[i], 0, gNumModels - 1, 0);
}

/* 0x80053fa4, size 0xa0 */
void InitCarDynamicObjs(signed short slot, signed short car, signed short model)
{
    int i; /* r27 */
    char partName[20]; /* stack */
    int objIndex;
    static const char *const carObjNames[9] = {
        "FRAME1", "HOOD", "SHEEN", "BRAKE", "HULK1", "DAMAGE1", "HOODSHEEN", "BRAKE2", "TRIM"
    };
    objIndex = slot * 9;
    for (i = 0; i < 9; i++, objIndex++) {
        sprintf(partName, "%s%s", CarNames[car], carObjNames[i]);
        gObjList[objIndex] = MBOX_FindObject_WORK(partName, model, model, 1);
    }
}

const int gNumCarPaintTexs[13] = { 29, 24, 10, 23, 26, 20, 23, 23, 22, 21, 14, 6, 6 };
const int gNumCarPaintJobs[13] = { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 };
const int gCarPaintJobWaveNums[13] = { 77, 89, 101, 113, 125, 137, 149, 161, 173, 185, 197, 209, 221 };


/* 0x80054044, size 0x138 */
void InitEnvs(void)
{
    int i; /* r4 */
    gFreeVis = gEnvirons;
    for (i = 0; i < 99; i++) {
        gEnvirons[i].next = &gEnvirons[i + 1];
        gEnvirons[i].objnum = -1;
    }
    gEnvList = 0;
    gEnvirons[i].next = 0;
    gEnvirons[i].objnum = -1;
    gEnvCount = 0;
}

/* 0x8005417c, size 0x1b4 */
void InitVisuals(int cars)
{
    int i; /* r6 */
    static int s_first = 1;
    for (i = 0; i < 4; i++) gCarTeamColor[i] = i;
    if (cars) {
        memset(gCarParts, 0, sizeof(gCarParts));
        if (gGameState & 0x100) {
            InitVisualsCarScr();
            for (i = 0; i < 468; i++) gObjList[i] = 0;
        } else {
            for (i = 0; i < 13; i++) {
                InitCarDynamicObjs(i, gLink[i].car, gCarModelNums[i]);
                gCarPaintJobInited[i] = 0;
            }
        }
        InitWheelDynamicObjs();
        InitDynamicTextures();
    }
    if (s_first) {
        for (i = 0; i < 18; i++) gGameCarShadowPack[i] = 0;
        s_first = 0;
    }
}

/* 0x80054330, size 0x34 */
void InitCarVisuals(tCarData *gc)
{
    tVisual *v; /* r5 */
    signed short i; /* r6 */
    for (v = gc->visuals, i = 0; i < 22; i++, v++) {
        v->func = 0;
        v->objnum = -1;
    }
}

/* 0x80054364, size 0x118 */
void SetSheen(signed short slot, signed short player, signed short sheenIdx)
{
    tTexture *tex; /* r3 */
    CarParts *car; /* r30 */
    unsigned short texidx; /* stack */
    signed short cam; /* r31 */
    static char *sheenName[11] = { "SHEEN0", "SHEEN1", "SHEEN2", "SHEEN3", "SHEEN4", "SHEEN5", "SHEEN6", "SHEEN7", "SHEEN8", "SHEEN9", "SHEEN10" };
    if (sheenIdx < -1 || sheenIdx > 10) return;
    switch (player) {
        case 0: cam = 1; break;
        case 1: cam = 2; break;
        case 2: cam = 4; break;
        case 3: cam = 8; break;
    }
    car = &gCarParts[slot + player * 13];
    if (sheenIdx == -1) MBOX_HideObject(car->sheen, 0, cam);
    else {
        tex = MBOX_FindTexture_WORK(sheenName[sheenIdx], &texidx, 0, gNumModels - 1, 1);
        ZOID_SetTexture(car->sheen, tex, -1);
        MBOX_ShowObject(car->sheen, 0, cam);
    }
}

/* 0x8005447c, size 0xa0 */
void SetFrameColor(signed short slot, tRGBA *col)
{
    CarParts *car; /* r31 */
    car = &gCarParts[slot];
    ZOID_SetColor(car->frame, col);
    ZOID_SetColor(car->hood, col);
    ZOID_SetColor(car->tires[0], col);
    ZOID_SetColor(car->tires[1], col);
    ZOID_SetColor(car->tires[2], col);
    ZOID_SetColor(car->tires[3], col);
}

/* 0x8005451c, size 0x60 */
void SetSheenColor(signed short slot, tRGBA *col, tRGBA *env)
{
    CarParts *car; /* r31 */
    car = &gCarParts[slot];
    ZOID_SetColor(car->sheen, col);
    ZOID_SetEnvColor(car->sheen, env);
}

/* 0x8005457c, size 0xcc */
int InitCarObj(int objnum, signed short camlink, int parent, unsigned int flags, signed short slot, signed short carObjNum, int forGame)
{
    tUVMatrix *mat; /* r4 */
    if (forGame) mat = ZOID_NewMatrix();
    else if (carObjNum == -1) {
        flags |= 128;
        mat = (tUVMatrix *)unitmatrix;
    } else mat = ((tUVMatrix **)gCarMatrices)[slot / 13 * 12 + carObjNum];
    return (signed short)MBOX_NewObjectCam(objnum, mat, camlink, parent, flags);
}

/* 0x80054648, size 0x6a4 */
void CreateCar(signed short slot, signed short camlink, int for_game)
{
CarParts *car;
    signed short offset;
    unsigned int flags;
    unsigned int flags2;
    unsigned int idx;
    idx = slot % 13;
    flags = (gNumPlayers == 1 || !for_game) ? 0 : 5;
    offset = slot * 9;
    car = &gCarParts[slot];
    flags2 = !for_game ? 0x40004 : 0;
    car->frame = InitCarObj(gObjList[0 + offset], camlink, -1, flags2 | 0x2018, slot, 0, for_game);
    car->hood = InitCarObj(gObjList[1 + offset], camlink, car->frame, flags2 | 0x2080, slot, -1, for_game);
    car->extraparts[0] = 0;
    if (!for_game) {
        if (idx == 3) goto create_extra;
        goto extra_done;
    }
    if (model[slot].body_type == 3 && gLink[slot].PaintJob == 5) {
create_extra:
        car->extraparts[0] = InitCarObj(MBOX_FindObject_WORK("CAR4_PLCCAR", 0, gNumModels - 1, 1), camlink, car->frame, 0x2080, slot, -1, for_game);
        MBOX_HideObject(car->extraparts[0], 1, 15);
    }
extra_done:
    if (for_game && (gTrackNum == 3 || gTrackNum == 5)) {
        car->brakes = InitCarObj(gObjList[7 + offset], camlink, car->frame, 0x80, slot, -1, for_game);
        car->trim = InitCarObj(gObjList[8 + offset], camlink, car->frame, 0x84, slot, -1, for_game);
        MBOX_ShowObject(car->trim, 0, 15);
    } else {
        car->brakes = InitCarObj(gObjList[3 + offset], camlink, car->frame, flags2 | 0x80, slot, -1, for_game);
        car->trim = 0;
    }
    car->sheen = InitCarObj(gObjList[2 + offset], camlink, car->frame, flags2 | 0x286080, slot, -1, for_game);
    ZOID_SetTexture(gCarParts[slot].sheen, &gTexDefPtrs[gTexList[11] >> 10].texDefs[gTexList[11] & 1023], -1);
    ObjInstanceList[car->sheen].w = 1.01f;
    SetSheenColor(slot, &Track1Colors[0], &Track1Colors[0]);
    car->tsheen = 0;
    if (for_game) flags2 = flags | (gNumPlayers == 1 ? 0 : 5);
    else flags2 = flags | 0x42004;
    car->tires[0] = InitCarObj(gObjList[468], camlink, car->frame, flags2 | 0x2000, slot, 3, for_game);
    car->tires[1] = InitCarObj(gObjList[468], camlink, car->frame, flags2 | 0x2000, slot, 4, for_game);
    car->tires[2] = InitCarObj(gObjList[468], camlink, car->frame, flags2 | 0x2000, slot, 5, for_game);
    car->tires[3] = InitCarObj(gObjList[468], camlink, car->frame, flags2 | 0x2000, slot, 6, for_game);
    ZOID_SetTexture(car->tires[0], gWheelTextures[24 + gTireRims[gLink[idx].owner == 6 ? idx + 1 : 0][idx]], -1);
    ZOID_SetTexture(car->tires[1], gWheelTextures[24 + gTireRims[gLink[idx].owner == 6 ? idx + 1 : 0][idx]], -1);
    ZOID_SetTexture(car->tires[2], gWheelTextures[24 + gTireRims[gLink[idx].owner == 6 ? idx + 1 : 0][idx]], -1);
    ZOID_SetTexture(car->tires[3], gWheelTextures[24 + gTireRims[gLink[idx].owner == 6 ? idx + 1 : 0][idx]], -1);
    if (for_game) {
        car->lfSpark = InitCarObj(gObjList[506], camlink, car->frame, 0xf00, slot, 7, for_game);
        car->lrSpark = InitCarObj(gObjList[506], camlink, car->frame, 0xf00, slot, 8, for_game);
        car->rfSpark = InitCarObj(gObjList[506], camlink, car->frame, 0xf00, slot, 9, for_game);
        car->rrSpark = InitCarObj(gObjList[506], camlink, car->frame, 0xf00, slot, 10, for_game);
        car->bSpark = InitCarObj(gObjList[506], camlink, car->frame, 0xf00, slot, 11, for_game);
    } else {
        car->lfSpark = -1;
        car->lrSpark = -1;
        car->rfSpark = -1;
        car->rrSpark = -1;
        car->bSpark = -1;
    }
    if (for_game) {
        gGameCar[slot].sheenAlpha = 255;
        gGameCar[slot].IRclr.rgba = 65535;
        gGameCar[slot].prevIRclr.rgba = 65535;
        SetupCarColor(slot, gLink[slot].car, model[slot].PaintJob);
    }
    SetFrameColor(slot, &Track1Colors[0]);
    if (idx == 3 && car->extraparts[0]) ZOID_SetColor(car->extraparts[0], &Track1Colors[0]);
}

char *Car1BaseTextureNames[29] = {
    "C1_BK_ROOF", "C1_BKWS1", "C1_BKWS2", "C1_BKWS", "C1_CONESDE", "C1_CONETOP", "C1_DKRAS",
    "C1_ENDCAP", "C1_FEND", "C1_FTSCOOP", "C1_FTWIND", "C1_INTAKE", "C1_LOWSDE", "C1_LOWTOP",
    "C1_MIRROR", "C1_SCPSD", "C1_SM_SDE", "C1_SPSDE", "C1_TAIL02", "C1_TAILBK", "C1_TLGHT",
    "C1_TOP01", "C1_TOP02", "C1_TOP04", "C1_TOP05", "C1_TSTRAS", "C1_UNDR", "C1_WINSD1",
    "C1_WINSDE"
};
char *Car2BaseTextureNames[24] = {
    "C12_BKTP1", "C12_BKTP1A", "C12_BKTP1B", "C12_BKTP1C", "C12_SCOOP", "C12_SDE01", "C12_SDE02",
    "C12_SDE03", "C12_SDE04", "C12_SDE05", "C12_SDE06", "C12_SDE07", "C12_SDE08", "C12_TLGHT1",
    "C12_TLGHT2", "C12_TLGHT3", "C12_TLGHT4", "C12_TPHD1", "C12_WIN1", "C12_WIN1D", "C_12LOWB",
    "C_12LOWFNT", "C_12LOWSIDE", "C_12LOWTOP"
};
char *Car3BaseTextureNames[10] = {
    "C3_LOWNOSE2", "C3_LOWNOSE", "C3_LOWSIDE", "C3_LOWTOP", "C3_SDE01", "C3_SDE02", "C3_TAILCNR",
    "C3_TPBDY1", "C3_TPBDY2", "C3_TPBDY3"
};
char *Car4BaseTextureNames[23] = {
    "C4_BF1", "C4_BP", "C4_BW1", "C4_ENGINE", "C4_LOWBACK", "C4_LOWSDE", "C4_LOWTOP", "C4_NOSE01",
    "C4_NOSE02", "C4_PANEL01", "C4_PANEL02", "C4_ROOF01", "C4_SIDE01", "C4_SIDE02", "C4_SIDE03",
    "C4_SIDE05", "C4_SIDE06", "C4_SIDE07", "C4_TAIL", "C4_VENT", "C4_WR1", "C4_WS1", "C4_WS2"
};
char *Car5BaseTextureNames[26] = {
    "C5_HOOD01", "C5_HOOD02", "C5_HOOD03", "C5_HOOD04", "C5_LOWBACK", "C5_LOWSIDE", "C5_LOWTOP",
    "C5_NOSE01", "C5_SIDE01", "C5_SIDE02", "C5_SIDE03", "C5_SIDE04", "C5_SIDE05", "C5_SIDE06",
    "C5_SIDE07", "C5_SIDE08", "C5_SPS", "C5_TE1", "C5_TE2", "C5_TE3", "C5_TE4", "C5_TOP01",
    "C5_TOP02", "C5_TOP03", "C5_TOP04", "C5_VENT01"
};
char *Car6BaseTextureNames[20] = {
    "C6_BACKWIN", "C6_FRONT01", "C6_FRONT02", "C6_FRONTW", "C6_HOOD01", "C6_HOOD02", "C6_LOWBACK",
    "C6_LOWF", "C6_LOWSIDE", "C6_LOWTOP", "C6_ROOF", "C6_SIDE02", "C6_SIDE03", "C6_SIDE04",
    "C6_SIDE05", "C6_SIDE06", "C6_SIDE08", "C6_TAIL01", "C6_TAIL02", "C6_TRUNK"
};
char *Car7BaseTextureNames[23] = {
    "C7_B1", "C7_B2", "C7_F", "C7_LOWB", "C7_LOWF", "C7_LOWS", "C7_LOWT", "C7_S1", "C7_S2",
    "C7_S3", "C7_S4", "C7_S5", "C7_S6", "C7_S7", "C7_S8", "C7_T1", "C7_T2", "C7_T3", "C7_T4",
    "C7_T5", "C7_T6", "C7_T7", "C7_T8"
};
char *Car8BaseTextureNames[23] = {
    "C8_B1", "C8_B2", "C8_B3", "C8_B4", "C8_LOWB", "C8_LOWF", "C8_LOWS", "C8_LOWT", "C8_S1",
    "C8_S2", "C8_S3", "C8_S4", "C8_S5", "C8_S6", "C8_S7", "C8_S8", "C8_T1", "C8_T2", "C8_T3",
    "C8_T4", "C8_T5", "C8_T6", "C8_W"
};

static const char *const Car9BaseTextureNames[22] = {
    "C9_B1", "C9_B2", "C9_B3", "C9_F1", "C9_F2", "C9_F3", "C9_LOWB", "C9_LOWF", "C9_LOWS",
    "C9_LOWT", "C9_ROOF", "C9_S10", "C9_S11", "C9_S1", "C9_S2", "C9_S3", "C9_S4", "C9_S5", "C9_S6",
    "C9_S7", "C9_S8", "C9_S9"
};
static const char *const Car10BaseTextureNames[21] = {
    "C10_B1", "C10_B2", "C10_B3", "C10_B4", "C10_FRONT", "C10_HOOD", "C10_LOWB", "C10_LOWF",
    "C10_LOWS", "C10_LOWT", "C10_LOWW", "C10_SIDE", "C10_T1", "C10_T2", "C10_T3", "C10_T4",
    "C10_T5", "C10_T6", "C10_W", "C10_WIN", "C10_WING"
};
static const char *const Car11BaseTextureNames[14] = {
    "C11_2049", "C11_BACK", "C11_FRONT", "C11_GRILL", "C11_HOOD", "C11_LOW2", "C11_LOW3",
    "C11_LOW4", "C11_LOW5", "C11_LOW", "C11_ROOF", "C11_SIDE01", "C11_SIDE02", "C11_TAIL"
};
static const char *const Car12BaseTextureNames[6] = { "C12A_LOWB", "C12A_LOWF", "C12A_LOWS", "C12A_LOWT", "C12A_SIDE", "C12A_TOP" };
static char *Car13BaseTextureNames[6] = { "C13_BACK", "C13_LOW", "C13_LOWF", "C13_LOWT", "C13_SIDE", "C13_TOP" };
static const char *const Car1TextureExt[12] = {
    "_BLU", "_RED", "_YEL", "_GRN", "_PUR", "_TEL", "_ORG", "_CHR", "_PUR", "_PUR", "_PUR", "_PUR"
};
static const char *const Car2TextureExt[12] = {
    "_BLU", "_RED", "_YEL", "_GRN", "_PUR", "_ORG", "_TEL", "_PNK", "_PUR", "_PUR", "_PUR", "_PUR"
};
static const char *const Car3TextureExt[12] = {
    "_BLU", "_RED", "_YEL", "_GRN", "_PUR", "_SGN", "_CYA", "_ORA", "_PUR", "_PUR", "_PUR", "_PUR"
};
static const char *const Car4TextureExt[12] = {
    "_BLU", "_RED", "_YEL", "_GRN", "_PUR", "_PLC", "_CYN", "_BLK", "_PUR", "_PUR", "_PUR", "_PUR"
};
static const char *const Car5TextureExt[12] = {
    "_BLU", "_RED", "_YEL", "_GRN", "_PUR", "_TEA", "_GOL", "_PNK", "_PUR", "_PUR", "_PUR", "_PUR"
};
static const char *const Car6TextureExt[12] = {
    "_BLU", "_RED", "_YEL", "_GRN", "_PUR", "_GRY", "_BWN", "_TBL", "_PUR", "_PUR", "_PUR", "_PUR"
};
static const char *const Car7TextureExt[12] = {
    "_BLU", "_RED", "_YEL", "_GRN", "_PUR", "_ROY", "_ORA", "_SIL", "_PUR", "_PUR", "_PUR", "_PUR"
};
static const char *const Car8TextureExt[12] = {
    "_BLU", "_RED", "_YEL", "_GRN", "_PUR", "_CYA", "_ORA", "_PIN", "_PUR", "_PUR", "_PUR", "_PUR"
};
static const char *const Car9TextureExt[12] = {
    "_BLU", "_RED", "_YEL", "_GRN", "_PUR", "_LIM", "_ORA", "_PIN", "_PUR", "_PUR", "_PUR", "_PUR"
};
static const char *const Car10TextureExt[12] = {
    "_BLU", "_RED", "_YEL", "_GRN", "_PUR", "_ORA", "_LIM", "_BLK", "_PUR", "_PUR", "_PUR", "_PUR"
};
static const char *const Car11TextureExt[12] = {
    "_BLU", "_RED", "_YEL", "_GRN", "_PUR", "_ORA", "_CAM", "_WHT", "_PUR", "_PUR", "_PUR", "_PUR"
};
static const char *const Car12TextureExt[12] = {
    "_BLU", "_RED", "_YEL", "_GRN", "_PUR", "_ORA", "_TEA", "_SLV", "_PUR", "_PUR", "_PUR", "_PUR"
};
static const char *const Car13TextureExt[12] = {
    "_BLU", "_RED", "_YEL", "_GRN", "_PUR", "_ORA", "_WHT", "_YER", "_PUR", "_PUR", "_PUR", "_PUR"
};



/* 0x80054cec, size 0x1e8 */
void SetupCarColor(signed short slot, signed short car, unsigned char PaintJob)
{
char **texnames;
    char name[16];
    char **extnames;
    int numtexs;
    tTexture *tex;
    unsigned short idx;
    int i;
    numtexs = gNumCarPaintTexs[car];
    switch (car) {
        case 0: texnames = Car1BaseTextureNames; extnames = (char **)Car1TextureExt; break;
        case 1: texnames = Car2BaseTextureNames; extnames = (char **)Car2TextureExt; break;
        case 2: texnames = Car3BaseTextureNames; extnames = (char **)Car3TextureExt; break;
        case 3: texnames = Car4BaseTextureNames; extnames = (char **)Car4TextureExt; break;
        case 4: texnames = Car5BaseTextureNames; extnames = (char **)Car5TextureExt; break;
        case 5: texnames = Car6BaseTextureNames; extnames = (char **)Car6TextureExt; break;
        case 6: texnames = Car7BaseTextureNames; extnames = (char **)Car7TextureExt; break;
        case 7: texnames = Car8BaseTextureNames; extnames = (char **)Car8TextureExt; break;
        case 8: texnames = (char **)Car9BaseTextureNames; extnames = (char **)Car9TextureExt; break;
        case 9: texnames = (char **)Car10BaseTextureNames; extnames = (char **)Car10TextureExt; break;
        case 10: texnames = (char **)Car11BaseTextureNames; extnames = (char **)Car11TextureExt; break;
        case 11: texnames = (char **)Car12BaseTextureNames; extnames = (char **)Car12TextureExt; break;
        case 12: texnames = Car13BaseTextureNames; extnames = (char **)Car13TextureExt; break;
    }
    for (i = 0; i < numtexs; i++) {
        tex = MBOX_FindTexture_WORK(texnames[i], &idx, gCarModelNums[slot], gCarModelNums[slot] + 1, 0);
        if (tex) {
            sprintf(name, "%s%s", texnames[i], extnames[PaintJob]);
            if (gGameState & 0x100) MBOX_FindTexture_WORK(name, &idx, gSelectCar[slot].pjModelNum, gSelectCar[slot].pjModelNum + 1, 0);
            else MBOX_FindTexture_WORK(name, &idx, 0, gNumModels - 1, 1);
            LinkWithAnotherTexture(tex, name);
        }
    }
    gCarPaintJobInited[slot] = 1;
}

/* 0x80054ed4, size 0x388 */
void SetupCar(signed short slot, unsigned char type, int for_game)
{
signed short i;
    unsigned int appear;
    if (for_game && type != 13 && (gGameCar[slot].appearance & 0x10)) {
        if (gLink[slot].owner != 6 || !gDriveHulk[slot]) appear = ~0x10;
        else if (gDriveHulk[slot] == 2) appear = ~0x60;
        else appear = ~0;
        model[slot].appearance &= appear;
        gGameCar[slot].appearance &= appear;
    }
    ZOID_SetObjectDef(gCarParts[slot].frame, (unsigned short)PickCar(slot, type));
    gGameCar[slot].objnum = gCarParts[slot].frame;
    ZOID_SetObjectDef(gCarParts[slot].hood, gObjList[(signed short)(slot * 9) + 1]);
    for (i = 0; i < 4; i++) ZOID_SetObjectDef(gCarParts[slot].tires[i], gObjList[468]);
    ZOID_SetTexture(gCarParts[slot].tires[0], gWheelTextures[24 + model[slot].tireRims], -1);
    ZOID_SetTexture(gCarParts[slot].tires[1], gWheelTextures[24 + model[slot].tireRims], -1);
    ZOID_SetTexture(gCarParts[slot].tires[2], gWheelTextures[24 + model[slot].tireRims], -1);
    ZOID_SetTexture(gCarParts[slot].tires[3], gWheelTextures[24 + model[slot].tireRims], -1);
    MBOX_ShowObject(gCarParts[slot].frame, 1, 15);
    if (gGameCar[slot].view != 1) MBOX_HideObject(gCarParts[slot].hood, 1, 15);
    if (for_game) {
        MBOX_HideObject(gCarParts[slot].lfSpark, 1, 15);
        MBOX_HideObject(gCarParts[slot].lrSpark, 1, 15);
        MBOX_HideObject(gCarParts[slot].rfSpark, 1, 15);
        MBOX_HideObject(gCarParts[slot].rrSpark, 1, 15);
        MBOX_HideObject(gCarParts[slot].bSpark, 1, 15);
        if (type == 13 || gGameCar[slot].view < 2) {
            for (i = 0; i < 4; i++) MBOX_HideObject(gCarParts[slot].tires[i], 1, 15);
            if (type == 13) {
                MBOX_HideObject(gCarParts[slot].hood, 1, 15);
                model[slot].appearance |= 0x10;
                gGameCar[slot].appearance |= 0x10;
            }
        }
    }
}

/* 0x8005525c, size 0xb4 */
void StartCar(signed short slot)
{
    signed short j; /* r31 */
    StartFrame(slot);
    StartSparks(slot);
    StartWing(slot, 0);
    StartWing(slot, 1);
    StartWingFlame(slot, 0);
    StartWingFlame(slot, 1);
    if (gRace == 6) StartWeapon(slot);
    StartHeadLights(slot);
    StartBrakeLights(slot);
    for (j = 0; j < 4; j++) StartTire(slot, j);
}

/* 0x80055310, size 0x40 */
int PickCar(signed short slot, unsigned char type)
{
    if (type == 13) return gObjList[slot * 9 + 4];
    return gObjList[slot * 9];
}

/* 0x80055350, size 0x48 */
void exitVisuals(void)
{
    int i; /* r31 */
    for (i = 0; i < gNumActiveCars; i++) RemoveCar(i);
}

/* 0x80055398, size 0x6c */
void RemoveCar(signed short slot)
{
    if (gGameCar[slot].objnum >= 0) {
        MBOX_HideObject(gCarParts[slot].frame, 2, 15);
        RemoveVisuals(slot);
    }
}

/* 0x80055404, size 0xdc */
void StartEnvironment(void)
{
    signed short i; /* r29 */
    for (i = 0; i < 6; i++) {
        if (model[i].in_game) {
            CreateCar(i, 15, 1);
            StartCar(i);
        }
    }
    for (i = 0; i < 6; i++) {
        if (model[i].in_game) {
            StartShadow(i);
            if (gNumPlayers > 1 && gLink[i].owner == 6) StartArrow(i);
        }
    }
}

/* 0x800554e0, size 0x68 */
tVisual *GrabEnvEntry(void)
{
    tVisual *v = gFreeVis;
    if (!v) return 0;
    gFreeVis = v->next;
    if (++gEnvCount > gEnvCountMax) gEnvCountMax = gEnvCount;
    v->next = 0;
    v->index = 0;
    v->objnum = -1;
    v->slot = 0;
    v->data = 0;
    v->timeStamp = 0.0f;
    v->func = 0;
    return v;
}

/* 0x80055548, size 0x10 */
void AddToEnvList(tVisual *v)
{
    v->next = gEnvList;
    gEnvList = v;
}

/* 0x80055558, size 0x74 */
void UpdateEnvirons(void)
{
    tVisual *v; /* r3 */
    tVisual *next; /* r31 */
    v = gEnvList;
    next = v ? v->next : 0;
    while (v) {
        if (v->func) v->func(v, 1);
        v = next;
        if (next) next = next->next;
    }
    AnimateSkids();
}

/* 0x800555cc, size 0xe0 */
void ReleaseEnviron(tVisual *v, int unlink)
{
    tVisual *p; /* r3 */
    if (v->objnum >= 0) MBOX_HideObject(v->objnum, 1, 15);
    v->func = 0;
    v->objnum = -1;
    if (unlink) {
        p = gEnvList;
        if (v == p) gEnvList = p->next;
        else {
            while (p && p->next != v) p = p->next;
            if (p && p->next == v) p->next = v->next;
        }
    }
    v->next = gFreeVis;
    gFreeVis = v;
    if (--gEnvCount > gEnvCountMax) gEnvCountMax = gEnvCount;
}

/* 0x800556ac, size 0x38 */
void RemoveEnvirons(void)
{
    while (gEnvList) ReleaseEnviron(gEnvList, 1);
    InitSkids();
}

/* 0x800556e4, size 0x27c */
void UpdateVisuals(signed short slot)
{
tCarData *car;
    tVisual *v;
    signed short i;
    float rtime;
    car = &gGameCar[slot];
    v = car->visuals;
    rtime = MyCountToMicro(gGfxRdpEndTime);
    if (gRetraceRate == 50.0f) gGoodFrameRate = rtime < 36000.0f;
    else if (gNumPlayers > 1) gGoodFrameRate = rtime < 45000.0f;
    else gGoodFrameRate = rtime < 30000.0f;
    if (car->appearance & 0x60) StartSmoke(slot, 4, 0, 0);
    if (model[slot].sviscode[0] == 2) StartSmoke(slot, 0, 1, 0);
    else if (car->appearance & 0x40000) StartSmoke(slot, 0, 0, 0);
    if (model[slot].sviscode[1] == 2) StartSmoke(slot, 1, 1, 0);
    else if (car->appearance & 0x80000) StartSmoke(slot, 1, 0, 0);
    if (model[slot].sviscode[2] == 2) StartSmoke(slot, 2, 1, 0);
    else if (car->appearance & 0x10000) StartSmoke(slot, 2, 0, 0);
    if (model[slot].sviscode[3] == 2) StartSmoke(slot, 3, 1, 0);
    else if (car->appearance & 0x20000) StartSmoke(slot, 3, 0, 0);
    for (i = 0; i < 22; i++, v++) if (v->func) v->func(v, 1);
    UpdateSkids(slot);
}

/* 0x80055960, size 0x11c */
static void RemoveVisuals(signed short slot)
{
    tVisual *v; /* r29 */
    signed short i; /* r28 */
    tPackedObject *shadow1[1]; /* r0 */
    tPackedObject *shadow2; /* r27 */
    tPackedObject *shadow3; /* r26 */
    v = gGameCar[slot].visuals;
    shadow1[0] = &gGameCarShadow[slot * 3];
    shadow2 = shadow1[0] + 1;
    shadow3 = shadow1[0] + 2;
    for (i = 0; i < 22; i++, v++) {
        if (v->func) v->func(v, 0);
        else if (v->objnum >= 0) {
            MBOX_HideObject(v->objnum, 1, 15);
            v->objnum = -1;
        }
    }
    if (shadow1[0]->strips) {
        DeallocateMemory(shadow1[0]->strips->vertexs);
        DeallocateMemory(shadow1[0]->strips);
        shadow1[0]->strips = 0;
    }
    if (shadow2->strips) {
        DeallocateMemory(shadow2->strips->vertexs);
        DeallocateMemory(shadow2->strips);
        shadow2->strips = 0;
    }
    if (shadow3->strips) {
        DeallocateMemory(shadow3->strips->vertexs);
        DeallocateMemory(shadow3->strips);
        shadow3->strips = 0;
    }
}

/* 0x80055a7c, size 0x1c0 */
void StartFrame(signed short slot)
{
    tVisual *v; /* r6 */
    v = &gGameCar[slot].visuals[15];
    v->func = AnimateFrame;
    v->slot = slot;
    v->objnum = gCarParts[slot].frame;
    MBOX_ShowObject(v->objnum, 0, 15);
    if (model[slot].body_type == 2) {
        gCarParts[slot].extraparts[1] = InitCarObj(gObjList[525], 15, -1, 0x42000, slot, -1, 1);
        gGameCar[slot].roktScale[1] = 0.25f;
        gCarParts[slot].extraparts[2] = InitCarObj(gObjList[526], 15, -1, 0x42000, slot, -1, 1);
        gGameCar[slot].roktScale[2] = 0.5f;
        gCarParts[slot].extraparts[0] = InitCarObj(gObjList[524], 15, -1, 0x42000, slot, -1, 1);
        gGameCar[slot].roktScale[0] = 0.25f;
    }
    if (model[slot].body_type == 3 && model[slot].PaintJob == 5)
        MBOX_ShowObject(gCarParts[slot].extraparts[0], 0, 15);
}

/* 0x80055c3c, size 0x774 */
static void AnimateRocketFlame(signed short slot, int hulk)
{
{
    tModelData *m;
    tCarData *car;
    tUVMatrix *mat;
    tUVMatrix *flames[3];
    tRGBA clr = { {255,255,255,255} };
    float scale1;
    float r;
    float pos[3];
    int mph;
    int b;
    m=&model[slot];
    car=&gGameCar[slot];
    if(hulk) {
        MBOX_HideObject(gCarParts[slot].extraparts[0],1,15);
        MBOX_HideObject(gCarParts[slot].extraparts[1],1,15);
        MBOX_HideObject(gCarParts[slot].extraparts[2],1,15);
        return;
    }
    MBOX_ShowObject(gCarParts[slot].extraparts[0],0,15);
    MBOX_ShowObject(gCarParts[slot].extraparts[1],0,15);
    MBOX_ShowObject(gCarParts[slot].extraparts[2],0,15);
    if(car->camera<0 || car->view>=2) {
        MBOX_ShowObject(gCarParts[slot].extraparts[0],0,1<<car->camera);
        MBOX_ShowObject(gCarParts[slot].extraparts[1],0,1<<car->camera);
        MBOX_ShowObject(gCarParts[slot].extraparts[2],0,1<<car->camera);
    } else {
        MBOX_HideObject(gCarParts[slot].extraparts[0],1,1<<car->camera);
        MBOX_HideObject(gCarParts[slot].extraparts[1],1,1<<car->camera);
        MBOX_HideObject(gCarParts[slot].extraparts[2],1,1<<car->camera);
    }
    if(gRace==2 && slot>0) clr.c[3]=128;
    else if(gRace==6 && (car->battle.defense.active&1)) clr.c[3]=car->battle.fadeAlpha;
    ZOID_SetColor(gCarParts[slot].extraparts[0], &clr);
    ZOID_SetColor(gCarParts[slot].extraparts[1], &clr);
    ZOID_SetColor(gCarParts[slot].extraparts[2], &clr);
    mat=ZOID_FindMatrix(gCarParts[slot].frame);
    pos[0]=mat->mat3.pos[0]+2.0f*mat->mat3.uvs[1][0];
    pos[1]=mat->mat3.pos[1]+2.0f*mat->mat3.uvs[1][1];
    pos[2]=mat->mat3.pos[2]+2.0f*mat->mat3.uvs[1][2];
    pos[0]+=-5.6f*mat->mat3.uvs[2][0];
    pos[1]+=-5.6f*mat->mat3.uvs[2][1];
    pos[2]+=-5.6f*mat->mat3.uvs[2][2];
    flames[0]=ZOID_FindMatrix(gCarParts[slot].extraparts[0]);
    CopyUV(mat->mat3.uvs,flames[0]->mat3.uvs);
    flames[0]->mat3.pos[0]=pos[0];
    flames[0]->mat3.pos[1]=pos[1];
    flames[0]->mat3.pos[2]=pos[2];
    flames[1]=ZOID_FindMatrix(gCarParts[slot].extraparts[1]);
    CopyUV(mat->mat3.uvs,flames[1]->mat3.uvs);
    flames[1]->mat3.pos[0]=pos[0];
    flames[1]->mat3.pos[1]=pos[1];
    flames[1]->mat3.pos[2]=pos[2];
    flames[2]=ZOID_FindMatrix(gCarParts[slot].extraparts[2]);
    CopyUV(mat->mat3.uvs,flames[2]->mat3.uvs);
    flames[2]->mat3.pos[0]=pos[0];
    flames[2]->mat3.pos[1]=pos[1];
    flames[2]->mat3.pos[2]=pos[2];
    b=0;
    if(gRace==6 && car->battle.weapon.type==3) {
        b=1;
        car->roktScale[0]=0.1f;
        car->roktScale[1]=0.1f;
        car->roktScale[2]=0.2f;
    } else if((car->rwmph>>2)<0) {
        car->roktScale[0]=0.25f-Random(0.15f);
        car->roktScale[1]=0.25f-Random(0.15f);
        car->roktScale[2]=0.55f-Random(0.1f);
    } else if(m->throttle<0.1f) {
        if(car->roktScale[0]<0.28f) car->roktScale[0]=0.25f-Random(0.15f);
        else {
            car->roktScale[0]-=0.01f+Random(0.025f);
            if(car->roktScale[0]<0.2f) car->roktScale[0]=0.2f;
        }
        if(car->roktScale[1]<0.28f) car->roktScale[1]=0.25f-Random(0.15f);
        else {
            car->roktScale[1]-=0.01f+Random(0.025f);
            if(car->roktScale[1]<0.2f) car->roktScale[1]=0.2f;
        }
        if(car->roktScale[2]<0.58f) car->roktScale[2]=0.55f-Random(0.1f);
        else {
            car->roktScale[2]-=0.01f+Random(0.025f);
            if(car->roktScale[2]<0.5f) car->roktScale[2]=0.5f;
        }
    } else {
        mph=m->rpm;
        scale1=0.2f+(1.35f*mph)/10000.0f;
        if(scale1>1.0f) scale1=1.0f;
        car->roktScale[0]=scale1-Random(0.15f);
        car->roktScale[1]=scale1-Random(0.15f);
        scale1=0.5f+mph/10000.0f;
        if(scale1>1.0f) scale1=1.0f;
        car->roktScale[2]=scale1-Random(0.1f);
    }
    flames[0]->mat3.uvs[2][0]*=car->roktScale[0];
    flames[0]->mat3.uvs[2][1]*=car->roktScale[0];
    flames[0]->mat3.uvs[2][2]*=car->roktScale[0];
    flames[1]->mat3.uvs[2][0]*=car->roktScale[1];
    flames[1]->mat3.uvs[2][1]*=car->roktScale[1];
    flames[1]->mat3.uvs[2][2]*=car->roktScale[1];
    if(b) ScaleUV(flames[2]->mat3.uvs,flames[2]->mat3.uvs,car->roktScale[2]);
    else {
        flames[2]->mat3.uvs[0][0]*=car->roktScale[2];
        flames[2]->mat3.uvs[0][1]*=car->roktScale[2];
        flames[2]->mat3.uvs[0][2]*=car->roktScale[2];
        flames[2]->mat3.uvs[1][0]*=car->roktScale[2];
        flames[2]->mat3.uvs[1][1]*=car->roktScale[2];
        flames[2]->mat3.uvs[1][2]*=car->roktScale[2];
    }
    if(gRace==6 && m->resurrect.moving_state>=0) {
        ObjInstanceList[gCarParts[slot].extraparts[0]].flags |=0x80000000;
        ObjInstanceList[gCarParts[slot].extraparts[1]].flags |=0x80000000;
        ObjInstanceList[gCarParts[slot].extraparts[2]].flags |=0x80000000;
    } else if(gInvisibleCar || (car->appearance&8)) {
        ObjInstanceList[gCarParts[slot].extraparts[0]].flags ^=0x80000000;
        ObjInstanceList[gCarParts[slot].extraparts[1]].flags ^=0x80000000;
        ObjInstanceList[gCarParts[slot].extraparts[2]].flags ^=0x80000000;
    } else {
        ObjInstanceList[gCarParts[slot].extraparts[0]].flags &=0x7fffffff;
        ObjInstanceList[gCarParts[slot].extraparts[1]].flags &=0x7fffffff;
        ObjInstanceList[gCarParts[slot].extraparts[2]].flags &=0x7fffffff;
    }
}
}

/* 0x800563b0, size 0x2cc */
static inline unsigned int _h(unsigned char *p, tCarData *car, unsigned int i0, unsigned int i1) { return p[i0*4] + p[i1*4] + p[((car->sound_flags[2]&0xf800)>>11)*4] + p[((car->sound_flags[3]&0xf800)>>11)*4]; }

static inline unsigned int _h0(unsigned char *p, tCarData *car, unsigned int i0, unsigned int i1) { return p[i0*4] + p[i1*4] + p[((car->sound_flags[2]&0xf800)>>11)*4] + p[((car->sound_flags[3]&0xf800)>>11)*4]; }

static tRGBA GetCarsIRColor(tCarData *car, tModelData *m)
{
tRGBA clr;
    tRGBA sclr2;
    float d1;
    float d2;
    float weights[2];
    unsigned int a;
    unsigned int b;
    unsigned int hobj;
    unsigned char rgb[3];
    unsigned char *column[3];
    static int gTrackBaseColors[19] = { 3, 3, 12, 6, 12, 5, 3, 3, 3, 6, 3, 3, 12, 5, 6, 6, 3, 3, 5 };




















    a=(car->sound_flags[0]&0xf800)>>11;
    hobj=(car->sound_flags[1]&0xf800)>>11;
    column[0]=(unsigned char *)gTrackColors[gTrackNum];
    column[2]=&gTrackColors[gTrackNum][0].c[2];


    b = _h0(column[0], car, a, hobj);
    rgb[0] = b >> 2;

    b = (&gTrackColors[gTrackNum][0].c[1])[(a) * (int)sizeof(tRGBA)] + (&gTrackColors[gTrackNum][0].c[1])[(hobj) * (int)sizeof(tRGBA)] + (&gTrackColors[gTrackNum][0].c[1])[((car->sound_flags[2]&0xf800)>>11) * (int)sizeof(tRGBA)] + (&gTrackColors[gTrackNum][0].c[1])[((car->sound_flags[3]&0xf800)>>11) * (int)sizeof(tRGBA)];
    rgb[1] = b >> 2;

    b = column[2][(a) * (int)sizeof(tRGBA)] + column[2][(hobj) * (int)sizeof(tRGBA)] + column[2][((car->sound_flags[2]&0xf800)>>11) * (int)sizeof(tRGBA)] + column[2][((car->sound_flags[3]&0xf800)>>11) * (int)sizeof(tRGBA)];
    rgb[2] = b >> 2;








    if (m->airdist[2] >= 20.0f)
    {
        d1 = 0.025f * (m->airdist[2] - 20.0f);
        if (d1 > 1.0f)
            return gTrackColors[gTrackNum][gTrackBaseColors[gTrackNum]];
        if (d1 > 0.0f) {
            d2 = 1.0f - d1;
            sclr2 = gTrackColors[gTrackNum][gTrackBaseColors[gTrackNum]];
            b = sclr2.c[0];
            rgb[0] = rgb[0] * d2 + b * d1;
            b = sclr2.c[1];
            rgb[1] = rgb[1] * d2 + b * d1;
            b = sclr2.c[2];
            rgb[2] = rgb[2] * d2 + b * d1;
        }
    }

    weights[0]=0.34f;
    weights[1]=0.66f;



    a = car->prevIRclr.c[0];
    clr.c[0] = rgb[0] * weights[0] + a * weights[1];
    a = car->prevIRclr.c[1];
    clr.c[1] = rgb[1] * weights[0] + a * weights[1];
    a = car->prevIRclr.c[2];
    clr.c[2] = rgb[2] * weights[0] + a * weights[1];



































    clr.c[3] = 255;

    return clr;
}

/* 0x8005667c, size 0x8e4 */
static void AnimateFrame(tVisual *v, signed short op)
{
{
    int hulk;
    signed short slot;
    tModelData *m;
    tCarData *car;
    int def;
    unsigned int flags;
    unsigned int hobj;
    unsigned int sobj;
    int rear;
    unsigned int tobj;
    unsigned int eobj;
    tRGBA clr;
    tRGBA sclr1;
    tRGBA sclr2;
    tRGBA nextclr;
    slot=v->slot;
    m=&model[slot];
    car=&gGameCar[slot];
    def=ObjInstanceList[v->objnum].index;
    if(!op) {
        if(v->objnum>=0) MBOX_HideObject(v->objnum,1,15);
        v->func=0;
        v->objnum=-1;
        return;
    }
    hulk=(car->appearance>>4)&1;
    sobj=gCarParts[slot].sheen;
    hobj=gCarParts[slot].hood;
    tobj=gCarParts[slot].trim;
    eobj=m->body_type==3?gCarParts[slot].extraparts[0]:0;
    if(eobj) {
        MBOX_ShowObject(eobj,0,15);
        if(car->camera<0 || car->view>=2) MBOX_ShowObject(gCarParts[slot].extraparts[0],0,1<<car->camera);
        else MBOX_HideObject(gCarParts[slot].extraparts[0],1,1<<car->camera);
    }
    if(m->body_type==2) AnimateRocketFlame(slot,hulk);
    nextclr=GetCarsIRColor(car,m);
    car->prevIRclr=car->IRclr;
    car->IRclr=nextclr;
    clr=nextclr;
    sclr1=SheenColors[gTrackNum][0];
    sclr2=SheenColors[gTrackNum][1];
    if(car->in_tunnel && !hulk) {
        if(car->sheenAlpha<32) car->sheenAlpha=0;
        else car->sheenAlpha-=25;
    } else {
        if(car->sheenAlpha>224) car->sheenAlpha=255;
        else car->sheenAlpha+=25;
    }
    sclr1.c[3]=car->sheenAlpha;
    sclr2.c[3]=car->sheenAlpha;
    if(gRace==2 && slot>0) clr.c[3]=128;
    else if(gRace==6 && (car->battle.defense.active&1)) {
        clr.c[3]=car->battle.fadeAlpha;
        sclr1.c[3]=car->battle.fadeAlpha;
        sclr2.c[3]=car->battle.fadeAlpha;
    } else clr.c[3]=255;
    SetFrameColor(slot,&clr);
    SetSheenColor(slot,&sclr1,&sclr2);
    if(eobj) ZOID_SetColor(eobj,&clr);
    if(hulk) {
        car->carMine=0;
        if(def!=PickCar(slot,13)) ZOID_SetObjectDef(v->objnum,PickCar(slot,13));
        MBOX_HideObject(hobj,1,15);
        if(eobj) MBOX_HideObject(eobj,1,15);
        if(tobj) MBOX_HideObject(tobj,1,15);
        ZOID_SetObjectDef(sobj,gObjList[slot*9+5]);
        ZOID_SetTexture(sobj,&gTexDefPtrs[gTexList[12]>>10].texDefs[gTexList[12]&1023],-1);
        flags=ZOID_ObjectFlags(sobj);
        ZOID_SetObjectFlags(sobj,flags&0xffd7ffff);
        MBOX_ShowObject(sobj,0,15);
        CheckVisible(v,1,car->camera);
        return;
    }
    if((gGameState&0x100) && gCarMines && !car->carMine) {
        gObjList[550]=MBOX_FindObject_WORK("MINE1",0,gNumModels-1,0);
        car->carMine=1;
    }
    if((gGameState&0x100) && !gCarMines && car->carMine) car->carMine=0;
    if(car->carMine) {
        if(def!=gObjList[550]) ZOID_SetObjectDef(v->objnum,gObjList[550]);
        MBOX_HideObject(hobj,1,15);
        MBOX_HideObject(sobj,1,15);
        CheckVisible(v,car->view>=2,car->camera);
        return;
    }
    if(gGameState&0x100) return;
    if(tobj) {
        if(gInvisibleCar || (car->appearance&8)) ObjInstanceList[tobj].flags^=0x80000000;
        else ObjInstanceList[tobj].flags&=0x7fffffff;
        if(car->camera<0 || car->view>=2) MBOX_ShowObject(tobj,0,1<<car->camera);
        else MBOX_HideObject(tobj,1,1<<car->camera);
    }
    CheckVisible(v,car->view>=2,car->camera);
    if(car->in_tunnel) {
        if(car->sheenAlpha<=16) MBOX_HideObject(sobj,1,15);
        else {
            sclr1.c[3]=car->sheenAlpha;
            sclr2.c[3]=car->sheenAlpha;
            SetSheenColor(slot,&sclr1,&sclr2);
        }
    } else if(car->camera<0 || car->view>=2) {
        if(model[slot].body_type==10 && model[slot].PaintJob==6) MBOX_HideObject(sobj,1,15);
        else {
            ZOID_SetObjectDef(sobj,gObjList[slot*9+2]);
            ZOID_SetTexture(sobj,&gTexDefPtrs[gTexList[11]>>10].texDefs[gTexList[11]&1023],-1);
            flags=ZOID_ObjectFlags(sobj);
            ZOID_SetObjectFlags(sobj,flags|0x280000);
            MBOX_ShowObject(sobj,0,15);
        }
    } else MBOX_HideObject(sobj,1,1<<car->camera);
    if(def!=PickCar(slot,model[slot].body_type)) ZOID_SetObjectDef(v->objnum,PickCar(slot,model[slot].body_type));
    if(gRace==6 && m->resurrect.moving_state>=0) {
        ObjInstanceList[v->objnum].flags|=0x80000000;
        if(eobj) ObjInstanceList[eobj].flags|=0x80000000;
    } else if(gInvisibleCar || (car->appearance&8)) {
        ObjInstanceList[v->objnum].flags^=0x80000000;
        if(eobj) ObjInstanceList[eobj].flags^=0x80000000;
    } else {
        ObjInstanceList[v->objnum].flags&=0x7fffffff;
        if(eobj) ObjInstanceList[eobj].flags&=0x7fffffff;
    }
    rear=(gPlayers[car->slot].levels&gPlayers[car->slot].controls[9])!=0;
    if(car->view==1 && rear) MBOX_HideObject(hobj,1,15);
    else if(car->view!=1 || gNumWindows>1) MBOX_HideObject(hobj,1,15);
    else {
        flags=ZOID_ObjectFlags(hobj);
        if((int)ObjInstanceList[hobj].index!=(int)gObjList[(signed short)(slot*9+1)]) ZOID_SetObjectDef(hobj,gObjList[(signed short)(slot*9+1)]);
        if(flags&(0x100<<car->camera)) MBOX_ShowObject(hobj,0,1<<car->camera);
        if(gInvisibleCar || (car->appearance&8)) ObjInstanceList[hobj].flags^=0x80000000;
        else ObjInstanceList[hobj].flags&=0x7fffffff;
    }
}
}

float wingOffsets[13][3] = {
    4.0f, 1.0f, 0.0f, 4.8f, 1.0f, 0.0f, 4.0f, 1.0f, 0.0f, 4.8f, 1.0f, 0.0f, 4.6f, 1.0f, 0.0f, 4.6f,
    1.0f, 0.0f, 4.5f, 1.0f, 0.0f, 4.5f, 1.0f, 0.0f, 4.5f, 1.0f, 0.0f, 5.5f, 1.0f, 0.0f, 5.0f, 3.0f,
    0.0f, 4.9f, 1.0f, 0.0f, 4.8f, 1.0f, 0.0f
};

/* 0x80056f60, size 0x16c */
static void StartWing(signed short slot, signed short side)
{
    tVisual *v; /* r31 */
    float iOffset[3]; /* stack */
    v = &gGameCar[slot].visuals[17 + side];
    if (gRace == 2 || gRace == 6 || model[slot].we_control) {
        v->func = 0;
        return;
    }
    v->func = AnimateWing;
    v->slot = slot;
    v->data = side;
    iOffset[0] = 0.0f;
    iOffset[1] = wingOffsets[model[slot].body_type][1];
    iOffset[2] = 0.0f;
    {
        int index = side;
        index += 4 * gCarWings[slot + 1][model[slot].body_type];
        index += 538;
        v->objnum = InitCarObj(gObjList[index], 15, gCarParts[slot].frame, 0, slot, -1, 1);
    }
    ZOID_UpdateObject(v->objnum, iOffset, 0);
    MBOX_HideObject(v->objnum, 1, 15);
    InitWingSmoke(slot, side);
}

/* 0x800570cc, size 0x4bc */
static void AnimateWing(tVisual *v, signed short op)
{
int hulk;
    signed short slot;
    tCarData *car;
    tModelData *m;
    float offset[3];
    float newUVS[3][3];
    slot = v->slot;
    car = &gGameCar[slot];
    m = &model[slot];
    if (!op) {
        if (v->objnum >= 0) MBOX_HideObject(v->objnum, 0, 15);
        v->func = 0;
        v->objnum = -1;
        return;
    }
    {
    hulk = v->data & 1;
    offset[0] = wingOffsets[m->body_type][0] * (hulk ? 1.0f : -1.0f);
    offset[1] = wingOffsets[m->body_type][1];
    offset[2] = 0.0f;
    if (!m->wing || gCoastFlag) {
        if (v->data & 0x10) {
            v->data &= ~0x100;
            v->data |= 0x1200;
            if (m->wingSnd == -1) {
                m->wingSnd = AudPlayEffect(77, m->slot, 2, 2);
                AudChangeEffectPitch(m->wingSnd, 0.75f);
                AudChangeEffectVol(m->wingSnd, 0.5f);
            }
        } else if (!(v->data & 0x1000)) {
            InitWingSmoke(slot, hulk);
            MBOX_HideObject(v->objnum, 0, 15);
            return;
        }
    }
    if (m->wing && !(v->data & 0x10)) {
        MBOX_ShowObject(v->objnum, 0, 15);
        v->data |= 0x1110;
        v->timeStamp = 0.015f + gModelTime;
        v->index = 0;
        if (m->wingSnd == -1) {
            m->wingSnd = AudPlayEffect(77, m->slot, 2, 2);
            AudChangeEffectPitch(m->wingSnd, 0.75f);
            AudChangeEffectVol(m->wingSnd, 0.5f);
        }
    }
    if (v->data & 0x1000) {
        if (v->data & 0x100) {
deploy:
            if (v->timeStamp < gModelTime) {
                offset[0] *= (float)v->index / 6.0f;
                ZOID_UpdateObject(v->objnum, offset, 0);
                v->timeStamp = 0.015f + gModelTime;
                v->index++;
                if (v->index == 6) {
                    v->data &= ~0x1100;
                    if (m->wingSnd != -1) {
                        AudStopEffect(m->wingSnd);
                        m->wingSnd = -1;
                    }
                }
            }
        } else if (v->data & 0x200) {
            if (m->wing && !gCoastFlag) {
                v->data &= ~0x200;
                v->data |= 0x100;
                goto deploy;
            }
            if (v->timeStamp < gModelTime) {
                offset[0] *= (float)v->index / 6.0f;
                ZOID_UpdateObject(v->objnum, offset, 0);
                v->timeStamp = 0.015f + gModelTime;
                v->index--;
                if (v->index == -1) {
                    v->data &= ~0x1210;
                    if (m->wingSnd != -1) {
                        AudStopEffect(m->wingSnd);
                        m->wingSnd = -1;
                    }
                }
            }
        }
    }
    if (v->data & 0x10) {
        CopyUV(unitmatrix, newUVS);
        PitchUV(0.507f * car->player->analog[1], newUVS);
        ZOID_UpdateObject(v->objnum, 0, newUVS);
        StartSmoke(slot, 6 + (hulk != 0), 0, offset);
    }
    hulk = (car->appearance & 0x10) != 0;
    {
        int inv = gInvisibleCar || (car->appearance & 8);
        if (CheckVisibleAll(v, !hulk && !car->carMine)) {
            CheckVisible(v, car->view >= 2, car->camera);
            if (inv) ObjInstanceList[v->objnum].flags ^= 0x80000000;
            else ObjInstanceList[v->objnum].flags &= 0x7fffffff;
        }
    }
}
}

/* 0x80057588, size 0x120 */
static void StartWingFlame(signed short slot, signed short side)
{
    tVisual *v; /* r31 */
    tUVMatrix *mat; /* r0 */
    v = &gGameCar[slot].visuals[19 + side];
    if (gRace == 2 || gRace == 6 || model[slot].we_control) {
        v->func = 0;
        return;
    }
    v->func = AnimateWingFlame;
    v->slot = slot;
    v->data = side;
    {
        int index = side;
        index += 4 * gCarWings[slot + 1][model[slot].body_type];
        index += 540;
        v->objnum = InitCarObj(gObjList[index], 15, gGameCar[slot].visuals[17 + side].objnum, 0, slot, -1, 1);
    }
    mat = ZOID_FindMatrix(v->objnum);
    mat->mat3.pos[1] = 0.55f;
    MBOX_HideObject(v->objnum, 1, 15);
}

/* 0x800576a8, size 0x1e8 */
static void AnimateWingFlame(tVisual *v, signed short op)
{
    int hulk; /* r4 */
    tUVMatrix *mat; /* r28 */
    tCarData *car; /* r31 */
    float scale; /* f1 */
    int index;
    car = &gGameCar[v->slot];
    if (!op) {
        if (v->objnum >= 0) MBOX_HideObject(v->objnum, 1, 15);
        v->func = 0;
        v->objnum = -1;
        return;
    }
    hulk = v->data & 1;
    index = hulk;
    index += 17;
    if (((unsigned int *)car)[index * (sizeof(tVisual)/sizeof(unsigned int)) + (0x110+0xc)/sizeof(unsigned int)] & 0x10) {
        MBOX_ShowObject(v->objnum, 0, 15);
        mat = ZOID_FindMatrix(v->objnum);
        CopyUV(unitmatrix, mat->mat3.uvs);
        scale = hulk ? 8.0f * car->player->analog[0] : -8.0f * car->player->analog[0];
        if (scale < 2.0f) scale = 2.0f;
        mat->mat3.uvs[1][1] = scale;
    } else MBOX_HideObject(v->objnum, 1, 15);
    hulk = (car->appearance & 0x10) != 0;
    {
        int inv = gInvisibleCar || (car->appearance & 8);
        if (CheckVisibleAll(v, !hulk && !car->carMine)) {
            CheckVisible(v, car->view >= 2, car->camera);
            if (inv) ObjInstanceList[v->objnum].flags ^= 0x80000000;
            else ObjInstanceList[v->objnum].flags &= 0x7fffffff;
        }
    }
}

/* 0x80057890, size 0x108 */
void InitBlast(void)
{
    signed short i; /* r29 */
    for (i = 0; i < 50; i++) gBlastMats[i].mat = ZOID_NewMatrix();
    for (i = 0; i < 6; i++) {
        gBlastRingObjs[i].ringIdx = -1;
        gBlastObjs[i].glowIdx = -1;
        gBlastObjs[i].blastObjs[0].mbIdx = -1;
        gBlastObjs[i].blastObjs[1].mbIdx = -1;
        gBlastObjs[i].blastObjs[2].mbIdx = -1;
        gBlastObjs[i].blastObjs[3].mbIdx = -1;
    }
    gBlastMatCnt = 0;
}

/* 0x80057998, size 0x330 */
static void StartNewBlast(signed short slot)
{
tVisual *v;
    int i;
    tRGBA clr = { 255, 255, 255, 255 };
    v = GrabEnvEntry();
    if (v) {
        v->func = AnimateNewBlast;
        v->timeStamp = gModelTime;
        v->slot = slot;
        clr.c[0] = 244;
        clr.c[1] = 205;
        clr.c[2] = 20;
        for (i = 0; i < 4; i++) {
            if (gBlastObjs[slot].blastObjs[i].mbIdx != -1) ZOID_RemoveObject(gBlastObjs[slot].blastObjs[i].mbIdx);
            CopyUV(unitmatrix, gBlastObjs[slot].blastObjs[i].mat.mat3.uvs);
            gBlastObjs[slot].blastObjs[i].mat.mat3.pos[0] = gGameCar[slot].dr_pos[0];
            gBlastObjs[slot].blastObjs[i].mat.mat3.pos[1] = gGameCar[slot].dr_pos[1] - 3.0f;
            gBlastObjs[slot].blastObjs[i].mat.mat3.pos[2] = gGameCar[slot].dr_pos[2];
            gBlastObjs[slot].blastObjs[i].mbIdx = (signed short)MBOX_NewObject(gObjList[470 + i], &gBlastObjs[slot].blastObjs[i].mat, -1, 0x42000);
            gBlastObjs[slot].blastObjs[i].scale = 0.4f;
            gBlastObjs[slot].blastObjs[i].scaleInc = 0.05f + Random(0.005f);
            gBlastObjs[slot].blastObjs[i].mat.mat3.uvs[0][0] = gBlastObjs[slot].blastObjs[i].mat.mat3.uvs[1][1] = gBlastObjs[slot].blastObjs[i].mat.mat3.uvs[2][2] = gBlastObjs[slot].blastObjs[i].scale;
            gBlastObjs[slot].blastObjs[i].yawInc = 0.1f - Random(0.2f);
            gBlastObjs[slot].blastObjs[i].pitchInc = 0.1f - Random(0.2f);
            clr.c[3] = 128;
            gBlastObjs[slot].blastObjs[i].ht = 0.0f;
            gBlastObjs[slot].blastObjs[i].yaw = 0.0f;
            gBlastObjs[slot].blastObjs[i].pitch = 0.0f;
            gBlastObjs[slot].blastObjs[i].alpha = 128;
            gBlastObjs[slot].blastObjs[i].alphaInc = (int)Random(10.0f) + 17;
            ZOID_SetColor(gBlastObjs[slot].blastObjs[i].mbIdx, &clr);
        }
        CopyUV(unitmatrix, gBlastObjs[slot].mat.mat3.uvs);
        gBlastObjs[slot].mat.mat3.pos[0] = gGameCar[slot].dr_pos[0];
        gBlastObjs[slot].mat.mat3.pos[1] = gGameCar[slot].dr_pos[1];
        gBlastObjs[slot].mat.mat3.pos[2] = gGameCar[slot].dr_pos[2];
        if (gBlastObjs[slot].glowIdx != -1) ZOID_RemoveObject(gBlastObjs[slot].glowIdx);
        gBlastObjs[slot].glowIdx = (signed short)MBOX_NewObject(gObjList[474], &gBlastObjs[slot].mat, -1, 0x52000);
        gBlastObjs[slot].alpha = 56;
        gBlastObjs[slot].alphaCnt = 0;
        ObjInstanceList[gBlastObjs[slot].glowIdx].scale = 0.2f;
        ObjInstanceList[gBlastObjs[slot].glowIdx].w = 1.4f;
        clr.c[0] = 255;
        clr.c[1] = 255;
        clr.c[2] = 255;
        clr.c[3] = gBlastObjs[slot].alpha;
        ZOID_SetColor(gBlastObjs[slot].glowIdx, &clr);
        gBlastObjs[slot].lifeTime = 0.7f;
        gBlastObjs[slot].incTime = 0.0333333f;
        AddToEnvList(v);
    }
    StartSmoke(slot, 5, 0, 0);
}

/* 0x80057cc8, size 0x590 */
static void AnimateNewBlast(tVisual *v, signed short op)
{
int i;
    tBlastObj *b;
    tRGBA clr;
    float x;
    float y;
    if (gPauseState) return;
    gBlastObjs[v->slot].lifeTime -= gFrameSeconds;
    if (gBlastObjs[v->slot].lifeTime <= 0.0f || model[v->slot].resurrect.moving_state >= 0) {
        for (i = 0; i < 4; i++) {
            b = &gBlastObjs[v->slot].blastObjs[i];
            if (b->mbIdx != -1) ZOID_RemoveObject(b->mbIdx);
            b->mbIdx = -1;
        }
    }
    if (!model[v->slot].crashflag) {
        if (gBlastObjs[v->slot].glowIdx != -1) {
            ZOID_RemoveObject(gBlastObjs[v->slot].glowIdx);
            gBlastObjs[v->slot].glowIdx = -1;
        }
        for (i = 0; i < 4; i++) {
            b = &gBlastObjs[v->slot].blastObjs[i];
            if (b->mbIdx != -1) ZOID_RemoveObject(b->mbIdx);
            b->mbIdx = -1;
        }
        goto release;
    }
    if (!op) {
release:
        ReleaseEnviron(v, 1);
        return;
    }
    gBlastObjs[v->slot].mat.mat3.pos[0] = gGameCar[v->slot].dr_pos[0];
    gBlastObjs[v->slot].mat.mat3.pos[1] = gGameCar[v->slot].dr_pos[1];
    gBlastObjs[v->slot].mat.mat3.pos[2] = gGameCar[v->slot].dr_pos[2];
    gBlastObjs[v->slot].incTime -= gFrameSeconds;
    if (gBlastObjs[v->slot].incTime <= 0.0f) {
        if (gBlastObjs[v->slot].alphaCnt == 0) gBlastObjs[v->slot].alpha -= 16;
        else if (gBlastObjs[v->slot].alphaCnt == 1) {
            if (gBlastObjs[v->slot].alpha < 56U) gBlastObjs[v->slot].alpha += 16;
        } else if (gBlastObjs[v->slot].alphaCnt > 1) {
            gBlastObjs[v->slot].alphaCnt = (int)Random(3.0f) * -1;
            gBlastObjs[v->slot].alpha = 56;
        }
        gBlastObjs[v->slot].alphaCnt++;
        clr.c[0] = 255;
        clr.c[1] = 255;
        clr.c[2] = 255;
        clr.c[3] = gBlastObjs[v->slot].alpha;
        ZOID_SetColor(gBlastObjs[v->slot].glowIdx, &clr);
        if (ObjInstanceList[gBlastObjs[v->slot].glowIdx].scale < 1.0f)
            ObjInstanceList[gBlastObjs[v->slot].glowIdx].scale += 0.05f;
        for (i = 0; i < 4; i++) {
            b = &gBlastObjs[v->slot].blastObjs[i];
            if (b->mbIdx != -1) {
                b->mat.mat3.uvs[0][0] = b->mat.mat3.uvs[1][1] = b->mat.mat3.uvs[2][2] = b->scale;
                b->mat.mat3.uvs[0][1] = b->mat.mat3.uvs[1][0] = b->mat.mat3.uvs[2][0] = 0.0f;
                b->mat.mat3.uvs[0][2] = b->mat.mat3.uvs[1][2] = b->mat.mat3.uvs[2][1] = 0.0f;
                switch (i) {
                    case 0: x = 0.0f; y = 0.0f; break;
                    case 1: x = 0.5f * b->ht; y = 0.5f * b->ht; break;
                    case 2: x = -0.5f * b->ht; y = -0.5f * b->ht; break;
                    case 3: x = -0.5f * b->ht; y = 0.5f * b->ht; break;
                }
                b->mat.mat3.pos[0] = x + gGameCar[v->slot].dr_pos[0];
                b->mat.mat3.pos[2] = y + gGameCar[v->slot].dr_pos[2];
                b->mat.mat3.pos[1] = b->ht + gGameCar[v->slot].dr_pos[1] - 3.0f;
                b->ht += 0.2f;
                PitchUV(b->pitch, b->mat.mat3.uvs);
                YawUV(b->yaw, b->mat.mat3.uvs);
                b->scale += b->scaleInc;
                b->scaleInc -= 0.002f;
                b->yaw += b->yawInc;
                b->pitch += b->pitchInc;
                if (gBlastObjs[v->slot].lifeTime < 0.2f) {
                    clr.c[0] = 244;
                    clr.c[1] = 205;
                    clr.c[2] = 20;
                    clr.c[3] = b->alpha;
                    if (b->alpha > b->alphaInc) b->alpha -= b->alphaInc;
                    ZOID_SetColor(b->mbIdx, &clr);
                }
            }
        }
        gBlastObjs[v->slot].incTime = 0.0333333f;
    }
}

/* 0x80058258, size 0x134 */
static void StartNewBlastRing(signed short slot)
{
    tVisual *v; /* r29 */
    tRGBA clr = { 255, 255, 255, 255 }; /* stack */
    v = GrabEnvEntry();
    if (!v) return;
    v->func = AnimateNewBlastRing;
    v->timeStamp = gModelTime;
    v->slot = slot;
    CopyUV(unitmatrix, gBlastRingObjs[slot].mat.mat3.uvs);
    gBlastRingObjs[slot].mat.mat3.pos[0] = gGameCar[slot].dr_pos[0];
    gBlastRingObjs[slot].mat.mat3.pos[1] = gGameCar[slot].dr_pos[1];
    gBlastRingObjs[slot].mat.mat3.pos[2] = gGameCar[slot].dr_pos[2];
    if (gBlastRingObjs[slot].ringIdx != -1) ZOID_RemoveObject(gBlastRingObjs[slot].ringIdx);
    gBlastRingObjs[slot].ringIdx = (signed short)MBOX_NewObject(gObjList[475], &gBlastRingObjs[slot].mat, -1, 0x42000);
    gBlastRingObjs[slot].ralpha = 240;
    gBlastRingObjs[slot].ralphaCnt = 0;
    ZOID_SetColor(gBlastRingObjs[slot].ringIdx, &clr);
    gBlastRingObjs[slot].incTime = 0.0333333f;
    AddToEnvList(v);
}

/* 0x8005838c, size 0x258 */
static void AnimateNewBlastRing(tVisual *v, signed short op)
{
    tRGBA clr; /* stack */
    if (gPauseState) return;
    if (!model[v->slot].crashflag) {
remove:
        if (gBlastRingObjs[v->slot].ringIdx != -1) {
            ZOID_RemoveObject(gBlastRingObjs[v->slot].ringIdx);
            gBlastRingObjs[v->slot].ringIdx = -1;
        }
        goto release;
    }
    if (!op) {
release:
        ReleaseEnviron(v, 1);
        return;
    }
    gBlastRingObjs[v->slot].mat.mat3.pos[0] = gGameCar[v->slot].dr_pos[0];
    gBlastRingObjs[v->slot].mat.mat3.pos[1] = gGameCar[v->slot].dr_pos[1];
    gBlastRingObjs[v->slot].mat.mat3.pos[2] = gGameCar[v->slot].dr_pos[2];
    gBlastRingObjs[v->slot].incTime -= gFrameSeconds;
    if (gBlastRingObjs[v->slot].incTime <= 0.0f) {
        gBlastRingObjs[v->slot].mat.mat3.uvs[0][0] += 1.5f;
        gBlastRingObjs[v->slot].mat.mat3.uvs[1][1] += 1.5f;
        gBlastRingObjs[v->slot].mat.mat3.uvs[2][2] += 1.5f;
        if (gBlastRingObjs[v->slot].ralphaCnt > 15) {
            if (gBlastRingObjs[v->slot].ringIdx != -1) {
                ZOID_RemoveObject(gBlastRingObjs[v->slot].ringIdx);
                gBlastRingObjs[v->slot].ringIdx = -1;
            }
            goto release;
        }
        if (gBlastRingObjs[v->slot].ralphaCnt > 6) {
            clr.c[0] = 255;
            clr.c[1] = 255;
            clr.c[2] = 255;
            clr.c[3] = gBlastRingObjs[v->slot].ralpha;
            ZOID_SetColor(gBlastRingObjs[v->slot].ringIdx, &clr);
            if (gBlastRingObjs[v->slot].ralpha > 24) gBlastRingObjs[v->slot].ralpha -= 24;
        }
        gBlastRingObjs[v->slot].ralphaCnt++;
        gBlastRingObjs[v->slot].incTime = 0.0333333f;
    }
}

/* 0x800585e4, size 0x1dc */
void StartBlast(void * data, int linkToCar, float scale, int playSound)
{
    tVisual *v; /* r31 */
    tUVMatrix *mat; /* r30 */
    if (linkToCar) StartNewBlastRing(*(signed short *)data);
    v = GrabEnvEntry();
    if (!v) return;
    mat = gBlastMats[gBlastMatCnt].mat;
    CopyUV(unitmatrix, mat->mat3.uvs);
    v->func = AnimateBlast;
    v->timeStamp = 0.0333333f;
    if (linkToCar) {
        v->slot = *(signed short *)data;
        mat->mat3.pos[0] = gGameCar[v->slot].dr_pos[0];
        mat->mat3.pos[1] = gGameCar[v->slot].dr_pos[1];
        mat->mat3.pos[2] = gGameCar[v->slot].dr_pos[2];
    } else {
        v->slot = -1;
        mat->mat3.pos[0] = ((float *)data)[0];
        mat->mat3.pos[1] = ((float *)data)[1];
        mat->mat3.pos[2] = ((float *)data)[2];
    }
    v->index = 0;
    v->objnum = MBOX_NewObject(gObjList[476], mat, -1, 0x50000);
    AddToEnvList(v);
    ObjInstanceList[v->objnum].scale = scale;
    if (++gBlastMatCnt >= 50) gBlastMatCnt = 0;
    if (playSound) EmitGameSound(mat->mat3.pos, unitmatrix[3], 400.0f, 0.0f, 1.0f, 0.0f, scale >= 1.0f ? 9 : scale >= 0.5f ? 61 : 62, v->objnum, 0, 128);
}

/* 0x800587c0, size 0x1ac */
static void AnimateBlast(tVisual *v, signed short op)
{
    if (gPauseState) return;
    if (!op) {
release:
        ZOID_RemoveObject(v->objnum);
        v->objnum = -1;
        ReleaseEnviron(v, 1);
        return;
    }
    if (v->slot >= 0) {
        if (!model[v->slot].crashflag) goto release;
        ObjInstanceList[v->objnum].mat->mat3.pos[0] = gGameCar[v->slot].dr_pos[0];
        ObjInstanceList[v->objnum].mat->mat3.pos[1] = gGameCar[v->slot].dr_pos[1];
        ObjInstanceList[v->objnum].mat->mat3.pos[2] = gGameCar[v->slot].dr_pos[2];
    }
    v->timeStamp -= gFrameSeconds;
    if (v->slot < 0 || (v->slot >= 0 && v->timeStamp <= 0.0f)) {
        v->timeStamp = 0.0333333f;
        if (v->index == 30) goto release;
        ZOID_SetObjectDef(v->objnum, gObjList[476 + v->index]);
        v->index++;
        if (v->slot >= 0) {
            if (v->index == 4) newWorldDamage(gGameCar[v->slot].dr_pos, v->slot);
            if (v->index == 3) StartNewBlast(v->slot);
        }
    }
}

/* 0x8005896c, size 0x6c */
static void StartTire(signed short slot, unsigned int tire)
{
    tVisual *v; /* r8 */
    v = &gGameCar[slot].visuals[tire + 7];
    v->func = AnimateTire;
    v->slot = slot;
    v->data = 0;
    v->index = tire;
    v->timeStamp = gModelTime;
    v->objnum = gCarParts[slot].tires[tire];
}

/* 0x800589d8, size 0x464 */
static void AnimateTire(tVisual *v, signed short op)
{
float pos[3];
    float mat[3][3];
    float pyr[3];
    float spring;
    float spring2;
    float t;
    float lastpos;
    tCarData *car;
    tModelData *m;
    signed short tire;
    signed short who;
    signed short mph;
    int camera;
    _tCar *cartp;
    int rim;
    int invisible;
    signed short slot;
    if (!op) {
        MBOX_HideObject(v->objnum, 1, 15);
        v->func = 0;
        v->objnum = -1;
        return;
    }
    slot = v->slot;
    car = &gGameCar[slot];
    m = &model[slot];
    tire = v->index;
    spring = m->suscomp[tire];
    invisible = gInvisibleCar || (car->appearance & 8);
    mph = car->rwmph >> 2;
    camera = car->camera;
    who = m->drone_type == 2 ? m->slot + 1 : 0;
    cartp = gCarModels[model[slot].body_type];
    if (CheckVisibleAll(v, !(car->appearance & 0x10) && !car->carMine)) {
        CheckVisible(v, car->view >= 2, camera);
        if (model[slot].sviscode[tire] == 1) {
            t = 0.05f * (float)(mph >> 2);
            if (t > 0.6f) t = 0.6f;
            spring += Random(t);
        }
        if (spring < 0.0f) spring = 0.0f;
        else if (spring > 0.5f) spring = 0.5f;
        pos[0] = cartp->tireR[tire][0];
        pos[1] = spring + (cartp->tireR[tire][1] + cartp->tires[tire]->tradius);
        pos[2] = cartp->tireR[tire][2];
        if (camera >= 0 && car->view == 1 && tire == 0) {
            spring = 0.002f * (float)(mph >> 2);
            if (model[slot].sviscode[tire] == 1) spring2 = spring * (Random(2.0f) - 1.0f);
            else if (model[slot].sviscode[tire] == 8) spring2 = 0.0f;
            else spring2 = 0.25f * (spring * (Random(2.0f) - 1.0f));
            gCamera[camera].view2spring = 0.75f * gCamera[camera].view2spring + spring2;
        }
        lastpos = *(float *)&v->data;
        if (m->tires[tire].angvel > 64.0f) lastpos += 3.7f;
        else lastpos += m->tires[tire].angvel * (gModelTime - v->timeStamp);
        *(float *)&v->data = lastpos;
        v->timeStamp = gModelTime;
        pyr[0] = -lastpos;
        pyr[1] = tire < 2 ? -m->steerangle : 0.0f;
        pyr[2] = 0.0f;
        rim = m->tireRims;
        if (m->tires[tire].angvel > 64.0f) rim = gWheelBlur[rim];
        else rim += 24;
        ZOID_SetTexture(v->objnum, gWheelTextures[rim], -1);
        CreateYPRMatrix(mat, pyr);
        if (tire >= 2) {
            t = gRearTireScale[who][m->body_type];
            if (t != 1.0f) ScaleUV(mat, mat, t);
        } else {
            t = gFrontTireScale[who][m->body_type];
            if (t != 1.0f) ScaleUV(mat, mat, t);
        }
        ZOID_UpdateObject(v->objnum, pos, mat);
        if (invisible) ObjInstanceList[v->objnum].flags ^= 0x80000000;
        else ObjInstanceList[v->objnum].flags &= 0x7fffffff;
    }
}

/* 0x80058e3c, size 0x73c */
static void StartShadow(signed short slot)
{
{
    tVisual *v;
    int i;
    _tdstVertext vert[4];
    anon_ob_tc_18 *vv;
    tPackedObject *shadow1[1];
    tPackedObject *shadow2;
    tPackedObject *shadow3;
    v=&gGameCar[slot].visuals[11];
    v->func=AnimateShadow;
    v->slot=slot;
    v->timeStamp=gModelTime;
    shadow1[0]=&gGameCarShadow[slot*3];
    shadow2=shadow1[0]+1;
    shadow3=shadow1[0]+2;
    gShadowColor.c[3]=192;
    v->objnum=ZOID_NewPoly(4,gDummyVtx,gTexList[8],&gShadowColor,0x8200|((gGameState&0x100)?1:15),1)-PolyInstanceList;
    shadow1[0]->numPack=1;
    shadow1[0]->name=0;
    shadow1[0]->tex=&gGameCarShadowTexDef;
    shadow1[0]->pPack=&gGameCarShadowPack[slot*3];
    shadow1[0]->displaylist=0;
    shadow2->numPack=1;
    shadow2->name=0;
    shadow2->tex=&gGameCarShadowTexDef;
    shadow2->pPack=&gGameCarShadowPack[slot*3+1];
    shadow2->displaylist=0;
    shadow3->numPack=1;
    shadow3->name=0;
    shadow3->tex=&gGameCarShadowTexDef;
    shadow3->pPack=&gGameCarShadowPack[slot*3+2];
    shadow3->displaylist=0;
    gGameCarShadowTexDef=&gTexDefPtrs[gTexList[8]>>10].texDefs[gTexList[8]&1023];
    gGameCarShadowTexDef=(tTexture *)gGameCarShadowTexDef->strip;
    if(shadow1[0]->strips) {
        DeallocateMemory(shadow1[0]->strips->vertexs);
        DeallocateMemory(shadow1[0]->strips);
    }
    shadow1[0]->nStrips=1;
    shadow1[0]->strips=AllocateMemoryHeap(0,16);
    shadow1[0]->strips->nVertex=4;
    shadow1[0]->strips->iTex=0;
    shadow1[0]->strips->iHasAlpha=0xfece0ee6;
    shadow1[0]->strips->vertexs=AllocateMemoryHeap(0,144);
    if(shadow2->strips) {
        DeallocateMemory(shadow2->strips->vertexs);
        DeallocateMemory(shadow2->strips);
    }
    shadow2->nStrips=1;
    shadow2->strips=AllocateMemoryHeap(0,16);
    shadow2->strips->nVertex=4;
    shadow2->strips->iTex=0;
    shadow2->strips->iHasAlpha=0xfece0ee6;
    shadow2->strips->vertexs=AllocateMemoryHeap(0,144);
    if(shadow3->strips) {
        DeallocateMemory(shadow3->strips->vertexs);
        DeallocateMemory(shadow3->strips);
    }
    shadow3->nStrips=1;
    shadow3->strips=AllocateMemoryHeap(0,16);
    shadow3->strips->nVertex=4;
    shadow3->strips->iTex=0;
    shadow3->strips->iHasAlpha=0xfece0ee6;
    shadow3->strips->vertexs=AllocateMemoryHeap(0,144);
    vv=PolyInstanceList[v->objnum].verts;
    for(i=0;i<2;i++) {
        vert[i].x=vv[i].ob[0];
        vert[i].y=vv[i].ob[1];
        vert[i].z=vv[i].ob[2];
        vert[i].nx=vert[i].ny=vert[i].nz=0.0f;
        vert[i].u=vv[i].tc[0];
        vert[i].v=vv[i].tc[1];
        vert[i].r=vv[i].cn[0];
        vert[i].g=vv[i].cn[1];
        vert[i].b=vv[i].cn[2];
        vert[i].a=vv[i].cn[3];
    }
    for(i=2;i<4;i++) {
        vert[i].x=vv[5-i].ob[0];
        vert[i].y=vv[5-i].ob[1];
        vert[i].z=vv[5-i].ob[2];
        vert[i].nx=vert[i].ny=vert[i].nz=0.0f;
        vert[i].u=vv[5-i].tc[0];
        vert[i].v=vv[5-i].tc[1];
        vert[i].r=vv[5-i].cn[0];
        vert[i].g=vv[5-i].cn[1];
        vert[i].b=vv[5-i].cn[2];
        vert[i].a=vv[5-i].cn[3];
    }
    for(i=0;i<4;i++) {
        shadow1[0]->strips->vertexs[i].pos[0]=vert[i].x;
        shadow1[0]->strips->vertexs[i].pos[1]=vert[i].y;
        shadow1[0]->strips->vertexs[i].pos[2]=vert[i].z;
        shadow1[0]->strips->vertexs[i].nom[0]=vert[i].nx;
        shadow1[0]->strips->vertexs[i].nom[1]=vert[i].ny;
        shadow1[0]->strips->vertexs[i].nom[2]=vert[i].nz;
        shadow1[0]->strips->vertexs[i].uv[0]=vert[i].u;
        shadow1[0]->strips->vertexs[i].uv[1]=vert[i].v;
        shadow1[0]->strips->vertexs[i].color=(vert[i].r<<24)+(vert[i].g<<16)+(vert[i].b<<8)+vert[i].a;
    }
    for(i=0;i<4;i++) {
        /* block local read before the stores: the target loads the blue byte first in this loop */
        unsigned int cb;
        cb=vert[i].b;
        shadow2->strips->vertexs[i].pos[0]=vert[i].x;
        shadow2->strips->vertexs[i].pos[1]=vert[i].y;
        shadow2->strips->vertexs[i].pos[2]=vert[i].z;
        shadow2->strips->vertexs[i].nom[0]=vert[i].nx;
        shadow2->strips->vertexs[i].nom[1]=vert[i].ny;
        shadow2->strips->vertexs[i].nom[2]=vert[i].nz;
        shadow2->strips->vertexs[i].uv[0]=vert[i].u;
        shadow2->strips->vertexs[i].uv[1]=vert[i].v;
        shadow2->strips->vertexs[i].color=(vert[i].r<<24)+(vert[i].g<<16)+(cb<<8)+vert[i].a;
    }
    for(i=0;i<4;i++) {
        shadow3->strips->vertexs[i].pos[0]=vert[i].x;
        shadow3->strips->vertexs[i].pos[1]=vert[i].y;
        shadow3->strips->vertexs[i].pos[2]=vert[i].z;
        shadow3->strips->vertexs[i].nom[0]=vert[i].nx;
        shadow3->strips->vertexs[i].nom[1]=vert[i].ny;
        shadow3->strips->vertexs[i].nom[2]=vert[i].nz;
        shadow3->strips->vertexs[i].uv[0]=vert[i].u;
        shadow3->strips->vertexs[i].uv[1]=vert[i].v;
        shadow3->strips->vertexs[i].color=(vert[i].r<<24)+(vert[i].g<<16)+(vert[i].b<<8)+vert[i].a;
    }
    gGameCarShadowIdx[slot]=0;
}
}

/* 0x80059578, size 0x99c */
static void AnimateShadow(tVisual *v, signed short op)
{
{
    float pos[3];
    float tvec[3];
    float xyz[4][3];
    float dist[4];
    float d1,d2,d3,d4;
    signed short i,ii;
    signed short body;
    tCarData *car;
    tModelData *m;
    float fscale,bscale,len;
    float scalePair[2];
    anon_ob_tc_18 *vv;
    tPackedObject *shadow1;
    car=&gGameCar[v->slot];
    m=&model[v->slot];
    shadow1=&gGameCarShadow[v->slot*3];
    if(!op) {
        if(v->objnum>=0) ZOID_RemovePoly(&PolyInstanceList[v->objnum]);
        v->func=0;
        v->objnum=-1;
        return;
    }
    d1=m->reckon.airdist[3]-m->reckon.airdist[1];
    d2=m->reckon.airdist[2]-m->reckon.airdist[0];
    d3=m->reckon.airdist[1]-m->reckon.airdist[0];
    d4=m->reckon.airdist[3]-m->reckon.airdist[2];
    if(fabs(d1)>20.0 || fabs(d2)>20.0 || fabs(d3)>20.0 || fabs(d4)>20.0 ||
       car->carMine || m->resurrect.moving_state>-1 || (car->appearance&8) || gInvisibleCar || m->hide_car) {
        ZOID_HidePoly(&PolyInstanceList[v->objnum]);
        return;
    }
    d1=i=0;
    for(;i<4;i++) {
        dist[i]=m->reckon.airdist[i];
        d1+=m->reckon.airdist[i];
        if(dist[i]<0.0f) dist[i]=0.0f;
    }
    d1*=0.25f;
    for(i=0;i<4;i++) {
        signed short tire=i<2?i:5-i;
        d2=dist[tire]>10.0f?2.0f:1.0f+0.1f*dist[tire];
        xyz[i][0]=car->dr_tirepos[tire][0];
        xyz[i][1]=d2+(car->dr_tirepos[tire][1]-dist[tire]);
        xyz[i][2]=car->dr_tirepos[tire][2];
    }
    if(car->dr_uvs[1][1]<0.0f) {
        pos[0]=xyz[0][0];
        pos[1]=xyz[0][1];
        pos[2]=xyz[0][2];
        xyz[0][0]=xyz[1][0];
        xyz[0][1]=xyz[1][1];
        xyz[0][2]=xyz[1][2];
        xyz[1][0]=pos[0];
        xyz[1][1]=pos[1];
        xyz[1][2]=pos[2];
        pos[0]=xyz[2][0];
        pos[1]=xyz[2][1];
        pos[2]=xyz[2][2];
        xyz[2][0]=xyz[3][0];
        xyz[2][1]=xyz[3][1];
        xyz[2][2]=xyz[3][2];
        xyz[3][0]=pos[0];
        xyz[3][1]=pos[1];
        xyz[3][2]=pos[2];
    }
    len=3.0*(1.0-fabs(car->dr_uvs[1][1]));
    if(car->dr_uvs[0][1]<0.0f) {
        pos[0]=car->dr_uvs[1][0]*len+xyz[0][0];
        pos[1]=car->dr_uvs[1][1]*len+xyz[0][1];
        pos[2]=car->dr_uvs[1][2]*len+xyz[0][2];
        xyz[0][0]=pos[0];
        xyz[0][2]=pos[2];
        pos[0]=car->dr_uvs[1][0]*len+xyz[3][0];
        pos[1]=car->dr_uvs[1][1]*len+xyz[3][1];
        pos[2]=car->dr_uvs[1][2]*len+xyz[3][2];
        xyz[3][0]=pos[0];
        xyz[3][2]=pos[2];
    } else if(car->dr_uvs[0][1]>0.0f) {
        pos[0]=car->dr_uvs[1][0]*len+xyz[1][0];
        pos[1]=car->dr_uvs[1][1]*len+xyz[1][1];
        pos[2]=car->dr_uvs[1][2]*len+xyz[1][2];
        xyz[1][0]=pos[0];
        xyz[1][2]=pos[2];
        pos[0]=car->dr_uvs[1][0]*len+xyz[2][0];
        pos[1]=car->dr_uvs[1][1]*len+xyz[2][1];
        pos[2]=car->dr_uvs[1][2]*len+xyz[2][2];
        xyz[2][0]=pos[0];
        xyz[2][2]=pos[2];
    }
    if((car->dr_uvs[1][1]>0.0f && car->dr_uvs[2][1]>0.95f) ||
       (car->dr_uvs[1][1]<0.0f && car->dr_uvs[2][1]<-0.95f)) {
        pos[0]=car->dr_uvs[1][0]*len+xyz[2][0];
        pos[1]=car->dr_uvs[1][1]*len+xyz[2][1];
        pos[2]=car->dr_uvs[1][2]*len+xyz[2][2];
        xyz[2][0]=pos[0];
        xyz[2][2]=pos[2];
        pos[0]=car->dr_uvs[1][0]*len+xyz[3][0];
        pos[1]=car->dr_uvs[1][1]*len+xyz[3][1];
        pos[2]=car->dr_uvs[1][2]*len+xyz[3][2];
        xyz[3][0]=pos[0];
        xyz[3][2]=pos[2];
    } else if((car->dr_uvs[1][1]<0.0f && car->dr_uvs[2][1]>0.95f) ||
       (car->dr_uvs[1][1]>0.0f && car->dr_uvs[2][1]<-0.95f)) {
        pos[0]=car->dr_uvs[1][0]*len+xyz[0][0];
        pos[1]=car->dr_uvs[1][1]*len+xyz[0][1];
        pos[2]=car->dr_uvs[1][2]*len+xyz[0][2];
        xyz[0][0]=pos[0];
        xyz[0][2]=pos[2];
        pos[0]=car->dr_uvs[1][0]*len+xyz[1][0];
        pos[1]=car->dr_uvs[1][1]*len+xyz[1][1];
        pos[2]=car->dr_uvs[1][2]*len+xyz[1][2];
        xyz[1][0]=pos[0];
        xyz[1][2]=pos[2];
    }
    body=(car->appearance&0x10)?13:m->body_type;
    fscale=shadow_scale[body][0];
    bscale=shadow_scale[body][1];
    for(i=0;i<3;i++) {
        d2=xyz[0][i]-xyz[1][i];
        d3=d2*fscale;
        xyz[0][i]+=d3;
        xyz[1][i]-=d3;
        d2=xyz[3][i]-xyz[2][i];
        d3=d2*bscale;
        xyz[3][i]+=d3;
        xyz[2][i]-=d3;
    }
    scalePair[0]=shadow_scale[body][2];
    scalePair[1]=shadow_scale[body][3];
    for(i=0;i<3;i++) {
        d2=xyz[0][i]-xyz[3][i];
        xyz[0][i]+=d2*scalePair[0];
        xyz[3][i]-=d2*scalePair[1];
        d2=xyz[1][i]-xyz[2][i];
        xyz[1][i]+=d2*scalePair[0];
        xyz[2][i]-=d2*scalePair[1];
    }
    for(i=0;i<4;i++) {
        signed short tire;
        if(car->dr_uvs[1][1]<0.0f) {
            switch(i) {
                case 0: ii=1;break;
                case 1: ii=0;break;
                case 2: ii=3;break;
                case 3: ii=2;break;
            }
        } else ii=i;
        tire=ii<2?ii:5-ii;
        if(!(xyz[ii][1]<car->dr_tirepos[tire][1]+1.15f)) {
            pos[0]=xyz[ii][0]-car->dr_tirepos[tire][0];
            pos[1]=xyz[ii][1]-car->dr_tirepos[tire][1];
            pos[2]=xyz[ii][2]-car->dr_tirepos[tire][2];
            BodyVector(pos,tvec,car->dr_uvs);
            if(tvec[1]>4.0f || (tvec[2]<1.25f && tvec[0]<1.25f && tvec[1]>1.25f)) {
                ZOID_HidePoly(&PolyInstanceList[v->objnum]);
                return;
            }
        }
    }
    fscale=8.0f*d1;
    gShadowColor.c[3]=255.0f-(fscale<20.0f?20.0f:fscale>255.0f?255.0f:fscale);
    if(gShadowColor.c[3]>192) gShadowColor.c[3]=192;
    if(car->camera>=0 && (car->view==0 || car->view==1)) ZOID_ClearPolyFlags(&PolyInstanceList[v->objnum],1<<car->camera);
    else ZOID_SetPolyFlags(&PolyInstanceList[v->objnum],15);
    ZOID_UpdatePoly(&PolyInstanceList[v->objnum],4,xyz,0,&gShadowColor,0,0);
    vv=PolyInstanceList[v->objnum].verts;
    shadow1->strips->vertexs[0].pos[0]=vv[0].ob[0];
    shadow1->strips->vertexs[0].pos[1]=vv[0].ob[1];
    shadow1->strips->vertexs[0].pos[2]=vv[0].ob[2];
    shadow1->strips->vertexs[1].pos[0]=vv[1].ob[0];
    shadow1->strips->vertexs[1].pos[1]=vv[1].ob[1];
    shadow1->strips->vertexs[1].pos[2]=vv[1].ob[2];
    shadow1->strips->vertexs[2].pos[0]=vv[3].ob[0];
    shadow1->strips->vertexs[2].pos[1]=vv[3].ob[1];
    shadow1->strips->vertexs[2].pos[2]=vv[3].ob[2];
    shadow1->strips->vertexs[3].pos[0]=vv[2].ob[0];
    shadow1->strips->vertexs[3].pos[1]=vv[2].ob[1];
    shadow1->strips->vertexs[3].pos[2]=vv[2].ob[2];
    if(numShadowObjToDraw>6) numShadowObjToDraw=0;
    ShadowObjList[numShadowObjToDraw++]=shadow1;
    ZOID_HidePoly(&PolyInstanceList[v->objnum]);
}
}

/* 0x80059f14, size 0x94 */
void SendCarShadows(void)
{
    int i; /* r30 */
    tRGBA tt; /* stack */
    tRGBA tt1; /* stack */
    tt = MyGetAmbientColor();
    tt1.rgba = 0xffffffff;
    MySetAmbientColor(tt1);
    currentz = 5000.0f;
    for (i = 0; i < numShadowObjToDraw; i++)
        PS2DrawObject(ShadowObjList[i], 0, -1);
    MySetAmbientColor(tt);
}

/* 0x80059fa8, size 0xc */
void ClearCarShadows(void)
{
    numShadowObjToDraw = 0;
}

static tRGBA arrowColors[5] = {
    0, 0, 224, 255, 224, 0, 0, 255, 224, 224, 0, 255, 0, 224, 0, 255, 224, 224, 224, 255
};
unsigned char arrowinvsColor[7] = { 32, 48, 80, 128, 160, 192, 239 };

/* 0x80059fb4, size 0x174 */
void UpdateArrowVis(void)
{
    int i; /* r0 */
    int j; /* r22 */
    float p1[3]; /* stack */
    float p2[3]; /* stack */
    float uvs[3][3]; /* stack */
    for (i = 0; i < 16; i++) ((signed char *)gFadePlayerArrow)[i] = 0;
    if (gShowPlayerArrows && gRace == 6) {
        for (i = 0; i < gNumPlayers - 1; i++) {
            p1[0] = gGameCar[i].dr_pos[0];
            p1[1] = gGameCar[i].dr_pos[1];
            p1[2] = gGameCar[i].dr_pos[2];
            p1[1] += 5.0f;
            for (j = 1; j < gNumPlayers; j++) {
                p2[0] = gGameCar[j].dr_pos[0];
                p2[1] = gGameCar[j].dr_pos[1];
                p2[2] = gGameCar[j].dr_pos[2];
                p2[1] += 5.0f;
                if (ShotToWorldCollision(p1, p2, uvs, 0.5f, 0, 65535)) {
                    gFadePlayerArrow[i][j] = 1;
                    gFadePlayerArrow[j][i] = 1;
                }
            }
        }
    }
}

/* 0x8005a128, size 0x240 */
static void StartArrow(signed short slot)
{
    tVisual *v = &gGameCar[slot].visuals[12];
    int i;
    tRGBA clr = { 255, 255, 255, 255 };
    tPoly *poly;
    if (gRace == 6) {
        clr.c[0] = arrowColors[gCarTeam[slot]].c[0];
        clr.c[1] = arrowColors[gCarTeam[slot]].c[1];
        clr.c[2] = arrowColors[gCarTeam[slot]].c[2];
        clr.c[3] = arrowColors[gCarTeam[slot]].c[3];
    } else {
        clr.c[0] = arrowColors[slot].c[0];
        clr.c[1] = arrowColors[slot].c[1];
        clr.c[2] = arrowColors[slot].c[2];
        clr.c[3] = arrowColors[slot].c[3];
    }
    for (i = 0; i < gNumWindows; i++) {
        if (i != slot) {
            v->func = (void (*)(tVisual *, signed short))AnimateArrow;
            v->slot = slot;
            v->index = i;
            if (gRace == 6) poly = ZOID_NewPoly(4, gDummyVtx, gTexList[9], &clr, (1 << i) | 0x3200, 1);
            else poly = ZOID_NewPoly(4, gDummyVtx, gTexList[9], &clr, (1 << i) | 0x1200, 1);
            ZOID_SetPolyZ(poly, 10000);
            v->objnum = poly - PolyInstanceList;
            v++;
        }
    }
    while (v < &gGameCar[slot].visuals[14]) {
        v->func = 0;
        v->objnum = -1;
        v++;
    }
}

/* 0x8005a368, size 0x5ec */
static void AnimateArrow(tVisual *v)
{
{
    tCarData *car;
    tModelData *m;
    float xyz[4][3];
    float scale;
    int oslot;
    int i;
    float vec[3];
    tWindowInfo *window;
    float x,y,z;
    float widthTerm;
    float border=19.0f;
    float vscale,voffset;
    int j;
    car=&gGameCar[v->slot];
    m=&model[v->slot];
    oslot=v->index;
    {
    tRGBA clr = {{255,255,255,255}};
    if(!gShowPlayerArrows || !(gGameState&0x400000) ||
        ((car->crash_state || m->resurrect.moving_state>=0) && gRace==6)) {
        ZOID_HidePoly(&PolyInstanceList[v->objnum]);
        return;
    }
    if(gRace==6 && (car->battle.defense.active&1)) {
        if(car->battle.fadeState==1) clr.c[3]=car->battle.fadeAlpha;
        else if(car->battle.fadeState==2) clr.c[3]=car->battle.fadeAlpha;
        else clr.c[3]=arrowinvsColor[car->battle.shoot];
        if(clr.c[3]<32) clr.c[3]=32;
    }
    if(m->drone_type==2) {
        if(gRace==6) {
            clr.c[0]=arrowColors[gCarTeam[m->slot]].c[0];
            clr.c[1]=arrowColors[gCarTeam[m->slot]].c[1];
            clr.c[2]=arrowColors[gCarTeam[m->slot]].c[2];
        } else {
            clr.c[0]=arrowColors[m->slot].c[0];
            clr.c[1]=arrowColors[m->slot].c[1];
            clr.c[2]=arrowColors[m->slot].c[2];
        }
        if(gFadePlayerArrow[m->slot][oslot] && clr.c[3]>112) clr.c[3]=112;
        if(gRace==2 && v->slot>0) clr.c[3]=128;
    } else {
        clr.c[0]=arrowColors[4].c[0];
        clr.c[1]=arrowColors[4].c[1];
        clr.c[2]=arrowColors[4].c[2];
    }
    vec[0]=car->dr_pos[0]-gCamera[oslot].mat.mat3.pos[0];
    vec[1]=car->dr_pos[1]-gCamera[oslot].mat.mat3.pos[1];
    vec[2]=car->dr_pos[2]-gCamera[oslot].mat.mat3.pos[2];
    if(gRace==6) {
        window=&ZoidWinInfo[oslot];
        U64WorldVector(vec,xyz[0],gCamera[oslot].mat.mat3.uvs);
        scale=fatan2(-xyz[0][0],xyz[0][2]);
        z=31.41592788696289f/window->whviewang;
        if(gSplitScreen>1) {
            voffset=0.75f*window->wvviewang;
            vscale=16.5f;
        } else {
            vscale=14.5f; /* vscale before voffset here (the reverse of the branch above) gives the original instruction order */
            voffset=1.1f*window->wvviewang;
        }
        if(scale<0.48f*-window->whviewang && scale>-1.5707963705062866f) {
            widthTerm=0.6366197466850281f*window->whviewang;
            x=widthTerm*border;
            scale=-scale;
            j=3;
        } else if(scale<-1.5707963705062866f) {
            scale=3.1415927410125732f+scale;
            goto arrow_middle;
        } else if(scale>1.5707963705062866f) {
            scale=scale-3.1415927410125732f;
arrow_middle:
            j=0;
            widthTerm=0.6366197466850281f*window->whviewang;
            x=0.6366197466850281f*(scale*(widthTerm*border));
            y=(0.6366197466850281f*window->wvviewang)*vscale;
            goto arrow_poly;
        } else if(scale>0.48f*window->whviewang && scale<1.5707963705062866f) {
            j=1;
            widthTerm=0.6366197466850281f*window->whviewang;
            x=widthTerm*-border;
        } else goto arrow_world;
        y=(1.0f/(1.5707963705062866f-voffset))*(((0.6366197466850281f*window->wvviewang)*vscale)*(scale-voffset));
arrow_poly:
        for(i=0;i<4;i++,j=(j+1)&3) {
            float (*p)[3]=&xyz[j];
            (*p)[0]=x+gDummyVtx[i][0];
            (*p)[1]=gDummyVtx[i][1]-y;
            (*p)[2]=z;
        }
        ZOID_UpdatePoly(&PolyInstanceList[v->objnum],4,xyz,0,&clr,(1<<oslot)|0x3610,0);
        return;
    }
arrow_world:
    scale=magnitude(vec)/25.0f;
    for(i=0;i<4;i++) {
        U64WorldVector(gDummyVtx[i],xyz[i],gCamera[oslot].n64.mat.mat3.uvs);
        xyz[i][0]=car->dr_pos[0]+scale*xyz[i][0];
        xyz[i][1]=5.0f+(scale+(car->dr_pos[1]+scale*xyz[i][1]));
        xyz[i][2]=car->dr_pos[2]+scale*xyz[i][2];
    }
    if(gRace==6) ZOID_UpdatePoly(&PolyInstanceList[v->objnum],4,xyz,0,&clr,(1<<oslot)|0x3700,0);
    else ZOID_UpdatePoly(&PolyInstanceList[v->objnum],4,xyz,0,&clr,(1<<oslot)|0x1600,0);
}
}
}

/* 0x8005a954, size 0x64 */
static void StartBrakeLights(signed short slot)
{
    tVisual *v; /* r8 */
    v = &gGameCar[slot].visuals[0];
    v->func = AnimateBrakeLights;
    v->slot = slot;
    MBOX_HideObject(gCarParts[slot].brakes, 1, 15);
}

/* 0x8005a9b8, size 0xb4 */
static void AnimateBrakeLights(tVisual *v, signed short op)
{
    tCarData *car; /* r7 */
    int on;
    car = &gGameCar[v->slot];
    on = ((car->appearance & 0x1000) && !(car->appearance & 0x10));
    if (!op || !on || car->view == 1 || car->view == 0)
        MBOX_HideObject(gCarParts[v->slot].brakes, 1, 15);
    else MBOX_ShowObject(gCarParts[v->slot].brakes, 0, 15);
}

/* 0x8005aa6c, size 0xe4 */
void StartSparks(signed short slot)
{
    tVisual *v; /* r30 */
    if (!(gGameState & 8)) {
        v = &gGameCar[slot].visuals[2];
        v->func = AnimateLFSpark;
        v->index = -1;
        v->slot = slot;
        v->objnum = gCarParts[slot].lfSpark;
        v = &gGameCar[slot].visuals[3];
        v->func = AnimateLRSpark;
        v->index = -1;
        v->slot = slot;
        v->objnum = gCarParts[slot].lrSpark;
        v = &gGameCar[slot].visuals[4];
        v->func = AnimateRFSpark;
        v->index = -1;
        v->slot = slot;
        v->objnum = gCarParts[slot].rfSpark;
        v = &gGameCar[slot].visuals[5];
        v->func = AnimateRRSpark;
        v->index = -1;
        v->slot = slot;
        v->objnum = gCarParts[slot].rrSpark;
        v = &gGameCar[slot].visuals[6];
        v->func = AnimateBSpark;
        v->index = -1;
        v->slot = slot;
        v->objnum = gCarParts[slot].bSpark;
    }
}

/* 0x8005ab50, size 0xec */
static void AnimateLFSpark(tVisual *v, signed short op)
{
    int rear; /* r7 */
    tCarData *car; /* r9 */
    car = &gGameCar[v->slot];
    rear = (gPlayers[v->slot].levels & gPlayers[v->slot].controls[9]) != 0;
    HandleASpark(v, (signed short)model[v->slot].magvel > 20 && (car->appearance & 0x100) && (!rear || (rear && car->view >= 2)), car->tireR[1][0] - 0.6f, car->tireR[1][1] + 2.0f * model[v->slot].tires[1].tradius, car->tireR[1][2], 506, op);
}

/* 0x8005ac3c, size 0xec */
static void AnimateLRSpark(tVisual *v, signed short op)
{
    tCarData *car; /* r9 */
    int rear; /* r7 */
    car = &gGameCar[v->slot];
    rear = (gPlayers[v->slot].levels & gPlayers[v->slot].controls[9]) != 0;
    HandleASpark(v, (signed short)model[v->slot].magvel > 20 && (car->appearance & 0x2000) && (!rear || (rear && car->view >= 2)), car->tireR[3][0] - 0.6f, car->tireR[3][1] + 2.0f * model[v->slot].tires[3].tradius, car->tireR[3][2], 506, op);
}

/* 0x8005ad28, size 0xec */
static void AnimateRFSpark(tVisual *v, signed short op)
{
    tCarData *car; /* r9 */
    int rear; /* r7 */
    car = &gGameCar[v->slot];
    rear = (gPlayers[v->slot].levels & gPlayers[v->slot].controls[9]) != 0;
    HandleASpark(v, (signed short)model[v->slot].magvel > 20 && (car->appearance & 0x80) && (!rear || (rear && car->view >= 2)), car->tireR[0][0] + 0.6f, car->tireR[0][1] + 2.0f * model[v->slot].tires[0].tradius, car->tireR[0][2], 506, op);
}

/* 0x8005ae14, size 0xec */
static void AnimateRRSpark(tVisual *v, signed short op)
{
    tCarData *car; /* r9 */
    int rear; /* r7 */
    car = &gGameCar[v->slot];
    rear = (gPlayers[v->slot].levels & gPlayers[v->slot].controls[9]) != 0;
    HandleASpark(v, (signed short)model[v->slot].magvel > 20 && (car->appearance & 0x4000) && (!rear || (rear && car->view >= 2)), car->tireR[2][0] + 0.6f, car->tireR[2][1] + 2.0f * model[v->slot].tires[2].tradius, car->tireR[2][2], 506, op);
}

/* 0x8005af00, size 0x12c */
static void AnimateBSpark(tVisual *v, signed short op)
{
    int top; /* r12 */
    tCarData *car; /* r10 */
    int rear; /* r8 */
    int on[1];
    car = &gGameCar[v->slot];
    top = model[v->slot].topscrape;
    rear = (gPlayers[v->slot].levels & gPlayers[v->slot].controls[9]) != 0;
    on[0] = 0;
    {
        int contact = (signed short)model[v->slot].magvel > 12 && (top || (car->appearance & 0x800));
        if (contact) {
            int visible = !rear || (rear && car->view >= 2);
            if (visible) on[0] = 1;
        }
    }
    HandleASpark(v, on[0], 0.0f, car->tireR[3][1] + (top ? 2.0f : model[v->slot].tires[3].tradius), car->tireR[3][2] - 2.0f, 506, op);
}

/* 0x8005b02c, size 0x1d8 */
static void HandleASpark(tVisual *v, int on, float x, float y, float z, int spark, signed short op)
{
    tCarData *car; /* r29 */
    tUVMatrix *mat; /* r4 */
    signed short mph; /* r29 */
    car = &gGameCar[v->slot];
    if (op && !(gGameState & 8) && on) {
        if (v->index < 0) v->index = Random(5.0f);
        v->index++;
        if (v->index > 7) v->index = 0;
        PositionObject(v->objnum, x, y, z, car->v[0], car->v[1], car->v[2]);
        mph = model[v->slot].magvel;
        if (mph < 100) {
            mat = ZOID_FindMatrix(v->objnum);
            ScaleUV(mat->mat3.uvs, mat->mat3.uvs, (float)mph / 100.0f);
        }
        if (CheckVisibleAll(v, v->index >= 0)) {
            ZOID_ChangeTexture(gObjList[spark], 0, gTexList[v->index]);
            return;
        }
    }
    MBOX_HideObject(v->objnum, 1, 15);
    v->index = -1;
}

/* 0x8005b204, size 0x7c */
static void PositionObject(int objnum, float x, float y, float z, float u, float v, float w)
{
    tUVMatrix *mat; /* r31 */
    mat = ZOID_FindMatrix(objnum);
    mat->mat3.pos[0] = u;
    mat->mat3.pos[1] = v;
    mat->mat3.pos[2] = w;
    LookInDir(mat->mat3.pos, mat);
    mat->mat3.pos[0] = x;
    mat->mat3.pos[1] = y;
    mat->mat3.pos[2] = z;
}

/* 0x8005b280, size 0x54 */
static int CheckVisibleAll(tVisual *v, int on)
{
    if (on) MBOX_ShowObject(v->objnum, 0, 15);
    else MBOX_HideObject(v->objnum, 1, 15);
    return on;
}

/* 0x8005b2d4, size 0x70 */
static int CheckVisible(tVisual *v, int on, int camera)
{
    if (camera < 0) return CheckVisibleAll(v, on);
    if (on) MBOX_ShowObject(v->objnum, 0, 1 << camera);
    else MBOX_HideObject(v->objnum, 1, 1 << camera);
    return on;
}






