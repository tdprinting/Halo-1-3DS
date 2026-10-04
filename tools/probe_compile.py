#!/usr/bin/env python3
"""Syntax-check upstream sources with clang (32-bit) plus our compat shims, and summarize errors.

Usage: tools/probe_compile.py UPSTREAM_DIR MODULE [MODULE...] [--show N]
Stand-in for devkitARM (also GCC + newlib, armv6k VFP hard-float).
"""
import collections, json, re, subprocess, sys
from pathlib import Path

args = sys.argv[1:]
show = 8
list_files = "--files" in args
args = [a for a in args if a != "--files"]
if "--show" in args:
    i = args.index("--show"); show = int(args[i + 1]); del args[i:i + 2]
include_replaced = "--include-replaced" in args
args = [a for a in args if a != "--include-replaced"]
up = Path(args[0]).resolve(); mods = args[1:]
here = Path(__file__).resolve().parent.parent
cfg = json.loads((up / "config/config.json").read_text())
inc = [up / d for d in cfg["projects"][0]["options"]["include_dirs"]]
cmd0 = ["arm-none-eabi-gcc", "-march=armv6k", "-mfloat-abi=hard", "-mfpu=vfp", "-c", "-o", "/dev/null", "-w",
        "-std=gnu99", "-fms-extensions", "-Werror=implicit-function-declaration", "-Werror=incompatible-pointer-types",
        "-fshort-wchar", "-fno-strict-aliasing", "-DNON_MATCHING", "-D__3DS__", "-include", str(here / "compat/halo_compat.h"), "-I", str(here / "compat")]
for d in inc: cmd0 += ["-I", str(d)]
SKIP = ("bitmaps/libtiff/", "memory/zlib/contrib")
REPLACED = []
if not include_replaced:
    for line in (here / "port/replaced_files.txt").read_text().splitlines():
        if line and not line.startswith("#"):
            REPLACED.append(line.split("|")[0].strip())

ok = bad = replaced_n = 0; failing = {}
errs = collections.Counter(); first = {}
for m in mods:
    for f in sorted((up / m).rglob("*.c")):
        rel = str(f.relative_to(up))
        if any(k in rel for k in SKIP): continue
        if any(rel == r or (r.endswith("*") and rel.startswith(r[:-1])) for r in REPLACED): replaced_n += 1; continue
        r = subprocess.run(cmd0 + [str(f)], capture_output=True, text=True)
        e = [l for l in r.stderr.splitlines() if re.search(r": (fatal )?error:", l)]
        if r.returncode != 0 and not e: e = [f"{f}: error: clang exited {r.returncode}"]
        if not e: ok += 1; continue
        bad += 1
        failing[str(f.relative_to(up))] = (re.sub(r"^.*?: (fatal )?error: ", "", e[0])[:95], len(e) - 1)
        for l in e:
            msg = re.sub(r"^.*?: (fatal )?error: ", "", l)
            errs[msg] += 1; first.setdefault(msg, re.split(r": (?:fatal )?error:", l)[0].replace(str(up) + "/", ""))
print(f"{ok} files ok, {bad} files with errors, {replaced_n} platform files replaced by port")
for msg, n in errs.most_common(show):
    print(f"{n:5d}  {msg[:110]}   [{first[msg]}]")
if list_files:
    for f, (first_err, more) in failing.items(): print(f"{f} | {first_err} (+{more})")
