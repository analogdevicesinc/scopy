# doc_screenshots.py

Automated documentation screenshot tool for Scopy. It starts an IIO emulator, launches Scopy with a JS script, and captures screenshots of every tool for the specified package.

## Prerequisites

- `iio-emu` installed and on `PATH`
- A built `scopy` binary
- Python 3.6+

## Usage

Run from the repository root:

```bash
python3 tools/doc_screenshots.py --package <package> --emu-file <file.xml> [options]
```

### Arguments

| Argument      | Required | Description |
|---------------|----------|-------------|
| `--package`   | yes      | Package directory name under `packages/` (e.g. `ad9371`) |
| `--emu-file`  | yes      | EMU XML filename inside `packages/<package>/emu-xml/` (e.g. `ad9371.xml`) |
| `--scopy`     | no       | Path to scopy binary (default: `./scopy`) |
| `--output`    | no       | Output directory (default: `docs/screenshots/<package>`) |

### Example

```bash
# From the build directory, capture ad9371 screenshots
python3 ../tools/doc_screenshots.py \
    --package ad9371 \
    --emu-file ad9371.xml \
    --scopy ./scopy
```

Screenshots are saved to `docs/screenshots/ad9371/` by default, one PNG per tool (e.g. `ad9371_plugin.png`), plus one per tab if the tool exposes `switchTab`/`getTabs` via its API.

## How it works

1. Reads `packages/<package>/emu-xml/emu_setup.json` to get the IIO URI for the given XML file.
2. Starts `iio-emu generic <emu-xml>` in the background and waits for port `30431` to be ready.
3. Creates a temporary JS wrapper that injects `scopyPackage`, `scopyUri`, and `scopyOutDir` globals, then evaluates `js/doc_screenshots.js`.
4. Launches `scopy --script <wrapper>` and waits for it to exit.
5. Terminates `iio-emu` on completion.

## emu_setup.json format

Each package's `emu-xml/emu_setup.json` must be a JSON array with entries that include an `xml_path` and `uri` field:

```json
[
    {
        "device": "ad9371",
        "xml_path": "ad9371.xml",
        "uri": "ip:127.0.0.1"
    }
]
```
