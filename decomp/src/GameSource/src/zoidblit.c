/* zoidblit.c - 2D blit (sprite) instance list for the zoid renderer.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\zoidblit.c; assembly: build/2049LE/asm/GameSource/src/zoidblit.s */
#include "rush/rush_protos.h"

int gNumBlitInsts;
int gMaxBlitInsts;
anon_palette_texture_20 gBlitInstanceList[200];

/* 0x8006274c, size 0x30 */
void ZOID_UpdateBlit(int bl_idx, int texIndex, anon_name_width_30 *texture, anon_name_start_18 *palette, int x, int y, int width, int height)
{
    anon_palette_texture_20 *blit; /* r3 */
    blit = &gBlitInstanceList[bl_idx];
    blit->texture = texture;
    blit->palette = palette;
    blit->texIndex = texIndex;
    blit->x = x;
    blit->y = y;
    blit->width = width;
    blit->height = height;
}

/* 0x8006277c, size 0x7c */
void ZOID_CropBlit(int bl_idx, int top, int bot, int left, int right)
{
    anon_palette_texture_20 *blit; /* r8 */
    blit = &gBlitInstanceList[bl_idx];
    if (top < 0) {
        blit->top = 0;
    } else {
        blit->top = top;
    }
    if (left < 0) {
        blit->left = 0;
    } else {
        blit->left = left;
    }
    if (bot < 0) {
        blit->bot = blit->height - 1;
    } else {
        blit->bot = bot;
    }
    if (right < 0) {
        blit->right = blit->width - 1;
    } else {
        blit->right = right;
    }
}

/* 0x800627f8, size 0x4c */
void ZOID_SetBlitData(int bl_idx, int hide, int alpha, unsigned short z)
{
    anon_palette_texture_20 *blit; /* r3 */
    blit = &gBlitInstanceList[bl_idx];
    if (blit->hide == 2) {
        return;
    }
    blit->hide = hide != 0;
    if (alpha >= 0) {
        blit->alpha = alpha;
    }
    if (z != 0xffff) {
        blit->z = z;
    }
}

/* 0x80062844, size 0x114 */
int ZOID_AddBlit(int texIndex, anon_name_width_30 *texture, anon_name_start_18 *palette, int x, int y, int width, int height)
{
    int newblit; /* r31 */
    anon_palette_texture_20 *blit; /* r7 */
    for (newblit = 0; newblit < gNumBlitInsts; newblit++) {
        if (gBlitInstanceList[newblit].hide == 2) {
            break;
        }
    }
    if (newblit >= 200) {
        return -1;
    }
    if (newblit >= gNumBlitInsts) {
        gNumBlitInsts++;
    }
    if (gMaxBlitInsts < gNumBlitInsts) {
        gMaxBlitInsts = gNumBlitInsts;
    }
    ZOID_UpdateBlit(newblit, texIndex, texture, palette, x, y, width, height);
    blit = &gBlitInstanceList[newblit];
    blit->z = 0;
    blit->alpha = 0xff;
    blit->top = 0;
    blit->left = 0;
    blit->bot = height - 1;
    blit->right = width - 1;
    blit->hide = 0;
    blit->flags = 0;
    return newblit;
}

/* 0x80062958, size 0x1c */
void ZOID_RemoveBlit(int bl_idx)
{
    gBlitInstanceList[bl_idx].hide = 2;
}

/* 0x80062974, size 0x38 */
void ZOID_FlipBlit(int bl_idx, int flip)
{
    anon_palette_texture_20 *blit; /* r3 */
    blit = &gBlitInstanceList[bl_idx];
    if (flip) {
        blit->flags |= 4;
    } else {
        blit->flags &= ~4;
    }
}

/* 0x800629ac, size 0x48 */
void ZOID_TextBlit(int idx, int text)
{
    if (text) {
        gBlitInstanceList[idx].flags |= 0x80;
    } else {
        gBlitInstanceList[idx].flags &= ~0x80;
    }
}

/* 0x800629f4, size 0x20 */
void ZOID_ForceDrawFirstBlit(int idx)
{
    gBlitInstanceList[idx].flags |= 2;
}

/* 0x80062a14, size 0x20 */
void ZOID_IgnoreDrawFirstBlit(int idx)
{
    gBlitInstanceList[idx].flags |= 0x10;
}

