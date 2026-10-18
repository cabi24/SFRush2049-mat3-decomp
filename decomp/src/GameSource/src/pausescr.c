/* pausescr.c - pause menus and controller/disc warning state machines.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\pausescr.c; assembly: build/2049LE/asm/GameSource/src/pausescr.s */
/* The original geometry is const, while the generated externs omit const. */
#define menucontrolpos menucontrolpos_header_decl
#define exitpicpos exitpicpos_header_decl
#define pausefontpos pausefontpos_header_decl
#define resumefontpos resumefontpos_header_decl
#define controlfontpos controlfontpos_header_decl
#define scrsizefontpos scrsizefontpos_header_decl
#define quitfontpos quitfontpos_header_decl
#define exitfontpos exitfontpos_header_decl
#define selectfontpos selectfontpos_header_decl
#define backfontpos backfontpos_header_decl
#define barpos1 barpos1_header_decl
#define barpos2 barpos2_header_decl
#define barpos3 barpos3_header_decl
#define barpos4 barpos4_header_decl
#define barpos5 barpos5_header_decl
#define changefontpos changefontpos_header_decl
#define screenfontpos screenfontpos_header_decl
#define areufontpos areufontpos_header_decl
#define yesfontpos yesfontpos_header_decl
#define nofontpos nofontpos_header_decl
#define barposyes barposyes_header_decl
#define barposno barposno_header_decl
#define SetFont SetFont_header_decl
#define UpdateBlit UpdateBlit_header_decl
#define RemoveBlit RemoveBlit_header_decl
#define NewMultiBlit NewMultiBlit_header_decl
#define appSleep appSleep_header_decl
#define gControllerData gControllerData_header_decl
#include "rush/rush_protos.h"
#undef gControllerData
/* This TU reads the volatile controller aggregate. Volatile scalar members in
   the generated shared type select a different update-addressing instruction. */
typedef struct {
    signed char attached, incompatible;
    unsigned int availButtons, buttons;
    signed char stick1x, stick1y, stick2x, stick2y;
    unsigned char analog1, analog2;
} PauseControllerData;
extern volatile PauseControllerData gControllerData[4];
#undef appSleep
void appSleep(double);
#undef NewMultiBlit
_tBlit_2 *NewMultiBlit(signed short, signed short, const anon_texname_dulx_24 *, signed short);
#undef UpdateBlit
#undef RemoveBlit
void UpdateBlit(_tBlit_2 *blit);
void RemoveBlit(_tBlit_2 *blit);
#undef SetFont
/* The original call promotes these constants to double. */
void SetFont();

#undef menucontrolpos
#undef exitpicpos
#undef pausefontpos
#undef resumefontpos
#undef controlfontpos
#undef scrsizefontpos
#undef quitfontpos
#undef exitfontpos
#undef selectfontpos
#undef backfontpos
#undef barpos1
#undef barpos2
#undef barpos3
#undef barpos4
#undef barpos5
#undef changefontpos
#undef screenfontpos
#undef areufontpos
#undef yesfontpos
#undef nofontpos
#undef barposyes
#undef barposno

static int _animControllerDim(_tBlit_2 *blt);
static int _animControllerText(void);
static void updatePauseStateCreate(void);
static void updateSelPauseStateCreate(void);
static void _TitlePane(void);
static int animPause(_tBlit_2 *blt);
static int animTitleText(_tBlit_2 *blt);
static int animSelTitleText(_tBlit_2 *blt);
static int animPauseText(_tBlit_2 *blt);
static int animSelPauseText(_tBlit_2 *blt);
static int animSureText(_tBlit_2 *blt);
static int animSelSureText(void);
static int animRumbleText(_tBlit_2 *blt);
static int animRumblePane(_tBlit_2 *blt);

static const anon_texname_dulx_24 DimMultiBlit[1] = { 0, 0, 0, 320, 224, -1, -1, -1, -1, 0, 0x10, (void *)&animPause, 0 };
static const anon_texname_dulx_24 TitleMultiBlit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animTitleText, 0 };
static const anon_texname_dulx_24 SelTitleMultiBlit[1] = {
    (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animSelTitleText, 0
};
static const anon_texname_dulx_24 PauseMultiBlit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animPauseText, 0 };
static const anon_texname_dulx_24 SelPauseMultiBlit[1] = {
    (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animSelPauseText, 0
};
static const anon_texname_dulx_24 SureMultiBlit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animSureText, 0 };
static const anon_texname_dulx_24 SelSureMultiBlit[1] = {
    (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animSelSureText, 0
};
static const anon_texname_dulx_24 RumbleMultiBlit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animRumbleText, 0 };
int g_CurrentControl_t[4] = { 0 }; /* explicit `= { 0 }`: the target keeps this object in initialised data (.data/.sdata), not .bss */
static anon_x_y_18 gPlayerRects[4][4] = {
    0, 0, 320, 110, 160, 55, 0, 0, 320, 220, 160, 110, 0, 0, 320, 220, 160, 110, 0, 0, 320, 220,
    160, 110, 0, 0, 320, 110, 160, 55, 0, 110, 320, 110, 160, 165, 0, 0, 320, 220, 160, 110, 0, 0,
    320, 220, 160, 110, 0, 0, 160, 110, 80, 55, 160, 0, 160, 110, 240, 55, 0, 110, 160, 110, 80,
    165, 0, 0, 320, 220, 160, 110, 0, 0, 160, 110, 80, 55, 160, 0, 160, 110, 240, 55, 0, 110, 160,
    110, 80, 165, 160, 110, 160, 110, 240, 165
};
static anon_x_y_4 titlePos = { 160, 10 };
signed char ChooseYes[4] = { 0 }; /* explicit `= { 0 }`: the target keeps this object in initialised data (.data/.sdata), not .bss */
static int _mcAlpha = 255;
int playerInPause;
static enum_kPauseMenuSize pauseMenuSelect;
static enum_kSelPauseMenuSize seletPauseMenuSelect;
signed char gPakInserted[4];
static int pre_control_choose;
unsigned int g_PlayerInControlDis;
unsigned int g_PlayerInPause;
int g_bDvdTrc;
int g_bQuitFromTrc;
int g_bGoToQuitTrc;
int g_bControllerTrc;
static int pre_dvd_PauseState;
static int pre_control_PauseState;
static int _mcID;
static signed char gYes;
static signed char gControllerRemoved;
static anon_used_perm_20 *gSelSurePane;
static anon_used_perm_20 *gSelMenuPane;
static anon_used_perm_20 *gSelTitlePane;
static anon_used_perm_20 *gTitlePane;
static _tBlit_2 *RumbleBlit;
static _tBlit_2 *SelSureBlit;
static _tBlit_2 *SureBlit;
static _tBlit_2 *SelPauseBlit;
static _tBlit_2 *PauseBlit;
static _tBlit_2 *SelTitleBlit;
static _tBlit_2 *TitleBlit;
static _tBlit_2 *SelDimBlit;
static _tBlit_2 *DimBlit;
enum_kSelNumPauseStates_2 gSelPauseState;
enum_kNumPauseStates_9 gPauseState;

