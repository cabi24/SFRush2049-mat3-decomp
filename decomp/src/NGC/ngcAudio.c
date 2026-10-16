/* ngcAudio.c - GameCube audio, movie and music control. Status: NonMatching (36/37 independently verified bodies).
Original: D:\rush_ngc\Rush_NGC\NGC\ngcAudio.c; assembly: build/2049LE/asm/NGC/ngcAudio.s */
#define fix_ShFile fix_ShFile_header_decl
#define sndMalloc sndMalloc_header_decl
#define sndPlayerMusicStart sndPlayerMusicStart_header_decl
#include "rush/rush_protos.h"
#undef fix_ShFile
#undef sndMalloc
void fix_ShFile(void *, int);
void *sndMalloc(int, int);
#undef sndPlayerMusicStart
void sndPlayerMusicStart(float volume);

/* SDK/Bink declarations local to this reconstructed TU. */
unsigned long ARGetBaseAddress(void);
unsigned long ARGetSize(void);
void AXRegisterCallback(void (*)(void));
_AXVPB *AXAcquireVoice(unsigned long, void (*)(void *), unsigned long);
anon_type_sampleRate_1c *SPGetSoundEntry(anon_entries_sound_20 *, unsigned long);
void SPPrepareSound(anon_type_sampleRate_1c *, _AXVPB *, unsigned long);
void MIXInitChannel(_AXVPB *, unsigned long, int, int, int, int, int, int);
unsigned long ARAlloc(unsigned long);
unsigned long ARFree(unsigned long *);
unsigned long ARInit(unsigned long *, unsigned long);
void ARQInit(void);
void AMInit(unsigned long, unsigned long);
void AIInit(void *);
void AXInit(void);
void MIXInit(void);
void AXSetCompressor(int);
int BinkSetSoundSystem(void *, void *);
void *BinkOpenAX();
void BinkDoFrame(BINK *);
void BinkCopyToBuffer(BINK *, void *, int, unsigned int, unsigned int, unsigned int, unsigned int);
int BinkWait(BINK *);
void BinkNextFrame(BINK *);
void BinkClose(BINK *);
BINK *BinkOpen(char *, unsigned int);
void BinkSetVolume(BINK *, unsigned int, int);
int BinkPause(BINK *, int);
void MIXReleaseChannel(_AXVPB *);
void AXFreeVoice(_AXVPB *);
void AXSetVoiceState(_AXVPB *, unsigned short);
void AXSetVoiceSrcRatio(_AXVPB *, float);
void AXSetVoiceVe(_AXVPB *, _AXPBVE *);
int OSDisableInterrupts(void);
int OSRestoreInterrupts(int);
unsigned long OSGetSoundMode(void);
void *AMLoadFile(char *, void *);
unsigned long __AMPushBuffered(char *, void *, unsigned long, void *, void *);
unsigned long AMGetZeroBuffer(void);
void SPInitSoundTable(anon_entries_sound_20 *, unsigned long, unsigned long);


static void * aram_malloc(unsigned int num_bytes);
static void aram_free(void);
static void Setup_surface_array(void);
static void Decompress_frame(BINK *bink, RAD3DIMAGE *image, int copy_all);
static _PS2Bank *get_idle_bank(void);
static _PS2Bank *find_loaded_bank(int bank_id);
static void sndInitialize(void);
static void sndMute(void);
static void sndTickChannels(void);
static void ax_ngc_callback(void);
static void ax_drop_voice_callback(void * p);

static char s_strCurMusicName[128] = { 0 };
static _sndLoadInfo s_li = { 0 };
static char s_channels[48] = { 0 };
static int g_bUseBink = 1;
static unsigned long aramAudioBase;
static char s_monoStreams[2];
static RAD3DIMAGE *Image;
int fileInUse;
static _AXVPB *s_MusicAxVpb;
static unsigned long s_MusicAmBase;
static anon_entries_sound_20 *s_MusicSpTable;
static int s_bInit;
static int s_bMusicIsPaused;
BINK *Bink;
anon_ax_voice_sp_entry_8 ngc_voice[64];
static unsigned char xfer_buffer[16384];
static unsigned long aramMemArray[3];
static unsigned int Bink_surface_type[5];

