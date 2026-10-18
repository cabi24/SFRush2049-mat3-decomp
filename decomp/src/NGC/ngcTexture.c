/* ngcTexture.c - GameCube texture linking and allocation. Status: Matching.
   Original: D:\rush_ngc\Rush_NGC\NGC\ngcTexture.c; assembly: build/2049LE/asm/NGC/ngcTexture.s */
/* Local call interfaces preserve the GC argument-register positions missing
   from the generated declarations. No shared header changes are required. */
#define MakeResident hidden_MakeResident
#define FindPs2Texture hidden_FindPs2Texture
#define FindPs2Texture2 hidden_FindPs2Texture2
#define NGCDC_SendBlit_MENU_RGBA hidden_NGCDC_SendBlit_MENU_RGBA
#include "rush/rush_protos.h"
#undef MakeResident
#undef FindPs2Texture
#undef FindPs2Texture2
#undef NGCDC_SendBlit_MENU_RGBA
void MakeResident(tTextureEx *tex);
tTextureEx *FindPs2Texture(void *unused, char *name);
tTextureEx *FindPs2Texture2(void *unused, char *name);
void NGCDC_SendBlit_MENU_RGBA(void *tex, void *tex_a, int count, void *vertices,
                            unsigned int c1, unsigned int c2, unsigned int c3, unsigned int c4);

