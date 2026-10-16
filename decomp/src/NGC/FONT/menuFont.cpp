/* menuFont.cpp - indexed font decoding, texture setup, glyph rendering and wrapping. */
#define DrawFont HeaderDrawFont
#define GetFontWidth HeaderGetFontWidth
#define PrintFontEx HeaderPrintFontEx
#define new rush_new
extern "C" {
#include "rush/rush_protos.h"
void GXInitTlutObj(_GXTlutObj *, void *, int, unsigned short);
void GXLoadTlut(_GXTlutObj *, unsigned long);
void GXInitTexObjCI(_GXTexObj *, void *, unsigned short, unsigned short, int, int, int, unsigned char, unsigned long);
void GXLoadTexObj(_GXTexObj *, int);
void GXSetBlendMode(int, int, int, int);
void GXSetNumTexGens(unsigned char);
void GXSetTexCoordGen2(int, int, int, unsigned long, unsigned char, unsigned long);
void GXSetTevSwapMode(int, int, int);
}
#undef DrawFont
#undef GetFontWidth
#undef PrintFontEx
#undef new

static unsigned long uTextWidth = 640;
static int textureHeight, fontHeight, paletteSize;
static unsigned char *texture, *palette;
static unsigned short *metrics, *ngcPalette_;
static void *swizzleTextureData_;
static float space, scale, wspace, hspace;

