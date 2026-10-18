/* bonus.c - GameCube bonus history/gallery menu. Private reconstruction.
   Original: D:\rush_ngc\Rush_NGC\NGC\bonus.c; assembly: build/2049LE/asm/NGC/bonus.s */
#define _ReleaseTex _ReleaseTex_dwarf_declaration
#include "rush/rush_protos.h"
#undef _ReleaseTex
/* GC target _ReleaseTex reads texDef from r4. Both original callers pass
   the texture name in r3; DWARF omits that unused first parameter. */
void _ReleaseTex(char *name, tTexPair *texDef);

tVertUV _SprBonusMenu[2] = { 25.0f, 25.0f, 0.0f, 0.0f, 615.0f, 455.0f, 1.0f, 1.0f };
tVertUV _SprL[2] = { 82.0f, 30.0f, 0.0f, 0.0f, 102.0f, 70.0f, 1.0f, 1.0f };
tVertUV _SprR[2] = { 542.0f, 30.0f, 0.0f, 0.0f, 562.0f, 70.0f, 1.0f, 1.0f };
tVertUV _SprHydro1[2] = { 90.0f, 140.0f, 0.0f, 0.0f, 230.0f, 320.0f, 1.0f, 1.0f };
tVertUV _SprHydro2[2] = { 240.0f, 140.0f, 0.0f, 0.0f, 380.0f, 320.0f, 1.0f, 1.0f };
tVertUV _SprHydro3[2] = { 390.0f, 140.0f, 0.0f, 0.0f, 530.0f, 320.0f, 1.0f, 1.0f };
tVertUV _SprHydro4[2] = { 540.0f, 140.0f, 0.0f, 0.0f, 680.0f, 320.0f, 1.0f, 1.0f };
tVertUV _SprBigHydro[2] = { 134.0f, 0.0f, 0.0f, 0.0f, 506.0f, 480.0f, 1.0f, 1.0f };
tVertUV _SprWindow1[2] = { 30.0f, 100.0f, 0.0f, 0.0f, 290.0f, 365.0f, 1.0f, 1.0f };
tVertUV _SprWindow2[2] = { 180.0f, 100.0f, 0.0f, 0.0f, 440.0f, 365.0f, 1.0f, 1.0f };
tVertUV _SprWindow3[2] = { 330.0f, 100.0f, 0.0f, 0.0f, 590.0f, 365.0f, 1.0f, 1.0f };
tVertUV _SprWindow4[2] = { 480.0f, 100.0f, 0.0f, 0.0f, 740.0f, 365.0f, 1.0f, 1.0f };
tVertUV _SprArrowL[2] = { 35.0f, 30.0f, 1.0f, 0.0f, 75.0f, 70.0f, 0.0f, 1.0f };
tVertUV _SprArrowR[2] = { 565.0f, 30.0f, 0.0f, 0.0f, 605.0f, 70.0f, 1.0f, 1.0f };
tVertUV _SprHistoryBar[2] = { 555.0f, 130.0f, 0.0f, 0.0f, 575.0f, 230.0f, 1.0f, 1.0f };
tVertUV _SprHistoryUp[2] = { 555.0f, 115.0f, 0.0f, 0.0f, 575.0f, 140.0f, 1.0f, 1.0f };
tVertUV _SprHistoryDown[2] = { 555.0f, 365.0f, 0.0f, 1.0f, 575.0f, 390.0f, 1.0f, 0.0f };
tVertUV _SprGalleryRight[2] = { 515.0f, 205.0f, 0.0f, 1.0f, 565.0f, 255.0f, 1.0f, 0.0f };
tVertUV _SprGalleryLeft[2] = { 55.0f, 205.0f, 1.0f, 0.0f, 105.0f, 255.0f, 0.0f, 1.0f };
float _fHistoryTexY = 90.0f;
int _nBonusTab = 0;
int _nWindowTab = 0;
int _nBonusCursorUsage = 0;
int _nBonusStage = 0;
int _nDirectButtonSignal = 0;
int _nStartMenu = 0;
static int _bBlackBG = 0;
float g_MoveCamX = 0;
float g_MoveCamY = 0;
float g_MoveCamZ = 0;
tTexPair _texWindow;
tTexPair _texHydro4;
tTexPair _texHydro3;
tTexPair _texHydro2;
tTexPair _texHydro1;
tTexPair _texBar;
tTexPair _texArrowRight2;
tTexPair _texArrowRight;
tTexPair _texArrowUp;
tTexPair _texR;
tTexPair _texL;
tTexPair _texVideo;
tTexPair _texGalley;
tTexPair _texHistory;