static char *tex_alphatest[59] = {
    "SILVERCOIN_M", "SILVERCOIN_R", "GOLDCOIN_M", "GOLDCOIN_R", "BRI_TREE_02", "FLOWERING_TREE",
    "JER_T1_TREE3", "LEAFY_TREE_02", "LUCKYTREE1", "RK_TREE4", "T1_TREE1", "JER_BIGPINEWALL",
    "PALM_01B", "PALM_02B", "PALM_03", "PALM_TREE_02", "PINE_01", "PINE_02", "SPRUCE_01",
    "RK_SCRAPER_01A", "RK_SCRAPER_01B", "RK_SCRAPER_01C", "RK_SCRAPER_01D", "RK_SCRAPER_01E",
    "RK_SCRAPER_01F", "RK_EXPLR_SUPT1A", "RK_EXPLR_SUPT1B", "RK_EXPLR_BEAM1", "RK_EXPLR_BEAM2",
    "AL_LIGHT", "BRI_TREE_03", "DECID2A", "JER_BIGPINESIDE", "T1_TREE5", "TREE_WALL2B",
    "TREE_WALL3B", "MTLPOLE", "R1-1", "R1-1B", "PARKMTR", "KA_ALPBEAM", "T1_TREE0", "T1_TREE2",
    "T1_TREE3", "TMP_BIGPINESIDE", "TMP_BIGPINEWALL", "AFRAME", "N_TROLSIDE5", "STOP64", "CABLE2",
    "ARROW", "CNTDWN1", "CNTDWN2", "CNTDWN3", "B2", "AL_FUTLIGH2", "AL_STRTBLUE", "AL_STRTLIGHT",
    "AL_STRTORNG"
};
static char *tex_dontwritez[66] = {
    "ROKTFLAME", "SHADBLUR", "SPLASH01", "SPLASH02", "SPLASH03", "SPLASH04", "SPLASH05",
    "SPLASH06", "SPLASH07", "SPLASH08", "FIREWALL01", "FIREWALL02", "FIREWALL03", "FIREWALL04",
    "FIREWALL05", "FIREWALL06", "FIREWALL07", "FIREWALL08", "FIREWALL09", "FIREWALL10",
    "SMOKENEW_01", "SMOKENEW_02", "SMOKENEW_03", "SMOKENEW_04", "SMOKENEW_05", "SMOKENEW_06",
    "SMOKENEW_07", "SMOKENEW_08", "GLASS", "HEADLIGHT_GLOW", "EXP01", "EXP02", "EXP03", "EXP04",
    "EXP05", "EXP06", "EXP07", "EXP08", "EXP09", "EXP10", "EXP11", "EXP12", "EXP13", "EXP14",
    "EXP15", "EXP16", "EXP17", "EXP18", "EXP19", "EXP20", "EXP21", "EXP22", "EXP23", "EXP24",
    "EXP25", "EXP26", "EXP27", "EXP28", "EXP29", "EXP30", "EXP_RING", "FIREBALL", "C3_BRAKE2",
    "C1_BRAKE", "BRAKE2", "MHOLELITE"
};
static char *tex_singleside_default[10] = {
    "SILVERCOIN_M", "SILVERCOIN_R", "GOLDCOIN_M", "GOLDCOIN_R", "BUTTON", "BUTTON_SELECT",
    "VAPORS", "BLUEFLAME", "GREENFLAME", "ICON_LIGHT"
};
static char *tex_singleside_track1[44] = {
    "SILVERCOIN_M", "SILVERCOIN_R", "GOLDCOIN_M", "GOLDCOIN_R", "BUTTON", "BUTTON_SELECT",
    "AL_REDLGHT", "AL_TRFLGHT1", "POFAGREE2_0", "POFAGREE2_1", "POFAGREE2_2", "POFAGREE2_3",
    "SUBWAY3", "CONCRETE2", "BRIDMOTC", "BRWINDOW", "GLASS_TL", "BLDGPLNT", "AL_TRFLGHT5",
    "AL_TRFLGHT2", "COMERCIAL02", "RK_WALL_02B", "RK_EXPLR_WALL1", "PLEXIROAD2B", "AL_BARRIER3",
    "AL_COLUN3", "TR_BTMSDE", "TR_LOWRSDE", "TR_ROOF", "TR_ROOFCAB", "TR_SDWWIN", "TR_WINDOW",
    "TR_SIRENS", "UNDERSDE", "UNDERSDE2", "TR_WHEELS1", "SDEWINFRT", "TR_INNRWINU", "TR_SIRENF",
    "TR_INRWIN", "TR_FRTNOSE", "TR_HEADLIGHT", "TR_BIGSDE", "TR_VENTROOF"
};
static char *tex_singleside_track2[16] = {
    "SILVERCOIN_M", "SILVERCOIN_R", "GOLDCOIN_M", "GOLDCOIN_R", "BUTTON", "BUTTON_SELECT", "R1-1B",
    "R1-1", "NOPARKIN", "R2-4", "THINK", "R4-3", "GETOFF", "AL_RDWLL2", "YELCURV4", "GLASS_TL"
};
static char *tex_singleside_track3[19] = {
    "SILVERCOIN_M", "SILVERCOIN_R", "GOLDCOIN_M", "GOLDCOIN_R", "BUTTON", "BUTTON_SELECT",
    "AL_REDLGHT", "AL_TRFLGHT1", "AL_TRFLGHT2", "AL_TRFLGHT3", "AL_TRFLGHT5", "AL_BARRIER1",
    "AL_BARRIER1B", "AL_BARRIER1C", "AL_BARRIER5", "CHEPSRACING", "GLASS", "ROAD3_BROWN",
    "SCOTCONCRETE"
};
static char *tex_singleside_track4[23] = {
    "SILVERCOIN_M", "SILVERCOIN_R", "GOLDCOIN_M", "GOLDCOIN_R", "BUTTON", "BUTTON_SELECT",
    "AL_TRFLGHT5", "AL_TRFLGHT1", "AL_TRFLGHT2", "AL_TRFLGHT3", "R1-1B", "R1-1", "DICKIESBARRIER",
    "GONZO_R_YELL", "PAPYOCTANE", "SLIMJIMDC", "DRAGONDC", "EMBARC_02A", "HARVEYATOMIC",
    "HEADLIGHT_GLOW", "WARN900N", "WARN900FF", "MHOLELITE"
};
static char *tex_singleside_track5[30] = {
    "SILVERCOIN_M", "SILVERCOIN_R", "GOLDCOIN_M", "GOLDCOIN_R", "BUTTON", "BUTTON_SELECT",
    "AL_REDLGHT", "AL_TRFLGHT1", "AL_TRFLGHT3", "AL_TRFLGHT5", "AL_BARRIER5", "AL_BARRIER1",
    "AL_BARRIER1B", "AL_BARRIER1C", "KA_SKY13", "MERRIL", "AL_STNDOOR", "BRI_SKS_05",
    "RK_STAIRS1A", "GREGMA", "BADWALL1", "CONCRETE2", "R2-4", "ASPHALT", "GLASS", "SHINYCYL",
    "MHOLELITE", "MOSTUCCO", "REESER", "SCOTCONCRETE"
};
static char *tex_singleside_track6[12] = {
    "SILVERCOIN_M", "SILVERCOIN_R", "GOLDCOIN_M", "GOLDCOIN_R", "BUTTON", "BUTTON_SELECT",
    "AL_REDLGHT", "AL_TRFLGHT1", "AL_TRFLGHT2", "AL_TRFLGHT3", "AL_TRFLGHT5", "DARK1"
};
static char *tex_singleside_obstalce[10] = {
    "SILVERCOIN_M", "SILVERCOIN_R", "GOLDCOIN_M", "GOLDCOIN_R", "BUTTON", "BUTTON_SELECT",
    "AL_BARRIER1", "AL_BARRIER1B", "AL_BARRIER1C", "ASPHALT"
};
static char *tex_singleside_dm5[10] = {
    "SILVERCOIN_M", "SILVERCOIN_R", "GOLDCOIN_M", "GOLDCOIN_R", "BUTTON", "BUTTON_SELECT",
    "VAPORS", "BLUEFLAME", "GREENFLAME", "BRI_LIGHTHOUSE_"
};
static char *tex_novclamp[7] = {
    "TREE_WALL2B", "TREE_WALL3B", "JER_BIGPINEWALL", "JER_BIGPINESIDE", "TMP_BIGPINEWALL",
    "TMP_BIGPINESIDE", "GREENFLAME2"
};
static char *tex_forcenoalpha[39] = {
    "TREE_WALL2B", "TREE_WALL3B", "JER_BIGPINEWALL", "JER_BIGPINESIDE", "TMP_BIGPINEWALL",
    "TMP_BIGPINESIDE", "SILVERCOIN_M", "SILVERCOIN_R", "GOLDCOIN_M", "GOLDCOIN_R", "CABLE2",
    "STOP64", "R1-1", "R1-1B", "PARKMTR", "RK_SCRAPER_01A", "RK_SCRAPER_01B", "RK_SCRAPER_01C",
    "RK_SCRAPER_01D", "RK_SCRAPER_01E", "RK_SCRAPER_01F", "RK_EXPLR_SUPT1A", "RK_EXPLR_SUPT1B",
    "RK_EXPLR_BEAM1", "RK_EXPLR_BEAM2", "AL_LIGHT", "AL_STRTLIGHT", "AL_STRTBLUE", "DECID2A",
    "TREE_WALL2B", "TREE_WALL3B", "AFRAME", "N_TROLSIDE5", "MTLPOLE", "KA_ALPBEAM", "CNTDWN1",
    "CNTDWN2", "CNTDWN3", "B2"
};
static char *tex_resident_track1[6] = {
    "LUCKYTREE1", "ROAD1_BLUE", "RUSTROAD6", "LANDTEX_03", "JER_BIGPINEWALL", "AKA_KASIDE"
};
static int texidused[900] = { 0 }; /* explicit `= { 0 }`: the target keeps this object in initialised data (.data/.sdata), not .bss */
static int JustLoadedTextureGroup = -1;
int g_bMenuLoaded;
int g_NumTextures;
static char debug_info[1024];
int g_NGCTextureDelLast[315];
_MENU_TEX g_MenuTex_a;
_MENU_TEX g_MenuTex;
_PS2TexWave g_PS2TextureWave[315];
int g_GroupUsed[500];
_PS2TexGroup g_TexGroups[500];
int stat_tex_swap[900];

