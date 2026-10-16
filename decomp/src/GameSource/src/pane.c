/* pane.c - bordered rectangles (panes) built from five blits.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\pane.c; assembly: build/2049LE/asm/GameSource/src/pane.s */
#include "rush/rush_protos.h"

#define SWAP(a, b) ((a) ^= (b), (b) ^= (a), (a) ^= (b))

static anon_used_perm_20 gPaneBlits[16] = { 0 };

/* 0x800305e8, size 0x218 */
anon_used_perm_20 *InitPane(int x1, int y1, int x2, int y2, int alpha, int (*animFunc)(_tBlit_2 *), unsigned int animID, int permanent)
{
    int i; /* r3 (the DWARF lists a second i in r21) */
    anon_used_perm_20 *pane; /* r28 */
    for (i = 0, pane = gPaneBlits; i < 16; i++, pane++) {
        if (!pane->used) {
            break;
        }
    }
    if (i >= 16) {
        return pane;
    }
    if (x1 > x2) {
        SWAP(x1, x2);
    }
    if (y1 > y2) {
        SWAP(y1, y2);
    }
    pane->blackColor[0] = 0;
    pane->blackColor[1] = 0;
    pane->blackColor[2] = 0x10;
    pane->blackColor[3] = alpha;
    pane->grayColor[0] = 0;
    pane->grayColor[1] = 0;
    pane->grayColor[2] = 0xc0;
    pane->grayColor[3] = alpha;
    pane->perm = permanent;
    pane->used = 1;
    for (i = 0; i < 5; i++) {
        pane->blits[i] = (_tBlit_2 *)NewBlit(0, x1, y1, 0);
        pane->blits[i]->alpha = alpha;
        pane->blits[i]->palette = (void *)pane->blackColor;
        pane->blits[i]->animFunc = animFunc;
        pane->blits[i]->animID = animID;
        pane->blits[i]->animDTA = 0;
    }
    pane->blits[0]->animDTA = (int)pane;
    pane->blits[0]->palette = (void *)pane->grayColor;
    pane->blits[1]->palette = (void *)pane->grayColor;
    pane->blits[3]->palette = (void *)pane->grayColor;
    pane->blits[4]->palette = (void *)pane->grayColor;
    ChangePaneSize(pane, x1, y1, x2, y2);
    for (i = 0; i < 5; i++) {
        ZOID_IgnoreDrawFirstBlit(pane->blits[i]->blidx);
    }
    return pane;
}

/* 0x80030800, size 0x1f0 */
void ChangePaneSize(anon_used_perm_20 *pane, int x1, int y1, int x2, int y2)
{
    int i; /* r30 */
    if (x1 > x2) {
        SWAP(x1, x2);
    }
    if (y1 > y2) {
        SWAP(y1, y2);
    }
    for (i = 0; i < 5; i++) {
        pane->blits[i]->x = x1;
        pane->blits[i]->y = y1;
        pane->blits[i]->width = x2 - x1;
        pane->blits[i]->height = y2 - y1;
    }
    pane->blits[0]->height = 1;
    pane->blits[1]->width = 1;
    pane->blits[1]->y += 1;
    pane->blits[1]->height -= 2;
    pane->blits[2]->x += 1;
    pane->blits[2]->y += 1;
    pane->blits[2]->width -= 2;
    pane->blits[2]->height -= 2;
    pane->blits[3]->x += pane->blits[3]->width - 1;
    pane->blits[3]->width = 1;
    pane->blits[3]->y += 1;
    pane->blits[3]->height -= 2;
    pane->blits[4]->y += pane->blits[4]->height - 1;
    pane->blits[4]->height = 1;
    for (i = 0; i < 5; i++) {
        UpdateBlit((_tBlit *)pane->blits[i]);
    }
}

/* 0x800309f0, size 0xc8 */
void ChangePaneColor(anon_used_perm_20 *pane, unsigned char *paneColor, unsigned char *borderColor)
{
    int i; /* r30 */
    pane->blackColor[0] = paneColor[0];
    pane->blackColor[1] = paneColor[1];
    pane->blackColor[2] = paneColor[2];
    pane->blackColor[3] = paneColor[3];
    pane->grayColor[0] = borderColor[0];
    pane->grayColor[1] = borderColor[1];
    pane->grayColor[2] = borderColor[2];
    pane->grayColor[3] = borderColor[3];
    for (i = 0; i < 5; i++) {
        if (i == 2) {
            pane->blits[i]->palette = (void *)pane->blackColor;
            pane->blits[i]->alpha = paneColor[3];
        } else {
            pane->blits[i]->palette = (void *)pane->grayColor;
            pane->blits[i]->alpha = borderColor[3];
        }
        UpdateBlit((_tBlit *)pane->blits[i]);
    }
}

/* 0x80030ab8, size 0x9c */
void RemovePanes(int permanent)
{
    int i; /* r28 */
    int j; /* r27 */
    for (i = 0; i < 16; i++) {
        if (gPaneBlits[i].used && (permanent || !gPaneBlits[i].perm)) {
            for (j = 0; j < 5; j++) {
                if (gPaneBlits[i].blits[j]) {
                    RemoveBlit((_tBlit *)gPaneBlits[i].blits[j]);
                    gPaneBlits[i].blits[j] = 0;
                }
            }
            gPaneBlits[i].used = 0;
        }
    }
}

/* 0x80030b54, size 0x88 */
void RemovePane(anon_used_perm_20 *pane)
{
    int j; /* r29 */
    if (pane && pane->used) {
        for (j = 0; j < 5; j++) {
            if (pane->blits[j]) {
                RemoveBlit((_tBlit *)pane->blits[j]);
                pane->blits[j] = 0;
            }
        }
        pane->used = 0;
    }
}

