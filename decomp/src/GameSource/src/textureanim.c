/* textureanim.c - per-track palette scrolling and texture flipbook animation.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\textureanim.c; assembly: build/2049LE/asm/GameSource/src/textureanim.s */
#include "rush/rush_protos.h"

#define FRAME_TIME 0.0333333f
#define TEX(p) ((anon_name_width_30 *)(p))
/* a texture's strip field holds the texture definition the animation writes to */
#define STRIP(t) ((anon_name_width_30 *)TEX(t)->strip)

tTextureBase al_barrier1Swap[3] = { "AL_BARRIER1", 0, 0, "AL_BARRIER1B", 0, 0, "AL_BARRIER1C", 0, 0 };
tTextureBase al_barrier1nSwap[4] = {
    "AL_BARRIER1N", 0, 0, "AL_BARRIER1BN", 0, 0, "AL_BARRIER1CN", 0, 0, "AL_BARRIER1DN", 0, 0
};
tTextureBase warnSwap[2] = { "WARN90ON", 0, 0, "WARN90OFF", 0, 0 };
tTextureBase sirenSwap[2] = { "C4_SIREN", 0, 0, "C4_SIREN_OFF", 0, 0 };
tTextureBase firewallSwap[10] = {
    "FIREWALL01", 0, 0, "FIREWALL02", 0, 0, "FIREWALL03", 0, 0, "FIREWALL04", 0, 0, "FIREWALL05",
    0, 0, "FIREWALL06", 0, 0, "FIREWALL07", 0, 0, "FIREWALL08", 0, 0, "FIREWALL09", 0, 0,
    "FIREWALL10", 0, 0
};
tTextureBase firetorchSwap[10] = {
    "FIRETORCH01", 0, 0, "FIRETORCH02", 0, 0, "FIRETORCH03", 0, 0, "FIRETORCH04", 0, 0,
    "FIRETORCH05", 0, 0, "FIRETORCH06", 0, 0, "FIRETORCH07", 0, 0, "FIRETORCH08", 0, 0,
    "FIRETORCH09", 0, 0, "FIRETORCH10", 0, 0
};
tTextureBase firegenSwap[10] = {
    "FIREGEN01", 0, 0, "FIREGEN02", 0, 0, "FIREGEN03", 0, 0, "FIREGEN04", 0, 0, "FIREGEN05", 0, 0,
    "FIREGEN06", 0, 0, "FIREGEN07", 0, 0, "FIREGEN08", 0, 0, "FIREGEN09", 0, 0, "FIREGEN10", 0, 0
};
tTextureBase lightgrnSwap[10] = {
    "LIGHT1_GRN_MED", 0, 0, "LIGHT2_GRN_ON", 0, 0, "LIGHT3_GRN_MED", 0, 0, "LIGHT4_GRN_OFF", 0, 0,
    "LIGHT5_GRN_OFF", 0, 0, "LIGHT6_GRN_OFF", 0, 0, "LIGHT7_GRN_OFF", 0, 0, "LIGHT8_GRN_OFF", 0, 0,
    "LIGHT9_GRN_OFF", 0, 0, "LIGHT10_GRN_OFF", 0, 0
};
tTextureBase lightpurpSwap[10] = {
    "LIGHT1_PURP_MED", 0, 0, "LIGHT2_PURP_ON", 0, 0, "LIGHT3_PURP_MED", 0, 0, "LIGHT4_PURP_OFF", 0,
    0, "LIGHT5_PURP_OFF", 0, 0, "LIGHT6_PURP_OFF", 0, 0, "LIGHT7_PURP_OFF", 0, 0,
    "LIGHT8_PURP_OFF", 0, 0, "LIGHT9_PURP_OFF", 0, 0, "LIGHT10_PURP_OF", 0, 0
};
tTextureBase lightredSwap[10] = {
    "LIGHT1_RED_MED", 0, 0, "LIGHT2_RED_ON", 0, 0, "LIGHT3_RED_MED", 0, 0, "LIGHT4_RED_OFF", 0, 0,
    "LIGHT5_RED_OFF", 0, 0, "LIGHT6_RED_OFF", 0, 0, "LIGHT7_RED_OFF", 0, 0, "LIGHT8_RED_OFF", 0, 0,
    "LIGHT9_RED_OFF", 0, 0, "LIGHT10_RED_OFF", 0, 0
};
tTextureBase steamSwap[10] = {
    "STEAM01", 0, 0, "STEAM02", 0, 0, "STEAM03", 0, 0, "STEAM04", 0, 0, "STEAM05", 0, 0, "STEAM06",
    0, 0, "STEAM07", 0, 0, "STEAM08", 0, 0, "STEAM09", 0, 0, "STEAM10", 0, 0
};
tTextureBase waterSwap[30] = {
    "WATER00", 0, 0, "WATER01", 0, 0, "WATER02", 0, 0, "WATER03", 0, 0, "WATER04", 0, 0, "WATER05",
    0, 0, "WATER06", 0, 0, "WATER07", 0, 0, "WATER08", 0, 0, "WATER09", 0, 0, "WATER10", 0, 0,
    "WATER11", 0, 0, "WATER12", 0, 0, "WATER13", 0, 0, "WATER14", 0, 0, "WATER15", 0, 0, "WATER16",
    0, 0, "WATER17", 0, 0, "WATER18", 0, 0, "WATER19", 0, 0, "WATER20", 0, 0, "WATER21", 0, 0,
    "WATER22", 0, 0, "WATER23", 0, 0, "WATER24", 0, 0, "WATER25", 0, 0, "WATER26", 0, 0, "WATER27",
    0, 0, "WATER28", 0, 0, "WATER29", 0, 0
};
tTextureBase shldSwap[30] = {
    "SHLD00", 0, 0, "SHLD01", 0, 0, "SHLD02", 0, 0, "SHLD03", 0, 0, "SHLD04", 0, 0, "SHLD05", 0, 0,
    "SHLD06", 0, 0, "SHLD07", 0, 0, "SHLD08", 0, 0, "SHLD09", 0, 0, "SHLD10", 0, 0, "SHLD11", 0, 0,
    "SHLD12", 0, 0, "SHLD13", 0, 0, "SHLD14", 0, 0, "SHLD15", 0, 0, "SHLD16", 0, 0, "SHLD17", 0, 0,
    "SHLD18", 0, 0, "SHLD19", 0, 0, "SHLD20", 0, 0, "SHLD21", 0, 0, "SHLD22", 0, 0, "SHLD23", 0, 0,
    "SHLD24", 0, 0, "SHLD25", 0, 0, "SHLD26", 0, 0, "SHLD27", 0, 0, "SHLD28", 0, 0, "SHLD29", 0, 0
};
tTextureSwap Track1TextureSwap[12] = {
    3, 0, 1, 0, 0.0f, 0.25f, (void *)&al_barrier1Swap, 4, 0, 1, 0, 0.0f, 0.25f,
    (void *)&al_barrier1nSwap, 4, 1, 1, 0, 0.0f, 0.25f, (void *)&al_barrier1nSwap, 4, 2, 1, 0,
    0.0f, 0.25f, (void *)&al_barrier1nSwap, 4, 3, 1, 0, 0.0f, 0.25f, (void *)&al_barrier1nSwap, 10,
    4, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10,
    0, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10,
    9, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 2, 0, 1, 0, 0.0f, 0.1f, (void *)&sirenSwap, 0, 0,
    1, 0, 0.0f, 0.0f, 0
};
tTextureSwap Track2TextureSwap[28] = {
    3, 0, 1, 0, 0.0f, 0.25f, (void *)&al_barrier1Swap, 10, 0, 0, 0, 0.0f, 0.075f,
    (void *)&lightgrnSwap, 10, 1, 0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap, 10, 2, 0, 0, 0.0f,
    0.075f, (void *)&lightgrnSwap, 10, 3, 0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap, 10, 4, 0, 0,
    0.0f, 0.075f, (void *)&lightgrnSwap, 10, 5, 0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap, 10, 6,
    0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap, 10, 7, 0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap,
    10, 8, 0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap, 10, 9, 0, 0, 0.0f, 0.075f,
    (void *)&lightgrnSwap, 10, 0, 0, 0, 0.0f, 0.075f, (void *)&lightredSwap, 10, 1, 0, 0, 0.0f,
    0.075f, (void *)&lightredSwap, 10, 2, 0, 0, 0.0f, 0.075f, (void *)&lightredSwap, 10, 3, 0, 0,
    0.0f, 0.075f, (void *)&lightredSwap, 10, 4, 0, 0, 0.0f, 0.075f, (void *)&lightredSwap, 10, 5,
    0, 0, 0.0f, 0.075f, (void *)&lightredSwap, 10, 6, 0, 0, 0.0f, 0.075f, (void *)&lightredSwap,
    10, 7, 0, 0, 0.0f, 0.075f, (void *)&lightredSwap, 10, 8, 0, 0, 0.0f, 0.075f,
    (void *)&lightredSwap, 10, 9, 0, 0, 0.0f, 0.075f, (void *)&lightredSwap, 10, 4, 1, 0, 0.0f,
    0.07f, (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 0, 1, 0, 0.0f,
    0.07f, (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 9, 1, 0, 0.0f,
    0.07f, (void *)&firegenSwap, 2, 0, 1, 0, 0.0f, 0.1f, (void *)&sirenSwap, 0, 0, 0, 0, 0.0f,
    0.0f, 0
};
tTextureSwap Track3TextureSwap[9] = {
    3, 0, 1, 0, 0.0f, 0.25f, (void *)&al_barrier1Swap, 10, 0, 0, 0, 0.0f, 0.075f,
    (void *)&lightredSwap, 10, 4, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 2, 1, 0, 0.0f,
    0.07f, (void *)&firegenSwap, 10, 0, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 6, 1, 0, 0.0f,
    0.07f, (void *)&firegenSwap, 10, 9, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 2, 0, 1, 0, 0.0f,
    0.1f, (void *)&sirenSwap, 0, 0, 1, 0, 0.0f, 0.0f, 0
};
tTextureSwap Track4TextureSwap[9] = {
    4, 0, 1, 0, 0.0f, 0.25f, (void *)&al_barrier1nSwap, 10, 4, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 0, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 9, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 2, 0, 1, 0, 0.0f, 0.5f, (void *)&warnSwap, 2, 0, 1, 0, 0.0f, 0.1f,
    (void *)&sirenSwap, 0, 0, 1, 0, 0.0f, 0.0f, 0
};
tTextureSwap Track5TextureSwap[8] = {
    3, 0, 1, 0, 0.0f, 0.25f, (void *)&al_barrier1Swap, 10, 4, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 0, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 9, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 2, 0, 1, 0, 0.0f, 0.1f, (void *)&sirenSwap, 0, 0, 1, 0, 0.0f, 0.0f, 0
};
tTextureSwap Track6TextureSwap[18] = {
    3, 0, 1, 0, 0.0f, 0.25f, (void *)&al_barrier1Swap, 10, 0, 0, 0, 0.0f, 0.075f,
    (void *)&lightpurpSwap, 10, 1, 0, 0, 0.0f, 0.075f, (void *)&lightpurpSwap, 10, 2, 0, 0, 0.0f,
    0.075f, (void *)&lightpurpSwap, 10, 3, 0, 0, 0.0f, 0.075f, (void *)&lightpurpSwap, 10, 4, 0, 0,
    0.0f, 0.075f, (void *)&lightpurpSwap, 10, 5, 0, 0, 0.0f, 0.075f, (void *)&lightpurpSwap, 10, 6,
    0, 0, 0.0f, 0.075f, (void *)&lightpurpSwap, 10, 7, 0, 0, 0.0f, 0.075f, (void *)&lightpurpSwap,
    10, 8, 0, 0, 0.0f, 0.075f, (void *)&lightpurpSwap, 10, 9, 0, 0, 0.0f, 0.075f,
    (void *)&lightpurpSwap, 10, 4, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 2, 1, 0, 0.0f,
    0.07f, (void *)&firegenSwap, 10, 0, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 6, 1, 0, 0.0f,
    0.07f, (void *)&firegenSwap, 10, 9, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 2, 0, 1, 0, 0.0f,
    0.1f, (void *)&sirenSwap, 0, 0, 1, 0, 0.0f, 0.0f, 0
};
tTextureSwap Dm1TextureSwap[9] = {
    3, 0, 1, 0, 0.0f, 0.25f, (void *)&al_barrier1Swap, 10, 4, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 0, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 9, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 30, 0, 1, 0, 0.0f, 0.07f, (void *)&shldSwap, 2, 0, 1, 0, 0.0f, 0.1f,
    (void *)&sirenSwap, 0, 0, 1, 0, 0.0f, 0.0f, 0
};
tTextureSwap Dm2TextureSwap[8] = {
    10, 4, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap,
    10, 0, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap,
    10, 9, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 30, 0, 1, 0, 0.0f, 0.07f, (void *)&shldSwap, 2,
    0, 1, 0, 0.0f, 0.1f, (void *)&sirenSwap, 0, 0, 1, 0, 0.0f, 0.0f, 0
};
tTextureSwap Dm3TextureSwap[9] = {
    10, 0, 1, 0, 0.0f, 0.07f, (void *)&firewallSwap, 10, 4, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 0, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 9, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 30, 0, 1, 0, 0.0f, 0.07f, (void *)&shldSwap, 2, 0, 1, 0, 0.0f, 0.1f,
    (void *)&sirenSwap, 0, 0, 1, 0, 0.0f, 0.0f, 0
};
tTextureSwap Dm4TextureSwap[10] = {
    3, 0, 1, 0, 0.0f, 0.25f, (void *)&al_barrier1Swap, 10, 4, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 0, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 9, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 0, 1, 0, 0.0f, 0.1f, (void *)&firetorchSwap, 30, 0, 1, 0, 0.0f,
    0.07f, (void *)&shldSwap, 2, 0, 1, 0, 0.0f, 0.1f, (void *)&sirenSwap, 0, 0, 1, 0, 0.0f, 0.0f,
    0
};
tTextureSwap Dm5TextureSwap[29] = {
    3, 0, 1, 0, 0.0f, 0.25f, (void *)&al_barrier1Swap, 10, 0, 0, 0, 0.0f, 0.075f,
    (void *)&lightgrnSwap, 10, 1, 0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap, 10, 2, 0, 0, 0.0f,
    0.075f, (void *)&lightgrnSwap, 10, 3, 0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap, 10, 4, 0, 0,
    0.0f, 0.075f, (void *)&lightgrnSwap, 10, 5, 0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap, 10, 6,
    0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap, 10, 7, 0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap,
    10, 8, 0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap, 10, 9, 0, 0, 0.0f, 0.075f,
    (void *)&lightgrnSwap, 10, 0, 0, 0, 0.0f, 0.075f, (void *)&lightredSwap, 10, 1, 0, 0, 0.0f,
    0.075f, (void *)&lightredSwap, 10, 2, 0, 0, 0.0f, 0.075f, (void *)&lightredSwap, 10, 3, 0, 0,
    0.0f, 0.075f, (void *)&lightredSwap, 10, 4, 0, 0, 0.0f, 0.075f, (void *)&lightredSwap, 10, 5,
    0, 0, 0.0f, 0.075f, (void *)&lightredSwap, 10, 6, 0, 0, 0.0f, 0.075f, (void *)&lightredSwap,
    10, 7, 0, 0, 0.0f, 0.075f, (void *)&lightredSwap, 10, 8, 0, 0, 0.0f, 0.075f,
    (void *)&lightredSwap, 10, 9, 0, 0, 0.0f, 0.075f, (void *)&lightredSwap, 10, 4, 1, 0, 0.0f,
    0.07f, (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 0, 1, 0, 0.0f,
    0.07f, (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 9, 1, 0, 0.0f,
    0.07f, (void *)&firegenSwap, 30, 0, 1, 0, 0.0f, 0.07f, (void *)&shldSwap, 2, 0, 1, 0, 0.0f,
    0.1f, (void *)&sirenSwap, 0, 0, 1, 0, 0.0f, 0.0f, 0
};
tTextureSwap Dm6TextureSwap[19] = {
    3, 0, 1, 0, 0.0f, 0.25f, (void *)&al_barrier1Swap, 10, 4, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 0, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 9, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 0, 0, 0, 0.0f, 0.075f, (void *)&lightpurpSwap, 10, 1, 0, 0, 0.0f,
    0.075f, (void *)&lightpurpSwap, 10, 2, 0, 0, 0.0f, 0.075f, (void *)&lightpurpSwap, 10, 3, 0, 0,
    0.0f, 0.075f, (void *)&lightpurpSwap, 10, 4, 0, 0, 0.0f, 0.075f, (void *)&lightpurpSwap, 10, 5,
    0, 0, 0.0f, 0.075f, (void *)&lightpurpSwap, 10, 6, 0, 0, 0.0f, 0.075f, (void *)&lightpurpSwap,
    10, 7, 0, 0, 0.0f, 0.075f, (void *)&lightpurpSwap, 10, 8, 0, 0, 0.0f, 0.075f,
    (void *)&lightpurpSwap, 10, 9, 0, 0, 0.0f, 0.075f, (void *)&lightpurpSwap, 30, 0, 1, 0, 0.0f,
    0.07f, (void *)&shldSwap, 2, 0, 1, 0, 0.0f, 0.1f, (void *)&sirenSwap, 0, 0, 1, 0, 0.0f, 0.0f,
    0
};
tTextureSwap Dm7TextureSwap[8] = {
    10, 4, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap,
    10, 0, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap,
    10, 9, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 30, 0, 1, 0, 0.0f, 0.07f, (void *)&shldSwap, 2,
    0, 1, 0, 0.0f, 0.1f, (void *)&sirenSwap, 0, 0, 1, 0, 0.0f, 0.0f, 0
};
tTextureSwap Dm8TextureSwap[19] = {
    10, 0, 1, 0, 0.0f, 0.1f, (void *)&steamSwap, 10, 4, 1, 0, 0.0f, 0.1f, (void *)&steamSwap, 10,
    7, 1, 0, 0.0f, 0.1f, (void *)&steamSwap, 3, 0, 1, 0, 0.0f, 0.1f, (void *)&al_barrier1Swap, 10,
    0, 0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap, 10, 1, 0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap,
    10, 2, 0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap, 10, 3, 0, 0, 0.0f, 0.075f,
    (void *)&lightgrnSwap, 10, 4, 0, 0, 0.0f, 0.075f, (void *)&lightgrnSwap, 10, 5, 0, 0, 0.0f,
    0.075f, (void *)&lightgrnSwap, 10, 4, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 2, 1, 0,
    0.0f, 0.07f, (void *)&firegenSwap, 10, 0, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 6, 1, 0,
    0.0f, 0.07f, (void *)&firegenSwap, 10, 9, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 0, 1, 0,
    0.0f, 0.1f, (void *)&firetorchSwap, 30, 0, 1, 0, 0.0f, 0.07f, (void *)&shldSwap, 2, 0, 1, 0,
    0.0f, 0.1f, (void *)&sirenSwap, 0, 0, 1, 0, 0.0f, 0.0f, 0
};
tTextureSwap Stunt1TextureSwap[7] = {
    10, 4, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap,
    10, 0, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap,
    10, 9, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 2, 0, 1, 0, 0.0f, 0.1f, (void *)&sirenSwap, 0,
    0, 1, 0, 0.0f, 0.0f, 0
};
tTextureSwap Stunt2TextureSwap[7] = {
    10, 4, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap,
    10, 0, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap,
    10, 9, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 2, 0, 1, 0, 0.0f, 0.1f, (void *)&sirenSwap, 0,
    0, 1, 0, 0.0f, 0.0f, 0
};
tTextureSwap Stunt3TextureSwap[11] = {
    2, 0, 1, 0, 0.0f, 0.1f, (void *)&sirenSwap, 30, 1, 1, 0, 0.0f, 0.07f, (void *)&waterSwap, 10,
    4, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10,
    0, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10,
    9, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 0, 1, 0, 0.0f, 0.1f, (void *)&steamSwap, 10, 4,
    1, 0, 0.0f, 0.1f, (void *)&steamSwap, 10, 7, 1, 0, 0.0f, 0.1f, (void *)&steamSwap, 0, 0, 1, 0,
    0.0f, 0.0f, 0
};
tTextureSwap Stunt4TextureSwap[7] = {
    10, 4, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap,
    10, 0, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap,
    10, 9, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 2, 0, 1, 0, 0.0f, 0.1f, (void *)&sirenSwap, 0,
    0, 1, 0, 0.0f, 0.0f, 0
};
tTextureSwap ObstacleTextureSwap[11] = {
    3, 0, 1, 0, 0.0f, 0.1f, (void *)&al_barrier1Swap, 10, 4, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 2, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 0, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 6, 1, 0, 0.0f, 0.07f, (void *)&firegenSwap, 10, 9, 1, 0, 0.0f, 0.07f,
    (void *)&firegenSwap, 10, 0, 1, 0, 0.0f, 0.1f, (void *)&firewallSwap, 10, 2, 1, 0, 0.0f, 0.1f,
    (void *)&firewallSwap, 10, 6, 1, 0, 0.0f, 0.1f, (void *)&firewallSwap, 2, 0, 1, 0, 0.0f, 0.1f,
    (void *)&sirenSwap, 0, 0, 1, 0, 0.0f, 0.0f, 0
};
tTextureSwap *TextureSwapLists[19] = {
    (void *)&Track1TextureSwap, (void *)&Track2TextureSwap, (void *)&Track3TextureSwap,
    (void *)&Track4TextureSwap, (void *)&Track5TextureSwap, (void *)&Track6TextureSwap,
    (void *)&Dm1TextureSwap, (void *)&Dm2TextureSwap, (void *)&Dm3TextureSwap,
    (void *)&Dm4TextureSwap, (void *)&Dm5TextureSwap, (void *)&Dm6TextureSwap,
    (void *)&Dm7TextureSwap, (void *)&Dm8TextureSwap, (void *)&Stunt1TextureSwap,
    (void *)&Stunt2TextureSwap, (void *)&Stunt3TextureSwap, (void *)&Stunt4TextureSwap,
    (void *)&ObstacleTextureSwap
};
tPaletteAnim Track1Palettes[5] = {
    "BRI_WATER2", 0, 2048, 4, 10, 1, 0.0f, 0, "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0,
    "GREENFLAME2", 0, 512, 16, 9, 1, 0.0f, 0, "ROKTFLAME", 0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0,
    0, 0, 0.0f, 0
};
tPaletteAnim Track2Palettes[6] = {
    "AROWGRN", 0, 1024, -64, 9, 1, 0.0f, 0, "BRI_WATER2", 0, 2048, 4, 10, 1, 0.0f, 0, "FIREDANCE",
    0, 2048, 16, 10, 1, 0.0f, 0, "GREENFLAME2", 0, 512, 16, 9, 1, 0.0f, 0, "ROKTFLAME", 0, 512, 16,
    9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim Track3Palettes[7] = {
    "AROWGRN", 0, 1024, -64, 9, 1, 0.0f, 0, "GREENFLAME2", 0, 512, 16, 9, 1, 0.0f, 0, "MAGMA", 0,
    2048, -4, 10, 1, 0.0f, 0, "VAPORST3", 0, 1024, -8, 10, 1, 0.0f, 0, "FIREDANCE", 0, 2048, 12,
    10, 1, 0.0f, 0, "ROKTFLAME", 0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim Track4Palettes[4] = {
    "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0, "GREENFLAME2", 0, 512, 16, 9, 1, 0.0f, 0,
    "ROKTFLAME", 0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim Track5Palettes[6] = {
    "BRI_WATER2", 0, 2048, 4, 10, 1, 0.0f, 0, "AROWGRN", 0, 1024, -64, 9, 1, 0.0f, 0, "FIREDANCE",
    0, 2048, 16, 10, 1, 0.0f, 0, "GREENFLAME2", 0, 512, 16, 9, 1, 0.0f, 0, "ROKTFLAME", 0, 512, 16,
    9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim Track6Palettes[4] = {
    "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0, "GREENFLAME2", 0, 512, 16, 9, 1, 0.0f, 0,
    "ROKTFLAME", 0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim Stunt1Palettes[4] = {
    "AROWGRN", 0, 1024, -64, 9, 1, 0.0f, 0, "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0, "ROKTFLAME",
    0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim Stunt2Palettes[4] = {
    "AROWGRN", 0, 1024, -64, 9, 1, 0.0f, 0, "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0, "ROKTFLAME",
    0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim Stunt3Palettes[6] = {
    "WATER01", 0, 2048, 12, 9, 1, 0.0f, 0, "BRI_WATERFALL", 0, 2048, 16, 10, 1, 0.0f, 0,
    "BRI_WATER3", 0, 2048, 12, 10, 1, 0.0f, 0, "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0,
    "ROKTFLAME", 0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim Stunt4Palettes[3] = {
    "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0, "ROKTFLAME", 0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0,
    0, 0, 0.0f, 0
};
tPaletteAnim Dm1Palettes[4] = {
    "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0, "SHLD00", 0, 2048, -16, 10, 1, 0.0f, 0, "ROKTFLAME",
    0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim Dm2Palettes[4] = {
    "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0, "SHLD00", 0, 2048, -16, 10, 1, 0.0f, 0, "ROKTFLAME",
    0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim Dm3Palettes[5] = {
    "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0, "VAPORS", 0, 1024, -8, 10, 1, 0.0f, 0, "SHLD00", 0,
    2048, -16, 10, 1, 0.0f, 0, "ROKTFLAME", 0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim Dm4Palettes[5] = {
    "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0, "SHLD00", 0, 2048, -16, 10, 1, 0.0f, 0, "VAPORS", 0,
    1024, -8, 10, 1, 0.0f, 0, "ROKTFLAME", 0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim Dm5Palettes[5] = {
    "AROWGRN", 0, 1024, -64, 9, 1, 0.0f, 0, "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0, "SHLD00", 0,
    2048, -16, 10, 1, 0.0f, 0, "ROKTFLAME", 0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim Dm6Palettes[5] = {
    "AROWGRN", 0, 1024, -64, 9, 1, 0.0f, 0, "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0, "SHLD00", 0,
    2048, -16, 10, 1, 0.0f, 0, "ROKTFLAME", 0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim Dm7Palettes[4] = {
    "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0, "SHLD00", 0, 2048, -16, 10, 1, 0.0f, 0, "ROKTFLAME",
    0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim Dm8Palettes[4] = {
    "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0, "SHLD00", 0, 2048, -16, 10, 1, 0.0f, 0, "ROKTFLAME",
    0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim ObstaclePalettes[5] = {
    "AROWGRN", 0, 1024, -64, 9, 1, 0.0f, 0, "FIREDANCE", 0, 2048, 16, 10, 1, 0.0f, 0, "MAGMA", 0,
    2048, -8, 10, 1, 0.0f, 0, "ROKTFLAME", 0, 512, 16, 9, 1, 0.0f, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0
};
tPaletteAnim *PaletteLists[19] = {
    (void *)&Track1Palettes, (void *)&Track2Palettes, (void *)&Track3Palettes,
    (void *)&Track4Palettes, (void *)&Track5Palettes, (void *)&Track6Palettes,
    (void *)&Dm1Palettes, (void *)&Dm2Palettes, (void *)&Dm3Palettes, (void *)&Dm4Palettes,
    (void *)&Dm5Palettes, (void *)&Dm6Palettes, (void *)&Dm7Palettes, (void *)&Dm8Palettes,
    (void *)&Stunt1Palettes, (void *)&Stunt2Palettes, (void *)&Stunt3Palettes,
    (void *)&Stunt4Palettes, (void *)&ObstaclePalettes
};

/* 0x8005082c, size 0x16c */
void InitTextureAnimation(void)
{
    tPaletteAnim *plist; /* r31 */
    tTextureSwap *tlist; /* r30 */
    anon_name_width_30 *ttDef; /* r0 */
    unsigned short index; /* stack */
    int i; /* r29 */
    plist = PaletteLists[gTrackNum];
    if (plist) {
        for (; plist->name; plist++) {
            plist->time = FRAME_TIME;
            if (plist->state >= 9) {
                plist->ptr = MBOX_FindTexture_WORK(plist->name, &index, 0, gNumModels - 1, 1);
            }
        }
    }
    if (gRestartGame) {
        return;
    }
    tlist = TextureSwapLists[gTrackNum];
    if (!tlist) {
        return;
    }
    for (; tlist->num; tlist++) {
        for (i = 0; i < tlist->num; i++) {
            tlist->swapTexs[i].tex = MBOX_FindTexture_WORK(tlist->swapTexs[i].name, &index, 0, gNumModels - 1, 1);
            tlist->swapTexs[i].texBase = tlist->swapTexs[i].tex->strip;
            tlist->time = tlist->initTime;
            tlist->idx = tlist->startnum;
        }
    }
}

/* 0x80050998, size 0x2dc */
void TextureAnimation(void)
{
    tPaletteAnim *plist; /* r3 */
    tTextureSwap *tlist; /* r6 */
    unsigned short *palette; /* r4 */
    plist = PaletteLists[gTrackNum];
    if (plist) {
        for (; plist->name; plist++) {
            plist->time -= gFrameSeconds;
            if (plist->time > 0.0f) {
                continue;
            }
            plist->time = FRAME_TIME * plist->speed;
            palette = plist->ptr;
            if (!palette) {
                return;
            }
            switch (plist->state) {
            case 9:
                plist->start += plist->offset * gLastFrameCount;
                if (plist->start < 0) {
                    plist->start += plist->end;
                } else if (plist->start >= plist->end) {
                    plist->start -= plist->end;
                }
                STRIP(palette)->uls = plist->start * (0.0625f / TEX(palette)->width);
                break;
            case 10:
                plist->start += plist->offset * gLastFrameCount;
                if (plist->start < 0) {
                    plist->start += plist->end;
                } else if (plist->start >= plist->end) {
                    plist->start -= plist->end;
                }
                STRIP(palette)->u.ult = plist->start * (0.0625 / TEX(palette)->height);
                break;
            }
        }
    }
    tlist = TextureSwapLists[gTrackNum];
    if (!tlist) {
        return;
    }
    for (; tlist->num; tlist++) {
        tlist->time -= gFrameSeconds;
        if (tlist->time <= 0.0f) {
            tlist->time = tlist->initTime;
            if (tlist->fd) {
                tlist->idx++;
                if (tlist->idx >= tlist->num) {
                    tlist->idx = 0;
                }
            } else {
                tlist->idx--;
                if (tlist->idx < 0) {
                    tlist->idx = tlist->num - 1;
                }
            }
            STRIP(tlist->swapTexs[tlist->startnum].tex)->texBase = tlist->swapTexs[tlist->idx].tex->texBase;
        }
    }
}

