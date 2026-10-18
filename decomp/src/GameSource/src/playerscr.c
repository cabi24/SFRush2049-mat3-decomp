/* playerscr.c - player selection screen: create, delete and claim players, memory-card states. Status: Matching.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\playerscr.c; assembly: build/2049LE/asm/GameSource/src/playerscr.s */
#define TB_FlashTextColors TB_FlashTextColors_pointer_declaration
#include "rush/rush_protos.h"
#undef TB_FlashTextColors
void TB_FlashTextColors(tRGBA color1, tRGBA color2);
#define abs(x) ((x) >= 0 ? (x) : -(x))
#define MSL_NO_SQRTF
#include "msl_math_inline.h"
#define PLAYER_MIN(a,b) ((a) < (b) ? (a) : (b))
#define PLAYER_MAX(a,b) ((a) > (b) ? (a) : (b))

static void initScreen(void);
static void uninitScreen(void);
static void updatePlayer(int p);
static void controlSelect(int p);
static void controlWait(int p);
static void controlControls(int p);
static void controlFormatMC(int p);
static void controlCreatePlayer(int p);
static void controlDeletePlayer(int p);
static void controlPakFull(int p);
static void controlPakDamaged(int p);
static void drawSelect(int p);
static void drawWait(int p);
static void drawControls(int p);
static void drawCreatePlayer(int p);
static void drawDeletePlayer(int p);
static void drawFormatMC(int p);
static void drawPakFull(int p);
static void drawPakDamaged(int p);
static int animText(void);
static void setPlayerState(int p, enum_kPlayerStateCount newState);
static void updateSelectInfo(int p);
static void updatePakInfo(void);
static void calcSelectMenuMaxSize(void);
static void claimPlayer(int playerIdx, tPlayerProfile **player);
static int playerClaimed(int playerIdx, tPlayerProfile **player);
static int playerBeingCreated(int playerIdx);
static int playerBeingDeleted(int playerIdx);
static void initButtons(signed char p);
static void uninitButtons(signed char p);
static void updateButtons(signed char p);

static char memcard[18] = { 77, 69, 77, 79, 82, 89, 32, 67, 65, 82, 68, 32, 40, 78, 71, 67, 41, 0 };
static char *buttonNames[12] = {
    "BUTTON_S2G1", "BUTTON_S2G1", "BUTTON_S2G1", "CURSORG1", "AB_BUTTONG1", "ARROW_BGG1",
    "ARROW_FGG1", "BUTTON_S4G1", "BUTTON_S0G1", "BUTTON_S8G1", "BAR_BGG1", "BAR_FGG1"
};
static tMenuButton buttonInfo[4][23] = {
 {
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 9, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 8, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 }
 },
 {
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 9, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 8, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 }
 },
 {
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 9, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 8, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 }
 },
 {
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 9, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 7, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 0, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 8, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 2, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 3, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 4, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 },
{ 6, -1, 0.0f, { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, 0 }
 }
};
static tBlitDef multiBlit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animText, 0 };
static anon_x_y_18 gPlayerRects[4][4] = {
    40, 20, 240, 200, 160, 120, 0, 0, 320, 220, 160, 110, 0, 0, 320, 220, 160, 110, 0, 0, 320, 220,
    160, 110, 60, 20, 200, 100, 160, 70, 60, 120, 200, 100, 160, 170, 0, 0, 320, 220, 160, 110, 0,
    0, 320, 220, 160, 110, 0, 20, 160, 100, 80, 70, 160, 20, 160, 100, 240, 70, 0, 120, 160, 100,
    80, 170, 0, 0, 320, 220, 160, 110, 0, 20, 160, 100, 80, 70, 160, 20, 160, 100, 240, 70, 0, 120,
    160, 100, 80, 170, 160, 120, 160, 100, 240, 170
};
static float gButtonZoom = 1.0f;
static float gButtonScale = 1.0f;
static float gButtonSpace = 40.0f;
static float maxY = 120.0f;
static float minY = -120.0f;
static float buttonZ = 300.0f;
static float cursorZ = 225.0f;
static signed short gNameSelect[4];
static signed short gNameLen[4];
static signed char gSelectMenuMaxSize;
static signed char gSelectMenuCount[4];
static signed char gSelectMenuControls[4];
static signed char gSelectPakIdx[4];
static signed char gSelectPlayerNum[4];
static signed char gDeleteMenuSelect[4];
static signed char gDeleteMenuConfirm[4];
static signed char gDeleteMenuYes[4];
static signed char gDeleteMenuNotify[4];
static signed char gFormatMenuConfirm[4];
static signed char gFormatMenuYes[4];
static signed char gFormatMenuNotify[4];
static signed char gFormatMenuEnd[4];
static signed char gCreateMenuNotify[4];
static _tBlit_2 *blit;
static enum_kNumMenuDir_8 exitPlayerScreen;
static signed char screenInited;
static float buttonX;

static float gCameraMatrix[4][4];
static float scrollSpeed[4];
static float cursorSpeed[4];
static int buttonsInited[4];
static signed char gSelectMenuVisible[4][15];
static unsigned char gDeleteMenuName[4][13];
static unsigned char gCreatePlayerNames[4][13];
static int gPakFull[4];
static int gNumPakPlayers[4];
static int buttonsJustInited[4];
static signed char gSelectMenuValid[4][15];
static tBlitSet *pane[4];
static enum_kSelectMenuCount gSelectMenuSelect[4];
static enum_kPlayerStateCount gPlayerState[4];
static tPlayerProfile **gSelectedPlayer[4];
signed char gDefaultControls_Add[11];

/* 0x80079e98, size 0xe8 */
void ChooseNewControl(int WitchPlayer)
{
    if (g_CurrentControl_t[WitchPlayer] == 0)
        SetDefaultControls(gDefaultControls_Add);
    else
        SetDefaultControls2(gDefaultControls_Add);
    SetPlayerControls(WitchPlayer, gDefaultControls_Add);
    if (gSelectedPlayer[WitchPlayer])
        memcpy((*gSelectedPlayer[WitchPlayer])->controls, gDefaultControls_Add, 11);
    if (gJustPlayers[WitchPlayer])
        memcpy(gJustPlayers[WitchPlayer]->controls, gDefaultControls_Add, 11);
    memcpy(gNewControls[WitchPlayer], gDefaultControls_Add, 11);
}

/* 0x80079f80, size 0x51c */
void updatePlayerScreen(void)
{
    int playerIdx;
    int p;
    while (1) {
        if (gSkipToGameState != 1) {
            if (gSkipToGameState != gGameState) {
                int i;
                for (i = 0; i < gNumPlayers; i++) {
                    if (gPlayers[i].save != &gJustPlayers[i]) {
                        tPlayerProfile **player;
                        for (player = gPlayerFiles.head; player; player = (*player)->ll.next)
                            if (gPlayers[i].save == player) break;
                        if (!player) {
                            gSkipToGameState = 1;
                            if (gOptions[14] <= 18) {
                                AudSetMusicVol(gOptions[12] / 10.0f, 0.05f);
                                AudPlaySong(19, 1);
                            }
                            goto restart;
                        }
                    }
                    if (i > 1) SetControllerStickEdges(i, 1);
                    FindPlayerNum(i, 1);
                }
                for (; i < 4; i++)
                    if (i > 1) SetControllerStickEdges(i, 0);
                CalcUnlockedStuff();
                setGameState(64);
                return;
            }
            gSkipToGameState = 1;
        }
        break;
restart:;
    }
    if (!screenInited) initScreen();
    for (playerIdx = 0; playerIdx < gNumPlayers; playerIdx++) {
        updatePlayer(playerIdx);
        switch (gPlayerState[playerIdx]) {
        case kPlayerStateSelect: controlSelect(playerIdx); break;
        case kPlayerStateWait: controlWait(playerIdx); break;
        case kPlayerStateCreatePlayer: controlCreatePlayer(playerIdx); break;
        case kPlayerStateDeletePlayer: controlDeletePlayer(playerIdx); break;
        case kPlayerStatePakFull: controlPakFull(playerIdx); break;
        case kPlayerStatePakDamaged: controlPakDamaged(playerIdx); break;
        case kPlayerStateFormatMC: controlFormatMC(playerIdx); break;
        }
    }
    updateControlScreen();
    for (playerIdx = 0; playerIdx < gNumPlayers; playerIdx++) {
        if (gPlayerState[playerIdx] == kPlayerStateControls)
            controlControls(playerIdx);
        updateButtons(playerIdx);
    }
    {
        signed char allReady = 1;
        int i;
        for (i = 0; i < gNumPlayers; i++)
            if (gPlayerState[i] != kPlayerStateWait) allReady = 0;
        if (allReady == 1) exitPlayerScreen = kMenuForward;
    }
    if (gSoftReset) {
        uninitScreen();
        setGameState(4);
    } else if (exitPlayerScreen != kMenuStay) {
        switch (exitPlayerScreen) {
        case kMenuForward: {
            tPlayerProfile **player;

            for (player = gPlayerFiles.head; player; player = (*player)->ll.next) {
                int i;
                for (i = 0; i < gNumPlayers; i++)
                    if (player == gPlayers[i].save) break;
                if (i >= gNumPlayers) PurgePlayerData(player);
            }
            for (p = 0; p < gNumPlayers; p++) {
                if (!ReadPlayerData(gPlayers[p].save)) break;
                SaveSelectPlayer(gPlayers[p].save);
                PS_LockFile((*gPlayers[p].save)->fileh, 1);
                if (p > 1) SetControllerStickEdges(p, 1);
                FindPlayerNum(p, 1);
            }
            if (p < gNumPlayers) {
                int i;
                for (i = 0; i < gNumPlayers; i++) {
                    gSelectMenuSelect[i] = kSelectMenuSelectPlayer;
                    setPlayerState(i, kPlayerStateSelect);
                }
                break;
            }
            for (; p < 4; p++)
                if (p > 1) SetControllerStickEdges(p, 0);
            CalcUnlockedStuff();
            uninitScreen();
            if ((*gPlayers[0].save)->fileh)
                PS2MCUpdateSame((*(*gPlayers[0].save)->fileh)->pak);
            setGameState(64);
            break;
        }
        case kMenuBackward: {
            int i;
            uninitScreen();
            setGameState(16);
            for (i = 0; i < gNumPlayers; i++) gPlayers[i].save = 0;
            break;
        }
        }
        exitPlayerScreen = kMenuStay;
    }
    updatePakInfo();
    updateSelectInfo(-1);
}

/* 0x8007a49c, size 0x258 */
static void initScreen(void)
{
    int p;
    for (p = 0; p < gNumPlayers; p++) {
        if (gPlayers[p].save) {
            PS_DiscardFileChanges((*gPlayers[p].save)->fileh);
            PS_LockFile((*gPlayers[p].save)->fileh, 0);
        }
    }
    updatePakInfo();
    for (p = 0; p < gNumPlayers; p++) {
        gCarTeam[p] = p;
        gPlayers[p].controller = p;
        gSelectMenuSelect[p] = kSelectMenuSelectPlayer;
        setPlayerState(p, kPlayerStateSelect);
        calcSelectMenuMaxSize();
        initButtons(p);
    }
    updateSelectInfo(-1);
    gNumWindows = gNumPlayers;
    if (gNumWindows == 3) gNumWindows = 4;
    if (gNumWindows == 1) {
        gButtonZoom = 1.0f; gButtonScale = 1.0f; gButtonSpace = 40.0f;
        buttonX = 0.0f; maxY = 100.0f; minY = -120.0f;
        buttonZ = 300.0f; cursorZ = 225.0f;
    } else if (gNumWindows == 2) {
        gButtonZoom = 1.0f; gButtonScale = 1.43f; gButtonSpace = 40.0f;
        buttonX = 0.0f; maxY = 69.0f; minY = -120.0f;
        buttonZ = 300.0f; cursorZ = 225.0f;
    } else {
        gButtonZoom = 1.0f; gButtonScale = 1.1f; gButtonSpace = 40.0f;
        buttonX = 0.0f; maxY = 69.0f; minY = -120.0f;
        buttonZ = 300.0f; cursorZ = 225.0f;
    }
    drawWindowBorders = 0;
    ZOID_SplitMenuScreen(gNumWindows);
    createBG();
    for (p = 0; p < gNumPlayers; p++)
        pane[p] = InitPane(-20, 0, -10, 10, 192, 0, 0, 0);
    blit = (_tBlit_2 *)NewMultiBlit(0, 0, multiBlit, 1);
    exitPlayerScreen = kMenuStay;
    ClearRushControls();
    screenInited = 1;
}

