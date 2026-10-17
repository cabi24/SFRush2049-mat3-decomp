/* hud.c - partial HUD lifecycle, updates and time formatting.
 * Functions omitted from this source remain unmatched. Keep NonMatching.
 */
#include "rush/rush_protos.h"

unsigned char *cStrCreditNames[14][11] = {
    { (unsigned char *)"ED LOGG", (unsigned char *)"MICHAEL A. KELLY", (unsigned char *)"SCOTT REESER", (unsigned char *)"DAVID MERRILL", 0, 0, 0, 0, 0, 0, 0 },
    { (unsigned char *)"MATT HARVEY", (unsigned char *)"GREG HOLT", (unsigned char *)"JOSE ERAZO", (unsigned char *)"TODD PAPY", (unsigned char *)"DAVE HANSON", (unsigned char *)"GIOVANNI ERAZO", (unsigned char *)"JOSE AMPARAN", 0, 0, 0, 0 },
    { (unsigned char *)"SCOT AMOS", (unsigned char *)"LOREN BRYANT", 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { (unsigned char *)"MIKE HENRY", (unsigned char *)"JOE LYFORD", (unsigned char *)"BARRY LEITCH", (unsigned char *)"PABLO BUITRAGO", 0, 0, 0, 0, 0, 0, 0 },
    { (unsigned char *)"MIKE HENRY", (unsigned char *)"BARRY LEITCH", 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { (unsigned char *)"CHRISTA WITTENBERG", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { (unsigned char *)"MIKE KRUSE", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { (unsigned char *)"LARRY CADELINA", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { (unsigned char *)"MARIO GUEVARA", (unsigned char *)"SPENCER RAY", 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { (unsigned char *)"PELE GAOTEOTE", (unsigned char *)"JOFFREY SUAREZ", (unsigned char *)"CHARLES YBARRA", (unsigned char *)"TRENTON LEWIS", (unsigned char *)"PABLO BUITRAGO", 0, 0, 0, 0, 0, 0 },
    { (unsigned char *)"FREDRIC MORA", (unsigned char *)"MARVIN HALE", (unsigned char *)"CHIP LYAS", (unsigned char *)"JAY GOITIA", (unsigned char *)"ABRAHAM ARANDA", (unsigned char *)"CJ SARRAGA", 0, 0, 0, 0, 0 },
    { (unsigned char *)"JUSTIN HENDRY", (unsigned char *)"DERRICK LAUREL", (unsigned char *)"JACOB RAINBOW", (unsigned char *)"SHAWN PARMER", (unsigned char *)"MIKE SHERRIL", (unsigned char *)"SOLARA FOXIE", (unsigned char *)"MICHELLE CADELINA", 0, 0, 0, 0 },
    { (unsigned char *)"THE COFFEE MACHINE", (unsigned char *)"TAKE-OUT FOOD", (unsigned char *)"FRIDAY DONUTS", (unsigned char *)"ROLLER COASTERS", (unsigned char *)" ", (unsigned char *)"YOU, THE GAMERS!", 0, 0, 0, 0, 0 },
    { (unsigned char *)" ", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

float gCheckPtTimeout[4];
float gLapTime[4];
float gElapsedTime[4];
static unsigned char CheckpointStr[4][8];
static signed short markers_valid;
static int animBones(_tBlit *blt);
int animCountdownTimer(void);
static int animCreditText(_tBlit *blt);
static int animDot(_tBlit *);
static int animElapsedTimer(_tBlit *blt);
static int animElapsedTimerText(void);
static int animFlag(_tBlit *);
int animGear(void);
static int animLapCount(_tBlit *blt);
static int animLapTime(_tBlit *blt);
static int animLapTimeText(void);
static int animOdometer(_tBlit *);
static int animPlace(_tBlit *blt);
static int animRadar(_tBlit *blt);
static int animRadarBlip(_tBlit *);
static int animResults(_tBlit *);
int animSpeed(_tBlit_2 *blt);
int animSpeedText(void);
int animTach(_tBlit_2 *blt);
static int animTrackMap(_tBlit *);
static int animWrongWay(void);
static int updateFrame(void);
static const anon_texname_dulx_24 RaceMultiBlit[90] = {
    { "SPEEDBG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animSpeed, 16 },
    { "SPEEDBG2", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animSpeed, 32 },
    { "SPEEDBG2", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animSpeed, 33 },
    { "SPEEDBG2", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animSpeed, 34 },
    { "SPEEDBG2", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animSpeed, 35 },
    { (const char *)-1, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))animSpeedText, 0 },
    { "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animTach, 0 },
    { "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animTach, 1 },
    { "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animTach, 16 },
    { "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animTach, 17 },
    { "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animTach, 32 },
    { "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animTach, 33 },
    { "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animTach, 48 },
    { "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animTach, 49 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 0 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 1 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 2 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 3 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 16 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 17 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 18 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 19 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 32 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 33 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 34 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 35 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 48 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 49 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 50 },
    { "ODOMETER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animOdometer, 51 },
    { (const char *)-1, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))animGear, 0 },
    { (const char *)-1, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))animCountdownTimer, 0 },
    { "ELAPSEBG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animElapsedTimer, 1 },
    { "ELAPSEBG2", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animElapsedTimer, 2 },
    { (const char *)-1, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))animElapsedTimerText, 0 },
    { (const char *)-1, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))animPlace, 0 },
    { (const char *)-1, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))animLapCount, 0 },
    { "ELAPSEBG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animLapTime, 16 },
    { "ELAPSEBG2", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animLapTime, 32 },
    { "ELAPSEBG2", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animLapTime, 33 },
    { "ELAPSEBG2", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animLapTime, 34 },
    { "ELAPSEBG2", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animLapTime, 35 },
    { (const char *)-1, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))animLapTimeText, 0 },
    { (const char *)-1, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))animWrongWay, 0 },
    { "RADAR_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 96, (int (*)(_tBlit *))animRadar, 0 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 0 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 1 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 2 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 3 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 4 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 5 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 6 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 7 },
    { "RADAR_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 96, (int (*)(_tBlit *))animRadar, 1 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 256 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 257 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 258 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 259 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 260 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 261 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 262 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 263 },
    { "RADAR_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 96, (int (*)(_tBlit *))animRadar, 2 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 512 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 513 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 514 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 515 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 516 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 517 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 518 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 519 },
    { "RADAR_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 96, (int (*)(_tBlit *))animRadar, 3 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 768 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 769 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 770 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 771 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 772 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 773 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 774 },
    { "RADARBLIP", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animRadarBlip, 775 },
    { "MAP_LG", 220, 72, -1, -1, -1, -1, -1, -1, 0, 128, (int (*)(_tBlit *))animTrackMap, 16 },
    { "FLAG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animFlag, 0 },
    { "MAPDOT", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animDot, 0 },
    { "MAPDOT", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animDot, 1 },
    { "MAPDOT", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animDot, 2 },
    { "MAPDOT", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animDot, 3 },
    { "MAPDOT", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animDot, 4 },
    { "MAPDOT", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animDot, 5 },
    { "MAPDOT", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animDot, 6 },
    { "MAPDOT", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animDot, 7 },
};

static const anon_texname_dulx_24 HudMultiBlit[5] = {
    { (const char *)-1, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))updateFrame, 0 },
    { "CROSSBONES", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animBones, 0 },
    { "CROSSBONES", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animBones, 1 },
    { "CROSSBONES", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animBones, 2 },
    { "CROSSBONES", 0, 0, -1, -1, -1, -1, -1, -1, 0, 255, (int (*)(_tBlit *))animBones, 3 },
};

static const anon_texname_dulx_24 ResultsMultiBlit[1] = {
    { (const char *)-1, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))animResults, 0 },
};

static const anon_texname_dulx_24 TwoPlayerMultiBlit[1] = {
    { "MULTI_LOGO", 240, 0, -1, -1, -1, -1, -1, -1, 1, 255, 0, 0 },
};

static const anon_texname_dulx_24 ThreePlayerMultiBlit[1] = {
    { "MULTI_LOGO", 160, 112, 320, 220, 0, 219, 160, 479, 1, 255, 0, 0 },
};

static const anon_texname_dulx_24 OverlayMultiBlit[1] = {
    { (const char *)-1, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))animCreditText, 0 },
};
float hudObjAngle;
static anon_idx_mat_34 gPlayerCoin[4][2];
static anon_used_perm_20 *resultPane[5];
static _tBlit *multiPlayerBlit;
static _tBlit *HudBlit1;
static _tBlit *HudBlit2;
static _tBlit *ResultBlit;
static void CleanupOverlay(void);
signed char gShowElapsed;
static int bonesPos[4][4][2] = {
    200, 4, 0, 0, 0, 0, 0, 0,
    150, 4, 150, 116, 0, 0, 0, 0,
    110, 4, 210, 4, 110, 116, 0, 0,
    110, 4, 210, 4, 110, 116, 210, 116
};
static int elapsedTimerPos[4][2] = {
    272, 144, 272, 144, 240, 200, 160, 136
};

static int radarPos[4][4][2] = {
    10, 60, 0, 0, 0, 0, 0, 0,
    10, 42, 10, 154, 0, 0, 0, 0,
    10, 42, 276, 42, 10, 154, 0, 0,
    10, 42, 276, 42, 10, 154, 276, 154,
};
static int speedPos[4][4][2] = {
    30, 4, 0, 0, 0, 0, 0, 0,
    30, 4, 30, 116, 0, 0, 0, 0,
    30, 4, 290, 4, 30, 116, 0, 0,
    30, 4, 290, 4, 30, 116, 290, 116,
};
static int lapCountPos[4][4][2] = {
    280, 34, 0, 0, 0, 0, 0, 0,
    210, 24, 210, 136, 0, 0, 0, 0,
    132, 24, 188, 24, 132, 136, 0, 0,
    132, 24, 188, 24, 132, 190, 188, 190,
};
static int gearPos[4][4][2] = {
    96, 14, 0, 0, 0, 0, 0, 0,
    92, 10, 92, 122, 0, 0, 0, 0,
    82, 10, 264, 10, 82, 122, 0, 0,
    82, 10, 264, 10, 82, 122, 264, 122,
};
static int tachPos[4][4][2] = {
    90, 4, 0, 0, 0, 0, 0, 0,
    90, 4, 90, 116, 0, 0, 0, 0,
    70, 6, 240, 6, 70, 118, 0, 0,
    70, 6, 240, 6, 70, 118, 240, 118,
};
static int placePos[4][4][2] = {
    280, 4, 0, 0, 0, 0, 0, 0,
    210, 4, 210, 116, 0, 0, 0, 0,
    136, 4, 184, 4, 136, 116, 0, 0,
    136, 4, 184, 4, 136, 200, 184, 200,
};
static unsigned char playerColor[4][4] = { 64, 128, 224, 63, 224, 64, 64, 63, 224, 224, 64, 63, 64, 224, 64, 63 };
static int radarWidth = 16;
static int radarHeight = 16;


static float wrongWayTime[4];
static int gCredit, gCorner, gCreditWidth, gNumCreditNames;
static int lapTimePos[4][4][2] = {
    160, 200, 0, 0, 0, 0, 0, 0,
    120, 88, 120, 200, 0, 0, 0, 0,
    80, 88, 240, 88, 80, 200, 0, 0,
    80, 88, 240, 88, 80, 200, 240, 200,
};
static unsigned char lapTimeBGColor[4][4] = { 0, 0, 0, 192, 0, 0, 0, 192, 0, 0, 0, 192, 0, 0, 0, 192 };

void ShowHUD(void)
{
    int init = 0;
    int x, y;
    static float delay;
    static float nextSwitch;
    if (!HudBlit1) {
        int slot;
        for (slot = 0; slot < 4; slot++) {
            gCheckPtTimeout[slot] = 0.0f;
            gLapTime[slot] = 0.0f;
            gElapsedTime[slot] = 0.0f;
            gPlayerCoin[slot][0].idx = -1;
            gPlayerCoin[slot][1].idx = -1;
        }
        if (gGameState & 0x2000000)
            HudBlit1 = NewMultiBlit(0, 0, OverlayMultiBlit, 1);
        else if (gGameState & 8)
            HudBlit1 = NewMultiBlit(0, 0, OverlayMultiBlit, 1);
        else {
            if (gNumWindows == 2)
                multiPlayerBlit = NewMultiBlit(0, 0, TwoPlayerMultiBlit, 1);
            else if (gNumWindows == 3)
                multiPlayerBlit = NewMultiBlit(0, 0, ThreePlayerMultiBlit, 1);
            if (gRace == kBattle || gRace == kStunt)
                ShowHUDExtra();
            else
                HudBlit2 = NewMultiBlit(0, 0, RaceMultiBlit, 90);
            HudBlit1 = NewMultiBlit(0, 0, HudMultiBlit, 5);
        }
        markers_valid = 0;
        init = 1;
    }
    if ((gGameState & 8) && gShowCredits) {
        if (init) {
            gCredit = -1;
            SetCountdownTimer(90.0f);
            delay = GetCountdownTimeRemaining() / 14.0f;
            nextSwitch = gSeconds;
        }
        if (gSeconds >= nextSwitch) {
            nextSwitch += delay;
            gCredit++;
            gCorner = Random(4.0f);
            HAL_SetFont(11);
            gCreditWidth = HAL_GetTextWidth(gText.taba[gText.tab[0] + gCredit], -1);
            for (gNumCreditNames = 0; cStrCreditNames[gCredit][gNumCreditNames]; gNumCreditNames++) {
                if (HAL_GetTextWidth(cStrCreditNames[gCredit][gNumCreditNames], -1) > gCreditWidth)
                    gCreditWidth = HAL_GetTextWidth(cStrCreditNames[gCredit][gNumCreditNames], -1);
            }
            switch (gCorner) {
            case 0: x = 20; y = 20; break;
            case 1: x = 280 - gCreditWidth; y = 20; break;
            case 2: x = 20; y = 149 - 10 * gNumCreditNames; break;
            default: x = 280 - gCreditWidth; y = 149 - 10 * gNumCreditNames; break;
            }
            RemovePanes(0);
            InitPane(x, y, x + gCreditWidth + 20, y + 10 * gNumCreditNames + 32, 192, 0, 0, 0);
        }
    }
}

void RemoveHUD(void)
{
    int player;

    RemovePanes(1);
    if (HudBlit1) {
        RemoveBlit(HudBlit1);
        HudBlit1 = 0;
        CleanupOverlay();
    }
    if (HudBlit2) {
        RemoveBlit(HudBlit2);
        HudBlit2 = 0;
    }
    if (ResultBlit) {
        RemoveBlit(ResultBlit);
        ResultBlit = 0;
    }
    for (player = 0; player < gNumWindows; player++)
        resultPane[player] = 0;
    resultPane[4] = 0;
    for (player = 0; player < gNumWindows; player++) {
        if (gPlayerCoin[player][0].idx != -1) {
            ZOID_RemoveObject(gPlayerCoin[player][0].idx);
            gPlayerCoin[player][0].idx = -1;
        }
        if (gPlayerCoin[player][1].idx != -1) {
            ZOID_RemoveObject(gPlayerCoin[player][1].idx);
            gPlayerCoin[player][1].idx = -1;
        }
    }
    if (gRace == kBattle || gRace == kStunt)
        RemoveHUDExtra();
}

void RemoveMultiplayerHUD(void)
{
    if (multiPlayerBlit) {
        RemoveBlit(multiPlayerBlit);
        multiPlayerBlit = 0;
    }
}

void UpdateHUDPostCamera(void)
{
    if (gGameState == -1 || (!(gGameState & 0x200000) && !(gGameState & 0x400000)))
        return;
    if (gPauseState == 0)
        hudObjAngle += 3.1415927f * gFrameSeconds;
    if ((unsigned int)gRace <= 3 || gRace == kStunt)
        animCoins(0);
    if (gRace == kBattle || gRace == kStunt)
        UpdateHUDExtraPostCamera();
}

void UpdateHUDPostCamera2(void)
{
    if (gGameState == -1 || (!(gGameState & 0x200000) && !(gGameState & 0x400000)))
        return;
    if ((unsigned int)gRace <= 3 || gRace == kStunt)
        animCoinsText(0);
}

void AddBattlePoint(signed char to, signed char from, signed char p)
{
    if (gRace == kBattle || gRace == kStunt)
        _AddBattlePoint(to, from, p);
}

static unsigned char TrackTimeStr[4][8];
static signed char dot_order_list[6];
static signed short markers_valid;
static void CheckResults(void);

#pragma push
#pragma force_active on
static int updateFrame(void)
{
    signed short cars_in_order[6];
    signed short index, i, j;
    int slot;
    for (slot = 0; slot < 4; slot++)
        cvt_time(gGameCar[slot].place_locked == 1 ? gGameCar[slot].score : gModelTime,
                 TrackTimeStr[slot], 'f');
    dot_order_list[0] = -1;
    dot_order_list[1] = -1;
    dot_order_list[2] = -1;
    dot_order_list[3] = -1;
    dot_order_list[4] = -1;
    dot_order_list[5] = -1;
    for (i = 0; i < gNumActiveCars; i++) {
        index = 0;
        for (j = 0; j < gNumActiveCars; j++) {
            index = model[j].slot;
            if (gGameCar[index].place == i)
                break;
        }
        cars_in_order[i] = index;
    }
    for (j = 0; j < *(volatile signed short *)&num_humans; j++)
        dot_order_list[gNumActiveCars - j - 1] = gPlayers[j].slot;
    for (i = 0; i < gNumActiveCars; i++) {
        if (model[index = cars_in_order[i]].drone_type == 1)
            dot_order_list[gNumActiveCars - 1 - j++] = index;
        if (j == gNumActiveCars)
            break;
    }
    markers_valid = 1;
    CheckResults();
    return 1;
}
#pragma pop

#pragma push
#pragma force_active on
static int animBones(_tBlit *blt)
{
    if (!gDeathMode || gNumWindows <= blt->animID ||
        gGameCar[model[blt->animID].slot].place_locked == 1)
        return Hidden((_tBlit_2 *)blt, 1);
    blt->x = bonesPos[gNumWindows - 1][blt->animID][0] - (blt->width / 2) / 2;
    blt->y = bonesPos[gNumWindows - 1][blt->animID][1];
    UpdateBlit(blt);
    return 1;
}

static int animElapsedTimer(_tBlit *blt)
{
    int win;
    int digitW;
    int colonW;
    signed char fixedWidth;

    win = blt->animID;
    if (Hidden((_tBlit_2 *)blt,
               (win == 1 && gNumWindows > 1) || (win == 2 && gNumWindows < 2))) {
        blt->animFunc = 0;
        return 1;
    }
    if (Hidden((_tBlit_2 *)blt, (gGameState & 8) || !(gGameState & 0x400000) || !gShowElapsed))
        return 1;
    if (gNumWindows == 1)
        HAL_SetFont(1);
    else
        HAL_SetFont(2);
    digitW = HAL_GetCharWidth('8') + 1;
    fixedWidth = HAL_SetFontFixedWidth(0);
    colonW = HAL_GetCharWidth(':') + 1;
    HAL_SetFontFixedWidth(fixedWidth);
    blt->x = elapsedTimerPos[gNumWindows - 1][0] - (7 * digitW + 2 * colonW) / 2;
    blt->y = elapsedTimerPos[gNumWindows - 1][1] - 1;
    blt->alpha = 128;
    UpdateBlit(blt);
    return 1;
}
#pragma pop

int Hidden(_tBlit_2 *blt, int hide)
{
    if (hide != blt->hide) {
        blt->hide = hide;
        UpdateBlit((_tBlit *)blt);
    }
    return blt->hide;
}

unsigned char cvt_time(float t, unsigned char *dest, char format)
{
    unsigned int hun;
    unsigned int sec;
    unsigned int min;

    if (t < 0.0f)
        t = 0.0f;
    switch (format) {
    case 'c':
        sec = (unsigned int)t;
        if (sec > 999)
            sec = 999;
        dest[0] = (sec / 100) % 10;
        dest[1] = (sec / 10) % 10;
        dest[2] = sec % 10;
        dest[3] = 0;
        return 3;
    case 'f':
        sec = (unsigned int)t;
        min = sec / 60;
        sec %= 60;
        hun = (unsigned int)(100.0f * t) % 100;
        dest[0] = (min / 10) % 10;
        dest[1] = min % 10;
        dest[2] = sec / 10;
        dest[3] = sec % 10;
        dest[4] = hun / 10;
        dest[5] = hun % 10;
        dest[6] = 0;
        return 6;
    }
    return 0;
}

#pragma push
#pragma force_active on
static void CleanupOverlay(void)
{
}
#pragma pop

#pragma push
#pragma force_active on
static int animRadar(_tBlit *blt)
{
    int slot = blt->animID;
    if (slot >= gNumWindows || (gGameState & 8) || gNumCars < 2) {
        blt->animFunc = 0;
        return Hidden((_tBlit_2 *)blt, 1);
    }
    if (Hidden((_tBlit_2 *)blt, !gShowRadar || !(gGameState & 0x400000) ||
               gGameCar[slot].place_locked == 1))
        return 1;
    blt->x = radarPos[gNumWindows - 1][slot][0];
    blt->y = radarPos[gNumWindows - 1][slot][1];
    if (gNumWindows > 1) {
        RenameBlit(blt, "RADAR_MD", 0);
        blt->alpha = 96;
    }
    UpdateBlit(blt);
    radarWidth = blt->width / 2;
    radarHeight = blt->height / 2.0f;
    return 1;
}
#pragma pop

int animSpeed(_tBlit_2 *blt)
{
    int win = blt->animID & 15;
    int show = (blt->animID >> 4) & 15;
    int digitW;
    if (Hidden((_tBlit_2 *)blt, (show == 1 && gNumWindows > 1) ||
               (show == 2 && gNumWindows < 2) || win >= gNumWindows)) {
        blt->animFunc = 0;
        return 1;
    }
    if (Hidden((_tBlit_2 *)blt, !gShowSpeed || !(gGameState & 0x400000) ||
               gGameCar[model[win].slot].place_locked == 1))
        return 1;
    if (gNumWindows == 1)
        HAL_SetFont(4);
    else
        HAL_SetFont(1);
    digitW = HAL_GetCharWidth('8') + 1;
    blt->x = speedPos[gNumWindows - 1][win][0] - 2 * digitW + (gNumWindows == 1);
    blt->y = speedPos[gNumWindows - 1][win][1] + (gNumWindows == 1);
    blt->alpha = 128;
    UpdateBlit((_tBlit *)blt);
    return 1;
}

#pragma push
#pragma force_active on
static int animLapCount(_tBlit *blt)
{
    int x, y, slot;
    if ((gGameState & 8) || !(gGameState & 0x400000) ||
        gRace == kPractice || gRace == kObstacle || !gShowPlace)
        return 1;
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 3);
    if (gNumWindows == 1)
        HAL_SetFont(11);
    else
        HAL_SetFont(12);
    for (slot = 0; slot < gNumWindows; slot++) {
        if (model[slot].laps == gNumLaps || gGameCar[slot].place_locked == 1)
            continue;
        x = lapCountPos[gNumWindows - 1][slot][0];
        y = lapCountPos[gNumWindows - 1][slot][1];
        TB_SetTextColor(0);
        TB_Printf(x + 1, y + 1, gText.single[217], model[slot].laps + 1, gNumLaps);
        TB_SetTextColor(1);
        TB_Printf(x, y, gText.single[217], model[slot].laps + 1, gNumLaps);
    }
    TB_SetJust(0, 3);
    HAL_SetTextDepth(-1.0f);
    return 1;
}
#pragma pop

int animGear(void)
{
    unsigned char str[2];
    int slot;
    signed short x, y;
    signed char gear;
    if (!gShowShifter || !(gGameState & 0x400000))
        return 1;
    HAL_SetTextDepth(0.0f);
    if (gNumWindows == 1)
        HAL_SetFont(10);
    else if (gNumWindows == 2)
        HAL_SetFont(11);
    else
        HAL_SetFont(12);
    for (slot = 0; slot < gNumWindows; slot++) {
        gear = model[slot].mainin.gear;
        if (slot >= gNumWindows || model[slot].autotrans ||
            gGameCar[model[slot].slot].place_locked == 1)
            continue;
        x = gearPos[gNumWindows - 1][slot][0] - 4;
        y = gearPos[gNumWindows - 1][slot][1];
        TB_SetTextColor(0);
        TB_DrawText(TB_RightText(gText.single[215], x + 1), y + 1, gText.single[215]);
        TB_SetTextColor(1);
        TB_DrawText(TB_RightText(gText.single[215], x), y, gText.single[215]);
        x = gearPos[gNumWindows - 1][slot][0] + 4;
        y = gearPos[gNumWindows - 1][slot][1];
        if (gear == -1)
            str[0] = 'R';
        else if (gear == 0)
            str[0] = 'N';
        else
            str[0] = gear + '0';
        str[1] = 0;
        TB_SetTextColor(0);
        TB_DrawText(x + 1, y + 1, str);
        TB_SetTextColor(14);
        TB_DrawText(x, y, str);
    }
    HAL_SetTextDepth(-1.0f);
    return 1;
}

int animTach(_tBlit_2 *blt)
{
    int slot = (blt->animID >> 4) & 15;
    int type = blt->animID & 15;
    float tach;
    if (slot >= gNumWindows) {
        if (slot >= gNumPlayers)
            blt->animFunc = 0;
        Hidden(blt, 1);
        return 1;
    }
    if (Hidden(blt, !gShowTach || !(gGameState & 0x400000) ||
               model[slot].autotrans || gGameCar[model[slot].slot].place_locked == 1))
        return 1;
    if (gNumWindows == 2)
        RenameBlit((_tBlit *)blt, "TACHOMETER_MD", 0);
    else if (gNumWindows > 2)
        RenameBlit((_tBlit *)blt, "TACHOMETER_SM", 0);
    blt->x = tachPos[gNumWindows - 1][slot][0] - (blt->width / 2) / 2;
    blt->y = tachPos[gNumWindows - 1][slot][1];
    if (type == 0)
        blt->bot = blt->height / 2 - 1;
    else if (type == 1) {
        tach = 0.0001f * (1.35f * model[slot].rpm);
        if (tach < 0.0f)
            tach = -tach;
        if (tach > 1.0f)
            tach = 1.0f;
        blt->right = (blt->width - 1) * tach;
        blt->top = blt->height / 2;
        blt->bot = blt->height - 1;
    }
    blt->alpha = 254;
    UpdateBlit((_tBlit *)blt);
    return 1;
}

int animSpeedText(void)
{
    float mph;
    unsigned char digits[3];
    unsigned char str[2];
    int digitW, x, y, digitIdx, slot;
    if (!gShowSpeed || !(gGameState & 0x400000))
        return 1;
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 3);
    if (gNumWindows == 1)
        HAL_SetFont(4);
    else
        HAL_SetFont(1);
    digitW = HAL_GetCharWidth('8') + 1;
    for (slot = 0; slot < gNumWindows; slot++) {
        if (gGameCar[model[slot].slot].place_locked == 1)
            continue;
        mph = gGameCar[slot].rwmph >> 2;
        mph *= 1.2f;
        if (gShowMetric)
            mph *= 1.61f;
        if (mph < 0.0f)
            mph = -mph;
        x = speedPos[gNumWindows - 1][slot][0];
        x -= digitW;
        y = speedPos[gNumWindows - 1][slot][1];
        digits[0] = ((int)(mph / 100.0f) % 10) + '0';
        digits[1] = ((int)(mph / 10.0f) % 10) + '0';
        digits[2] = ((int)mph % 10) + '0';
        if (gNumWindows == 1)
            HAL_SetFont(4);
        else
            HAL_SetFont(1);
        for (digitIdx = 0; digitIdx < 3; digitIdx++) {
            str[1] = 0;
            if (digits[digitIdx] != ':' && digits[digitIdx] != '.') {
                str[0] = '8';
                TB_SetTextColor(0);
                TB_DrawText(x, y, str);
            }
            str[0] = digits[digitIdx];
            TB_SetTextColor(14);
            TB_DrawText(x, y, str);
            x += digitW;
        }
        x = speedPos[gNumWindows - 1][slot][0];
        y += TB_GetFontHeight() - 4;
        HAL_SetFont(12);
        if (gShowMetric) {
            TB_SetTextColor(0);
            TB_DrawText(x + 1, y + 1, gText.single[214]);
            TB_SetTextColor(1);
            TB_DrawText(x, y, gText.single[214]);
        } else {
            TB_SetTextColor(0);
            TB_DrawText(x + 1, y + 1, gText.single[213]);
            TB_SetTextColor(1);
            TB_DrawText(x, y, gText.single[213]);
        }
    }
    TB_SetJust(0, 3);
    HAL_SetTextDepth(-1.0f);
    return 1;
}

#pragma push
#pragma force_active on
static int animPlace(_tBlit *blt)
{
    unsigned char digits[3];
    unsigned char str[2];
    int digitW, x, y, digitIdx, slot;
    if ((gGameState & 8) || !(gGameState & 0x400000) || gNumCars < 2 ||
        gRace == kPractice || !gShowPlace)
        return 1;
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 3);
    if (gNumWindows == 1)
        HAL_SetFont(8);
    else
        HAL_SetFont(5);
    digitW = HAL_GetCharWidth('8');
    digits[1] = '/';
    digits[2] = gNumCars + '0';
    for (slot = 0; slot < gNumWindows; slot++) {
        if (!AreWeSolid(slot) || gGameCar[slot].place_locked == 1)
            continue;
        x = placePos[gNumWindows - 1][slot][0];
        y = placePos[gNumWindows - 1][slot][1];
        digits[0] = gGameCar[slot].place + '1';
        x -= digitW;
        for (digitIdx = 0; digitIdx < 3; digitIdx++) {
            str[0] = digits[digitIdx];
            str[1] = 0;
            if (digitIdx < 2) {
                if (gNumWindows == 1)
                    HAL_SetFont(8);
                else
                    HAL_SetFont(5);
            } else {
                if (gNumWindows == 1)
                    HAL_SetFont(5);
                else
                    HAL_SetFont(6);
            }
            TB_SetTextColor(0);
            TB_DrawText(x + 1, y + 1, str);
            if (digitIdx == 0)
                HAL_SetTextColor(playerColor[slot][0], playerColor[slot][1], playerColor[slot][2], 255);
            else
                TB_SetTextColor(1);
            TB_DrawText(x, y, str);
            x += digitW;
        }
    }
    TB_SetJust(0, 3);
    HAL_SetTextDepth(-1.0f);
    return 1;
}
#pragma pop

