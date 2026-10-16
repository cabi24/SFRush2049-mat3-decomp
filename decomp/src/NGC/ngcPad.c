/* ngcPad.c - GameCube pads read into the PS2-style g_Pad state the game uses, plus rumble.
   Original: D:\rush_ngc\Rush_NGC\NGC\ngcPad.c; assembly: build/2049LE/asm/NGC/ngcPad.s */
#include "rush/rush_protos.h"

#define PAD_BUTTON_LEFT 0x0001
#define PAD_BUTTON_RIGHT 0x0002
#define PAD_BUTTON_DOWN 0x0004
#define PAD_BUTTON_UP 0x0008
#define PAD_TRIGGER_Z 0x0010
#define PAD_TRIGGER_R 0x0020
#define PAD_TRIGGER_L 0x0040
#define PAD_BUTTON_A 0x0100
#define PAD_BUTTON_B 0x0200
#define PAD_BUTTON_X 0x0400
#define PAD_BUTTON_Y 0x0800
#define PAD_BUTTON_START 0x1000
#define PAD_ERR_NO_CONTROLLER -1
#define PAD_CHAN0_BIT 0x80000000
#define PAD_CHAN1_BIT 0x40000000
#define PAD_CHAN2_BIT 0x20000000
#define PAD_CHAN3_BIT 0x10000000
#define PAD_MOTOR_STOP 0
#define PAD_MOTOR_RUMBLE 1

/* buttons that were down last frame and are up now */
#define RELEASED(p) (prestatus[p].button & (prestatus[p].button ^ status[p].button))

int MtapConnected = 1;
static int LastUpdateTime;
int g_bUseRumble;
static char debug_info[1024];
static PADStatus prestatus[4];
static PADStatus status[4];
static int padattached[4];
static unsigned long uRumblePad[4];
_PadInfo PadStates[4];

/* Stripped by the linker. Kept because it is the first function to use PadStates, uRumblePad and
   padattached, which decides their order in .bss; its body is a placeholder that uses them in the
   order the DWARF lists (in reverse) for it. */
static void PS2PadInit(void)
{
    int i;

    for (i = 0; i < 4; i++) {
        PadStates[i].PortNum = i;
        uRumblePad[i] = 0;
        padattached[i] = 0;
    }
    memset(g_Pad, 0, sizeof(g_Pad));
    MtapConnected = 1;
}