/* 0x80077978, size 0x2c */
static int _animControllerDim(_tBlit_2 *blt)
{
    static unsigned int color = 0xe0;
    blt->palette = (anon_name_start_18 *)&color;
    UpdateBlit(blt);
    return 1;
}

/* 0x800779a4, size 0x104 */
static int _animControllerText(void)
{
    unsigned char str[40];
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(11);
    TB_Sprintf(str, "MEMORY CARD (PS2) IN SLOT %d WAS REMOVED", _mcID + 1);
    TB_SetTextColor(1);
    HAL_SetTextTranslucency((float)_mcAlpha);
    TB_DrawText(TB_CenterText(str, 160), 92, str);
    TB_Sprintf(str, "WHICH IS NECESSARY WHILE SAVING YOUR PROGRESS", _mcID + 1);
    TB_DrawText(TB_CenterText(str, 160), 107, str);
    TB_Sprintf(str, "WHEN GAME IS OVER!", _mcID + 1);
    TB_DrawText(TB_CenterText(str, 160), 122, str);
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x80077aa8, size 0x3d0 */
void updatePauseScreen(void)
{
    static const anon_texname_dulx_24 dimMultiBlit[1] = { 0, 0, 0, 320, 224, -1, -1, -1, -1, 0, 0xc0, (void *)&_animControllerDim, 0 };
    static const anon_texname_dulx_24 titleMultiBlit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&_animControllerText, 0 };
    int i;
    if ((gPauseState == 0 || gPauseState == 1 || gPauseState == 6 || gPauseState == 7 || gPauseState == 8) && gSoftReset) {
        gRestartGame = 0;
        setPauseState(0);
        ResumeTimer();
        AudStopAllEffects();
        AbortGame();
        return;
    }
    switch (gPauseState) {
    case 0:
        for (i = 0; i < gNumWindows; i++) {
            if (gPlayers[i].edges & 1) {
                playerInPause = i;
                gControllerRemoved = !gControllerData[i].attached;
                goto pause_game;
            }
        }
        break;
    pause_game:
        setPauseState(1);
        pauseMenuSelect = 0;
        PauseTimer();
        AudPauseAllEffects();
        PauseGameMusic();
        StopListeners();
        modelstop();
        PauseTargets();
        SetControllerStickEdges(-1, 1);
        SaveFlush(0);
        AudPlayEffect(7, 0, 1, 0);
        break;
    case 5: updateOptionScreen(); break;
    case 4: updateControlScreen(); break;
    case 2: updateAudioScreen(); break;
    case 3: updateVideoScreen(); break;
    case 1:
        if (gPlayers[playerInPause].edges & 2) {
            AudPlayEffect(0, 0, 1, 0);
            switch (pauseMenuSelect) {
            case 0:
            resume_game:
                setPauseState(0);
                ResumeTimer();
                StartListeners();
                AudResumeAllEffects();
                ResumeGameMusic();
                if (!(gGameState & 0x200000)) StartGameMusic();
                modelgo();
                SetControllerStickEdges(-1, 0);
                break;
            case 4:
                if (gModelTime < 15.0f) {
                    gRestartGame = 1;
                    goto abort_game;
                } else setPauseState(7);
                break;
            case 5: setPauseState(8); break;
            case 2: setPauseState(2); break;
            case 3: setPauseState(3); break;
            case 1: setPauseState(5); break;
            }
        } else if (gPlayers[playerInPause].edges & 0x400) {
            PlayUDSound(gPlayers[playerInPause].edges);
            if (--pauseMenuSelect < 0) pauseMenuSelect = 5;
        } else if (gPlayers[playerInPause].edges & 0x800) {
            PlayUDSound(gPlayers[playerInPause].edges);
            if (++pauseMenuSelect >= 6) pauseMenuSelect = 0;
        } else if (gPlayers[playerInPause].edges & 5) {
            PlayStartABSound(gPlayers[playerInPause].edges);
            goto resume_game;
        }
        break;
    case 7: case 8:
        if (gPlayers[playerInPause].edges & 0x3000) {
            gYes = !gYes;
            PlayLRSound(gPlayers[playerInPause].edges);
        }
        if (gPlayers[playerInPause].edges & 2) {
            if (gYes) {
                if ((gGameState & 0x400000) && gPauseState == 8) {
                    gRetireGame = 1;
                    setPauseState(0);
                    ResumeTimer();
                    AudStopAllEffects();
                } else {
                    gRestartGame = gPauseState == 7;
                abort_game:
                    setPauseState(0);
                    ResumeTimer();
                    AudStopAllEffects();
                    AbortGame();
                }
            } else setPauseState(1);
        } else if (gPlayers[playerInPause].edges & 5) {
            PlayStartABSound(gPlayers[playerInPause].edges);
            setPauseState(1);
        }
        break;
    case 6:
        if (gPlayers[playerInPause].edges & 7) {
            PlayStartABSound(gPlayers[playerInPause].edges);
            setPauseState(1);
        }
        break;
    }
}

