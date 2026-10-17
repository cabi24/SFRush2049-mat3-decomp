/* trackscr.c - private partial track-selection-screen reconstruction. Not link verified.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\trackscr.c; assembly: build/2049LE/asm/GameSource/src/trackscr.s */
#include "rush/rush_protos.h"

static void _InitScreen(void);
static void _UninitScreen(void);
static void showTrackSelect(void);
static int animNewSlider(_tBlit_2 *blt);
static int animArrows(_tBlit_2 *blt);
static int AnimZ(_tBlit_2 *blt);
static int animFan(_tBlit_2 *blt);
static int animText(void);
static void animButtonText(void);
static void drawOptions(void);
static int TrackValid(int track);
static int OptionValid(int opt);
static void SetFogIntensity(int now);
static void BlowFog(void);
static void setCamera(void);
static void loadTrackScreen(void);
static void unloadTrackScreen(void);
static void buildTrackMap(void);
static void clearTrackMap(void);
static void animTrackMap(void);
static void buildTrackMaps(void);
static void buildRaceTrackMaps(void);
static void buildStuntTrackMaps(void);
static void buildBattleTrackMaps(void);
static void buildObstacleTrackMaps(void);
static void clearTrackMaps(void);
static void animTrackMaps(void);
static void loadPathData(void);
static void unloadPathData(void);
static void initButtons(void);
static void uninitButtons(void);
static void updateButtons(void);

static char *buttonNames[9] = {
    "BUTTON_S1G1", "BUTTON_S1G1", "BUTTON_S2G1", "CURSORG1", "AB_BUTTONG1", "ARROW_BGG1",
    "ARROW_FGG1", "BAR_BGG1", "BAR_FGG1"
};
static anon_buttonType_objIdx_40 buttonInfo[48] = {
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 0, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 1, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 2, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 3, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 4, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 6, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 4, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 },
    { 6, -1, 0.0f, {{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }}, 0 }
};
static char *trackMapObjNames[20] = {
    "TRK_T1G1", "TRK_T2G1", "TRK_T3G1", "TRK_T4G1", "TRK_T5G1", "TRK_T6G1", "TRK_D1G1", "TRK_D2G1",
    "TRK_D3G1", "TRK_D4G1", "TRK_D5G1", "TRK_D6G1", "TRK_D7G1", "TRK_D8G1", "TRK_S1G1", "TRK_S2G1",
    "TRK_S3G1", "TRK_S4G1", "TRK_O1G1", "TRK_SELG1"
};
static char *trackScreenNames[19] = {
    "TPIC1", "TPIC2", "TPIC3", "TPIC4", "TPIC5", "TPIC6", "DPIC1", "DPIC2", "DPIC3", "DPIC4",
    "DPIC5", "DPIC6", "DPIC7", "DPIC8", "SPIC1", "SPIC2", "SPIC3", "SPIC4", "OPIC1"
};
static int validOption[7][21] = {
    1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0
};
float yFog[19] = {
    15.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f,
    15.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f
};
static anon_vp_20 vp = { 1280.0f, 896.0f, 0.0f, 0.0f, 1280.0f, 896.0f, 0.0f, 0.0f };
static float cameraPos[3] = { 0.0f, 3300.0f, -6900.0f };
anon_objIdx_cityX_48 trackMap[20] = {
    -1, 6.1861f, 54.2055f, -0.182355f, 0.846221f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -18.4636f, 11.3307f, -0.246364f, 0.957244f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -19.0545f, 21.7559f,
    -0.235454f, 0.924175f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, -1, 12.1082f, 47.8031f, -0.189593f, 0.87063f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -11.2441f, -11.6614f, 1.4315f, 1.26748f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -29.026f, 61.6535f,
    -0.303568f, 0.661969f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, -1, 37.4f, -12.92f, 0.735f, 0.995f, 1.99f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -53.09f, 37.34f, 3.145f, 0.52f, 1.04f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -41.8f, -51.6f, -0.155f, 0.761f,
    1.522f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -19.0f,
    18.72f, -2.361f, 1.013f, 2.026f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, -1, 15.34f, 12.23f, -1.476f, 0.795f, 1.59f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -46.95f, 2.35f, -4.811f, 0.631f, 1.262f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, 8.23f, 45.59f, 1.302f, 0.744f,
    1.488f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, 5.21f,
    -19.78f, -1.725f, 1.068f, 2.136f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, -1, 27.07f, -4.02f, -0.79f, 0.84f, 1.68f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, 9.76f, 53.25f, 2.69f, 0.29f, 0.58f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -16.96f, -44.02f, 1.48f, 0.69f, 1.38f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -48.5f, 11.6f,
    0.094f, 0.554f, 1.108f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    -1, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, -1, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f
};
anon_c_rgba_4 trackColors[19] = {
    {{ 240, 0, 0, 255 }},
    {{ 0, 240, 0, 255 }},
    {{ 240, 240, 0, 255 }},
    {{ 0, 0, 240, 255 }},
    {{ 240, 0, 240, 255 }},
    {{ 0, 240, 240, 255 }},
    {{ 240, 0, 0, 255 }},
    {{ 240, 127, 0, 255 }},
    {{ 240, 240, 0, 255 }},
    {{ 127, 240, 0, 255 }},
    {{ 0, 240, 0, 255 }},
    {{ 0, 240, 127, 255 }},
    {{ 0, 240, 240, 255 }},
    {{ 240, 0, 240, 255 }},
    {{ 240, 240, 0, 255 }},
    {{ 240, 0, 0, 255 }},
    {{ 240, 240, 0, 255 }},
    {{ 0, 240, 0, 255 }},
    {{ 240, 240, 240, 255 }}
};
static anon_numPack_name_2c mapObj[7] = { 0 };
static float gButtonScale = 0.85f;
static float gButtonSpace = 40.0f;
static float buttonX = -55.0f;
static float maxY = -109.0f;
static float minY = -169.0f;
static float buttonZ = 300.0f;
static float cursorZ = 225.0f;
int kNumMapSects = 100;
static int pathWaveIdx = -1;
int trackMapObj = -1;
static signed short oldTrack;
static signed short gNumTracks;
static signed short gTrack;
static signed short gActiveOption;
static signed short gFirstOption;
static signed short gNumOptions;
static signed char gActiveOptionEnabled;
static signed char gActiveZ;
static signed char trackMapsBuilt;
int currentTrackMap;
int desiredTrackMap;
float mapTransition;
static anon_name_radius_48 *selTrackRomObjDef;
anon_nverts_flags_68 *trackFogPoly;
anon_nverts_flags_68 *trackScreenPoly;
static float pathPos;
static signed char inited;
static _tBlit_2 *blit;
static int buttonsJustInited;
static int buttonsInited;
static float gCameraMatrix[4][4];
anon_mat3_mat4_30 trackMapMat;
static anon_gTrackNum_pos_18 gTracks[19];
static signed short gOpts[21];

