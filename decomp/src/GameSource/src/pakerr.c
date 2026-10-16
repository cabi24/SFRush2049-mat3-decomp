/* pakerr.c - memory-card error controls and dialogs. Status: private candidate; awaiting full-link review.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\pakerr.c; assembly: build/2049LE/asm/GameSource/src/pakerr.s */
#include "rush/rush_protos.h"

int sprintf(char *buffer, const char *format, ...);

static void _ControlChoice(int pak, int id);
static void _ControlPakErrNQA(int pak, int id);
static void _ControlPakErrTimeout(int pak, int id);
/* GC callers pass str1/str2 in r6/r7; this callee ignores both and uses fixed labels. */
static void _DrawChoice(int pak, int id, const unsigned char *str, const unsigned char *str1, const unsigned char *str2, int choice1);
static void _DrawPakErrNoController(int pak, int id);
static int _animNewPakText(void);
static int _animNoPakText(void);
static void _DrawPakErrNoPak(int pak, int id);
static void _DrawPakErrNQA(int pak, int id, const unsigned char *str);
static void _DrawPakErrNewPak(int pak, int id);
static void _DrawPakErrInconsistent(int pak, int id);
static void _DrawPakErrFull(int pak, int id);
static void _DrawPakErrBadData(int pak, int id);
static void _DrawPakErrUnknown(int pak, int id);
static int _AnimDim(_tBlit_2 *blt);

static int _mcPak = 0;
static int _mcId = 0;
static anon_done_choice1_34 gPakError[4][3];

/* 0x8002d900, size 0xb4 */
static void _ControlChoice(int pak, int id)
{
    if (gPakError[pak][id].edges & 0xc00) {
        PlayLRSound(gPakError[pak][id].edges);
        gPakError[pak][id].choice1 = !gPakError[pak][id].choice1;
    }
    if (gPakError[pak][id].edges & 2) {
        AudPlayEffect(0, 0, 1, 0);
        gPakError[pak][id].done = 1;
    }
}

/* 0x8002d9b4, size 0x68 */
static void _ControlPakErrNQA(int pak, int id)
{
    if (gPakError[pak][id].edges & 7) {
        PlayStartABSound(gPakError[pak][id].edges);
        gPakError[pak][id].done = 1;
    }
}

/* 0x8002da1c, size 0x3c */
static void _ControlPakErrTimeout(int pak, int id)
{
    if (gPakError[pak][id].timeout < sc.frameCount)
        gPakError[pak][id].done = 1;
}

/* 0x8002da58, size 0x1cc */
static void _DrawChoice(int pak, int id, const unsigned char *str, const unsigned char *str1, const unsigned char *str2, int choice1)
{
    int x, y, h;
    TB_SetTextColor(1);
    HAL_SetFont(11);
    TB_SetJust(1, 1);
    TB_DrawTextScrollBox(gPakError[pak][id].left + gPakError[pak][id].width / 2,
                        gPakError[pak][id].top + 20 + (gPakError[pak][id].height - 40) / 2,
                        gPakError[pak][id].width, gPakError[pak][id].height - 40,
                        -1, 0, (void *)str);
    h = TB_GetFontLineHeight();
    x = gPakError[pak][id].left + gPakError[pak][id].width / 2;
    y = gPakError[pak][id].top + gPakError[pak][id].height - h * 3;
    HAL_SetFont(choice1 ? 11 : 10);
    TB_SetTextColor(choice1 ? 1 : 22);
    TB_DrawText(x, y - !choice1 + h, gText.single[245]);
    HAL_SetFont(choice1 ? 10 : 11);
    TB_SetTextColor(choice1 ? 22 : 1);
    TB_DrawText(x, y - (choice1 != 0), gText.taba[gText.tab[26]]);
    TB_SetJust(0, 3);
}