#pragma push
#pragma force_active on
static int animCreditText(_tBlit *blt)
{
    int x, y, i;
    if (Hidden((_tBlit_2 *)blt, !(gGameState & 8) || !gShowCredits))
        return 1;
    switch (gCorner) {
    case 0: x = 30; y = 30; break;
    case 1: x = 290 - gCreditWidth; y = 30; break;
    case 2: x = 30; y = 159 - 10 * gNumCreditNames; break;
    default: x = 290 - gCreditWidth; y = 159 - 10 * gNumCreditNames; break;
    }
    HAL_SetFont(11);
    TB_SetTextColor(14);
    TB_DrawText(x, y, gText.taba[gText.tab[0] + gCredit]);
    TB_SetTextColor(1);
    y += 15;
    for (i = 0; i < gNumCreditNames; i++, y += 10) {
        TB_DrawText(x, y, cStrCreditNames[gCredit][i]);
    }
    return 1;
}

static int animLapTime(_tBlit *blt)
{
    int w, digitW, colonW;
    int win = blt->animID & 15;
    int show = (blt->animID >> 4) & 15;
    signed char fixedWidth;
    if (Hidden((_tBlit_2 *)blt, (show == 1 && gNumWindows > 1) ||
               (show == 2 && gNumWindows < 2) || win >= gNumWindows)) {
        blt->animFunc = 0;
        return 1;
    }
    if (Hidden((_tBlit_2 *)blt, (gGameState & 8) || !gShowElapsed ||
               gCheckPtTimeout[win] <= 0.0f || gGameCar[win].place_locked == 1))
        return 1;
    if (gNumWindows == 1)
        HAL_SetFont(1);
    else
        HAL_SetFont(2);
    digitW = HAL_GetCharWidth('8') + 1;
    fixedWidth = HAL_SetFontFixedWidth(0);
    colonW = HAL_GetCharWidth(':') + 1;
    HAL_SetFontFixedWidth(fixedWidth);
    w = 7 * digitW + 2 * colonW;
    if (gCheckPtTimeout[win] < 3.0f)
        lapTimeBGColor[win][3] = 192.0f * gCheckPtTimeout[win] / 3.0f;
    else
        lapTimeBGColor[win][3] = 192;
    blt->x = lapTimePos[gNumWindows - 1][win][0] - w / 2;
    blt->y = lapTimePos[gNumWindows - 1][win][1];
    blt->alpha = lapTimeBGColor[win][3];
    UpdateBlit(blt);
    return 1;
}
#pragma pop

