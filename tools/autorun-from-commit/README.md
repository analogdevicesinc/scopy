# Scopy Autorun from Commit Scripts

Automated tools for downloading and launching Scopy builds from CI/CD artifacts based on commit SHA.

## Scripts

### `scopy-commit-build.py`
Downloads Scopy builds from **GitHub Actions** for Linux (AppImage & Flatpak)/Windows/ARM64/ARM32/macOS platforms.

## Requirements

- Python 3.6+
- `requests` library: `pip install requests`

## Authentication

The scripts require a github token and it can be provided from one of the following sources:

### 1. Command Line (Highest Priority)
```bash
./scopy-commit-build.py <commit_sha> --token your_github_token
```

### 2. Local Token File
Create a `github_token.txt` file:
```bash
echo "your_github_token" > github_token.txt
```

### 3. Git Credentials (Lowest Priority)
The script automatically reads from `~/.git-credentials` if configured:
```
https://username:token@github.com
```

## Usage

### Basic Usage
```bash
# Download and launch with commit SHA
./scopy-commit-build.py abc12347b84461f5704e351c80e38a02bdfebb54

# Use partial SHA (minimum 7 characters)
./scopy-commit-build.py abc1234

# Specify platform explicitly available options : x86, arm64, arm32, windows, macos, flatpak
./scopy-commit-build.py abc1234 --platform x86


# For more informations run 
./scopy-commit-build.py --help
```

### Platforms

| `--platform` | Artifact selected | Host detection |
|---|---|---|
| `x86` | `scopy-qt6-x86_64-appimage-ubuntu{2204,2404,2604}` | Reads `/etc/os-release`; picks the AppImage matching the host Ubuntu version, else falls back to the newest available (logged) |
| `macos` | `Scopy-macOS14-arm64-qt6` / `Scopy-macOS15-x86_64-qt6` | Uses `platform.machine()`; Apple Silicon → arm64, Intel → x86_64, else first available (logged) |
| `flatpak` | `scopy-x86_64-flatpak` (contains `Scopy.flatpak`) | Linux only; installed and launched via the `flatpak` CLI |
| `arm64` | `scopy-arm64-qt6-appimage` | — |
| `arm32` | `scopy-armhf-qt6-appimage` | — |
| `windows` | `scopy-windows-qt6-portable` | — |

### Flatpak Usage
```bash
# Linux host with the flatpak CLI installed
./scopy-commit-build.py abc1234 --platform flatpak
```
This downloads and extracts the Flatpak bundle, then runs
`flatpak install --user -y Scopy.flatpak` and `flatpak run org.adi.Scopy`.
If the `flatpak` CLI is not installed, the bundle is downloaded but not launched,
and its path is printed. Flatpak is Linux-only and is never auto-detected — it
must be requested explicitly with `--platform flatpak`.

### macOS Usage
```bash
./scopy-commit-build.py abc1234 --platform macos
```

## How It Works

### Commit SHA Handling
- **Full SHA**: Direct API lookup with `?head_sha=<full_sha>`
- **Partial SHA**: Searches through the first 5 paginated workflow runs for substring matches (for older commits we recommend using full SHA due to processing times)
- **Minimum length**: 7 characters required for partial matching


## Notes

- Script automatically detects platform if not specified (flatpak excluded — opt-in only)
- On Linux, the host Ubuntu version selects the matching x86_64 AppImage
- On macOS, the host CPU arch selects the matching build
- Downloads are cached locally with timestamps
- Nested ZIP files are automatically extracted
- Linux/ARM64 executables get proper execute permissions
- Windows executables are launched via shell
- macOS apps have quarantine attributes removed
- Flatpak bundles are installed and launched via the `flatpak` CLI (`org.adi.Scopy`)