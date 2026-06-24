"""
Scopy communication bridge — cross-platform.

Connects to Scopy's QLocalServer named 'scopy_mcp':
  - Linux/macOS: Unix domain socket at /tmp/scopy_mcp (or $TMPDIR/scopy_mcp)
  - Windows:     Named pipe at \\\\.\\pipe\\scopy_mcp

Two modes:
- Attach mode: connect to an already-running Scopy instance
- Launch mode: spawn a new Scopy process, then wait for its MCP server to appear
"""

import os
import socket
import subprocess
import sys
import time

RESPONSE_TIMEOUT = 10  # seconds
SERVER_NAME = "scopy_mcp"


def _socket_path() -> str:
    """Return the platform-appropriate path for the QLocalServer socket."""
    if sys.platform == "win32":
        return rf"\\.\pipe\{SERVER_NAME}"
    # Linux / macOS — Qt places the socket in the temp directory
    tmp = os.environ.get("TMPDIR", "/tmp")
    return os.path.join(tmp, SERVER_NAME)


def _execute_unix(command: str, timeout: float) -> str:
    """Unix/macOS: connect via AF_UNIX socket, send command, read response."""
    path = _socket_path()
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.settimeout(timeout)
    try:
        sock.connect(path)
        sock.sendall((command + "\n").encode("utf-8"))
        # Read until newline or EOF
        data = b""
        while True:
            chunk = sock.recv(4096)
            if not chunk:
                break
            data += chunk
            if b"\n" in data:
                break
        return data.decode("utf-8").strip()
    finally:
        sock.close()


def _execute_win32(command: str, timeout: float) -> str:
    """Windows: connect via named pipe, send command, read response."""
    import win32file  # type: ignore[import-untyped]
    import pywintypes  # type: ignore[import-untyped]

    pipe_name = _socket_path()
    # Wait for pipe to be available (handles transient busy state)
    deadline = time.time() + timeout
    handle = None
    while time.time() < deadline:
        try:
            handle = win32file.CreateFile(
                pipe_name,
                win32file.GENERIC_READ | win32file.GENERIC_WRITE,
                0,
                None,
                win32file.OPEN_EXISTING,
                0,
                None,
            )
            break
        except pywintypes.error as e:
            import winerror  # type: ignore[import-untyped]
            if e.args[0] == winerror.ERROR_PIPE_BUSY:
                time.sleep(0.05)
                continue
            raise
    if handle is None:
        raise TimeoutError(f"Could not connect to Scopy named pipe within {timeout}s")

    try:
        win32file.WriteFile(handle, (command + "\n").encode("utf-8"))
        data = b""
        while True:
            try:
                _, chunk = win32file.ReadFile(handle, 4096)
                data += chunk
                if b"\n" in data:
                    break
            except pywintypes.error as e:
                import winerror  # type: ignore[import-untyped]
                if e.args[0] == winerror.ERROR_MORE_DATA:
                    continue  # more data available
                break  # pipe closed or other error
        return data.decode("utf-8").strip()
    finally:
        win32file.CloseHandle(handle)


def _connect_and_execute(command: str, timeout: float = RESPONSE_TIMEOUT) -> str:
    """Connect to Scopy's local server, send command, read response."""
    if sys.platform == "win32":
        return _execute_win32(command, timeout)
    return _execute_unix(command, timeout)


class ScopyBridge:
    def __init__(self):
        self.mode = None  # "attach" or "launch"
        self.process = None
        self._ready = False

    def is_ready(self) -> bool:
        """Return True if the bridge is already connected to a Scopy instance."""
        return self._ready

    def detect_running_scopy(self) -> bool:
        """Check if a running Scopy's QLocalServer socket/pipe exists."""
        path = _socket_path()
        if sys.platform == "win32":
            # On Windows, named pipes have no filesystem presence — try connecting
            try:
                import win32file  # type: ignore[import-untyped]
                import pywintypes  # type: ignore[import-untyped]
                handle = win32file.CreateFile(
                    path,
                    win32file.GENERIC_READ | win32file.GENERIC_WRITE,
                    0,
                    None,
                    win32file.OPEN_EXISTING,
                    0,
                    None,
                )
                win32file.CloseHandle(handle)
                return True
            except Exception:
                return False
        else:
            if not os.path.exists(path):
                return False
            sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            try:
                sock.connect(path)
                sock.close()
                return True
            except (ConnectionRefusedError, OSError):
                return False

    def attach(self):
        """Attach to a running Scopy instance via its QLocalServer socket."""
        if not self.detect_running_scopy():
            raise RuntimeError(
                "No running Scopy instance found (MCP socket is not accepting connections)"
            )
        self.mode = "attach"
        self._ready = True

    def launch(self, scopy_path: str):
        """Launch Scopy as a subprocess and wait for its MCP server to appear."""
        env = os.environ.copy()
        env["QT_LOGGING_RULES"] = "*.debug=false\n*.info=false\n*.warning=false"

        # Always run Scopy from its own directory — it resolves plugins and
        # resources relative to the executable location.
        scopy_dir = os.path.dirname(os.path.abspath(scopy_path))

        kwargs: dict = {
            "stdin": subprocess.DEVNULL,
            "stdout": subprocess.DEVNULL,
            "stderr": subprocess.DEVNULL,
            "env": env,
            "cwd": scopy_dir,
        }

        if sys.platform == "win32":
            kwargs["creationflags"] = subprocess.CREATE_NEW_PROCESS_GROUP
        else:
            kwargs["preexec_fn"] = os.setsid

        self.process = subprocess.Popen(
            [os.path.abspath(scopy_path), "--accept-license"],
            **kwargs,
        )
        self.mode = "launch"

        # Wait for the QLocalServer socket/pipe to appear
        self._wait_for_server(timeout=30)
        self._ready = True

    def _wait_for_server(self, timeout: float = 30):
        """Poll until Scopy's QLocalServer is accepting connections."""
        deadline = time.time() + timeout
        while time.time() < deadline:
            if self.detect_running_scopy():
                return
            time.sleep(0.5)
        raise RuntimeError(
            f"Scopy did not create MCP server within {timeout}s. "
            "It may still be loading or may have failed to start."
        )

    def execute(self, js_code: str) -> str:
        """Execute a JS command in Scopy and return the result."""
        if not self._ready:
            raise RuntimeError("ScopyBridge is not connected to any Scopy instance")
        response = _connect_and_execute(js_code)
        return self._parse_response(response)

    def _parse_response(self, response: str) -> str:
        """Parse OK:/ERROR: prefix from QLocalServer response."""
        if response.startswith("OK:"):
            return response[3:]
        elif response.startswith("ERROR:"):
            raise RuntimeError(f"Scopy JS error: {response[6:]}")
        # Unexpected format — return as-is rather than silently drop
        return response

    def close(self):
        """Clean up resources."""
        if self.process is not None:
            self.process.terminate()
            try:
                self.process.wait(timeout=15)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
            self.process = None
        self._ready = False
        self.mode = None
