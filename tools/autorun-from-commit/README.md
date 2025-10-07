# Scopy Autorun from Commit Scripts

Automated tools for downloading and launching Scopy builds from CI/CD artifacts based on commit SHA.

## Scripts

### `scopy-commit-build.py`
Downloads Scopy builds from **GitHub Actions** for Linux/Windows/ARM64 platforms.

### `scopy-commit-build-macos.sh`
Downloads Scopy builds from **Azure DevOps** for macOS platform.

## Requirements

### Python Script Dependencies
- Python 3.6+
- `requests` library: `pip install requests`

### Bash Script Dependencies (macOS)
- `curl`, `jq`, `unzip`, `base64`
- Install with: `brew install curl jq unzip`

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

# Specify platform explicitly available options : x86, arm64, arm32, windows
./scopy-commit-build.py abc1234 --platform x86


# For more informations run 
./scopy-commit-build.py --help
```

### macOS Usage
```bash
# Basic usage
./scopy-commit-build-macos.sh abc123467b84461f5704e351c80e38a02bdfebb54

# With token parameter
./scopy-commit-build-macos.sh f6d3466 github_token
```

## How It Works

### Commit SHA Handling
- **Full SHA**: Direct API lookup with `?head_sha=<full_sha>`
- **Partial SHA**: Searches through the first 5 paginated workflow runs for substring matches (for older commits we recommend using full SHA due to processing times)
- **Minimum length**: 7 characters required for partial matching


## Notes

- Script automatically detects platform if not specified
- Downloads are cached locally with timestamps
- Nested ZIP files are automatically extracted
- Linux/ARM64 executables get proper execute permissions
- Windows executables are launched via shell
- macOS apps have quarantine attributes removed