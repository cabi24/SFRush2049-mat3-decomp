/* HUD overlays, text and object animations. Status: Matching (13/13 functions).
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\hudextra.c; assembly: build/2049LE/asm/GameSource/src/hudextra.s */
#include "rush/rush_protos.h"

static int animStuntPoints();
static int animStuntCoins();
static int animHealth(_tBlit_2 *blt);
static int animAmmoCounts();
static int animWeaponIcons();
static int animPowerupIcons();
static int animBattleCoins();
static void ClearBattlePoints(void);
static int animBattlePoints();

static char *stuntCoinTexNames[10] = {
    "STNT_FLIP_B", "STNT_FLIP_F", "STNT_ROLL_L", "STNT_ROLL_R", "STNT_HELI", "STNT_MULTI",
    "STNT_2WHL_S", "STNT_2WHL_B", "STNT_2WHL_F", "STNT_AIR"
};
static char *powerupIconNames[3] = { 0, "WEPICON_INVSG1", "WEPICON_SHLDG1" };

static const anon_texname_dulx_24 StuntMultiBlit[17] = {
    "SPEEDBG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff, (void *)&animSpeed, 0x10, "SPEEDBG2", 0, 0,
    -1, -1, -1, -1, -1, -1, 0, 0xff, (void *)&animSpeed, 0x20, "SPEEDBG2", 0, 0, -1, -1, -1, -1,
    -1, -1, 0, 0xff, (void *)&animSpeed, 0x21, "SPEEDBG2", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animSpeed, 0x22, "SPEEDBG2", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animSpeed, 0x23, (void *)0xffffffff, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0,
    (void *)&animSpeedText, 0, "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animTach, 0, "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animTach, 1, "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animTach, 0x10, "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animTach, 0x11, "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animTach, 0x20, "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animTach, 0x21, "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animTach, 0x30, "TACHOMETER_LG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animTach, 0x31, (void *)0xffffffff, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0,
    (void *)&animGear, 0, (void *)0xffffffff, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0,
    (void *)&animCountdownTimer, 0, (void *)0xffffffff, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0,
    (void *)&animStuntPoints, 0
};
static const anon_texname_dulx_24 BattleMultiBlit[10] = {
    "HEALTHBG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff, (void *)&animHealth, 0, "HEALTHBAR", 0, 0,
    -1, -1, -1, -1, -1, -1, 0, 0xff, (void *)&animHealth, 1, "HEALTHBG", 0, 0, -1, -1, -1, -1, -1,
    -1, 0, 0xff, (void *)&animHealth, 0x10, "HEALTHBAR", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animHealth, 0x11, "HEALTHBG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animHealth, 0x20, "HEALTHBAR", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animHealth, 0x21, "HEALTHBG", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animHealth, 0x30, "HEALTHBAR", 0, 0, -1, -1, -1, -1, -1, -1, 0, 0xff,
    (void *)&animHealth, 0x31, (void *)0xffffffff, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0,
    (void *)&animAmmoCounts, 0, (void *)0xffffffff, 0, 0, -1, -1, -1, -1, -1, -1, 0, 0,
    (void *)&animBattlePoints, 0
};
static int whichCam[4] = { 1, 2, 4, 8 };
static int stuntPos[4][4][2] = {
    160, 212, 0, 0, 0, 0, 0, 0, 120, 100, 120, 212, 0, 0, 0, 0, 80, 100, 240, 100, 80, 212, 0, 0,
    80, 100, 240, 100, 80, 212, 240, 212
};
static int stuntCoinPos[4][4][2] = {
    264, 212, 0, 0, 0, 0, 0, 0, 208, 100, 208, 212, 0, 0, 0, 0, 128, 100, 172, 100, 128, 212, 0, 0,
    128, 100, 172, 100, 128, 212, 172, 212
};
int coinStuntPos[4][4][2] = {
    16, 40, 0, 0, 0, 0, 0, 0, 16, 40, 16, 152, 0, 0, 0, 0, 16, 40, 284, 40, 16, 152, 0, 0, 16, 40,
    284, 40, 16, 152, 284, 152
};
static int fragPos[4][4][2] = {
    280, 12, 0, 0, 0, 0, 0, 0, 224, 100, 224, 124, 0, 0, 0, 0, 144, 100, 176, 100, 144, 124, 0, 0,
    144, 100, 176, 100, 144, 124, 176, 124
};
static int healthPos[4][4][2] = {
    160, 212, 0, 0, 0, 0, 0, 0, 120, 100, 120, 212, 0, 0, 0, 0, 80, 100, 240, 100, 80, 212, 0, 0,
    80, 100, 240, 100, 80, 212, 240, 212
};
static signed short ammoPos[4][4][2] = {
    32, 212, 0, 0, 0, 0, 0, 0, 32, 100, 32, 212, 0, 0, 0, 0, 40, 100, 280, 100, 40, 212, 0, 0, 40,
    100, 280, 100, 40, 212, 280, 212
};
static signed short powerupPos[4][4][2] = {
    256, 222, 0, 0, 0, 0, 0, 0, 176, 110, 176, 222, 0, 0, 0, 0, 120, 110, 200, 110, 120, 222, 0, 0,
    120, 110, 200, 110, 120, 222, 200, 222
};
static unsigned char playerPaneColors[4][2][4] = {
    0, 0, 240, 63, 0, 0, 240, 127, 240, 0, 0, 63, 240, 0, 0, 127, 240, 240, 0, 63, 240, 240, 0,
    127, 0, 240, 0, 63, 0, 240, 0, 127
};
static unsigned char playerColor[4][4] = { 64, 128, 224, 63, 224, 64, 64, 63, 224, 224, 64, 63, 64, 224, 64, 63 };
static unsigned char healthColor[4][2][4] = {
    0, 224, 0, 255, 0, 0, 0, 0, 0, 224, 0, 255, 0, 0, 0, 0, 0, 224, 0, 255, 0, 0, 0, 0, 0, 224, 0,
    255, 0, 0, 0, 0
};
static signed char battlePointCnt;
static _tBlit_2 *blit;
static float gCameraMatrix[4][4];
static anon_active_cnt_18 battlePoint[4];
static anon_name_start_18 healthPal[4];
static int bonuscnt[4];
static anon_type_idx_54 stuntCoin[4][10];
static anon_idx_mat_34 battleCoin[4];
static anon_type_idx_34 powerupIcon[4];
static anon_type_idx_34 weaponIcon[4];

