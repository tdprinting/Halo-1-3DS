#include "ctr_texture.h"

#include <stdlib.h>
#include <string.h>

int ctr_texture_padded_size(int size)
{
    return size < 8 ? 8 : (size + 7) & ~7;
}

unsigned ctr_texture_morton8(unsigned x, unsigned y)
{
    unsigned r = 0;
    for (unsigned bit = 0; bit < 3; bit++)
        r |= (((x >> bit) & 1u) << (2 * bit)) | (((y >> bit) & 1u) << (2 * bit + 1));
    return r;
}

ctr_texture_format ctr_texture_format_for_halo(int f)
{
    switch (f) {
    case HALO_BITMAP_A8: return CTR_TEX_A8;
    case HALO_BITMAP_Y8: return CTR_TEX_L8;
    case HALO_BITMAP_AY8:
    case HALO_BITMAP_A8Y8: return CTR_TEX_LA8;
    case HALO_BITMAP_R5G6B5: return CTR_TEX_RGB565;
    case HALO_BITMAP_A1R5G5B5:
    case HALO_BITMAP_DXT1: return CTR_TEX_RGBA5551;
    case HALO_BITMAP_A4R4G4B4:
    case HALO_BITMAP_DXT3:
    case HALO_BITMAP_DXT5: return CTR_TEX_RGBA4;
    case HALO_BITMAP_X8R8G8B8:
    case HALO_BITMAP_A8R8G8B8: return CTR_TEX_RGBA8;
    default: return CTR_TEX_INVALID;
    }
}

int ctr_texture_bytes_per_pixel(ctr_texture_format f)
{
    switch (f) {
    case CTR_TEX_RGBA8: return 4;
    case CTR_TEX_RGB8: return 3;
    case CTR_TEX_RGBA5551: case CTR_TEX_RGB565: case CTR_TEX_RGBA4: case CTR_TEX_LA8: return 2;
    case CTR_TEX_L8: case CTR_TEX_A8: return 1;
    default: return 0;
    }
}

size_t ctr_texture_converted_size(int halo_format, int w, int h)
{
    int bpp = ctr_texture_bytes_per_pixel(ctr_texture_format_for_halo(halo_format));
    return (size_t)ctr_texture_padded_size(w) * ctr_texture_padded_size(h) * bpp;
}

/* ---- DXT decode to A8R8G8B8 ---- */

