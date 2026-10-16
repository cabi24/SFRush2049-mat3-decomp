/* menuFunc.c - pause menu and DVD error message text.
   Original: D:\rush_ngc\Rush_NGC\NGC\FONT\menuFunc.c; assembly: build/2049LE/asm/NGC/FONT/menuFunc.s */
#include "rush/rush_protos.h"

/* 0x800af944, size 0xfc */
void ShowPauseMianFont(int bInRace)
{
    char strPauseMenu[128]; /* stack */
    sprintf(strPauseMenu, "PAUSE MENU - PLAYER %d", g_PlayerInPause + 1);
    PrintFont(120, 52, strPauseMenu, 1);
    PrintFont(240, 138, "RESUME GAME", 1);
    PrintFont(255, 177, "CONTROLS", 1);
    PrintFont(250, 216, "SCREEN SIZE", 1);
    if (bInRace) {
        PrintFont(258, 255, "EXIT RACE", 1);
    } else {
        PrintFont(258, 255, "EXIT RACE", 0);
    }
    PrintFont(253, 294, "EXIT GAME", 1);
    PrintFont(344, 388, "\x80 SELECT \x81 BACK", 1);
}

/* 0x800afa40, size 0x60 */
void ShowPauContrlFontAndPic(void)
{
    char strControlMenu[128]; /* stack */
    sprintf(strControlMenu, "CONTROLS FOR PLAYER %d", g_PlayerInPause + 1);
    PrintFont(120, 52, strControlMenu, 1);
    PrintFont(450, 388, "\x81 BACK", 1);
}

/* 0x800afaa0, size 0x90 */
void ShowPauseExitRaceFont(void)
{
    PrintFont(120, 52, "EXIT RACE", 1);
    PrintFont(240, 140, "Are you Sure?", 1);
    PrintFont(287, 216, "NO", 1);
    PrintFont(287, 255, "YES", 1);
    PrintFont(344, 388, "\x80 SELECT \x81 BACK", 1);
}

/* 0x800afb30, size 0x90 */
void ShowPauseExitGameFont(void)
{
    PrintFont(120, 52, "EXIT GAME", 1);
    PrintFont(102, 140, "If you quit now, your progress in this\ngame will be lost. Quit?", 1);
    PrintFont(287, 216, "NO", 1);
    PrintFont(287, 255, "YES", 1);
    PrintFont(344, 388, "\x80 SELECT \x81 BACK", 1);
}

/* 0x800afbc0, size 0x68 */
void ShowPauseZoomFont(void)
{
    PrintFont(120, 52, "SCREEN SIZE", 1);
    PrintFont(102, 140, "Press +Control Pad left to decrease\nthe screen size. Press +Control Pad\nright to increase the screen size.", 1);
    PrintFont(334, 388, "\x80 ACCEPT \x81 CANCEL", 1);
}

/* 0x800afc28, size 0x64 */
void ShowPauseController(void)
{
    char strControlDis[256]; /* stack */
    sprintf(strControlDis, "Please reconnect the Controller to\nController Socket %d and press START\nto continue.", g_PlayerInControlDis + 1);
    PrintFont(120, 52, "CONTROLLER DISCONNECTED", 1);
    PrintFont(102, 140, strControlDis, 1);
}

/* 0x800afc8c, size 0x34 */
void ShowDVDOpen(void)
{
    PrintFont(135, 170, "The Disc Cover is open.\nIf you want to continue the game,\nplease close the Disc Cover.", 1);
}

/* 0x800afcc0, size 0x34 */
void ShowDVDNoDisk(void)
{
    PrintFont(170, 170, "Please insert the\nMidway Arcade Treasures 3\nGame Disc.", 1);
}

/* 0x800afcf4, size 0x34 */
void ShowDVDWrongDisk(void)
{
    PrintFont(105, 170, "This is not the\nMidway Arcade Treasures 3\nGame Disc.\nPlease insert the\nMidway Arcade Treasures 3\nGame Disc.", 1);
}

/* 0x800afd28, size 0x34 */
void ShowDVDBrokenDisk(void)
{
    PrintFont(105, 170, "The Game Disc could not be read.\nPlease read the Nintendo GameCube\nInstruction Booklet for more information.", 1);
}