/* 0x8002271c, size 0x218 */
void ShowHUDExtra(void)
{
    int i;
    anon_used_perm_20 *pane;
    int w, h;
    int playerIdx;
    int s;
    if (!blit) {
        if (gRace == kBattle)
            blit = (_tBlit_2 *)NewMultiBlit(0, 0, (anon_texname_dulx_24 *)BattleMultiBlit, 10);
        else if (gRace == kStunt) {
            if (gNumWindows == 1) HAL_SetFont(10);
            else if (gNumWindows == 2) HAL_SetFont(11);
            else HAL_SetFont(12);
            w = HAL_GetTextWidth("8888888", -1) + 8;
            h = TB_GetFontLineHeight() + 4;
            w /= 2;
            h /= 2;
            for (playerIdx = 0; playerIdx < gNumPlayers; playerIdx++) {
                pane = InitPane(stuntPos[gNumWindows-1][playerIdx][0]-w,
                    stuntPos[gNumWindows-1][playerIdx][1]-h,
                    stuntPos[gNumWindows-1][playerIdx][0]+w,
                    stuntPos[gNumWindows-1][playerIdx][1]+h,
                    128, 0, playerIdx << 4, 1);
                ChangePaneColor(pane, playerPaneColors[playerIdx][0], playerPaneColors[playerIdx][1]);
            }
            blit = (_tBlit_2 *)NewMultiBlit(0, 0, (anon_texname_dulx_24 *)StuntMultiBlit, 17);
        }
        ClearBattlePoints();
        for (i=0; i<4; i++) {
            weaponIcon[i].type = 8;
            weaponIcon[i].idx = -1;
            powerupIcon[i].type = 0;
            powerupIcon[i].idx = -1;
            battleCoin[i].idx = -1;
            for (s=0; s<10; s++) {
                stuntCoin[i][s].type = 11;
                stuntCoin[i][s].idx = -1;
            }
        }
    }

}