static uint32_t rgb565_to_argb(uint16_t c, uint32_t a)
{
    uint32_t r = (c >> 11) & 31, g = (c >> 5) & 63, b = c & 31;
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

static uint32_t lerp_argb(uint32_t c0, uint32_t c1, unsigned w0, unsigned w1, unsigned div)
{
    uint32_t out = 0xFF000000u;
    for (int s = 0; s <= 16; s += 8) {
        unsigned v = (((c0 >> s) & 255) * w0 + ((c1 >> s) & 255) * w1) / div;
        out |= v << s;
    }
    return out;
}

static void dxt_color_block(const uint8_t *b, uint32_t pal[4], int allow_color_key)
{
    uint16_t c0 = b[0] | (b[1] << 8), c1 = b[2] | (b[3] << 8);
    pal[0] = rgb565_to_argb(c0, 255);
    pal[1] = rgb565_to_argb(c1, 255);
    if (c0 > c1 || !allow_color_key) {
        pal[2] = lerp_argb(pal[0], pal[1], 2, 1, 3);
        pal[3] = lerp_argb(pal[0], pal[1], 1, 2, 3);
    } else {
        pal[2] = lerp_argb(pal[0], pal[1], 1, 1, 2);
        pal[3] = 0; /* transparent black */
    }
}

static void dxt_alpha5_palette(const uint8_t *b, uint8_t a[8])
{
    a[0] = b[0]; a[1] = b[1];
    if (a[0] > a[1]) {
        for (int i = 1; i <= 6; i++) a[1 + i] = (uint8_t)(((7 - i) * a[0] + i * a[1]) / 7);
    } else {
        for (int i = 1; i <= 4; i++) a[1 + i] = (uint8_t)(((5 - i) * a[0] + i * a[1]) / 5);
        a[6] = 0; a[7] = 255;
    }
}

static uint32_t *dxt_decode(int fmt, int w, int h, const uint8_t *src)
{
    uint32_t *out = malloc((size_t)w * h * 4);
    if (!out) return NULL;
    int bw = (w + 3) / 4, bh = (h + 3) / 4;
    int block_bytes = fmt == HALO_BITMAP_DXT1 ? 8 : 16;
    for (int by = 0; by < bh; by++) {
        for (int bx = 0; bx < bw; bx++) {
            const uint8_t *blk = src + ((size_t)by * bw + bx) * block_bytes;
            const uint8_t *cblk = fmt == HALO_BITMAP_DXT1 ? blk : blk + 8;
            uint32_t pal[4];
            dxt_color_block(cblk, pal, fmt == HALO_BITMAP_DXT1);
            uint8_t a5[8];
            if (fmt == HALO_BITMAP_DXT5) dxt_alpha5_palette(blk, a5);
            uint32_t bits = cblk[4] | (cblk[5] << 8) | (cblk[6] << 16) | ((uint32_t)cblk[7] << 24);
            uint64_t abits = 0;
            if (fmt == HALO_BITMAP_DXT5)
                for (int i = 0; i < 6; i++) abits |= (uint64_t)blk[2 + i] << (8 * i);
            for (int py = 0; py < 4; py++) {
                for (int px = 0; px < 4; px++) {
                    int x = bx * 4 + px, y = by * 4 + py, i = py * 4 + px;
                    if (x >= w || y >= h) continue;
                    uint32_t c = pal[(bits >> (2 * i)) & 3];
                    if (fmt == HALO_BITMAP_DXT3) {
                        uint8_t n = (blk[(i >> 1)] >> ((i & 1) * 4)) & 15;
                        c = (c & 0x00FFFFFFu) | ((uint32_t)(n * 17) << 24);
                    } else if (fmt == HALO_BITMAP_DXT5) {
                        c = (c & 0x00FFFFFFu) | ((uint32_t)a5[(abits >> (3 * i)) & 7] << 24);
                    }
                    out[(size_t)y * w + x] = c;
                }
            }
        }
    }
    return out;
}

/* ---- per-pixel packing ---- */

static uint16_t pack_5551(uint32_t argb)
{
    return (uint16_t)((((argb >> 19) & 31) << 11) | (((argb >> 11) & 31) << 6) | (((argb >> 3) & 31) << 1) | (argb >> 31));
}

static uint16_t pack_4444(uint32_t argb)
{
    return (uint16_t)((((argb >> 20) & 15) << 12) | (((argb >> 12) & 15) << 8) | (((argb >> 4) & 15) << 4) | (argb >> 28));
}

static uint32_t pack_rgba8(uint32_t argb)
{
    return (argb << 8) | (argb >> 24); /* 0xAARRGGBB -> 0xRRGGBBAA */
}

/* Expands a 1-/4-/5-bit-per-channel source pixel to A8R8G8B8. */
static uint32_t expand_1555(uint16_t p)
{
    uint32_t r = (p >> 10) & 31, g = (p >> 5) & 31, b = p & 31;
    return ((p & 0x8000) ? 0xFF000000u : 0) | (((r << 3) | (r >> 2)) << 16) | (((g << 3) | (g >> 2)) << 8) | ((b << 3) | (b >> 2));
}

static uint32_t expand_4444(uint16_t p)
{
    return (uint32_t)(((p >> 12) & 15) * 17) << 24 | (uint32_t)(((p >> 8) & 15) * 17) << 16 |
           (uint32_t)(((p >> 4) & 15) * 17) << 8 | (uint32_t)((p & 15) * 17);
}

static void put(uint8_t *dst, int bpp, size_t off_px, uint32_t v)
{
    uint8_t *d = dst + off_px * bpp;
    for (int i = 0; i < bpp; i++) d[i] = (uint8_t)(v >> (8 * i)); /* little-endian */
}

ctr_texture_format ctr_texture_convert(int halo_format, int w, int h, const void *src, void *dst)
{
    ctr_texture_format out_fmt = ctr_texture_format_for_halo(halo_format);
    if (out_fmt == CTR_TEX_INVALID || w <= 0 || h <= 0 || !src || !dst) return CTR_TEX_INVALID;

    const uint8_t *s8 = src;
    uint32_t *decoded = NULL;
    if (halo_format >= HALO_BITMAP_DXT1 && halo_format <= HALO_BITMAP_DXT5) {
        decoded = dxt_decode(halo_format, w, h, s8);
        if (!decoded) return CTR_TEX_INVALID;
    }

    int bpp = ctr_texture_bytes_per_pixel(out_fmt);
    int pw = ctr_texture_padded_size(w), ph = ctr_texture_padded_size(h);
    int tiles_x = pw / 8;

    for (int y = 0; y < ph; y++) {
        for (int x = 0; x < pw; x++) {
            int sx = x % w, sy = y % h; /* repeat to pad small/non-multiple-of-8 sources */
            size_t si = (size_t)sy * w + sx;
            uint32_t v;
            switch (halo_format) {
            case HALO_BITMAP_A8:
            case HALO_BITMAP_Y8: v = s8[si]; break;
            case HALO_BITMAP_AY8: v = (s8[si] << 8) | s8[si]; break;
            case HALO_BITMAP_A8Y8: { uint16_t p = (uint16_t)(s8[si * 2] | (s8[si * 2 + 1] << 8));
                                     /* Xbox A8L8 is (A<<8)|L; PICA LA8 is assumed (L<<8)|A, matching
                                      * the RGBA8 "name order, high to low" rule. UNVERIFIED on hardware. */
                                     v = (uint16_t)((p << 8) | (p >> 8)); break; }
            case HALO_BITMAP_R5G6B5: v = (uint16_t)(s8[si * 2] | (s8[si * 2 + 1] << 8)); break;
            case HALO_BITMAP_A1R5G5B5:
                v = pack_5551(expand_1555((uint16_t)(s8[si * 2] | (s8[si * 2 + 1] << 8)))); break;
            case HALO_BITMAP_A4R4G4B4:
                v = pack_4444(expand_4444((uint16_t)(s8[si * 2] | (s8[si * 2 + 1] << 8)))); break;
            case HALO_BITMAP_X8R8G8B8:
            case HALO_BITMAP_A8R8G8B8: {
                uint32_t argb = s8[si * 4] | (s8[si * 4 + 1] << 8) | (s8[si * 4 + 2] << 16) | ((uint32_t)s8[si * 4 + 3] << 24);
                if (halo_format == HALO_BITMAP_X8R8G8B8) argb |= 0xFF000000u;
                v = pack_rgba8(argb); break; }
            case HALO_BITMAP_DXT1: v = pack_5551(decoded[si]); break;
            default: v = pack_4444(decoded[si]); break; /* DXT3 / DXT5 */
            }
            size_t tile = (size_t)(y / 8) * tiles_x + (x / 8);
            size_t px = tile * 64 + ctr_texture_morton8(x & 7, y & 7);
            put(dst, bpp, px, v);
        }
    }
    free(decoded);
    return out_fmt;
}
