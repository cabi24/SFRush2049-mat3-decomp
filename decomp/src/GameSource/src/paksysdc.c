/* paksysdc.c - memory-card file management. Status: Matching.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\paksysdc.c; assembly: build/2049LE/asm/GameSource/src/paksysdc.s */
#define PS_ReadPartialFile PS_ReadPartialFile_dwarf_decl
#define PS2McCreateFileReader PS2McCreateFileReader_dwarf_decl
#define PS2McCreateFileWriter PS2McCreateFileWriter_dwarf_decl
#include "rush/rush_protos.h"
#undef PS_ReadPartialFile
#undef PS2McCreateFileReader
#undef PS2McCreateFileWriter
void PS_ReadPartialFile();
int PS2McCreateFileReader();
int PS2McCreateFileWriter();


/* GC DWARF: size 0x100; info 0x40, bfh 0x80, icon 0xe0,
   psfile 0xe4. The generated type currently omits both reserved regions. */
typedef struct PakFile {
    tDLLE ll;
    signed char exists, locked, flush, partial;
    unsigned char reserved_c[0x34];
    anon__Create__Modify_40 info;
    BUS_BACKUPFILEHEADERSTRUET bfh;
    char **icon;
    tPS_File **psfile;
    unsigned char reserved_e8[0x18];
} PakFile;

static int _CompleteCallback(void);
static int _SamePak(int pakInMem, int pakInPort);
static void _RemovePak(int pak);
static void _ReadDirectory(int pak);
static void _GetFileExt(char *name, char *ext);
static int _DeleteFile(int pak, char *name);
static void _ClearFlush(int pak);
static int _PS_FlushFile(tPS_File **psfileh, int ignoreErrors);
static void _ClearPak(unsigned char pak);

char *gPS_PakName[4] = { "SLOT 1-A", "SLOT 1-B", "SLOT 1-C", "SLOT 1-D" };
char *gPS_PakNameNoMTP[4] = { "SLOT 1", "SLOT 2", "SLOT 3", "SLOT 4" };
char *gPS_ContName[4] = { "A", "A", "B", "B" };
static signed char gRumble = 1;
static signed char gErrPak = -1;
static void (*gErrorFunc)(int, enum_kPS_ErrUnknown_2, void *, int, signed char *, signed char *, int (*)(int));
static int s_bPakInied;
static signed char gInited;
static int HasFirstPlay;
unsigned char rwbuff[1024];
anon__Create__Modify_40 _IdFileInfo;
/* Split target .bss has sh_addralign=64; table begins at 0x9c0. */
static anon__Create__Modify_40 table[99] __attribute__((aligned(64)));
tDLList gPS_Files[4];
anon_changed_inserted_25c gPaks[4];

/* 0x8002ef58, size 0xac */
void PS_InitPakSys(void (*errorFunc)(int, enum_kPS_ErrUnknown_2, void *, int, signed char *, signed char *, int (*)(int)))
{
    int i;
    PS2MCInit();
    gErrorFunc = errorFunc;
    for (i = 0; i < 4; i++) {
        InitLL(&gPaks[i].files, 1, 1);
        InitLL(&gPS_Files[i], 1, 1);
        gPaks[i].damaged = 0;
        _ClearPak((unsigned char)i);
    }
    _CompleteCallback();
    gInited = 1;
}

/* 0x8002f004, size 0x8 */
static int _CompleteCallback(void)
{
    return 1;
}

