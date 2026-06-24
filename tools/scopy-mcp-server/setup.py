#!/usr/bin/env python3
"""Setup script for standalone Scopy MCP server (downloaded zip).

Checks Python version, installs via uv (preferred) or pip (fallback),
configures Claude Code MCP entry, and optionally adds permission allowlist.
"""

import json
import os
import platform
import shutil
import subprocess
import sysconfig
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

GREEN = "\033[1;32m"
BLUE  = "\033[1;34m"
YELLOW = "\033[1;33m"
RED   = "\033[1;31m"
RESET = "\033[0m"


def info(msg):  print(f"{BLUE}[INFO]{RESET}  {msg}")
def ok(msg):    print(f"{GREEN}[OK]{RESET}    {msg}")
def warn(msg):  print(f"{YELLOW}[WARN]{RESET}  {msg}")
def fail(msg):  print(f"{RED}[FAIL]{RESET}  {msg}"); sys.exit(1)


def _claude_config_dir() -> str | None:
    """Return the Claude Code config directory for this OS, or None if unknown."""
    system = platform.system()
    if system in ("Linux", "Darwin"):
        return os.path.expanduser("~/.claude")
    elif system == "Windows":
        appdata = os.environ.get("APPDATA")
        if appdata:
            return os.path.join(appdata, "Claude")
    return None


def _mcp_entry(command: str, used_uv: bool) -> dict:
    if used_uv:
        return {
            "command": command,
            "args": ["run", "--directory", SCRIPT_DIR, "scopy-mcp-server"],
        }
    return {"command": command}


# ---------------------------------------------------------------------------
# Step 1 — Python version check
# ---------------------------------------------------------------------------

def check_python():
    info("Checking Python version...")
    major, minor = sys.version_info.major, sys.version_info.minor
    if major < 3 or (major == 3 and minor < 10):
        fail(f"Python 3.10+ required (found {major}.{minor}). "
             "Install from https://www.python.org/downloads/")
    ok(f"Python {major}.{minor} ({sys.executable})")


# ---------------------------------------------------------------------------
# Step 2 — Install dependencies
# ---------------------------------------------------------------------------

def install_with_uv() -> str | None:
    """Try to install with uv. Returns absolute uv path on success, None on failure."""
    uv = shutil.which("uv")
    if not uv:
        info("uv not found, trying pip instead...")
        return None
    ver = subprocess.run([uv, "--version"], capture_output=True, text=True)
    ok(f"uv found ({ver.stdout.strip()})")
    info("Installing with uv sync...")
    # On Windows, include the optional pywin32 dependency for named-pipe IPC
    cmd = [uv, "sync"]
    if platform.system() == "Windows":
        cmd += ["--extra", "windows"]
    result = subprocess.run(cmd, cwd=SCRIPT_DIR)
    if result.returncode != 0:
        info("uv sync failed, trying pip instead...")
        return None
    ok("Dependencies installed (uv)")
    return uv


def _resolve_entry_point() -> str:
    """Return the absolute path to the scopy-mcp-server script after pip install."""
    # Try PATH first (works when ~/.local/bin is in PATH)
    found = shutil.which("scopy-mcp-server")
    if found:
        return found
    # Fallback: derive from sysconfig scripts dir (covers user installs not yet in PATH)
    scripts_dir = sysconfig.get_path("scripts", f"{os.name}_user")
    if scripts_dir:
        candidate = os.path.join(scripts_dir, "scopy-mcp-server")
        if os.path.isfile(candidate):
            return candidate
    # Last resort: scripts dir for the current Python prefix
    scripts_dir = sysconfig.get_path("scripts")
    if scripts_dir:
        candidate = os.path.join(scripts_dir, "scopy-mcp-server")
        if os.path.isfile(candidate):
            return candidate
    return "scopy-mcp-server"  # give up, use bare name


def install_with_pip() -> str:
    """Install with pip. Returns absolute path to the installed entry point."""
    info("Installing with pip...")
    # On Windows, include the optional pywin32 dependency for named-pipe IPC
    if platform.system() == "Windows":
        install_target = ".[windows]"
    else:
        install_target = "."
    result = subprocess.run(
        [sys.executable, "-m", "pip", "install", install_target], cwd=SCRIPT_DIR
    )
    if result.returncode != 0:
        fail("pip install failed. Check the output above for errors.")
    ok("Dependencies installed (pip)")
    return _resolve_entry_point()


# ---------------------------------------------------------------------------
# Step 3A — Write ~/.claude/.mcp.json
# ---------------------------------------------------------------------------

