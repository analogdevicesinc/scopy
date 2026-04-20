"""
Scopy MCP Server — Proof of Concept

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
import logging
import os
import sys

from mcp.server.fastmcp import FastMCP

from scopy_bridge import ScopyBridge

logging.basicConfig(level=logging.INFO, stream=sys.stderr)
logger = logging.getLogger("scopy-mcp")

mcp = FastMCP("Scopy MCP Server")
bridge = ScopyBridge()

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
def switch_tool(tool_name: str, device_id: str = "") -> str:
    """Switch to a specific tool in Scopy.

    Args:
        tool_name: Name of the tool to switch to (e.g., 'AD936X', 'Oscilloscope').
        device_id: Optional device ID. If omitted, uses the currently active device.

    Returns:
        'true' if switch succeeded, 'false' otherwise, or an error message.
    """
    if err := _ensure_connected():
        return err
    if device_id:
        js = f'scopy.switchTool("{device_id}", "{tool_name}")'
    else:
        js = f'scopy.switchTool("{tool_name}")'
    result = _run_tool(js)
    logger.info(f"switch_tool({tool_name}): {result}")
    return result


def _cleanup():
    bridge.close()


atexit.register(_cleanup)


def main():
    """Entry point for the MCP server. Does NOT connect to or launch Scopy."""
    mcp.run(transport="stdio")


if __name__ == "__main__":
    main()
