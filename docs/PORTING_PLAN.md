# Halo 1 → New 3DS: porting plan

Upstream: https://github.com/bnunu/halo-1 (CC0). Fetch with `tools/fetch_upstream.sh`.
Per-module dependency numbers are in [PORTING_AUDIT.md](PORTING_AUDIT.md) (regenerate with `tools/audit_platform_deps.py upstream`).

Target: **New 3DS only** (804 MHz ARM11, ~124 MB app RAM, L2 cache). Old 3DS is out of scope.

## What the audit shows
- ~490k LOC in 1091 files. Most gameplay modules (ai, units, objects, physics, hs, items, effects,
  models, structures, camera, cutscene, devices) have **no** direct Xbox/D3D/asm references and are the
  easy part, but still need MSVC-ism cleanup (`__int64`, calling conventions) in places.
- Platform-heavy: `source/rasterizer` (27 D3D8 files), `libs/d3d8` (Xbox D3D8 driver, drop entirely),
  `libs/binkxbox` + `source/bink` (video), `source/sound` (DirectSound), `source/cache`, `source/saved games`,
  `source/input`, `source/main`, `source/shell`, `source/cseries` (asm).
- `libs/libcmt` is the Xbox CRT; replace with newlib. `libs/xapilib` is trivial to drop.

## Stages
1. **Bring-up (this commit).** devkitARM Makefile, platform layer, a boot-to-console `.3dsx`. *Untested on hardware/emulator.*
2. **Compile core.** Add upstream `source/` modules to the build in dependency order: cseries → math → memory →
   tag_files/cache → scenario → game logic. Stub D3D8/sound/bink behind `platform.h`-style headers.
   Replace the 10 inline-asm files with C. Define a 3DS `long`/`tag` size check (ARM `long` = 32 bit, matches).
3. **Data path.** Load an original Xbox/PC map + tag cache from `sdmc:/halo/`. Check endianness/struct packing
   assumptions. Budget memory: maps must be streamed/trimmed to fit ~124 MB with the engine.
4. **Renderer.** New citro3d backend replacing `source/rasterizer/xbox/*`. PICA200 has no pixel shaders:
   the Xbox pixel-shader effects (active camo, plasma, water, chicago/generic transparents) must be
   approximated with the 6 fixed texture combiners. Start with BSP + models + HUD, then transparents.
   Vertex shaders must be rewritten in picasso (`.v.pica`).
5. **Audio/video/input/UI.** DirectSound → ndsp (ADPCM → PCM decode), Bink → drop or cutscene fallback, dual-stick via
   circle pad + C-stick/Circle Pad Pro, bottom-screen UI. Networking: skip initially.
6. **Performance pass.** Lower-resolution rendering (400x240), actor/particle caps, LOD, texture downscaling at load.

## Risks
- The decomp is a *matching* build; some code depends on exact x86 behavior (struct padding, x87 float
  semantics). Expect determinism/physics divergences on ARM VFP.
- GPU: PICA200 state, 8 hardware vertex attributes and no per-pixel lighting limit visual parity.
- Legal: no game data ships here. Users supply their own copy.
