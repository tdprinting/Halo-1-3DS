/* Thin citro3d layer: render target, the basic vertex shader, textures and triangle submission.
 * The rasterizer_* entry points (see upstream source/rasterizer/rasterizer.h) are built on top of this. */
#ifndef CTR_GPU_H
#define CTR_GPU_H

#include <3ds.h>
#include <citro3d.h>
#include <stdbool.h>

#include "ctr_texture.h"

typedef struct {
    float x, y, z;
    float u, v;
    uint8_t color[4]; /* r, g, b, a */
} ctr_vertex;

typedef struct {
    C3D_Tex tex;
    ctr_texture_format format;
    bool valid;
} ctr_gpu_texture;

bool ctr_gpu_init(void);
void ctr_gpu_dispose(void);

/* Frame: begin binds the top-screen target and clears it. */
void ctr_gpu_frame_begin(uint32_t clear_rgba8);
void ctr_gpu_frame_end(void);

/* mvp is column-vector convention (clip = M * p), 4x4 row-major as C3D_Mtx.r[] rows. */
void ctr_gpu_set_mvp(const C3D_Mtx *mvp);

/* Both bitmaps and meshes must be in linear (GPU-visible) memory; use linearAlloc. */
bool ctr_gpu_texture_create(ctr_gpu_texture *out, int halo_format, int width, int height, const void *halo_pixels);
void ctr_gpu_texture_destroy(ctr_gpu_texture *t);
void ctr_gpu_texture_bind(const ctr_gpu_texture *t, int unit);

void ctr_gpu_draw_triangles(const ctr_vertex *vertices, const uint16_t *indices, int index_count);

#endif
