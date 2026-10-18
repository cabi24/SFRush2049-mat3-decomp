/* blit.c - blit (sprite) objects layered over the zoid blit list, and multi-part blits.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\blit.c; assembly: build/2049LE/asm/GameSource/src/blit.s */
#include "rush/rush_protos.h"

static unsigned char gNoTexBlitDefaultColor[4] = { 0, 0, 0, 255 };
int NumBlits;
_tBlit *BlitPtrs[200];
_tBlit Blits[200];

/* 0x800a462c, size 0x8c */
void InitBlits(void)
{
    int i; /* r0 */

    HideControllerWarning();
    for (i = 0; i < 200; i++) {
        BlitPtrs[i] = &Blits[i];
    }
    NumBlits = 0;
    inactiveBlit = 0;
}

/* 0x800a46b8, size 0xa8 */
void RemoveBlit(_tBlit *dbp)
{
    int i; /* r30 */

    while (dbp) {
        for (i = 0; i < NumBlits; i++) {
            if (dbp == BlitPtrs[i]) {
                break;
            }
        }
        ZOID_RemoveBlit(dbp->blidx);
        NumBlits--;
        BlitPtrs[i] = BlitPtrs[NumBlits];
        BlitPtrs[NumBlits] = dbp;
        dbp = dbp->child;
    }
}

/* 0x800a4760, size 0x98 */
void DrawBlits(void)
{
    int i; /* r30 */
    _tBlit *blit; /* r29 */

    for (i = 0; i < NumBlits; i++) {
        blit = BlitPtrs[i];
        if (blit->animFunc && blit->name != (const char *)-1) {
            if (!blit->animFunc(blit)) {
                RemoveBlit(blit);
                i--;
            }
        }
    }
}

/* 0x800a47f8, size 0xdc */
void InitBlit(_tBlit *blit)
{
    tTexture *ti; /* r0 */

    if (!blit->name) {
        blit->texIndex = 0;
        blit->texture = 0;
        blit->width = 0;
        blit->height = 0;
        blit->palette = (void *)gNoTexBlitDefaultColor;
    } else if (blit->name == (const char *)-1) {
        blit->texIndex = 0;
        blit->texture = 0;
        blit->width = 0;
        blit->height = 0;
        blit->palette = 0;
    } else {
        ti = MBOX_FindTexture_WORK(blit->name, &blit->texIndex, 0, gNumModels - 1, 1);
        blit->texture = ti;
        blit->width = ti->width;
        blit->height = ti->height;
        blit->palette = 0;
    }
    blit->z = 0;
    blit->alpha = 0xff;
    blit->top = -1;
    blit->bot = -1;
    blit->left = -1;
    blit->right = -1;
    blit->color = -1;
}

/* 0x800a48d4, size 0xa0 */
void UpdateBlit(_tBlit *blit)
{
    ZOID_UpdateBlit(blit->blidx, blit->texIndex, blit->texture, blit->palette, blit->x, blit->y, blit->width,
                    blit->height);
    ZOID_CropBlit(blit->blidx, blit->top, blit->bot, blit->left, blit->right);
    ZOID_SetBlitData(blit->blidx, blit->hide, blit->alpha, blit->z);
    ZOID_FlipBlit(blit->blidx, blit->flip);
    ZOID_TextBlit(blit->blidx, blit->name == (const char *)-1);
}

/* 0x800a4974, size 0x11c */
_tBlit *NewBlit(const char *name, int x, int y, unsigned int cont)
{
    _tBlit *blit; /* r31 */

    if (NumBlits >= 200) {
        return 0;
    }
    blit = BlitPtrs[NumBlits];
    NumBlits++;
    if (cont) {
        x /= 2;
        y *= 0.5f;
    }
    blit->name = name;
    blit->x = x;
    blit->y = y;
    blit->hide = 0;
    blit->flip = 0;
    blit->init = 1;
    blit->child = 0;
    blit->animFunc = 0;
    blit->animID = -1;
    blit->animDTA = -1;
    blit->data = 0;
    InitBlit(blit);
    blit->blidx = ZOID_AddBlit(blit->texIndex, 0, 0, x, y, -1, -1);
    UpdateBlit(blit);
    return blit;
}

/* 0x800a4a90, size 0x68 */
void RenameBlit(_tBlit *blit, const char *name, int preserve)
{
    tTexture *ti; /* r0 */

    blit->name = name;
    if (preserve) {
        ti = MBOX_FindTexture_WORK(blit->name, &blit->texIndex, 0, gNumModels - 1, 1);
        blit->texture = ti;
    } else {
        InitBlit(blit);
    }
    UpdateBlit(blit);
}

/* 0x800a4af8, size 0x124 */
void SelectBlit(_tBlit *blt, int index, int hSize, int vSize)
{
    int col; /* r4 */
    int secPerRow; /* r5 */

    if (!blt) {
        return;
    }
    if (!blt->texture || !blt->texture->width) {
        RenameBlit(blt, blt->name, 1);
        if (!blt->texture) {
            return;
        }
    }
    secPerRow = blt->texture->width / hSize;
    if (!secPerRow) {
        secPerRow = 1;
    }
    col = index / secPerRow;
    blt->top = col * vSize;
    col = index % secPerRow;
    blt->bot = blt->top + vSize - 1;
    if (blt->flip) {
        col = secPerRow - col - 1;
        blt->left = col * hSize + blt->texture->width % hSize;
        blt->right = blt->left + hSize - 1;
    } else {
        blt->left = col * hSize;
        blt->right = blt->left + hSize - 1;
    }
    UpdateBlit(blt);
}

/* 0x800a4c1c, size 0x164 */
_tBlit *NewMultiBlit(signed short ulx, signed short uly, const tBlitDef *mblit, signed short nblits)
{
    _tBlit *rootblit; /* r27 */
    _tBlit *lastblit; /* r26 */
    _tBlit *curblit; /* r25 */
    int i; /* r24 */

    if (nblits <= 0) {
        return 0;
    }
    rootblit = 0;
    lastblit = 0;
    for (i = 0; i < nblits; i++) {
        curblit = NewBlit(mblit[i].texname, ulx + mblit[i].dulx, uly + mblit[i].duly, mblit[i].animid & 0x80000000);
        curblit->alpha = mblit[i].alpha;
        curblit->z = mblit[i].zdepth;
        curblit->left = mblit[i].left;
        curblit->top = mblit[i].top;
        curblit->right = mblit[i].right;
        curblit->bot = mblit[i].bot;
        if (mblit[i].width >= 0) {
            curblit->width = mblit[i].width;
        }
        if (mblit[i].height >= 0) {
            curblit->height = mblit[i].height;
        }
        UpdateBlit(curblit);
        if (mblit[i].animfunc) {
            curblit->animID = mblit[i].animid & 0x7fffffff;
            if (mblit[i].texname == (const char *)-1) {
                curblit->texture = (void *)mblit[i].animfunc;
                curblit->palette = (void *)curblit;
                UpdateBlit(curblit);
            } else {
                curblit->animFunc = mblit[i].animfunc;
                if (!curblit->animFunc(curblit)) {
                    RemoveBlit(curblit);
                    return rootblit;
                }
            }
        }
        if (i == 0) {
            rootblit = curblit;
        } else {
            lastblit->child = curblit;
        }
        lastblit = curblit;
    }
    return rootblit;
}

