/* pakscr.c - Memory-pak selection, deletion and status screen. NonMatching.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\pakscr.c; assembly: build/2049LE/asm/GameSource/src/pakscr.s */
#include "rush/rush_protos.h"

static int _CancelFunc(int pak);
static tPS_File **_FindFile(int idx);
static void _SetState(enum_kNumStates state);
static void _BuildFileDelList(void);
static void _ControlPickPak(void);
static void _ControlFileDel(void);
static void _ControlFileDelVerify(void);
static void _ControlRumblePak(void);
static void _ControlDamaged(void);
static void _GetPakStatusStr(int pak, unsigned char *str);
static int _AnimText(void);
static void _DrawPickPak(void);
static void _DrawFileDel(void);
static void _DrawFileDelVerify(void);
static void _DrawRumblePak(void);
static void _DrawDamaged(void);

int gBlocksNeeded;
static enum_kNumStates gState;
static int gMaxMenuHeight;
static signed char gYes;
static signed short gPickedPak;
static signed short gFreePages;
static tPS_File **gDeleteFile;
static signed short gDeleteSize;
static signed short gDeleteSelect;
static signed short gDeleteSelTop;
static signed short gDeleteHeight;
static signed short gDeleteSelPos;
static enum_kPS_ErrUnknown_3 gPakErr;
static signed char gDrawText;
static _tBlit_2 *gBlit;
static signed char gInited;

/* 0x80076734, size 0x5c */
static int _CancelFunc(int pak)
{
    if (PS_PakChanged(pak) || (gPakErr == 2 && PS_PakDamaged(pak)))
        return 1;
    return 0;
}

/* 0x80076790, size 0x19c */
void updatePakScreen(void)
{
    int i;
    signed char done;
    signed char choice1;
    signed char choice2;
    if (!gInited) {
        anon_texname_dulx_24 mblit[1] = {
            (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)_AnimText, 0
        };
        _SetState(0);
        if (gGameState & 0x7c03fffe) createBG();
        gBlit = (_tBlit_2 *)NewMultiBlit(0, 0, mblit, 1);
        SetControllerActive(-1, 1);
        gInited = 1;
    }
    if (gPakErr) {
        ShowPakError(gPickedPak, &choice1, &choice2, &done, _CancelFunc);
        if (done) gPakErr = 0;
        gDrawText = 0;
    } else {
        gDrawText = 1;
        for (i = 0; i < 4; i++) {
            if (PS_PakChanged(i)) { _SetState(0); break; }
        }
        switch (gState) {
        case 0: _ControlPickPak(); break;
        case 1: _ControlFileDel(); break;
        case 2: _ControlFileDelVerify(); break;
        case 3: _ControlRumblePak(); break;
        case 4: _ControlDamaged(); break;
        }
    }
}

/* 0x8007692c, size 0x44 */
static tPS_File **_FindFile(int idx)
{
    tPS_File **file;
    int i;
    file = gPS_Files[gPickedPak].head;
    for (i = 0; file; i++, file = (*file)->ll.next) {
        if (i == idx) break;
    }
    return file;
}

/* 0x80076970, size 0x58 */
static void _SetState(enum_kNumStates state)
{
    switch (state) {
    case 0: break;
    case 1: _BuildFileDelList(); break;
    case 2: gYes = 0; break;
    }
    gState = state;
}

/* 0x800769c8, size 0x64 */
static void _BuildFileDelList(void)
{
    gMaxMenuHeight = 8;
    gFreePages = PS_GetFreeBlocks(gPickedPak);
    gDeleteSize = gPS_Files[gPickedPak].numElements;
    gDeleteHeight = gMaxMenuHeight < gDeleteSize ? gMaxMenuHeight : gDeleteSize;
}