/* 0x80099bfc, size 0x4 */
void MakeResident(tTextureEx *tex)
{

}

/* 0x80099c00, size 0x44 */
void ReleaseTexID(int id)
{
    if (id >= 0 && id < 900) {
        if (texidused[id]) {
            texidused[id] = 0;
            stat_tex_swap[id] = 0;
        } else {
            return;
        }
        return;
    }
}

/* 0x80099c44, size 0xaa0 */
void LinkWithPS2Texture(char *packet, tTexture *tex)
{
    char tt[64]; /* stack */
    int i; /* r21 */
    int j; /* r21 */
    int k; /* r21 */
    _PS2TexGroup *group; /* r28 */
    char **singleside; /* r27 */
    int numsingle; /* r26 */
    int numresident; /* r25 */
    char **resident; /* r24 */
    k = 0;
    numresident = k;
    if (gTrackNum == 0) {
        singleside = tex_singleside_track1;
        resident = tex_resident_track1;
        numsingle = 44;
        numresident = 0; /* The GC build disables this residency table. */
    } else if (gTrackNum == 1) { singleside = tex_singleside_track2; numsingle = 16; }
    else if (gTrackNum == 2) { singleside = tex_singleside_track3; numsingle = 19; }
    else if (gTrackNum == 3) { singleside = tex_singleside_track4; numsingle = 23; }
    else if (gTrackNum == 4) { singleside = tex_singleside_track5; numsingle = 30; }
    else if (gTrackNum == 5) { singleside = tex_singleside_track6; numsingle = 12; }
    else if (gTrackNum == 18) { singleside = tex_singleside_obstalce; numsingle = 10; }
    else if (gTrackNum == 10) { singleside = tex_singleside_dm5; numsingle = 10; }
    else { singleside = tex_singleside_default; numsingle = 10; }
    sprintf(tt, "%s", tex->name);
    if (JustLoadedTextureGroup >= 0 && JustLoadedTextureGroup < 500) {
        group = &g_TexGroups[JustLoadedTextureGroup];
        for (i = 0; i < group->_NumTex; i++) {
            if (!strcmp(group->_Tex[i].name, tt)) {
                tex->texBase = group->_Tex[i].texBase;
                if (!strcmp(tt, "B2")) {
                    tex->width = group->_Tex[i].width;
                    tex->height = group->_Tex[i].height;
                }
                if (!strcmp(tt, "CONTROLS_SM")) {
                    tex->width = group->_Tex[i].height;
                    tex->height = group->_Tex[i].width;
                }
                if (tex->flags & 1) group->_Tex[i].ps2flags |= 1;
                if (strstr(group->_Tex[i].name, "CNTDWN")) {
                    group->_Tex[i].ps2flags |= 1;
                    tex->flags |= 1;
                }
                for (k = 0; k < 59; k++) {
                    if (!strcmp(tt, tex_alphatest[k])) {
                        group->_Tex[i].ps2flags |= 4;
                        group->_Tex[i].ps2flags |= 1;
                        break;
                    }
                }
                for (j = 0; j < 66; j++) {
                    if (!strcmp(tt, tex_dontwritez[j])) {
                        group->_Tex[i].ps2flags |= 8;
                        break;
                    }
                }
                for (k = 0; k < numsingle; k++) {
                    if (!strcmp(tt, singleside[k])) {
                        group->_Tex[i].ps2flags |= 16;
                        break;
                    }
                }
                for (k = 0; k < 7; k++) {
                    if (!strcmp(tt, tex_novclamp[k])) {
                        group->_Tex[i].ps2flags |= 32;
                        break;
                    }
                }
                for (k = 0; k < 39; k++) {
                    if (!strcmp(tt, tex_forcenoalpha[k])) {
                        group->_Tex[i].ps2flags &= ~1;
                        break;
                    }
                }
                if (!strcmp(tt, "FRONTLIGHT")) {
                    group->_Tex[i].ps2flags |= 64;
                    group->_Tex[i].ps2flags |= 8;
                    group->_Tex[i].ps2flags |= 1;
                }
                if (!strcmp(tt, "SIDELIGHT")) {
                    group->_Tex[i].ps2flags |= 64;
                    group->_Tex[i].ps2flags |= 8;
                    group->_Tex[i].ps2flags |= 1;
                }
                if (!strcmp(tt, "HEADLIGHT_GLOW")) {
                    group->_Tex[i].ps2flags |= 8;
                    group->_Tex[i].ps2flags |= 1;
                }
                if (!strcmp(tt, "ARROW")) {
                    if (gRace == 6) group->_Tex[i].xflags |= 8;
                    tex->flags |= 1;
                }
                if (strstr(tt, "SUNSET")) group->_Tex[i].ps2flags |= 128;
                if (strstr(tt, "BLUSKY")) group->_Tex[i].ps2flags |= 128;
                if (strstr(tt, "DESRTSKY")) group->_Tex[i].ps2flags |= 128;
                if (strstr(tt, "AL_GRNSKY")) group->_Tex[i].ps2flags |= 128;
                if (strstr(tt, "POFAGREE2_")) group->_Tex[i].ps2flags |= 128;
                if (strstr(tt, "AL_STRTLIGHT")) {
                    group->_Tex[i].xflags |= 16;
                    group->_Tex[i].ps2flags |= 1;
                }
                if (strstr(tt, "AL_STRTBLUE")) {
                    group->_Tex[i].xflags |= 16;
                    group->_Tex[i].ps2flags |= 1;
                }
                if (strstr(tt, "AL_STRTORNG")) {
                    group->_Tex[i].xflags |= 16;
                    group->_Tex[i].ps2flags |= 1;
                }
                if (strstr(tt, "AL_FUTLIGH2")) {
                    group->_Tex[i].xflags |= 16;
                    group->_Tex[i].ps2flags |= 1;
                }
                if (strstr(tt, "SHEEN")) group->_Tex[i].ps2flags |= 8;
                if (!strcmp(tt, "MHOLELITE") && gTrackNum == 4) group->_Tex[i].xflags |= 1;
                if (gTrackNum == 0 && !strcmp(tt, "PLEXIROAD2B")) group->_Tex[i].xflags |= 32;
                if (gTrackNum == 0 && !strcmp(tt, "COMERCIAL02")) group->_Tex[i].xflags |= 32;

                for (k = 0; k < numresident; k++) {
                    if (!strcmp(tt, resident[k])) { MakeResident(&group->_Tex[i]); break; }
                }
                group->_Tex[i].flags = tex->flags;
                tex->strip = (int)&group->_Tex[i];
                return;
            }
        }
    }
    for (i = 0; i < 500; i++) {
        if (g_GroupUsed[i]) {
            group = &g_TexGroups[i];
            for (j = 0; j < group->_NumTex; j++) {
                if (!strcmp(group->_Tex[j].name, tt)) {
                    if (tex->flags & 1) group->_Tex[j].ps2flags |= 1;
                if (strstr(group->_Tex[j].name, "CNTDWN")) {
                    group->_Tex[j].ps2flags |= 1;
                    tex->flags |= 1;
                }
                for (k = 0; k < 59; k++) {
                    if (!strcmp(tt, tex_alphatest[k])) {
                        group->_Tex[j].ps2flags |= 4;
                        break;
                    }
                }
                for (k = 0; k < 66; k++) {
                    if (!strcmp(tt, tex_dontwritez[k])) {
                        group->_Tex[j].ps2flags |= 8;
                        break;
                    }
                }
                for (k = 0; k < numsingle; k++) {
                    if (!strcmp(tt, singleside[k])) {
                        group->_Tex[j].ps2flags |= 16;
                        break;
                    }
                }
                for (k = 0; k < 7; k++) {
                    if (!strcmp(tt, tex_novclamp[k])) {
                        group->_Tex[j].ps2flags |= 32;
                        break;
                    }
                }
                for (k = 0; k < 39; k++) {
                    if (!strcmp(tt, tex_forcenoalpha[k])) {
                        group->_Tex[j].ps2flags &= ~1;
                        break;
                    }
                }
                if (!strcmp(tt, "FRONTLIGHT")) {
                    group->_Tex[j].ps2flags |= 64;
                    group->_Tex[j].ps2flags |= 8;
                    group->_Tex[j].ps2flags |= 1;
                }
                if (!strcmp(tt, "SIDELIGHT")) {
                    group->_Tex[j].ps2flags |= 64;
                    group->_Tex[j].ps2flags |= 8;
                    group->_Tex[j].ps2flags |= 1;
                }

                    group->_Tex[j].flags = tex->flags;
                    tex->strip = (int)&group->_Tex[j];
                    return;
                }
            }
        }
    }
    printf("&&&&&&&&&&&&&&&&&&&&&&&&%s\\%s not found!\n", packet, tex->name);
    tex->strip = 0;
}