/* 0x800acc38, size 0x20 */
static void * aram_malloc(unsigned int num_bytes)
{
    return (void *)ARAlloc(num_bytes);
}

/* 0x800acc58, size 0x24 */
static void aram_free(void)
{
    unsigned long length;
    ARFree(&length);
}

/* 0x800acc7c, size 0x34 */
static void Setup_surface_array(void)
{
    Bink_surface_type[0] = 3;
    Bink_surface_type[1] = 5;
    Bink_surface_type[2] = 10;
    Bink_surface_type[3] = 7;
    Bink_surface_type[4] = 13;
}

/* 0x800accb0, size 0xb0 */
static void Decompress_frame(BINK *bink, RAD3DIMAGE *image, int copy_all)
{
    void *pixels;
    unsigned int pixel_pitch;
    unsigned int pixel_format;
    BinkDoFrame(bink);
    if (Lock_RAD_3D_image(image, &pixels, &pixel_pitch, &pixel_format)) {
        BinkCopyToBuffer(bink, pixels, pixel_pitch, bink->Height, 0, 0,
        Bink_surface_type[pixel_format] | (copy_all ? 0x80000000 : 0));
        Unlock_RAD_3D_image(image);
    }
}

/* 0x800acd60, size 0x68 */
void GetMovieTex(void)
{
    if (!BinkWait(Bink)) {
        Decompress_frame(Bink, Image, 0);
        BinkNextFrame(Bink);
    }
    GXSetBlendMode(0, 4, 5, 0);
    GXLoadTexObj(&Image->texObj, 0);
}

/* 0x800acdc8, size 0x220 */
void DrawMovieSprite(void)
{
    ngcBlitInit_swap();
    GXSetAlphaCompare(7, 0, 1, 7, 0);
    GXSetZCompLoc(1);
    GXSetZMode(0, 3, 0);
    GXSetBlendMode(1, 4, 5, 0);
    GXSetNumChans(1);
    GXSetTevOp(0, 0);
    GXSetTevOrder(0, 0, 0, 4);
    GXSetChanCtrl(4, 0, 0, 1, 0, 0, 2);
    GXSetCullMode(0);
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1);
    GXSetVtxDesc(11, 1);
    GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0);
    GXSetVtxAttrFmt(0, 11, 1, 5, 0);
    GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    GetMovieTex();
    GXBegin(0x80, 0, 4);
#define VERTEX(x,y,u,v) \
    GXWGFifo.f32 = x; GXWGFifo.f32 = y; GXWGFifo.f32 = 1.0f; \
    GXWGFifo.u8 = 255; GXWGFifo.u8 = 255; GXWGFifo.u8 = 255; GXWGFifo.u8 = 255; \
    GXWGFifo.f32 = u; GXWGFifo.f32 = v
    VERTEX(0.0f, 0.0f, 0.0f, 0.0f);
    VERTEX(0.0f, 480.0f, 0.0f, 1.0f);
    VERTEX(640.0f, 480.0f, 1.0f, 1.0f);
    VERTEX(640.0f, 0.0f, 1.0f, 0.0f);
#undef VERTEX
}

/* 0x800acfe8, size 0xec */
void PlayMovie(void)
{
    int bSkipMovie = 0;
    if (Bink) {
        Image = Open_RAD_3D_image(0, Bink->Width, Bink->Height, 0);
        if (Image) {
            for (;;) {
                ngcBeforeRender();
                if (DRAW_TRC_DVD()) {
                    bSkipMovie = 1;
                    if (Bink) BinkClose(Bink);
                    Bink = 0;
                } else {
                    if (bSkipMovie) break;
                    DrawMovieSprite();
                    if (Bink->FrameNum == Bink->Frames) break;
                    XBX_Input_GetInput();
                    if (just_button_up[0] & 0x100) break;
                    if (just_button_up[0] & 0x1000) break;
                }
                ngcDoneRender();
            }
            Close_RAD_3D_image(Image);
            Image = 0;
        }
        if (Bink) BinkClose(Bink);
        Bink = 0;
    }
}

/* 0x800ad0d4, size 0x48 */
void ngcInitBink(void)
{
    RADARAMCALLBACKS aram_callbacks = {aram_malloc, (void (*)(void *))aram_free};
    AXSetCompressor(0);
    BinkSetSoundSystem(BinkOpenAX, &aram_callbacks);
    Setup_surface_array();
}

