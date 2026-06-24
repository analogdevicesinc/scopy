# Scopy MCP Server — Design Document

## 1. What is this?

The Scopy MCP Server is a bridge between **Claude Code** (or any MCP-compatible AI client) and **Scopy** (the ADI test/measurement GUI). It allows natural-language control of Scopy — you say *"add device 192.168.2.1 and connect to it"*, and Claude translates that into API calls executed inside Scopy's JavaScript engine.

## 2. The Problem

Scopy is a powerful tool with a rich API, but controlling it today requires either:
- Clicking through the GUI manually
- Writing JS test scripts and running them via `--script`
- Typing commands into the stdin REPL (if running from a terminal)

None of these integrate with AI assistants. There's no remote control interface — no TCP server, no REST API, no IPC mechanism.

## 3. Design Decisions

### 3.1 Why MCP?

MCP (Model Context Protocol) is the standard protocol that Claude Code uses to communicate with external tools. By building a Scopy MCP server:

- Claude can discover Scopy's capabilities automatically (tool listing)
- The server integrates natively into Claude Code's configuration (`.mcp.json`)
- No custom protocol design needed — MCP defines request/response format
- Any MCP-compatible client can use it, not just Claude Code

**Alternative considered: Custom REST API.** We rejected this because it would require building both server and client, and wouldn't integrate natively with Claude Code.

### 3.2 Why use Scopy's console (JS engine)?

The core requirement is that the MCP server **uses Scopy's console to pass functions**. This means all commands go through `ScopyJS::engine()->evaluate()` — the same path used by:
- The stdin REPL (`ScopyJS::hasText()`)
- The GUI scripting tool (`ScriptingTool::evaluateCode()`)
- CLI `--script` execution (`ScopyMainWindow_API::runScript()`)

**Why this matters:**
- **Full API access**: Every `Q_INVOKABLE` method on every registered `ApiObject` is available. This includes `scopy.*` (core), `ad936x.*` (plugin), and any future plugin APIs — automatically, without MCP server changes.
- **Battle-tested path**: This is the same evaluation pipeline used by hundreds of existing test scripts in `js/testAutomations/`. It's proven and stable.
- **No API duplication**: We don't need to create a separate RPC layer that mirrors the JS API. The JS engine IS the RPC layer.

**Alternative considered: Direct C++ function calls via a Qt RPC framework.** Rejected because it would require maintaining a separate call interface for every API function.

### 3.3 Why FIFO pipes for IPC?

We need two communication modes:

#### Attach mode (primary): Named pipes (FIFOs)
When Scopy is already running, the MCP server needs to send commands to it. We chose **FIFOs** (`/tmp/scopy_mcp_cmd` and `/tmp/scopy_mcp_rsp`) because:

- **Minimal C++ changes**: ~45 lines added to `ScopyJS`. We add a `QSocketNotifier` watching the command FIFO — the same mechanism Scopy already uses for stdin.
- **No new dependencies**: FIFOs are a POSIX primitive. No need for ZeroMQ, gRPC, or any external library.
- **Thread-safe by design**: `QSocketNotifier` fires on the main thread, which is the same thread that owns the `QJSEngine`. No locking needed.
- **Simple protocol**: Write a JS command to the cmd pipe, read `OK:<result>` or `ERROR:<message>` from the rsp pipe. One command at a time, synchronous.

**Alternatives considered:**
- **QLocalServer (Unix domain socket)**: More robust and bidirectional, but overkill for a PoC. ~50+ lines of C++ with connection management. Good upgrade path for production.
- **QTcpServer**: Even more complex, adds network concerns. Only needed if Scopy and Claude run on different machines.
- **DBus**: Heavy dependency, complex setup, not available on all platforms.

#### Launch mode (fallback): PTY (pseudo-terminal)
When no Scopy is running, the MCP server can spawn one. The challenge: Scopy's stdin REPL only activates when `isatty(STDIN_FILENO)` returns true (see `scopyjs.cpp:78`). A regular subprocess pipe is not a TTY. Solution: use Python's `pty` module to create a pseudo-terminal.

- The MCP server creates a PTY master/slave pair
- Scopy is launched with the slave as stdin → `isatty()` returns true → REPL activates
- Commands are written to the PTY master, responses read until `scopy > ` prompt appears

**Why not just always use launch mode?** Because the user wants to control an already-running Scopy instance. Launch mode creates a *new* Scopy process — it can't interact with one that's already connected to hardware.

### 3.4 Why Python + FastMCP?

- **Official SDK**: The `mcp` Python SDK is the official, maintained SDK from the MCP project (modelcontextprotocol/python-sdk). High quality, well-documented.
- **FastMCP decorator pattern**: Defining tools is as simple as `@mcp.tool()` on a Python function. The SDK handles JSON-RPC serialization, stdio transport, and protocol compliance.
- **Python is natural for scripting bridges**: `pty`, `os.mkfifo`, `select`, `subprocess` — all the IPC primitives we need are in the standard library.
- **No build step**: The MCP server is a standalone Python script. It doesn't need to be compiled with Scopy.

**Alternative considered: TypeScript MCP SDK.** Also excellent (higher benchmark score), but adds a Node.js dependency. Python is simpler for process/pipe management.

### 3.5 Why these 3 tools for the PoC?