/* 0x80077e78, size 0x7e8 */
void updateSelectPauseScreen(void)
{
    int i;
    if (!g_bDvdTrc) {
        if (g_bDvdIsError) {
            g_bDvdTrc = 1;
            pre_dvd_PauseState = gSelPauseState;
            setSelPauseState(1);
            PauseTimer();
            AudPauseAllEffects();
            sndPlayerMusicPause();
            PauseGameMusic();
            modelstop();
            PauseTargets();
            SetControllerStickEdges(-1, 1);
            SaveFlush(0);
            return;
        }
    } else {
        if (!g_bDvdIsError) {
            g_bDvdTrc = 0;
            setSelPauseState(pre_dvd_PauseState);
            sndResumeMusic();
            sndPlayerMusicUnpause();
            sndPlayerMusicPause();
            if (pre_dvd_PauseState == 0 && !IsInSubMenu()) {
                ResumeTimer();
                AudResumeAllEffects();
                sndPlayerMusicUnpause();
                ResumeGameMusic();
                modelgo();
                SetControllerStickEdges(-1, 0);
            }
        }
        return;
    }
    if (!g_bControllerTrc) {
        for (i = 0; i < (gRace == 2 ? 1 : gNumPlayers); i++) {
            if (!gControllerData[i].attached) {
                g_bControllerTrc = 1;
                g_bGoToQuitTrc = 0;
                g_bQuitFromTrc = 0;
                pre_control_PauseState = gSelPauseState;
                setSelPauseState(2);
                g_PlayerInControlDis = i;
                PauseTimer();
                AudPauseAllEffects();
                sndPlayerMusicPause();
                PauseGameMusic();
                modelstop();
                PauseTargets();
                SetControllerStickEdges(-1, 1);
                SaveFlush(0);
                AudPlayEffect(7, 0, 1, 0);
                return;
            }
        }
    } else {
        if (!g_bGoToQuitTrc) {
            if (gContEdges[g_PlayerInControlDis] & 0x40001) {
            restore_controller:
                g_bControllerTrc = 0;
                setSelPauseState(pre_control_PauseState);
                AudPlayEffect(0, 0, 1, 0);
                if (pre_control_PauseState == 0 && gPauseState == 0) {
                    ResumeTimer();
                    AudResumeAllEffects();
                    sndPlayerMusicUnpause();
                    ResumeGameMusic();
                    modelgo();
                    SetControllerStickEdges(-1, 0);
                }
            }
        } else {
            if (gContEdges[0] & 0xc00) {
                PlayLRSound(gContEdges[0]);
                g_bQuitFromTrc = !g_bQuitFromTrc;
            }
            if (gContEdges[0] & 2) {
                AudPlayEffect(0, 0, 1, 0);
                if (!g_bQuitFromTrc) goto restore_controller;
                else Run("ffe", 0);
            }
            if (gContEdges[0] & 0x40004) goto restore_controller;
        }
        return;
    }
    switch (gSelPauseState) {
    case 0:
        for (i = 0; i < gNumWindows; i++) {
            if ((gContEdges[i] & 0x40000) && !IsInSubMenu()) {
                g_PlayerInPause = i;
                gControllerRemoved = !gControllerData[i].attached;
                goto pause_select;
            }
        }
        break;
    pause_select:
        setSelPauseState(3);
        seletPauseMenuSelect = 0;
        PauseTimer();
        AudPauseAllEffects();
        sndPlayerMusicPause();
        PauseGameMusic();
        modelstop();
        PauseTargets();
        SetControllerStickEdges(-1, 1);
        SaveFlush(0);
        AudPlayEffect(7, 0, 1, 0);
        break;
    case 3:
        if (gContEdges[g_PlayerInPause] & 2) {
            AudPlayEffect(0, 0, 1, 0);
            switch (seletPauseMenuSelect) {
            case 0:
            resume_select:
                setSelPauseState(0);
                sndPlayerMusicUnpause();
                if (gPauseState == 0) {
                    ResumeTimer();
                    AudResumeAllEffects();
                    ResumeGameMusic();
                    modelgo();
                    SetControllerStickEdges(-1, 0);
                }
                break;
            case 1:
                pre_control_choose = ChooseYes[g_PlayerInPause];
                setSelPauseState(4);
                break;
            case 2: setSelPauseState(5); break;
            case 3: if (gGameState & 0x400000) setSelPauseState(6); break;
            case 4: setSelPauseState(7); break;
            }
        } else if (gContEdges[g_PlayerInPause] & 0x400) {
            PlayUDSound(gContEdges[g_PlayerInPause]);
            if (--seletPauseMenuSelect < 0) seletPauseMenuSelect = 4;
            if (seletPauseMenuSelect == 3 && !(gGameState & 0x400000)) seletPauseMenuSelect = 2;
        } else if (gContEdges[g_PlayerInPause] & 0x800) {
            PlayUDSound(gContEdges[g_PlayerInPause]);
            if (++seletPauseMenuSelect >= 5) seletPauseMenuSelect = 0;
            if (seletPauseMenuSelect == 3 && !(gGameState & 0x400000)) seletPauseMenuSelect = 4;
        } else if (gContEdges[g_PlayerInPause] & 0x40004) {
            PlayStartABSound(gContEdges[g_PlayerInPause]);
            goto resume_select;
        }
        break;
    case 4:
        if (gContEdges[g_PlayerInPause] & 0x40004) {
            PlayStartABSound(gContEdges[g_PlayerInPause]);
            setSelPauseState(3);
            ChooseYes[g_PlayerInPause] = pre_control_choose;
        }
        break;
    case 5:
        if (gContEdges[g_PlayerInPause] & 2) {
            PlayStartABSound(gContEdges[g_PlayerInPause]);
            setSelPauseState(3);
            ConfirmScreenSize();
        }
        if (gContEdges[g_PlayerInPause] & 0x40004) {
            PlayStartABSound(gContEdges[g_PlayerInPause]);
            setSelPauseState(3);
            CancelScreenSize();
        }
        if (gControllerData[g_PlayerInPause].buttons & 0x2000) TurnScreenSize(-5);
        else if (gControllerData[g_PlayerInPause].buttons & 0x1000) TurnScreenSize(5);
        break;
    case 6:
        if (gContEdges[g_PlayerInPause] & 0xc00) {
            gYes = !gYes;
            PlayLRSound(gPlayers[g_PlayerInPause].edges);
        }
        if (gContEdges[g_PlayerInPause] & 2) {
            PlayStartABSound(gContEdges[g_PlayerInPause]);
            if (gYes) {
                AudioMain();
                if (gGameState & 0x400000) {
                    gRetireGame = 1;
                    setSelPauseState(0);
                    ResumeTimer();
                    AudStopAllEffects();
                    setPauseState(0);
                    AbortGame();
                }
            } else setSelPauseState(3);
        } else if (gPlayers[g_PlayerInPause].edges & 0x40004) {
            PlayStartABSound(gPlayers[g_PlayerInPause].edges);
            setSelPauseState(3);
        }
        break;
    case 7:
        if (gContEdges[g_PlayerInPause] & 0xc00) {
            gYes = !gYes;
            PlayLRSound(gContEdges[g_PlayerInPause]);
        }
        if (gContEdges[g_PlayerInPause] & 2) {
            PlayStartABSound(gContEdges[g_PlayerInPause]);
            if (gYes) {
                AudioMain();
                appSleep(0.5);
                Run("ffe", 0);
                if ((gGameState & 0x400000) && gSelPauseState == 7) {
                    gRetireGame = 1;
                    setSelPauseState(0);
                    ResumeTimer();
                    AudStopAllEffects();
                }
            } else setSelPauseState(3);
        } else if (gContEdges[g_PlayerInPause] & 0x40004) {
            PlayStartABSound(gContEdges[g_PlayerInPause]);
            setSelPauseState(3);
        }
        break;
    }
}

