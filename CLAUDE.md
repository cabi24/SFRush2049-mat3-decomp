# CLAUDE.md

**Start here: read [HANDOFF.md](HANDOFF.md) first.** It has the current status, the folders on this
PC, resume commands, open work and the per-file workflow. Then read `writeup.md`, the local-only
knowledge base.

Project: matching decompilation of *San Francisco Rush 2049* for the GameCube (Midway Arcade
Treasures 3). The target is `rush_d.dol` (game ID `2049LE`). The dtk template is in `decomp/`.
The game code compiles with CodeWarrior GC 2.6 (`-O4,p -fp_contract on`) and is linked with GC 2.7.

Hard rules:
- Never commit original game files, compilers, `decomp/build`, `decomp/orig`, `work/`, `vanila/`,
  `rush_d_*.csv`, `DATA_GDF_contents.csv` or `writeup.md` (all git-ignored).
- Never hand-edit generated headers in `decomp/include/rush/`; change `overrides.json` and
  regenerate with `python -B tools/dwarftypes.py rush_d.elf decomp/include/rush`.
- A file is `Matching` only when `check.py` is 100% code+data with full layout **and**
  `tools/verify_link.py` passes (5 baseline `.dtors` bytes only).
- Use `candidate.py`/`cwmatch.py` for experiments; they never touch canonical source.
- Record every codegen lesson in `writeup.md`.
- The PC port lives in its own checkout, `C:\rush2049-pc` (branch `pc-port`). Never do port work
  here.