/* 0x80022934, size 0x10c */
void RemoveHUDExtra(void)
{
    int player, j;
    if (blit) { RemoveBlit((_tBlit *)blit); blit = 0; }
    for (player=0; player<gNumWindows; player++) {
        if (weaponIcon[player].idx != -1) {
            ZOID_RemoveObject(weaponIcon[player].idx);
            weaponIcon[player].idx = -1;
        }
        weaponIcon[player].type = 8;
        if (powerupIcon[player].idx != -1) {
            ZOID_RemoveObject(powerupIcon[player].idx);
            powerupIcon[player].idx = -1;
        }
        powerupIcon[player].type = 0;
        if (battleCoin[player].idx != -1) {
            ZOID_RemoveObject(battleCoin[player].idx);
            battleCoin[player].idx = -1;
        }
        for (j=0; j<10; j++) {
            if (stuntCoin[player][j].idx != -1) {
                ZOID_RemoveObject(stuntCoin[player][j].idx);
                stuntCoin[player][j].idx = -1;
            }
            stuntCoin[player][j].type = 11;
        }
    }

}

/* 0x80022a40, size 0x54 */
void UpdateHUDExtraPostCamera(void)
{
    if (gRace == kBattle) {
        animWeaponIcons(0); animPowerupIcons(0); animBattleCoins(0);
    }
    if (gRace == kStunt) animStuntCoins(0);

}

/* 0x80022a94, size 0x534 */
static int animStuntPoints()
{
    unsigned char str[32];
    int i, j;
    int x, y;
    tStuntPlayer *p;
    if (gGameState & 0x7c03fffe) return 1;
    HAL_SetTextDepth(0.0f);
    TB_SetJust(0,1);
    if (gNumWindows == 1) HAL_SetFont(10); else HAL_SetFont(12);
    for (i=0; i<gNumPlayers; i++) {
        p=&gStuntPlayer[i];
        TB_SetTextColor(1);
        if (p->show && (gModelTime-p->time < 2.69f || p->numstunts)) {
            for (j=0; j<10; j++) {
                if (stuntCoin[i][j].type != 11 && p->stuntsHud[stuntCoin[i][j].type] && stuntCoin[i][j].alpha != 0.0f) {
                    HAL_SetTextTranslucency((unsigned char)stuntCoin[i][j].alpha);
                    x = stuntCoin[i][j].screenPos[0];
                    x = x/2 + (gNumWindows==1 ? 12 : 6);
                    y = stuntCoin[i][j].screenPos[1] / 2.0f;
                    TB_Sprintf(str,"X %i",p->stuntsHud[stuntCoin[i][j].type]);
                    TB_DrawText(x,y,str);
                }
            }
            x=stuntCoinPos[gNumWindows-1][i][0]+(gNumWindows==1?12:6);
            y=stuntCoinPos[gNumWindows-1][i][1];
            TB_Sprintf(str,"%i",p->pointsHud);
            TB_DrawText(TB_CenterText(str,x),y,str);
        } else if (!p->numstunts) {
            p->show=0;
            p->crash=0;
            p->bonusHud=1;
            p->wingBonusHud=0;
            p->doneBonusHud=0;
            p->pointsHud=0;
            for (j=0; j<10; j++) p->stuntsHud[j]=0;
        }
        HAL_SetTextTranslucency(-1.0f);
        if (p->showBonus > 0) {
            if (bonuscnt[i] == -1) { p->showBonus--; bonuscnt[i]=-2; }
            else if (bonuscnt[i] == -2) bonuscnt[i]=10;
            if (bonuscnt[i] >= 0) {
                int x=wrongWayPos[gNumWindows-1][i][0];
                int y=wrongWayPos[gNumWindows-1][i][1];
                TB_Sprintf(str,gText.single[191]);
                TB_Sprintf(str,"%s X %i",str,p->bonusHud);
                TB_DrawText(TB_CenterText(str,x),y,str);
                bonuscnt[i]--;
            }
        } else bonuscnt[i]=-2;
        if (!p->done) p->pointsHud=p->points;
        else if (!p->crash) {
            x=wrongWayPos[gNumWindows-1][i][0];
            y=wrongWayPos[gNumWindows-1][i][1];
            if (p->bonusHud > 1) {
                TB_Sprintf(str,gText.single[191]);
                TB_Sprintf(str,"%s X %i",str,p->bonusHud);
                TB_DrawText(TB_CenterText(str,x),y,str);
                y += TB_GetFontLineHeight();
            }
            if (p->doneBonusHud) {
                TB_DrawText(TB_CenterText(gText.taba[gText.tab[39]+(p->doneBonusHud-3)],x),y,gText.taba[gText.tab[39]+(p->doneBonusHud-3)]);
                y += TB_GetFontLineHeight();
            }
            if (p->wingBonusHud == 1) {
                TB_DrawText(TB_CenterText(gText.single[189],x),y,gText.single[189]);
            } else if (p->wingBonusHud == 2) {
                TB_DrawText(TB_CenterText(gText.single[190],x),y,gText.single[190]);
            }
        }
        TB_SetTextColor(1);
        x=stuntPos[gNumWindows-1][i][0];
        y=stuntPos[gNumWindows-1][i][1];
        TB_Sprintf(str,"%i",p->totalPoints);
        TB_DrawText(TB_CenterText(str,x),y,str);
    }
    TB_SetJust(0,3);
    HAL_SetTextDepth(-1.0f);
    return 1;

}

