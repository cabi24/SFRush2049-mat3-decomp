/* TB_text.c - Text layout, formatting, and byte/double-byte string helpers. Status: in progress.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\TB_text.c; assembly: build/2049LE/asm/GameSource/src/TB_text.s */
#include "rush/rush_protos.h"
#include "rush/libc.h"

static void TB_TextInit(void);
static void TB_TextOncePerFrame(void);

static unsigned char gFontColors[22][2][4] = {
    0, 0, 0, 255, 0, 0, 0, 255, 230, 230, 230, 255, 60, 60, 60, 255, 160, 160, 160, 255, 40, 40,
    40, 255, 120, 120, 120, 255, 40, 40, 40, 255, 80, 80, 80, 255, 32, 32, 32, 255, 230, 0, 0, 255,
    0, 0, 0, 255, 230, 0, 230, 255, 0, 0, 0, 255, 64, 64, 230, 255, 0, 0, 0, 255, 0, 0, 175, 255,
    0, 0, 0, 255, 0, 0, 127, 255, 0, 0, 0, 255, 0, 230, 230, 255, 0, 0, 0, 255, 0, 230, 0, 255, 0,
    0, 0, 255, 0, 175, 0, 255, 0, 0, 0, 255, 0, 127, 0, 255, 0, 0, 0, 255, 230, 230, 0, 255, 40,
    40, 0, 255, 127, 127, 0, 255, 30, 30, 0, 255, 255, 255, 0, 255, 0, 0, 0, 255, 230, 230, 0, 255,
    190, 0, 0, 255, 230, 0, 0, 255, 230, 230, 0, 255, 230, 0, 0, 255, 230, 0, 0, 255, 0, 230, 230,
    255, 0, 230, 230, 255, 0, 230, 230, 255, 0, 230, 0, 255
};
static anon_h_v_8 gJust = { 0, 3 };
static tRGBA gFlashColor1 = { 230, 230, 0, 255 };
static tRGBA gFlashColor2 = { 0, 0, 0, 255 };
static float gColorCycle;
static float gColorCycleTime;
static signed char gInited;

/* 0x8004f1bc, size 0x40 */
static void TB_TextInit(void)
{
    if (!gInited) { gInited = 1; TB_TextOncePerFrame(); HAL_InstallOncePerFrameFunc(TB_TextOncePerFrame); }
}

/* 0x8004f1fc, size 0x80 */
static void TB_TextOncePerFrame(void)
{
    gColorCycleTime += gFrameSeconds;
    gColorCycle = 2.0f * gColorCycleTime;
    gColorCycle -= (int)gColorCycle;
    if (gColorCycle > 0.5f) gColorCycle = 1.0f - gColorCycle;
    gColorCycle *= 2.0f;
}

/* 0x8004f27c, size 0x40 */
signed short TB_CenterText(void * text, signed short centerX)
{
    return centerX - (HAL_GetTextWidth(text, -1) >> 1);
}

/* 0x8004f2bc, size 0x3c */
signed short TB_RightText(void * text, signed short rightX)
{
    return rightX - HAL_GetTextWidth(text, -1);
}

/* 0x8004f2f8, size 0x10 */
void TB_SetJust(enum_kTB_JustBottom hJust, enum_kTB_JustBottom vJust)
{
    gJust.h = hJust; gJust.v = vJust;
}

/* 0x8004f308, size 0x16c */
void TB_SetTextColor(enum_kTB_FlashText color)
{
    float c2;
    if (color == 22) {
        if (!gInited) TB_TextInit();
        c2 = 1.0f - gColorCycle;
        HAL_SetTextColor(gFlashColor1.c[0]*gColorCycle + gFlashColor2.c[0]*c2,
                        gFlashColor1.c[1]*gColorCycle + gFlashColor2.c[1]*c2,
                        gFlashColor1.c[2]*gColorCycle + gFlashColor2.c[2]*c2,
                        gFlashColor1.c[3]*gColorCycle + gFlashColor2.c[3]*c2);
    } else HAL_SetTextColor(gFontColors[color][0][0],gFontColors[color][0][1],gFontColors[color][0][2],gFontColors[color][0][3]);
}

/* 0x8004f474, size 0x14 */
void TB_FlashTextColors(tRGBA * __restrict color1, tRGBA * __restrict color2)
{
    gFlashColor1.rgba = color1->rgba;
    gFlashColor2.rgba = color2->rgba;
}

/* 0x8004f488, size 0x8 */
float TB_GetFlashTextCycle(void)
{
    return gColorCycle;
}

