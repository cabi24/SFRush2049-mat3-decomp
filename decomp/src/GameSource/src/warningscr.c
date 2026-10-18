/* warningscr.c - memory-card warning screen. */
#include "rush/rush_protos.h"

static signed char gPakScr = 0; /* explicit `= 0`: emitted at its declaration point, before the function-local statics */

/* 0x8008d30c */
void updateWarningScreen(void)
{
    static _tBlit *sBlit;

    if (sBlit) {
        RemoveBlit(sBlit);
        sBlit = NULL;
    }
    HideControllerWarning();
    gBlocksNeeded = PS_GetBlocks(0x814);
    setGameState(gPakScr ? 0x4000000 : 4);
    SetControllerActive(-1, 1);
    SetControllerStickEdges(-1, 1);
}