#pragma push
#pragma force_active on
static int animWrongWay(void)
{
    int x, y, slot;
    static signed char wasStrayed[4];
    if (gRace == kObstacle)
        return 1;
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 1);
    for (slot = 0; slot < gNumWindows; slot++) {
        if ((-1.5707964f <= gGameCar[slot].strayAngle &&
             gGameCar[slot].strayAngle <= 1.5707964f) ||
            !(gGameState & 0x400000) || gGameCar[slot].place_locked == 1 ||
            !gShowWrongWay || gRace == kPractice) {
            wrongWayTime[slot] = gSeconds;
            wasStrayed[slot] = 0;
            continue;
        }
        wasStrayed[slot] = 1;
        if (gSeconds - wrongWayTime[slot] < 2.0f)
            continue;
        if (gNumWindows == 1)
            HAL_SetFont(10);
        else
            HAL_SetFont(11);
        HAL_SetTextTranslucency(255.0f * TB_GetFlashTextCycle());
        x = wrongWayPos[gNumWindows - 1][slot][0];
        y = wrongWayPos[gNumWindows - 1][slot][1];
        TB_SetTextColor(0);
        TB_DrawText(x + 1, y + 1, gText.single[26]);
        TB_SetTextColor(14);
        TB_DrawText(x, y, gText.single[26]);
    }
    TB_SetJust(0, 3);
    HAL_SetTextTranslucency(-1.0f);
    HAL_SetTextDepth(-1.0f);
    return 1;
}
#pragma pop

