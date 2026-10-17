# Handoff — Rush 2049 (GameCube, MAT3) matching decomp

Last session: 2026-10-16 (Claude Code, targets). Read this first, then `writeup.md` (local-only, not in git).

## Where things stand

- **Status:** 1,621 / 1,844 functions, 514,184 / 715,968 code bytes (71.82%), **124 / 148 files**
  linked from C. The GC/2.7 relink passes `decomp/tools/verify_link.py` (only the 5 recorded
  `.dtors` bytes differ) on every commit. (`GameSource/src/targets` is 32/33 with one
  register-allocation near miss and stays NonMatching; see open work.)
- **Branch:** `ngcblit-match` on <https://github.com/cabi24/SFRush2049-mat3-decomp> (private),
  commits ahead of `main`, pushed, no PR opened yet:
  - `24dc654` Match NGC/ngcBlit (16/16)
  - `3091952` Match NGC/ngcBeginMenu (38/38)
  - `2ea74ee` Add NGC/ngcZoid (16/19, left NonMatching)
  - `0556df2` Match NGC/ngcVu1pack (34/34, 122/148 units linked from C)
  - `f7416d3` Match GameSource/src/zip (24/24, 123/148 units linked from C)
  - `83506b0` Match GameSource/src/highscorescr (21/21, 124/148 units linked from C)
  - (this commit) Add GameSource/src/targets (32/33, left NonMatching: `SetTargetDefs` regalloc)
- **Repo layout on this PC:** `C:\rush2049-mat3` *is* the git checkout (source-only). Originals
  (`Rush2049.iso`, `DATA.GDF`, `rush_d.elf`, `rush_d_*.csv`, `rush_d_dwarf.c`), compilers,
  `decomp/build`, `decomp/orig`, `work/`, `writeup.md` are local and git-ignored — never commit
  them. Codex pushes its own waves to GitHub, so **`git fetch` first**; GitHub may be ahead of disk.
  Repo-local git identity is set (`cabi24 <cabi24@gmail.com>`).

## Resume checklist

