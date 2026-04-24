# Scopy MCP Server

An MCP (Model Context Protocol) server that lets Claude Code control [Scopy](https://github.com/analogdevicesinc/scopy) by sending JavaScript commands through Scopy's built-in console (QJSEngine).

Say *"add device 192.168.2.1, connect to it, and switch to the AD936X tool"* and Claude will do it.

```
Claude Code ──(MCP stdio)──> scopy-mcp-server ──(FIFO)──> Scopy JS engine
                                               <──(FIFO)──
```

## Prerequisites

- **Linux** (uses Unix named pipes)
- **Python 3.10+**
- **Scopy** installed — [download here](https://github.com/analogdevicesinc/scopy/releases)
- **Claude Code** (CLI, desktop, or IDE extension)

## Install

**Prerequisites:** Python 3.10+, Scopy installed, Claude Code

Clone the Scopy repo (or download the [source zip](https://github.com/analogdevicesinc/scopy/archive/refs/heads/main.zip)):

```bash
git clone https://github.com/analogdevicesinc/scopy
cd scopy/tools/scopy-mcp-server
bash install.sh
```

The script checks Python, detects your Scopy binary, installs the package, and prints the
`.mcp.json` snippet to paste into Claude Code. That's it.

## Configure Claude Code

Add to your `.mcp.json` (project root or `~/.claude/.mcp.json` for global):

```json
{
  "mcpServers": {
    "scopy": {
      "command": "scopy-mcp-server"
    }
  }
}
```

If Scopy is not on your `$PATH`, set `SCOPY_PATH`:

```json
{
  "mcpServers": {
    "scopy": {
      "command": "scopy-mcp-server",
      "env": {
        "SCOPY_PATH": "/path/to/scopy"
      }
    }
  }
}
```

### Allow all Scopy tools without prompting (optional)

By default Claude Code asks for permission on each MCP tool call. To approve all Scopy tools at once, add to `.claude/settings.json`:

```json
{
  "permissions": {
    "allow": [
      "mcp__scopy__*"
    ]
  }
}
```

Restart Claude Code after changing `.mcp.json`.

## How it works

The server always communicates with Scopy via named pipes (`/tmp/scopy_mcp_cmd`, `/tmp/scopy_mcp_rsp`). Scopy creates these FIFOs automatically on startup.

- **Attach mode** — connects to an already-running Scopy instance. The preferred workflow.
- **Launch mode** — spawns a new Scopy process via `start_scopy()`, then waits for the FIFOs to appear before sending any commands. Scopy's stdout/stderr are discarded to keep responses clean.

## Available tools

### Core tools

| Tool | Description |
|------|-------------|
| `start_scopy()` | Launch a new Scopy session |
| `add_device(uri)` | Add a device by URI (e.g. `192.168.2.1`) |
| `connect_device(device_id)` | Connect to an added device |
| `disconnect_device(device_id)` | Disconnect from a device |
| `switch_tool(tool_name)` | Switch to a named instrument tool |

### Per-plugin tools (auto-generated)

For each Scopy API object (`ad936x`, `adrv9002`, `regmap`, `datalogger`, etc.), these tools are registered:

| Tool pattern | Description |
|-------------|-------------|
| `{obj}_list()` | List all available methods/parameters |
| `{obj}_read(key, channel)` | Read a parameter value |
| `{obj}_write(key, value, channel)` | Write a parameter value |
| `{obj}_call(method, arg1, ...)` | Call any method on the object |

### Usage examples

```
"List all ad936x parameters"           → ad936x_list()
"Read the RX LO frequency"            → ad936x_read("rx_lo")
"Set TX gain on channel 0 to -10 dB"  → ad936x_write("tx_hardware_gain", "-10", channel=0)
"Add device 192.168.2.1 and connect"  → add_device + connect_device
```

## Troubleshooting

| Problem | Fix |
|---------|-----|
| `No running Scopy instance found` | Start Scopy first, or call `start_scopy()` |
| `Scopy binary not found` | Set `SCOPY_PATH` in `.mcp.json` env, or add `scopy` to your PATH |
| `Scopy did not create the MCP pipe within 30s` | Scopy took too long to start — check it launches correctly |
| `Scopy did not respond within 10s` | Check Scopy is responsive, try a simpler command |
| MCP server not showing in Claude Code | Check `.mcp.json` syntax, restart Claude Code |
| `Stale Scopy pipe detected` | Scopy exited uncleanly — restart Scopy |

## For Scopy developers

### Regenerating the API metadata

The `scopy_api.json` file is generated from Scopy's `*_api.h` headers. Re-run when the API changes:

```bash
python3 tools/scopy-mcp-server/generate_api_tools.py \
  --scopy-root . \
  --out tools/scopy-mcp-server/scopy_mcp_server/scopy_api.json
```

### Running from source (development)

```bash
cd tools/scopy-mcp-server
pip install -e .
scopy-mcp-server
```

Or using the repo's `.mcp.json` with `uv run`:

```json
{
  "mcpServers": {
    "scopy": {
      "command": "uv",
      "args": ["run", "--directory", "tools/scopy-mcp-server", "scopy-mcp-server"],
      "env": {
        "SCOPY_PATH": "/path/to/your/build/scopy"
      }
    }
  }
}
```

### File structure

```
tools/scopy-mcp-server/
  scopy_mcp_server/           # Python package (pip-installable)
    __init__.py
    server.py                 # MCP server — loads API JSON, registers tools
    bridge.py                 # Communication bridge (FIFO-only)
    scopy_api.json            # Generated API metadata (bundled in wheel)
  generate_api_tools.py       # Dev tool — parses *_api.h → scopy_api.json
  pyproject.toml
  README.md
```

See [design.md](design.md) for architecture details.
