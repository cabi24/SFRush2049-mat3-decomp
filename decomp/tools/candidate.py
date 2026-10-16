"""Build and score bounded complete-TU candidates in fresh, private directories.

No Ninja build, canonical source writes, global report, linker, or downloads.
See candidate_README.md. Requires existing local CodeWarrior and objdiff tools.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
import time


VERSION = "2049LE"
MAX_VARIANTS = 16
DIFF_CONFIG = ["functionRelocDiffs=data_value"]


def digest(data):
    return hashlib.sha256(data).hexdigest()


def identity(path):
    return {"path": str(path), "size": path.stat().st_size,
            "sha256": digest(path.read_bytes())}


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")


def inside(path, root):
    return path.resolve().is_relative_to(root.resolve())


def copy_input(source, destination, records):
    if source.is_symlink():
        raise ValueError(f"Symlink input is unsupported: {source}")
    before = source.stat()
    data = source.read_bytes()
    after = source.stat()
    # Detect concurrent changes, including edits preserving mtime.
    if (before.st_size, before.st_mtime_ns) != (after.st_size, after.st_mtime_ns) \
            or data != source.read_bytes():
        raise ValueError(f"Input changed during snapshot: {source}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(data)
    records.append({"origin": str(source), "snapshot": str(destination),
                    "size": len(data), "mtime_ns": after.st_mtime_ns,
                    "sha256": digest(data)})


def snapshot(project, destination, records):
    for directory in ("src", "include", "config", "tools", f"build/{VERSION}/include"):
        source = project / directory
        if not source.is_dir() or source.is_symlink():
            raise ValueError(f"Missing or symlink input directory: {source}")
        for path in sorted(source.rglob("*")):
            if path.is_symlink():
                raise ValueError(f"Symlink input is unsupported: {path}")
            if path.is_file() and "__pycache__" not in path.parts:
                copy_input(path, destination / path.relative_to(project), records)
    for relative in ("configure.py", "objdiff.json", f"build/{VERSION}/config.json",
                     f"orig/{VERSION}/files/rush_d.dol"):
        copy_input(project / relative, destination / relative, records)


def command(argv, cwd, tmp, timeout, record):
    """Record every invocation and reap the child on timeout or Ctrl-C."""
    env = os.environ.copy()
    env.update(TEMP=str(tmp), TMP=str(tmp), TMPDIR=str(tmp),
               PYTHONDONTWRITEBYTECODE="1")
    record.update(argv=[str(arg) for arg in argv], cwd=str(cwd),
                  temp=str(tmp), timeout_seconds=timeout)
    started = time.monotonic()
    try:
        with (cwd / (record["phase"] + ".stdout.log")).open("wb") as stdout, \
                (cwd / (record["phase"] + ".stderr.log")).open("wb") as stderr:
            process = subprocess.Popen(argv, cwd=cwd, env=env,
                                       stdin=subprocess.DEVNULL, stdout=stdout, stderr=stderr)
            try:
                deadline = time.monotonic() + timeout
                while True:
                    remaining = deadline - time.monotonic()
                    if remaining <= 0:
                        raise subprocess.TimeoutExpired(argv, timeout)
                    try:
                        record["returncode"] = process.wait(timeout=min(0.2, remaining))
                        break
                    except subprocess.TimeoutExpired:
                        continue
            except (subprocess.TimeoutExpired, KeyboardInterrupt):
                process.kill()
                process.wait()
                raise
    except subprocess.TimeoutExpired:
        record["status"] = "timeout"
        raise RuntimeError(f"{record['phase']} exceeded {timeout}s")
    except KeyboardInterrupt:
        record["status"] = "interrupted"
        raise
    except OSError as error:
        record["status"] = "failed"
        raise RuntimeError(f"Cannot start {record['phase']}: {error}") from error
    finally:
        record["elapsed_seconds"] = round(time.monotonic() - started, 3)
    record["status"] = "ok" if record["returncode"] == 0 else "failed"
    if record["returncode"]:
        raise RuntimeError(f"{record['phase']} exited {record['returncode']}; see logs in {cwd}")


def compile_profile(build_file, object_path, source_path):
    # Parse only the freshly generated direct MWCC edge. Other rule types need
    # wrappers/postprocessing and must fail, rather than silently approximate.
    text = re.sub(r"\$\r?\n\s*", "", build_file.read_text(encoding="utf-8"))
    lines = text.splitlines()
    target = object_path.replace("/", "\\")
    for index, line in enumerate(lines):
        if not line.startswith("build " + target + ": "):
            continue
        rule, inputs = line.split(": ", 1)[1].split(" ", 1)
        actual_source = inputs.split(" |", 1)[0].strip().replace("\\", "/")
        if rule != "mwcc" or actual_source != source_path:
            raise ValueError(f"Unsupported compile edge: {line}")
        variables = {}
        for following in lines[index + 1:]:
            if not following.startswith("  "):
                break
            key, value = following.strip().split(" = ", 1)
            variables[key] = value
        flags = shlex.split(variables["cflags"])
        if any("$" in flag for flag in flags):
            raise ValueError("Unexpanded compiler profile variable")
        if any(flag in flags for flag in ("-o", "-c", "-MMD")):
            raise ValueError("Compiler flags contain an output or build action")
        if "-nodefaults" not in flags or "-nosyspath" not in flags:
            raise ValueError("Profile must disable external default/system headers")
        return {"rule": rule, "mw_version": variables["mw_version"].replace("\\", "/"),
                "flags": flags, "source": source_path, "object": object_path}
    raise ValueError(f"No direct compile edge for {object_path}")


def dependencies(depfile, cwd):
    text = re.sub(r"\\\r?\n", " ", depfile.read_text(encoding="utf-8"))
    # The target is relative and our file names have no spaces. MWCC depfile
    # prerequisites quote/escape paths containing spaces.
    if ": " not in text:
        raise ValueError(f"Malformed dependency file: {depfile}")
    prerequisites = text.split(": ", 1)[1]
    result = []
    for token in re.findall(r"(?:\\ |[^\s])+", prerequisites):
        path = (cwd / token.replace("\\ ", " ").replace("\\", "/")).resolve()
        if not inside(path, cwd):
            raise ValueError(f"Compiler read an input outside the private candidate: {path}")
        result.append(identity(path))
    if not result:
        raise ValueError("Compiler emitted no dependencies")
    return result


def run(args):
    project = args.project.resolve()
    output = args.output_root.resolve()
    if inside(output, project) or inside(project, output):
        raise ValueError("Output root must be disjoint from the input project")
    output.mkdir(parents=True, exist_ok=True)
    root = Path(tempfile.mkdtemp(prefix="mat3-candidate-", dir=output))
    manifest_path = root / "manifest.json"
    manifest = {"schema": 1, "unit": args.unit, "project": str(project),
                "run_root": str(root), "python": {"executable": sys.executable,
                "version": sys.version}, "status": "preparing", "inputs": [],
                "commands": [], "candidates": [], "objdiff_config": DIFF_CONFIG,
                "strict_verification": "not performed"}
    exit_code = 1
    try:
        frozen = root / "snapshot"
        snapshot(project, frozen, manifest["inputs"])
        tmp = root / "tmp"
        tmp.mkdir()
        # Executables are reused read-only. No generated Ninja rule is executed.
        tool_root = (args.tool_root or project / "build").resolve()
        objdiff = tool_root / "tools/objdiff-cli.exe"
        dtk = tool_root / "tools/dtk.exe"
        ninja = tool_root / "tools/ninja.exe"
        manifest["tools"] = [identity(path) for path in (objdiff, dtk, ninja)]
        existing_settings = json.loads((frozen / "objdiff.json").read_text(encoding="utf-8"))
        configure = [sys.executable, "configure.py", "--no-progress", "--compilers",
                     str(tool_root / "compilers"), "--dtk", str(dtk), "--objdiff", str(objdiff),
                     "--ninja", str(ninja), "--binutils", str(root / "not-installed-binutils"),
                     "--sjiswrap", str(root / "not-installed-sjiswrap.exe")]
        record = {"phase": "configure"}
        manifest["commands"].append(record)
        command(configure, frozen, tmp, args.timeout, record)
        config = json.loads((frozen / "objdiff.json").read_text(encoding="utf-8"))
        units = [unit for unit in config["units"]
                 if unit["name"].removeprefix("rush_d/") == args.unit]
        if len(units) != 1 or "base_path" not in units[0]:
            raise ValueError(f"Unit has no unique configured source: {args.unit}")
        unit = units[0]
        source = unit["metadata"]["source_path"]
        profile = compile_profile(frozen / "build.ninja", unit["base_path"], source)
        manifest["profile"] = profile
        manifest["generated_build"] = identity(frozen / "build.ninja")
        # The profile's include roots must all resolve inside the copied tree.
        for index, flag in enumerate(profile["flags"]):
            if flag == "-i":
                include = frozen / profile["flags"][index + 1]
                if not inside(include, frozen) or not include.is_dir():
                    raise ValueError(f"Unsupported include path: {include}")
        compiler = tool_root / "compilers" / profile["mw_version"] / "mwcceppc.exe"
        compiler_inputs = [identity(path) for path in sorted(compiler.parent.iterdir())
                           if path.is_file()]
        manifest["compiler_inputs"] = compiler_inputs
        target = unit["target_path"]
        copy_input(project / target, frozen / target, manifest["inputs"])
        candidates = args.candidate or [frozen / source]
        for index, candidate in enumerate(candidates):
            work = root / f"candidate-{index:02d}"
            work.mkdir()
            result = {"index": index, "input": str(candidate), "directory": str(work),
                      "status": "preparing", "commands": []}
            manifest["candidates"].append(result)
            write_json(manifest_path, manifest)
            try:
                for relative in ("src", "include", f"build/{VERSION}/include"):
                    shutil.copytree(frozen / relative, work / relative)
                candidate_tmp = work / "tmp"
                candidate_tmp.mkdir()
                copy_input(candidate.resolve(), work / source, result.setdefault("inputs", []))
                result["source"] = identity(work / source)
                destination = work / profile["object"]
                destination.parent.mkdir(parents=True, exist_ok=True)
                copied_target = work / target
                copied_target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(frozen / target, copied_target)
                invocation = [str(compiler), *profile["flags"], "-MMD", "-c", source,
                              "-o", str(Path(profile["object"]).parent)]
                compile_record = {"phase": "compile"}
                result["commands"].append(compile_record)
                command(invocation, work, candidate_tmp, args.timeout, compile_record)
                if not destination.is_file():
                    raise RuntimeError("Successful compiler produced no requested object")
                result["dependencies"] = dependencies(destination.with_suffix(".d"), work)
                result["object"] = identity(destination)
                focused_unit = {"name": unit["name"], "target_path": target,
                                "base_path": profile["object"]}
                if unit.get("symbol_mappings"):
                    focused_unit["symbol_mappings"] = unit["symbol_mappings"]
                focused = {"min_version": config["min_version"], "build_target": False,
                           "build_base": False, "units": [focused_unit]}
                # Keep only supported diff settings; never carry build commands.
                if "diff_settings" in existing_settings:
                    focused["diff_settings"] = existing_settings["diff_settings"]
                write_json(work / "objdiff.json", focused)
                report_record = {"phase": "report"}
                result["commands"].append(report_record)
                command([str(objdiff), "report", "generate", "-p", str(work),
                         "-o", "report.json", "-c", *DIFF_CONFIG],
                        work, candidate_tmp, args.timeout, report_record)
                report = json.loads((work / "report.json").read_text(encoding="utf-8"))
                if len(report["units"]) != 1 or report["units"][0]["name"] != unit["name"]:
                    raise RuntimeError("Focused report did not contain exactly the requested TU")
                result["measures"] = report["units"][0]["measures"]
                if args.function:
                    functions = report["units"][0].get("functions", [])
                    if not any(item["name"] == args.function for item in functions):
                        raise ValueError(f"Function missing from report: {args.function}")
                    diff_record = {"phase": "alignment"}
                    result["commands"].append(diff_record)
                    command([str(objdiff), "diff", "-p", str(work), "-u", unit["name"],
                             "-o", "alignment.json", "-c", *DIFF_CONFIG, args.function],
                            work, candidate_tmp, args.timeout, diff_record)
                    alignment = json.loads((work / "alignment.json").read_text(encoding="utf-8"))
                    for side in ("left", "right"):
                        if not any(symbol.get("name") == args.function
                                   and symbol.get("kind") == "SYMBOL_FUNCTION"
                                   and symbol.get("instructions")
                                   for symbol in alignment.get(side, {}).get("symbols", [])):
                            raise ValueError(f"Function missing from {side} alignment: {args.function}")
                if compiler_inputs != [identity(Path(item["path"])) for item in compiler_inputs]:
                    raise RuntimeError("Compiler inputs changed during candidate evaluation")
                result["status"] = "ok"
            except (OSError, ValueError, RuntimeError, KeyError) as error:
                result.update(status="failed", error=str(error))
            except KeyboardInterrupt:
                result["status"] = "interrupted"
                raise
            finally:
                write_json(manifest_path, manifest)
            print(f"[{index}] {result['status']}: {work}", flush=True)
        exit_code = int(any(item["status"] != "ok" for item in manifest["candidates"]))
        manifest["status"] = "ok" if exit_code == 0 else "failed"
    except KeyboardInterrupt:
        manifest["status"] = "interrupted"
        exit_code = 130
    except (OSError, ValueError, RuntimeError, KeyError) as error:
        manifest.update(status="failed", error=str(error))
    finally:
        write_json(manifest_path, manifest)
        print(f"Manifest: {manifest_path}", flush=True)
    return exit_code, manifest_path


def parser():
    result = argparse.ArgumentParser(description=__doc__)
    result.add_argument("unit", help="Configured TU without extension, e.g. NGC/FONT/menuFont")
    result.add_argument("--project", type=Path, default=Path(__file__).resolve().parents[1])
    result.add_argument("--tool-root", type=Path, help="Existing build directory holding compilers/tools")
    result.add_argument("--output-root", type=Path, required=True, help="Private writable root outside project")
    result.add_argument("--candidate", type=Path, action="append", help="Complete TU file; repeat for variants")
    result.add_argument("--function", help="Save installed objdiff's aligned instruction diff")
    result.add_argument("--max-variants", type=int, required=True, help="Explicit candidate bound, 1..16")
    result.add_argument("--timeout", type=int, required=True, help="Each command timeout in seconds, 1..300")
    return result


def main(argv=None):
    options = parser()
    args = options.parse_args(argv)
    if not 1 <= args.max_variants <= MAX_VARIANTS or not 1 <= args.timeout <= 300:
        options.error("Bounds: --max-variants 1..16, --timeout 1..300")
    if len(args.candidate or [None]) > args.max_variants:
        options.error("Candidate count exceeds --max-variants")
    if not re.fullmatch(r"[A-Za-z0-9_/-]+", args.unit) or ".." in args.unit or args.unit.startswith("/"):
        options.error("Unit must be a relative configured name without an extension")
    try:
        code, _ = run(args)
        return code
    except (OSError, ValueError) as error:
        options.exit(1, f"candidate: {error}\n")


if __name__ == "__main__":
    sys.exit(main())
