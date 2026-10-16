/* fakegdfs.c - "fake GDFS": file loading from the disc (roms/...) with a load cache. */
#include "rush/rush_protos.h"

static int gNumLoads;
static unsigned int gCacheHits;
static unsigned int gCacheMisses;
static unsigned int gCacheHitSize;
static unsigned int gCacheMissSize;
static void * gCacheHeap;
char g_MyPacketName[32];
static anon_handle_dbl_10 gUnloaded;
static anon_handle_dbl_10 gCache;

/* the original declares _LoadFile without a prototype and calls it with five arguments */
static anon_ll_name_3c **_FindFile(anon_handle_dbl_10 *list, char *filename);
static void *_LoadFile();

/* 0x80013860 */
void GDFS_Init(void)
{
    InitLL(&gCache, 1, 1);
    InitLL(&gUnloaded, 1, 1);
    gCacheHeap = NULL;
    gNumLoads = 0;
    gCacheMisses = 0;
    gCacheHits = 0;
    gCacheMissSize = 0;
    gCacheHitSize = 0;
}

/* 0x800138c0 */
void *GDFS_LoadFile(char *filename, void *buffer, unsigned int *size, int cache, int check)
{
    anon_ll_name_3c **fileh;
    anon_ll_name_3c *file;

    gNumLoads++;
    fileh = cache ? _FindFile(&gCache, filename) : NULL;
    if (!fileh)
        return _LoadFile(filename, buffer, size, cache, check);
    file = *fileh;
    gCacheHits++;
    gCacheHitSize += file->size;
    *size = file->size;
    if (!buffer)
        buffer = AllocateMemoryHeap(NULL, file->size);
    memcpy(buffer, file + 1, file->size); /* data follows the cache header */
    file->numAccesses++;
    file->lastAccess = gNumLoads;
    return buffer;
}

/* 0x800139ac */
static anon_ll_name_3c **_FindFile(anon_handle_dbl_10 *list, char *filename)
{
    anon_ll_name_3c **fileh;

    for (fileh = list->head; fileh; fileh = (*fileh)->ll.next) {
        if (!strcmp((*fileh)->name, filename))
            return fileh;
    }
    return NULL;
}

/* 0x80013a18 */
static void *_LoadFile(char *file, void *buffer, unsigned int *size)
{
    int gdfs;
    char filename[256];
    int ssize;

    sprintf(filename, "roms/%s", file);
    gdfs = appFileOpen(filename, 0);
    ssize = appFileGetSize(gdfs);
    *size = ssize;
    appFileRead(gdfs, buffer, ssize);
    appFileClose(gdfs);
    return buffer;
}
