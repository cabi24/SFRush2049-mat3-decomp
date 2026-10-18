/* targets.c - track target (interactive object) manager: targetInfo table, SetupTarget, per-type
   start/animate callbacks, collision dispatch. Status: Matching, 33/33 (SetTargetDefs via the _NB inline
   helper below TInfo), see HANDOFF.md.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\targets.c; assembly: build/2049LE/asm/GameSource/src/targets.s */
#include "rush/rush_protos.h"
#define MSL_NO_SQRTF
#include "msl_math_inline.h"

/* camera.c/tires.c variant: automatic constants, so the 0.5/3.0 doubles are plain literals */
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

/* Not in DWARF (inlined): the original reads the type flags through a pointer temporary,
   which gives the base-in-r0 / index-product-in-r3 allocation of the targetInfo address. */
static inline tTargetInfo *TInfo(tTarget *t) { return &targetInfo[t->infoIdx]; }
/* SetTargetDefs: the visibility/collision SetupTarget arguments come back through an inline; the
   named vis/col copies are then propagated away and the inline temporaries are coloured first. */
static inline int _NB(unsigned int f, int b) { return ((f >> b) & 1) ^ 1; }

static void _InitBBoxModelStruct(tTarget *t);
static void SetTargetDef(tTarget *t);
static int _LocalPointInBody(tMiniModelData *m, float *pt);
static int _TargetCollBBox(void * data1, void * data2);
static int _TargetCollTrigger(void * data1, void * data2);
static int _TargetCollCylinder(void * data1, void * data2, void * data3, void * data4);
static int _TargetCollMineCylinder(void * data1, void * data2, void * data3, void * data4);
static int _TargetCollRadius(void * data1, void * data2, void * data3, void * data4);
static int _TargetColl3Piece(void);
static void _StartGuardrail(tTarget *t);
static void _AnimateGuardrail(tVisual *v, signed short op);
static void _StartMine(tTarget *t);
static void _AnimateMine(tVisual *v, signed short op);
static void _StartWeaponIcon(tTarget *t);
static void _AnimateWeaponIcon(tVisual *v, signed short op);
static void _AnimateFlag(tVisual *v, signed short op);
static void _StartCactus(tTarget *t);
static void _StartFence(tTarget *t);
static void _StartWindow(tTarget *t);
static void _StartCone(tTarget *t);
static void _StartCoin(tTarget *t);
static void _AnimateCoin(tVisual *v, signed short op);
static void _AnimateVelRotGeneric(tVisual *v, signed short op);
static void _AnimateRotGeneric(tVisual *v, signed short op);
static void _AnimateObjSwapGeneric(tVisual *v, signed short op);
static void _StartKnockdown(tTarget *t);
static void _AnimateKnockdown(tVisual *v, signed short op);

