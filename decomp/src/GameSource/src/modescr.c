/* modescr.c - race mode menu and animated 3D buttons. Status: in progress.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\modescr.c; assembly: build/2049LE/asm/GameSource/src/modescr.s */
#include "rush/rush_protos.h"
#include "msl_math_inline.h"

static void initScreen(void);
static void uninitScreen(void);
static int animText(void);
static signed char optionValid(enum_kModeMenuCount opt);
static void initButtons(void);
static void uninitButtons(void);
static void updateButtons(void);

static char *buttonNames[8] = {
    "BUTTON_S2G1", "BUTTON_S2G1", "CURSORG1", "AB_BUTTONG1", "ARROW_BGG1", "ARROW_FGG1",
    "BAR_BGG1", "BAR_FGG1"
};
static const anon_texname_dulx_24 mblit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animText, 0 };
static anon_buttonType_objIdx_40 buttonInfo[13] = { {0,-1}, {0,-1}, {0,-1}, {0,-1}, {0,-1}, {0,-1}, {0,-1}, {2,-1}, {3,-1}, {5,-1}, {3,-1}, {5,-1}, {1,-1} };
static signed char validOption[4][7] = {
    1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1
};
static float gButtonScale = 1.0f;
static float gButtonSpace = 40.0f;
static float maxY = 100.0f;
static float buttonZ = 300.0f;
static float cursorZ = 225.0f;
static float cursorSpeed;
static enum_kNumMenuDir_6 exitModeScreen;
static enum_kModeMenuCount gModeMenuSelect;
static _tBlit_2 *blit;
static signed char inited;
static float buttonX;
static int buttonsJustInited;
static int buttonsInited;

/* 0x80073a38, size 0x2b8 */
void updateModeScreen(void)
{
    int i; /* r30 */

    if(gSkipToGameState != 1) {
        if(gSkipToGameState != gGameState) {
            if(gSkipToGameState == 0x4000) setGameState(0x4000);
            else setGameState(0x80);
            return;
        }
        gSkipToGameState=1;
    }
    if(!inited) initScreen();
    while(!optionValid(gModeMenuSelect)) {
        if(++gModeMenuSelect >= 7) gModeMenuSelect=0;
    }
    updateButtons();
    if(gActiveEdges & 3) { PlayStartABSound(gActiveEdges); exitModeScreen=1; }
    else if(gActiveEdges & 4) { PlayStartABSound(gActiveEdges); exitModeScreen=2; }
    else if(gActiveEdges & 0x400) {
        PlayUDSound(gActiveEdges);
        do { if(--gModeMenuSelect < 0) gModeMenuSelect=6; } while(!optionValid(gModeMenuSelect));
    } else if(gActiveEdges & 0x800) {
        PlayUDSound(gActiveEdges);
        do { if(++gModeMenuSelect >= 7) gModeMenuSelect=0; } while(!optionValid(gModeMenuSelect));
    }
    for(i=0;i<gNumPlayers;i++) {
        tPS_File **fileh=(*gPlayers[i].save)->fileh;
        if(fileh && PS_PakChanged((*fileh)->pak)) {
            uninitScreen(); setGameState(0x20); return;
        }
    }
    if(gSoftReset) { uninitScreen(); setGameState(4); return; }
    if(exitModeScreen) {
        uninitScreen();
        switch(exitModeScreen) {
        case 1:
            switch(gModeMenuSelect) {
            case 0: gRace=0; setGameState(0x80); break;
            case 1: gRace=1; setGameState(0x80); break;
            case 2: gRace=2; setGameState(0x80); break;
            case 3: gRace=3; setGameState(0x4000); break;
            case 4: gRace=4; setGameState(0x80); break;
            case 5: gRace=5; setGameState(0x80); break;
            case 6: gRace=6; setGameState(0x80); break;
            } break;
        case 2: setGameState(0x20); break;
        }
        exitModeScreen=0;
    }
}

/* 0x80073cf0, size 0xf8 */
static void initScreen(void)
{
    SaveFlush(0); createBG(); initButtons();
    blit=(_tBlit_2 *)NewMultiBlit(0,0,mblit,1); exitModeScreen=0;
    ClearRushControls();
    gModeMenuSelect=GetPlayerRaceType(gPlayers[0].save);
    switch(gModeMenuSelect) {
    case 1: gModeMenuSelect=1; break;
    case 2: gModeMenuSelect=2; break;
    case 3: gModeMenuSelect=3; break;
    case 4: gModeMenuSelect=4; break;
    case 5: gModeMenuSelect=5; break;
    case 6: gModeMenuSelect=6; break;
    default: gModeMenuSelect=0; break;
    }
    inited=1;
}

