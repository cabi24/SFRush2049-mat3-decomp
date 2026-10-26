# Handoff: Rush 2049 (GameCube, MAT3) matching decomp

Working notes for whoever picks this up next (people and agents). Last updated 2026-10-26.
Read this first, then `writeup.md`: the full knowledge base, local-only and not in git.

## Where things stand

- **Status:**
  - 1,841 / 1,844 functions; 713,440 / 715,968 code bytes (99.65%); 146 / 148 files linked from C.
  - Unchanged since the 2026-10-17 matching round.
  - The GC/2.7 relink passes `decomp/tools/verify_link.py` on every commit. Only the 5 recorded
    `.dtors`/copy-table bytes differ. The built DOL's SHA-256 is
    `eae81eee681311baaa8c2dfe1c262d2d8b8a6c44d471ef8d6929a67818485fd5`.
- **Not matched:** 3 functions in the 2 NonMatching units.
  - `GetCarsIRColor` (visuals): 21/179 words differ.
  - `Blits` and `Blits_First` (ngcZoid): 32/234 and 19/219 words, at the exact size.
  - All three have the original's statement layout (DWARF `.line` table, `tools/lines.py`) and
    DWARF local lists. Only register colouring remains; see "Open work".
- **Git:** `main` on <https://github.com/cabi24/SFRush2049-mat3-decomp> (private), pushed
  2026-10-26. Since the matching round:
  - `3207e28` (2026-10-19): the annotation pilot. `decomp/include/rush_flags.h`, plus header blocks
    and `PORT:` notes in `mdrive.c` and `resurrect.c`; DOL hash unchanged.
  - `ac161d6` (2026-10-26): `.gitignore` now covers `vanila/` and `DATA_GDF_contents.csv`; the
    CSV is untracked but kept on disk.
  - `ngcblit-match` and `review-sprint-20261017` are older branches.
  - Codex used to push its own waves to GitHub, so **`git fetch` first**: GitHub may be ahead of
    disk.
  - Repo-local git identity: `cabi24 <cabi24@gmail.com>`.
- **The 2026-10-17 round**, commits `8e42d11..1a625c0`:
  - Promoted playerscr, carscr, controlscr, pakscr, drivetra, ngcTexture, ngcHelper, recordscr,
    maxpath, game, ngcAudio, resurrect, gameoverscr, mdrive, drones, trackscr, videoscr, smoke,
    hud, targets and stree.
  - Added one `block_relocations` entry to `config/2049LE/config.yml` (commit `9957440`). dtk had
    read a Bink LogoData word as a pointer into `CylinderInSurf`, which forced that static to be
    global and renamed it.
  - Reformatted the minified units, commented every matching hack in place and named the 40
    most-used anonymous types, all under the unchanged DOL hash (writeup "Readability pass").
  - `ContinueCircuit` matched (writeup: pointer locals naming hoisted `&a[i].field` addresses).
- **Stale published files:** `progress.json`, `docs/REVIEW.md`, `docs/file-allowlist.json`,
  `docs/provenance/` and `research/` still describe the 2026-10-16 snapshot (60%). Every partial
  body listed in `research/` has matched since. `tools/audit_publication.py` fails on the stale
  allowlist hashes, not on content. See "Open work" item 5.

## Folders on this PC

| Folder | What it is |
|---|---|
| `C:\rush2049-mat3` | This git checkout. The originals beside it are local and git-ignored, never to be committed: `Rush2049.iso`, `rush_d.elf`, `rush_d_*.csv`, `rush_d_dwarf.c`, `rush.exe`/`DATA.GDF` (the PC build), `vanila/` (retail MAT3 USA disc image and extracted files). So are compilers, `decomp/build`, `decomp/orig`, `work/` and `writeup.md`. |
| `C:\rush2049-mat3-review` | Review worktree, branch `review-sprint-20261017`, with junctions for `build/compilers` and `build/tools`. Holds `review/` (local-only: REVIEW-REPORT.md, LESSONS.md, agent notes, patches) and `work/<unit>-rev*/`. |
| `C:\rush2049-pc` | The Windows PC port. A git clone on branch `pc-port`, remote `decomp` = this checkout; not on GitHub. Its own `pc/README.md` explains it. **Never do port work in this checkout.** |
| `C:\rush2049-dreamcast` | The separate Dreamcast decomp. |

