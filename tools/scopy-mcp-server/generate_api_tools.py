"""
generate_api_tools.py — Scopy MCP API Generator

Parses all *_api.h / *_api.hpp files in the Scopy repo and writes scopy_api.json.
Re-run this whenever the Scopy API changes.

Usage:
    python tools/scopy-mcp-server/generate_api_tools.py [--scopy-root .] [--out tools/scopy-mcp-server/scopy_api.json]
"""

import argparse
import json
import re
import sys
from pathlib import Path

# Maps C++ class name → JS object name (as registered via setObjectName in each plugin)
CLASS_TO_JSOBJ = {
    # Core
    "ScopyMainWindow_API": "scopy",
    # AD936X / FMCOMMS
    "AD936X_API": "ad936x",
    "AD936X_ADVANCED_API": "ad936x_advanced",
    "FMCOMMS5_API": "fmcomms5",
    "FMCOMMS5_ADVANCED_API": "fmcomms5_advanced",
    # AD9084
    "AD9084_API": "ad9084",
    # ADRV9002 / ADRV9009
    "ADRV9002_API": "adrv9002",
    "Adrv9009Plugin_API": "adrv9009",
    # Generic plugins
    "RegMap_API": "regmap",
    "IIOExplorerInstrument_API": "iioExplorer",
    "DataLogger_API": "datalogger",
    # PQM / SWIOT
    "PQM_API": "pqm",
    "SWIOT_API": "swiot",
    # M2K instruments
    "Oscilloscope_API": "osc",
    "SpectrumAnalyzer_API": "spectrum",
    "NetworkAnalyzer_API": "network",
    "SignalGenerator_API": "siggen",
    "LogicAnalyzer_API": "logic",
    "PatternGenerator_API": "pattern",
    "DigitalIO_API": "dio",
    "DMM_API": "dmm",
    "PowerController_API": "power",
    "Calibration_API": "calib",
}

# Nested API classes: C++ class → (js_name, parent_js_obj, parent_property_name)
NESTED_CLASS_MAP = {
    "Channel_API": ("osc_channel", "osc", "channels"),
    "Channel_Digital_Filter_API": ("osc_channel_filter", "osc_channel", "digFilter"),
}

# Maps package directory name → list of JS object names it exposes
PLUGIN_OBJECTS: dict[str, list[str]] = {
    "core": ["scopy"],
    "ad936x": ["ad936x", "ad936x_advanced", "fmcomms5", "fmcomms5_advanced"],
    "apollo-ad9084": ["ad9084"],
    "adrv9002": ["adrv9002"],
    "adrv9009": ["adrv9009"],
    "generic-plugins": ["regmap", "iioExplorer", "datalogger"],
    "pqmon": ["pqm"],
    "swiot": ["swiot"],
    "m2k": ["osc", "spectrum", "network", "siggen", "logic", "pattern", "dio", "dmm", "power", "calib"],
}

_INVOKABLE_RE = re.compile(
    r"Q_INVOKABLE\s+"
    r"([\w:<>*& ]+?)\s+"      # return type (group 1)
    r"(\w+)\s*"                # method name (group 2)
    r"\(([^)]*)\)"             # argument list (group 3)
)

_PROPERTY_RE = re.compile(
    r"Q_PROPERTY\(\s*"
    r"([\w:<>*& ]+?)\s+"      # type (group 1)
    r"(\w+)\s+"                # property name (group 2)
    r"READ\s+(\w+)"           # read method (group 3)
    r"(?:\s+WRITE\s+(\w+))?"  # optional write method (group 4)
)

_CLASS_RE = re.compile(
    r"class\s+(?:\w+_EXPORT\s+)?(\w+)\s*:\s*public\s+(?:scopy::)?(?:ApiObject|QObject)"
)


def _clean_type(raw: str) -> str:
    """Normalize a C++ type string: strip const, &, *, whitespace."""
    t = raw.replace("const ", "").replace("&", "").replace("*", "").strip()
    return " ".join(t.split())