/* 0x80022fc8, size 0x78c */
/* The backward-scroll pointer is block-local, matching the original register lifetime. */
static int animStuntCoins()
{
    int i, j, k;
    int d, e;
    tStuntPlayer *p;
    anon_type_idx_54 *coin;
    anon_c_rgba_4 coinColor = {255,255,255,255};
    float alpha;
    anon_name_width_30 *tex;
    unsigned short texIdx;
    if ((gGameState & 0x7c03fffe) || gPauseState) return 1;
    for (i=0; i<gNumPlayers; i++) {
        p=&gStuntPlayer[i];
        if (p->crash) alpha=255.0f*(1.0f-(gModelTime-p->time)/2.69f);
        else alpha=255.0f;
        if (alpha < 0.0f) alpha=0.0f;
        coinColor.c[3]=alpha;
        if (p->show && (gModelTime-p->time<2.69f || p->numstunts)) {
            for (j=0; j<10; j++) {
                if (!p->done) p->stuntsHud[j]=p->stunts[j];
                if (p->stuntsHud[j]) {
                    for (k=0; k<10; k++) {
                        coin=&stuntCoin[i][k];
                        if (coin->type == j) break;
                        if (coin->type == 11) {
                            if (coin->idx == -1)
                                coin->idx=MBOX_NewObject(MBOX_FindObject_WORK("STUNTCOING1",0,(signed char)(gNumModels-1),1),&coin->mat,-1,0x42000);
                            tex=MBOX_FindTexture_WORK(stuntCoinTexNames[j],&texIdx,0,(signed char)(gNumModels-1),1);
                            ZOID_SetTexture(coin->idx,tex,-1);
                            ZOID_SetColor(coin->idx,&coinColor);
                            ObjInstanceList[coin->idx].w=1.5f;
                            coin->type=j;
                            coin->scrollTime=0.5f;
                            d=gNumWindows==1?-28:gNumWindows==2?-16:-12;
                            e=gNumWindows==1?-28:gNumWindows==2?-16:-12;
                            coin->startPos[0]=2*wrongWayPos[gNumWindows-1][i][0];
                            coin->startPos[1]=2.0f*(wrongWayPos[gNumWindows-1][i][1]+d);
                            coin->destPos[0]=2*stuntCoinPos[gNumWindows-1][i][0];
                            coin->destPos[1]=2.0f*(stuntCoinPos[gNumWindows-1][i][1]+d);
                            coin->startPos_Coin[0]=2*wrongWayPos[gNumWindows-1][i][0];
                            coin->startPos_Coin[1]=2.0f*(wrongWayPos[gNumWindows-1][i][1]+d);
                            coin->destPos_Coin[0]=2*stuntCoinPos[gNumWindows-1][i][0];
                            coin->destPos_Coin[1]=2.0f*(stuntCoinPos[gNumWindows-1][i][1]+d);
                            {
                                anon_type_idx_54 *prev;
                                for (k--; k>=0; k--) {
                                    prev=&stuntCoin[i][k];
                                    d-=gNumWindows==1?18:gNumWindows==2?8:8;
                                    e-=gNumWindows==1?18:gNumWindows==2?13:8;
                                    prev->scrollTime=0.5f;
                                    prev->startPos[0]=prev->screenPos[0];
                                    prev->startPos[1]=prev->screenPos[1];
                                    prev->destPos[0]=2*stuntCoinPos[gNumWindows-1][i][0];
                                    prev->destPos[1]=2.0f*(stuntCoinPos[gNumWindows-1][i][1]+d);
                                    prev->startPos_Coin[0]=prev->screenPos_Coin[0];
                                    prev->startPos_Coin[1]=prev->screenPos_Coin[1];
                                    prev->destPos_Coin[0]=2*stuntCoinPos[gNumWindows-1][i][0];
                                    prev->destPos_Coin[1]=2.0f*(stuntCoinPos[gNumWindows-1][i][1]+e);
                                }
                            }
                            break;
                        }
                    }
                    if (coin->idx != -1) {
                        coin->scrollTime-=gFrameSeconds;
                        if (coin->scrollTime<0.0f) coin->scrollTime=0.0f;
                        coin->screenPos[0]=coin->destPos[0]+(coin->scrollTime/0.5f)*(coin->startPos[0]-coin->destPos[0]);
                        coin->screenPos[1]=coin->destPos[1]+(coin->scrollTime/0.5f)*(coin->startPos[1]-coin->destPos[1]);
                        coin->screenPos_Coin[0]=coin->destPos_Coin[0]+(coin->scrollTime/0.5f)*(coin->startPos_Coin[0]-coin->destPos_Coin[0]);
                        coin->screenPos_Coin[1]=coin->destPos_Coin[1]+(coin->scrollTime/0.5f)*(coin->startPos_Coin[1]-coin->destPos_Coin[1]);
                        if (gNumWindows==1) e=10;
                        if (gNumWindows==2) e=35;
                        if (gNumWindows==3) e=5;
                        if (gNumWindows==4) e=5;
                        coin->screenPos[1]+=e;
                        coin->screenPos_Coin[1]+=e;
                        ZOID_WindowTo3D(i,coin->screenPos_Coin,&gCamera[i].mat,2.0f,coin->mat.uvs[3]);
                        coin->screenPos[1]-=e;
                        coin->screenPos_Coin[1]-=e;
                        ScaleUV(gCamera[i].mat.uvs,coin->mat.uvs,gNumWindows==1?0.005f:gNumWindows==2?0.0035f:0.005f);
                        coin->alpha=alpha;
                        coinColor.c[3]=coin->alpha;
                        ZOID_SetColor(coin->idx,&coinColor);
                        MBOX_ShowObject(coin->idx,0,whichCam[i]);
                    }
                }
            }
        } else if (!p->numstunts) {
            p->show=0;
            p->crash=0;
            p->bonusHud=1;
            p->doneBonusHud=0;
            p->wingBonusHud=0;
            p->pointsHud=0;
            for (j=0; j<10; j++) {
                p->stuntsHud[j]=0;
                if (stuntCoin[i][j].idx != -1) {
                    ZOID_RemoveObject(stuntCoin[i][j].idx);
                    stuntCoin[i][j].idx=-1;
                }
                stuntCoin[i][j].type=11;
            }
        }
    }
    return 1;

}