/* 0x8002dc24, size 0x11c */
static void _DrawPakErrNoController(int pak, int id)
{
    unsigned char str[256];
    HAL_SetFont(11);
    TB_SetTextColor(1);
    TB_Strcpy(str, gText.single[109]);
    TB_Strcat(str, "\n\n");
    TB_Strcat(str, gText.single[110]);
    TB_Strcat(str, gText.single[112]);
    TB_SetJust(1, 1);
    TB_DrawTextScrollBox(gPakError[pak][id].left + gPakError[pak][id].width / 2,
                        gPakError[pak][id].top + 20 + (gPakError[pak][id].height - 20) / 2,
                        gPakError[pak][id].width, gPakError[pak][id].height - 20,
                        -1, 0, str);
    TB_SetJust(0, 3);
    HAL_SetFont(11);
}

/* 0x8002dd40, size 0x42c */
static int _animNewPakText(void)
{
    int x, y, h;
    char tt[256];
    unsigned char str[256];
    int ask;
    ask = !(gGameState & 0x7c03fffe);
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(11);
    if (gPakError[_mcPak][_mcId].name) {
        if (MtapConnected)
            sprintf((char *)str, "MEMORY CARD %s", gPS_PakName[_mcPak]);
        else if (_mcPak < 2)
            sprintf((char *)str, "MEMORY CARD %s", gPS_PakNameNoMTP[_mcPak]);
        else
            return; /* The original callback does not define a return value. */
        TB_DrawText(TB_CenterText(str, gPakError[_mcPak][_mcId].left + gPakError[_mcPak][_mcId].width / 2), gPakError[_mcPak][_mcId].top, str);
        if (MtapConnected)
            sprintf(tt, "DATA ON MEMORY CARD (NGC) IN MEMORY CARD %s HAS CHANGED. INSERT MEMORY CARD (NGC) WITH PROFILE:%s.",
                    gPS_PakName[_mcPak], (*gPlayers[0].save)->name);
        else
            sprintf(tt, "DATA ON MEMORY CARD (NGC) IN MEMORY CARD %s HAS CHANGED. INSERT MEMORY CARD (NGC) WITH PROFILE:%s.",
                    gPS_PakNameNoMTP[_mcPak], (*gPlayers[0].save)->name);
        TB_Strcpy(str, tt);
        TB_SetJust(1, 1);
        TB_DrawTextScrollBox(gPakError[_mcPak][_mcId].left + gPakError[_mcPak][_mcId].width / 2,
                            gPakError[_mcPak][_mcId].top + 20 + (gPakError[_mcPak][_mcId].height - (ask ? 40 : 20)) / 2,
                            gPakError[_mcPak][_mcId].width, gPakError[_mcPak][_mcId].height - (ask ? 40 : 20),
                            -1, 0, str);
        if (ask) {
            h = TB_GetFontLineHeight();
            x = gPakError[_mcPak][_mcId].left + gPakError[_mcPak][_mcId].width / 2;
            y = gPakError[_mcPak][_mcId].top + gPakError[_mcPak][_mcId].height - h * 3;
            HAL_SetFont(gPakError[_mcPak][_mcId].choice1 ? 11 : 10);
            TB_SetTextColor(gPakError[_mcPak][_mcId].choice1 ? 1 : 22);
            TB_DrawText(x, y - !gPakError[_mcPak][_mcId].choice1 + h, gText.single[245]);
            HAL_SetFont(gPakError[_mcPak][_mcId].choice1 ? 10 : 11);
            TB_SetTextColor(gPakError[_mcPak][_mcId].choice1 ? 22 : 1);
            TB_DrawText(x, y - (gPakError[_mcPak][_mcId].choice1 != 0), gText.taba[gText.tab[26]]);
        }
        TB_SetJust(0, 3);
        HAL_SetTextDepth(-1.0f);
    }
    /* GC original has no explicit return value. */
}

/* 0x8002e16c, size 0x8 */
static int _animNoPakText(void)
{
    return 0;
}

/* 0x8002e174, size 0xc */
static void _DrawPakErrNoPak(int pak, int id)
{
    _mcPak = pak;
    _mcId = id;
}