`add_device`, `connect_device`, and `switch_tool` represent the **minimum viable workflow**: add hardware → connect → navigate to the right tool. This is the sequence every Scopy session starts with.

These were chosen because:
- They exercise the full communication path (MCP → Python → FIFO/PTY → JS engine → Qt API → UI)
- They have clear, observable effects in the Scopy GUI (device appears, connection establishes, tool switches)
- They match the demo scenario: *"Add 192.168.2.1, connect, switch to AD936X"*
- They use both return types: `addDevice` returns a string (device ID), `connectDevice`/`switchTool` return booleans

## 4. Architecture Diagram

```
┌──────────────┐     MCP (stdio)     ┌─────────────────────┐
│              │ ──────────────────── │                     │
│  Claude Code │                     │  scopy_mcp_server   │
│              │ ──────────────────── │  (Python + FastMCP) │
└──────────────┘                     └──────────┬──────────┘
                                                │
                                    ┌───────────┴───────────┐
                                    │     ScopyBridge       │
                                    │                       │
                                    │  detect_running()     │
                                    │  attach() / launch()  │
                                    │  execute(js) → str    │
                                    └───────────┬───────────┘
                                                │
                              ┌─────────────────┼─────────────────┐
                              │                                   │
                     Attach mode (FIFO)                  Launch mode (PTY)
                              │                                   │
                    /tmp/scopy_mcp_cmd                    pty master fd
                    /tmp/scopy_mcp_rsp                    (write JS, read result)
                              │                                   │
                              └─────────────────┬─────────────────┘
                                                │
                                    ┌───────────┴───────────┐
                                    │      ScopyJS          │
                                    │  (QJSEngine singleton)│
                                    │                       │
                                    │  engine()->evaluate() │
                                    └───────────┬───────────┘
                                                │
                                    ┌───────────┴───────────┐
                                    │  Registered API       │
                                    │  Objects               │
                                    │                       │
                                    │  scopy.addDevice()    │
                                    │  scopy.connectDevice()│
                                    │  scopy.switchTool()   │
                                    │  ad936x.getRx...()    │
                                    └───────────────────────┘
```

## 5. Communication Protocol

### Attach mode (FIFO pair)

```
MCP Server                          Scopy (ScopyJS)
    │                                      │
    │── open /tmp/scopy_mcp_cmd (write) ──│
    │── write: scopy.addDevice("...")\\n ──│
    │── close write fd ───────────────────│
    │                                      │── QSocketNotifier fires
    │                                      │── read command
    │                                      │── engine()->evaluate()
    │                                      │── format: "OK:device_id_123"
    │                                      │── open /tmp/scopy_mcp_rsp (write)
    │                                      │── write response + \\n
    │                                      │── close write fd
    │── open /tmp/scopy_mcp_rsp (read) ───│
    │── read response line ───────────────│
    │── parse OK:/ERROR: prefix ──────────│
    │── return to MCP client ─────────────│
```

### Launch mode (PTY)

```
MCP Server                          Scopy (stdin REPL)
    │                                      │
    │── write to pty master: ─────────────│
    │   scopy.addDevice("...")\\n          │
    │                                      │── QSocketNotifier(STDIN) fires
    │                                      │── hasText() reads line
    │                                      │── engine()->evaluate()
    │                                      │── print result to stdout
    │                                      │── print "scopy > " prompt
    │── read from pty master ─────────────│
    │── buffer until "scopy > " ──────────│
    │── extract result ───────────────────│
    │── return to MCP client ─────────────│
```

## 6. Scopy C++ Changes (Attach Mode)

The only C++ modification is in `ScopyJS` (the JS engine singleton). We add:

1. **Two FIFOs** created in `init()`: `/tmp/scopy_mcp_cmd` (commands in) and `/tmp/scopy_mcp_rsp` (results out)
2. **A `QSocketNotifier`** watching the command FIFO for incoming data
3. **`onPipeData()` slot** that reads the command, evaluates it, and writes the result to the response FIFO
4. **EOF handling**: when the writer closes, reopen the FIFO to accept the next command

This mirrors exactly how the stdin REPL works (also uses `QSocketNotifier`), just on a FIFO instead of STDIN.

## 7. What the PoC does NOT include

- **Error recovery**: If Scopy crashes, the MCP server doesn't auto-restart it
- **Concurrent commands**: One command at a time (FIFO is single-threaded)
- **Plugin-specific tools**: Only core `scopy.*` API. No `ad936x.*` tools yet (easy to add later)
- **MCP resources/prompts**: Only tools. Resources (device list, tool list) and prompts (workflows) are future work
- **Cross-machine control**: FIFO is local-only. TCP would be needed for remote

## 8. Future evolution path

1. **More tools**: Wrap any `Q_INVOKABLE` method as an MCP tool. Or add a generic `execute_js(code)` tool.
2. **QLocalServer**: Replace FIFO pair with Unix domain socket for bidirectional, multiplexed communication.
3. **MCP resources**: Expose `scopy.getDevicesName()`, `scopy.getTools()` as live-updating MCP resources.
4. **MCP prompts**: Provide workflow templates like *"Connect to PlutoSDR and configure TX at 2.4 GHz"*.
5. **Plugin auto-discovery**: When a device connects, auto-register its plugin API methods as new MCP tools.
