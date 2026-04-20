# Scopy MCP Server

An MCP (Model Context Protocol) server that lets Claude Code control Scopy by sending JavaScript commands through Scopy's built-in console (QJSEngine).

Say *"add device 192.168.2.1, connect to it, and switch to the AD936X tool"* and Claude will do it.

## How it works

```
Claude Code ──(MCP stdio)──> scopy_mcp_server.py ──(FIFO or PTY)──> Scopy JS engine
                                                  <──(FIFO or PTY)──
```

The MCP server translates tool calls into JavaScript commands (e.g. `scopy.addDevice("192.168.2.1")`) and sends them to Scopy's QJSEngine for evaluation. Two communication modes are supported:

- **Attach mode** — connects to an already-running Scopy instance via named pipes (`/tmp/scopy_mcp_cmd`, `/tmp/scopy_mcp_rsp`)
- **Launch mode** — spawns a new Scopy process with a PTY so the stdin REPL activates

## Available tools

### Core tools (always available)

| Tool | Description | Example |
|------|-------------|---------|
| `start_scopy()` | Launch a new Scopy session | `start_scopy()` |
| `add_device(uri)` | Add a device by URI | `add_device("192.168.2.1")` |
| `connect_device(device_id)` | Connect to an added device | `connect_device("0")` |
| `switch_tool(tool_name)` | Switch to a named tool | `switch_tool("AD936X")` |

### Per-plugin generic tools (auto-generated from `scopy_api.json`)

For each Scopy API object (`ad936x`, `ad936x_advanced`, `adrv9002`, `fmcomms5`, `iioExplorer`, `pqm`, `regmap`, `swiot`, `ad9084`, `adrv9009`, `fmcomms5_advanced`), three tools are registered:

| Tool pattern | Description |
|-------------|-------------|
| `{obj}_list()` | List all valid parameter keys for this object |
| `{obj}_read(key, channel=-1)` | Read a parameter. Pass `channel` ≥ 0 for channel-indexed parameters |
| `{obj}_write(key, value, channel=-1)` | Write a parameter. Pass `channel` ≥ 0 for channel-indexed parameters |

**Example usage:**
```
"List all ad936x parameters"      → ad936x_list()
"Read the rx_lo from ad936x"      → ad936x_read("rx_lo")
"Set tx gain on adrv9002 ch0"     → adrv9002_write("tx_hardware_gain", "30", channel=0)
```

## Prerequisites