/* 0x800ad11c, size 0x5c */
void ngcInitAudio(void)
{
    ARInit(aramMemArray, 3);
    ARQInit();
    aramAudioBase = ARAlloc(0x800000);
    AMInit(aramAudioBase, 0x800000);
    AIInit(0);
    AXInit();
    MIXInit();
    sndInitialize();
    ngcInitBink();
}

/* 0x800ad178, size 0x298 */
int ngcLoadEffectBank(int nBankID, int nSubID, int bLoadingEffect)
{
    char pathname[100];
    int fd;
    int file_size;
    char *p;
    int sb_size;
    _PS2Bank *bnk;
    anon_entries_sound_20 *sp_table;
    unsigned long am_user_base;
    if (find_loaded_bank(nBankID)) return 1;
    bnk = get_idle_bank();
    if (nSubID != -1) sprintf(pathname, "PS2SOUND/banks/bnk_%02d_%02d.sh", nBankID, nSubID);
    else sprintf(pathname, "PS2SOUND/banks/bnk_%02d.sh", nBankID);
    fd = appFileOpen(pathname, 0);
    if (!fd) return 0;
    file_size = (appFileGetSize(fd) + 31) & ~31;
    bnk->bLoadingEffect = bLoadingEffect;
    bnk->pBuf = sndMalloc(file_size, 0);
    appFileRead(fd, bnk->pBuf, file_size);
    appFileClose(fd);
    fix_ShFile(bnk->pBuf, file_size);
    p = bnk->pBuf;
    bnk->nResCount = *(int *)(p + 8);
    p += 12;
    if (bnk->nResCount > 0) {
        bnk->pResources = (_SND_RESOURCE_HEADER *)p;
        p += bnk->nResCount * sizeof(_SND_RESOURCE_HEADER);
    }
    bnk->nEvtCount = *(int *)p;
    p += 4;
    if (bnk->nEvtCount > 0) {
        bnk->pEvents = (_SND_EVENT *)p;
        p += bnk->nEvtCount * sizeof(_SND_EVENT);
    }
    sb_size = *(int *)p;
    if (nSubID != -1) sprintf(pathname, "rush2049_data/NGC_SOUND/bnk_%02d.spt", nBankID, nSubID);
    else sprintf(pathname, "rush2049_data/NGC_SOUND/bnk_%02d.spt", nBankID);
    g_bLoadingNow = 1;
    enter_cs_read();
    sp_table = bnk->pSpTable = AMLoadFile(pathname, 0);
    if (nSubID != -1) sprintf(pathname, "rush2049_data/NGC_SOUND/bnk_%02d.spd", nBankID, nSubID);
    else sprintf(pathname, "rush2049_data/NGC_SOUND/bnk_%02d.spd", nBankID);
    am_user_base = bnk->pAmBase = __AMPushBuffered(pathname, xfer_buffer, 16384, 0, 0);
    exit_cs_read();
    g_bLoadingNow = 0;
    ARGetSize();
    SPInitSoundTable(sp_table, am_user_base, AMGetZeroBuffer());
    if (bnk->bLoadingEffect) bnk->uSpuStartAddr = am_user_base;
    else bnk->uSpuStartAddr = s_li.uNextSpuAddr;
    if (sb_size > 0 && !bnk->bLoadingEffect) s_li.uNextSpuAddr += sb_size;
    bnk->nDiskStartAddr = -1;
    bnk->nBankID = nBankID;
    s_li.aBankSequence[s_li.nNextBankPos++] = bnk;
    return 1;
}

/* 0x800ad410, size 0x48 */
static _PS2Bank *get_idle_bank(void)
{
    int i;
    for (i = 0; i < 30; i++) {
        if (s_li.banks[i].nBankID == -1) return &s_li.banks[i];
    }
    return 0;
}

/* 0x800ad458, size 0x48 */
static _PS2Bank *find_loaded_bank(int bank_id)
{
    int i;
    for (i = 0; i < 30; i++) {
        if (s_li.banks[i].nBankID == bank_id) return &s_li.banks[i];
    }
    return 0;
}

