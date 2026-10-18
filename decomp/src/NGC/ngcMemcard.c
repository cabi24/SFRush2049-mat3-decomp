/* ngcMemcard.c - GameCube memory-card I/O behind the PS2 memory-card API. Status: Matching.
   Original: D:\rush_ngc\Rush_NGC\NGC\ngcMemcard.c; assembly: build/2049LE/asm/NGC/ngcMemcard.s */
#include "rush/rush_protos.h"
#include "rush/libc.h"

extern long CARDOpen(long, const char *, CARDFileInfo *);
extern long CARDCreate(long, const char *, unsigned long, CARDFileInfo *);
extern long CARDClose(CARDFileInfo *);
extern long CARDRead(CARDFileInfo *, void *, long, long);
extern long CARDWrite(CARDFileInfo *, const void *, long, long);
extern long CARDGetStatus(long, long, CARDStat *);
extern long CARDSetStatus(long, long, CARDStat *);
extern long CARDGetResultCode(long);
extern long CARDGetSectorSize(long, unsigned long *);
extern long CARDMount(long, void *, void (*)(long, long));
extern void OSReport(const char *, ...);
#define s_MemData s_MemData_8022A72C
/* The split Bink LogoData contains a relocation to this originally local symbol.
   Export its disambiguated split name so the reconstructed unit satisfies it. */
anon_uVersion_bRumble_a040 s_MemData[4];

static unsigned char icondata[14912] = {
#include "generated/icondata.inc"
};
int g_AutoSave = 1;
int CurSlot;
static int Checking;
static int Pos;
unsigned long _SectorSize;
static int ArIsError;
static int s_MemcardFileExist;
static int s_MemcardLoaded;
static int s_NowPort;
static char s_NowFileName[32];
char _NgcDirectory[128];
unsigned char _CardWorkArea[40960] __attribute__((aligned(32)));
CARDFileInfo CardFileInfo[127];

/* 0x800981a0, size 0x94 */
int ngcMcCreateFileReader(int Port, char *filename, int mode)
{
    int i, result;
    for (i = 0; i < 127; i++)
        if (!CardFileInfo[i].fileNo)
            break;
    PS2MCWaitStatus(1);
    result = CARDOpen(CurSlot, filename, &CardFileInfo[i]);
    if (result == 0)
        return i;
    else
        return -1;
}

/* 0x80098234, size 0x178 */
int ngcMcCreateFileWriter(int Port, char *Filename, int mode)
{
    int i, result;
    char *temBuf;
    for (i = 0; i < 127; i++)
        if (!CardFileInfo[i].fileNo)
            break;
    PS2MCWaitStatus(1);
    result = CARDOpen(CurSlot, Filename, &CardFileInfo[i]);
    if (result != 0 && result != -4)
        return -1;
    s_MemcardFileExist = result != -4;
    if (!s_MemcardFileExist) {
        if (CARDCreate(CurSlot, Filename, _SectorSize * 33, &CardFileInfo[i]) != 0)
            return -1;
        if (CARDOpen(CurSlot, Filename, &CardFileInfo[i]) != 0)
            return -1;
        temBuf = AllocateMemoryHeap(0, _SectorSize * 33);
        memset(temBuf, 0, _SectorSize * 33);
        result = CARDWrite(&CardFileInfo[i], temBuf, _SectorSize * 33, 0);
        DeallocateMemory(temBuf);
        if (result != 0)
            return -1;
        CARDClose(&CardFileInfo[i]);
    }
    result = CARDOpen(CurSlot, Filename, &CardFileInfo[i]);
    if (result == 0)
        return i;
    else
        return -1;
}

/* 0x800983ac, size 0x6c */
int ngcMcClose(int File)
{
    int result;
    PS2MCWaitStatus(1);
    result = CARDClose(&CardFileInfo[File]);
    if (result == 0) {
        memset(&CardFileInfo[File], 0, sizeof(CARDFileInfo));
        return 1;
    }
    return 0;
}

/* 0x80098418, size 0x8 */
void ngcMCSeek(int File, int InPos)
{
    Pos = InPos;
}

