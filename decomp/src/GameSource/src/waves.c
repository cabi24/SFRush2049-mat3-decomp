/* waves.c - wave resources, deferred deletion, and WAD model relocation. Status: in progress.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\waves.c; assembly: build/2049LE/asm/GameSource/src/waves.s */
#include "rush/rush_protos.h"

static int _NewWaveInstance(int waveNum, int handle, void * heap);
static void _ReplaceWaveInstance(int idx, int newWave);
static void _SetWaveInstanceModel(int idx);
static int _FindWaveInstance(int waveNum, signed char handle);
static void _ValidateWaveInstance(int idx, int loaded, int ready);
static void _ReadWave(int idx);
static int _BGLoadWaveCheck(int wait);
static void _SetupWaveModel(int idx, int first, int loadTextures);

static char *gWaveFilenames[315] = {
    "texte.lzs", "texts.lzs", "texti.lzs", "textg.lzs", "textd.lzs", "textf.lzs", "HighScore.rom",
    "Select.rom", "debugfonttex.lzs", "digitlgfonttex.lzs", "digitmdfonttex.lzs",
    "digitsmfonttex.lzs", "digitxlfonttex.lzs", "hudlgfonttex.lzs", "hudmdfonttex.lzs",
    "hudsmfonttex.lzs", "hudxlfonttex.lzs", "hudxxlfonttex.lzs", "menulgfonttex.lzs",
    "menumdfonttex.lzs", "menusmfonttex.lzs", "menutitlefonttex.lzs", "pakfonttex.lzs",
    "testfonttex.lzs", "debugfontdat.lzs", "digitlgfontdat.lzs", "digitmdfontdat.lzs",
    "digitsmfontdat.lzs", "digitxlfontdat.lzs", "hudlgfontdat.lzs", "hudmdfontdat.lzs",
    "hudsmfontdat.lzs", "hudxlfontdat.lzs", "hudxxlfontdat.lzs", "menulgfontdat.lzs",
    "menumdfontdat.lzs", "menusmfontdat.lzs", "menutitlefontdat.lzs", "pakfontdat.lzs",
    "testfontdat.lzs", "attract.lzs", "interface.lzs", "selcar.lzs", "selcirc.lzs", "selcom.lzs",
    "selopt.lzs", "seltrk.lzs", "hud.lzs", "hudbattle.lzs", "hudcom.lzs", "hudlogo.lzs",
    "hudmulti.lzs", "hudstunt.lzs", "targetsnopak.lzs", "hiscore.lzs", "weapons.lzs", "wings.lzs",
    "vehicles.lzs", "targetst1.lzs", "targetst2.lzs", "targetst3.lzs", "targetst4.lzs",
    "targetst5.lzs", "targetst6.lzs", "car1.lzs", "car2.lzs", "car3.lzs", "car4.lzs", "car5.lzs",
    "car6.lzs", "car7.lzs", "car8.lzs", "car9.lzs", "car10.lzs", "car11.lzs", "car12.lzs",
    "car13.lzs", "car1pj1.lzs", "car1pj2.lzs", "car1pj3.lzs", "car1pj4.lzs", "car1pj5.lzs",
    "car1pj6.lzs", "car1pj7.lzs", "car1pj8.lzs", "car1pj9.lzs", "car1pj10.lzs", "car1pj11.lzs",
    "car1pj12.lzs", "car2pj1.lzs", "car2pj2.lzs", "car2pj3.lzs", "car2pj4.lzs", "car2pj5.lzs",
    "car2pj6.lzs", "car2pj7.lzs", "car2pj8.lzs", "car2pj9.lzs", "car2pj10.lzs", "car2pj11.lzs",
    "car2pj12.lzs", "car3pj1.lzs", "car3pj2.lzs", "car3pj3.lzs", "car3pj4.lzs", "car3pj5.lzs",
    "car3pj6.lzs", "car3pj7.lzs", "car3pj8.lzs", "car3pj9.lzs", "car3pj10.lzs", "car3pj11.lzs",
    "car3pj12.lzs", "car4pj1.lzs", "car4pj2.lzs", "car4pj3.lzs", "car4pj4.lzs", "car4pj5.lzs",
    "car4pj6.lzs", "car4pj7.lzs", "car4pj8.lzs", "car4pj9.lzs", "car4pj10.lzs", "car4pj11.lzs",
    "car4pj12.lzs", "car5pj1.lzs", "car5pj2.lzs", "car5pj3.lzs", "car5pj4.lzs", "car5pj5.lzs",
    "car5pj6.lzs", "car5pj7.lzs", "car5pj8.lzs", "car5pj9.lzs", "car5pj10.lzs", "car5pj11.lzs",
    "car5pj12.lzs", "car6pj1.lzs", "car6pj2.lzs", "car6pj3.lzs", "car6pj4.lzs", "car6pj5.lzs",
    "car6pj6.lzs", "car6pj7.lzs", "car6pj8.lzs", "car6pj9.lzs", "car6pj10.lzs", "car6pj11.lzs",
    "car6pj12.lzs", "car7pj1.lzs", "car7pj2.lzs", "car7pj3.lzs", "car7pj4.lzs", "car7pj5.lzs",
    "car7pj6.lzs", "car7pj7.lzs", "car7pj8.lzs", "car7pj9.lzs", "car7pj10.lzs", "car7pj11.lzs",
    "car7pj12.lzs", "car8pj1.lzs", "car8pj2.lzs", "car8pj3.lzs", "car8pj4.lzs", "car8pj5.lzs",
    "car8pj6.lzs", "car8pj7.lzs", "car8pj8.lzs", "car8pj9.lzs", "car8pj10.lzs", "car8pj11.lzs",
    "car8pj12.lzs", "car9pj1.lzs", "car9pj2.lzs", "car9pj3.lzs", "car9pj4.lzs", "car9pj5.lzs",
    "car9pj6.lzs", "car9pj7.lzs", "car9pj8.lzs", "car9pj9.lzs", "car9pj10.lzs", "car9pj11.lzs",
    "car9pj12.lzs", "car10pj1.lzs", "car10pj2.lzs", "car10pj3.lzs", "car10pj4.lzs", "car10pj5.lzs",
    "car10pj6.lzs", "car10pj7.lzs", "car10pj8.lzs", "car10pj9.lzs", "car10pj10.lzs",
    "car10pj11.lzs", "car10pj12.lzs", "car11pj1.lzs", "car11pj2.lzs", "car11pj3.lzs",
    "car11pj4.lzs", "car11pj5.lzs", "car11pj6.lzs", "car11pj7.lzs", "car11pj8.lzs", "car11pj9.lzs",
    "car11pj10.lzs", "car11pj11.lzs", "car11pj12.lzs", "car12pj1.lzs", "car12pj2.lzs",
    "car12pj3.lzs", "car12pj4.lzs", "car12pj5.lzs", "car12pj6.lzs", "car12pj7.lzs", "car12pj8.lzs",
    "car12pj9.lzs", "car12pj10.lzs", "car12pj11.lzs", "car12pj12.lzs", "car13pj1.lzs",
    "car13pj2.lzs", "car13pj3.lzs", "car13pj4.lzs", "car13pj5.lzs", "car13pj6.lzs", "car13pj7.lzs",
    "car13pj8.lzs", "car13pj9.lzs", "car13pj10.lzs", "car13pj11.lzs", "car13pj12.lzs",
    "track1.lzs", "track2.lzs", "track3.lzs", "track4.lzs", "track5.lzs", "track6.lzs", "dm1.lzs",
    "dm2.lzs", "dm3.lzs", "dm4.lzs", "dm5.lzs", "dm6.lzs", "dm7.lzs", "dm8.lzs", "stunt1.lzs",
    "stunt2.lzs", "stunt3.lzs", "stunt4.lzs", "obstacle1.lzs", "track1worlds.lzs",
    "track2worlds.lzs", "track3worlds.lzs", "track4worlds.lzs", "track5worlds.lzs",
    "track6worlds.lzs", "dm1worlds.lzs", "dm2worlds.lzs", "dm3worlds.lzs", "dm4worlds.lzs",
    "dm5worlds.lzs", "dm6worlds.lzs", "dm7worlds.lzs", "dm8worlds.lzs", "stunt1worlds.lzs",
    "stunt2worlds.lzs", "stunt3worlds.lzs", "stunt4worlds.lzs", "obstacle1worlds.lzs",
    "track1road.lzs", "track2road.lzs", "track3road.lzs", "track4road.lzs", "track5road.lzs",
    "track6road.lzs", "dm1road.lzs", "dm2road.lzs", "dm3road.lzs", "dm4road.lzs", "dm5road.lzs",
    "dm6road.lzs", "dm7road.lzs", "dm8road.lzs", "stunt1road.lzs", "stunt2road.lzs",
    "stunt3road.lzs", "stunt4road.lzs", "obstacle1road.lzs", "track1path.lzs", "track2path.lzs",
    "track3path.lzs", "track4path.lzs", "track5path.lzs", "track6path.lzs", "dm1path.lzs",
    "dm2path.lzs", "dm3path.lzs", "dm4path.lzs", "dm5path.lzs", "dm6path.lzs", "dm7path.lzs",
    "dm8path.lzs", "stunt1path.lzs", "stunt2path.lzs", "stunt3path.lzs", "stunt4path.lzs",
    "obstacle1path.lzs", "track1pathback.lzs", "track2pathback.lzs", "track3pathback.lzs",
    "track4pathback.lzs", "track5pathback.lzs", "track6pathback.lzs"
};
static const signed char gWaveModel[315] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static const signed char gWaveReplace[315] = {
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static unsigned int gWaveDelNum[2] = { 0 }; /* explicit `= { 0 }`: the target keeps this object in initialised data (.data/.sdata), not .bss */
signed char gMirrorWaves;
static anon_loaded_ready_14 gWaveInstance[64];
static unsigned int gWaveDelLater[2][1024];

/* 0x8005b344, size 0x4 */
void BeforeWaveDel(void)
{
    
}

/* 0x8005b348, size 0x28 */
void AddWaveDel(unsigned int Index)
{
    gWaveDelNum[1]++; gWaveDelLater[1][gWaveDelNum[1]] = Index;
}

/* 0x8005b370, size 0x144 */
void EndWaveDel(void)
{
    int i; /* r28 */
    if (gWaveDelNum[1] > 512) {
        for (i=1; i<=gWaveDelNum[0]; i++) DeallocateMemory((void *)gWaveDelLater[0][i]);
        for (i=1; i<=gWaveDelNum[1]; i++) gWaveDelLater[0][i] = gWaveDelLater[1][i];
        gWaveDelNum[0] = gWaveDelNum[1]; gWaveDelNum[1] = 0;
    }
}

/* 0x8005b4b4, size 0x194 */
void InitWaves(void)
{
    int i; /* r29 */
    _BGLoadWaveCheck(1);
    while (*(volatile signed char *)&gRenderInProgress) {}
    ZOID_Reset(); InitBlits();
    for (i=0;i<64;i++) {
        if (gWaveInstance[i].ptr && !gWaveInstance[i].locked) {
            if (gWaveInstance[i].wave >= 77 && gWaveInstance[i].wave < 233) UnloadWaveInstance(i,0);
            else UnloadWaveInstance(i,1);
        }
    }
    gNumModels = 0;
    for (i=0;i<64;i++) if (gWaveInstance[i].ptr) gNumModels = i+1;
    InitTextWave();
}

/* 0x8005b648, size 0xa4 */
int LoadWave(int waveNum, int loadAgain, int loadBackground, int handle, void * heap)
{
    int idx; /* r31 */
    if (!loadAgain) {
        idx = _FindWaveInstance(waveNum,handle);
        if (idx >= 0) {
            if (gWaveInstance[idx].model) MirrorWaveInstance(idx);
            return idx;
        }
    }
    idx = _NewWaveInstance(waveNum,handle,heap);
    _ReadWave(idx);
    return idx;
}

/* 0x8005b6ec, size 0x7c */
void ReplaceWave(int curWave, int newWave)
{
    int idx; /* r31 */
    if (_FindWaveInstance(newWave,-1) < 0) {
        idx = _FindWaveInstance(curWave,-1);
        if (idx >= 0) {
            while (*(volatile signed char *)&gRenderInProgress) {}
            _ReplaceWaveInstance(idx,newWave); _ReadWave(idx);
        }
    }
}

/* 0x8005b768, size 0x4c */
int WaveInstanceReady(int idx)
{
    _ValidateWaveInstance(idx,0,0); return gWaveInstance[idx].ready;
}

/* 0x8005b7b4, size 0x48 */
void * GetWaveInstanceAddr(int idx)
{
    _ValidateWaveInstance(idx,0,0); return gWaveInstance[idx].ptr;
}

/* 0x8005b7fc, size 0x9c */
void UnloadWaveAll(int waveNum)
{
    int i; /* r31 */
    anon_loaded_ready_14 *winst; /* r30 */
    winst = gWaveInstance;
    for (i=0;i<64;i++,winst++) {
        if (winst->ptr && winst->wave == waveNum) {
            if (waveNum>=77 && waveNum<233) UnloadWaveInstance(i,0);
            else UnloadWaveInstance(i,1);
        }
    }
}

/* 0x8005b898, size 0x124 */
void UnloadWaveInstance(int idx, int UnloadTextures)
{
    anon_loaded_ready_14 *winst; /* r31 */
    int i; /* r26 */
    winst = &gWaveInstance[idx];
    if (winst->ptr) {
        while (*(volatile signed char *)&gRenderInProgress) {}
        if (winst->model && UnloadTextures) {
            for(i=0;i<gTexDefPtrs[idx].count;i++) {
                if (!(gTexDefPtrs[idx].texDefs[i].xflags & 0x8000)) DeleteTexture(&gTexDefPtrs[idx].texDefs[i]);
            }
        }
        WaitTillDMAEnd(); ReleaseTextureBinFile(idx); ReleaseGeoBinFile(idx); ps2GraphResumeSwappingThread();
        DeallocateMemory(winst->ptr); winst->ptr = 0;
        memset(&gRomObjDefPtrs[idx],0,sizeof(gRomObjDefPtrs[idx]));
        memset(&gTexDefPtrs[idx],0,sizeof(gTexDefPtrs[idx]));
        memset(&gPalDefPtrs[idx],0,sizeof(gPalDefPtrs[idx]));
    }
}

/* 0x8005b9bc, size 0x108 */
static int _NewWaveInstance(int waveNum, int handle, void * heap)
{
    anon_loaded_ready_14 *winst; /* r31 */
    int idx; /* r30 */
    for (idx=0;idx<64;idx++) {
        if (!gWaveInstance[idx].ptr) {
            winst = &gWaveInstance[idx];
            winst->loaded=0; winst->ready=0; winst->locked=0; winst->mirrored=0;
            winst->wave=waveNum; winst->handle=handle;
            if (handle) winst->ptr = AllocateHandleHeap(heap,(((gWaveSizes[waveNum]+2047)&~2047)+31)&~31);
            else winst->ptr = AllocateMemoryHeap(heap,(((gWaveSizes[waveNum]+2047)&~2047)+31)&~31);
            winst->size=GetPtrSize(winst->ptr);
            _SetWaveInstanceModel(idx); return idx;
        }
    }
    return -1;
}

/* 0x8005bac4, size 0xd0 */
static void _ReplaceWaveInstance(int idx, int newWave)
{
    anon_loaded_ready_14 *winst; /* r26 */
    int i; /* r25 */
    winst = &gWaveInstance[idx];
    _ValidateWaveInstance(idx,1,1);
    /* Keep the original mutually exclusive bounds test. */
    if (winst->model && winst->wave < 77 && winst->wave >= 233) {
        for(i=0;i<gTexDefPtrs[idx].count;i++) if (!(gTexDefPtrs[idx].texDefs[i].xflags&0x8000)) DeleteTexture(&gTexDefPtrs[idx].texDefs[i]);
    }
    winst->loaded=0; winst->ready=0; winst->wave=newWave; winst->mirrored=0;
    _SetWaveInstanceModel(idx);
}

/* 0x8005bb94, size 0x44 */
static void _SetWaveInstanceModel(int idx)
{
    if ((gWaveInstance[idx].model = gWaveModel[gWaveInstance[idx].wave]) && idx>=gNumModels) gNumModels=idx+1;
}

/* 0x8005bbd8, size 0x54 */
void LockWaveInstance(int idx)
{
    anon_loaded_ready_14 *winst; /* r31 */
    winst=&gWaveInstance[idx]; _ValidateWaveInstance(idx,0,0); PurgeableMemory(winst->ptr,0); winst->locked=1;
}

/* 0x8005bc2c, size 0x54 */
void UnlockWaveInstance(int idx)
{
    anon_loaded_ready_14 *winst; /* r31 */
    winst=&gWaveInstance[idx]; _ValidateWaveInstance(idx,0,0); PurgeableMemory(winst->ptr,1); winst->locked=0;
}

/* 0x8005bc80, size 0x44 */
int ModelActive(int idx)
{
    anon_loaded_ready_14 *winst; /* r3 */
    winst=&gWaveInstance[idx]; if (!winst->ptr) return 0; if (winst->ready) return winst->model; return 0;
}

/* 0x8005bcc4, size 0x24 */
int FindWaveInstance(int waveNum)
{
    return _FindWaveInstance(waveNum,-1);
}

/* 0x8005bce8, size 0x124 */
static int _FindWaveInstance(int waveNum, signed char handle)
{
    int i; /* r7 */
    for(i=0;i<64;i++) {
        if (gWaveInstance[i].ptr && waveNum==gWaveInstance[i].wave && (handle<0 || handle==gWaveInstance[i].handle)) return i;
    }
    return -1;
}

/* 0x8005be0c, size 0x4 */
static void _ValidateWaveInstance(int idx, int loaded, int ready)
{
    
}

/* 0x8005be10, size 0x15c */
static void _ReadWave(int idx)
{
    int i; /* r26 */
    unsigned int size; /* stack */
    anon_loaded_ready_14 *winst; /* r26 */
    int waveNum; /* r30 */
    char name[32]; /* stack */
    int geoloaded; /* r0 */
    winst=&gWaveInstance[idx]; waveNum=winst->wave; size=winst->size;
    GDFS_LoadFile(gWaveFilenames[waveNum],winst->handle ? *(void **)winst->ptr : winst->ptr,&size,1,0);
    if (!gWaveReplace[waveNum]) ReallocateMemory(winst->ptr,size);
    winst->loaded=1;
    strcpy(name,gWaveFilenames[waveNum]);
    for(i=0;i<strlen(name);i++) if (name[i]=='.') name[i]=0;
    LoadTextureBinFile(idx,name); geoloaded=LoadGeoBinFile(idx,name,waveNum);
    if (gWaveInstance[idx].wave>=77 && gWaveInstance[idx].wave<233) _SetupWaveModel(idx,1,0);
    else _SetupWaveModel(idx,1,1);
    if (geoloaded) ReMapTexToDCDef();
}

/* 0x8005bf6c, size 0x18 */
int GetWaveByIndex(int idx)
{
    return gWaveInstance[idx].wave;
}

/* 0x8005bf84, size 0x8 */
static int _BGLoadWaveCheck(int wait)
{
    return 1;
}

/* 0x8005bf8c, size 0x5f8 */
static void _SetupWaveModel(int idx, int first, int loadTextures)
{
    anon_base_TOCentries_c wad; /* stack */
    unsigned int *WADptr; /* r31 */
    int *TXLDptr; /* stack */
    int TXLDcnt; /* stack */
    unsigned int IMAGptr; /* stack */
    int offset; /* r30 */
    tTexture *ptTexDef; /* r21 */
    tModelDef *ptRomObjDef; /* r21 */
    tBasicTargetPath *ptBasicTargetPath; /* r18 */
    anon_name_start_18 *ptPalDef; /* r18 */
    int i; /* r18 */
    int j; /* r26 */
    int waveNum; /* r0 */
    
    if (!gWaveInstance[idx].model) { gWaveInstance[idx].ready=1; return; }
    if (!first && !gWaveInstance[idx].handle) return;
    if (gWaveInstance[idx].handle) WADptr=*(unsigned int **)gWaveInstance[idx].ptr; else WADptr=(unsigned int *)gWaveInstance[idx].ptr;
    if (first) MBSetupWad(&wad,WADptr);
    else { offset = (int)*(unsigned int **)gWaveInstance[idx].ptr - (int)gWaveInstance[idx].lastPtr; MBOffsetWad(&wad,WADptr,offset); }
    gWaveInstance[idx].lastPtr=*(unsigned char **)gWaveInstance[idx].ptr;
    gRomObjDefPtrs[idx].romObjDefs=MBGetFromWad(&wad,0x4f424844,&gRomObjDefPtrs[idx].count);
    ptRomObjDef=gRomObjDefPtrs[idx].romObjDefs;
    _Enable_swap();
    for(i=0;i<gRomObjDefPtrs[idx].count;ptRomObjDef++,i++) {
        _swap_x32(&ptRomObjDef->radius); _swap_x16(&ptRomObjDef->unused); _swap_x16(&ptRomObjDef->lodCount);
        for(j=0;j<4;j++) {
            _swap_x16(&ptRomObjDef->lod[j].texidx); _swap_x16(&ptRomObjDef->lod[j].flags);
            _swap_x32(&ptRomObjDef->lod[j].lodRad); _swap_x32(&ptRomObjDef->lod[j].displayList);
        }
    }
    gTexDefPtrs[idx].texDefs=MBGetFromWad(&wad,0x54584844,&gTexDefPtrs[idx].count);
    ptTexDef=gTexDefPtrs[idx].texDefs;
    for(i=0;i<gTexDefPtrs[idx].count;ptTexDef++,i++) {
        _swap_x16((char *)ptTexDef+0x10); _swap_x16((char *)ptTexDef+0x12); _swap_x16((char *)ptTexDef+0x16); _swap_x32((char *)ptTexDef+0x18); _swap_x32((char *)ptTexDef+0x1c); _swap_x32((char *)ptTexDef+0x20); _swap_x32((char *)ptTexDef+0x24); _swap_x32((char *)ptTexDef+0x28); _swap_x32((char *)ptTexDef+0x2c);
    }
    gPalDefPtrs[idx].palDefs=MBGetFromWad(&wad,0x504c4844,&gPalDefPtrs[idx].count);
    ptPalDef=gPalDefPtrs[idx].palDefs;
    for(i=0;i<gPalDefPtrs[idx].count;ptPalDef++,i++) {
        if(ptPalDef) { _swap_x16((char *)ptPalDef+0x12); _swap_x32((char *)ptPalDef+0x14); }
    }
    waveNum=gWaveInstance[idx].wave;
    if(gWaveInstance[idx].wave>=233 && gWaveInstance[idx].wave<252) {
        gTargetPaths.paths=MBGetFromWad(&wad,0x50544844,&gTargetPaths.count);
        ptBasicTargetPath=gTargetPaths.paths;
        for(i=0;i<gTargetPaths.count;ptBasicTargetPath++,i++) {
            _swap_x32((char *)ptBasicTargetPath+0x10); _swap_x16((char *)ptBasicTargetPath+0x14); _swap_x16((char *)ptBasicTargetPath+0x16); _swap_x32((char *)ptBasicTargetPath+0x18); _swap_x32((char *)ptBasicTargetPath+0x1c); _swap_x32((char *)ptBasicTargetPath+0x20);
        }
    }
    TXLDptr=MBGetFromWad(&wad,0x54584c44,&TXLDcnt); _swap_x32_array((unsigned char *)&TXLDptr,TXLDcnt*4);
    IMAGptr=(unsigned int)MBGetFromWad(&wad,0x494d4147,0); _swap_x32(&IMAGptr);
    for(i=0;i<gTexDefPtrs[idx].count;i++) if(first && loadTextures) AddTexture(&gTexDefPtrs[idx].texDefs[i]);
    if(first) {
        for(i=0;i<gRomObjDefPtrs[idx].count;i++) {
            for(j=0;j<gRomObjDefPtrs[idx].romObjDefs[i].lodCount;j++) {
                gRomObjDefPtrs[idx].romObjDefs[i].lod[j].displayList=(unsigned int *)((int)gRomObjDefPtrs[idx].romObjDefs[i].lod[j].displayList+(first ? (int)WADptr : offset));
                gRomObjDefPtrs[idx].romObjDefs[i].lod[j].displayList=(unsigned int *)RegisterObjName(gRomObjDefPtrs[idx].romObjDefs[i].name,gRomObjDefPtrs[idx].romObjDefs[i].lod[j].displayList);
                if(!gRomObjDefPtrs[idx].romObjDefs[i].lod[j].displayList && j==1 && gRomObjDefPtrs[idx].romObjDefs[i].lodCount==2) gRomObjDefPtrs[idx].romObjDefs[i].lodCount--;
                waveNum=gWaveInstance[idx].wave;
                if(waveNum>=64 && waveNum<=76) {
                    if(strstr(gRomObjDefPtrs[idx].romObjDefs[i].name,"HOODSHEEN")) {
                        if(gRomObjDefPtrs[idx].romObjDefs[i].lodCount!=1) return;
                        ReCalculateNormal(gRomObjDefPtrs[idx].romObjDefs[i].lod[j].displayList);
                    } else if(strstr(gRomObjDefPtrs[idx].romObjDefs[i].name,"SHEEN")) {
                        if(gRomObjDefPtrs[idx].romObjDefs[i].lodCount!=1) return;
                        ReCalculateNormal(gRomObjDefPtrs[idx].romObjDefs[i].lod[j].displayList);
                    } else if(strstr(gRomObjDefPtrs[idx].romObjDefs[i].name,"BRAKE")) {
                        if(gRomObjDefPtrs[idx].romObjDefs[i].lodCount!=1) return;
                        EnlargeALittle(gRomObjDefPtrs[idx].romObjDefs[i].lod[j].displayList);
                    }
                }
            }
        }
    }
    CheckNullObj();
    waveNum=gWaveInstance[idx].wave;
    if(gWaveInstance[idx].wave>=233 && gWaveInstance[idx].wave<252) {
        int pathOffset=(int)offset;
        for(i=0;i<gTargetPaths.count;i++) {
            gTargetPaths.paths[i].pts=(void *)((int)gTargetPaths.paths[i].pts+(first ? (int)WADptr : (int)pathOffset));
            _swap_x32_array((unsigned char *)gTargetPaths.paths[i].pts,gTargetPaths.paths[i].numPts*68);
            gTargetPaths.paths[i].trigPtr=(void *)((int)gTargetPaths.paths[i].trigPtr+(first ? (int)WADptr : (int)pathOffset));
        }
    }
    gWaveInstance[idx].ready=1; MirrorWaveInstance(idx);

}

/* 0x8005c584, size 0x7c */
void MirrorWaveInstance(int idx)
{
    _ValidateWaveInstance(idx,1,1);
    if (!gWaveInstance[idx].model || gWaveInstance[idx].mirrored == (gMirrorWaves != 0)) return;
    gWaveInstance[idx].mirrored = (gMirrorWaves != 0);
}