/* 0x8009a6e4, size 0x134 */
void LinkWithAnotherTexture(tTexture *tex, char *name)
{
    int i; /* r23 */
    int j; /* r22 */
    _PS2TexGroup *group; /* r0 */
    char *texBase; /* r29 */
    texBase = 0;
    for (j = 0; j < 500; j++) {
        if (g_GroupUsed[j]) {
            group = &g_TexGroups[j];
            for (i = 0; i < group->_NumTex; i++) {
                if (!strcmp(group->_Tex[i].name, name)) {
                    texBase = group->_Tex[i].texBase;
                    if (j == JustLoadedTextureGroup) goto found;
                    break;
                }
            }
        }
    }
found:
    if (texBase) {
        for (j = 0; j < 500; j++) {
            if (g_GroupUsed[j]) {
                group = &g_TexGroups[j];
                for (i = 0; i < group->_NumTex; i++) {
                    if (!strcmp(group->_Tex[i].name, tex->name)) {
                        ((tTextureEx *)tex->strip)->texBase = texBase;
                        return;
                    }
                }
            }
        }
    }
}

/* 0x8009a818, size 0x124 */
tTextureEx *FindPs2Texture(void *unused, char *name)
{
    char tt[64]; /* stack */
    int i; /* r28 */
    int j; /* r27 */
    _PS2TexGroup *group; /* r27 */
    sprintf(tt, "%s", name);
    if (JustLoadedTextureGroup >= 0 && JustLoadedTextureGroup < 500) {
        group = &g_TexGroups[JustLoadedTextureGroup];
        for (i = 0; i < group->_NumTex; i++) {
            if (!strcmp(group->_Tex[i].name, tt)) return &group->_Tex[i];
        }
    }
    for (j = 0; j < 500; j++) {
        if (g_GroupUsed[j]) {
            group = &g_TexGroups[j];
            for (i = 0; i < group->_NumTex; i++) {
                if (!strcmp(group->_Tex[i].name, tt)) return &group->_Tex[i];
            }
        }
    }
    return 0;
}