```bash
cd C:/rush2049-mat3 && git fetch && git status -sb
```
```bash
cd C:/rush2049-mat3 && python -B tools/bootstrap_local_assets.py
```
(only after a pull that touched font/icon code; regenerates `decomp/include/generated/*.inc` from
your `rush_d.dol`.)
```bash
cd C:/rush2049-mat3/decomp && python -B configure.py --no-progress --compilers build/compilers --dtk build/tools/dtk.exe --objdiff build/tools/objdiff-cli.exe --ninja build/tools/ninja.exe --binutils build/binutils-not-installed --sjiswrap build/tools/sjiswrap-not-installed.exe
```
(the two `-not-installed` flags stop the template from adding download steps; **re-run configure
whenever a new `src/` file appears**, ninja won't notice it otherwise.)
```bash
cd C:/rush2049-mat3/decomp && build/tools/ninja.exe; build/tools/ninja.exe build/2049LE/report.json; python tools/verify_link.py --no-build; python tools/status.py
```
`ninja` always ends with `build/2049LE/ok FAILED` (the sha1 step trips on the 5 `.dtors` bytes);
that is expected. `verify_link.py` is the real gate.

## Open work, in suggested order

0. **`GameSource/src/targets` — 1 near miss, everything else done** (`decomp/src/GameSource/src/targets.c`,
   32/33, data 100%, layout exact, the link dry run with it marked Matching differed from the DOL only inside
   `SetTargetDefs`). `SetTargetDefs` 99.9% (9/373 instructions, registers only): the hoisted `vis`/`col`
   `SetupTarget` arguments and the inner `j*0x44` strength-reduction temp are coloured r16/r15/r14 instead of
   r14/r15/r16. Agent D's `#pragma opt_strength_reduction off` probe shows the target created the induction
   *after* the two hoisted invariants; ours creates it before. Tried-lists: `work/targets/agent{A,D}/NOTES.md`
   (well over a thousand variants: argument spellings, named locals in every scope, dead references, loop
   wrappers, index/pointer forms, inline helpers). When it matches: `python tools/mark.py Matching
   GameSource/src/targets.c`, configure, ninja, verify, commit (125/148).
1. **`NGC/ngcZoid` — 3 near misses, everything else done** (`decomp/src/NGC/ngcZoid.c`).
   `Blits_First` 97.3% and `Blits` 96.3% share one cause: the original emits
   `mr r6,r30; add r6,r28,r6` for `BlitDraw(..., x + w, y + h, ...)` (copy loop-carried operand
   into the arg register, add in place) — a pattern that occurs **nowhere else in the DOL**; ~150
   source variants tried (`work/ngcZoid/agent{B,C}/NOTES.md`). `ZOID_EndFrame` 99.2% keeps the
   loop-2 walker address (`&ZoidWinInfo[..]`, r26) live across loop 1; the outer-`while (1)` /
   loop-splitting trick that fixed `MenuMCMount` has **not** been tried on it yet. When these
   three match: `python tools/mark.py Matching NGC/ngcZoid.c`, rebuild, verify, commit.
2. **Next untouched files, smallest first:** `trackscr`, `recordscr`, then the
   big screens `playerscr`, `visuals`, `carscr` (27–31 KB each). (`GameSource/src/targets` done
   2026-10-16 except `SetTargetDefs`: 32/33 in ~2 h with two agents, a third on the three
   `_TargetColl*` FP near misses (matched in 5 min) and a fourth on `SetTargetDefs`; see writeup
   "targets". `GameSource/src/highscorescr` done 2026-10-16: 21/21 in ~2.5 h with two agents plus a
   third on the one near miss; see writeup "highscorescr matched".)
   (`NGC/ngcVu1pack` done 2026-10-16: 34/34, ~2.5 h wall clock with two agents; `GameSource/src/zip`
   done 2026-10-16: 24/24 in ~1 h, it is zlib 1.0.4-era deflate and matched almost verbatim; see
   writeup "ngcVu1pack matched" / "zip matched".)
3. **Old near misses** (all heavily tried, register-allocation residue): `enginetorque`,
   `appGetTime`, `ContinueCircuit`, `_DrawFileDel`, `ngcLoadEffectBank`, `AvoidAreas`,
   `LinkWithPS2Texture`, `frictioncircle`; lower-scoring and probably structural: `drawRect` 91.5%
   (`gameoverscr`), `calcMenuRect` 93.4% (`videoscr`). Codex's wave25/26 lanes (`hud`, `mdrive`,
   `resurrect`, `maxpath`, `stree`) have 16 partial bodies — see `research/README.md`.
4. **Linker:** only GC 2.7 completes a full link, hence the 5-byte `.dtors` difference; pre-2.7
   linkers hang (writeup "Status" section). Research-grade.
