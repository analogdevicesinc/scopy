"""Tests for generate_api_tools.py."""
import sys
import pathlib

sys.path.insert(0, str(pathlib.Path(__file__).parent.parent))

from generate_api_tools import _parse_args_list, _parse_arg


class TestParseArgsList:
    def test_empty_string_returns_empty_list(self):
        assert _parse_args_list("") == []

    def test_single_arg(self):
        result = _parse_args_list("const QString &mode")
        assert len(result) == 1
        assert result[0]["name"] == "mode"

    def test_two_simple_args(self):
        result = _parse_args_list("int a, int b")
        assert len(result) == 2
        assert result[0]["name"] == "a"
        assert result[1]["name"] == "b"

    def test_template_type_with_comma_is_single_arg(self):
        # QMap<QString, int> contains a comma — must be treated as one arg
        result = _parse_args_list("QMap<QString, int> mapping")
        assert len(result) == 1, (
            f"Expected 1 arg for template type, got {len(result)}: {result}"
        )
        assert result[0]["name"] == "mapping"

    def test_nested_template_type(self):
        result = _parse_args_list("QList<QMap<QString, int>> items, int count")
        assert len(result) == 2
        assert result[0]["name"] == "items"
        assert result[1]["name"] == "count"

    def test_arg_with_default(self):
        result = _parse_args_list('const QString &mode = "iio"')
        assert len(result) == 1
        assert result[0]["default"] == "iio"


class TestParseArg:
    def test_const_ref_arg(self):
        result = _parse_arg("const QString &name")
        assert result["name"] == "name"
        assert result["type"] == "QString"

    def test_arg_with_default_value(self):
        result = _parse_arg('const QString &mode = "iio"')
        assert result["name"] == "mode"
        assert result["default"] == "iio"

    def test_plain_int_arg(self):
        result = _parse_arg("int count")
        assert result["name"] == "count"
        assert result["type"] == "int"
