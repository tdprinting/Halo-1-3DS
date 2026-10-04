/* Halo bitmap -> PICA200 texture conversion. Pure C, no libctru dependency, host-testable. */
#ifndef CTR_TEXTURE_H
#define CTR_TEXTURE_H

#include <stddef.h>
#include <stdint.h>

/* Values match libctru's GPU_TEXCOLOR (checked by a static assert in ctr_gpu.c). */
typedef enum {
    CTR_TEX_RGBA8 = 0,
    CTR_TEX_RGB8 = 1,
    CTR_TEX_RGBA5551 = 2,
    CTR_TEX_RGB565 = 3,
    CTR_TEX_RGBA4 = 4,
    CTR_TEX_LA8 = 5,
    CTR_TEX_L8 = 7,
    CTR_TEX_A8 = 8,
    CTR_TEX_INVALID = -1
} ctr_texture_format;

/* Halo bitmap_format values (tag definition). */
enum {
    HALO_BITMAP_A8 = 0,
    HALO_BITMAP_Y8 = 1,
    HALO_BITMAP_AY8 = 2,
    HALO_BITMAP_A8Y8 = 3,
    HALO_BITMAP_R5G6B5 = 6,
    HALO_BITMAP_A1R5G5B5 = 8,
    HALO_BITMAP_A4R4G4B4 = 9,
    HALO_BITMAP_X8R8G8B8 = 10,
    HALO_BITMAP_A8R8G8B8 = 11,
    HALO_BITMAP_DXT1 = 14,
    HALO_BITMAP_DXT3 = 15,
    HALO_BITMAP_DXT5 = 16,
    HALO_BITMAP_P8_BUMP = 17
};

/* PICA texture dimensions are rounded up to a multiple of 8 (min 8); smaller sources repeat. */
int ctr_texture_padded_size(int size);

/* GPU format used for a given Halo format. DXT1 maps to RGBA5551, DXT3/5 to RGBA4 (memory). */
ctr_texture_format ctr_texture_format_for_halo(int halo_format);
int ctr_texture_bytes_per_pixel(ctr_texture_format format);
size_t ctr_texture_converted_size(int halo_format, int width, int height);

/* Converts one mip level. src is tightly packed in Halo's layout (DXT: 4x4 blocks).
 * dst must hold ctr_texture_converted_size() bytes and is written in 8x8 Morton tiles.
 * Returns the GPU format, or CTR_TEX_INVALID for unsupported input / allocation failure. */
ctr_texture_format ctr_texture_convert(int halo_format, int width, int height,
                                       const void *src, void *dst);

/* Offset (in pixels, within one 8x8 tile) of tile-local pixel (x,y), 0..7. */
unsigned ctr_texture_morton8(unsigned x, unsigned y);

#endif
