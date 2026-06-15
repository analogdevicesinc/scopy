#!/usr/bin/env python3
"""
qt6_layout_runner.py — Capture layout screenshots for all emulated devices.

Iterates every packages/*/emu-xml/emu_setup.json entry, starts the emulator,
launches Scopy with qt6_layout_test.js, and saves screenshots per device.

Usage:
    python3 tests/qt6-regression/js/qt6_layout_runner.py --scopy ./build/scopy
    python3 tests/qt6-regression/js/qt6_layout_runner.py --scopy ./build/scopy --output /tmp/qt6-screenshots
"""

import argparse
import json
import os
import socket
import subprocess
import sys
import tempfile
import time

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(SCRIPT_DIR)))

EMU_PORT = 30431
EMU_TIMEOUT = 30
SCOPY_TIMEOUT = 120


def wait_for_port(host, port, timeout):
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            with socket.create_connection((host, port), timeout=1):
                return True
        except (ConnectionRefusedError, OSError):
            time.sleep(0.5)
    return False


def start_emulator(entry, emu_dir):
    emu_type = entry.get("emu-type", "generic")
    cmd = ["iio-emu", emu_type]

    if entry.get("xml_path"):
        cmd.append(entry["xml_path"])

    if entry.get("rx_tx_device") and entry.get("rx_tx_bin_path"):
        bin_path = os.path.join(emu_dir, entry["rx_tx_bin_path"])
        cmd.append(f"{entry['rx_tx_device']}@{bin_path}")

    try:
        proc = subprocess.Popen(
            cmd, cwd=emu_dir,
            stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
        )
    except FileNotFoundError:
        print("  ERROR: iio-emu not found")
        return None

    time.sleep(0.5)
    if proc.poll() is not None:
        stderr = proc.stderr.read().decode(errors="replace").strip()
        print(f"  ERROR: iio-emu exited immediately: {stderr}")
        return None

    return proc


def stop_emulator(proc):
    if proc is None:
        return
    proc.terminate()
    try:
        proc.wait(timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()


def make_wrapper(out_dir, device_name, js_script):
    abs_js = os.path.abspath(js_script).replace("\\", "/")
    abs_out = os.path.abspath(out_dir).replace("\\", "/") + "/"
    content = (
        f'var qt6ScreenshotDir = "{abs_out}";\n'
        f'var qt6DeviceName = "{device_name}";\n'
        f'evaluateFile("{abs_js}");\n'
    )
    fd, path = tempfile.mkstemp(suffix=".js", prefix="qt6_layout_")
    with os.fdopen(fd, "w") as f:
        f.write(content)
    return path


def discover_all_entries():
    packages_dir = os.path.join(PROJECT_ROOT, "packages")
    entries = []
    for pkg in sorted(os.listdir(packages_dir)):
        setup_path = os.path.join(packages_dir, pkg, "emu-xml", "emu_setup.json")
        if not os.path.isfile(setup_path):
            continue
        with open(setup_path) as f:
            data = json.load(f)
        emu_dir = os.path.join(packages_dir, pkg, "emu-xml")
        for entry in data:
            if entry.get("xml_path") or entry.get("emu-type"):
                entries.append((pkg, emu_dir, entry))
    return entries


def main():
    parser = argparse.ArgumentParser(description="Qt6 layout screenshot runner")
    parser.add_argument("--scopy", default="./scopy", help="Path to scopy binary")
    parser.add_argument("--output", default="/tmp/qt6-regression-screenshots",
                        help="Output directory")
    args = parser.parse_args()

    scopy_bin = os.path.abspath(args.scopy)
    build_dir = os.path.dirname(scopy_bin)
    scopy_name = os.path.basename(scopy_bin)
    output_dir = os.path.abspath(args.output)
    js_script = os.path.join(SCRIPT_DIR, "qt6_layout_test.js")

    os.makedirs(output_dir, exist_ok=True)

    entries = discover_all_entries()
    print(f"Found {len(entries)} emulator entries across all packages\n")

    failures = []
    successes = []

    for package, emu_dir, entry in entries:
        device = entry.get("device", "unknown")
        label = f"{package}/{device}"
        print(f"--- {label} ---")

        emu_proc = start_emulator(entry, emu_dir)
        if emu_proc is None:
            failures.append(f"{label}: emulator failed")
            continue

        try:
            if not wait_for_port("127.0.0.1", EMU_PORT, EMU_TIMEOUT):
                print(f"  ERROR: port {EMU_PORT} not ready")
                failures.append(f"{label}: port timeout")
                continue

            wrapper = make_wrapper(output_dir, device, js_script)
            try:
                cmd = [os.path.join(".", scopy_name), "--script", wrapper]
                env = os.environ.copy()
                env["QT_QPA_PLATFORM"] = "offscreen"
                proc = subprocess.Popen(cmd, cwd=build_dir, env=env)
                try:
                    rc = proc.wait(timeout=SCOPY_TIMEOUT)
                except subprocess.TimeoutExpired:
                    print(f"  WARNING: Scopy timed out after {SCOPY_TIMEOUT}s")
                    proc.kill()
                    proc.wait()
                    rc = 1

                if rc == 0:
                    successes.append(label)
                    print(f"  OK")
                else:
                    failures.append(f"{label}: scopy exit {rc}")
                    print(f"  FAIL (exit {rc})")
            finally:
                os.unlink(wrapper)
        finally:
            stop_emulator(emu_proc)

        print()

    print("=" * 50)
    print(f"Results: {len(successes)} passed, {len(failures)} failed")
    if failures:
        print("Failures:")
        for f in failures:
            print(f"  - {f}")
    print(f"\nScreenshots saved to: {output_dir}")

    sys.exit(1 if failures else 0)


if __name__ == "__main__":
    main()