/* 0x8009a93c, size 0x124 */
tTextureEx *FindPs2Texture2(void *unused, char *name)
{
    char tt[64]; /* stack */
    int i; /* r28 */
    int j; /* r27 */
    _PS2TexGroup *group; /* r27 */
    sprintf(tt, "%s", name);
    if (JustLoadedTextureGroup >= 0 && JustLoadedTextureGroup < 500) {
        group = &g_TexGroups[JustLoadedTextureGroup];
        for (i = group->_NumTex - 1; i >= 0; i--) {
            if (!strcmp(group->_Tex[i].name, tt)) return &group->_Tex[i];
        }
    }
    for (j = 0; j < 500; j++) {
        if (g_GroupUsed[j]) {
            group = &g_TexGroups[j];
            for (i = group->_NumTex - 1; i >= 0; i--) {
                if (!strcmp(group->_Tex[i].name, tt)) return &group->_Tex[i];
            }
        }
    }
    return 0;
}

/* 0x8009aa60, size 0x24 */
int CheckAlpha(tTextureEx *tex)
{
    int usealpha;
    if (tex == 0) return 0;
    usealpha = tex->ps2flags & 1;
    usealpha += tex->ps2flags & 8;
    return usealpha;
}

/* 0x8009aa84, size 0x398 */
void LoadTextureBinFile(int waveNum, char *groupname)
{
    int i; /* r7 */
    int fd2; /* r24 */
    int fd3; /* r24 */
    int totalsize; /* r8 */
    int size; /* r25 */
    char *buffer; /* r26 */
    int *iBuffer; /* r7 */
    char name[256]; /* stack */
    char name_alpha[256]; /* stack */
    int groupid; /* r0 */
    _PS2TexGroup *group; /* r28 */
    _GXTexObj *texObjs; /* r25 */
    _GXTexObj *texObjs_a; /* r24 */
    int UseAlphaNum; /* r24 */
    int n; /* r29 */
    sprintf(name, "ngctex_info/%s/NGCTexInfo.bin", groupname);
    fd2 = appFileOpen(name, 0);
    if (fd2) {
        if (g_PS2TextureWave[waveNum].loaded) {
            g_PS2TextureWave[waveNum].loaded++;
            JustLoadedTextureGroup = g_PS2TextureWave[waveNum].group - g_TexGroups;
            appFileClose(fd2);
        } else {
            groupid = GetAnEmptyGroup();
            JustLoadedTextureGroup = groupid;
            group = &g_TexGroups[groupid];
            appFileRead4(fd2, group, 4);
            group->_Tex = AllocateMemoryHeap(0, group->_NumTex * sizeof(tTextureEx));
            appFileReadTex(fd2, group->_Tex, sizeof(tTextureEx), group->_NumTex);
            appFileClose(fd2);
            totalsize = 0;
            for (i = 0; i < group->_NumTex; i++) {
                group->_Tex[i].xflags = 0;
                if (group->_Tex[i].ps2flags & 2) {
                    /* The GC build reuses the now-dead group index for this swap. */
                    groupid = group->_Tex[i].width;
                    group->_Tex[i].width = group->_Tex[i].height;
                    group->_Tex[i].height = groupid;
                }
                totalsize += (int)group->_Tex[i].texhead;
                totalsize += (int)group->_Tex[i].cluthead;
            }
            texObjs = AllocateMemoryHeap(0, group->_NumTex * sizeof(_GXTexObj));
            group->_TexObjs = texObjs;
            sprintf(name, "rush2049_data/ngctex_info/%s/NGCTexFile.tpl", groupname);
            group->_TexTpl = 0;
            NGCTEXGetPalette((tTPLHeader **)&group->_TexTpl, name);
            for (i = 0; i < group->_NumTex; i++) {
                NGCTEXGetGXTexObjFromPalette(name, group->_TexTpl, &texObjs[i], i);
                group->_Tex[i].texBase = (char *)&texObjs[i];
            }
            g_PS2TextureWave[waveNum].group = group;
            g_PS2TextureWave[waveNum].loaded = 1;
            g_NumTextures += group->_NumTex;
            sprintf(name, "ngctex_info/%s/NGCTexFile_ai.dat", groupname);
            fd3 = appFileOpen(name, 0);
            size = appFileGetSize(fd3);
            buffer = AllocateMemoryHeap(0, size);
            appFileRead4Array(fd3, buffer, size);
            appFileClose(fd3);
            iBuffer = (int *)buffer;
            UseAlphaNum = 0;
            for (i = 0; i < group->_NumTex; i++) {
                group->_Tex[i].psm = *iBuffer++;
                if (group->_Tex[i].psm != 0xffff) UseAlphaNum++;
            }
            DeallocateMemory(buffer);
            if (UseAlphaNum) {
                n = 0;
                texObjs_a = AllocateMemoryHeap(0, UseAlphaNum * sizeof(_GXTexObj));
                group->_TexObjs_a = texObjs_a;
                sprintf(name_alpha, "rush2049_data/ngctex_info/%s/NGCTexFile_a.tpl", groupname);
                group->_TexTpl_a = 0;
                NGCTEXGetPalette((tTPLHeader **)&group->_TexTpl_a, name_alpha);
                for (i = 0; i < group->_NumTex; i++) {
                    if (group->_Tex[i].psm != 0xffff) {
                        NGCTEXGetGXTexObjFromPalette(name_alpha, group->_TexTpl_a, &texObjs_a[n], n);
                        group->_Tex[i].texhead = (_PS2TexHead *)&texObjs_a[n];
                        n++;
                    }
                }
            } else {
                group->_TexObjs_a = 0;
                group->_TexTpl_a = 0;
            }
        }
    }
}

