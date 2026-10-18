/* ngcFile.c - DVD file access, endian swapping of loaded data, and fatal disc errors.
   Original: D:\rush_ngc\Rush_NGC\NGC\ngcFile.c; assembly: build/2049LE/asm/NGC/ngcFile.s */
#include "rush/rush_protos.h"

#define READ_CHUNK 0x20000
#define USERDATA(fd) ((exUserData *)((DVDFileInfo *)(fd))->cb.userData)

static unsigned long FatalLine;
static exUserData g_UserData;
int g_bLoadingNow;
static int bFatalError;
static unsigned int nSwapNum;
static int bNeedSwap;
static char FatalFunc[128];
static char FatalFile[128];
static DVDFileInfo g_FileInfo;
static unsigned int SwapDataList[4096];
static unsigned int SwapList[4096];
static DVDFileInfo g_FileHandle;

/* 0x800a4d80, size 0x4c */
int _create_filehandle(DVDFileInfo *FileInfo)
{
    g_UserData.iOffset = 0;
    FileInfo->cb.userData = &g_UserData;
    memcpy(&g_FileHandle, FileInfo, sizeof(DVDFileInfo));
    return (int)&g_FileHandle;
}

/* 0x800a4dcc, size 0x4 */
void _delete_filehandle()
{

}

/* 0x800a4dd0, size 0xc */
void _Enable_swap(void)
{
    bNeedSwap = 1;
}

/* 0x800a4ddc, size 0xc */
void _Disenable_swap(void)
{
    bNeedSwap = 0;
}

/* 0x800a4de8, size 0xb0 */
void _Check_swap(unsigned int *Address)
{
    int i; /* r0 */

    _Enable_swap();
    for (i = 0; i < nSwapNum; i++) {
        if ((unsigned int)Address == SwapList[i] && *Address == SwapDataList[i]) {
            _Disenable_swap();
            return;
        }
    }
    SwapList[nSwapNum] = (unsigned int)Address;
    SwapDataList[nSwapNum] = *Address;
    _swap_x32(&SwapDataList[nSwapNum]);
    nSwapNum++;
}

/* 0x800a4e98, size 0x20 */
void _swap_byte(unsigned char *a, unsigned char *b)
{
    unsigned char x; /* r5 */

    if (bNeedSwap) {
        x = *b;
        *b = *a;
        *a = x;
    }
}

/* 0x800a4eb8, size 0x24 */
void _swap_x16(void * i)
{
    unsigned char *b; /* r0 */

    b = i;
    _swap_byte(b, b + 1);
}

/* 0x800a4edc, size 0x3c */
void _swap_x32(void * i)
{
    unsigned char *b; /* r0 */

    b = i;
    _swap_byte(b, b + 3);
    _swap_byte(b + 1, b + 2);
}

/* 0x800a4f18, size 0x50 */
void _swap_x16_array(unsigned char *i, int size)
{
    while (size) {
        _swap_x16(i);
        i += 2;
        size -= 2;
    }
}

/* 0x800a4f68, size 0x50 */
void _swap_x32_array(unsigned char *i, int size)
{
    while (size) {
        _swap_x32(i);
        i += 4;
        size -= 4;
    }
}

/* 0x800a4fb8, size 0x60 */
/* callers pass a mode argument (see overrides.json) */
int appFileOpen(const char *filename, int mode)
{
    int bSuccess; /* r0 */
    char filename2[128]; /* stack */

    sprintf(filename2, "rush2049_data/%s", filename);
    bSuccess = DVDOpen(filename2, &g_FileInfo);
    if (bSuccess) {
        return _create_filehandle(&g_FileInfo);
    }
    return 0;
}

/* 0x800a5018, size 0x34 */
void appFileClose(int fd)
{
    DVDClose((DVDFileInfo *)fd);
    _delete_filehandle(fd);
}

