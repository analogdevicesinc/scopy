"""
Scopy MCP Server

An MCP server that lets Claude Code control Scopy by sending JavaScript
commands through Scopy's console (QJSEngine). Communicates with Scopy
via named pipes (attach mode) or PTY (launch mode).

The bridge is initialized lazily — Scopy is never started automatically.
If no Scopy is running when a tool is called, the user is notified and
given the option to call start_scopy() explicitly.

Environment variables:
    SCOPY_PATH  Path to the Scopy binary (default: "scopy")
"""

import atexit
import json
import logging
import os
import pathlib
import sys

from mcp.server.fastmcp import FastMCP

from scopy_bridge import ScopyBridge

logging.basicConfig(level=logging.INFO, stream=sys.stderr)
logger = logging.getLogger("scopy-mcp")

mcp = FastMCP("Scopy MCP Server")
bridge = ScopyBridge()

_API = json.loads((pathlib.Path(__file__).parent / "scopy_api.json").read_text())

_NO_SCOPY_MSG = (
    "No running Scopy instance found. "
    "Please start Scopy manually, or call start_scopy() to launch it."
)


def _ensure_connected() -> str | None:
    """Lazily attach to a running Scopy. Returns an error string if not available."""
    if bridge.is_ready():
        return None
    if bridge.detect_running_scopy():
        bridge.attach()
        logger.info("Attached to running Scopy instance.")
        return None
    return _NO_SCOPY_MSG


def _run_tool(js_code: str) -> str:
    """Run a JS command, catching stale-pipe and timeout errors as user-friendly strings."""
    try:
        return bridge.execute(js_code)
    except RuntimeError as e:
        msg = str(e)
        if "stale FIFO" in msg or "stale pipe" in msg.lower():
            return (
                "Stale Scopy pipe detected — Scopy exited without cleaning up. "
                "Please start Scopy again, or call start_scopy()."
            )
        return f"Scopy error: {msg}"
    except TimeoutError:
        return (
            "Scopy did not respond within the timeout. "
            "Check that Scopy is running and the MCP pipe listener is active "
            "(look for 'MCP pipe listener active' in Scopy's log)."
        )


def _build_js_call(obj: str, method: str, args: list[str]) -> str:
    """Build a JS function call with type-aware argument quoting."""
    js_args = []
    for a in args:
        if a in ("true", "false"):
            js_args.append(a)
        else:
            try:
                int(a)
                js_args.append(a)
            except ValueError:
                try:
                    float(a)
                    js_args.append(a)
                except ValueError:
                    js_args.append(f'"{a}"')
    return f"{obj}.{method}({', '.join(js_args)})"


# ---------------------------------------------------------------------------
# Named core tools (convenience wrappers for common scopy operations)
# ---------------------------------------------------------------------------

@mcp.tool()
def start_scopy() -> str:
    """Launch a new Scopy session.

    Only call this if Scopy is not already running. The user must explicitly
    request this — the server never auto-launches Scopy on its own.

    Returns:
        A message confirming Scopy was started, or an error description.
    """
    if bridge.is_ready():
        return "Scopy is already running and connected."

    scopy_path = os.environ.get("SCOPY_PATH", "scopy")
    if not os.path.isfile(scopy_path):
        return (
            f"Scopy binary not found at '{scopy_path}'. "
            "Set the SCOPY_PATH environment variable in .mcp.json to the correct path."
        )

    logger.info(f"Launching Scopy from {scopy_path}...")
    try:
        bridge.launch(scopy_path)
        return "Scopy launched successfully and ready to use."
    except Exception as e:
        logger.error(f"Failed to launch Scopy: {e}")
        return f"Failed to launch Scopy: {e}"


@mcp.tool()
def add_device(uri: str, category: str = "iio") -> str:
    """Add a device to Scopy by URI.

    Args:
        uri: The device URI (e.g., '192.168.2.1' or 'ip:192.168.2.1')
        category: Device category, defaults to 'iio'

    Returns:
        The device ID string assigned by Scopy, or an error message.
    """
    if err := _ensure_connected():
        return err
    js = f'scopy.addDevice("{uri}", "{category}")'
    result = _run_tool(js)
    logger.info(f"add_device({uri}): {result}")
    return result


@mcp.tool()
def connect_device(device_id: str) -> str:
    """Connect to a device that has been added to Scopy.

    Args:
        device_id: The device ID returned by add_device, or a 0-based index.

    Returns:
        'true' if connection succeeded, 'false' otherwise, or an error message.
    """
    if err := _ensure_connected():
        return err
    try:
        idx = int(device_id)
        js = f"scopy.connectDevice({idx})"
    except ValueError:
        js = f'scopy.connectDevice("{device_id}")'
    result = _run_tool(js)
    logger.info(f"connect_device({device_id}): {result}")
    return result