#pragma push
#pragma force_active on
static int animLapTimeText(void)
{
    int t;
    float remaining;
    unsigned char digits[9];
    unsigned char str[2];
    int digitW, colonW, x, y, digitIdx, slot;
    signed char fixedWidth;
    if ((gGameState & 8) || !gShowElapsed)
        return 1;
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 3);
    if (gNumWindows == 1)
        HAL_SetFont(1);
    else
        HAL_SetFont(2);
    digitW = HAL_GetCharWidth('8') + 1;
    fixedWidth = HAL_SetFontFixedWidth(0);
    colonW = HAL_GetCharWidth(':') + 1;
    HAL_SetFontFixedWidth(fixedWidth);
    for (slot = 0; slot < gNumWindows; slot++) {
        if (gPauseState == 0)
            gCheckPtTimeout[slot] -= gFrameSeconds;
        remaining = gCheckPtTimeout[slot];
        if (remaining <= 0.0f || gGameCar[slot].place_locked == 1) {
            gCheckPtTimeout[slot] = 0.0f;
            continue;
        }
        t = 1000.0f * gLapTime[slot];
        if (t < 0)
            t = 0;
        digits[0] = t / 600000 + '0';
        digits[1] = (t / 60000) % 10 + '0';
        digits[2] = ':';
        digits[3] = (t / 10000) % 6 + '0';
        digits[4] = (t / 1000) % 10 + '0';
        digits[5] = '.';
        digits[6] = (t / 100) % 10 + '0';
        digits[7] = (t / 10) % 10 + '0';
        digits[8] = t % 10 + '0';
        y = lapTimePos[gNumWindows - 1][slot][1];
        str[1] = 0;
        if (remaining < 3.0f)
            HAL_SetTextTranslucency((int)(255.0f * remaining / 3.0f));
        x = lapTimePos[gNumWindows - 1][slot][0] - (6 * digitW + 2 * colonW) / 2;
        for (digitIdx = 0; digitIdx < 9; digitIdx++) {
            str[0] = digits[digitIdx];
            if (digits[digitIdx] != ':' && digits[digitIdx] != '.') {
                str[0] = '8';
                TB_SetTextColor(0);
                TB_DrawText(x, y, str);
                str[0] = digits[digitIdx];
            }
            TB_SetTextColor(22);
            TB_DrawText(x, y, str);
            if (digitIdx == 1 || digitIdx == 2 || digitIdx == 4 || digitIdx == 5)
                x += (digitW + colonW) / 2;
            else
                x += digitW;
        }
        HAL_SetTextTranslucency(-1.0f);
    }
    TB_SetJust(0, 3);
    HAL_SetTextDepth(-1.0f);
    return 1;
}
#pragma pop

