#!/bin/sh
# Fetch the upstream Halo decomp into ./upstream (gitignored).
set -e
cd "$(dirname "$0")/.."
[ -d upstream/.git ] || GIT_LFS_SKIP_SMUDGE=1 git clone --depth 1 https://github.com/bnunu/halo-1 upstream
git -C upstream rev-parse HEAD > upstream.rev
echo "upstream at $(cat upstream.rev)"
