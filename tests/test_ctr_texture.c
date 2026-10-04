#include "../source/render3ds/ctr_texture.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

static void test_morton(void)
{
    int seen[64] = {0};
    for (unsigned y = 0; y < 8; y++)
        for (unsigned x = 0; x < 8; x++) seen[ctr_texture_morton8(x, y)]++;
    for (int i = 0; i < 64; i++) CHECK(seen[i] == 1);
    CHECK(ctr_texture_morton8(1, 0) == 1);
    CHECK(ctr_texture_morton8(0, 1) == 2);
    CHECK(ctr_texture_morton8(1, 1) == 3);
    CHECK(ctr_texture_morton8(2, 0) == 4);
    CHECK(ctr_texture_morton8(7, 7) == 63);
}

static void test_a8_tiling(void)
{
    uint8_t src[16 * 16], dst[16 * 16];
    for (int i = 0; i < 256; i++) src[i] = (uint8_t)i;
    CHECK(ctr_texture_convert(HALO_BITMAP_A8, 16, 16, src, dst) == CTR_TEX_A8);
    /* pixel (9,3) lives in tile (1,0), local (1,3) */
    CHECK(dst[(0 * 2 + 1) * 64 + ctr_texture_morton8(1, 3)] == src[3 * 16 + 9]);
    /* pixel (2,10) lives in tile (0,1) */
    CHECK(dst[(1 * 2 + 0) * 64 + ctr_texture_morton8(2, 2)] == src[10 * 16 + 2]);
}

static void test_argb8(void)
{
    uint32_t src[64], dst[64];
    for (int i = 0; i < 64; i++) src[i] = 0x80112233u;
    CHECK(ctr_texture_convert(HALO_BITMAP_A8R8G8B8, 8, 8, src, dst) == CTR_TEX_RGBA8);
    CHECK(dst[0] == 0x11223380u);
    CHECK(ctr_texture_convert(HALO_BITMAP_X8R8G8B8, 8, 8, src, dst) == CTR_TEX_RGBA8);
    CHECK(dst[0] == 0x112233FFu);
}

static void test_16bit(void)
{
    uint16_t src[64], dst[64];
    for (int i = 0; i < 64; i++) src[i] = 0xF800; /* red */
    CHECK(ctr_texture_convert(HALO_BITMAP_R5G6B5, 8, 8, src, dst) == CTR_TEX_RGB565);
    CHECK(dst[0] == 0xF800);
    for (int i = 0; i < 64; i++) src[i] = 0xFC00; /* a1r5g5b5: opaque red */
    CHECK(ctr_texture_convert(HALO_BITMAP_A1R5G5B5, 8, 8, src, dst) == CTR_TEX_RGBA5551);
    CHECK(dst[0] == 0xF801);
    for (int i = 0; i < 64; i++) src[i] = 0x8F00; /* a4r4g4b4: a=8 r=15 */
    CHECK(ctr_texture_convert(HALO_BITMAP_A4R4G4B4, 8, 8, src, dst) == CTR_TEX_RGBA4);
    CHECK(dst[0] == 0xF008);
}

static void test_a8y8(void)
{
    uint16_t src[64], dst[64];
    for (int i = 0; i < 64; i++) src[i] = 0xAA55; /* A=AA L=55 */
    CHECK(ctr_texture_convert(HALO_BITMAP_A8Y8, 8, 8, src, dst) == CTR_TEX_LA8);
    CHECK(dst[0] == 0x55AA);
}

static void test_small_padding(void)
{
    uint8_t src[4 * 4], dst[8 * 8];
    for (int i = 0; i < 16; i++) src[i] = (uint8_t)(i + 1);
    CHECK(ctr_texture_converted_size(HALO_BITMAP_A8, 4, 4) == 64);
    CHECK(ctr_texture_convert(HALO_BITMAP_A8, 4, 4, src, dst) == CTR_TEX_A8);
    CHECK(dst[ctr_texture_morton8(5, 6)] == src[(6 % 4) * 4 + (5 % 4)]);
}

static void test_dxt1(void)
{
    /* opaque: c0 red > c1 blue, all indices 0 -> red */
    uint8_t blk[8] = {0x00, 0xF8, 0x1F, 0x00, 0, 0, 0, 0};
    uint16_t dst[64];
    CHECK(ctr_texture_convert(HALO_BITMAP_DXT1, 4, 4, blk, dst) == CTR_TEX_RGBA5551);
    CHECK(dst[0] == 0xF801);
    /* color-key: c0 <= c1, index 3 = transparent. pixel(0,0) idx 3 */
    uint8_t key[8] = {0x1F, 0x00, 0x00, 0xF8, 0x03, 0, 0, 0};
    CHECK(ctr_texture_convert(HALO_BITMAP_DXT1, 4, 4, key, dst) == CTR_TEX_RGBA5551);
    CHECK((dst[ctr_texture_morton8(0, 0)] & 1) == 0);
    CHECK((dst[ctr_texture_morton8(1, 0)] & 1) == 1);
}

static void test_dxt35(void)
{
    uint8_t b3[16] = {0xF0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0xF8, 0x00, 0xF8, 0, 0, 0, 0};
    uint16_t dst[64];
    CHECK(ctr_texture_convert(HALO_BITMAP_DXT3, 4, 4, b3, dst) == CTR_TEX_RGBA4);
    CHECK((dst[ctr_texture_morton8(0, 0)] & 15) == 0);  /* nibble 0 */
    CHECK((dst[ctr_texture_morton8(1, 0)] & 15) == 15); /* nibble 1 */
    /* dxt5: a0=255 a1=0, pixel0 idx0 (255), pixel1 idx1 (0) */
    uint8_t b5[16] = {255, 0, 0x08, 0, 0, 0, 0, 0, 0x00, 0xF8, 0x00, 0xF8, 0, 0, 0, 0};
    CHECK(ctr_texture_convert(HALO_BITMAP_DXT5, 4, 4, b5, dst) == CTR_TEX_RGBA4);
    CHECK((dst[ctr_texture_morton8(0, 0)] & 15) == 15);
    CHECK((dst[ctr_texture_morton8(1, 0)] & 15) == 0);
}

static void test_invalid(void)
{
    uint8_t buf[64] = {0};
    CHECK(ctr_texture_convert(HALO_BITMAP_P8_BUMP, 8, 8, buf, buf) == CTR_TEX_INVALID);
    CHECK(ctr_texture_convert(99, 8, 8, buf, buf) == CTR_TEX_INVALID);
}

int main(void)
{
    test_morton(); test_a8_tiling(); test_argb8(); test_16bit(); test_a8y8();
    test_small_padding(); test_dxt1(); test_dxt35(); test_invalid();
    printf(failures ? "%d failure(s)\n" : "all tests passed\n", failures);
    return failures != 0;
}
