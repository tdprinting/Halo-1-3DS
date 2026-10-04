#!/bin/sh
# Host-side unit tests; no devkitPro needed.
set -e
cd "$(dirname "$0")/.."
out=${TMPDIR:-/tmp}/halo3ds_test_ctr_texture
gcc -std=c99 -Wall -Wextra -fsanitize=address,undefined -g -o "$out" tests/test_ctr_texture.c source/render3ds/ctr_texture.c
"$out"
# Shader assembly check if picasso is on PATH
if command -v picasso >/dev/null; then picasso -o "${TMPDIR:-/tmp}/ctr_basic.shbin" shaders/ctr_basic.v.pica && echo "shader assembles"; fi