def _parse_arg(raw: str) -> dict:
    """Parse a single C++ argument like 'const QString &mode = "iio"' into structured form."""
    raw = raw.strip()
    default = None
    if "=" in raw:
        raw, default = raw.split("=", 1)
        default = default.strip().strip('"')
        raw = raw.strip()

    cleaned = _clean_type(raw)
    parts = cleaned.split()

    if len(parts) >= 2:
        type_name = " ".join(parts[:-1])
        arg_name = parts[-1]
    elif len(parts) == 1:
        type_name = parts[0]
        arg_name = parts[0]
    else:
        type_name = raw
        arg_name = raw

    result = {"name": arg_name, "type": type_name}
    if default is not None:
        result["default"] = default
    return result


def _parse_args_list(raw: str) -> list[dict]:
    """Parse a C++ argument list, handling template types with commas."""
    raw = raw.strip()
    if not raw:
        return []

    args: list[str] = []
    depth = 0
    current: list[str] = []

    for ch in raw:
        if ch == "<":
            depth += 1
            current.append(ch)
        elif ch == ">":
            depth -= 1
            current.append(ch)
        elif ch == "," and depth == 0:
            token = "".join(current).strip()
            if token:
                args.append(token)
            current = []
        else:
            current.append(ch)

    token = "".join(current).strip()
    if token:
        args.append(token)

    return [_parse_arg(a) for a in args]


def _is_list_type(type_str: str) -> bool:
    return type_str.startswith("QList<") or type_str == "QVariantList"


def _element_type(type_str: str) -> str:
    """Extract element type from QList<T>."""
    m = re.match(r"QList<\s*(.+?)\s*>", type_str)
    if m:
        return _clean_type(m.group(1))
    if type_str == "QVariantList":
        return "QVariant"
    return type_str


def _parse_property(m: re.Match) -> dict:
    """Parse a Q_PROPERTY regex match into structured form."""
    raw_type = _clean_type(m.group(1))
    prop = {
        "name": m.group(2),
        "type": raw_type,
        "read": m.group(3),
        "is_list": _is_list_type(raw_type),
    }
    if m.group(4):
        prop["write"] = m.group(4)
    if prop["is_list"]:
        prop["element_type"] = _element_type(raw_type)
    return prop


def _find_all_classes(text: str) -> list[str]:
    """Find all class names that inherit from ApiObject or QObject."""
    return _CLASS_RE.findall(text)


def _extract_class_block(text: str, class_name: str) -> str:
    """Extract the text block for a specific class declaration."""
    pattern = re.compile(
        r"class\s+(?:\w+_EXPORT\s+)?" + re.escape(class_name) + r"\s*:\s*public"
    )
    m = pattern.search(text)
    if not m:
        return ""

    start = m.start()
    brace_count = 0
    in_class = False

    for i in range(start, len(text)):
        if text[i] == "{":
            brace_count += 1
            in_class = True
        elif text[i] == "}":
            brace_count -= 1
            if in_class and brace_count == 0:
                return text[start:i + 1]

    return text[start:]


def parse_api_header(path: Path) -> tuple[dict[str, dict], list[str]]:
    """
    Returns (objects_dict, js_names_found).
    objects_dict maps js_obj_name → {"methods": [...], "properties": [...]}.
    Handles both main API classes and nested classes.
    """
    text = path.read_text(errors="replace")
    all_classes = _find_all_classes(text)
    result: dict[str, dict] = {}
    js_names: list[str] = []

    for class_name in all_classes:
        js_obj = None
        nested_info = None

        if class_name in CLASS_TO_JSOBJ:
            js_obj = CLASS_TO_JSOBJ[class_name]
        elif class_name in NESTED_CLASS_MAP:
            js_obj, parent_obj, parent_prop = NESTED_CLASS_MAP[class_name]
            nested_info = {"parent": parent_obj, "parent_property": parent_prop}
        else:
            continue

        block = _extract_class_block(text, class_name)
        if not block:
            continue

        methods = []
        for m in _INVOKABLE_RE.finditer(block):
            methods.append({
                "name": m.group(2),
                "args": _parse_args_list(m.group(3)),
                "returns": _clean_type(m.group(1)),
            })

        properties = []
        for m in _PROPERTY_RE.finditer(block):
            prop = _parse_property(m)
            if prop["type"] == "QVariantList" and prop["name"] in ("channels", "digFilter"):
                for nc, (nc_js, nc_parent, nc_prop) in NESTED_CLASS_MAP.items():
                    if nc_prop == prop["name"]:
                        prop["nested"] = nc_js
                        break
            properties.append(prop)

        obj_meta = {"methods": methods}
        if properties:
            obj_meta["properties"] = properties
        if nested_info:
            obj_meta["nested"] = nested_info

        result[js_obj] = obj_meta
        js_names.append(js_obj)

    return result, js_names