/* 0x8002e180, size 0xc8 */
static void _DrawPakErrNQA(int pak, int id, const unsigned char *str)
{
    HAL_SetFont(11);
    TB_SetTextColor(1);
    TB_SetJust(1, 1);
    TB_DrawTextScrollBox(gPakError[pak][id].left + gPakError[pak][id].width / 2,
                        gPakError[pak][id].top + gPakError[pak][id].height / 2,
                        gPakError[pak][id].width, gPakError[pak][id].height,
                        -1, 0, (void *)str);
    TB_SetJust(0, 3);
}

/* 0x8002e248, size 0xc */
static void _DrawPakErrNewPak(int pak, int id)
{
    _mcPak = pak;
    _mcId = id;
}

/* 0x8002e254, size 0x80 */
static void _DrawPakErrInconsistent(int pak, int id)
{
    unsigned char str[256];
    TB_Strcpy(str, gText.single[119]);
    TB_Strcat(str, "\n\n");
    TB_Strcat(str, gText.single[120]);
    _DrawPakErrNQA(pak, id, str);
}

/* 0x8002e2d4, size 0x80 */
static void _DrawPakErrFull(int pak, int id)
{
    unsigned char str[256];
    TB_Strcpy(str, gText.single[123]);
    TB_Strcat(str, "\n\n");
    TB_Strcat(str, gText.single[124]);
    _DrawPakErrNQA(pak, id, str);
}

/* 0x8002e354, size 0x80 */
static void _DrawPakErrBadData(int pak, int id)
{
    unsigned char str[256];
    TB_Strcpy(str, gText.single[125]);
    TB_Strcat(str, "\n\n");
    TB_Strcat(str, gText.single[120]);
    _DrawPakErrNQA(pak, id, str);
}

/* 0x8002e3d4, size 0x90 */
static void _DrawPakErrUnknown(int pak, int id)
{
    unsigned char str[256];
    TB_Strcpy(str, gText.single[121]);
    TB_Strcat(str, "\n\n");
    TB_Strcat(str, gText.single[110]);
    TB_Strcat(str, gText.single[112]);
    _DrawPakErrNQA(pak, id, str);
}

/* 0x8002e464, size 0x264 */
void SetPakError(int pak, enum_kPS_ErrUnknown err, void * name, int data, int left, int top, int width, int height)
{
    static signed char sInited;
    int i, j;
    if (!sInited) {
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 3; j++) {
                gPakError[i][j].err = kPS_ErrNone;
                gPakError[i][j].done = 1;
            }
        }
        sInited = 1;
    }
    for (i = 0; i < 3; i++) {
        if (gPakError[pak][i].err == kPS_ErrNone)
            break;
    }
    gPakError[pak][i].done = 0;
    gPakError[pak][i].timeout = sc.frameCount + (int)(3.0f * gRetraceRate);
    gPakError[pak][i].name = name;
    gPakError[pak][i].data = data;
    gPakError[pak][i].choice1 = 1;
    gPakError[pak][i].choice2 = 0;
    gPakError[pak][i].err = err;
    for (j = 0; j < 4; j++) {
        gPakError[pak][i].contActive[j] = GetControllerActive(j);
        gPakError[pak][i].contStick[j] = GetControllerStickEdges(j);
        gPakError[pak][i].contDebug[j] = 0;
    }
    SetControllerActive(-1, 0);
    SetControllerStickEdges(-1, 0);
    if (err == kPS_ErrNoCont) {
        SetControllerActive(-1, 1);
        SetControllerStickEdges(-1, 1);
    } else {
        gPakError[pak][i].cont = pak / 2;
        if (err == kPS_ErrNoPak) {
            SetControllerActive(-1, 1);
            SetControllerStickEdges(-1, 1);
        } else {
            SetControllerActive(gPakError[pak][i].cont, 1);
            SetControllerStickEdges(gPakError[pak][i].cont, 1);
        }
    }
    gPakError[pak][i].rumble = PS_PakRumble(pak);
    gPakError[pak][i].left = left;
    gPakError[pak][i].top = top;
    gPakError[pak][i].width = width;
    gPakError[pak][i].height = height;
}

