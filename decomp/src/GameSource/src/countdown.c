/* countdown.c - the 3-2-1 race-start countdown, drawn as a camera-facing textured quad per window.
   Data placement: function statics are emitted (in reverse order) when ShowCountdown is compiled,
   before the file's tentative gCountDown. */
#include "rush/rush_protos.h"

signed short gCountDown;

/* 0x8000e518 */
void ShowCountdown(int active)
{
    static tPoly *poly[4];
    static unsigned short polyTidx;
    static unsigned short lastCountDown;
    static tRGBA polyColor = { 0xff, 0xff, 0xff, 0xc0 };
    static float polyXYZ[4][3] = {
        { 1.0f, 1.0f, 6.0f }, { -1.0f, 1.0f, 6.0f }, { -1.0f, -1.0f, 6.0f }, { 1.0f, -1.0f, 6.0f }
    };
    float xyz[4][3];
    float dist;
    int i;
    int winidx;
    char str[20];

    for (winidx = 0; winidx < gNumWindows; winidx++) {
        if (active) {
            dist = GetCountdownTimeRemaining() - 0.001f;
            dist = 12.0f * (dist - (float)(int)dist);
            if (dist < 5.0f)
                dist = 5.0f;
            for (i = 0; i < 4; i++) {
                polyXYZ[i][2] = dist;
                WorldVector(polyXYZ[i], xyz[i], gCamera[winidx].mat.mat3.uvs);
                xyz[i][0] += gCamera[winidx].mat.mat3.pos[0];
                xyz[i][1] += gCamera[winidx].mat.mat3.pos[1];
                xyz[i][2] += gCamera[winidx].mat.mat3.pos[2];
            }
            if (!poly[winidx]) {
                MBOX_FindTexture_WORK("CNTDWN3", &polyTidx, 0, gNumModels - 1, 1);
                poly[winidx] = ZOID_NewPoly(4, xyz, polyTidx, &polyColor, (1 << winidx) | 0x8280, 1);
                lastCountDown = 4;
            } else if (lastCountDown > gCountDown) {
                if (winidx == gNumWindows - 1) {
                    sprintf(str, "CNTDWN%d", gCountDown);
                    MBOX_FindTexture_WORK(str, &polyTidx, 0, gNumModels - 1, 1);
                    lastCountDown = gCountDown;
                    if (gCountDown == 3)
                        PlayGameSound(0x58, 0, 1, 0);
                    else if (gCountDown == 2)
                        PlayGameSound(0x5a, 0, 1, 0);
                    else
                        PlayGameSound(0x4e, 0, 1, 0);
                }
                ZOID_UpdatePoly(poly[winidx], 4, xyz, polyTidx, &polyColor, 0, 1);
                ZOID_ShowPoly(poly[winidx]);
            } else {
                ZOID_UpdatePoly(poly[winidx], 4, xyz, polyTidx, &polyColor, 0, 1);
            }
        } else {
            if (poly[winidx]) {
                ZOID_RemovePoly(poly[winidx]);
                poly[winidx] = NULL;
            }
            gCountDown = -1;
        }
    }
}

/* 0x8000e7cc */
void CountDown(void)
{
    gCountDown = 0.999f + GetCountdownTimeRemaining();
    if (gCountDown <= 0)
        setGameState(0x100000);
}
