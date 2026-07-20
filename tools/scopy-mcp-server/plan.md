# Scopy MCP Server — Implementation Plan

## Context

Scopy has a built-in JavaScript engine (`QJSEngine` via the `ScopyJS` singleton) that exposes all API objects (`scopy.*`, `ad936x.*`, etc.) to JS. It also has a **stdin REPL** (`ScopyJS::hasText()`) that evaluates JS commands typed into the terminal. Currently, there is **no remote control mechanism** — all control is local (stdin, GUI, or CLI `--script` flag).

We want to build an **MCP server** that lets Claude Code control a running Scopy instance by sending JS commands through Scopy's console. This enables natural-language control of test equipment: *"Add device 192.168.2.1, connect, and switch to the AD936X tool."*

## Architecture

```
Claude Code ──(MCP stdio)──> scopy_mcp_server.py ──(FIFO or PTY)──> Scopy JS engine
                                                  <──(FIFO or PTY)──
```

**Two modes:**
1. **Attach mode** — detect a running Scopy via `/tmp/scopy_mcp_cmd` FIFO, connect to it
2. **Launch mode** — if no Scopy is running, notify user and optionally spawn one with a PTY

## PoC Scope — 3 MCP Tools

| MCP Tool | JS Command | Returns |
|----------|-----------|---------|
| `add_device(uri)` | `scopy.addDevice("192.168.2.1")` | Device ID string |
| `connect_device(device_id)` | `scopy.connectDevice(deviceId)` | `true`/`false` |
| `switch_tool(tool_name)` | `scopy.switchTool("AD936X")` | `true`/`false` |

## Implementation Steps

### Phase 1: C++ — Add FIFO listener to ScopyJS (enables attach mode)

**Files to modify:**
- `pluginbase/include/pluginbase/scopyjs.h` — add `m_pipeFd`, `m_pipeNotifier`, `initMcpPipe()`, `onPipeData()` slot
- `pluginbase/src/scopyjs.cpp` — implement FIFO creation, watcher, evaluation, response writing

**What it does:**
- On `init()`, create two FIFOs: `/tmp/scopy_mcp_cmd` (commands in) and `/tmp/scopy_mcp_rsp` (results out)
- `QSocketNotifier` watches the command FIFO
- `onPipeData()` reads the JS command, calls `m_engine.evaluate()`, writes `OK:<result>` or `ERROR:<message>` to the response FIFO
- Cleanup: unlink FIFOs and close fds in destructor
- ~45 lines of new C++

### Phase 2: Python MCP server

**New files to create:**
```
tools/scopy-mcp-server/
├── scopy_mcp_server.py    # FastMCP server with 3 tools
├── scopy_bridge.py        # Communication bridge (FIFO + PTY modes)
├── requirements.txt       # mcp>=1.0.0
```

**`scopy_bridge.py`** — `ScopyBridge` class:
- `detect_running_scopy()` → checks if `/tmp/scopy_mcp_cmd` exists as a FIFO
- `attach()` → sets mode to use FIFO communication
- `launch(scopy_path)` → spawns Scopy with PTY via `pty.openpty()` + `subprocess.Popen`, waits for `scopy > ` prompt
- `execute(js_code) → str` — unified interface, delegates to `_execute_via_fifo()` or `_execute_via_pty()`

**`scopy_mcp_server.py`** — FastMCP server:
- On startup: check for running Scopy → attach or prompt to launch
- 3 `@mcp.tool()` decorated functions: `add_device`, `connect_device`, `switch_tool`
- Each tool builds a JS string and calls `bridge.execute(js)`
- Runs with `transport="stdio"` (standard MCP transport for Claude Code)

### Phase 3: Configuration and testing

**Claude Code config** (`.mcp.json` or `~/.claude/settings.json`):
```json
{
  "mcpServers": {
    "scopy": {
      "command": "python3",
      "args": ["tools/scopy-mcp-server/scopy_mcp_server.py"],
      "env": {
        "SCOPY_PATH": "/path/to/scopy/build/scopy"
      }
    }
  }
}
```

**Demo test:** Say to Claude: *"Add device 192.168.2.1, connect to it, then switch to the device tool"*

## Key Technical Details

### FIFO communication protocol (attach mode)
- MCP server opens `/tmp/scopy_mcp_cmd` for write, sends JS + `\n`, closes
- MCP server opens `/tmp/scopy_mcp_rsp` for read (blocks until Scopy responds)
- Scopy writes `OK:<value>\n` or `ERROR:<message>\n`
- 10-second timeout on response read

### PTY communication (launch mode)
- `pty.openpty()` creates master/slave pair
- Scopy launched with slave fd as stdin/stdout → `isatty()` returns true → REPL activates
- Parse output by reading until `scopy > ` prompt
- Suppress log noise: send `suppressScopyMessages(true)` as first command

### Thread safety
- `onPipeData()` fires on Qt's main thread via `QSocketNotifier` → same thread as `QJSEngine` → safe

### FIFO EOF handling
- When MCP server closes write end, Scopy gets EOF → `onPipeData()` reopens the FIFO fd to accept next command

## Critical files to reference

| File | Purpose |
|------|---------|
| `pluginbase/include/pluginbase/scopyjs.h` | ScopyJS header — modify for FIFO |
| `pluginbase/src/scopyjs.cpp` | ScopyJS impl — add FIFO watcher |
| `core/include/core/scopymainwindow_api.h` | API signatures (addDevice, connectDevice, switchTool) |
| `core/src/scopymainwindow_api.cpp` | API implementation |
| `pluginbase/include/pluginbase/apiobject.h` | Base class for all API objects |
| `js/scopycommon.js` | Reference JS usage patterns |

## Verification

1. **Build Scopy** with the FIFO changes, launch it
2. **Manual FIFO test**: `echo 'scopy.getDevicesName()' > /tmp/scopy_mcp_cmd && cat /tmp/scopy_mcp_rsp` — should return `OK:[]`
3. **MCP tool listing**: `echo '{"jsonrpc":"2.0","method":"tools/list","id":1}' | python3 scopy_mcp_server.py` — should list 3 tools
4. **End-to-end via Claude Code**: Start Scopy, restart Claude Code, ask it to add+connect+switch — verify Scopy UI responds
5. **Launch mode**: Stop Scopy, ask Claude to add a device — MCP server should report no running instance and offer to start one
