# CLAUDE.md

**Start here: read [HANDOFF.md](HANDOFF.md) first** — current status, branch, resume commands,
open work and the per-file workflow. Then `writeup.md` (local-only knowledge base).

Project: matching decompilation of San Francisco Rush 2049 (GameCube, Midway Arcade Treasures 3),
target `rush_d.dol`, dtk template in `decomp/`, CodeWarrior GC 2.6 `-O4,p -fp_contract on`,
linked with GC 2.7.

Hard rules:
- Never commit original game files, compilers, `decomp/build`, `decomp/orig`, `work/`, or
  `writeup.md` (all git-ignored).
- Never hand-edit generated headers in `decomp/include/rush/`; change `overrides.json` and
  regenerate with `python -B tools/dwarftypes.py rush_d.elf decomp/include/rush`.
- A file is `Matching` only when `check.py` is 100% code+data with full layout **and**
  `tools/verify_link.py` passes (5 baseline `.dtors` bytes only).
- Use `candidate.py`/`cwmatch.py` for experiments; they never touch canonical source.
- Record every codegen lesson in `writeup.md`.
