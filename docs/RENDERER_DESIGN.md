# Renderer design (PICA200 backend)

Seam: the engine calls ~100 `rasterizer_*` functions declared in upstream `source/rasterizer/rasterizer.h`,
implemented for Xbox in `source/rasterizer/xbox/*` on top of D3D8. The 3DS backend implements the same
functions in `source/render3ds/` (future `rasterizer_ctr_*.c`) on top of `ctr_gpu`.

## Done (this commit)
| piece | file | status |
|---|---|---|
| Halo bitmap → PICA texture (A8, Y8, AY8, A8Y8, R5G6B5, A1R5G5B5, A4R4G4B4, X8R8G8B8/A8R8G8B8, DXT1/3/5; 8x8 Morton tiling; pad small sizes) | `ctr_texture.c` | **unit-tested on host** (`tests/run_host_tests.sh`) |
| citro3d init, rotated top-screen target, shader, texture create/bind, indexed triangle draw | `ctr_gpu.c` | type-checked against real libctru/citro3d headers; **not run** |
| Basic vertex shader (MVP, UV, color) | `shaders/ctr_basic.v.pica` | assembles with real picasso; **not run** |
| Bring-up scene (checker quad via the real conversion path) | `source/platform/main.c` | **not run** |

## Conventions chosen (unverified on hardware)
- Texture memory is **not** flipped: row 0 stays the top row, so Halo/D3D UVs (v=0 at top) are used unchanged.
- PICA RGBA8/RGBA4/RGBA5551/LA8 are treated as "name order, high→low" little-endian words.
  LA8 is the least certain; flip the swap in `ctr_texture.c` if grey textures show alpha artifacts.
- DXT is decoded at load (PICA has only ETC1): DXT1 → RGBA5551, DXT3/5 → RGBA4 to save RAM (lossy).
- P8_BUMP (palettized bump maps) is not supported yet; needs a decision (RGB8 normal map or drop).
- Max texture 1024; Halo's larger bitmaps need downscaling at load.

## Pass mapping (next work), Xbox profile pass → PICA plan
| Xbox pass | plan |
|---|---|
| environment lightmaps + diffuse textures | one pass: texenv0 = base × lightmap (needs 2 texture units + 2nd UV set) |
| models | skinning on the vertex shader (≤ ~44 nodes in Halo, PICA has 96 vec4 uniforms, so cap bones/draw) |
| decals, detail objects | alpha-blended / alpha-tested quads, cap counts |
| transparents (chicago/generic) | ≤6 texenv stages; shaders needing more get simplified |
| specular, reflections, mirrors | skip initially |
| active camouflage, plasma, water refraction, screen effects | approximate or skip (no pixel shaders, no render-to-texture sampling of the framebuffer without a copy) |
| fog | per-vertex fog or `GPU_FOG` (gas) LUT |
| HUD / text | 2D ortho quads via the same path (first thing to wire to `rasterizer_hud_*` and `rasterizer_draw_string`) |

## Order of work
1. Implement `rasterizer_initialize/frame_begin/frame_end/present/dispose` over `ctr_gpu` (needs core code compiling).
2. HUD + text + `rasterizer_dynamic_screen_geometry_draw` (2D) so menus are visible.
3. Environment (BSP) lightmap pass, then models, then transparents.
4. Hardware bitmap upload hooks (`rasterizer_xbox_hardware_bitmaps.c` equivalent) calling `ctr_gpu_texture_create`.
