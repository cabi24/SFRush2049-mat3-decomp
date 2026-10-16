/* zoidlight.c - per-object light assignment for the "zoid" renderer. Matches. */
#include "rush/rush_protos.h"

signed short gNumLights;
anon_a_l_24 *gLights;

/* 0x80062a34 */
void ZOID_SetObjLights(int idx, int numLights, anon_a_l_24 *lights)
{
    ObjInstanceList[idx].numLights = numLights;
    ObjInstanceList[idx].lights = lights;
}
