/* mb_wad.c - "wad" resource files: a byte-swapped table of contents of (tag, address, length)
   entries, relocated in place to a load address. */
#include "rush/rush_protos.h"

/* 0x800288ec */
void MBSetupWad(anon_base_TOCentries_c *wad, void *base)
{
    int i;
    int fixup;
    anon_tag_address_c *e;

    if (!wad)
        return;
    wad->base = base;
    _Enable_swap();
    _swap_x32(wad->base);
    wad->TOC = (anon_tag_address_c *)wad->base[0];
    _swap_x32(&wad->base[1]);
    wad->TOCentries = wad->base[1];
    wad->TOC = (anon_tag_address_c *)((unsigned int)wad->TOC + (unsigned int)base);
    wad->base[0] = (unsigned int)wad->TOC;
    for (i = 0; i < wad->TOCentries; i++) {
        e = &wad->TOC[i];
        _swap_x32(&e->tag);
        _swap_x32(&e->address);
        _swap_x32(&e->length);
        e->address += (unsigned int)base;
    }
}

/* 0x800289b4 */
void MBOffsetWad(anon_base_TOCentries_c *wad, void *base, int offset)
{
    int i;
    anon_tag_address_c *e;

    if (!wad)
        return;
    wad->base = (unsigned int *)((char *)base + offset);
    _Enable_swap();
    _swap_x32(wad->base);
    wad->base[0] += offset;
    wad->TOC = (anon_tag_address_c *)wad->base[0];
    _swap_x32(&wad->base[1]);
    wad->TOCentries = wad->base[1];
    for (i = 0; i < wad->TOCentries; i++) {
        e = &wad->TOC[i];
        _swap_x32(&e->tag);
        _swap_x32(&e->address);
        _swap_x32(&e->length);
        e->address += offset;
    }
}

/* 0x80028a78 */
anon_tag_address_c *MBFindInWad(anon_base_TOCentries_c *wad, unsigned int tag)
{
    int i;

    if (!wad)
        return NULL;
    for (i = 0; i < wad->TOCentries; i++) {
        if (wad->TOC[i].tag == tag)
            return &wad->TOC[i];
    }
    return NULL;
}

/* 0x80028ad0 */
void *MBGetFromWad(anon_base_TOCentries_c *wad, unsigned int tag, int *length)
{
    anon_tag_address_c *e = NULL;

    if (wad)
        e = MBFindInWad(wad, tag);

    if (!e) {
        if (length)
            *length = 0;
        return NULL;
    }
    if (length)
        *length = e->length;
    return (void *)e->address;
}
