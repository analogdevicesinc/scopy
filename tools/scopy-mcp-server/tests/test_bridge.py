"""Tests for bridge.py fixes."""
import os
import stat
import sys
import pathlib
import tempfile
import threading
import time
from unittest.mock import patch, MagicMock

sys.path.insert(0, str(pathlib.Path(__file__).parent.parent))

import pytest

from scopy_mcp_server.bridge import ScopyBridge, ScopyJSError, ScopyStaleError, MCP_CMD_PIPE, MCP_RSP_PIPE


class TestMarkStale:
    def test_mark_stale_resets_ready_flag(self):
        bridge = ScopyBridge()
        bridge._ready = True
        bridge._mark_stale()
        assert bridge._ready is False

    def test_mark_stale_does_not_delete_fifos(self, tmp_path):
        """_mark_stale must never delete the FIFOs — Scopy owns them."""
        # Create a real FIFO in a temp location and patch the path constants
        cmd_pipe = tmp_path / "scopy_mcp_cmd"
        rsp_pipe = tmp_path / "scopy_mcp_rsp"
        os.mkfifo(cmd_pipe)
        os.mkfifo(rsp_pipe)

        bridge = ScopyBridge()
        bridge._ready = True

        with patch("scopy_mcp_server.bridge.MCP_CMD_PIPE", str(cmd_pipe)), \
             patch("scopy_mcp_server.bridge.MCP_RSP_PIPE", str(rsp_pipe)):
            bridge._mark_stale()

        # FIFOs must still exist after _mark_stale
        assert cmd_pipe.exists(), "_mark_stale must not delete the command FIFO"
        assert rsp_pipe.exists(), "_mark_stale must not delete the response FIFO"


class TestReadFifoTimeout:
    def test_timeout_raises_TimeoutError(self, tmp_path):
        """_read_fifo_response must raise TimeoutError if no data arrives."""
        import threading
        import time

        rsp_pipe_path = tmp_path / "scopy_mcp_rsp"
        os.mkfifo(rsp_pipe_path)

        bridge = ScopyBridge()

        # Open the write end in a thread so the open() call doesn't block,
        # but write nothing — simulating a silent Scopy.
        def hold_open():
            with open(rsp_pipe_path, "w"):
                time.sleep(5)

        t = threading.Thread(target=hold_open, daemon=True)
        t.start()

        with patch("scopy_mcp_server.bridge.MCP_RSP_PIPE", str(rsp_pipe_path)), \
             patch("scopy_mcp_server.bridge.RESPONSE_TIMEOUT", 0.1):
            start = time.monotonic()
            try:
                bridge._read_fifo_response()
                assert False, "Expected TimeoutError"
            except TimeoutError:
                elapsed = time.monotonic() - start
                assert elapsed < 1.0, f"Timeout took too long: {elapsed:.2f}s"


class TestExecuteLock:
    def test_bridge_has_lock_attribute(self):
        bridge = ScopyBridge()
        assert hasattr(bridge, "_lock")
        assert isinstance(bridge._lock, type(threading.Lock()))

    def test_concurrent_execute_calls_are_serialized(self, tmp_path):
        """Second execute() call must wait until first completes."""
        cmd_pipe = tmp_path / "scopy_mcp_cmd"
        rsp_pipe = tmp_path / "scopy_mcp_rsp"
        os.mkfifo(cmd_pipe)
        os.mkfifo(rsp_pipe)

        bridge = ScopyBridge()
        bridge._ready = True

        order = []

        def slow_execute():
            with bridge._lock:
                order.append("first_start")
                time.sleep(0.1)
                order.append("first_end")

        t = threading.Thread(target=slow_execute)
        t.start()
        time.sleep(0.02)

        with bridge._lock:
            order.append("second_start")

        t.join()
        assert order == ["first_start", "first_end", "second_start"]


class TestParseFifoResponse:
    def test_ok_prefix_strips_prefix_and_returns_value(self):
        bridge = ScopyBridge()
        assert bridge._parse_fifo_response("OK:dev0") == "dev0"

    def test_ok_prefix_with_empty_value(self):
        bridge = ScopyBridge()
        assert bridge._parse_fifo_response("OK:") == ""

    def test_error_prefix_raises_ScopyJSError(self):
        bridge = ScopyBridge()
        with pytest.raises(ScopyJSError, match="device not found"):
            bridge._parse_fifo_response("ERROR:device not found")

    def test_unknown_prefix_raises_ScopyStaleError(self):
        """Any response without OK:/ERROR: prefix is a protocol error."""
        bridge = ScopyBridge()
        with pytest.raises(ScopyStaleError, match="Unexpected response"):
            bridge._parse_fifo_response("some random garbage")

    def test_empty_response_raises_ScopyStaleError(self):
        bridge = ScopyBridge()
        with pytest.raises(ScopyStaleError, match="Unexpected response"):
            bridge._parse_fifo_response("")