/* 0x80076a2c, size 0x1ec */
static void _ControlPickPak(void)
{
    if (gActiveEdges & 0x400) {
        AudPlayEffect(8, 0, 1, 0);
        if (--gPickedPak < 0) gPickedPak = 3;
    } else if (gActiveEdges & 0x800) {
        AudPlayEffect(2, 0, 1, 0);
        if (++gPickedPak >= 4) gPickedPak = 0;
    }
    if (gActiveEdges & 2) {
        AudPlayEffect(0, 0, 1, 0);
        if (!gControllerData[gPickedPak / 2].attached) {
            SetPakError(gPickedPak, 1, 0, 0, 10, 50, 300, 144);
            gPakErr = 1;
        } else if (!PS_PakInserted(gPickedPak)) {
            SetPakError(gPickedPak, 2, 0, 0, 10, 50, 300, 144);
            gPakErr = 2;
        } else if (PS_PakDamaged(gPickedPak)) {
            _SetState(4);
        } else if (PS_PakRumble(gPickedPak)) {
            _SetState(3);
        } else {
            gDeleteSelTop = gDeleteSelPos = gDeleteSelect = 0;
            gDeleteFile = _FindFile(gDeleteSelect);
            _SetState(1);
        }
    } else if (gActiveEdges & 5) {
        PlayStartABSound(gActiveEdges);
        setGameState(0x10000000);
        if (gInited) {
            gInited = 0;
            if (gBlit) { RemoveBlit((_tBlit *)gBlit); gBlit = 0; }
            removeBG();
        }
    }
}

/* 0x80076c18, size 0x1c0 */
static void _ControlFileDel(void)
{
    if (gActiveAutoEdges & 0x400) {
        AudPlayEffect(8, 0, 1, 0);
        if (--gDeleteSelect < 0) gDeleteSelect = gDeleteSize - 1;
        if (gDeleteSize > gDeleteHeight) {
            if (gDeleteSelPos == 0) {
                if (--gDeleteSelTop < 0) gDeleteSelTop = gDeleteSize - 1;
            } else gDeleteSelPos--;
        }
    } else if (gActiveAutoEdges & 0x800) {
        AudPlayEffect(2, 0, 1, 0);
        if (++gDeleteSelect >= gDeleteSize) gDeleteSelect = 0;
        if (gDeleteSize > gDeleteHeight) {
            if (gDeleteSelPos == gDeleteHeight - 1) {
                if (++gDeleteSelTop >= gDeleteSize) gDeleteSelTop = 0;
            } else gDeleteSelPos++;
        }
    }
    if (gActiveAutoEdges & 0xc00) gDeleteFile = _FindFile(gDeleteSelect);
    if (((gActiveEdges & 0x100) && (gActiveLevels & 0x200)) ||
        ((gActiveLevels & 0x100) && (gActiveEdges & 0x200))) {
        AudPlayEffect(5, 0, 1, 0);
        if (gDeleteFile) _SetState(2);
    } else if (gActiveEdges & 5) {
        AudPlayEffect(1, 0, 1, 0);
        _SetState(0);
    }
}

/* 0x80076dd8, size 0xac */
static void _ControlFileDelVerify(void)
{
    if (gActiveEdges & 0x3000) {
        PlayLRSound(gActiveEdges);
        gYes = !gYes;
    }
    if (gActiveEdges & 2) {
        AudPlayEffect(0, 0, 1, 0);
        if (gYes) {
            PS_DeleteFile(gDeleteFile);
            gFreePages = PS_GetFreeBlocks(gPickedPak);
            gDeleteFile = _FindFile(gDeleteSelect);
        }
        _SetState(1);
    } else if (gActiveEdges & 5) {
        PlayStartABSound(gActiveEdges);
        _SetState(1);
    }
}

/* 0x80076e84, size 0x34 */
static void _ControlRumblePak(void)
{
    if (gActiveEdges & 7) {
        PlayStartABSound(gActiveEdges);
        _SetState(0);
    }
}

/* 0x80076eb8, size 0x34 */
static void _ControlDamaged(void)
{
    if (gActiveEdges & 7) {
        PlayStartABSound(gActiveEdges);
        _SetState(0);
    }
}