/* 0x8002e6c8, size 0x5c */
void CancelPakError(int pak)
{
    int i;
    for (i = 2; i >= 0; i--) {
        if (gPakError[pak][i].err != kPS_ErrNone) {
            gPakError[pak][i].err = kPS_ErrNone;
            return;
        }
    }
}

/* 0x8002e724, size 0x5d0 */
void ShowPakError(int pak, signed char *choice1, signed char *choice2, signed char *done, int (*cancelFunc)(int))
{
    int i, j;
    char str[256];
    if (cancelFunc && cancelFunc(pak))
        CancelPakError(pak);
    for (i = 2; i >= 0; i--) {
        if (gPakError[pak][i].err != kPS_ErrNone)
            break;
        if (!gPakError[pak][i].done) {
            gPakError[pak][i].done = 1;
            *done = 1;
            *choice2 = 0;
            *choice1 = 0;
            goto restore;
        }
    }
    if (i < 0)
        return;
    gPakError[pak][i].edges = gActiveEdges;
    gPakError[pak][i].levels = gActiveLevels;
    switch (gPakError[pak][i].err) {
    case kPS_ErrNoCont: _ControlChoice(pak, i); break;
    case kPS_ErrNoPak:
        if (!(gGameState & 0x7c03fffe)) _ControlChoice(pak, i);
        else _ControlPakErrNQA(pak, i);
        break;
    case kPS_ErrNewPak:
        if (!(gGameState & 0x7c03fffe)) _ControlChoice(pak, i);
        else _ControlPakErrNQA(pak, i);
        break;
    case kPS_ErrInconsistent: _ControlPakErrNQA(pak, i); break;
    case kPS_ErrUnknown: _ControlPakErrNQA(pak, i); break;
    case kPS_ErrComm:
    case kPS_ErrFail: _ControlChoice(pak, i); break;
    case kPS_ErrExist: _ControlChoice(pak, i); break;
    case kPS_ErrFull: _ControlChoice(pak, i); break;
    case kPS_ErrBadData: _ControlPakErrNQA(pak, i); break;
    case kPS_ErrRecognize: _ControlPakErrTimeout(pak, i); break;
    case kPS_ErrRemoved: _ControlPakErrNQA(pak, i); break;
    case kPS_ErrCorruptFile: _ControlPakErrNQA(pak, i); break;
    case kPS_ErrDevice: gPakError[pak][i].done = 1; break;
    }
    *done = gPakError[pak][i].done;
    if (gPakError[pak][i].done) {
        gPakError[pak][i].err = kPS_ErrNone;
        *choice1 = gPakError[pak][i].choice1;
        *choice2 = gPakError[pak][i].choice2;
        goto restore;
    }
    HAL_SetFont(11);
    TB_SetTextColor(1);
    switch (gPakError[pak][i].err) {
    case kPS_ErrNoCont: _DrawPakErrNoController(pak, i); break;
    case kPS_ErrNoPak: _DrawPakErrNoPak(pak, i); break;
    case kPS_ErrNewPak: _DrawPakErrNewPak(pak, i); break;
    case kPS_ErrInconsistent: _DrawPakErrInconsistent(pak, i); break;
    case kPS_ErrUnknown: _DrawPakErrUnknown(pak, i); break;
    case kPS_ErrComm:
    case kPS_ErrFail:
        _DrawChoice(pak, i, gText.single[121], gText.taba[gText.tab[27] + 1], gText.single[245], gPakError[pak][i].choice1);
        break;
    case kPS_ErrExist:
        _DrawChoice(pak, i, gText.single[122], gText.taba[(int)gText.tab[25]], gText.taba[(int)gText.tab[25] + 1], gPakError[pak][i].choice1);
        break;
    case kPS_ErrFull: _DrawPakErrFull(pak, i); break;
    case kPS_ErrBadData: _DrawPakErrBadData(pak, i); break;
    case kPS_ErrRecognize:
        sprintf(str, "%s%s%s", gText.single[133], gPS_ContName[pak], gText.single[134]);
        _DrawPakErrNQA(pak, i, (unsigned char *)str);
        break;
    case kPS_ErrRemoved:
        if (MtapConnected)
            sprintf(str, "%s%s%s\n\n%s%s", gText.single[135], gPS_PakName[pak], gText.single[136], gText.single[110], gText.single[112]);
        else if (pak < 2)
            sprintf(str, "%s%s%s\n\n%s%s", gText.single[135], gPS_PakNameNoMTP[pak], gText.single[136], gText.single[110], gText.single[112]);
        else
            return;
        _DrawPakErrNQA(pak, i, (unsigned char *)str);
        break;
    case kPS_ErrCorruptFile:
        if (MtapConnected)
            sprintf(str, "%s\"%s\"%s%s\n\n%s%s", gText.single[137], gPakError[pak][i].name, gText.single[138], gPS_PakName[pak], gText.single[110], gText.single[112]);
        else if (pak < 2)
            sprintf(str, "%s\"%s\"%s%s\n\n%s%s", gText.single[137], gPakError[pak][i].name, gText.single[138], gPS_PakNameNoMTP[pak], gText.single[110], gText.single[112]);
        else
            return;
        _DrawPakErrNQA(pak, i, (unsigned char *)str);
        break;
    case kPS_ErrDevice: gPakError[pak][i].done = 1; break;
    }
    return;
restore:
    for (j = 0; j < 4; j++) {
        SetControllerActive(j, gPakError[pak][i].contActive[j]);
        SetControllerStickEdges(j, gPakError[pak][i].contStick[j]);
    }
    UpdateRushControls();
}

