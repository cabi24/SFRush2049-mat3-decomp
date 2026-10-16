/* HAL_Support.c - hardware-abstraction helpers. Matches. */
#include "rush/rush_protos.h"

/* 0x8001a2e4 */
unsigned char HAL_IntLog2(unsigned int num)
{
    unsigned char shift;

    for (shift = 31; shift; shift--) {
        if (num & (1 << shift))
            return shift;
    }
    return 0;
}