static void RecordError(char *, int, int *, char *, int) {}
static unsigned long byte_swap(unsigned long n)
{
    return ((n << 24) & 0xff000000) | ((n << 8) & 0x00ff0000) | ((n >> 8) & 0x0000ff00) | ((n >> 24) & 0x000000ff);
}
static unsigned short byte_swap(unsigned short n) { return ((n << 8) & 0xff00) | (n >> 8); }
static unsigned short ReadAndFix16(unsigned char *mem, int needswap)
{
    unsigned short *mem16 = (unsigned short *)mem;
    if (needswap) return *mem16 = byte_swap(*mem16);
    else return *mem16;
}
static unsigned long ReadAndFix32(unsigned char *mem, int needswap)
{
    unsigned long *mem32 = (unsigned long *)mem;
    if (needswap) return *mem32 = byte_swap(*mem32);
    else return *mem32;
}
static void SwizzleTexture_8Bit(void *target, void *source, unsigned long textureWidth,
                              unsigned long visibleWidth, unsigned long visibleHeight)
{
    unsigned long *src = (unsigned long *)source;
    unsigned long y, y1;
    unsigned long *foo;
    unsigned long col;
    visibleWidth = (visibleWidth + 7) & ~7UL;
    for (y = 0; y < visibleHeight; y += 4) {
        for (y1 = 0; y1 < 4; y1++) {
            foo = (unsigned long *)((unsigned char *)target + y * textureWidth + y1 * 8);
            for (col = 0; col < visibleWidth; col += 32) {
                foo[0] = *src++; foo[1] = *src++;
                foo[8] = *src++; foo[9] = *src++;
                foo[16] = *src++; foo[17] = *src++;
                foo[24] = *src++; foo[25] = *src++;
                foo += 32;
            }
            src += ((textureWidth - visibleWidth) / 32) * 8;
        }
    }
}
static void PlatformSpecificInitialization(void)
{
    swizzleTextureData_ = AllocateMemoryHeap(0, GXGetTexBufferSize(256, 256, 9, 0, 0));
    SwizzleTexture_8Bit(swizzleTextureData_, texture, 256, 256, 256);
    static int bIgnoreAlways = 0;
    if (!bIgnoreAlways && !(long)swizzleTextureData_)
        RecordError("ASSERTION FAILED:\n     Description:Unable to aquire memory for texture object\n     Expression:swizzleTextureData_\n", 1, &bIgnoreAlways, "menuFont.cpp", 137);
}
static inline long FontNeedsSwap(unsigned long note) { return note == 0x78563412 ? 1 : 0; }
static void Initialize(unsigned char *rawData)
{
    unsigned long *swapNote;
    int mustSwap;
    {
        static int bIgnoreAlways = 0;
        if (!bIgnoreAlways && !rawData)
            RecordError("ASSERTION FAILED:\n     Description:NULL data given to Font\n     Expression:rawData != 0\n", 1, &bIgnoreAlways, "menuFont.cpp", 142);
    }
    {
        static int bIgnoreAlways = 0;
        if (!bIgnoreAlways && !(rawData[0] == 'T' && rawData[1] == 'O' && rawData[2] == 'N' && rawData[3] == 'F'))
            RecordError("ASSERTION FAILED:\n     Description:Invalid font data - bad magic number.\n     Expression:rawData[0] =='T' && rawData[1] =='O' && rawData[2] =='N' && rawData[3] =='F'\n", 1, &bIgnoreAlways, "menuFont.cpp", 144);
    }
    swapNote = (unsigned long *)(rawData + 4);
    if (*swapNote != 0x11223344) {
        {
            static int bIgnoreAlways = 0;
            if (!bIgnoreAlways && !(*swapNote == 0x78563412 || *swapNote == 0x12345678))
                RecordError("ASSERTION FAILED:\n     Description:Cannot initialize font data; byte order not understood.\n     Expression:*swapNote == SWAP_REQUIRED || *swapNote == ORDER_OK\n", 1, &bIgnoreAlways, "menuFont.cpp", 153);
        }
        mustSwap = FontNeedsSwap(*swapNote);
        paletteSize = ReadAndFix32(rawData + 8, mustSwap);
        palette = rawData + 12;
        metrics = (unsigned short *)(palette + paletteSize * 4);
        if (mustSwap) {
            for (int i = 0; i < 1024; i++) metrics[i] = byte_swap(metrics[i]);
        }
        if (metrics[130] == 0) metrics[130] = metrics[310] / 2;
        texture = (unsigned char *)(metrics + 1024);
        fontHeight = ReadAndFix16(texture, mustSwap);
        texture += 2;
        textureHeight = ReadAndFix16(texture, mustSwap);
        texture += 2;
        texture = rawData + (((texture - rawData) + 15) & ~15);
        PlatformSpecificInitialization();
        *swapNote = 0x11223344;
        unsigned char *p = palette;
        for (int i = 0; i < paletteSize; p += 4, i++) {
            for (int j = 0; j < 4; j++) {
                if (j != 3) {
                    if (p[j] > 235) p[j] = 235;
                    if (p[j] < 15) p[j] = 15;
                }
            }
        }
    }
}
void InitFont(void)
{
    Initialize(defaultFontData);
    ngcPalette_ = (unsigned short *)AllocateMemoryHeap(0, 512);
    space = 10.0f;
    scale = 1.0f;
}
static CharMetrics GetCharMetrics(int ch)
{
    CharMetrics cm;
    cm.tx = metrics[ch * 4]; cm.ty = metrics[ch * 4 + 1];
    cm.width = metrics[ch * 4 + 2]; cm.height = metrics[ch * 4 + 3];
    return cm;
}
void SetFont(float s1, float s2, float s3) { wspace = s1; hspace = s2; scale = s3; }
static void EndPrintFont(void) {}
static inline float FontBottom(float h, float y) { return y + h; }
float DrawFont(int x, int y, char ch, unsigned long uColor)
{
    CharMetrics charMetrics = GetCharMetrics(ch & 255);
    float u0, v0, u1, v1, w;
    float glyphScale;
    float z = 0.0f;
    u0 = charMetrics.tx / 256.0f; v0 = charMetrics.ty / 256.0f;
    u1 = (charMetrics.tx + charMetrics.width) / 256.0f;
    v1 = (charMetrics.ty + charMetrics.height) / 256.0f;
    glyphScale = scale;
    w = charMetrics.width * glyphScale;
    GXBegin(0x80, 0, 4);
    GXPosition3f32(x, y, z); GXColor1u32(uColor); GXTexCoord2f32(u0, v0);
    GXPosition3f32(x + w, y, z); GXColor1u32(uColor); GXTexCoord2f32(u1, v0);
    GXPosition3f32(x + w, FontBottom(charMetrics.height * glyphScale, y), z); GXColor1u32(uColor); GXTexCoord2f32(u1, v1);
    GXPosition3f32(x, FontBottom(charMetrics.height * glyphScale, y), z); GXColor1u32(uColor); GXTexCoord2f32(u0, v1);
    return w;
}
float GetFontWidth(char ch)
{
    CharMetrics charMetrics = GetCharMetrics((unsigned char)ch);
    return charMetrics.width * scale;
}
void PrintFontEx(int x, int y, char *str, int bEnable)
{
    unsigned long i;
    _GXTlutObj tlutObj;
    unsigned long bufferSize;
    _GXTexObj texObj;
    float width;
    ngcBlitInit_swap();
    GXSetBlendMode(1, 4, 5, 0);
    for (i = 0; i < 256; i++) {
        unsigned char *p = palette + i * 4;
        unsigned long color;
        if (p[3] >= 224)
            color = ((p[0] & 248) << 7) | ((p[1] & 248) << 2) | ((p[2] & 248) >> 3) | 0x8000;
        else
            color = ((p[0] & 240) << 4) | (p[1] & 240) | ((p[2] & 240) >> 4) | ((p[3] & 224) << 7);
        ngcPalette_[i] = color;
    }
    GXInitTlutObj(&tlutObj, ngcPalette_, 2, 256);
    DCStoreRange(ngcPalette_, 512); DCStoreRange(&tlutObj, 12); GXLoadTlut(&tlutObj, 0);
    bufferSize = GXGetTexBufferSize(256, 256, 9, 0, 0);
    DCStoreRange(swizzleTextureData_, bufferSize);
    GXInitTexObjCI(&texObj, swizzleTextureData_, 256, 256, 9, 0, 0, 0, 0);
    DCStoreRange(&texObj, 32); GXLoadTexObj(&texObj, 0);
    GXSetNumTevStages(1); GXSetNumChans(1); GXSetNumTexGens(1);
    GXSetTexCoordGen2(0, 1, 4, 60, 0, 125);
    GXSetTevOp(0, 0); GXSetTevSwapMode(0, 0, 0);
    GXClearVtxDesc(); GXSetVtxDesc(9, 1); GXSetVtxDesc(11, 1); GXSetVtxDesc(13, 1);
    GXSetVtxAttrFmt(0, 9, 1, 4, 0); GXSetVtxAttrFmt(0, 11, 1, 5, 0); GXSetVtxAttrFmt(0, 13, 1, 4, 0);
    width = 0.0f;
    while (*str) {
        if (*str == '\n') { width = 0.0f; y += hspace; str++; }
        else if (*str == ' ') { width += wspace; str++; }
        else { width += DrawFont(x + width, y, *str, bEnable ? 0xffffffff : 0x88888888); str++; }
    }
    EndPrintFont();
}
void PrintFont(int x, int y, char *str, int bEnable)
{
    float width = 0.0f;
    char *oriStr;
    char *tempStr;
    static char ConvStr[32768];
    strcpy(ConvStr, str);
    str = ConvStr; oriStr = str;
    while (*str) {
        if (*str == '\n') { width = 0.0f; str++; }
        else if (*str == ' ') { width += wspace; str++; }
        else {
            if (width > uTextWidth) {
                tempStr = str;
                while (tempStr != oriStr) {
                    if (*--tempStr == ' ') { *tempStr = '\n'; width = 0.0f; str = tempStr + 1; break; }
                }
            }
            width += GetFontWidth(*str); str++;
        }
    }
    PrintFontEx(x, y, ConvStr, bEnable);
}
void SetPrintWidth(unsigned long uWidth) { uTextWidth = uWidth; }
