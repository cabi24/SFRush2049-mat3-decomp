/* controls.c - player control mapping, shifting, pedals and steering inputs. */
#include "rush/rush_protos.h"

signed char gControlsSet[4];
signed char gMirrorControls;
signed char gNoBrakes;
signed char gNewControls[4][11];
signed char gDefaultControls[11];

static void _ReadShifter(tModelData *m, tPadState *player)
{
    int mph;
    if (m->crashflag) return;
    if (gGameCar[m->slot].dead > 0 || gCoastFlag) return;
    if (player->controls[5] & gRealLevels[player->controller]) {
        m->mainin.gear = -1;
    } else if (player->controls[4] & gRealEdges[player->controller]) {
        if (--m->mainin.gear <= 0) m->mainin.gear = 1;
    } else {
        if (player->controls[3] & gRealEdges[player->controller]) {
            m->mainin.gear++;
            if (m->mainin.gear > 4) m->mainin.gear = 4;
            if (m->mainin.gear != 0) return;
        } else if (m->mainin.gear >= 0) return;
        mph = gGameCar[m->slot].rwmph >> 2;
        if (mph > 110) m->mainin.gear = 4;
        else if (mph > 80) m->mainin.gear = 3;
        else if (mph > 50) m->mainin.gear = 2;
        else m->mainin.gear = 1;
    }
}

static void _ReadGasAndBrake(tModelData *m, tPadState *player)
{
    if (gCoastFlag) {
        m->mainin.throttle = 0.0f;
        m->mainin.brake = 0.0f;
        return;
    }
    if (player->controls[0] == 19 || player->controls[0] == 25) {
        m->mainin.throttle = gRealStick[player->controller][1];
        if (m->mainin.throttle < 0.0f) m->mainin.throttle = 0.0f;
    } else if (player->controls[0] == 21) m->mainin.throttle = gControllerData[player->controller].analog1 / 255.0f;
    else if (player->controls[0] == 22) m->mainin.throttle = gControllerData[player->controller].analog2 / 255.0f;
    else if (player->controls[0] & gRealLevels[player->controller]) m->mainin.throttle = 1.0f;
    else m->mainin.throttle = 0.0f;
    if (player->controls[5] & gRealLevels[player->controller]) m->mainin.throttle = 1.0f;
    if (gNoBrakes) m->mainin.brake = 0.0f;
    else if (player->controls[1] == 19 || player->controls[1] == 25) {
        m->mainin.brake = -gRealStick[player->controller][1];
        if (m->mainin.brake < 0.2f) m->mainin.brake = 0.0f;
    } else if (player->controls[1] == 21) m->mainin.brake = gControllerData[player->controller].analog1 / 255.0f;
    else if (player->controls[1] == 22) m->mainin.brake = gControllerData[player->controller].analog2 / 255.0f;
    else if (player->controls[1] & gRealLevels[player->controller]) m->mainin.brake = 1.0f;
    else m->mainin.brake = 0.0f;
    m->mainin.throttle = (int)(15.0f * m->mainin.throttle < 0.0f
        ? 15.0f * m->mainin.throttle - 0.5f
        : 15.0f * m->mainin.throttle + 0.5f) / 15.0f;
    m->mainin.brake = (int)(15.0f * m->mainin.brake < 0.0f
        ? 15.0f * m->mainin.brake - 0.5f
        : 15.0f * m->mainin.brake + 0.5f) / 15.0f;
}

static void _ReadSteeringWheel(tModelData *m, tPadState *player)
{
    float playpos;
    float diff;
    float pos;
    if (gCoastFlag) { m->mainin.wheel = 0.0f; return; }
    playpos = m->mainin.wheel;
    if (player->controls[2] == 19) pos = gRealStick[player->controller][0];
    else if (player->controls[2] == 20) pos = gRealStick2[player->controller][0];
    if (player->controls[2] == 25) {
        if (pos < -0.05f) pos += 0.05f;
        else if (pos > 0.05f) pos -= 0.05f;
        else pos = 0.0f;
    } else pos *= (pos >= 0.0f ? pos : -pos) * (pos >= 0.0f ? pos : -pos);
    pos = (int)(127.0f * pos < 0.0f
        ? 127.0f * pos - 0.5f : 0.5f + 127.0f * pos) / 127.0f;
    diff = pos - playpos;
    if (diff > 0.01f) playpos = pos;
    else if (diff < -0.01f) playpos = pos;
    if (gUpsideDownTrack || gMirrorWaves) {
        if (!(gUpsideDownTrack && gMirrorWaves && !gMirrorControls)) playpos = -playpos;
    }
    m->mainin.wheel = playpos;
}

