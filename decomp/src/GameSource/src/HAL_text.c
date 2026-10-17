/* HAL_text.c - Font setup, text metrics, buffered text commands and glyph drawing.
   Status: 28 of 28 functions exact; verified in isolated GC/2.6 compile and GC/2.7 link.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\HAL_text.c; assembly: build/2049LE/asm/GameSource/src/HAL_text.s */
#include "rush/rush_protos.h"

static int _HAL_SetFont(int fontID);
static void _SetupFontData(int force);
static void _DrawText(const unsigned char *text, unsigned short length);
static void _SetDefaults(anon_fore_4 *fontColor, float *trans, signed short *fontXpos, signed short *fontYpos,
                         signed char *fontIndex, signed char *leading, signed char *spacing);
static unsigned int _TextLen(const unsigned char *text, signed short numChars);
static void _InitSemaphore(void);
static void _GetSemaphore(void);
static void _ReleaseSemaphore(void);

static signed char gFinished = 1;
static float gDepth = -1.0f;
static float gTrans = -1.0f;
static int gFontDatWaveInst;
static int gFontTexWaveInst;
static anon_version_type_10 *gFontHeader;
static anon_kerns_value_c *gFontLetters;
static anon_fore_4 gFontColor;
static signed char gLeading;
static signed char gSpacing;
static int gFontAttribute;
static signed short gFontXpos;
static signed short gFontYpos;
static signed char gFontIndex;
static int gBufLen;
static void *gFontHeap = 0;
static signed char gBufferInited = 0;
static anon_name_width_30 *gFontTextures[16];
static signed short gCharMap[256];
static unsigned char gBuffer[800];

/* 0x8001a31c, size 0x28 */
void HAL_InitText(void)
{
    _InitSemaphore();
    gFontIndex = -1;
}

/* 0x8001a344, size 0x68 */
void HAL_StartText(void)
{
    _GetSemaphore();
    _SetDefaults(&gFontColor, &gTrans, &gFontXpos, &gFontYpos, &gFontIndex, &gLeading, &gSpacing);
    _HAL_SetFont(gFontIndex);
    gFontAttribute = 0;
    gFontAttribute |= 0x8000;
    gBufferInited = 0;
    gFinished = 0;
    _ReleaseSemaphore();
}

/* 0x8001a3ac, size 0x1dc */
void HAL_FinishText(signed char end)
{
    int i; /* r30 */
    int length; /* r29 */
    unsigned int aLong; /* stack */
    if (gBufferInited) {
        _GetSemaphore();
        gFinished = end;
        _SetDefaults(&gFontColor, &gTrans, &gFontXpos, &gFontYpos, &gFontIndex, &gLeading, &gSpacing);
        _HAL_SetFont(gFontIndex);
        gFontAttribute &= 0x8000;
        for (i = 0; i < gBufLen;) {
            switch (gBuffer[i++]) {
                case 0:
                    _HAL_SetFont(gBuffer[i++]);
                    break;
                case 1:
                    HAL_SetFontLeading(gBuffer[i++]);
                    break;
                case 2:
                    HAL_SetFontSpacing(gBuffer[i++]);
                    break;
                case 3:
                    HAL_SetTextPos(((short)gBuffer[i] << 8) | gBuffer[i + 1],
                                   ((short)gBuffer[i + 2] << 8) | gBuffer[i + 3]);
                    i += 4;
                    break;
                case 4:
                    aLong =
                        (gBuffer[i] << 24) | (gBuffer[i + 1] << 16) | (gBuffer[i + 2] << 8) | gBuffer[i + 3];
                    i += 4;
                    HAL_SetTextTranslucency(*(float *)&aLong);
                    break;
                case 5:
                    HAL_SetTextColor(gBuffer[i], gBuffer[i + 1], gBuffer[i + 2], gBuffer[i + 3]);
                    i += 4;
                    break;
                case 6:
                    length = gBuffer[i++];
                    _DrawText(gBuffer + i, length);
                    i += length;
                    break;
            }
        }
        _ReleaseSemaphore();
    }
}

