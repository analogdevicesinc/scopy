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

| Tool | Description | Example |
|------|-------------|---------|
| `add_device(uri)` | Add a device by URI | `add_device("192.168.2.1")` |
| `connect_device(device_id)` | Connect to an added device | `connect_device("0")` |
| `switch_tool(tool_name)` | Switch to a named tool | `switch_tool("AD936X")` |

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

### Step 4: Restart Claude Code

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

## File structure

```
tools/scopy-mcp-server/
├── scopy_mcp_server.py   # MCP server (FastMCP, 3 tools)
├── scopy_bridge.py       # Communication bridge (FIFO + PTY modes)
├── pyproject.toml        # Python package definition (used by uv)
├── requirements.txt      # Python dependencies (fallback for pip users)
├── plan.md               # Implementation plan
├── design.md             # Design decisions and architecture
└── README.md             # This file
```

## Architecture details

See [design.md](design.md) for a full explanation of the design choices (why MCP, why FIFOs, why Python, etc.).
