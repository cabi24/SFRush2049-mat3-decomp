/* stdlib.c - PS2 libc shims for the GameCube port. Matches. */
#include "rush/rush_protos.h"

extern int rand(void);

/* 0x80094b7c */
unsigned int rand_r(void)
{
    return rand();
}
