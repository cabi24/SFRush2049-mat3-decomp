#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

# Game versions
DEFAULT_VERSION = 0
VERSIONS = [
    "2049LE",  # 0
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--ninja",
    metavar="BINARY",
    type=Path,
    help="path to ninja binary (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--warn",
    dest="warn",
    type=str,
    choices=["all", "off", "error"],
    help="how to handle warnings",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.ninja_path = args.ninja
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-2"
config.compilers_tag = "20251118"
config.dtk_tag = "v1.8.3"
config.objdiff_tag = "v3.6.1"
config.sjiswrap_tag = "v1.2.2"
config.wibo_tag = "1.0.3"

# Project
config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I build/{config.version}/include",
    f"--defsym BUILD_VERSION={version_num}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
]
if args.debug:
    config.ldflags.append("-g")  # Or -gdwarf-2 for Wii linkers
if args.map:
    config.ldflags.append("-mapunused")
    # config.ldflags.append("-listclosure") # For Wii linkers

# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = []

# Optional numeric ID for decomp.me preset
# Can be overridden in libraries or objects
config.scratch_preset_id = None

# Base flags, common to most GC/Wii games.
# Generally leave untouched, with overrides added below.
cflags_base = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    "-O4,p",
    "-inline auto",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-fp_contract on",
    "-str reuse",
    "-multibyte",  # For Wii compilers, replace with `-enc SJIS`
    "-i include",
    f"-i build/{config.version}/include",
    f"-DBUILD_VERSION={version_num}",
    f"-DVERSION_{config.version}",
]

# Debug flags
if args.debug:
    # Or -sym dwarf-2 for Wii compilers
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

# Warning flags
if args.warn == "all":
    cflags_base.append("-W all")
elif args.warn == "off":
    cflags_base.append("-W off")
elif args.warn == "error":
    cflags_base.append("-W error")

# Metrowerks library flags
cflags_runtime = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-common off",
    "-inline auto",
]

# REL flags
cflags_rel = [
    *cflags_base,
    "-sdata 0",
    "-sdata2 0",
]

# Rush 2049 game code: matches GC/2.0-2.7 output with these flags (2.6 chosen: MetroTRK v2.6,
# and the 2.7 linker flags the game's runtime objects as older than its own).
cflags_game = [
    *[f for f in cflags_base if f not in ("-str reuse", "-inline auto")],
    "-inline on",  # only explicit `inline` functions are inlined (HAL_time.c's static HAL_InitTime is called)
    "-use_lmw_stmw on",  # stmw/lmw register saves (bsearch, _ReadDirectory)
    "-str reuse,readonly",  # string literals in .rodata/.sdata2 (seen in cars.c); 'reuse' not yet confirmed
    "-lang c",
]

config.linker_version = "GC/2.7"  # TODO: 2.6 hangs on this link; see docs in writeup


# Helper function for Dolphin libraries
def DolphinLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": objects,
    }


# Helper function for REL script objects
def Rel(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.3.2",
        "cflags": cflags_rel,
        "progress_category": "game",
        "objects": objects,
    }


Matching = True                   # Object matches and should be linked
NonMatching = False               # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching


# Object is only matching for specific versions
def MatchingFor(*versions):
    return config.version in versions