/* 0x800b2d40, size 0xd0 */
void _LoadTex(char *name, tTexPair *texDef, int mask)
{
    char FileName[1024];
    sprintf(FileName, "rush2049_data/extra/texture/%s.tpl", name);
    NGCTEXGetPalette(&texDef->tpl, FileName);
    enter_cs();
    NGCTEXGetGXTexObjFromPalette(FileName, texDef->tpl, &texDef->to, 0);
    exit_cs();
    if (mask) {
        sprintf(FileName, "rush2049_data/extra/texture/mask/%s_a.tpl", name);
        NGCTEXGetPalette(&texDef->tpl2, FileName);
        enter_cs();
        NGCTEXGetGXTexObjFromPalette(FileName, texDef->tpl2, &texDef->to2, 0);
        exit_cs();
    }
    texDef->usemask = mask;
}

/* 0x800b2e10, size 0x44 */
void _ReleaseTex(char *name, tTexPair *texDef)
{
    NGCTEXReleasePalette(&texDef->tpl);
    if (texDef->usemask) NGCTEXReleasePalette(&texDef->tpl2);
}

/* 0x800b2e54, size 0x11c */
void _LoadBonusTex(void)
{
    _LoadTex("history", &_texHistory, 1);
    _LoadTex("galley", &_texGalley, 1);
    _LoadTex("video", &_texVideo, 1);
    _LoadTex("l", &_texL, 1);
    _LoadTex("r", &_texR, 1);
    _LoadTex("arrow_up", &_texArrowUp, 1);
    _LoadTex("arrow_right", &_texArrowRight, 1);
    _LoadTex("arrow_right2", &_texArrowRight2, 1);
    _LoadTex("bar", &_texBar, 1);
    _LoadTex("Rush2049_1", &_texHydro1, 0);
    _LoadTex("Rush2049_2", &_texHydro2, 0);
    _LoadTex("Rush2049_3", &_texHydro3, 0);
    _LoadTex("Rush2049_4", &_texHydro4, 0);
    _LoadTex("window", &_texWindow, 1);
}

/* 0x800b2f70, size 0xe4 */
void _ReleaseBonusTex(void)
{
    _ReleaseTex("history", &_texHistory);
    _ReleaseTex("galley", &_texGalley);
    _ReleaseTex("video", &_texVideo);
    _ReleaseTex("l", &_texL);
    _ReleaseTex("r", &_texR);
    _ReleaseTex("arrow_up", &_texArrowUp);
    _ReleaseTex("arrow_right", &_texArrowRight);
    _ReleaseTex("arrow_right2", &_texArrowRight2);
    _ReleaseTex("bar", &_texBar);
    _ReleaseTex("Rush2049_1", &_texHydro1);
    _ReleaseTex("Rush2049_2", &_texHydro2);
    _ReleaseTex("Rush2049_3", &_texHydro3);
    _ReleaseTex("Rush2049_4", &_texHydro4);
    _ReleaseTex("window", &_texWindow);
}

/* 0x800b3054, size 0x64 */
void _DrawSprite(tTexPair *texDef, tVertUV *sprite, unsigned long color)
{
    BlitOneTpl_LURDRGBAUV_TEX(sprite[0].x, sprite[0].y, sprite[1].x, sprite[1].y,
        sprite[0].u, sprite[0].v, sprite[1].u, sprite[1].v,
        color, &texDef->to, texDef->usemask ? &texDef->to2 : 0);
}

/* 0x800b30b8, size 0xcc */
void _SetScissor(unsigned long xOrig, unsigned long yOrig, unsigned long wd, unsigned long ht)
{
    GXSetScissor((unsigned long)(float)xOrig, (unsigned long)(float)yOrig,
                 (unsigned long)(float)wd, (unsigned long)(float)ht);
}

/* 0x800b3184, size 0x60 */
void _SetDefualtScissor(void)
{
    float x = 0.0f, w = 640.0f, h = 480.0f;
    GXSetScissor(x, x, w, h);
}

/* 0x800b31e4, size 0x10 */
int _JustButtonDown(unsigned short button)
{
    return just_button_down[0] & button;
}

/* 0x800b31f4, size 0x10 */
int _ButtonPressed(unsigned short button)
{
    return just_button_press[0] & button;
}