void InitRushControls(void)
{
    int i;
    InitControlSys();
    SetDefaultControls(gDefaultControls);
    for (i = 0; i < 4; i++) memcpy(gNewControls[i], gDefaultControls, 11);
}

void SetDefaultControls(signed char *controls)
{
    controls[0] = 9; controls[1] = 8; controls[2] = 19;
    controls[3] = 6; controls[4] = 5; controls[5] = 1; controls[6] = 10;
    controls[8] = 11; controls[10] = 12; controls[7] = 2; controls[9] = 13;
}

void SetDefaultControls2(signed char *controls)
{
    controls[0] = 1; controls[1] = 2; controls[2] = 19;
    controls[3] = 9; controls[4] = 8; controls[5] = 5; controls[6] = 10;
    controls[8] = 11; controls[10] = 12; controls[7] = 6; controls[9] = 13;
}

void ClearRushControls(void)
{
    tPadState *player;
    int i;
    ClearControls();
    player = gPlayers;
    for (i = 0; i < gNumPlayers; i++, player++) {
        player->autoEdges = player->edges = player->levels = 0;
        player->analog[1] = player->analog[0] = 0.0f;
    }
}

void UpdateRushControls(void)
{
    int i;
    ReadInPS2GamePad();
    UpdateControls();
    for (i = 0; i < 4; i++) {
        tPadState *player = &gPlayers[i];
        if (player->controller == 5 || i >= gNumPlayers) {
            player->autoEdges = player->edges = player->levels = 0;
            player->analog[1] = player->analog[0] = 0.0f;
        } else {
            player->levels = gContLevels[player->controller];
            player->edges = gContEdges[player->controller];
            player->autoEdges = gAutoEdges[player->controller];
            player->analog[0] = gContStick[player->controller][0];
            player->analog[1] = gContStick[player->controller][1];
        }
    }
}

void UpdateModelControls(void)
{
    tPadState *player;
    int i;
    tModelData *m;
    player = gPlayers;
    for (i = 0; i < gNumPlayers; i++, player++) {
        m = &model[player->slot];
        if (!gPauseState && !(gGameState & 8)) {
            _ReadGasAndBrake(m, player);
            _ReadSteeringWheel(m, player);
            _ReadShifter(m, player);
            if (gRace != 2 && !gCoastFlag && (player->controls[7] & gRealLevels[player->controller])) m->mainin.wing = 1;
            else m->mainin.wing = 0;
            if (gRace != 6 && !gCoastFlag && (player->controls[6] & gRealEdges[player->controller])) m->mainin.abort = 1;
            else m->mainin.abort = 0;
        }
    }
}

void FindPlayerNum(int num, int force)
{
    tPadState *player;
    player = &gPlayers[num];
    if (force || (player->controller == 5 && (gContEdges[num] & 1))) {
        player->controller = num;
        player->levels = gContLevels[num];
        player->edges = gContEdges[num];
        player->autoEdges = gAutoEdges[num];
        player->analog[0] = gContStick[num][0];
        player->analog[1] = gContStick[num][1];
        SetControllerActive(player->controller, 1);
    }
}

void SetControls(int resetToDefault)
{
    int p;
    signed char controls[12];
    for (p = 0; p < gNumPlayers; p++) {
        if (resetToDefault) {
            memcpy(controls, gDefaultControls, 11);
            controls[11] = 1;
        } else if (gPlayers[p].controller != 5) memcpy(controls, gNewControls[gPlayers[p].controller], 11);
        if (gPlayers[p].controller != 5 && gControlsSet[gPlayers[p].controller]) {
            gControlsSet[gPlayers[p].controller] = 0;
            if (gPlayers[p].save) {
                controls[11] = (*gPlayers[p].save)->rumble;
                SavePlayerControls(gPlayers[p].save, controls);
            }
            if (!gPlayers[p].save || resetToDefault) SavePlayerControls(&gJustPlayers[gPlayers[p].controller], controls);
        }
        SetPlayerControls(p, controls);
    }
}

void SetPlayerControls(int player, signed char *controls)
{
    static unsigned int gControlTable[26] = {
        1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,
        65536,131072,262144,19,20,21,22,23,24,25
    };
    int i;
    for (i = 0; i < 11; i++) gPlayers[player].controls[i] = gControlTable[controls[i]];
}
