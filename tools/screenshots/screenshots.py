#!/usr/bin/env python3
"""
screenshots.py — Auto-capture Scopy documentation screenshots.

Usage:
    # Single package mode:
    python3 tools/screenshots/screenshots.py --package ad936x --emu-file fmcomms5.xml [options]

    # Batch mode (all packages with emu-xml):
    python3 tools/screenshots/screenshots.py --all --scopy ./scopy [options]

Options:
    --package      Package directory name (e.g. ad936x)
    --emu-file     EMU XML filename or device name (e.g. fmcomms5.xml or adalm2000)
    --scopy        Path to scopy binary (default: ./scopy)
    --output       Output directory for single package mode
    --all          Discover and run all packages with emu_setup.json
    --output-root  Base output directory for --all mode (default: docs/screenshots)
"""

import argparse
import glob
import json
import os
import re
import socket
import subprocess
import sys
import tempfile
import time

EMU_PORT = 30431
EMU_TIMEOUT = 60

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(os.path.dirname(SCRIPT_DIR))


def find_emu_setup(package, emu_file):
    """Read emu_setup.json and find the entry matching emu_file by xml_path or device name."""
    emu_dir = os.path.join(PROJECT_ROOT, "packages", package, "emu-xml")
    setup_path = os.path.join(emu_dir, "emu_setup.json")

    if not os.path.isfile(setup_path):
        print(f"[doc] ERROR: emu_setup.json not found at {setup_path}")
        sys.exit(1)

    with open(setup_path) as f:
        entries = json.load(f)

    for entry in entries:
        if entry.get("xml_path") == emu_file or entry.get("device") == emu_file:
            return emu_dir, entry

    available = [e.get("xml_path") or e.get("device") for e in entries]
    print(f"[doc] ERROR: '{emu_file}' not found in {setup_path}")
    print(f"[doc] Available: {available}")
    sys.exit(1)


def can_emulate(entry):
    """Check if an entry has enough info to start an emulator."""
    return bool(entry.get("xml_path") or entry.get("emu-type"))


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


