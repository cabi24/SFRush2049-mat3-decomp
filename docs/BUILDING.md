# Building and verifying locally

## Original files

Supply your own lawfully obtained MAT3 GameCube game files and authorized toolchain. This project supplies no game or proprietary-tool downloads and grants no rights to third-party material. The target is `rush_d.dol`, not the disc's boot `main.dol` or the N64 release.

Place the original at `decomp/orig/2049LE/files/rush_d.dol`:

- SHA-1: `c6a4b203cc2446cf71cf524d49f0fb7839bafef1`
- SHA-256: `27eac444d9fee753864ef2978cc8c1999f07eb8d459d0dee60f254c18cd05e67`

Optional symbol-rich `rush_d.elf`, needed by several analysis tools, belongs at the repository root and is ignored. Its SHA-256 is `d4a06e4516ab9a75e6de780beee10ea07b8358016e4964a8baf9e8eaa282908c`; SHA-1 is `81181994c6231ce20886f571525d740e10f2796f`. Locally derived `rush_d_dwarf.c` and `rush_d_*.csv` also belong at the root and are ignored. They are not prerequisites for compiling the checked-in sources. `tools/dwarf1.py`, `tools/dwarftypes.py`, and `tools/extract_syms.py` preserve the existing recovery workflows; inspect their usage before generating files. `ctx.py`/`unit2c.py` additionally require the local DWARF C reconstruction and locally generated split assembly. No raw DWARF or assembly dump is shipped.

From the repository root, regenerate the four omitted art initializer includes:

```sh
python -B tools/bootstrap_local_assets.py
```

The generator verifies the entire original DOL and every selected payload before writing to `decomp/include/generated/`. It refuses to overwrite different local includes. The font wrapper, memory-card code, and save code retain their reviewed types and source bodies. Their payload replacement is documented in `provenance/source-baseline.json`. Generated includes must remain local and ignored.

## Toolchain and build

Use Python 3.10 or later. Obtain decomp-toolkit `v1.8.3`, objdiff-cli `v3.6.1`, Ninja, and any required wrappers from their respective upstream projects under their own terms. Supply an authorized CodeWarrior compiler tree: game code uses `GC/2.6`, the configured linker is `GC/2.7`. The template's proprietary compiler download action is disabled. No binaries or SDK tree are included here.

From `decomp`, using your own actual paths:

```sh
python -B configure.py --compilers /path/to/compilers --dtk /path/to/dtk --objdiff /path/to/objdiff-cli --ninja /path/to/ninja
ninja
```

On Windows use the corresponding `.exe` paths. On supported non-Windows systems the generator exposes `--wrapper` for the user's compatible runner. The private reviewed environment used Windows native tools; portability of a complete clean-clone build has not been established by this publication.

The generator can create download edges for open-source auxiliary tools if paths are omitted. Provide explicit tool paths to avoid those edges. Some local verification helpers retain the established Windows-relative `build/tools/*.exe` layout and placeholder paths for unused binutils/sjiswrap; adjust arguments or arrange your local tools as described by each helper's `--help`. The local `build/` tree is never committed.

Default matching builds link unimplemented/NonMatching units from the original split objects. `--non-matching` changes that behavior and must not be used to claim matching progress. The five partial lanes (`hud`, `resurrect`, `mdrive`, `maxpath`, `stree`) remain NonMatching. The inherited `gamerecords` Matching flag and CylinderInSurf symbol mapping are preserved.

## Verification and scope

```sh
python -B -m unittest discover -s tools/tests -v
python -B tools/verify_link.py --no-build
```

The first command has synthetic verification controls and optional local-original tests. The second needs your locally generated binary and original. The pinned historical GC/2.7 relink has a narrowly specified five-byte destructor-table exception; read [the exact gate and limits](../decomp/docs/verification-baseline.md). It is not an unrestricted mismatch allowance. A historical gate pass does not establish that current sources were freshly compiled or linked.

`candidate.py` can compile a bounded candidate in a new output directory using existing local tools. Publication did not run a matching search. `verify_layout.py`, `check.py`, and `verify_linked_layout.py` have different evidence scopes; consult their usage and the verification baseline. A masked equality screen, register-renaming explanation, or per-function score is insufficient for whole-unit promotion.

The original private graph proofs and native receipts are summarized by hashes and review results here. Their original binaries, toolchains, exhaustive payload graphs, and thousands of control artifacts are withheld. This source-only checkout cannot independently replay the entire private wave26 proof packet without those separately retained local prerequisites.