/* 0x80073de8, size 0x44 */
static void uninitScreen(void)
{
    uninitButtons();
    if(blit) { RemoveBlit((_tBlit *)blit); blit=0; }
    removeBG(); inited=0;
}

/* 0x80073e2c, size 0x34c */
static int animText(void)
{
    int i; /* r28 */
    signed short x; /* r27 */
    signed short y; /* r26 */
    unsigned char *str; /* r29 */
    unsigned char str1[40]; /* stack */
    signed short screenPos[2]; /* stack */

    HAL_SetTextDepth(0.0f); HAL_SetFont(13); TB_SetTextColor(1);
    x=160; y=10;
    if(buttonsInited == 1) {
        TB_SetJust(0,1);
        ZOID_ProjectWindow(0,buttonInfo[12].mat.uvs[3],&gCamera[0].mat,0,screenPos);
        x=screenPos[0]/2; y=screenPos[1]/2.0f;
    }
    str=gText.single[40];
    TB_DrawText(TB_CenterText(str,x),y,str);
    y+=TB_GetFontLineHeight()*2;
    if(gNumPlayers == 1) TB_Sprintf(str1,"%d %s",gNumPlayers,gText.single[29]);
    else TB_Sprintf(str1,"%d %s",gNumPlayers,gText.taba[gText.tab[21]]);
    TB_SetTextColor(10); y+=TB_GetFontLineHeight()*2; HAL_SetFont(10);
    for(i=0;i<7;i++) {
        if(!optionValid(i)) continue;
        TB_SetTextColor(i == gModeMenuSelect ? 22 : 1);
        if(buttonsInited == 1) {
            if(buttonInfo[i].angle>0.6981317f && buttonInfo[i].angle<2.443461f) continue;
            if(buttonInfo[i].angle>0.34906584f && buttonInfo[i].angle<2.7925267f) HAL_SetTextTranslucency(128.0f);
            else HAL_SetTextTranslucency(-1.0f);
            if(buttonInfo[i].angle<1.5707964f) {
                anon_c_rgba_4 c1,c2; c2=flashColors[1]; c1=flashColors[0];
                TB_FlashTextColors(&c1,&c2); TB_SetTextColor(1);
            } else {
                anon_c_rgba_4 c1,c2; c2=flashColors[1]; c1=flashColors[0];
                TB_FlashTextColors(&c1,&c2); TB_SetTextColor(22);
            }
            ZOID_ProjectWindow(0,buttonInfo[i].mat.uvs[3],&gCamera[0].mat,0,screenPos);
            x=screenPos[0]/2-45; y=screenPos[1]/2.0f;
        }
        TB_DrawText(x,y,gText.taba[gText.tab[23]+i]);
        y+=TB_GetFontLineHeight();
    }
    TB_SetJust(0,3); HAL_SetTextTranslucency(-1.0f);
    { anon_c_rgba_4 c1,c2; c2=flashColors[1]; c1=flashColors[0]; TB_FlashTextColors(&c1,&c2); }
    HAL_SetTextDepth(-1.0f); return 1;
}

/* 0x80074178, size 0x60 */
static signed char optionValid(enum_kModeMenuCount opt)
{
    signed char valid; /* r31 */

    valid=validOption[gNumPlayers-1][opt];
    if(opt == 5 && !IsTrackUnlocked(18)) valid=0;
    return valid;
}