def start_emulator(entry, emu_dir):
    """Start iio-emu as a background process. Returns the Popen object."""
    emu_type = entry.get("emu-type", "generic")
    cmd = ["iio-emu", emu_type]

    if entry.get("xml_path"):
        cmd.append(entry["xml_path"])

    if entry.get("rx_tx_device") and entry.get("rx_tx_bin_path"):
        bin_path = os.path.join(emu_dir, entry["rx_tx_bin_path"])
        cmd.append(f"{entry['rx_tx_device']}@{bin_path}")

    print(f"[doc] Starting iio-emu: {' '.join(cmd)} (cwd: {emu_dir})")
    try:
        proc = subprocess.Popen(
            cmd,
            cwd=emu_dir,
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
        return None

    print(f"[doc] iio-emu started (PID {proc.pid})")
    return proc


def stop_emulator(proc):
    """Stop an emulator process."""
    if proc is None:
        return
    proc.terminate()
    try:
        proc.wait(timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()
    print("[doc] iio-emu stopped")


def make_wrapper(uri, out_dir, js_script_path, skip_plugins=None):
    """Create a temp JS file that injects globals then evaluates the real script."""
    abs_js = os.path.abspath(js_script_path).replace("\\", "/")
    abs_out = os.path.abspath(out_dir).replace("\\", "/") + "/"
    skip_json = json.dumps(skip_plugins or [])
    content = (
        f'var scopyUri = "{uri}";\n'
        f'var scopyOutDir = "{abs_out}";\n'
        f"var scopySkipPlugins = {skip_json};\n"
        f'evaluateFile("{abs_js}");\n'
    )
    fd, path = tempfile.mkstemp(suffix=".js", prefix="scopy_doc_")
    with os.fdopen(fd, "w") as f:
        f.write(content)
    return path

# 3 minutes per device should be more than enough, but we don't want to wait indefinitely if something hangs
SCOPY_TIMEOUT = 180

def configure_scopy_preferences(scopy_bin):
    """Configure Scopy preferences for headless screenshot capture."""
    import configparser

    prefs_dir = os.path.join(os.path.expanduser("~"), ".config", "ADI", "Scopy-v2")
    os.makedirs(prefs_dir, exist_ok=True)
    prefs_file = os.path.join(prefs_dir, "preferences.ini")

    config = configparser.ConfigParser(strict=False)
    config.read(prefs_file)
    if "General" not in config:
        config["General"] = {}
    config["General"]["general_first_run"] = "false"
    config["General"]["general_dont_show_whats_new"] = "true"

    packages_dir = os.path.join(os.path.dirname(os.path.abspath(scopy_bin)), "packages")
    config["General"]["packages_path"] = packages_dir

    with open(prefs_file, "w") as f:
        config.write(f)
    print(f"[doc] Configured Scopy preferences in {prefs_file}")
    print(f"[doc]   packages_path = {packages_dir}")


def run_scopy(scopy_bin, wrapper_path):
    """Launch scopy with the wrapper script and wait for it to exit."""
    build_dir = os.path.dirname(os.path.abspath(scopy_bin))
    scopy_name = os.path.basename(scopy_bin)
    wrapper_rel = os.path.relpath(os.path.abspath(wrapper_path), build_dir)
    cmd = [os.path.join(".", scopy_name), "--script", wrapper_rel]
    print(f"[doc] Launching: {' '.join(cmd)} (cwd: {build_dir})")
    proc = subprocess.Popen(cmd, cwd=build_dir)
    try:
        proc.wait(timeout=SCOPY_TIMEOUT)
    except subprocess.TimeoutExpired:
        print(f"[doc] WARNING: Scopy timed out after {SCOPY_TIMEOUT}s, killing")
        proc.kill()
        proc.wait()
        return 1
    return proc.returncode


def discover_generic_plugin_names():
    """Read SCOPY_PLUGIN_NAME from generic-plugins and extproc header files."""
    patterns = [
        os.path.join(PROJECT_ROOT, "packages", "generic-plugins", "plugins", "*", "include", "*", "*.h"),
        os.path.join(PROJECT_ROOT, "packages", "extproc", "plugins", "*", "include", "*", "*.h"),
    ]
    names = []
    for pattern in patterns:
        for header in glob.glob(pattern):
            with open(header) as f:
                for line in f:
                    m = re.search(r"#define\s+SCOPY_PLUGIN_NAME\s+(\w+)", line)
                    if m:
                        names.append(m.group(1))
                        break
    print(f"[doc] Generic plugin names: {names}")
    return names


def discover_all_packages():
    """Find all packages with emu_setup.json. Yields (package, emu_dir, entry) tuples.

    generic-plugins entries are yielded first so they capture all tools,
    then other packages capture only device-specific tools.
    """
    packages_dir = os.path.join(PROJECT_ROOT, "packages")
    generic_entries = []
    other_entries = []

    for pkg in sorted(os.listdir(packages_dir)):
        setup_path = os.path.join(packages_dir, pkg, "emu-xml", "emu_setup.json")
        if not os.path.isfile(setup_path):
            continue
        with open(setup_path) as f:
            entries = json.load(f)
        emu_dir = os.path.join(packages_dir, pkg, "emu-xml")
        for entry in entries:
            if pkg == "generic-plugins":
                generic_entries.append((pkg, emu_dir, entry))
            else:
                other_entries.append((pkg, emu_dir, entry))

    return generic_entries + other_entries


def run_single(args):
    """Run screenshot capture for a single package/emu-file."""
    configure_scopy_preferences(args.scopy)
    emu_dir, entry = find_emu_setup(args.package, args.emu_file)
    uri = entry["uri"]

    if not can_emulate(entry):
        print(f"[doc] ERROR: entry has no xml_path or emu-type, cannot emulate")
        sys.exit(1)

    out_dir = args.output or os.path.join(
        PROJECT_ROOT, "docs", "screenshots", args.package
    )
    os.makedirs(out_dir, exist_ok=True)
    print(f"[doc] Output directory: {out_dir}")

    js_script = os.path.join(SCRIPT_DIR, "screenshots.js")
    if not os.path.isfile(js_script):
        print(f"[doc] ERROR: JS script not found at {js_script}")
        sys.exit(1)

    emu_proc = start_emulator(entry, emu_dir)
    if emu_proc is None:
        sys.exit(1)
    try:
        print(f"[doc] Waiting for port {EMU_PORT} (timeout {EMU_TIMEOUT}s)...")
        if not wait_for_port("127.0.0.1", EMU_PORT, EMU_TIMEOUT):
            print(
                f"[doc] ERROR: iio-emu did not open port {EMU_PORT} within {EMU_TIMEOUT}s"
            )
            sys.exit(1)
        print(f"[doc] Port {EMU_PORT} ready")

        wrapper = make_wrapper(uri, out_dir, js_script)
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
        stop_emulator(emu_proc)


def run_all(args):
    """Discover all packages and run screenshot capture for each device entry."""
    configure_scopy_preferences(args.scopy)
    js_script = os.path.join(SCRIPT_DIR, "screenshots.js")
    if not os.path.isfile(js_script):
        print(f"[doc] ERROR: JS script not found at {js_script}")
        sys.exit(1)

    output_root = args.output_root or os.path.join(
        PROJECT_ROOT, "docs", "screenshots"
    )

    generic_names = discover_generic_plugin_names()
    all_entries = discover_all_packages()
    failures = []

    for package, emu_dir, entry in all_entries:
        device = entry.get("device", "unknown")
        print(f"\n[doc] === {package}/{device} ===")

        if not can_emulate(entry):
            print(f"[doc] Skipping {package}/{device}: no xml_path or emu-type")
            continue

        uri = entry["uri"]
        out_dir = os.path.join(output_root, package, device)
        os.makedirs(out_dir, exist_ok=True)

        skip_plugins = [] if package == "generic-plugins" else generic_names

        emu_proc = start_emulator(entry, emu_dir)
        if emu_proc is None:
            failures.append(f"{package}/{device}: emulator failed to start")
            continue
        try:
            print(f"[doc] Waiting for port {EMU_PORT} (timeout {EMU_TIMEOUT}s)...")
            if not wait_for_port("127.0.0.1", EMU_PORT, EMU_TIMEOUT):
                print(f"[doc] ERROR: port {EMU_PORT} not ready for {package}/{device}")
                failures.append(f"{package}/{device}: port timeout")
                continue

            print(f"[doc] Port {EMU_PORT} ready")
            wrapper = make_wrapper(uri, out_dir, js_script, skip_plugins)
            try:
                rc = run_scopy(args.scopy, wrapper)
            finally:
                os.unlink(wrapper)

            if rc == 0:
                print(f"[doc] Done: {package}/{device} -> {out_dir}")
            else:
                print(f"[doc] WARNING: Scopy exited with code {rc} for {package}/{device}")
                failures.append(f"{package}/{device}: scopy exit code {rc}")
        finally:
            stop_emulator(emu_proc)

    if failures:
        print(f"\n[doc] === FAILURES ({len(failures)}) ===")
        for f in failures:
            print(f"[doc]   - {f}")
        sys.exit(1)
    else:
        print(f"\n[doc] All screenshots captured successfully to: {output_root}")


def main():
    parser = argparse.ArgumentParser(
        description="Capture Scopy documentation screenshots via IIO emulator"
    )
    parser.add_argument("--package", help="Package name (e.g. ad936x)")
    parser.add_argument(
        "--emu-file", help="EMU XML filename or device name (e.g. fmcomms5.xml or adalm2000)"
    )
    parser.add_argument("--scopy", default="./scopy", help="Path to scopy binary")
    parser.add_argument(
        "--output",
        default=None,
        help="Output directory for single package mode",
    )
    parser.add_argument(
        "--all",
        action="store_true",
        help="Discover and run all packages with emu_setup.json",
    )
    parser.add_argument(
        "--output-root",
        default=None,
        help="Base output directory for --all mode (default: docs/screenshots)",
    )
    args = parser.parse_args()

    if args.all:
        run_all(args)
    elif args.package and args.emu_file:
        run_single(args)
    else:
        parser.error("Either --all or both --package and --emu-file are required")


if __name__ == "__main__":
    main()