5. **Tooling gaps that bit three times today:** `tools/skel.py` emits non-compiling skeletons on
   (a) nested-block locals flattened into duplicate declarations, (b) DWARF definitions with fewer
   parameters than the header prototype (`DC_SendFillRect`), (c) a `static` function whose name is
   a global array in the shared header (`Blits`), (d) **function-local statics are omitted entirely**
   (ngcVu1pack: five `camPt/at/up` triples and `DrawSortedTex`'s seven statics had to be added by
   hand from the DWARF), (e) the DWARF signature is used even when `overrides.json` already has a
   prototype for the function (`MySetAmbientColor`, `LoadGeoBinFile`), (f) a backslash-n in the original path
   becomes a newline in the header comment, (g) a mid-file file-scope static (`memcard` in highscorescr: the
   DWARF lists it right before a function like a local static, but the target symbol has no `$NNN` suffix) is
   hoisted to the top with the other data (targets: three of them, `gMineRingColors`, `ammoCnts`, `flag_pals`;
   a DWARF line *with an address* right before a function is a definition, a reference inside the function has
   none), (h) a function-local static whose address is in `.rodata` must be `static const` (`gravity$1511`),
   (i) a function-pointer table initialiser is left as `/* TODO initializer */` (needs casts when the
   functions' arities differ), (j) a `const` global (`gMineRingColors`) is not `const` in the header until
   added to `overrides.json` `const_variables`. Fixing those in `skel.py` saves a round-trip per file.

## Workflow that worked today (per file, ~1–2 h wall clock)

1. `python tools/skel.py NGC/<unit> > src/NGC/<unit>.c`; `python tools/check.py NGC/<unit>
   --output-root ../work/<unit>/check-base` until the skeleton compiles (dedupe locals etc.).
2. Write `work/<unit>/BRIEF.md` (copy `work/ngcZoid/BRIEF.md` — it has the rules, tool commands and
   the traps) and launch **two Opus subagents** on disjoint function sets, each in a private copy
   `work/<unit>/agent{A,B}/<unit>.c`. Agents use only `tools/cwmatch.py` (5 s byte compare) and
   `tools/candidate.py` (private whole-TU objdiff); they never touch `decomp/src|include|config|build`
   and never run ninja/mark/verify/check/fill. Deliverables: `bodies.txt` (`== Func` sections) +
   `NOTES.md` (table, header/override needs, preamble additions, codegen rules).
3. Parent merges with a short Python script (fill `/* TODO */` bodies, apply preambles), puts
   prototype/`volatile`/`const` fixes in `decomp/include/rush/overrides.json` and regenerates with
   `python -B tools/dwarftypes.py rush_d.elf decomp/include/rush` (from repo root; check the
   `rush_protos.h` diff is only the intended lines). **Never hand-edit generated headers.**
4. `check.py` on the merged file; fix layout from its symbol list (see rules below); then
   near-miss agents with the merged file as base.
5. `mark.py Matching`, full rebuild, `verify_link.py`, `status.py`, add lessons to `writeup.md`,
   commit on the branch, push.

## Rules learned today (details and more in `writeup.md`, sections dated 2026-10-16)

- **Data emission order:** CodeWarrior emits an *explicitly initialised* static (even `= 0`, still
  `.sbss`) at its declaration point in declaration order, and defers every *uninitialised* static
  (file-scope and function-local alike) to end of TU in **reverse** declaration order. Decode a
  `check.py` layout mismatch with that; fix declarations/initialisers, not function order.
- **Stripped functions still shape data:** linker-dropped functions leave local-array initialisers
  and string literals in `.rodata`; keep them as `static` placeholders (`/* stripped */`) in their
  original position (ngcBlit, ngcBeginMenu). `rush_d_functions.csv` lists them (`stripped=1`).
- **Mid-file data:** large arrays (`MCMessages`) may be defined between functions in the original;
  `.rodata`/`.data` displacement-only diffs tell you where.
- **Hoisting needs a real outer loop:** `while (1) { for (;;) {...} }` keeps a separate loop
  header; loops sharing a start are merged and lose the hoist. "Rotated registers" that resist all
  declaration orders usually mean a missing/extra hoist, or an address spelled two ways.
- **Commutative float operands** are ordered by value creation time, not source order; read an
  operand into fresh locals in a new block to move it first.
- **Store order is not always reliable** (`Polys`, cross products): when otherwise exact, permute.
- **Declaration initialisers** (`T *p = expr;`) evaluate before local aggregate copies.
- **High bytes in strings** must be raw bytes in the source, not `\x80`; objdiff can say 100% on a
  data section that is not byte-equal — only `verify_link.py`/`doldiff.py` catch it.
- Unprototyped originals (`SetFont` doubles, `PrintFont` 3/4 args, `BlitColors` 4 args):
  `#define X X_DWARF_decl` around the include, then `void X();`.
- Objdiff/cwmatch "CANDIDATE" = bytes equal, only a relocation symbol name differs → a match.
- **`b<cc>lr; blr` at the end of a void function = the condition was tested twice**
  (`if (x >= K && x >= K) return;`); every single-test form folds the branch (ngcVu1pack).
- **A `.bss` gap at the end of a unit's split can be the next unit's alignment**: fix it with
  `__attribute__((aligned(32)))` on the next unit's first `.bss` object (`g_VW` in ngcGraph.c);
  `align:N` in `symbols.txt` does nothing for C-linked units.
- `work/ngcVu1pack/merge.py` merges whole function definitions from agent copies into the
  canonical file (adapt the paths per unit; `work/zip/merge.py` is the zip copy, it also accepts
  `/* stripped by the linker (no data) */` placeholder headers).
- **Literal types change CSE** (zip): `x >= 0L` vs `x >= 0` decides whether a struct field is
  reloaded after a ternary. CW 2.6 never forwards a store to a later load of the same field through
  a pointer, so verbatim library text (zlib) reproduces the target's reloads.
- **Functions that address several `.bss` arrays from one hoisted base only match once the whole
  TU fixes the `.bss` order** (zip's `compress_block` needs `tr_static_init`'s first-use order).
- Third-party library units (zlib, etc.): start from the library's original text of the matching
  era; it compiled byte-exact almost unchanged under CW 2.6 `-O4,p`.
- **Run the link dry run right after the merge** (`mark.py Matching`, `ninja`, `verify_link.py --no-build`),
  before chasing near misses: a stripped function's float literal missing from `.sdata2` (highscorescr's
  `_HiScoreRank` 0.0f) shifts every later `.sdata2` pointer in the DOL, and `check.py`/objdiff still report
  data 100% and layout OK. Give the stripped placeholder a body that emits the literal.