`n64-helper.md` is a guide for the agent working on the separate N64 decomp: where to find things
here and how to use this C as a reference. Keep its status figures and paths current when they
change.

Never run `configure.py` without the tool-path flags below: it re-downloads dtk/objdiff into
`build/tools`.

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
that is expected. `verify_link.py` is the real gate, **but only if every object compiled**: ninja relinks
the DOL from stale objects after a compile failure and the hash still matches (2026-10-17 incident). So
grep the full ninja output for any other `FAILED`/`Error` line, and run `build/tools/ninja.exe -n`: it must
list only the CHECK step. Identifier renames must also cover the hand-written headers in
`decomp/include/rush/` (`dolphin.h`), not just the generated ones and `decomp/src`.

## Open work, in suggested order

0. **`GameSource/src/visuals` — `GetCarsIRColor` 21/179 words** (61/62, data exact, layout full).
   Pure register allocation: target (base, base+1, base+2, idx0..3) = (r5, r31, r28, r12, r11, r30,
   r29), ours (r5, r31, r12, r28, r11, r30, r29). Five sessions, ~60,000 objects (notes in
   `C:\rush2049-mat3-review\work\visuals-rev{,3,4,5}\NOTES.md`). Best lead: the original DWARF lists
   `a // r0`, `b // r6`, `hobj // r11` as surviving locals; `a = gTrackNum;` used across blocks
   reproduces `a r0` with no code change, `b`/`hobj` not yet. 
   2026-10-17 evening: the line table and the kept-definition rule (writeup) reconstructed the blend and
   tail exactly and the schedule is now the target's; a red-channel inline helper taking idx0/idx1
   makes the bases callee-saved. Residual: base+2/idx0 swapped (5 of 7 registers right); the helper
   knob is exhausted (`C:\rush2049-mat3-review\work\visuals-rev6\NOTES.md`).
1. **`NGC/ngcZoid` — `Blits`/`Blits_First` 32/234 and 19/219** (17/19). The target's `mr r6,rX / add r6,rY,r6` inside
   the branchless BlitDraw arms is a phi-copy shape; CodeWarrior places phi copies at the
   definitions and never sinks them, every other integer copy is removed before codegen, and no
   compiler 1.0–2.7 produces it. **Found 2026-10-17:** a select whose alternative is an IR-distinct
   but equal twin (`int d[2]; d[0] = ww; d[1] = hh; if (!vflip) { d[0] = w; d[1] = h; }` with
   `ww = b->right - b->left` recomputed) leaves exactly that copy in the arm and both functions at
   the target size (20/219 and 76/234 words). Residual: the copies land lowest-free (r0/r10) where
   the target coalesces them into the argument registers r6/r7; five passes
   (`C:\rush2049-mat3-review\work\ngcZoid-rev2\NOTES.md`) rule out guard spellings, in-place adds, inline wrappers
   and evaluation order; `ngcZoid-rev3` rules out the SDK ClampStick analogy (older compiler), pointer
   locals, late-folding zeros and reassociation; `ngcZoid-rev4` (line table) fixed the statement layout
   (select on the BlitDraw line), Blits' tex double assignment (76 -> 32) and the ww/hh DWARF listing.
   Residual: creation order of the temporaries (target hh r31, ww r30, gWhite r29; ours gWhite first)
   and the else-arm copies in r0/r10 vs r6/r7. Untried: a real outer `while (1)` around the loop.
2. **Tooling:** run `tools/datacheck.py <unit>` (per-unit data-byte check incl. `.sdata2` literal
   order) before `mark.py` in the per-file workflow; the `skel.py` gaps below still stand.
3. **Linker:** only GC 2.7 completes a full link, hence the 5-byte `.dtors` difference; pre-2.7
   linkers hang (writeup "Status" section). Research-grade.