#pragma push
#pragma force_active on
static int animElapsedTimerText(void)
{
    int t;
    unsigned char digits[9];
    unsigned char str[2];
    int digitW, colonW;
    signed char fixedWidth;
    int x, y, digitIdx;
    if ((gGameState & 8) || !(gGameState & 0x400000) || !gShowElapsed)
        return 1;
    t = 1000.0f * gModelTime;
    if (t < 0)
        t = 0;
    digits[0] = (t / 600000) % 6 + '0';
    digits[1] = (t / 60000) % 10 + '0';
    digits[2] = ':';
    digits[3] = (t / 10000) % 6 + '0';
    digits[4] = (t / 1000) % 10 + '0';
    digits[5] = '.';
    digits[6] = (t / 100) % 10 + '0';
    digits[7] = (t / 10) % 10 + '0';
    digits[8] = t % 10 + '0';
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 3);
    if (gNumWindows == 1)
        HAL_SetFont(1);
    else
        HAL_SetFont(2);
    digitW = HAL_GetCharWidth('8') + 1;
    fixedWidth = HAL_SetFontFixedWidth(0);
    colonW = HAL_GetCharWidth(':') + 1;
    HAL_SetFontFixedWidth(fixedWidth);
    y = elapsedTimerPos[gNumWindows - 1][1];
    str[1] = 0;
    x = elapsedTimerPos[gNumWindows - 1][0] - (5 * digitW + 2 * colonW) / 2;
    for (digitIdx = 0; digitIdx < 8; digitIdx++) {
        str[0] = digits[digitIdx];
        if (digits[digitIdx] != ':' && digits[digitIdx] != '.') {
            str[0] = '8';
            TB_SetTextColor(0);
            TB_DrawText(x, y, str);
            str[0] = digits[digitIdx];
        }
        TB_SetTextColor(1);
        TB_DrawText(x, y, str);
        if (digitIdx == 1 || digitIdx == 2 || digitIdx == 4 || digitIdx == 5)
            x += (digitW + colonW) / 2;
        else
            x += digitW;
    }
    TB_SetJust(0, 3);
    HAL_SetTextDepth(-1.0f);
    return 1;
}
#pragma pop

static int countdownTimerPos[4][2] = {160, 4, 272, 56, 240, 112, 160, 72};
static int mapPos[4][2] = {280, 112, 280, 112, 240, 168, 160, 112};

