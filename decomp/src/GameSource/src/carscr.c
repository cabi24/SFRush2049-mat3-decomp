/* carscr.c - car selection screen: component options, stat bars, grid map. Status: Matching.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\carscr.c; assembly: build/2049LE/asm/GameSource/src/carscr.s */
#include "rush/rush_protos.h"
#define MSL_NO_SQRTF
#include "msl_math_inline.h"


#define CAR_MAX(a,b) ((a) > (b) ? (a) : (b))
#define CAR_MIN(a,b) ((a) < (b) ? (a) : (b))

enum { kCarTransmission = 1, kCarHandling = 2, kCarEngine = 3, kCarTire = 4, kCarFrame = 5 };

static void showCarSelect(void);
static void _InitScreen(void);
static void _UninitScreen(void);
static void setGhosts(void);
static void updateCarComponents(int p);
static void clearCarComponents(int p);
static int animNewSlider(_tBlit_2 *blt);
static int animStatBars(_tBlit_2 *blt);
static int animStatText(void);
static int animText(void);
static void animButtonText(int p);
static void drawOptions(int p);
static void calcValidOptions(int p);
static int validOption(int p, int opt);
static void calcStats(int p);
static void createGridMap(void);
static void releaseGridMap(void);
static void showGridMap(int visible);
static void initButtons(signed char p);
static void uninitButtons(signed char p);
static void updateButtons(signed char p);
static tGhostFile **getNextGhost(tGhostFile **currGhost, signed char whichWay);