/* 0x800b3204, size 0x10 */
int _JustButtonUp(unsigned short button)
{
    return just_button_up[0] & button;
}

/* 0x800b3214, size 0x30 */
float _GetJustStickX(void)
{
    if (stick_left) return 0.0f;
    if (stick_right) return 1.0f;
    return 0.5f;
}

/* 0x800b3244, size 0x30 */
float _GetStickX(void)
{
    if (stick_left_press) return 0.0f;
    if (stick_right_press) return 1.0f;
    return 0.5f;
}

/* 0x800b3274, size 0x30 */
float _GetStickY(void)
{
    if (stick_down_press) return 0.0f;
    if (stick_up_press) return 1.0f;
    return 0.5f;
}

/* 0x800b32a4, size 0x34 */
float _GetSubStickY(void)
{
    return substick_down_press ? -0.5 : substick_up_press ? 0.5 : 0.0;
}

/* 0x800b32d8, size 0x54 */
void _PressGallerayImage(void)
{
    if (_nBonusTab == 1 && _nWindowTab != -1 && _JustButtonDown(0x100)) {
        ngcPlayMenuEffect(0xfd);
        g_bShowGalleray = 1;
    }
}

/* 0x800b332c, size 0x24 */
void _ReturnMainMenuFromBonusMenu(void)
{
    _nBonusStage = 0;
    _nBonusCursorUsage = 0;
    _nStartMenu = 1;
    g_bShowBonus = 0;
    bMenu = 1;
    bSubMenu = 0;
}

/* 0x800b3350, size 0x3a8 */
void _ProcessBonusMenu(void)
{
    if (_nBonusStage == 0) {
        _PressGallerayImage();
        _JustButtonUp(0x400);
        if (_JustButtonUp(0x200)) {
            ngcPlayMenuEffect(0xfc);
            _ReturnMainMenuFromBonusMenu();
        }
        if (_nDirectButtonSignal == 0) {
            if (_JustButtonDown(0x40)) { ngcPlayMenuEffect(0xff); _nBonusTab = 0; }
            if (_JustButtonDown(0x20)) { ngcPlayMenuEffect(0xff); _nBonusTab = 1; _nBonusCursorUsage = 1; }
            if ((_JustButtonDown(1) || _GetJustStickX() > 0.5f) && _nBonusTab == 1) {
                ngcPlayMenuEffect(0xff);
                if (_nWindowTab == 0) _nWindowTab = 0;
                else if (_nWindowTab == 1) _nWindowTab = 0;
                else if (_nWindowTab == 2) _nWindowTab = 1;
                else if (_nWindowTab == 3) _nWindowTab = 2;
            }
            if ((_JustButtonDown(2) || _GetJustStickX() < 0.5f) && _nBonusTab == 1) {
                ngcPlayMenuEffect(0xff);
                if (_nWindowTab == 0) _nWindowTab = 1;
                else if (_nWindowTab == 1) _nWindowTab = 2;
                else if (_nWindowTab == 2) _nWindowTab = 3;
                else if (_nWindowTab == 3) _nWindowTab = 3;
            }
        } else if (_nDirectButtonSignal == 1) {
            if (_JustButtonUp(1)) _nDirectButtonSignal = 0;
        } else if (_nDirectButtonSignal == 2) {
            if (_JustButtonUp(2)) _nDirectButtonSignal = 0;
        } else if (_nDirectButtonSignal == 3) {
            if (_JustButtonUp(8)) _nDirectButtonSignal = 0;
        } else if (_nDirectButtonSignal == 4) {
            if (_JustButtonUp(4)) _nDirectButtonSignal = 0;
        }
        if (_ButtonPressed(4) && _nBonusTab == 0 && _fHistoryTexY > -640.0f) {
            _fHistoryTexY -= 2.0f;
            _SprHistoryBar[0].y += 0.39726027846336365f;
            _SprHistoryBar[1].y += 0.39726027846336365f;
        }
        if (_ButtonPressed(8) && _nBonusTab == 0 && _fHistoryTexY < 90.0f) {
            _fHistoryTexY += 2.0f;
            _SprHistoryBar[0].y -= 0.39726027846336365f;
            _SprHistoryBar[1].y -= 0.39726027846336365f;
        }
        if (_GetStickY() > 0.5f && _nBonusTab == 0 && _fHistoryTexY < 90.0f) {
            _fHistoryTexY += 2.0f;
            _SprHistoryBar[0].y -= 0.39726027846336365f;
            _SprHistoryBar[1].y -= 0.39726027846336365f;
        }
        if (_GetStickY() < 0.5f && _nBonusTab == 0 && _fHistoryTexY > -640.0f) {
            _fHistoryTexY -= 2.0f;
            _SprHistoryBar[0].y += 0.39726027846336365f;
            _SprHistoryBar[1].y += 0.39726027846336365f;
        }
    }
}