/* 0x8007a6f4, size 0xbc */
static void uninitScreen(void)
{
    int i;
    for (i = 0; i < gNumPlayers; i++) {
        uninitButtons(i);
        if (pane[i]) {
            RemovePane(pane[i]);
            pane[i] = 0;
        }
    }
    cleanupControlScreen();
    if (blit) {
        RemoveBlit((_tBlit *)blit);
        blit = 0;
    }
    removeBG();
    gNumWindows = 1;
    HFOV = 90.0f;
    drawWindowBorders = 1;
    ZOID_ResetWindows();
    ZOID_Reset();
    screenInited = 0;
}

/* 0x8007a7b0, size 0xa4 */
static void updatePlayer(int p)
{
    tPlayerProfile **player;
    if (gSelectedPlayer[p] && gSelectedPlayer[p] != &gJustPlayers[p]) {
        for (player = gPlayerFiles.head; player; player = (*player)->ll.next)
            if (gSelectedPlayer[p] == player)
                break;
        if (!player) {
            gSelectedPlayer[p] = 0;
            updateSelectInfo(-1);
            setPlayerState(p, kPlayerStateSelect);
        }
    }
}

/* 0x8007a854, size 0x850 */
static void controlSelect(int p)
{
    while (!gSelectMenuValid[p][gSelectMenuSelect[p]] || playerClaimed(p, gSelectedPlayer[p]) == 1 ||
           (gSelectMenuSelect[p] == kSelectMenuCreatePlayer && playerBeingCreated(p) == 1) ||
           (gSelectMenuSelect[p] == kSelectMenuDeletePlayer && playerBeingDeleted(p) == 1)) {
        gSelectMenuControls[p] = 0;
        gSelectMenuSelect[p]--;
        if (gSelectMenuSelect[p] < 0) gSelectMenuSelect[p] = kSelectMenuCount - 1;
        if (gSelectMenuSelect[p] >= kSelectMenuSelectPlayer && gSelectMenuSelect[p] < kSelectMenuCreatePlayer &&
            gSelectMenuValid[p][gSelectMenuSelect[p]]) {
                if (gSelectedPlayer[p]) {
                    do {
                        gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.prev;
                    } while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak);
                }
                if (!gSelectedPlayer[p]) {
                    gSelectedPlayer[p] = gPlayerFiles.tail;
                    while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak)
                        gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.prev;
                }
        } else gSelectedPlayer[p] = 0;

    }
    if (gPlayers[p].edges & 1) {
        PlayStartABSound(gPlayers[p].edges);
        if (gSelectMenuSelect[p] >= kSelectMenuSelectPlayer && gSelectMenuSelect[p] < kSelectMenuCreatePlayer)
            claimPlayer(p, gSelectedPlayer[p]);
        else claimPlayer(p, &gJustPlayers[p]);
        setPlayerState(p, kPlayerStateWait);
    } else if (gPlayers[p].edges & 2) {
        PlayStartABSound(gPlayers[p].edges);
        if (gSelectMenuSelect[p] >= kSelectMenuSelectPlayer && gSelectMenuSelect[p] < kSelectMenuCreatePlayer) {

            claimPlayer(p, gSelectedPlayer[p]);
            if (gSelectMenuControls[p] == 1) {
                if (ReadPlayerData(gPlayers[p].save)) {
                    controlScrActive[p] = 1;
                    PS_LockFile((*gPlayers[p].save)->fileh, 1);
                    setPlayerState(p, kPlayerStateControls);
                }
            } else setPlayerState(p, kPlayerStateWait);
        
        } else switch (gSelectMenuSelect[p]) {
        case kSelectMenuJustPlay:
            claimPlayer(p, &gJustPlayers[p]);
            if (gSelectMenuControls[p] == 1) {
                controlScrActive[p] = 1;
                PS_LockFile((*gPlayers[p].save)->fileh, 1);
                setPlayerState(p, kPlayerStateControls);
            } else setPlayerState(p, kPlayerStateWait);
            break;
        case kSelectMenuSelectPak:
            break;
        case kSelectMenuPakStatus:
            break;
        case kSelectMenuCreatePlayer: setPlayerState(p, kPlayerStateCreatePlayer); break;
        case kSelectMenuDeletePlayer:
            gDeleteMenuSelect[p] = 0;
            setPlayerState(p, kPlayerStateDeletePlayer);
            break;
        case kSelectMenuResetPak: setPlayerState(p, kPlayerStatePakDamaged); break;
        case kSelectMenuFormatMC: setPlayerState(p, kPlayerStateFormatMC); break;
                }
    } else if (gPlayers[p].edges & 4) {
        PlayStartABSound(gPlayers[p].edges);
        exitPlayerScreen = kMenuBackward;
    } else if (gPlayers[p].autoEdges & 0x2000) {
        if (gSelectMenuSelect[p] == kSelectMenuJustPlay) return;
        if (gSelectMenuSelect[p] >= kSelectMenuSelectPlayer && gSelectMenuSelect[p] < kSelectMenuCreatePlayer) return;
        if (gSelectMenuSelect[p] == kSelectMenuSelectPak) {
            gSelectPakIdx[p]++;
            if (MtapConnected) {
                if (gSelectPakIdx[p] > 3) gSelectPakIdx[p] = 0;
            } else if (gSelectPakIdx[p] >= 2) gSelectPakIdx[p] = 0;
            updateSelectInfo(p);
            buttonsJustInited[p] = 1;
            PlayLRSound(gPlayers[p].edges);
        }
    } else if (gPlayers[p].autoEdges & 0x1000) {
        if (gSelectMenuSelect[p] == kSelectMenuJustPlay) return;
        if (gSelectMenuSelect[p] >= kSelectMenuSelectPlayer && gSelectMenuSelect[p] < kSelectMenuCreatePlayer) return;
        if (gSelectMenuSelect[p] == kSelectMenuSelectPak) {
            gSelectPakIdx[p]--;
            if (MtapConnected) {
                if (gSelectPakIdx[p] < 0) gSelectPakIdx[p] = 3;
            } else if (gSelectPakIdx[p] < 0) gSelectPakIdx[p] = 1;
            updateSelectInfo(p);
            buttonsJustInited[p] = 1;
            PlayLRSound(gPlayers[p].edges);
        }
    } else if (gPlayers[p].autoEdges & 0x400) {
        PlayUDSound(gPlayers[p].edges);
        gSelectMenuControls[p] = 0;
        do {
            if (gSelectMenuSelect[p] == kSelectMenuSelectPlayer && gSelectMenuValid[p][gSelectMenuSelect[p]]) {
                if (gSelectedPlayer[p]) {
                    do {
                        gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.prev;
                    } while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak);
                }
                if (gSelectedPlayer[p]) break;
            }
        gSelectMenuSelect[p]--;
        if (gSelectMenuSelect[p] < 0) gSelectMenuSelect[p] = kSelectMenuCount - 1;
        if (gSelectMenuSelect[p] >= kSelectMenuSelectPlayer && gSelectMenuSelect[p] < kSelectMenuCreatePlayer &&
            gSelectMenuValid[p][gSelectMenuSelect[p]]) {
                if (gSelectedPlayer[p]) {
                    do {
                        gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.prev;
                    } while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak);
                }
                if (!gSelectedPlayer[p]) {
                    gSelectedPlayer[p] = gPlayerFiles.tail;
                    while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak)
                        gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.prev;
                }
        } else gSelectedPlayer[p] = 0;

        } while (!gSelectMenuValid[p][gSelectMenuSelect[p]] || playerClaimed(p, gSelectedPlayer[p]) == 1 ||
           (gSelectMenuSelect[p] == kSelectMenuCreatePlayer && playerBeingCreated(p) == 1) ||
           (gSelectMenuSelect[p] == kSelectMenuDeletePlayer && playerBeingDeleted(p) == 1));
    } else if (gPlayers[p].autoEdges & 0x800) {
        PlayUDSound(gPlayers[p].edges);
        gSelectMenuControls[p] = 0;
        do {
            if (gSelectMenuSelect[p] == kSelectMenuCreatePlayer-1 && gSelectMenuValid[p][gSelectMenuSelect[p]]) {
                if (gSelectedPlayer[p]) {
                    do {
                        gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.next;
                    } while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak);
                }
                if (gSelectedPlayer[p]) break;
            }
        gSelectMenuSelect[p]++;
        if (gSelectMenuSelect[p] > kSelectMenuCount - 1) gSelectMenuSelect[p] = 0;
        if (gSelectMenuSelect[p] >= kSelectMenuSelectPlayer && gSelectMenuSelect[p] < kSelectMenuCreatePlayer &&
            gSelectMenuValid[p][gSelectMenuSelect[p]]) {
                if (gSelectedPlayer[p]) {
                    do {
                        gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.next;
                    } while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak);
                }
                if (!gSelectedPlayer[p]) {
                    gSelectedPlayer[p] = gPlayerFiles.head;
                    while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak)
                        gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.next;
                }
        } else gSelectedPlayer[p] = 0;

        } while (!gSelectMenuValid[p][gSelectMenuSelect[p]] || playerClaimed(p, gSelectedPlayer[p]) == 1 ||
           (gSelectMenuSelect[p] == kSelectMenuCreatePlayer && playerBeingCreated(p) == 1) ||
           (gSelectMenuSelect[p] == kSelectMenuDeletePlayer && playerBeingDeleted(p) == 1));
    }
}

/* 0x8007b0a4, size 0x80 */
static void controlWait(int p)
{
    if (!gPlayers[p].save || (gPlayers[p].edges & 4)) {
        if (gPlayers[p].edges & 4)
            PlayStartABSound(gPlayers[p].edges);
        updateSelectInfo(-1);
        setPlayerState(p, kPlayerStateSelect);
    }
}

/* 0x8007b124, size 0x94 */
static void controlControls(int p)
{
    if (!controlScrActive[p]) {
        SaveFlush(0);
        PS_LockFile((*gPlayers[p].save)->fileh, 0);
        PurgePlayerData(gPlayers[p].save);
        updatePakInfo();
        updateSelectInfo(-1);
        setPlayerState(p, kPlayerStateSelect);
        updateButtons(p);
    }
}

/* 0x8007b1b8, size 0x2ac */
static void controlFormatMC(int p)
{
    if (!gFormatMenuNotify[p] && gNumPakPlayers[gSelectPakIdx[p]] != -4) {
        gFormatMenuConfirm[p] = 0;
        gFormatMenuYes[p] = 0;
        gFormatMenuNotify[p] = 0;
        setPlayerState(p, kPlayerStateSelect);
        return;
    }
    if (gFormatMenuNotify[p] && gNumPakPlayers[gSelectPakIdx[p]] < 0) {
        gFormatMenuConfirm[p] = 0;
        gFormatMenuYes[p] = 0;
        gFormatMenuNotify[p] = 0;
        setPlayerState(p, kPlayerStateSelect);
        return;
    }
    if (gPlayers[p].edges & 3) {
        if (gFormatMenuNotify[p] == 1) {
            PlayStartABSound(gPlayers[p].edges);
            gFormatMenuNotify[p] = 0;
            setPlayerState(p, kPlayerStateSelect);
        } else if (gFormatMenuConfirm[p] == 1) {
            if (gFormatMenuYes[p] == 1) {
                PlayStartABSound(gPlayers[p].edges);
                PS2MCFormat(gSelectPakIdx[p]);
                updatePakInfo();
                updateSelectInfo(-1);
                gFormatMenuNotify[p] = 1;
            } else {
                PlayStartABSound(4);
                setPlayerState(p, kPlayerStateSelect);
            }
            gFormatMenuConfirm[p] = 0;
            gFormatMenuYes[p] = 0;
        }
    } else if (gPlayers[p].edges & 4) {
        PlayStartABSound(gPlayers[p].edges);
        if (gFormatMenuNotify[p] == 1) {
            gFormatMenuNotify[p] = 0;
            setPlayerState(p, kPlayerStateSelect);
        } else if (gFormatMenuConfirm[p] == 1) {
            gFormatMenuYes[p] = 0;
            setPlayerState(p, kPlayerStateSelect);
        } else setPlayerState(p, kPlayerStateSelect);
    }
    if (gPlayers[p].autoEdges & 0x2000) {
        if (gFormatMenuNotify[p] == 1) return;
        if (gFormatMenuConfirm[p] == 1) {
            PlayLRSound(gPlayers[p].edges);
            gFormatMenuYes[p] = !gFormatMenuYes[p];
        }
    } else if (gPlayers[p].autoEdges & 0x1000) {
        if (gFormatMenuNotify[p] == 1) return;
        if (gFormatMenuConfirm[p] == 1) {
            PlayLRSound(gPlayers[p].edges);
            gFormatMenuYes[p] = !gFormatMenuYes[p];
        }
    }
}

