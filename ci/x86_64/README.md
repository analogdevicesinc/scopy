# x86_64 CI Directory

## Folder overview

This directory contains scripts for building Scopy as an AppImage for x86_64 Linux systems. AppImage is a portable application format that runs on most Linux distributions without installation.

### Scripts

#### `x86-64_appimage_process.sh`- Main build script for creating x86_64 AppImage

#### `copy-deps.sh`- Copies runtime dependencies into AppDir

### Docker Support

#### `create_docker_image.sh`- **Purpose**: Creates Docker image using Ubuntu20 or Ubuntu24 as starting image

#### `docker/Dockerfile`- Defines x86_64 build environment

## Build Process

### Building Scopy using the Docker Image

`Prerequisites:`

- [Docker](https://docs.docker.com/desktop/setup/install/windows-install/)
- the rest of the dependencies are installed using the scripts

1. Pull the Docker image

   ```bash
      docker pull cristianbindea/scopy2-x86_64-appimage
   ```

2. Run build in container, and bind the scopy repo to a volume inside the container

   ```bash
      docker run -v $(pwd):/scopy cristianbindea/scopy2-x86_64-appimage
   ```

3. Execute the build script inside the container

   ```bash
      scopy/ci/x86-64_appimage_process.sh
   ```

### Building the Docker Image

To build the Docker image, just run the script and select the required architecture.

   ```bash
      ci/x86_64/create_docker_image.sh ubuntu20
      # or
      ci/x86_64/create_docker_image.sh ubuntu24
   ```

### Building locally from sources

Run build script:

   ```bash
      ./x86-64_appimage_process.sh configure_system generate_appimage
   ```

## AppImage Creation Process

1. **Build Phase**:
   - Compiles Scopy

2. **AppDir Assembly**:
   - Creates AppDir directory structure
   - Copies executables to AppDir/usr/bin
   - Copies libraries to AppDir/usr/lib
   - Copies the rest of the required files in AppDir

3. **Dependency Resolution**:
   - Runs `copy-deps.sh` to bundle libraries
   - Includes Qt plugins and platform files
   - Bundles required system libraries

4. **AppImage Generation**:
   - Uses appimagetool to create final image
   - Embeds AppRun and desktop files
   - Creates self-contained executable

## Output

- **scopy.AppDir/**: Unpacked application directory (for debugging)
- **Scopy-x86_64.AppImage**: Portable application

## AppDir Structure

```text
scopy.AppDir/
├── AppRun           # Entry point script
├── scopy.desktop    # Desktop integration
├── scopy.png        # Application icon
└── usr/
    ├── bin/         # Executables
    │   └── scopy
    ├── lib/         # Libraries
    │   ├── *.so     # Shared libraries
    │   └── qt5/     # Qt plugins
    └── share/       # Resources
        └── scopy/   # Application data
```

## Running the AppImage

```bash
   chmod +x Scopy-x86_64.AppImage
   ./Scopy-x86_64.AppImage
```

You can even extract and inspect the files inside the AppImage:

```bash
   ./Scopy-x86_64.AppImage --appimage-extract
```

## CI Integration

- **GitHub Actions**: `.github/workflows/appimage-x86_64.yml`

## Troubleshooting

Common issues:

- **"No such file"**: Make executable with `chmod +x`
- **Missing libraries**: Run `QT_DEBUG_PLUGINS=1 ./Scopy-x86_64.AppImage` for details
- **FUSE errors**: Extract with `--appimage-extract` if FUSE unavailable

## Notes

- AppImage provides portability
- No root permissions required
- Compatible with most x86_64 Linux distributions
- Works on systems without Qt installed
- Larger file size due to bundled dependencies
- Can run from USB drives or network shares
- Updates require downloading new AppImage