/* 0x8001a588, size 0xec */
static int _HAL_SetFont(int fontID)
{
    int old; /* r31 */
    old = gFontIndex;
    gFontIndex = fontID;
    if (fontID != -1) {
        gFontDatWaveInst = FindWaveInstance(gFontIndex + 24);
        if (gFontDatWaveInst < 0)
            gFontDatWaveInst = LoadWave(gFontIndex + 24, 0, 0, 1, 0);
        gFontTexWaveInst = FindWaveInstance(gFontIndex + 8);
        if (gFontTexWaveInst < 0)
            gFontTexWaveInst = LoadWave(gFontIndex + 8, 0, 0, 0, gFontHeap);
        _SetupFontData(old != fontID);
    }
    if (fontID == 0)
        HAL_SetFontFixedWidth(1);
    return old;
}

/* 0x8001a674, size 0x40 */
int HAL_SetFont(int fontID)
{
    int old; /* r31 */
    _GetSemaphore();
    old = _HAL_SetFont(fontID);
    _ReleaseSemaphore();
    return old;
}

/* 0x8001a6b4, size 0x5c */
void HAL_PurgeFonts(void)
{
    int wave; /* r0 */
    int i; /* r31 */
    for (i = 0; i < 16; i++) {
        wave = FindWaveInstance(i + 24);
        if (wave >= 0)
            UnlockWaveInstance(wave);
        wave = FindWaveInstance(i + 8);
        if (wave >= 0)
            UnlockWaveInstance(wave);
    }
}

/* 0x8001a710, size 0x238 */
static void _SetupFontData(int force)
{
    int i; /* r6 */
    unsigned char *data; /* r30 */
    void *handle; /* r0 */
    handle = GetWaveInstanceAddr(gFontDatWaveInst);
    data = *(unsigned char **)handle;
    if (force || gFontHeader != (void *)data) {
        gFontHeader = (void *)data;
        data += sizeof(*gFontHeader);
        _Enable_swap();
        gFontLetters = (void *)data;
        for (i = 0; i < gFontHeader->numChars; i++) {
        }
        data += gFontHeader->numChars * sizeof(*gFontLetters);
        if (gFontHeader->kern)
            for (i = 0; i < gFontHeader->numChars; i++) {
                gFontLetters[i].kerns = data;
                data += gFontHeader->numChars;
            }
        for (i = 0; i < 256; i++)
            gCharMap[i] = -1;
        for (i = 0; i < gFontHeader->numChars; i++)
            gCharMap[gFontLetters[i].value] = i;
    }
    if (force) {
        for (i = 0; i < gFontHeader->numTextures; i++)
            gFontTextures[i] = gTexDefPtrs[gFontTexWaveInst].texDefs + i;
        gLeading = gFontHeader->leading;
        gSpacing = gFontHeader->spacing;
    }
}

/* 0x8001a948, size 0x78 */
void HAL_SetTextColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
    if (gFontColor.fore.rgba[0] != red || gFontColor.fore.rgba[1] != green ||
        gFontColor.fore.rgba[2] != blue || gFontColor.fore.rgba[3] != alpha) {
        gFontColor.fore.rgba[0] = red;
        gFontColor.fore.rgba[1] = green;
        gFontColor.fore.rgba[2] = blue;
        gFontColor.fore.rgba[3] = alpha;
        BlitClearColors();
    }
}

/* 0x8001a9c0, size 0x58 */
void HAL_SetTextDepth(float depth)
{
    gDepth = depth;
    if (depth > 0.0f) {
        gFontAttribute |= 16;
        BlitZDist(depth);
    } else
        gFontAttribute &= ~16;
}

/* 0x8001aa18, size 0x58 */
void HAL_SetMenuTextDepth(float depth)
{
    gDepth = depth;
    if (depth > 0.0f) {
        gFontAttribute |= 16;
        BlitZDist(depth);
    } else
        gFontAttribute &= ~16;
}

/* 0x8001aa70, size 0x68 */
void HAL_SetTextTranslucency(float xlu)
{
    gTrans = xlu;
    if (xlu < 0.0f || xlu >= 255.0f)
        gFontAttribute &= ~32;
    else {
        gFontAttribute |= 32;
        BlitXLU(xlu);
    }
}