/* 0x800b36f8, size 0xfc */
void _DrawBounsHistory(void)
{
    _DrawSprite(&_texHistory, _SprBonusMenu, 0xffffffff);
    _DrawSprite(&_texBar, _SprHistoryBar, 0xffffffff);
    _DrawSprite(&_texArrowUp, _SprHistoryUp, 0xffffffff);
    _DrawSprite(&_texArrowUp, _SprHistoryDown, 0xffffffff);
    _SetScissor(70, 82, 472, 280);
    SetFont(10.0f, 36.0f, 0.9f);
    SetPrintWidth(430);
    PrintFont(82, (int)_fHistoryTexY, "SAN FRANCISCO RUSH:  2049\012Year: 1999\012Genre:  Racing\012Gameplay:  Linked\012Number of Simultaneous Players:  1\012Maximum number of Players:  2 Linked\012Manufacturer:  Atari Games\012Hardware:  Denver\012Sound: Stereo\012Cabinet Design:  Sit-down\012Control Panel Layout:  Single Player\012Controls:  Steering Wheel, shifter, 3 pedals, 5 buttons\012San Francisco Rush 2049 was the third and final game in the series by Atari Games. This game returns to take the driver into the year 2049.  While the city looks familiar there have definitely been changes to the city's skyline over the years.  The effects of the second gold rush are ever present in the city by the bay, 200 years after the original gold rush of 1849. Through the process of excavating the gold from around and under the city, underground tunnels lace across the city. For the adventuring driver, coins can be found in some of the more remote locations by use of shortcuts and jumps.", 1);
    SetPrintWidth(640);
    SetFont(10.0f, 36.0f, 1.0f);
    _SetDefualtScissor();
    PrintFont(420, 395, "\201 BACK", 1);
}

/* 0x800b37f4, size 0x204 */
void _DrawBonusGallery(void)
{
    _DrawSprite(&_texGalley, _SprBonusMenu, 0xffffffff);
    g_MoveCamX = 0.0f;
    g_MoveCamY = 0.0f;
    g_MoveCamZ = 0.0f;
    if (_nWindowTab == 2 || _nWindowTab == 3) g_MoveCamX = 151.0f;
    _SetScissor(90, 82, 450, 280);
    _DrawSprite(&_texHydro1, _SprHydro1, 0xffffffff);
    _DrawSprite(&_texHydro2, _SprHydro2, 0xffffffff);
    _DrawSprite(&_texHydro3, _SprHydro3, 0xffffffff);
    _DrawSprite(&_texHydro4, _SprHydro4, 0xffffffff);
    _SetDefualtScissor();
    if (_nWindowTab == 0) _DrawSprite(&_texWindow, _SprWindow1, 0xffffffff);
    else if (_nWindowTab == 1) _DrawSprite(&_texWindow, _SprWindow2, 0xffffffff);
    else if (_nWindowTab == 2) _DrawSprite(&_texWindow, _SprWindow3, 0xffffffff);
    else if (_nWindowTab == 3) _DrawSprite(&_texWindow, _SprWindow4, 0xffffffff);
    g_MoveCamX = 0.0f;
    g_MoveCamY = 0.0f;
    g_MoveCamZ = 0.0f;
    if (_nWindowTab < 3) _DrawSprite(&_texArrowRight, _SprGalleryRight, 0xffffffff);
    if (_nWindowTab > 0) _DrawSprite(&_texArrowRight, _SprGalleryLeft, 0xffffffff);
    PrintFont(300, 350, _nWindowTab == 0 ? "1/4" : _nWindowTab == 1 ? "2/4" : _nWindowTab == 2 ? "3/4" : "4/4", 1);
    SetFont(2.0f, 30.0f, 0.95f);
    PrintFont(350, 397, "\200   VI EW        \201     BACK", 1);
    SetFont(10.0f, 30.0f, 1.0f);
}

