#!/usr/bin/env python3
#
# Copyright (c) 2025 Analog Devices Inc.
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
Scopy Plugin Test Runner

Runs JS automated tests for a given plugin, optionally starting iio-emu
when no device IP is provided.

Usage:
    python3 js/run_tests.py ad9371                       # emu mode (auto-start iio-emu)
    python3 js/run_tests.py ad9371 ip:192.168.2.1        # hardware mode (no emu)
    python3 js/run_tests.py ad9371 --scopy-bin ./build/scopy
    python3 js/run_tests.py ad9371 ip:10.0.0.5 --timeout 180
"""

import argparse
import json
import os
import select
import socket
import subprocess
import sys
import time


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.normpath(os.path.join(SCRIPT_DIR, ".."))
EMU_PORT = 30431
EMU_READY_TIMEOUT = 10
EMU_KILL_TIMEOUT = 3


def resolve_paths(plugin_name, scopy_bin, emu_xml, js_file):
    """Resolve all paths relative to the repo root."""
    paths = {}

    # JS test script
    if js_file:
        if os.path.isabs(js_file) or os.sep in js_file or "/" in js_file:
            paths["js_script"] = js_file
        else:
            paths["js_script"] = os.path.join(
                SCRIPT_DIR, "testAutomations", plugin_name, js_file
            )
    else:
        paths["js_script"] = os.path.join(
            SCRIPT_DIR, "testAutomations", plugin_name, plugin_name + "DocTests.js"
        )

    # Scopy binary
    if scopy_bin:
        paths["scopy_bin"] = scopy_bin
    else:
        paths["scopy_bin"] = os.path.join(REPO_ROOT, "build", "scopy")

    # Emu XML: read from emu_setup.json or fall back to <plugin>.xml
    if emu_xml:
        paths["emu_xml"] = emu_xml
    else:
        emu_dir = os.path.join(REPO_ROOT, "packages", plugin_name, "emu-xml")
        setup_json = os.path.join(emu_dir, "emu_setup.json")
        if os.path.isfile(setup_json):
            with open(setup_json, "r") as f:
                setup = json.load(f)
            if setup and isinstance(setup, list) and "xml_path" in setup[0]:
                paths["emu_xml"] = os.path.join(emu_dir, setup[0]["xml_path"])
            else:
                paths["emu_xml"] = os.path.join(emu_dir, plugin_name + ".xml")
        else:
            paths["emu_xml"] = os.path.join(emu_dir, plugin_name + ".xml")

    return paths


def wait_for_port(host, port, timeout):
    """Poll until a TCP port is accepting connections."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            with socket.create_connection((host, port), timeout=1):
                return True
        except (ConnectionRefusedError, OSError):
            time.sleep(0.5)
    return False


def parse_args():
    parser = argparse.ArgumentParser(
        description="Scopy Plugin Test Runner",
        usage="%(prog)s <plugin> [device_uri] [options]",
    )
    parser.add_argument(
        "plugin",
        help="Plugin/package name (e.g., ad9371, ad936x, adrv9002)",
    )
    parser.add_argument(
        "device_uri",
        nargs="?",
        default=None,
        help="Device URI (e.g., ip:192.168.2.1). If omitted, starts iio-emu automatically.",
    )
    parser.add_argument(
        "--scopy-bin",
        default=None,
        help="Path to the built scopy binary (default: build/scopy)",
    )
    parser.add_argument(
        "--emu-xml",
        default=None,
        help="Path to the iio-emu XML file (default: auto-discovered from packages/<plugin>/emu-xml/)",
    )
    parser.add_argument(
        "--js-file",
        default=None,
        help="JS test file name or path. If just a name, looked up in js/testAutomations/<plugin>/. Default: <plugin>DocTests.js",
    )
    parser.add_argument(
        "--timeout",
        type=int,
        default=120,
        help="Max seconds before killing a hung test run (default: 120)",
    )
    return parser.parse_args()