/* 0x8009ae1c, size 0x114 */
void ReleaseTextureBinFile(int waveNum)
{
    int i; /* r27 */
    _PS2TexGroup *group; /* r29 */
    if (g_PS2TextureWave[waveNum].loaded) {
        if (g_PS2TextureWave[waveNum].loaded > 1) {
            g_PS2TextureWave[waveNum].loaded--;
        } else {
            group = g_PS2TextureWave[waveNum].group;
            g_GroupUsed[group - g_TexGroups] = 0;
            for (i = 0; i < group->_NumTex; i++)
                ReleaseTexID(group->_Tex[i].palIndex);
            NGCTEXReleasePalette((tTPLHeader **)&group->_TexTpl);
            AddWaveDel((unsigned int)group->_TexObjs);
            if (group->_TexTpl_a)
                NGCTEXReleasePalette((tTPLHeader **)&group->_TexTpl_a);
            if (group->_TexObjs_a)
                AddWaveDel((unsigned int)group->_TexObjs_a);
            DeallocateMemory(group->_Tex);
            g_NumTextures -= group->_NumTex;
            group->_NumTex = 0;
            g_PS2TextureWave[waveNum].loaded = 0;
        }
    }
}

/* 0x8009af30, size 0x44 */
void PS2InitTexWave(void)
{
    memset(g_PS2TextureWave, 0, sizeof(g_PS2TextureWave));
    memset(g_GroupUsed, 0, sizeof(g_GroupUsed));
}