int animCountdownTimer(void)
{
    int t;
    unsigned char digits[5];
    unsigned char str[2];
    int digitW, colonW, x, y, digitIdx;
    static int sec;
    static int nextBeep = 30;
    if (!(gGameState & 0x400000) || !gShowRemaining ||
        (gRace == kBattle && gBattleMode == 0) ||
        (gRace == kStunt && gStuntMode == kPointStuntMode) ||
        (unsigned int)gRace <= 1 || gRace == kCircuit)
        return 1;
    t = GetCountdownTimeRemaining();
    if (t < 0)
        t = 0;
    sec = t;
    if (sec <= 30) {
        if (sec <= nextBeep) {
            if (nextBeep > 5)
                nextBeep = 5;
            else
                nextBeep--;
            PlayGameSound(54, 0, 2, 0);
        }
        if (sec <= 5 && (sc.frameCount & 8))
            return 1;
    } else
        nextBeep = 30;
    x = countdownTimerPos[gNumWindows - 1][0];
    y = countdownTimerPos[gNumWindows - 1][1];
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 3);
    if (gRace == kStunt && gNumWindows > 1) {
        x = mapPos[gNumWindows - 1][0];
        y = mapPos[gNumWindows - 1][1];
        TB_SetJust(1, 1);
    }
    digits[0] = t / 600 + '0';
    digits[1] = (t / 60) % 10 + '0';
    digits[2] = ':';
    digits[3] = (t / 10) % 6 + '0';
    digits[4] = t % 10 + '0';
    if (gNumWindows == 1)
        HAL_SetFont(9);
    else if (gNumWindows <= 3)
        HAL_SetFont(8);
    else
        HAL_SetFont(5);
    digitW = HAL_GetCharWidth('8') + 1;
    colonW = HAL_GetCharWidth(':') + 1;
    x -= (3 * digitW + colonW) / 2;
    for (digitIdx = 0; digitIdx < 5; digitIdx++) {
        str[0] = digits[digitIdx];
        str[1] = 0;
        if (digitIdx > 0 || digits[digitIdx] != '0') {
            TB_SetTextColor(0);
            TB_DrawText(x + 1, y + 1, str);
            TB_SetTextColor(14);
            TB_DrawText(x, y, str);
        }
        if (digitIdx == 1 || digitIdx == 2)
            x += (digitW + colonW) / 2;
        else
            x += digitW;
    }
    TB_SetJust(0, 3);
    HAL_SetTextDepth(-1.0f);
    return 1;
}

static int coinPos[4][4][2] = {
    270, 54, 0, 0, 0, 0, 0, 0,
    200, 40, 200, 152, 0, 0, 0, 0,
    124, 40, 176, 40, 124, 152, 0, 0,
    124, 40, 176, 40, 124, 174, 176, 174,
};

int animCoinsText(void)
{
    signed short player;
    signed short coin;
    unsigned char str[4];
    signed short x, y;
    HAL_SetTextDepth(0.0f);
    if (gNumWindows == 1)
        HAL_SetFont(10);
    else
        HAL_SetFont(12);
    TB_SetJust(0, 1);
    for (player = 0; player < gNumWindows; player++) {
        for (coin = 0; coin < 2; coin++) {
            if (!gOptions[11] || !(gGameState & 0x400000) ||
                gGameCar[player].place_locked == 1)
                continue;
            if (gRace == kStunt) {
                x = coinStuntPos[gNumWindows - 1][player][0] + (gNumWindows == 1 ? 12 : 8);
                y = coinStuntPos[gNumWindows - 1][player][1] + coin * (gNumWindows == 1 ? 15 : gNumWindows == 2 ? 12 : 8);
            } else {
                x = coinPos[gNumWindows - 1][player][0] + (gNumWindows == 1 ? 12 : 8);
                y = coinPos[gNumWindows - 1][player][1] + coin * (gNumWindows == 1 ? 15 : gNumWindows == 2 ? 12 : 8);
            }
            TB_Sprintf(str, "%d/8", NumCoinsFound(player, coin + 1));
            TB_SetTextColor(0);
            TB_DrawText(x + 1, y + 1, str);
            TB_SetTextColor(1);
            TB_DrawText(x, y, str);
        }
    }
    TB_SetJust(0, 3);
    HAL_SetTextDepth(-1.0f);
    return 1;
}


void ShowCheckpointTime(int slot, float time)
{
    gCheckPtTimeout[slot] = 6.0f;
    gLapTime[slot] = time - gElapsedTime[slot];
    gElapsedTime[slot] = time;
    cvt_time(time, CheckpointStr[slot], 'f');
}

static int odometerPos[4][4][2] = {
    30, 32, 0, 0, 0, 0, 0, 0,
    30, 30, 30, 142, 0, 0, 0, 0,
    30, 30, 290, 30, 30, 142, 0, 0,
    30, 30, 290, 30, 30, 142, 290, 142,
};

static inline tCarData *hudCar(int slot) { return &gGameCar[slot]; }

static int animOdometer(_tBlit *blt)
{
    int p = (blt->animID >> 4) & 15;
    int d = blt->animID & 15;
    float carMileage = hudCar(p)->mileage;
    float flipAmount = 0.0f;
    if (gOptions[9])
        carMileage /= 0.6f;
    if (p >= gNumWindows || gRace == kObstacle) {
        blt->animFunc = 0;
        Hidden((_tBlit_2 *)blt, 1);
        return 1;
    }
    if (Hidden((_tBlit_2 *)blt, !gShowOdometer || !(gGameState & 0x400000) || gGameCar[model[p].slot].place_locked == 1))
        return 1;
    if ((int)(carMileage / 528.0f) % 10 == 9)
        flipAmount = carMileage / 528.0f - (int)(carMileage / 528.0f);
    switch (d) {
    case 0:
        blt->x = odometerPos[gNumWindows - 1][p][0] - (blt->width / 2 * 2) / 2;
        blt->y = odometerPos[gNumWindows - 1][p][1];
        blt->left = 0;
        blt->top = ((int)(carMileage / 528000.0f) % 10) * (blt->width / 2);
        if ((int)(carMileage / 52800.0f) % 10 == 9 && (int)(carMileage / 5280.0f) % 10 == 9 && (int)(carMileage / 528.0f) % 10 == 9)
            blt->top += flipAmount * (blt->width / 2);
        break;
    case 1:
        blt->x = odometerPos[gNumWindows - 1][p][0] - (blt->width / 2) / 2;
        blt->y = odometerPos[gNumWindows - 1][p][1];
        blt->left = 0;
        blt->top = ((int)(carMileage / 52800.0f) % 10) * (blt->width / 2);
        if ((int)(carMileage / 5280.0f) % 10 == 9 && (int)(carMileage / 528.0f) % 10 == 9)
            blt->top += flipAmount * (blt->width / 2);
        break;
    case 2:
        blt->x = odometerPos[gNumWindows - 1][p][0];
        blt->y = odometerPos[gNumWindows - 1][p][1];
        blt->left = 0;
        blt->top = ((int)(carMileage / 5280.0f) % 10) * (blt->width / 2);
        if ((int)(carMileage / 528.0f) % 10 == 9)
            blt->top += flipAmount * (blt->width / 2);
        break;
    case 3:
        blt->x = odometerPos[gNumWindows - 1][p][0] + (blt->width / 2) / 2;
        blt->y = odometerPos[gNumWindows - 1][p][1];
        blt->left = blt->width / 2;
        flipAmount = (int)(carMileage / 528.0f) % 10 + carMileage / 528.0f - (int)(carMileage / 528.0f);
        blt->top = flipAmount * (blt->width / 2);
        break;
    }
    blt->right = blt->left + blt->width / 2 - 1;
    blt->bot = blt->top + blt->width / 2 - 1;
    UpdateBlit(blt);
    return 1;
}

static void *dataMap;
static void *dataObj;
static int whichCam[4] = {1,2,4,8};