/* 0x800741d8, size 0x3a4 */
static void initButtons(void)
{
    anon_c_rgba_4 buttonColor = {255,255,255,255}; /* stack */
    anon_name_width_30 *tex; /* r3 */
    unsigned short texIdx; /* stack */
    int buttonIdx; /* r30 */

    for(buttonIdx=0;buttonIdx<13;buttonIdx++) {
        if(buttonInfo[buttonIdx].objIdx == -1) {
            memcpy(buttonInfo[buttonIdx].mat.uvs,unitmatrix,36);
            buttonInfo[buttonIdx].objIdx=MBOX_NewObject(MBOX_FindObject_WORK(buttonNames[buttonInfo[buttonIdx].buttonType],0,gNumModels-1,1),&buttonInfo[buttonIdx].mat,-1,(buttonInfo[buttonIdx].buttonType == 0 || buttonInfo[buttonIdx].buttonType == 1) ? 0x42000 : 0);
            ZOID_SetColor(buttonInfo[buttonIdx].objIdx,&buttonColor);
        }
    }
    tex=MBOX_FindTexture_WORK("CURSOR",&texIdx,0,gNumModels-1,1);
    ZOID_SetTexture(buttonInfo[7].objIdx,tex,-1);
    tex=MBOX_FindTexture_WORK("A1",&texIdx,0,gNumModels-1,1);
    ZOID_SetTexture(buttonInfo[8].objIdx,tex,-1);
    memcpy(buttonInfo[8].mat.uvs,unitmatrix,36);
    WPitchUV(-1.5707964f,buttonInfo[8].mat.uvs);
    buttonInfo[8].mat.uvs[3][0]=75.1f; buttonInfo[8].mat.uvs[3][1]=59.2f; buttonInfo[8].mat.uvs[3][2]=100.0f;
    ScaleUV(buttonInfo[8].mat.uvs,buttonInfo[8].mat.uvs,0.5f);
    memcpy(buttonInfo[9].mat.uvs,unitmatrix,36);
    WPitchUV(-1.5707964f,buttonInfo[9].mat.uvs); WRollUV(3.1415927f,buttonInfo[9].mat.uvs);
    buttonInfo[9].mat.uvs[3][0]=87.1f; buttonInfo[9].mat.uvs[3][1]=59.2f; buttonInfo[9].mat.uvs[3][2]=100.0f;
    tex=MBOX_FindTexture_WORK("B1",&texIdx,0,gNumModels-1,1);
    ZOID_SetTexture(buttonInfo[10].objIdx,tex,-1);
    memcpy(buttonInfo[10].mat.uvs,unitmatrix,36);
    WPitchUV(-1.5707964f,buttonInfo[10].mat.uvs);
    buttonInfo[10].mat.uvs[3][0]=-75.1f; buttonInfo[10].mat.uvs[3][1]=59.2f; buttonInfo[10].mat.uvs[3][2]=100.0f;
    ScaleUV(buttonInfo[10].mat.uvs,buttonInfo[10].mat.uvs,0.5f);
    memcpy(buttonInfo[11].mat.uvs,unitmatrix,36);
    WPitchUV(-1.5707964f,buttonInfo[11].mat.uvs);
    buttonInfo[11].mat.uvs[3][0]=-87.1f; buttonInfo[11].mat.uvs[3][1]=59.2f; buttonInfo[11].mat.uvs[3][2]=100.0f;
    tex=MBOX_FindTexture_WORK("BUTTON",&texIdx,0,gNumModels-1,1);
    ZOID_SetTexture(buttonInfo[12].objIdx,tex,-1);
    memcpy(buttonInfo[12].mat.uvs,unitmatrix,36);
    WPitchUV(-1.5707964f,buttonInfo[12].mat.uvs);
    buttonInfo[12].mat.uvs[3][0]=0.0f; buttonInfo[12].mat.uvs[3][1]=115.0f; buttonInfo[12].mat.uvs[3][2]=200.0f;
    buttonsInited=1; buttonsJustInited=1; ZeroCameras();
}

/* 0x8007457c, size 0x74 */
static void uninitButtons(void)
{
    int buttonIdx; /* r29 */

    for(buttonIdx=0;buttonIdx<13;buttonIdx++) {
        if(buttonInfo[buttonIdx].objIdx != -1) {
            ZOID_RemoveObject(buttonInfo[buttonIdx].objIdx);
            buttonInfo[buttonIdx].objIdx=-1;
        }
    }
    buttonsInited=0;
}

