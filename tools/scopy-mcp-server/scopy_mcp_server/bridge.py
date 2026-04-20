"""
Scopy communication bridge.

Handles two modes of communication with a Scopy instance:
- Attach mode: connect to a running Scopy via named pipes (FIFOs)
- Launch mode: spawn a new Scopy process with a PTY for stdin REPL access
"""

import errno
import os
import pty
import select
import signal
import stat
import subprocess
import sys
import time

MCP_CMD_PIPE = "/tmp/scopy_mcp_cmd"
MCP_RSP_PIPE = "/tmp/scopy_mcp_rsp"

RESPONSE_TIMEOUT = 10  # seconds


class ScopyBridge:
    def __init__(self):
        self.mode = None  # "attach" or "launch"
        self.pty_master = None
        self.process = None
        self._ready = False

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
        self.mode = "attach"
        self._ready = True

    def launch(self, scopy_path: str):
        """Launch Scopy as a subprocess with a PTY so the stdin REPL activates."""
        master, slave = pty.openpty()

        env = os.environ.copy()
        env["QT_LOGGING_RULES"] = "*.debug=false\n*.info=false\n*.warning=false"

        # Always run Scopy from its own directory — it resolves plugins and
        # resources relative to the executable location.
        scopy_dir = os.path.dirname(os.path.abspath(scopy_path))

        self.process = subprocess.Popen(
            [os.path.abspath(scopy_path), "--accept-license"],
            stdin=slave,
            stdout=slave,
            stderr=slave,
            env=env,
            cwd=scopy_dir,
            preexec_fn=os.setsid,
        )
        os.close(slave)
        self.pty_master = master
        self.mode = "launch"

        # Wait for Scopy to finish initializing (look for the "scopy > " prompt)
        self._wait_for_prompt(timeout=30)
        self._ready = True

    def execute(self, js_code: str) -> str:
        """Execute a JS command in Scopy and return the result."""
        if not self._ready:
            raise RuntimeError("ScopyBridge is not connected to any Scopy instance")

        if self.mode == "attach":
            return self._execute_via_fifo(js_code)
        else:
            return self._execute_via_pty(js_code)

    def _execute_via_fifo(self, js_code: str) -> str:
        """Send command via FIFO, read response from response FIFO."""
        # Use O_NONBLOCK so we get ENXIO immediately if Scopy is not listening
        # (i.e. stale FIFO on disk but no reader). A blocking open would hang forever.
        try:
            fd = os.open(MCP_CMD_PIPE, os.O_WRONLY | os.O_NONBLOCK)
        except OSError as e:
            if e.errno == errno.ENXIO:
                self._mark_stale()
                raise RuntimeError(
                    "Scopy pipe found but no listener (stale FIFO). "
                    "Scopy is not running or was not built with MCP support."
                )
            raise

        with os.fdopen(fd, "w") as cmd_pipe:
            cmd_pipe.write(js_code + "\n")

        # Read response from the response pipe with timeout
        response = self._read_fifo_response()
        return self._parse_fifo_response(response)

    def _mark_stale(self):
        """Clean up state when a stale FIFO is detected."""
        self._ready = False
        self.mode = None
        for path in (MCP_CMD_PIPE, MCP_RSP_PIPE):
            try:
                os.unlink(path)
            except OSError:
                pass

    def _read_fifo_response(self) -> str:
        """Read a line from the response FIFO with timeout."""
        # Use alarm for timeout
        def timeout_handler(signum, frame):
            raise TimeoutError(f"Scopy did not respond within {RESPONSE_TIMEOUT} seconds")

        old_handler = signal.signal(signal.SIGALRM, timeout_handler)
        signal.alarm(RESPONSE_TIMEOUT)

        try:
            with open(MCP_RSP_PIPE, "r") as rsp_pipe:
                line = rsp_pipe.readline().strip()
            return line
        finally:
            signal.alarm(0)
            signal.signal(signal.SIGALRM, old_handler)

    def _parse_fifo_response(self, response: str) -> str:
        """Parse OK:/ERROR: prefix from FIFO response."""
        if response.startswith("OK:"):
            return response[3:]
        elif response.startswith("ERROR:"):
            raise RuntimeError(f"Scopy JS error: {response[6:]}")
        else:
            return response

    def _execute_via_pty(self, js_code: str) -> str:
        """Send command via PTY master, read output until prompt."""
        # Write the JS command
        os.write(self.pty_master, (js_code + "\n").encode())

        # Read output until we see the "scopy > " prompt
        output = self._read_until_prompt()

        # The output contains: echoed command, result, then "scopy > "
        lines = output.strip().split("\n")

        # Filter out the echoed command and the prompt
        result_lines = []
        for line in lines:
            stripped = line.strip()
            if stripped == js_code.strip():
                continue
            if stripped == "scopy >":
                continue
            if stripped.startswith("scopy >"):
                continue
            result_lines.append(stripped)

        result = "\n".join(result_lines).strip()

        if result.startswith("Exception:"):
            raise RuntimeError(f"Scopy JS error: {result}")

        return result

    def _read_until_prompt(self, timeout: float = RESPONSE_TIMEOUT) -> str:
        """Read PTY output until 'scopy > ' prompt appears."""
        output = ""
        deadline = time.time() + timeout

        while time.time() < deadline:
            remaining = deadline - time.time()
            if remaining <= 0:
                break

            ready, _, _ = select.select([self.pty_master], [], [], min(remaining, 0.5))
            if ready:
                try:
                    data = os.read(self.pty_master, 4096).decode("utf-8", errors="replace")
                    output += data
                    if "scopy > " in output or "scopy >" in output:
                        return output
                except OSError:
                    break

        raise TimeoutError(f"Scopy did not respond within {timeout} seconds")

    def _wait_for_prompt(self, timeout: float = 30):
        """Wait for the initial 'scopy > ' prompt after launch."""
        try:
            self._read_until_prompt(timeout=timeout)
        except TimeoutError:
            # Scopy may not print a prompt if stdin is not interactive enough.
            # In launch mode, we also check if the MCP pipe was created (since
            # Scopy's init() creates it).
            if self.detect_running_scopy():
                return
            raise RuntimeError(
                f"Scopy did not produce a prompt within {timeout}s. "
                "It may still be loading or may have failed to start."
            )

    def close(self):
        """Clean up resources."""
        if self.pty_master is not None:
            try:
                os.close(self.pty_master)
            except OSError:
                pass
            self.pty_master = None

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
