# Qt6 CI Docker Images & Pipelines Plan

> **Date:** 2026-05-06
> **Branch:** `qt6_clean`
> **Status:** Phase 1 in progress
> **Scope:** CI Docker images for all platforms, published to Cloudsmith

---

## Context

Scopy's Qt6 migration (branch `qt6_clean`) compiles 100% locally inside a Docker image built from `ci/ubuntu/docker_ubuntu/Dockerfile.qt6`. The next step is to publish Docker images to a registry and wire them into CI so every push is automatically built and tested across all platforms.

**Current state:** Qt5 Docker images live on Docker Hub (`cristianbindea/` account). Qt6 images will move to **Cloudsmith** (`docker.cloudsmith.io/adi/scopy-dockers/`). Qt5 and Qt6 pipelines run in parallel — Qt5 on `main`, Qt6 on `qt6_clean`.

**Registry:** `docker.cloudsmith.io/adi/scopy-dockers/<image>:<tag>`
**Auth:** API key (see [Auth section](#cloudsmith-auth) for decision rationale)
**Qt version:** 6.8.3 everywhere
**M2K:** disabled on all Qt6 builds (`-DENABLE_PACKAGE_M2K=OFF`)

---

## Phase 1: Ubuntu 24 Qt6 → Cloudsmith (Simplest — already works locally)

### Status: In progress — image pushed, CI auth wired up, awaiting first green build

### What was done

**1.1** ✅ Updated `ci/ubuntu/create_docker_image.sh` — `ubuntu24_qt6()` tag now points to `docker.cloudsmith.io/adi/scopy-dockers/scopy2-ubuntu24-qt6:testing`

**1.2** ✅ Created `.github/workflows/push-docker-qt6.yml` — manual `workflow_dispatch` workflow that builds and pushes Qt6 Docker images to Cloudsmith using API key auth

**1.3** ✅ Replaced `PLACEHOLDER_QT6_IMAGE` in `.github/workflows/ubuntubuild-qt6.yml` with actual Cloudsmith image reference

**1.4** ✅ Updated `.github/workflows/ci-qt6.yml` — passes `CLOUDSMITH_API_KEY` secret to child workflow

**1.5** ✅ Updated `.github/workflows/ubuntubuild-qt6.yml` — added `credentials:` block for private registry auth and declared `CLOUDSMITH_API_KEY` secret in `workflow_call`

**1.6** ✅ Updated `.github/workflows/get_docker_tag.yml` — `qt6_*` branches now map to `testing` tag (previously defaulted to `latest`)

**1.7** ✅ Updated `.github/workflows/ci.yml` — added `branches-ignore: ['qt6_*']` to push and pull_request triggers so Qt5 CI doesn't run on Qt6 branches

**1.8** ✅ Docker image pushed to Cloudsmith manually via `docker push` from local machine

### Decisions made

| Decision | Choice | Reason |
|----------|--------|--------|
| Auth method | API key (`CLOUDSMITH_API_KEY` GitHub secret) | OIDC requires Cloudsmith org admin to configure an OIDC service. Using personal API key is simpler during initial development when only the CI setup team uses the images. Will migrate to OIDC when images go to production use across the org. |
| Registry name | `scopy-dockers` (not `scopy-docker`) | Matches the actual Cloudsmith repository name created on the web UI |
| Docker tag for qt6 branches | `testing` | `get_docker_tag.yml` defaults to `latest` for non-testing branches. Added `qt6_*` pattern to map to `testing` since Qt6 images are not yet production-ready |
| Qt5 CI on qt6 branches | Disabled via `branches-ignore` | Qt5 `ci.yml` had no branch filter, causing it to trigger on `qt6_clean` pushes and always fail. Used `branches-ignore: ['qt6_*']` wildcard to cover all future Qt6 branches |
| First image push method | Manual `docker push` from local machine | Faster than debugging GitHub Actions workflow visibility. The `push-docker-qt6.yml` workflow only appears in the Actions UI when it exists on the default branch (`main`). Can be triggered via CLI: `gh workflow run push-docker-qt6.yml --ref qt6_clean` |

### Files modified
| File | Change |
|------|--------|
| `ci/ubuntu/create_docker_image.sh` | Update tag in `ubuntu24_qt6()` to Cloudsmith registry |
| `.github/workflows/ubuntubuild-qt6.yml` | Replace placeholder, add registry credentials, declare secret |
| `.github/workflows/ci-qt6.yml` | Pass `CLOUDSMITH_API_KEY` secret to child workflow |
| `.github/workflows/ci.yml` | Add `branches-ignore: ['qt6_*']` to prevent Qt5 CI on Qt6 branches |
| `.github/workflows/get_docker_tag.yml` | Map `qt6_*` branches to `testing` tag |

### Files created
| File | Purpose |
|------|---------|
| `.github/workflows/push-docker-qt6.yml` | Manual Docker image push to Cloudsmith (API key auth) |

### Verification
- [x] `docker push docker.cloudsmith.io/adi/scopy-dockers/scopy2-ubuntu24-qt6:testing` succeeds
- [ ] CI build on `qt6_clean` pulls image and compiles Scopy
- [ ] `ctest` passes with `QT_QPA_PLATFORM=offscreen`

### Setup completed
- Cloudsmith `scopy-dockers` repository created (Docker format, under `adi` org)
- `CLOUDSMITH_API_KEY` GitHub secret added to repository

---

## Phase 2: Windows Qt6 Docker (Moderate complexity)

### Current Qt5 approach
Windows uses MSYS2/MinGW64 on Windows Server LTSC 2022. Qt5 comes from `mingw-w64-x86_64-qt5` via pacman.

### Qt6 approach
Replace pacman Qt5 with aqtinstall Qt 6.8.3 (`win64_mingw` arch). Everything else stays MSYS2/MinGW64.

### Steps

**2.1** Create `ci/windows/mingw_toolchain_qt6.sh` — copy of `mingw_toolchain.sh` with:
- Qt6 path: `QT=/c/Qt/6.8.3/mingw_64`
- Updated branches: `KDDOCK_BRANCH=2.2`, `ECM_BRANCH=v6.8.0`, `KARCHIVE_BRANCH=v6.8.0`

**2.2** Create `ci/windows/windows_build_process_qt6.sh` — based on `windows_build_process.sh`:
- Remove `mingw-w64-x86_64-qt5` from pacman deps
- Add `install_qt()` using aqtinstall: `aqt install-qt --outputdir /c/Qt windows desktop 6.8.3 win64_mingw -m qt3d qtscxml`
- Add `-DKDDockWidgets_QT6=ON` to kddock build
- Add `-DENABLE_PACKAGE_M2K=OFF` to scopy build

**2.3** Create `ci/windows/build_and_create_installer_qt6.sh` — based on `build_and_create_installer.sh`:
- Use `windeployqt6.exe` instead of `windeployqt.exe`
- Update DLL references from `Qt5*.dll` to `Qt6*.dll`

**2.4** Create `ci/windows/docker/Dockerfile.qt6` — based on existing Dockerfile:
- Same Windows Server LTSC 2022 + MSYS2 base
- COPY and run Qt6 scripts instead of Qt5

**2.5** Update `ci/windows/.dockerignore` — whitelist Qt6 scripts

**2.6** Create `.github/workflows/mingwbuild-qt6.yml` — based on `mingwbuild.yml`:
- Image: `docker.cloudsmith.io/adi/scopy-dockers/scopy2-mingw64-qt6:<tag>`
- Runs `build_and_create_installer_qt6.sh run_workflow`

**2.7** Wire into `ci-qt6.yml` — add `mingw-qt6` job

**2.8** Expand `push-docker-qt6.yml` — add Windows platform option (needs `windows-2022` runner)

### Files created
| File | Purpose |
|------|---------|
| `ci/windows/mingw_toolchain_qt6.sh` | Qt6 Windows toolchain config |
| `ci/windows/windows_build_process_qt6.sh` | Qt6 Windows build script |
| `ci/windows/build_and_create_installer_qt6.sh` | Qt6 Windows installer creation |
| `ci/windows/docker/Dockerfile.qt6` | Qt6 Windows Docker image |
| `.github/workflows/mingwbuild-qt6.yml` | Qt6 Windows CI workflow |

### Files modified
| File | Change |
|------|--------|
| `ci/windows/.dockerignore` | Whitelist Qt6 scripts |
| `.github/workflows/ci-qt6.yml` | Add Windows Qt6 job |
| `.github/workflows/push-docker-qt6.yml` | Add Windows platform |

### Key risk
Need to verify aqtinstall `win64_mingw` arch exists for Qt 6.8.3. If not, may need `win64_msvc2022_64` + MSVC toolchain (bigger change).

### Verification
- aqtinstall downloads Qt 6.8.3 for MinGW inside Docker
- All deps build (KDDockWidgets 2.2, ECM v6.8.0, KArchive v6.8.0)
- Scopy compiles, Windows installer is produced

---

## Phase 3: macOS Qt6 on Azure Pipelines (No Docker)

### Current Qt5 approach
Azure Pipelines runner, Homebrew `qt@5`, deps installed via `install_macos_deps.sh`.

### Qt6 approach
Replace Homebrew Qt5 with aqtinstall Qt 6.8.3 (arch: `clang_64`, or `mac_arm64` for Apple Silicon if available).

### Steps

**3.1** Create `ci/macOS/macos_config_qt6.sh` — based on `macos_config.sh`:
- Remove `QT_FORMULAE=qt@5` and Homebrew Qt path
- Add aqtinstall Qt6 path: `QT=$HOME/Qt/6.8.3/macos` (or arch-specific)
- Updated branches: `KDDOCK_BRANCH=2.2`, `ECM_BRANCH=v6.8.0`, `KARCHIVE_BRANCH=v6.8.0`

**3.2** Create `ci/macOS/install_macos_deps_qt6.sh` — based on `install_macos_deps.sh`:
- Remove `${QT_FORMULAE}` from Homebrew packages
- Add `install_qt()` using aqtinstall for macOS
- Add `-DKDDockWidgets_QT6=ON`

**3.3** Create `ci/macOS/build_azure_macos_qt6.sh` — based on `build_azure_macos.sh`:
- Source `macos_config_qt6.sh`
- Add `-DENABLE_PACKAGE_M2K=OFF`

**3.4** Create `azure-pipelines-qt6.yml` — based on existing Azure config:
- Trigger on `qt6_clean` branch
- Use Qt6 scripts
- Update cache keys with `qt6` prefix

### Files created
| File | Purpose |
|------|---------|
| `ci/macOS/macos_config_qt6.sh` | Qt6 macOS config |
| `ci/macOS/install_macos_deps_qt6.sh` | Qt6 macOS dep installation |
| `ci/macOS/build_azure_macos_qt6.sh` | Qt6 macOS build |
| `azure-pipelines-qt6.yml` | Qt6 Azure pipeline |

### Key risk
Need to verify aqtinstall `mac_arm64` arch exists for Qt 6.8.3 (for Apple Silicon runners). Fallback: `clang_64` works via Rosetta 2.

### Verification
- aqtinstall downloads Qt 6.8.3 on macOS
- Scopy builds and produces `.dmg`
- Both Intel and Apple Silicon runners work

---

## Phase 4: ARM64 + ARM32 Qt6 Docker (Most complex)

### Current Qt5 approach
Cross-compile Qt5 from source using `build_qt.sh` + qmake. Uses Kuiper Linux sysroot + cross-compiler toolchain.

### Qt6 approach
Qt6 cross-compilation is fundamentally different from Qt5:
- Qt6 uses **CMake** (not qmake) for its own build
- Requires a **host Qt6** installation first (for moc, rcc, uic tools)
- Uses `-DQT_HOST_PATH=<host-qt>` instead of `-device <device-name>`
- Needs Ninja build system

**aqtinstall does NOT have ARM binaries for Linux** — only `linux_gcc_64`. So Qt6 must be cross-compiled from source.

### Steps

**4.1** Create `ci/arm/build_qt6.sh` — cross-compile Qt 6.8.3:
1. Install host Qt6 via aqtinstall (`linux_gcc_64`)
2. Download Qt 6.8.3 source tarball
3. Configure with CMake:
   - `-DQT_HOST_PATH=/opt/Qt/6.8.3/gcc_64`
   - `-DCMAKE_TOOLCHAIN_FILE=<arm-toolchain>`
   - `-DCMAKE_SYSROOT=$SYSROOT`
4. Build and install

**4.2** Create `ci/arm/arm_build_config_qt6.sh` — updated dependency branches and Qt6 paths

**4.3** Create `ci/arm/arm_build_process_qt6.sh` — based on `arm_build_process.sh`:
- Source `arm_build_config_qt6.sh`
- Add `-DKDDockWidgets_QT6=ON`, `-DENABLE_PACKAGE_M2K=OFF`
- Update AppImage creation for Qt6 libs (`libQt6*.so*`)

**4.4** Create `ci/arm/cmake_toolchain_qt6.cmake` — based on existing, update Qt paths from qt5 to qt6

**4.5** Create `ci/arm/docker/Dockerfile.qt6`:
- Base: Ubuntu 22.04 (Qt6 needs newer toolchain than 20.04)
- Multi-stage: sysroot → Qt6 cross-compile → dependency builds
- Install host Qt6 via aqtinstall in build stage

**4.6** Update `ci/arm/.dockerignore` — whitelist Qt6 scripts

**4.7** Create `ci/arm/create_docker_image_qt6.sh` — tags for Cloudsmith:
- `docker.cloudsmith.io/adi/scopy-dockers/scopy2-arm64-appimage-qt6:testing`
- `docker.cloudsmith.io/adi/scopy-dockers/scopy2-armhf-appimage-qt6:testing`

**4.8** Create workflows: `appimage-arm64-qt6.yml`, `appimage-armhf-qt6.yml`

**4.9** Wire into `ci-qt6.yml`

### Files created
| File | Purpose |
|------|---------|
| `ci/arm/build_qt6.sh` | Qt6 ARM cross-compilation |
| `ci/arm/arm_build_config_qt6.sh` | Qt6 ARM config |
| `ci/arm/arm_build_process_qt6.sh` | Qt6 ARM build orchestrator |
| `ci/arm/cmake_toolchain_qt6.cmake` | Qt6 ARM CMake toolchain |
| `ci/arm/docker/Dockerfile.qt6` | Qt6 ARM Docker image |
| `ci/arm/create_docker_image_qt6.sh` | Qt6 ARM image build script |
| `.github/workflows/appimage-arm64-qt6.yml` | Qt6 ARM64 CI workflow |
| `.github/workflows/appimage-armhf-qt6.yml` | Qt6 ARM32 CI workflow |

### Files modified
| File | Change |
|------|--------|
| `ci/arm/.dockerignore` | Whitelist Qt6 scripts |
| `.github/workflows/ci-qt6.yml` | Add ARM jobs + pass secrets |
| `.github/workflows/push-docker-qt6.yml` | Add ARM platforms |

### Key risks
- Qt6 cross-compilation is complex and may require significant debugging
- Docker image build time will be long (Qt6 source build: 1-3 hours)
- ARM32 support in Qt6 may have issues — Qt6 has reduced ARM32 focus
- Sysroot may need to be updated for Qt6 compatibility (newer glibc, etc.)

### Verification
- Qt 6.8.3 cross-compiles for both ARM64 and ARM32
- Scopy builds and produces AppImage
- AppImage runs on Raspberry Pi hardware

---

## Phase 5: Consolidation

**5.1** Finalize `push-docker-qt6.yml` with all 4 platforms (Ubuntu, Windows, ARM64, ARM32)

**5.2** Add tag promotion: re-tag `testing` → `latest` after validation

**5.3** Update `ci-qt6.yml` to include all platform jobs

---

## Implementation Order Summary

| Phase | Platform | Effort | Dependencies |
|-------|----------|--------|-------------|
| 1 | Ubuntu 24 Qt6 | ~1 day | Cloudsmith repo creation |
| 2 | Windows Qt6 | ~3-4 days | Phase 1 (push workflow), aqtinstall `win64_mingw` verification |
| 3 | macOS Qt6 | ~2-3 days | aqtinstall macOS arch verification |
| 4 | ARM64 + ARM32 | ~5-7 days | Phase 1, Qt6 cross-compilation expertise |
| 5 | Consolidation | ~1 day | Phases 1-4 |

**Total: ~12-16 days**

---

## Cloudsmith Auth

### Current approach: API key (2026-05-06)

Using a personal Cloudsmith API key stored as a GitHub secret (`CLOUDSMITH_API_KEY`). This is simpler and sufficient while Docker images are being developed and only used by the team setting up CI.

**Required setup:**
- GitHub secret: `CLOUDSMITH_API_KEY` (personal API key from Cloudsmith account → API page)

**TODO:** Migrate to OIDC auth when images are used in production CI across the org. OIDC eliminates static keys and uses short-lived tokens instead.

### Future: OIDC (reference)

1. GitHub Actions requests an OIDC token from GitHub's token endpoint
2. `cloudsmith-io/cloudsmith-cli-action@v2.0.2` exchanges the token for a Cloudsmith API key
3. The API key is stored as `CLOUDSMITH_API_KEY` env var
4. For Docker: `docker login docker.cloudsmith.io -u token -p $CLOUDSMITH_API_KEY`
5. For Debian: `cloudsmith push deb ...` uses the env var automatically

**Required setup:**
- GitHub secret: `CLOUDSMITH_SERVICE_SLUG`
- Permissions: `id-token: write`
- Cloudsmith: OIDC service configured for the GitHub repo

This is the same pattern already working for ARM Debian uploads in `appimage-arm64.yml`.

---

## Docker Image Naming Convention

| Platform | Image Name |
|----------|-----------|
| Ubuntu 24 Qt6 | `docker.cloudsmith.io/adi/scopy-dockers/scopy2-ubuntu24-qt6` |
| Windows Qt6 | `docker.cloudsmith.io/adi/scopy-dockers/scopy2-mingw64-qt6` |
| ARM64 Qt6 | `docker.cloudsmith.io/adi/scopy-dockers/scopy2-arm64-appimage-qt6` |
| ARM32 Qt6 | `docker.cloudsmith.io/adi/scopy-dockers/scopy2-armhf-appimage-qt6` |

Tags: `testing` (dev) and `latest` (promoted after validation)
