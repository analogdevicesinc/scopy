#!/usr/bin/env python3
"""
doc_screenshots.py — Auto-capture Scopy documentation screenshots.

Usage:
    python3 tools/doc_screenshots.py --package ad9371 --emu-file ad9371.xml [options]

Options:
    --package   Package directory name (e.g. ad9371)
    --emu-file  EMU XML filename in packages/<package>/emu-xml/ (e.g. ad9371.xml)
    --scopy     Path to scopy binary (default: ./scopy)
    --output    Output directory (default: docs/screenshots/<package>)
"""

import argparse
import json
import os
import socket
import subprocess
import sys
import tempfile
import time

EMU_PORT = 30431
EMU_TIMEOUT = 60

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def find_emu_setup(package, emu_file):
    """Read emu_setup.json and find the entry matching emu_file by xml_path."""
    emu_dir = os.path.join(PROJECT_ROOT, "packages", package, "emu-xml")
    setup_path = os.path.join(emu_dir, "emu_setup.json")

    if not os.path.isfile(setup_path):
        print(f"[doc] ERROR: emu_setup.json not found at {setup_path}")
        sys.exit(1)

    with open(setup_path) as f:
        entries = json.load(f)

    for entry in entries:
        if entry.get("xml_path") == emu_file:
            return emu_dir, entry["uri"]

    available = [e["xml_path"] for e in entries]
    print(f"[doc] ERROR: '{emu_file}' not found in {setup_path}")
    print(f"[doc] Available files: {available}")
    sys.exit(1)


def wait_for_port(host, port, timeout):
    """Poll until TCP port accepts connections. Returns True if successful."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            with socket.create_connection((host, port), timeout=1):
                return True
        except (ConnectionRefusedError, OSError):
            time.sleep(0.5)
    return False


def start_emulator(emu_xml_path):
    """Start iio-emu as a background process. Returns the Popen object."""
    print(f"[doc] Starting iio-emu: {emu_xml_path}")
    try:
        proc = subprocess.Popen(
            ["iio-emu", "generic", emu_xml_path],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
        )
    except FileNotFoundError:
        print("[doc] ERROR: 'iio-emu' not found on PATH")
        sys.exit(1)

    time.sleep(0.5)
    if proc.poll() is not None:
        stderr = proc.stderr.read().decode(errors="replace").strip()
        print(f"[doc] ERROR: iio-emu exited immediately. stderr: {stderr}")
        sys.exit(1)

    print(f"[doc] iio-emu started (PID {proc.pid})")
    return proc


def make_wrapper(package, uri, out_dir, js_script_path):
    """Create a temp JS file that injects globals then evaluates the real script."""
    abs_js = os.path.abspath(js_script_path).replace("\\", "/")
    abs_out = os.path.abspath(out_dir).replace("\\", "/") + "/"
    content = (
        f'var scopyPackage = "{package}";\n'
        f'var scopyUri = "{uri}";\n'
        f'var scopyOutDir = "{abs_out}";\n'
        f'evaluateFile("{abs_js}");\n'
    )
    fd, path = tempfile.mkstemp(suffix=".js", prefix="scopy_doc_")
    with os.fdopen(fd, "w") as f:
        f.write(content)
    return path


def run_scopy(scopy_bin, wrapper_path):
    """Launch scopy with the wrapper script and wait for it to exit."""
    build_dir = os.path.dirname(os.path.abspath(scopy_bin))
    scopy_name = os.path.basename(scopy_bin)
    wrapper_rel = os.path.relpath(os.path.abspath(wrapper_path), build_dir)
    cmd = [os.path.join(".", scopy_name), "--script", wrapper_rel]
    print(f"[doc] Launching: {' '.join(cmd)} (cwd: {build_dir})")
    proc = subprocess.Popen(cmd, cwd=build_dir)
    proc.wait()
    return proc.returncode


def main():
    parser = argparse.ArgumentParser(
        description="Capture Scopy documentation screenshots via IIO emulator"
    )
    parser.add_argument("--package", required=True, help="Package name (e.g. ad9371)")
    parser.add_argument(
        "--emu-file", required=True, help="EMU XML filename (e.g. ad9371.xml)"
    )
    parser.add_argument("--scopy", default="./scopy", help="Path to scopy binary")
    parser.add_argument(
        "--output",
        default=None,
        help="Output directory (default: docs/screenshots/<package>)",
    )
    args = parser.parse_args()

    emu_dir, uri = find_emu_setup(args.package, args.emu_file)
    emu_xml_path = os.path.join(emu_dir, args.emu_file)

    out_dir = args.output or os.path.join(
        PROJECT_ROOT, "docs", "screenshots", args.package
    )
    os.makedirs(out_dir, exist_ok=True)
    print(f"[doc] Output directory: {out_dir}")

    js_script = os.path.join(PROJECT_ROOT, "js", "doc_screenshots.js")
    if not os.path.isfile(js_script):
        print(f"[doc] ERROR: JS script not found at {js_script}")
        sys.exit(1)

    emu_proc = start_emulator(emu_xml_path)
    try:
        print(f"[doc] Waiting for port {EMU_PORT} (timeout {EMU_TIMEOUT}s)...")
        if not wait_for_port("127.0.0.1", EMU_PORT, EMU_TIMEOUT):
            print(
                f"[doc] ERROR: iio-emu did not open port {EMU_PORT} within {EMU_TIMEOUT}s"
            )
            sys.exit(1)
        print(f"[doc] Port {EMU_PORT} ready")

        wrapper = make_wrapper(args.package, uri, out_dir, js_script)
        try:
            rc = run_scopy(args.scopy, wrapper)
        finally:
            os.unlink(wrapper)

        if rc == 0:
            print(f"[doc] Done. Screenshots saved to: {out_dir}")
        else:
            print(f"[doc] Scopy exited with code {rc}")
            sys.exit(rc)
    finally:
        emu_proc.terminate()
        try:
            emu_proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            emu_proc.kill()
        print("[doc] iio-emu stopped")


if __name__ == "__main__":
    main()