/* 0x8002f00c, size 0x4f8 */
void PS_PakServer(void)
{
    int i;
    int j;
    signed char changed[4];
    signed char dup;
    signed char same;
    tPS_File **psfileh;
    if (!gInited) return;
    for (i = 0; i < 4; i++) changed[i] = 0;
    for (i = 0; i < 4; i++) {
        if (gErrPak >= 0 && i != gErrPak) continue;
        if (!gPaks[i].inserted || !gPaks[i].formatted || gPaks[i].wasRemoved) {
            if (gPaks[i].inserted && gPaks[i].formatted) {
                changed[i] = 1;
                if (gPaks[i].wasRemoved) {
                    changed[i] = PS_PakNotTheSame(i);
                    gPaks[i].wasRemoved = changed[i];
                }
                if (gPaks[i].inserted) {
                    gPaks[i].inserted = 1;
                    gPaks[i].damaged = 0;
                    if (changed[i]) gPaks[i].original = 0;
                    else gPaks[i].original = 1;
                }
                if (!gPaks[i].locked && !gPaks[i].flush && !gPaks[i].flushing &&
                    !gPaks[i].inUse && changed[i]) {
                    _RemovePak(i);
                    _ClearPak(i);
                }
            } else if (gPaks[i].inserted && !gPaks[i].formatted) {
                gPaks[i].inserted = 1;
                gPaks[i].original = 0;
                gPaks[i].damaged = 0;
                changed[i] = 1;
            } else if (!gPaks[i].inserted && gPaks[i].wasRemoved) {
                changed[i] = 1;
            } else if (gPaks[i].inited && !gPaks[i].locked && !gPaks[i].flush &&
                       !gPaks[i].flushing && !gPaks[i].inUse) {
                _RemovePak(i);
                _ClearPak(i);
            }
            continue;
        }
        if (gPaks[i].inserted && (gPaks[i].original || !gPaks[i].inited ||
            gPaks[i].locked || gPaks[i].flush || gPaks[i].flushing ||
            gPaks[i].inUse || gPaks[i].damaged)) continue;
        gPaks[i].inserted = 1;
        changed[i] = 1;
        dup = 0;
        same = 0;
        if (!gPaks[i].inited) gPaks[i].inited = 1;
        else if (_SamePak(i, i)) {
            gPaks[i].original = 1;
            changed[i] = 0;
            if (gPaks[i].flushing) continue;
            same = 1;
            goto flush_pak;
        }
        for (j = 0; j < 4; j++) {
            if (i != j && gPaks[j].inited && _SamePak(j, i)) {
                if (gPaks[i].inUse) break;
                if (gPaks[i].inited) {
                    dup = 1;
                    goto check_locked;
                }
move_pak:
                gPaks[i].original = 1;
                memcpy(&gPS_Files[i], &gPS_Files[j], sizeof(gPS_Files[i]));
                ClearLL(&gPS_Files[j]);
                for (psfileh = gPS_Files[i].head; psfileh;
                     psfileh = (tPS_File **)(*psfileh)->ll.next)
                    (*psfileh)->pak = i;
                memcpy(&gPaks[i].files, &gPaks[j].files, sizeof(gPaks[i].files));
                ClearLL(&gPaks[j].files);
                gPaks[j].inited = 0;
                gPaks[j].original = 0;
                changed[i] = 0;
                gPaks[i].locked = gPaks[j].locked;
                gPaks[j].locked = 0;
                gPaks[i].flush = gPaks[j].flush;
                gPaks[j].flush = 0;
                if (gPaks[j].inserted && !gPaks[j].damaged) gPaks[j].wasRemoved = 1;
                changed[j] = 1;
                break;
            }
        }
        if (j < 4) continue;
        if (gPaks[i].flushing) { changed[i] = 0; continue; }
        if (gPaks[i].inUse) {
            changed[i] = 0;
            gPaks[i].original = 1;
            continue;
        }
check_locked:
        if (gPaks[i].locked) { changed[i] = 0; continue; }
flush_pak:
        if (!PS_FlushPak(i, 0)) { changed[i] = 0; continue; }
        if (same) continue;
        _RemovePak(i);
        if (dup) goto move_pak;
        _ReadDirectory(0);
        _ReadDirectory(1);
        _ReadDirectory(2);
        _ReadDirectory(3);
        gPaks[i].original = 1;
        gPaks[i].locked = 0;
        gPaks[i].flush = 0;
    }
    for (i = 0; i < 4; i++) gPaks[i].changed = changed[i];
    s_bPakInied = 1;
}