/* 0x8001aad8, size 0x60 */
void HAL_SetTextPos(signed short x, signed short y)
{
    if (x != -32768)
        gFontXpos = x * 2;
    if (y != -32768)
        gFontYpos = (int)(2.0f * y);
}

/* 0x8001ab38, size 0x74 */
void HAL_SetFontLeading(signed short leading)
{
    if (leading < 0) {
        _SetupFontData(0);
        gLeading = gFontHeader->leading;
    } else
        gLeading = (int)(2.0f * leading);
}

/* 0x8001abac, size 0x48 */
void HAL_SetFontSpacing(signed short spacing)
{
    if (spacing < 0) {
        _SetupFontData(0);
        gSpacing = gFontHeader->spacing;
    } else
        gSpacing = spacing * 2;
}

/* 0x8001abf4, size 0x40 */
signed char HAL_SetFontFixedWidth(signed char fixedWidth)
{
    signed char retFixedWidth; /* r3 */
    _SetupFontData(0);
    retFixedWidth = gFontHeader->nonProp;
    gFontHeader->nonProp = fixedWidth;
    return retFixedWidth;
}

/* 0x8001ac34, size 0x58 */
unsigned char HAL_GetFontAscent(void)
{
    _SetupFontData(0);
    return gFontHeader->ascent / 2.0f;
}

/* 0x8001ac8c, size 0x58 */
unsigned char HAL_GetFontDescent(void)
{
    _SetupFontData(0);
    return gFontHeader->descent / 2.0f;
}

/* 0x8001ace4, size 0x44 */
signed char HAL_GetFontLeading(void)
{
    return gLeading / 2.0f;
}

/* 0x8001ad28, size 0x1c */
signed char HAL_GetFontSpacing(void)
{
    return gSpacing / 2;
}

/* 0x8001ad44, size 0xc8 */
unsigned int HAL_GetCharWidth(unsigned short ch)
{
    signed short c; /* r3 */
    _SetupFontData(0);
    if (gFontHeader->nonProp)
        return gFontHeader->maxWidth / 2;
    if (ch == 32 || ch > 255)
        return gFontHeader->spaceWidth / 2;
    if (ch == 10)
        return 0;
    c = gCharMap[ch];
    if (c < 0)
        return gFontHeader->spaceWidth / 2;
    return (gFontLetters[c].lrs - gFontLetters[c].uls + 1) / 2;
}

/* 0x8001ae0c, size 0x1bc */
unsigned int HAL_GetTextWidth(void *text, signed short numChars)
{
    int i; /* r6 */
    int len; /* r7 */
    int step; /* r8 */
    int ch; /* r31 */
    int chPrev; /* r9 */
    int nc; /* r10 */
    int dbl; /* r11 */
    const unsigned char *t; /* r12 */
    _SetupFontData(0);
    if (*(unsigned char *)text == 255) {
        t = (unsigned char *)text + 1;
        dbl = 1;
        step = 2;
    } else {
        t = text;
        dbl = 0;
        step = 1;
    }
    if (!t[0] && (!dbl || !t[1]))
        return 0;
    i = 0;
    len = 0;
    nc = 0;
    chPrev = -1;
    while ((numChars < 0 || nc < numChars) && (t[i] || (dbl && t[i + 1]))) {
        if (gFontHeader->nonProp)
            len = gFontHeader->maxWidth + gSpacing + len;
        else {
            if (dbl)
                ch = (t[i] << 8) | t[i + 1];
            else
                ch = t[i];
            if (ch == 32 || ch > 255) {
                len += gFontHeader->spaceWidth;
                ch = -1;
            } else if (ch == 10)
                break;
            else {
                ch = gCharMap[ch];
                if (ch < 0)
                    len += gFontHeader->spaceWidth;
                else {
                    len += gFontLetters[ch].lrs - gFontLetters[ch].uls + 1 + gSpacing;
                    if (chPrev > 0 && gFontHeader->kern)
                        len -= gFontLetters[ch].kerns[chPrev];
                }
            }
        }
        chPrev = ch;
        i += step;
        nc++;
    }
    return (len - gSpacing) / 2;
}

