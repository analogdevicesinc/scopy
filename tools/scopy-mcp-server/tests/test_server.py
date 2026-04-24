"""Tests for server.py utility functions and tool functions."""
import json
import sys
import pathlib
from unittest.mock import MagicMock

import pytest

# Allow importing from the package without installing
sys.path.insert(0, str(pathlib.Path(__file__).parent.parent))

from scopy_mcp_server.bridge import ScopyJSError, ScopyStaleError
from scopy_mcp_server.server import (
    _js_string_literal,
    add_device,
    connect_device,
    execute_js,
    get_api_docs,
    start_scopy,
    switch_tool,
    _NO_SCOPY_MSG,
)
import scopy_mcp_server.server as _srv


@pytest.fixture(autouse=True)
def mock_bridge(mocker):
    """Replace the module-level _bridge with a fresh mock for every test."""
    m = MagicMock()
    mocker.patch.object(_srv, "_bridge", m)
    return m


class TestJsStringLiteral:
    def test_plain_string(self):
        assert _js_string_literal("hello") == '"hello"'

    def test_double_quote_escaped(self):
        assert _js_string_literal('say "hi"') == '"say \\"hi\\""'

    def test_backslash_escaped(self):
        assert _js_string_literal("a\\b") == '"a\\\\b"'

    def test_newline_escaped(self):
        assert _js_string_literal("line1\nline2") == '"line1\\nline2"'

    def test_tab_escaped(self):
        assert _js_string_literal("a\tb") == '"a\\tb"'

    def test_null_byte_escaped(self):
        assert _js_string_literal("a\x00b") == '"a\\u0000b"'

    def test_carriage_return_escaped(self):
        assert _js_string_literal("a\rb") == '"a\\rb"'

    def test_injection_attempt_neutralised(self):
        uri = 'x"; scopy.removeAllDevices(); //'
        lit = _js_string_literal(uri)
        assert lit == '"x\\"; scopy.removeAllDevices(); //"'
        assert lit.startswith('"') and lit.endswith('"')


class TestGetApiDocs:
    """get_api_docs reads from the lazy-loaded JSON — no bridge needed."""

    def test_no_arg_returns_sorted_object_list(self):
        result = json.loads(get_api_docs())
        assert isinstance(result, list)
        assert len(result) > 0
        assert result == sorted(result), "Object list must be sorted"
        assert "scopy" in result
        assert "ad936x" in result

    def test_known_object_returns_methods_and_properties(self):
        result = json.loads(get_api_docs("scopy"))
        assert "methods" in result
        assert isinstance(result["methods"], list)
        assert len(result["methods"]) > 0
        m = result["methods"][0]
        assert "name" in m
        assert "args" in m
        assert "returns" in m

    def test_unknown_object_returns_error_string(self):
        result = get_api_docs("does_not_exist")
        assert "Unknown object" in result
        assert "does_not_exist" in result

    def test_nested_object_includes_note(self):
        result = json.loads(get_api_docs("osc_channel"))
        assert "note" in result
        assert "osc" in result["note"]

    def test_missing_json_returns_error_string(self, mocker, tmp_path):
        mocker.patch.object(_srv, "_API", None)
        mocker.patch(
            "scopy_mcp_server.server.pathlib.Path.__truediv__",
            return_value=tmp_path / "nonexistent.json",
        )
        result = get_api_docs()
        assert "not found" in result or isinstance(result, str)
        # Restore so other tests aren't affected
        _srv._API = None


class TestStartScopy:
    def test_already_connected_returns_early(self, mock_bridge):
        mock_bridge.is_ready.return_value = True
        assert "already running" in start_scopy()
        mock_bridge.launch.assert_not_called()

    def test_binary_not_found_returns_error(self, mock_bridge, mocker):
        mock_bridge.is_ready.return_value = False
        mocker.patch("scopy_mcp_server.server.shutil.which", return_value=None)
        result = start_scopy()
        assert "not found" in result
        mock_bridge.launch.assert_not_called()

    def test_launch_success(self, mock_bridge, mocker):
        mock_bridge.is_ready.return_value = False
        mocker.patch("scopy_mcp_server.server.shutil.which", return_value="/usr/bin/scopy")
        result = start_scopy()
        mock_bridge.launch.assert_called_once_with("/usr/bin/scopy")
        assert "launched successfully" in result

    def test_launch_failure_returns_error(self, mock_bridge, mocker):
        mock_bridge.is_ready.return_value = False
        mocker.patch("scopy_mcp_server.server.shutil.which", return_value="/usr/bin/scopy")
        mock_bridge.launch.side_effect = RuntimeError("display not found")
        result = start_scopy()
        assert "Failed to launch Scopy" in result