def start_emulator(emu_xml):
    """Start iio-emu as a background process. Returns the Popen object."""
    print(f"[runner] Starting iio-emu with: {emu_xml}")
    try:
        proc = subprocess.Popen(
            ["iio-emu", "generic", emu_xml],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
        )
    except FileNotFoundError:
        print("[runner] ERROR: 'iio-emu' not found on PATH")
        sys.exit(1)

    # Check it didn't immediately crash
    time.sleep(0.5)
    if proc.poll() is not None:
        stderr_output = proc.stderr.read().decode(errors="replace").strip()
        print("[runner] ERROR: iio-emu exited immediately")
        if stderr_output:
            print(f"[runner]   stderr: {stderr_output}")
        sys.exit(1)

    print(f"[runner] iio-emu started (PID {proc.pid})")
    return proc


def kill_emulator(proc):
    """Gracefully terminate the emulator, force-kill if needed."""
    if proc is None or proc.poll() is not None:
        return

    print(f"[runner] Stopping iio-emu (PID {proc.pid})...")
    proc.terminate()
    try:
        proc.wait(timeout=EMU_KILL_TIMEOUT)
        print("[runner] iio-emu terminated gracefully")
    except subprocess.TimeoutExpired:
        print("[runner] iio-emu did not stop, sending SIGKILL...")
        proc.kill()
        proc.wait()
        print("[runner] iio-emu killed")


def run_scopy_tests(scopy_bin, js_script, timeout):
    """
    Run scopy with the JS test script.
    Scopy must be launched from its build directory.
    Returns (stdout_lines, return_code).
    """
    build_dir = os.path.dirname(os.path.abspath(scopy_bin))
    scopy_name = os.path.basename(scopy_bin)
    js_script_rel = os.path.relpath(os.path.abspath(js_script), build_dir)

    cmd = [os.path.join(".", scopy_name), "--script", js_script_rel]
    print(f"[runner] Launching from: {build_dir}")
    print(f"[runner] Command: {' '.join(cmd)}")
    print(f"[runner] Timeout: {timeout}s")
    print("=" * 60)

    lines = []
    try:
        proc = subprocess.Popen(
            cmd,
            cwd=build_dir,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )

        IDLE_TIMEOUT = 120  # Kill scopy if no output for 2 minutes
        last_output_time = time.time()
        deadline = time.time() + timeout

        while True:
            now = time.time()

            # Check overall timeout
            if now > deadline:
                print("\n[runner] ERROR: Test run timed out, killing scopy...")
                proc.kill()
                proc.wait()
                return lines, -1

            # Check idle timeout (no output for 2 minutes = scopy is hung)
            if now - last_output_time > IDLE_TIMEOUT:
                print(f"\n[runner] No output for {IDLE_TIMEOUT}s, scopy appears hung. Killing...")
                proc.kill()
                proc.wait()
                return lines, 0

            # Non-blocking read with 1s poll
            ready, _, _ = select.select([proc.stdout], [], [], 1.0)
            if ready:
                line = proc.stdout.readline()
                if line == "" and proc.poll() is not None:
                    break
                if line:
                    line = line.rstrip("\n")
                    print(line)
                    lines.append(line)
                    last_output_time = time.time()
            elif proc.poll() is not None:
                break

        return lines, proc.returncode

    except FileNotFoundError:
        print(f"[runner] ERROR: scopy binary not found at: {scopy_bin}")
        return lines, -1


def parse_results(lines):
    """
    Parse test output for individual results and summary counts.
    Returns (failed_tests, total, passed, failed, skipped).
    """
    failed_tests = []
    total = 0
    passed = 0
    failed = 0
    skipped = 0

    current_uid = None
    for i, line in enumerate(lines):
        stripped = line.strip()

        # Track current test UID
        if stripped.startswith("Running Test UID: "):
            current_uid = stripped.replace("Running Test UID: ", "")

        # Collect failures
        if current_uid and (stripped.startswith("\u2717 FAIL") or stripped.startswith("\u2717 ERROR")):
            reason = ""
            if i + 1 < len(lines) and ("Reason:" in lines[i + 1] or "ERROR:" in lines[i + 1]):
                reason = lines[i + 1].strip()
            failed_tests.append((current_uid, stripped, reason))
            current_uid = None

        if stripped.startswith("\u2713 PASS"):
            current_uid = None

        if stripped.startswith("\u2298 SKIPPED"):
            current_uid = None

        # Parse summary counts
        if stripped.startswith("Total Tests:"):
            try:
                total = int(stripped.split(":")[1].strip())
            except (ValueError, IndexError):
                pass
        elif stripped.startswith("Passed:"):
            try:
                passed = int(stripped.split(":")[1].strip().split()[0])
            except (ValueError, IndexError):
                pass
        elif stripped.startswith("Failed:"):
            try:
                failed = int(stripped.split(":")[1].strip())
            except (ValueError, IndexError):
                pass
        elif stripped.startswith("Skipped:"):
            try:
                skipped = int(stripped.split(":")[1].strip())
            except (ValueError, IndexError):
                pass

    return failed_tests, total, passed, failed, skipped