/* 0x8004f490, size 0x3c */
signed short TB_GetFontHeight(void)
{
    return HAL_GetFontAscent() + HAL_GetFontDescent();
}

/* 0x8004f4cc, size 0x50 */
signed short TB_GetFontLineHeight(void)
{
    return HAL_GetFontAscent() + HAL_GetFontDescent() + HAL_GetFontLeading();
}

/* 0x8004f51c, size 0x14c */
signed short TB_GetTextNumLines(signed short width, void * text)
{
    signed short spacing, charSize, numLines, currWidth, ch;
    const unsigned char *str, *s, *lastSpace;
    const unsigned char *txt = text;
    signed char dbl = *txt == 255;
    if (dbl) { str = txt + 1; charSize = 2; } else { str = txt; charSize = 1; }
    spacing = HAL_GetFontSpacing(); numLines = 0;
    do {
        s = str; currWidth = 0; lastSpace = 0;
        for (;;) {
            if (dbl) ch = (s[0]<<8)|s[1]; else ch = *s;
            if (ch == ' ' && currWidth == 0) { str += charSize; s = str; continue; }
            if (!ch || ch == 13 || ch == 10) break;
            currWidth += HAL_GetCharWidth(ch);
            if (ch != ' ') currWidth += spacing;
            else if (lastSpace != s-charSize) lastSpace = s;
            if (currWidth > width && lastSpace) { s = lastSpace; break; }
            s += charSize;
        }
        numLines++;
        if (!ch || numLines == 32767) break;
        str = s + charSize;
    } while (1);
    return numLines;
}

/* 0x8004f668, size 0x190 */
void TB_EllipseText(void * dst, void * src, signed short width)
{
    unsigned char *d = dst; int w; int dbl; int len; unsigned char eStr[16]; int ew; const unsigned char *ellipse = (const unsigned char *)"...";
    w = HAL_GetTextWidth(src,-1);
    if (w <= width) TB_Strcpy(d,src);
    else {
        dbl = *d == 255;
        TB_Strcpy(d,src);
        len = TB_Strlen(d);
        if (len <= 1) return;
        len -= 2;
        if (dbl) {
            eStr[0] = 255; TB_Strcat(eStr,(void *)ellipse);
            eStr[7] = d[len*2+1]; eStr[8] = d[len*2+2];
            eStr[9] = d[len*2+3]; eStr[10] = d[len*2+4];
            eStr[11] = 0; eStr[12] = 0; d[len*2+1]=0; d[len*2+2]=0;
        } else {
            unsigned char *end;
            TB_Strcpy(eStr,(void *)ellipse);
            end=d+len;
            TB_Strcat(eStr,end);
            *end=0;
        }
        ew = width - HAL_GetTextWidth(eStr,-1);
        while (w > ew - HAL_GetFontSpacing()) {
            len--;
            if (dbl) { d[len*2+1]=0; d[len*2+2]=0; } else d[len]=0;
            if (len == 0) break;
            w = HAL_GetTextWidth(d,-1);
        }
        TB_Strcat(d,eStr);
    }
}

/* 0x8004f7f8, size 0x5c */
unsigned int TB_Strlen(void * s)
{
    unsigned int len=0;
    const unsigned char *str=s;
    if (*str == 255) { str++; while (str[0] || str[1]) { str+=2; len++; } }
    else while (*str++) len++;
    return len;
}

/* 0x8004f854, size 0x7c */
void * TB_Strcpy(void * str1, void * str2)
{
    unsigned char *s=str1, *s1=str1;
    const unsigned char *s2=str2;
    if (*s2 == 255) {
        *s1++ = *s2++;
        while (s2[0] || s2[1]) { unsigned char a=*(const volatile unsigned char *)s2,b=s2[1]; s2+=2; s1[0]=a; s1[1]=b; s1+=2; }
        s1[0]=0; s1[1]=0;
    } else while ((*s1++ = *s2++) != 0) {}
    return s;
}

/* 0x8004f8d0, size 0x160 */
void * TB_Strcat(void * str1, void * str2)
{
    unsigned char *s1=str1; unsigned char temp[256]; const unsigned char *s2=str2; unsigned char *s=str1; unsigned char *t;
    if (*s1 == 255 || *s2 == 255) {
        if (*s1 == 255) { s1++; while (s1[0] || s1[1]) s1+=2; }
        else {
            t=temp; while ((*t++ = *s++) != 0) {}
            *s1++=255; s=str1; t=temp;
            while (*t) { s1[0]=0; s1[1]=*t++; s1+=2; }
        }
        if (*s2 == 255) {
            s2++; while (s2[0] || s2[1]) { unsigned char a=*(const volatile unsigned char *)s2,b=s2[1]; s2+=2; s1[0]=a; s1[1]=b; s1+=2; }
        } else while (*s2) { unsigned char c=*s2++; s1[0]=0; s1[1]=c; s1+=2; }
        s1[0]=0; s1[1]=0;
    } else {
        while (*s1) s1++;
        while ((*s1++ = *s2++) != 0) {}
    }
    return s;
}