/* 0x800ad4a0, size 0x240 */
static void sndInitialize(void)
{
    int i;
    if (!s_bInit) {
        s_li.uNextSpuAddr = ARGetBaseAddress();
        for (i = 0; i < 30; i++) {
            memset(&s_li.banks[i], 0, sizeof(_PS2Bank));
            s_li.banks[i].nBankID = -1;
            s_li.aBankSequence[i] = 0;
            s_li.banks[i].pSpTable = 0;
            s_li.banks[i].pAmBase = 0;
        }
        /* Reset load state before channel initialization, as in the GC target. */
        s_li.nNextBankPos = 0;
        s_li.bLoadingEffect = 0;
        for (i = 0; i < 4; i++) s_channels[i] = 1;
        for (i = 4; i < 48; i++) s_channels[i] = 0;
        for (i = 0; i < 2; i++) s_monoStreams[i] = 0;
        for (i = 0; i < 64; i++) {
            ngc_voice[i].ax_voice = 0;
            ngc_voice[i].sp_entry = 0;
        }
        AXRegisterCallback(ax_ngc_callback);
        s_bInit = 1;
        sndMute();
    }
}

/* 0x800ad6e0, size 0x4 */
static void sndMute(void)
{

}

/* 0x800ad6e4, size 0x78 */
_SND_RESOURCE_HEADER *sndFindEffectResource(int nID, _PS2Bank **bnk)
{
    int nBankID = nID >> 16;
    int nRefID = nID & 0xffff;
    int i;
    for (i = 0; i < 30; i++) {
        if (s_li.banks[i].nBankID == nBankID) {
            if (bnk) *bnk = &s_li.banks[i];
            return &s_li.banks[i].pResources[nRefID];
        }
    }
    return 0;
}

/* 0x800ad75c, size 0x20 */
void sndTick(void)
{
    sndTickChannels();
}

/* 0x800ad77c, size 0xd0 */
static void sndTickChannels(void)
{
    int i;
    for (i = 0; i < 48; i++) if (s_channels[i] == 2) s_channels[i] = 0;
    for (i = 0; i < 2; i++) if (s_monoStreams[i] == 2) s_monoStreams[i] = 0;
}

/* 0x800ad84c, size 0x7c */
static void ax_ngc_callback(void)
{
    unsigned long i;
    for (i = 0; i < 64; i++) {
        if (ngc_voice[i].ax_voice && !ngc_voice[i].ax_voice->pb.state) {
            MIXReleaseChannel(ngc_voice[i].ax_voice);
            AXFreeVoice(ngc_voice[i].ax_voice);
            ngc_voice[i].ax_voice = 0;
        }
    }
}

/* 0x800ad8c8, size 0x88 */
static void ax_drop_voice_callback(void * p)
{
    unsigned long i;
    for (i = 0; i < 64; i++) {
        if (p == ngc_voice[i].ax_voice) {
            MIXReleaseChannel(ngc_voice[i].ax_voice);
            ngc_voice[i].ax_voice = 0;
            ngc_voice[i].sp_entry = 0;
            break;
        }
    }
}

/* 0x800ad950, size 0x100 */
void ngcLoopSfx(void * snd, void *unused_table, unsigned long unused_sfx, float volume)
{
    anon_used_started_4c *sfxSnd;
    _AXVPB *axVPB;
    float fSampleRatioCovert;
    if (snd) {
        _AXPBVE axPBVE;
        sfxSnd = snd;
        axVPB = sfxSnd->ngcEffect;
        if (!axVPB->pb.type) axVPB->pb.type = 1;
        if (axVPB) {
            fSampleRatioCovert = sfxSnd->sampleRate / 32000.0f;
            fSampleRatioCovert *= sfxSnd->playingPitch;
            AXSetVoiceSrcRatio(axVPB, fSampleRatioCovert);
            axPBVE.currentVolume = 34952.0f * volume * g_SfxVol;
            axPBVE.currentDelta = 0;
            AXSetVoiceVe(axVPB, &axPBVE);
        }
    } else {
        _AXPBVE axPBVE;
        axPBVE.currentVolume = 34952.0f * volume;
        axPBVE.currentDelta = 0;
        if (s_MusicAxVpb) AXSetVoiceVe(s_MusicAxVpb, &axPBVE);
    }
}