/* 0x8001afc8, size 0x448 */
void HAL_DrawText(void *text, signed short numChars)
{
    int size; /* r31 */
    int length; /* r30 */
    char str[256]; /* stack */
    static anon_fore_4 lastFontColor;
    static float lastTrans;
    static signed short lastFontXpos;
    static signed short lastFontYpos;
    static signed char lastFontIndex;
    static signed char lastLeading;
    static signed char lastSpacing;
    if (gDepth >= 0.0f) {
        _DrawText(text, _TextLen(text, numChars));
        return;
    }
    if (gFinished) {
        if (numChars < 0)
            strcpy(str, text);
        else
            strncpy(str, text, numChars);
    }
    _GetSemaphore();
    if (!gBufferInited) {
        gBufferInited = 1;
        gBufLen = 0;
        _SetDefaults(&lastFontColor, &lastTrans, &lastFontXpos, &lastFontYpos, &lastFontIndex, &lastLeading,
                     &lastSpacing);
    }
    size = 0;
    if (lastFontIndex != gFontIndex)
        size = 2;
    if (lastLeading != gLeading)
        size += 2;
    if (lastSpacing != gSpacing)
        size += 2;
    if (lastFontXpos != gFontXpos || lastFontYpos != gFontYpos)
        size += 5;
    if (lastTrans != gTrans)
        size += 5;
    if (lastFontColor.fore.rgbaU32 != gFontColor.fore.rgbaU32)
        size += 5;
    length = _TextLen(text, numChars);
    size += length + 2;
    if (gBufLen + size >= 800) {
        _ReleaseSemaphore();
        return;
    }
    if (lastFontIndex != gFontIndex) {
        gBuffer[gBufLen++] = 0;
        gBuffer[gBufLen++] = lastFontIndex = gFontIndex;
    }
    if (lastLeading != gLeading) {
        gBuffer[gBufLen++] = 1;
        gBuffer[gBufLen++] = lastLeading = gLeading;
    }
    if (lastSpacing != gSpacing) {
        gBuffer[gBufLen++] = 2;
        lastSpacing = gSpacing;
        gBuffer[gBufLen++] = gSpacing / 2;
    }
    if (lastFontXpos != gFontXpos || lastFontYpos != gFontYpos) {
        lastFontXpos = gFontXpos / 2;
        lastFontYpos = gFontYpos / 2.0f;
        gBuffer[gBufLen++] = 3;
        gBuffer[gBufLen++] = (lastFontXpos >> 8) & 255;
        gBuffer[gBufLen++] = lastFontXpos;
        gBuffer[gBufLen++] = (lastFontYpos >> 8) & 255;
        gBuffer[gBufLen++] = lastFontYpos;
        lastFontXpos = gFontXpos;
        lastFontYpos = gFontYpos;
    }
    if (lastTrans != gTrans) {
        lastTrans = gTrans;
        gBuffer[gBufLen++] = 4;
        gBuffer[gBufLen++] = (*(unsigned int *)&gTrans) >> 24;
        gBuffer[gBufLen++] = (*(unsigned int *)&gTrans) >> 16;
        gBuffer[gBufLen++] = (*(unsigned int *)&gTrans) >> 8;
        gBuffer[gBufLen++] = *(unsigned int *)&gTrans;
    }
    if (lastFontColor.fore.rgbaU32 != gFontColor.fore.rgbaU32) {
        lastFontColor = gFontColor;
        gBuffer[gBufLen++] = 5;
        memcpy(gBuffer + gBufLen, &gFontColor, 4);
        gBufLen += 4;
    }
    gBuffer[gBufLen++] = 6;
    gBuffer[gBufLen++] = length;
    memcpy(gBuffer + gBufLen, text, length);
    gBufLen += length;
    _ReleaseSemaphore();
}