/* 0x8004fa30, size 0x8c */
void TB_Sprintf(void * buf, void * fmt, ...)
{
    va_list ap; va_start(ap,fmt); vsprintf(buf,fmt,(char *)ap); va_end(ap);
}

/* 0x8004fabc, size 0xbc */
void TB_Printf(signed short x, signed short y, void * fmt, ...)
{
    char str[256]; va_list ap; va_start(ap,fmt); vsprintf(str,fmt,(char *)ap); va_end(ap); TB_DrawText(x,y,str);
}

/* 0x8004fb78, size 0xd0 */
void TB_DrawText(signed short x, signed short y, void * text)
{
    if (gJust.h == 1) x=TB_CenterText(text,x);
    else if (gJust.h == 2) x=TB_RightText(text,x);
    if (gJust.v == 1) y -= TB_GetFontHeight()/2;
    else if (gJust.v == 4) y -= TB_GetFontHeight();
    HAL_SetTextPos(x,y); HAL_DrawText(text,-1);
}

/* 0x8004fc48, size 0x7c */
void TB_DrawEllipseText(signed short x, signed short y, signed short w, void * text)
{
    unsigned char str[128] = {0}; TB_EllipseText(str,text,w); TB_DrawText(x,y,str);
}

/* 0x8004fcc4, size 0x58 */
void TB_DrawTextC(signed short x, signed short y, enum_kTB_FlashText color, void * text)
{
    TB_SetTextColor(color); TB_DrawText(x,y,text);
}

/* 0x8004fd1c, size 0x33c */
void TB_DrawTextScrollBox(signed short x, signed short y, signed short width, signed short height, signed short page, signed short line, void * text)
{
    signed short top,bottom,fontHeight,spacing,charSize,numLines,currentLine,currentPage,len,overlap,currWidth,ch;
    const unsigned char *s,*lastSpace;
    unsigned char *str; unsigned char saveChar; signed char dbl;
    fontHeight=TB_GetFontLineHeight(); spacing=HAL_GetFontSpacing(); top=y;
    if (gJust.v != 3) {
        numLines=TB_GetTextNumLines(width,text);
        if (gJust.v == 1) top=y-(numLines*fontHeight)/2;
        else top=y-numLines*fontHeight;
    }
    str=text; bottom=top+height; dbl=*str == 255;
    if (dbl) { str++; charSize=2; } else charSize=1;
    currentLine=0;
    if (page<0) { currentPage=page; bottom+=fontHeight; } else currentPage=0;
    overlap=currentPage<page ? fontHeight:(signed short)0;
    do {
        s=str; len=0; currWidth=0; lastSpace=0;
        for (;;) {
            if (dbl) ch=(s[0]<<8)|s[1]; else ch=*s;
            if (ch==' ' && currWidth==0) { str+=charSize; s=str; continue; }
            if (!ch || ch==13 || ch==10) break;
            currWidth+=HAL_GetCharWidth(ch);
            if (ch!=' ') currWidth+=spacing;
            else if (lastSpace!=s-charSize) lastSpace=s;
            if (currWidth>width && lastSpace) {
                if (dbl) len-=(s-lastSpace)>>1; else len-=s-lastSpace;
                s=lastSpace; break;
            }
            s+=charSize; len++;
        }
        if (currentPage>=page) {
            if (currentLine>=line) {
                if (len>0) {
                    int xpos; signed short tw;
                    if (dbl) { saveChar=str[-1]; str[-1]=255; }
                    tw=HAL_GetTextWidth(str-(dbl ? 1 : 0),len);
                    if (gJust.h==1) xpos=x-(tw>>1);
                    else { xpos=x; if (gJust.h==2) xpos-=tw; }
                    HAL_SetTextPos(xpos,top); HAL_DrawText(str-(dbl ? 1 : 0),len);
                    if (dbl) str[-1]=saveChar;
                }
                top+=fontHeight;
            }
            currentLine++;
        } else top+=fontHeight;
        if (!ch) break;
        str=(unsigned char *)(s+charSize);
        if (top>bottom-fontHeight-overlap) {
            currentPage++; top=y;
            if (currentPage>page) break;
            if (currentPage==page) overlap=0;
        }
    } while (1);
}

