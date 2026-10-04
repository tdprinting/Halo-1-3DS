#include "platform.h"

#include <3ds.h>
#include <stdio.h>

#include "ctr_gpu.h"

/* Bring-up scene: a checkerboard A8R8G8B8 "Halo bitmap" run through the real texture
 * conversion path and drawn as a quad. Proves target + shader + texture + draw. */
enum { CHECK_SIZE = 64 };

int main(void)
{
    if (!platform_init()) return 1;
    printf("Halo 1 (3DS port, bring-up build)\nSTART to exit\n");

    bool gpu_ok = ctr_gpu_init();
    ctr_gpu_texture tex = {0};
    ctr_vertex *verts = NULL;
    uint16_t *idx = NULL;
    C3D_Mtx proj;

    if (!gpu_ok) {
        printf("ctr_gpu_init failed\n");
    } else {
        static uint32_t pixels[CHECK_SIZE * CHECK_SIZE];
        for (int y = 0; y < CHECK_SIZE; y++)
            for (int x = 0; x < CHECK_SIZE; x++)
                pixels[y * CHECK_SIZE + x] = ((x / 8 + y / 8) & 1) ? 0xFFFFFFFFu : 0xFF2060C0u;
        if (!ctr_gpu_texture_create(&tex, HALO_BITMAP_A8R8G8B8, CHECK_SIZE, CHECK_SIZE, pixels))
            printf("texture create failed\n");

        verts = linearAlloc(4 * sizeof(ctr_vertex));
        idx = linearAlloc(6 * sizeof(uint16_t));
        const ctr_vertex quad[4] = {
            {100, 40, 0.5f, 0, 0, {255, 255, 255, 255}},
            {300, 40, 0.5f, 1, 0, {255, 255, 255, 255}},
            {300, 200, 0.5f, 1, 1, {255, 255, 255, 255}},
            {100, 200, 0.5f, 0, 1, {255, 255, 255, 255}},
        };
        const uint16_t quad_idx[6] = {0, 1, 2, 0, 2, 3};
        for (int i = 0; i < 4; i++) verts[i] = quad[i];
        for (int i = 0; i < 6; i++) idx[i] = quad_idx[i];
        Mtx_OrthoTilt(&proj, 0.0f, 400.0f, 240.0f, 0.0f, 0.0f, 1.0f, true); /* y down, like D3D screen space */
    }

    while (platform_main_loop_running()) {
        platform_input input;
        platform_input_poll(&input);
        if (input.buttons & KEY_START) break;

        if (gpu_ok && tex.valid) {
            ctr_gpu_frame_begin(0x202020FFu);
            ctr_gpu_set_mvp(&proj);
            ctr_gpu_texture_bind(&tex, 0);
            ctr_gpu_draw_triangles(verts, idx, 6);
            ctr_gpu_frame_end();
        } else {
            gspWaitForVBlank();
        }
    }

    if (gpu_ok) {
        ctr_gpu_texture_destroy(&tex);
        linearFree(verts);
        linearFree(idx);
        ctr_gpu_dispose();
    }
    platform_dispose();
    return 0;
}