def print_summary(failed_tests, total, passed, failed, skipped):
    """Print the final test summary with failed test details."""
    print()

    if failed_tests:
        print("=" * 45)
        print("  FAILED TESTS")
        print("=" * 45)
        for uid, status, reason in failed_tests:
            print(f"  {uid} -- {status}")
            if reason:
                print(f"    {reason}")
        print("=" * 45)
        print(f"  {failed} of {total} tests FAILED")
        if skipped > 0:
            print(f"  ({skipped} skipped)")
        print("=" * 45)
    elif total > 0:
        print("=" * 45)
        print("  ALL TESTS PASSED")
        print(f"  {passed} of {total} tests passed")
        if skipped > 0:
            print(f"  ({skipped} skipped)")
        print("=" * 45)
    else:
        print("[runner] WARNING: Could not parse test results from output")


def main():
    args = parse_args()
    paths = resolve_paths(args.plugin, args.scopy_bin, args.emu_xml, args.js_file)
    use_emu = args.device_uri is None

    # Validate paths
    if not os.path.isfile(paths["scopy_bin"]):
        print(f"[runner] ERROR: scopy binary not found: {paths['scopy_bin']}")
        sys.exit(1)

    if not os.path.isfile(paths["js_script"]):
        print(f"[runner] ERROR: JS test script not found: {paths['js_script']}")
        print(f"[runner] Expected: js/testAutomations/{args.plugin}/{args.plugin}DocTests.js")
        sys.exit(1)

    if use_emu and not os.path.isfile(paths["emu_xml"]):
        print(f"[runner] ERROR: emu XML not found: {paths['emu_xml']}")
        print(f"[runner] Expected: packages/{args.plugin}/emu-xml/{args.plugin}.xml")
        sys.exit(1)

    # Print mode
    if use_emu:
        print(f"[runner] Mode: EMULATOR (iio-emu)")
        print(f"[runner] Plugin: {args.plugin}")
        print(f"[runner] Emu XML: {paths['emu_xml']}")
    else:
        print(f"[runner] Mode: HARDWARE ({args.device_uri})")
        print(f"[runner] Plugin: {args.plugin}")

    print(f"[runner] JS test: {paths['js_script']}")
    print(f"[runner] Scopy: {paths['scopy_bin']}")
    print()

    emu_proc = None
    exit_code = 1

    try:
        if use_emu:
            # Start emulator
            emu_proc = start_emulator(paths["emu_xml"])

            # Wait for emulator to be ready
            print(f"[runner] Waiting for emulator on port {EMU_PORT}...")
            if not wait_for_port("127.0.0.1", EMU_PORT, EMU_READY_TIMEOUT):
                print(f"[runner] ERROR: Emulator not ready after {EMU_READY_TIMEOUT}s")
                sys.exit(1)
            print("[runner] Emulator is ready")
        else:
            print(f"[runner] Skipping emulator (using hardware at {args.device_uri})")

        # Run tests
        lines, scopy_rc = run_scopy_tests(paths["scopy_bin"], paths["js_script"], args.timeout)

        print("=" * 60)

        if scopy_rc == -1:
            print("[runner] ERROR: Test run failed (timeout or crash)")
        elif scopy_rc != 0:
            print(f"[runner] Scopy exited with code {scopy_rc}")

        # Parse and report
        failed_tests, total, passed, failed, skipped = parse_results(lines)
        print_summary(failed_tests, total, passed, failed, skipped)

        if failed == 0 and total > 0:
            exit_code = 0
        else:
            exit_code = 1

    except KeyboardInterrupt:
        print("\n[runner] Interrupted by user")
        exit_code = 1

    finally:
        # Always clean up emulator
        if emu_proc is not None:
            kill_emulator(emu_proc)

    sys.exit(exit_code)


if __name__ == "__main__":
    main()