/* 0x8007b464, size 0x594 */
static void controlCreatePlayer(int p)
{
    tPlayerProfile **player;
    int pIdx;
    if (gCreateMenuNotify[p] == 1) {
        if (gPlayers[p].edges & 3) {
            PlayStartABSound(gPlayers[p].edges);
            updatePakInfo();
            updateSelectInfo(-1);
            setPlayerState(p, kPlayerStateSelect);
        }
        return;
    }
    if (gNumPakPlayers[gSelectPakIdx[p]] < 0 || gPakFull[gSelectPakIdx[p]])
        setPlayerState(p, kPlayerStateSelect);
    if (gPlayers[p].autoEdges & 0x400) {
        PlayUDSound(gPlayers[p].autoEdges);
        if (gNameSelect[p] == -1) gNameSelect[p] = 52;
        else if (gNameSelect[p] == -2) gNameSelect[p] = 49;
        else if (gNameSelect[p] == -3) gNameSelect[p] = 46;
        else if (gNameSelect[p] < 3) gNameSelect[p] = -3;
        else if (gNameSelect[p] < 6) gNameSelect[p] = -2;
        else if (gNameSelect[p] < 9) gNameSelect[p] = -1;
        else gNameSelect[p] -= 9;
    } else if (gPlayers[p].autoEdges & 0x800) {
        PlayUDSound(gPlayers[p].autoEdges);
        if (gNameSelect[p] == -1) gNameSelect[p] = 7;
        else if (gNameSelect[p] == -2) gNameSelect[p] = 4;
        else if (gNameSelect[p] == -3) gNameSelect[p] = 1;
        else if (gNameSelect[p] < 45) gNameSelect[p] += 9;
        else if (gNameSelect[p] < 48) gNameSelect[p] = -3;
        else if (gNameSelect[p] < 51) gNameSelect[p] = -2;
        else gNameSelect[p] = -1;
    } else if (gPlayers[p].autoEdges & 0x1000) {
        PlayLRSound(gPlayers[p].autoEdges);
        if (gNameSelect[p] == -1) gNameSelect[p] = -2;
        else if (gNameSelect[p] == -2) gNameSelect[p] = -3;
        else if (gNameSelect[p] == -3) gNameSelect[p] = -1;
        else if (gNameSelect[p] % 9 == 0) gNameSelect[p] += 8;
        else gNameSelect[p]--;
    } else if (gPlayers[p].autoEdges & 0x2000) {
        PlayLRSound(gPlayers[p].autoEdges);
        if (gNameSelect[p] == -1) gNameSelect[p] = -3;
        else if (gNameSelect[p] == -2) gNameSelect[p] = -1;
        else if (gNameSelect[p] == -3) gNameSelect[p] = -2;
        else if (gNameSelect[p] % 9 == 8) gNameSelect[p] -= 8;
        else gNameSelect[p]++;
    }
    if (gPlayers[p].edges & 3) {
        PlayStartABSound(gPlayers[p].edges);
        if (gCreateMenuNotify[p] == 1) return;
        if (gPlayers[p].edges & 1) gNameSelect[p] = -1;
        if (gNameSelect[p] == -2) {
            if (gNameLen[p] > 0) {
                gNameLen[p]--;
                gCreatePlayerNames[p][gNameLen[p]] = 0;
            }
        } else if (gNameSelect[p] == -1) {
            while (gCreatePlayerNames[p][gNameLen[p] - 1] == ' ') {
                gNameLen[p]--;
                gCreatePlayerNames[p][gNameLen[p]] = 0;
            }
            if (gCreatePlayerNames[p][0]) {
                player = CreatePlayer(gSelectPakIdx[p], gCreatePlayerNames[p]);
                if (player) {
                    claimPlayer(p, player);
                    gSelectMenuSelect[p] = kSelectMenuSelectPlayer;
                    gSelectedPlayer[p] = player;
                    for (pIdx = 0; pIdx < gNumPlayers; pIdx++) {
                        if (pIdx != p && gSelectPakIdx[pIdx] == gSelectPakIdx[p] &&
                            gSelectMenuSelect[pIdx] >= kSelectMenuSelectPlayer && gSelectMenuSelect[pIdx] < kSelectMenuCreatePlayer) {
                            gSelectMenuSelect[pIdx] = kSelectMenuSelectPak;
                            gSelectedPlayer[pIdx] = 0;
                        }
                    }
                    updatePakInfo();
                    updateSelectInfo(-1);
                    setPlayerState(p, kPlayerStateSelect);
                } else gCreateMenuNotify[p] = 1;
            } else setPlayerState(p, kPlayerStateSelect);
        } else if (gNameLen[p] < 12 && (gNameSelect[p] != -3 || gNameLen[p] > 0)) {
            if (gNameSelect[p] == -3)
                gCreatePlayerNames[p][gNameLen[p]] = ' ';
            else
                gCreatePlayerNames[p][gNameLen[p]] = gPlayerNameChars[gNameSelect[p]];
            gNameLen[p]++;
            gCreatePlayerNames[p][gNameLen[p]] = 0;
            if (gNameLen[p] == 12) gNameSelect[p] = -1;
        }
    } else if (gPlayers[p].edges & 4) {
        PlayStartABSound(gPlayers[p].edges);
        setPlayerState(p, kPlayerStateSelect);
    }
}

/* 0x8007b9f8, size 0x908 */
static void controlDeletePlayer(int p)
{
    signed char origSelect = gDeleteMenuSelect[p];
    int pIdx;
    tPlayerProfile **player;
    if (gNumPakPlayers[gSelectPakIdx[p]] <= 0) {
        setPlayerState(p, kPlayerStateSelect);
        return;
    }
    while (!gSelectMenuValid[p][gDeleteMenuSelect[p]] || playerClaimed(p, gSelectedPlayer[p]) == 1) {
        gDeleteMenuSelect[p]++;
        if (gDeleteMenuSelect[p] > 14) gDeleteMenuSelect[p] = 0;
        if (gDeleteMenuSelect[p] == origSelect) {
            setPlayerState(p, kPlayerStateSelect);
            return;
        }
        if (gDeleteMenuSelect[p] >= 3 && gDeleteMenuSelect[p] < 11 && gSelectMenuValid[p][gDeleteMenuSelect[p]]) {
                if (gSelectedPlayer[p]) {
                    do { gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.next; }
                    while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak);
                }
            if (!gSelectedPlayer[p]) {
                gSelectedPlayer[p] = gPlayerFiles.head;
                while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak)
                    gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.next;
            }
        } else gSelectedPlayer[p] = 0;
        buttonsJustInited[p] = 1;
    }
    if (gPlayers[p].edges & 1) {
        PlayStartABSound(gPlayers[p].edges);
        if (gDeleteMenuNotify[p] == 1) {
            gDeleteMenuNotify[p] = 0;
            setPlayerState(p, kPlayerStateSelect);
        } else if (gDeleteMenuConfirm[p] == 1) {
            gDeleteMenuConfirm[p] = 0;
            gDeleteMenuYes[p] = 0;
        } else setPlayerState(p, kPlayerStateSelect);
    } else if (gPlayers[p].edges & 2) {
        if (gDeleteMenuNotify[p] == 1) {
            PlayStartABSound(gPlayers[p].edges);
            gDeleteMenuNotify[p] = 0;
            setPlayerState(p, kPlayerStateSelect);
        } else if (gDeleteMenuConfirm[p] == 1) {
            if (gDeleteMenuYes[p] == 1) {
                PlayStartABSound(gPlayers[p].edges);
                DeletePlayer(gSelectedPlayer[p]);
                for (player = gPlayerFiles.head; player; player = (*player)->ll.next)
                    if (gSelectedPlayer[p] == player) break;
                if (!player) {
                    for (pIdx = 0; pIdx < gNumPlayers; pIdx++) {
                        if (pIdx != p && gSelectPakIdx[pIdx] == gSelectPakIdx[p] &&
                            gSelectMenuSelect[pIdx] >= 3 && gSelectMenuSelect[pIdx] < 11) {
                            gSelectMenuSelect[pIdx] = kSelectMenuSelectPak;
                            gSelectedPlayer[pIdx] = 0;
                        }
                    }
                    gSelectedPlayer[p] = 0;
                    updatePakInfo();
                    updateSelectInfo(-1);
                    gDeleteMenuSelect[p] = 0;
                    gDeleteMenuNotify[p] = 1;
                }
            } else PlayStartABSound(4);
            gDeleteMenuConfirm[p] = 0;
            gDeleteMenuYes[p] = 0;
        } else if (gDeleteMenuSelect[p] >= 3 && gDeleteMenuSelect[p] < 11) {
            PlayStartABSound(gPlayers[p].edges);
            TB_Strcpy(gDeleteMenuName[p], (*gSelectedPlayer[p])->name);
            gDeleteMenuConfirm[p] = 1;
            gDeleteMenuYes[p] = 0;
        }
    } else if (gPlayers[p].edges & 4) {
        PlayStartABSound(gPlayers[p].edges);
        if (gDeleteMenuNotify[p] == 1) {
            gDeleteMenuNotify[p] = 0;
            setPlayerState(p, kPlayerStateSelect);
        } else if (gDeleteMenuConfirm[p] == 1) {
            gDeleteMenuConfirm[p] = 0;
            gDeleteMenuYes[p] = 0;
        } else setPlayerState(p, kPlayerStateSelect);
    }
    if (gPlayers[p].autoEdges & 0x2000) {
        if (gDeleteMenuNotify[p] == 1) return;
        if (gDeleteMenuConfirm[p] == 1) {
            PlayLRSound(gPlayers[p].edges);
            gDeleteMenuYes[p] = !gDeleteMenuYes[p];
        } else if (gDeleteMenuSelect[p] == 1) {
            gSelectPakIdx[p]++;
            if (gSelectPakIdx[p] > 3) gSelectPakIdx[p] = 0;
            updateSelectInfo(p);
            PlayLRSound(gPlayers[p].edges);
        }
    } else if (gPlayers[p].autoEdges & 0x1000) {
        if (gDeleteMenuNotify[p] == 1) return;
        if (gDeleteMenuConfirm[p] == 1) {
            PlayLRSound(gPlayers[p].edges);
            gDeleteMenuYes[p] = !gDeleteMenuYes[p];
        } else if (gDeleteMenuSelect[p] == 1) {
            gSelectPakIdx[p]--;
            if (gSelectPakIdx[p] < 0) gSelectPakIdx[p] = 3;
            updateSelectInfo(p);
            PlayLRSound(gPlayers[p].edges);
        }
    } else if (gPlayers[p].autoEdges & 1024) {
        if (gDeleteMenuNotify[p] == 1) return;
        if (gDeleteMenuConfirm[p] == 1) return;
        PlayUDSound(gPlayers[p].edges);
        do {
            if (gDeleteMenuSelect[p] == 3 && gSelectMenuValid[p][gDeleteMenuSelect[p]]) {
                if (gSelectedPlayer[p]) {
                    do { gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.prev; }
                    while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak);
                }
                if (gSelectedPlayer[p]) break;
            }
            gDeleteMenuSelect[p]--;
            if (gDeleteMenuSelect[p] < 0) gDeleteMenuSelect[p] = 14;
        if (gDeleteMenuSelect[p] >= 3 && gDeleteMenuSelect[p] < 11 && gSelectMenuValid[p][gDeleteMenuSelect[p]]) {
                if (gSelectedPlayer[p]) {
                    do { gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.prev; }
                    while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak);
                }
            if (!gSelectedPlayer[p]) {
                gSelectedPlayer[p] = gPlayerFiles.tail;
                while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak)
                    gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.prev;
            }
        } else gSelectedPlayer[p] = 0;
        } while (!gSelectMenuValid[p][gDeleteMenuSelect[p]] || playerClaimed(p, gSelectedPlayer[p]) == 1);
    } else if (gPlayers[p].autoEdges & 2048) {
        if (gDeleteMenuNotify[p] == 1) return;
        if (gDeleteMenuConfirm[p] == 1) return;
        PlayUDSound(gPlayers[p].edges);
        do {
            if (gDeleteMenuSelect[p] == 10 && gSelectMenuValid[p][gDeleteMenuSelect[p]]) {
                if (gSelectedPlayer[p]) {
                    do { gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.next; }
                    while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak);
                }
                if (gSelectedPlayer[p]) break;
            }
            gDeleteMenuSelect[p]++;
            if (gDeleteMenuSelect[p] > 14) gDeleteMenuSelect[p] = 0;
        if (gDeleteMenuSelect[p] >= 3 && gDeleteMenuSelect[p] < 11 && gSelectMenuValid[p][gDeleteMenuSelect[p]]) {
                if (gSelectedPlayer[p]) {
                    do { gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.next; }
                    while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak);
                }
            if (!gSelectedPlayer[p]) {
                gSelectedPlayer[p] = gPlayerFiles.head;
                while (gSelectedPlayer[p] && gSelectPakIdx[p] != (*(*gSelectedPlayer[p])->fileh)->pak)
                    gSelectedPlayer[p] = (*gSelectedPlayer[p])->ll.next;
            }
        } else gSelectedPlayer[p] = 0;
        } while (!gSelectMenuValid[p][gDeleteMenuSelect[p]] || playerClaimed(p, gSelectedPlayer[p]) == 1);
    }
}