/* 0x80078660, size 0x10c */
void updatePauseStateCleanup(void)
{
    int p;
    switch (gPauseState) {
    case 0:
        if (DimBlit) { RemoveBlit(DimBlit); DimBlit = 0; }
        for (p = 0; p < gNumPlayers; p++)
            if ((*gPlayers[p].save)->datah) SaveSelectPlayer(gPlayers[p].save);
    case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
        if (TitleBlit) { RemoveBlit(TitleBlit); TitleBlit = 0; }
        if (PauseBlit) { RemoveBlit(PauseBlit); PauseBlit = 0; }
        if (SureBlit) { RemoveBlit(SureBlit); SureBlit = 0; }
        if (RumbleBlit) { RemoveBlit(RumbleBlit); RumbleBlit = 0; }
    }
    RemovePanes(0);
    gTitlePane = 0;
}

/* 0x8007876c, size 0x138 */
void updateSelPauseStateCleanup(void)
{
    int p;
    switch (gSelPauseState) {
    case 0:
        if (SelDimBlit) { RemoveBlit(SelDimBlit); SelDimBlit = 0; }
        for (p = 0; p < gNumPlayers; p++)
            if (gPlayers[p].save && (*gPlayers[p].save)->datah) SaveSelectPlayer(gPlayers[p].save);
    case 3: case 7:
        if (SelTitleBlit) { RemoveBlit(SelTitleBlit); SelTitleBlit = 0; }
        if (SelPauseBlit) { RemoveBlit(SelPauseBlit); SelPauseBlit = 0; }
        if (SelSureBlit) { RemoveBlit(SelSureBlit); SelSureBlit = 0; }
        if (gSelSurePane) { RemovePane(gSelSurePane); gSelSurePane = 0; }
    }
    RemovePane(gSelTitlePane);
    gSelTitlePane = 0;
    if (gSelMenuPane) { RemovePane(gSelMenuPane); gSelMenuPane = 0; }
}

