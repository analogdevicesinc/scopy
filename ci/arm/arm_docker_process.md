# ARM Native Docker Image Build -- Issue Tracker

**Images:** Kuiper Linux ARM64 (`image_2026-04-23-ADI-Kuiper-Linux-arm64.img`), Kuiper Linux ARM32 (`image_2026-04-23-ADI-Kuiper-Linux-armhf.zip`)
**Dockerfiles:** `ci/arm/docker/Dockerfile.arm64`, `ci/arm/docker/Dockerfile.armhf`
**Branch:** `scopy_qt6_dev`
**Started:** 2026-05-19

---

## Issues

### #1 -- Docker build context path wrong
**Error:** `lstat docker: no such file or directory`
**Cause:** `-f docker/Dockerfile.arm64` was a relative path but the script didn't `cd` into `ci/arm/`.
**Fix:** Changed to absolute path `-f "$SRC_DIR/ci/arm/docker/Dockerfile.arm64"` in `create_docker_image_arm64_native_qt6.sh`.

### #2 -- `software-properties-common` not available
**Error:** `E: Unable to locate package software-properties-common`
**Cause:** Kuiper is Debian-based, not Ubuntu. `software-properties-common` (provides `add-apt-repository`) is Ubuntu-specific.
**Fix:** Removed from Dockerfile -- not needed since we don't add PPAs.

### #3 -- `sudo` fails inside Docker build
**Error:** `sudo: effective uid is not 0, is /usr/bin/sudo on a file system with the 'nosuid' option set`
**Cause:** `configure_system` was running as `USER runner` which can't use `sudo` during Docker build.
**Fix:** Moved `COPY` + `RUN configure_system` before the `USER $USER` directive so it runs as root.

### #4 -- `python3.11` / `python3.12` not available
**Error:** `E: Unable to locate package python3.11` (then `python3.12` after first fix attempt)
**Cause:** Kuiper is Debian Trixie 13 with **Python 3.13.5**. The versioned package names don't exist.
**Fix:** Auto-detect Python version at runtime via `python3 -c 'import sys; ...'`. Use unversioned package names (`python3-dev`) instead of versioned ones.

### #5 -- DNS not configured inside container
**Error:** `wget: unable to resolve host address 'github.com'`
**Cause:** Container inherited no DNS config from the Kuiper rootfs.
**Fix:** Run container with `--dns 8.8.8.8`. Committed container state first to preserve installed packages.

### #6 -- `qtgraphs` and `qtquick3dphysics` depend on skipped `qtquick3d`
**Error:** `Module 'qtgraphs'/'qtquick3dphysics' depends on 'qtquick3d', but building 'qtquick3d' was explicitly disabled.`
**Cause:** Qt6 dependency chain -- both modules require `qtquick3d` which is in our skip list.
**Fix:** Added `-skip qtgraphs -skip qtquick3dphysics` to the Qt6 configure command.