/* 0x800ada50, size 0x324 */
void ngcPlaySfx(void * snd, void * _sp_table, unsigned long sfx, float volume)
{
    anon_ax_voice_sp_entry_8 *v;
    int old;
    unsigned long i;
    anon_entries_sound_20 *sp_table;
    _AXPBVE axPBVE;
    int looped;
    unsigned long zero_base;
    unsigned long _zero_base;
    sp_table = _sp_table;
    if (sfx >= sp_table->entries) sfx = 0;
    old = OSDisableInterrupts();
    for (i = 0; i < 64; i++) if (!ngc_voice[i].ax_voice) break;
    v = &ngc_voice[i];
    if (v) {
        v->ax_voice = AXAcquireVoice(15, ax_drop_voice_callback, 0);
        if (v->ax_voice) {
            v->sp_entry = SPGetSoundEntry(sp_table, sfx);
            /* The original uses uninitialized zero_base in these looped cases. */
            switch ((int)v->sp_entry->type) {
                case 0: looped = 0; break;
                case 1: _zero_base = zero_base << 3; looped = 1; break;
                case 4: looped = 0; break;
                case 5: _zero_base = zero_base; looped = 1; break;
                case 2: looped = 0; break;
                case 3: _zero_base = zero_base >> 1; looped = 1; break;
            }
            if (snd) {
                if (!((anon_used_started_4c *)snd)->loop) {
                    if (looped) {
                        v->sp_entry->type &= ~1;
                        v->sp_entry->loopAddr = _zero_base;
                        v->sp_entry->loopEndAddr = 0;
                    }
                } else {
                    if (!looped) {
                        v->sp_entry->type |= 1;
                        v->sp_entry->loopAddr = v->sp_entry->currentAddr;
                        v->sp_entry->loopEndAddr = v->sp_entry->endAddr;
                    }
                }
            } else {
                v->sp_entry->type |= 1;
                v->sp_entry->loopAddr = v->sp_entry->currentAddr;
                v->sp_entry->loopEndAddr = v->sp_entry->endAddr;
            }
            SPPrepareSound(v->sp_entry, v->ax_voice, v->sp_entry->sampleRate);
            MIXInitChannel(v->ax_voice, 0, 0, -960, -960, 64, 127, 0);
            axPBVE.currentVolume = 34952.0f * volume * g_SfxVol;
            axPBVE.currentDelta = 0;
            AXSetVoiceVe(v->ax_voice, &axPBVE);
            if (snd) {
                ((anon_used_started_4c *)snd)->ngcEffect = v->ax_voice;
                ((anon_used_started_4c *)snd)->sampleRate = v->sp_entry->sampleRate;
            } else s_MusicAxVpb = v->ax_voice;
            AXSetVoiceState(v->ax_voice, 1);
            OSRestoreInterrupts(old);
        } else {
            OSRestoreInterrupts(old);
            OSReport("SFX: AX Voice allocation failed.\n");
        }
    } else {
        OSRestoreInterrupts(old);
        OSReport("No free voices in abstraction layer)\n");
    }
}

/* 0x800add74, size 0x54 */
void ngcStopSfx(void * snd, void *unused_table, unsigned long unused_sfx)
{
    int old;
    _AXVPB *axVPB;
    axVPB = ((anon_used_started_4c *)snd)->ngcEffect;
    old = OSDisableInterrupts();
    if (axVPB) AXSetVoiceState(axVPB, 0);
    OSRestoreInterrupts(old);
}

/* 0x800addc8, size 0x90 */
void sndPlayerMusicPrepare(char *name, int sampleRate, int flag2, int flag3)
{
    if (!strstr(name, "Select")) {
        fileInUse = 1;
        if (Bink) {
            BinkClose(Bink);
            Bink = 0;
        }
        Bink = BinkOpen(name, 0);
        g_bUseBink = 1;
    } else {
        ngcPlaySelectEffect();
        g_bUseBink = 0;
    }
    strcpy(s_strCurMusicName, name);
}

/* 0x800ade58, size 0x8 */
int sndPlayerMusicIsReady(void)
{
    return 1;
}

/* 0x800ade60, size 0x20 */
void sndPlayerMusicStart(float volume)
{
    sndPlayerMusicSetVol();
}