static const anon_texname_dulx_24 mblit[18] = {
    "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe, (void *)&animNewSlider, 0xe0000, "SLIDER", 0,
    0, -1, -1, -1, -1, -1, -1, 0, 0xfe, (void *)&animNewSlider, 0xe0001, "SLIDER", 0, 0, -1, -1,
    -1, -1, -1, -1, 0, 0xfe, (void *)&animNewSlider, 0xf0000, "SLIDER", 0, 0, -1, -1, -1, -1, -1,
    -1, 0, 0xfe, (void *)&animNewSlider, 0xf0001, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0x120000, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0x120001, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0x130000, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0x130001, "ARROW_SML_LR_BG", 0, 0, -1, -1, -1, -1, -1, -1, 0x11, 0xff,
    (void *)&animArrows, 0, "ARROW_SML_LR_FG", 0, 0, -1, -1, -1, -1, -1, -1, 0x10, 0xfe,
    (void *)&animArrows, 0x1001, "ARROW_SML_LR_BG", 0, 0, -1, -1, -1, -1, -1, -1, 0x11, 0xff,
    (void *)&animArrows, 0x2010, "ARROW_SML_LR_FG", 0, 0, -1, -1, -1, -1, -1, -1, 0x10, 0xfe,
    (void *)&animArrows, 0x3011, "ARROW_SML_UD_BG", 0, 0, -1, -1, -1, -1, -1, -1, 0x11, 0xff,
    (void *)&animArrows, 0x4020, "ARROW_SML_UD_FG", 0, 0, -1, -1, -1, -1, -1, -1, 0x10, 0xfe,
    (void *)&animArrows, 0x5021, "ARROW_SML_UD_BG", 0, 0, -1, -1, -1, -1, -1, -1, 0x11, 0xff,
    (void *)&animArrows, 0x6030, "ARROW_SML_UD_FG", 0, 0, -1, -1, -1, -1, -1, -1, 0x10, 0xfe,
    (void *)&animArrows, 0x7031, "RUSHFLAGS", 150, 82, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animFan, 0x6400, (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0,
    (void *)&animText, 0
};

/* 0x80087abc, size 0xcf0 */
void updateTrackScreen(void)
{
    /* TODO: original body not recovered; research candidate excluded. */
}

/* 0x800887ac, size 0x4fc */
static void _InitScreen(void)
{
    static float fogPos[3];
    /* TODO: original body not recovered; research candidate excluded. */
}

/* 0x80088ca8, size 0x5c */
static void _UninitScreen(void)
{
    if (inited) {
        uninitButtons();
        if (blit) { RemoveBlit((_tBlit *)blit); blit = 0; }
        removeBG();
        clearTrackMap();
        clearTrackMaps();
        ZOID_Reset();
        inited = 0;
    }
}

/* 0x80088d04, size 0x314 */
static void showTrackSelect(void)
{
    static float angle;
    static float fogPos[3];
    /* TODO: body excluded until original static layout is recovered. */
}

/* 0x80089018, size 0x310 */
static int animNewSlider(_tBlit_2 *blt)
{
    int type;
    int opt;
    signed short screenPos[2];
    type = blt->animID & 15;
    opt = blt->animID >> 16;
    if (Hidden(blt,(buttonInfo[opt+21].angle > 0.6981316804885864f && buttonInfo[opt+21].angle < 2.4434609413146973f) || buttonInfo[opt+21].alpha == 0)) return 1;
    ZOID_ProjectWindow(0,buttonInfo[opt+21].mat.mat3.pos,&gCamera[0].mat,0,screenPos);
    blt->x = (screenPos[0] - blt->width/2) / 2;
    blt->y = screenPos[1] / 2.0f - (blt->height/4/2) / 2.0f;
    blt->top = 0;
    blt->bot = blt->height/4 - 1;
    blt->left = 0;
    blt->right = blt->width-1;
    blt->alpha = buttonInfo[opt+21].alpha;
    if (blt->alpha == 255) blt->alpha = 254;
    if (buttonInfo[opt+21].angle > 1.5707963705062866f) {
        blt->top += blt->height/2;
        blt->bot += blt->height/2;
    }
    if (type == 1) {
        blt->top += blt->height/4;
        blt->bot += blt->height/4;
        switch (opt) {
        case 14: blt->right = (blt->width-1)*gFogIntensity/3; break;
        case 15: blt->right = (blt->width-1)*gWind/4; break;
        case 18: blt->right = (blt->width-1)*gDifficulty/5; break;
        case 19: blt->right = (blt->width-1)*gCatchup/2; break;
        }
    }
    UpdateBlit((_tBlit *)blt);
    return 1;
}

/* 0x80089328, size 0x2dc */
static int animArrows(_tBlit_2 *blt)
{
    static const signed short arrowPos[2][4][2] = { 94, 181, 106, 181, 106, 181, 106, 220, 16, 155, 304, 155, 160, 139, 160, 171 };
    static char *arrowNames[16][2] = { "BAR_SML_LR_BG", "ARROW_SML_LR_BG", "BAR_SML_LR_FG", "ARROW_SML_LR_FG", "BAR_SML_LR_BG", "ARROW_SML_LR_BG", "BAR_SML_LR_FG", "ARROW_SML_LR_FG", "BAR_SML_UD_BG", "ARROW_SML_UD_BG", "BAR_SML_UD_FG", "ARROW_SML_UD_FG", "BAR_SML_UD_BG", "ARROW_SML_UD_BG", "BAR_SML_UD_FG", "ARROW_SML_UD_FG", "BAR_LRG_LR_BG", "ARROW_LRG_LR_BG", "BAR_LRG_LR_FG", "ARROW_LRG_LR_FG", "BAR_LRG_LR_BG", "ARROW_LRG_LR_BG", "BAR_LRG_LR_FG", "ARROW_LRG_LR_FG", "BAR_LRG_UD_BG", "ARROW_LRG_UD_BG", "BAR_LRG_UD_FG", "ARROW_LRG_UD_FG", "BAR_LRG_UD_BG", "ARROW_LRG_UD_BG", "BAR_LRG_UD_FG", "ARROW_LRG_UD_FG" };
    signed short fg, dir, size, tex;
    fg = blt->animID & 15;
    dir = (blt->animID >> 4) & 15;
    size = (blt->animID >> 8) & 15;
    tex = (blt->animID >> 12) & 15;
    if (Hidden(blt,buttonsInited == 1)) return 1;
    if (size == 1 && Hidden(blt,dir == 2)) return 1;
    if (!size) {
        if (dir == 2) { if (Hidden(blt,gFirstOption == 0)) return 1; }
        else if (dir == 3) { if (Hidden(blt,gNumOptions-gFirstOption <= 4)) return 1; }
    }
    RenameBlit((_tBlit *)blt,arrowNames[tex][(signed short)(gActiveOptionEnabled || dir >= 2)],1);
    if (fg) blt->alpha = TB_GetFlashTextCycle() * 255.0f;
    blt->x = arrowPos[size][dir][0] - (blt->width/2)/2;
    blt->y = arrowPos[size][dir][1] - blt->height / 2.0f / 2.0f;
    if (!size) {
        HAL_SetFont(11);
        if (!dir) blt->x -= HAL_GetTextWidth(gText.taba[gText.tab[1]+gOpts[gActiveOption]],-1);
        if (!dir || dir == 1) blt->y += (gActiveOption-gFirstOption) * 13;
    }
    if (dir == 1) blt->flip = 1;
    else if (dir == 3) gBlitInstanceList[blt->blidx].flags |= 8;
    UpdateBlit((_tBlit *)blt);
    return 1;
}

/* 0x80089604, size 0x230 */
static int AnimZ(_tBlit_2 *blt)
{
    gActiveZ = 0;
    if (!gActiveOptionEnabled) { Hidden(blt,1); return 1; }
    switch (gOpts[gActiveOption]) {
    case 0: Hidden(blt,1); return 1;
    case 16: Hidden(blt,gOptions[23] == 1); break;
    case 12: Hidden(blt,gOptions[27] == 0); break;
    case 13: Hidden(blt,gOptions[28] == 0); break;
    case 14: Hidden(blt,gOptions[29] == 0); break;
    case 15: Hidden(blt,gOptions[30] == 0); break;
    case 1: Hidden(blt,gOptions[25] == 0); break;
    case 2: Hidden(blt,gOptions[21] == 3); break;
    case 17: Hidden(blt,gOptions[22] == 5); break;
    case 18: Hidden(blt,gOptions[24] == 2); break;
    case 19: Hidden(blt,gOptions[26] == 2); break;
    case 20: Hidden(blt,gOptions[31] == 0); break;
    }
    blt->alpha = TB_GetFlashTextCycle() * 255.0f;
    UpdateBlit((_tBlit *)blt);
    gActiveZ = 1;
    return 1;
}

/* 0x80089834, size 0x180 */
static int animFan(_tBlit_2 *blt)
{
    int frame;
    frame = (blt->animID >> 8) & 0xfff;
    if (Hidden(blt, !OptionValid(15))) return 1;
    blt->top = gWind * (blt->width / 16);
    blt->bot = blt->top + blt->width / 16 - 1;
    frame += gFrameSeconds * (gWind == 2 ? 4500.0f : 3000.0f);
    frame %= 800;
    blt->left = (frame / 100) * (blt->width / 8);
    blt->right = blt->left + blt->width / 8 - 1;
    blt->animID = (blt->animID & ~0xfff00) | (frame << 8);
    UpdateBlit((_tBlit *)blt);
    { anon_palette_texture_20 *inst = &gBlitInstanceList[blt->blidx]; inst->flags |= 1; }
    return 1;
}

/* 0x800899b4, size 0xb4 */
static int animText(void)
{
    int i;
    anon_ll_fileh_4c **player;
    for (i = 0; i < gNumPlayers; i++) {
        if (gPlayers[i].save != &gJustPlayers[i]) {
            player = gPlayerFiles.head;
            while (player) {
                if (gPlayers[i].save == player) break;
                player = (*player)->ll.next;
            }
            if (!player) return 1;
        }
    }
    if (buttonsInited == 1) animButtonText();
    else drawOptions();
    return 1;
}

/* 0x80089a68, size 0x7dc */
static void animButtonText(void)
{
    int i;
    int x;
    int y;
    unsigned char str1[64];
    unsigned char *str;
    signed short screenPos[2];
    int item;
    HAL_SetTextDepth(0.0f);
    TB_SetJust(0,1);
    ZOID_ProjectWindow(0,buttonInfo[42].mat.mat3.pos,&gCamera[0].mat,0,screenPos);
    x = screenPos[0]/2;
    y = screenPos[1]/2.0f;
    HAL_SetFont(13);
    TB_SetTextColor(1);
    TB_Sprintf(str1,gText.single[45]);
    TB_DrawText(TB_CenterText(str1,x),y,str1);
    HAL_SetFont(11);
    for (i = 0; i < gNumOptions; i++) {
        item = gOpts[i];
        if ((buttonInfo[item].angle <= 0.6981316804885864f || buttonInfo[item].angle >= 2.4434609413146973f) && buttonInfo[item].alpha) {
            HAL_SetTextTranslucency(buttonInfo[item].alpha);
            ZOID_ProjectWindow(0,buttonInfo[item].mat.mat3.pos,&gCamera[0].mat,0,screenPos);
            x = screenPos[0]/2 - 35.0f*gButtonScale;
            y = screenPos[1]/2.0f;
            if (buttonInfo[item].angle < 1.5707963705062866f) {
                anon_c_rgba_4 c1,c2; c2=flashColors[1]; c1=flashColors[0];
                TB_FlashTextColors(&c1,&c2); TB_SetTextColor(1);
            } else {
                anon_c_rgba_4 c1,c2; c2=flashColors[1]; c1=flashColors[0];
                TB_FlashTextColors(&c1,&c2); TB_SetTextColor(22);
            }
            TB_DrawText(x,y,gText.taba[gText.tab[1]+gOpts[i]]);
        }
        if ((buttonInfo[item+21].angle <= 0.6981316804885864f || buttonInfo[item+21].angle >= 2.4434609413146973f) && buttonInfo[item+21].alpha) {
            HAL_SetTextTranslucency(buttonInfo[item+21].alpha);
            ZOID_ProjectWindow(0,buttonInfo[item+21].mat.mat3.pos,&gCamera[0].mat,0,screenPos);
            x = screenPos[0]/2 - 35.0f*gButtonScale;
            y = screenPos[1]/2.0f;
            if (buttonInfo[item+21].angle < 1.5707963705062866f) {
                anon_c_rgba_4 c1,c2; c2=flashColors[1]; c1=flashColors[0];
                TB_FlashTextColors(&c1,&c2); TB_SetTextColor(1);
            } else {
                anon_c_rgba_4 c1,c2; c2=flashColors[1]; c1=flashColors[0];
                TB_FlashTextColors(&c1,&c2); TB_SetTextColor(22);
            }
            switch (item) {
            case 0: TB_DrawText(x,y,gTrackTitles[gTrackNum]); break;
            case 1: TB_DrawText(x,y,gText.taba[gText.tab[36]+gGhostSelect]); break;
            case 2: sprintf((char *)str1,"%d",gNumLaps); TB_DrawText(x,y,str1); break;
            case 3:
                if (gOptions[32] == 0) str = gText.single[219];
                else if (gOptions[32] == 1) str = gText.single[140];
                else str = gText.single[12];
                TB_DrawText(x,y,str); break;
            case 4: TB_DrawText(x,y,gText.taba[gText.tab[37]+(gOptions[33] != 0)]); break;
            case 5: sprintf((char *)str1,"%d",gOptions[34]); TB_DrawText(x,y,str1); break;
            case 6: sprintf((char *)str1,"%d",gOptions[35]); TB_DrawText(x,y,str1); break;
            case 7: sprintf((char *)str1,"%d",gOptions[36]); TB_DrawText(x,y,str1); break;
            case 8: TB_DrawText(x,y,gOptions[37] == 1 ? gText.single[219] : gText.single[140]); break;
            case 9: sprintf((char *)str1,"%d",gOptions[38]); TB_DrawText(x,y,str1); break;
            case 10: sprintf((char *)str1,"%d",gOptions[39]); TB_DrawText(x,y,str1); break;
            case 11: TB_DrawText(x,y,gText.taba[gText.tab[17]+(gOptions[40] != 0)]); break;
            case 12: TB_DrawText(x,y,gText.taba[gText.tab[17]+(gBackwards != 0)]); break;
            case 13:
                if (gRace == kCircuit) TB_DrawText(x,y,gText.taba[gText.tab[17]+(gCircuitTracks[gCircuitTrack].mirror != 0)]);
                else if (gOptions[28] == 2) TB_DrawText(x,y,gText.taba[gText.tab[19]+2]);
                else TB_DrawText(x,y,gText.taba[gText.tab[17]+(gOptions[28] != 0)]);
                break;
            case 17: sprintf((char *)str1,"%d",gNumDrones); TB_DrawText(x,y,str1); break;
            case 20: TB_DrawText(x,y,gText.taba[gText.tab[17]+(gDeathMode != 0)]); break;
            }
        }
    }
    TB_SetJust(0,3);
    HAL_SetTextTranslucency(-1.0f);
    HAL_SetTextDepth(-1.0f);
}

/* 0x8008a244, size 0x4 */
static void drawOptions(void)
{}

/* 0x8008a248, size 0xdc */
static int TrackValid(int track)
{
    static signed char trackReady[19] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    if (!trackReady[track]) return 0;
    if (!IsTrackUnlocked(track)) return 0;
    if (gRace == kStunt) return track >= 14 && track <= 17;
    if (gRace == kObstacle) return track >= 18 && track <= 18;
    if (gRace == kBattle) return track >= 6 && track <= 13;
    return track >= 0 && track <= 5;
}

/* 0x8008a324, size 0x170 */
static int OptionValid(int opt)
{
    int valid;
    valid = validOption[gRace][opt];
    if (gRace == kOneRace && opt == 17) valid = gNumPlayers < 3;
    if (gRace == kStunt) {
        if (opt == 5) valid = gOptions[32] == 1;
        if (opt == 6) valid = gOptions[32] == 0;
        if (opt == 7) valid = gOptions[32] == 2;
    }
    if (gRace == kBattle) {
        if (opt == 9) valid = gOptions[37] == 0;
        if (opt == 11) valid = gOptions[37] == 0;
        if (opt == 10) valid = gOptions[37] == 1;
    }
    if (opt == 19 && gNumPlayers == 1) valid = 0;
    if (opt == 12) valid = gTrackNum >= 0 && gTrackNum <= 5;
    return valid;
}

/* 0x8008a494, size 0x130 */
static void SetFogIntensity(int now)
{
    static anon_c_rgba_4 fogColor[2];
    int red, green, blue;
    if (gFogColor < 19) gFogColor = gTrackNum;
    red = gFogColors[gFogColor][0];
    green = gFogColors[gFogColor][1];
    blue = gFogColors[gFogColor][2];
    if (now) {
        fogColor[0].c[0] = red;
        fogColor[0].c[1] = green;
        fogColor[0].c[2] = blue;
    } else {
        fogColor[0].c[0] += (red - fogColor[0].c[0]) / 10;
        fogColor[0].c[1] += (green - fogColor[0].c[1]) / 10;
        fogColor[0].c[2] += (blue - fogColor[0].c[2]) / 10;
    }
    fogColor[0].c[3] = gFogIntensity * 192 / 3;
    if (trackFogPoly) ZOID_UpdatePoly(trackFogPoly, 0, 0, 0, fogColor, 0, 0);
}

/* 0x8008a5c4, size 0x48 */
static void BlowFog(void)
{
    static anon_uls_ult_8 fogTileSize;
    anon_uls_ult_8 *tile = &fogTileSize;
    int s;
    int speed;
    speed = gWind ? gWind * 5 : 1;
    s = tile->uls + speed;
    if (s < 0) s += 2048;
    if (s >= 2048) s -= 2048;
    tile->uls = s;
    tile->ult = 0;
}

/* 0x8008a60c, size 0x28 */
static void setCamera(void)
{
    ZeroCamera(1);
    UpdateCameras();
}

/* 0x8008a634, size 0x1a4 */
static void loadTrackScreen(void)
{
    float polyVerts[4][3];
    unsigned short texIdx;
    anon_c_rgba_4 backColor = {{255,255,255,255}};
    unloadTrackScreen();
    if (!((gTrackNum >= 0 && gTrackNum <= 5) ||
        (gTrackNum >= 6 && gTrackNum <= 13) ||
        (gTrackNum >= 14 && gTrackNum <= 17) ||
        (gTrackNum >= 18 && gTrackNum <= 18))) return;
    {
        polyVerts[0][0] = 320.0f;
        polyVerts[0][1] = 180.0f;
        polyVerts[0][2] = 400.0f;
        polyVerts[1][0] = 80.0f;
        polyVerts[1][1] = 180.0f;
        polyVerts[1][2] = 400.0f;
        polyVerts[2][0] = 80.0f;
        polyVerts[2][1] = -60.000003814697266f;
        polyVerts[2][2] = 400.0f;
        polyVerts[3][0] = 320.0f;
        polyVerts[3][1] = -60.000003814697266f;
        polyVerts[3][2] = 400.0f;
        MBOX_FindTexture_WORK(trackScreenNames[gTrackNum], &texIdx, 0, gNumModels-1, 1);
        trackScreenPoly = ZOID_NewPoly(4,polyVerts,texIdx,&backColor,0x201,1);
        trackScreenPoly->flags |= 0x40;
        ZOID_ShowPoly(trackScreenPoly);
        if (OptionValid(14) == 1) {
            polyVerts[0][2] = 399.0f;
polyVerts[1][2] = 399.0f;
polyVerts[2][2] = 399.0f;
polyVerts[3][2] = 399.0f;
            MBOX_FindTexture_WORK("FOG",&texIdx,0,gNumModels-1,1);
            backColor.c[3] = 128;
            trackFogPoly = ZOID_NewPoly(4,polyVerts,texIdx,&backColor,0x201,1);
            trackFogPoly->flags |= 0x40;
            ZOID_ShowPoly(trackFogPoly);
        }
    }
}

/* 0x8008a7d8, size 0x4c */
static void unloadTrackScreen(void)
{
    if (trackScreenPoly) { ZOID_RemovePoly(trackScreenPoly); trackScreenPoly = 0; }
    if (trackFogPoly) { ZOID_RemovePoly(trackFogPoly); trackFogPoly = 0; }
}

/* 0x8008a824, size 0x18c */
static void buildTrackMap(void)
{
    static float mapPos[3] = {50.0f,-15.0f,100.0f};
    /* TODO: body excluded until original static layout is recovered. */
}

/* 0x8008a9b0, size 0x3c */
static void clearTrackMap(void)
{
    unloadTrackScreen();
    if (trackMapObj != -1) { ZOID_RemoveObject(trackMapObj); trackMapObj = -1; }
}

/* 0x8008a9ec, size 0x324 */
static void animTrackMap(void)
{
    static float cityPos[3] = {-100.0f,30.0f,200.0f};
    static float mapPos[3] = {50.0f,-15.0f,100.0f};
    static float rotAngle;
    /* TODO: body excluded until original static layout is recovered. */
}

/* 0x8008ad10, size 0x64 */
static void buildTrackMaps(void)
{
    if (gRace == kStunt) buildStuntTrackMaps();
    else if (gRace == kBattle) buildBattleTrackMaps();
    else if (gRace == kObstacle) buildObstacleTrackMaps();
    else buildRaceTrackMaps();
    trackMapsBuilt = 1;
    pathPos = 0.0f;
}

/* 0x8008ad74, size 0xf2c */
static void buildRaceTrackMaps(void)
{
    static int posIdxs[10] = { 0x0, 0x4, 0x1, 0x5, 0x2, 0x6, 0x3, 0x7, 0x0, 0x4 };
    static int colIdxs[10] = { 0x0, 0x1, 0x0, 0x1, 0x2, 0x3, 0x2, 0x3, 0x0, 0x1 };
    static unsigned int vertFlags[10] = { 0x0, 0x8000000, 0xc000000, 0xc000000, 0x8000000, 0x8000000, 0xc000000, 0xc000000, 0x1c000000, 0x3c000000 };
    static float mapPos[3] = { -100.0f, 30.0f, 200.0f };
    int saveTrackNum; /* stack */
    int pointIdx; /* r20 */
    int vertIdx; /* r0 */
    int prevPt; /* r19 */
    int currStartPt; /* r18 */
    int currEndPt; /* r0 */
    int nextPt; /* r5 */
    float x0; /* f4 */
    float y0; /* f5 */
    float x1; /* f10 */
    float y1; /* f3 */
    float x2; /* f11 */
    float y2; /* f12 */
    float x3; /* f2 */
    float y3; /* f6 */
    float dx0; /* f1 */
    float dy0; /* f13 */
    float dx1; /* f8 */
    float dy1; /* f15 */
    float dx2; /* f9 */
    float dy2; /* f0 */
    float xoffset; /* f26 */
    float yoffset; /* f25 */
    float zoffset; /* f24 */
    int trackMBIdx; /* r17 */
    anon_name_radius_48 *trackRomObjDef; /* r16 */
    anon_numPack_name_2c *trackPS2; /* r15 */
    int sizePS2; /* stack */
    int i; /* r0 */
    float sectVerts[8][3]; /* stack */
    _tdstVertext ps2sectVertes[10]; /* stack */
    { static float mapPos[3] = { -100.0f, 30.0f, 200.0f }; }
    /* TODO */
}

/* 0x8008bca0, size 0x184 */
static void buildStuntTrackMaps(void)
{
    static float mapPos[3] = {-100.0f,30.0f,200.0f};
    /* TODO: body excluded until original static layout is recovered. */
}

/* 0x8008be24, size 0x184 */
static void buildBattleTrackMaps(void)
{
    static float mapPos[3] = {-100.0f,30.0f,200.0f};
    /* TODO: body excluded until original static layout is recovered. */
}

/* 0x8008bfa8, size 0x4 */
static void buildObstacleTrackMaps(void)
{}

/* 0x8008bfac, size 0x148 */
static void clearTrackMaps(void)
{
    int trackIdx;
    int i;
    if (trackMapsBuilt) {
        for (trackIdx = 0; trackIdx <= 19; trackIdx++) {
            if (trackMap[trackIdx].objIdx != -1) {
                ZOID_RemoveObject(trackMap[trackIdx].objIdx);
                trackMap[trackIdx].objIdx = -1;
            }
        }
        while (*(volatile signed char *)&gRenderInProgress) {}
        for (trackIdx = 0; trackIdx <= 6; trackIdx++)
            MBOX_FindObject_WORK(trackMapObjNames[trackIdx < 6 ? trackIdx : 19], 0, gNumModels - 1, 1);
        WaitTillDMAEnd();
        for (trackIdx = 0; trackIdx <= 5; trackIdx++) {
            if (mapObj[trackIdx].numPack) {
                DeallocateMemory(mapObj[trackIdx].tex);
                DeallocateMemory(mapObj[trackIdx].pPack);
                mapObj[trackIdx].numPack = 0;
                for (i = 0; i < mapObj[trackIdx].nStrips; i++)
                    DeallocateMemory(mapObj[trackIdx].strips[i].vertexs);
                DeallocateMemory(mapObj[trackIdx].strips);
                mapObj[trackIdx].nStrips = 0;
            }
        }
        trackMapsBuilt = 0;
    }
}

/* 0x8008c0f4, size 0x580 */
static void animTrackMaps(void)
{
    static float mapPos[3] = { -100.0f, 30.0f, 200.0f };
    /* TODO: original body not recovered; research candidate excluded. */
}

/* 0x8008c674, size 0x6c */
static void loadPathData(void)
{
    if (gTrackNum >= 0 && gTrackNum <= 5) {
        pathWaveIdx = LoadWave(gTrackNum + 0x122, 0, 0, 0, 0);
        gPathWave = GetWaveInstanceAddr(pathWaveIdx);
        ReadCheckpointData();
        ReadPathData();
    } else pathWaveIdx = -1;
}

/* 0x8008c6e0, size 0x38 */
static void unloadPathData(void)
{
    if (pathWaveIdx != -1) UnloadWaveInstance(pathWaveIdx, 1);
    pathWaveIdx = -1;
}

/* 0x8008c718, size 0x3ac */
static void initButtons(void)
{
    anon_c_rgba_4 buttonColor = {{255,255,255,255}};
    anon_name_width_30 *tex;
    unsigned short texIdx;
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 48; buttonIdx++) {
        if (buttonInfo[buttonIdx].objIdx == -1) {
            memcpy(buttonInfo[buttonIdx].mat.mat3.uvs,unitmatrix,36);
            buttonInfo[buttonIdx].objIdx = MBOX_NewObject(MBOX_FindObject_WORK(buttonNames[buttonInfo[buttonIdx].buttonType],0,gNumModels-1,1),&buttonInfo[buttonIdx].mat,-1,(buttonInfo[buttonIdx].buttonType == 0 || buttonInfo[buttonIdx].buttonType == 1 || buttonInfo[buttonIdx].buttonType == 2) ? 0x42000 : 0);
            ZOID_SetColor(buttonInfo[buttonIdx].objIdx,&buttonColor);
        }
    }
    tex = MBOX_FindTexture_WORK("BUTTON",&texIdx,0,gNumModels-1,1);
    ZOID_SetTexture(buttonInfo[42].objIdx,tex,-1);
    memcpy(buttonInfo[42].mat.mat3.uvs,unitmatrix,36);
    WPitchUV(-1.5707963705062866f,buttonInfo[42].mat.mat3.uvs);
    buttonInfo[42].mat.mat3.pos[0] = 0.0f;
    buttonInfo[42].mat.mat3.pos[1] = 115.0f;
    buttonInfo[42].mat.mat3.pos[2] = 200.0f;
    tex = MBOX_FindTexture_WORK("CURSOR",&texIdx,0,gNumModels-1,1);
    ZOID_SetTexture(buttonInfo[43].objIdx,tex,-1);
    tex = MBOX_FindTexture_WORK("A1",&texIdx,0,gNumModels-1,1);
    ZOID_SetTexture(buttonInfo[44].objIdx,tex,-1);
    memcpy(buttonInfo[44].mat.mat3.uvs,unitmatrix,36);
    WPitchUV(-1.5707963705062866f,buttonInfo[44].mat.mat3.uvs);
    buttonInfo[44].mat.mat3.pos[0] = 75.1f;
    buttonInfo[44].mat.mat3.pos[1] = 59.2f;
    buttonInfo[44].mat.mat3.pos[2] = 100.0f;
    ScaleUV(buttonInfo[44].mat.mat3.uvs,buttonInfo[44].mat.mat3.uvs,0.5f);
    memcpy(buttonInfo[45].mat.mat3.uvs,unitmatrix,36);
    WPitchUV(-1.5707963705062866f,buttonInfo[45].mat.mat3.uvs);
    WRollUV(3.1415927410125732f,buttonInfo[45].mat.mat3.uvs);
    buttonInfo[45].mat.mat3.pos[0] = 87.1f;
    buttonInfo[45].mat.mat3.pos[1] = 59.2f;
    buttonInfo[45].mat.mat3.pos[2] = 100.0f;
    tex = MBOX_FindTexture_WORK("B1",&texIdx,0,gNumModels-1,1);
    ZOID_SetTexture(buttonInfo[46].objIdx,tex,-1);
    memcpy(buttonInfo[46].mat.mat3.uvs,unitmatrix,36);
    WPitchUV(-1.5707963705062866f,buttonInfo[46].mat.mat3.uvs);
    buttonInfo[46].mat.mat3.pos[0] = -75.1f;
    buttonInfo[46].mat.mat3.pos[1] = 59.2f;
    buttonInfo[46].mat.mat3.pos[2] = 100.0f;
    ScaleUV(buttonInfo[46].mat.mat3.uvs,buttonInfo[46].mat.mat3.uvs,0.5f);
    memcpy(buttonInfo[47].mat.mat3.uvs,unitmatrix,36);
    WPitchUV(-1.5707963705062866f,buttonInfo[47].mat.mat3.uvs);
    buttonInfo[47].mat.mat3.pos[0] = -87.1f;
    buttonInfo[47].mat.mat3.pos[1] = 59.2f;
    buttonInfo[47].mat.mat3.pos[2] = 100.0f;
    buttonsInited = 1;
    buttonsJustInited = 1;
    ZeroCameras();
}

/* 0x8008cac4, size 0x90 */
static void uninitButtons(void)
{
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 48; buttonIdx++) {
        if (buttonInfo[buttonIdx].objIdx != -1) {
            if (gGameState & 0x7c03fffe) ZOID_RemoveObject(buttonInfo[buttonIdx].objIdx);
            buttonInfo[buttonIdx].objIdx = -1;
        }
    }
    buttonsInited = 0;
}

/* 0x8008cb54, size 0x7b8 */
static void updateButtons(void)
{
    int buttonIdx; /* r27 */
    float prevPos[3]; /* stack */
    float buttonY; /* f27 */
    float selectedPos; /* f27 */
    signed char clearScrollSpeed; /* r26 */
    anon_name_width_30 *tex; /* r0 */
    unsigned short texIdx; /* stack */
    anon_c_rgba_4 buttonColor; /* stack */
    /* TODO */
}

