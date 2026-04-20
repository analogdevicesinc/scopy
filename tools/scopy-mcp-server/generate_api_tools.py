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


def _clean_type(raw: str) -> str:
    """Normalize a C++ type string: strip const, &, *, whitespace."""
    t = raw.replace("const ", "").replace("&", "").replace("*", "").strip()
    # Collapse multiple spaces
    return " ".join(t.split())


def _parse_arg(raw: str) -> dict:
    """Parse a single C++ argument like 'const QString &mode = "iio"' into structured form."""
    raw = raw.strip()
    default = None
    if "=" in raw:
        raw, default = raw.split("=", 1)
        default = default.strip().strip('"')
        raw = raw.strip()

    # Strip const, &, *
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
    """Parse a comma-separated C++ argument list into structured args."""
    raw = raw.strip()
    if not raw:
        return []
    return [_parse_arg(a) for a in raw.split(",") if a.strip()]


def extract_class_name(text: str) -> str | None:
    m = re.search(r"class\s+(?:\w+_EXPORT\s+)?(\w+)\s*:\s*public\s+(?:scopy::)?ApiObject", text)
    return m.group(1) if m else None


def parse_api_header(path: Path) -> tuple[str | None, list[dict]]:
    """
    Returns (js_obj_name, methods).
    js_obj_name is None if the class is not in CLASS_TO_JSOBJ.
    methods is a flat list of all Q_INVOKABLE methods with their signatures.
    """
    text = path.read_text(errors="replace")
    class_name = extract_class_name(text)
    if not class_name or class_name not in CLASS_TO_JSOBJ:
        return None, []

    js_obj = CLASS_TO_JSOBJ[class_name]
    methods: list[dict] = []

    for m in _INVOKABLE_RE.finditer(text):
        return_type = _clean_type(m.group(1))
        method_name = m.group(2)
        args = _parse_args_list(m.group(3))

        methods.append({
            "name": method_name,
            "args": args,
            "returns": return_type,
        })

    return js_obj, methods


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
        js_obj, methods = parse_api_header(header)
        if js_obj is None:
            print(f"  skip  {header.relative_to(root)} (class not in mapping)", file=sys.stderr)
            continue

        print(
            f"  parse {header.relative_to(root)} → {js_obj} ({len(methods)} methods)",
            file=sys.stderr,
        )

        if js_obj in objects:
            existing_sigs = {(m["name"], tuple(a["type"] for a in m["args"]))
                            for m in objects[js_obj]["methods"]}
            for m in methods:
                sig = (m["name"], tuple(a["type"] for a in m["args"]))
                if sig not in existing_sigs:
                    objects[js_obj]["methods"].append(m)
                    existing_sigs.add(sig)
        else:
            objects[js_obj] = {"methods": methods}

    # Sort methods by name within each object for stable output
    for obj_meta in objects.values():
        obj_meta["methods"].sort(key=lambda m: m["name"])

    # Build plugin grouping
    parsed_objects = set(objects.keys())
    plugins = {
        plugin: [obj for obj in obj_list if obj in parsed_objects]
        for plugin, obj_list in PLUGIN_OBJECTS.items()
    }
    plugins = {p: objs for p, objs in plugins.items() if objs}

    output = {"plugins": plugins, "objects": objects}
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(output, indent=2))

    total_methods = sum(len(o["methods"]) for o in objects.values())
    print(
        f"\nWrote {out_path} ({len(objects)} objects, {total_methods} methods "
        f"across {len(plugins)} plugins)",
        file=sys.stderr,
    )


if __name__ == "__main__":
    main()
