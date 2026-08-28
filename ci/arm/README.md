# ARM CI Directory

## Folder overview

This directory contains scripts and configuration for building Scopy for ARM platforms (both 32-bit armhf and 64-bit aarch64). The build process uses cross-compilation toolchains and creates AppImage packages for ARM devices like Raspberry Pi. More information about this build can be found here [Creating an Installer for other architectures](https://analogdevicesinc.github.io/scopy/user_guide/build_instructions/linuxBuild.html#creating-an-installer-for-other-architectures).

### Scripts

#### `arm_build_config.sh` - Configuration file that sets up build environment variables

#### `arm_build_process.sh` - Main build orchestration script

#### `create_sysroot.sh` - Creates a sysroot from ADI Kuiper Linux image, the sysroot is used during cross-compilation

#### `build_qt.sh` - Cross-compiles Qt 5.15.16 for ARM

#### `create_docker_image_qt6.sh` - Creates the Qt6 Docker images for ARM builds (`arm64` native, `armhf_cross`)

#### `setup_pi_for_docker_build.sh` - Prepares a Kuiper arm64 Raspberry Pi and produces the two tarballs `docker/Dockerfile.arm64` needs

#### `docker/Dockerfile` - Base image configuration for ARM build environment

#### `copy-deps.sh` - Copies runtime dependencies into AppImage directory

#### `inside_chroot_arm64.sh` / `inside_chroot_armhf.sh` - Scripts that run inside chroot environment

#### `local_build_scopy_for_kuiper.sh` - Local build script specifically for Kuiper Linux

### Supporting Files

#### `cmake_toolchain.cmake` - Contains the toolchain used for cross-compiling

#### `qt_patch_arm32/arm64.patch` - A patch to some Qt files to be able to cross-compile it for arm32/64

#### `runtime-armhf/aarch64` - This is the runtime used inside the AppImage

## Build Process

### Building Scopy using the Docker Image

`Prerequisites:`

- [Docker](https://docs.docker.com/desktop/setup/install/windows-install/)
- the rest of the dependencies are installed using the scripts

This is by far the easiest method of building an armhf or arm64 Scopy AppImage.

A temporary Docker volume is created to link the local environment with the Docker container. The compilation process takes place inside the container using
the pre-configured filesystem and all the changes inside the volume will reflect to the locally stored files.

The next steps will describe how to build Scopy and create a armhf(arm32) AppImage, the process is the same for arm64(aarch64), just change the argument.

1. **Install the packages needed for emulating the required architecture**

   ```bash
      ci/arm/create_sysroot.sh arm32 install_packages install_qemu
   ```

2. **Pull the Docker image**

   ```bash
      docker pull cristianbindea/scopy2-armhf-appimage:latest
   ```

3. **Run the image, while creating a docker volume**

   ```bash
      docker run -it --mount type=bind,source=path/to/scopy/repository,target=/home/runner/scopy cristianbindea/scopy2-armhf-appimage:latest
   ```

4. **Using the Docker environment you can compile and package the application with one command:**

   ```bash
      /home/runner/scopy/ci/arm/arm_build_process.sh arm32 run_workflow
   ```

`TIP:` Inspect the arm_build_process.sh file. You can call any function inside that file using **$ ./arm_build_process.sh architecture function**.
This way you can only build Scopy, you can create an AppDir folder or even run the whole workflow and create the AppImage.

Finally, after the development is done use this to clean the system

   ```bash
      docker container ls -a # get the container id
      docker container rm -v (container id)
      docker image rm cristianbindea/scopy2-armhf-appimage:latest
   ```

### Building the Docker Image

The tag defaults to `testing`, the staging tag a freshly built image lands on. `main` and ordinary
branches currently build against `:slim` (the reduced dependency set), by agreement, until `:slim` is
retagged to `:latest` and `get_docker_tag.yml`'s default is flipped — so pass `TAG=slim` when
rebuilding an image `main` depends on. `:latest` is not in use yet.

The **armhf cross image** builds on any x86_64 host and downloads its prebuilt tarballs from
Cloudsmith:

   ```bash
      ci/arm/create_docker_image_qt6.sh armhf_cross
   ```

The **arm64 native image** must be built on an arm64 host, because `docker/Dockerfile.arm64`
compiles the whole dependency stack natively. Run `setup_pi_for_docker_build.sh` on a Kuiper
arm64 Raspberry Pi first - it writes `kuiper-rootfs.tar.gz` and `qt6-arm64-installed.tar.gz`
into `ci/arm/`, where the Dockerfile reads them from:

   ```bash
      sudo ci/arm/setup_pi_for_docker_build.sh
      sudo ci/arm/create_docker_image_qt6.sh arm64
   ```

### Building locally from sources

In order to build Scopy locally you will need a device with the armhf or arm64 architecture, for example a Raspberry Pi.

To build the app directly on the Raspberry Pi, just run:

   ```bash
      ci\arm\local_build_scopy_for_kuiper.sh install_apt clone buid_deps build_scopy
   ```

This script is made for [Kuiper Linux](https://wiki.analog.com/resources/tools-software/linux-software/kuiper-linux), but you can adapt it for another distro by updating the packages installed via apt.

## CI Integration

- **GitHub Actions**: `.github/workflows/appimage-armhf.yml`
- **GitHub Actions**: `.github/workflows/appimage-arm64.yml`

## Notes

- Uses ADI Kuiper Linux as the base system
- Requires significant disk space (~15GB) for sysroot and build artifacts
- Build time can be several hours depending on hardware