static int animDot(_tBlit *blt)
{
    int gDotSize;
    short flash;
    short slot = dot_order_list[blt->animID & 15];
    tModelData *m;
    tCarData *c;
    int width, height, w, h, yOffset;
    if (Hidden((_tBlit_2 *)blt, gNumWindows > 4 || slot == -1 || !(gGameState & 0x400000) || !gShowTrackMap))
        return 1;
    gDotSize = blt->height;
    m = &model[slot];
    c = &gGameCar[slot];
    width = gPathMax[0] - gPathMin[0];
    height = gPathMax[2] - gPathMin[2];
    if (width > height) {
        w = kMapSize - 8;
        h = height * (kMapSize - 8) / width;
    } else {
        w = kMapSize - 8;
        h = kMapSize - 8;
    }
    yOffset = (kMapSize - 8 - h) / 2 + 4;
    blt->x = mapPos[gNumWindows - 1][0] + ((kMapSize - 8 - w) / 2 + 4 - (gDotSize + kMapSize) / 2) / 2;
    blt->y = mapPos[gNumWindows - 1][1] + (yOffset - (gDotSize + kMapSize) / 2) / 2.0f;
    if (gMirrorWaves)
        blt->x += w * (c->dr_pos[0] - gPathMin[0]) / width / 2.0f;
    else
        blt->x += (w - 1 - w * (c->dr_pos[0] - gPathMin[0]) / width) / 2.0f;
    blt->y += h * (c->dr_pos[2] - gPathMin[2]) / height / 2.0f;
    flash = c->crash_state || m->crashflag || !m->solid || m->hide_car || m->resurrect.moving_state != -1;
    if (c->dead > 0)
        flash = 0;
    if (slot < gNumCars) {
        blt->alpha = (sc.frameCount & 8) && flash ? 64 : 255;
        if (m->drone_type == 2)
            SelectBlit(blt, slot, gDotSize, gDotSize);
        else if (m->drone_type == 1)
            SelectBlit(blt, 7, gDotSize, gDotSize);
    } else {
        return Hidden((_tBlit_2 *)blt, 1);
    }
    UpdateBlit(blt);
    return 1;
}

static int animFlag(_tBlit *blt)
{
    int x, y, width, height, w, h;
    if (Hidden((_tBlit_2 *)blt, gNumWindows > 4 || !(gGameState & 0x400000) || gNumLapsLeft > 1 || gRace == 6 || gRace == 4 || gRace == 5 || !gShowTrackMap))
        return 1;
    width = gPathMax[0] - gPathMin[0];
    height = gPathMax[2] - gPathMin[2];
    if (width > height) {
        w = kMapSize - 8;
        h = height * (kMapSize - 8) / width;
    } else {
        h = kMapSize - 8;
        w = width * (kMapSize - 8) / height;
    }
    x = mapPos[gNumWindows - 1][0] + ((kMapSize - 8 - w) / 2 + 4 - kMapSize / 2) / 2;
    y = mapPos[gNumWindows - 1][1] + ((kMapSize - 8 - h) / 2 + 4 - kMapSize / 2) / 2.0f;
    if (gMirrorWaves)
        x += w * (gCheckpoints.points[gCheckpoints.finishPt].pos[0] - gPathMin[0]) / width / 2.0f;
    else
        x += (w - 1 - w * (gCheckpoints.points[gCheckpoints.finishPt].pos[0] - gPathMin[0]) / width) / 2.0f;
    y += h * (gCheckpoints.points[gCheckpoints.finishPt].pos[2] - gPathMin[2]) / height / 2.0f;
    blt->x = x - (blt->width / 2) / 2;
    blt->y = y - (blt->width / 2) / 2.0f;
    if (Hidden((_tBlit_2 *)blt, gRace == 1 || gNumLapsLeft > 1 || (gGameState & 8))) {
        if (gRace != 1 && !(gGameState & 8)) {
            if (gNumWindows < 4)
                HAL_SetFont(6);
            else
                HAL_SetFont(7);
            HAL_SetTextDepth(0.0f);
            TB_SetJust(1, 1);
            TB_SetTextColor(0);
            TB_Printf(x + 1, y + 1, "%d", gNumLapsLeft);
            TB_SetTextColor(11);
            TB_Printf(x, y, "%d", gNumLapsLeft);
            TB_SetJust(0, 3);
            HAL_SetTextDepth(-1.0f);
        }
        return 1;
    }
    if (gMirrorWaves)
        blt->flip = 1;
    blt->alpha = 128;
    if (!gPauseState)
        blt->animDTA -= 1000.0f * gFrameSeconds;
    if (blt->animDTA <= 0) {
        blt->animDTA = 100;
        blt->animID = (blt->animID + 1) % 5;
    }
    SelectBlit(blt, blt->animID, blt->width, blt->width);
    return 1;
}

static int animRadarBlip(_tBlit *blt)
{
    float scale = 1.0f / 480.0f;
    int window = (blt->animID >> 8) & 15;
    short slot;
    float rpos[3], lpos[3];
    tCarData *gc;
    tModelData *m;
    int blipWidth = blt->width / 8.0f / 2.0f;
    int blipHeight = blt->height / 2.0f;
    if (window >= gNumWindows || (gGameState & 8) || gNumCars < 2) {
        blt->animFunc = 0;
        return Hidden((_tBlit_2 *)blt, 1);
    }
    slot = blt->animID & 15;
    m = &model[slot];
    gc = &gGameCar[slot];
    if (Hidden((_tBlit_2 *)blt, !gShowRadar || !(gGameState & 0x400000) || gGameCar[window].place_locked == 1))
        return 1;
    if (!m->in_game) {
        blt->animFunc = 0;
        return Hidden((_tBlit_2 *)blt, 1);
    }
    if (Hidden((_tBlit_2 *)blt, slot != window && !AreWeSolid(slot)))
        return 1;
    lpos[0] = gc->dr_pos[0] - gGameCar[window].dr_pos[0];
    lpos[1] = gc->dr_pos[1] - gGameCar[window].dr_pos[1];
    lpos[2] = gc->dr_pos[2] - gGameCar[window].dr_pos[2];
    BodyVector(lpos, rpos, gGameCar[window].dr_uvs);
    blt->x = radarWidth / 2.0f;
    blt->y = 16;
    if (gMirrorWaves)
        blt->x += radarWidth * (-rpos[0] * scale);
    else
        blt->x += radarWidth * (rpos[0] * scale);
    blt->y += radarHeight * (rpos[2] * -scale);
    if (Hidden((_tBlit_2 *)blt, blt->x < blipWidth || blt->x > radarWidth - blipWidth || blt->y < blipHeight || blt->y > radarHeight - blipHeight))
        return 1;
    blt->x += radarPos[gNumWindows - 1][window][0];
    blt->y += radarPos[gNumWindows - 1][window][1];
    if (m->drone_type == 2)
        SelectBlit(blt, m->slot, blt->width / 8.0f, blt->height);
    else if (m->drone_type == 1)
        SelectBlit(blt, 5, blt->width / 8.0f, blt->height);
    blt->x -= blipWidth / 2;
    blt->y -= blipHeight / 2;
    UpdateBlit(blt);
    return 1;
}

