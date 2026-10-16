/* ngcTexHelp.c - texture palette (.tpl) loading, the SDK's texPalette.c with the game's loader.
   Original: D:\rush_ngc\Rush_NGC\NGC\ngcTexHelp.c; assembly: build/2049LE/asm/NGC/ngcTexHelp.s */
#include "rush/rush_protos.h"

/* The original's __FILE__ and the __LINE__s below are baked into the code; #line reproduces them. */
#define TEX_PALETTE_VERSION 2142000
#define OSRoundUp32B(x) (((unsigned long)(x) + 31) & ~31)

static void LoadTexPalette(anon_versionNumber_numDescriptors_c **pal, char *name);
static void UnpackTexPalette(anon_versionNumber_numDescriptors_c *pal);
static void TexFreeFunc(anon_versionNumber_numDescriptors_c **pal);

/* 0x800abd48, size 0xb0 */
void NGCTEXGetPalette(anon_versionNumber_numDescriptors_c **pal, char *name)
{
    void * p = (void *)TexFreeFunc; /* r31 */

    if (DOCacheInitialized) {
        *pal = DSGetCacheObj(&DODisplayCache, name);
    }
    if (!*pal) {
        LoadTexPalette(pal, name);
        if (DOCacheInitialized) {
            DSAddCacheNode(&DODisplayCache, name, *pal, p);
            DSGetCacheObj(&DODisplayCache, name);
        }
    }
}

/* 0x800abdf8, size 0xb0 */
static void LoadTexPalette(anon_versionNumber_numDescriptors_c **pal, char *name)
{
    DVDFileInfo dfi; /* stack */
    long sResult; /* r0 */

    DVDOpen(name, &dfi);
    *pal = (anon_versionNumber_numDescriptors_c *)_mem_driver_malloc(OSRoundUp32B(dfi.length));
    enter_cs_read();
    g_bLoadingNow = 1;
    sResult = DVDReadPrio(&dfi, *pal, OSRoundUp32B(dfi.length), 0, 2);
    g_bLoadingNow = 0;
    if (sResult == -1) {
#line 83 "ngcTexHelp.c"
        SetFatalData(__FILE__, __LINE__, __FUNCTION__);
    }
    exit_cs_read();
    DVDClose(&dfi);
    UnpackTexPalette(*pal);
}

/* 0x800abea8, size 0x114 */
static void UnpackTexPalette(anon_versionNumber_numDescriptors_c *pal)
{
    unsigned short i; /* r3 */

    if (pal->versionNumber != TEX_PALETTE_VERSION) {
#line 102 "ngcTexHelp.c"
        OSPanic(__FILE__, __LINE__, "invalid version number for texture palette");
    }
    pal->descriptorArray = (anon_textureHeader_CLUTHeader_8 *)((unsigned long)pal->descriptorArray + (unsigned long)pal);
    for (i = 0; i < pal->numDescriptors; i++) {
        if (pal->descriptorArray[i].textureHeader) {
            pal->descriptorArray[i].textureHeader =
                (anon_height_width_24 *)((unsigned long)pal->descriptorArray[i].textureHeader + (unsigned long)pal);
            if (!pal->descriptorArray[i].textureHeader->unpacked) {
                pal->descriptorArray[i].textureHeader->data =
                    (char *)((unsigned long)pal->descriptorArray[i].textureHeader->data + (unsigned long)pal);
                pal->descriptorArray[i].textureHeader->unpacked = 1;
            }
        }
        if (pal->descriptorArray[i].CLUTHeader) {
            pal->descriptorArray[i].CLUTHeader =
                (anon_numEntries_unpacked_c *)((unsigned long)pal->descriptorArray[i].CLUTHeader + (unsigned long)pal);
            if (!pal->descriptorArray[i].CLUTHeader->unpacked) {
                pal->descriptorArray[i].CLUTHeader->data =
                    (char *)((unsigned long)pal->descriptorArray[i].CLUTHeader->data + (unsigned long)pal);
                pal->descriptorArray[i].CLUTHeader->unpacked = 1;
            }
        }
    }
}

/* 0x800abfbc, size 0x10 */
anon_textureHeader_CLUTHeader_8 *TEXGet(anon_versionNumber_numDescriptors_c *pal, unsigned long id)
{
    return &pal->descriptorArray[id];
}

/* 0x800abfcc, size 0x38 */
static void TexFreeFunc(anon_versionNumber_numDescriptors_c **pal)
{
    _mem_driver_free(*pal);
    *pal = 0;
}

/* 0x800ac004, size 0x58 */
void NGCTEXReleasePalette(anon_versionNumber_numDescriptors_c **pal)
{
    if (DOCacheInitialized) {
        DSReleaseCacheObj(&DODisplayCache, *pal);
    } else {
        _mem_driver_free(*pal);
        *pal = 0;
    }
}

/* 0x800ac05c, size 0xfc (the first parameter is unnamed in the original) */
void NGCTEXGetGXTexObjFromPalette(void *unused, anon_versionNumber_numDescriptors_c *pal, _GXTexObj *to, unsigned long id)
{
    anon_textureHeader_CLUTHeader_8 *tdp; /* r0 */
    unsigned char mipMap; /* r30 */

    tdp = TEXGet(pal, id);
    if (tdp->textureHeader->minLOD == tdp->textureHeader->maxLOD) {
        mipMap = 0;
    } else {
        mipMap = 1;
    }
    g_bLoadingNow = 1;
    enter_cs();
    GXInitTexObj(to, tdp->textureHeader->data, tdp->textureHeader->width, tdp->textureHeader->height,
                 tdp->textureHeader->format, tdp->textureHeader->wrapS, tdp->textureHeader->wrapT, mipMap);
    GXInitTexObjLOD(to, tdp->textureHeader->minFilter, tdp->textureHeader->magFilter, tdp->textureHeader->minLOD,
                    tdp->textureHeader->maxLOD, tdp->textureHeader->LODBias, 0, tdp->textureHeader->edgeLODEnable, 0);
    exit_cs();
    g_bLoadingNow = 0;
}