@mcp.tool()
def disconnect_device(device_id: str = "") -> str:
    """Disconnect from a device in Scopy.

    Args:
        device_id: The device ID string, or a 0-based index. If omitted, disconnects
                   the currently active device.

    Returns:
        'true' if disconnection succeeded, 'false' otherwise, or an error message.
    """
    if err := _ensure_connected():
        return err
    if not device_id:
        js = "scopy.disconnectDevice()"
    else:
        try:
            idx = int(device_id)
            js = f"scopy.disconnectDevice({idx})"
        except ValueError:
            js = f'scopy.disconnectDevice("{device_id}")'
    result = _run_tool(js)
    logger.info(f"disconnect_device({device_id}): {result}")
    return result


@mcp.tool()
def switch_tool(tool_name: str, device_id: str = "") -> str:
    """Switch to a specific tool in Scopy.

    Args:
        tool_name: Name of the tool to switch to. Use scopy_call with getTools() to
                   discover exact tool names. Common names include 'AD936X', 'Oscilloscope',
                   'Signal Generator', 'Voltmeter', 'Power Supply', 'Spectrum Analyzer',
                   'Digital I/O', 'Calibration', 'Debugger', 'Register Map', 'Data Logger'.
        device_id: Optional device ID. If omitted, uses the currently active device.

    Returns:
        'true' if switch succeeded, 'false' otherwise. If 'false', call scopy_call
        with method getTools() to see the exact available tool names.
    """
    if err := _ensure_connected():
        return err
    if device_id:
        js = f'scopy.switchTool("{device_id}", "{tool_name}")'
    else:
        js = f'scopy.switchTool("{tool_name}")'
    result = _run_tool(js)
    if result == "false":
        tools = _run_tool("scopy.getTools()")
        result = f"false — tool '{tool_name}' not found. Available tools: {tools}"
    logger.info(f"switch_tool({tool_name}): {result}")
    return result


# ---------------------------------------------------------------------------
# Plugin-level discovery tools — one per plugin, lists all JS objects it exposes
# ---------------------------------------------------------------------------

def _make_plugin_list_tool(plugin: str, obj_names: list[str]):
    @mcp.tool(name=f"{plugin}_list_tools")
    def list_tools() -> str:
        f"""List all JS API objects exposed by the '{plugin}' Scopy plugin.

        Returns a JSON list of object names. Each object has a _call tool
        (e.g. {obj_names[0]}_call) for invoking its methods.
        """
        return json.dumps(obj_names)

    return list_tools


for _plugin, _obj_names in _API.get("plugins", {}).items():
    if _obj_names:
        _make_plugin_list_tool(_plugin, _obj_names)


# ---------------------------------------------------------------------------
# Per-object call tools — one tool per JS API object, full method list
# ---------------------------------------------------------------------------

def _format_method_sig(m: dict) -> str:
    """Format a method entry from the JSON into a readable signature line."""
    args_parts = []
    for a in m["args"]:
        s = a["name"]
        if "default" in a:
            s += f"={a['default']}"
        args_parts.append(s)
    arg_str = ", ".join(args_parts)
    return f"  {m['name']}({arg_str}) -> {m['returns']}"


def _make_object_call_tool(obj: str, methods: list[dict]):
    """Create a single _call tool for a JS API object with full method list in description."""
    # Deduplicate method names for validation (overloads share a name)
    valid_names = {m["name"] for m in methods}

    desc_lines = [f"Call a method on the '{obj}' Scopy API object.", "",
                  "Available methods:"]
    for m in methods:
        desc_lines.append(_format_method_sig(m))

    docstring = "\n".join(desc_lines)

    @mcp.tool(name=f"{obj}_call")
    def call_method(method: str,
                    arg1: str = "", arg2: str = "",
                    arg3: str = "", arg4: str = "") -> str:
        """placeholder"""
        if err := _ensure_connected():
            return err
        if method not in valid_names:
            return (
                f"Unknown method '{method}' on {obj}. "
                f"Available: {sorted(valid_names)}"
            )
        args = [a for a in (arg1, arg2, arg3, arg4) if a]
        js = _build_js_call(obj, method, args)
        result = _run_tool(js)
        logger.info(f"{obj}.{method}({', '.join(args)}): {result}")
        return result

    call_method.__doc__ = docstring
    return call_method


def _make_object_list_tool(obj: str, methods: list[dict]):
    """Create a _list tool that returns the full method inventory as JSON."""

    @mcp.tool(name=f"{obj}_list")
    def list_methods() -> str:
        f"""List all available methods on the '{obj}' Scopy API object.

        Returns a JSON array of method signatures with name, arguments, and return type.
        """
        return json.dumps([
            {
                "name": m["name"],
                "args": [
                    a["name"] + ("=" + a["default"] if "default" in a else "")
                    for a in m["args"]
                ],
                "returns": m["returns"],
            }
            for m in methods
        ], indent=2)

    return list_methods


# Register per-object tools for all objects (including scopy)
for _obj, _meta in _API.get("objects", {}).items():
    _methods = _meta.get("methods", [])
    if not _methods:
        continue
    _make_object_call_tool(_obj, _methods)
    _make_object_list_tool(_obj, _methods)


# ---------------------------------------------------------------------------

def _cleanup():
    bridge.close()


atexit.register(_cleanup)


def main():
    """Entry point for the MCP server. Does NOT connect to or launch Scopy."""
    mcp.run(transport="stdio")


if __name__ == "__main__":
    main()