- **Python 3.10+**
- **[uv](https://docs.astral.sh/uv/)** — fast Python package manager (handles dependencies automatically)
- **Scopy** built with the MCP FIFO listener (see Step 2 below)
- **Claude Code** (CLI, desktop, or IDE extension)

## Installation

### Step 1: Install uv

`uv` manages Python dependencies automatically — no manual `pip install` or virtual environments needed.

```bash
# Linux / macOS
curl -LsSf https://astral.sh/uv/install.sh | sh

# Or with pip (if you prefer)
pip install uv

# Or with Homebrew (macOS)
brew install uv
```

Verify it works:

```bash
uv --version
```

### Step 2: Rebuild Scopy with MCP pipe support

The MCP server communicates with Scopy through named pipes (FIFOs). This requires a small addition to `ScopyJS` (already included in this branch).

**What was changed:**
- `pluginbase/include/pluginbase/scopyjs.h` — added FIFO members and `onPipeData()` slot
- `pluginbase/src/scopyjs.cpp` — added `initMcpPipe()`, `onPipeData()`, `cleanupMcpPipe()`

Rebuild Scopy as usual:

```bash
# Qt5 (Desktop-Debug)
cd build/Desktop-Debug
cmake --build . -j$(nproc)

```

After rebuilding, when Scopy starts it will create `/tmp/scopy_mcp_cmd` and `/tmp/scopy_mcp_rsp` automatically. You should see this log message:

```
Scopy_JS: MCP pipe listener active on /tmp/scopy_mcp_cmd
```

### Step 3: Configure Claude Code

A `.mcp.json` file is included in the project root. Edit the `SCOPY_PATH` environment variable to match your Scopy build location:

```json
{
  "mcpServers": {
    "scopy": {
      "command": "uv",
      "args": ["run", "--directory", "tools/scopy-mcp-server", "scopy-mcp-server"],
      "env": {
        "SCOPY_PATH": "/path/to/your/scopy/build/scopy"
      }
    }
  }
}
```

**How this works:** When Claude Code starts, it runs `uv run --directory tools/scopy-mcp-server scopy-mcp-server`. `uv` automatically:
1. Creates an isolated virtual environment (cached, reused on subsequent runs)
2. Installs the `mcp` dependency from `pyproject.toml`
3. Runs the `scopy-mcp-server` entry point

No manual `pip install` or `venv` management needed.

### Step 4: Allow all Scopy MCP tool calls without prompting (optional)

By default, Claude Code asks for permission on every MCP tool call. With many Scopy tools registered, this means a confirmation prompt for each `ad936x_read`, `adrv9002_write`, etc.

To approve all Scopy MCP tools in advance, create (or edit) `.claude/settings.json` in the **project root** and add the `mcp__scopy__*` rule:

```json
{
  "permissions": {
    "allow": [
      "mcp__scopy__*"
    ]
  }
}
```

The pattern `mcp__scopy__*` matches every tool exposed by the `scopy` MCP server. If you already have other entries in `permissions.allow`, just add this line to the existing array.

> **Note:** The MCP protocol has no mechanism for a server to request blanket trust — this must be configured in Claude Code's settings. It cannot be done from inside the MCP server itself.

If you prefer to approve tools globally (across all projects), add the same rule to `~/.claude/settings.json` instead.

### Step 5: Restart Claude Code

Claude Code reads `.mcp.json` on startup. Restart it to pick up the new server:

```bash
# If using the CLI
claude

# Or restart the IDE extension / desktop app
```

## Usage

### Quick test (manual, no Claude)

Start Scopy, then in another terminal:

```bash
# Send a command
echo 'scopy.getDevicesName()' > /tmp/scopy_mcp_cmd

# Read the response
cat /tmp/scopy_mcp_rsp
# Expected output: OK:
```

### Run the MCP server standalone (for debugging)

```bash
cd tools/scopy-mcp-server
uv run scopy-mcp-server
```

This will try to attach to a running Scopy or launch a new one.

### With Claude Code

Start Scopy first, then ask Claude:

> "Add device 192.168.2.1, connect to it, then switch to the AD936X tool"

Claude will call the MCP tools in sequence:
1. `add_device(uri="192.168.2.1")` → returns device ID
2. `connect_device(device_id=<id>)` → returns `true`
3. `switch_tool(tool_name="AD936X")` → returns `true`

You will see the Scopy UI respond in real time.

## Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| `No running Scopy instance found` | Scopy is not running or was built without FIFO support | Start Scopy (rebuilt with the C++ changes) |
| `/tmp/scopy_mcp_cmd` does not exist | Scopy didn't create the pipe | Check the build includes the `scopyjs.cpp` changes, look for the log message on startup |
| `Scopy did not respond within 10s` | Command is hanging or Scopy is busy | Check Scopy is responsive in the GUI, try a simpler command |
| `uv: command not found` | uv is not installed | Install uv: `curl -LsSf https://astral.sh/uv/install.sh \| sh` |
| MCP server not showing in Claude Code | `.mcp.json` not found or malformed | Ensure `.mcp.json` is in the project root, restart Claude Code |
| `Scopy JS error: ...` | The JavaScript command failed inside Scopy | Check the error message — usually a wrong tool name or invalid device URI |

## Regenerating the API library

The `scopy_api.json` file is generated from Scopy's `*_api.h` header files. Re-run the generator whenever the Scopy API changes (new methods added, removed, or renamed):

```bash
# From the Scopy repo root
python3 tools/scopy-mcp-server/generate_api_tools.py \
  --scopy-root . \
  --out tools/scopy-mcp-server/scopy_api.json
```

The generator will print a summary of how many keys and special methods it found per object. Review the output, then commit both the generator and the updated JSON.

Restart Claude Code after regenerating so it picks up the new tool list.

## File structure

```
tools/scopy-mcp-server/
├── scopy_mcp_server.py      # MCP server — loads JSON, registers all tools
├── scopy_bridge.py          # Communication bridge (FIFO + PTY modes)
├── generate_api_tools.py    # Generator script — parses *_api.h → scopy_api.json
├── scopy_api.json           # Generated API metadata (committed, re-run generator to update)
├── pyproject.toml           # Python package definition (used by uv)
├── requirements.txt         # Python dependencies (fallback for pip users)
├── plan.md                  # Implementation plan
├── design.md                # Design decisions and architecture
└── README.md                # This file
```

## Architecture details

See [design.md](design.md) for a full explanation of the design choices (why MCP, why FIFOs, why Python, etc.).
