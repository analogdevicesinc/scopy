# Flatpak CI Directory

## Install Scopy using the Flatpak

1. **Install Flatpak using [these instructions](http://flatpak.org/getting.html)**
2. **Download **Scopy.flatpak** from the [Releases Tab](https://github.com/analogdevicesinc/scopy/releases)**
3. **Install the application using:**

```bash
   flatpak install Scopy.flatpak
```

4. **Run the application using:**

```bash
   flatpak run org.adi.Scopy
```

## Overview

This directory contains scripts and configuration for building Scopy as a Flatpak package for Linux. Flatpak provides a sandboxed, distribution-independent packaging format that works across different Linux distributions.

### Scripts

#### `flatpak_build_process.sh` - Main build orchestration script for Flatpak builds

#### `build_flatpak_deps.sh`- Builds Flatpak dependencies

### Docker Support

#### `create_docker_image.sh` - Creates Docker image for Flatpak builds

#### `docker/Dockerfile`- Defines the Docker image for Flatpak builds

### Build Configuration

#### `Makefile` - Contains the compile commands for building Flatpak

#### `org.adi.Scopy.json.c` - Template file for generating the Flatpak manifest

#### `shared-modules/` - Git submodule containing shared Flatpak modules

## Building the Docker image

To build the Docker image just execute the ***create_docker.sh*** script.

```bash
   ./create_docker.sh
```

The provided Dockerfile will install the KDE Runtime over the Ubuntu 20.04 base image. This Docker image is built in such a way that it contains the dependencies needed for the packaging of the Scopy application. It leverages the Flatpak Builder caching system, where after each step in the build process the result is saved as cache in order to be reused for later builds.

`TIP:` To build the Flatpak package for Scopy inside this Docker image, it needs to be run using ***--privileged***, otherwise there is a lack of access to necessary utilities.

## Generating the Scopy.flatpak artifact

`Prerequisites:`

- flatpak-builder
- flatpak platform and sdk

All of these can be installed using:

```bash
   sudo apt install flatpak jq flatpak-builder
   sudo flatpak remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
   sudo flatpak install flathub org.kde.Platform//5.15-23.08 -y
   sudo flatpak install flathub org.kde.Sdk//5.15-23.08 -y
```

Running ```make``` inside the ***scopy/ci/flatpak*** folder will build the Scopy.flatpak artifact.
This command will build the whole dependency tree and create an installer.

## Build Process

### Build using the Docker Image

1. Pull Docker image

   ```bash
      docker pull cristianbindea/scopy2-flatpak:latest
   ```

2. Run container with privileged access and bind the local Scopy repo to a volume inside the container

   ```bash
       docker run -it --privileged --mount type=bind,source=path/to/scopy/repository,target=/home/runner/scopy cristianbindea/scopy2-flatpak:latest
   ```

3. Execute the build script inside container

   ```bash
      /home/runner/scopy/flatpak_build_process.sh
   ```

   Script handles:

- Dependency setup from cache
- Manifest generation
- Flatpak build execution

### For Local Development

1. Initialize submodules `git submodule update --init`

2. Build Flatpak using: `make`

   This will build all the projects listed in the manifest, the last one being Scopy.

3. Output: *Scopy.flatpak* installer in current directory

## Manifest Structure

The manifest file `org.adi.Scopy.json` defines:

- Permissions required by the application
- Build dependencies
- The order of building the dependencies
- Source locations
- Build commands

Flatpak documentation for [manifest structure](https://docs.flatpak.org/en/latest/manifests.html).

## Caching System

The Docker image leverages Flatpak Builder's caching system:

- Each build step result is cached
- Subsequent builds reuse cached results
- CI copies pre-built cache from Docker image
- The result is that using the Docker Image, one can build only the Scopy app with the rest of the dependencies already cached

## GitHub Actions Integration

- Workflow: `.github/workflows/linuxflatpakbuild.yml`

## Notes

- Flatpak provides sandboxed environment for application
- All dependencies are bundled in the package
- Provides consistent runtime across Linux distributions