/* 0x80023754, size 0x334 */
static int animHealth(_tBlit_2 *blt)
{
    int slot = (blt->animID >> 4) & 15;
    int type = blt->animID & 15;
    float healthPct;
    if (slot >= gNumPlayers) {
        blt->animFunc = 0;
        return Hidden(blt,1);
    }
    healthPct = gGameCar[slot].battle.health / 800.0f;
    blt->x = healthPos[gNumWindows-1][slot][0] - blt->width / 2 / 2;
    blt->y = healthPos[gNumWindows-1][slot][1] - blt->height / 2 / 2;
    if (type == 0) {
        { anon_palette_texture_20 *inst = &gBlitInstanceList[blt->blidx]; inst->flags |= 1; }
    } else {
        blt->x += 8;
        blt->left = 16;
        blt->right = 16.0f + (blt->width-33) * healthPct;
        if (healthPct > 0.8f) {
            healthColor[slot][0][0]=0; healthColor[slot][0][1]=224; healthColor[slot][0][2]=0;
            healthColor[slot][1][0]=0; healthColor[slot][1][1]=112; healthColor[slot][1][2]=0;
        } else if (healthPct > 0.6f) {
            healthColor[slot][0][0] = 224.0f * (5.0f * (0.8f-healthPct));
            healthColor[slot][0][1] = 224;
            healthColor[slot][0][2] = 0;
            healthColor[slot][1][0] = 112.0f * (5.0f * (0.8f-healthPct));
            healthColor[slot][1][1] = 112;
            healthColor[slot][1][2] = 0;
        } else if (healthPct > 0.4f) {
            healthColor[slot][0][0]=224; healthColor[slot][0][1]=224; healthColor[slot][0][2]=0;
            healthColor[slot][1][0]=112; healthColor[slot][1][1]=112; healthColor[slot][1][2]=0;
        } else if (healthPct > 0.2f) {
            healthColor[slot][0][0] = 224;
            healthColor[slot][0][1] = 224.0f * (5.0f * (healthPct-0.2f));
            healthColor[slot][0][2] = 0;
            healthColor[slot][1][0] = 112;
            healthColor[slot][1][1] = 112.0f * (5.0f * (healthPct-0.2f));
            healthColor[slot][1][2] = 0;
        } else {
            healthColor[slot][0][0]=224; healthColor[slot][0][1]=0; healthColor[slot][0][2]=0;
            healthColor[slot][1][0]=112; healthColor[slot][1][1]=0; healthColor[slot][1][2]=0;
        }
        healthPal[slot].palBase = (char *)healthColor[slot];
        blt->palette = &healthPal[slot];
    }
    UpdateBlit((_tBlit *)blt);
    return 1;

}