/* 0x8002f504, size 0x10c */
static int _SamePak(int pakInMem, int pakInPort)
{
    tPS_File **psfileh;
    PakFile *file;
    int err;
    anon__Create__Modify_40 info;
    err = PS2McChangeDir(pakInPort, "/BASLUS-00000");
    if (err != 0) return 0;
    for (psfileh = gPS_Files[pakInMem].head; psfileh;
         psfileh = (tPS_File **)(*psfileh)->ll.next) {
        file = *(PakFile **)(*psfileh)->id;
        if (file->locked || file->flush) {
            /* The target compares an uninitialized directory record. Its
               six scalar comparisons reuse untouched r31; keep this defect. */
            if (err != -1 && info.FileSizeByte == file->info.FileSizeByte &&
                info._Create.Year == file->info._Create.Year &&
                info._Create.Sec == file->info._Create.Sec &&
                info._Create.Min == file->info._Create.Min &&
                info._Create.Hour == file->info._Create.Hour &&
                info._Create.Day == file->info._Create.Day &&
                info._Create.Month == file->info._Create.Month)
                return 1;
        }
    }
    return 0;
}

/* 0x8002f610, size 0xd8 */
static void _RemovePak(int pak)
{
    tPS_File **psfileh;
    tPS_File *psfile;
    tPS_File **next;
    PakFile **fileh;
    PakFile *file;
    for (psfileh = gPS_Files[pak].head; psfileh; psfileh = next) {
        psfile = *psfileh;
        fileh = (PakFile **)psfile->id;
        file = *fileh;
        next = (tPS_File **)psfile->ll.next;
        if (psfile->data) {
            if (psfile->RemoveFile) psfile->RemoveFile(psfileh);
            DeallocateMemory(psfile->data);
            psfile->data = 0;
        }
        RemoveLLE(&gPS_Files[pak], psfileh);
        DeallocateMemory(psfileh);
        if (file->icon) DeallocateMemory(file->icon);
        RemoveLLE(&gPaks[pak].files, fileh);
        DeallocateMemory(fileh);
    }
}

/* 0x8002f6e8, size 0x1fc */
static void _ReadDirectory(int pak)
{
    int ret;
    PakFile **fileh;
    PakFile *file;
    tPS_File **psfileh;
    tPS_File *psfile;
    ret = PS2McChangeDir(pak, "/BASLUS-00000");
    if (ret != 0) return;
    HasFirstPlay = 1;
    PS2McGetDir(pak, "*", 0, 99, table);
    for (ret = 0; table[ret].AttrFile; ret++) {
        if (!table[ret].FileSizeByte) continue;
        fileh = AllocateHandleHeap(0, sizeof(*file));
        file = *fileh;
        memset(file, 0, sizeof(*file));
        file->exists = 1;
        file->info = table[ret];
        gPaks[pak].error = 0;
        InsertLLE(&gPaks[pak].files, fileh, 0);
        psfileh = AllocateHandleHeap(0, sizeof(*psfile));
        psfile = *psfileh;
        memset(psfile, 0, sizeof(*psfile));
        file->psfile = psfileh;
        psfile->pak = pak;
        psfile->id = (unsigned int)fileh;
        psfile->size = table[ret].FileSizeByte;
        psfile->blocks = (psfile->size + 511) >> 9;
        strncpy(psfile->filename, (char *)table[ret].EntryName, 13);
        _GetFileExt(psfile->filename, psfile->ext);
        strcpy(psfile->name, psfile->filename);
        strncpy(psfile->filename, (char *)table[ret].EntryName, 13);
        InsertLLE(&gPS_Files[pak], psfileh, 0);
    }
}

/* 0x8002f8e4, size 0xd4 */
static void _GetFileExt(char *name, char *ext)
{
    int end;
    int c;
    int l;
    ext[3] = 0;
    name += 11;
    ext += 2;
    end = 0;
    c = 11;
    l = 0;
    while (c >= 0 && l < 3) {
        if (end || *name > ' ') {
            *ext-- = *name;
            end = 1;
            l++;
        }
        c--;
        name--;
    }
    if (l >= 3) *name = 0;
    for (; l < 3; l++) *ext-- = 0;
}

/* 0x8002f9b8, size 0x8 */
int PS_PakChanged(void)
{
    return 0;
}

/* 0x8002f9c0, size 0x8 */
int PS_PakInserted(void)
{
    return 1;
}

/* 0x8002f9c8, size 0x8 */
int PS_PakNotTheSame(void)
{
    return 0;
}

/* 0x8002f9d0, size 0x8 */
int PS_PakDamaged(void)
{
    return 0;
}