/* 0x80098420, size 0x10 */
void ngcMCSeek2(int File, int AddPos)
{
    Pos += AddPos;
}

/* 0x80098430, size 0xd0 */
int ngcMCRead(int File, void *V, int Length)
{
    unsigned char *ReadBuffer;
    int nReadBuffer;
    char *pBuffer = V;
    long sResult;
    nReadBuffer = Length / 512;
    if (Length % 512)
        nReadBuffer++;
    nReadBuffer *= 512;
    ReadBuffer = AllocateMemoryHeap(0, nReadBuffer);
    memset(ReadBuffer, 0, nReadBuffer);
    sResult = CARDRead(&CardFileInfo[File], ReadBuffer, nReadBuffer, Pos);
    ngcMCSeek2(File, nReadBuffer);
    memcpy(pBuffer, ReadBuffer, Length);
    DeallocateMemory(ReadBuffer);
    if (sResult != 0)
        return -1;
    else
        return 0;
}

/* 0x80098500, size 0x100 */
int ngcMCWrite(int File, void *V, int Length, int SeakLength)
{
    unsigned char *WriteBuffer, *pBuffer;
    int nWriteBuffer;
    long sResult;
    PS2MCWaitStatus(1);
    ArIsError = 0;
    pBuffer = V;
    nWriteBuffer = Length / _SectorSize;
    if (Length % _SectorSize)
        nWriteBuffer++;
    WriteBuffer = AllocateMemoryHeap(0, nWriteBuffer * _SectorSize);
    memset(WriteBuffer, 0, nWriteBuffer * _SectorSize);
    memcpy(WriteBuffer, pBuffer, Length);
    sResult = CARDWrite(&CardFileInfo[File], WriteBuffer, nWriteBuffer * _SectorSize, Pos);
    if (SeakLength > nWriteBuffer * _SectorSize)
        ngcMCSeek2(File, SeakLength);
    else
        ngcMCSeek2(File, nWriteBuffer * _SectorSize);
    DeallocateMemory(WriteBuffer);
    if (sResult != 0)
        return -1;
    else
        return 0;
}

/* 0x80098600, size 0x78 */
long ngcSaveIconToMemcard(int file)
{
    unsigned long uIconSize = (sizeof(icondata) + _SectorSize - 1) & ~(_SectorSize - 1);
    if (!s_MemcardFileExist) {
        ngcMCWrite(file, icondata, sizeof(icondata), 0);
        appSetIconData(CardFileInfo[file].fileNo);
    } else
        ngcMCSeek2(file, uIconSize);
}

/* 0x80098678, size 0x6c */
long ngcLoadIconFromMemcard(int file)
{
    unsigned long uIconSize = (sizeof(icondata) + _SectorSize - 1) & ~(_SectorSize - 1);
    unsigned char *TempMem = AllocateMemoryHeap(0, uIconSize);
    ngcMCRead(file, TempMem, uIconSize);
    DeallocateMemory(TempMem);
}

