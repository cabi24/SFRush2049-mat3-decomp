/* texture.c - texture bookkeeping (PS2 leftovers). Matches. */
#include "rush/rush_protos.h"

extern char g_MyPacketName[];

unsigned int gTextureRamUsed;

/* 0x800507ec */
void MirrorTextures(void)
{
    int i;

    for (i = 0; i < 8192; i++) {
    }
}

/* 0x800507fc */
void AddTexture(tTexture *tex)
{
    LinkWithPS2Texture(g_MyPacketName, tex);
}

/* 0x80050828 */
void DeleteTexture(void)
{
}
