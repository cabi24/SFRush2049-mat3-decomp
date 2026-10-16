/* tracks.c - staged track loading (geometry, world, collision, path and shared waves).
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\tracks.c; assembly: build/2049LE/asm/GameSource/src/tracks.s */
#include "rush/rush_protos.h"

unsigned char *gTrackTitles[19] = {
    (unsigned char *)"MARINA", (unsigned char *)"HAIGHT", (unsigned char *)"CIVIC", (unsigned char *)"METRO", (unsigned char *)"MISSION", (unsigned char *)"PRESIDIO", (unsigned char *)"STADIUM", (unsigned char *)"MELEE", (unsigned char *)"TUNDRA",
    (unsigned char *)"ATOMIC", (unsigned char *)"DOWNTOWN", (unsigned char *)"PLAZA", (unsigned char *)"ROAD KILL", (unsigned char *)"FACTORY", (unsigned char *)"THE RIM", (unsigned char *)"DISCO", (unsigned char *)"OASIS",
    (unsigned char *)"WAREHOUSE", (unsigned char *)"GAUNTLET"
};
static char *gWorldNames[19] = {
    "TRACK1", "TRACK2", "TRACK3", "TRACK4", "TRACK5", "TRACK6", "DM1", "DM2", "DM3", "DM4", "DM5",
    "DM6", "DM7", "DM8", "STUNT1", "STUNT2", "STUNT3", "STUNT4", "OBSTACLE1"
};
signed char gTrackNum;
static anon_trackInited_track_2c gTrackLoadInfo;