/* 0x80023a88, size 0x114 */
static int animAmmoCounts()
{
    signed short slot;
    signed short x, y;
    unsigned char str[7];
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(10);
    TB_SetJust(1,1);
    for (slot=0; slot<gNumWindows; slot++) {
        if (slot < gNumWindows && gGameCar[slot].battle.weapon.type != 8) {
            x = ammoPos[gNumWindows-1][slot][0];
            y = ammoPos[gNumWindows-1][slot][1];
            if (gGameCar[slot].battle.weapon.ammoCnt >= 0) {
                TB_Sprintf(str, "%d", gGameCar[slot].battle.weapon.ammoCnt);
                TB_SetTextColor(0);
                TB_DrawText(x+1,y+1,str);
                TB_SetTextColor(1);
                TB_DrawText(x,y,str);
            }
        }
    }
    TB_SetJust(0,3);
    HAL_SetTextDepth(-1.0f);
    return 1;

}

/* 0x80023b9c, size 0x230 */
static int animWeaponIcons()
{
    signed short screenPos[2];
    int player;
    int def;
    for (player=0; player<gNumWindows; player++) {
        tCarData *car = &gGameCar[player];
        if ((signed char)car->battle.weapon.type != weaponIcon[player].type) {
            if (weaponIcon[player].idx != -1) {
                ZOID_RemoveObject(weaponIcon[player].idx);
                weaponIcon[player].idx = -1;
            }
            weaponIcon[player].type = (signed char)car->battle.weapon.type;
        }
        if (weaponIcon[player].type != 8 && weaponIcon[player].idx == -1) {
            switch (weaponIcon[player].type) {
                case 0: def=0x212; break;
                case 1: def=0x213; break;
                case 2: def=0x214; break;
                case 3: def=0x215; break;
                case 4: def=0x216; break;
                case 5: def=0x217; break;
                case 6: def=0x218; break;
                case 7: def=0x219; break;
            }
            weaponIcon[player].idx = MBOX_NewObject(gObjList[def], &weaponIcon[player].mat, -1, 0);
            ObjInstanceList[weaponIcon[player].idx].w = 1.5f;
            MBOX_HideObject(weaponIcon[player].idx,1,15);
            MBOX_ShowObject(weaponIcon[player].idx,0,whichCam[player]);
        }
        ScaleUV(gCamera[player].mat.uvs, weaponIcon[player].mat.uvs, 0.069f);
        YawUV(hudObjAngle,weaponIcon[player].mat.uvs);
        screenPos[0] = 2 * ammoPos[gNumWindows-1][player][0];
        screenPos[1] = 2.0f * ammoPos[gNumWindows-1][player][1];
        ZOID_WindowTo3D(player, screenPos, &gCamera[player].mat, 2.0f, weaponIcon[player].mat.uvs[3]);
    }
    return 1;

}