/* 0x800745f0, size 0x414 */
static void updateButtons(void)
{
    int buttonIdx; /* r30 */
    float prevPos[3]; /* stack */
    float buttonY; /* f31 */
    float selectY; /* f30 */
    anon_name_width_30 *tex; /* r0 */
    unsigned short texIdx; /* stack */

    if(!buttonsInited) return;
    buttonY=maxY;
    for(buttonIdx=0;buttonIdx<7;buttonIdx++) {
        if(buttonIdx == gModeMenuSelect) {
            selectY=buttonY;
            if(buttonInfo[buttonIdx].angle<3.1415927f) buttonInfo[buttonIdx].angle+=12.566371f*gFrameSeconds;
            if(buttonInfo[buttonIdx].angle>3.1415927f || buttonsJustInited == 1) buttonInfo[buttonIdx].angle=3.1415927f;
        } else {
            if(buttonInfo[buttonIdx].angle>0.17453292f) buttonInfo[buttonIdx].angle-=12.566371f*gFrameSeconds;
            if(buttonInfo[buttonIdx].angle<0.17453292f || buttonsJustInited == 1) buttonInfo[buttonIdx].angle=0.17453292f;
        }
        memcpy(buttonInfo[buttonIdx].mat.uvs,unitmatrix,36);
        memcpy(prevPos,buttonInfo[buttonIdx].mat.uvs[3],12);
        buttonInfo[buttonIdx].mat.uvs[3][0]=buttonX;
        buttonInfo[buttonIdx].mat.uvs[3][1]=buttonY;
        buttonInfo[buttonIdx].mat.uvs[3][2]=buttonZ;
        if(!buttonsJustInited) {
            if(buttonInfo[buttonIdx].mat.uvs[3][1]>prevPos[1]+180.0f*gFrameSeconds) buttonInfo[buttonIdx].mat.uvs[3][1]=prevPos[1]+180.0f*gFrameSeconds;
            if(buttonInfo[buttonIdx].mat.uvs[3][1]<prevPos[1]-180.0f*gFrameSeconds) buttonInfo[buttonIdx].mat.uvs[3][1]=prevPos[1]-180.0f*gFrameSeconds;
        }
        WPitchUV(buttonInfo[buttonIdx].angle-1.5707964f,buttonInfo[buttonIdx].mat.uvs);
        if(buttonInfo[buttonIdx].angle>1.5707964f) tex=MBOX_FindTexture_WORK("BUTTON_SELECT",&texIdx,0,gNumModels-1,1);
        else tex=MBOX_FindTexture_WORK("BUTTON",&texIdx,0,gNumModels-1,1);
        ZOID_SetTexture(buttonInfo[buttonIdx].objIdx,tex,-1);
        if(!optionValid(buttonIdx)) MBOX_HideObject(buttonInfo[buttonIdx].objIdx,0,15);
        else { MBOX_ShowObject(buttonInfo[buttonIdx].objIdx,0,15); buttonY-=gButtonSpace*gButtonScale; }
    }
    memcpy(prevPos,buttonInfo[7].mat.uvs[3],12);
    buttonInfo[7].angle+=6.2831855f*gFrameSeconds;
    if(buttonInfo[7].angle>6.2831855f) buttonInfo[7].angle-=6.2831855f;
    CopyUV(unitmatrix,buttonInfo[7].mat.uvs);
    buttonInfo[7].mat.uvs[3][2]=cursorZ;
    buttonInfo[7].mat.uvs[3][0]=buttonX-80.0f;
    buttonInfo[7].mat.uvs[3][1]=selectY*cursorZ/buttonZ;
    if(!buttonsJustInited) {
        if(60.0*fabs(buttonInfo[7].mat.uvs[3][1]-prevPos[1])/10.0>cursorSpeed) {
            cursorSpeed=60.0*fabs(buttonInfo[7].mat.uvs[3][1]-prevPos[1])/10.0;
            cursorSpeed=cursorSpeed<540.0f*gButtonScale ? cursorSpeed : 540.0f*gButtonScale;
            cursorSpeed=cursorSpeed>180.0f*gButtonScale ? cursorSpeed : 180.0f*gButtonScale;
        }
        if(buttonInfo[7].mat.uvs[3][1]>prevPos[1]+cursorSpeed*gFrameSeconds) buttonInfo[7].mat.uvs[3][1]=prevPos[1]+cursorSpeed*gFrameSeconds;
        else if(buttonInfo[7].mat.uvs[3][1]<prevPos[1]-cursorSpeed*gFrameSeconds) buttonInfo[7].mat.uvs[3][1]=prevPos[1]-cursorSpeed*gFrameSeconds;
        else cursorSpeed=180.0f*gButtonScale;
    }
    WRollUV(buttonInfo[7].angle,buttonInfo[7].mat.uvs);
    WPitchUV(1.5707964f,buttonInfo[7].mat.uvs);
    ScaleUV(buttonInfo[7].mat.uvs,buttonInfo[7].mat.uvs,0.4f);
    buttonsJustInited=0;
}

