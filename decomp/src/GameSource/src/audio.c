/* audio.c - recovered C bodies and original unit data layout.
 * Original DWARF and linked ELF: D:\rush_ngc\Rush_NGC\GameSource\src\audio.c.
 */
#define AudUpdateEmitter AudUpdateEmitter_DWARF_decl
#define AudEmitEffect AudEmitEffect_DWARF_decl
#include "rush/rush_protos.h"
#undef AudEmitEffect
#undef AudUpdateEmitter
/* Original callers retain the direction argument omitted from callee DWARF. */
extern void AudUpdateEmitter(int tag, float *pos, float *dir, float vol, float pitch);
/* The original GC ABI includes direction and fade compression, omitted from
 * the generated callee prototype. The callers preserve all four float args. */
extern int AudEmitEffect(float *pos, float *dir, float fadeDistance,
                        float fadeCompression, float maxVol, float minVol,
                        int fxid, int subId, enum_kFxNoReplace_4 replace,
                        unsigned char priority);

anon_id_basePitchRpm_1c gEngineSpecs[9][2] = {
  {
    { 36, 2900, 4000, 3000, 4500, 6000, 1.0f, 0.800000011920929f, 0.0f },
    { 41, 5000, 5000, 3000, 4500, 6000, 0.25f, 1.0f, 1.0f }
  },
  {
    { 39, 4000, 4000, 3000, 3000, 6000, 1.0f, 1.0f, 0.0f },
    { 38, 5000, 5000, 2000, 3500, 6000, 0.25f, 1.0f, 1.0f }
  },
  {
    { 44, 3000, 3000, 3000, 4500, 6000, 1.0f, 1.0f, 0.0f },
    { 43, 5000, 5000, 2000, 5000, 5000, 0.25f, 1.0f, 1.0f }
  },
  {
    { 34, 2500, 3000, 3000, 3000, 6000, 1.0f, 1.0f, 0.0f },
    { 33, 5000, 5000, 2000, 5000, 5000, 0.25f, 1.0f, 1.0f }
  },
  {
    { 42, 5000, 5000, 1500, 3000, 5000, 1.0f, 1.0f, 1.0f },
    { -1, 5000, 5000, 2000, 5000, 5000, 0.25f, 1.0f, 1.0f }
  },
  {
    { 47, 2500, 3000, 2000, 3000, 6000, 0.5f, 0.75f, 0.0f },
    { 40, 5000, 5000, 900, 3000, 5000, 1.0f, 1.0f, 1.0f }
  },
  {
    { 37, 7000, 6900, 5000, 5000, 5000, 1.0f, 1.0f, 1.0f },
    { -1, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f }
  },
  {
    { 46, 3100, 3000, 3000, 3000, 6000, 1.0f, 1.0f, 0.0f },
    { 45, 4000, 5000, 2000, 5000, 5000, 0.6000000238418579f, 1.0f, 1.0f }
  },
  {
    { 36, 3000, 3000, 3000, 3000, 6000, 1.0f, 1.0f, 0.0f },
    { 35, 5000, 5000, 2000, 5000, 5000, 0.25f, 1.0f, 1.0f }
  }
};

/* The original inline sound-state helpers shared this first .sdata2 value.
 * Keep one addressable float so the merged audio/audiodebug split retains it. */