static int (*gTargetCollFuncPtrs[6])(void *, void *, void *, void *) = {
    (int (*)(void *, void *, void *, void *))_TargetCollRadius,
    (int (*)(void *, void *, void *, void *))_TargetCollCylinder,
    (int (*)(void *, void *, void *, void *))_TargetCollMineCylinder,
    (int (*)(void *, void *, void *, void *))_TargetColl3Piece,
    (int (*)(void *, void *, void *, void *))_TargetCollBBox,
    (int (*)(void *, void *, void *, void *))_TargetCollTrigger
};
tTargetInfo targetInfo[118] = {
    "GOLDCOIN", "GOLDCOINL4_LOD1", (void *)&_StartCoin, (void *)&_AnimateCoin, 0, 0x22a, -1, 6, 0,
    9.0f, 59, -1, -1, 0, 400.0f, -1.0f, "SILVERCOIN", "SILVERCOINL4_LO", (void *)&_StartCoin,
    (void *)&_AnimateCoin, 0, 0x22a, -1, 6, 0, 9.0f, 59, -1, -1, 0, 400.0f, -1.0f, "BARGE",
    "BARGEL2_LOD1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0,
    -1.0f, -1, 140, -1, 0, 400.0f, -1.0f, "BKWBARIER", "BKWBARIERL3_LOD",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, 144, 77,
    143, 0, 400.0f, -1.0f, "BLOCK1FOR", "BLOCK1FORL1_LOD", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, 144, 77, 143, 0, 400.0f, -1.0f,
    "BLOCK1BAC", "BLOCK1BACL1_LOD", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6,
    0x4228, -1, 4, 0, -1.0f, 144, 77, 143, 0, 400.0f, -1.0f, "BLOCK2FOR", "BLOCK2FORL1_LOD",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, 144, 77,
    143, 0, 400.0f, -1.0f, "BLOCK2BAC", "BLOCK2BACL1_LOD", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, 144, 77, 143, 0, 400.0f, -1.0f,
    "BLOCKNV", "BLOCKL37_LOD1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 4, 0xc228,
    -1, 4, 0, 10.0f, 65, 77, 143, 0, 400.0f, -1.0f, "BLOCK", "BLOCKL37_LOD1",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, 65, 77,
    143, 0, 400.0f, -1.0f, "BOULDER", "BOULDERG2", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 4, 0xc228, -1, 4, 0, 9.0f, -1, -1, -1, 0, 400.0f, -1.0f, "BULB",
    "BULBL2_LOD1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0,
    -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "BUMPHIT", "BUMPHITG1", (void *)&_StartKnockdown,
    (void *)&_AnimateKnockdown, 0, 0x200, -1, 0, 1, 2.5f, 146, -1, -1, 0, 400.0f, -1.0f, "CACTUS",
    "CACTUSG2", (void *)&_StartCactus, (void *)&_AnimateObjSwapGeneric, 0, 0x1220, 577, 7, 0,
    -1.0f, 89, -1, -1, 0, 400.0f, -1.0f, "COLLISION", 0, 0, 0, 6, 4, 0, 7, 0, 0.0f, -1, -1, -1, 0,
    400.0f, -1.0f, "CONE1", "CONE1G1", (void *)&_StartCone, (void *)&_AnimateVelRotGeneric, 0,
    0x2208, 550, 2, 0, 3.5f, 124, -1, -1, 0, 400.0f, -1.0f, "CURVEHIT", "CURVEHITG1",
    (void *)&_StartKnockdown, (void *)&_AnimateKnockdown, 0, 0x200, -1, 0, 1, 2.5f, 146, -1, -1, 0,
    400.0f, -1.0f, "ELEVATOR1", "ELEVATOR1L1_LOD", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 4, 0xc208, -1, 4, 0, 2.0f, -1, -1, -1, 0, 400.0f, -1.0f, "FLAG2",
    "FLAG2G13", 0, (void *)&_AnimateFlag, 6, 0x606, 596, 7, 0, 0.0f, -1, -1, -1, 0, 400.0f, -1.0f,
    "F1FLAG", "F1FLAGL1_LOD1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4208,
    680, 4, 0, -1.0f, -1, 83, -1, 0, 400.0f, -1.0f, "F2FLAG", "F2FLAGL37_LOD1",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4a28, -1, 4, 0, -1.0f, -1, -1, -1,
    0, 400.0f, -1.0f, "FENCE", "FENCEG1", (void *)&_StartFence, (void *)&_AnimateObjSwapGeneric, 0,
    0x2200, 584, 7, 0, -1.0f, 135, -1, -1, 0, 400.0f, -1.0f, "GASPUMP", "GASPUMPG1",
    (void *)&_StartCone, (void *)&_AnimateVelRotGeneric, 0, 0x2208, -1, 2, 0, -1.0f, 124, -1, -1,
    0, 400.0f, -1.0f, "GETOFF", "GETOFFG1", (void *)&_StartKnockdown, (void *)&_AnimateKnockdown,
    0, 0x200, -1, 0, 2, 2.5f, 135, -1, -1, 0, 400.0f, -1.0f, "GONDOLA1", "GONDOLA1L244_LO",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 4, 0xc208, -1, 4, 0, 0.0f, -1, 145, -1,
    0, 400.0f, -1.0f, "GONDOLA2", "GONDOLA2TAXI1_L", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 4, 0xc208, -1, 4, 0, 1.0f, -1, 145, -1, 0, 400.0f, -1.0f,
    "GONDOLA3", "GONDOLA3GONDOLA", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6,
    0x4208, -1, 4, 0, -1.0f, -1, 145, -1, 0, 400.0f, -1.0f, "GONDOLA4", "GONDOLA4TAXI1_L",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0, -1.0f, -1, 145,
    -1, 0, 400.0f, -1.0f, "GONDOLAN1", "GONDOLAN1L244_L", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0, -1.0f, -1, 145, -1, 0, 400.0f, -1.0f,
    "GLASSWALL", "GLASSWALLL3_LOD", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6,
    0x4228, -1, 4, 0, -1.0f, 139, -1, -1, 0, 400.0f, -1.0f, "GREGDOOR1", "GREGDOOR1L37_LO",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, 144, 77,
    143, 0, 400.0f, -1.0f, "GREGDOOR2B", "GREGDOOR2BL37_L", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, 144, 77, 143, 0, 400.0f, -1.0f,
    "GREGDOOR2", "GREGDOOR2L37_LO", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6,
    0x4228, -1, 4, 0, -1.0f, 144, 77, 143, 0, 400.0f, -1.0f, "GREGDOOR3", "GREGDOOR3G1",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0, -1.0f, 144, 77,
    143, 0, 400.0f, -1.0f, "GREGDOOR4", "GREGDOOR4L37_LO", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, -1, 77, -1, 0, 400.0f, -1.0f,
    "GREGDOOR5", "GREGDOOR5L37_LO", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6,
    0x4228, -1, 4, 0, -1.0f, 144, 77, 143, 0, 400.0f, -1.0f, "GREGDOOR6", "GREGDOOR6L37_LO",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, 144, 77,
    143, 0, 400.0f, -1.0f, "GREGRAMP", "GREGRAMPL37_LOD", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, 144, 77, 143, 0, 400.0f, -1.0f,
    "GUARDRAIL", 0, 0, 0, 6, 4, -1, 7, 0, 0.0f, 59, -1, -1, 0, 400.0f, -1.0f, "JUMP", "JUMPG1",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, -1, -1, -1,
    0, 400.0f, -1.0f, "METER", "METERG2", (void *)&_StartKnockdown, (void *)&_AnimateKnockdown, 0,
    0x200, -1, 0, 1, 2.5f, 146, -1, -1, 0, 400.0f, -1.0f, "MINITRAINN", "MINITRAINNL1_LO",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 4, 0xc208, -1, 4, 0, 5.0f, -1, 53, -1,
    0, 400.0f, -1.0f, "MINITRAIN", "MINITRAINL1_LOD", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 4, 0xc208, -1, 4, 0, 5.0f, -1, 53, -1, 0, 400.0f, -1.0f,
    "MPH45HIT", "MPH45HITG1", (void *)&_StartKnockdown, (void *)&_AnimateKnockdown, 0, 0x200, -1,
    0, 1, 2.5f, 146, -1, -1, 0, 400.0f, -1.0f, "MPH75HIT", "MPH75HITG1", (void *)&_StartKnockdown,
    (void *)&_AnimateKnockdown, 0, 0x200, -1, 0, 1, 2.5f, 146, -1, -1, 0, 400.0f, -1.0f, "NOPARK",
    "NOPARKG1", (void *)&_StartKnockdown, (void *)&_AnimateKnockdown, 0, 0x200, -1, 0, 1, 2.5f,
    146, -1, -1, 0, 400.0f, -1.0f, "PILLARS", "PILLARSBLOCKS_L", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f,
    "PISTON", "PISTONG1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 4, 0xc228, -1, 4,
    0, 7.0f, -1, -1, -1, 0, 400.0f, -1.0f, "PLANENIGHT", "PLANENIGHTPLANE",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0, -1.0f, -1, -1, -1,
    0, 400.0f, -1.0f, "PLANE", "PLANEL255_LOD1", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 4, 0xc208, -1, 4, 0, 6.0f, -1, 153, -1, 2, 2000.0f, -1.0f,
    "PLATFORM", "PLATFORMG1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228,
    -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "POST", "POSTL1_LOD1",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0, -1.0f, -1, -1, -1,
    0, 400.0f, -1.0f, "RADAR", "RADARL3_LOD1", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f,
    "RAMP02", "RAMP02L1_LOD1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228,
    -1, 4, 0, -1.0f, 144, 77, 143, 0, 400.0f, -1.0f, "RAMP", "RAMPL1_LOD1",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, 144, 77,
    143, 0, 400.0f, -1.0f, "RATCONE", "RATCONEG1", (void *)&_StartCone,
    (void *)&_AnimateVelRotGeneric, 0, 0x288, 653, 2, 0, -1.0f, 62, -1, -1, 0, 400.0f, -1.0f,
    "RAT", "RATG1", (void *)&_StartCone, (void *)&_AnimateVelRotGeneric, 0, 0x288, 551, 2, 0,
    -1.0f, 62, -1, -1, 0, 400.0f, -1.0f, "ROAD", "ROADL1_LOD1", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f,
    "ROTOR2", "ROTOR2WINDROTOR", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4208,
    -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "ROTOR", "ROTORL3_LOD1",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, -1, -1, -1,
    0, 400.0f, -1.0f, "SHARKSIT", "SHARKSITL37_LOD", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "SHARK",
    "SHARKL1_LOD1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4208, 676, 4, 0,
    -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "SHATPANE", "SHATPANEG2", (void *)&_StartWindow,
    (void *)&_AnimateObjSwapGeneric, 0, 0x2a00, 563, 7, 0, -1.0f, 139, -1, -1, 0, 400.0f, -1.0f,
    "SLOWHIT", "SLOWHITG2", (void *)&_StartKnockdown, (void *)&_AnimateKnockdown, 0, 0x200, -1, 0,
    1, 2.5f, 146, -1, -1, 0, 400.0f, -1.0f, "SPIKEDBALL", "SPIKEDBALLG1",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, -1, -1, -1,
    0, 400.0f, -1.0f, "SPOT1", "SPOT1L1_LOD1", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f,
    "SPOTLIGHT1", "SPOTLIGHT1L1_LO", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6,
    0x4208, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "SPOTLIGHT2", "SPOTLIGHT2L1_LO",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0, -1.0f, -1, -1, -1,
    0, 400.0f, -1.0f, "STOPHIT", "STOPHITG1", (void *)&_StartKnockdown, (void *)&_AnimateKnockdown,
    0, 0x200, -1, 0, 1, 2.5f, 146, -1, -1, 0, 400.0f, -1.0f, "TRAPDOORT1", "TRAPDOORT1L3_LO",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, -1, -1, -1,
    0, 400.0f, -1.0f, "TRAPDOOR2", "TRAPDOOR2L3_LOD", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "T1",
    "T1L3_LOD1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0,
    -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "T2", "T2L3_LOD1", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f,
    "T3DOOR1", "T3DOOR1G1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4208, -1,
    4, 0, -1.0f, 144, 77, 143, 0, 400.0f, -1.0f, "T3POD", "T3PODL37_LOD1",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 5, 0x4228, -1, 4, 0, -1.0f, -1, -1, -1,
    0, 400.0f, -1.0f, "T3PYRAMID", "T3PYRAMIDL37_LO", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4268, -1, 4, 0, -1.0f, 144, 77, 143, 0, 400.0f, -1.0f,
    "T3TEETH1", "T3TEETH1L3_LOD1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6,
    0x4228, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "T3TEETH2", "T3TEETH2L3_LOD1",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, -1, -1, -1,
    0, 400.0f, -1.0f, "T3TEETER", "T3TEETERL37_LOD", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 5, 0x4228, -1, 4, 0, -1.0f, 144, 77, 143, 0, 400.0f, -1.0f, "T3",
    "T3L3_LOD1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0,
    -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "TDOOR1", "TDOOR1L1_LOD1", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f,
    "TDOOR2", "TDOOR2L1_LOD1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4208,
    -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "TDOOR3", "TDOOR3L1_LOD1",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0, -1.0f, -1, -1, -1,
    0, 400.0f, -1.0f, "TDOOR4", "TDOOR4L1_LOD1", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f,
    "TDOOR5", "TDOOR5L1_LOD1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4208,
    -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "TDOOR6", "TDOOR6G1", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f,
    "TDOOR7", "TDOOR7G1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4,
    0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "TDOOR8", "TDOOR8G1", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "TEETH",
    "TEETHG1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 4, 0xc228, -1, 4, 0, 8.0f,
    -1, -1, -1, 0, 400.0f, -1.0f, "TEETHC", "TEETHG1", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f,
    "THINKHIT", "THINKHITG1", (void *)&_StartKnockdown, (void *)&_AnimateKnockdown, 0, 0x200, -1,
    0, 1, 2.5f, 146, -1, -1, 0, 400.0f, -1.0f, "TOILET", "TOILETL3_LOD1",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, 139, -1,
    -1, 0, 400.0f, -1.0f, "TRAINNIGHT", "TRAINNIGHTL11_L", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 4, 0xc208, -1, 4, 0, 3.0f, -1, 53, -1, 0, 800.0f, -1.0f,
    "TRAINORG", "TRAINORGL1_LOD1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 4,
    0xc208, -1, 4, 0, 3.0f, -1, 53, -1, 0, 800.0f, -1.0f, "TRIGGER", "TRIGGERL37_LOD1",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 5, 0x4228, -1, 4, 0, -1.0f, 149, -1,
    -1, 0, 400.0f, -1.0f, "TROLLEY2", "TROLLEY2G1", 0, (void *)&_AnimateRotGeneric, 6, 0x206, -1,
    0, 3, -1.0f, -1, 122, -1, 0, 400.0f, -1.0f, "TROLLEYNT", "TROLLEYNTL1_LOD",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 4, 0xc208, -1, 4, 0, 4.0f, -1, 122, -1,
    0, 400.0f, -1.0f, "TROLLEY", "TROLLEYL1_LOD1", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 4, 0xc208, -1, 4, 0, 4.0f, -1, 122, -1, 0, 400.0f, -1.0f, "WALL2",
    "WALL2G1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f,
    -1, -1, -1, 0, 400.0f, -1.0f, "WALL3", "WALL3G1", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "WALL",
    "WALLWALL_LOD1", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0,
    -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "WEPICON_CANN", "WEPICON_CANNL1_",
    (void *)&_StartWeaponIcon, (void *)&_AnimateWeaponIcon, 1, 0xa22, 664, 0, 0, 6.0f, 32, -1, -1,
    0, 400.0f, -1.0f, "WEPICON_GATT", "WEPICON_GATTL1_", (void *)&_StartWeaponIcon,
    (void *)&_AnimateWeaponIcon, 1, 0xa22, 665, 0, 0, 6.0f, 32, -1, -1, 0, 400.0f, -1.0f,
    "WEPICON_GREN", "WEPICON_GRENL1_", (void *)&_StartWeaponIcon, (void *)&_AnimateWeaponIcon, 1,
    0xa22, 666, 0, 0, 6.0f, 32, -1, -1, 0, 400.0f, -1.0f, "WEPICON_MINE", "WEPICON_MINEL1_",
    (void *)&_StartWeaponIcon, (void *)&_AnimateWeaponIcon, 1, 0xa22, 667, 0, 0, 6.0f, 32, -1, -1,
    0, 400.0f, -1.0f, "WEPICON_MISS", "WEPICON_MISSL1_", (void *)&_StartWeaponIcon,
    (void *)&_AnimateWeaponIcon, 1, 0xa22, 668, 0, 0, 6.0f, 32, -1, -1, 0, 400.0f, -1.0f,
    "WEPICON_RAM", "WEPICON_RAML1_L", (void *)&_StartWeaponIcon, (void *)&_AnimateWeaponIcon, 1,
    0xa22, 669, 0, 0, 6.0f, 32, -1, -1, 0, 400.0f, -1.0f, "WEPICON_ROCK", "WEPICON_ROCKL1_",
    (void *)&_StartWeaponIcon, (void *)&_AnimateWeaponIcon, 1, 0xa22, 670, 0, 0, 6.0f, 32, -1, -1,
    0, 400.0f, -1.0f, "WEPICON_SONC", "WEPICON_SONCL1_", (void *)&_StartWeaponIcon,
    (void *)&_AnimateWeaponIcon, 0, 0xa22, 671, 0, 0, 6.0f, 32, -1, -1, 0, 400.0f, -1.0f,
    "WEPICON_POWUP", "WEPICON_INVSG1", (void *)&_StartWeaponIcon, (void *)&_AnimateWeaponIcon, 0,
    0xa26, 674, 0, 0, 4.0f, 32, -1, -1, 0, 400.0f, -1.0f, "WINDMILL3", "WINDMILL3L2_LOD",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4208, -1, 4, 0, -1.0f, -1, -1, -1,
    0, 400.0f, -1.0f, "WINDMILL2", "WINDMILL2WINDRO", 0, (void *)&_AnimateRotGeneric, 6, 0x206, -1,
    0, 5, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f, "WINDMILL", "WINDMILLWINDROT", 0,
    (void *)&_AnimateRotGeneric, 6, 0x206, -1, 0, 4, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f,
    "WINDROTOR1", "WINDROTOR1L3_LO", (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6,
    0x4208, -1, 4, 0, -1.0f, -1, 152, -1, 0, 400.0f, -1.0f, "WINDROTOR2", "WINDROTOR2L2_LO",
    (void *)&_StartPathTarget, (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, -1, -1, -1,
    0, 400.0f, -1.0f, "WINDROTOR3", "WINDROTOR3G1", (void *)&_StartPathTarget,
    (void *)&_AnimatePathTarget, 6, 0x4228, -1, 4, 0, -1.0f, -1, -1, -1, 0, 400.0f, -1.0f,
    "WPR_MINE", "WPR_MINEBASE", (void *)&_StartMine, (void *)&_AnimateMine, 2, 0x22a, -1, 5, 0,
    12.5f, 61, -1, -1, 0, 400.0f, -1.0f, "YIELDHIT", 0, 0, 0, 6, 4, -1, 7, 0, 0.0f, 62, -1, -1, 0,
    400.0f, -1.0f
};
tBBoxColl BBoxCollInfo[11] = {
    -7.0f, 7.0f, 30.0f, -30.0f, 0.0f, -14.6f, 45.0f, 4000.0f, -7.0f, 7.0f, 33.0f, -33.0f, 0.0f,
    -14.6f, 45.0f, 4000.0f, -4.0f, 4.5f, 10.0f, -10.0f, 28.0f, 0.0f, 45.0f, 4000.0f, -10.0f, 8.0f,
    142.0f, -144.0f, 12.0f, 0.0f, 155.0f, 5000.0f, -6.5f, 6.5f, 17.7f, -18.5f, 18.5f, 0.0f, 40.0f,
    4000.0f, -5.4f, 5.4f, 22.0f, -22.0f, 14.5f, 0.0f, 40.0f, 4000.0f, -20.0f, 20.0f, 30.0f, -30.0f,
    10.0f, -10.0f, 70.0f, 6000.0f, -5.0f, 5.0f, 10.0f, -10.0f, 15.0f, 0.0f, 50.0f, 6000.0f, -10.0f,
    10.0f, 5.0f, -5.0f, 10.0f, 0.0f, 30.0f, 6000.0f, -40.0f, 40.0f, 40.0f, -40.0f, 40.0f, -40.0f,
    120.0f, 6000.0f, -18.0f, 18.0f, 10.0f, -10.0f, 39.0f, 0.0f, 120.0f, 6000.0f
};
tTargetStructRot targetRotInfo[6] = {
    0.0f, 3.0f, 0.0f, -0.19198623f, 0.0f, 0.0f, -0.18762289f, 0.0f, 0.0f, 0.0f, 1.5f, 0.0f, 1.5f,
    0.0f, 0.0f, -1.5f, 0.0f, 0.0f
};
/* explicit `= { 0 }`: the target keeps these objects in initialised data (.data/.sdata), not .bss */
tTargetStructVel targetVelInfo[1] = { 0 };
tTargetStructVelRot targetVelRotInfo[1] = { 0 };
static void * gTargetStructInfoPtrs[8] = {
    (void *)&targetRotInfo, (void *)&targetVelInfo, (void *)&targetVelRotInfo, 0, 0, 0, 0, 0
};
static unsigned char gTargetStructSizes[8] = { 12, 12, 24, 104, 72, 136, 76, 0 };
signed char gMineCones;
signed char gCarMines;
signed char gRandomWeapons;
signed char gDemolitionBattle;
tTargetGroup *gTargetGroups;
int numgTargetGroups;
tTarget **gTargetGroupList;
int numgTargetGroupList;
static unsigned short gMaskedRandomWeapons;
static int gPathCount;
static signed char Restarting;
static signed char gTargetStructCount[8];
static tMiniModelData *gBBoxModelStructs;
static int gBBoxModelCount;
static int gBBoxModelUsed;
static int gBattleMineCnt;
static int gGoldCoinCnt;
static int gWhichGoldCoin;
static int gSilverCoinCnt;
static int gWhichSilverCoin;
static tAlwaysCol *gAlwaysCol;
static int s_init;
static float gCameraMatrix[4][4];
static void * gTargetOwnStructPtrs[8];
tStatList gAlwaysColList;
static tPlayerMines gPlayerMines[4];
static tTarget gTargetArray[150];
tStatList gTargetList;
static void * gTargetStructPtrs[8];

/* 0x8004b54c, size 0x98 */
void exitTargetStructs(void)
{
    int i; /* r29 */
    if (s_init) {
        for (i = 0; i < 8; i++) {
            if (gTargetStructPtrs[i]) {
                DeallocateMemory(gTargetStructPtrs[i]);
                gTargetStructPtrs[i] = 0;
            }
        }
        if (gBBoxModelStructs) {
            DeallocateMemory(gBBoxModelStructs);
            gBBoxModelStructs = 0;
        }
        exitBattlePowerUps();
        s_init = 0;
    }
}

/* 0x8004b5e4, size 0x19c */
void initTargetStructs(void)
{
    InitStatLL(&gTargetList, gTargetArray, 0x70, 150, 0);
    Restarting = gRestartGame;
    InitCollisionTargetPtrs();
    gPathCount = 0;
    { signed char *p = gTargetStructCount; int i; for (i = 0; i < 8; i++) *p++ = 0; }
    if (!s_init) {
        int i = 0;
        while (i < 8) gTargetStructPtrs[i++] = 0; /* a while with the increment in the store, not a for: all eight stores then go through the addi copy of the base, as in the target */
    }
    gBattleMineCnt = 0;
    {
        tPlayerMines *p = gPlayerMines; int i;
        for (i = 0; i < 4; i++) {
            p->m[0] = 0;
            p->m[1] = 0;
            p->m[2] = 0;
            p->m[3] = 0;
            p->num = 0;
            p++;
        }
    }
    gMaskedRandomWeapons = 0;
    if (gRace == kBattle) {
        if (!Restarting) {
            gAlwaysCol = AllocateMemoryHeap(0, 0x78);
            memset(gAlwaysCol, 0, 0x78);
        }
        gTargetStructCount[5] = 16;
        PreInitBattlePowerUps();
        InitStatLL(&gAlwaysColList, gAlwaysCol, 8, 15, 0);
    }
    gBBoxModelCount = 0;
    if (!s_init) gBBoxModelStructs = 0;
    gGoldCoinCnt = gSilverCoinCnt = 0;
    gWhichGoldCoin = gWhichSilverCoin = 0;
    s_init = 1;
}

/* 0x8004b780, size 0x768 */
int SetupTarget(void * obj, signed short Parent, float *pos, int fromWorld, signed char owner, int vis, int col)
{
    int i; /* r0 */
    int superGroup; /* r21 */
    int ranweap; /* r6 */
    char *name; /* r27 */
    tTarget *t; /* r24 */
    unsigned int dlflags; /* r23 */
    tTargetInfo *tInfo; /* r22 */
    signed char targDir; /* r24 */
    float tmpf1; /* f6 */
    float tmpf2; /* f31 */
    tUVMatrix *mat; /* r3 */
    tAlwaysCol *acol; /* r0 */
    unsigned short flags; /* not in the DWARF: a 16-bit copy of the type flags that the original code needs */
    name = obj;
    if (fromWorld) dlflags = ((tModelPart *)obj)->dlflags;
    else dlflags = 0;
    for (i = 0; i < sizeof(targetInfo) / sizeof(targetInfo[0]); i++) {
        if (!strncmp(name, targetInfo[i].name, strlen(targetInfo[i].name))) {
            tInfo = &targetInfo[i];
            if (gRace == kGhost && tInfo->collFuncIdx == 4 && col) return 1;
            if (!(tInfo->Flags & ~4)) return 1;
            targDir = -1;
            if (strstr(name, "_BW")) targDir = 1;
            if (strstr(name, "_FW")) targDir = 0;
            if ((gBackwards && !targDir) || (!gBackwards && targDir == 1)) return 1;
            t = GetStatLLE(&gTargetList);
            if (!t) return 1;
            flags = tInfo->Flags;
            if (flags & 8) gTargetStructCount[tInfo->structureID]++;
            if (flags & 0x8000) gBBoxModelCount++;
            t->mbIdx = MBOX_FindObject_WORK(tInfo->mboxname, 0, gNumModels - 1, 1);
            t->infoIdx = i;
            t->otype = t->type = tInfo->objListIdx;
            t->slot = -1;
            t->owner = owner;
            t->frames = 0;
            t->timeStamp = 0.0f;
            t->sound = -1;
            if (fromWorld) {
                if (tInfo->oRad == -1.0f) tInfo->oRad = MBOX_GetObjectRadius(t->mbIdx);
                tmpf1 = 1.0f;
                for (i = 0; i < 3; i++) {
                    tmpf2 = sqrtf(((tModelPart *)obj)->mat.uvs[i][0] * ((tModelPart *)obj)->mat.uvs[i][0] +
                                  ((tModelPart *)obj)->mat.uvs[i][1] * ((tModelPart *)obj)->mat.uvs[i][1] +
                                  ((tModelPart *)obj)->mat.uvs[i][2] * ((tModelPart *)obj)->mat.uvs[i][2]);
                    if (tmpf2 > tmpf1) tmpf1 = tmpf2;
                }
                if (tmpf1 > 1.0f) {
                    tmpf2 = tInfo->oRad * tmpf1;
                    if (tmpf2 > MBOX_GetObjectRadius(t->mbIdx))
                        gRomObjDefPtrs[t->mbIdx >> 10].romObjDefs[t->mbIdx & 0x3ff].radius = tmpf2;
                }
            }
            if (gRace == kBattle) {
                if (gDemolitionBattle && t->type != 674) {
                    t->otype = t->type = 669;
                } else if (gRandomWeapons && t->type != 674 && tInfo->structureID != 5) {
                    switch (gRandomWeapons) {
                    case 1:
                        ranweap = Random(8.0f);
                        for (;;) {
                            if (!(gMaskedRandomWeapons & (1 << ranweap))) {
                                gMaskedRandomWeapons |= 1 << ranweap;
                                break;
                            }
                            ranweap++;
                            if (ranweap >= 8) ranweap = 0;
                        }
                        t->otype = t->type = ranweap + 664;
                        break;
                    case 2:
                        t->otype = t->type = 672;
                        break;
                    }
                }
            }
            if (flags & 0x400) t->data = obj;
            if (flags & 0x80) dlflags |= 0x8000;
            if (tInfo->Flags & 0x40) dlflags |= 0x400000;
            if (!(flags & 4)) {
                if (tInfo->structureID == 5) t->Flags = 0;
                else t->Flags = 2;
            } else {
                t->Flags = 0;
            }
            if (tInfo->structureID == 4 && fromWorld) t->Flags |= 0x10;
            if (tInfo->structureID == 6) {
                if (strstr(name, "GOLD")) {
                    t->Flags |= 0x40;
                    gGoldCoinCnt++;
                } else {
                    gSilverCoinCnt++;
                }
            }
            if (vis) {
                if (fromWorld) {
                    t->mat.mat3.pos[0] = ((tModelPart *)obj)->mat.mat3.pos[0];
                    t->mat.mat3.pos[1] = ((tModelPart *)obj)->mat.mat3.pos[1];
                    t->mat.mat3.pos[2] = ((tModelPart *)obj)->mat.mat3.pos[2];
                    CopyUV(((tModelPart *)obj)->mat.mat3.uvs, t->mat.mat3.uvs);
                } else {
                    t->mat.mat3.pos[0] = pos[0];
                    t->mat.mat3.pos[1] = pos[1];
                    t->mat.mat3.pos[2] = pos[2];
                    CopyUV(unitmatrix, t->mat.mat3.uvs);
                }
                if (flags & 0x800) {
                    dlflags |= 0x40000;
                    superGroup = -1;
                } else if (Parent == -2) {
                    superGroup = FindWorldGroup(pos);
                } else {
                    superGroup = Parent;
                }
                if (tInfo->structureID == 6) {
                    superGroup = -1;
                    CopyUV(unitmatrix, t->mat.mat3.uvs);
                    dlflags |= 0x52000;
                }
                if (gNumPlayers <= 1 || !(flags & 0x100))
                    t->dlIdx = MBOX_NewObject(t->mbIdx, &t->mat, superGroup, dlflags);
                if ((flags & 0x300) && superGroup >= 0) {
                    mat = ObjInstanceList[superGroup].mat;
                    t->mat.mat3.pos[0] -= mat->mat3.pos[0];
                    t->mat.mat3.pos[1] -= mat->mat3.pos[1];
                    t->mat.mat3.pos[2] -= mat->mat3.pos[2];
                }
                if (tInfo->structureID == 4 && MBOX_GetObjectRadius(t->mbIdx) < 50.0f) {
                    i = ZOID_ObjectFlags(t->dlIdx); /* i reused for the flags: gives mr r0,r3; ori r4,r0,K like the target, where a nested call gives ori r4,r3,K */
                    ZOID_SetObjectFlags(t->dlIdx, i | 0x20);
                }
                MBOX_GetPosition(t->pos, t->dlIdx);
            }
            if (tInfo->collVal == -1.0f) {
                if (tInfo->oRad != -1.0f) t->radius = tInfo->oRad;
                else t->radius = MBOX_GetObjectRadius(t->mbIdx);
            } else {
                t->radius = tInfo->collVal;
            }
            if (tInfo->collFuncIdx != 6) t->Flags |= 8;
            if (fromWorld) t->mbIdx = ((tModelPart *)obj)->tidx;
            else if (tInfo->structureID == 4) t->mbIdx = (int)gTargetPaths.paths[owner].parent;
            if (gGameState & 0x00400000) {
                if (gRace == kBattle) {
                    acol = GetStatLLE(&gAlwaysColList);
                    acol->t = t;
                }
                SetTargetDef(t);
            }
            return 1;
        }
    }
    return 0;
}

/* 0x8004bee8, size 0xa8 */
static void _InitBBoxModelStruct(tTarget *t)
{
    tMiniModelData *m; /* r31 */
    m = &gBBoxModelStructs[t->owner];
    CopyUV(unitmatrix, m->uvs);
    m->pos[0] = m->vel[0] = 0.0f;
    m->pos[1] = m->vel[1] = 0.0f;
    m->pos[2] = m->vel[2] = 0.0f;
    m->bboxInfo = &BBoxCollInfo[(int)targetInfo[t->infoIdx].collVal];
}

/* 0x8004bf90, size 0x84 */
void UpdateMiniModelStruct(int miniIdx, float *vel, float *pos, float (*uvs)[3])
{
    tMiniModelData *m; /* r31 */
    m = &gBBoxModelStructs[miniIdx];
    CopyUV(uvs, m->uvs);
    m->pos[0] = pos[0];
    m->pos[1] = pos[1];
    m->pos[2] = pos[2];
    m->vel[0] = vel[0];
    m->vel[1] = vel[1];
    m->vel[2] = vel[2];
}

/* 0x8004c014, size 0x134 */
void TargetPathAvoid(tModelData *m, float *scale1)
{
    tMiniModelData *tp; /* r31 */
    float pos[3]; /* stack */
    float rpos[3]; /* stack */
    float scale2; /* f1 */
    if (!gBBoxModelUsed) return;
    for (tp = gBBoxModelStructs; tp < &gBBoxModelStructs[gBBoxModelUsed - 1]; tp++) {
        pos[0] = tp->pos[0] - m->reckon.rwr[0];
        pos[1] = tp->pos[1] - m->reckon.rwr[1];
        pos[2] = tp->pos[2] - m->reckon.rwr[2];
        BodyVector(pos, rpos, m->reckon.uvs);
        if (rpos[2] < 0.0f || rpos[2] > 225.0f || fabs(rpos[0]) > 14.0) continue;
        if (rpos[2] < 125.0f) scale2 = 0.75f;
        else if (rpos[2] < 225.0f) scale2 = 1.0f - 0.0025f * (225.0f - rpos[2]);
        else scale2 = 1.0f;
        if (scale2 < *scale1) *scale1 = scale2;
    }
}

/* 0x8004c148, size 0x364 */
static void SetTargetDef(tTarget *t)
{
    int x; /* r0 */
    int i; /* r9 */
    tTargetInfo *tInfo; /* r31 */
    tVisual *v; /* r0 */
    tInfo = &targetInfo[t->infoIdx];
    x = tInfo->structureID;
    if (!(gGameState & 0x00400000)) {
        if (tInfo->Flags & 8) {
            t->data = gTargetOwnStructPtrs[x];
            if (!(t->Flags & 0x10) && x == 4) {
                ((tTargetStructPath *)t->data)->pathData = &gTargetPaths.paths[t->owner];
                if (((tTargetStructPath *)t->data)->pathData->flags & 0x10) t->Flags &= ~8;
            }
            gTargetOwnStructPtrs[x] = (void *)((int)gTargetOwnStructPtrs[x] + gTargetStructSizes[x]);
        } else if (x != 7) {
            t->data = (char *)gTargetStructInfoPtrs[x] + gTargetStructSizes[x] * tInfo->structureIdx;
        }
        if (tInfo->collFuncIdx == 4) {
            t->owner = gBBoxModelUsed;
            gBBoxModelUsed++;
            _InitBBoxModelStruct(t);
        }
    } else if ((tInfo->Flags & 8) && x == 5) {
        for (i = 0; i < 16; i++) {
            if (!((tTargetStructMine *)gTargetStructPtrs[5])[i].used && ((tTargetStructMine *)gTargetStructPtrs[5])[i].dlIdx == -1 &&
                ((tTargetStructMine *)gTargetStructPtrs[5])[i].glowdls[0] == -1 && ((tTargetStructMine *)gTargetStructPtrs[5])[i].glowdls[1] == -1) break;
        }
        ((tTargetStructMine *)gTargetStructPtrs[5])[i].used = 1;
        t->data = (char *)gTargetOwnStructPtrs[x] + i * gTargetStructSizes[x];
    }
    if (t->otype != -1) ZOID_SetObjectDef(t->dlIdx, gObjList[t->otype]);
    if (tInfo->Flags & 2) {
        v = GrabEnvEntry();
        if (v) {
            v->index = 0;
            v->func = tInfo->animFunc;
            v->data = (unsigned int)t;
            v->timeStamp = 0.0f;
            AddToEnvList(v);
            if ((tInfo->Flags & 4) && tInfo->startFunc) tInfo->startFunc(t);
        }
    }
    if (tInfo->Flags & 0x4000) {
        if (!(t->Flags & 0x10)) {
            tInfo->startFunc(t);
        } else {
            float vel[3] = { 0.0f, 0.0f, 0.0f };
            UpdateMiniModelStruct(t->owner, vel, t->pos, t->mat.mat3.uvs);
        }
    }
}

/* 0x8004c4ac, size 0x5d4 */
void SetTargetDefs(void)
{
    int i; /* r20 */
    int idx; /* r7 */
    tTarget *t; /* r8 */
    float pos[3]; /* stack */
    int j; /* r0 */
    gBBoxModelUsed = 0;
    if (gTargetPaths.count) {
        pos[0] = pos[1] = pos[2] = 0.0f;
        for (i = 0; i < gTargetPaths.count; i++) {
            if (gRace == kGhost && gTargetPaths.paths[i].collIdx > 0) {
                for (j = 0; j < gTargetPaths.paths[i].numPts; j++) {
                    if (gTargetPaths.paths[i].pts[j].flags & 0x01000000) goto normal;
                }
                if (gTargetPaths.paths[i].collIdx > 0) RemoveCollisionObject(gTargetPaths.paths[i].collIdx);
                continue;
            }
        normal:
            {
                unsigned int flags;
                unsigned int fw;
                tBasicTargetPath *p;
                int vis, col;
                p = &gTargetPaths.paths[i];
                flags = p->flags;
                fw = flags & 4;
                if ((fw && (flags & 8)) || (gBackwards && (flags & 8)) || (!gBackwards && fw)) {
                    vis = _NB(flags, 15);
                    col = _NB(flags, 4);
                    for (j = 0; j < gTargetPaths.paths[i].numPts; j++) {
                        if (Restarting) gTargetPaths.paths[i].pts[j].flags &= ~0x1000;
                        if (gTargetPaths.paths[i].pts[j].flags & 1)
                            SetupTarget(&gTargetPaths.paths[i], -1, pos, 0, i, vis, col);
                    }
                    if (Restarting && gTargetPaths.paths[i].collIdx > 0) ResetCollisionObject(gTargetPaths.paths[i].collIdx);
                } else if (flags & 0x800) {
                    RemoveCollisionObject(p->collIdx);
                }
            }
        }
    }
    if (!Restarting) {
        for (i = 0; i < 8; i++) {
            if (gTargetStructCount[i]) {
                if (gTargetStructPtrs[i]) {
                    DeallocateMemory(gTargetStructPtrs[i]);
                    gTargetStructPtrs[i] = 0;
                }
                gTargetStructPtrs[i] = AllocateMemoryHeap(0, gTargetStructSizes[i] * gTargetStructCount[i]);
                memset(gTargetStructPtrs[i], 0, gTargetStructSizes[i] * gTargetStructCount[i]);
            }
        }
        if (gBBoxModelCount) {
            if (gBBoxModelStructs) {
                DeallocateMemory(gBBoxModelStructs);
                gBBoxModelStructs = 0;
            }
            gBBoxModelStructs = AllocateMemoryHeap(0, gBBoxModelCount * sizeof(tMiniModelData));
            memset(gBBoxModelStructs, 0, gBBoxModelCount * sizeof(tMiniModelData));
        }
        for (i = 0; i < numgTargetGroupList; i++) {
            idx = (int)gTargetGroupList[i];
            for (t = gTargetList.head; t; t = t->ll.next) {
                if ((t->Flags & 8) && idx == t->mbIdx) {
                    gTargetGroupList[i] = t;
                    break;
                }
            }
            if (idx == (int)gTargetGroupList[i]) gTargetGroupList[i] = 0;
        }
    }
    if (gRace == kBattle) {
        for (i = 0; i < 16; i++) {
            ((tTargetStructMine *)gTargetStructPtrs[5])[i].used = 0;
            ((tTargetStructMine *)gTargetStructPtrs[5])[i].dlIdx = -1;
            ((tTargetStructMine *)gTargetStructPtrs[5])[i].glowdls[0] = -1;
            ((tTargetStructMine *)gTargetStructPtrs[5])[i].glowdls[1] = -1;
        }
    }
    for (i = 0; i < 8; i++) {
        if (gTargetStructCount[i]) gTargetOwnStructPtrs[i] = gTargetStructPtrs[i];
    }
    for (t = gTargetList.head; t; t = t->ll.next) SetTargetDef(t);
    if (gRace == kBattle) InitBattlePowerUps(Restarting);
}

/* 0x8004ca80, size 0x290 */
void CheckTargetCollision(signed short slot)
{
    tTarget *t; /* r28 */
    void * d1; /* r3 */
    void * d2; /* r4 */
    void * d3; /* r5 */
    float x, z; /* not in the DWARF */
    float mh; /* f3 */
    tTargetInfo *tInfo; /* r27 */
    tTargetGroup *tg; /* r30 */
    int currentG; /* r0 */
    int i; /* r29 */
    tCarData *gc; /* r29 */
    int infoindex; /* r0 */
    tAlwaysCol *acol; /* r26 */
    tAlwaysCol *acolnext; /* r25 */
    gc = &gGameCar[slot];
    if (gRace == kBattle) {
        for (acol = gAlwaysColList.head; acol; acol = acolnext) {
            t = acol->t;
            acolnext = acol->ll.next;
            tInfo = &targetInfo[t->infoIdx];
            if (gTargetCollFuncPtrs[tInfo->collFuncIdx](&slot, t->pos, &t->radius, 0)) {
                t->Flags |= 4;
                t->slot = slot;
                if (tInfo->startFunc) tInfo->startFunc(t);
                FreeStatLLE(&gAlwaysColList, acol);
            }
        }
    }
    x = gc->dr_pos[0];
    z = gc->dr_pos[2];
    currentG = 0;
    for (;;) {
        tg = &gTargetGroups[currentG];
        if (x >= tg->minh && x < tg->maxh && z >= tg->minv && z < tg->maxv) {
            if (tg->flags & 1) {
                mh = gTargetGroups[tg->child[2]].maxh;
                if (z >= gTargetGroups[tg->child[2]].maxv) {
                    if (x < mh) currentG = tg->child[0];
                    else currentG = tg->child[1];
                } else {
                    if (x < mh) currentG = tg->child[2];
                    else currentG = tg->child[3];
                }
            } else {
                for (i = 0; i < tg->child[0]; i++) {
                    t = gTargetGroupList[tg->child[1] + i];
                    if (t && (t->Flags & 2) && (t->Flags & 8)) {
                        infoindex = t->infoIdx;
                        if (infoindex < 0) return;
                        tInfo = &targetInfo[infoindex];
                        switch (tInfo->collFuncIdx) {
                        case 0:
                        case 1:
                        case 2:
                            d1 = &slot;
                            d2 = t->pos;
                            d3 = &t->radius;
                            break;
                        case 3:
                        case 4:
                        case 5:
                            d1 = t;
                            d2 = &slot;
                            d3 = 0;
                            break;
                        default:
                            continue;
                        }
                        if (gTargetCollFuncPtrs[tInfo->collFuncIdx](d1, d2, d3, 0)) {
                            t->Flags |= 4;
                            t->slot = slot;
                            if (tInfo->startFunc) tInfo->startFunc(t);
                        }
                    }
                }
                return;
            }
        } else return;
    }
}

/* 0x8004cd10, size 0x48 */
void PauseTargets(void)
{
    tTarget *t; /* r31 */
    for (t = gTargetList.head; t; t = t->ll.next) PauseTargetPaths(t);
}

/* 0x8004cd58, size 0x78 */
static int _LocalPointInBody(tMiniModelData *m, float *pt)
{
    if (pt[0] > m->bboxInfo->right || pt[0] < m->bboxInfo->left) return 0;
    if (pt[1] > m->bboxInfo->top || pt[1] < m->bboxInfo->bot) return 0;
    if (pt[2] > m->bboxInfo->front || pt[2] < m->bboxInfo->back) return 0;
    return 1;
}

/* 0x8004cdd0, size 0x1ac */
static int _TargetCollBBox(void * data1, void * data2)
{
    tModelData *m; /* r30 */
    tMiniModelData *tm; /* r29 */
    float vec[3]; /* stack */
    float pos[3]; /* stack */
    float posr[3]; /* stack */
    float dsq; /* f5 */
    int i; /* r28 */
    m = &model[*(signed short *)data2];
    tm = &gBBoxModelStructs[((tTarget *)data1)->owner];
    if (!m->collidable) return 0;
    dsq = 0.0f;
    for (i = 0; i < 3; i++) vec[i] = m->reckon.rwr[i] - tm->pos[i];
    for (i = 0; i < 3; i++) dsq += vec[i] * vec[i];
    if (dsq > (tm->bboxInfo->rad + m->colrad) * (tm->bboxInfo->rad + m->colrad)) return 0;
    for (i = 0; i < 4; i++) {
        WorldVector(m->bodyR[i], pos, m->reckon.uvs);
        pos[0] += m->reckon.rwr[0];
        pos[1] += m->reckon.rwr[1];
        pos[2] += m->reckon.rwr[2];
        pos[0] -= tm->pos[0];
        pos[1] -= tm->pos[1];
        pos[2] -= tm->pos[2];
        BodyVector(pos, posr, tm->uvs);
        if (_LocalPointInBody(tm, posr)) {
            setMiniFBCollisionForce(m, m, tm, vec, posr);
            return 0;
        }
    }
    return 0;
}

/* 0x8004cf7c, size 0x128 */
static int _TargetCollTrigger(void * data1, void * data2)
{
    tTarget *t; /* r0 */
    signed short slot; /* r4 */
    tModelData *m; /* r8 */
    tBasicTargetPath *pPtr; /* r3 */
    int surfNum; /* r7 */
    unsigned short ts1; /* r10 */
    unsigned short ts2; /* r11 */
    unsigned short ts3; /* r5 */
    unsigned short ts4; /* r6 */
    t = data1;
    slot = *(signed short *)data2;
    m = &model[slot];
    if (m->crashflag || m->resurrect.moving_state >= 0) return 0;
    pPtr = ((tTargetStructPath *)t->data)->pathData;
    surfNum = pPtr->collIdx << 11;
    ts1 = surf[m->tiresli[0]].snd_flags;
    ts2 = surf[m->tiresli[1]].snd_flags;
    ts3 = surf[m->tiresli[2]].snd_flags;
    ts4 = surf[m->tiresli[3]].snd_flags;
    if (((ts1 & 0x20) && (ts1 & 0xf800) == surfNum) || ((ts2 & 0x20) && (ts2 & 0xf800) == surfNum) ||
        ((ts3 & 0x20) && (ts3 & 0xf800) == surfNum) || ((ts4 & 0x20) && (ts4 & 0xf800) == surfNum)) {
        if (!(pPtr->flags & 0xff000000) || (pPtr->flags & 0x4000)) pPtr->flags |= 0x200;
        pPtr->flags |= 1 << (slot + 24);
    } else {
        pPtr->flags &= ~(1 << (slot + 24));
    }
    return 0;
}

/* 0x8004d0a4, size 0xbc */
static int _TargetCollCylinder(void * data1, void * data2, void * data3, void * data4)
{
    float dx, dy, dz; /* not in the DWARF: function-scope deltas declared before the DWARF locals give radius its original FP registers */
    float gap; /* f2 */
    float radius; /* f5 */
    float *dist; /* r0 */
    tCarData *gc; /* r3 */
    tModelData *m; /* not in the DWARF: fixes the integer colouring of the collidable test */
    m = &model[*(signed short *)data1];
    radius = *(float *)data3;
    dist = data4;
    gc = &gGameCar[*(signed short *)data1];
    if (!m->collidable) return 0;
    dx = gc->dr_pos[0] - ((float *)data2)[0];
    dy = gc->dr_pos[1] - ((float *)data2)[1];
    dz = gc->dr_pos[2] - ((float *)data2)[2];
    radius += 3.5f; /* after the deltas on purpose: this source position decides radius's FP colouring even though the add is scheduled in the same slot */
    gap = dx * dx + dz * dz - radius * radius;
    if (dist) *dist = gap;
    if (gap > 0.0f) return 0;
    if (dy > -2.0f && dy < 18.0f) return 1;
    return 0;
}

/* 0x8004d160, size 0xbc */
static int _TargetCollMineCylinder(void * data1, void * data2, void * data3, void * data4)
{
    float dx, dy, dz; /* not in the DWARF: function-scope deltas declared before the DWARF locals give radius its original FP registers */
    float gap; /* f2 */
    float radius; /* f5 */
    float *dist; /* r0 */
    tCarData *gc; /* r3 */
    tModelData *m; /* not in the DWARF: fixes the integer colouring of the collidable test */
    m = &model[*(signed short *)data1];
    radius = *(float *)data3;
    dist = data4;
    gc = &gGameCar[*(signed short *)data1];
    if (!m->collidable) return 0;
    dx = gc->dr_pos[0] - ((float *)data2)[0];
    dy = gc->dr_pos[1] - ((float *)data2)[1];
    dz = gc->dr_pos[2] - ((float *)data2)[2];
    radius += 3.5f; /* after the deltas on purpose: this source position decides radius's FP colouring */
    gap = dx * dx + dz * dz - radius * radius;
    if (dist) *dist = gap;
    if (gap > 0.0f) return 0;
    if (dy > -2.0f && dy < 4.0f) return 1;
    return 0;
}

/* 0x8004d21c, size 0x9c */
static int _TargetCollRadius(void * data1, void * data2, void * data3, void * data4)
{
    float dx, dy, dz; /* not in the DWARF: function-scope deltas declared before the DWARF locals give radius its original FP registers */
    float gap; /* f1 */
    float radius; /* f6 */
    float *dist; /* r0 */
    tCarData *gc; /* r3 */
    tModelData *m; /* not in the DWARF: fixes the integer colouring of the collidable test */
    m = &model[*(signed short *)data1];
    radius = *(float *)data3;
    dist = data4;
    gc = &gGameCar[*(signed short *)data1];
    if (!m->collidable) return 0;
    dx = gc->dr_pos[0] - ((float *)data2)[0];
    dy = gc->dr_pos[1] - ((float *)data2)[1];
    dz = gc->dr_pos[2] - ((float *)data2)[2];
    radius += 3.5f; /* after the deltas on purpose: this source position decides radius's FP colouring */
    gap = dx * dx + dy * dy + dz * dz - radius * radius;
    if (dist) *dist = gap;
    return gap <= 0.0f;
}

/* 0x8004d2b8, size 0x8 */
static int _TargetColl3Piece(void)
{
    return 0;
}

/* stripped by the linker and leaves no data; an empty placeholder kept only at its DWARF position */
static void _StartGuardrail(tTarget *t)
{
    signed short parent; /* r0 */
    unsigned int flags; /* r28 */
    char name[20]; /* stack */
    tTargetInfo *tInfo; /* r0 */
    int i; /* r0 */
    /* stripped */
}

/* stripped by the linker and leaves no data; an empty placeholder kept only at its DWARF position */
static void _AnimateGuardrail(tVisual *v, signed short op)
{
    tTarget *t; /* r31 */
    /* stripped */
}

/* 0x8004d2c0, size 0x168 */
static void _StartMine(tTarget *t)
{
    tCarData *gh; /* r30 */
    float pos[3]; /* stack */
    float pos2[3]; /* stack */
    if (model[t->slot].crashflag) return;
    gh = &gGameCar[t->slot];
    if (gh->battle.weapon.type == 5) {
        pos[0] = t->pos[0] - gh->dr_pos[0];
        pos[1] = t->pos[1] - gh->dr_pos[1];
        pos[2] = t->pos[2] - gh->dr_pos[2];
        BodyVector(pos, pos2, gh->dr_uvs);
        if (pos2[2] >= 0.0f) BattleDamage(&gGameCar[t->owner], gh, 280);
        else BattleDamage(&gGameCar[t->owner], gh, 800);
    } else BattleDamage(&gGameCar[t->owner], gh, 800);
    t->Flags &= ~2;
    PlayTargetSound(targetInfo[t->infoIdx].startSnd, t->slot, t->mat.mat3.pos, 2);
}

/* mid-file definition at its DWARF position: the .rodata order (targetInfo strings, @688, gMineRingColors, ammoCnts) needs it here */
const unsigned char gMineRingColors[4][3] = { 0, 132, 255, 255, 0, 0, 255, 255, 0, 0, 255, 0 };

/* 0x8004d428, size 0x86c */
static void _AnimateMine(tVisual *v, signed short op)
{
    int i; /* r0 */
    int j; /* r7 */
    tTarget *t; /* r26 */
    tTargetStructMine *m; /* r25 */
    tRGBA clr; /* stack */
    tRGBA *clrPtr; /* r0 */
    float opos[3]; /* stack */
    float uvs[3][3]; /* stack */
    float oldT; /* f1 */
    tAlwaysCol *acol; /* r4 */
    tAlwaysCol *acolnext; /* r27 */
    if (!op) {
        ReleaseEnviron(v, 1);
        return;
    }
    if (gPauseState != kNoPause) return;
    t = (tTarget *)v->data;
    m = t->data;
    if (!(t->Flags & 1)) {
        if (gPlayerMines[t->owner].num == 3) {
            oldT = 0.0f;
            j = -1;
            for (i = 0; i < 4; i++) {
                if (gPlayerMines[t->owner].m[i] && gPlayerMines[t->owner].m[i]->timeStamp > oldT) {
                    j = i;
                    oldT = gPlayerMines[t->owner].m[i]->timeStamp;
                }
            }
            if (j != -1) gPlayerMines[t->owner].m[j]->timeStamp = 121.0f;
        }
        for (i = 0; i < 4; i++) {
            if (!gPlayerMines[t->owner].m[i]) {
                gPlayerMines[t->owner].m[i] = t;
                gPlayerMines[t->owner].num++;
                break;
            }
        }
        if (RoadPosition(t->pos, opos, uvs)) {
            for (i = 0; i < 3; i++) {
                for (j = 0; j < 3; j++) t->mat.mat3.uvs[i][j] = uvs[i][j];
            }
        }
        m->dlIdx = MBOX_NewObject(gWprObjList[5], (tUVMatrix *)unitmatrix, t->dlIdx, 0x80);
        ZOID_SetTexture(m->dlIdx, &gTexDefPtrs[gMineTextures[4] >> 10].texDefs[gMineTextures[4] & 1023], -1);
        m->flashTime = 0.0f;
        m->on = 0;
        clr.c[0] = gMineRingColors[gCarTeam[t->owner]][0];
        clr.c[1] = gMineRingColors[gCarTeam[t->owner]][1];
        clr.c[2] = gMineRingColors[gCarTeam[t->owner]][2];
        CopyUV(t->mat.mat3.uvs, m->mats[0].mat.mat3.uvs);
        CopyUV(t->mat.mat3.uvs, m->mats[1].mat.mat3.uvs);
        m->mats[0].mat.mat3.pos[0] = t->pos[0];
        m->mats[0].mat.mat3.pos[1] = t->pos[1];
        m->mats[0].mat.mat3.pos[2] = t->pos[2];
        m->mats[0].mat.mat3.pos[0] += 0.75f * m->mats[0].mat.mat3.uvs[1][0];
        m->mats[0].mat.mat3.pos[1] += 0.75f * m->mats[0].mat.mat3.uvs[1][1];
        m->mats[0].mat.mat3.pos[2] += 0.75f * m->mats[0].mat.mat3.uvs[1][2];
        m->mats[1].mat.mat3.pos[0] = t->pos[0];
        m->mats[1].mat.mat3.pos[1] = t->pos[1];
        m->mats[1].mat.mat3.pos[2] = t->pos[2];
        m->mats[1].mat.mat3.pos[0] += m->mats[1].mat.mat3.uvs[1][0];
        m->mats[1].mat.mat3.pos[1] += m->mats[1].mat.mat3.uvs[1][1];
        m->mats[1].mat.mat3.pos[2] += m->mats[1].mat.mat3.uvs[1][2];
        m->glowdls[0] = MBOX_NewObject(gWprObjList[6], &m->mats[0].mat, -1, 0x42000);
        ZOID_SetColor(m->glowdls[0], &clr);
        m->glowdls[1] = MBOX_NewObject(gWprObjList[6], &m->mats[1].mat, -1, 0x42000);
        ZOID_SetColor(m->glowdls[1], &clr);
        m->sc[1] = 1.0f;
        m->sc[0] = 1.0f;
        m->alphs[0] = 255;
        m->alphs[1] = 255;
        m->glowTimes[0] = 0.0f;
        m->glowTimes[1] = 0.0f;
        m->glowCnts[0] = 0;
        m->glowCnts[1] = -1;
        MBOX_HideObject(m->glowdls[1], 0, 15);
        t->Flags |= 3;
    }
    m->flashTime += gFrameSeconds;
    if (m->flashTime >= 0.25f) {
        if (m->on) {
            m->on = 0;
            ZOID_SetTexture(m->dlIdx, &gTexDefPtrs[gMineTextures[4] >> 10].texDefs[gMineTextures[4] & 1023], -1);
        } else {
            m->on = 1;
            ZOID_SetTexture(m->dlIdx, &gTexDefPtrs[gMineTextures[gCarTeam[t->owner]] >> 10].texDefs[gMineTextures[gCarTeam[t->owner]] & 1023], -1);
        }
        m->flashTime = 0.0f;
    }
    for (i = 0; i < 2; i++) {
        if (m->glowCnts[i] < 0) continue;
        if (m->glowCnts[i] >= 45) {
            m->sc[i] = 1.0f;
            m->alphs[i] = 255;
            clrPtr = ZOID_GetColor(m->glowdls[i]);
            clrPtr->c[3] = 255;
            CopyUV(t->mat.mat3.uvs, m->mats[i].mat.mat3.uvs);
            m->glowCnts[i] = -1;
            MBOX_HideObject(m->glowdls[i], 0, 15);
            continue;
        }
        if (m->glowTimes[i] >= 0.0333333f) {
            CopyUV(t->mat.mat3.uvs, m->mats[i].mat.mat3.uvs);
            for (j = 0; j < 9; j++) (&m->mats[i].mat.mat3.uvs[0][0])[j] *= m->sc[i]; /* a loop over the matrix as a flat array: nine explicit statements allocate the first multiply differently */
            m->sc[i] += 0.25f;
            m->glowTimes[i] = 0.0f;
            m->glowCnts[i]++;
        }
        m->glowTimes[i] += gFrameSeconds;
        if (m->glowCnts[i] >= 20) {
            if (m->alphs[i] > 8) m->alphs[i] -= 8;
            clrPtr = ZOID_GetColor(m->glowdls[i]);
            clrPtr->c[3] = m->alphs[i];
        }
        if (m->glowCnts[i] >= 25) {
            j = !i;
            if (m->glowCnts[j] < 0) {
                m->glowCnts[j] = 0;
                MBOX_ShowObject(m->glowdls[j], 0, 15);
            }
        }
    }
    t->timeStamp += gFrameSeconds;
    if (!(t->Flags & 2) || (t->Flags & 4) || t->timeStamp > 120.0f) {
        acol = gAlwaysColList.head;
        while (acol) {
            acolnext = acol->ll.next;
            if (acol->t == t) FreeStatLLE(&gAlwaysColList, acol);
            acol = acolnext;
        }
        for (i = 0; i < 4; i++) {
            if (gPlayerMines[t->owner].m[i] == t) {
                gPlayerMines[t->owner].m[i] = 0;
                gPlayerMines[t->owner].num--;
                break;
            }
        }
        if (t->timeStamp > 120.0f) ProximityDamage(t->pos, t->pos, t->owner, 400.0f, 1600);
        StartBlast(t->pos, 0, 0.4f, 1);
        if (m->glowdls[0] != -1) {
            ZOID_RemoveObject(m->glowdls[0]);
            m->glowdls[0] = -1;
        }
        if (m->glowdls[1] != -1) {
            ZOID_RemoveObject(m->glowdls[1]);
            m->glowdls[1] = -1;
        }
        if (m->dlIdx != -1) {
            ZOID_RemoveObject(m->dlIdx);
            m->dlIdx = -1;
        }
        m->used = 0;
        ZOID_RemoveObject(t->dlIdx);
        t->Flags = 0;
        FreeStatLLE(&gTargetList, t);
        ReleaseEnviron(v, 1);
    }
}

/* mid-file definition at its DWARF position, after gMineRingColors in .rodata */
static const int ammoCnts[10] = { 20, 100, 20, 3, 3, 5, 20, 5, -1, 0 };

/* 0x8004dc94, size 0x428 */
static void _StartWeaponIcon(tTarget *t)
{
    tCarData *car; /* r31 */
    tModelData *m; /* r30 */
    int type; /* r3 */
    int ammo; /* r0 */
    int i; /* r7 */
    int wep; /* r29 */
    anon_colfront_colrear_10 *sizep; /* r3 */
    if (t->Flags & 4) {
        car = &gGameCar[t->slot];
        m = &model[t->slot];
        t->Flags &= ~6;
        if (car->appearance & 0x10) {
            t->Flags |= 2;
            return;
        }
        t->timeStamp = 0.0f;
        PlayTargetSound(targetInfo[t->infoIdx].startSnd, t->slot, t->mat.mat3.pos, 2);
        switch (t->type) {
        case 672:
            wep = 1;
            type = Random(8.0f);
    again:
            for (i = 0; i < gNumPlayers; i++) {
                if (i != t->slot && type == gGameCar[i].battle.weapon.type) {
                    type++;
                    if (type >= 8) type = 0;
                    goto again;
                }
            }
            ammo = ammoCnts[type];
            break;
        case 664:
            type = 0;
            ammo = ammoCnts[0];
            wep = 1;
            break;
        case 665:
            type = 1;
            ammo = ammoCnts[1];
            wep = 1;
            break;
        case 666:
            type = 2;
            ammo = ammoCnts[2];
            wep = 1;
            break;
        case 673:
            car->battle.health = 800;
            return;
        case 674:
            car->battle.defense.active |= 1;
            car->battle.defense.invsTime = 30.0f;
            car->battle.fadeState = 1;
            car->battle.fadeTime = 0.666667f;
            car->battle.fadeAlpha = 255;
            return;
        case 667:
            type = 3;
            ammo = ammoCnts[3];
            wep = 1;
            break;
        case 668:
            type = 4;
            ammo = ammoCnts[4];
            wep = 1;
            break;
        case 669:
            type = 5;
            ammo = ammoCnts[5];
            wep = 1;
            break;
        case 670:
            type = 6;
            ammo = ammoCnts[6];
            wep = 1;
            break;
        case 675:
            car->battle.defense.active |= 0xa;
            car->battle.defense.shldTime = 0.0333333f;
            car->battle.defense.shldScale = 0.05f;
            return;
        case 671:
            type = 7;
            ammo = ammoCnts[7];
            wep = 1;
            break;
        default:
            type = 1;
            ammo = ammoCnts[1];
            wep = 1;
            break;
        }
        if (wep) {
            car->battle.weapon.type = type;
            car->battle.weapon.ammoCnt = ammo;
            if (type == 5) {
                float z = 3.0f; /* a block local, not the literal: with a literal CodeWarrior always makes the constant operand A of the fadds */
                sizep = &collsize[m->body_type];
                m->bodyR[0][2] = m->bodyR[1][2] = sizep->colfront + z;
                z = m->bodyR[0][2] > sizep->colrear ? m->bodyR[0][2] : sizep->colrear;
                m->colrad = sqrtf(z * z + sizep->colside * sizep->colside + sizep->colheight * sizep->colheight);
            }
        }
    } else {
        IncBattlePowerUps();
        t->Flags &= ~2;
        t->Flags |= 0x20;
        MBOX_HideObject(t->dlIdx, 0, 15);
    }
}

/* 0x8004e0bc, size 0x1f4 */
static void _AnimateWeaponIcon(tVisual *v, signed short op)
{
    tTarget *t; /* r31 */
    float *angV; /* r5 */
    float angVF[3]; /* stack */
    int i; /* r0 */
    int ok; /* r4 */
    int wep; /* r5 */
    if (!op) {
        ReleaseEnviron(v, 1);
        return;
    }
    if (gPauseState != kNoPause) return;
    t = (tTarget *)v->data;
    if (t->Flags & 2) {
        angV = t->data;
        angVF[0] = angV[0] * gFrameSeconds;
        angVF[1] = angV[1] * gFrameSeconds;
        angVF[2] = angV[2] * gFrameSeconds;
        RotateUV(angVF, t->mat.mat3.uvs);
    } else if (ZOID_ObjectFlags(t->dlIdx) & 0x80000000) {
        switch (t->type) {
        case 672:
            wep = -1;
            t->timeStamp += gFrameSeconds;
            if (t->timeStamp > 15.0f) goto show;
            break;
        case 664: wep = 0; break;
        case 665: wep = 1; break;
        case 666: wep = 2; break;
        case 667: wep = 3; break;
        case 668: wep = 4; break;
        case 669:
            if (gDemolitionBattle) {
                wep = -1;
                t->timeStamp += gFrameSeconds;
                if (t->timeStamp > 15.0f) goto show;
            } else wep = 5;
            break;
        case 670: wep = 6; break;
        case 671: wep = 7; break;
        default: wep = -1; break;
        }
        if (wep != -1) {
            ok = 1;
            for (i = 0; i < gNumPlayers; i++) {
                if (wep == gGameCar[i].battle.weapon.type) ok = 0;
            }
            if (ok) {
            show:
                MBOX_ShowObject(t->dlIdx, 0, 15);
                t->Flags |= 2;
            }
        }
    } else {
        t->timeStamp = 0.0f;
        MBOX_HideObject(t->dlIdx, 0, 15);
    }
}

/* mid-file definition at its DWARF position: in .data it follows the two switch jump tables of the functions above */
static char *flag_pals[3] = { "REDFLAG", "GRNFLAG", "BLUFLAG" };

/* 0x8004e2b0, size 0x144 */
static void _AnimateFlag(tVisual *v, signed short op)
{
    tTarget *t; /* r31 */
    tTexture *tex; /* r3 */
    signed char clrIdx; /* r0 */
    unsigned short ind; /* stack */
    char *color; /* r29 */
    if (!op) {
        ReleaseEnviron(v, 1);
    } else if (gPauseState == kNoPause) {
        t = (tTarget *)v->data;
        if (!(t->Flags & 1)) {
            t->frames = 10;
            color = t->data;
            if (strstr(color, "BLU")) clrIdx = 2;
            else if (strstr(color, "GRN")) clrIdx = 1;
            else clrIdx = 0;
            tex = MBOX_FindTexture_WORK(flag_pals[clrIdx], &ind, 0, gNumModels - 1, 1);
            ZOID_SetTexture(t->dlIdx, tex, -1);
            color = strstr(t->data, "_OFF");
            if (color) clrIdx = color[4] - '0';
            else clrIdx = 0;
            v->index = clrIdx;
            t->Flags |= 1;
        }
        v->timeStamp = 0.0f;
        v->func = (void *)_AnimateObjSwapGeneric;
    }
}

/* 0x8004e3f4, size 0x108 */
static void _StartCactus(tTarget *t)
{
    tVisual *v; /* r30 */
    tTargetInfo *tInfo; /* r29 */
    tCarData *car; /* r5 */
    float vel[3]; /* stack */
    tInfo = &targetInfo[t->infoIdx];
    t->frames = 7;
    v = GrabEnvEntry();
    if (v) {
        v->index = 0;
        v->func = tInfo->animFunc;
        v->data = (unsigned int)t;
        v->timeStamp = 0.0333333f;
        car = &gGameCar[t->slot];
        vel[0] = 0.125f * car->dr_vel[0];
        vel[1] = 0.125f * car->dr_vel[1];
        vel[2] = 0.125f * car->dr_vel[2];
        LookInDir(vel, &t->mat);
        t->Flags &= ~6;
        AddToEnvList(v);
        PlayTargetSound(targetInfo[t->infoIdx].startSnd, t->slot, t->mat.mat3.pos, 2);
    }
}

/* 0x8004e4fc, size 0xb0 */
static void _StartFence(tTarget *t)
{
    tVisual *v; /* r0 */
    tTargetInfo *tInfo; /* r31 */
    tInfo = &targetInfo[t->infoIdx];
    t->frames = 12;
    v = GrabEnvEntry();
    if (v) {
        v->index = 0;
        v->func = tInfo->animFunc;
        v->data = (unsigned int)t;
        v->timeStamp = 0.0333333f;
        t->Flags &= ~6;
        AddToEnvList(v);
        PlayTargetSound(targetInfo[t->infoIdx].startSnd, t->slot, t->mat.mat3.pos, 2);
    }
}

/* 0x8004e5ac, size 0x110 */
static void _StartWindow(tTarget *t)
{
    tVisual *v; /* r30 */
    tTargetInfo *tInfo; /* r29 */
    tCarData *car; /* r3 */
    tInfo = &targetInfo[t->infoIdx];
    t->frames = 7;
    v = GrabEnvEntry();
    if (v) {
        v->index = 0;
        v->func = tInfo->animFunc;
        v->data = (unsigned int)t;
        v->timeStamp = 0.0333333f;
        car = &gGameCar[t->slot];
        if (car->dr_vel[0] * t->mat.mat3.uvs[2][0] + car->dr_vel[1] * t->mat.mat3.uvs[2][1] + car->dr_vel[2] * t->mat.mat3.uvs[2][2] > 0.0f)
            YawUV(3.1415927f, t->mat.mat3.uvs);
        t->Flags &= ~6;
        AddToEnvList(v);
        PlayTargetSound(targetInfo[t->infoIdx].startSnd, t->slot, t->mat.mat3.pos, 2);
    }
}

/* 0x8004e6bc, size 0x224 */
static void _StartCone(tTarget *t)
{
    tVisual *v; /* r29 */
    tUVMatrix tmat; /* stack */
    float *vel; /* r28 */
    float *angV; /* r0 */
    tCarData *car; /* r27 */
    tTargetInfo *tInfo; /* r28 */
    tInfo = &targetInfo[t->infoIdx];
    car = &gGameCar[t->slot];
    if (gCarMines && tInfo->objListIdx == 550) {
        car->carMine = 1;
        MBOX_HideObject(t->dlIdx, 0, 15);
        t->Flags &= ~6;
        return;
    }
    if (gMineCones && tInfo->objListIdx == 550) {
        MBOX_HideObject(t->dlIdx, 0, 15);
        StartBlast(t->pos, 0, 0.4f, 1);
        if (!model[t->slot].crashflag) model[t->slot].crashflag = 1;
        t->Flags &= ~6;
        return;
    }
    v = GrabEnvEntry();
    if (v) {
        v->index = 0;
        v->func = tInfo->animFunc;
        v->data = (unsigned int)t;
        v->timeStamp = 5.0f;
        t->Flags &= ~6;
        angV = ((tTargetStructVelRot *)t->data)->angV;
        vel = ((tTargetStructVelRot *)t->data)->vel;
        vel[0] = 0.125f * car->dr_vel[0];
        vel[1] = 0.125f * car->dr_vel[1];
        vel[2] = 0.125f * car->dr_vel[2];
        LookInDir(vel, &tmat);
        if (t->type == 653 || t->type == 551) {
            vel[1] += 1.0f;
            angV[0] = 0.0f;
            angV[1] = 12.0f;
            angV[2] = 15.0f;
        } else {
            vel[1] += 2.0f;
            angV[0] = 0.0f;
            angV[1] = 12.0f;
            angV[2] = 15.0f;
        }
        CopyUV(tmat.mat3.uvs, t->mat.mat3.uvs);
        AddToEnvList(v);
        PlayTargetSound(targetInfo[t->infoIdx].startSnd, t->slot, t->mat.mat3.pos, 2);
    }
}

/* 0x8004e8e0, size 0x148 */
static void _StartCoin(tTarget *t)
{
    tTargetStructCoin *c; /* r30 */
    t->Flags &= ~4;
    c = t->data;
    if (gGameCar[t->slot].camera >= 0 && !IsCoinFound(t->slot, c->whichcoin)) {
        FoundCoin(t->slot, c->whichcoin);
        MBOX_HideObject(t->dlIdx, 1, 1 << gGameCar[t->slot].camera);
        MBOX_HideObject(c->dlIdx, 1, 1 << gGameCar[t->slot].camera);
        c->cameras &= ~(1 << gGameCar[t->slot].camera);
        if (!c->cameras) t->Flags |= 0x80;
        PlayTargetSound(targetInfo[t->infoIdx].startSnd, t->slot, t->mat.mat3.pos, 2);
    }
}

/* 0x8004ea28, size 0x318 */
static void _AnimateCoin(tVisual *v, signed short op)
{
    tTarget *t; /* r30 */
    float *angV; /* r0 */
    float angVF[3]; /* stack */
    tTargetStructCoin *c; /* r29 */
    int i; /* r26 */
    int p; /* r25 */
    int coinNum; /* r28 */
    if (!op) {
        ReleaseEnviron(v, 1);
        return;
    }
    if (gPauseState != kNoPause) return;
    t = (tTarget *)v->data;
    c = t->data;
    if (t->Flags & 0x80) {
        ZOID_RemoveObject(c->dlIdx);
        ZOID_RemoveObject(t->dlIdx);
        FreeStatLLE(&gTargetList, t);
        ReleaseEnviron(v, 1);
        return;
    }
    if (!(t->Flags & 1)) {
        tRGBA gClr = { 0x00, 0xC8, 0xFF, 0xFF };
        tRGBA sClr = { 0xFF, 0xE6, 0xC8, 0xFF };
        if (t->Flags & 0x40) {
            c->whichcoin = coinNum = gWhichGoldCoin + 8;
            gWhichGoldCoin++;
        } else {
            coinNum = gWhichSilverCoin;
            c->whichcoin = coinNum;
            gWhichSilverCoin++;
        }
        ZOID_SetObjectDef(t->dlIdx, MBOX_FindObject_WORK("COIN_GLOWL3_LOD", 0, gNumModels - 1, 1));
        c->angV[0] = 0.0f;
        c->angV[1] = 3.0f;
        c->angV[2] = 0.0f;
        CopyUV(unitmatrix, c->mat.mat3.uvs);
        c->mat.mat3.pos[0] = t->mat.mat3.pos[0];
        c->mat.mat3.pos[1] = t->mat.mat3.pos[1];
        c->mat.mat3.pos[2] = t->mat.mat3.pos[2];
        if (t->Flags & 0x40) {
            c->dlIdx = MBOX_NewObject(MBOX_FindObject_WORK("GOLDCOINL4_LOD1", 0, gNumModels - 1, 1), &c->mat, -1, 0x40000);
            ZOID_SetColor(t->dlIdx, &gClr);
        } else {
            c->dlIdx = MBOX_NewObject(MBOX_FindObject_WORK("SILVERCOINL4_LO", 0, gNumModels - 1, 1), &c->mat, -1, 0x40000);
            ZOID_SetColor(t->dlIdx, &sClr);
        }
        c->cameras = 0;
        p = 1;
        for (i = 0; i < gNumPlayers; i++) {
            if (!IsCoinFound(i, coinNum)) {
                c->cameras |= 1 << i;
                p = 0;
            } else {
                MBOX_HideObject(t->dlIdx, 1, 1 << gGameCar[i].camera);
                MBOX_HideObject(c->dlIdx, 1, 1 << gGameCar[i].camera);
            }
        }
        if (p) {
            ZOID_RemoveObject(c->dlIdx);
            ZOID_RemoveObject(t->dlIdx);
            FreeStatLLE(&gTargetList, t);
            ReleaseEnviron(v, 1);
            return;
        }
        t->Flags |= 1;
    }
    angV = ((tTargetStructCoin *)t->data)->angV;
    angVF[0] = angV[0] * gFrameSeconds;
    angVF[1] = angV[1] * gFrameSeconds;
    angVF[2] = angV[2] * gFrameSeconds;
    RotateUV(angVF, c->mat.mat3.uvs);
}

/* 0x8004ed40, size 0x178 */
static void _AnimateVelRotGeneric(tVisual *v, signed short op)
{
    static const float gravity[3] = { 0.0f, -0.25f, 0.0f };
    tTarget *t; /* r31 */
    float *vel; /* r0 */
    float *angV; /* r0 */
    float angVF[3]; /* stack */
    if (!op) {
        ReleaseEnviron(v, 1);
    } else if (gPauseState == kNoPause) {
        t = (tTarget *)v->data;
        vel = ((tTargetStructVelRot *)t->data)->vel;
        angV = ((tTargetStructVelRot *)t->data)->angV;
        vel[0] += 0.15f * gravity[0];
        t->mat.mat3.pos[0] += 0.15f * gravity[0] + vel[0];
        vel[1] += 0.15f * gravity[1];
        t->mat.mat3.pos[1] += 0.15f * gravity[1] + vel[1];
        vel[2] += 0.15f * gravity[2];
        t->mat.mat3.pos[2] += 0.15f * gravity[2] + vel[2];
        angVF[0] = angV[0] * gFrameSeconds;
        angVF[1] = angV[1] * gFrameSeconds;
        angVF[2] = angV[2] * gFrameSeconds;
        RotateUV(angVF, t->mat.mat3.uvs);
        v->timeStamp -= gFrameSeconds;
        if (v->timeStamp <= 0.0f) {
            if (TInfo(t)->Flags & 0x2000) ZOID_RemoveObject(t->dlIdx);
            FreeStatLLE(&gTargetList, t);
            ReleaseEnviron(v, 1);
        }
    }
}

/* 0x8004eeb8, size 0x80 */
static void _AnimateRotGeneric(tVisual *v, signed short op)
{
    tTarget *t; /* r4 */
    float *angV; /* r5 */
    float angVF[3]; /* stack */
    if (!op) {
        ReleaseEnviron(v, 1);
    } else if (gPauseState == kNoPause) {
        t = (tTarget *)v->data;
        angV = t->data;
        angVF[0] = angV[0] * gFrameSeconds;
        angVF[1] = angV[1] * gFrameSeconds;
        angVF[2] = angV[2] * gFrameSeconds;
        RotateUV(angVF, t->mat.mat3.uvs);
    }
}

/* 0x8004ef38, size 0x118 */
static void _AnimateObjSwapGeneric(tVisual *v, signed short op)
{
    tTarget *t; /* r31 */
    signed short type; /* r30 */
    if (!op) {
    release:
        ReleaseEnviron(v, 1);
        return;
    }
    if (gPauseState != kNoPause) return;
    t = (tTarget *)v->data;
    v->timeStamp -= gFrameSeconds;
    if (v->timeStamp > 0.0f) return;
    v->timeStamp = 0.0625f;
    v->index++;
    if (v->index >= t->frames) {
        if (TInfo(t)->Flags & 0x1000) goto release;
        if (TInfo(t)->Flags & 0x2000) {
            ZOID_RemoveObject(t->dlIdx);
            goto release;
        }
        v->index = 0;
    }
    if (t->type != (type = t->otype + v->index)) {
        ZOID_SetObjectDef(t->dlIdx, gObjList[type]);
        t->type = type;
    }
}

/* 0x8004f050, size 0xe4 */
static void _StartKnockdown(tTarget *t)
{
    tVisual *v; /* r31 */
    tUVMatrix mat; /* stack */
    v = GrabEnvEntry();
    if (v) {
        v->index = 0;
        v->func = targetInfo[t->infoIdx].animFunc;
        v->data = (unsigned int)t;
        v->timeStamp = 0.0333333f;
        t->frames = 8;
        t->Flags &= ~6;
        LookInDir(gGameCar[t->slot].dr_vel, &mat);
        CopyUV(mat.mat3.uvs, t->mat.mat3.uvs);
        AddToEnvList(v);
        PlayTargetSound(targetInfo[t->infoIdx].startSnd, t->slot, t->mat.mat3.pos, 2);
    }
}

/* 0x8004f134, size 0x88 */
static void _AnimateKnockdown(tVisual *v, signed short op)
{
    tTarget *t; /* r31 */
    float *angV; /* r3 */
    if (!op) ReleaseEnviron(v, 1);
    if (gPauseState != kNoPause) return;
    t = (tTarget *)v->data;
    angV = t->data;
    if (t->frames) {
        t->frames--;
        RotateUV(angV, t->mat.mat3.uvs);
    }
    if (!t->frames) ReleaseEnviron(v, 1);
}

