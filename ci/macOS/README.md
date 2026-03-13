# macOS CI Directory

## Overview

This directory contains scripts for building Scopy on macOS. The build process uses Homebrew for dependency management and creates a DMG installer for distribution.

### Scripts

#### `macos_config.sh`- Configuration file for macOS builds

#### `build_azure_macos.sh`- Main build script for Azure Pipelines

#### `install_macos_deps.sh`- Installs macOS build dependencies

#### `before_install_lib.sh`- Pre-installation setup for libraries (not used anymore)

#### `package_darwin.sh`- Creates the macOS DMG installer

## Build Process

### Prerequisites

- **Homebrew**: Package manager for macOS. Can be installed from here, [Homebrew Installation](https://docs.brew.sh/Installation).

### Build Steps

1. **Setup the environment and install the dependencies**:

   ```bash
      ./install_macos_deps.sh
   ```

   This will install packages using brew, so you will have to make sure that you have brew installed on the machine. The rest of the dependencies that can’t be found on brew will be built from the source files.

2. **Build Scopy**:

   ```bash
      ./build_azure_macos.sh
   ```

3. **Create Installer**:

   ```bash
      ./package_darwin.sh
   ```

   To run the application, the final step is linking the dependencies to the Scopy binary, enabling the operating system to locate them at runtime.

   This process is handled by a script that manages both the linking and packaging. Once the script completes, inside the build folder, it generates a file named Scopy.app, which can be opened either by running “open Scopy.app” in the terminal or by double-clicking it in the file explorer.

## Output

- **Scopy.app**: macOS application bundle
- **Scopy.dmg**: Distributable disk image installer

## CI Integration

- Built on Azure: See `azure-pipelines.yml` in repository root

## Notes

- Only x86_64 architecture is currently supported
- Apple Silicon (M1/M2) support is provided by [Rosetta](https://support.apple.com/en-us/102527)
- All dependencies must be built with same architecture
