# Halo 1 for 3DS

Work-in-progress port of the [Halo: Combat Evolved decompilation](https://github.com/bnunu/halo-1) to the **New Nintendo 3DS**.

**Status: renderer foundation.** Texture conversion (host-tested), a citro3d GPU layer and a vertex shader exist, and the app draws a test quad. The game itself is not linked in yet.
The 3DS build has not been run on hardware or an emulator.

Host tests (no devkitPro): `tests/run_host_tests.sh`

## Build
Requires [devkitPro](https://devkitpro.org) with devkitARM, libctru and citro3d.

    tools/fetch_upstream.sh      # clones the upstream decomp into ./upstream
    make                         # produces halo3ds.3dsx

## Docs
- [docs/PORTING_PLAN.md](docs/PORTING_PLAN.md): stages and risks
- [docs/RENDERER_DESIGN.md](docs/RENDERER_DESIGN.md): PICA200 renderer design and status
- [docs/PORTING_AUDIT.md](docs/PORTING_AUDIT.md): per-module platform dependencies

No game data is included; you need your own copy of the game.