/* 0x8002f9d8, size 0x8 */
int PS_PakFormatted(void)
{
    return 1;
}

/* 0x8002f9e0, size 0x8 */
int PS_PakRumble(void)
{
    return 1;
}

/* 0x8002f9e8, size 0x30 */
unsigned int PS_GetFreeBlocks(int pak)
{
    return gPaks[pak].FreeSpace / 1000;
}

/* 0x8002fa18, size 0x48 */
unsigned int PS_GetFreeFiles(int pak)
{
    return PS_GetFreeBlocks(pak) / PS_GetBlocks(1);
}

/* 0x8002fa60, size 0xc */
unsigned int PS_GetBlocks(int numBytes)
{
    return ((unsigned int)numBytes + 9493) >> 10;
}

/* 0x8002fa6c, size 0xc4 */
void PS_LockFile(tPS_File **psfileh, int lock)
{
    PakFile **fileh;
    PakFile *file;
    int pak;
    if (!psfileh) return;
    file = *(PakFile **)(*psfileh)->id;
    if (file->locked == lock) return;
    file->locked = lock;
    pak = (*psfileh)->pak;
    if (lock) {
        gPaks[pak].locked = 1;
    } else {
        fileh = gPaks[pak].files.head;
        while (fileh) {
            file = *fileh;
            if (file->locked) break;
            fileh = (PakFile **)file->ll.next;
        }
        if (!fileh) {
            gPaks[pak].locked = 0;
            if (!gPaks[pak].flush && !gPaks[pak].original)
                gPaks[pak].inserted = 0;
        }
    }
}

/* 0x8002fb30, size 0x240 */
tPS_File **PS_CreateFile(int pak, void * name, void * ext, unsigned int numBytes, char *gameName, char *bootStr, char *vmuStr)
{
    PakFile **fileh;
    PakFile *file;
    tPS_File **psfileh;
    tPS_File *psfile;
    char str[64];
    gPaks[pak].inUse = 1;
    fileh = AllocateHandleHeap(0, sizeof(*file));
    file = *fileh;
    memset(file, 0, sizeof(*file));
    memset(file->bfh.vms_comment, ' ', sizeof(file->bfh.vms_comment));
    strncpy(file->bfh.vms_comment, vmuStr, strlen(vmuStr));
    memset(file->bfh.btr_comment, ' ', sizeof(file->bfh.btr_comment));
    strncpy(file->bfh.btr_comment, bootStr, strlen(bootStr));
    memset(file->bfh.game_name, ' ', sizeof(file->bfh.game_name));
    strncpy(file->bfh.game_name, gameName, strlen(gameName));
    if (!gPaks[pak].HasDir && !PS2MCSetupMcDirectory(pak)) return 0;
    file->bfh.visual_type = 0;
    file->bfh.save_size = numBytes;
    sprintf(str, "%s.%s", name, ext);
    psfileh = AllocateHandleHeap(0, sizeof(*psfile));
    psfile = *psfileh;
    memset(psfile, 0, sizeof(*psfile));
    psfile->pak = pak;
    psfile->id = (unsigned int)fileh;
    psfile->blocks = 0;
    psfile->size = numBytes;
    memset(psfile->name, ' ', 8);
    strncpy(psfile->filename, str, strlen(str));
    strcpy(psfile->name, name);
    strcpy(psfile->ext, ext);
    file->psfile = psfileh;
    psfile->data = AllocateHandleHeap(0, numBytes);
    memset(*(void **)psfile->data, 0, numBytes);
    file->bfh.save_data = *(void **)psfile->data;
    file->bfh.save_size = psfile->size;
    InsertLLE(&gPaks[pak].files, fileh, 0);
    InsertLLE(&gPS_Files[pak], psfileh, 0);
    gPaks[pak].inUse = 0;
    return psfileh;
}

