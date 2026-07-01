Stage 1 — Create sysroot tarball from Kuiper image (in Docker)
  
  This extracts the Kuiper armhf rootfs, chroots into it with QEMU, installs all -dev packages, and produces sysroot-armhf.tar.gz.
  Everything runs inside a privileged Docker container — no sudo needed on the host.

  # 1a. Ensure QEMU binfmt is registered (one-time, already done on your machine)
  docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

  # 1b. Start a privileged container with ci/arm mounted (needs --privileged for loop mount + chroot)
  cd ~/claude/scopy

  docker run -it --rm --privileged \
      -v $(pwd)/ci/arm:/work \
      -w /work \
      ubuntu:24.04 \
      bash

  # 1c. Inside the container, install host tools and run sysroot creation:

  apt-get update && apt-get -y install sudo qemu-user-static wget python3 util-linux mount

  ./create_sysroot_armhf.sh extract_sysroot
  ./create_sysroot_armhf.sh configure_sysroot
  ./create_sysroot_armhf.sh fix_relativelinks

  # Package it
  cd staging
  tar -czf ../docker/tarballs/sysroot-armhf.tar.gz --transform='s|^sysroot|sysroot|' sysroot

  # Exit the container
  exit

  This produces: ci/arm/docker/tarballs/sysroot-armhf.tar.gz (~1.7GB)

  # 1d. Back on host — verify the result
  cd ~/claude/scopy/ci/arm
  ./verify_sysroot.sh


  ---
  Stage 2 — Build Qt6 host + cross tarballs (in Docker)
  
  This builds Qt6 twice: once for x86_64 (host tools: moc, rcc, uic), once cross-compiled for armhf.
  Runs inside a Docker container to guarantee all host dependencies are captured.
  Uses Dockerfile.qt6-build which has: ubuntu:24.04 + host build tools + sysroot from Stage 1.

  # 2a. Build the Qt6 build image
  cd ~/claude/scopy/ci/arm

  docker build \
      --progress plain \
      --tag scopy-qt6-build:latest \
      -f docker/Dockerfile.qt6-build \
      .

  # 2b. Clean any leftover staging from previous host runs (ownership conflicts)
  sudo rm -rf ci/arm/staging/qt-everywhere-src ci/arm/staging/build-qt6-host ci/arm/staging/build-qt6-armhf

  # 2c. Start an interactive container with the full scopy repo mounted
  cd ~/claude/scopy

  docker run -it --rm \
      -v $(pwd):/home/runner/scopy \
      -w /home/runner/scopy/ci/arm \
      scopy-qt6-build:latest \
      bash

  # 2d. Inside the container, run each step one by one:

  export SYSROOT=/opt/sysroot/armhf

  # Download Qt6 source (~1GB)
  ./build_qt6_armhf_cross.sh download

  # Build host tools — moc, rcc, uic (x86_64)
  ./build_qt6_armhf_cross.sh build_host

  # Configure cross-build (armhf)
  ./build_qt6_armhf_cross.sh configure

  # Build cross (armhf)
  ./build_qt6_armhf_cross.sh build

  # Install to /opt/Qt/6.8.3/armhf
  ./build_qt6_armhf_cross.sh install

  # Package tarballs (written to ci/arm/ via the mount)
  ./build_qt6_armhf_cross.sh package

  # Exit the container
  exit

  # 2e. Back on host — copy tarballs into the Docker tarballs directory
  cd ~/claude/scopy/ci/arm
  cp qt6-host-installed.tar.gz docker/tarballs/
  cp qt6-armhf-cross-installed.tar.gz docker/tarballs/

  This produces:
  - ci/arm/qt6-host-installed.tar.gz (~80MB) — x86_64 host tools
  - ci/arm/qt6-armhf-cross-installed.tar.gz (~53MB) — armhf cross libraries

  ---
  Stage 3 — Build the Docker image
  
  This takes the 3 tarballs and builds all Scopy third-party deps (libiio, gnuradio, qwt, etc.) inside the container.

  cd ~/claude/scopy/ci/arm

  docker build \
      --progress plain \
      --tag scopy-armhf-cross:latest \
      -f docker/Dockerfile.armhf-cross \
      .

  The Dockerfile copies arm_cross_build_process.sh, cmake_toolchain_armhf.cmake, and copy-deps.sh into the image, then runs clone and build_deps which cross-compiles all 17 third-party
  libraries.

  ---
  Stage 4 — Build Scopy + AppImage using the Docker image

  cd ~/claude/scopy

  docker run --rm \
      -v $(pwd):/home/runner/scopy \
      -w /home/runner/scopy \
      scopy-armhf-cross:latest \
      bash -c "./ci/arm/arm_cross_build_process.sh build_scopy generate_appimage"

  This produces: Scopy-armhf.AppImage in the repo root.
