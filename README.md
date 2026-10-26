# San Francisco Rush 2049 (GameCube, Midway Arcade Treasures 3): decompilation

A matching decompilation of *San Francisco Rush 2049* as it runs on the GameCube in *Midway
Arcade Treasures 3* (2005). The C source in [`decomp/src`](decomp/src) compiles, with the original
compiler, to the exact bytes of the game's executable.

The target is the debug build `rush_d.dol` (game ID `2049LE`). Its ELF twin, `rush_d.elf`, still
carries the full symbol table and CodeWarrior debug information, so the function, variable,
parameter, local and struct names in the source are the developers' own.

## Status

| | Done | Total | |
|---|---:|---:|---:|
| Game code bytes | 713,440 | 715,968 | **99.65%** |
| Functions | 1,841 | 1,844 | 99.84% |
| Source files fully matching | 146 | 148 | |

The relinked executable is checked against the original after every change
(`decomp/tools/verify_link.py`). It differs in exactly 5 known bytes: four in the destructor table
and one in the start-up copy table. They come from the linker: only CodeWarrior's GC 2.7 linker
completes the link, and it writes those bytes differently from the original's.

**What's left:** three functions are written in C and behave correctly, but the compiler assigns
different registers than the original:

- `GetCarsIRColor` in `visuals.c`
- `Blits` and `Blits_First` in `NGC/ngcZoid.c`

Not decompiled, by design:

- The Nintendo SDK, the C runtime and the Bink libraries. They are linked from the original binary.
- The small `main.c` unit (the main loop), still in assembly.

Run `python -B tools/status.py` in `decomp/` for the live figures.

## What's where

| Path | Contents |
|---|---|
| [`decomp/src/GameSource/src/`](decomp/src/GameSource/src) | The game: 120 files of car physics, AI, tracks, camera, HUD, menus, save data, audio and the "zoid" 3D object system. This code is shared with the game's other ports. |
| [`decomp/src/NGC/`](decomp/src/NGC) | The GameCube layer: GX rendering, disc and memory-card access, pads, Bink movies, the MAT3 menus |
| [`decomp/include/rush/`](decomp/include/rush) | Types (`rush_types.h`) and prototypes (`rush_protos.h`), generated from the debug info. Edit [`overrides.json`](decomp/include/rush/overrides.json) and regenerate; never edit the headers by hand. |
| [`decomp/include/rush_flags.h`](decomp/include/rush_flags.h) | Readable names for magic numbers (game states, car slots, effect bits). Hand-written; the names are ours. |
| [`decomp/config/2049LE/`](decomp/config/2049LE) | Symbols, file splits and build settings for decomp-toolkit |
| [`decomp/tools/`](decomp/tools) | Matching and verification tools: compare, check, link verification, status |
| [`tools/`](tools) | Recovery scripts: symbol extraction, DWARF decoding, header generation, local asset generation |
| [`docs/`](docs) | [Building and verifying](docs/BUILDING.md), the template's licence and the 2026-10-16 publication review |
| [`HANDOFF.md`](HANDOFF.md) | Working notes for whoever continues: resume steps, open work, compiler lessons |
| [`n64-helper.md`](n64-helper.md) | A guide for the separate N64 decompilation: how to use this C as a reference to name and understand N64 functions |

Most source files start with a one-line summary, the file's original path on the developers'
machine (`D:\rush_ngc\Rush_NGC\...`) and its matching status. Each function has its original
address above it.

## Building

You need your own copies of:

- the original `rush_d.dol`;
- the CodeWarrior GameCube compilers (GC 2.6 for the game, GC 2.7 for the link);
- decomp-toolkit, objdiff and Ninja.

This repository contains none of them. [docs/BUILDING.md](docs/BUILDING.md) gives the file hashes,
where everything goes and the exact commands.

Only the source is committed. Original game files, extracted art, compilers, build output and
disassembly are not, and must never be. The font and save-icon data that some source files include
is generated locally from your own `rush_d.dol` (`python -B tools/bootstrap_local_assets.py`).

## History

| Date | Code matched | Milestone |
|---|---:|---|
| 2026-10-14 | 22% | First consolidated waves, 91 files linked from C |
| 2026-10-16 | 60% | Source-only snapshot published and reviewed ([docs/REVIEW.md](docs/REVIEW.md), [research/](research)) |
| 2026-10-17 | 99.65% | 43 commits in one day: all but three functions matched, units reformatted for readability |
| 2026-10-19 | 99.65% | Annotation pilot: `rush_flags.h`, header comments for `mdrive.c` and `resurrect.c` |

`progress.json`, `docs/REVIEW.md` and `research/` describe the 2026-10-16 snapshot and have not
been regenerated since. Use the figures above, or `status.py`.

## Rights

See [LICENSES.md](LICENSES.md). This project grants no rights to the game, its assets, the Nintendo
SDK or the CodeWarrior tools. The build scaffold comes from
[encounter/dtk-template](https://github.com/encounter/dtk-template).
