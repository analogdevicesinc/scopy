# screenshots

Automated documentation screenshot tool for Scopy. It starts an IIO emulator, launches Scopy with a JS script, and captures screenshots of every tool for the specified package.

## File Structure

```
tools/screenshots/
├── README.md               # This file
├── screenshots.py          # Main orchestrator (Python)
├── screenshots.js          # Screenshot capture script (JS, runs inside Scopy)
└── push_screenshots.sh     # Push screenshots to doc_resources branch
```

## Prerequisites

- `iio-emu` installed and on `PATH`
- A built `scopy` binary
- Python 3.6+

## Usage

### Single Package Mode

Capture screenshots for one package/device:

```bash
python3 tools/screenshots/screenshots.py --package <package> --emu-file <file.xml> [options]
```

| Argument      | Required | Description |
|---------------|----------|-------------|
| `--package`   | yes      | Package directory name under `packages/` (e.g. `ad9371`) |
| `--emu-file`  | yes      | EMU XML filename or device name (e.g. `ad9371.xml` or `adalm2000`) |
| `--scopy`     | no       | Path to scopy binary (default: `./scopy`) |
| `--output`    | no       | Output directory (default: `docs/screenshots/<package>`) |

```bash
# Example: capture ad9371 screenshots from the build directory
python3 ../tools/screenshots/screenshots.py \
    --package ad9371 \
    --emu-file ad9371.xml \
    --scopy ./scopy
```

### Batch Mode

Discover and capture screenshots for all packages that have an `emu_setup.json`:

```bash
python3 tools/screenshots/screenshots.py --all [options]
```

| Argument        | Required | Description |
|-----------------|----------|-------------|
| `--all`         | yes      | Enable batch discovery mode |
| `--scopy`       | no       | Path to scopy binary (default: `./scopy`) |
| `--output-root` | no       | Base output directory (default: `docs/screenshots`) |

In batch mode the tool:
1. Discovers all packages with `emu-xml/emu_setup.json`
2. Runs `generic-plugins` entries first to capture generic tool screenshots
3. Runs all other packages, skipping generic plugin tools (to avoid duplicates)
4. Reports a summary of successes and failures at the end

Output is organized as `<output-root>/<package>/<device>/`.

### Pushing Screenshots

After capturing, push them to the `doc_resources` branch:

```bash
bash tools/screenshots/push_screenshots.sh <package>
```

This creates a git worktree, copies the screenshots from `docs/screenshots/<package>/` to `resources/<package>/` on the `doc_resources` branch, commits, and pushes.

## How It Works

1. Reads `packages/<package>/emu-xml/emu_setup.json` to find the emulator config for the given device.
2. Starts `iio-emu` with the configured type and XML in the background, waits for port `30431`.
3. Creates a temporary JS wrapper that injects globals (`scopyUri`, `scopyOutDir`, `scopySkipPlugins`), then evaluates `screenshots.js`.
4. Launches `scopy --script <wrapper>` with a 60s timeout per device.
5. Terminates `iio-emu` on completion.

### What screenshots.js captures

For each plugin/tool on the connected device:

- **Full window screenshot** -- `<tool>.png`
- **Scroll area screenshots** -- captures all visible scroll areas (menus/panels) with full scrolled content via `scopy.screenshotAllScrollAreas()`
- **Tab screenshots** -- if the tool has tabs (`scopy.getTabs()`), switches to each tab and captures both a full screenshot and scroll areas per tab (`<tool>_<tab>.png`)

Generic plugin tools (DataLoggerPlugin, ADCPlugin, etc.) are taken as part of ADALM-Pluto device then skipped for non-generic packages to avoid duplicate screenshots.

## emu_setup.json Format

Each package's `emu-xml/emu_setup.json` must be a JSON array:

```json
[
    {
        "device": "ad9371",
        "xml_path": "ad9371.xml",
        "uri": "ip:127.0.0.1"
    }
]
```

| Field            | Required | Description |
|------------------|----------|-------------|
| `device`         | yes      | Device identifier used for output directory naming |
| `xml_path`       | no*      | EMU XML filename (used with `iio-emu generic <xml_path>`) |
| `emu-type`       | no*      | Emulator type (default: `generic`). Use for built-in types like `adalm2000` |
| `uri`            | yes      | IIO device URI (e.g. `ip:127.0.0.1`) |
| `rx_tx_device`   | no       | Device name for binary data injection (e.g. `iio:device0`) |
| `rx_tx_bin_path` | no       | Path to binary data file, relative to `emu-xml/` directory |

*At least one of `xml_path` or `emu-type` is required for the emulator to start.
