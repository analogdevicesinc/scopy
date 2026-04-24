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

from scopy_mcp_server.bridge import ScopyBridge, ScopyJSError, ScopyStaleError

logging.basicConfig(level=logging.INFO, stream=sys.stderr)
logger = logging.getLogger("scopy-mcp")

mcp = FastMCP("Scopy MCP Server")
_bridge = ScopyBridge()

_API: dict | None = None

_NO_SCOPY_MSG = (
    "No running Scopy instance found. "
    "Please start Scopy manually, or call start_scopy() to launch it."
)


def _load_api() -> dict | str:
    """Load scopy_api.json on first use. Returns the dict or an error string."""
    global _API
    if _API is not None:
        return _API
    path = pathlib.Path(__file__).parent / "scopy_api.json"
    try:
        _API = json.loads(path.read_text())
        return _API
    except FileNotFoundError:
        return (
            f"scopy_api.json not found at {path}. "
            "Run: python tools/scopy-mcp-server/generate_api_tools.py"
        )
    except json.JSONDecodeError as e:
        return f"scopy_api.json is corrupt ({e}). Re-run generate_api_tools.py."


def _ensure_connected() -> str | None:
    """Lazily attach to a running Scopy. Returns an error string if not available."""
    if _bridge.is_ready():
        return None
    if _bridge.detect_running_scopy():
        _bridge.attach()
        logger.info("Attached to running Scopy instance.")
        return None
    return _NO_SCOPY_MSG


def _run_tool(js_code: str) -> str:
    """Run a JS command, catching connection and JS errors as user-friendly strings."""
    try:
        return _bridge.execute(js_code)
    except ScopyStaleError:
        return (
            "Stale Scopy pipe detected — Scopy exited without cleaning up. "
            "Please start Scopy again, or call start_scopy()."
        )
    except ScopyJSError as e:
        return f"Scopy JS error: {e}"
    except TimeoutError:
        return (
            "Scopy did not respond within the timeout. "
            "Check that Scopy is running and the MCP pipe listener is active "
            "(look for 'MCP pipe listener active' in Scopy's log)."
        )


def _js_string_literal(value: str) -> str:
    """Return a JS string literal (with surrounding quotes) for safe embedding in JS code."""
    return json.dumps(value)


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
    if _bridge.is_ready():
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
        _bridge.launch(scopy_path)
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
    js = f'scopy.addDevice({_js_string_literal(uri)}, {_js_string_literal(category)})'
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
        js = f'scopy.connectDevice({_js_string_literal(device_id)})'
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
            js = f'scopy.disconnectDevice({_js_string_literal(device_id)})'
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
        'true' if switch succeeded, 'false' otherwise. If 'false', call get_api_docs()
        or execute_js("scopy.getTools()") to see the exact available tool names.
    """
    if err := _ensure_connected():
        return err
    if device_id:
        js = f'scopy.switchTool({_js_string_literal(device_id)}, {_js_string_literal(tool_name)})'
    else:
        js = f'scopy.switchTool({_js_string_literal(tool_name)})'
    result = _run_tool(js)
    logger.info(f"switch_tool({tool_name}): {result}")
    return result


# ---------------------------------------------------------------------------
# Generic JS execution and API discovery tools (replaces ~160 per-object tools)
# ---------------------------------------------------------------------------

@mcp.tool()
def execute_js(code: str) -> str:
    """Execute JavaScript in Scopy's console and return the result.

    This is the primary way to interact with Scopy's API. Use get_api_docs()
    to discover available objects and methods, then call them directly.

    Examples:
        execute_js("ad936x.setRxLo(2400000000)")
        execute_js("osc.channels[0].enabled")
        execute_js("scopy.getDevices()")
        execute_js("ad936x.rxFrequency")

    Returns the JS result as a string, or an error message prefixed with
    'Scopy JS error:' if Scopy's engine raised an exception.
    """
    if err := _ensure_connected():
        return err
    return _run_tool(code)


@mcp.tool()
def get_api_docs(obj: str = "") -> str:
    """Get Scopy API documentation.

    With no argument: returns a JSON list of all available API object names.
    With an object name: returns that object's methods and properties as JSON.

    Workflow:
        1. get_api_docs()            → ["ad936x", "osc", "scopy", ...]
        2. get_api_docs("ad936x")    → {"methods": [...], "properties": [...]}
        3. execute_js("ad936x.setRxLo(2400000000)")

    For nested objects (e.g. osc_channel), the result includes a 'note'
    showing the correct JS access path (e.g. osc.channels[0].enabled).
    """
    api = _load_api()
    if isinstance(api, str):
        return api

    objects = api.get("objects", {})

    if not obj:
        return json.dumps(sorted(objects.keys()))

    meta = objects.get(obj)
    if not meta:
        return f"Unknown object '{obj}'. Available: {sorted(objects.keys())}"

    result: dict = {}

    if meta.get("methods"):
        result["methods"] = [
            {
                "name": m["name"],
                "args": [
                    a["name"] + ("=" + a["default"] if "default" in a else "")
                    for a in m["args"]
                ],
                "returns": m["returns"],
            }
            for m in meta["methods"]
        ]

    if meta.get("properties"):
        result["properties"] = [
            {
                "name": p["name"],
                "type": p["type"],
                "writable": "write" in p,
            }
            for p in meta["properties"]
        ]

    if meta.get("nested"):
        n = meta["nested"]
        result["note"] = (
            f"Nested object — access via "
            f"{n['parent']}.{n['parent_property']}[index].propertyName"
        )

    return json.dumps(result, indent=2)


# ---------------------------------------------------------------------------

def _cleanup():
    _bridge.close()


atexit.register(_cleanup)


def main():
    """Entry point for the MCP server. Does NOT connect to or launch Scopy."""
    mcp.run(transport="stdio")


if __name__ == "__main__":
    main()