/* 0x8007c300, size 0x54 */
static void controlPakFull(int p)
{
    if (gPlayers[p].edges & 7) {
        PlayStartABSound(gPlayers[p].edges);
        setPlayerState(p, kPlayerStateSelect);
    }
}

/* 0x8007c354, size 0x54 */
static void controlPakDamaged(int p)
{
    if (gPlayers[p].edges & 7) {
        PlayStartABSound(gPlayers[p].edges);
        setPlayerState(p, kPlayerStateSelect);
    }
}

/* 0x8007c3a8, size 0xd64 */
static void drawSelect(int p)
{
    int x;
    int y;
    int cx;
    unsigned char *str;
    unsigned char str1[40];
    int menuItem;
    signed short screenPos[2];
    float gTextPosScale = gButtonScale;
    tPlayerProfile **player;
    int fullx;
    int fully;
    float tempVec[3];
    int i;
    signed short w;
    static unsigned char s_str[] = "NOT FORMATTED";
    if (gNumPlayers > 1) gTextPosScale *= 0.5f;
    ChangePaneSize(pane[p], -20, 0, -10, 10);
    if (gNumPlayers == 1) HAL_SetFont(13);
    else if (gNumPlayers == 2) HAL_SetFont(11);
    else HAL_SetFont(12);
    TB_Sprintf(str1, "%s %d", gText.single[29], p+1);
    TB_SetTextColor(playerTextColor[p]);
    TB_SetJust(0, 1);
    ZOID_ProjectWindow(p, buttonInfo[p][17].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
    cx = screenPos[0] / 2;
    y = screenPos[1] / 2.0f;
    TB_SetTextColor(1);
    TB_DrawText(TB_CenterText(str1, cx), y, str1);
    HAL_SetFont(12);
    if (gSelectMenuSelect[p] == 0) {
        tempVec[0] = buttonInfo[p][0].mat.mat3.pos[0];
        tempVec[1] = buttonInfo[p][0].mat.mat3.pos[1];
        tempVec[2] = buttonInfo[p][0].mat.mat3.pos[2];
        tempVec[0] = 0.0f;
        tempVec[1] = gNumPlayers == 1 ? 130 : 115;
        ZOID_ProjectWindow(p, tempVec, &gCamera[p].mat, 0, screenPos);
        cx = screenPos[0] / 2;
        y = screenPos[1] / 2.0f;
        TB_SetTextColor(1);
        str = gText.single[103];
        TB_SetTextColor(22);
        TB_DrawText(TB_CenterText(str, cx), y, str);
    }
    TB_GetFontLineHeight();
    if (gNumPlayers == 1) HAL_SetFont(10); else HAL_SetFont(12);
    for (menuItem = 0; menuItem < 15; menuItem++) {
        if (buttonInfo[p][menuItem].angle > 0.6981317f && buttonInfo[p][menuItem].angle < 2.443461f) continue;
        if (!buttonInfo[p][menuItem].alpha) continue;
        HAL_SetTextTranslucency(buttonInfo[p][menuItem].alpha);
        if (buttonInfo[p][menuItem].angle < 1.5707964f) {
            TB_FlashTextColors(flashColors[0], flashColors[1]);
            TB_SetTextColor(1);
        } else {
            TB_FlashTextColors(flashColors[0], flashColors[1]);
            TB_SetTextColor(22);
        }
        ZOID_ProjectWindow(p, buttonInfo[p][menuItem].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
        if (buttonInfo[p][menuItem].buttonType == 9) x = screenPos[0]/2;
        else if (buttonInfo[p][menuItem].buttonType == 7)
            x = screenPos[0]/2 - 300.0f * (65.0f * gTextPosScale) / buttonZ;
        else x = screenPos[0]/2 - 300.0f * (45.0f * gTextPosScale) / buttonZ;
        y = screenPos[1] / 2.0f;
        if (menuItem == 1) {
            ZOID_ProjectWindow(p, buttonInfo[p][16].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
            cx = screenPos[0]/2 - 300.0f * (25.0f * gTextPosScale) / buttonZ;
        }
        if (menuItem <= 3) {
            str = gText.taba[gText.tab[22] + menuItem];
            if (strcmp((char *)str, "VMU") == 0) str = (unsigned char *)memcard;
        } else if (menuItem < 11) str = gText.taba[gText.tab[22] + 3];
        else if (menuItem == 14) str = (unsigned char *)"FORMAT";
        else str = gText.taba[menuItem - 7 + gText.tab[22]];
        if (menuItem == 0) {
            if (menuItem == gSelectMenuSelect[p]) TB_SetTextColor(!gSelectMenuControls[p] ? 22 : 1);
            else TB_SetTextColor(1);
            TB_DrawText(x, y, str);
        } else if (menuItem == 1) {
            TB_SetTextColor(gSelectMenuSelect[p] == 1 ? 22 : 1);
            TB_DrawText(x, y, str);
            if (MtapConnected) str = (unsigned char *)gPS_PakName[gSelectPakIdx[p]];
            else if (gSelectPakIdx[p] < 2) str = (unsigned char *)gPS_PakNameNoMTP[gSelectPakIdx[p]];
            else {
                gSelectPakIdx[p] = 0;
                str = (unsigned char *)gPS_PakNameNoMTP[gSelectPakIdx[p]];
            }
            TB_DrawText(cx, y, str);
        } else if (menuItem == 2) {
            switch (gSelectPlayerNum[p]) {
            case -3: str = gText.single[179]; break;
            case 0: str = gText.single[158]; break;
            case -2: str = gText.single[176]; break;
            case -5: str = gText.single[171]; break;
            case -1: str = gText.single[178]; break;
            case -4: str = s_str; break;
            }
            if (gPakFull[gSelectPakIdx[p]]) str = gText.single[171];
            TB_SetTextColor(0);
            TB_DrawText(TB_CenterText(str, x), y, str);
            if (gPakFull[gSelectPakIdx[p]]) { fullx=x; fully=y; }
        } else if (menuItem >= 3 && menuItem < 11) {
            i = 0;
            for (player = gSelectedPlayer[p]; player; ) {
                if ((*player)->fileh && (*(*player)->fileh)->pak == gSelectPakIdx[p]) {
                    if (i == abs(menuItem - gSelectMenuSelect[p])) break;
                    i++;
                }
                if (menuItem < gSelectMenuSelect[p]) player = (*player)->ll.prev;
                else player = (*player)->ll.next;
            }
            if (!player) {
                player = (menuItem > gSelectMenuSelect[p] || gSelectPlayerNum[p] < 8) ? gPlayerFiles.head : gPlayerFiles.tail;
                for (; player;) {
                    if ((*player)->fileh && (*(*player)->fileh)->pak == gSelectPakIdx[p]) {
                        if (i == abs(menuItem - ((menuItem > gSelectMenuSelect[p] || gSelectPlayerNum[p] < 8) ? 3 : 10))) break;
                        i++;
                    }
                    player = (menuItem > gSelectMenuSelect[p] || gSelectPlayerNum[p] < 8) ? (*player)->ll.next : (*player)->ll.prev;
                }
            }
            if (player) {
                w = gNumPlayers == 1 ? 98 : (gNumPlayers == 2 ? 70 : 55);
                str = (*player)->name;
                if (HAL_GetTextWidth(str,-1) > w) HAL_SetFontSpacing(0);
                if (menuItem == gSelectMenuSelect[p] && gSelectedPlayer[p] == player)
                    TB_SetTextColor(!gSelectMenuControls[p] ? 22 : 1);
                else {
                    TB_SetTextColor(1);
                    if (playerClaimed(p,player)) TB_SetTextColor(0);
                }
                TB_DrawEllipseText(x,y,w,str);
                HAL_SetFontSpacing(-1);
            }
        } else {
            TB_SetTextColor(menuItem == gSelectMenuSelect[p] ? 22 : 1);
            if ((menuItem == 11 && playerBeingCreated(p)) || (menuItem == 12 && playerBeingDeleted(p))) TB_SetTextColor(0);
            TB_DrawText(x,y,str);
        }
        TB_GetFontLineHeight();
    }
    if (gSelectMenuSelect[p] == 0 || (gSelectMenuSelect[p] >= 3 && gSelectMenuSelect[p] < 11)) {
        ZOID_ProjectWindow(p, buttonInfo[p][15].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
        TB_SetTextColor(gSelectMenuControls[p] == 1 ? 22 : 1);
        if ((buttonInfo[p][15].angle <= 0.6981317f || buttonInfo[p][15].angle >= 2.443461f) && buttonInfo[p][15].alpha) {
            HAL_SetTextTranslucency(buttonInfo[p][15].alpha);
            if (buttonInfo[p][15].angle < 1.5707964f) {
                TB_FlashTextColors(flashColors[0],flashColors[1]);
                TB_SetTextColor(1);
            } else {
                TB_FlashTextColors(flashColors[0],flashColors[1]);
                TB_SetTextColor(22);
            }
        }
    }
    if (gPakFull[gSelectPakIdx[p]] && !gSelectMenuVisible[p][3]) {
        if (gNumPlayers == 1) {
            TB_SetTextColor(1);
            str = (unsigned char *)"AT LEAST 20KB IS NEEDED TO";
            TB_DrawText(TB_CenterText(str,fullx),fully+20,str);
            str = (unsigned char *)"CREATE A PLAYER PROFILE.";
            TB_DrawText(TB_CenterText(str,fullx),fully+40,str);
        } else {
            TB_SetTextColor(1);
            str = (unsigned char *)"AT LEAST 20KB IS NEEDED TO";
            TB_DrawText(TB_CenterText(str,fullx),fully+12,str);
            str = (unsigned char *)"CREATE A PLAYER PROFILE.";
            TB_DrawText(TB_CenterText(str,fullx),fully+22,str);
        }
    }
    TB_SetJust(0,3);
    HAL_SetTextTranslucency(-1.0f);
    HAL_SetTextDepth(0.0f);
}

/* 0x8007d10c, size 0x2e8 */
static void drawWait(int p)
{
    int x;
    int y;
    unsigned char *str;
    signed short screenPos[2];
    signed short paneT;
    signed short paneW;
    int pIdx;
    int notReadyIdx;
    int notReadyCount;
    if (gNumPlayers > 1) HAL_SetFont(11); else HAL_SetFont(13);
    if (gPlayers[p].save) str = (*gPlayers[p].save)->name;
    else str = gJustPlayers[p]->name;
    TB_SetTextColor(playerTextColor[p]);
    TB_SetJust(1, 1);
    ZOID_ProjectWindow(p, buttonInfo[p][17].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
    x = screenPos[0] / 2;
    y = screenPos[1] / 2.0f;
    TB_SetTextColor(1);
    TB_DrawEllipseText(x, y, 69, str);
    y += TB_GetFontLineHeight() * 2;
    paneT = y - 4;
    TB_SetJust(0, 3);
    str = gText.single[161];
    TB_SetTextColor(1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    paneW = HAL_GetTextWidth(str, -1);
    y += TB_GetFontLineHeight();
    notReadyCount = 0;
    for (pIdx = 0; pIdx < gNumPlayers; pIdx++) {
        if (pIdx != p && gPlayerState[pIdx] != kPlayerStateWait) {
            notReadyIdx = pIdx;
            notReadyCount++;
        }
    }
    if (notReadyCount == 1) {
        TB_SetTextColor(playerTextColor[notReadyIdx]);
        if (gPlayers[notReadyIdx].save) str = (*gPlayers[notReadyIdx].save)->name;
        else str = gJustPlayers[notReadyIdx]->name;
    } else str = gText.single[31];
    TB_DrawText(TB_CenterText(str, x), y, str);
    if (paneW < HAL_GetTextWidth(str, -1)) paneW = HAL_GetTextWidth(str, -1);
    x = gPlayerRects[gNumPlayers - 1][p].cx;
    ChangePaneSize(pane[p], x - 4 - paneW/2, paneT, 4 + x + paneW/2, 8 + paneT + TB_GetFontLineHeight()*2);
}

/* 0x8007d3f4, size 0x40 */
static void drawControls(int p)
{
    ChangePaneSize(pane[p], -20, 0, -10, 10);
}

/* 0x8007d434, size 0x4bc */
static void drawCreatePlayer(int p)
{
    unsigned char *str;
    unsigned char ltr[2];
    int x;
    int y;
    int i;
    signed short screenPos[2];
    signed short paneT;
    char tt[64];
    static unsigned char del[4] = "DEL";
    static unsigned char end[4] = "END";
    static unsigned char spc[4] = "SPC";
    if (gNumPlayers > 1) HAL_SetFont(11); else HAL_SetFont(13);
    str = gCreatePlayerNames[p];
    TB_SetJust(1, 1);
    ZOID_ProjectWindow(p, buttonInfo[p][17].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
    x = screenPos[0] / 2;
    y = screenPos[1] / 2.0f;
    TB_SetTextColor(22);
    TB_DrawEllipseText(x, y, gNumPlayers == 1 ? 150 : 69, str);
    TB_SetJust(0, 1);
    HAL_SetFont(11);
    TB_SetJust(0, 3);
    y = gPlayerRects[gNumPlayers - 1][p].y + 32;
    x = gPlayerRects[gNumPlayers - 1][p].cx;
    ltr[1] = 0;
    if (gNumPlayers > 1) y -= 26;
    if (gCreateMenuNotify[p] == 1) {
        TB_SetTextColor(1);
        if (gNumPlayers > 1) y += 10;
        paneT = y - 4;
        TB_DrawText(TB_CenterText("SAVE FAILED!", x), y, "SAVE FAILED!");
        HAL_SetFont(12);
        TB_DrawText(TB_CenterText("CHECK MEMORY CARD (NGC)", x), y + 12, "CHECK MEMORY CARD (NGC)");
        if (MtapConnected)
            sprintf(tt, "IN MEMORY CARD %s", gPS_PakName[gSelectPakIdx[p]]);
        else
            sprintf(tt, "IN MEMORY CARD %s", gPS_PakNameNoMTP[gSelectPakIdx[p]]);
        TB_DrawText(TB_CenterText(tt, x), y + 24, tt);
        TB_DrawText(TB_CenterText("AND PLEASE TRY AGAIN.", x), y + 36, "AND PLEASE TRY AGAIN.");
        ChangePaneSize(pane[p], gPlayerRects[gNumPlayers - 1][p].cx - 66, paneT,
                       gPlayerRects[gNumPlayers - 1][p].cx + 66, y + 52);
    } else {
        if (gNumPlayers == 1) {
            paneT = y - 4;
            TB_SetTextColor(1);
            str = gText.single[160];
            TB_DrawText(TB_CenterText(str, x), y, str);
        }
        HAL_SetFont(12);
        x = gPlayerRects[gNumPlayers - 1][p].cx - 48;
        y = gPlayerRects[gNumPlayers - 1][p].y + 46;
        if (gNumPlayers > 1) {
            paneT = y - 30;
            y -= 26;
        }
        for (i = 0; i < 54; i++) {
            ltr[0] = gPlayerNameChars[i];
            TB_SetTextColor(i == gNameSelect[p] ? 22 : 1);
            TB_DrawText(TB_CenterText(ltr, x), y, ltr);
            if ((i + 1) % 9 == 0) {
                y += 10;
                x = gPlayerRects[gNumPlayers - 1][p].cx - 48;
            } else x += 12;
        }
        x = gPlayerRects[gNumPlayers - 1][p].cx - 36;
        TB_SetTextColor(gNameSelect[p] == -3 ? 22 : 16);
        TB_DrawText(TB_CenterText(spc, x), y, spc);
        TB_SetTextColor(gNameSelect[p] == -2 ? 22 : 16);
        TB_DrawText(TB_CenterText(del, x + 36), y, del);
        TB_SetTextColor(gNameSelect[p] == -1 ? 22 : 16);
        TB_DrawText(TB_CenterText(end, x + 72), y, end);
        ChangePaneSize(pane[p], gPlayerRects[gNumPlayers - 1][p].cx - 56, paneT,
                       gPlayerRects[gNumPlayers - 1][p].cx + 56, y + 14);
    }
}

/* 0x8007d8f0, size 0xcf0 */
static void drawDeletePlayer(int p)
{
    int x;
    int y;
    int cx;
    unsigned char *str;
    unsigned char str1[128];
    signed short paneT;
    int menuItem;
    signed short screenPos[2];
    float gTextPosScale = gButtonScale;
    tPlayerProfile **player;
    int i;
    signed short w;
    static unsigned char s_str[] = "NOT FORMATTED";
    if (gNumPlayers > 1) gTextPosScale *= 0.5f;
    if (gNumPlayers > 1) HAL_SetFont(12); else HAL_SetFont(13);
    y = 0;
    str = gText.single[150];
    TB_SetTextColor(playerTextColor[p]);
    TB_SetJust(y,1);
    ZOID_ProjectWindow(p, buttonInfo[p][17].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
    cx = screenPos[0]/2;
    y = screenPos[1]/2.0f;
    TB_SetTextColor(1);
    TB_DrawText(TB_CenterText(str,cx),y,str);
    y += TB_GetFontLineHeight();
    if (gNumPlayers > 1) HAL_SetFont(12); else HAL_SetFont(10);
    if (gDeleteMenuConfirm[p] == 1) {
        TB_SetJust(0,3);
        y += TB_GetFontLineHeight();
        paneT = y-4;
        TB_Sprintf(str1,"%s%s%s",gText.single[151],gDeleteMenuName[p],gText.single[152]);
        TB_SetTextColor(1);
        TB_SetJust(1,3);
        TB_DrawTextScrollBox(cx,y,gPlayerRects[gNumPlayers-1][p].w,
             gPlayerRects[gNumPlayers-1][p].y+gPlayerRects[gNumPlayers-1][p].h-y,-1,0,str1);
        TB_SetJust(0,3);
        y += TB_GetFontLineHeight() * (TB_GetTextNumLines(gPlayerRects[gNumPlayers-1][p].w,str1)+1);
        str = gText.taba[gText.tab[25]];
        TB_SetTextColor(gDeleteMenuYes[p] == 1 ? 22 : 1);
        TB_DrawText(TB_RightText(str,cx-5),y,str);
        str = gText.taba[gText.tab[25]+1];
        TB_SetTextColor(gDeleteMenuYes[p] == 0 ? 22 : 1);
        TB_DrawText(cx+5,y,str);
        y += TB_GetFontLineHeight();
        ChangePaneSize(pane[p],cx-gPlayerRects[gNumPlayers-1][p].w/2-4,paneT,
                       cx+gPlayerRects[gNumPlayers-1][p].w/2+4,y+4);
    } else if (gDeleteMenuNotify[p] == 1) {
        TB_SetJust(0,3);
        y += TB_GetFontLineHeight();
        paneT = y-4;
        TB_Sprintf(str1,gText.single[155],gDeleteMenuName[p]);
        TB_SetTextColor(1);
        TB_SetJust(1,3);
        TB_DrawTextScrollBox(cx,y,gPlayerRects[gNumPlayers-1][p].w,
             gPlayerRects[gNumPlayers-1][p].y+gPlayerRects[gNumPlayers-1][p].h-y,-1,0,str1);
        TB_SetJust(0,3);
        y += TB_GetFontLineHeight() * TB_GetTextNumLines(gPlayerRects[gNumPlayers-1][p].w,str1);
        ChangePaneSize(pane[p],cx-gPlayerRects[gNumPlayers-1][p].w/2-4,paneT,
                       cx+gPlayerRects[gNumPlayers-1][p].w/2+4,y+4);
    } else {
        ChangePaneSize(pane[p],-20,0,-10,10);
        for (menuItem=0; menuItem<15;menuItem++) {
            if (buttonInfo[p][menuItem].angle > 0.6981317f && buttonInfo[p][menuItem].angle < 2.443461f) continue;
            if (!buttonInfo[p][menuItem].alpha) continue;
            HAL_SetTextTranslucency(buttonInfo[p][menuItem].alpha);
            if (buttonInfo[p][menuItem].angle < 1.5707964f) {
                TB_FlashTextColors(flashColors[0],flashColors[1]);
                TB_SetTextColor(1);
            } else {
                TB_FlashTextColors(flashColors[0],flashColors[1]);
                TB_SetTextColor(22);
            }
            ZOID_ProjectWindow(p, buttonInfo[p][menuItem].mat.mat3.pos, &gCamera[p].mat,0,screenPos);
            if (buttonInfo[p][menuItem].buttonType == 9) x = screenPos[0]/2 - 300.0f * (105.0f * gTextPosScale) / buttonZ;
            else if (buttonInfo[p][menuItem].buttonType == 7) x = screenPos[0]/2 - 300.0f * (65.0f * gTextPosScale) / buttonZ;
            else x = screenPos[0]/2 - 300.0f * (45.0f * gTextPosScale) / buttonZ;
            y = screenPos[1]/2.0f;
            if (menuItem == 0 || (menuItem >= 3 && menuItem < 11)) {
                ZOID_ProjectWindow(p,buttonInfo[p][15].mat.mat3.pos,&gCamera[p].mat,0,screenPos);
                cx = screenPos[0]/2 - 300.0f * (45.0f * gTextPosScale) / buttonZ;
            } else if (menuItem == 1) {
                ZOID_ProjectWindow(p,buttonInfo[p][16].mat.mat3.pos,&gCamera[p].mat,0,screenPos);
                cx = screenPos[0]/2 - 300.0f * (25.0f * gTextPosScale) / buttonZ;
            }
            if (menuItem == 1) {
                TB_SetTextColor(gDeleteMenuSelect[p] == 1 ? 22 : 1);
                TB_DrawText(x,y,gText.taba[gText.tab[22]+menuItem]);
                if (MtapConnected) str = (unsigned char *)gPS_PakName[gSelectPakIdx[p]];
                else if (p < 2) str = (unsigned char *)gPS_PakNameNoMTP[gSelectPakIdx[p]];
                else {
                    gSelectPakIdx[p] = 0;
                    str = (unsigned char *)gPS_PakNameNoMTP[gSelectPakIdx[p]];
                }
                TB_DrawText(cx,y,str);
            } else if (menuItem == 2) {
                switch (gSelectPlayerNum[p]) {
                case -3: str = gText.single[179]; break;
                case 0: str = gText.single[158]; break;
                case -2: str = gText.single[176]; break;
                case -5: str = gText.single[171]; break;
                case -1: str = gText.single[178]; break;
                case -4: str = s_str; break;
                }
                x += 300.0f * (65.0f * gTextPosScale) / buttonZ;
                TB_SetTextColor(0);
                TB_DrawText(TB_CenterText(str,x),y,str);
            } else if (menuItem >= 3 && menuItem < 11) {
            i = 0;
            for (player = gSelectedPlayer[p]; player; ) {
                if ((*player)->fileh && (*(*player)->fileh)->pak == gSelectPakIdx[p]) {
                    if (i == abs(menuItem - gDeleteMenuSelect[p])) break;
                    i++;
                }
                if (menuItem < gDeleteMenuSelect[p]) player = (*player)->ll.prev;
                else player = (*player)->ll.next;
            }
            if (!player) {
                player = (menuItem > gDeleteMenuSelect[p] || gSelectPlayerNum[p] < 8) ? gPlayerFiles.head : gPlayerFiles.tail;
                for (; player;) {
                    if ((*player)->fileh && (*(*player)->fileh)->pak == gSelectPakIdx[p]) {
                        if (i == abs(menuItem - ((menuItem > gDeleteMenuSelect[p] || gSelectPlayerNum[p] < 8) ? 3 : 10))) break;
                        i++;
                    }
                    player = (menuItem > gDeleteMenuSelect[p] || gSelectPlayerNum[p] < 8) ? (*player)->ll.next : (*player)->ll.prev;
                }
            }
            if (player) {
                str = (*player)->name;
                w = gNumPlayers == 1 ? 98 : (gNumPlayers == 2 ? 70 : 55);
                if (HAL_GetTextWidth(str,-1) > w) HAL_SetFontSpacing(0);
                if (gDeleteMenuSelect[p] == menuItem && gSelectedPlayer[p] == player)
                    TB_SetTextColor(22);
                else {
                    TB_SetTextColor(1);
                    if (playerClaimed(p,player)) TB_SetTextColor(0);
                    if (!gSelectMenuValid[p][menuItem]) TB_SetTextColor(4);
                }
                TB_DrawEllipseText(x,y,w,str);
                HAL_SetFontSpacing(-1);
            }

            }
            TB_GetFontLineHeight();
        }
    }
    TB_SetJust(0,3);
    HAL_SetTextTranslucency(-1.0f);
    HAL_SetTextDepth(0.0f);
}

/* 0x8007e5e0, size 0x4d4 */
static void drawFormatMC(int p)
{
    int y;
    int cx;
    unsigned char *str;
    unsigned char str1[256];
    signed short paneT;
    signed short screenPos[2];
    if (gNumPlayers > 1) HAL_SetFont(12); else HAL_SetFont(13);
    str = (unsigned char *)"FORMAT";
    TB_SetTextColor(playerTextColor[p]);
    TB_SetJust(0, 1);
    ZOID_ProjectWindow(p, buttonInfo[p][17].mat.mat3.pos, &gCamera[p].mat, 0, screenPos);
    cx = screenPos[0]/2;
    y = screenPos[1]/2.0f;
    TB_SetTextColor(1);
    TB_DrawText(TB_CenterText(str,cx),y,str);
    if (gFormatMenuConfirm[p] == 1) {
        TB_SetJust(0,3);
        y += TB_GetFontLineHeight();
        paneT = y-4;
        str = (unsigned char *)"ARE YOU SURE YOU WANT TO FORMAT THE MEMORY CARD (NGC) IN MEMORY CARD ";
        if (MtapConnected)
            TB_Sprintf(str1,"%s%s ?",str,gPS_PakName[gSelectPakIdx[p]]);
        else {
            if (gSelectPakIdx[p] >= 2) gSelectPakIdx[p] = 0;
            TB_Sprintf(str1,"%s%s ?",str,gPS_PakNameNoMTP[gSelectPakIdx[p]]);
        }
        TB_SetTextColor(1);
        TB_SetJust(1,3);
        TB_DrawTextScrollBox(cx,y,gPlayerRects[gNumPlayers-1][p].w,
             gPlayerRects[gNumPlayers-1][p].y+gPlayerRects[gNumPlayers-1][p].h-y,-1,0,str1);
        TB_SetJust(0,3);
        y += TB_GetFontLineHeight() * (TB_GetTextNumLines(gPlayerRects[gNumPlayers-1][p].w,str1)+1);
        str = gText.taba[gText.tab[25]];
        TB_SetTextColor(gFormatMenuYes[p] == 1 ? 22 : 1);
        TB_DrawText(TB_RightText(str,cx-5),y,str);
        str = gText.taba[gText.tab[25]+1];
        TB_SetTextColor(gFormatMenuYes[p] == 0 ? 22 : 1);
        TB_DrawText(cx+5,y,str);
        y += TB_GetFontLineHeight();
        ChangePaneSize(pane[p],cx-gPlayerRects[gNumPlayers-1][p].w/2-4,paneT,
                       cx+gPlayerRects[gNumPlayers-1][p].w/2+4,y+4);
    }
    if (gFormatMenuNotify[p] == 1) {
        TB_SetJust(0,3);
        y += TB_GetFontLineHeight();
        paneT = y-4;
        TB_Sprintf(str1,"%s",gNumPakPlayers[gSelectPakIdx[p]] < 0 ? "FORMAT FAILED." : "FORMAT SUCCESSFUL.");
        TB_SetTextColor(1);
        TB_SetJust(1,3);
        TB_DrawTextScrollBox(cx,y,gPlayerRects[gNumPlayers-1][p].w,
             gPlayerRects[gNumPlayers-1][p].y+gPlayerRects[gNumPlayers-1][p].h-y,-1,0,str1);
        TB_SetJust(0,3);
        y += TB_GetFontLineHeight() * TB_GetTextNumLines(gPlayerRects[gNumPlayers-1][p].w,str1);
        ChangePaneSize(pane[p],cx-gPlayerRects[gNumPlayers-1][p].w/2-4,paneT,
                       cx+gPlayerRects[gNumPlayers-1][p].w/2+4,y+4);
    }
    TB_SetJust(0,3);
    HAL_SetTextTranslucency(-1.0f);
    HAL_SetTextDepth(0.0f);
}

/* 0x8007eab4, size 0x144 */
static void drawPakFull(int p)
{
    int y = gPlayerRects[gNumPlayers - 1][p].y;
    int cx = gPlayerRects[gNumPlayers - 1][p].cx;
    unsigned char *str;
    unsigned char str1[40];
    if (gNumPlayers > 2) HAL_SetFont(11);
    else if (gNumPlayers == 2) HAL_SetFont(10);
    else HAL_SetFont(13);
    TB_Sprintf(str1, "%s %d", gText.single[29], p + 1);
    TB_SetTextColor(playerTextColor[p]);
    TB_DrawText(TB_CenterText(str1, cx), y, str1);
    y += TB_GetFontLineHeight() * 2;
    if (gNumPlayers > 2) HAL_SetFont(12);
    else if (gNumPlayers == 2) HAL_SetFont(11);
    else HAL_SetFont(10);
    str = (unsigned char *)"PAK FULL";
    TB_SetTextColor(17);
    TB_DrawText(TB_CenterText(str, cx), y, str);
}

/* 0x8007ebf8, size 0x144 */
static void drawPakDamaged(int p)
{
    int y = gPlayerRects[gNumPlayers - 1][p].y;
    int cx = gPlayerRects[gNumPlayers - 1][p].cx;
    unsigned char *str;
    unsigned char str1[40];
    if (gNumPlayers > 2) HAL_SetFont(11);
    else if (gNumPlayers == 2) HAL_SetFont(10);
    else HAL_SetFont(13);
    TB_Sprintf(str1, "%s %d", gText.single[29], p + 1);
    TB_SetTextColor(playerTextColor[p]);
    TB_DrawText(TB_CenterText(str1, cx), y, str1);
    y += TB_GetFontLineHeight() * 2;
    if (gNumPlayers > 2) HAL_SetFont(12);
    else if (gNumPlayers == 2) HAL_SetFont(11);
    else HAL_SetFont(10);
    str = (unsigned char *)"PAK DAMAGED";
    TB_SetTextColor(17);
    TB_DrawText(TB_CenterText(str, cx), y, str);
}

/* 0x8007ed3c, size 0xec */
static int animText(void)
{
    int playerIdx;
    HAL_SetTextDepth(0.0f);
    for (playerIdx = 0; playerIdx < gNumPlayers; playerIdx++) {
        updatePlayer(playerIdx);
        switch (gPlayerState[playerIdx]) {
        case kPlayerStateSelect: drawSelect(playerIdx); break;
        case kPlayerStateWait: drawWait(playerIdx); break;
        case kPlayerStateControls: drawControls(playerIdx); break;
        case kPlayerStateCreatePlayer: drawCreatePlayer(playerIdx); break;
        case kPlayerStateDeletePlayer: drawDeletePlayer(playerIdx); break;
        case kPlayerStatePakFull: drawPakFull(playerIdx); break;
        case kPlayerStatePakDamaged: drawPakDamaged(playerIdx); break;
        case kPlayerStateFormatMC: drawFormatMC(playerIdx); break;
        }
    }
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x8007ee28, size 0x2d0 */
static void setPlayerState(int p, enum_kPlayerStateCount newState)
{
    tPlayerProfile **player;
    tPlayerProfile **pPlayer;
    if (newState == kPlayerStateSelect) {
        gSelectedPlayer[p] = 0;
        for (player = gPlayerFiles.head; player; player = (*player)->ll.next)
            if (player == gPlayers[p].save) break;
        gPlayers[p].save = 0;
        if (!player || !(*player)->fileh) {
            gSelectedPlayer[p] = &gJustPlayers[p];
            if (gPlayerState[p] != kPlayerStateCreatePlayer && gPlayerState[p] != kPlayerStateDeletePlayer)
                gSelectPakIdx[p] = ((int)gPlayers[p].controller << 2) / 4;
            if (gSelectMenuSelect[p] >= kSelectMenuSelectPlayer && gSelectMenuSelect[p] < kSelectMenuCreatePlayer)
                gSelectMenuSelect[p] = kSelectMenuJustPlay;
        } else {
            gSelectedPlayer[p] = player;
            gSelectPakIdx[p] = (*(*player)->fileh)->pak;
            if (gSelectMenuSelect[p] >= kSelectMenuSelectPlayer && gSelectMenuSelect[p] < kSelectMenuCreatePlayer) {
                gSelectMenuSelect[p] = kSelectMenuSelectPlayer;
                for (pPlayer = gPlayerFiles.head; pPlayer; pPlayer = (*pPlayer)->ll.next) {
                    if (pPlayer == player) break;
                    if ((*(*pPlayer)->fileh)->pak == (*(*player)->fileh)->pak && gSelectMenuSelect[p] < 10)
                        gSelectMenuSelect[p]++;
                }
            }
        }
        gSelectMenuControls[p] = 0;
    } else if (newState == kPlayerStateCreatePlayer) {
        memset(gCreatePlayerNames[p], 0, 13);
        gNameSelect[p] = 0;
        gNameLen[p] = 0;
        gPlayers[p].save = 0;
        gCreateMenuNotify[p] = 0;
    } else if (newState == kPlayerStateDeletePlayer) {
        gDeleteMenuSelect[p] = 1;
        gDeleteMenuConfirm[p] = 0;
        gDeleteMenuYes[p] = 0;
        gDeleteMenuNotify[p] = 0;
        gPlayers[p].save = 0;
    } else if (newState == kPlayerStateFormatMC) {
        gFormatMenuConfirm[p] = 1;
        gFormatMenuYes[p] = 0;
        gFormatMenuNotify[p] = 0;
        gFormatMenuEnd[p] = 0;
    }
    gPlayerState[p] = newState;
    buttonsJustInited[p] = 1;
    updateSelectInfo(p);
}

/* 0x8007f0f8, size 0x31c */
static void updateSelectInfo(int p)
{
    int i;
    int firstPlayer = p;
    int lastPlayer = p;
    if (p == -1) {
        firstPlayer = 0;
        lastPlayer = gNumPlayers - 1;
    }
    for (p = firstPlayer; p <= lastPlayer; p++) {
        gSelectPlayerNum[p] = gNumPakPlayers[gSelectPakIdx[p]];
        gSelectMenuValid[p][0] = 1;
        gSelectMenuValid[p][1] = 0;
        gSelectMenuValid[p][2] = 0;
        gSelectMenuValid[p][3] = gSelectPlayerNum[p] > 0;
        gSelectMenuValid[p][4] = gSelectPlayerNum[p] > 1;
        gSelectMenuValid[p][5] = gSelectPlayerNum[p] > 2;
        gSelectMenuValid[p][6] = gSelectPlayerNum[p] > 3;
        gSelectMenuValid[p][7] = gSelectPlayerNum[p] > 4;
        gSelectMenuValid[p][8] = gSelectPlayerNum[p] > 5;
        gSelectMenuValid[p][9] = gSelectPlayerNum[p] > 6;
        gSelectMenuValid[p][10] = gSelectPlayerNum[p] > 7;

        gSelectMenuValid[p][14] = gSelectPlayerNum[p] == -4;
        gSelectMenuValid[p][11] = gSelectPlayerNum[p] > -1 && gSelectPlayerNum[p] < 8 && !gPakFull[gSelectPakIdx[p]];
        gSelectMenuValid[p][12] = gSelectPlayerNum[p] > 0;
        gSelectMenuValid[p][13] = 0;
        if (gPlayerState[p] == kPlayerStateDeletePlayer) {
            gSelectMenuValid[p][0] = 0;
            gSelectMenuValid[p][1] = 0;
            gSelectMenuValid[p][11] = 0;
            gSelectMenuValid[p][12] = 0;
            gSelectMenuValid[p][13] = 0;
        }
        gSelectMenuVisible[p][0] = gSelectMenuValid[p][0];
        gSelectMenuVisible[p][1] = 0;
        gSelectMenuVisible[p][2] = 0;
        gSelectMenuVisible[p][3] = gSelectMenuValid[p][3];
        gSelectMenuVisible[p][4] = gSelectMenuValid[p][4];
        gSelectMenuVisible[p][5] = gSelectMenuValid[p][5];
        gSelectMenuVisible[p][6] = gSelectMenuValid[p][6];
        gSelectMenuVisible[p][7] = gSelectMenuValid[p][7];
        gSelectMenuVisible[p][8] = gSelectMenuValid[p][8];
        gSelectMenuVisible[p][9] = gSelectMenuValid[p][9];
        gSelectMenuVisible[p][10] = gSelectMenuValid[p][10];
        gSelectMenuVisible[p][11] = gSelectMenuValid[p][11];
        gSelectMenuVisible[p][12] = gSelectMenuValid[p][12];
        gSelectMenuVisible[p][13] = gSelectMenuValid[p][13];
        gSelectMenuVisible[p][14] = gSelectMenuValid[p][14];

        gSelectMenuCount[p] = 0;
        for (i = 0; i < kSelectMenuCount; i++)
            if (gSelectMenuVisible[p][i]) gSelectMenuCount[p]++;
    }
}

/* 0x8007f414, size 0xdc */
static void updatePakInfo(void)
{
    int pak;
    tPlayerProfile **player;
    int p;
    for (pak = 0; pak < 4; pak++) {
        gPakFull[pak] = 0;
        if (gNumPakPlayers[pak] < 0) {
            for (p = 0; p < gNumPlayers; p++)
                if (gSelectPakIdx[p] == pak)
                    buttonsJustInited[p] = 1;
        }
        gNumPakPlayers[pak] = 0;
        for (player = gPlayerFiles.head; player; player = (*player)->ll.next) {
            if ((*(*player)->fileh)->pak == pak)
                gNumPakPlayers[pak]++;
        }
        if (gNumPakPlayers[pak] == 0)
            gNumPakPlayers[pak] = 0;
    }
}

/* 0x8007f4f0, size 0x94 */
static void calcSelectMenuMaxSize(void)
{
    int rectHeight = gPlayerRects[gNumPlayers - 1][0].h;
    if (gNumPlayers > 1) HAL_SetFont(11); else HAL_SetFont(13);
    rectHeight -= TB_GetFontLineHeight();
    if (gNumPlayers > 1) HAL_SetFont(12); else HAL_SetFont(10);
    gSelectMenuMaxSize = rectHeight / TB_GetFontLineHeight();
}

/* 0x8007f584, size 0x44 */
static void claimPlayer(int playerIdx, tPlayerProfile **player)
{
    gPlayers[playerIdx].save = player;
    SetPlayerControls(playerIdx, (*player)->controls);
    updateSelectInfo(-1);
}

/* 0x8007f5c8, size 0x54 */
static int playerClaimed(int playerIdx, tPlayerProfile **player)
{
    int claimed = 0;
    signed char p;
    if (player) {
        for (p = 0; p < gNumPlayers; p++)
            if (p != playerIdx && gPlayers[p].save == player)
                claimed = 1;
    }
    return claimed;
}

/* 0x8007f61c, size 0x4c */
static int playerBeingCreated(int playerIdx)
{
    int creating = 0;
    signed char p;
    for (p = 0; p < gNumPlayers; p++)
        if (p != playerIdx && gPlayerState[p] == kPlayerStateCreatePlayer)
            creating = 1;
    return creating;
}

/* 0x8007f668, size 0x4c */
static int playerBeingDeleted(int playerIdx)
{
    int deleting = 0;
    signed char p;
    for (p = 0; p < gNumPlayers; p++)
        if (p != playerIdx && gPlayerState[p] == kPlayerStateDeletePlayer)
            deleting = 1;
    return deleting;
}

/* 0x8007f6b4, size 0x4cc */
static void initButtons(signed char p)
{
    tRGBA buttonColor = { { 255, 255, 255, 255 } };
    tTexture *tex;
    unsigned short texIdx;
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 23; buttonIdx++) {
        if (buttonInfo[p][buttonIdx].objIdx == -1) {
            memcpy(buttonInfo[p][buttonIdx].mat.mat3.uvs, unitmatrix, 36);
            buttonInfo[p][buttonIdx].objIdx = MBOX_NewObject(
                MBOX_FindObject_WORK(buttonNames[buttonInfo[p][buttonIdx].buttonType], 0, gNumModels - 1, 1),
                &buttonInfo[p][buttonIdx].mat, -1,
                (buttonInfo[p][buttonIdx].buttonType == 0 || buttonInfo[p][buttonIdx].buttonType == 1 ||
                 buttonInfo[p][buttonIdx].buttonType == 2 || buttonInfo[p][buttonIdx].buttonType == 7 ||
                 buttonInfo[p][buttonIdx].buttonType == 8 || buttonInfo[p][buttonIdx].buttonType == 9) ? 0x42000 : 0);
            ZOID_SetColor(buttonInfo[p][buttonIdx].objIdx, &buttonColor);
            MBOX_HideObject(buttonInfo[p][buttonIdx].objIdx, 1, 15);
            MBOX_ShowObject(buttonInfo[p][buttonIdx].objIdx, 0, 1 << p);
        }
    }
    tex = MBOX_FindTexture_WORK("BUTTON", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[p][17].objIdx, tex, -1);
    memcpy(buttonInfo[p][17].mat.mat3.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[p][17].mat.mat3.uvs);
    buttonInfo[p][17].mat.mat3.pos[0] = 0.0f;
    buttonInfo[p][17].mat.mat3.pos[1] = gNumPlayers > 1 ? 105.0f : 115.0f;
    buttonInfo[p][17].mat.mat3.pos[2] = 200.0f;
    /* no-op copies kept on purpose: the target reloads and re-stores both coordinates */
    buttonInfo[p][17].mat.mat3.pos[0] = buttonInfo[p][17].mat.mat3.pos[0];
    buttonInfo[p][17].mat.mat3.pos[1] = buttonInfo[p][17].mat.mat3.pos[1];
    tex = MBOX_FindTexture_WORK("CURSOR", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[p][18].objIdx, tex, -1);
    tex = MBOX_FindTexture_WORK("A1", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[p][19].objIdx, tex, -1);
    memcpy(buttonInfo[p][19].mat.mat3.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[p][19].mat.mat3.uvs);
    buttonInfo[p][19].mat.mat3.pos[0] = 75.1f;
    buttonInfo[p][19].mat.mat3.pos[1] = 4.0f + (gNumPlayers > 1 ? 50.2f : 55.2f);
    buttonInfo[p][19].mat.mat3.pos[2] = 100.0f;
    ScaleUV(buttonInfo[p][19].mat.mat3.uvs, buttonInfo[p][19].mat.mat3.uvs, 0.5f);
    memcpy(buttonInfo[p][20].mat.mat3.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[p][20].mat.mat3.uvs);
    WRollUV(3.1415927f, buttonInfo[p][20].mat.mat3.uvs);
    buttonInfo[p][20].mat.mat3.pos[0] = 87.1f;
    buttonInfo[p][20].mat.mat3.pos[1] = 4.0f + (gNumPlayers > 1 ? 50.2f : 55.2f);
    buttonInfo[p][20].mat.mat3.pos[2] = 100.0f;
    tex = MBOX_FindTexture_WORK("B1", &texIdx, 0, gNumModels - 1, 1);
    ZOID_SetTexture(buttonInfo[p][21].objIdx, tex, -1);
    memcpy(buttonInfo[p][21].mat.mat3.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[p][21].mat.mat3.uvs);
    buttonInfo[p][21].mat.mat3.pos[0] = -75.1f;
    buttonInfo[p][21].mat.mat3.pos[1] = 4.0f + (gNumPlayers > 1 ? 50.2f : 55.2f);
    buttonInfo[p][21].mat.mat3.pos[2] = 100.0f;
    /* no-op copies kept on purpose: the target reloads and re-stores both coordinates */
    buttonInfo[p][21].mat.mat3.pos[0] = buttonInfo[p][21].mat.mat3.pos[0];
    buttonInfo[p][21].mat.mat3.pos[1] = buttonInfo[p][21].mat.mat3.pos[1];
    ScaleUV(buttonInfo[p][21].mat.mat3.uvs, buttonInfo[p][21].mat.mat3.uvs, 0.5f);
    memcpy(buttonInfo[p][22].mat.mat3.uvs, unitmatrix, 36);
    WPitchUV(-1.5707964f, buttonInfo[p][22].mat.mat3.uvs);
    buttonInfo[p][22].mat.mat3.pos[0] = -87.1f;
    buttonInfo[p][22].mat.mat3.pos[1] = 4.0f + (gNumPlayers > 1 ? 50.2f : 55.2f);
    buttonInfo[p][22].mat.mat3.pos[2] = 100.0f;
    /* no-op copies kept on purpose: the target reloads and re-stores both coordinates */
    buttonInfo[p][22].mat.mat3.pos[0] = buttonInfo[p][22].mat.mat3.pos[0];
    buttonInfo[p][22].mat.mat3.pos[1] = buttonInfo[p][22].mat.mat3.pos[1];
    buttonsInited[p] = 1;
    buttonsJustInited[p] = 1;
    ZeroCameras();
}

/* 0x8007fb80, size 0x94 */
static void uninitButtons(signed char p)
{
    int buttonIdx;
    for (buttonIdx = 0; buttonIdx < 23; buttonIdx++) {
        if (buttonInfo[p][buttonIdx].objIdx != -1) {
            ZOID_RemoveObject(buttonInfo[p][buttonIdx].objIdx);
            buttonInfo[p][buttonIdx].objIdx = -1;
        }
    }
    buttonsInited[p] = 0;
}

/* 0x8007fc14, size 0xa90 */
static void updateButtons(signed char p)
{
    int buttonIdx;
    int i;
    float prevPos[3];
    int whichItem;
    float buttonY;
    float selectedPos;
    signed char clearScrollSpeed = 1;
    tTexture *tex;
    unsigned short texIdx;
    tRGBA buttonColor = { { 255,255,255,255 } };
    if (!buttonsInited[p]) return;
    if (gPlayerState[p] == kPlayerStateWait) {
        for (buttonIdx=0;buttonIdx<23;buttonIdx++) {
            if (buttonIdx == 17 || buttonIdx == 21 || buttonIdx == 22)
                MBOX_ShowObject(buttonInfo[p][buttonIdx].objIdx,0,1<<p);
            else MBOX_HideObject(buttonInfo[p][buttonIdx].objIdx,1,15);
        }
        return;
    }
    if (gPlayerState[p] == kPlayerStateCreatePlayer || gPlayerState[p] == kPlayerStateFormatMC ||
        (gPlayerState[p] == kPlayerStateDeletePlayer && (gDeleteMenuNotify[p] == 1 || gDeleteMenuConfirm[p] == 1))) {
        for (buttonIdx=0;buttonIdx<23;buttonIdx++) {
            if (buttonIdx == 17 || (buttonIdx == 21 || buttonIdx == 22) || buttonIdx == 19 || buttonIdx == 20)
                MBOX_ShowObject(buttonInfo[p][buttonIdx].objIdx,0,1<<p);
            else MBOX_HideObject(buttonInfo[p][buttonIdx].objIdx,1,15);
        }
        return;
    }
    if (gPlayerState[p] == kPlayerStateControls || (gPlayerState[p] == kPlayerStatePakFull || gPlayerState[p] == kPlayerStatePakDamaged) ||
        gPlayerState[p] == kPlayerStatePaks) {
        for (buttonIdx=0;buttonIdx<23;buttonIdx++) MBOX_HideObject(buttonInfo[p][buttonIdx].objIdx,1,15);
        return;
    }
    for (i=0;i<23;i++) MBOX_ShowObject(buttonInfo[p][i].objIdx,0,1<<p);
    whichItem = 0;
    if (gPlayerState[p] == kPlayerStateSelect) {
        for (i=0;i<gSelectMenuSelect[p];i++) if (gSelectMenuVisible[p][i]) whichItem++;
    } else if (gPlayerState[p] == kPlayerStateDeletePlayer) {
        for (i=0;i<gDeleteMenuSelect[p];i++) if (gSelectMenuVisible[p][i]) whichItem++;
    }
    if (gButtonScale * ((gSelectMenuCount[p]-1)*gButtonSpace) > maxY-minY)
        selectedPos = maxY - whichItem*(maxY-minY)/(gSelectMenuCount[p]-1);
    else selectedPos = maxY - gButtonScale*(whichItem*gButtonSpace);
    memcpy(prevPos,buttonInfo[p][18].mat.mat3.pos,12);
    buttonInfo[p][18].angle += 6.2831855f*gFrameSeconds;
    if (buttonInfo[p][18].angle > 6.2831855f) buttonInfo[p][18].angle -= 6.2831855f;
    memcpy(buttonInfo[p][18].mat.mat3.uvs,unitmatrix,36);
    buttonInfo[p][18].mat.mat3.pos[0] = buttonX*cursorZ/buttonZ + -165.0f*gButtonScale;
    buttonInfo[p][18].mat.mat3.pos[1] = selectedPos*cursorZ/buttonZ;
    buttonInfo[p][18].mat.mat3.pos[2] = cursorZ;
    if (!buttonsJustInited[p]) {
        if (60.0*fabs(buttonInfo[p][18].mat.mat3.pos[1]-prevPos[1])*gButtonScale/10.0 > cursorSpeed[p]) {
            cursorSpeed[p] = 60.0*fabs(buttonInfo[p][18].mat.mat3.pos[1]-prevPos[1])*gButtonScale/10.0;
            cursorSpeed[p] = PLAYER_MIN(cursorSpeed[p],540.0f*gButtonScale);
            cursorSpeed[p] = PLAYER_MAX(cursorSpeed[p],180.0f*gButtonScale);
        }
        if (buttonInfo[p][18].mat.mat3.pos[1] > prevPos[1]+gFrameSeconds*cursorSpeed[p])
            buttonInfo[p][18].mat.mat3.pos[1] = prevPos[1]+gFrameSeconds*cursorSpeed[p];
        else if (buttonInfo[p][18].mat.mat3.pos[1] < prevPos[1]-gFrameSeconds*cursorSpeed[p])
            buttonInfo[p][18].mat.mat3.pos[1] = prevPos[1]-gFrameSeconds*cursorSpeed[p];
        else cursorSpeed[p] = 180.0f*gButtonScale;
    }
    WRollUV(buttonInfo[p][18].angle,buttonInfo[p][18].mat.mat3.uvs);
    WPitchUV(1.5707964f,buttonInfo[p][18].mat.mat3.uvs);
    ScaleUV(buttonInfo[p][18].mat.mat3.uvs,buttonInfo[p][18].mat.mat3.uvs,0.4f*gButtonScale);
    buttonY = selectedPos + gButtonScale*(whichItem*gButtonSpace);
    for (buttonIdx=0;buttonIdx<15;buttonIdx++) {
        if (((gPlayerState[p] == kPlayerStateSelect && buttonIdx == gSelectMenuSelect[p]) ||
             (gPlayerState[p] == kPlayerStateDeletePlayer && buttonIdx == gDeleteMenuSelect[p])) && !gSelectMenuControls[p]) {
            if (buttonInfo[p][buttonIdx].angle < 3.1415927f) buttonInfo[p][buttonIdx].angle += 12.566371f*gFrameSeconds;
            if (buttonInfo[p][buttonIdx].angle > 3.1415927f) buttonInfo[p][buttonIdx].angle = 3.1415927f;
            if (buttonsJustInited[p]) buttonInfo[p][buttonIdx].angle = 3.1415927f;
        } else {
            if (buttonInfo[p][buttonIdx].angle > 0.0f) buttonInfo[p][buttonIdx].angle -= 12.566371f*gFrameSeconds;
            if (buttonInfo[p][buttonIdx].angle < 0.0f) buttonInfo[p][buttonIdx].angle = 0.0f;
            if (buttonsJustInited[p]) buttonInfo[p][buttonIdx].angle = 0.0f;
        }
        memcpy(prevPos,buttonInfo[p][buttonIdx].mat.mat3.pos,12);
        if (buttonInfo[p][buttonIdx].buttonType == 9) buttonInfo[p][buttonIdx].mat.mat3.pos[0] = buttonX;
        else if (buttonInfo[p][buttonIdx].buttonType == 7) buttonInfo[p][buttonIdx].mat.mat3.pos[0] = buttonX + -80.0f*gButtonScale;
        else buttonInfo[p][buttonIdx].mat.mat3.pos[0] = buttonX + -120.0f*gButtonScale;
        buttonInfo[p][buttonIdx].mat.mat3.pos[1] = buttonY;
        buttonInfo[p][buttonIdx].mat.mat3.pos[2] = buttonZ;
        if (!buttonsJustInited[p]) {
            if (60.0*fabs(buttonInfo[p][buttonIdx].mat.mat3.pos[1]-prevPos[1])/10.0 > scrollSpeed[p]) {
                scrollSpeed[p] = 60.0*fabs(buttonInfo[p][buttonIdx].mat.mat3.pos[1]-prevPos[1])/10.0;
                scrollSpeed[p] = PLAYER_MIN(scrollSpeed[p],540.0f*gButtonScale);
                scrollSpeed[p] = PLAYER_MAX(scrollSpeed[p],180.0f*gButtonScale);
            }
            if (buttonInfo[p][buttonIdx].mat.mat3.pos[1] > prevPos[1]+gFrameSeconds*scrollSpeed[p]) {
                buttonInfo[p][buttonIdx].mat.mat3.pos[1] = prevPos[1]+gFrameSeconds*scrollSpeed[p];
                clearScrollSpeed = 0;
            } else if (buttonInfo[p][buttonIdx].mat.mat3.pos[1] < prevPos[1]-gFrameSeconds*scrollSpeed[p]) {
                buttonInfo[p][buttonIdx].mat.mat3.pos[1] = prevPos[1]-gFrameSeconds*scrollSpeed[p];
                clearScrollSpeed = 0;
            }
        }
        ScaleUV(unitmatrix,buttonInfo[p][buttonIdx].mat.mat3.uvs,gButtonScale);
        WPitchUV(buttonInfo[p][buttonIdx].angle-1.5707964f,buttonInfo[p][buttonIdx].mat.mat3.uvs);
        if (buttonInfo[p][buttonIdx].angle > 1.5707964f)
            tex = MBOX_FindTexture_WORK("BUTTON_SELECT",&texIdx,0,gNumModels-1,1);
        else tex = MBOX_FindTexture_WORK("BUTTON",&texIdx,0,gNumModels-1,1);
        ZOID_SetTexture(buttonInfo[p][buttonIdx].objIdx,tex,-1);
        if (buttonInfo[p][buttonIdx].mat.mat3.pos[1] > maxY+gButtonSpace*gButtonScale ||
            buttonInfo[p][buttonIdx].mat.mat3.pos[1] < minY-gButtonSpace*gButtonScale) buttonInfo[p][buttonIdx].alpha = 0;
        else if (buttonInfo[p][buttonIdx].mat.mat3.pos[1] > maxY)
            buttonInfo[p][buttonIdx].alpha = 255.0f - 255.0f*(buttonInfo[p][buttonIdx].mat.mat3.pos[1]-maxY)/(gButtonSpace*gButtonScale);
        else if (buttonInfo[p][buttonIdx].mat.mat3.pos[1] < minY)
            buttonInfo[p][buttonIdx].alpha = 255.0f - 255.0f*(minY-buttonInfo[p][buttonIdx].mat.mat3.pos[1])/(gButtonSpace*gButtonScale);
        else buttonInfo[p][buttonIdx].alpha = 255;
        if (!gSelectMenuVisible[p][buttonIdx]) buttonInfo[p][buttonIdx].alpha = 0;
        buttonColor.c[3] = buttonInfo[p][buttonIdx].alpha;
        ZOID_SetColor(buttonInfo[p][buttonIdx].objIdx,&buttonColor);
        if (!gSelectMenuVisible[p][buttonIdx] || !buttonInfo[p][buttonIdx].alpha) MBOX_HideObject(buttonInfo[p][buttonIdx].objIdx,1,15);
        else MBOX_ShowObject(buttonInfo[p][buttonIdx].objIdx,0,1<<p);
        if (buttonIdx == 1) {
            buttonInfo[p][16].mat.mat3.pos[0] = buttonX + 160.0f*gButtonScale;
            buttonInfo[p][16].mat.mat3.pos[1] = buttonInfo[p][buttonIdx].mat.mat3.pos[1];
            buttonInfo[p][16].mat.mat3.pos[2] = buttonInfo[p][buttonIdx].mat.mat3.pos[2];
            buttonInfo[p][16].angle = buttonInfo[p][buttonIdx].angle;
            ScaleUV(unitmatrix,buttonInfo[p][16].mat.mat3.uvs,gButtonScale);
            WPitchUV(buttonInfo[p][16].angle-1.5707964f,buttonInfo[p][16].mat.mat3.uvs);
            ZOID_SetTexture(buttonInfo[p][16].objIdx,tex,-1);
            buttonInfo[p][16].alpha = buttonInfo[p][buttonIdx].alpha;
            ZOID_SetColor(buttonInfo[p][16].objIdx,&buttonColor);
            if (!gSelectMenuVisible[p][buttonIdx] || !buttonInfo[p][buttonIdx].alpha) MBOX_HideObject(buttonInfo[p][16].objIdx,1,15);
            else MBOX_ShowObject(buttonInfo[p][16].objIdx,0,1<<p);
        } else {
            buttonInfo[p][15].angle = 0.0f;
            MBOX_HideObject(buttonInfo[p][15].objIdx,1,15);
        }
        if (gSelectMenuVisible[p][buttonIdx] == 1) buttonY -= gButtonSpace*gButtonScale;
    }
    if (clearScrollSpeed == 1) scrollSpeed[p] = 0.0f;
    buttonsJustInited[p] = 0;
}