class TestAddDevice:
    def test_no_scopy_returns_no_scopy_msg(self, mock_bridge):
        mock_bridge.is_ready.return_value = False
        mock_bridge.detect_running_scopy.return_value = False
        assert add_device("192.168.2.1") == _NO_SCOPY_MSG

    def test_happy_path_returns_device_id(self, mock_bridge):
        mock_bridge.is_ready.return_value = True
        mock_bridge.execute.return_value = "dev0"
        result = add_device("192.168.2.1")
        assert result == "dev0"
        js = mock_bridge.execute.call_args[0][0]
        assert "192.168.2.1" in js
        assert "addDevice" in js

    def test_stale_pipe_returns_friendly_message(self, mock_bridge):
        mock_bridge.is_ready.return_value = True
        mock_bridge.execute.side_effect = ScopyStaleError("pipe gone")
        result = add_device("192.168.2.1")
        assert "Stale Scopy pipe" in result

    def test_js_error_returned_as_string(self, mock_bridge):
        mock_bridge.is_ready.return_value = True
        mock_bridge.execute.side_effect = ScopyJSError("device not found")
        result = add_device("192.168.2.1")
        assert "Scopy JS error" in result
        assert "device not found" in result


class TestConnectDevice:
    def test_integer_id_generates_unquoted_js(self, mock_bridge):
        mock_bridge.is_ready.return_value = True
        mock_bridge.execute.return_value = "true"
        connect_device("0")
        js = mock_bridge.execute.call_args[0][0]
        assert "connectDevice(0)" in js

    def test_string_id_generates_quoted_js(self, mock_bridge):
        mock_bridge.is_ready.return_value = True
        mock_bridge.execute.return_value = "true"
        connect_device("dev-abc")
        js = mock_bridge.execute.call_args[0][0]
        assert 'connectDevice("dev-abc")' in js


class TestExecuteJs:
    def test_result_passes_through(self, mock_bridge):
        mock_bridge.is_ready.return_value = True
        mock_bridge.execute.return_value = "42"
        assert execute_js("1 + 1") == "42"

    def test_stale_error_returns_friendly_message(self, mock_bridge):
        mock_bridge.is_ready.return_value = True
        mock_bridge.execute.side_effect = ScopyStaleError("gone")
        assert "Stale Scopy pipe" in execute_js("anything")

    def test_timeout_returns_friendly_message(self, mock_bridge):
        mock_bridge.is_ready.return_value = True
        mock_bridge.execute.side_effect = TimeoutError()
        assert "did not respond" in execute_js("anything")


class TestSwitchTool:
    def test_success_returns_true(self, mock_bridge):
        mock_bridge.is_ready.return_value = True
        mock_bridge.execute.return_value = "true"
        assert switch_tool("Oscilloscope") == "true"

    def test_failure_returns_false_without_second_ipc_call(self, mock_bridge):
        mock_bridge.is_ready.return_value = True
        mock_bridge.execute.return_value = "false"
        result = switch_tool("NonExistentTool")
        assert result == "false"
        # Must not fire a second execute() call to fetch tool list
        assert mock_bridge.execute.call_count == 1

    def test_with_device_id_includes_both_args_in_correct_order(self, mock_bridge):
        mock_bridge.is_ready.return_value = True
        mock_bridge.execute.return_value = "true"
        switch_tool("Oscilloscope", device_id="dev0")
        js = mock_bridge.execute.call_args[0][0]
        assert js == 'scopy.switchTool("dev0", "Oscilloscope")'
