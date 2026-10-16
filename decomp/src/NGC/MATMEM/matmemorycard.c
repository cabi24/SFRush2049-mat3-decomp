/* matmemorycard.c - memory-card layout for the Midway Arcade Treasures 3 save file. */
#include "rush/rush_protos.h"

#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))

/* save sizes of the five games sharing the card file */
int size[5] = { 17856, 4096, 11264, 196608, 8192 };

/* 0x800b42d4 */
int appGetGameSaveOffset(int nGame)
{
    int i;
    int offset;

    if (nGame > 5)
        return -1;
    offset = 0;
    for (i = 0; i < nGame; i++) {
        offset = ALIGN(offset, _SectorSize);
        offset += size[i];
    }
    offset = ALIGN(offset, _SectorSize);
    return offset;
}
