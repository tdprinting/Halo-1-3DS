#include "ctr_gpu.h"

#include <stdlib.h>
#include <string.h>

#include "ctr_basic_shbin.h"

_Static_assert((int)CTR_TEX_RGBA8 == GPU_RGBA8 && (int)CTR_TEX_RGB565 == GPU_RGB565 &&
               (int)CTR_TEX_RGBA4 == GPU_RGBA4 && (int)CTR_TEX_RGBA5551 == GPU_RGBA5551 &&
               (int)CTR_TEX_LA8 == GPU_LA8 && (int)CTR_TEX_L8 == GPU_L8 && (int)CTR_TEX_A8 == GPU_A8,
               "ctr_texture_format must match GPU_TEXCOLOR");

enum { SCREEN_W = 400, SCREEN_H = 240 };

static struct {
    bool initialized;
    C3D_RenderTarget *target;
    DVLB_s *dvlb;
    shaderProgram_s program;
    int uloc_mvp;
    C3D_AttrInfo attr;
} g;

bool ctr_gpu_init(void)
{
    if (g.initialized) return true;
    if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) return false;

    /* The 3DS framebuffer is rotated 90 degrees: the target is created as 240x400 (w x h). */
    g.target = C3D_RenderTargetCreate(SCREEN_H, SCREEN_W, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    if (!g.target) { C3D_Fini(); return false; }
    C3D_RenderTargetSetOutput(g.target, GFX_TOP, GFX_LEFT,
                              GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) |
                              GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |
                              GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));

    g.dvlb = DVLB_ParseFile((u32 *)ctr_basic_shbin, ctr_basic_shbin_size);
    if (!g.dvlb) { C3D_Fini(); return false; }
    shaderProgramInit(&g.program);
    shaderProgramSetVsh(&g.program, &g.dvlb->DVLE[0]);
    C3D_BindProgram(&g.program);
    g.uloc_mvp = shaderInstanceGetUniformLocation(g.program.vertexShader, "mvp");

    AttrInfo_Init(&g.attr);
    AttrInfo_AddLoader(&g.attr, 0, GPU_FLOAT, 3);         /* position */
    AttrInfo_AddLoader(&g.attr, 1, GPU_FLOAT, 2);         /* texcoord */
    AttrInfo_AddLoader(&g.attr, 2, GPU_UNSIGNED_BYTE, 4); /* color */
    C3D_SetAttrInfo(&g.attr);

    /* Stage 0: modulate texture by vertex color. Real Halo shader passes will replace this. */
    C3D_TexEnv *env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);

    g.initialized = true;
    return true;
}

void ctr_gpu_dispose(void)
{
    if (!g.initialized) return;
    shaderProgramFree(&g.program);
    DVLB_Free(g.dvlb);
    C3D_Fini();
    g.initialized = false;
}

void ctr_gpu_frame_begin(uint32_t clear_rgba8)
{
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C3D_RenderTargetClear(g.target, C3D_CLEAR_ALL, clear_rgba8, 0);
    C3D_FrameDrawOn(g.target);
}

void ctr_gpu_frame_end(void)
{
    C3D_FrameEnd(0);
}

void ctr_gpu_set_mvp(const C3D_Mtx *mvp)
{
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, g.uloc_mvp, mvp);
}

bool ctr_gpu_texture_create(ctr_gpu_texture *out, int halo_format, int width, int height, const void *halo_pixels)
{
    memset(out, 0, sizeof(*out));
    int pw = ctr_texture_padded_size(width), ph = ctr_texture_padded_size(height);
    ctr_texture_format fmt = ctr_texture_format_for_halo(halo_format);
    if (fmt == CTR_TEX_INVALID || pw > 1024 || ph > 1024) return false;

    void *tmp = linearAlloc(ctr_texture_converted_size(halo_format, width, height));
    if (!tmp) return false;
    if (ctr_texture_convert(halo_format, width, height, halo_pixels, tmp) == CTR_TEX_INVALID ||
        !C3D_TexInit(&out->tex, (u16)pw, (u16)ph, (GPU_TEXCOLOR)fmt)) {
        linearFree(tmp);
        return false;
    }
    C3D_TexLoadImage(&out->tex, tmp, GPU_TEXFACE_2D, 0);
    C3D_TexSetFilter(&out->tex, GPU_LINEAR, GPU_LINEAR);
    C3D_TexSetWrap(&out->tex, GPU_REPEAT, GPU_REPEAT);
    linearFree(tmp);
    out->format = fmt;
    out->valid = true;
    return true;
}

void ctr_gpu_texture_destroy(ctr_gpu_texture *t)
{
    if (t->valid) C3D_TexDelete(&t->tex);
    t->valid = false;
}

void ctr_gpu_texture_bind(const ctr_gpu_texture *t, int unit)
{
    C3D_TexBind(unit, (C3D_Tex *)&t->tex);
}

void ctr_gpu_draw_triangles(const ctr_vertex *vertices, const uint16_t *indices, int index_count)
{
    C3D_BufInfo *buf = C3D_GetBufInfo();
    BufInfo_Init(buf);
    BufInfo_Add(buf, vertices, sizeof(ctr_vertex), 3, 0x210);
    C3D_DrawElements(GPU_TRIANGLES, index_count, C3D_UNSIGNED_SHORT, indices);
}
