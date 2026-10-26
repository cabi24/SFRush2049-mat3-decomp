# Reviewed source state

> **Historical.** This describes the source-only snapshot published and reviewed on 2026-10-16 (60%
> of code). The project reached 99.65% on 2026-10-17; see the [README](../README.md) for the current
> state.

The export uses the sealed cumulative wave26 project as its source, rather than applying a two-file patch directly onto the older canonical checkout. Every selected source/header/config/script and research patch was checked against the private seal before copying. The original checkout and private packages were left unchanged.

The private review seal SHA-256 is `af8e508257fe03d2498a39316c4b72b8f82cbb28264cd12cd70c2cfe9a1a332c`; the delivery-manifest SHA-256 is `8ef574e17bd4300af3a34c644293d5aeba4766d3f74825e8677a099847408849`. Compact derived receipts are in `provenance/`. These fingerprints identify the retained private evidence; they do not make the omitted packet publicly reproducible.

| State | Functions | Code bytes | Configured C-linked units |
| --- | ---: | ---: | ---: |
| Canonical at review | 1,271 / 1,844 | 342,476 / 715,968 | 113 / 148 |
| Sealed cumulative wave25 | 1,438 / 1,844 | 429,912 / 715,968 | 119 / 148 |
| Reviewed cumulative wave26 | 1,441 / 1,844 | 432,240 / 715,968 | 119 / 148 |

Wave26 adds `SetResurrectionData` (1,328 bytes), `ShowHUD` (904), and `ShowCheckpointTime` (96). All 1,438 baseline function report pairs persist. There are no new promotions or symbol mappings. The cumulative snapshot already includes earlier accepted work and prerequisites.

For the five partial lanes the independent review verified 107 exact function bodies / 51,916 bytes, 1,824 text relocation records and 246 nested data records. Those records include original provider identity and payload checks, not just masked instruction equality. HUD's checkpoint proof includes the original credit-name table, strings and lifecycle allocation identities. Its BSS/readonly suffixes and whole-TU placement remain outside that proof. A raw literal-offset change in resurrect is explicitly distinguished from proven final linked placement.

`progress.json` is a compact derivative of the historical native objdiff report (SHA-256 `6d0257df4a540503da7236a1381fcf1a4c14a64d5fccbacfd13f3730218ed599`). It includes game units and function names/sizes/scores, without raw bytes. Its totals were recomputed during export. Configured linkage is a source-configuration measurement; it is not a fresh completed game link. Research patches earn zero credit.

## Export adaptations

`source-baseline.json` records the reviewed and export hashes for all selected sealed files, including any changed file. All source/header bytes and mixed line endings are preserved except for four art initializers in three source files. They become local generated includes. No function body, matching flag, compiler profile or symbol mapping is changed. The compiler downloader is disabled, and candidate-documentation path examples are portable. Git text normalization is disabled so it cannot silently change reviewed byte hashes.

Symbol files contain names, sections, addresses, extents and linkage annotations required to split/link the executable. Type/declaration JSON contains C declarations, types, locations and register annotations used by reconstruction tools. Historical `D:\rush_ngc\Rush_NGC` strings identify original source compile units; they are not paths into the user's private workspace. These files contain no encoded ELF, executable section or asset payload.

The private full writeup, complete manifests containing local paths, object/ELF payload proofs, native binaries and indexed review archives are intentionally not copied. Useful accepted limits and failed experiments are curated in `research/`, with four separately labeled research-only patches. The exact wave26 accepted patch and its two wave25 preimage source files are retained in `provenance/` to preserve its actual baseline.

## Verification recorded for this import

Publication checks verify the exact allowlisted Git tree, UTF-8/text policy, excluded extensions and directories, secret indicators, embedded payload indicators, reviewed source hashes, original patch preimages, and progress totals. The local report records actual commands and outcomes. No CI watcher is added. A fresh clean-clone native compile or full-game link needs user-provided original files and toolchains and is not claimed here.