/* 0x800986e4, size 0x1b0 */
int appSetIconData(int file)
{
    int resIO;
    CARDStat cs;
    int res;
    resIO = CARDGetStatus(CurSlot, file, &cs);
    if (resIO < 0) {
        res = appCheckIO(0, resIO);
        if (res)
            return res;
    }
    cs.commentAddr = 0;
    cs.bannerFormat = (cs.bannerFormat & ~3) | 2;
    cs.iconAddr = 64;
    cs.bannerFormat &= ~4;
    cs.iconFormat = (cs.iconFormat & ~(3 << 0)) | (1 << 0);
    cs.iconSpeed = (cs.iconSpeed & ~(3 << 0)) | (2 << 0);
    cs.iconFormat = (cs.iconFormat & ~(3 << 2)) | (1 << 2);
    cs.iconSpeed = (cs.iconSpeed & ~(3 << 2)) | (2 << 2);
    cs.iconFormat = (cs.iconFormat & ~(3 << 4)) | (1 << 4);
    cs.iconSpeed = (cs.iconSpeed & ~(3 << 4)) | (2 << 4);
    cs.iconFormat = (cs.iconFormat & ~(3 << 6)) | (1 << 6);
    cs.iconSpeed = (cs.iconSpeed & ~(3 << 6)) | (2 << 6);
    cs.iconFormat = (cs.iconFormat & ~(3 << 8)) | (1 << 8);
    cs.iconSpeed = (cs.iconSpeed & ~(3 << 8)) | (2 << 8);
    cs.iconFormat = (cs.iconFormat & ~(3 << 10)) | (1 << 10);
    cs.iconSpeed = (cs.iconSpeed & ~(3 << 10)) | (2 << 10);
    cs.iconFormat = (cs.iconFormat & ~(3 << 12)) | (1 << 12);
    cs.iconSpeed = (cs.iconSpeed & ~(3 << 12)) | (2 << 12);
    cs.iconFormat = (cs.iconFormat & ~(3 << 14)) | (1 << 14);
    cs.iconSpeed = (cs.iconSpeed & ~(3 << 14)) | (2 << 14);

    resIO = CARDSetStatus(CurSlot, file, &cs);
    if (resIO < 0) {
        res = appCheckIO(0, resIO);
        if (res)
            return res;
    }
    return 1;
}

/* 0x80098894, size 0x1c4 */
long ngcSaveToMemcard(void)
{
    char *TempMem, *pTempMem;
    int i, k, iResult, iResult2 = 0;
    PS2MCTick();
    TempMem = AllocateMemoryHeap(0, 49152);
    iResult = ngcMcCreateFileWriter(k, "Midway Arcade Treasures 3", 0);
    if (iResult != -1) {
        ngcMCSeek(iResult, 0);
        ngcSaveIconToMemcard(iResult);
        ngcMCSeek2(iResult, appGetGameSaveOffset(3));
        for (k = 0; k < 4; k++) {
            s_MemData[k].uVersion = 0x12405;
            s_MemData[k].bConfig = ChooseYes[0];
            s_MemData[k].bAutoSave = g_AutoSave;
            g_CurrentControl_t[0] = ChooseYes[0];
            s_MemData[k].bRumble = g_bUseRumble;
            memcpy(&s_MemData[k].RushOptions, gOptions, 41);
            memcpy(TempMem, &s_MemData[k], 20);
            memcpy(TempMem + 20, &s_MemData[k].RushOptions, 41);
            pTempMem = TempMem + 61;
            for (i = 0; i < 1024; i++)
                if (strcmp(s_MemData[k].pFile[i].FileName, "") != 0) {
                    memcpy(pTempMem, s_MemData[k].pFile[i].FileName, 32);
                    memcpy(pTempMem + 32, &s_MemData[k].pFile[i].FileLen, 4);
                    pTempMem += 36;
                    memcpy(pTempMem, s_MemData[k].pFile[i].FileData, s_MemData[k].pFile[i].FileLen);
                    pTempMem += s_MemData[k].pFile[i].FileLen;
                }
            iResult2 = ngcMCWrite(iResult, TempMem, pTempMem - TempMem, 49152);
            if (iResult2 == -1)
                break;
        }
        ngcMcClose(iResult);
    }
    DeallocateMemory(TempMem);
    if (iResult == 0)
        return iResult2;
    else
        return iResult;
}