def configure_mcp_json(command: str, used_uv: bool):
    print()
    info("Configuring Claude Code MCP entry...")

    claude_dir = _claude_config_dir()
    if not claude_dir:
        warn("Unrecognised OS — could not locate Claude Code config directory.")
        _print_mcp_snippet(command, used_uv)
        return

    mcp_path = os.path.expanduser("~/.claude.json")

    if not os.path.isdir(claude_dir):
        warn(f"Claude Code config directory not found at {claude_dir}.")
        warn("Is Claude Code installed? Skipping automatic .mcp.json setup.")
        _print_mcp_snippet(command, used_uv)
        return

    entry = _mcp_entry(command, used_uv)

    if os.path.isfile(mcp_path):
        try:
            with open(mcp_path) as f:
                data = json.load(f)
        except json.JSONDecodeError:
            warn(f"{mcp_path} exists but is not valid JSON — skipping automatic update.")
            _print_mcp_snippet(command, used_uv)
            return
    else:
        data = {}

    data.setdefault("mcpServers", {})["scopy"] = entry

    with open(mcp_path, "w") as f:
        json.dump(data, f, indent=2)
        f.write("\n")

    ok(f"Updated {mcp_path} with 'scopy' MCP server entry (global scope).")


def _print_mcp_snippet(command: str, used_uv: bool):
    config = {"mcpServers": {"scopy": _mcp_entry(command, used_uv)}}
    print()
    print("  Add this to your Claude Code global config manually")
    print("  (~/.claude.json or <project>/.mcp.json):")
    print()
    print(json.dumps(config, indent=2))
    print()
    print('  If Scopy is not on your PATH, add to the "scopy" entry:')
    print('    "env": { "SCOPY_PATH": "/path/to/scopy" }')


# ---------------------------------------------------------------------------
# Step 3B — Opt-in: add mcp__scopy__* to Claude Code permissions
# ---------------------------------------------------------------------------

SCOPY_PERMISSION = "mcp__scopy__*"


def configure_permissions():
    print()
    claude_dir = _claude_config_dir()
    if not claude_dir:
        return

    settings_path = os.path.join(claude_dir, "settings.json")

    if not os.path.isfile(settings_path):
        info(f"Claude Code settings.json not found at {settings_path} — skipping permissions setup.")
        return

    try:
        with open(settings_path) as f:
            settings = json.load(f)
    except json.JSONDecodeError:
        warn(f"{settings_path} is not valid JSON — skipping permissions setup.")
        return

    existing = settings.get("permissions", {}).get("allow", [])
    if SCOPY_PERMISSION in existing:
        ok(f"'{SCOPY_PERMISSION}' is already in your permissions list.")
        return

    print(f"  Adding \"{SCOPY_PERMISSION}\" to your permissions list will make the")
    print("  experience smoother — Claude won't ask for approval on each Scopy")
    print("  tool call. Add it? [Y/n] ", end="", flush=True)

    try:
        answer = input().strip().lower()
    except EOFError:
        answer = "n"

    if answer in ("", "y", "yes"):
        settings.setdefault("permissions", {}).setdefault("allow", []).append(SCOPY_PERMISSION)
        with open(settings_path, "w") as f:
            json.dump(settings, f, indent=2)
            f.write("\n")
        ok(f"Added '{SCOPY_PERMISSION}' to {settings_path}")
    else:
        info("Skipped. To add it manually, append to permissions.allow in:")
        info(f"  {settings_path}")


# ---------------------------------------------------------------------------
# Step 4 — Summary
# ---------------------------------------------------------------------------

def print_summary():
    print()
    print("────────────────────────────────────────────────")
    ok("Scopy MCP server installed successfully!")
    print()
    print("  Next steps:")
    print("    1. If Scopy is not on your PATH, add to the 'scopy' entry in .mcp.json:")
    print('         "env": { "SCOPY_PATH": "/path/to/scopy" }')
    print("    2. Restart Claude Code to pick up the new MCP config")
    print("    3. Start Scopy, then ask Claude to interact with your instruments")
    print("────────────────────────────────────────────────")


# ---------------------------------------------------------------------------

def main():
    check_python()
    uv_path = install_with_uv()
    if uv_path:
        command, used_uv = uv_path, True
    else:
        command, used_uv = install_with_pip(), False
    configure_mcp_json(command, used_uv)
    configure_permissions()
    print_summary()


if __name__ == "__main__":
    main()