/* 0x80023dcc, size 0x20c */
static int animPowerupIcons()
{
    signed short screenPos[2];
    int desiredPowerupIcon;
    int player;
    for (player=0; player<gNumWindows; player++) {
        if (gGameCar[player].battle.defense.active) {
            if (gGameCar[player].battle.defense.active & 1) desiredPowerupIcon=1;
            else if (gGameCar[player].battle.defense.active & 2) desiredPowerupIcon=2;
        } else desiredPowerupIcon=0;
        if (desiredPowerupIcon != powerupIcon[player].type) {
            if (powerupIcon[player].idx != -1) {
                ZOID_RemoveObject(powerupIcon[player].idx);
                powerupIcon[player].idx=-1;
            }
            powerupIcon[player].type=desiredPowerupIcon;
        }
        if ((signed short)powerupIcon[player].type && powerupIcon[player].idx == -1) {
            powerupIcon[player].idx = MBOX_NewObject(
                MBOX_FindObject_WORK(powerupIconNames[powerupIcon[player].type],0,(signed char)(gNumModels-1),1),
                &powerupIcon[player].mat,-1,0);
            MBOX_HideObject(powerupIcon[player].idx,1,15);
            MBOX_ShowObject(powerupIcon[player].idx,0,whichCam[player]);
            ObjInstanceList[powerupIcon[player].idx].w=1.5f;
        }
        ScaleUV(gCamera[player].mat.uvs,powerupIcon[player].mat.uvs,0.025f);
        YawUV(hudObjAngle,powerupIcon[player].mat.uvs);
        screenPos[0]=2 * powerupPos[gNumWindows-1][player][0];
        screenPos[1]=2.0f * powerupPos[gNumWindows-1][player][1];
        ZOID_WindowTo3D(player,screenPos,&gCamera[player].mat,2.0f,powerupIcon[player].mat.uvs[3]);
    }
    return 1;

}

/* 0x80023fd8, size 0x22c */
static int animBattleCoins()
{
    signed short screenPos[2];
    int player;
    anon_c_rgba_4 coinColor;
    anon_name_width_30 *tex;
    unsigned short texIdx;
    for (player=0; player<gNumWindows; player++) {
        if (battleCoin[player].idx == -1) {
            coinColor.c[0]=playerColor[gCarTeam[player]][0];
            coinColor.c[1]=playerColor[gCarTeam[player]][1];
            coinColor.c[2]=playerColor[gCarTeam[player]][2];
            coinColor.c[3]=255;
            battleCoin[player].idx = MBOX_NewObject(MBOX_FindObject_WORK("BATTLECOING1",0,(signed char)(gNumModels-1),1),&battleCoin[player].mat,-1,0x2000);
            tex=MBOX_FindTexture_WORK("BATTLECOIN",&texIdx,0,(signed char)(gNumModels-1),1);
            ZOID_SetTexture(battleCoin[player].idx,tex,-1);
            ZOID_SetColor(battleCoin[player].idx,&coinColor);
            ObjInstanceList[battleCoin[player].idx].w=1.5f;
            MBOX_HideObject(battleCoin[player].idx,1,15);
            MBOX_ShowObject(battleCoin[player].idx,0,whichCam[player]);
        }
        ScaleUV(gCamera[player].mat.uvs,battleCoin[player].mat.uvs,gNumPlayers>2 ? 0.012f : 0.009f);
        PitchUV(-1.5707963705062866f,battleCoin[player].mat.uvs);
        screenPos[0]=2 * fragPos[gNumWindows-1][player][0];
        screenPos[1]=2.0f * fragPos[gNumWindows-1][player][1];
        ZOID_WindowTo3D(player,screenPos,&gCamera[player].mat,2.0f,battleCoin[player].mat.uvs[3]);
    }
    return 1;

}

/* 0x80024204, size 0x20 */
static void ClearBattlePoints(void)
{
    int i;
    battlePointCnt = 0;
    for (i=0; i<4; i++) battlePoint[i].active = 0;

}

/* 0x80024224, size 0x194 */
void _AddBattlePoint(signed char to, signed char from, signed char p)
{
    int j;
    if (p < 0) {
        RecordBattlePoints(to, from, p);
        if (!gOptions[40]) return;
        if (gGameCar[to].battle.Points <= 0) {
            gGameCar[to].battle.Points = 0;
            return;
        }
    }
    j = battlePointCnt;
    battlePointCnt++;
    if (battlePointCnt >= 4) battlePointCnt = 0;
    battlePoint[j].active = 1;
    battlePoint[j].cnt = 0;
    battlePoint[j].plto = to;
    battlePoint[j].plfrom = from;
    battlePoint[j].p = p;
    battlePoint[j].startx = wrongWayPos[gNumWindows-1][from][0];
    battlePoint[j].starty = wrongWayPos[gNumWindows-1][from][1];
    battlePoint[j].endx = fragPos[gNumWindows-1][to][0];
    battlePoint[j].endy = fragPos[gNumWindows-1][to][1];

}

