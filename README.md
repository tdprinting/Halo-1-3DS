# Halo 1 for 3DS

Work-in-progress port of the [Halo: Combat Evolved decompilation](https://github.com/bnunu/halo-1) to the **New Nintendo 3DS**.

**Status: bring-up only.** This builds a stub `.3dsx` that boots and shows a console. The game is not yet linked in.
Not tested on hardware or an emulator.

## Build
Requires [devkitPro](https://devkitpro.org) with devkitARM, libctru and citro3d.

    tools/fetch_upstream.sh      # clones the upstream decomp into ./upstream
    make                         # produces halo3ds.3dsx

## Docs
- [docs/PORTING_PLAN.md](docs/PORTING_PLAN.md): stages and risks
- [docs/PORTING_AUDIT.md](docs/PORTING_AUDIT.md): per-module platform dependencies

No game data is included; you need your own copy of the game.