4. **`skel.py` gaps** (each cost a round-trip): (a) nested-block locals flattened into duplicate
   declarations, (b) DWARF definitions with fewer parameters than the header prototype
   (`DC_SendFillRect`), (c) a `static` function whose name is a global array in the shared header
   (`Blits`), (d) function-local statics omitted entirely, (e) the DWARF signature used even when
   `overrides.json` has a prototype, (f) a backslash-n in the original path becomes a newline in the
   header comment, (g) a mid-file file-scope static hoisted to the top (a DWARF line *with an
   address* right before a function is a definition), (h) a function-local static whose address is
   in `.rodata` must be `static const`, (i) function-pointer table initialisers left as `/* TODO */`,
   (j) a `const` global is not `const` in the header until added to `overrides.json`
   `const_variables`.
5. **Refresh the published snapshot files** before the repository is made public:
   - Regenerate `progress.json` from the current objdiff report (`decomp/build/2049LE/report.json`).
   - Update `docs/file-allowlist.json` and `docs/provenance/source-baseline.json`, so that
     `tools/audit_publication.py --git git` passes again.
   - Retire or archive `research/`; everything it lists has matched.
   - The tracked-but-unlisted files were checked by hand on 2026-10-26: no secrets, binaries, private
     paths or embedded data.
   - `DATA_GDF_contents.csv` is untracked but still in the history before `ac161d6`. Purging it
     needs a history rewrite and force push: only worth it for a public release.
6. **Annotation pass** (readability for porting; DOL hash must stay unchanged). The pilot
   (`3207e28`) set the pattern:
   - Named constants in `decomp/include/rush_flags.h`: macros of the same type and value as the
     literal they replace.
   - A header block per file: purpose, frame flow, `PORT:` notes.
   Extend it file by file; `rush_flags.h` explains the matching rule for constants.

## Per-file matching workflow (how the 146 units were done; ~1–2 h each)

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

## CodeWarrior codegen rules (summary; details and more in `writeup.md`, sections dated 2026-10-16/17)

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
- `work/<unit>/BRIEF.md`, `work/<unit>/agent*/NOTES.md`: the agent briefs and findings from the matching
  rounds (local-only). `work/targets/BRIEF.md` is the latest template (per-function literal map from the `.s`,
  type offsets, layout facts, the fresh-shell `cd` note); `work/targets/merge.py` the latest merge script;
  `work/targets/litmap.py` maps every `@NNN` literal to the function that references it; `agentA/qd.py`
  is the fastest per-function diff (GC 2.6 compile + dtk disassembly against the target `.s`).
- `decomp/docs/verification-baseline.md`: Codex's verification rules (the exact 5-byte gate), still
  current. `docs/REVIEW.md` and `research/README.md`: the 2026-10-16 snapshot review and its
  remaining-work list, now historical (open work item 5).
- `decomp/include/rush/overrides.json`: qualifiers, prototypes, externs and **`type_names`** (chosen names
  for anonymous DWARF types, keyed by the synthetic `anon_<m1>_<m2>_<size>` name; 40 named so far, 318
  references to rarer ones remain). Regenerate headers with `python -B tools/dwarftypes.py rush_d.elf
  decomp/include/rush` after editing it, then rename the references in `decomp/src` the same way.
- `decomp/.clang-format`: the formatting used for reformatted units (whitespace-only; verify token
  streams and restore CRLF afterwards). `pip install clang-format` provides the binary.
- `decomp/tools/`: `skel.py`, `check.py`, `cwmatch.py`, `cwmatch_addr.py` (address-selected cwmatch
  over the ledger, rejects same-named statics), `datacheck.py` (target ELF vs built object data bytes,
  relocations masked: catches `.sdata2` literal order and data order that objdiff misses), `reorder.py`
  (restore original function order, prototypes, attach data per DWARF), `litorder.py` / `sbssorder.py`
  (print a TU's `.sdata2` pool / `.sbss`-`.bss`-`.data` symbol order in seconds), `candidate.py`,
  `fvar.py`, `fdiff.py`, `fields.py`, `dolbytes.py`, `doldiff.py`, `mark.py`, `verify_link.py`, `status.py`.
