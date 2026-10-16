#ifndef DOLPHIN_GX_TYPES_H
#define DOLPHIN_GX_TYPES_H

/* Minimal Dolphin SDK types used by the game, laid out as in rush_d.elf's debug info. */

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef enum { VI_NTSC = 0, VI_PAL = 1, VI_MPAL = 2, VI_DEBUG = 3, VI_DEBUG_PAL = 4, VI_EURGB60 = 5 } VITVFormat;

typedef struct _GXRenderModeObj {
    int viTVmode;
    unsigned short fbWidth;
    unsigned short efbHeight;
    unsigned short xfbHeight;
    unsigned short viXOrigin;
    unsigned short viYOrigin;
    unsigned short viWidth;
    unsigned short viHeight;
    int xFBmode;
    unsigned char field_rendering;
    unsigned char aa;
    unsigned char sample_pattern[12][2];
    unsigned char vfilter[7];
} GXRenderModeObj;

#endif
