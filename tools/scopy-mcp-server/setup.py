#!/usr/bin/env python3
"""Setup script for standalone Scopy MCP server (downloaded zip).

Checks Python version, installs via uv (preferred) or pip (fallback),
and prints the .mcp.json snippet for Claude Code.
"""

import json
import os
import shutil
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

GREEN = "\033[1;32m"
BLUE = "\033[1;34m"
RED = "\033[1;31m"
RESET = "\033[0m"


def info(msg):
    print(f"{BLUE}[INFO]{RESET}  {msg}")


def ok(msg):
    print(f"{GREEN}[OK]{RESET}    {msg}")


def fail(msg):
    print(f"{RED}[FAIL]{RESET}  {msg}")
    sys.exit(1)


def run(cmd, **kwargs):
    return subprocess.run(cmd, **kwargs)


def check_python():
    info("Checking Python version...")
    major, minor = sys.version_info.major, sys.version_info.minor
    if major < 3 or (major == 3 and minor < 10):
        fail(f"Python 3.10+ is required (found {major}.{minor}). "
             "Install from https://www.python.org/downloads/")
    ok(f"Python {major}.{minor} ({sys.executable})")


def install_with_uv():
    """Try to install with uv. Returns True on success."""
    uv = shutil.which("uv")
    if not uv:
        info("uv not found, trying pip instead...")
        return False

    ver = subprocess.run([uv, "--version"], capture_output=True, text=True)
    ok(f"uv found ({ver.stdout.strip()})")

    info("Installing with uv sync...")
    result = run([uv, "sync"], cwd=SCRIPT_DIR)
    if result.returncode != 0:
        info("uv sync failed, trying pip instead...")
        return False

    ok("Dependencies installed (uv)")
    return True


def install_with_pip():
    """Install with pip. Returns True on success."""
    info("Installing with pip...")
    result = run([sys.executable, "-m", "pip", "install", "."], cwd=SCRIPT_DIR)
    if result.returncode != 0:
        fail("pip install failed. Check the output above for errors.")
    ok("Dependencies installed (pip)")
    return True


def print_config(used_uv):
    if used_uv:
        config = {
            "mcpServers": {
                "scopy": {
                    "command": "uv",
                    "args": ["run", "--directory", SCRIPT_DIR,
                             "scopy-mcp-server"],
                }
            }
        }
    else:
        config = {
            "mcpServers": {
                "scopy": {
                    "command": "scopy-mcp-server",
                }
            }
        }

    print()
    print("────────────────────────────────────────────────")
    ok("Scopy MCP server installed successfully!")
    print()
    print("  Add this to your Claude Code MCP config")
    print("  (~/.claude/.mcp.json or <project>/.mcp.json):")
    print()
    print(json.dumps(config, indent=2))
    print()
    print('  If Scopy is not on your PATH, add to the "scopy" entry:')
    print('    "env": { "SCOPY_PATH": "/path/to/scopy" }')
    print()
    print("  Then restart Claude Code.")
    print("────────────────────────────────────────────────")


def main():
    check_python()
    used_uv = install_with_uv()
    if not used_uv:
        install_with_pip()
    print_config(used_uv)


if __name__ == "__main__":
    main()