/* 0x800788a4, size 0x42c */
static void updatePauseStateCreate(void)
{
    int max;
    int width;
    int i;
    int x;
    int y;
    switch (gPauseState) {
    case 1:
        if (!DimBlit) DimBlit = NewMultiBlit(0, 0, DimMultiBlit, 1);
        _TitlePane();
        HAL_SetFont(13);
        x = titlePos.x;
        y = titlePos.y + 2 * TB_GetFontLineHeight();
        HAL_SetFont(10);
        max = 0;
        for (i = 0; i < 5; i++) {
            if (i > 3) width = HAL_GetTextWidth(gText.taba[i + 2 + gText.tab[58]], -1);
            else width = HAL_GetTextWidth(gText.taba[i + 1 + gText.tab[58]], -1);
            if (max < width) max = width;
        }
        max /= 2;
        InitPane(x - max - 8, y - 4, x + max + 8, y + 6 * TB_GetFontLineHeight() + 4, 176, 0, 0, 0);
        TitleBlit = NewMultiBlit(0, 0, TitleMultiBlit, 1);
        PauseBlit = NewMultiBlit(0, 0, PauseMultiBlit, 1);
        break;
    case 6:
        gYes = 0;
        _TitlePane();
        HAL_SetFont(10);
        x = gPlayerRects[0][1].cx;
        y = gPlayerRects[0][1].cy - TB_GetFontLineHeight() / 2;
        width = HAL_GetTextWidth(gText.single[238], -1);
        width /= 2;
        InitPane(x - width - 8, y - 4, x + width + 8, y + TB_GetFontLineHeight() + 4, 176, animRumblePane, 0, 0);
        HAL_SetFont(11);
        width = HAL_GetTextWidth(gText.single[181], -1);
        width = (unsigned int)width > HAL_GetTextWidth(gText.single[180], -1) ? width : HAL_GetTextWidth(gText.single[180], -1);
        width /= 2;
        for (max = 0; max < gNumPlayers; max++) {
            x = gPlayerRects[gNumPlayers - 1][max].cx;
            y = gPlayerRects[gNumPlayers - 1][max].cy - TB_GetFontLineHeight();
            InitPane(x - width - 8, y - 4, x + width + 8, y + 2 * TB_GetFontLineHeight() + 4, 176, 0, 0, 0);
        }
        TitleBlit = NewMultiBlit(0, 0, TitleMultiBlit, 1);
        RumbleBlit = NewMultiBlit(0, 0, RumbleMultiBlit, 1);
        break;
    case 7: case 8:
        gYes = 0;
        _TitlePane();
        HAL_SetFont(13);
        x = titlePos.x;
        y = titlePos.y + 2 * TB_GetFontLineHeight();
        HAL_SetFont(10);
        width = HAL_GetTextWidth(gText.single[227], -1);
        width /= 2;
        InitPane(x - width - 8, y - 4, x + width + 8, y + 2 * TB_GetFontLineHeight() + 4, 176, 0, 0, 0);
        TitleBlit = NewMultiBlit(0, 0, TitleMultiBlit, 1);
        SureBlit = NewMultiBlit(0, 0, SureMultiBlit, 1);
        break;
    }
}

/* 0x80078cd0, size 0x28 */
static void updateSelPauseStateCreate(void)
{
    switch (gSelPauseState) {
    case 3: break;
    case 7: gYes = 0; break;
    }
}

static char *strSelPauseMenu[2] = { "RESUME", "EXIT" };

/* 0x80078cf8, size 0x210 */
static void _TitlePane(void)
{
    int x;
    int y;
    int width;
    unsigned char *str;
    unsigned char str1[64];
    HAL_SetFont(13);
    str = str1;
    if (gControllerRemoved) {
        TB_Sprintf(str1, "CONTROLLER %d %s", playerInPause + 1, gText.single[242]);
        width = HAL_GetTextWidth(str, -1);
    } else if (gPauseState == 7 || gPauseState == 8) {
        width = HAL_GetTextWidth(gText.taba[gText.tab[58] + gPauseState], -1);
    } else if (gPauseState == 6) {
        width = 0;
    } else {
        if (gPlayers[playerInPause].save)
            TB_Sprintf(str1, "%s %s", (*gPlayers[playerInPause].save)->name, gText.taba[gText.tab[58]]);
        else
            TB_Sprintf(str1, "%s", gText.taba[gText.tab[58]]);
        width = HAL_GetTextWidth(str, -1);
    }
    x = titlePos.x;
    y = titlePos.y;
    if (!gTitlePane) gTitlePane = InitPane(-20, 0, -10, 10, 176, 0, 0, 0);
    if (!width) ChangePaneSize(gTitlePane, -20, 0, -10, 10);
    else {
        width /= 2;
        ChangePaneSize(gTitlePane, x - width - 8, y - 4, x + width + 8, y + TB_GetFontLineHeight() + 4);
    }
}

/* 0x80078f08, size 0x28 */
void setPauseState(enum_kNumPauseStates_9 state)
{
    gPauseState = state;
    updatePauseStateCleanup();
    updatePauseStateCreate();
}

/* 0x80078f30, size 0x28 */
void setSelPauseState(enum_kSelNumPauseStates_2 state)
{
    gSelPauseState = state;
    updateSelPauseStateCleanup();
    updateSelPauseStateCreate();
}

/* 0x80078f58, size 0x28 */
int IsInSubMenu(void)
{
    if (gPauseState == 0) return 0;
    return gPauseState != 1;
}

/* 0x80078f80, size 0x70 */
static int animPause(_tBlit_2 *blt)
{
    static unsigned int color = 0x80;
    if (Hidden(blt, pauseMenuSelect != 0)) return 1;
    ZOID_IgnoreDrawFirstBlit(blt->blidx);
    blt->palette = (anon_name_start_18 *)&color;
    UpdateBlit(blt);
    blt->animFunc = 0;
    return 1;
}

