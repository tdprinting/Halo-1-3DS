#!/bin/sh
# Rebuild ./work = pristine upstream (source, libs, config) + patches/*.patch.
# Edit files under work/, then run tools/save_patches.sh to turn the edits into per-module patches.
set -e
cd "$(dirname "$0")/.."
rm -rf work && mkdir work
for d in source libs config; do cp -r upstream/$d work/$d; done
for p in patches/*.patch; do
    [ -e "$p" ] || continue
    (cd work && patch -p1 -s < "../$p")
done
echo "work/ ready ($(ls patches/*.patch 2>/dev/null | wc -l) patches)"