/* 0x80099370, size 0x678 */
void ReadInPS2GamePad(void)
{
    int portid; /* r27 */

    PS2UpdatePadStatus();
    for (portid = 0; portid < 4; portid++) {
        g_Pad[portid].type = 0;
        if (!padattached[portid]) {
            continue;
        }
        memset(&g_Pad[portid], 0, sizeof(g_Pad[portid]));
        g_Pad[portid].type = 1;
        g_Pad[portid].on = 0;
        g_Pad[portid].support = 0xffffff;
        g_Pad[portid].x1 = 0;
        g_Pad[portid].y1 = 0;
        g_Pad[portid].x2 = 0;
        g_Pad[portid].y2 = 0;
        g_Pad[portid].l = 0;
        g_Pad[portid].r = 0;
        if (status[portid].stickX) {
            g_Pad[portid].x1 = status[portid].stickX;
        }
        if (status[portid].stickY) {
            g_Pad[portid].y1 = status[portid].stickY;
        }
        if (status[portid].substickX) {
            g_Pad[portid].x2 = status[portid].substickX;
        }
        if (status[portid].substickY) {
            g_Pad[portid].y2 = status[portid].substickY;
        }
        if (gGameState & 0x8000000) {
            if (status[portid].button & PAD_BUTTON_A) {
                g_Pad[portid].on |= 0x2;
            }
            if (status[portid].button & PAD_BUTTON_B) {
                g_Pad[portid].on |= 0x4;
            }
            if (status[portid].button & PAD_BUTTON_START) {
                g_Pad[portid].on |= 0x40000;
            }
            if (status[portid].button & PAD_TRIGGER_R) {
                g_Pad[portid].on |= 0x200;
            }
            if (status[portid].button & PAD_TRIGGER_L) {
                g_Pad[portid].on |= 0x100;
            }
            if (status[portid].button & PAD_BUTTON_LEFT) {
                g_Pad[portid].on |= 0x1000;
            }
            if (status[portid].button & PAD_BUTTON_RIGHT) {
                g_Pad[portid].on |= 0x2000;
            }
            if (status[portid].button & PAD_BUTTON_UP) {
                g_Pad[portid].on |= 0x400;
            }
            if (status[portid].button & PAD_BUTTON_DOWN) {
                g_Pad[portid].on |= 0x800;
            }
            if (status[portid].button & PAD_TRIGGER_R) {
                g_Pad[portid].on |= 0x10;
            }
            if (status[portid].button & PAD_TRIGGER_L) {
                g_Pad[portid].on |= 0x8;
            }
            if (status[portid].button & PAD_BUTTON_X) {
                g_Pad[portid].on |= 0x20;
            }
            if (status[portid].button & PAD_BUTTON_Y) {
                g_Pad[portid].on |= 0x40;
            }
        } else if (!(gGameState & 0x7c0000) || gPauseState || gSelPauseState) {
            if (RELEASED(portid) & PAD_BUTTON_A) {
                g_Pad[portid].on |= 0x2;
            }
            if (RELEASED(portid) & PAD_BUTTON_B) {
                g_Pad[portid].on |= 0x4;
            }
            if (RELEASED(portid) & PAD_BUTTON_START) {
                if ((gGameState & 0x1) || (gGameState & 0x2) || (gGameState & 0x40000000) || (gGameState & 0x4) ||
                    (gGameState & 0x8)) {
                    g_Pad[portid].on |= 0x1;
                } else {
                    g_Pad[portid].on |= 0x40000;
                }
            }
            if (status[portid].button & PAD_TRIGGER_R) {
                g_Pad[portid].on |= 0x200;
            }
            if (status[portid].button & PAD_TRIGGER_L) {
                g_Pad[portid].on |= 0x100;
            }
            if (status[portid].button & PAD_BUTTON_LEFT) {
                g_Pad[portid].on |= 0x1000;
            }
            if (status[portid].button & PAD_BUTTON_RIGHT) {
                g_Pad[portid].on |= 0x2000;
            }
            if (status[portid].button & PAD_BUTTON_UP) {
                g_Pad[portid].on |= 0x400;
            }
            if (status[portid].button & PAD_BUTTON_DOWN) {
                g_Pad[portid].on |= 0x800;
            }
            if (status[portid].button & PAD_TRIGGER_R) {
                g_Pad[portid].on |= 0x10;
            }
            if (status[portid].button & PAD_TRIGGER_L) {
                g_Pad[portid].on |= 0x8;
            }
            if (status[portid].button & PAD_BUTTON_X) {
                g_Pad[portid].on |= 0x20;
            }
            if (status[portid].button & PAD_BUTTON_Y) {
                g_Pad[portid].on |= 0x40;
            }
            if ((status[portid].button & PAD_TRIGGER_Z) && gPauseState && gSelPauseState != 2) {
                g_Pad[portid].on |= 0x1;
            }
        } else {
            if (status[portid].button & PAD_BUTTON_A) {
                g_Pad[portid].on |= 0x2;
            }
            if (status[portid].button & PAD_BUTTON_B) {
                g_Pad[portid].on |= 0x4;
            }
            if (RELEASED(portid) & PAD_BUTTON_START) {
                g_Pad[portid].on |= 0x40000;
            }
            if (status[portid].button & PAD_TRIGGER_R) {
                g_Pad[portid].on |= 0x200;
            }
            if (status[portid].button & PAD_TRIGGER_L) {
                g_Pad[portid].on |= 0x100;
            }
            if (status[portid].button & PAD_BUTTON_LEFT) {
                g_Pad[portid].on |= 0x1000;
                g_iSilvCoin++;
            }
            if (status[portid].button & PAD_BUTTON_RIGHT) {
                g_Pad[portid].on |= 0x2000;
                g_iSilvCoin--;
            }
            if (status[portid].button & PAD_BUTTON_UP) {
                g_Pad[portid].on |= 0x400;
                g_iGoldCoin++;
            }
            if (status[portid].button & PAD_BUTTON_DOWN) {
                g_Pad[portid].on |= 0x800;
                g_iGoldCoin--;
            }
            if (status[portid].button & PAD_BUTTON_X) {
                g_Pad[portid].on |= 0x20;
            }
            if (status[portid].button & PAD_BUTTON_Y) {
                g_Pad[portid].on |= 0x40;
            }
            if (status[portid].button & PAD_TRIGGER_Z) {
                g_Pad[portid].on |= 0x1;
            }
        }
        if (!(gGameState & 0x7c0000) || gPauseState || gSelPauseState) {
            if (g_Pad[portid].x1 > 80) {
                g_Pad[portid].on |= 0x2000;
            }
            if (g_Pad[portid].x1 < -80) {
                g_Pad[portid].on |= 0x1000;
            }
            if (g_Pad[portid].y1 > 80) {
                g_Pad[portid].on |= 0x400;
            }
            if (g_Pad[portid].y1 < -80) {
                g_Pad[portid].on |= 0x800;
            }
        } else if (g_Pad[portid].y1 <= 100 && g_Pad[portid].y1 >= -100) {
            g_Pad[portid].y1 *= -1;
        } else if (g_Pad[portid].y1 > 100) {
            g_Pad[portid].y1 = -100;
        } else if (g_Pad[portid].y1 < -100) {
            g_Pad[portid].y1 = 100;
        }
        prestatus[portid].button = status[portid].button;
    }
}

