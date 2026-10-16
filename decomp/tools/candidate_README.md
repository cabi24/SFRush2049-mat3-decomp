# Isolated MAT3 TU candidates

`candidate.py` compiles complete C/C++ translation units in fresh private directories.
It copies source, headers and build inputs, runs the copied `configure.py`, then
uses that TU's actual compiler version and flags. Existing compilers/tools are
read-only inputs. It executes no Ninja edges, download rules or linker.

From `decomp`, use a writable output root outside the input project:

```powershell
python -B tools/candidate.py GameSource/src/eggs --output-root ../local-runs --max-variants 1 --timeout 60 --function InitEgg
python -B tools/candidate.py NGC/FONT/menuFont --output-root ../local-runs --max-variants 2 --timeout 60 --candidate ../candidate-a.cpp --candidate ../candidate-b.cpp
```

Candidate files must contain the entire real TU, including data and other
functions. They are copied to the TU's configured source path and compiled with
the real headers; their external filename does not select the compiler language.
Without `--candidate`, the frozen source is compiled. Make replacements in your
private candidate file, then pass it here. The companion `fvar.py` wrapper safely
creates text replacements in private files and includes the original as a control:

```powershell
python -B tools/fvar.py GameSource/src/bsearch bsearch "nmemb >>= 1;" "nmemb >>= 1; /* trial */" --output-root ../local-runs --max-variants 2 --timeout 60
```

The old mutable `variants.py` and `permute.py` workflows are superseded for
concurrent experiments. No existing caller should wrap their canonical writes
around the new runner.

Both bounds are required: at most 16 candidates and 1–300 seconds per command.
Each invocation and each candidate gets its own source/build/report/temp paths.
Every candidate rebuilds from scratch; source/header timestamps cannot select an
old object. A failed candidate does not stop subsequent candidates. Exit status
is 1 if any candidate fails, and 130 on interruption; running children are reaped.
No restore operation can overwrite canonical source because it was never changed.

`manifest.json` records source/build-input/header/target/tool SHA-256 identities,
the copied compiler profile, commands, working directories, temp paths, timeouts,
exit statuses and object identities. Actual included files come from the new
MWCC depfile and must resolve inside the candidate directory. Reports, depfiles,
stdout/stderr logs and optional `alignment.json` remain beside each candidate.
The alignment is the installed objdiff's supported instruction alignment.

Reports contain exactly one TU and use `functionRelocDiffs=data_value`. This is
screening: a referenced literal mutation changes the code score, while some
named relocation addend mutations still score 100%. Objdiff's
[relocation comparison](https://github.com/encounter/objdiff/blob/v3.8.2/objdiff-core/src/diff/code.rs)
uses data values in this mode. Lane 1's exact relocation/layout checks and final
`verify_link.py` remain necessary before promotion. The runner does not assert
strict verification or full-DOL equality.

The separable `check.py` plumbing patch changes only CLI routing: focused private
checks are the default and require `--output-root`; `--global-report` explicitly
requests the legacy full build/report. `main()` remains the legacy API so lane 1's
verifier tests continue to exercise it. Focused checks call the unchanged lane-1
`verify_layout.check_paths(target, built)` and return 2 for layout failures.
Documented erased helpers can legitimately shift object functions; a layout
failure is retained for review and must be resolved by a separately verified
full link. No exception is inferred from a 100% fuzzy score.

Apply lane 1's check-layout patch first, then `lane2-check-plumbing.patch`.
The independent `lane2-configure-relocations.patch` enables the same supported
data-value screening for explicitly requested global reports.

Opt-in local validation, with existing tools and no canonical writes:

```powershell
python -B tools/test_candidate.py --project . --output-root ../local-tests
```

Eleven integration checks cover unrelated concurrent TUs, failure continuation,
repeatable objects/scores, timestamp-preserving source/header edits, C++ profile,
interruption/child cleanup, timeout/missing-function failures and unchanged
canonical sources/headers/objects/global reports. `--tool-root` can reuse another
existing build directory's compilers/tools when the input project is a snapshot.
Unsupported wrappers/postprocessing, symlink inputs or external dependencies fail
explicitly. Keep native CodeWarrior output paths reasonably short.