static const float sUninitializedSoundValue[1] = { -2.0f };
static signed char gGameSoundsEnabled = 1;
static signed char gCarActive[6] = { 0, 0, 0, 0, 0, 0 };
static signed char gCarUpdatedLastTime[6] = { 0, 1, 0, 1, 0, 1 };
static int gWaterfall = { -1 };
static const signed short gBumpLRTab[16] = {
    0, 0, 0, 0, 1, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0
};
static const signed short gBumpFBTab[16] = {
    0, 1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
struct AudioEmitter {
    // Members
    unsigned char numFx; // offset 0x0, size 0x1
    int fxid; // offset 0x4, size 0x4
    int tag; // offset 0x8, size 0x4
    float pos[3]; // offset 0xC, size 0xC
    unsigned short radius; // offset 0x18, size 0x2
    unsigned int nextTime; // offset 0x1C, size 0x4
    float freqMin; // offset 0x20, size 0x4
    float freqMax; // offset 0x24, size 0x4
};
static struct AudioEmitter gStEmitters[6][20] = {
    3, 99, -1, 3638.0f, 450.0f, -708.0f, 0xc8, 0, 10.0f, 30.0f, 3, 99, -1, 3722.0f, 450.0f,
    -650.0f, 0xc8, 0, 0.0f, -1.0f, 3, 99, -1, 3802.0f, 450.0f, -592.0f, 0xc8, 0, 0.0f, -1.0f, 3,
    99, -1, 3926.0f, 450.0f, -504.0f, 0xc8, 0, 0.0f, -1.0f, 3, 99, -1, 3300.0f, 450.0f, -750.0f,
    0xc8, 0, 0.0f, -1.0f, 3, 99, -1, 3155.0f, 450.0f, -547.0f, 0xc8, 0, 0.0f, -1.0f, 3, 99, -1,
    2967.0f, 450.0f, -302.0f, 0xc8, 0, 0.0f, -1.0f, 3, 99, -1, 5044.0f, 450.0f, 116.0f, 0xc8, 0,
    0.0f, -1.0f, 20, 102, -1, 664.0f, 530.0f, 1622.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 693.0f,
    530.0f, 1482.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 858.0f, 530.0f, 1198.0f, 0x190, 0, 0.5f,
    5.0f, 20, 102, -1, 1003.0f, 530.0f, 1910.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 1449.0f,
    530.0f, 1759.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 2202.0f, 545.0f, 1765.0f, 0x190, 0, 0.5f,
    5.0f, 20, 102, -1, 1323.0f, 520.0f, 1297.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 3217.0f,
    656.0f, 1110.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 4724.0f, 573.0f, 0.0f, 0x190, 0, 0.5f,
    5.0f, 0, -1, -1, 0.0f, 0.0f, 0.0f, 0, 0, 0.0f, 0.0f, 0, -1, -1, 0.0f, 0.0f, 0.0f, 0, 0, 0.0f,
    0.0f, 0, -1, -1, 0.0f, 0.0f, 0.0f, 0, 0, 0.0f, 0.0f, 20, 102, -1, 1921.0f, 206.0f, -2686.0f,
    0x190, 0, 0.5f, 5.0f, 20, 102, -1, 1797.0f, 200.0f, -2876.0f, 0x190, 0, 0.5f, 5.0f, 20, 102,
    -1, 2387.0f, 173.0f, -2762.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 3154.0f, 173.0f, -2724.0f,
    0x190, 0, 0.5f, 5.0f, 20, 102, -1, 1653.0f, 182.0f, -2475.0f, 0x190, 0, 0.5f, 5.0f, 20, 102,
    -1, 1523.0f, 207.0f, -2473.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 1558.0f, 219.0f, -2248.0f,
    0x190, 0, 0.5f, 5.0f, 20, 102, -1, 985.0f, 191.0f, -2273.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1,
    -206.0f, 235.0f, -2114.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, -634.0f, 174.0f, -2165.0f, 0x190,
    0, 0.5f, 5.0f, 20, 102, -1, -1755.0f, -17.0f, -3463.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1,
    -1676.0f, 9.0f, -3711.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, -1806.0f, 5.0f, -3852.0f, 0x190,
    0, 0.5f, 5.0f, 20, 102, -1, -1755.0f, 16.0f, -3968.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1,
    -1621.0f, 43.0f, -4223.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, -1902.0f, 19.0f, -4335.0f, 0x190,
    0, 0.5f, 5.0f, 20, 102, -1, -1759.0f, -6.0f, -4618.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1,
    -1533.0f, -6.0f, -5175.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, -1383.0f, -5.0f, -5601.0f, 0x190,
    0, 0.5f, 5.0f, 20, 102, -1, 3171.0f, 330.0f, -3477.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1,
    94.0f, 230.0f, 626.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 249.0f, 230.0f, 459.0f, 0x190, 0,
    0.5f, 5.0f, 20, 102, -1, -263.0f, 230.0f, 481.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, -155.0f,
    250.0f, 740.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, -267.0f, 250.0f, 1486.0f, 0x190, 0, 0.5f,
    5.0f, 20, 102, -1, 518.0f, 250.0f, 1134.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 754.0f, 250.0f,
    1441.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 565.0f, 240.0f, 1847.0f, 0x190, 0, 0.5f, 5.0f, 20,
    102, -1, 2135.0f, 600.0f, -515.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 2924.0f, 540.0f, -357.0f,
    0x190, 0, 0.5f, 5.0f, 20, 102, -1, 2882.0f, 420.0f, 623.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1,
    3262.0f, 380.0f, 565.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 1593.0f, 590.0f, -2209.0f, 0x190,
    0, 0.5f, 5.0f, 20, 102, -1, 1509.0f, 550.0f, -1385.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1,
    2123.0f, 662.0f, -1320.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 684.0f, 424.0f, -2552.0f, 0x190,
    0, 0.5f, 5.0f, 0, -1, -1, 0.0f, 0.0f, 0.0f, 0, 0, 0.0f, 0.0f, 0, -1, -1, 0.0f, 0.0f, 0.0f, 0,
    0, 0.0f, 0.0f, 0, -1, -1, 0.0f, 0.0f, 0.0f, 0, 0, 0.0f, 0.0f, 0, -1, -1, 0.0f, 0.0f, 0.0f, 0,
    0, 0.0f, 0.0f, 1, 125, -1, 5408.0f, 24.0f, 88.0f, 0x190, 0, 0.27f, 0.42f, 1, 126, -1, 5340.0f,
    24.0f, 106.0f, 0x190, 0, 1.2f, 1.55f, 1, 127, -1, 5377.0f, 0.0f, 198.0f, 0x190, 0, 0.2f, 0.6f,
    1, 128, -1, 5424.0f, 8.0f, 149.0f, 0x190, 0, 0.65f, 1.2f, 1, 129, -1, 5442.0f, 26.0f, 66.0f,
    0x190, 0, 0.3f, 0.9f, 1, 130, -1, 4958.0f, 207.0f, -1386.0f, 0x190, 0, 0.6f, 1.0f, 1, 131, -1,
    4859.0f, 199.0f, -1398.0f, 0x190, 0, 0.15f, 0.21f, 1, 132, -1, 4716.0f, 205.0f, -1333.0f,
    0x190, 0, 0.2f, 0.25f, 1, 133, -1, 4771.0f, 205.0f, -1300.0f, 0x190, 0, 0.37f, 0.5f, 1, 134,
    -1, 4870.0f, 216.0f, -1257.0f, 0x190, 0, 0.27f, 0.32f, 1, 125, -1, 5004.0f, 218.0f, -1171.0f,
    0x190, 0, 0.27f, 0.42f, 1, 126, -1, 5026.0f, 208.0f, -1263.0f, 0x190, 0, 1.2f, 1.55f, 1, 127,
    -1, 5405.0f, 134.0f, -253.0f, 0x190, 0, 0.2f, 0.6f, 1, 128, -1, 5425.0f, 124.0f, -241.0f,
    0x190, 0, 0.65f, 1.2f, 1, 129, -1, 5430.0f, 133.0f, -310.0f, 0x190, 0, 0.3f, 0.9f, 1, 130, -1,
    5980.0f, 193.0f, -1361.0f, 0x190, 0, 0.6f, 1.0f, 1, 131, -1, 6012.0f, 183.0f, -1408.0f, 0x190,
    0, 0.15f, 0.21f, 1, 132, -1, 4154.0f, 77.0f, -3579.0f, 0x190, 0, 0.2f, 0.25f, 1, 133, -1,
    4252.0f, 50.0f, -3719.0f, 0x190, 0, 0.37f, 0.5f, 1, 134, -1, 4339.0f, 50.0f, -3654.0f, 0x190,
    0, 0.27f, 0.32f, 3, 99, -1, -312.0f, 155.0f, -1003.0f, 0x190, 0, 10.0f, 30.0f, 20, 102, -1,
    346.0f, 36.0f, 1800.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 496.0f, 66.0f, 1798.0f, 0x190, 0,
    0.5f, 5.0f, 20, 102, -1, 550.0f, 66.0f, 1737.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 2809.0f,
    260.0f, -205.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 3132.0f, 276.0f, -550.0f, 0x190, 0, 0.5f,
    5.0f, 20, 102, -1, 3022.0f, 277.0f, -315.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 3052.0f,
    301.0f, -969.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 3062.0f, 300.0f, -1140.0f, 0x190, 0, 0.5f,
    5.0f, 20, 102, -1, 3215.0f, 300.0f, -1243.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 1690.0f,
    75.0f, -1663.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 1988.0f, 139.0f, -1987.0f, 0x190, 0, 0.5f,
    5.0f, 20, 102, -1, 1819.0f, 130.0f, -2094.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 2309.0f,
    179.0f, -1916.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, -758.0f, 62.0f, -1070.0f, 0x190, 0, 0.5f,
    5.0f, 20, 102, -1, -741.0f, 48.0f, -676.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, -530.0f, 87.0f,
    525.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, -719.0f, 88.0f, 503.0f, 0x190, 0, 0.5f, 5.0f, 20,
    102, -1, -751.0f, 88.0f, 644.0f, 0x190, 0, 0.5f, 5.0f, 20, 102, -1, 2602.0f, 101.0f, 569.0f,
    0x190, 0, 0.5f, 5.0f, 1, 125, -1, 300.0f, 100.0f, 1639.0f, 0x190, 0, 0.27f, 0.42f, 1, 126, -1,
    302.0f, 100.0f, 1577.0f, 0x190, 0, 1.2f, 1.55f, 1, 127, -1, 148.0f, 56.0f, 1538.0f, 0x190, 0,
    0.2f, 0.6f, 1, 128, -1, 307.0f, 96.0f, 1771.0f, 0x190, 0, 0.65f, 1.2f, 1, 129, -1, 159.0f,
    69.0f, 1895.0f, 0x190, 0, 0.3f, 0.9f, 1, 130, -1, 16.0f, 70.0f, 1670.0f, 0x190, 0, 0.6f, 1.0f,
    1, 131, -1, -71.0f, 68.0f, 1672.0f, 0x190, 0, 0.15f, 0.21f, 1, 132, -1, -72.0f, 68.0f, 1724.0f,
    0x190, 0, 0.2f, 0.25f, 1, 133, -1, 32.0f, 70.0f, 1694.0f, 0x190, 0, 0.37f, 0.5f, 1, 134, -1,
    108.0f, 70.0f, 1490.0f, 0x190, 0, 0.27f, 0.32f, 1, 125, -1, 1806.0f, 144.0f, 1549.0f, 0x190, 0,
    0.27f, 0.42f, 1, 126, -1, 1720.0f, 123.0f, 1597.0f, 0x190, 0, 1.2f, 1.55f, 1, 127, -1, 1866.0f,
    156.0f, 1589.0f, 0x190, 0, 0.2f, 0.6f, 1, 128, -1, 1936.0f, 174.0f, 1520.0f, 0x190, 0, 0.65f,
    1.2f, 1, 129, -1, 1889.0f, 163.0f, 1570.0f, 0x190, 0, 0.3f, 0.9f, 1, 130, -1, 421.0f, 108.0f,
    1734.0f, 0x190, 0, 0.6f, 1.0f, 1, 131, -1, 553.0f, 108.0f, 1822.0f, 0x190, 0, 0.15f, 0.21f, 1,
    132, -1, 707.0f, 108.0f, 1714.0f, 0x190, 0, 0.2f, 0.25f, 1, 133, -1, 777.0f, 108.0f, 1792.0f,
    0x190, 0, 0.37f, 0.5f, 1, 134, -1, 901.0f, 108.0f, 1664.0f, 0x190, 0, 0.27f, 0.32f
};

static void _StopStationaryEmitters(void);

void PlayExitSound(unsigned int edges)
{
    AudPlayEffect((edges & 1) ? 7 : 1, 0, 1, 0);
}

void PlayStartABSound(unsigned int edges)
{
    AudPlayEffect((edges & 1) ? 7 : ((edges & 2) ? 0 : 1), 0, 1, 0);
}

void PlayLRSound(unsigned int edges)
{
    AudPlayEffect((edges & 0x1000) ? 3 : 6, 0, 1, 0);
}

void PlayUDSound(unsigned int edges)
{
    AudPlayEffect((edges & 0x400) ? 8 : 2, 0, 1, 0);
}

void EnableGameSounds(int enabled)
{
    gGameSoundsEnabled = enabled;
    AudSetFxVol(enabled ? 0.0f : gOptions[13] / 10.0f, 0.1f);
}

int PlayGameSound(int fxid, int subId, enum_kFxNoReplace_3 replace,
                  unsigned char priority)
{
    if (!gGameSoundsEnabled) return -1;
    if (fxid == -1) return -1;
    return AudPlayEffect(fxid, subId, replace, priority);
}

int EmitGameSound(float *pos, float *dir, float fadeDistance,
                  float fadeCompression, float maxVol, float minVol,
                  int fxid, int subId, enum_kFxNoReplace_3 replace,
                  unsigned char priority)
{
    if (!gGameSoundsEnabled) return -1;
    if (fxid == -1) return -1;
    return AudEmitEffect(pos, dir, fadeDistance, fadeCompression, maxVol,
                         minVol, fxid, subId, replace, priority);
}

static void _StartGameMusic(int resume)
{
    int song;
    static int sTrackSongs[19] = {
        5, 7, 12, 6, 2, 13, 4, 15, 10, 8, 0, 2, 16, 4, 11, 1, 17, 14, 3
    };
    if (gOptions[12]) {
        song = (sTrackSongs[gTrackNum] == -1) ? Random(18.0f) : sTrackSongs[gTrackNum];
        AudSetMusicVol(gOptions[12] / 10.0f, resume ? 1.0f : 0.05f);
        if (gOptions[14] == 18) AudPlaySong(song, 1);
        else if (gOptions[14] < 18) AudPlaySong(gOptions[14], 1);
    }
}

void StartGameMusic(void) { _StartGameMusic(0); }
void PauseGameMusic(void) {}
void ResumeGameMusic(void) {}

void StartListeners(void)
{
    int p;
    if (!(gGameState & 8)) {
        for (p = 0; p < gNumPlayers; p++) {
            if (gRace != 2 || gPlayers[p].slot == 0)
                gPlayers[p].listener = AudAddListener(unitmatrix[3], unitmatrix[3],
                                                     unitmatrix[2], unitmatrix[1]);
            else gPlayers[p].listener = -1;
        }
        AudSetFxVol(gOptions[13] / 10.0f, 0.1f);
    }
}

void UpdateListeners(void)
{
    int p;
    for (p = 0; p < gNumPlayers; p++) {
        if (gPlayers[p].listener != -1)
            AudUpdateListener(gPlayers[p].listener, model[gPlayers[p].slot].rwr,
                              unitmatrix[3], gCamera[p].mat.mat4.uvs[2], gCamera[p].mat.mat4.uvs[1]);
    }
}

void StopListeners(void)
{
    int p;
    _StopStationaryEmitters();
    for (p = 0; p < gNumPlayers; p++) {
        AudRemoveListener(gPlayers[p].listener);
        gPlayers[p].listener = -1;
    }
}

void UpdateStationaryEmitters(void)
{
    int i;
    int fxid;
    struct AudioEmitter *em;
    struct AudioEmitter *prevem;
    float pos[3];
    if (gGameSoundsEnabled) {
        if (gTrackNum == 16 && gWaterfall == -1) {
            pos[0] = -482.0f;
            pos[1] = 217.0f;
            pos[2] = -1772.0f;
            gWaterfall = AudEmitEffect(pos, unitmatrix[3], 1000.0f, 0.0f,
                                      1.0f, 0.0f, 91, 0, 0, 255);
        }
        if (gTrackNum <= 5) {
            em = gStEmitters[gTrackNum];
            for (i = 0, prevem = em; i < 20; i++, prevem = em, em++) {
                if (em->fxid != -1 && em->freqMax != 0.0f && em->nextTime < sc.frameCount) {
                    if (em->freqMax < 0.0f) em->nextTime = prevem->nextTime;
                    else {
                        fxid = em->fxid;
                        if (em->numFx > 1) fxid += Random(em->numFx);
                        em->nextTime = sc.frameCount + (int)(gRetraceRate *
                                        (em->freqMin + Random(em->freqMax - em->freqMin)));
                    }
                    em->tag = AudEmitEffect(em->pos, unitmatrix[3], em->radius,
                                           0.0f, 1.0f, 0.0f, fxid, 0, 0, 255);
                }
            }
        }
    }
}

static void _StopStationaryEmitters(void)
{
    int i;
    struct AudioEmitter *em;
    if (gGameSoundsEnabled) {
        if (gTrackNum == 16) {
            AudStopEmitter(gWaterfall);
            gWaterfall = -1;
        }
        if (gTrackNum <= 5) {
            em = gStEmitters[gTrackNum];
            for (i = 0; i < 20; i++, em++) {
                if (em->fxid != -1) AudStopEmitter(em->tag);
            }
        }
    }
}

/* Car-sound state and helpers, recovered from the original unit DWARF. */
#pragma force_active on
static anon_specs_state_54 gEngineSounds[6];
static anon_sound_vol_14 gPaveSkidSounds[6];
static anon_dirt_water_3c gRoadNoises[4];
static int gScrapes[4];
static int gHornSounds[4];
static int gBogSounds[4];
static float gBumpTime[4];
static anon_peak_peakVec_18 gBump[4][5];
static float gScrapeTime[4];
static float gThumpTime[4];
static signed short gScrapeState[4];
/* Preserved original audio.c debug buffer, including its BSS extent. */
static char debug_info[1024];
#pragma force_active reset

static void _GetForceAndPeak(signed short index, int p, float *vec, float threshold);

static void _SetSoundVP(anon_sound_vol_14 *sound, float vol, float pitch)
{
    if (sound->vol != vol) {
        sound->vol = vol;
        AudChangeEffectVol(sound->sound, vol);
    }
    if (sound->pitch != pitch) {
        sound->pitch = pitch;
        AudChangeEffectPitch(sound->sound, pitch);
    }
}

/* Engine specifications retained at their original typed extent. */

static void _InitBumps(signed short p);
static void _UpdateEngineSound(struct tModelData *model);
static void _UpdateSkids(struct tModelData *model);
static void _UpdateRoadSounds(struct tModelData *model);
static void _UpdateBumps(struct tModelData *model);
void StartCarSounds(struct tModelData *model)
{
    int i;
    int p = model->slot;
    anon_specs_state_54 *eng = &gEngineSounds[p];
    if (gGameSoundsEnabled && !model->crashflag) {
        eng->specs = gEngineSpecs[model->engine_type];
        for (i = 0; i < 2; i++) {
            _InitSoundState(&eng->state[i]);
            if (eng->specs[i].id != -1) {
                if (model->drone_type == 2)
                    eng->state[i].sound = AudPlayEffectVP(eng->specs[i].id, p, 0, 0, 0.0f, 0.0f, 0.0f);
                else
                    eng->state[i].sound = AudEmitEffect(unitmatrix[3], unitmatrix[3], 400.0f, 0.0f, 1.0f, 0.0f, eng->specs[i].id, p, 0, 128);
            }
        }
        _InitSoundState(&eng->hum);
        if (model->body_type == 2) {
            if (model->drone_type == 2)
                eng->hum.sound = AudPlayEffectVP(83, p, 0, 0, 0.0f, 0.0f, 0.0f);
            else
                eng->hum.sound = AudEmitEffect(unitmatrix[3], unitmatrix[3], 400.0f, 0.0f, 1.0f, 0.0f, 83, p, 0, 128);
        }
        _InitSoundState(&eng->rev);
        _InitSoundState(&gPaveSkidSounds[p]);
        if (p < 4) {
            _InitSoundState(&gRoadNoises[p].dirt);
            _InitSoundState(&gRoadNoises[p].water);
            _InitSoundState(&gRoadNoises[p].pave);
            gScrapes[p] = -1;
            _InitBumps(p);
            gHornSounds[p] = -1;
            gBogSounds[p] = -1;
        }
        gCarActive[p] = 1;
    }
}

void UpdateCarSounds(struct tModelData *model)
{
    int p = model->slot;
    anon_slot_controller_50 *player = &gPlayers[p];
    static float sLastHornTime[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    if (gGameSoundsEnabled && gCarActive[p]) {
        _UpdateEngineSound(model);
        _UpdateSkids(model);
        if (model->drone_type == 2) {
            _UpdateRoadSounds(model);
            _UpdateBumps(model);
            if (gSeconds < sLastHornTime[p]) sLastHornTime[p] = 0.0f;
            if (!(model->appearance & 16) && (player->edges & player->controls[10]) &&
                gSeconds - sLastHornTime[p] > 0.1f) {
                sLastHornTime[p] = gSeconds;
                gHornSounds[p] = AudPlayEffect(gCarHorn[p+1][model->body_type] + 10, p, 1, 4);
            }
            if (model->bogState == 1) {
                gBogSounds[p] = AudPlayEffect(55, p, 2, 1);
                model->bogState = 2;
            } else if (model->bogState == 3) {
                AudStopEffect(gBogSounds[p]);
                gBogSounds[p] = -1;
                model->bogState = 0;
            }
        }
    }
}

static void _UpdateEngineSound(struct tModelData *model)
{
    float rpm;
    float torq;
    float pitch;
    float vol;
    int p = model->slot;
    int i;
    anon_specs_state_54 *eng = &gEngineSounds[p];
    float scale1;
    if (gGameSoundsEnabled) {
        rpm = model->rpm < 0 ? -model->rpm : model->rpm;
        torq = model->engtorque;
        if (!(gGameState & 0x400000) && model->drone_type != 2)
            rpm = rpm < 890.0f ? rpm : 890.0f;
        for (i = 0; i < 2; i++) {
            if (eng->state[i].sound == -1) break;
            pitch = 1.0f + (rpm / eng->specs[i].basePitchRpm - 1.0f) *
                    ((float)eng->specs[i].basePitchRpm / eng->specs[i].pitchRpmRange);
            pitch = pitch < 0.0f ? 0.0f : (pitch > 2.0f ? 2.0f : pitch);
            if (rpm < eng->specs[i].lowVolRpm) vol = eng->specs[i].lowVol;
            else if (rpm < eng->specs[i].midVolRpm)
                vol = eng->specs[i].lowVol +
                    ((rpm - eng->specs[i].lowVolRpm) / (eng->specs[i].midVolRpm - eng->specs[i].lowVolRpm)) *
                    (eng->specs[i].midVol - eng->specs[i].lowVol);
            else if (rpm < eng->specs[i].hiVolRpm)
                vol = eng->specs[i].midVol +
                    ((rpm - eng->specs[i].midVolRpm) / (eng->specs[i].hiVolRpm - eng->specs[i].midVolRpm)) *
                    (eng->specs[i].hiVol - eng->specs[i].midVol);
            else vol = eng->specs[i].hiVol;
            scale1 = 0.85f + 0.15f * ((200.0f + torq) / 900.0f);
            vol *= scale1;
            if (model->drone_type == 2) {
                vol *= 0.8f - 0.05f * (gRace == 2 ? 1 : gNumPlayers);
                if (eng->state[i].pitch != pitch) {
                    eng->state[i].pitch = pitch;
                    AudChangeEffectPitch(eng->state[i].sound, eng->state[i].pitch);
                }
                if (eng->state[i].vol != vol) {
                    eng->state[i].vol = vol;
                    AudChangeEffectVol(eng->state[i].sound, vol);
                }
            } else {
                vol *= 0.85f;
                if (gGameState & 0x400000) {
                    if (eng->rev.sound != -1) _StopSound(&eng->rev, 1);
                } else if (gCountDown > 0.75f &&
                           (eng->rev.sound == -1 || !AudEmitterPlaying(eng->rev.sound)) &&
                           (int)Random(5.0f) == 0) {
                    eng->rev.sound = AudEmitEffect(unitmatrix[3], unitmatrix[3],
                        400.0f, 0.0f, 1.0f, 0.0f, 80.0f + Random(3.0f), p, 0, 128);
                    AudUpdateEmitter(eng->rev.sound, model->rwr, unitmatrix[3], 0.8f, sUninitializedSoundValue[0]);
                }
                if (eng->state[i].pitch != pitch) eng->state[i].pitch = pitch;
                else pitch = sUninitializedSoundValue[0];
                AudUpdateEmitter(eng->state[i].sound, model->rwr, unitmatrix[3], vol, pitch);
            }
        }
        if (eng->hum.sound != -1) {
            if ((model->reckon.rwmph >> 2) < 0) vol = 0.25f - Random(0.15f);
            else if (model->throttle < 0.1f) {
                if (vol < 0.28f) vol = 0.25f - Random(0.15f);
                else {
                    vol -= 0.01f + Random(0.025f);
                    if (vol < 0.2f) vol = 0.2f;
                }
            } else {
                vol = 0.2f + (1.35f * model->rpm) / 10000.0f;
                vol = vol > 1.0f ? 1.0f : vol;
                vol -= Random(0.15f);
            }
            if (model->drone_type == 2) {
                vol *= 0.8f - 0.05f * (gRace == 2 ? 1 : gNumPlayers);
                if (eng->hum.vol != vol) {
                    eng->hum.vol = vol;
                    AudChangeEffectVol(eng->hum.sound, vol);
                }
            } else {
                vol *= 0.75f;
                AudUpdateEmitter(eng->hum.sound, model->rwr, unitmatrix[3], vol, 1.0f);
            }
        }
    }
}

static void _UpdateSkids(struct tModelData *model)
{
    int p = model->slot;
    int i;
    float pitch;
    float maxPitch = 0.5f;
    float maxVol = 0.0f;
    static const float sPitchRange[4] = { 0.8f, 0.8f, 1.0f, 0.6f };
    for (i = 0; i < 4; i++) {
        if (model->sviscode[i] == 0) {
            pitch = 1.0f + (sPitchRange[i] * model->skidIntensity[i] - sPitchRange[i] / 2.0f);
            if (pitch > maxPitch) maxPitch = pitch;
            pitch = 0.7f * (model->skidIntensity[i] * sPitchRange[i]);
            if (pitch > maxVol) maxVol = pitch;
        }
    }
    if (gPaveSkidSounds[p].sound == -1 && maxVol > 0.0f) {
        if (model->drone_type == 2)
            gPaveSkidSounds[p].sound = AudPlayEffect(79, model->slot, 2, 2);
        else
            gPaveSkidSounds[p].sound = AudEmitEffect(model->rwr, unitmatrix[3], 400.0f, 0.0f, 1.0f, 0.0f, 79, p, 0, 130);
    }
    if (gPaveSkidSounds[p].sound != -1) {
        if (maxVol <= 0.0f) {
            if (model->drone_type == 2) AudStopEffect(gPaveSkidSounds[p].sound);
            else AudStopEmitter(gPaveSkidSounds[p].sound);
            _InitSoundState(&gPaveSkidSounds[p]);
        } else if (model->drone_type == 2) {
            if (gPaveSkidSounds[p].vol != maxVol) {
                gPaveSkidSounds[p].vol = maxVol;
                AudChangeEffectVol(gPaveSkidSounds[p].sound, maxVol);
            }
            if (gPaveSkidSounds[p].pitch != maxPitch) {
                gPaveSkidSounds[p].pitch = maxPitch;
                AudChangeEffectPitch(gPaveSkidSounds[p].sound, gPaveSkidSounds[p].pitch);
            }
        } else {
            if (gPaveSkidSounds[p].pitch != maxPitch)
                gPaveSkidSounds[p].pitch = maxPitch;
            else maxPitch = sUninitializedSoundValue[0];
            AudUpdateEmitter(gPaveSkidSounds[p].sound, model->rwr, unitmatrix[3], maxVol, maxPitch);
        }
    }
}
static void _UpdateRoadSounds(struct tModelData *model)
{
    int i;
    int p = model->slot;
    int numPave = 0;
    int numDirt = 0;
    int numWater = 0;
    float vol;
    float dirtVol = 0.0f;
    float paveVol = 0.0f;
    float waterVol = 0.0f;
    for (i = 0; i < 4; i++) {
        if (model->sviscode[i] == 0) {
            vol = (model->reckon.rwmph >> 2) / 160.0f;
            vol = vol < 0.0f ? -vol : vol;
            vol = vol < 0.0f ? 0.0f : (vol > 1.0f ? 1.0f : vol);
            vol = 1.0f - (1.0f - vol) * (1.0f - vol);
            ++numPave;
            vol /= (numPave == 1 ? 2.0f : (numPave == 2 ? 4.0f : 8.0f));
            paveVol += vol;
        } else if (model->sviscode[i] == 1) {
            vol = (model->reckon.rwmph >> 2) / 80.0f;
            if (vol) {
                if (vol < 0.0f) vol = -vol;
                vol = vol < 0.0f ? 0.0f : (vol > 1.0f ? 1.0f : vol);
                vol = 1.0f - (1.0f - vol) * (1.0f - vol);
            }
            if (model->skidIntensity[i] > vol) vol = model->skidIntensity[i];
            ++numDirt;
            vol /= (numDirt == 1 ? 2.0f : (numDirt == 2 ? 4.0f : 8.0f));
            dirtVol += vol;
        } else if (model->sviscode[i] == 2 || model->sviscode[i] == 3) {
            vol = (model->reckon.rwmph >> 2) / 80.0f;
            if (vol) {
                if (vol < 0.0f) vol = -vol;
                vol = vol < 0.0f ? 0.0f : (vol > 1.0f ? 1.0f : vol);
                vol = 1.0f - (1.0f - vol) * (1.0f - vol);
            }
            ++numWater;
            vol /= (numWater == 1 ? 2.0f : (numWater == 2 ? 4.0f : 8.0f));
            waterVol += vol;
        }
    }
    if (dirtVol > 0.0f) {
        if (gRoadNoises[p].dirt.sound == -1)
            gRoadNoises[p].dirt.sound = AudPlayEffect(63, p, 1, 2);
        _SetSoundVP(&gRoadNoises[p].dirt, 0.7f * dirtVol,
                    (dirtVol > 0.5f ? dirtVol : 0.5f) - 0.25f);
    } else if (!numDirt && gRoadNoises[p].dirt.sound != -1) {
        AudStopEffect(gRoadNoises[p].dirt.sound);
        _InitSoundState(&gRoadNoises[p].dirt);
    }
    if (waterVol > 0.0f) {
        if (gRoadNoises[p].water.sound == -1) {
            if (waterVol > 0.33f) AudPlayEffect(86, model->slot, 2, 1);
            gRoadNoises[p].water.sound = AudPlayEffect(92, model->slot, 1, 2);
        }
        _SetSoundVP(&gRoadNoises[p].water, waterVol, 1.0f);
    } else if (!numWater && gRoadNoises[p].water.sound != -1) {
        AudStopEffect(gRoadNoises[p].water.sound);
        _InitSoundState(&gRoadNoises[p].water);
    }
    if (paveVol > 0.0f) { }
    else if (!numPave && gRoadNoises[p].pave.sound != -1) {
        AudStopEffect(gRoadNoises[p].pave.sound);
        _InitSoundState(&gRoadNoises[p].pave);
    }
}
#include "msl_math_inline.h"
static void _CheckForces(struct tModelData *model);

static void _UpdateBumps(struct tModelData *model)
{
    int i;
    signed short *scrapeState;
    int p = model->slot;
    float vol;
    float highForce;
    signed short bumpIndex;
    signed short scrapeSide;
    signed short scrapeSnd;
    if (model->crashflag) {
        _InitBumps(p);
        return;
    }
    _CheckForces(model);
    highForce = 0.0f;
    bumpIndex = 0;
    if (gBumpTime[p] && (gSeconds - gBumpTime[p] > 1.0f || gSeconds < gBumpTime[p]))
        gBumpTime[p] = 0.0f;
    for (i = 0; i < 4; i++) {
        if (gBump[p][i].bump && gBump[p][i].peak > highForce) highForce = gBump[p][i].peak;
    }
    vol = (0.01f * highForce < 150.0f ? 150.0f : (0.01f * highForce > 235.0f ? 235.0f : 0.01f * highForce));
    vol *= 0.00390625f;
    vol *= 0.65f;
    highForce = 0.5f * highForce;
    for (i = 0; i < 4; i++) {
        if (gBump[p][i].bump) {
            if (gBump[p][i].peak > highForce) bumpIndex |= 1 << i;
            gBump[p][i].peak = 0.0f;
            gBump[p][i].bump = 0;
        }
    }
    if (bumpIndex) {
        if (gBumpTime[p] == 0.0f) {
            AudPlayEffectVP(64.0f + Random(13.0f), model->slot, 1, 1, vol,
                            gBumpLRTab[bumpIndex], gBumpFBTab[bumpIndex]);
            gBumpTime[p] = gSeconds;
        } else {
            AudPlayEffectVP(64.0f + Random(13.0f), model->slot, 1, 1, vol,
                            gBumpLRTab[bumpIndex], gBumpFBTab[bumpIndex]);
            gBumpTime[p] = 0.0f;
        }
    } else {
        bumpIndex = 0;
        if (gBump[p][4].bump) {
            highForce = 0.0f;
            for (i = 0; i < 3; i++)
                if (fabs(gBump[p][4].peakVec[i]) > highForce) highForce = fabs(gBump[p][4].peakVec[i]);
            vol = (0.001f * highForce < 180.0f ? 180.0f : (0.001f * highForce > 235.0f ? 235.0f : 0.001f * highForce));
            vol *= 0.00390625f;
            if (fabs(gBump[p][4].peakVec[0]) > 0.5f * highForce) {
                if (gBump[p][4].peakVec[0] < 0.0f) bumpIndex |= 1;
                else bumpIndex |= 2;
            }
            if (fabs(gBump[p][4].peakVec[1]) > 0.5f * highForce) {
                if (gBump[p][4].peakVec[1] < 0.0f) bumpIndex |= 4;
                else bumpIndex |= 8;
            }
            if (fabs(gBump[p][4].peakVec[2]) > 0.5f * highForce) bumpIndex |= 1;
            gBump[p][4].peak = 0.0f;
            gBump[p][4].bump = 0;
        }
        if (bumpIndex)
            AudPlayEffectVP(64.0f + Random(13.0f), model->slot, 1, 1, vol,
                            gBumpLRTab[bumpIndex], gBumpFBTab[bumpIndex]);
    }
    if (!bumpIndex && model->magvel > 20.0f) {
        scrapeSide = 0;
        if (model->bodyForce[0][0] || model->bodyForce[2][0]) scrapeSide |= 1;
        if (model->bodyForce[1][0] || model->bodyForce[3][0]) scrapeSide |= 2;
        scrapeState = &gScrapeState[p];
        scrapeSnd = 0;
        switch (*scrapeState) {
        case 0:
            *scrapeState = scrapeSide;
            scrapeSnd = scrapeSide;
            gScrapeTime[p] = gSeconds;
            break;
        case 1: case 2: case 3:
            if (!scrapeSide) {
                AudStopEffect(gScrapes[p]);
                gScrapes[p] = -1;
                *scrapeState = 0;
                gScrapeTime[p] = 0.0f;
            } else if (gSeconds - gScrapeTime[p] > 0.1f || gSeconds < gScrapeTime[p]) {
                *scrapeState += 3;
                scrapeSnd = *scrapeState;
            }
            break;
        case 4: case 5: case 6:
            if (!scrapeSide) {
                AudStopEffect(gScrapes[p]);
                gScrapes[p] = -1;
                *scrapeState = 0;
                gScrapeTime[p] = 0.0f;
            }
            break;
        }
        switch (scrapeSnd) {
        case 1: AudPlayEffectVP(64.0f + Random(13.0f), model->slot, 1, 1, 0.65f, 1.0f, 0.0f); break;
        case 2: AudPlayEffectVP(64.0f + Random(13.0f), model->slot, 1, 1, 0.65f, -1.0f, 0.0f); break;
        case 3: AudPlayEffectVP(64.0f + Random(13.0f), model->slot, 1, 1, 0.65f, 0.0f, 0.0f); break;
        case 4: gScrapes[p] = AudPlayEffectVP(85, model->slot, 1, 2, 0.65f, 1.0f, 0.0f); break;
        case 5: gScrapes[p] = AudPlayEffectVP(85, model->slot, 1, 2, 0.65f, -1.0f, 0.0f); break;
        case 6: gScrapes[p] = AudPlayEffectVP(85, model->slot, 1, 2, 0.65f, 0.0f, 0.0f); break;
        }
    } else {
        AudStopEffect(gScrapes[p]);
        gScrapes[p] = -1;
        gScrapeState[p] = 0;
        gScrapeTime[p] = 0.0f;
    }
    if (gThumpTime[p] != 0.0f && (gSeconds - gThumpTime[p] > (1.0f / 6.0f) || gSeconds < gThumpTime[p]))
        gThumpTime[p] = 0.0f;
    if (model->mthumpflag > model->thumpflag) model->mthumpflag = model->thumpflag;
    if (model->mthumpflag != model->thumpflag) {
        model->mthumpflag = model->thumpflag;
        if (!gThumpTime[p]) {
            gThumpTime[p] = gSeconds;
            if (!(gGameState & 0x200000)) {
                switch (model->mthumpflag) {
                case 1: AudPlayEffectVP(60, model->slot, 1, 2, 1.0f, gBumpLRTab[model->thump_side], gBumpFBTab[model->thump_side]); break;
                case 2: AudPlayEffectVP(58, model->slot, 1, 2, 1.0f, gBumpLRTab[model->thump_side], gBumpFBTab[model->thump_side]); break;
                case 3: AudPlayEffectVP(56, model->slot, 1, 2, 1.0f, gBumpLRTab[model->thump_side], gBumpFBTab[model->thump_side]); break;
                }
            }
        }
    }
}

static void _CheckForces(struct tModelData *model)
{
    signed short i;
    for (i = 0; i < 4; i++)
        _GetForceAndPeak(i, model->slot, model->bodyForce[i], 5000.0f);
    _GetForceAndPeak(4, model->slot, model->centerForceLast, 5000.0f);
}

static void _GetForceAndPeak(signed short index, int p, float *vec, float threshold)
{
    if (gBump[p][index].bump) return;
    if (gBump[p][index].time && gBump[p][index].peak == 0.0f) {
        if (vec[0] == 0.0f && vec[1] == 0.0f && vec[2] == 0.0f) {
            if (gSeconds - gBump[p][index].time > 0.5f)
                gBump[p][index].time = 0.0f;
        } else gBump[p][index].time = gSeconds;
        return;
    }
    if (vec[0] != 0.0f || vec[1] != 0.0f || vec[2] != 0.0f) {
        float force = sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
        if (threshold < force && force > gBump[p][index].peak) {
            gBump[p][index].peak = force;
            gBump[p][index].peakVec[0] = vec[0];
            gBump[p][index].peakVec[1] = vec[1];
            gBump[p][index].peakVec[2] = vec[2];
            gBump[p][index].time = gSeconds;
        }
    }
    if (gBump[p][index].time && gSeconds - gBump[p][index].time > 0.1f) {
        gBump[p][index].bump = 1;
        gBump[p][index].time = gSeconds;
    }
}

static void _InitBumps(signed short p)
{
    int i;
    for (i = 0; i < 5; i++) {
        gBump[p][i].peak = 0.0f;
        gBump[p][i].time = 0.0f;
        gBump[p][i].bump = 0;
    }
    AudStopEffect(gScrapes[p]);
    gScrapes[p] = -1;
    gScrapeState[p] = 0;
    gScrapeTime[p] = 0.0f;
    gBumpTime[p] = 0.0f;
    gThumpTime[p] = 0.0f;
}

void StopCarSounds(struct tModelData *model)
{
    int i;
    int p = model->slot;
    if (gCarActive[p]) {
        gCarActive[p] = 0;
        gCarUpdatedLastTime[p] = !(p & 1);
        for (i = 0; i < 2; i++) {
            if (gEngineSounds[p].state[i].sound != -1)
                _StopSound(&gEngineSounds[p].state[i], model->drone_type != 2);
        }
        if (gEngineSounds[p].hum.sound != -1)
            _StopSound(&gEngineSounds[p].hum, model->drone_type != 2);
        if (gEngineSounds[p].rev.sound != -1)
            _StopSound(&gEngineSounds[p].rev, model->drone_type != 2);
        _StopSound(&gPaveSkidSounds[p], model->drone_type != 2);
        if (model->drone_type == 2) {
            _StopSound(&gRoadNoises[p].dirt, 0);
            _StopSound(&gRoadNoises[p].water, 0);
            _StopSound(&gRoadNoises[p].pave, 0);
            AudStopEffect(gScrapes[p]);
            gScrapes[p] = -1;
            gScrapeState[p] = 0;
            gScrapeTime[p] = 0.0f;
            AudStopEffect(gHornSounds[p]);
            gHornSounds[p] = -1;
            AudStopEffect(gBogSounds[p]);
            gBogSounds[p] = -1;
        }
    }
}

int PlayTargetSound(int fxid, int slot, float *pos, unsigned char priority)
{
    int bumpIndex;
    float pos2[3];
    float delta[3];
    float sq_mag;
    float vec1;
    float vec2;
    float vec3;
    struct tCarData *gc = &gGameCar[slot];
    if (!gGameSoundsEnabled) return -1;
    if (gLink[slot].owner != 6) return -1;
    delta[0] = pos[0] - gc->dr_pos[0];
    delta[1] = pos[1] - gc->dr_pos[1];
    delta[2] = pos[2] - gc->dr_pos[2];
    BodyVector(delta, pos2, gc->dr_uvs);
    bumpIndex = 0;
    vec1 = pos2[2] * pos2[2];
    vec2 = pos2[0] * pos2[0];
    sq_mag = vec1 + vec2;
    if (sq_mag > 1.0f) {
        vec1 *= pos2[2] >= 0.0f ? 1 : -1;
        vec2 *= pos2[0] >= 0.0f ? 1 : -1;
        vec3 = (0.924f * 0.924f) * sq_mag;
        if (vec2 > vec3) bumpIndex |= 4;
        else if (vec2 < -vec3) bumpIndex |= 8;
        if (vec1 > vec3) bumpIndex |= 1;
        else if (vec1 < -vec3) bumpIndex |= 2;
    }
    if (fxid == 59)
        return AudPlayEffectVP(fxid, slot, 1, priority, 1.0f, 0.0f, 0.0f);
    return AudPlayEffectVP(fxid, slot, 1, priority, 1.0f,
                           gBumpLRTab[bumpIndex], gBumpFBTab[bumpIndex]);
}

/* The final two bodies retain their original audiodebug.c call boundaries. */
#pragma dont_inline on
void _InitSoundState(anon_sound_vol_14 *s)
{
    s->sound = -1;
    s->vol = sUninitializedSoundValue[0];
    s->pitch = sUninitializedSoundValue[0];
    s->lrpan = sUninitializedSoundValue[0];
    s->fbpan = sUninitializedSoundValue[0];
}

void _StopSound(anon_sound_vol_14 *s, int emitter)
{
    if (emitter) AudStopEmitter(s->sound);
    else AudStopEffect(s->sound);
    _InitSoundState(s);
}
#pragma dont_inline reset
