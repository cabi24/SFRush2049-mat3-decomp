# San Francisco Rush 2049 — MAT3 decompilation

Source reconstruction of the GameCube version shipped in **Midway Arcade Treasures 3**, targeting the symbol-rich `rush_d` debug executable (`2049LE`).

The independently reviewed wave26 snapshot contains **1,441 / 1,844 functions** and **432,240 / 715,968 code bytes (60.371%)**, with **119 / 148 game translation units configured for C linkage**. These are historical review measurements. Partial function matches do not establish complete unit layout or final linked placement. This import performs no fresh full-game link and claims no new accepted DOL.

This repository contains source, headers, purpose-specific symbol/type metadata, build configuration, verification tools, and curated research notes. Original executables, discs, extracted art, object files, compiler binaries, raw assembly dumps, and private review artifacts are excluded. Font and save-icon initializer payloads are generated locally from your own verified original file.

- [Build and local prerequisites](docs/BUILDING.md)
- [Reviewed state, provenance, and verification limits](docs/REVIEW.md)
- [Research and remaining work](research/README.md)
- [Progress data](progress.json)
- [Publication scan](docs/publication-scan.json) and [file allowlist](docs/file-allowlist.json)
- [Rights and attribution](LICENSES.md)

Run the source-only checks from the repository root:

```sh
python -B tools/audit_publication.py
cd decomp
python -B -m unittest discover -s tools/tests -v
python -B configure.py --help
```

Tests that need original game files or recorded native objects explicitly skip in a clean source-only checkout. See the build guide before generating build files or compiling. There is no active CI workflow or paid service integration.