/* 0x80098a58, size 0x2b8 */
long ngcLoadFromMemcard(void)
{
    char *TempMem, *pTempMem;
    int i, k, iResult, iResult2 = 0;
    CARDStat stat;
    signed char sOptionsStereo;
    char buffer[1024];
    PS2MCTick();
    TempMem = AllocateMemoryHeap(0, 49152);
    if (s_MemcardLoaded == 1)
        for (k = 0; k < 4; k++)
            for (i = 0; i < 1024; i++)
                if (strcmp(s_MemData[k].pFile[i].FileName, "") != 0)
                    DeallocateMemory(s_MemData[k].pFile[i].FileData);
    iResult = ngcMcCreateFileReader(k, "Midway Arcade Treasures 3", 0);
    if (iResult != -1) {
        CARDGetStatus(CurSlot, CardFileInfo[iResult].fileNo, &stat);
        ngcMCSeek(iResult, 0);
        ngcLoadIconFromMemcard(iResult);
        ngcMCSeek2(iResult, appGetGameSaveOffset(3));
        for (k = 0; k < 4; k++) {
            memset(&s_MemData[k], 0, sizeof(s_MemData[k]));
            iResult2 = ngcMCRead(iResult, TempMem, 49152);
            if (iResult2 == -1)
                break;
            memcpy(&s_MemData[k], TempMem, 20);
            memcpy(&s_MemData[k].RushOptions, TempMem + 20, 41);
            pTempMem = TempMem + 61;
            if (s_MemData[k].uVersion != 0x12405) {
                memset(&s_MemData[k], 0, sizeof(s_MemData[k]));
                goto done;
            }
            sprintf(buffer, "**********\nMEM FILE : %d\n**********\n", s_MemData[k].nFileNum);
            OSReport(buffer);
            for (i = 0; i < s_MemData[k].nFileNum; i++) {
                memcpy(s_MemData[k].pFile[i].FileName, pTempMem, 32);
                memcpy(&s_MemData[k].pFile[i].FileLen, pTempMem + 32, 4);
                pTempMem += 36;
                s_MemData[k].pFile[i].FileData = AllocateMemoryHeap(0, s_MemData[k].pFile[i].FileLen);
                memcpy(s_MemData[k].pFile[i].FileData, pTempMem, s_MemData[k].pFile[i].FileLen);
                pTempMem += s_MemData[k].pFile[i].FileLen;
            }
            s_MemcardLoaded = 1;
            sOptionsStereo = gOptions[15];
            g_CurrentControl_t[0] = ChooseYes[0] = s_MemData[k].bConfig;
            g_AutoSave = s_MemData[k].bAutoSave;
            g_bUseRumble = s_MemData[k].bRumble;
            memcpy(gOptions, &s_MemData[k].RushOptions, 41);
            gOptions[15] = sOptionsStereo;
        }
        ngcMcClose(iResult);
    }
done:
    DeallocateMemory(TempMem);
    return iResult == 0 ? iResult2 : iResult;
}

/* 0x80098d10, size 0x58 */
void PS2MCInit(void)
{
    int i;
    CurSlot = 0;
    for (i = 0; i < 4; i++) {
        gPaks[i].CheckedStatus = 0;
        gPaks[i].inserted = 0;
        gPaks[i].CheckCountdown = 0;
        gPaks[i].NotTheSame = 0;
    }
    Checking = 0;
}

/* Erased card-status mount helper, stripped by the linker; retained only to preserve work-area .bss placement. */
static void PS2MCCheckStatus(void)
{
    CARDMount(CurSlot, _CardWorkArea, 0);
}

/* 0x80098d68, size 0x4 */
void PS2MCUpdateSame(void) {}

/* 0x80098d6c, size 0x74 */
void PS2MCWaitStatus(void)
{
    long result;
    if (Checking) {
        result = CARDGetResultCode(CurSlot);
        if (result == 0) {
            gPaks[CurSlot].inserted = 1;
            gPaks[CurSlot].formatted = 1;
            gPaks[CurSlot].inited = 1;
            gPaks[CurSlot].CheckCountdown = 20;
        } else if (result == -4)
            return;
        else if (result == -7)
            return;
    }
}

/* 0x80098de0, size 0x50 */
void PS2MCTick(void)
{
    gPaks[CurSlot].inserted = 1;
    gPaks[CurSlot].formatted = 1;
    gPaks[CurSlot].inited = 1;
    gPaks[CurSlot].CheckCountdown = 20;
    CARDGetSectorSize(CurSlot, &_SectorSize);
}

/* 0x80098e30, size 0x2c */
int PS2McChangeDir(int Port, char *dir)
{
    strcpy(_NgcDirectory, dir);
    return 0;
}