/* 0x8002fd70, size 0x110 */
void PS_DeleteFile(tPS_File **psfileh)
{
    int pak;
    PakFile **fileh;
    tPS_File *psfile;
    psfile = *psfileh;
    pak = psfile->pak;
    fileh = (PakFile **)psfile->id;
    gPaks[pak].inUse = 1;
    PS_LockFile(psfileh, 0);
    if ((*fileh)->exists && !_DeleteFile(pak, psfile->filename)) {
        gPaks[pak].inUse = 0;
        return;
    }
    if (psfile->RemoveFile) psfile->RemoveFile(psfileh);
    RemoveLLE(&gPS_Files[pak], psfileh);
    RemoveLLE(&gPaks[pak].files, fileh);
    if (psfile->data) DeallocateMemory(psfile->data);
    if ((*fileh)->icon) DeallocateMemory((*fileh)->icon);
    DeallocateMemory(psfileh);
    DeallocateMemory(fileh);
    _ClearFlush(pak);
    gPaks[pak].inUse = 0;
}

/* 0x8002fe80, size 0x74 */
static int _DeleteFile(int pak, char *name)
{
    int err;
    char filelocal[80];
    char filelocalex[80];
    strcpy(filelocal, name);
    strcpy(filelocalex, "EX");
    strcat(filelocalex, filelocal + 1);
    err = PS2MCDeleteFile(pak, filelocal);
    if (err == -1) return 0;
    PS2MCDeleteFile(pak, filelocalex);
    return 1;
}

/* 0x8002fef4, size 0x48 */
void PS_ReadFile(tPS_File **psfileh)
{
    PakFile *file;
    file = *(PakFile **)(*psfileh)->id;
    PS_ReadPartialFile(psfileh, 0, (*psfileh)->blocks * 512);
    file->partial = 0;
}

/* 0x8002ff3c, size 0x15c */
void PS_ReadPartialFile(tPS_File **psfileh)
{
    int pak;
    PakFile *file;
    tPS_File *psfile;
    int ret;
    int Length;
    char tempstr[128];
    char *Buffer;
    void *V;
    Buffer = 0;
    psfile = *psfileh;
    file = *(PakFile **)psfile->id;
    pak = psfile->pak;
    if (psfile->data) return;
    gPaks[pak].inUse = 1;
    ret = PS2McCreateFileReader(pak, psfile->filename, 0);
    if (ret >= 0) {
        PS2MCSeek(ret, 0);
        if (PS2MCRead(ret, &file->bfh, sizeof(file->bfh))) {
            Length = file->bfh.save_size;
            Buffer = AllocateMemoryHeap(0, Length);
            if (ret >= 0) PS2McClose(ret);
            strcpy(tempstr, "EX");
            strcat(tempstr, psfile->filename + 1);
            ret = PS2McCreateFileReader(pak, tempstr, 0);
            if (ret >= 0) {
                PS2MCSeek(ret, 0);
                if (PS2MCRead(ret, Buffer, Length)) {
                    psfile->data = AllocateHandleHeap(0, file->bfh.save_size);
                    memcpy(*(void **)psfile->data, Buffer, file->bfh.save_size);
                }
            }
        }
    }
    if (ret >= 0) PS2McClose(ret);
    if (Buffer) DeallocateMemory(Buffer);
    file->partial = 1;
    gPaks[pak].inUse = 0;
}

/* 0x80030098, size 0x54 */
void PS_PurgeFile(tPS_File **psfileh)
{
    PakFile *file;
    tPS_File *psfile;
    psfile = *psfileh;
    file = *(PakFile **)psfile->id;
    if (psfile->data) {
        DeallocateMemory(psfile->data);
        psfile->data = 0;
        file->partial = 0;
    }
}

/* 0x800300ec, size 0x20 */
void PS_WriteFile(tPS_File **psfileh)
{
    if (psfileh)
        (*(PakFile **)(*psfileh)->id)->flush = 1;
}

/* 0x8003010c, size 0xd8 */
int PS_FlushFile(tPS_File **psfileh, int ignoreErrors)
{
    gPaks[(*psfileh)->pak].inUse = 1;
    gPaks[(*psfileh)->pak].flushing = 1;
    gPaks[(*psfileh)->pak].flushed = 0;
    gPaks[(*psfileh)->pak].flushed = _PS_FlushFile(psfileh, ignoreErrors);
    gPaks[(*psfileh)->pak].flushing = 0;
    gPaks[(*psfileh)->pak].inUse = 0;
    return gPaks[(*psfileh)->pak].flushed;
}

