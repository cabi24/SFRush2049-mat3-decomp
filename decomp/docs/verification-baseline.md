# Trusted GC/2.7 verification exception

This is a pinned exception for the recorded split/relink experiment, not a way
to accept any five mismatching bytes. Changing the constants requires a separate
review of new evidence; the verifier has no baseline-learning option.

Evidence checked on BURNES_STUDIO, 2026-10-14:

- `../../writeup.md`, Status / split-and-relink round-trip (line 218), records
  the all-split GC/2.7 link and its five-byte destructor-table defect. Later
  completed source batches record the same full-DOL baseline.
- The preserved `build/2049LE/link_2.7.dol` (2026-10-12) and accepted
  `build/2049LE/rush_d.dol` (2026-10-13) have identical SHA-256:
  `eae81eee681311baaa8c2dfe1c262d2d8b8a6c44d471ef8d6929a67818485fd5`.
- The trusted `orig/2049LE/files/rush_d.dol` has SHA-256:
  `27eac444d9fee753864ef2978cc8c1999f07eb8d459d0dee60f254c18cd05e67`.
- All eight initialized target sections match the recovered `../../rush_d.elf`
  byte for byte at their recorded virtual addresses. That ELF's SHA-1 is
  `81181994c6231ce20886f571525d740e10f2796f`, as recorded in the writeup.

The exact loaded-byte signature is:

| Segment | Virtual address | Target | GC/2.7 build | Meaning |
| --- | --- | --- | --- | --- |
| T0 | 0x80005573 | 08 | 04 | `.dtors` length in `_rom_copy_info` |
| D1 | 0x801086A0 | 80 | 00 | `__destroy_global_chain` pointer byte 0 |
| D1 | 0x801086A1 | 0E | 00 | pointer byte 1 |
| D1 | 0x801086A2 | 19 | 00 | pointer byte 2 |
| D1 | 0x801086A3 | B8 | 00 | pointer byte 3 |

The fifth byte is in the initialization copy table, not inside `.dtors` itself.
The table entry is `[0x801086A0, 0x801086A0, 0x00000008]` in the target and has
length four in the GC/2.7 link. The original `.dtors` payload is
`800e19b800000000`; GC/2.7 emits a zero word. Both DOLs retain the same padded
32-byte D1 segment. This explains the five-byte count without blessing unrelated
differences. The verifier also requires identical segment sets, virtual addresses,
loaded sizes, BSS and entry, and rejects malformed/truncated/overlapping extents.
Unloaded file padding and file offsets are not claimed to be byte-identical.

## Verification commands and limits

From `decomp`:

```
python -B -m unittest discover -s tools/tests -v
python -B tools/verify_link.py --no-build
python -B tools/verify_link.py --no-build --built build/2049LE/link_2.7.dol
```

Without `--no-build`, `verify_link.py` retains the configure/Ninja workflow.
Build-path isolation is a separate runner concern. Existing-artifact checks do
not establish that current source has been freshly compiled or relinked.

`check.py` now returns status 2 for missing requested units, missing objects, or
target-symbol section/offset/size/type/extent failures. Function-local numeric
suffixes remain normalized, but collisions retain all records. Extra built
symbols are explicitly unvalidated, and do not prove stripping or linked layout.
Documented erased-helper shifts, including `ngcMemcard`, fail this strict object
check. They are not auto-exempted, and a historical DOL pass cannot clear them.

`cwmatch.py` fixes the ET_EXEC relocation-address handling and compares exact
relative relocation sites, types, named targets and signed addends. A known
signature mismatch fails. Missing or unnamed metadata and unsupported relocation
types can yield only `CANDIDATE (masked equality; relocation identity/metadata
needs evidence; full link required)`. Local-static suffix changes (`sInited$74`
versus `$49`) and literal counter changes (`@112` versus `@71`) also require
evidence: recognizing the counter pattern does not establish equivalent targets.
`MATCH` is function-level instruction/signature evidence,
not proof of final addresses, data layout, source semantics or a full binary match.
Multiple original functions with the same name are rejected as ambiguous.
Original function extraction validates the declared virtual and file extents
before slicing; duplicate original relocation sites are rejected before caching.

## Fresh linked-layout evidence for erased helpers

Use the separate `tools/verify_linked_layout.py` route after the private runner
has produced its objects. It does not change `check.py`'s strict layout result.
An ordered JSON array must list the full current linker object inputs, with the
selected C object included exactly once and its split target object excluded.
Relative input paths resolve against `--project-root`. Lane 2 can export this
array from its private build's link edge; no configure/Ninja invocation is hidden
inside this verifier.

```
python -B tools/verify_linked_layout.py NGC/ngcMemcard --inputs-json ../evidence/fresh-link-inputs.json --output-root ../evidence/fresh-linked
```

For other private runners, pass `--project-root`, `--build-root`, `--linker`, and
`--dtk` explicitly. `--build-root` contains `obj/`, `src/`, and `ldscript.lcf`.
The command uses the existing GC/2.7 linker with the project's `-fp hardware
-nodefaults` flags and existing dtk; it does not fetch tools or compile source.
It copies each input into a newly created evidence directory, hashes the exact
ordered inputs and tools, and directs both outputs and temporary files there.
Reparse-point input paths require separate isolation evidence and are rejected.

This route permits only object function offset shifts to await linked evidence:
data placement, symbol sizes/types, missing symbols and invalid extents still
fail. It requires each selected target function's unique original identity and
fresh linked address, size, section and instruction bytes to agree, requires extra
object helpers to be absent from the linked ELF, and runs the unchanged exact
full-DOL verifier. It accepts no pre-existing ELF/DOL or receipt as a fresh result.
The receipt records `source_compilation_verified: false`; fresh source compilation
must be established by the runner separately.

Objdiff's `functionRelocDiffs=data_value` report setting belongs to runner/build
configuration and must be enabled and independently validated there. This patch
does not change `configure.py` or treat its current lenient report as strict proof.