### #7 -- QEMU segfault building `qtpositioning`
**Error:** `Segmentation fault (core dumped)` compiling `clip2tri.cpp` in qtpositioning
**Cause:** QEMU user-mode emulation crashes on certain C++ code patterns, especially under high parallelism.
**Fix:** Added `-skip qtpositioning` (Scopy doesn't use it). Reduced Qt build parallelism to 4 jobs (`QT_BUILD_JOBS` env var, default 4).

### #8 -- QEMU segfaults continue at --parallel 4
**Error:** `c++: internal compiler error: Segmentation fault` on various files (qhooks.cpp, qqmlcomponent.cpp)
**Cause:** QEMU user-mode emulation is unreliable with multiple parallel GCC processes.
**Fix:** Reduced to `--parallel 1`. Build dir on host volume mount so progress survives container restarts.

### #9 -- Pi ran out of disk space (first attempt, small SD card)
**Error:** `fatal error: cannot write PCH file: No space left on device` at 5036/9141
**Cause:** Kuiper image rootfs was 27 GB, fully consumed by packages + Qt source + Qt build artifacts.
**Fix:** Got larger 64 GB SD card.

### #10 -- Debian mirror stale / apt 404 errors on Pi
**Error:** `404 Not Found` for multiple packages (libpython3.13, libglib2.0-dev, libssl-dev, etc.)
**Cause:** Default `deb.debian.org` CDN was serving stale package lists for Trixie (fast-moving release).
**Fix:** Switched to `ftp.debian.org` mirror: `sudo sed -i 's|http://deb.debian.org|http://ftp.debian.org|g' /etc/apt/sources.list` then `apt-get clean && apt-get update`.

### #11 -- SSL certificate errors on Pi
**Error:** `The certificate of 'github.com' is not yet activated`
**Cause:** Pi's system clock was set to 2026-04-23 (date baked into Kuiper image), making SSL certs appear "not yet valid."
**Fix:** `sudo date -s "2026-05-XX HH:MM:SS"`

### #12 -- QEMU binfmt lost after reboot
**Error:** `exec format error` when running ARM64 container
**Cause:** QEMU binfmt registrations don't survive host reboots.
**Fix:** Re-register with `sudo docker run --rm --privileged multiarch/qemu-user-static --reset -p yes`

### #13 -- `/tmp` missing inside Docker container
**Error:** `Unable to mkstemp /tmp/apt.sig.xxx - GetTempFile (2: No such file or directory)`
**Cause:** Rootfs tarball excluded `/tmp` directory.
**Fix:** Added `RUN mkdir -p /tmp /var/tmp && chmod 1777 /tmp /var/tmp` to Dockerfile before apt-get.

### #14 -- Docker cleanup deletes build artifacts (permission denied)
**Error:** `rm: cannot remove '/home/runner/scripts/staging/...': Permission denied`
**Cause:** Cleanup step ran as `USER runner` but staging files were created by root.
**Fix:** Moved cleanup `RUN` lines before `USER` switch so they run as root.

### #15 -- Docker image squash doubles disk usage
**Error:** `no space left on device` during `FROM scratch` / `COPY --from=0 / /`
**Cause:** The squash step copies the entire filesystem, doubling disk usage on the 64 GB SD card.
**Fix:** Removed `FROM scratch` / `COPY --from=0` squash step.

### #16 -- `containerd` fills disk with 49 GB of build cache
**Error:** `no space left on device` -- Docker can't even prune itself.
**Cause:** `/var/lib/containerd/` accumulated 49 GB of build layer data.
**Fix:** `sudo systemctl stop containerd && sudo rm -rf /var/lib/containerd && sudo systemctl start containerd`

### #17 -- `libfuse2` package renamed on ARM32 Trixie
**Error:** `E: Package 'libfuse2' has no installation candidate`
**Cause:** Package renamed to `libfuse2t64` on Debian Trixie ARM32.
**Fix:** Separated `libfuse2` from main apt-get call, added fallback: `apt-get install libfuse2 || apt-get install libfuse2t64`

### #18 -- `pkg-config` not installed on ARM32 Pi
**Error:** `QT_FEATURE_xkbcommon_x11 = "OFF"` despite dev packages being installed
**Cause:** `pkg-config` was not installed, so cmake couldn't find `.pc` files for xkbcommon-x11.
**Fix:** Ensured `pkg-config` is in the `install_packages` list.

### #19 -- Missing XCB/GL/EGL packages due to install_packages aborting early
**Error:** Qt6 configure fails with `WrapOpenGL_FOUND = FALSE`, `EGL_FOUND = FALSE`, `TEST_xcb_syslibs = FALSE`
**Cause:** `set -e` causes `apt-get install` to abort when one package fails (e.g., `libfuse2`), skipping everything after it.
**Fix:** Fixed `libfuse2` handling so main install completes. Added missing packages: `libgl-dev`, `libegl-dev`, `libgles-dev`, `libxcb-cursor-dev`, `libxcb-util-dev`, etc.

### #20 -- Qt6 install fails on missing `tracepointgen`/`tracegen`
**Error:** `file INSTALL cannot find ".../tracepointgen": No such file or directory`
**Cause:** Connection drop during `compile_qt6` left some targets unbuilt.
**Fix:** Re-run `cmake --build . --parallel $(nproc)` to build remaining targets, then `cmake --install . || true`.

### #21 -- `build_deps` partially installed after connection drop
**Error:** `genalyzer`, `kddockwidgets`, `libtinyiiod` missing from `/usr/local/lib/`
**Cause:** Connection drop interrupted `build_deps` partway through.
**Fix:** Rebuild missing deps individually: `build_genalyzer`, `build_kddock`, `build_libtinyiiod`.

### #22 -- CI pull fails: no matching manifest for linux/amd64
**Error:** `no matching manifest for linux/amd64 in the manifest list entries`
**Cause:** CI runner is x86 but Docker image is ARM64-only. `docker pull` defaults to host arch.
**Fix:** Added `--platform linux/arm64` to `docker pull` in CI workflow.

### #23 -- cmake deleted by Docker cleanup, unavailable in CI
**Error:** cmake not found when running `build_scopy` in CI container.
**Cause:** Dockerfile cleanup (`rm -rf /home/runner/scripts`) deletes the staging dir including cmake.
**Fix:** Added `download_cmake` before `build_scopy` in CI workflow command.

---

## Build Progress -- ARM64

| Step | Status | Notes |
|------|--------|-------|
| Extract rootfs | Done | 775M tarball from Pi's own rootfs |
| install_packages | Done | Committed to Docker image |
| build_qt6 | Done (Pi) | Qt 6.8.3 installed to /opt/Qt/6.8.3/gcc_arm64 |
| clone deps | Done (Pi) | All dependency repos cloned |
| build_deps | Done (Pi) | All dependencies compiled natively |
| build_scopy | Done (Pi) | Scopy compiled successfully |
| Docker image | Done | Built on Pi, pushed to Cloudsmith |
| CI workflow | In progress | Using ubuntu-24.04-arm runner |

## Build Progress -- ARM32

| Step | Status | Notes |
|------|--------|-------|
| install_packages | Done | Fixed libfuse2, pkg-config issues |
| download_cmake | Done | Using system cmake (no ARM32 prebuilt) |
| download_qt6_source | Done | |
| configure_qt6 | Done | Fixed XCB, GL, EGL, xkbcommon issues |
| compile_qt6 | Done | Connection drop required rebuild |
| install_qt6 | Done | tracepointgen/tracegen required manual rebuild |
| clone deps | Done | |
| build_deps | Partial | genalyzer, kddockwidgets, libtinyiiod, ECM, karchive need rebuild |
| build_scopy | In progress | QWT fixed, checking remaining deps |
| Docker image | Pending | |

## Lessons Learned
- QEMU binfmt resets on reboot -- always re-register before starting
- Use `--parallel 1` for QEMU builds -- anything higher causes random segfaults
- Mount build dirs as host volumes (`-v`) so progress survives container restarts
- Always commit container state after completing a major step
- 64 GB+ SD card required for Qt6 source build
- Debian Trixie mirrors can be stale -- use `ftp.debian.org` instead of `deb.debian.org`
- Kuiper image has baked-in date -- fix clock before any SSL operations
- `set -e` + `apt-get install` = one bad package aborts entire install -- handle problematic packages separately
- Split Docker `RUN configure_system` into individual `RUN` lines for layer caching
- Pre-build Qt6 on Pi and COPY tarball into Docker to skip 2-3 hour compile
- Run cleanup as root in Dockerfile (before `USER` switch)
- Don't use `FROM scratch` squash on space-constrained devices
- ARM32 has no prebuilt cmake binary -- use system cmake
- `libfuse2` -> `libfuse2t64` on ARM32 Trixie