/* 0x800301e4, size 0x50 */
void PS_DiscardFileChanges(tPS_File **psfileh)
{
    PakFile *file;
    if (psfileh) {
        file = *(PakFile **)(*psfileh)->id;
        if (file->flush) {
            file->flush = 0;
            _ClearFlush((*psfileh)->pak);
        }
    }
}

/* 0x80030234, size 0xa4 */
static void _ClearFlush(int pak)
{
    tPS_File **psfileh;
    tPS_File **next;
    PakFile *file;
    if (!gPaks[pak].flush) return;
    for (psfileh = gPS_Files[pak].head; psfileh; psfileh = next) {
        file = *(PakFile **)(*psfileh)->id;
        next = (tPS_File **)(*psfileh)->ll.next;
        if ((*psfileh)->data && file->flush) break;
    }
    if (!psfileh) {
        gPaks[pak].flush = 0;
        if (!gPaks[pak].locked && !gPaks[pak].original)
            gPaks[pak].inserted = 0;
    }
}

/* 0x800302d8, size 0x178 */
static int _PS_FlushFile(tPS_File **psfileh, int ignoreErrors)
{
    int pak;
    int ret;
    PakFile *file;
    tPS_File *psfile;
    char filelocal[64];
    char tempstr[128];
    if (!psfileh) return 1;
    psfile = *psfileh;
    file = *(PakFile **)psfile->id;
    pak = psfile->pak;
    if (!file->flush) return 1;
    file->bfh.save_data = *(void **)psfile->data;
    PS2McDeleteDummyFile();
    strcpy(filelocal, psfile->filename);
    strlen(psfile->filename);
    ret = PS2McCreateFileWriter(pak, filelocal, 0);
    if (ret < 0) return 0;
    PS2MCSeek(ret, 0);
    if (!PS2MCWrite(ret, &file->bfh, sizeof(file->bfh))) return 0;
    if (ret >= 0) PS2McClose(ret);
    strcpy(tempstr, "EX");
    strcat(tempstr, filelocal + 1);
    ret = PS2McCreateFileWriter(pak, tempstr, 0);
    if (ret < 0) return 0;
    PS2MCSeek(ret, 0);
    if (!PS2MCWrite(ret, file->bfh.save_data, file->bfh.save_size)) return 0;
    if (!PS2McClose(ret)) return 0;
    file->exists = 1;
    file->flush = 0;
    _ClearFlush(pak);
    return 1;
}

/* 0x80030450, size 0xf0 */
int PS_FlushPak(int pak, int ignoreErrors)
{
    tPS_File **psfileh;
    tPS_File **next;
    PakFile *file;
    gPaks[pak].inUse = 1;
    gPaks[pak].flushed = 0;
    if (!gPaks[pak].flush) {
        gPaks[pak].inUse = 0;
        return 1;
    }
    gPaks[pak].flushing = 1;
    for (psfileh = gPS_Files[pak].head; psfileh; psfileh = next) {
        file = *(PakFile **)(*psfileh)->id;
        next = (tPS_File **)(*psfileh)->ll.next;
        if ((*psfileh)->data && file->flush) {
            if (!_PS_FlushFile(psfileh, ignoreErrors)) break;
            gPaks[pak].flushed = 1;
        }
    }
    gPaks[pak].flushing = 0;
    gPaks[pak].inUse = 0;
    /* Preserve the target's negation before its null test. Unsigned negation
       is defined for every pointer representation used by this 32-bit ABI. */
    return -(unsigned int)psfileh == 0;
}

/* 0x80030540, size 0x78 */
static void _ClearPak(unsigned char pak)
{
    signed char damaged;
    damaged = gPaks[pak].damaged;
    memset(&gPaks[pak], 0, sizeof(gPaks[pak]));
    gPaks[pak].damaged = damaged;
    InitLL(&gPaks[pak].files, 1, 1);
}

/* 0x800305b8, size 0xc */
void PS_RumbleGo(void)
{
    gRumble = 1;
}

/* 0x800305c4, size 0x4 */
void PS_RumbleStop(void)
{

}

/* 0x800305c8, size 0x20 */
void PS_SetRumblePercent(int controller, int percent)
{
    NGCRumblePad(controller, percent);
}