/* 0x8001b410, size 0x2b4 */
static void _DrawText(const unsigned char *text, unsigned short length)
{
    int i; /* r29 */
    int x; /* r4 */
    int x0; /* r28 */
    int y0; /* r27 */
    int step; /* r26 */
    int ch; /* r0 */
    int chPrev; /* r5 */
    int lastPage; /* r25 */
    const unsigned char *t; /* r24 */
    int dbl; /* r23 */
    anon_kerns_value_c *l; /* r22 */
    _SetupFontData(0);
    BlitScissor(0, 0, 639, 447);
    BlitSetAttribute(gFontAttribute);
    BlitClearAttribute(~gFontAttribute);
    BlitClearColors();
    if (gFontAttribute & 32)
        gFontColor.fore.rgba[3] = (int)gTrans;
    ((void (*)(void *, int, int, int))BlitColors)(&gFontColor, 0, 0, 0);
    lastPage = 0;
    if (text[0] == 255) {
        t = text + 1;
        dbl = 1;
        step = 2;
        length -= 2;
    } else {
        t = text;
        dbl = 0;
        step = 1;
        length -= 1;
    }
    x0 = gFontXpos;
    y0 = gFontYpos;
    i = 0;
    chPrev = -1;
    for (; i < length;) {
        if (dbl)
            ch = (t[i] << 8) | t[i + 1];
        else
            ch = t[i];
        if (ch == 10) {
            ch = -1;
            y0 += gFontHeader->ascent + gFontHeader->descent + gLeading;
            x0 = gFontXpos;
        } else {
            if (ch == 32 || ch > 255) {
            space:
                x0 += gFontHeader->nonProp ? gFontHeader->maxWidth + gSpacing : gFontHeader->spaceWidth;
                ch = -1;
            } else {
                ch = gCharMap[ch];
                if (ch < 0)
                    goto space;
                if (chPrev > 0 && gFontHeader->kern && !gFontHeader->nonProp)
                    x0 -= gFontLetters[ch].kerns[chPrev];
                l = gFontLetters + ch;
                if (l->texIdx != lastPage)
                    lastPage = l->texIdx;
                x = gFontHeader->nonProp ? (gFontHeader->maxWidth - (l->lrs - l->uls + 1)) / 2 : 0;
                x = x0 + x;
                BlitDraw(gFontTextures[lastPage], x, y0, x + l->lrs - l->uls, y0 + l->lrt - l->ult + 1,
                         l->uls + 1, l->ult + 1);
                x0 += (gFontHeader->nonProp ? gFontHeader->maxWidth : l->lrs - l->uls + 1) + gSpacing;
            }
        }
        i += step;
        chPrev = ch;
    }
}

/* 0x8001b6c4, size 0x40 */
static void _SetDefaults(anon_fore_4 *fontColor, float *trans, signed short *fontXpos, signed short *fontYpos,
                         signed char *fontIndex, signed char *leading, signed char *spacing)
{
    fontColor->fore.rgba[0] = fontColor->fore.rgba[1] = fontColor->fore.rgba[2] = 230;
    fontColor->fore.rgba[3] = 255;
    *trans = -1.0f;
    *fontXpos = 0;
    *fontYpos = 0;
    *fontIndex = -1;
    *leading = -1;
    *spacing = -1;
}

/* 0x8001b704, size 0x88 */
static unsigned int _TextLen(const unsigned char *text, signed short numChars)
{
    unsigned int len; /* r7 */
    int nc; /* r8 */
    nc = 0;
    if (text[0] == 255) {
        len = 1;
        do {
            len += 2;
            nc++;
        } while ((text[len - 1] || text[len - 2]) && (numChars < 0 || nc <= numChars));
    } else {
        len = 0;
        do {
            len++;
            nc++;
        } while (*text++ && (numChars < 0 || nc <= numChars));
    }
    return len;
}

/* 0x8001b78c, size 0x18 */
static void _InitSemaphore(void)
{
    static signed char sInited = 0;
    if (!sInited)
        sInited = 1;
}

/* 0x8001b7a4, size 0x4 */
static void _GetSemaphore(void) {}

/* 0x8001b7a8, size 0x4 */
static void _ReleaseSemaphore(void) {}