/* 0x800243b8, size 0x518 */
static int animBattlePoints()
{
    unsigned char str[32];
    int i;
    int x, y;
    float xoffset, yoffset;
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(10);
    TB_SetJust(1,1);
    for (i=0; i<gNumPlayers; i++) {
        if (gNumWindows == 2) {
            if (i == 0) { xoffset=-2.0f; yoffset=-18.0f; }
            else if (i == 1) { xoffset=-2.0f; yoffset=18.0f; }
        } else if (gNumWindows == 3) {
            if (i == 0) { xoffset=-3.0f; yoffset=-4.0f; }
            else if (i == 1) { xoffset=3.0f; yoffset=-4.0f; }
            else if (i == 2) { xoffset=-3.0f; yoffset=4.0f; }
        } else if (gNumPlayers == 4) { /* Original tests player count here. */
            if (i == 0) { xoffset=-3.0f; yoffset=-4.0f; }
            else if (i == 1) { xoffset=3.0f; yoffset=-4.0f; }
            else if (i == 2) { xoffset=-3.0f; yoffset=4.0f; }
            else if (i == 3) { xoffset=3.0f; yoffset=4.0f; }
        }
        x=xoffset+fragPos[gNumWindows-1][i][0];
        y=yoffset+fragPos[gNumWindows-1][i][1];
        sprintf((char *)str,"%d",gGameCar[i].battle.Points);
        TB_SetTextColor(0);
        TB_DrawText(x+1,y+1,str);
        TB_SetTextColor(1);
        TB_DrawText(x,y,str);
    }
    for (i=0; i<4; i++) {
        if (battlePoint[i].active) {
            if (battlePoint[i].cnt > 40) {
                battlePoint[i].active=0;
                if (battlePoint[i].p > 0) RecordBattlePoints(battlePoint[i].plto,battlePoint[i].plfrom,battlePoint[i].p);
                gGameCar[battlePoint[i].plto].battle.Points += battlePoint[i].p;
            } else {
                x=battlePoint[i].startx+battlePoint[i].cnt*((battlePoint[i].endx-battlePoint[i].startx)/40.0f);
                y=battlePoint[i].starty+battlePoint[i].cnt*((battlePoint[i].endy-battlePoint[i].starty)/40.0f);
                if (battlePoint[i].plto == battlePoint[i].plfrom) {
                    TB_SetTextColor(1);
                    TB_DrawText(x,y,"-1");
                    x=wrongWayPos[gNumWindows-1][battlePoint[i].plfrom][0];
                    y=wrongWayPos[gNumWindows-1][battlePoint[i].plfrom][1];
                    if (gNumPlayers>2) {
                        HAL_SetFont(11);
                        TB_SetTextColor(0);
                        TB_DrawText(x+1,y+1,gText.single[27]);
                        TB_SetTextColor(1);
                        TB_DrawText(x,y,gText.single[27]);
                    }
                } else {
                    TB_SetTextColor(1);
                    TB_DrawText(x,y,"+1");
                    x=wrongWayPos[gNumWindows-1][battlePoint[i].plfrom][0];
                    y=wrongWayPos[gNumWindows-1][battlePoint[i].plfrom][1];
                    TB_Sprintf(str,gText.single[28],(*gPlayers[battlePoint[i].plto].save)->name);
                    if (gNumPlayers>2) {
                        HAL_SetFont(11);
                        TB_SetTextColor(0);
                        TB_DrawTextScrollBox(x+1,y+1,144,112,-1,0,str);
                        TB_SetTextColor(playerTextColor[gCarTeam[battlePoint[i].plto]]);
                        TB_DrawTextScrollBox(x,y,144,112,-1,0,str);
                    }
                }
                if (!gOutOfTime && !gPauseState) battlePoint[i].cnt++;
            }
        }
    }
    TB_SetJust(0,3);
    HAL_SetTextDepth(-1.0f);
    return 1;

}

