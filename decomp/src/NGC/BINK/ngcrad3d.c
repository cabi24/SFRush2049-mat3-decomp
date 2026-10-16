/* ngcrad3d.c - RAD Game Tools 3D surface layer (Bink video frames as GX textures). */
#include "rush/rush_protos.h"

static int Built_tables;
static unsigned int Pixel_info[5];
static _GXTexFmt D3D_surface_type[5];


static void Setup_surface_array(void);

#define RAD_ALPHA 0x80000000

/* 0x800b28c8 */
static void Setup_surface_array(void)
{
    if (Built_tables)
        return;
    D3D_surface_type[0] = 6;  /* GX_TF_RGBA8 */
    D3D_surface_type[1] = 6;
    D3D_surface_type[2] = 4;  /* GX_TF_RGB565 */
    D3D_surface_type[3] = 5;  /* GX_TF_RGB5A3 */
    D3D_surface_type[4] = -1;
    Pixel_info[0] = 4;
    Pixel_info[1] = RAD_ALPHA | 4;
    Pixel_info[2] = 2;
    Pixel_info[3] = RAD_ALPHA | 2;
    Pixel_info[4] = 2;
    Built_tables = 1;
}

/* 0x800b2938 (the first parameter is unnamed in the original) */
RAD3DIMAGE *Open_RAD_3D_image(void *rad_3d, unsigned int width, unsigned int height, unsigned int rad3d_surface_format)
{
    RAD3DIMAGE *rad_image;
    unsigned int pixel_size;

    Setup_surface_array();
    pixel_size = Pixel_info[rad3d_surface_format] & 0xff;
    rad_image = OSAllocFromHeap(__OSCurrHeap, sizeof(RAD3DIMAGE));
    if (!rad_image)
        return NULL;
    rad_image->width = width;
    rad_image->height = height;
    rad_image->alpha_pixels = Pixel_info[rad3d_surface_format] >> 31;
    rad_image->pixel_size = pixel_size;
    rad_image->rad_surface_type = rad3d_surface_format;
    rad_image->texture_size = GXGetTexBufferSize((signed short)width, (signed short)height, D3D_surface_type[rad3d_surface_format], 0, 0);
    rad_image->texture = OSAllocFromHeap(__OSCurrHeap, rad_image->texture_size);
    GXInitTexObj(&rad_image->texObj, rad_image->texture, width, height, D3D_surface_type[rad3d_surface_format], 0, 0, 0);
    GXInitTexObjLOD(&rad_image->texObj, 1, 0, 0.0f, 0.0f, 0.0f, 0, 0, 0);
    return rad_image;
}

/* 0x800b2a3c */
void Close_RAD_3D_image(RAD3DIMAGE *rad_image)
{
    if (rad_image) {
        if (rad_image->texture) {
            OSFreeToHeap(__OSCurrHeap, rad_image->texture);
            rad_image->texture = NULL;
        }
        OSFreeToHeap(__OSCurrHeap, rad_image);
    }
}

/* 0x800b2a90 */
int Lock_RAD_3D_image(RAD3DIMAGE *rad_image, void *out_pixel_buffer, unsigned int *out_buffer_pitch, unsigned int *out_surface_type)
{
    if (!rad_image)
        return 0;
    if (out_pixel_buffer)
        *(void **)out_pixel_buffer = rad_image->texture;
    if (out_buffer_pitch)
        *out_buffer_pitch = rad_image->width * rad_image->pixel_size;
    if (out_surface_type)
        *out_surface_type = rad_image->rad_surface_type;
    return 1;
}

/* 0x800b2ae0 */
void Unlock_RAD_3D_image(RAD3DIMAGE *rad_image)
{
    if (rad_image)
        DCStoreRange(rad_image->texture, rad_image->texture_size);
}