static char *buttonNames[9] = {
    "BUTTON_S2G1", "BUTTON_S2G1", "BUTTON_S2G1", "CURSORG1", "AB_BUTTONG1", "ARROW_BGG1",
    "ARROW_FGG1", "BAR_BGG1", "BAR_FGG1"
};
static tMenuButton buttonInfo[4][40] = {
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
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
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
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
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
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
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
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 1, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
        { 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 }
    }
};
static anon_x_y_18 gPlayerRects[4][4] = {
    0, 20, 320, 200, 160, 120, 0, 0, 320, 220, 160, 110, 0, 0, 320, 220, 160, 110, 0, 0, 320, 220,
    160, 110, 0, 20, 320, 100, 160, 70, 0, 120, 320, 100, 160, 170, 0, 0, 320, 220, 160, 110, 0, 0,
    320, 220, 160, 110, 0, 20, 160, 100, 80, 70, 160, 20, 160, 100, 240, 70, 0, 120, 160, 100, 80,
    170, 0, 0, 320, 220, 160, 110, 0, 20, 160, 100, 80, 70, 160, 20, 160, 100, 240, 70, 0, 120,
    160, 100, 80, 170, 160, 120, 160, 100, 240, 170
};
int gTurntableIdx[4][2] = { -1, -1, -1, -1, -1, -1, -1, -1 };
static char *carComponentNames[5] = { "ENGINE%02dG1", "FRAME%02dG1", "SHOCK%02dG1", "TIRE%02dG1", "TRANS%02dG1" };
static signed short componentToName[11] = { 0, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4 };
static const tBlitDef mb[26] = {
    "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe, (void *)&animNewSlider, 0xb0000, "SLIDER", 0,
    0, -1, -1, -1, -1, -1, -1, 0, 0xfe, (void *)&animNewSlider, 0xb0001, "SLIDER", 0, 0, -1, -1,
    -1, -1, -1, -1, 0, 0xfe, (void *)&animNewSlider, 0xc0000, "SLIDER", 0, 0, -1, -1, -1, -1, -1,
    -1, 0, 0xfe, (void *)&animNewSlider, 0xc0001, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0xb0010, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0xb0011, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0xc0010, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0xc0011, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0xb0020, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0xb0021, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0xc0020, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0xc0021, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0xb0030, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0xb0031, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0xc0030, "SLIDER", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animNewSlider, 0xc0031, "CARSTATBARS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animStatBars, 0, "CARSTATBARS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animStatBars, 1, "CARSTATBARS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animStatBars, 0x10, "CARSTATBARS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animStatBars, 0x11, "CARSTATBARS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animStatBars, 0x20, "CARSTATBARS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animStatBars, 0x21, "CARSTATBARS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animStatBars, 0x30, "CARSTATBARS_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xfe,
    (void *)&animStatBars, 0x31, (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0,
    (void *)&animStatText, 0, (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0,
    (void *)&animText, 0
};
anon_idx_level_34 gCarComponents[1][11] = {
    -1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -1, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -1, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, -1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, -1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -1,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -1, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1, -1, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
}
;
static anon_a_l_24 compLight[2] = {
    192, 192, 192, 0, 192, 192, 192, 255, 0.0f, 1.0f, 0.0f, 0, 0, 0, 255, 1.0f, 0.0f, 0.0f, 255,
    255, 0, 0, 192, 192, 192, 255, 0.0f, 1.0f, 0.0f, 0, 0, 0, 255, 1.0f, 0.0f, 0.0f
}
;
static int gActiveOption[4] = { 0 }; /* explicit `= { 0 }`: the target keeps this object in initialised data (.data/.sdata), not .bss */
static float gTransStats[8][4] = {
    0.95f, 0.85f, 1.0f, 1.0f, 1.0f, 0.85f, 1.0f, 1.0f, 0.95f, 0.9f, 1.0f, 1.0f, 1.0f, 0.9f, 1.0f,
    1.0f, 0.95f, 0.95f, 1.0f, 1.0f, 1.0f, 0.95f, 1.0f, 1.0f, 0.85f, 1.0f, 1.0f, 1.0f, 0.9f, 1.0f,
    1.0f, 1.0f
}
;
static float gHandlingStats[3][4] = { 0.9f, 0.9f, 1.0f, 1.0f, 0.95f, 0.95f, 0.9f, 1.0f, 1.0f, 1.0f, 0.8f, 1.0f };
static float gEngineStats[9][4] = {
    0.71f, 0.7f, 1.0f, 1.0f, 0.7f, 0.71f, 1.0f, 1.0f, 0.71f, 0.71f, 1.0f, 1.0f, 0.75f, 0.75f, 1.0f,
    1.0f, 0.8f, 0.8f, 1.0f, 1.0f, 0.85f, 0.85f, 1.0f, 1.0f, 0.9f, 0.9f, 1.0f, 1.0f, 0.95f, 0.95f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
}
;
static float gTireStats[5][4] = {
    0.9f, 0.9f, 0.9f, 1.0f, 0.95f, 0.95f, 0.95f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.85f, 0.85f, 0.95f,
    1.0f, 0.8f, 0.8f, 0.9f, 1.0f
}
;
static float gFrameStats[6][4] = {
    0.9f, 0.9f, 0.9f, 0.33f, 0.85f, 0.85f, 0.95f, 0.69f, 0.8f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 0.9f,
    0.33f, 0.95f, 0.95f, 0.95f, 0.69f, 0.9f, 0.9f, 1.0f, 1.0f
}
;
static signed char gCarOrder[13] = { 2, 3, 0, 1, 7, 4, 5, 6, 8, 9, 10, 11, 12 };
static float gButtonZoom = 1.0f;
static float gButtonScale = 0.075f;
static float gButtonSpace = 40.0f;
static float buttonX = -8.0f;
static float maxY = -10.8f;
static float minY = -16.8f;
static float buttonZ = 30.0f;
static float cursorZ = 22.5f;
int gCarPartFrame = -1;
static int gridCarIdx = -1;
static signed short gCarSheen[4] = { 0 }; /* explicit `= { 0 }`: the target keeps this object in initialised data (.data/.sdata), not .bss */
static signed char gThisCar[4];
static signed char gThisCarIndex[4];
static signed char gNumGhosts;
static tGhostFile **limboGhosts;
static signed short gLastCar[4];
static signed short gNumCarsActive[4];
static signed char gCarLocked[4];
static signed char allLocked;
static int wait;
static signed char gActiveOptionEnabled[4];
static signed char gActiveZ[4];
static _tBlit_2 *blit;
static signed char inited;
static int gridVisible;
static float gCameraMatrix[4][4];
static anon_texidx_flags_c savedLOD;
static anon_texidx_flags_c gridLOD;
static int buttonsInited[4];
static float gStats[4][4];
tUVMatrix *gCarMatrices[48];
tUVMatrix gTurntableMat[4][2];
static anon_alpha_angle_c cars[4][13];
static int gFirstOption[4];
static int gHornSnd[4];
static int buttonsJustInited[4];
static signed char gCars[4][13];
static int gOpts[4][17];
static int gNumOptions[4];
static tBlitSet *pane[4];

/* 0x8008d384, size 0x1a58 */
void updateCarScreen(void)
{
    signed short i;
    signed short p;
    int sound = 1;
    tGhostFile **ghosth;
    tPlayerProfile **player;
    tGhostFile *ghost;
    tGhostFile **nextGhost;
    limboGhosts = gLimboGhosts + (gTrackNum + (gBackwards ? 6 : 0)) * 3;
    if (gSkipToGameState != 1) {
        if (gSkipToGameState != gGameState) { setGhosts(); setGameState(0x40000); return; }
        gSkipToGameState = 1;
    }
    for (i = 0; i < gNumPlayers; i++) {
        if (gPlayers[i].save != &gJustPlayers[i]) {
            player = gPlayerFiles.head;
            while (player) {
                if (gPlayers[i].save == player) break;
                player = (*player)->ll.next;
            }
            if (!player) { _UninitScreen(); setGameState(32); return; }
        }
    }
    for (i = 1; i < 4; i++) {
        for (ghosth = gGhostFiles.head; ghosth; ghosth = nextGhost) {
            ghost = *ghosth;
            nextGhost = ghost->ll.next;
            if ((!ghost->fileh || PS_PakInserted((*ghost->fileh)->pak)) && ghosth == gGhosts[i]) break;
        }
        if (!ghosth) {
            for (p = 0; p < 3; p++) {
                ghosth = limboGhosts + p;
                if (ghosth == gGhosts[i]) break;
                ghosth = 0;
            }
        }
        gGhosts[i] = ghosth;
    }
    if (!inited) { _InitScreen(); showCarSelect(); leEndLoadingEffect(); }
    else showCarSelect();
    if (gRace == 2) {
        for (i = 0; i < gNumPlayers; i++) {
            if ((*gPlayers[i].save)->fileh && PS_PakChanged((*(*gPlayers[i].save)->fileh)->pak)) {
                _UninitScreen(); setGameState(32); wait = 0; return;
            }
        }
    }
    if (gSoftReset) { _UninitScreen(); setGameState(4); wait = 0; return; }
    wait++;
    for (p = 0; p < gNumPlayers; p++) {
        if (!allLocked && (gPlayers[p].edges & 4)) {
            if (gCarLocked[p] == 1) gCarLocked[p] = 0;
            else { AudPlayEffect(1,0,1,0); _UninitScreen(); setGameState(128); wait = 0; return; }
        }
        if (gCarLocked[p]) {
            int notReadyCount;
            int notReadyIdx;
            int pIdx;
            signed short x, y, w, h;
            signed short screenPos[2];
            float msgPos[3] = {0.0f, -120.0f, 300.0f};
            if (gNumWindows == 1) HAL_SetFont(13); else HAL_SetFont(12);
            ZOID_ProjectWindow(p, msgPos, &gCamera[p].mat, 0, screenPos);
            x = screenPos[0] / 2;
            y = screenPos[1] / 2.0f - (signed short)TB_GetFontLineHeight();
            notReadyCount = 0;
            for (pIdx = 0; pIdx < gNumPlayers; pIdx++) {
                if (pIdx != p && !gCarLocked[pIdx]) { notReadyIdx = pIdx; notReadyCount++; }
            }
            if (!notReadyCount) {
                y += TB_GetFontLineHeight();
                h = TB_GetFontLineHeight();
                if (gRace == 5) w = HAL_GetTextWidth(gText.single[58], -1);
                else if (gRace == 6) w = HAL_GetTextWidth(gText.single[57], -1);
                else if (gRace == 4) w = HAL_GetTextWidth(gText.single[56], -1);
                else w = HAL_GetTextWidth(gText.single[55], -1);
            } else {
                w = HAL_GetTextWidth(gText.single[161], -1);
                h = (signed short)TB_GetFontLineHeight() * 2;
                if (notReadyCount == 1) w = CAR_MAX(w, HAL_GetTextWidth((*gPlayers[notReadyIdx].save)->name, -1));
                else w = CAR_MAX(w, HAL_GetTextWidth(gText.single[31], -1));
            }
            y -= (signed short)TB_GetFontLineHeight() / 2;
            ChangePaneSize(pane[p], x - w/2 - 4, y - 4, x + w/2 + 4, y + h + 4);
            updateButtons(p);
            continue;
        }
        ChangePaneSize(pane[p], -20, 0, -10, 10);
        if (gPlayers[p].edges & 3) {
            PlayStartABSound(gPlayers[p].edges);
            gLink[p].flags = 160;
            gCarLocked[p] = 1;
            gLink[p].car = gThisCar[p];
            if (gRace == 6 && gCheatState[13] == 0) gLink[p].PaintJob = gCarTeamColor[gCarTeam[p]];
            else gLink[p].PaintJob = gCarPaintJob[p+1][gThisCar[p]];
            wait = 0;
        }
        if (gPlayers[p].autoEdges & 0x400) {
            AudPlayEffect(8,0,1,0);
            gActiveOption[p]--;
            if (gActiveOption[p] < 0) gActiveOption[p] = gNumOptions[p]-1;
        } else if (gPlayers[p].autoEdges & 0x800) {
            AudPlayEffect(2,0,1,0);
            gActiveOption[p]++;
            if (gActiveOption[p] > gNumOptions[p]-1) gActiveOption[p] = 0;
        } else if (gPlayers[p].autoEdges & 0x3080) {
            int d = (gPlayers[p].autoEdges & 0x1000) ? -1 : !!(gPlayers[p].autoEdges & 0x2000);
            if (!gActiveOptionEnabled[p] || (!d && !gActiveZ[p])) {
                AudPlayEffect(4,0,1,0);
                goto after_options;
            }
            switch (gOpts[p][gActiveOption[p]]) {
            case 0:
                if (d) {
                    do {
                        gThisCarIndex[p] += d;
                        if (gThisCarIndex[p] < 0) gThisCarIndex[p] = gNumCarsActive[p] - 1;
                        else if (gThisCarIndex[p] >= gNumCarsActive[p]) gThisCarIndex[p] = 0;
                    } while (!IsCarUnlocked(p, gCars[p][gThisCarIndex[p]]));
                }
                gThisCar[p] = gCars[p][gThisCarIndex[p]];
                if (gRace == 6) gPlayers[p].trans = 6;
                else gPlayers[p].trans = GetPlayerCarTrans(gPlayers[p].save);
                for (i = 0; i < 13; i++) gCarTrans[p+1][i] = (signed char)gPlayers[p].trans;
                gPlayers[p].trans = gPlayers[p].trans; /* no-op copy kept on purpose: the target reloads and re-stores the value here */
                calcValidOptions(p);
                buttonsJustInited[p] = 1;
                break;
            case 1:
                if (gRace != 6) {
                    if (d) {
                        do {
                            gCarTrans[p+1][gThisCar[p]] += d;
                            if (gCarTrans[p+1][gThisCar[p]] < 0) gCarTrans[p+1][gThisCar[p]] = 7;
                            else if (gCarTrans[p+1][gThisCar[p]] >= 8) gCarTrans[p+1][gThisCar[p]] = 0;
                        } while (!IsTransUnlocked(p, gCarTrans[p+1][gThisCar[p]]) || (gRace == 6 && (gCarTrans[p+1][gThisCar[p]] & 1)));
                    } else gCarTrans[p+1][gThisCar[p]] = gCarTrans[0][gThisCar[p]];
                    gPlayers[p].trans = gCarTrans[p+1][gThisCar[p]];
                    SavePlayerCarTrans(gPlayers[p].save, gPlayers[p].trans);
                } else sound = 0;
                break;
            case 2:
                if (d) {
                    gCarHandling[p+1][gThisCar[p]] += d;
                    if (gCarHandling[p+1][gThisCar[p]] < 0) gCarHandling[p+1][gThisCar[p]] = 2;
                    else if (gCarHandling[p+1][gThisCar[p]] > 2) gCarHandling[p+1][gThisCar[p]] = 0;
                } else gCarHandling[p+1][gThisCar[p]] = gCarHandling[0][gThisCar[p]];
                SavePlayerCarSetting(gPlayers[p].save, gThisCar[p], 0, gCarHandling[p+1][gThisCar[p]]);
                AudPlayEffect(gCarHandling[p+1][gThisCar[p]] == 0 ? 50 : (gCarHandling[p+1][gThisCar[p]] == 1 ? 48 : 49), 0, 1, 0);
                sound = 0;
                break;
            case 3:
                if (d) {
                    do {
                        gCarEngines[p+1][gThisCar[p]] += d;
                        if (gCarEngines[p+1][gThisCar[p]] < 0) gCarEngines[p+1][gThisCar[p]] = 8;
                        else if (gCarEngines[p+1][gThisCar[p]] > 8) gCarEngines[p+1][gThisCar[p]] = 0;
                    } while (!IsEngineUnlocked(p, gCarEngines[p+1][gThisCar[p]]));
                } else gCarEngines[p+1][gThisCar[p]] = gCarEngines[0][gThisCar[p]];
                SavePlayerCarSetting(gPlayers[p].save, gThisCar[p], 1, gCarEngines[p+1][gThisCar[p]]);
                break;
            case 4:
                if (d) {
                    do {
                        gCarTires[p+1][gThisCar[p]] += d;
                        if (gCarTires[p+1][gThisCar[p]] < 0) gCarTires[p+1][gThisCar[p]] = 4;
                        else if (gCarTires[p+1][gThisCar[p]] > 4) gCarTires[p+1][gThisCar[p]] = 0;
                    } while (!IsTiresUnlocked(p, gCarTires[p+1][gThisCar[p]]));
                } else gCarTires[p+1][gThisCar[p]] = gCarTires[0][gThisCar[p]];
                SavePlayerCarSetting(gPlayers[p].save, gThisCar[p], 2, gCarTires[p+1][gThisCar[p]]);
                break;
            case 5:
                if (d) {
                    do {
                        gCarFrame[p+1][gThisCar[p]] += d;
                        if (gCarFrame[p+1][gThisCar[p]] < 0) gCarFrame[p+1][gThisCar[p]] = 5;
                        else if (gCarFrame[p+1][gThisCar[p]] > 5) gCarFrame[p+1][gThisCar[p]] = 0;
                    } while (!IsFrameUnlocked(p, gCarFrame[p+1][gThisCar[p]]));
                } else gCarFrame[p+1][gThisCar[p]] = gCarFrame[0][gThisCar[p]];
                SavePlayerCarSetting(gPlayers[p].save, gThisCar[p], 3, gCarFrame[p+1][gThisCar[p]]);
                break;
            case 6:
                if (d) {
                    gCarWings[p+1][gThisCar[p]] += d;
                    if (gCarWings[p+1][gThisCar[p]] < 0) gCarWings[p+1][gThisCar[p]] = (gRace != kStunt ? 1 : 2);
                    else if (gCarWings[p+1][gThisCar[p]] > (gRace != kStunt ? 1 : 2)) gCarWings[p+1][gThisCar[p]] = 0;
                } else gCarWings[p+1][gThisCar[p]] = gCarWings[0][gThisCar[p]];
                SavePlayerCarSetting(gPlayers[p].save, gThisCar[p], 4, gCarWings[p+1][gThisCar[p]]);
                break;
            case 7:
                if (d) {
                    gCarPaintJob[p+1][gThisCar[p]] += d;
                    if (gCarPaintJob[p+1][gThisCar[p]] < 0) gCarPaintJob[p+1][gThisCar[p]] = (gNumCarPaintJobs[gThisCar[p]] - 1);
                    else if (gCarPaintJob[p+1][gThisCar[p]] >= gNumCarPaintJobs[gThisCar[p]]) gCarPaintJob[p+1][gThisCar[p]] = 0;
                } else gCarPaintJob[p+1][gThisCar[p]] = gCarPaintJob[0][gThisCar[p]];
                SavePlayerCarSetting(gPlayers[p].save, gThisCar[p], 5, gCarPaintJob[p+1][gThisCar[p]]);
                break;
            case 8:
                if (d) {
                    int teamFull = 1;
                    do {
                        gCarTeam[p] += d;
                        if (gCarTeam[p] < 0) gCarTeam[p] = 3;
                        else if (gCarTeam[p] > 3) gCarTeam[p] = 0;
                        for (i = 0; i < gNumPlayers; i++) {
                            if (i != p && gCarTeam[i] != gCarTeam[p]) teamFull = 0;
                        }
                    } while (teamFull == 1);
                } else gCarTeam[p] = p;
                break;
            case 9:
                if (d) {
                    gTireRims[p+1][gThisCar[p]] += d;
                    if (gTireRims[p+1][gThisCar[p]] < 0) gTireRims[p+1][gThisCar[p]] = 23;
                    else if (gTireRims[p+1][gThisCar[p]] > 23) gTireRims[p+1][gThisCar[p]] = 0;
                } else gTireRims[p+1][gThisCar[p]] = gTireRims[0][gThisCar[p]];
                SavePlayerCarSetting(gPlayers[p].save, gThisCar[p], 6, gTireRims[p+1][gThisCar[p]]);
                break;
            case 10:
                if (d) {
                    gCarHorn[p+1][gThisCar[p]] += d;
                    if (gCarHorn[p+1][gThisCar[p]] < 0) gCarHorn[p+1][gThisCar[p]] = 9;
                    else if (gCarHorn[p+1][gThisCar[p]] > 9) gCarHorn[p+1][gThisCar[p]] = 0;
                } else gCarHorn[p+1][gThisCar[p]] = gCarHorn[0][gThisCar[p]];
                AudStopEffect(gHornSnd[p]);
                gHornSnd[p] = AudPlayEffect(gCarHorn[p+1][gThisCar[p]] + 10, 0, 1, 0);
                sound = 0;
                SavePlayerCarSetting(gPlayers[p].save, gThisCar[p], 7, gCarHorn[p+1][gThisCar[p]]);
                break;
            case 11:
                if (d) {
                    gFrontTireScale[p+1][gThisCar[p]] += 0.05f * d;
                    if (gFrontTireScale[p+1][gThisCar[p]] < 0.74f) gFrontTireScale[p+1][gThisCar[p]] = 1.5f;
                    else if (gFrontTireScale[p+1][gThisCar[p]] > 1.6f) gFrontTireScale[p+1][gThisCar[p]] = 0.75f;
                } else gFrontTireScale[p+1][gThisCar[p]] = gFrontTireScale[0][gThisCar[p]];
                SavePlayerCarSetting(gPlayers[p].save, gThisCar[p], 8, (100.0f * gFrontTireScale[p+1][gThisCar[p]] - 75.0f));
                break;
            case 12:
                if (d) {
                    gRearTireScale[p+1][gThisCar[p]] += 0.05f * d;
                    if (gRearTireScale[p+1][gThisCar[p]] < 0.74f) gRearTireScale[p+1][gThisCar[p]] = 1.5f;
                    else if (gRearTireScale[p+1][gThisCar[p]] > 1.5f) gRearTireScale[p+1][gThisCar[p]] = 0.75f;
                } else gRearTireScale[p+1][gThisCar[p]] = gRearTireScale[0][gThisCar[p]];
                SavePlayerCarSetting(gPlayers[p].save, gThisCar[p], 9, (100.0f * gRearTireScale[p+1][gThisCar[p]] - 75.0f));
                break;
            case 13:
                if (d) {
                    gCarSheen[p] += d;
                    if (gCarSheen[p] < 0) gCarSheen[p] = 10;
                    else if (gCarSheen[p] > 10) gCarSheen[p] = 0;
                } else gCarSheen[p] = 0;
                SetSheen(gThisCar[p], p, gCarSheen[p]);
                break;
            case 14: gGhosts[1] = getNextGhost(gGhosts[1], d); break;
            case 15: gGhosts[2] = getNextGhost(gGhosts[2], d); break;
            case 16: gGhosts[3] = getNextGhost(gGhosts[3], d); break;
            }
            if (sound) PlayLRSound(gPlayers[p].autoEdges);
        }
        if (gFirstOption[p] > gActiveOption[p] - 1) gFirstOption[p] = gActiveOption[p] - 1;
        if (gFirstOption[p] < gActiveOption[p] - 1) gFirstOption[p] = gActiveOption[p] - 1;
        if (gFirstOption[p] > gNumOptions[p] - 3) gFirstOption[p] = gNumOptions[p] - 3;
        if (gFirstOption[p] < 1) gFirstOption[p] = 1;
        gActiveOptionEnabled[p] = 1;
after_options:
        if (gThisCar[p] != gLastCar[p]) {
            if (gLastCar[p] >= 0) SavePlayerCar(gPlayers[p].save, gThisCar[p]);
            gLastCar[p] = gThisCar[p];
        }
        if (gRace == 2) {
            int ghostIdx;
            for (ghostIdx = 1; ghostIdx < gNumGhosts + 1; ghostIdx++) {
                if (pane[ghostIdx]) {
                    float pos[3];
                    signed short screenPos[2];
                    signed short x, y, w, h;
                    pos[0] = buttonInfo[0][34].mat.mat3.pos[0];
                    pos[1] = buttonInfo[0][34].mat.mat3.pos[1];
                    pos[2] = buttonInfo[0][34].mat.mat3.pos[2];
                    pos[1] = -pos[1] / 2.0f;
                    ZOID_ProjectWindow(ghostIdx, pos, &gCamera[ghostIdx].mat, 0, screenPos);
                    x = screenPos[0] / 2;
                    y = screenPos[1] / 2.0f;
                    HAL_SetFont(12);
                    if (!gGhosts[ghostIdx]) {
                        w = HAL_GetTextWidth(gText.single[236], -1);
                        h = TB_GetFontLineHeight();
                    } else {
                        w = HAL_GetTextWidth((*gGhosts[ghostIdx])->name, -1);
                        w = CAR_MAX(w, HAL_GetTextWidth("TIME 0:00.000", -1));
                        h = (signed short)TB_GetFontLineHeight() * 2;
                    }
                    y -= (signed short)TB_GetFontLineHeight() / 2;
                    ChangePaneSize(pane[ghostIdx], x - w/2 - 4, y - 4, x + w/2 + 4, y + h + 4);
                }
            }
        }
        updateButtons(p);
        calcStats(p);
    }
    allLocked = 1;
    for (i = 0; i < gNumPlayers; i++) if (!gCarLocked[i]) allLocked = 0;
    if (allLocked && wait >= 10) {
        AudSetMusicVol(0.0f, 3.0f);
        setGameState(0x40000);
        _UninitScreen();
        if (gRace == 3) ContinueCircuit();
        if (gRace == 2) {
            for (ghosth = gGhostFiles.head; ghosth; ghosth = (*ghosth)->ll.next) PurgeGhostData(ghosth);
            for (i = 1; i < gNumGhosts; i++) {
                if (!gGhosts[i]) { gGhosts[i] = gGhosts[i+1]; gGhosts[i+1] = 0; }
            }
            for (i = 1; i < gNumGhosts; i++) {
                if (!gGhosts[i]) { gGhosts[i] = gGhosts[i+1]; gGhosts[i+1] = 0; }
            }
        }
    }
}


/* 0x8008eddc, size 0xb28 */
static void showCarSelect(void)
{
    float uv[3][3];
    int i;
    int p;
    float carPos[3];
    unsigned char alpha;
    int framesLeft;
    signed char reflectIdx;
    float reflectUV[3][3];
    float reflectPos[3];
    int ghostIdx;
    int ghostCar;
    for (p = 0; p < gNumPlayers; p++) {
        for (i = 0; i < gNumCarsActive[p]; i++) {
            if (gThisCarIndex[p] == i) {
                cars[p][i].alpha = 1.0f;
                if (cars[p][i].alpha > 1.0f) cars[p][i].alpha = 1.0f;
            } else {
                cars[p][i].alpha = 0.0f;
                if (cars[p][i].alpha < 0.0f) cars[p][i].alpha = 0.0f;
            }
            if (gCarLocked[p]) {
                framesLeft = 10 - wait;
                if (framesLeft <= 1) cars[p][i].angle = 3.6651914f;
                else {
                    cars[p][i].angle -= 3.6651914f;
                    while (cars[p][i].angle > 3.1415927f) cars[p][i].angle -= 6.2831855f;
                    while (cars[p][i].angle <= -3.1415927f) cars[p][i].angle += 6.2831855f;
                    cars[p][i].angle -= cars[p][i].angle / (framesLeft - 1);
                    cars[p][i].angle += 3.6651914f;
                }
            } else if (gOpts[p][gActiveOption[p]] == 9) {
                cars[p][i].angle -= 1.5707964f;
                while (cars[p][i].angle > 3.1415927f) cars[p][i].angle -= 6.2831855f;
                while (cars[p][i].angle <= -3.1415927f) cars[p][i].angle += 6.2831855f;
                if (cars[p][i].angle > 1.5707964f) cars[p][i].angle += CAR_MIN(4.0f * (1.0471976f * gFrameSeconds), 3.1415927f - cars[p][i].angle);
                else if (cars[p][i].angle < -1.5707964f) cars[p][i].angle -= CAR_MIN(4.0f * (1.0471976f * gFrameSeconds), 3.1415927f + cars[p][i].angle);
                else if (cars[p][i].angle > 0.0f) cars[p][i].angle -= CAR_MIN(4.0f * (1.0471976f * gFrameSeconds), cars[p][i].angle);
                else cars[p][i].angle += CAR_MIN(4.0f * (1.0471976f * gFrameSeconds), -cars[p][i].angle);
                cars[p][i].angle += 1.5707964f;
            } else {
                cars[p][i].angle += 1.0471976f * gFrameSeconds;
                if (cars[p][i].angle >= 6.2831855f) cars[p][i].angle -= 6.2831855f;
            }
            ScaleUV(unitmatrix, uv, 2.5f);
            PitchUV(0.34906584f, uv);
            YawUV(cars[p][i].angle, uv);
            if (cars[p][i].slot % 13 == 10) {
                if (gNumWindows == 1) { carPos[0] = 0.0f; carPos[1] = 3.2f; carPos[2] = 40.5f; }
                else { carPos[0] = 0.0f; carPos[1] = 5.9f; carPos[2] = 40.5f; }
            } else {
                if (gNumWindows == 1) { carPos[0] = 0.0f; carPos[1] = 2.6f; carPos[2] = 40.5f; }
                else { carPos[0] = 0.0f; carPos[1] = 5.3f; carPos[2] = 40.5f; }
            }
            alpha = 255.0f * cars[p][i].alpha;
            if (gNumPlayers == 1 && gRace != 2 && !gCarLocked[0] &&
                (gOpts[p][gActiveOption[p]] == 1 || gOpts[p][gActiveOption[p]] == 2 ||
                 gOpts[p][gActiveOption[p]] == 3 || gOpts[p][gActiveOption[p]] == 4 ||
                 gOpts[p][gActiveOption[p]] == 5)) alpha /= 4;
            if (gRace == 6 && gCheatState[13] == 0)
                UpdateSelectCar(cars[p][i].slot, p, gCarTeamColor[gCarTeam[p]], gCarSheen[p], carPos, uv, alpha);
            else UpdateSelectCar(cars[p][i].slot, p, gCarPaintJob[p+1][cars[p][i].slot % 13], gCarSheen[p], carPos, uv, alpha);
            if (gNumPlayers < 3 && gRace != 2) {
                reflectIdx = p == 0 ? 2 : 3;
                ScaleUV(uv, reflectUV, 1.2f);
                RollUV(3.1415927f, reflectUV);
                reflectPos[0] = 1.2f * carPos[0];
                reflectPos[1] = 1.2f * carPos[1];
                reflectPos[2] = 1.2f * carPos[2];
                if (cars[p][i].slot % 13 == 10) reflectPos[1] -= 4.0f;
                else reflectPos[1] -= 2.5f;
                cars[reflectIdx][i].slot = cars[p][i].slot + 26;
                if (gRace == 6 && gCheatState[13] == 0)
                    UpdateSelectCar(cars[reflectIdx][i].slot, reflectIdx, gCarTeamColor[gCarTeam[p]], gCarSheen[p], reflectPos, reflectUV, alpha);
                else UpdateSelectCar(cars[reflectIdx][i].slot, reflectIdx, gCarPaintJob[p+1][cars[p][i].slot % 13], gCarSheen[p], reflectPos, reflectUV, alpha);
            }
            if (gThisCarIndex[p] == i) {
                ScaleUV(unitmatrix, gTurntableMat[p][0].mat3.uvs, 1.3f);
                ScaleUV(unitmatrix, gTurntableMat[p][1].mat3.uvs, 1.3f);
                PitchUV(0.34906584f, gTurntableMat[p][0].mat3.uvs);
                PitchUV(0.34906584f, gTurntableMat[p][1].mat3.uvs);
                YawUV(cars[p][i].angle, gTurntableMat[p][0].mat3.uvs);
                if (gNumWindows == 1) {
                    gTurntableMat[p][0].mat3.pos[0] = 0.0f;
                    gTurntableMat[p][0].mat3.pos[1] = 1.8f;
                    gTurntableMat[p][0].mat3.pos[2] = 45.0f;
                    gTurntableMat[p][1].mat3.pos[0] = 0.0f;
                    gTurntableMat[p][1].mat3.pos[1] = 1.8f;
                    gTurntableMat[p][1].mat3.pos[2] = 45.0f;
                } else {
                    gTurntableMat[p][0].mat3.pos[0] = 0.0f;
                    gTurntableMat[p][0].mat3.pos[1] = 4.8f;
                    gTurntableMat[p][0].mat3.pos[2] = 45.0f;
                    gTurntableMat[p][1].mat3.pos[0] = 0.0f;
                    gTurntableMat[p][1].mat3.pos[1] = 4.8f;
                    gTurntableMat[p][1].mat3.pos[2] = 45.0f;
                }
                if (gRace == 2) {
                    for (ghostIdx = 1; ghostIdx < gNumGhosts+1; ghostIdx++) {
                        if (!gGhosts[ghostIdx] || gLastCar[ghostIdx] != (*(*gGhosts[ghostIdx])->header)->car) {
                            if (gLastCar[ghostIdx] != -1) UpdateSelectCar(cars[ghostIdx][gLastCar[ghostIdx]].slot, ghostIdx, 0, 0, carPos, uv, 0);
                        }
                        if (gGhosts[ghostIdx]) {
                            ghostCar = (*(*gGhosts[ghostIdx])->header)->car;
                            gLastCar[ghostIdx] = ghostCar;
                            cars[ghostIdx][ghostCar].slot = ghostCar + ghostIdx * 13;
                            gCarTires[ghostIdx+1][ghostCar] = (*(*gGhosts[ghostIdx])->header)->settings[2];
                            gTireRims[ghostIdx+1][ghostCar] = (*(*gGhosts[ghostIdx])->header)->settings[6];
                            gFrontTireScale[ghostIdx+1][ghostCar] = 0.75f + (*(*gGhosts[ghostIdx])->header)->settings[8] / 100.0f;
                            gRearTireScale[ghostIdx+1][ghostCar] = 0.75f + (*(*gGhosts[ghostIdx])->header)->settings[9] / 100.0f;
                            UpdateSelectCar(cars[ghostIdx][ghostCar].slot, ghostIdx, (*(*gGhosts[ghostIdx])->header)->settings[5], 0, carPos, uv, 255);
                        }
                    }
                }
            }
        }
        if (gNumPlayers == 1 && gRace != 2) {
            int show;
            createGridMap();
            show = 0;
            if (!gCarLocked[0]) {
                if (gOpts[p][gActiveOption[p]] == kCarTransmission) goto grid_on;
                if (gOpts[p][gActiveOption[p]] == kCarHandling) goto grid_on;
                if (gOpts[p][gActiveOption[p]] == kCarEngine) goto grid_on;
                if (gOpts[p][gActiveOption[p]] == kCarTire) goto grid_on;
                if (gOpts[p][gActiveOption[p]] == kCarFrame) { grid_on: show = 1; }
            }
            showGridMap(show);
            updateCarComponents(p);
        }
    }
}


/* 0x8008f904, size 0xbc8 */
static void _InitScreen(void)
{
    int i;
    int p;
    setGhosts();
    LoadWave(47, 0, 0, 0, 0);
    allLocked = 0;
    for (p = 0; p < gNumPlayers; p++) {
        gActiveOption[p] = 0;
        gActiveOptionEnabled[p] = 1;
        gFirstOption[p] = 1;
        gCarLocked[p] = 0;
        gHornSnd[p] = -1;
        calcStats(p);
    }
    for (p = 0; p < gNumPlayers; p++) {
        gNumCarsActive[p] = 0;
        for (i = 0; i < 13; i++) {
            if (i < 13) gCars[p][gNumCarsActive[p]++] = gCarOrder[i];
        }
    }
    for (i = 0; i < 48; i++) gCarMatrices[i] = ZOID_NewMatrix();
    SetOptions();
    for (i = 0; i < 4; i++) gDriveHulk[i] = 0;
    HFOV = 90.0f;
    ZOID_ResetWindows();
    gNumWindows = gNumPlayers;
    if (gNumWindows == 3) gNumWindows = 4;
    if (gRace == 2) { gNumGhosts = 3; gNumWindows = 4; }
    drawWindowBorders = 0;
    ZOID_SplitMenuScreen(gNumWindows);
    if (gNumWindows == 1) {
        gButtonZoom = 1.0f;
        gButtonScale = 0.045f;
        gButtonSpace = 40.0f;
        buttonX = 0.0f;
        maxY = -5.4f;
        minY = -8.4f;
        buttonZ = 15.0f;
        cursorZ = 11.25f;
    } else {
        gButtonZoom = 1.0f;
        gButtonScale = 0.065f;
        gButtonSpace = 40.0f;
        buttonX = 0.0f;
        maxY = -3.45f;
        minY = -7.7f;
        buttonZ = 15.0f;
        cursorZ = 11.25f;
    }
    LoadCars(1);
    InitVisuals(1);
    createBG();
    for (i = 0; i < 4; i++) pane[i] = InitPane(-20, 0, -10, 10, 192, 0, 0, 0);
    for (p = 0; p < gNumPlayers; p++) {
        CopyUV(unitmatrix, gTurntableMat[p][0].mat3.uvs);
        CopyUV(unitmatrix, gTurntableMat[p][1].mat3.uvs);
        gTurntableIdx[p][0] = MBOX_NewObject(MBOX_FindObject_WORK("TURNTABLEG1", 0, gNumModels-1, 1), &gTurntableMat[p][0], -1, 0x40000);
        gTurntableIdx[p][1] = MBOX_NewObject(MBOX_FindObject_WORK("TURNTABLEBASEG1", 0, gNumModels-1, 1), &gTurntableMat[p][1], -1, 0);
        MBOX_HideObject(gTurntableIdx[p][0], 1, 15);
        MBOX_HideObject(gTurntableIdx[p][1], 1, 15);
        if (gRace == 2) {
            MBOX_ShowObject(gTurntableIdx[p][0], 0, 15);
            MBOX_ShowObject(gTurntableIdx[p][1], 0, 15);
        } else {
            MBOX_ShowObject(gTurntableIdx[p][0], 0, 1 << p);
            MBOX_ShowObject(gTurntableIdx[p][1], 0, 1 << p);
        }
        gTurntableMat[p][0].mat3.pos[0] = 0.0f;
        gTurntableMat[p][0].mat3.pos[1] = 2.0f;
        gTurntableMat[p][0].mat3.pos[2] = 15.0f;
        gTurntableMat[p][1].mat3.pos[0] = 0.0f;
        gTurntableMat[p][1].mat3.pos[1] = 2.0f;
        gTurntableMat[p][1].mat3.pos[2] = 15.0f;
        initButtons(p);
        for (i = 0; i < 13; i++) {
            gCarHandling[p+1][i] = GetPlayerCarSetting(gPlayers[p].save, i, 0);
            gCarEngines[p+1][i] = GetPlayerCarSetting(gPlayers[p].save, i, 1);
            while (!IsEngineUnlocked(p, gCarEngines[p+1][i])) {
                gCarEngines[p+1][i]--;
                if (gCarEngines[p+1][i] < 0) { gCarEngines[p+1][i] = 0; break; }
            }
            SavePlayerCarSetting(gPlayers[p].save, i, 1, gCarEngines[p+1][i]);
            gCarTires[p+1][i] = GetPlayerCarSetting(gPlayers[p].save, i, 2);
            while (!IsTiresUnlocked(p, gCarTires[p+1][i])) {
                gCarTires[p+1][i]--;
                if (gCarTires[p+1][i] < 0) { gCarTires[p+1][i] = 0; break; }
            }
            SavePlayerCarSetting(gPlayers[p].save, i, 2, gCarTires[p+1][i]);
            gCarFrame[p+1][i] = GetPlayerCarSetting(gPlayers[p].save, i, 3);
            while (!IsFrameUnlocked(p, gCarFrame[p+1][i])) {
                gCarFrame[p+1][i]--;
                if (gCarFrame[p+1][i] < 0) { gCarFrame[p+1][i] = 0; break; }
            }
            SavePlayerCarSetting(gPlayers[p].save, i, 3, gCarFrame[p+1][i]);
            gCarWings[p+1][i] = GetPlayerCarSetting(gPlayers[p].save, i, 4);
            if (gRace != 4 && gCarWings[p+1][i] > 1) gCarWings[p+1][i] = 1;
            gCarPaintJob[p+1][i] = GetPlayerCarSetting(gPlayers[p].save, i, 5);
            gTireRims[p+1][i] = GetPlayerCarSetting(gPlayers[p].save, i, 6);
            gCarHorn[p+1][i] = GetPlayerCarSetting(gPlayers[p].save, i, 7);
            gFrontTireScale[p+1][i] = 0.75f + GetPlayerCarSetting(gPlayers[p].save, i, 8) / 100.0f;
            gRearTireScale[p+1][i] = 0.75f + GetPlayerCarSetting(gPlayers[p].save, i, 9) / 100.0f;
            if (!gEnableTireScaling) {
                gFrontTireScale[p+1][i] = gFrontTireScale[0][i];
                gRearTireScale[p+1][i] = gRearTireScale[0][i];
            }
        }
        gThisCar[p] = GetPlayerCar(gPlayers[p].save);
        while (!IsCarUnlocked(p, gThisCar[p])) {
            if (!gThisCar[p]) gThisCar[p] = 12;
            else gThisCar[p]--;
        }
        for (gThisCarIndex[p] = 0; gThisCarIndex[p] < gNumCarsActive[p]; gThisCarIndex[p]++) {
            if (gThisCar[p] == gCars[p][gThisCarIndex[p]]) break;
        }
        if (gThisCarIndex[p] >= gNumCarsActive[p]) {
            gThisCarIndex[p] = p + 1;
            gThisCar[p] = gCars[p][gThisCarIndex[p]];
            SavePlayerCar(gPlayers[p].save, gThisCar[p]);
        }
        gLastCar[p] = -1;
        calcValidOptions(p);
        for (i = 0; i < gNumCarsActive[p]; i++) {
            cars[p][i].index = i - gThisCarIndex[p];
            if (cars[p][i].index < 0) cars[p][i].index += gNumCarsActive[p];
            cars[p][i].slot = gCars[p][i] + p * 13;
            cars[p][i].angle = 3.1415927f;
            cars[p][i].alpha = 0.0f;
        }
        gThisCarIndex[p] = gThisCarIndex[p]; /* no-op copy kept on purpose: the target reloads and re-stores the value here */
        if (gRace == 6) gPlayers[p].trans = 6;
        else {
            gPlayers[p].trans = GetPlayerCarTrans(gPlayers[p].save);
            while (!IsTransUnlocked(p, gPlayers[p].trans)) gPlayers[p].trans -= 2;
            SavePlayerCarTrans(gPlayers[p].save, gPlayers[p].trans);
        }
        gPlayers[p].trans = gPlayers[p].trans; /* no-op copy kept on purpose: the target reloads and re-stores the value here */
        for (i = 0; i < 13; i++) gCarTrans[p+1][i] = gPlayers[p].trans;
        gPlayers[p].trans = gPlayers[p].trans; /* same no-op copy again, as in the target */
    }
    blit = (_tBlit_2 *)NewMultiBlit(0, 0, mb, 26);
    ZeroCameras();
    UpdateCameras();
    ClearRushControls();
    inited = 1;
}


/* 0x800904cc, size 0x16c */
static void _UninitScreen(void)
{
    int p;
    if (!inited) return;
    UnloadWaveAll(47);
    if (gNumPlayers == 1 && gRace != 2) releaseGridMap();
    for (p = 0; p < 4; p++) {
        if (pane[p]) {
            RemovePane(pane[p]);
            pane[p] = 0;
        }
    }
    for (p = 0; p < gNumPlayers; p++) {
        uninitButtons(p);
        if (gTurntableIdx[p][0] != -1) {
            ZOID_RemoveObject((signed short)gTurntableIdx[p][0]);
            gTurntableIdx[p][0] = -1;
        }
        if (gTurntableIdx[p][1] != -1) {
            ZOID_RemoveObject((signed short)gTurntableIdx[p][1]);
            gTurntableIdx[p][1] = -1;
        }
        clearCarComponents(p);
    }
    if (blit) { RemoveBlit((_tBlit *)blit); blit = 0; }
    removeBG();
    HFOV = 90.0f;
    gNumWindows = 1;
    drawWindowBorders = 1;
    ZOID_ResetWindows();
    ZOID_Reset();
    for (p = 0; p < gNumPlayers; p++) AudStopEffect(gHornSnd[p]);
    inited = 0;
}


/* 0x80090638, size 0xac0 */
static void setGhosts(void)
{
    tGhostFile **nearGhosts[5];
    tGhostFile **ghosth;
    tGhostFile *ghost;
    tGhostFile **best = 0;
    tGhostFile **nextGhost;
    tPlayerProfile *player;
    int ghostCount;
    int i;
    int j;
    player = *gPlayers[0].save;
    for (i = 0; i < 5; i++) nearGhosts[i] = 0;
    for (i = 0; i < 4; i++) gGhosts[i] = 0;
    for (i = 1; i < 4; i++) gLastCar[i] = -1;
    if (gRace != 2) return;
    if (!gGhostSelect) {
        for (ghosth = gGhostFiles.head; ghosth; ghosth = nextGhost) {
            ghost = *ghosth;
            nextGhost = ghost->ll.next;
            if (ghost->track == gTrackNum && ghost->backward == gBackwards &&
                !strcmp((char *)ghost->name, (char *)player->name) &&
                ghost->playerId[0] == player->id[0] && ghost->playerId[1] == player->id[1]) {
                if ((!best || ghost->finish < (*best)->finish) && ReadGhostData(ghosth)) best = ghosth;
            }
        }
        for (i = 0; i < 3; i++) {
            ghosth = limboGhosts + i;
            ghost = *ghosth;
            if (ghost->header && ghost->track == gTrackNum && ghost->backward == gBackwards &&
                !strcmp((char *)ghost->name, (char *)player->name) &&
                ghost->playerId[0] == player->id[0] && ghost->playerId[1] == player->id[1]) {
                if ((!best || ghost->finish < (*best)->finish) && ReadGhostData(ghosth)) best = ghosth;
            }
        }
        if (!best) {
            for (ghosth = gGhostFiles.head; ghosth; ghosth = nextGhost) {
                ghost = *ghosth;
                nextGhost = ghost->ll.next;
                if (ghost->track == gTrackNum && ghost->backward == gBackwards) {
                    for (i = 3; i >= 1; i--) {
                        if ((!gGhosts[i] || ghost->finish > (*gGhosts[i])->finish) && ReadGhostData(ghosth)) {
                            for (j = 1; j < i; j++) gGhosts[j] = gGhosts[j+1];
                            gGhosts[i] = ghosth;
                            break;
                        }
                    }
                }
            }
            for (ghosth = limboGhosts; ghosth < limboGhosts + 3; ghosth++) {
                ghost = *ghosth;
                if (ghost->header && ghost->track == gTrackNum && ghost->backward == gBackwards) {
                    for (i = 3; i >= 1; i--) {
                        if ((!gGhosts[i] || ghost->finish > (*gGhosts[i])->finish) && ReadGhostData(ghosth)) {
                            for (j = 1; j < i; j++) gGhosts[j] = gGhosts[j+1];
                            gGhosts[i] = ghosth;
                            break;
                        }
                    }
                }
            }
            for (i = 1; i < 3; i++) {
                if (!gGhosts[i] && gGhosts[i+1]) {
                    gGhosts[i] = gGhosts[i+1];
                    gGhosts[i+1] = 0;
                    i = 0;
                }
            }
        } else {
            nearGhosts[2] = best;
            for (ghosth = gGhostFiles.head; ghosth; ghosth = nextGhost) {
                ghost = *ghosth;
                nextGhost = ghost->ll.next;
                if (ghost->track == gTrackNum && ghost->backward == gBackwards) {
                    if (ghosth != best) {
                        if (ghost->finish < (*best)->finish) {
                            if (!nearGhosts[1] || ghost->finish > (*nearGhosts[1])->finish) {
                                if (ReadGhostData(ghosth)) {
                                    nearGhosts[0] = nearGhosts[1];
                                    nearGhosts[1] = ghosth;
                                }
                            } else if (!nearGhosts[0] || ghost->finish > (*nearGhosts[0])->finish) {
                                if (ReadGhostData(ghosth)) nearGhosts[0] = ghosth;
                            }
                        } else {
                            if (!nearGhosts[3] || ghost->finish < (*nearGhosts[3])->finish) {
                                if (ReadGhostData(ghosth)) {
                                    nearGhosts[4] = nearGhosts[3];
                                    nearGhosts[3] = ghosth;
                                }
                            } else if (!nearGhosts[4] || ghost->finish < (*nearGhosts[4])->finish) {
                                if (ReadGhostData(ghosth)) nearGhosts[4] = ghosth;
                            }
                        }
                    }
                }
            }
            for (ghosth = limboGhosts; ghosth < limboGhosts + 3; ghosth++) {
                ghost = *ghosth;
                if (ghost->header && ghost->track == gTrackNum && ghost->backward == gBackwards) {
                    if (ghosth != best) {
                        if (ghost->finish < (*best)->finish) {
                            if (!nearGhosts[1] || ghost->finish > (*nearGhosts[1])->finish) {
                                if (ReadGhostData(ghosth)) {
                                    nearGhosts[0] = nearGhosts[1];
                                    nearGhosts[1] = ghosth;
                                }
                            } else if (!nearGhosts[0] || ghost->finish > (*nearGhosts[0])->finish) {
                                if (ReadGhostData(ghosth)) nearGhosts[0] = ghosth;
                            }
                        } else {
                            if (!nearGhosts[3] || ghost->finish < (*nearGhosts[3])->finish) {
                                if (ReadGhostData(ghosth)) {
                                    nearGhosts[4] = nearGhosts[3];
                                    nearGhosts[3] = ghosth;
                                }
                            } else if (!nearGhosts[4] || ghost->finish < (*nearGhosts[4])->finish) {
                                if (ReadGhostData(ghosth)) nearGhosts[4] = ghosth;
                            }
                        }
                    }
                }
            }
            ghostCount = 1;
            for (i = 0; i < 5; i++) {
                if (nearGhosts[i] && ghostCount < 4) { gGhosts[ghostCount] = nearGhosts[i]; ghostCount++; }
            }
        }
    } else {
        for (ghosth = gGhostFiles.head; ghosth; ghosth = nextGhost) {
            ghost = *ghosth;
            nextGhost = ghost->ll.next;
            if (ghost->track == gTrackNum && ghost->backward == gBackwards) {
                for (i = 1; i < 4; i++) {
                    if ((!gGhosts[i] || ghost->finish < (*gGhosts[i])->finish) && ReadGhostData(ghosth)) {
                        for (j = 3; j > i; j--) gGhosts[j] = gGhosts[j-1];
                        gGhosts[i] = ghosth;
                        break;
                    }
                }
            }
        }
        for (ghosth = limboGhosts; ghosth < limboGhosts + 3; ghosth++) {
            ghost = *ghosth;
            if (ghost->header && ghost->track == gTrackNum && ghost->backward == gBackwards) {
                for (i = 1; i < 4; i++) {
                    if ((!gGhosts[i] || ghost->finish < (*gGhosts[i])->finish) && ReadGhostData(ghosth)) {
                        for (j = 3; j > i; j--) gGhosts[j] = gGhosts[j-1];
                        gGhosts[i] = ghosth;
                        break;
                    }
                }
            }
        }
    }
}


/* 0x800910f8, size 0x830 */
static void updateCarComponents(int p)
{
    int i;
    float lightVec[3] = {-0.57f, 0.57f, -0.57f};
    CarParts *car;
    tRGBA buttonColor = { {255,255,255,255} };
    signed short currentLevel;
    int whichLight;
    float compScale;
    char componentName[16];
    if (gNumPlayers > 1 || gRace == 2) return;
    if (gCarLocked[p] != 1) {
        if (gOpts[p][gActiveOption[p]] == kCarTransmission) goto show_components;
        if (gOpts[p][gActiveOption[p]] == kCarHandling) goto show_components;
        if (gOpts[p][gActiveOption[p]] == kCarEngine) goto show_components;
        if (gOpts[p][gActiveOption[p]] == kCarTire) goto show_components;
        if (gOpts[p][gActiveOption[p]] == kCarFrame) goto show_components;
    }
    {
        for (i = 0; i < 11; i++) {
            if (gCarComponents[p][i].idx != -1) {
                MBOX_HideObject(gCarComponents[p][i].idx, 1, 15);
                ZOID_SetObjectParent(gCarComponents[p][i].idx, -1);
            }
        }
        return;
    }
show_components:
    car = &gCarParts[p * 13 + gThisCar[p]];
    if (gCarPartFrame == -1) {
        gCarPartFrame = MBOX_NewObjectCam(MBOX_FindObject_WORK("BAR_FGG1", 0, gNumModels-1, 1),
            ZOID_FindMatrix((signed short)car->frame), 15, -1, 0);
        MBOX_HideObject(gCarPartFrame, 1, 15);
    } else {
        if (gCarPartFrame == 48) printf("weruw");
        ObjInstanceList[gCarPartFrame].mat = ZOID_FindMatrix((signed short)car->frame);
    }
    compLight[1].a.col[1] = 255.0f * TB_GetFlashTextCycle();
    BodyVector(lightVec, compLight[0].l[0].dir, ZOID_FindMatrix((signed short)car->frame)->mat3.uvs);
    BodyVector(lightVec, compLight[1].l[0].dir, ZOID_FindMatrix((signed short)car->frame)->mat3.uvs);
    compLight[0].l[0].dir[0] = 0.05f * compLight[0].l[0].dir[0];
    compLight[0].l[0].dir[1] = 0.05f * compLight[0].l[0].dir[1];
    compLight[0].l[0].dir[2] = 0.05f * compLight[0].l[0].dir[2];
    compLight[1].l[0].dir[0] = 0.05f * compLight[1].l[0].dir[0];
    compLight[1].l[0].dir[1] = 0.05f * compLight[1].l[0].dir[1];
    compLight[1].l[0].dir[2] = 0.05f * compLight[1].l[0].dir[2];
    for (i = 0; i < 11; i++) {
        currentLevel = -1;
        compScale = 1.0f;
        whichLight = 0;
        switch (componentToName[i]) {
        case 0:
            if (gOpts[p][gActiveOption[p]] == kCarEngine) {
                whichLight = 1;
                compScale = 0.1f * TB_GetFlashTextCycle() + 1.0f;
            }
            currentLevel = gCarEngines[p+1][gThisCar[p]] % 5;
            break;
        case 1:
            if (gOpts[p][gActiveOption[p]] == kCarFrame) {
                whichLight = 1;
                compScale = 0.1f * TB_GetFlashTextCycle() + 1.0f;
            }
            currentLevel = gCarFrame[p+1][gThisCar[p]] % 3;
            break;
        case 2:
            if (gOpts[p][gActiveOption[p]] == kCarHandling) {
                whichLight = 1;
                compScale = 0.1f * TB_GetFlashTextCycle() + 1.0f;
            }
            currentLevel = gCarHandling[p+1][gThisCar[p]] % 3;
            break;
        case 3:
            if (gOpts[p][gActiveOption[p]] == kCarTire) {
                whichLight = 1;
                compScale = 0.1f * TB_GetFlashTextCycle() + 1.0f;
            }
            currentLevel = gCarTires[p+1][gThisCar[p]] % 4;
            break;
        case 4:
            if (gOpts[p][gActiveOption[p]] == kCarTransmission) {
                whichLight = 1;
                compScale = 0.1f * TB_GetFlashTextCycle() + 1.0f;
            }
            currentLevel = gCarTrans[p+1][gThisCar[p]] % 2;
            break;
        }
        if (currentLevel != gCarComponents[p][i].level) {
            sprintf(componentName, carComponentNames[componentToName[i]], currentLevel + 1);
            if (gCarComponents[p][i].idx != -1) {
                ZOID_RemoveObject(gCarComponents[p][i].idx);
                gCarComponents[p][i].idx = -1;
                gCarComponents[p][i].level = -1;
            }
            gCarComponents[p][i].idx = MBOX_NewObject(MBOX_FindObject_WORK(componentName, 0, gNumModels-1, 1), &gCarComponents[p][i].mat, -1, 0);
            MBOX_HideObject(gCarComponents[p][i].idx, 1, 15);
            if (gNumWindows == 1) {
                gCarComponents[p][i].mat.mat3.pos[0] = 0.0f;
                gCarComponents[p][i].mat.mat3.pos[1] = 1.0f;
                gCarComponents[p][i].mat.mat3.pos[2] = 15.0f;
            } else {
                gCarComponents[p][i].mat.mat3.pos[0] = 0.0f;
                gCarComponents[p][i].mat.mat3.pos[1] = 2.0f;
                gCarComponents[p][i].mat.mat3.pos[2] = 15.0f;
            }
            gCarComponents[p][i].level = currentLevel;
        }
        switch (i) {
        case 0: case 1: case 10:
            ScaleUV(unitmatrix, gCarComponents[p][i].mat.mat3.uvs, compScale);
            gCarComponents[p][i].mat.mat3.pos[0] = 0.0f;
            gCarComponents[p][i].mat.mat3.pos[1] = 0.0f;
            gCarComponents[p][i].mat.mat3.pos[2] = 0.0f;
            gCarComponents[p][i].mat.mat3.pos[1] += 0.1f;
            break;
        case 2: case 6:
            ScaleUV(ZOID_FindMatrix((signed short)car->tires[0])->mat3.uvs, gCarComponents[p][i].mat.mat3.uvs, compScale);
            gCarComponents[p][i].mat.mat3.pos[0] = ZOID_FindMatrix((signed short)car->tires[0])->mat3.pos[0];
            gCarComponents[p][i].mat.mat3.pos[1] = ZOID_FindMatrix((signed short)car->tires[0])->mat3.pos[1];
            gCarComponents[p][i].mat.mat3.pos[2] = ZOID_FindMatrix((signed short)car->tires[0])->mat3.pos[2];
            YawUV(3.1415927f, gCarComponents[p][i].mat.mat3.uvs);
            break;
        case 3: case 7:
            ScaleUV(ZOID_FindMatrix((signed short)car->tires[1])->mat3.uvs, gCarComponents[p][i].mat.mat3.uvs, compScale);
            gCarComponents[p][i].mat.mat3.pos[0] = ZOID_FindMatrix((signed short)car->tires[1])->mat3.pos[0];
            gCarComponents[p][i].mat.mat3.pos[1] = ZOID_FindMatrix((signed short)car->tires[1])->mat3.pos[1];
            gCarComponents[p][i].mat.mat3.pos[2] = ZOID_FindMatrix((signed short)car->tires[1])->mat3.pos[2];
            break;
        case 5: case 9:
            ScaleUV(ZOID_FindMatrix((signed short)car->tires[2])->mat3.uvs, gCarComponents[p][i].mat.mat3.uvs, compScale);
            gCarComponents[p][i].mat.mat3.pos[0] = ZOID_FindMatrix((signed short)car->tires[2])->mat3.pos[0];
            gCarComponents[p][i].mat.mat3.pos[1] = ZOID_FindMatrix((signed short)car->tires[2])->mat3.pos[1];
            gCarComponents[p][i].mat.mat3.pos[2] = ZOID_FindMatrix((signed short)car->tires[2])->mat3.pos[2];
            YawUV(3.1415927f, gCarComponents[p][i].mat.mat3.uvs);
            break;
        case 4: case 8:
            ScaleUV(ZOID_FindMatrix((signed short)car->tires[3])->mat3.uvs, gCarComponents[p][i].mat.mat3.uvs, compScale);
            gCarComponents[p][i].mat.mat3.pos[0] = ZOID_FindMatrix((signed short)car->tires[3])->mat3.pos[0];
            gCarComponents[p][i].mat.mat3.pos[1] = ZOID_FindMatrix((signed short)car->tires[3])->mat3.pos[1];
            gCarComponents[p][i].mat.mat3.pos[2] = ZOID_FindMatrix((signed short)car->tires[3])->mat3.pos[2];
            break;
        }
        ZOID_SetObjectParent(gCarComponents[p][i].idx, (signed short)gCarPartFrame);
        MBOX_ShowObject(gCarComponents[p][i].idx, 0, 1 << p);
        ZOID_SetObjLights(gCarComponents[p][i].idx, 1, &compLight[whichLight]);
        buttonColor.c[0] = compLight[whichLight].a.col[0];
        buttonColor.c[1] = compLight[whichLight].a.col[1];
        buttonColor.c[2] = compLight[whichLight].a.col[2];
        buttonColor.c[3] = 255;
        ZOID_SetColor(gCarComponents[p][i].idx, &buttonColor);
    }
}


/* 0x80091928, size 0x98 */
static void clearCarComponents(int p)
{
    int i;
    if (p > 0) return;
    for (i = 0; i < 11; i++) {
        if (gCarComponents[p][i].idx != -1) {
            ZOID_RemoveObject(gCarComponents[p][i].idx);
            gCarComponents[p][i].idx = -1;
            gCarComponents[p][i].level = -1;
        }
    }
    if (gCarPartFrame != -1) {
        ZOID_RemoveObject((signed short)gCarPartFrame);
        gCarPartFrame = -1;
    }
}


/* 0x800919c0, size 0x390 */
static int animNewSlider(_tBlit_2 *blt)
{
    int type = blt->animID & 15;
    int p = (blt->animID >> 4) & 15;
    int opt = blt->animID >> 16;
    signed short screenPos[2];
    if (p >= gNumPlayers) {
        Hidden(blt, 1); blt->animFunc = 0; return 1;
    }
    if (gCarLocked[p]) { Hidden(blt, 1); return 1; }
    if (Hidden(blt, (buttonInfo[p][opt].angle > 0.6981317f && buttonInfo[p][opt].angle < 2.443461f) || !buttonInfo[p][opt].alpha)) return 1;
    ZOID_ProjectWindow(p, buttonInfo[p][opt+17].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
    blt->x = (screenPos[0] - blt->width / 2) / 2;
    blt->y = screenPos[1] / 2.0f - ((blt->height / 4) / 2) / 2.0f;
    blt->left = 0;
    blt->right = blt->width - 1;
    blt->top = 0;
    blt->bot = blt->height / 4 - 1;
    blt->alpha = buttonInfo[p][opt].alpha;
    if (buttonInfo[p][opt].angle > 1.5707964f) {
        blt->top += blt->height / 2;
        blt->bot += blt->height / 2;
    }
    if (type == 1) {
        blt->top += blt->height / 4;
        blt->bot += blt->height / 4;
        switch (opt) {
        case 11:
            blt->right = (blt->width - 1) * ((gFrontTireScale[p+1][gThisCar[p]] - 0.75f) / 0.05f) / 15.0f;
            break;
        case 12:
            blt->right = (blt->width - 1) * ((gRearTireScale[p+1][gThisCar[p]] - 0.75f) / 0.05f) / 15.0f;
            break;
        }
    }
    UpdateBlit((_tBlit *)blt);
    return 1;
}


/* 0x80091d50, size 0x258 */
static int animStatBars(_tBlit_2 *blt)
{
    int type = blt->animID & 15;
    int stat = (blt->animID >> 4) & 15;
    int p = (blt->animID >> 8) & 15;
    if (!(blt->animID >> 31)) {
        if (gNumPlayers > 2 && p < 2) RenameBlit((_tBlit *)blt, "CARSTATBARS_SM", 0);
        blt->animID |= 0x80000000;
        if (p < gNumPlayers && gNumPlayers == 1 && gRace != 2) {
            blt->x = 10;
            blt->y = (blt->height / 8 * (stat * 2)) / 2.0f + 48.0f;
        } else {
            Hidden(blt, 1);
            blt->animFunc = 0;
            return 1;
        }
        blt->top = blt->height / 8 * (stat * 2 + !type) + 16;
        if (blt->top > 128) blt->top -= 128;
        blt->bot = blt->top + blt->height / 8 - 1;
        if (blt->bot > 128) { blt->top -= 32; blt->bot -= 32; }
        blt->left = 0;
    }
    if (Hidden(blt, gCarLocked[p] == 1)) return 1;
    if (type == 1) {
        blt->right = blt->width * gStats[p][stat] - 1.0f;
        blt->right = blt->right < 2 ? 2 : blt->right;
    } else blt->right = blt->width - 1;
    UpdateBlit((_tBlit *)blt);
    return 1;
}


/* 0x80091fa8, size 0x144 */
static int animStatText(void)
{
    int p;
    int stat;
    unsigned short texIdx;
    tTexture *tex;
    int y;
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(12);
    TB_SetTextColor(1);
    for (p = 0; p < gNumPlayers; p++) {
        if (gCarLocked[p] != 1) {
            for (stat = 0; stat < 4; stat++) {
                if (p < gNumPlayers && gNumPlayers == 1 && gRace != 2) {
                    tex = MBOX_FindTexture_WORK("CARSTATBARS_LG", &texIdx, 0, gNumModels - 1, 1);
                    y = ((tex->height / 8) * (2 * stat)) / 2.0f + 48.0f;
                    TB_DrawText(10, y - TB_GetFontLineHeight(), gText.taba[gText.tab[14] + stat]);
                }
            }
        }
    }
    HAL_SetTextDepth(-1.0f);
    return 1;
}


/* 0x800920ec, size 0xd0 */
static int animText(void)
{
    int i;
    tPlayerProfile **player;
    for (i = 0; i < gNumPlayers; i++) {
        if (gPlayers[i].save != &gJustPlayers[i]) {
            player = gPlayerFiles.head;
            while (player) {
                if (gPlayers[i].save == player) break;
                player = (*player)->ll.next;
            }
            if (!player) return 1;
        }
        if (buttonsInited[i] == 1) animButtonText(i);
        else drawOptions(i);
    }
    return 1;
}


/* inline helper taking both flash colours by value (as videoscr.c's flashText): the copies stay out of the DWARF locals and give the original stack use */
static inline void carFlashColors(tRGBA c1, tRGBA c2)
{
    TB_FlashTextColors(&c1, &c2);
}

/* 0x800921bc, size 0xcac */
static void animButtonText(int p)
{
    int i;
    int x;
    int y;
    int w;
    unsigned char *str;
    unsigned char str1[64];
    signed short screenPos[2];
    float gTextPosScale = gButtonScale;
    if (gNumWindows > 1) gTextPosScale *= 0.5f;
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1,1);
    if (gNumWindows == 1) { w = 140; HAL_SetFont(13); }
    else { w = 69; HAL_SetFont(12); }
    ZOID_ProjectWindow(p, buttonInfo[p][34].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
    x = screenPos[0] / 2;
    y = screenPos[1] / 2.0f;
    str = (*gPlayers[p].save)->name;
    TB_SetTextColor(1);
    TB_DrawEllipseText(x,y,w,str);
    TB_SetJust(0,1);
    if (gRace == 2) {
        int ghostIdx;
        for (ghostIdx = 1; ghostIdx < gNumGhosts + 1; ghostIdx++) {
            float pos[3];
            ZOID_ProjectWindow(ghostIdx, buttonInfo[0][34].mat.mat3.pos, &gCamera[ghostIdx].mat, 0, screenPos);
            x = screenPos[0] / 2;
            y = screenPos[1] / 2.0f;
            TB_Sprintf(str1, "GHOST %d", ghostIdx);
            TB_SetTextColor(1);
            TB_DrawText(TB_CenterText(str1, x), y, str1);
            pos[0] = buttonInfo[0][34].mat.mat3.pos[0];
            pos[1] = buttonInfo[0][34].mat.mat3.pos[1];
            pos[2] = buttonInfo[0][34].mat.mat3.pos[2];
            pos[1] = -pos[1] / 2.0f;
            ZOID_ProjectWindow(ghostIdx, pos, &gCamera[ghostIdx].mat, 0, screenPos);
            x = screenPos[0] / 2;
            y = screenPos[1] / 2.0f;
            if (!gGhosts[ghostIdx]) {
                str = gText.single[236];
                TB_SetTextColor(1);
                TB_DrawText(TB_CenterText(str, x), y, str);
            } else {
                str = (*gGhosts[ghostIdx])->name;
                TB_SetTextColor(playerTextColor[ghostIdx]);
                TB_DrawText(TB_CenterText(str, x), y, str);
                y += (signed short)TB_GetFontLineHeight();
                TB_Sprintf(str1, "TIME %01d:%02d.%03d", (int)(*gGhosts[ghostIdx])->finish / 60, (int)(*gGhosts[ghostIdx])->finish % 60, (int)(1000.0f * (*gGhosts[ghostIdx])->finish) % 1000);
                TB_SetTextColor(1);
                TB_DrawText(TB_CenterText(str1, x), y, str1);
            }
        }
    }
    if (gCarLocked[p]) {
        int notReadyCount;
        int notReadyIdx;
        int pIdx;
        float msgPos[3] = {0.0f, -120.0f, 300.0f};
        ZOID_ProjectWindow(p, msgPos, &gCamera[p].mat, 0, screenPos);
        x = screenPos[0] / 2;
        y = screenPos[1] / 2.0f - (signed short)TB_GetFontLineHeight();
        notReadyCount = 0;
        for (pIdx = 0; pIdx < gNumPlayers; pIdx++) {
            if (pIdx != p && !gCarLocked[pIdx]) { notReadyIdx = pIdx; notReadyCount++; }
        }
        if (!notReadyCount) {
            y += (signed short)TB_GetFontLineHeight();
            if (gRace == 5) str = gText.single[58];
            else if (gRace == 6) str = gText.single[57];
            else if (gRace == 4) str = gText.single[56];
            else str = gText.single[55];
            TB_SetTextColor(1);
        } else {
            str = gText.single[161];
            TB_SetTextColor(1);
            TB_DrawText(TB_CenterText(str, x), y, str);
            y += (signed short)TB_GetFontLineHeight();
            if (notReadyCount == 1) {
                TB_SetTextColor(playerTextColor[notReadyIdx]);
                str = (*gPlayers[notReadyIdx].save)->name;
            } else str = gText.single[31];
        }
        TB_DrawText(TB_CenterText(str, x), y, str);
        TB_SetJust(0,3);
        HAL_SetTextTranslucency(-1.0f);
        return;
    }
    if (gNumWindows == 1) HAL_SetFont(11); else HAL_SetFont(12);
    for (i = 0; i < gNumOptions[p]; i++) {
        int item = gOpts[p][i];
        if (buttonInfo[p][item].angle > 0.6981317f && buttonInfo[p][item].angle < 2.443461f) continue;
        if (!buttonInfo[p][item].alpha) continue;
        HAL_SetTextTranslucency(buttonInfo[p][item].alpha);
        if (buttonInfo[p][item].angle < 1.5707964f) {
            carFlashColors(flashColors[0], flashColors[1]);
            TB_SetTextColor(1);
        } else {
            carFlashColors(flashColors[0], flashColors[1]);
            TB_SetTextColor(22);
        }
        HAL_SetMenuTextDepth(buttonInfo[p][item].mat.mat3.pos[2] - 10.0f);
        ZOID_ProjectWindow(p, buttonInfo[p][item].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
        x = screenPos[0] / 2 - 300.0f * (45.0f * gTextPosScale) / buttonZ;
        y = screenPos[1] / 2.0f;
        TB_DrawText(x, y, gNumWindows == 1 ? gText.taba[gText.tab[2] + item] : gText.taba[gText.tab[3] + item]);
        switch (item) {
        case 0: str = (unsigned char *)CarTitles[gThisCar[p]]; break;
        case 1: str = gText.taba[gText.tab[4] + gCarTrans[p+1][gThisCar[p]]]; break;
        case 2: str = gText.taba[gText.tab[6] + gCarHandling[p+1][gThisCar[p]]]; break;
        case 3: str = gText.taba[gText.tab[7] + gCarEngines[p+1][gThisCar[p]]]; break;
        case 4: str = gText.taba[gText.tab[8] + gCarTires[p+1][gThisCar[p]]]; break;
        case 5: str = gText.taba[gText.tab[9] + gCarFrame[p+1][gThisCar[p]]]; break;
        case 6: str = gText.taba[gText.tab[10] + gCarWings[p+1][gThisCar[p]]]; break;
        case 7: TB_Sprintf(str1, "%d", gCarPaintJob[p+1][gThisCar[p]] + 1); str = str1; break;
        case 8: str = gText.taba[gText.tab[13] + gCarTeam[p]]; break;
        case 9: str = gText.taba[gText.tab[11] + gTireRims[p+1][gThisCar[p]]]; break;
        case 10: str = gText.taba[gText.tab[12] + gCarHorn[p+1][gThisCar[p]]]; break;
        case 13: TB_Sprintf(str1, "%d", gCarSheen[p]+1); str = str1; break;
        case 14:
            if (gGhosts[1]) TB_EllipseText(str1, (*gGhosts[1])->name, 60);
            else TB_Strcpy(str1, gText.taba[gText.tab[16]]);
            str = str1;
            break;
        case 15:
            if (gGhosts[2]) TB_EllipseText(str1, (*gGhosts[2])->name, 60);
            else TB_Strcpy(str1, gText.taba[gText.tab[16]]);
            str = str1;
            break;
        case 16:
            if (gGhosts[3]) TB_EllipseText(str1, (*gGhosts[3])->name, 60);
            else TB_Strcpy(str1, gText.taba[gText.tab[16]]);
            str = str1;
            break;
        default: str = 0; break;
        }
        if (str) {
            ZOID_ProjectWindow(p, buttonInfo[p][item+17].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
            x = screenPos[0] / 2 - 300.0f * (50.0f * gTextPosScale) / buttonZ;
            y = screenPos[1] / 2.0f;
            TB_DrawText(x,y,str);
        }
    }
    TB_SetJust(0,3);
    HAL_SetTextTranslucency(-1.0f);
    HAL_SetTextDepth(-1.0f);
}


/* 0x80092e68, size 0x4 */
static void drawOptions(int p)
{
    /* TODO */
}


/* 0x80092e6c, size 0x84 */
static void calcValidOptions(int p)
{
    int i;
    gNumOptions[p] = 0;
    for (i = 0; i < 17; i++) {
        if (validOption(p, i)) gOpts[p][gNumOptions[p]++] = i;
    }
}


/* 0x80092ef0, size 0x98 */
static int validOption(int p, int opt)
{
    int optionValid = 1;
    if ((opt == 11 || opt == 12) && !gEnableTireScaling) optionValid = 0;
    if (opt == 6 && (gRace == 2 || gRace == 6)) optionValid = 0;
    if (opt == 8 && gRace != 6) optionValid = 0;
    if ((opt == 14 || opt == 15 || opt == 16) && gRace != 2) optionValid = 0;
    if (opt == 10 || opt == 13) optionValid = 0;
    return optionValid;
}


/* 0x80092f88, size 0x190 */
static void calcStats(int p)
{
    signed short i;
    for (i = 0; i < 4; i++) {
        gStats[p][i] = 1.0f;
        gStats[p][i] *= gTransStats[gCarTrans[p+1][gThisCar[p]]][i];
        gStats[p][i] *= gHandlingStats[gCarHandling[p+1][gThisCar[p]]][i];
        gStats[p][i] *= gEngineStats[gCarEngines[p+1][gThisCar[p]]][i];
        gStats[p][i] *= gTireStats[gCarTires[p+1][gThisCar[p]]][i];
        gStats[p][i] *= gFrameStats[gCarFrame[p+1][gThisCar[p]]][i];
    }
    /* no-op copies kept on purpose: the target reloads and re-stores each of these values */
    gCarFrame[p+1][gThisCar[p]] = gCarFrame[p+1][gThisCar[p]];
    gCarTires[p+1][gThisCar[p]] = gCarTires[p+1][gThisCar[p]];
    gCarEngines[p+1][gThisCar[p]] = gCarEngines[p+1][gThisCar[p]];
    gCarHandling[p+1][gThisCar[p]] = gCarHandling[p+1][gThisCar[p]];
    gCarTrans[p+1][gThisCar[p]] = gCarTrans[p+1][gThisCar[p]];
    gThisCar[p] = gThisCar[p];
}


/* Display-list vertex writer reconstructed from the original inline stores. */
static inline void gridHeader(unsigned int *dst, unsigned int cmd, unsigned int idx)
{
    *dst = 0;
    *dst |= cmd;
    *dst |= (idx << 16) & 0xff0000;
    *dst |= (idx << 8) & 0xff00;
    *dst |= idx & 255;
}
#define GRID_VERTEX(v, cmd) do { \
    gridHeader(dstList, cmd, vertIdx++); \
    vertIdx &= 127; \
    dstList++; \
    *(float *)dstList++ = (v).pos[0]; \
    *(float *)dstList++ = (v).pos[1]; \
    *(float *)dstList++ = (v).pos[2]; \
    *(float *)dstList++ = (v).uv[0]; \
    *(float *)dstList++ = (v).uv[1]; \
    if (normals) { \
        *(float *)dstList++ = (v).normal[0]; \
        *(float *)dstList++ = (v).normal[1]; \
        *(float *)dstList++ = (v).normal[2]; \
    } else *dstList++ = (v).color; \
} while (0)

/* inline helper (no DWARF entry) with scalarised d1/d2/n arrays: the original lists no locals for this computation, and the helper temporaries reproduce its FP registers */
static inline int gridAlignAxis(float *a, float *b, float *c)
{
    float d1[3], d2[3], n[3];
    #define nx n[0]
    #define ny n[1]
    #define nz n[2]
    d1[0] = a[0] - b[0]; d1[1] = a[1] - b[1]; d1[2] = a[2] - b[2];
    d2[0] = b[0] - c[0]; d2[1] = b[1] - c[1]; d2[2] = b[2] - c[2];
    #define dx1 d1[0]
    #define dy1 d1[1]
    #define dz1 d1[2]
    #define dx2 d2[0]
    #define dy2 d2[1]
    #define dz2 d2[2]
    nx = dy1 * dz2 - dz1 * dy2;
    ny = dz1 * dx2 - dx1 * dz2;
    nz = dx1 * dy2 - dy1 * dx2;
    if (fabs(nx) > fabs(ny)) return fabs(nx) > fabs(nz) ? 0 : 2;
    return fabs(ny) > fabs(nz) ? 1 : 2;
}

/* 0x80093118, size 0x81c */
static void createGridMap(void)
{
    unsigned int *srcList;
    unsigned int *dstList;
    unsigned int size;
    char carName[32];
    int carMBIdx;
    tModelDef *carRomObjDef;
    tTexture *gridTexDef;
    int endGeo;
    unsigned int header;
    unsigned int command;
    anon_pos_uv_24 vertBuff[3];
    unsigned int buffIdx = 0;
    unsigned int vertIdx = 0;
    float posBuff[256][3];
    float normalBuff[256][3];
    int alignAxis = -1;
    int normals;
    unsigned int posIdx;
    unsigned int colorIdx;
    if (gridCarIdx == gThisCar[0]) return;
    if (FindWaveInstance(gThisCar[0] + 64) == -1) return;
    releaseGridMap();
    size = LargestAvailableMemoryHeapChunk(0);
    gridLOD.displayList = AllocateMemoryHeap(0, size);
    gridCarIdx = gThisCar[0];
    sprintf(carName, "%sFRAME1", CarNames[gThisCar[0]]);
    carMBIdx = MBOX_FindObject_WORK(carName, 0, gNumModels-1, 1);
    carRomObjDef = &gRomObjDefPtrs[carMBIdx >> 10].romObjDefs[carMBIdx & 1023];
    gridTexDef = MBOX_FindTexture_WORK("GREEN_GRID", &gridLOD.texidx, 0, gNumModels-1, 1);
    gridLOD.flags = 1;
    gridLOD.lodRad = 1000.0f;
    dstList = gridLOD.displayList;
    endGeo = 0;
    gridLOD.flags = carRomObjDef->lod[0].flags;
    normals = gridLOD.flags & 16;
    gridLOD.lodRad = carRomObjDef->lod[0].lodRad;
    srcList = carRomObjDef->lod[0].displayList;
    while (!endGeo) {
        header = *srcList++;
        command = header & 0xe0000000;
        switch (command) {
        case 0xe0000000:
            *dstList++ = header;
            endGeo = 1;
            break;
        case 0xa0000000:
            *dstList++ = header;
            srcList++;
            *dstList++ = gridTexDef->strip;
            break;
        case 0:
        case 0x20000000:
            posIdx = (header >> 16) & 255;
            colorIdx = header & 255;
            if (!(header & 0x10000000)) {
                posBuff[posIdx][0] = ((float *)srcList)[0];
                posBuff[posIdx][1] = ((float *)srcList)[1];
                posBuff[posIdx][2] = ((float *)srcList)[2];
                srcList += 3;
            }
            if (!(header & 0x08000000)) srcList += 2;
            if (!(header & 0x04000000)) {
                if (normals) {
                    normalBuff[colorIdx][0] = ((float *)srcList)[0];
                    normalBuff[colorIdx][1] = ((float *)srcList)[1];
                    normalBuff[colorIdx][2] = ((float *)srcList)[2];
                    srcList += 3;
                } else srcList++;
            }
            if (buffIdx == 3) {
                GRID_VERTEX(vertBuff[0], 0);
                memcpy(&vertBuff[0], &vertBuff[1], sizeof(vertBuff[0]));
                memcpy(&vertBuff[1], &vertBuff[2], sizeof(vertBuff[0]));
                buffIdx--;
            }
            vertBuff[buffIdx].pos[0] = posBuff[posIdx][0];
            vertBuff[buffIdx].pos[1] = posBuff[posIdx][1];
            vertBuff[buffIdx].pos[2] = posBuff[posIdx][2];
            if (normals) {
                vertBuff[buffIdx].normal[0] = normalBuff[colorIdx][0];
                vertBuff[buffIdx].normal[1] = normalBuff[colorIdx][1];
                vertBuff[buffIdx].normal[2] = normalBuff[colorIdx][2];
            } else vertBuff[buffIdx].color = 0xffffffff;
            buffIdx++;
            if (buffIdx >= 3) {
                int axis = gridAlignAxis(vertBuff[0].pos, vertBuff[1].pos, vertBuff[2].pos);
                if (alignAxis != -1 && axis != alignAxis) {
                    GRID_VERTEX(vertBuff[0], 0);
                    GRID_VERTEX(vertBuff[1], 0x20000000);
                }
                alignAxis = axis;
                if (alignAxis == 0) {
                    vertBuff[0].uv[0] = vertBuff[0].pos[1]; vertBuff[0].uv[1] = vertBuff[0].pos[2];
                    vertBuff[1].uv[0] = vertBuff[1].pos[1]; vertBuff[1].uv[1] = vertBuff[1].pos[2];
                    vertBuff[2].uv[0] = vertBuff[2].pos[1]; vertBuff[2].uv[1] = vertBuff[2].pos[2];
                } else if (alignAxis == 1) {
                    vertBuff[0].uv[0] = vertBuff[0].pos[0]; vertBuff[0].uv[1] = vertBuff[0].pos[2];
                    vertBuff[1].uv[0] = vertBuff[1].pos[0]; vertBuff[1].uv[1] = vertBuff[1].pos[2];
                    vertBuff[2].uv[0] = vertBuff[2].pos[0]; vertBuff[2].uv[1] = vertBuff[2].pos[2];
                } else if (alignAxis == 2) {
                    vertBuff[0].uv[0] = vertBuff[0].pos[0]; vertBuff[0].uv[1] = vertBuff[0].pos[1];
                    vertBuff[1].uv[0] = vertBuff[1].pos[0]; vertBuff[1].uv[1] = vertBuff[1].pos[1];
                    vertBuff[2].uv[0] = vertBuff[2].pos[0]; vertBuff[2].uv[1] = vertBuff[2].pos[1];
                }
                if (command == 0x20000000) {
                    GRID_VERTEX(vertBuff[0], 0);
                    GRID_VERTEX(vertBuff[1], 0);
                    GRID_VERTEX(vertBuff[2], 0x20000000);
                    buffIdx = 0;
                    vertIdx = 0;
                    alignAxis = -1;
                }
            }
            break;
        }
    }
    ReallocateMemory(gridLOD.displayList, (unsigned char *)dstList - (unsigned char *)gridLOD.displayList);
}


/* 0x80093934, size 0x58 */
static void releaseGridMap(void)
{
    if (gridLOD.displayList) {
        showGridMap(0);
        DeallocateMemory(gridLOD.displayList);
    }
    gridLOD.displayList = 0;
    gridCarIdx = -1;
}


/* 0x8009398c, size 0x184 */
static void showGridMap(int visible)
{
    char carName[32];
    int carMBIdx;
    tModelDef *carRomObjDef;
    if (gridVisible != visible && gridCarIdx != -1) {
        sprintf(carName, "%sFRAME1", CarNames[gThisCar[0]]);
        carMBIdx = MBOX_FindObject_WORK(carName, 0, gNumModels - 1, 1);
        carRomObjDef = &gRomObjDefPtrs[carMBIdx >> 10].romObjDefs[carMBIdx & 1023];
        if (visible == 1) {
            SetSheen(gThisCar[0], 0, -1);
            savedLOD = carRomObjDef->lod[0];
            carRomObjDef->lod[0] = gridLOD;
        } else {
            SetSheen(gThisCar[0], 0, gCarSheen[0]);
            carRomObjDef->lod[0] = savedLOD;
        }
        gridVisible = visible;
    }
    if (visible == 1) {
        tRGBA solidColor = { { 255, 255, 255, 64 } };
        if (gCarParts[gThisCar[0]].frame) ZOID_SetColor((signed short)gCarParts[gThisCar[0]].frame, &solidColor);
    }
}


/* 0x80093b10, size 0x564 */
static void initButtons(signed char p)
{
    tRGBA buttonColor = { {255, 255, 255, 255} };
    tTexture *tex;
    unsigned short texIdx;
    int buttonIdx;
    static float zoom = 1.0f;
    if (gNumWindows == 1) zoom = 1.0f;
    for (buttonIdx = 0; buttonIdx < 40; buttonIdx++) {
        if (buttonInfo[p][buttonIdx].objIdx == -1) {
            memcpy(buttonInfo[p][buttonIdx].mat.mat3.uvs, unitmatrix, 36);
            buttonInfo[p][buttonIdx].objIdx = MBOX_NewObject(
                MBOX_FindObject_WORK(buttonNames[buttonInfo[p][buttonIdx].buttonType], 0, gNumModels - 1, 1),
                &buttonInfo[p][buttonIdx].mat, -1,
                (buttonInfo[p][buttonIdx].buttonType == 0 || buttonInfo[p][buttonIdx].buttonType == 1 || buttonInfo[p][buttonIdx].buttonType == 2) ? 0x42000 : 0);
            ZOID_SetColor((signed short)buttonInfo[p][buttonIdx].objIdx, &buttonColor);
            MBOX_HideObject(buttonInfo[p][buttonIdx].objIdx, 1, 15);
            MBOX_ShowObject(buttonInfo[p][buttonIdx].objIdx, 0, 1 << p);
        }
    }
    tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture((signed short)buttonInfo[p][34].objIdx, tex, -1);
    memcpy(buttonInfo[p][34].mat.mat3.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[p][34].mat.mat3.uvs);
    buttonInfo[p][34].mat.mat3.pos[0] = 0.0f;
    buttonInfo[p][34].mat.mat3.pos[1] = gNumWindows > 1 ? 105.0f : 115.0f;
    buttonInfo[p][34].mat.mat3.pos[2] = 200.0f;
    buttonInfo[p][34].mat.mat3.pos[0] = (1.0f / zoom) * buttonInfo[p][34].mat.mat3.pos[0];
    buttonInfo[p][34].mat.mat3.pos[1] = (1.0f / zoom) * buttonInfo[p][34].mat.mat3.pos[1];
    buttonInfo[p][34].mat.mat3.pos[2] = (1.0f / zoom) * buttonInfo[p][34].mat.mat3.pos[2];
    if (gRace == 2) MBOX_ShowObject(buttonInfo[p][34].objIdx, 0, 15);
    tex = MBOX_FindTexture_WORK("CURSOR", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture((signed short)buttonInfo[p][35].objIdx, tex, -1);
    tex = MBOX_FindTexture_WORK("A1", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture((signed short)buttonInfo[p][36].objIdx, tex, -1);
    memcpy(buttonInfo[p][36].mat.mat3.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[p][36].mat.mat3.uvs);
    buttonInfo[p][36].mat.mat3.pos[0] = 75.1f;
    buttonInfo[p][36].mat.mat3.pos[1] = 4.0f + (gNumWindows > 1 ? 50.2f : 55.2f);
    buttonInfo[p][36].mat.mat3.pos[2] = 100.0f;
    ScaleUV(buttonInfo[p][36].mat.mat3.uvs, buttonInfo[p][36].mat.mat3.uvs, 0.5f);
    memcpy(buttonInfo[p][37].mat.mat3.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[p][37].mat.mat3.uvs);
    WRollUV(3.1415927f, buttonInfo[p][37].mat.mat3.uvs);
    buttonInfo[p][37].mat.mat3.pos[0] = 87.1f;
    buttonInfo[p][37].mat.mat3.pos[1] = 4.0f + (gNumWindows > 1 ? 50.2f : 55.2f);
    buttonInfo[p][37].mat.mat3.pos[2] = 100.0f;
    tex = MBOX_FindTexture_WORK("B1", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture((signed short)buttonInfo[p][38].objIdx, tex, -1);
    memcpy(buttonInfo[p][38].mat.mat3.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[p][38].mat.mat3.uvs);
    buttonInfo[p][38].mat.mat3.pos[0] = -75.1f;
    buttonInfo[p][38].mat.mat3.pos[1] = 4.0f + (gNumWindows > 1 ? 50.2f : 55.2f);
    buttonInfo[p][38].mat.mat3.pos[2] = 100.0f;
    buttonInfo[p][38].mat.mat3.pos[0] = (1.0f / zoom) * buttonInfo[p][38].mat.mat3.pos[0];
    buttonInfo[p][38].mat.mat3.pos[1] = (1.0f / zoom) * buttonInfo[p][38].mat.mat3.pos[1];
    buttonInfo[p][38].mat.mat3.pos[2] = (1.0f / zoom) * buttonInfo[p][38].mat.mat3.pos[2];
    ScaleUV(buttonInfo[p][38].mat.mat3.uvs, buttonInfo[p][38].mat.mat3.uvs, 0.5f);
    memcpy(buttonInfo[p][39].mat.mat3.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[p][39].mat.mat3.uvs);
    buttonInfo[p][39].mat.mat3.pos[0] = -87.1f;
    buttonInfo[p][39].mat.mat3.pos[1] = 4.0f + (gNumWindows > 1 ? 50.2f : 55.2f);
    buttonInfo[p][39].mat.mat3.pos[2] = 100.0f;
    buttonInfo[p][39].mat.mat3.pos[0] = (1.0f / zoom) * buttonInfo[p][39].mat.mat3.pos[0];
    buttonInfo[p][39].mat.mat3.pos[1] = (1.0f / zoom) * buttonInfo[p][39].mat.mat3.pos[1];
    buttonInfo[p][39].mat.mat3.pos[2] = (1.0f / zoom) * buttonInfo[p][39].mat.mat3.pos[2];
    buttonsInited[p] = 1;
    buttonsJustInited[p] = 1;
    ZeroCameras();
}


/* 0x80094074, size 0x94 */
static void uninitButtons(signed char p)
{
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 40; buttonIdx++) {
        if (buttonInfo[p][buttonIdx].objIdx != -1) {
            ZOID_RemoveObject((signed short)buttonInfo[p][buttonIdx].objIdx);
            buttonInfo[p][buttonIdx].objIdx = -1;
        }
    }
    buttonsInited[p] = 0;
}


/* 0x80094108, size 0x8b8 */
static void updateButtons(signed char p)
{
    static float cursorSpeed[4];
    static float scrollSpeed[4];
    int buttonIdx;
    float prevPos[3];
    float buttonY;
    float selectedPos;
    signed char clearScrollSpeed = 1;
    tTexture *tex;
    unsigned short texIdx;
    tRGBA buttonColor = { {255,255,255,255} };
    if (!buttonsInited[p]) return;
    if (gButtonScale * (gButtonSpace * (gNumOptions[p] - 1)) > maxY - minY)
        selectedPos = maxY - gActiveOption[p] * (maxY - minY) / (gNumOptions[p] - 1);
    else selectedPos = maxY - gButtonScale * (gButtonSpace * gActiveOption[p]);
    memcpy(prevPos, buttonInfo[p][35].mat.mat3.pos, 12);
    buttonInfo[p][35].angle += 6.2831855f * gFrameSeconds;
    if (buttonInfo[p][35].angle > 6.2831855f) buttonInfo[p][35].angle -= 6.2831855f;
    buttonInfo[p][35].mat.mat3.pos[0] = buttonX * cursorZ / buttonZ + -170.0f * gButtonScale;
    buttonInfo[p][35].mat.mat3.pos[1] = selectedPos * cursorZ / buttonZ;
    buttonInfo[p][35].mat.mat3.pos[2] = cursorZ;
    if (!buttonsJustInited[p]) {
        if (gButtonScale * (60.0 * fabs(buttonInfo[p][35].mat.mat3.pos[1] - prevPos[1])) / 10.0 > cursorSpeed[p]) {
            cursorSpeed[p] = gButtonScale * (60.0 * fabs(buttonInfo[p][35].mat.mat3.pos[1] - prevPos[1])) / 10.0;
            cursorSpeed[p] = cursorSpeed[p] < 540.0f * gButtonScale ? cursorSpeed[p] : 540.0f * gButtonScale;
            cursorSpeed[p] = cursorSpeed[p] > 180.0f * gButtonScale ? cursorSpeed[p] : 180.0f * gButtonScale;
        }
        if (buttonInfo[p][35].mat.mat3.pos[1] > prevPos[1] + gFrameSeconds * cursorSpeed[p])
            buttonInfo[p][35].mat.mat3.pos[1] = prevPos[1] + gFrameSeconds * cursorSpeed[p];
        else if (buttonInfo[p][35].mat.mat3.pos[1] < prevPos[1] - gFrameSeconds * cursorSpeed[p])
            buttonInfo[p][35].mat.mat3.pos[1] = prevPos[1] - gFrameSeconds * cursorSpeed[p];
        else cursorSpeed[p] = 180.0f * gButtonScale;
    }
    ScaleUV(unitmatrix, buttonInfo[p][35].mat.mat3.uvs, 0.4f * gButtonScale);
    WRollUV(buttonInfo[p][35].angle, buttonInfo[p][35].mat.mat3.uvs);
    WPitchUV(1.5707964f, buttonInfo[p][35].mat.mat3.uvs);
    buttonY = selectedPos + gButtonScale * (gButtonSpace * gActiveOption[p]);
    for (buttonIdx = 0; buttonIdx < 17; buttonIdx++) {
        if (buttonIdx == gOpts[p][gActiveOption[p]]) {
            if (buttonInfo[p][buttonIdx].angle < 3.1415927f) buttonInfo[p][buttonIdx].angle += 12.566371f * gFrameSeconds;
            if (buttonInfo[p][buttonIdx].angle > 3.1415927f) buttonInfo[p][buttonIdx].angle = 3.1415927f;
            if (buttonsJustInited[p]) buttonInfo[p][buttonIdx].angle = 3.1415927f;
        } else {
            if (buttonInfo[p][buttonIdx].angle > 0.08726646f) buttonInfo[p][buttonIdx].angle -= 12.566371f * gFrameSeconds;
            if (buttonInfo[p][buttonIdx].angle < 0.08726646f) buttonInfo[p][buttonIdx].angle = 0.08726646f;
            if (buttonsJustInited[p]) buttonInfo[p][buttonIdx].angle = 0.08726646f;
        }
        memcpy(prevPos, buttonInfo[p][buttonIdx].mat.mat3.pos, 12);
        buttonInfo[p][buttonIdx].mat.mat3.pos[0] = buttonX + -120.0f * gButtonScale;
        buttonInfo[p][buttonIdx].mat.mat3.pos[1] = buttonY;
        buttonInfo[p][buttonIdx].mat.mat3.pos[2] = buttonZ;
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
        buttonInfo[p][buttonIdx+17].mat.mat3.pos[0] = buttonX + 120.0f * gButtonScale;
        buttonInfo[p][buttonIdx+17].mat.mat3.pos[1] = buttonInfo[p][buttonIdx].mat.mat3.pos[1];
        buttonInfo[p][buttonIdx+17].mat.mat3.pos[2] = buttonInfo[p][buttonIdx].mat.mat3.pos[2];
        buttonInfo[p][buttonIdx+17].angle = buttonInfo[p][buttonIdx].angle;
        ScaleUV(unitmatrix, buttonInfo[p][buttonIdx].mat.mat3.uvs, gButtonScale);
        ScaleUV(unitmatrix, buttonInfo[p][buttonIdx+17].mat.mat3.uvs, gButtonScale);
        WPitchUV(buttonInfo[p][buttonIdx].angle - 1.5707964f, buttonInfo[p][buttonIdx].mat.mat3.uvs);
        WPitchUV(buttonInfo[p][buttonIdx+17].angle - 1.5707964f, buttonInfo[p][buttonIdx+17].mat.mat3.uvs);
        if (buttonInfo[p][buttonIdx].angle > 1.5707964f) tex = MBOX_FindTexture_WORK("BUTTON_SELECT", &texIdx, 0, gNumModels - 1, 1);
        else tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels - 1, 1);
        ZOID_SetTexture((signed short)buttonInfo[p][buttonIdx].objIdx, tex, -1);
        ZOID_SetTexture((signed short)buttonInfo[p][buttonIdx+17].objIdx, tex, -1);
        if (buttonInfo[p][buttonIdx].mat.mat3.pos[1] > maxY + gButtonSpace * gButtonScale || buttonInfo[p][buttonIdx].mat.mat3.pos[1] < minY - gButtonSpace * gButtonScale) buttonInfo[p][buttonIdx].alpha = 0;
        else if (buttonInfo[p][buttonIdx].mat.mat3.pos[1] > maxY) buttonInfo[p][buttonIdx].alpha = 80;
        else if (buttonInfo[p][buttonIdx].mat.mat3.pos[1] < minY) buttonInfo[p][buttonIdx].alpha = 80;
        else buttonInfo[p][buttonIdx].alpha = 255;
        if (!validOption(p, buttonIdx)) buttonInfo[p][buttonIdx].alpha = 0;
        buttonColor.c[3] = buttonInfo[p][buttonIdx+17].alpha = buttonInfo[p][buttonIdx].alpha;
        ZOID_SetColor((signed short)buttonInfo[p][buttonIdx].objIdx, &buttonColor);
        ZOID_SetColor((signed short)buttonInfo[p][buttonIdx+17].objIdx, &buttonColor);
        if (!validOption(p, buttonIdx)) {
            MBOX_HideObject(buttonInfo[p][buttonIdx].objIdx, 1, 15);
            MBOX_HideObject(buttonInfo[p][buttonIdx+17].objIdx, 1, 15);
        } else if (!buttonInfo[p][buttonIdx].alpha || gCarLocked[p] == 1) {
            MBOX_HideObject(buttonInfo[p][buttonIdx].objIdx, 1, 15);
            MBOX_HideObject(buttonInfo[p][buttonIdx+17].objIdx, 1, 15);
            buttonY -= gButtonSpace * gButtonScale;
        } else {
            MBOX_ShowObject(buttonInfo[p][buttonIdx].objIdx, 0, 1 << p);
            MBOX_ShowObject(buttonInfo[p][buttonIdx+17].objIdx, 0, 1 << p);
            buttonY -= gButtonSpace * gButtonScale;
        }
    }
    if (gCarLocked[p] == 1) {
        MBOX_HideObject(buttonInfo[p][36].objIdx, 1, 15);
        MBOX_HideObject(buttonInfo[p][37].objIdx, 1, 15);
        buttonInfo[p][35].mat.mat3.pos[2] = 2000.0f;
    } else {
        MBOX_ShowObject(buttonInfo[p][36].objIdx, 0, 1 << p);
        MBOX_ShowObject(buttonInfo[p][37].objIdx, 0, 1 << p);
    }
    if (allLocked == 1) {
        MBOX_HideObject(buttonInfo[p][38].objIdx, 1, 15);
        MBOX_HideObject(buttonInfo[p][39].objIdx, 1, 15);
    } else {
        MBOX_ShowObject(buttonInfo[p][38].objIdx, 0, 1 << p);
        MBOX_ShowObject(buttonInfo[p][39].objIdx, 0, 1 << p);
    }
    if (clearScrollSpeed == 1) scrollSpeed[p] = 0.0f;
    buttonsJustInited[p] = 0;
}


/* 0x800949c0, size 0x1ac */
static tGhostFile **getNextGhost(tGhostFile **currGhost, signed char whichWay)
{
    tGhostFile **nextGhost;
    if (!currGhost) {
        if (whichWay > 0) {
            currGhost = gGhostFiles.head;
            if (!currGhost) currGhost = limboGhosts;
        } else currGhost = limboGhosts + 2;
    }
    while (currGhost && *currGhost) {
        if (whichWay > 0) nextGhost = (*currGhost)->ll.next;
        else nextGhost = (*currGhost)->ll.prev;
        if ((*currGhost)->track == gTrackNum && (*currGhost)->backward == gBackwards &&
            currGhost != gGhosts[1] && currGhost != gGhosts[2] && currGhost != gGhosts[3] &&
            (!(*currGhost)->fileh || ReadGhostData(currGhost))) {
            if ((*currGhost)->datah) break;
        }
        if (!nextGhost) {
            if (currGhost == limboGhosts) {
                if (whichWay > 0) nextGhost = limboGhosts + 1;
                else nextGhost = gGhostFiles.tail;
            } else if (currGhost == limboGhosts + 1) {
                if (whichWay > 0) nextGhost = limboGhosts + 2;
                else nextGhost = limboGhosts;
            } else if (currGhost == limboGhosts + 2) {
                if (whichWay < 0) nextGhost = limboGhosts + 1;
            } else if (whichWay > 0) nextGhost = limboGhosts;
        }
        currGhost = nextGhost;
    }
    if (currGhost && !*currGhost) currGhost = 0;
    return currGhost;
}









