/* carload.c - chooses and loads the race cars, falling back when texture memory runs out.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\carload.c; assembly: build/2049LE/asm/GameSource/src/carload.s */
#include "rush/rush_protos.h"

/* 0x800091b8, size 0x418 */
void LoadCars(int select)
{
    int i; /* r25 */
    int j; /* r0 */
    int numLoadedCars; /* r24 */
    if (select) {
        InitSelectCarLoad();
        return;
    }
    numLoadedCars = 0;
    for (i = 0; i < 13; i++) {
        if (i < 6 && ((gGameState & 8) || model[i].drone_type == 1)) {
        again:
            gLink[i].car = Random(13.0f);
            for (j = 0; j < i; j++) {
                if (gLink[i].car == gLink[j].car) {
                    goto again;
                }
            }
            gLink[i].PaintJob = gCarPaintJob[0][gLink[i].car];
            if (0x440000 - gTextureRamUsed < gWaveSizes[gLink[i].car + 0x40] + 0x200) {
                for (j = 0; j < 13; j++) {
                    if (FindWaveInstance(gLink[i].car + 0x40) < 0 &&
                        0x440000 - gTextureRamUsed >= gWaveSizes[gLink[i].car + 0x40] + 0x200) {
                        goto load;
                    }
                    gLink[i].car = (gLink[i].car + 1) % 13;
                }
                for (j = 0; j < 13; j++) {
                    if (0x440000 - gTextureRamUsed >= gWaveSizes[gLink[i].car + 0x40] + 0x200) {
                        goto load;
                    }
                    gLink[i].car = (gLink[i].car + 1) % 13;
                }
                do {
                    j = Random(numLoadedCars);
                } while (numLoadedCars > gNumPlayers && gLink[j].car >= 13);
                gLink[i].car = gLink[j].car;
                gCarModelNums[i] = gCarModelNums[j];
            } else {
            load:
                gCarModelNums[i] = LoadWave(gLink[i].car + 0x40, 1, 0, 0, 0);
                numLoadedCars++;
            }
            LoadWave(gLink[i].PaintJob + gCarPaintJobWaveNums[gLink[i].car], 0, 0, 0, 0);
            SetupCarColor(i, gLink[i].car, gLink[i].PaintJob);
        } else if (i < 6 && model[i].drone_type == 2) {
            if (0x440000 - gTextureRamUsed < gWaveSizes[gLink[i].car + 0x40] + 0x200) {
                for (gLink[i].car = 0; gLink[i].car < 13; gLink[i].car++) {
                    if (0x440000 - gTextureRamUsed >= gWaveSizes[gLink[i].car + 0x40] + 0x200) {
                        goto load2;
                    }
                }
                gLink[i].car = gLink[0].car;
                gCarModelNums[i] = gCarModelNums[0];
            } else {
            load2:
                gCarModelNums[i] = LoadWave(gLink[i].car + 0x40, 1, 0, 0, 0);
                LoadWave(gLink[i].PaintJob + gCarPaintJobWaveNums[gLink[i].car], 0, 0, 0, 0);
                SetupCarColor(i, gLink[i].car, gLink[i].PaintJob);
                numLoadedCars++;
            }
        } else {
            gLink[i].car = gLink[0].car;
            gCarModelNums[i] = gCarModelNums[0];
        }
    }
}