/* 0x800b39f8, size 0x4 */
void _DrawBosnuVideo(void)
{

}

/* 0x800b39fc, size 0x108 */
void _DrawBonusMenu(void)
{
    if (_nBonusStage == 0) {
        _DrawSprite(&_texArrowRight2, _SprArrowL, 0xffffffff);
        _DrawSprite(&_texArrowRight2, _SprArrowR, 0xffffffff);
        _DrawSprite(&_texL, _SprL, 0xffffffff);
        _DrawSprite(&_texR, _SprR, 0xffffffff);
        if (_nBonusTab == 0) _DrawBounsHistory();
        else if (_nBonusTab == 1) _DrawBonusGallery();
        else if (_nBonusTab == 2) _DrawBosnuVideo();
        SetFont(2.0f, 30.0f, 0.95f);
        PrintFont(160, 30, "I NFO", 1);
        SetFont(10.0f, 30.0f, 0.95f);
        PrintFont(289, 30, "GALLERY", 1);
    } else {
        _DrawGalleryImage();
    }
}

/* 0x800b3b04, size 0x6ac */
void _ProcessGalleryImage(void)
{
    float s;
    float width;
    float height;
    float l;
    float r;
    float t;
    float b;
    static float cx = 0;
    static float cy = 0;

    _JustButtonDown(0x100);
    if (_JustButtonUp(0x200)) {
        cx = 0.0f;
        cy = 0.0f;
        _SprBigHydro[0].x = 134.0f;
        _SprBigHydro[1].x = 506.0f;
        _SprBigHydro[0].y = 0.0f;
        _SprBigHydro[1].y = 480.0f;
        ngcPlayMenuEffect(0xfc);
        _nBonusStage = 0;
        _nBonusCursorUsage = 1;
        _nStartMenu = 2;
        _bBlackBG = 0;
        g_bShowGalleray = 0;
    }
    l = _SprBigHydro[0].x;
    r = _SprBigHydro[1].x;
    t = _SprBigHydro[0].y;
    b = _SprBigHydro[1].y;
    width = r - l;
    height = b - t;
    if (_GetSubStickY() > 0.0f) {
        if (cx < 1440.0f) {
            s = 5.0f;
            if (_ButtonPressed(0x20)) s *= width / 372.0f;
            cx += s;
            cy += s;
            _SprBigHydro[0].x -= s;
            _SprBigHydro[1].x += s;
            _SprBigHydro[0].y -= s * 1.2929;
            _SprBigHydro[1].y += s * 1.2929;
        }
    } else if (_GetSubStickY() < 0.0f) {
        if (cx > -60.0f) {
            s = 5.0f;
        s = -s;
            if (_ButtonPressed(0x20)) s *= width / 372.0f;
            cx += s;
            cy += s;
            _SprBigHydro[0].x -= s;
            _SprBigHydro[1].x += s;
            _SprBigHydro[0].y -= s * 1.2929;
            _SprBigHydro[1].y += s * 1.2929;
        }
    }
    if (_GetStickX() < 0.5f) {
        s = 5.0f;
        if (_ButtonPressed(0x20)) s *= width / 372.0f;
        if (width < 640.0f) {
            if (r < 640.0f) {
            float xl, xr;
            xl = _SprBigHydro[0].x + s;
            xr = _SprBigHydro[1].x + s;
            _SprBigHydro[1].x = xr;
            _SprBigHydro[0].x = xl;
            }
        } else {
            if (l < 0.0f) {
            float xl, xr;
            xl = _SprBigHydro[0].x + s;
            xr = _SprBigHydro[1].x + s;
            _SprBigHydro[1].x = xr;
            _SprBigHydro[0].x = xl;
            }
        }
    }
    if (_GetStickX() > 0.5f) {
        s = 5.0f;
        s = -s;
        if (_ButtonPressed(0x20)) s *= width / 372.0f;
        if (width < 640.0f) {
            if (l > 0.0f) {
            float xl, xr;
            xl = _SprBigHydro[0].x + s;
            xr = _SprBigHydro[1].x + s;
            _SprBigHydro[1].x = xr;
            _SprBigHydro[0].x = xl;
            }
        } else {
            if (r > 640.0f) {
            float xl, xr;
            xl = _SprBigHydro[0].x + s;
            xr = _SprBigHydro[1].x + s;
            _SprBigHydro[1].x = xr;
            _SprBigHydro[0].x = xl;
            }
        }
    }
    if (_GetStickY() > 0.5f) {
        s = 5.0f;
        s = -s;
        if (just_button_down[0] & 0x20) s *= height / 480.0f;
        if (height < 480.0f) {
            if (t > 0.0f) {
            _SprBigHydro[0].y += s;
            _SprBigHydro[1].y += s;
            }
        } else {
            if (b > 480.0f) {
            _SprBigHydro[0].y += s;
            _SprBigHydro[1].y += s;
            }
        }
    }
    if (_GetStickY() < 0.5f) {
        s = 5.0f;
        if (just_button_down[0] & 0x20) s *= height / 480.0f;
        if (height < 480.0f) {
            if (b < 480.0f) {
            _SprBigHydro[0].y += s;
            _SprBigHydro[1].y += s;
            }
        } else {
            if (t < 0.0f) {
            _SprBigHydro[0].y += s;
            _SprBigHydro[1].y += s;
            }
        }
    }
    if (_ButtonPressed(8)) {
        s = 5.0f;
        s = -s;
        if (_ButtonPressed(0x20)) s *= height / 480.0f;
        if (height < 480.0f) {
            if (t > 0.0f) {
            _SprBigHydro[0].y += s;
            _SprBigHydro[1].y += s;
            }
        } else {
            if (b > 480.0f) {
            _SprBigHydro[0].y += s;
            _SprBigHydro[1].y += s;
            }
        }
    }
    if (_ButtonPressed(4)) {
        s = 5.0f;
        if (_ButtonPressed(0x20)) s *= height / 480.0f;
        if (height < 480.0f) {
            if (b < 480.0f) {
            _SprBigHydro[0].y += s;
            _SprBigHydro[1].y += s;
            }
        } else {
            if (t < 0.0f) {
            _SprBigHydro[0].y += s;
            _SprBigHydro[1].y += s;
            }
        }
    }
    if (_ButtonPressed(1)) {
        s = 5.0f;
        s = -s;
        if (_ButtonPressed(0x20)) s *= width / 372.0f;
        if (width < 640.0f) {
            if (l > 0.0f) {
            float xl, xr;
            xl = _SprBigHydro[0].x + s;
            xr = _SprBigHydro[1].x + s;
            _SprBigHydro[1].x = xr;
            _SprBigHydro[0].x = xl;
            }
        } else {
            if (r > 640.0f) {
            float xl, xr;
            xl = _SprBigHydro[0].x + s;
            xr = _SprBigHydro[1].x + s;
            _SprBigHydro[1].x = xr;
            _SprBigHydro[0].x = xl;
            }
        }
    }
    if (_ButtonPressed(2)) {
        s = 5.0f;
        if (_ButtonPressed(0x20)) s *= width / 372.0f;
        if (width < 640.0f) {
            if (r < 640.0f) {
            float xl, xr;
            xl = _SprBigHydro[0].x + s;
            xr = _SprBigHydro[1].x + s;
            _SprBigHydro[1].x = xr;
            _SprBigHydro[0].x = xl;
            }
        } else {
            if (l < 0.0f) {
            float xl, xr;
            xl = _SprBigHydro[0].x + s;
            xr = _SprBigHydro[1].x + s;
            _SprBigHydro[1].x = xr;
            _SprBigHydro[0].x = xl;
            }
        }
    }
}

/* 0x800b41b0, size 0x124 */
void _DrawGalleryImage(void)
{
    if (_nWindowTab == 0) _DrawSprite(&_texHydro1, _SprBigHydro, 0xffffffff);
    else if (_nWindowTab == 1) _DrawSprite(&_texHydro2, _SprBigHydro, 0xffffffff);
    else if (_nWindowTab == 2) _DrawSprite(&_texHydro3, _SprBigHydro, 0xffffffff);
    else if (_nWindowTab == 3) _DrawSprite(&_texHydro4, _SprBigHydro, 0xffffffff);
    XBX_DrawPausePic2(50.0f, 460.0f, 590.0f, 350.0f, 16);
    SetFont(2.0f, 30.0f, 1.0f);
    PrintFont(115, 383, "\212      MOVE", 1);
    PrintFont(270, 383, "ZOOM     I N/OUT     \201      BACK", 1);
    SetFont(10.0f, 30.0f, 1.0f);
    XBX_DrawPausePic2(234.0f, 416.0f, 260.0f, 390.0f, 17);
}

