#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
MCP_JSON="$REPO_ROOT/.mcp.json"
API_JSON="$SCRIPT_DIR/scopy_mcp_server/scopy_api.json"
GENERATOR="$SCRIPT_DIR/generate_api_tools.py"

info()  { printf "\033[1;34m[INFO]\033[0m  %s\n" "$*"; }
ok()    { printf "\033[1;32m[OK]\033[0m    %s\n" "$*"; }
warn()  { printf "\033[1;33m[WARN]\033[0m  %s\n" "$*"; }
fail()  { printf "\033[1;31m[FAIL]\033[0m  %s\n" "$*"; exit 1; }

# ── 1. Check Python >= 3.10 ──────────────────────────────────────────────────

info "Checking Python version..."
PYTHON=""
for candidate in python3 python; do
    if command -v "$candidate" &>/dev/null; then
        ver=$("$candidate" -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")
        major=$("$candidate" -c "import sys; print(sys.version_info.major)")
        minor=$("$candidate" -c "import sys; print(sys.version_info.minor)")
        if [ "$major" -ge 3 ] && [ "$minor" -ge 10 ]; then
            PYTHON="$candidate"
            break
        fi
    fi
done

if [ -z "$PYTHON" ]; then
    fail "Python 3.10+ is required but not found. Install it from https://www.python.org/downloads/"
fi
ok "Python $ver ($PYTHON)"

# ── 2. Check / install uv ────────────────────────────────────────────────────

info "Checking for uv..."
if command -v uv &>/dev/null; then
    uv_ver=$(uv --version 2>/dev/null || echo "unknown")
    ok "uv found ($uv_ver)"
else
    info "uv not found — installing via pip..."
    "$PYTHON" -m pip install --user uv --quiet
    if command -v uv &>/dev/null; then
        ok "uv installed ($(uv --version))"
    else
        fail "Failed to install uv. Install manually: pip install uv"
    fi
fi

# ── 3. Regenerate scopy_api.json ──────────────────────────────────────────────

info "Regenerating API metadata from source headers..."
"$PYTHON" "$GENERATOR" --scopy-root "$REPO_ROOT" --out "$API_JSON" 2>&1 | tail -1
ok "API metadata written to scopy_api.json"

# ── 4. Sync the venv ─────────────────────────────────────────────────────────

info "Syncing dependencies..."
(cd "$SCRIPT_DIR" && uv sync --quiet)
ok "Dependencies synced"

# ── 5. Update .mcp.json ──────────────────────────────────────────────────────

info "Updating $MCP_JSON..."
"$PYTHON" -c "
import json, os, sys

mcp_path = sys.argv[1]
server_dir = sys.argv[2]

entry = {
    'command': 'uv',
    'args': ['run', '--directory', server_dir, 'scopy-mcp-server']
}

if os.path.exists(mcp_path):
    with open(mcp_path) as f:
        data = json.load(f)
else:
    data = {}

data.setdefault('mcpServers', {})['scopy'] = entry

with open(mcp_path, 'w') as f:
    json.dump(data, f, indent=2)
    f.write('\n')
" "$MCP_JSON" "tools/scopy-mcp-server"

ok ".mcp.json updated"

# ── Summary ───────────────────────────────────────────────────────────────────

echo ""
echo "────────────────────────────────────────────────"
ok "Scopy MCP server installed successfully!"
echo ""
echo "  Next steps:"
echo "    1. Restart Claude Code to pick up the new MCP config"
echo "    2. Start Scopy (any method: AppImage, build, system install)"
echo "    3. Ask Claude to interact with your Scopy instruments"
echo ""
echo "  To set a custom Scopy binary path, add to .mcp.json:"
echo '    "env": { "SCOPY_PATH": "/path/to/scopy" }'
echo "────────────────────────────────────────────────"