- **Register priority follows first reference in the body, not declaration order** (highscorescr
  `DrawGhost`): two locals with swapped callee-saved registers that resist every declaration order are fixed
  by an early constant-propagated reference (`y = 10;` passed where the literal was) to the one that should
  get the higher register number. Emits no code.
- Enum names vs int literals in a ternary decide branch-free vs branching selects; `width /= 2` vs
  `width / 2` inline, block-scoped loop counters, `n--; a[n] = 0;` vs `a[--n] = 0;` all change allocation
  (details in writeup "highscorescr matched").
- Agents' `cwmatch.py` workaround for static names that exist in several units (`ambiguous-orig`):
  `work/highscorescr/agentA/fd.py`, `agentB/cm.py`. Worth folding into `cwmatch.py`.
- **A `.rodata` literal with no reference in the `.s` is not necessarily an orphan** (targets `@1507`): CW keeps
  the section anchor (`@102`) in a register and addresses later literals as `addi rX, rAnchor, off`. Check the
  offsets before inventing dead code.
- **Two `sqrtf` inline variants:** anonymous `.sdata2` doubles (`@NNN` 0.5/3.0/0.0) mean the camera.c/tires.c
  variant (`#define MSL_NO_SQRTF` + automatic `_half`/`_three`); `_half$localstatic3$sqrtf` symbols mean
  `msl_math_inline.h`'s. Same code, different symbol names.
- **In-place update position decides FP colouring** (targets `_TargetColl*`): `radius += 3.5f;` after the delta
  computations, with the deltas as function-scope `float dx, dy, dz;` declared before the DWARF locals, even
  though the scheduler emits the add in the same slot either way.
- **Unrolled zeroing loop stored entirely through the `addi r3, base, 0` copy** = `while (i < 8) a[i++] = 0;`;
  `for (i...) a[i] = 0;` stores the first element through the base register. `switch` case bodies are emitted in
  source order, not jump-table order; CW never tail-merges identical blocks (duplicate the code, `goto` shared
  exits). `work/targets/BRIEF.md` is the latest brief template; `work/targets/merge.py` takes agents A-D.

## Files to know

- `writeup.md` (repo root, local-only): the full knowledge base — compiler facts, per-file lessons,
  tool table, status/linker notes. Keep adding to it.
- `work/<unit>/BRIEF.md`, `work/<unit>/agent*/NOTES.md`: today's agent briefs and findings
  (local-only). `work/targets/BRIEF.md` is the latest template (per-function literal map from the `.s`,
  type offsets, layout facts, the fresh-shell `cd` note); `work/targets/merge.py` the latest merge script;
  `work/targets/litmap.py` maps every `@NNN` literal to the function that references it; `agentA/qd.py`
  is the fastest per-function diff (GC 2.6 compile + dtk disassembly against the target `.s`).
- `decomp/docs/verification-baseline.md`, `docs/REVIEW.md`, `research/README.md`: Codex's
  verification rules and remaining-work notes (in git).
- `decomp/tools/`: `skel.py`, `check.py`, `cwmatch.py`, `candidate.py`, `fvar.py`, `fdiff.py`,
  `fields.py`, `dolbytes.py`, `doldiff.py`, `mark.py`, `verify_link.py`, `status.py`.
