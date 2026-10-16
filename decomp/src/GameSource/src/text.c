/* text.c - game text: loads the per-language text wave and fixes up its tables.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\text.c; assembly: build/2049LE/asm/GameSource/src/text.s */
#include "rush/rush_protos.h"

signed short gLang;
static unsigned char gTextWave;
static int gTextWaveInst;
anon_numSingle_single_28 gText;

static void _AdjustTextWavePointers(void);

/* 0x800502dc, size 0x5c */
void InitTextWave(void)
{
    static signed char sInited;

    if (!sInited) {
        sInited = 1;
        gTextWave = gLargestTextWave;
        gTextWaveInst = LoadWave(gLargestTextWave, 0, 0, 0, 0);
        LockWaveInstance(gTextWaveInst);
        _AdjustTextWavePointers();
        LoadTextWave();
    }
}

/* 0x80050338, size 0x3c */
void LoadTextWave(void)
{
    if (gLang != gTextWave) {
        ReplaceWave(gTextWave, gLang);
        gTextWave = gLang;
        _AdjustTextWavePointers();
    }
}

/* 0x80050374, size 0x478 */
static void _AdjustTextWavePointers(void)
{
    int i; /* r16 */
    unsigned char *data; /* r27 */
    unsigned int start; /* r26 */
    unsigned int u; /* stack */
    data = GetWaveInstanceAddr(gTextWaveInst);
    start = (unsigned int)data;
    u = *(unsigned int *)data;
    _Check_swap(&u);
    _swap_x32(&u);
    gText.numSingle = u;
    data += 4;
    gText.single = (unsigned char **)data;
    for (i = 0; i < gText.numSingle; i++) {
        _swap_x32(&gText.single[i]);
    }
    data += gText.numSingle * 4;
    u = *(unsigned int *)data;
    _swap_x32(&u);
    gText.numTab = u;
    data += 4;
    gText.tab = (unsigned short *)data;
    for (i = 0; i < ((gText.numTab + 1) & ~1); i++) {
        _swap_x16(&gText.tab[i]);
    }
    data += ((gText.numTab + 1) & ~1) * 2;
    u = *(unsigned int *)data;
    _swap_x32(&u);
    gText.numTabA = u;
    data += 4;
    gText.taba = (unsigned char **)data;
    for (i = 0; i < gText.numTabA; i++) {
        _swap_x32(&gText.taba[i]);
    }
    data += gText.numTabA * 4;
    u = *(unsigned int *)data;
    _swap_x32(&u);
    gText.numTab2 = u;
    data += 4;
    gText.tab2 = (unsigned short *)data;
    for (i = 0; i < ((gText.numTab2 + 1) & ~1); i++) {
        _swap_x16(&gText.tab2[i]);
    }
    data += ((gText.numTab2 + 1) & ~1) * 2;
    u = *(unsigned int *)data;
    _swap_x32(&u);
    gText.numTab2a = u;
    data += 4;
    gText.tab2a = (unsigned short *)data;
    for (i = 0; i < ((gText.numTab2a + 1) & ~1); i++) {
        _swap_x16(&gText.tab2a[i]);
    }
    data += ((gText.numTab2a + 1) & ~1) * 2;
    u = *(unsigned int *)data;
    _swap_x32(&u);
    gText.numTab2b = u;
    data += 4;
    gText.tab2b = (unsigned char **)data;
    for (i = 0; i < gText.numTab2b; i++) {
        _swap_x32(&gText.tab2b[i]);
    }
    for (i = 0; i < gText.numSingle; i++) {
        gText.single[i] += start;
    }
    for (i = 0; i < gText.numTabA; i++) {
        gText.taba[i] += start;
    }
    for (i = 0; i < gText.numTab2b; i++) {
        gText.tab2b[i] += start;
    }
    /* NGC-specific replacements for strings from the original text */
    gText.single[38] = (unsigned char *)"PRESS THE    BUTTON TO CHANGE CONTROLS";
    gText.single[93] = (unsigned char *)" KB";
    gText.single[94] = (unsigned char *)" KB";
    gText.single[95] = (unsigned char *)"KB";
    gText.single[123] = (unsigned char *)"THERE IS NOT ENOUGH SPACE ON THIS MEMORY CARD (NGC) FOR A RUSH 2049 FILE.";
    gText.single[121] = (unsigned char *)"UNKNOWN ERROR. MAKE SURE THE CONTROLLER AND THE MEMORY CARD (NGC) ARE CONNECTED PROPERLY.";
    gText.single[119] = (unsigned char *)"THIS MEMORY CARD (NGC) IS CORRUPTED.";
    gText.single[118] = (unsigned char *)"PLEASE INSERT THE MEMORY CARD (NGC) CONTAINING THE FILE.";
    gText.single[117] = (unsigned char *)"THIS IS A DIFFERENT MEMORY CARD (NGC).";
    gText.single[115] = (unsigned char *)"AN ACTIVE MEMORY CARD (NGC) HAS BEEN REMOVED. PLEASE REINSERT THE MEMORY CARD (NGC) AND ";
    gText.single[105] = (unsigned char *)"THIS VMU DOES NOT CONTAIN ANY RUSH 2049 FILES.";
    gText.single[114] = (unsigned char *)"NO VMU CAN BE FOUND IN THIS CONTROLLER. IF THERE IS A VMU IN THIS CONTROLLER MAKE SURE IT IS FIRMLY SEATED AND ";
    gText.single[124] = (unsigned char *)"INSERT OR SELECT A VMU WITH ENOUGH SPACE.";
    gText.single[135] = (unsigned char *)"MEMORY CARD (NGC) IN SLOT ";
    gText.single[136] = (unsigned char *)" WAS REMOVED.  YOU MUST REPLACE THE MEMORY CARD (NGC) TO RECORD YOUR PROGRESS WHEN THE GAME IS OVER.";
    gText.single[138] = (unsigned char *)" ON MEMORY CARD (NGC)";
    gText.single[162] = (unsigned char *)"NO MEMORY CARD (NGC) DETECTED.";
    gText.single[163] = (unsigned char *)"NO RUSH 2049 PLAYER FILE DETECTED AND THERE IS NOT ENOUGH SPACE ON THE AVAILABLE MEMORY CARDS TO CREATE A PLAYER FILE.";
    gText.single[166] = (unsigned char *)"MANAGE MEMORY CARDS";
    gText.single[167] = (unsigned char *)"MEMORY CARD (NGC)";
    gText.single[171] = (unsigned char *)"MEMORY CARD (NGC) FULL";
    gText.single[177] = (unsigned char *)"THIS MEMORY CARD (NGC) IS DAMAGED.";
    gText.single[178] = (unsigned char *)"NO PLAYER EXIST";
    gText.single[195] = (unsigned char *)"ON MEMORY CARD (NGC)";
    gText.single[196] = (unsigned char *)"ON MEMORY CARD (NGC)";
    gText.single[234] = (unsigned char *)"GHOST SAVED ON MEMORY CARD %s";
    gText.taba[183] = (unsigned char *)"MEMORY CARD";
    gText.taba[322] = (unsigned char *)"KEEP";
    gText.taba[183] = (unsigned char *)"FILE SAVE";
    gText.taba[323] = (unsigned char *)"DO NOT KEEP";
}