/* 0x800a504c, size 0x158 */
int appFileRead(int fd, void * buf, int size)
{
    void * temp_buf; /* r31 */
    int temp_size_left; /* r30 */
    int temp_size_clip; /* r29 */
    long sResult; /* r0 */

    temp_buf = AllocateMemoryHeap(0, READ_CHUNK);
    temp_size_left = (size + 31) & ~31;
    temp_size_clip = temp_size_left - size;
    enter_cs_read();
    for (;;) {
        if (!temp_size_left) {
            break;
        }
        if (temp_size_left >= READ_CHUNK) {
            g_bLoadingNow = 1;
            sResult = DVDReadPrio((DVDFileInfo *)fd, temp_buf, READ_CHUNK, USERDATA(fd)->iOffset, 2);
            g_bLoadingNow = 0;
            if (sResult == -1) {
/* #line reproduces the original's bare file name and line number, which are baked into the SetFatalData call */
#line 267 "ngcFile.c"
                SetFatalData(__FILE__, __LINE__, __FUNCTION__);
            }
            USERDATA(fd)->iOffset += READ_CHUNK;
            memcpy(buf, temp_buf, READ_CHUNK);
            temp_size_left -= READ_CHUNK;
            buf = (char *)buf + READ_CHUNK;
        } else {
            g_bLoadingNow = 1;
            sResult = DVDReadPrio((DVDFileInfo *)fd, temp_buf, temp_size_left, USERDATA(fd)->iOffset, 2);
            g_bLoadingNow = 0;
            if (sResult == -1) {
#line 279 "ngcFile.c"
                SetFatalData(__FILE__, __LINE__, __FUNCTION__);
            }
            USERDATA(fd)->iOffset += temp_size_left - temp_size_clip;
            memcpy(buf, temp_buf, temp_size_left - temp_size_clip);
            break;
        }
    }
    exit_cs_read();
    DeallocateMemory(temp_buf);
    return 0;
}

/* 0x800a51a4, size 0x8 */
int appFileGetSize(int fd)
{
    return ((DVDFileInfo *)fd)->length;
}

/* 0x800a51ac, size 0x38 */
int appFileRead4(int fd, void * buf, int size)
{
    appFileRead(fd, buf, size);
    _swap_x32(buf);
    return 0;
}

/* 0x800a51e4, size 0x58 */
int appFileRead4Array(int fd, void * buf, int size)
{
    unsigned char *p; /* r31 */

    appFileRead(fd, buf, size);
    p = buf;
    while (size) {
        _swap_x32(p);
        p += 4;
        size -= 4;
    }
    return 0;
}

/* 0x800a523c, size 0x68 */
int appFileReadTex(int fd, void * buf, int size, int num)
{
    int i; /* r31 */
    tTextureEx *pTexDef; /* r30 */

    appFileRead(fd, buf, size * num);
    for (i = 0, pTexDef = buf; i < num; pTexDef++, i++) {
        _swap_tex(pTexDef);
    }
    return 0;
}

/* 0x800a52a4, size 0x80 */
void _swap_tex(tTextureEx *pTex)
{
    _swap_x16(&pTex->width);
    _swap_x16(&pTex->height);
    _swap_x16(&pTex->palIndex);
    _swap_x32(&pTex->flags);
    _swap_x32(&pTex->xflags);
    _swap_x32(&pTex->uls);
    _swap_x32(&pTex->u);
    _swap_x16(&pTex->psm);
    _swap_x16(&pTex->ps2flags);
    _swap_x32(&pTex->texhead);
    _swap_x32(&pTex->cluthead);
}

/* 0x800a5324, size 0xcc */
void fix_ShFile(void * mem)
{
    int p; /* r31 */
    int *bnk_id; /* r31 */
    int *res_cnt; /* r30 */
    int i; /* r29 */
    _SND_RESOURCE_HEADER *psh; /* r28 */

    psh = 0;
    bnk_id = (int *)((int)mem + 4);
    _Check_swap((unsigned int *)bnk_id);
    _swap_x32(bnk_id);
    p = (int)mem + 8;
    res_cnt = (int *)p;
    _swap_x32(res_cnt);
    if (*(int *)p > 0) {
        p += 4;
        psh = (_SND_RESOURCE_HEADER *)p;
    }
    for (i = 0; i < *res_cnt; psh++, i++) {
        _swap_x32(&psh->res_type);
        _swap_x32_array((unsigned char *)&psh->resource, 0x20);
        _swap_x32(&psh->vol_line);
    }
    p += *res_cnt * sizeof(_SND_RESOURCE_HEADER);
    _swap_x32((void *)p);
    _swap_x32((void *)(p + 4));
}

/* 0x800a53f0, size 0x74 */
void SetFatalData(const char *strFile, unsigned long uLine, const char *strFunc)
{
    strcpy(FatalFile, strFile);
    FatalLine = uLine;
    strcpy(FatalFunc, strFunc);
    bFatalError = 1;
    g_bDvdIsError = 1;
    g_bShowLoadingEffect = 1;
    OSSuspendThread(OSGetCurrentThread());
}

/* 0x800a5464, size 0x8 */
int GetFatalError(void)
{
    return bFatalError;
}

/* 0x800a546c, size 0x34 */
void ShowDVDFatal(void)
{
    PrintFont(100, 175, "An error has occurred. Turn the power\noff and refer to the NINTENDO GAMECUBE\nInstruction Booklet for further instructions.\n", 1);
}

