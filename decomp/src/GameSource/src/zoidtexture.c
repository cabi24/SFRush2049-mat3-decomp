/* zoidtexture.c - per-object colour and texture assignment for the "zoid" renderer. */
#include "rush/rush_protos.h"

/* the original copies colours as one 32-bit word through casts (likely a macro) */
#define COPY_RGBA(dst, src) (*(unsigned int *)&(dst) = *(unsigned int *)(src))

/* 0x80062dfc */
void ZOID_SetColor(signed short objnum, anon_c_rgba_4 *col)
{
    COPY_RGBA(ObjInstanceList[objnum].color, col);
}

/* 0x80062e1c */
anon_c_rgba_4 *ZOID_GetColor(signed short objnum)
{
    return &ObjInstanceList[objnum].color;
}

/* 0x80062e38 */
void ZOID_SetEnvColor(signed short objnum, anon_c_rgba_4 *col)
{
    COPY_RGBA(ObjInstanceList[objnum].envcolor, col);
}

/* 0x80062e58 */
void ZOID_SetTexture(signed short objnum, anon_name_width_30 *tex, int lod)
{
    anon_flags_culling_60 *obj = &ObjInstanceList[objnum];

    if (lod < 0) {
        obj->texture[0] = tex;
        obj->texture[1] = tex;
        obj->texture[2] = tex;
        obj->texture[3] = tex;
    } else {
        obj->texture[lod] = tex;
    }
}

/* 0x80062e98 */
int ZOID_ChangeTexture(unsigned short objidx, signed short lod, unsigned short texidx)
{
    anon_name_radius_48 *robj = &gRomObjDefPtrs[objidx >> 10].romObjDefs[objidx & 0x3ff];
    int i;

    if (lod >= 0) {
        if (lod >= robj->lodCount)
            return 0;
        robj->lod[lod].texidx = texidx;
        robj->lod[lod].flags |= 0x8000;
    } else {
        for (i = 0; i < robj->lodCount; i++) {
            robj->lod[i].texidx = texidx;
            robj->lod[lod].flags |= 0x8000; /* sic: the original indexes with lod, not i */
        }
    }
    return 1;
}