/* 0x800ade80, size 0x10 */
int sndPlayerMusicIsEnd(void)
{
    return fileInUse == 0;
}

/* 0x800ade90, size 0x6c */
void sndPlayerMusicSetVol(void)
{
    if (g_bUseBink == 1) {
        if (Bink) BinkSetVolume(Bink, 0, (int)(65536.0f * g_MusicVol));
    } else {
        ngcLoopSfx(0, 0, 0, g_MusicVol);
    }
}

/* 0x800adefc, size 0x60 */
void sndPlayerMusicStop(void)
{
    if (g_bUseBink == 1) {
        fileInUse = 0;
        if (Bink) {
            BinkClose(Bink);
            Bink = 0;
        }
    } else {
        ngcStopSelectEffect();
    }
    strcpy(s_strCurMusicName, "");
}

/* 0x800adf5c, size 0x60 */
void sndResumeMusic(void)
{
    if (g_bUseBink && strcmp(s_strCurMusicName, "")) {
        sndPlayerMusicPrepare(s_strCurMusicName, 0, 0, 0);
        sndPlayerMusicStart(0.0f);
    }
}

/* 0x800adfbc, size 0x80 */
void sndPlayerMusicPause(void)
{
    if (strcmp(s_strCurMusicName, "") && !s_bMusicIsPaused) {
        if (g_bUseBink == 1) {
            if (Bink) BinkPause(Bink, 1);
        } else {
            ngcLoopSfx(0, 0, 0, 0.0f);
        }
        s_bMusicIsPaused = 1;
    }
}

/* 0x800ae03c, size 0x80 */
void sndPlayerMusicUnpause(void)
{
    if (strcmp(s_strCurMusicName, "") && s_bMusicIsPaused) {
        if (g_bUseBink == 1) {
            if (Bink) BinkPause(Bink, 0);
        } else {
            ngcLoopSfx(0, 0, 0, 1.0f);
        }
        s_bMusicIsPaused = 0;
    }
}

/* 0x800ae0bc, size 0xa4 */
/* Erased GC helper: retain only its proven surviving diagnostic literals.
Its executable body is a placeholder and is never accepted as a match. */
static void ngcRefreshDVDStatus(void)
{
    OSReport("Now loading....%3d%%(%d/%d)", 0, 0, 0);
    OSReport("reset is pressed\n");
    OSReport("reset is released\n");
}

void ngcPlayMenuEffect(int id)
{
    switch (id) {
        case 252: AudPlayEffect(1, 0, 1, 0); break;
        case 253: AudPlayEffect(0, 0, 1, 0); break;
        case 254: AudPlayEffect(7, 0, 1, 0); break;
        case 255: AudPlayEffect(2, 0, 1, 0); break;
    }
    AudioMain();
}

/* 0x800ae160, size 0x8c */
int ngcLoadSelectEffect(void)
{
    char pathname[100];
    sprintf(pathname, "rush2049_data/NGC_MUSIC/Select.spt");
    s_MusicSpTable = AMLoadFile(pathname, 0);
    sprintf(pathname, "rush2049_data/NGC_MUSIC/Select.spd");
    s_MusicAmBase = __AMPushBuffered(pathname, xfer_buffer, 16384, 0, 0);
    SPInitSoundTable(s_MusicSpTable, s_MusicAmBase, AMGetZeroBuffer());
    return 1;
}

/* 0x800ae1ec, size 0x30 */
int ngcPlaySelectEffect(void)
{
    ngcPlaySfx(0, s_MusicSpTable, 0, 1.0f);
}

/* 0x800ae21c, size 0x54 */
int ngcStopSelectEffect(void)
{
    int old;
    old = OSDisableInterrupts();
    if (s_MusicAxVpb) {
        AXSetVoiceState(s_MusicAxVpb, 0);
        s_MusicAxVpb = 0;
    }
    OSRestoreInterrupts(old);
}

/* 0x800ae270, size 0x30 */
int ngcGetAxMode(void)
{
    unsigned long mode;
    mode = OSGetSoundMode();
    return mode != 1;
}

/* 0x800ae2a0, size 0x4 */
void ngcSetAxMode(void)
{

}