/* 0x800999e8, size 0x184 */
void PS2UpdatePadStatus(void)
{
    int i; /* r31 */

    appGetTime();
    LastUpdateTime = appGetTime();
    memset(status, 0, sizeof(status));
    PADRead(status);
    if (status[0].err == PAD_ERR_NO_CONTROLLER) {
        PADReset(PAD_CHAN0_BIT);
    }
    if (status[1].err == PAD_ERR_NO_CONTROLLER) {
        PADReset(PAD_CHAN1_BIT);
    }
    if (status[2].err == PAD_ERR_NO_CONTROLLER) {
        PADReset(PAD_CHAN2_BIT);
    }
    if (status[3].err == PAD_ERR_NO_CONTROLLER) {
        PADReset(PAD_CHAN3_BIT);
    }
    for (i = 0; i < 4; i++) {
        if ((gGameState & 0x1) || (gGameState & 0x2) || (gGameState & 0x40000000) || (gGameState & 0x4) ||
            (gGameState & 0x8)) {
            padattached[i] = 1;
        } else {
            padattached[i] = status[i].err != PAD_ERR_NO_CONTROLLER;
        }
        if (uRumblePad[i]) {
            uRumblePad[i]--;
        } else {
            PADControlMotor(i, PAD_MOTOR_STOP);
        }
    }
}

/* 0x80099b6c, size 0x44 */
void NGCResetPadStatus(void)
{
    memset(status, 0, sizeof(status));
    memset(prestatus, 0, sizeof(prestatus));
}

/* 0x80099bb0, size 0x4c */
void NGCRumblePad(unsigned int Controller, unsigned int power)
{
    if (g_bUseRumble && power) {
        uRumblePad[Controller] = 2;
        PADControlMotor(Controller, PAD_MOTOR_RUMBLE);
    }
}