/* 0x8002ecf4, size 0x238 */
void PS_ErrorFunc(int pak, enum_kPS_ErrUnknown error, void * name, int data, signed char *choice1, signed char *choice2, int (*cancelFunc)(int))
{
    static const anon_texname_dulx_24 dimMultiBlit[1] = {
        { 0, 0, 0, 320, 224, -1, -1, -1, -1, 0, 192, (int (*)(_tBlit *))_AnimDim, 0 }
    };
    static const anon_texname_dulx_24 noPakBlit[1] = {
        { (const char *)-1, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))_animNoPakText, 0 }
    };
    static const anon_texname_dulx_24 newPakBlit[1] = {
        { (const char *)-1, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (int (*)(_tBlit *))_animNewPakText, 0 }
    };
    signed char done = 0;
    _tBlit *dimBlit;
    _tBlit *nopaktext = 0;
    _tBlit *newpaktext = 0;
    signed short min[4], max[4];
    unsigned char red[4], green[4], blue[4], alpha[4];
    int i;
    dimBlit = NewMultiBlit(0, 0, dimMultiBlit, 1);
    ZOID_IgnoreDrawFirstBlit(dimBlit->blidx);
    if (error == kPS_ErrNoPak)
        nopaktext = NewMultiBlit(0, 0, noPakBlit, 1);
    else if (error == kPS_ErrNewPak)
        newpaktext = NewMultiBlit(0, 0, newPakBlit, 1);
    SetPakError(pak, error, name, data, 10, 10, 300, 184);
    for (i = 0; i < gNumWindows; i++) {
        ZOID_GetFog(&min[i], &max[i], &red[i], &green[i], &blue[i], &alpha[i], i);
        ZOID_SetFog(0, 0, 0, 0, 0, 255, i);
    }
    while (!done) {
        UpdateFrame();
        SaveServer(0, 0);
        ShowPakError(pak, choice1, choice2, &done, cancelFunc);
    }
    RemoveBlit(dimBlit);
    if (nopaktext)
        RemoveBlit(nopaktext);
    if (newpaktext)
        RemoveBlit(newpaktext);
    for (i = 0; i < gNumWindows; i++)
        ZOID_SetFog(min[i], max[i], red[i], green[i], blue[i], alpha[i], i);
}

/* 0x8002ef2c, size 0x2c */
static int _AnimDim(_tBlit_2 *blt)
{
    static unsigned char color[4] = { 0, 0, 0, 224 };
    blt->palette = (anon_name_start_18 *)color;
    UpdateBlit((_tBlit *)blt);
    return 1;
}

