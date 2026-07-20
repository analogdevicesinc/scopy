# Scopy MCP Server

An MCP (Model Context Protocol) server that lets Claude Code control [Scopy](https://github.com/analogdevicesinc/scopy) by sending JavaScript commands through Scopy's built-in console (QJSEngine).

Say *"add device 192.168.2.1, connect to it, and switch to the AD936X tool"* and Claude will do it.

```
Claude Code ──(MCP stdio)──> scopy-mcp-server ──(Unix socket / named pipe)──> Scopy JS engine
                                               <──(Unix socket / named pipe)──
```

## Prerequisites

- **Python 3.10+**
- **Scopy** installed — [download here](https://github.com/analogdevicesinc/scopy/releases)
- **Claude Code** (CLI, desktop, or IDE extension)

## Install

### Option A: Download the standalone package (Scopy app users)

Download `scopy-mcp-server.zip` from the latest [CI build artifacts](https://github.com/analogdevicesinc/scopy/actions/workflows/mcp-server-package.yml), then:

**Linux / macOS:**
```bash
unzip scopy-mcp-server.zip
cd scopy-mcp-server
python3 setup.py
```

**Windows:**
```cmd
unzip scopy-mcp-server.zip
cd scopy-mcp-server
python setup.py
```

The script installs dependencies (tries [uv](https://docs.astral.sh/uv/), falls back to pip), including the Windows-only `pywin32` package on Windows, and prints the `.mcp.json` snippet to paste into Claude Code.

### Option B: From source (developers)

**Linux / macOS:**
```bash
git clone https://github.com/analogdevicesinc/scopy
cd scopy/tools/scopy-mcp-server
bash install.sh
```

**Windows:**
```cmd
git clone https://github.com/analogdevicesinc/scopy
cd scopy\tools\scopy-mcp-server
python setup.py
```

This regenerates the API metadata from Scopy headers, installs dependencies, and updates the repo's `.mcp.json`.

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

The server communicates with Scopy in two modes:

- **Attach mode** — connects to an already-running Scopy via its `QLocalServer` socket (`/tmp/scopy_mcp` on Linux/macOS, `\\.\pipe\scopy_mcp` on Windows). Scopy creates this automatically on startup when built with MCP support.
- **Launch mode** — spawns a new Scopy process and waits for its MCP server socket to appear. Use the `start_scopy()` tool.

Each JS command is a fresh connection: connect → send one line → read `OK:<result>` or `ERROR:<msg>` → close.

### Windows support

On Windows, the bridge uses the Win32 named-pipe API instead of Unix domain sockets. The `pywin32` package provides this:

```cmd
pip install .[windows]
```

Or with uv:
```cmd
uv sync --extra windows
```

The `setup.py` installer handles this automatically on Windows.

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
| `No running Scopy instance found` | Start Scopy first (must be built with MCP support), or call `start_scopy()` |
| `Scopy binary not found` | Set `SCOPY_PATH` in `.mcp.json` env, or add `scopy` to your PATH |
| `Scopy did not respond within 10s` | Check Scopy is responsive, try a simpler command |
| MCP server not showing in Claude Code | Check `.mcp.json` syntax, restart Claude Code |
| `Cannot connect to Scopy` | Scopy exited — restart it, or call `start_scopy()` |
| `error: externally-managed-environment` | Your system Python blocks global pip installs. Install uv and re-run `setup.py`: `curl -LsSf https://astral.sh/uv/install.sh \| sh`. For more information about this check the [official documentation](https://packaging.python.org/en/latest/specifications/externally-managed-environments/#externally-managed-environments). |

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
    bridge.py                 # Communication bridge (QLocalSocket, cross-platform)
    scopy_api.json            # Generated API metadata (bundled in wheel)
  generate_api_tools.py       # Dev tool — parses *_api.h → scopy_api.json
  pyproject.toml
  README.md
```

See [design.md](design.md) for architecture details.