config.warn_missing_config = True
config.warn_missing_source = False
config.libs = [
    {
        "lib": "GameSource",
        "mw_version": "GC/2.6",
        "cflags": cflags_game,
        "progress_category": "game",
        "shift_jis": False,
        "objects": [
            Object(Matching, "GameSource/src/blit_pc.c"),
            Object(Matching, "GameSource/src/boot_pc.c"),
            Object(Matching, "GameSource/src/bsearch.c"),
            Object(Matching, "GameSource/src/camera.c"),
            Object(Matching, "GameSource/src/carload.c"),
            Object(Matching, "GameSource/src/cars.c"),
            Object(Matching, "GameSource/src/checkpoint.c"),
            Object(NonMatching, "GameSource/src/circuit.c"),
            Object(Matching, "GameSource/src/collision.c"),
            Object(Matching, "GameSource/src/controls.c"),
            Object(Matching, "GameSource/src/controlsys.c"),
            Object(Matching, "GameSource/src/countdown.c"),
            Object(Matching, "GameSource/src/demo.c"),
            Object(Matching, "GameSource/src/drivetra.c"),
            Object(Matching, "GameSource/src/drivsym.c"),
            Object(NonMatching, "GameSource/src/drones.c"),
            Object(Matching, "GameSource/src/eggs.c"),
            Object(Matching, "GameSource/src/fake.c"),
            Object(Matching, "GameSource/src/fakegdfs.c"),
            Object(Matching, "GameSource/src/front.c"),
            Object(Matching, "GameSource/src/game.c"),
            Object(Matching, "GameSource/src/gamerecords.c"),
            Object(Matching, "GameSource/src/gfx.c"),
            Object(Matching, "GameSource/src/ghost.c"),
            Object(Matching, "GameSource/src/HAL_Support.c"),
            Object(Matching, "GameSource/src/HAL_text.c"),
            Object(Matching, "GameSource/src/HAL_time.c"),
            Object(Matching, "GameSource/src/headlights.c"),
            Object(NonMatching, "GameSource/src/hud.c"),
            Object(Matching, "GameSource/src/hudextra.c"),
            Object(Matching, "GameSource/src/Init.c"),
            Object(Matching, "GameSource/src/initiali.c"),
            Object(Matching, "GameSource/src/ll.c"),
            Object(Matching, "GameSource/src/maxpath.c"),
            Object(Matching, "GameSource/src/mb_wad.c"),
            Object(Matching, "GameSource/src/mbox.c"),
            Object(NonMatching, "GameSource/src/mdrive.c"),
            Object(Matching, "GameSource/src/memsys.c"),
            Object(Matching, "GameSource/src/model.c"),
            Object(Matching, "GameSource/src/os.c"),
            Object(Matching, "GameSource/src/modf.c"),
            Object(Matching, "GameSource/src/options.c"),
            Object(Matching, "GameSource/src/pakerr.c"),
            Object(Matching, "GameSource/src/paksysdc.c"),
            Object(Matching, "GameSource/src/pane.c"),
            Object(Matching, "GameSource/src/password.c"),
            Object(Matching, "GameSource/src/pathcommon.c"),
            Object(Matching, "GameSource/src/powerups.c"),
            Object(Matching, "GameSource/src/rekon.c"),
            Object(NonMatching, "GameSource/src/resurrect.c"),
            Object(Matching, "GameSource/src/road.c"),
            Object(Matching, "GameSource/src/save.c"),
            Object(Matching, "GameSource/src/sched.c"),
            Object(Matching, "GameSource/src/selectcarload.c"),
            Object(Matching, "GameSource/src/shots.c"),
            Object(Matching, "GameSource/src/skids.c"),
            Object(Matching, "GameSource/src/sky.c"),
            Object(NonMatching, "GameSource/src/smoke.c"),
            Object(Matching, "GameSource/src/state.c"),
            Object(Matching, "GameSource/src/statll.c"),
            Object(Matching, "GameSource/src/streaks.c"),
            Object(NonMatching, "GameSource/src/stree.c"),
            Object(Matching, "GameSource/src/stunt.c"),
            Object(Matching, "GameSource/src/tan.c"),
            Object(Matching, "GameSource/src/targetpaths.c"),
            Object(NonMatching, "GameSource/src/targets.c"),
            Object(Matching, "GameSource/src/TB_text.c"),
            Object(Matching, "GameSource/src/TB_Vector.c"),
            Object(Matching, "GameSource/src/text.c"),
            Object(Matching, "GameSource/src/texture.c"),
            Object(Matching, "GameSource/src/textureanim.c"),
            Object(Matching, "GameSource/src/timer.c"),
            Object(Matching, "GameSource/src/timingbar.c"),
            Object(Matching, "GameSource/src/tires.c"),
            Object(Matching, "GameSource/src/tracks.c"),
            Object(Matching, "GameSource/src/unzip.c"),
            Object(NonMatching, "GameSource/src/visuals.c"),
            Object(Matching, "GameSource/src/waveinfo.c"),
            Object(Matching, "GameSource/src/waves.c"),
            Object(Matching, "GameSource/src/weapons.c"),
            Object(Matching, "GameSource/src/world.c"),
            Object(Matching, "GameSource/src/worlddam.c"),
            Object(Matching, "GameSource/src/zip.c"),
            Object(Matching, "GameSource/src/zoid.c"),
            Object(Matching, "GameSource/src/zoidblit.c"),
            Object(Matching, "GameSource/src/zoidlight.c"),
            Object(Matching, "GameSource/src/zoidpoly.c"),
            Object(Matching, "GameSource/src/zoidtexture.c"),
            Object(Matching, "GameSource/src/zoidwindow.c"),
            Object(Matching, "GameSource/src/stats.c"),
            Object(NonMatching, "GameSource/src/videoscr.c"),
            Object(Matching, "GameSource/src/audioscr.c"),
            Object(Matching, "GameSource/src/cheatscr.c"),
            Object(Matching, "GameSource/src/circuitscr.c"),
            Object(Matching, "GameSource/src/controlscr.c"),
            Object(NonMatching, "GameSource/src/gameoverscr.c"),
            Object(Matching, "GameSource/src/highscorescr.c"),
            Object(Matching, "GameSource/src/licensescr.c"),
            Object(Matching, "GameSource/src/modescr.c"),
            Object(Matching, "GameSource/src/optionscr.c"),
            Object(Matching, "GameSource/src/pakscr.c"),
            Object(Matching, "GameSource/src/pausescr.c"),
            Object(Matching, "GameSource/src/playerscr.c"),
            Object(Matching, "GameSource/src/recordscr.c"),
            Object(Matching, "GameSource/src/startscr.c"),
            Object(Matching, "GameSource/src/titlescr.c"),
            Object(NonMatching, "GameSource/src/trackscr.c"),
            Object(Matching, "GameSource/src/warningscr.c"),
            Object(Matching, "GameSource/src/carscr.c"),
            Object(Matching, "GameSource/src/fmath.c"),
            Object(Matching, "GameSource/src/asincos.c"),
            Object(Matching, "GameSource/src/atan.c"),
            Object(Matching, "GameSource/src/bstree.c"),
            Object(Matching, "GameSource/src/audiosfx.c"),
            Object(Matching, "GameSource/src/audiosfxps2.c"),
            Object(Matching, "GameSource/src/audiosongps2.c"),
            Object(Matching, "GameSource/src/audiosysps2.c"),
            Object(Matching, "GameSource/src/audio.c"),
            Object(Matching, "GameSource/src/audiomgr.c"),
            Object(Matching, "GameSource/src/blit.c"),
        ],
    },
    {
        "lib": "NGC",
        "mw_version": "GC/2.6",
        "cflags": cflags_game,
        "progress_category": "game",
        "shift_jis": False,
        "objects": [
            Object(Matching, "NGC/libgraph.c"),
            Object(Matching, "NGC/stdlib.c"),
            Object(Matching, "NGC/ngcVu1pack.c"),
            Object(Matching, "NGC/ngcDmalist.c"),
            Object(Matching, "NGC/ngcGraph.c"),
            Object(Matching, "NGC/ngcHelper.c"),
            Object(Matching, "NGC/ngcMemcard.c"),
            Object(Matching, "NGC/ngcPad.c"),
            Object(Matching, "NGC/ngcTexture.c"),
            Object(Matching, "NGC/ngcInit.c"),
            Object(Matching, "NGC/ngcFile.c"),
            Object(Matching, "NGC/ngcCamera.c"),
            Object(NonMatching, "NGC/ngcZoid.c"),
            Object(Matching, "NGC/ngcLoadEffect.c"),
            Object(Matching, "NGC/ngcBlit.c"),
            Object(Matching, "NGC/ngcTexHelp.c"),
            Object(Matching, "NGC/ngcMtx.c"),
            Object(NonMatching, "NGC/ngcAudio.c"),
            Object(Matching, "NGC/FONT/defaultFont.cpp", extra_cflags=["-lang c++"]),
            Object(Matching, "NGC/ngcMovie.c"),
            Object(Matching, "NGC/ngcDvdCheck.c"),
            Object(Matching, "NGC/FONT/menuFont.cpp", cflags=[f for f in cflags_game if f != "-lang c"] + ["-lang c++"]),
            Object(Matching, "NGC/FONT/menuFunc.c"),
            Object(Matching, "NGC/ngcBeginMenu.c"),
            Object(Matching, "NGC/BINK/ngcrad3d.c"),
            Object(Matching, "NGC/ngcSaveRegion.c"),
            Object(Matching, "NGC/bonus.c"),
            Object(Matching, "NGC/MATMEM/matmemorycard.c"),
        ],
    },
]


# Optional callback to adjust link order. This can be used to add, remove, or reorder objects.
# This is called once per module, with the module ID and the current link order.
#
# For example, this adds "dummy.c" to the end of the DOL link order if configured with --non-matching.
# "dummy.c" *must* be configured as a Matching (or Equivalent) object in order to be linked.
def link_order_callback(module_id: int, objects: List[str]) -> List[str]:
    # Don't modify the link order for matching builds
    if not config.non_matching:
        return objects
    if module_id == 0:  # DOL
        return objects + ["dummy.c"]
    return objects


# Uncomment to enable the link order callback.
# config.link_order_callback = link_order_callback


# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("game", "Game Code"),
    ProgressCategory("sdk", "SDK Code"),
]
config.progress_each_module = args.verbose
# Optional extra arguments to `objdiff-cli report generate`
config.progress_report_args = [
    # Marks relocations as mismatching if the target value is different
    # Default is "functionRelocDiffs=none", which is most lenient
    "--config functionRelocDiffs=data_value",
]

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress information
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
