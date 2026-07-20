#!/usr/bin/env python3
#
# Copyright (c) 2026 Analog Devices Inc.
#
# This file is part of Scopy
# (see https://www.github.com/analogdevicesinc/scopy).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#
"""
buildinfo.py — Headless, emulator-free capture of Scopy's build-info page.

Launches a freshly built Scopy under QT_QPA_PLATFORM=offscreen, opens the About
page, navigates its embedded browser to qrc:/buildinfo.html, and screenshots it.

Deliberately dumb about libraries: the calling workflow step / self-contained
bundle is responsible for LD_LIBRARY_PATH / QT_PLUGIN_PATH / DYLD_* / PYTHONHOME.
This launcher only forces QT_QPA_PLATFORM=offscreen on top of whatever the caller
set, seeds preferences to suppress first-run / What's-New dialogs, and launches.

Usage:
    python3 tools/screenshots/buildinfo.py --scopy <exe> --output-root <dir>
"""

import argparse
import os
import sys

# Reuse the shared helpers from screenshots.py (same directory).
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from screenshots import configure_scopy_preferences, make_wrapper, run_scopy

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BUILDINFO_JS = os.path.join(SCRIPT_DIR, "buildinfo.js")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scopy", required=True, help="Path to the scopy executable")
    parser.add_argument(
        "--output-root", required=True, help="Directory to write build-info.png into"
    )
    args = parser.parse_args()

    scopy_bin = os.path.abspath(args.scopy)
    if not os.path.exists(scopy_bin):
        print(f"[buildinfo] ERROR: scopy binary not found: {scopy_bin}")
        return 1

    out_dir = os.path.abspath(args.output_root)
    os.makedirs(out_dir, exist_ok=True)

    # Seed preferences (first_run=false, dont_show_whats_new=true, packages_path).
    configure_scopy_preferences(scopy_bin)

    # Wrapper injects scopyOutDir then evaluates buildinfo.js. buildinfo.js only
    # reads scopyOutDir; the other globals make_wrapper injects are harmless.
    wrapper = make_wrapper(uri="", out_dir=out_dir, js_script_path=BUILDINFO_JS)

    # Force offscreen on top of whatever library env the caller/bundle set up.
    env = {**os.environ, "QT_QPA_PLATFORM": "offscreen"}

    print(f"[buildinfo] Capturing build-info to {out_dir}")
    rc = run_scopy(scopy_bin, wrapper, env=env)

    # The screenshot PNG is the success signal, not Scopy's exit code: Scopy is
    # known to crash on shutdown (exit 245) *after* the capture is written, and
    # screenshots.py already treats a nonzero Scopy exit as a warning. So we key
    # success off a valid, non-trivial PNG and only warn on a bad exit code.
    #
    # A blank/black offscreen frame or a truncated write compresses to a tiny
    # PNG, so a minimum-size floor catches those mechanical failures. It does NOT
    # verify the *right* page is shown (build-info vs. plain About) — that is
    # confirmed by visually inspecting the uploaded artifact.
    MIN_PNG_BYTES = 3072  # blank frames are ~1-2 KB; a text page is much larger

    out_png = os.path.join(out_dir, "build-info.png")
    if not os.path.exists(out_png):
        print(f"[buildinfo] ERROR: expected screenshot missing: {out_png}")
        return rc if rc != 0 else 1

    size = os.path.getsize(out_png)
    print(f"[buildinfo] Wrote {out_png} ({size} bytes)")
    if size < MIN_PNG_BYTES:
        print(
            f"[buildinfo] ERROR: screenshot is implausibly small ({size} < "
            f"{MIN_PNG_BYTES} bytes) — likely blank/black or truncated"
        )
        return rc if rc != 0 else 1

    if rc != 0:
        print(
            f"[buildinfo] WARNING: Scopy exited with code {rc} (crash-on-shutdown "
            f"is expected); screenshot captured, treating as success"
        )
    return 0


if __name__ == "__main__":
    sys.exit(main())