/* 0x80051d68, size 0x63c */
int LoadTrack(int track, int background, int reloading)
{
    anon_base_TOCentries_c wad; /* stack */
    anon_woff_wname_14 *whdr; /* r26 */
    int *worldptr; /* r25 */
    int i; /* r24 */
    tTargetGroup *swapTG; /* r24 */
    tTarget **swapT; /* r24 */

    if (!gTrackLoadInfo.track.loading) {
        gTrackLoadInfo.track.idx = LoadWave(track + 0xe9, 0, background, 0, 0);
        gTrackLoadInfo.track.loading = 1;
    }
    if (!WaveInstanceReady(gTrackLoadInfo.track.idx)) {
        return 0;
    }
    gTrackLoadInfo.track.loaded = 1;
    gTrackLoadInfo.track.inited = 1;
    gTrackLoadInfo.track.ready = 1;
    if (!gTrackLoadInfo.world.loading) {
        gTrackLoadInfo.world.idx = LoadWave(track + 0xfc, 0, background, 0, 0);
        gTrackLoadInfo.world.loading = 1;
    }
    if (!WaveInstanceReady(gTrackLoadInfo.world.idx)) {
        return 0;
    }
    gTrackLoadInfo.world.loaded = 1;
    if (!gTrackLoadInfo.world.ready) {
        worldptr = GetWaveInstanceAddr(gTrackLoadInfo.world.idx);
        MBSetupWad(&wad, worldptr);
        gWorldData = MBGetFromWad(&wad, 'WHDR', 0);
        _Enable_swap();
        _swap_x32(gWorldData);
        WorldTable = (anon_woff_wname_14 *)(gWorldData + 1);
        if (!gTrackLoadInfo.world.inited) {
            for (i = 0, whdr = WorldTable; i < *gWorldData; i++, whdr++) {
                _swap_x32(&whdr->woff);
                whdr->woff += (unsigned int)worldptr;
            }
            gTrackLoadInfo.world.inited = 1;
        }
        gTargetGroups = MBGetFromWad(&wad, 'GDAT', &numgTargetGroups);
        for (i = 0, swapTG = gTargetGroups; i < numgTargetGroups; swapTG++, i++) {
            _swap_x16(&swapTG->parent);
            _swap_x32(&swapTG->minh);
            _swap_x32(&swapTG->maxh);
            _swap_x32(&swapTG->minv);
            _swap_x32(&swapTG->maxv);
            _swap_x16(&swapTG->child[0]);
            _swap_x16(&swapTG->child[1]);
            _swap_x16(&swapTG->child[2]);
            _swap_x16(&swapTG->child[3]);
        }
        gTargetGroupList = MBGetFromWad(&wad, 'GTLD', &numgTargetGroupList);
        for (i = 0, swapT = gTargetGroupList; i < numgTargetGroupList; swapT++, i++) {
            _swap_x32(swapT);
        }
        gTrackLoadInfo.world.ready = 1;
    }
    if (!gTrackLoadInfo.collision.loading) {
        gTrackLoadInfo.collision.idx = LoadWave(track + 0x10f, 0, background, 0, 0);
        gTrackLoadInfo.collision.loading = 1;
    }
    if (!WaveInstanceReady(gTrackLoadInfo.collision.idx)) {
        return 0;
    }
    gTrackLoadInfo.collision.loaded = 1;
    gTrackLoadInfo.collision.inited = 1;
    if (!gTrackLoadInfo.collision.ready) {
        gRoadDat = GetWaveInstanceAddr(gTrackLoadInfo.collision.idx);
        gTrackLoadInfo.collision.ready = 1;
    }
    if (!gTrackLoadInfo.path.loading) {
        gTrackLoadInfo.path.idx = LoadWave(track + (gBackwards ? 0x135 : 0x122), 0, background, 0, 0);
        gTrackLoadInfo.path.loading = 1;
    }
    if (!WaveInstanceReady(gTrackLoadInfo.path.idx)) {
        return 0;
    }
    gTrackLoadInfo.path.loaded = 1;
    gTrackLoadInfo.path.inited = 1;
    if (!gTrackLoadInfo.path.ready) {
        gPathWave = GetWaveInstanceAddr(gTrackLoadInfo.path.idx);
        gTrackLoadInfo.path.ready = 1;
    }
    if (!gTrackLoadInfo.other.loading) {
        gTrackLoadInfo.other.loading = 1;
        if (!(gGameState & 0x100)) {
            if (gRace == 6 || (track >= 6 && track <= 13)) {
                LoadWave(0x37, 0, 0, 0, 0);
            }
            LoadWave(0x35, 0, 0, 0, 0);
            if (track == 18) {
                LoadWave(0x39, 0, 0, 0, 0);
            } else if (track >= 0 && track <= 5) {
                LoadWave(0x39, 0, 0, 0, 0);
                LoadWave(gTrackNum + 0x3a, 0, 0, 0, 0);
            }
            LoadWave(0x38, 0, 0, 0, 0);
            LoadWave(0x2f, 0, 0, 0, 0);
            LoadWave(0x31, 0, 0, 0, 0);
            if (gNumWindows > 1) {
                LoadWave(0x33, 0, 0, 0, 0);
                LoadWave(0x32, 0, 0, 0, 0);
            }
            if (gRace == 6) {
                LoadWave(0x30, 0, 0, 0, 0);
            } else if (gRace == 4) {
                LoadWave(0x34, 0, 0, 0, 0);
            }
        }
        gTrackLoadInfo.other.loaded = 1;
        gTrackLoadInfo.other.ready = 1;
        gTrackLoadInfo.other.inited = 1;
    }
    if (gTrackLoadInfo.trackInited) {
        return 1;
    }
    gTrackLoadInfo.trackInited = 1;
    initTargetStructs((signed short)track);
    SkyInit();
    NewWorld(gWorldNames[track], -1);
    InitEnvs();
    InitTrackDynamicObjs();
    if (!(gGameState & 0x100) && !reloading) {
        LoadCars(0);
    }
    InitVisuals(!(gGameState & 0x100));
    InitCheckpointData();
    InitPathData();
    if (gTrackNum >= 0 && gTrackNum <= 5) {
        InitDemoPathPoint();
    }
    InitSkids();
    InitSmoke();
    InitBlast();
    initWorldDamage();
    return 1;
}

/* 0x800523a4, size 0x3c */
void ReloadTrack(void)
{
    gTrackLoadInfo.trackInited = 0;
    LoadTrack(gTrackNum, 0, 1);
}

/* 0x800523e0, size 0x80 */
void UnloadTrack(void)
{
    WorldClear();
    SkyClear();
    ClearSkids();
    UnloadWaveInstance(gTrackLoadInfo.track.idx, 1);
    UnloadWaveInstance(gTrackLoadInfo.world.idx, 1);
    UnloadWaveInstance(gTrackLoadInfo.collision.idx, 1);
    memset(&gTrackLoadInfo, 0, sizeof(gTrackLoadInfo));
    exitTargetStructs();
    exitVisuals();
}