static int animTrackMap(_tBlit *blt)
{
    int width, height, w, h, xOffset, yOffset;
    int x, y, x1, y1, x2, p, i;
    anon_name_width_38 *ps2tex;
    unsigned char *data;
    _GXTexObj *pTexObj;
    if (Hidden((_tBlit_2 *)blt, gNumWindows > 4 || !(gGameState & 0x400000) || !gShowTrackMap))
        return 1;
    if (!(blt->animID & 16))
        return 1;
    if (gNumWindows == 4)
        RenameBlit(blt, "MAP_MD", 0);
    kMapSize = blt->width;
    if (blt->animID & 16) {
        blt->animID &= 15;
        blt->x = mapPos[gNumWindows - 1][0] - (kMapSize / 2) / 2;
        blt->y = mapPos[gNumWindows - 1][1] - (kMapSize / 2) / 2.0f;
        UpdateBlit(blt);
    }
    width = gPathMax[0] - gPathMin[0];
    height = gPathMax[2] - gPathMin[2];
    if (width > height) {
        w = kMapSize - 8;
        h = height * (kMapSize - 8) / width;
    } else {
        w = kMapSize - 8;
        h = kMapSize - 8;
    }
    xOffset = (kMapSize - 8 - w) / 2 + 4;
    yOffset = (kMapSize - 8 - h) / 2 + 4;
    ps2tex = (anon_name_width_38 *)blt->texture->strip;
    if (ps2tex) {
        if (dataMap) {
            DeallocateMemory(dataMap);
            DeallocateMemory(dataObj);
        }
        pTexObj = ngcCreateTextureMapRGBA8(kMapSize * 4, kMapSize * 4, &data);
        ps2tex->texBase = (char *)pTexObj;
        dataObj = pTexObj;
        dataMap = data;
        for (y = 0; y < kMapSize; y++) {
        }
        for (p = 0; p < gResPath.numPts; p++) {
            if (p >= gResPath.numPts) {
                for (i = 0; i < gResPath.numAlt; i++) {
                    if (gResPathPtList + p >= gResPath.alt[i].pts && gResPathPtList + p < gResPath.alt[i].pts + gResPath.alt[i].numPts)
                        break;
                }
                if (!gResPath.alt[i].resurrect) {
                    p += gResPath.alt[i].numPts - 1;
                    continue;
                }
            }
            x = xOffset + w * (gResPath.pts[p][0] - gPathMin[0]) / width;
            y = yOffset + h * (gResPath.pts[p][2] - gPathMin[2]) / height;
            for (y1 = y - 3; y1 <= y + 3; y1++) {
                for (x1 = x - 3; x1 <= x + 3; x1++) {
                    if (x1 >= 0 && x1 < kMapSize && y1 >= 0 && y1 < kMapSize) {
                        x2 = gMirrorWaves ? x1 : kMapSize - 1 - x1;
                        ngcWriteTexelRGBA8(data, kMapSize * 4, kMapSize * 4, x2 * 4, y1 * 4, 255, 255, 255, 255);
                    }
                }
            }
        }
        ngcFlushTexObjDataRGBA8(pTexObj, data, kMapSize * 4, kMapSize * 4);
        return 1;
    }
}

int animCoins(void)
{
    short screenPos[2];
    short player, coin;
    float xoffset, yoffset;
    screenPos[0] = 0;
    screenPos[1] = 0;
    HAL_SetTextDepth(0.0f);
    if (gNumWindows == 1)
        HAL_SetFont(10);
    else
        HAL_SetFont(12);
    TB_SetJust(0, 1);
    for (player = 0; player < gNumWindows; player++) {
        for (coin = 0; coin < 2; coin++) {
            if (gPlayerCoin[player][coin].idx == -1) {
                char *playerCoinName[2] = {"GOLDCOINL4_LOD1", "SILVERCOINL4_LO"};
                gPlayerCoin[player][coin].idx = MBOX_NewObject(MBOX_FindObject_WORK(playerCoinName[coin], 0, gNumModels - 1, 1), &gPlayerCoin[player][coin].mat, -1, 0);
                MBOX_HideObject(gPlayerCoin[player][coin].idx, 1, 15);
                ObjInstanceList[gPlayerCoin[player][coin].idx].w = 10.0f;
            }
            ScaleUV(gCamera[player].mat.mat3.uvs, gPlayerCoin[player][coin].mat.mat3.uvs, gNumPlayers == 1 ? 0.03f : gNumPlayers == 2 ? 0.03f : 0.04f);
            YawUV(hudObjAngle, gPlayerCoin[player][coin].mat.mat3.uvs);
            if (gRace == kStunt) {
            if (gNumWindows == 1) {
                if (coin == 0) {
                    xoffset = 0.0f; yoffset = -8.0f;
                } else {
                    xoffset = 0.0f; yoffset = 48.0f;
                }
            }
            else if (gNumWindows == 2) {
                if (player == 0) {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = -8.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 48.0f;
                    }
                }                 else {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = -8.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 164.0f;
                    }
                } 
            }
            else if (gNumWindows == 3) {
                if (player == 0) {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = 0.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 48.0f;
                    }
                }                 else if (player == 1) {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = 0.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 48.0f;
                    }
                }                 else {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = 0.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 160.0f;
                    }
                } 
            }
            else {
                if (player == 0) {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = 0.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 48.0f;
                    }
                }                 else if (player == 1) {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = 0.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 48.0f;
                    }
                }                 else if (player == 2) {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = 0.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 160.0f;
                    }
                }                 else {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = 0.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 160.0f;
                    }
                } 
            }
            screenPos[0] = (xoffset + coinStuntPos[gNumWindows - 1][player][0]) * 2.0f;
            screenPos[1] = (yoffset + (coinStuntPos[gNumWindows - 1][player][1] + coin * (gNumWindows == 1 ? 15 : gNumWindows == 2 ? 12 : 8))) * 2.0f;
            } else if (gRace == kGhost) {
                if (coin == 0) {
                    xoffset = 0.0f; yoffset = -4.0f;
                } else {
                    xoffset = 0.0f; yoffset = 64.0f;
                }
            screenPos[0] = (xoffset + coinPos[gNumWindows - 1][player][0]) * 2.0f;
            screenPos[1] = (yoffset + (coinPos[gNumWindows - 1][player][1] + coin * (gNumWindows == 1 ? 15 : gNumWindows == 2 ? 12 : 8))) * 2.0f;
            } else if (gRace == kCircuit) {
                if (coin == 0) {
                    xoffset = 0.0f; yoffset = -4.0f;
                } else {
                    xoffset = 0.0f; yoffset = 64.0f;
                }
            screenPos[0] = (xoffset + coinPos[gNumWindows - 1][player][0]) * 2.0f;
            screenPos[1] = (yoffset + (coinPos[gNumWindows - 1][player][1] + coin * (gNumWindows == 1 ? 15 : gNumWindows == 2 ? 12 : 8))) * 2.0f;
            } else {
            if (gNumWindows == 1) {
                if (coin == 0) {
                    xoffset = 0.0f; yoffset = -4.0f;
                } else {
                    xoffset = 0.0f; yoffset = 64.0f;
                }
            }
            else if (gNumWindows == 2) {
                if (player == 0) {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = -8.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 52.0f;
                    }
                }                 else {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = -8.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 164.0f;
                    }
                } 
            }
            else if (gNumWindows == 3) {
                if (player == 0) {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = 0.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 48.0f;
                    }
                }                 else if (player == 1) {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = 0.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 48.0f;
                    }
                }                 else {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = 0.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 160.0f;
                    }
                } 
            }
            else {
                if (player == 0) {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = 0.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 48.0f;
                    }
                }                 else if (player == 1) {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = 0.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 48.0f;
                    }
                }                 else if (player == 2) {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = 0.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 184.0f;
                    }
                }                 else {
                    if (coin == 0) {
                        xoffset = 0.0f; yoffset = 0.0f;
                    } else {
                        xoffset = 0.0f; yoffset = 184.0f;
                    }
                } 
            }
            screenPos[0] = (xoffset + coinPos[gNumWindows - 1][player][0]) * 2.0f;
            screenPos[1] = (yoffset + (coinPos[gNumWindows - 1][player][1] + coin * (gNumWindows == 1 ? 15 : gNumWindows == 2 ? 12 : 8))) * 2.0f;
            }
            if (isInLowRes())
                screenPos[1] /= 2.0f;
            ZOID_WindowTo3D(player, screenPos, &gCamera[player].mat, 2.0f, gPlayerCoin[player][coin].mat.mat3.pos);
            if (gOptions[11] && (gGameState & 0x400000) && gGameCar[player].place_locked != 1)
                MBOX_ShowObject(gPlayerCoin[player][coin].idx, 0, whichCam[player]);
            else
                MBOX_HideObject(gPlayerCoin[player][coin].idx, 1, 15);
        }
    }
    TB_SetJust(0, 3);
    HAL_SetTextDepth(-1.0f);
    return 1;
}
