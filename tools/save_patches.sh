#!/bin/sh
# Regenerate patches/<dir>.patch for every top-level dir of source/ and libs/ that differs from upstream.
set -e
cd "$(dirname "$0")/.."
tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
ln -s "$PWD/upstream" "$tmp/a"; ln -s "$PWD/work" "$tmp/b"
rm -f patches/*.patch
for top in source libs; do
    ls "work/$top" | while IFS= read -r d; do
        out="patches/$top-$(echo "$d" | tr ' ' '_').patch"
        (cd "$tmp" && diff -ruN "a/$top/$d" "b/$top/$d" || true) > "$out"
        [ -s "$out" ] || rm -f "$out"
    done
done
ls patches
