"""
Scopy communication bridge.

Handles two modes of communication with a Scopy instance:
- Attach mode: connect to a running Scopy via named pipes (FIFOs)
- Launch mode: spawn a new Scopy process and communicate via FIFOs once ready
"""

import errno
import os
import select
import stat
import subprocess
import threading
import time

MCP_CMD_PIPE = "/tmp/scopy_mcp_cmd"
MCP_RSP_PIPE = "/tmp/scopy_mcp_rsp"

RESPONSE_TIMEOUT = 10  # seconds


class ScopyStaleError(RuntimeError):
    """Raised when the Scopy MCP pipe is stale or Scopy has exited."""


class ScopyJSError(RuntimeError):
    """Raised when Scopy's JS engine returns an error result."""


class ScopyBridge:
    def __init__(self):
        self.process = None  # set when Scopy was launched by this bridge
        self._ready = False
        self._lock = threading.Lock()

    def is_ready(self) -> bool:
        """Return True if the bridge is already connected to a Scopy instance."""
        return self._ready

    def detect_running_scopy(self) -> bool:
        """Check if a running Scopy instance has created the MCP command pipe."""
        try:
            if os.path.exists(MCP_CMD_PIPE) and stat.S_ISFIFO(os.stat(MCP_CMD_PIPE).st_mode):
                return True
        except OSError:
            pass
        return False

    def attach(self):
        """Attach to a running Scopy instance via FIFOs."""
        if not self.detect_running_scopy():
            raise RuntimeError("No running Scopy instance found (MCP pipe does not exist)")
        self._ready = True

    def launch(self, scopy_path: str, timeout: float = 30):
        """Launch Scopy as a subprocess and communicate via FIFOs once ready."""
        env = os.environ.copy()
        env["QT_LOGGING_RULES"] = "*.debug=false\n*.info=false\n*.warning=false"

        scopy_dir = os.path.dirname(os.path.abspath(scopy_path))

        self.process = subprocess.Popen(
            [os.path.abspath(scopy_path), "--accept-license"],
            stdin=subprocess.DEVNULL,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            env=env,
            cwd=scopy_dir,
            preexec_fn=os.setsid,
        )

        self._wait_for_mcp_pipe(timeout=timeout)
        self._ready = True

    def execute(self, js_code: str) -> str:
        """Execute a JS command in Scopy and return the result."""
        with self._lock:
            if not self._ready:
                raise RuntimeError("ScopyBridge is not connected to any Scopy instance")
            return self._execute_via_fifo(js_code)

    def _execute_via_fifo(self, js_code: str) -> str:
        """Send command via FIFO, read response from response FIFO."""
        # Use O_NONBLOCK so we get ENXIO immediately if Scopy is not listening
        # (i.e. stale FIFO on disk but no reader). A blocking open would hang forever.
        try:
            fd = os.open(MCP_CMD_PIPE, os.O_WRONLY | os.O_NONBLOCK)
        except OSError as e:
            if e.errno in (errno.ENXIO, errno.ENOENT):
                self._mark_stale()
                raise ScopyStaleError(
                    "Scopy pipe not found (stale FIFO). "
                    "Scopy is not running or was not built with MCP support."
                )
            raise

        with os.fdopen(fd, "w") as cmd_pipe:
            cmd_pipe.write(js_code + "\n")

        # Read response from the response pipe with timeout
        response = self._read_fifo_response()
        return self._parse_fifo_response(response)

    def _mark_stale(self):
        self._ready = False

    def _read_fifo_response(self) -> str:
        """Read a line from the response FIFO with timeout using select().

        Opens with O_NONBLOCK so we don't block waiting for a writer that will
        never come (e.g. Scopy crashed between writing the command and reading
        the response). On Linux, O_NONBLOCK on the read end succeeds even when
        no writer exists; the subsequent select() then catches both timeout and
        EOF (empty read = Scopy gone).
        """
        fd = os.open(MCP_RSP_PIPE, os.O_RDONLY | os.O_NONBLOCK)
        with os.fdopen(fd, "r") as rsp_pipe:
            ready, _, _ = select.select([rsp_pipe], [], [], RESPONSE_TIMEOUT)
            if not ready:
                raise TimeoutError(
                    f"Scopy did not respond within {RESPONSE_TIMEOUT} seconds"
                )
            line = rsp_pipe.readline().strip()
            if not line:
                self._mark_stale()
                raise ScopyStaleError(
                    "Scopy pipe closed unexpectedly — Scopy likely exited mid-request."
                )
            return line

    def _parse_fifo_response(self, response: str) -> str:
        """Parse OK:/ERROR: prefix from FIFO response."""
        if response.startswith("OK:"):
            return response[3:]
        if response.startswith("ERROR:"):
            raise ScopyJSError(response[6:])
        raise ScopyStaleError(
            f"Unexpected response from Scopy (no OK:/ERROR: prefix): {response!r}. "
            "Scopy may not be built with MCP support, or the protocol version is mismatched."
        )

    def _wait_for_mcp_pipe(self, timeout: float = 30):
        """Poll until Scopy creates the MCP command FIFO, indicating it has initialized."""
        deadline = time.time() + timeout
        while time.time() < deadline:
            if self.detect_running_scopy():
                return
            if self.process is not None and self.process.poll() is not None:
                raise RuntimeError(
                    f"Scopy exited unexpectedly (code {self.process.returncode}) before creating MCP pipe."
                )
            time.sleep(0.5)
        raise RuntimeError(
            f"Scopy did not create the MCP pipe within {timeout}s. "
            "It may still be loading or may have failed to start."
        )

    def close(self):
        """Clean up resources."""
        if self.process is not None:
            self.process.terminate()
            try:
                # Give Scopy enough time to run its Qt destructor and unlink FIFOs.
                # If it still hasn't exited after 15s, SIGKILL it.
                self.process.wait(timeout=15)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
            self.process = None

        self._ready = False
