# Using the MAT3 GameCube decomp as a reference for the N64 decomp

## What this file is for, and why

This is a briefing for an AI agent, or a person, working on the **separate decompilation of the
N64 release** of *San Francisco Rush 2049* (2000). Point that agent at this file. It says where the
C code in this repository is, how far to trust it, and how to use it to identify and understand N64
functions.

Why it exists:

- **The N64 ROM has no symbols.** Every function there starts life as `func_800XXXXX`. This
  GameCube build shipped with full debug information. Its C has been verified byte for byte against
  the original executable, so the names, types and logic here are the closest thing available to
  the original source.
- **Both versions come from one code base.** Much of the shared game code exists in both builds,
  so a function here often corresponds one-to-one to an N64 function: car physics, AI, tracks, HUD,
  menus, save data.
- **The C says much more than the symbols.** Until now the N64 project used only this build's
  symbol and type information. The C adds control flow, constants and string usage, which identify
  an unnamed N64 function far faster than names and types alone.
- **It already worked the other way round.** This project once used an N64 function to recover a
  GameCube one (section 7). This guide makes the reverse direction systematic.

The N64 project only reads from this repository; nothing here depends on it. Some files mentioned
below are local and not in git (section 4 says how to regenerate them). Those, and the separate PC
port in section 1, exist only on the maintainer's PC unless you generate them yourself.

---

You are decompiling **San Francisco Rush 2049 for the N64**. This directory holds a
**byte-matching C decompilation of the GameCube version** of the same game, the one in *Midway
Arcade Treasures 3* (2005), built from the debug executable `rush_d.elf`.

Both builds come from one code base, Midway's `GameSource` tree. The GameCube copy still carries
N64 code and names: Controller Pak and Rumble Pak screens, an N64 scheduler, RSP/RDP profiling
timers, `U64*` helpers. That makes it a strong reference for:

- **Names:** functions, globals, parameters, locals, struct and field names.
- **Types:** struct layouts and field offsets.
- **Logic:** control flow, constants and string usage.

Use it to identify and name N64 functions, then reconstruct them for the N64 compiler.