/* 0x80078ff0, size 0x1c4 */
static int animTitleText(_tBlit_2 *blt)
{
    int x;
    int y;
    unsigned char *str;
    unsigned char str1[40];
    if (Hidden(blt, 0)) return 1;
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(13);
    x = titlePos.x;
    y = titlePos.y;
    str = str1;
    if (gControllerRemoved) {
        TB_Sprintf(str1, "CONTROLLER %d %s", playerInPause + 1, gText.single[242]);
        TB_SetTextColor(1);
    } else if (gPauseState == 7 || gPauseState == 8) {
        str = gText.taba[gText.tab[58] + gPauseState];
        TB_SetTextColor(1);
    } else if (gPauseState == 6) {
        str1[0] = 0;
    } else {
        if (gPlayers[playerInPause].save)
            TB_Sprintf(str1, "%s %s", (*gPlayers[playerInPause].save)->name, gText.taba[gText.tab[58]]);
        else
            TB_Sprintf(str1, "%s", gText.taba[gText.tab[58]]);
        TB_SetTextColor(playerTextColor[playerInPause]);
    }
    TB_DrawText(TB_CenterText(str, x), y, str);
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x800791b4, size 0x1c8 */
static int animSelTitleText(_tBlit_2 *blt)
{
    int x;
    int y;
    unsigned char *str;
    unsigned char str1[40];
    if (Hidden(blt, 0)) return 1;
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(13);
    x = titlePos.x;
    y = titlePos.y + 50;
    str = str1;
    if (gControllerRemoved) {
        TB_Sprintf(str1, "CONTROLLER %d %s", playerInPause + 1, gText.single[242]);
        TB_SetTextColor(1);
    } else if (gPauseState == 7 || gPauseState == 8) {
        str = gText.taba[gText.tab[58] + gPauseState];
        TB_SetTextColor(1);
    } else if (gPauseState == 6) {
        str1[0] = 0;
    } else {
        if (gPlayers[playerInPause].save)
            TB_Sprintf(str1, "%s %s", (*gPlayers[playerInPause].save)->name, gText.taba[gText.tab[58]]);
        else
            TB_Sprintf(str1, "%s", gText.taba[gText.tab[58]]);
        TB_SetTextColor(playerTextColor[playerInPause]);
    }
    TB_DrawText(TB_CenterText(str, x), y, str);
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x8007937c, size 0x144 */
static int animPauseText(_tBlit_2 *blt)
{
    int i;
    int x;
    int y;
    unsigned char *str;
    if (Hidden(blt, 0)) return 1;
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(13);
    x = titlePos.x;
    y = titlePos.y + 2 * TB_GetFontLineHeight();
    HAL_SetFont(10);
    for (i = 0; i < 6; i++) {
        TB_SetTextColor(i == pauseMenuSelect ? 22 : 1);
        if (i > 1 && i <= 3) str = gText.taba[i + 2 + gText.tab[58]];
        else if (i > 3) str = gText.taba[i + 3 + gText.tab[58]];
        else str = gText.taba[i + 1 + gText.tab[58]];
        TB_DrawText(TB_CenterText(str, x), y, str);
        y += TB_GetFontLineHeight();
    }
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x800794c0, size 0xd8 */
static int animSelPauseText(_tBlit_2 *blt)
{
    int i;
    int x;
    int y;
    unsigned char *str;
    if (Hidden(blt, 0)) return 1;
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(13);
    x = titlePos.x;
    y = titlePos.y + 2 * TB_GetFontLineHeight() + 50;
    HAL_SetFont(10);
    for (i = 0; i < 5; i++) {
        TB_SetTextColor(i == seletPauseMenuSelect ? 22 : 1);
        str = (unsigned char *)strSelPauseMenu[i];
        TB_DrawText(TB_CenterText(str, x), y, str);
        y += TB_GetFontLineHeight();
    }
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x80079598, size 0x18c */
static int animSureText(_tBlit_2 *blt)
{
    int width;
    int x;
    int y;
    unsigned char *str;
    if (Hidden(blt, gPauseState < 7)) return 1;
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(13);
    x = titlePos.x;
    y = titlePos.y + 2 * TB_GetFontLineHeight();
    HAL_SetFont(10);
    TB_SetTextColor(1);
    str = gText.single[227];
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    width = HAL_GetTextWidth(gText.single[227], -1);
    x = titlePos.x - width / 4;
    str = gText.taba[gText.tab[25]];
    TB_SetTextColor(gYes ? 22 : 1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    x = titlePos.x + width / 4;
    str = gText.taba[gText.tab[25] + 1];
    TB_SetTextColor(gYes ? 1 : 22);
    TB_DrawText(TB_CenterText(str, x), y, str);
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x80079724, size 0x160 */
static int animSelSureText(void)
{
    int width;
    int x;
    int y;
    unsigned char *str;
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(13);
    x = titlePos.x;
    y = titlePos.y + 2 * TB_GetFontLineHeight() + 50;
    HAL_SetFont(10);
    TB_SetTextColor(1);
    str = gText.single[227];
    TB_DrawText(TB_CenterText(str, x), y, str);
    y += TB_GetFontLineHeight();
    width = HAL_GetTextWidth(gText.single[227], -1);
    x = titlePos.x - width / 4;
    str = gText.taba[gText.tab[25]];
    TB_SetTextColor(gYes ? 22 : 1);
    TB_DrawText(TB_CenterText(str, x), y, str);
    x = titlePos.x + width / 4;
    str = gText.taba[gText.tab[25] + 1];
    TB_SetTextColor(gYes ? 1 : 22);
    TB_DrawText(TB_CenterText(str, x), y, str);
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x80079884, size 0x22c */
static int animRumbleText(_tBlit_2 *blt)
{
    unsigned char *str;
    unsigned short i;
    signed short x;
    signed short y;
    int allRumblesPresent = 1;
    if (Hidden(blt, gPauseState != 6)) return 1;
    HAL_SetTextDepth(0.0f);
    for (i = 0; i < gNumPlayers; i++)
        if (!PS_PakRumble(gPlayers[i].controller)) allRumblesPresent = 0;
    if (!allRumblesPresent) {
        HAL_SetFont(10);
        x = gPlayerRects[0][1].cx;
        y = gPlayerRects[0][1].cy - TB_GetFontLineHeight() / 2;
        TB_SetTextColor(22);
        str = gText.single[238];
        TB_DrawText(TB_CenterText(str, x), y, str);
    }
    HAL_SetFont(11);
    TB_SetTextColor(1);
    for (i = 0; i < gNumPlayers; i++) {
        x = gPlayerRects[gNumPlayers - 1][i].cx;
        y = gPlayerRects[gNumPlayers - 1][i].cy - TB_GetFontLineHeight();
        TB_SetTextColor(playerTextColor[i]);
        str = (*gPlayers[i].save)->name;
        TB_DrawText(TB_CenterText(str, x), y, str);
        y += TB_GetFontLineHeight();
        str = PS_PakRumble(gPlayers[i].controller) ? gText.single[181] : gText.single[180];
        TB_SetTextColor(1);
        TB_DrawText(TB_CenterText(str, x), y, str);
    }
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x80079ab0, size 0x8c */
static int animRumblePane(_tBlit_2 *blt)
{
    int i;
    int allRumblesPresent = 1;
    for (i = 0; i < gNumPlayers; i++)
        if (!PS_PakRumble(gPlayers[i].controller)) allRumblesPresent = 0;
    Hidden(blt, allRumblesPresent);
    return 1;
}

const KMVERTICES menucontrolpos[4] = {
    32.0f, 84.0f, 0.0f, 0.0f, 0.2f, 487.0f, 84.0f, 0.0f, 1.0f, 0.2f, 32.0f, 288.0f, 0.0f, 0.0f,
    0.86f, 487.0f, 288.0f, 0.0f, 1.0f, 0.86f
};
const KMVERTICES exitpicpos[4] = {
    10.0f, 16.0f, 0.0f, 0.0f, 0.0f, 518.0f, 16.0f, 0.0f, 1.0f, 0.0f, 10.0f, 386.0f, 0.0f, 0.0f,
    1.0f, 518.0f, 386.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES pausefontpos[4] = {
    36.0f, 14.0f, 0.0f, 0.0f, 0.0f, 244.0f, 14.0f, 0.0f, 1.0f, 0.0f, 36.0f, 49.0f, 0.0f, 0.0f,
    1.0f, 244.0f, 49.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES resumefontpos[4] = {
    196.0f, 98.0f, 0.0f, 0.0f, 0.0f, 322.0f, 98.0f, 0.0f, 1.0f, 0.0f, 196.0f, 127.0f, 0.0f, 0.0f,
    1.0f, 322.0f, 127.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES controlfontpos[4] = {
    156.0f, 137.0f, 0.0f, 0.0f, 0.0f, 397.0f, 137.0f, 0.0f, 1.0f, 0.0f, 156.0f, 166.0f, 0.0f, 0.0f,
    1.0f, 397.0f, 166.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES scrsizefontpos[4] = {
    131.0f, 176.0f, 0.0f, 0.0f, 0.0f, 392.0f, 176.0f, 0.0f, 1.0f, 0.0f, 131.0f, 205.0f, 0.0f, 0.0f,
    1.0f, 392.0f, 205.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES quitfontpos[4] = {
    131.0f, 215.0f, 0.0f, 0.0f, 0.0f, 392.0f, 215.0f, 0.0f, 1.0f, 0.0f, 131.0f, 244.0f, 0.0f, 0.0f,
    1.0f, 392.0f, 244.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES exitfontpos[4] = {
    131.0f, 254.0f, 0.0f, 0.0f, 0.0f, 392.0f, 254.0f, 0.0f, 1.0f, 0.0f, 131.0f, 283.0f, 0.0f, 0.0f,
    1.0f, 392.0f, 283.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES selectfontpos[4] = {
    263.0f, 354.0f, 0.0f, 0.0f, 0.0f, 409.0f, 354.0f, 0.0f, 1.0f, 0.0f, 263.0f, 376.0f, 0.0f, 0.0f,
    1.0f, 409.0f, 376.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES backfontpos[4] = {
    405.0f, 354.0f, 0.0f, 0.0f, 0.0f, 485.0f, 354.0f, 0.0f, 1.0f, 0.0f, 405.0f, 376.0f, 0.0f, 0.0f,
    1.0f, 485.0f, 376.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES barpos1[4] = {
    37.0f, 98.0f, 0.0f, 0.0f, 0.0f, 487.0f, 98.0f, 0.0f, 1.0f, 0.0f, 37.0f, 132.0f, 0.0f, 0.0f,
    1.0f, 487.0f, 132.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES barpos2[4] = {
    37.0f, 137.0f, 0.0f, 0.0f, 0.0f, 487.0f, 137.0f, 0.0f, 1.0f, 0.0f, 37.0f, 171.0f, 0.0f, 0.0f,
    1.0f, 487.0f, 171.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES barpos3[4] = {
    37.0f, 178.0f, 0.0f, 0.0f, 0.0f, 487.0f, 178.0f, 0.0f, 1.0f, 0.0f, 37.0f, 212.0f, 0.0f, 0.0f,
    1.0f, 487.0f, 212.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES barpos4[4] = {
    37.0f, 217.0f, 0.0f, 0.0f, 0.0f, 487.0f, 217.0f, 0.0f, 1.0f, 0.0f, 37.0f, 251.0f, 0.0f, 0.0f,
    1.0f, 487.0f, 251.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES barpos5[4] = {
    37.0f, 256.0f, 0.0f, 0.0f, 0.0f, 487.0f, 256.0f, 0.0f, 1.0f, 0.0f, 37.0f, 290.0f, 0.0f, 0.0f,
    1.0f, 487.0f, 290.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES changefontpos[4] = {
    47.0f, 286.0f, 0.0f, 0.0f, 0.0f, 478.0f, 286.0f, 0.0f, 1.0f, 0.0f, 47.0f, 316.0f, 0.0f, 0.0f,
    1.0f, 478.0f, 316.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES screenfontpos[4] = {
    35.0f, 84.0f, 0.0f, 0.0f, 0.0f, 485.0f, 84.0f, 0.0f, 1.0f, 0.0f, 35.0f, 288.0f, 0.0f, 0.0f,
    1.0f, 485.0f, 288.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES areufontpos[4] = {
    67.0f, 90.0f, 0.0f, 0.0f, 0.0f, 323.0f, 90.0f, 0.0f, 1.0f, 0.0f, 67.0f, 122.0f, 0.0f, 0.0f,
    1.0f, 323.0f, 122.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES yesfontpos[4] = {
    123.0f, 164.0f, 0.0f, 0.0f, 0.0f, 187.0f, 164.0f, 0.0f, 1.0f, 0.0f, 123.0f, 196.0f, 0.0f, 0.0f,
    1.0f, 187.0f, 196.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES nofontpos[4] = {
    123.0f, 202.0f, 0.0f, 0.0f, 0.0f, 187.0f, 202.0f, 0.0f, 1.0f, 0.0f, 123.0f, 234.0f, 0.0f, 0.0f,
    1.0f, 187.0f, 234.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES barposyes[4] = {
    37.0f, 175.0f, 0.0f, 0.0f, 0.0f, 487.0f, 175.0f, 0.0f, 1.0f, 0.0f, 37.0f, 209.0f, 0.0f, 0.0f,
    1.0f, 487.0f, 209.0f, 0.0f, 1.0f, 1.0f
};
const KMVERTICES barposno[4] = {
    37.0f, 213.0f, 0.0f, 0.0f, 0.0f, 487.0f, 213.0f, 0.0f, 1.0f, 0.0f, 37.0f, 247.0f, 0.0f, 0.0f,
    1.0f, 487.0f, 247.0f, 0.0f, 1.0f, 1.0f
};

/* 0x80079b3c, size 0x208 */
void updatePauseMenuxb(void)
{
    if (gSelPauseState != 0) {
        MENU_SendBlit((KMVERTICES *)exitpicpos, 1, 1);
        if (gSelPauseState == 3) {
            switch (seletPauseMenuSelect) {
            case 0: MENU_SendBlit((KMVERTICES *)barpos1, 0, 1); break;
            case 1: MENU_SendBlit((KMVERTICES *)barpos2, 0, 1); break;
            case 2: MENU_SendBlit((KMVERTICES *)barpos3, 0, 1); break;
            case 3: MENU_SendBlit((KMVERTICES *)barpos4, 0, 1); break;
            case 4: MENU_SendBlit((KMVERTICES *)barpos5, 0, 1); break;
            }
            ShowPauseMianFont(gGameState & 0x400000);
        } else if (gSelPauseState == 4) {
            if (!ChooseYes[g_PlayerInPause]) MENU_SendBlit((KMVERTICES *)menucontrolpos, 2, 1);
            else MENU_SendBlit((KMVERTICES *)menucontrolpos, 3, 1);
            ShowPauContrlFontAndPic(!ChooseYes[g_PlayerInPause]);
        } else if (gSelPauseState == 5) ShowPauseZoomFont();
        else if (gSelPauseState == 6) {
            switch (gYes) {
            case 0: MENU_SendBlit((KMVERTICES *)barposyes, 0, 1); break;
            case 1: MENU_SendBlit((KMVERTICES *)barposno, 0, 1); break;
            }
            ShowPauseExitRaceFont();
        } else if (gSelPauseState == 7) {
            switch (gYes) {
            case 0: MENU_SendBlit((KMVERTICES *)barposyes, 0, 1); break;
            case 1: MENU_SendBlit((KMVERTICES *)barposno, 0, 1); break;
            }
            ShowPauseExitGameFont();
        }
    }
}

/* 0x80079d44, size 0xac */
int DRAW_TRC_CONTROLLER(void)
{
    if (g_bControllerTrc == 1) {
        MENU_SendBlit((KMVERTICES *)exitpicpos, 1, 1);
        if (!g_bGoToQuitTrc) ShowPauseController();
        else {
            switch (g_bQuitFromTrc) {
            case 0: MENU_SendBlit((KMVERTICES *)barposyes, 0, 1); break;
            case 1: MENU_SendBlit((KMVERTICES *)barposno, 0, 1); break;
            }
            ShowPauseExitGameFont();
        }
        return 1;
    }
    return 0;
}

/* 0x80079df0, size 0xa8 */
int DRAW_TRC_DVD(void)
{
    SetFont(10.0, 36.0, 1.0);
    if (g_bDvdIsError == 1) {
        ngcBlackBackground();
        if (g_bDvdErrorCode == 1) ShowDVDFatal();
        else if (g_bDvdErrorCode == 0x10000000) ShowDVDOpen();
        else if (g_bDvdErrorCode == 0x01000000) ShowDVDNoDisk();
        else if (g_bDvdErrorCode == 0x00100000) ShowDVDWrongDisk();
        else if (g_bDvdErrorCode == 0x00010000) ShowDVDBrokenDisk();
        return 1;
    }
    return 0;
}