def main():
    parser = argparse.ArgumentParser(description="Generate scopy_api.json from *_api.h/hpp headers")
    parser.add_argument("--scopy-root", default=".", help="Path to Scopy repo root")
    parser.add_argument("--out", default="tools/scopy-mcp-server/scopy_mcp_server/scopy_api.json", help="Output JSON path")
    args = parser.parse_args()

    root = Path(args.scopy_root).resolve()
    out_path = Path(args.out)

    api_headers = sorted(
        h for h in list(root.rglob("*_api.h")) + list(root.rglob("*_api.hpp"))
        if "build" not in h.parts
    )
    if not api_headers:
        print(f"ERROR: No *_api.h/hpp files found under {root}", file=sys.stderr)
        sys.exit(1)

    print(f"Found {len(api_headers)} API header files", file=sys.stderr)

    objects: dict[str, dict] = {}

    for header in api_headers:
        header_objects, js_names = parse_api_header(header)

        if not header_objects:
            print(f"  skip  {header.relative_to(root)} (no mapped classes)", file=sys.stderr)
            continue

        for js_obj, obj_meta in header_objects.items():
            n_methods = len(obj_meta.get("methods", []))
            n_props = len(obj_meta.get("properties", []))
            print(
                f"  parse {header.relative_to(root)} → {js_obj} "
                f"({n_methods} methods, {n_props} properties)",
                file=sys.stderr,
            )

            if js_obj in objects:
                existing = objects[js_obj]
                existing_sigs = {(m["name"], tuple(a["type"] for a in m["args"]))
                                for m in existing.get("methods", [])}
                for m in obj_meta.get("methods", []):
                    sig = (m["name"], tuple(a["type"] for a in m["args"]))
                    if sig not in existing_sigs:
                        existing.setdefault("methods", []).append(m)
                        existing_sigs.add(sig)

                existing_props = {p["name"] for p in existing.get("properties", [])}
                for p in obj_meta.get("properties", []):
                    if p["name"] not in existing_props:
                        existing.setdefault("properties", []).append(p)
                        existing_props.add(p["name"])

                if "nested" in obj_meta and "nested" not in existing:
                    existing["nested"] = obj_meta["nested"]
            else:
                objects[js_obj] = obj_meta

    for obj_meta in objects.values():
        if "methods" in obj_meta:
            obj_meta["methods"].sort(key=lambda m: m["name"])
        if "properties" in obj_meta:
            obj_meta["properties"].sort(key=lambda p: p["name"])

    # Build plugin grouping — include nested objects under parent's plugin
    all_js_names = set(objects.keys())
    plugins = {
        plugin: [obj for obj in obj_list if obj in all_js_names]
        for plugin, obj_list in PLUGIN_OBJECTS.items()
    }
    # Add nested objects to their parent's plugin
    for js_obj, meta in objects.items():
        if "nested" in meta:
            parent = meta["nested"]["parent"]
            for plugin, obj_list in plugins.items():
                if parent in obj_list and js_obj not in obj_list:
                    obj_list.append(js_obj)
    plugins = {p: objs for p, objs in plugins.items() if objs}

    output = {"plugins": plugins, "objects": objects}
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(output, indent=2))

    total_methods = sum(len(o.get("methods", [])) for o in objects.values())
    total_props = sum(len(o.get("properties", [])) for o in objects.values())
    print(
        f"\nWrote {out_path} ({len(objects)} objects, {total_methods} methods, "
        f"{total_props} properties across {len(plugins)} plugins)",
        file=sys.stderr,
    )


if __name__ == "__main__":
    main()
