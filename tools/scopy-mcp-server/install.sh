#!/usr/bin/env bash
set -e

BOLD='\033[1m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

info()  { echo -e "${BOLD}$*${NC}"; }
ok()    { echo -e "${GREEN}✓${NC} $*"; }
warn()  { echo -e "${YELLOW}⚠${NC}  $*"; }
fail()  { echo -e "${RED}✗${NC}  $*" >&2; exit 1; }

echo ""
info "Scopy MCP Server — installer"
echo ""

# ── 1. Check Python 3.10+ ────────────────────────────────────────────────────

if ! command -v python3 &>/dev/null; then
    fail "Python 3 not found. Install Python 3.10+ from https://python.org and retry."
fi

PY_VERSION=$(python3 -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")')
PY_MAJOR=$(echo "$PY_VERSION" | cut -d. -f1)
PY_MINOR=$(echo "$PY_VERSION" | cut -d. -f2)

if [[ "$PY_MAJOR" -lt 3 || ("$PY_MAJOR" -eq 3 && "$PY_MINOR" -lt 10) ]]; then
    fail "Python $PY_VERSION found, but 3.10+ is required. Please upgrade."
fi

ok "Python $PY_VERSION"

# ── 2. Detect Scopy binary ───────────────────────────────────────────────────

SCOPY_BIN=""
SCOPY_ON_PATH=false

# Honour SCOPY_PATH if the caller set it
if [[ -n "${SCOPY_PATH:-}" ]]; then
    if [[ -x "$SCOPY_PATH" ]]; then
        SCOPY_BIN="$SCOPY_PATH"
        ok "Scopy found (from SCOPY_PATH): $SCOPY_BIN"
    else
        warn "SCOPY_PATH is set to '$SCOPY_PATH' but that file is not executable."
    fi
fi

# Auto-detect if not already found
if [[ -z "$SCOPY_BIN" ]]; then
    if command -v scopy &>/dev/null; then
        SCOPY_BIN=$(command -v scopy)
        SCOPY_ON_PATH=true
        ok "Scopy found on PATH: $SCOPY_BIN"
    else
        for candidate in \
            /opt/scopy/bin/scopy \
            /usr/local/bin/scopy \
            /usr/bin/scopy \
            "$HOME/.local/bin/scopy"
        do
            if [[ -x "$candidate" ]]; then
                SCOPY_BIN="$candidate"
                ok "Scopy found: $SCOPY_BIN"
                break
            fi
        done
    fi
fi

if [[ -z "$SCOPY_BIN" ]]; then
    warn "Scopy binary not found. You will need to set SCOPY_PATH manually in .mcp.json."
fi

# ── 3. Install the package ───────────────────────────────────────────────────

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo ""
info "Installing scopy-mcp-server..."

if command -v uv &>/dev/null; then
    uv pip install --quiet --python python3 "$SCRIPT_DIR" \
        || fail "Installation failed. Check the output above for details."
    ok "Installed via uv"
else
    pip3 install --quiet "$SCRIPT_DIR" \
        || fail "Installation failed. Check the output above for details."
    ok "Installed via pip"
fi

# ── 4. Confirm entry point is reachable ──────────────────────────────────────

if ! command -v scopy-mcp-server &>/dev/null; then
    warn "'scopy-mcp-server' is not on your PATH."
    warn "You may need to add ~/.local/bin to PATH:"
    warn "  export PATH=\"\$HOME/.local/bin:\$PATH\""
    CMD=$(python3 -c "import sysconfig, os; print(os.path.join(sysconfig.get_path('scripts'), 'scopy-mcp-server'))")
    warn "Full path to the entry point: $CMD"
else
    ok "scopy-mcp-server is on PATH"
    CMD="scopy-mcp-server"
fi

# ── 5. Print .mcp.json snippet ───────────────────────────────────────────────

echo ""
echo "════════════════════════════════════════════════════════════"
echo -e "${GREEN}${BOLD} Scopy MCP Server installed successfully!${NC}"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "Add this to your .mcp.json"
echo "(project root, or ~/.claude/.mcp.json for all projects):"
echo ""

if [[ -n "$SCOPY_BIN" && "$SCOPY_ON_PATH" == false ]]; then
    # Scopy found at a non-PATH location — include SCOPY_PATH
    cat <<EOF
{
  "mcpServers": {
    "scopy": {
      "command": "$CMD",
      "env": {
        "SCOPY_PATH": "$SCOPY_BIN"
      }
    }
  }
}
EOF
elif [[ "$SCOPY_ON_PATH" == true ]]; then
    # Scopy is on PATH — no env block needed
    cat <<EOF
{
  "mcpServers": {
    "scopy": {
      "command": "$CMD"
    }
  }
}
EOF
else
    # Scopy not found — show placeholder
    cat <<EOF
{
  "mcpServers": {
    "scopy": {
      "command": "$CMD",
      "env": {
        "SCOPY_PATH": "/path/to/scopy"
      }
    }
  }
}
EOF
    echo ""
    warn "Replace /path/to/scopy with the actual path to your Scopy binary."
fi

echo ""
echo "Then restart Claude Code to pick up the new server."
echo "════════════════════════════════════════════════════════════"
echo ""