/* 0x8009af74, size 0x4c */
int GetAnEmptyGroup(void)
{
    int i; /* r5 */
    for (i = 0; i < 500; i++) {
        if (!g_GroupUsed[i]) {
            g_GroupUsed[i] = 1;
            return i;
        }
    }
    /* Original falls through without defining a return value when full. */
}

/* Erased diagnostic placeholder, stripped by the linker. Its executable body is not reconstructed or
   counted; this retains the original pooled @1118 diagnostic literal. */
static void PrintTop10SwappedTex(void)
{
    printf("the number %d tex is:%s\n", stat_tex_swap[0],
           g_PS2TextureWave[0].group->_Tex[0].name);
}

/* 0x8009afc0, size 0xd8 */
void ngcWriteTexelRGBA8(void * data, int w, int h, int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    unsigned char *ar;
    unsigned char *gb;
    if (x / 4 >= w / 4 || y / 4 >= h / 4) return;
    /* Keep the original signed divisions and the separate coordinate guards. */
    if (x / 4 < 0 || y / 4 < 0) return;
    {
        ar = (unsigned char *)data + (((x / 16 + (y / 16) * (w / 16)) * 64) + (((y / 4) % 4) * 8) + (((x / 4) % 4) * 2));
        gb = ar + 32;
        ar[0] = a;
        ar[1] = r;
        gb[0] = g;
        gb[1] = b;
    }
}