/* 0x80076eec, size 0x228 */
static void _GetPakStatusStr(int pak, unsigned char *str)
{
    anon_ll_fileh_4c **player;
    anon_ll_fileh_30 **ghost;
    if (!gControllerData[pak / 2].attached)
        TB_Sprintf(str, "(%s)", gText.single[0x2bc / 4]);
    else if (!PS_PakInserted(pak))
        TB_Sprintf(str, "(%s)", gText.single[0x2c8 / 4]);
    else if (PS_PakDamaged(pak))
        TB_Sprintf(str, "(%s)", gText.single[0x2c0 / 4]);
    else if (PS_PakRumble(pak))
        TB_Sprintf(str, "(%s)", gText.single[0x2cc / 4]);
    else {
        for (player = gPlayerFiles.head; player; player = (*player)->ll.next)
            if ((*player)->fileh && (*(*player)->fileh)->pak == pak) break;
        for (ghost = gGhostFiles.head; ghost; ghost = (*ghost)->ll.next)
            if ((*ghost)->fileh && (*(*ghost)->fileh)->pak == pak) break;
        if (player || ghost)
            TB_Sprintf(str, "(%s)", gText.single[0x2a4 / 4]);
        else if (PS_GetFreeFiles(pak) >= 1 && PS_GetFreeBlocks(pak) >= gBlocksNeeded)
            TB_Sprintf(str, "(%s)", gText.single[0x2a0 / 4]);
        else
            TB_Sprintf(str, "(%s)", gText.single[0x2ac / 4]);
    }
}