Paths below are relative to the repository root (`C:\rush2049-mat3` on the maintainer's PC) unless
they are absolute.

---

## 1. Where the C code is

| Path | What it is | Use for N64? |
|---|---|---|
| `decomp/src/GameSource/src/*.c` (120 files) | The shared game code: car physics, AI drones, tracks and paths, camera, HUD, every front-end screen, save data, audio manager, battle mode, the "zoid" 3D object system | **Yes, this is the main reference** |
| `decomp/src/NGC/` (28 files) | GameCube platform layer: GX rendering back end, DVD files, memory card, pads, Bink movies and music, MAT3 collection menus | Behaviour only; the N64 has its own platform layer |
| `decomp/include/rush/rush_types.h` | Every struct, union and enum from the debug info, with byte offsets in comments (`/* 0x30 */`) and `/* size 0x98 */` | Yes: layouts and field names |
| `decomp/include/rush/rush_protos.h` | Every function prototype and `extern` global | Yes: signatures and names |
| `decomp/include/rush_flags.h` | Hand-written names for magic numbers (`GS_*` game states, car-slot counts, effect bits). The names are ours, not original | Yes, as documentation |
| `decomp/include/rush/overrides.json` | Corrections fed into the header generator | Rarely |

**Do not read from `C:\rush2049-pc`.** That folder, on the maintainer's PC only, holds a PC port of
this tree with `#ifdef RUSH_PC` patches throughout. The one exception: `C:\rush2049-pc\pc\src\recovered\main_loop.c` is the only C version
of `main.c` (`MainLoop`, `checkForExit`, `fps_control`). The matching tree has that unit only as
assembly. The PC version follows the disassembly but is not byte-matched.

## 2. How far to trust it

- **Status:** 1,841 of 1,844 functions compile to the exact bytes of the GameCube executable
  (Metrowerks CodeWarrior GC 2.6), in 146 of 148 files. The three exceptions are complete C that
  differs only in register allocation: `GetCarsIRColor` (`visuals.c`), `Blits` and `Blits_First`
  (`NGC/ngcZoid.c`). Treat all of it as an exact statement of what the GameCube build does.
- **Authoritative status:** the `Object(Matching|NonMatching, ...)` list in `decomp/configure.py`.
  Status words in individual file headers ("in progress", "private candidate") can be stale.
- **Names are original** where they come from the debug info: functions, globals, statics,
  parameters, locals, struct tags and fields. **Not original:**
  - `anon_<fields>_<size>` type names, generated for anonymous structs. For example
    `anon_mat_n64_98` is an anonymous 0x98-byte struct whose fields include `mat` and `n64`.
  - Everything in `rush_flags.h`.
  - Aliases such as `foo_dwarf_decl` and `foo_unqualified`. These are `#define` tricks around the
    header include that hide a conflicting generated declaration.
- **Code shape is tuned for CodeWarrior.** To get identical bytes, some functions use small inline
  helpers, extra temporaries, `do { ... } while (0)` blocks, repeated loads, `volatile` globals or
  a particular evaluation order. Each such trick has a comment in place. The meaning is exact; the
  statement shape may not be what the developers typed, and it is the wrong shape for the N64
  compiler. Rebuild the logic in whatever shape your compiler needs.
- **Function order within a file is the original order**, because the linker layout had to match.
  The line ranges in `rush_d_functions.csv` (section 4) are the original source line numbers.

## 3. Conventions inside each file

```c
/* zoidwindow.c - viewport setup, fog, and screen projection. Status: matched.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\zoidwindow.c; assembly: build/2049LE/asm/GameSource/src/zoidwindow.s */
...
/* 0x80063094, size 0x150 */                 <- GameCube address and size of the function
tWindowInfo *ZOID_SetWindow(unsigned int winidx, float hangle, ...)
{
    tWindowInfo *winfo; /* r31 */            <- register or stack slot from the debug info (GameCube only)
```

- `D:\rush_ngc\Rush_NGC\GameSource\src\` is the developers' own path. GameCube-only code sits in
  `...\Rush_NGC\NGC\`.
- `/* rNN */` and `/* stack */` on locals describe GameCube register allocation. Ignore them for
  the N64.
- Float literals often appear in full float precision, for example `0.6366197466850281f` (2/π) or
  `31.41592788696289f`. When searching for a constant, search a short prefix such as `0.63661`.

## 4. Indexes and raw material

Rows marked *(local)* are not in git; the others are.

| File | Contents |
|---|---|
| `rush_d_functions.csv` *(local)* | All 2,196 functions in the debug info: `name, address, end, size, scope, stripped, source_file, line, end_line, source_path`. **`stripped=1`** (352 functions) means the GameCube linker dropped the function: there is no code, only its name, signature and locals. Some of these may be live in the N64 build. |
| `rush_d_variables.csv` *(local)* | All globals and statics with their address: one row per compile unit whose debug info mentions the variable (`stripped=1`: dropped by the linker) |
| `rush_d_compile_units.csv` *(local)* | The 149 compile units: original path, address range, counts |
| `rush_d_dwarf.c` *(local)* | Full DWARF dump (5 MB) in C-like form. Every function, stripped ones included, with parameters, locals, `// Range:` addresses and inlined-call info. Use it for stripped functions, which have no body in `decomp/src`. |
| `decomp/config/2049LE/symbols.txt` | Every symbol by address (`name = .section:0xADDR; // type size scope`) |
| `decomp/config/2049LE/splits.txt` | Which address ranges belong to which source file |
| `decomp/build/2049LE/asm/` *(local)* | Disassembly of every unit, GameCube SDK included (`GameSource/src/*.s`, `NGC/*.s`, `main.s`), written by the decomp build |
| `writeup.md` *(local)* | The project's knowledge base. Mostly CodeWarrior matching lessons. The "Donor evidence" section records how an N64 function was used in this project (section 7) |
| `HANDOFF.md` | Current status and open work |

In a fresh clone, regenerate the *(local)* indexes from your own copy of the disc (repository root;
`dtk` is decomp-toolkit; [docs/BUILDING.md](docs/BUILDING.md) covers the build that writes the
`asm/` folder):

```bash
python tools/extract_syms.py Rush2049.iso 0x55056800 5611448 rush_d.elf rush_d_symbols.csv
python tools/dwarf1.py rush_d.elf . decomp/config/2049LE/splits.txt
dtk dwarf dump --include-erased --no-color rush_d.elf -o rush_d_dwarf.c
```

Do not copy `rush_d.elf`, `Rush2049.iso`, `DATA.GDF` or anything under `vanila/` into your
project. Those are original game files.

## 5. Finding things

From the repository root (ripgrep or grep both work):

```bash
# a function's definition (one per file; the address comment is on the line above)
rg -n "^[A-Za-z_].*\bSetCountdownTimer\(" decomp/src
# every use of a global
rg -n "\bgRetraceTime\b" decomp/src
# a struct and its field offsets (the "{" skips the forward typedef)
rg -n -A30 "^typedef struct tWindowInfo \{" decomp/include/rush/rush_types.h
# a global's GameCube address, and every file whose debug info mentions it
rg -n "^gCamera," rush_d_variables.csv
# a stripped function's signature and locals (no code in the GameCube build)
rg -n -A12 "PakErrorActive" rush_d_dwarf.c
# a constant as written in the C
rg -n "0\.63661" decomp/src
```

**Matching an unnamed N64 function to a GameCube one.** In order of reliability:

1. **String literals.** Search `decomp/src` for strings the N64 function references: menu text,
   file names, debug and assert text.
2. **Distinctive constants.** Float literals (search a decimal prefix), table sizes, bit masks
   (often named in `rush_flags.h`), the `GS_*` state values.
3. **Calls and globals.** Once a few neighbours are named, the call graph and global accesses pin
   down the rest. Function order inside a file is the original order, and the N64 build was
   probably linked from the same files, so neighbours tend to stay neighbours.
4. **Struct offsets.** Field offsets in `rush_types.h` often hold on the N64: both machines are
   32-bit, big-endian, and align doubles to 8. Check them against the N64 accesses before trusting
   them; platform fields and save formats differ (section 7).

## 6. Map of the shared game code (`decomp/src/GameSource/src`)

| Area | Files |
|---|---|
| Boot, game flow, state machine | `boot_pc.c` (entry), `Init.c` (start-up, PAL60), `state.c` (game-state transitions), `game.c`, `front.c` (front-end dispatch), `demo.c` (attract race), `timer.c`, `sched.c`, `os.c`, `gfx.c`, `HAL_time.c`, `HAL_Support.c` |
| Car simulation | `mdrive.c` (per-frame driver; has a frame-flow overview at the top), `model.c` (model thread, rubber-band speed), `drivsym.c` (vehicle integration), `drivetra.c` (engine, clutch, gearbox), `tires.c` (suspension, grip), `road.c` (tyre road contact), `collision.c`, `resurrect.c` (respawn; annotated), `rekon.c` (dead reckoning), `initiali.c`, `cars.c` (car definition tables), `controls.c` |
| AI and paths | `drones.c` (AI cars), `maxpath.c` (max-path control), `pathcommon.c` (path and checkpoint loading and queries), `checkpoint.c` (laps, placement), `targetpaths.c`, `ghost.c` (ghost record and replay) |
| Track and world | `tracks.c` (staged track loading), `world.c` (world tree, zone culling, fog), `stree.c` (spatial tree), `sky.c`, `textureanim.c`, `worlddam.c` (scorch decals), `targets.c` (interactive track objects) |
| Rendering objects | `zoid.c`, `zoidpoly.c`, `zoidblit.c`, `zoidlight.c`, `zoidtexture.c`, `zoidwindow.c` (the "zoid" object/renderer API; `ZoidWinInfo` and FOV), `visuals.c` (car visuals: tyres, shadows, sparks, env maps), `camera.c`, `skids.c`, `smoke.c`, `streaks.c`, `headlights.c`, `countdown.c`, `blit.c`, `blit_pc.c`, `pane.c`, `fake.c` (matrix helpers) |
| HUD and text | `hud.c`, `hudextra.c`, `timingbar.c`, `stats.c`, `HAL_text.c` (fonts, glyphs), `TB_text.c`, `text.c` (per-language text) |
| Front-end screens | `titlescr.c`, `licensescr.c`, `startscr.c`, `playerscr.c`, `modescr.c`, `trackscr.c`, `carscr.c`, `optionscr.c`, `options.c`, `controlscr.c`, `audioscr.c`, `videoscr.c`, `recordscr.c`, `highscorescr.c`, `gameoverscr.c`, `pausescr.c`, `circuitscr.c`, `cheatscr.c`, `warningscr.c`, `pakscr.c` |
| Modes | `circuit.c`, `stunt.c`, battle: `weapons.c`, `shots.c`, `powerups.c`; secrets: `eggs.c`, `password.c` |
| Save data and records | `save.c`, `gamerecords.c`, `paksysdc.c` (memory-pak file system), `pakerr.c`, `memsys.c` (heap and handles) |
| Input | `controlsys.c` (pad to button bits, auto-repeat), `controls.c` |
| Audio | `audio.c`, `audiomgr.c`, `audiosfx.c`, `waveinfo.c`, plus PS2-interface layers `audiosfxps2.c`, `audiosongps2.c`, `audiosysps2.c` |
| Resources and files | `fakegdfs.c` (file loading with cache), `mb_wad.c` ("wad" resource files), `waves.c` (wave resources, model relocation), `carload.c`, `selectcarload.c`, `mbox.c`, `texture.c`, `zip.c` / `unzip.c` (deflate and inflate) |
| Libraries | `fmath.c` (vectors, matrices, quaternions), `TB_Vector.c`, `asincos.c`, `atan.c`, `tan.c`, `modf.c`, `bsearch.c`, `bstree.c`, `ll.c`, `statll.c` |

Each file's first line gives a one-sentence summary. `rg -n -m1 "" decomp/src/GameSource/src/*.c`
lists them all.

## 7. What differs from the N64, and N64 traces to anchor on

**N64 code still visible in the GameCube build** (good starting anchors):

- `pakscr.c`, `pakerr.c`, `paksysdc.c`: Controller Pak / Rumble Pak vocabulary, for example
  `_DrawRumblePak`, `_ControlRumblePak`, `_ControlPakErrNQA`, `PS_PakRumble`, `PS_PakDamaged`.
  On the GameCube these sit on top of the memory card.
- `sched.c`: "N64-style scheduler state kept for the port" (`OSScTask`, `SCClient`;
  `rdp_buffers` survives in the debug info).
- `timingbar.c`: the N64 RSP/RDP profiling timers. `gGfxRdpEndTime` is still live; the stripped
  `gGfxRspStartTime`, `gGfxRspEndTime`, `gAudRspTime` and `gAudRspStartTime` survive only as
  entries in `rush_d_variables.csv`.
- `camera.c` `UpdateCameras`: `gCamera[i].n64` (type `anon_mat_n64_98`) is a copy of each camera
  matrix with four off-diagonal rotation terms negated (a handedness flip), kept for the N64
  renderer's convention.
- `fmath.c` has `U64WorldVector`; `blit_pc.c` had `U64_TextureWrap` (stripped). "U64" is "Ultra 64".
- The `zoid*` object system and `model.c`'s "ModelThread" come from the arcade lineage the N64
  version also derives from.

**GameCube- or PS2-lineage code that the N64 version will not share in this form:**

- All of `decomp/src/NGC/`: GX rendering (`ngcZoid.c`, `ngcVu1pack.c`), DVD (`ngcFile.c`), memory
  card, pads, Bink (`ngcAudio.c`, `BINK/ngcrad3d.c`), and the MAT3 menus, bonus gallery and save
  file (`ngcBeginMenu.c`, `bonus.c`, `MATMEM/`).
- The PS2-interface layers inside `GameSource`: `audiosfxps2.c`, `audiosongps2.c`,
  `audiosysps2.c`, PS2 shims (`NGC/libgraph.c`, `NGC/stdlib.c`, `NGC/ngcDmalist.c`, `NGC/ngcMtx.c`),
  and the C++ font code (`NGC/FONT/*.cpp`).
- **Byte swapping in data loaders** (`_swap_x16`/`_swap_x32` in `pathcommon.c` and elsewhere,
  `SWAP16`/`SWAP32`): the MAT3 data came through the PS2 build. On the N64 the data is native
  big-endian, so expect no swaps.
- **PowerPC intrinsics:** `__fabs`, `__frsqrte`. For example, `surface_sqrt` in `resurrect.c` is a
  reciprocal-square-root Newton step; the N64 will use `sqrtf` or its own routine.
- **Sizes, revisions and limits can differ while the structure is the same.** This project once
  used an N64 function (`func_800CCCCC` in the N64 decomp) to recover GameCube `_InitPlayer`
  (`save.c`). The loop and call sequence carried over exactly. The constants did not: the N64 uses
  save revision 16 and clears 44 and 2064 bytes, the GameCube revision 19 and 0x48 and 0x814 bytes.
  Expect that pattern everywhere save data, memory sizes or platform buffers are involved.
- **Timing:** the simulation steps `gRetraceTime` per tick (1/60 s NTSC, 1/50 s PAL; `Init.c`,
  `timer.c`). `mdrive.c`'s header explains the tick structure.

## 8. Ground rules for this directory

- **Read only.** This is a matching project with a verified executable hash. Do not edit files,
  and do not run `configure.py` or `ninja`: `configure.py` without its tool-path flags downloads
  tools into `decomp/build/tools`. Copy what you need into your own project.
- `CLAUDE.md` and `HANDOFF.md` are for agents working on *this* decomp; you don't
  need their workflow.
- If a GameCube name or struct contradicts what the N64 binary does, the N64 binary wins. This
  tree tells you what the shared source most likely says, not what the N64 build compiled.