/* 0x8009b098, size 0xbc */
_GXTexObj *ngcCreateTextureMapRGBA8(int width, int height, unsigned char **pImage)
{
    int size; /* r31 */
    unsigned char *image; /* r31 */
    _GXTexObj *tex_obj; /* r30 */
    size = (height / 16 + 1) * ((width / 16 + 1) * 4) * 16;
    tex_obj = AllocateMemoryHeap(0, sizeof(_GXTexObj));
    image = AllocateMemoryHeap(0, size);
    *pImage = image;
    memset(image, 0, size);
    GXInitTexObj(tex_obj, image, width / 4, height / 4, 6, 0, 0, 0);
    return tex_obj;
}

/* 0x8009b154, size 0x74 */
void ngcFlushTexObjDataRGBA8(void * obj, void * data, int w, int h)
{
    w /= 4;
    h /= 4;
    GXInitTexObj(obj, data, w, h, 6, 0, 0, 0);
    DCFlushRange(data, w * h * 4);
}

/* 0x8009b1c8, size 0x104 */
int LoadMenuTexData(void)
{
    char name[128]; /* stack */
    int i; /* r29 */
    if (!g_bMenuLoaded) {
        sprintf(name, "rush2049_data/ngctex_info/rush_pau.tpl");
        g_MenuTex._TexTpl = 0;
        NGCTEXGetPalette((tTPLHeader **)&g_MenuTex._TexTpl, name);
        for (i = 0; i < 5; i++)
            NGCTEXGetGXTexObjFromPalette(name, g_MenuTex._TexTpl, &g_MenuTex._TexObjs[i], i);
        sprintf(name, "rush2049_data/ngctex_info/rush_pau_a.tpl");
        g_MenuTex_a._TexTpl = 0;
        NGCTEXGetPalette((tTPLHeader **)&g_MenuTex_a._TexTpl, name);
        for (i = 0; i < 5; i++)
            NGCTEXGetGXTexObjFromPalette(name, g_MenuTex_a._TexTpl, &g_MenuTex_a._TexObjs[i], i);
        g_bMenuLoaded = 1;
    }
    return 0;
}

/* 0x8009b2cc, size 0xd4 */
int MENU_SendBlit(KMVERTICES *pVer, int nTex, int enable)
{
    if (!nTex) {
        NGCDC_SendBlit_MENU_RGBA(0, 0, 4, pVer, 0x2b4b64a0, 0x2b4b8ca0, 0x2b4b8ca0, 0x2b4b64a0);
        return 0;
    }
    if (enable)
        NGCDC_SendBlit_MENU(&g_MenuTex._TexObjs[nTex], &g_MenuTex_a._TexObjs[nTex], 4, pVer, 0xffffffff);
    else
        NGCDC_SendBlit_MENU(&g_MenuTex._TexObjs[nTex], &g_MenuTex_a._TexObjs[nTex], 4, pVer, 0x88888888);
    return 0;
}

