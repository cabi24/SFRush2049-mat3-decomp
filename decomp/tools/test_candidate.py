"""Opt-in integration checks using existing MAT3 compilers, never canonical writes."""
import _thread
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
import os
from pathlib import Path
import shutil
import sys
import tempfile
import threading
from unittest.mock import patch

import candidate


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project", type=Path, required=True)
    parser.add_argument("--output-root", type=Path, required=True)
    parser.add_argument("--tool-root", type=Path)
    args = parser.parse_args()
    original = args.project.resolve()
    output = args.output_root.resolve()
    if candidate.inside(output, original) or candidate.inside(original, output):
        parser.error("Test outputs must be disjoint from canonical project")
    output.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix="candidate-tests-", dir=output))
    project = root / "input-project"
    results = {"tests": [], "project": str(project), "run_root": str(root)}
    units = ("GameSource/src/bsearch", "GameSource/src/eggs", "NGC/FONT/menuFont")
    watched = [original / "configure.py", original / "build.ninja", original / "objdiff.json",
               original / "build/2049LE/report.json", original / ".ninja_log", original / ".ninja_deps"]
    watched += list((original / "src").rglob("*")) + list((original / "include").rglob("*"))
    watched += list((original / "build/2049LE/src").rglob("*.o"))
    before = [candidate.identity(path) for path in sorted(watched) if path.is_file()]
    try:
        candidate.snapshot(original, project, [])
        for unit in units:
            target = f"build/2049LE/obj/{unit}.o"
            candidate.copy_input(original / target, project / target, [])

        def check(name, condition, evidence):
            results["tests"].append({"name": name, "passed": bool(condition), "evidence": evidence})
            candidate.write_json(root / "test-results.json", results)
            if not condition:
                raise AssertionError(name)

        def evaluate(unit, sources=None, function=None, timeout=30):
            options = candidate.parser().parse_args([
                unit, "--project", str(project), "--tool-root",
                str((args.tool_root or original / "build").resolve()),
                "--output-root", str(root / "runs"), "--max-variants", "3", "--timeout", str(timeout),
                *(["--function", function] if function else []),
                *[argument for source in sources or [] for argument in ("--candidate", str(source))]])
            code, path = candidate.run(options)
            report = json.loads(path.read_text(encoding="utf-8"))
            report["manifest_path"] = str(path)
            return code, report

        # Two unrelated real translation units compile concurrently.
        with ThreadPoolExecutor(max_workers=2) as workers:
            bsearch_future = workers.submit(evaluate, units[0], function="bsearch")
            eggs_future = workers.submit(evaluate, units[1], function="InitEgg")
            bsearch_code, bsearch = bsearch_future.result()
            eggs_code, eggs = eggs_future.result()
        check("independent concurrent TUs", bsearch_code == eggs_code == 0
              and bsearch["run_root"] != eggs["run_root"]
              and bsearch["candidates"][0]["measures"]["matched_code_percent"] == 100
              and eggs["candidates"][0]["measures"]["matched_code_percent"] == 100,
              [bsearch["manifest_path"], eggs["manifest_path"]])

        sources = root / "candidates"
        sources.mkdir()
        valid = sources / "bsearch.c"
        shutil.copyfile(project / "src/GameSource/src/bsearch.c", valid)
        bad = sources / "invalid.c"
        bad.write_text("#error intentional isolated candidate failure\n", encoding="utf-8")
        code, failure = evaluate(units[0], [bad, valid], "bsearch")
        check("failure does not block next candidate", code == 1
              and [item["status"] for item in failure["candidates"]] == ["failed", "ok"]
              and failure["candidates"][0]["commands"][0]["returncode"] != 0
              and failure["candidates"][1]["object"]["sha256"] == bsearch["candidates"][0]["object"]["sha256"],
              failure["manifest_path"])
        code, repeated = evaluate(units[0], [valid, valid], "bsearch")
        check("identical candidates repeat exactly", code == 0
              and all(item["object"]["sha256"] == bsearch["candidates"][0]["object"]["sha256"]
                      and item["measures"] == bsearch["candidates"][0]["measures"]
                      for item in repeated["candidates"]), repeated["manifest_path"])

        # Use real headers with a tiny appended probe; preserve source/header
        # timestamps across edits so only content freshness can distinguish them.
        source = project / "src/GameSource/src/bsearch.c"
        initial_source = source.read_bytes()
        header = project / "include/isolation_probe.h"
        header.write_text("#define ISOLATION_FRESH 1\n", encoding="utf-8")
        source.write_bytes(initial_source + b'\n#include "isolation_probe.h"\n'
                           b'int isolation_probe(void) { return ISOLATION_FRESH; }\n')
        first_code, first = evaluate(units[0])
        timestamp = header.stat()
        header.write_text("#define ISOLATION_FRESH 2\n", encoding="utf-8")
        os.utime(header, ns=(timestamp.st_atime_ns, timestamp.st_mtime_ns))
        second_code, second = evaluate(units[0])
        get_probe_hash = lambda report: next(item["sha256"] for item in report["candidates"][0]["dependencies"]
                                            if item["path"].endswith("isolation_probe.h"))
        check("header freshness with identical timestamp", first_code == second_code == 0
              and first["candidates"][0]["object"]["sha256"] != second["candidates"][0]["object"]["sha256"]
              and get_probe_hash(first) != get_probe_hash(second),
              [first["manifest_path"], second["manifest_path"]])
        timestamp = source.stat()
        source.write_bytes(source.read_bytes().replace(b"return ISOLATION_FRESH;", b"return ISOLATION_FRESH + 7;"))
        os.utime(source, ns=(timestamp.st_atime_ns, timestamp.st_mtime_ns))
        third_code, third = evaluate(units[0])
        check("source freshness with identical timestamp", third_code == 0
              and second["candidates"][0]["object"]["sha256"] != third["candidates"][0]["object"]["sha256"],
              third["manifest_path"])
        source.write_bytes(initial_source)

        cpp_code, cpp = evaluate(units[2])
        languages = [cpp["profile"]["flags"][index + 1]
                     for index, flag in enumerate(cpp["profile"]["flags"]) if flag == "-lang"]
        check("actual per-unit C++ profile", cpp_code == 0 and languages == ["c++"]
              and cpp["candidates"][0]["measures"]["matched_code_percent"] == 100,
              cpp["manifest_path"])

        # Deterministically interrupt a running variant child. This substitutes a
        # private sleeping Python child at the compile boundary only for the test.
        real_command = candidate.command
        children = []
        real_popen = candidate.subprocess.Popen

        def track_child(*arguments, **keywords):
            child = real_popen(*arguments, **keywords)
            children.append(child)
            return child

        def interrupt_compile(argv, cwd, tmp, timeout, record):
            if record["phase"] != "compile":
                return real_command(argv, cwd, tmp, timeout, record)
            timer = threading.Timer(0.3, _thread.interrupt_main)
            timer.start()
            try:
                return real_command([sys.executable, "-c", "import time; time.sleep(30)"],
                                    cwd, tmp, timeout, record)
            finally:
                timer.cancel()

        private_before = candidate.identity(source)
        with patch.object(candidate, "command", interrupt_compile), \
                patch.object(candidate.subprocess, "Popen", track_child):
            interrupt_code, interrupted = evaluate(units[0], [valid, valid])
        check("interrupted variant is reaped and leaves source untouched", interrupt_code == 130
              and interrupted["status"] == "interrupted"
              and len(interrupted["candidates"]) == 1
              and all(child.poll() is not None for child in children)
              and children[-1].returncode != 0
              and interrupted["candidates"][0]["commands"][0]["elapsed_seconds"] < 5
              and candidate.identity(source) == private_before,
              interrupted["manifest_path"])

        def timeout_compile(argv, cwd, tmp, timeout, record):
            if record["phase"] == "compile":
                argv = [sys.executable, "-c", "import time; time.sleep(30)"]
                timeout = 1
            return real_command(argv, cwd, tmp, timeout, record)

        with patch.object(candidate, "command", timeout_compile):
            timeout_code, timed_out = evaluate(units[0])
        check("timeout returns failure", timeout_code == 1
              and timed_out["candidates"][0]["commands"][0]["status"] == "timeout",
              timed_out["manifest_path"])
        missing_code, missing = evaluate(units[0], function="function_that_does_not_exist")
        check("missing function returns failure", missing_code == 1
              and "Function missing" in missing["candidates"][0]["error"], missing["manifest_path"])
        no_body = sources / "no-body.c"
        no_body.write_text('#include "rush/rush_protos.h"\n', encoding="utf-8")
        missing_body_code, missing_body = evaluate(units[0], [no_body], "bsearch")
        check("missing compiled function returns failure", missing_body_code == 1
              and missing_body["candidates"][0]["status"] == "failed", missing_body["manifest_path"])

        after = [candidate.identity(Path(item["path"])) for item in before]
        check("canonical sources, headers, objects and build reports unchanged", before == after,
              {"watched_files": len(before), "before": before, "after": after})
        results["status"] = "passed"
    except Exception as error:
        results.update(status="failed", error=str(error))
        raise
    finally:
        candidate.write_json(root / "test-results.json", results)
        print(f"Test evidence: {root / 'test-results.json'}", flush=True)
    print(f"PASS: {len(results['tests'])} integration checks")


if __name__ == "__main__":
    main()