/* 0x80098e5c, size 0xb0 */
int PS2McGetDir(int Port, void *name, int flags, int max, anon__Create__Modify_40 *tables)
{
    int i;
    char filename[128];
    anon__Create__Modify_40 *ptables = tables;
    for (i = 0; i < 1024; i++)
        if (strcmp(s_MemData[Port].pFile[i].FileName, "") != 0) {
            strcpy(filename, s_MemData[Port].pFile[i].FileName);
            ptables->AttrFile = 1;
            strcpy((char *)ptables->EntryName, filename);
            ptables->FileSizeByte = s_MemData[Port].pFile[i].FileLen;
            ptables++;
        }
    return 0;
}

/* 0x80098f0c, size 0xa4 */
int PS2McCreateFileReader(int Port, char *filename)
{
    int i;
    for (i = 0; i < 1024; i++)
        if (strcmp(filename, s_MemData[Port].pFile[i].FileName) == 0) {
            strcpy(s_NowFileName, filename);
            s_NowPort = Port;
            return 0;
        }
    return -1;
}

/* 0x80098fb0, size 0xb0 */
int PS2MCDeleteFile(int Port, char *filename)
{
    int i;
    for (i = 0; i < 1024; i++)
        if (strcmp(filename, s_MemData[Port].pFile[i].FileName) == 0) {
            s_MemData[Port].nFileNum--;
            DeallocateMemory(s_MemData[Port].pFile[i].FileData);
            memset(&s_MemData[Port].pFile[i], 0, sizeof(s_MemData[Port].pFile[i]));
            break;
        }
    return 1;
}

/* 0x80099060, size 0x144 */
int PS2McCreateFileWriter(int Port, char *Filename)
{
    int i;
    for (i = 0; i < 1024; i++)
        if (strcmp(Filename, s_MemData[Port].pFile[i].FileName) == 0) {
            DeallocateMemory(s_MemData[Port].pFile[i].FileData);
            memset(&s_MemData[Port].pFile[i], 0, sizeof(s_MemData[Port].pFile[i]));
            strcpy(s_MemData[Port].pFile[i].FileName, Filename);
            strcpy(s_NowFileName, Filename);
            s_NowPort = Port;
            return 0;
        }
    for (i = 0; i < 1024; i++)
        if (strcmp(s_MemData[Port].pFile[i].FileName, "") == 0) {
            s_MemData[Port].nFileNum++;
            strcpy(s_MemData[Port].pFile[i].FileName, Filename);
            strcpy(s_NowFileName, Filename);
            s_NowPort = Port;
            return 0;
        }
    return -1;
}

/* 0x800991a4, size 0x8 */
int PS2McClose(void)
{
    return 1;
}

/* 0x800991ac, size 0x8 */
void PS2MCSeek(int File, int InPos)
{
    Pos = InPos;
}

/* 0x800991b4, size 0xbc */
int PS2MCRead(int File, void *V, int Length)
{
    int i;
    for (i = 0; i < 1024; i++)
        if (strcmp(s_NowFileName, s_MemData[s_NowPort].pFile[i].FileName) == 0) {
            memcpy(V, s_MemData[s_NowPort].pFile[i].FileData, Length);
            break;
        }
    return 1;
}

/* 0x80099270, size 0xf0 */
int PS2MCWrite(int File, void *V, int Length)
{
    int i;
    for (i = 0; i < 1024; i++)
        if (strcmp(s_NowFileName, s_MemData[s_NowPort].pFile[i].FileName) == 0) {
            s_MemData[s_NowPort].pFile[i].FileLen = Length;
            s_MemData[s_NowPort].pFile[i].FileData =
                AllocateMemoryHeap(0, s_MemData[s_NowPort].pFile[i].FileLen);
            memcpy(s_MemData[s_NowPort].pFile[i].FileData, V, Length);
            break;
        }
    return 1;
}

/* 0x80099360, size 0x4 */
void PS2MCFormat(void) {}

/* 0x80099364, size 0x8 */
int PS2MCSetupMcDirectory(void)
{
    return 1;
}

/* 0x8009936c, size 0x4 */
void PS2McDeleteDummyFile(void) {}
