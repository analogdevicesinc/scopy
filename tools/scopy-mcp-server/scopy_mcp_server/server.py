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
import shutil
import sys

from mcp.server.fastmcp import FastMCP

from scopy_mcp_server.bridge import ScopyBridge

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
        js_args.append(_quote_js_value(a))
    return f"{obj}.{method}({', '.join(js_args)})"


def _quote_js_value(value: str) -> str:
    """Quote a value for JavaScript: booleans and numbers unquoted, strings quoted."""
    if value in ("true", "false"):
        return value
    try:
        int(value)
        return value
    except ValueError:
        pass
    try:
        float(value)
        return value
    except ValueError:
        pass
    return f'"{value}"'


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
    resolved = shutil.which(scopy_path)
    if resolved is None:
        return (
            f"Scopy binary not found at '{scopy_path}'. "
            "Set the SCOPY_PATH environment variable in .mcp.json, "
            "or ensure 'scopy' is on your PATH."
        )
    scopy_path = resolved

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


def _make_object_list_tool(obj: str, methods: list[dict], properties: list[dict] | None = None):
    """Create a _list tool that returns the full method and property inventory as JSON."""

    @mcp.tool(name=f"{obj}_list")
    def list_api() -> str:
        f"""List all available methods and properties on the '{obj}' Scopy API object.

        Returns a JSON object with 'methods' and 'properties' arrays.
        """
        result = {}
        if methods:
            result["methods"] = [
                {
                    "name": m["name"],
                    "args": [
                        a["name"] + ("=" + a["default"] if "default" in a else "")
                        for a in m["args"]
                    ],
                    "returns": m["returns"],
                }
                for m in methods
            ]
        if properties:
            result["properties"] = [
                {
                    "name": p["name"],
                    "type": p["type"],
                    "writable": "write" in p,
                    "is_list": p.get("is_list", False),
                }
                for p in properties
            ]
        return json.dumps(result, indent=2)

    return list_api


# Register per-object tools for all objects (including scopy)
for _obj, _meta in _API.get("objects", {}).items():
    _methods = _meta.get("methods", [])
    if _methods:
        _make_object_call_tool(_obj, _methods)
    _props = _meta.get("properties", [])
    if _methods or _props:
        _make_object_list_tool(_obj, _methods, _props)


# ---------------------------------------------------------------------------
# Property read/write tools — for Q_PROPERTY-based APIs (M2K instruments)
# ---------------------------------------------------------------------------

def _format_prop_line(p: dict) -> str:
    """Format a property for the tool description."""
    rw = "read/write" if "write" in p else "read-only"
    suffix = f" (list, per-channel)" if p.get("is_list") else ""
    return f"  {p['name']} ({p['type']}) [{rw}]{suffix}"


def _build_js_path(obj: str, meta: dict) -> str:
    """Build the JS object path, handling nested objects like osc.channels[idx]."""
    nested = meta.get("nested")
    if nested:
        return f"{nested['parent']}.{nested['parent_property']}"
    return obj


def _make_property_read_tool(obj: str, properties: list[dict], meta: dict):
    """Create a _read tool for reading Q_PROPERTY values."""
    valid_names = {p["name"] for p in properties}
    nested = meta.get("nested")

    desc_lines = [f"Read a property from the '{obj}' Scopy API object.", ""]
    if nested:
        desc_lines.append(
            f"This is a nested object accessed via {nested['parent']}.{nested['parent_property']}[index]."
        )
        desc_lines.append("")
    desc_lines.append("Available properties:")
    for p in properties:
        desc_lines.append(_format_prop_line(p))

    docstring = "\n".join(desc_lines)

    if nested:
        @mcp.tool(name=f"{obj}_read")
        def read_nested_property(property: str, index: int = 0, channel: int = -1) -> str:
            """placeholder"""
            if err := _ensure_connected():
                return err
            if property not in valid_names:
                return f"Unknown property '{property}' on {obj}. Available: {sorted(valid_names)}"
            base = f"{nested['parent']}.{nested['parent_property']}[{index}]"
            if channel >= 0:
                js = f"{base}.{property}[{channel}]"
            else:
                js = f"{base}.{property}"
            result = _run_tool(js)
            logger.info(f"{base}.{property}: {result}")
            return result

        read_nested_property.__doc__ = docstring
    else:
        @mcp.tool(name=f"{obj}_read")
        def read_property(property: str, channel: int = -1) -> str:
            """placeholder"""
            if err := _ensure_connected():
                return err
            if property not in valid_names:
                return f"Unknown property '{property}' on {obj}. Available: {sorted(valid_names)}"
            if channel >= 0:
                js = f"{obj}.{property}[{channel}]"
            else:
                js = f"{obj}.{property}"
            result = _run_tool(js)
            logger.info(f"{obj}.{property}: {result}")
            return result

        read_property.__doc__ = docstring


def _make_property_write_tool(obj: str, properties: list[dict], meta: dict):
    """Create a _write tool for writing Q_PROPERTY values."""
    writable = [p for p in properties if "write" in p]
    if not writable:
        return

    valid_names = {p["name"] for p in writable}
    nested = meta.get("nested")

    desc_lines = [f"Write a property on the '{obj}' Scopy API object.", ""]
    if nested:
        desc_lines.append(
            f"This is a nested object accessed via {nested['parent']}.{nested['parent_property']}[index]."
        )
        desc_lines.append("")
    desc_lines.append("Writable properties:")
    for p in writable:
        desc_lines.append(_format_prop_line(p))

    docstring = "\n".join(desc_lines)

    if nested:
        @mcp.tool(name=f"{obj}_write")
        def write_nested_property(property: str, value: str, index: int = 0, channel: int = -1) -> str:
            """placeholder"""
            if err := _ensure_connected():
                return err
            if property not in valid_names:
                return f"Unknown or read-only property '{property}' on {obj}. Writable: {sorted(valid_names)}"
            quoted = _quote_js_value(value)
            base = f"{nested['parent']}.{nested['parent_property']}[{index}]"
            if channel >= 0:
                js = f"{base}.{property}[{channel}] = {quoted}"
            else:
                js = f"{base}.{property} = {quoted}"
            result = _run_tool(js)
            logger.info(f"{base}.{property} = {value}: {result}")
            return result

        write_nested_property.__doc__ = docstring
    else:
        @mcp.tool(name=f"{obj}_write")
        def write_property(property: str, value: str, channel: int = -1) -> str:
            """placeholder"""
            if err := _ensure_connected():
                return err
            if property not in valid_names:
                return f"Unknown or read-only property '{property}' on {obj}. Writable: {sorted(valid_names)}"
            quoted = _quote_js_value(value)
            if channel >= 0:
                js = f"{obj}.{property}[{channel}] = {quoted}"
            else:
                js = f"{obj}.{property} = {quoted}"
            result = _run_tool(js)
            logger.info(f"{obj}.{property} = {value}: {result}")
            return result

        write_property.__doc__ = docstring


# Register property tools for objects that have properties
for _obj, _meta in _API.get("objects", {}).items():
    _props = _meta.get("properties", [])
    if not _props:
        continue
    _make_property_read_tool(_obj, _props, _meta)
    _make_property_write_tool(_obj, _props, _meta)


# ---------------------------------------------------------------------------

def _cleanup():
    bridge.close()


atexit.register(_cleanup)


def main():
    """Entry point for the MCP server. Does NOT connect to or launch Scopy."""
    mcp.run(transport="stdio")


if __name__ == "__main__":
    main()