/* 0x80077114, size 0xd0 */
static int _AnimText(void)
{
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(13);
    TB_SetTextColor(12);
    TB_DrawText(TB_CenterText(gText.single[0x29c / 4], 160), 10, gText.single[0x29c / 4]);
    if (gDrawText) {
        switch (gState) {
        case 0: _DrawPickPak(); break;
        case 1: _DrawFileDel(); break;
        case 2: _DrawFileDelVerify(); break;
        case 3: _DrawRumblePak(); break;
        case 4: _DrawDamaged(); break;
        }
    }
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x800771e4, size 0x114 */
static void _DrawPickPak(void)
{
    unsigned char statStr[24];
    unsigned char str[40];
    int y;
    int i;
    int selected; /* Snapshot selection before the text alignment call. */
    for (y = 66, i = 0; i < 4; i++, y += 36) {
        HAL_SetFont(i == gPickedPak ? 10 : 11);
        TB_SetTextColor(i == gPickedPak ? 22 : 1);
        TB_Sprintf(str, "%s %d", gText.single[0x29c / 4], i + 1);
        selected = i == gPickedPak;
        TB_DrawText(TB_CenterText(str, 80), y - selected, str);
        _GetPakStatusStr(i, statStr);
        selected = i == gPickedPak;
        TB_DrawText(TB_CenterText(statStr, 80), y - selected + 12, statStr);
    }
}

/* 0x800772f8, size 0x354 */
static void _DrawFileDel(void)
{
    int y;
    int yDel;
    tPS_File **file;
    char str[64];
    char str1[16];
    yDel = 45;
    HAL_SetFont(11);
    TB_SetTextColor(1);
    if (gDeleteFile) {
        TB_Strcpy(str, gText.taba[gText.tab[49]]);
        if (gText.taba[gText.tab[49] + 1]) {
            TB_Strcat(str, " ");
            TB_Strcat(str, gText.taba[gText.tab[49] + 1]);
        }
        TB_DrawText(TB_CenterText(str, 160), yDel, str);
        yDel += 10;
    }
    y = 65;
    TB_DrawText(35, y, gText.single[0x160 / 4]);
    TB_DrawText(TB_CenterText(gText.single[0x17c / 4], 255), y, gText.single[0x17c / 4]);
    { int i; int idx;
    y += 15;
    HAL_SetFont(14);
    for (i = 0; i < gDeleteHeight; i++, y += 12) {
        idx = i + gDeleteSelTop;
        if (idx >= gDeleteSize) idx -= gDeleteSize;
        file = _FindFile(idx);
        TB_SetTextColor(gDeleteSelect == idx ? 22 : 1);
        TB_Sprintf(str, "%d", idx + 1);
        TB_DrawText(TB_CenterText(str, 23), y, str);
        if (file) TB_Strcpy(str, (*file)->filename);
        else TB_Strcpy(str, gText.single[0x27c / 4]);
        TB_DrawText(35, y, str);
        TB_Sprintf(str1, "%d", file ? (*file)->blocks : 0);
        TB_DrawText(TB_CenterText(str1, 255), y, str1);
        if (gDeleteSelect == idx && file) {
            TB_SetTextColor(1);
            TB_DrawText(TB_CenterText(str, 160), yDel, str);
            yDel += 10;
        }
    }
    }
    HAL_SetFont(11);
    TB_SetTextColor(1);
    if (gDeleteFile && gText.single[0x15c / 4])
        TB_DrawText(TB_CenterText(gText.single[0x15c / 4], 160), yDel, gText.single[0x15c / 4]);
    TB_Sprintf(str, "%d", gFreePages);
    TB_Strcat(str, gFreePages == 1 ? gText.single[0x174 / 4] : gText.single[0x178 / 4]);
    TB_Strcat(str, gFreePages == 1 ? gText.single[0x18c / 4] : gText.single[0x190 / 4]);
    TB_DrawText(TB_CenterText(str, 160), 186, str);
    sprintf(str, "1%s %s %d%s%s", gText.single[0x180 / 4], gText.single[0x188 / 4],
        gBlocksNeeded, gText.single[0x178 / 4], gText.single[0x294 / 4]);
    TB_DrawText(TB_CenterText(str, 160), 196, str);
}

/* 0x8007764c, size 0x1cc */
static void _DrawFileDelVerify(void)
{
    int fontHeight;
    char str[128];
    char str1[128];
    int offset;
    str[0] = 0;
    TB_Strcpy(str1, (*gDeleteFile)->filename);
    TB_Strcpy(str, gText.single[0x25c / 4]);
    TB_Strcat(str1, gText.single[0x260 / 4]);
    HAL_SetFont(11);
    TB_SetTextColor(1);
    fontHeight = TB_GetFontLineHeight();
    TB_SetJust(1, 1);
    TB_DrawText(160, 132 - fontHeight * 3, str);
    HAL_SetFont(14);
    TB_DrawText(160, 132 - fontHeight * 2, str1);
    TB_SetJust(0, 3);
    HAL_SetFont(gYes ? 11 : 10);
    TB_SetTextColor(gYes ? 1 : 22);
    offset = gYes ? 0 : 1;
    TB_DrawText(TB_RightText(gText.taba[gText.tab[25] + 1], 145), 132 - offset, gText.taba[gText.tab[25] + 1]);
    HAL_SetFont(gYes ? 10 : 11);
    TB_SetTextColor(gYes ? 22 : 1);
    TB_DrawText(175, 132 - (gYes != 0), gText.taba[gText.tab[25]]);
}

/* 0x80077818, size 0xb0 */
static void _DrawRumblePak(void)
{
    unsigned char str[256];
    HAL_SetFont(11);
    TB_SetTextColor(1);
    TB_Strcpy(str, gText.single[0x1d0 / 4]);
    TB_Strcat(str, "\n\n");
    TB_Strcat(str, gText.single[0x1b8 / 4]);
    TB_Strcat(str, gText.single[0x1c0 / 4]);
    TB_SetJust(1, 1);
    TB_DrawTextScrollBox(160, 132, 300, 164, -1, 0, str);
    TB_SetJust(0, 3);
}

/* 0x800778c8, size 0xb0 */
static void _DrawDamaged(void)
{
    unsigned char str[256];
    HAL_SetFont(11);
    TB_SetTextColor(1);
    TB_Strcpy(str, gText.single[0x2c4 / 4]);
    TB_Strcat(str, "\n\n");
    TB_Strcat(str, gText.single[0x1b8 / 4]);
    TB_Strcat(str, gText.single[0x1c0 / 4]);
    TB_SetJust(1, 1);
    TB_DrawTextScrollBox(160, 132, 300, 164, -1, 0, str);
    TB_SetJust(0, 3);
}

