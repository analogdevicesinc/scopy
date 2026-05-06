# Qt6 x86_64 AppImage CI Plan

> **Date:** 2026-05-06
> **Branch:** `qt6_clean`
> **Status:** Complete — tested locally in Docker, CI wired up
> **Depends on:** Phase 1 of `2026-05-06-qt6-ci-docker-plan.md` (Ubuntu 24 Qt6 Docker on Cloudsmith)

---

## Context

The Qt6 CI pipeline on `qt6_clean` currently builds and tests Scopy but does not produce an AppImage artifact. The Qt5 pipeline generates AppImages via a separate workflow (`appimage-x86_64.yml`) using a dedicated build script (`ci/x86_64/x86-64_appimage_process.sh`) and its own Docker image (`cristianbindea/scopy2-x86_64-appimage`).

**Goal:** Add a Qt6 AppImage CI job that produces a portable `.AppImage` artifact, reusing the existing Qt6 Docker image (`docker.cloudsmith.io/adi/scopy-dockers/scopy2-ubuntu24-qt6:testing`).

---

## Can the existing Qt6 Docker image be reused?

**Yes.** Here's why:

The Qt5 AppImage uses a separate Docker image only because it was set up independently — not because it requires fundamentally different tooling. Both the Qt6 regular build image and the Qt5 AppImage image:
- Install the same system build packages
- Build ALL dependencies from source and install to system paths (`/usr/local`)
- Install Qt via aqtinstall to `/opt/Qt/`

The AppImage-specific tools (linuxdeploy, linuxdeploy-plugin-qt, linuxdeploy-plugin-appimage) are **downloaded at runtime** by the `download_tools()` function — they don't need to be baked into the Docker image.

**One issue to handle:** The existing Qt6 Docker image's staging directory (with dependency source repos) is cleaned up during image build. The AppImage script's `run_workflow` calls `build_iio-emu` which needs the iio-emu source. The Qt6 AppImage script must clone iio-emu if the source isn't present.

**Summary of what the Qt6 Docker image already provides:**
- ✅ System build packages (gcc, cmake, autotools, etc.)
- ✅ Qt 6.8.3 at `/opt/Qt/6.8.3/gcc_64`
- ✅ All dependencies built and installed to `/usr/local` (libiio, gnuradio, spdlog, kddock, etc.)
- ✅ Python 3.12 runtime
- ❌ linuxdeploy tools → downloaded at runtime by `download_tools()` (~30s)
- ❌ iio-emu source → must be cloned in the Qt6 AppImage script (~10s)

---

## What changes between Qt5 and Qt6 in the AppImage script

### Variable changes (top of script)

| Variable | Qt5 value | Qt6 value | Source |
|----------|-----------|-----------|--------|
| `QT` | `/opt/Qt/5.15.2/gcc_64` | `/opt/Qt/6.8.3/gcc_64` | `ubuntu_build_process_qt6.sh:38` |
| `QMAKE_BIN` | `$QT/bin/qmake` | `$QT/bin/qmake6` | `ubuntu_build_process_qt6.sh:39` |
| `IIOEMU_BRANCH` | `master` | `main` | `ubuntu_build_process_qt6.sh:30` |
| `KDDOCK_BRANCH` | `2.1` | `2.2` | `ubuntu_build_process_qt6.sh:31` |
| `ECM_BRANCH` | `kf5` | `v6.8.0` | `ubuntu_build_process_qt6.sh:32` |
| `KARCHIVE_BRANCH` | `kf5` | `v6.8.0` | `ubuntu_build_process_qt6.sh:33` |

### aqtinstall command change

**Qt5 (`x86-64_appimage_process.sh:174`):** `sudo python3 -m aqt install-qt --outputdir $QT_INSTALL_LOCATION linux desktop 5.15.2`
**Qt6 (`ubuntu_build_process_qt6.sh:179`):** `sudo python3 -m aqt install-qt --outputdir $QT_INSTALL_LOCATION linux desktop 6.8.3 linux_gcc_64 -m qt3d qtscxml`

### build_kddock() — add Qt6 flag

**Qt5:** no special flag
**Qt6:** add `CURRENT_BUILD_CMAKE_OPTS="-DKDDockWidgets_QT6=ON"` (source: `ubuntu_build_process_qt6.sh:356`)

### build_karchive() — ECM cmake path export

**Qt6 requires:** `export CMAKE_PREFIX_PATH=$STAGING_AREA_DEPS/share/ECM/cmake:$CMAKE_PREFIX_PATH` (source: `ubuntu_build_process_qt6.sh:371`)

### build_scopy() — M2K disabled

**Qt5:** `-DCMAKE_INSTALL_PREFIX=$APP_DIR/usr`
**Qt6:** add `-DENABLE_PACKAGE_M2K=OFF` (source: `ubuntu_build_process_qt6.sh:401-404`)

### create_appdir() — Qt library names

**Qt5 (`x86-64_appimage_process.sh:536-539`):**
```bash
cp $QT/lib/libQt5XcbQpa.so* $APP_DIR/usr/lib
cp $QT/lib/libQt5WaylandClient.so* $APP_DIR/usr/lib
cp $QT/lib/libQt5EglFSDeviceIntegration.so* $APP_DIR/usr/lib
cp $QT/lib/libQt5DBus.so* $APP_DIR/usr/lib
```

**Qt6 equivalents:**
```bash
cp $QT/lib/libQt6XcbQpa.so* $APP_DIR/usr/lib
cp $QT/lib/libQt6WaylandClient.so* $APP_DIR/usr/lib
cp $QT/lib/libQt6EglFSDeviceIntegration.so* $APP_DIR/usr/lib
cp $QT/lib/libQt6DBus.so* $APP_DIR/usr/lib
```

**Note:** Verify these libraries exist in Qt 6.8.3. Qt6 may have restructured some platform plugins. Check `/opt/Qt/6.8.3/gcc_64/lib/` in the Docker image. If any are missing, the `cp` will fail and needs to be removed or replaced.

### run_workflow() — clone iio-emu

Since the existing Docker image doesn't preserve source repos, add a clone step before `build_iio-emu`:

```bash
run_workflow(){
    [ "$CI_SCRIPT" == "ON" ] && move_tools || download_tools
    # Clone iio-emu if source not present (Docker image cleans staging)
    if [ ! -d "$STAGING_AREA/iio-emu" ]; then
        mkdir -p $STAGING_AREA
        git clone --recursive https://github.com/analogdevicesinc/iio-emu -b $IIOEMU_BRANCH $STAGING_AREA/iio-emu
    fi
    build_iio-emu
    build_scopy
    create_appdir
    create_appimage
    move_appimage
}
```

---

## Implementation steps

### Step 1: Create `ci/x86_64/x86-64_appimage_process_qt6.sh`

Copy `ci/x86_64/x86-64_appimage_process.sh` and apply all changes listed above:

1. Update Qt path: line 84 → `6.8.3/gcc_64`
2. Update qmake: line 87 → `qmake6`
3. Update branch pins: lines 50-53 → Qt6 values
4. Update aqtinstall: line 174 → Qt6 command with modules
5. Update `build_kddock()`: add `-DKDDockWidgets_QT6=ON`
6. Update `build_karchive()`: add ECM cmake path export
7. Update `build_scopy()`: add `-DENABLE_PACKAGE_M2K=OFF`
8. Update `create_appdir()`: rename `libQt5*.so*` → `libQt6*.so*` (lines 536-539)
9. Update `run_workflow()`: add iio-emu clone step
10. Update `APP_IMAGE_FOLDER`: use `scopy-qt6-x86_64-appimage` to distinguish from Qt5

**Reuse unchanged from Qt5 script:** All other functions (clone, download_tools, build_with_cmake, install_packages, build_libserialport through build_genalyzer, copy-deps integration, create_appimage, move_appimage).

**Reuse unchanged from repo:**
- `ci/general/AppRun` — no Qt5/Qt6 references, fully compatible
- `ci/x86_64/copy-deps.sh` — generic ldd-based copier, no Qt references
- `ci/general/scopy.desktop` — generic desktop file

### Step 2: Create `.github/workflows/appimage-x86_64-qt6.yml`

Based on `.github/workflows/appimage-x86_64.yml` with these differences:
- Single build (Ubuntu 24 only, no matrix — Qt6 doesn't target Ubuntu 20)
- Container: `docker.cloudsmith.io/adi/scopy-dockers/scopy2-ubuntu24-qt6:${{ inputs.docker_tag }}`
- Credentials block for private Cloudsmith registry (same pattern as `ubuntubuild-qt6.yml`)
- Declares `CLOUDSMITH_API_KEY` secret in `workflow_call`
- Runs: `./ci/x86_64/x86-64_appimage_process_qt6.sh run_workflow`
- Artifact name uses qt6 prefix

### Step 3: Wire into `.github/workflows/ci-qt6.yml`

Add `appimage-x86_64-qt6` job after `ubuntu-qt6-build`:
```yaml
appimage-x86_64-qt6:
  needs: docker_tag
  if: always() && (needs.docker_tag.result == 'success' || needs.docker_tag.result == 'skipped')
  uses: ./.github/workflows/appimage-x86_64-qt6.yml
  with:
    docker_tag: ${{ inputs.docker_tag || needs.docker_tag.outputs.docker_tag }}
  secrets:
    CLOUDSMITH_API_KEY: ${{ secrets.CLOUDSMITH_API_KEY }}
```

### Step 4: Update `ci/x86_64/.dockerignore`

Add `!x86-64_appimage_process_qt6.sh` to the whitelist (for future Docker image builds if needed).

---

## Files summary

### Created

| File | Purpose |
|------|---------|
| `ci/x86_64/x86-64_appimage_process_qt6.sh` | Qt6 AppImage build script (fork of Qt5 script with ~10 targeted changes) |
| `.github/workflows/appimage-x86_64-qt6.yml` | Qt6 AppImage CI workflow |

### Modified

| File | Change |
|------|--------|
| `.github/workflows/ci-qt6.yml` | Add `appimage-x86_64-qt6` job |
| `ci/x86_64/.dockerignore` | Whitelist Qt6 script |

### Reused unchanged

| File | Role |
|------|------|
| `ci/general/AppRun` | Runtime wrapper (Qt-version-agnostic) |
| `ci/x86_64/copy-deps.sh` | Library dependency copier (Qt-version-agnostic) |
| `ci/general/scopy.desktop` | Desktop integration file |

---

## Verification

1. ✅ **Local test (in Docker):** Run `./ci/x86_64/x86-64_appimage_process_qt6.sh run_workflow` inside the Qt6 Docker container — AppImage produced successfully (2026-05-06)
2. ✅ **CI test:** CI on `qt6_clean` produced AppImage artifact successfully (2026-05-06)
3. ✅ **Functional test:** AppImage downloaded from CI, runs and launches Scopy successfully (2026-05-06)

---

## Risks

| Risk | Mitigation |
|------|-----------|
| Qt6 may not have `libQt6XcbQpa.so` or other platform libs | Check `/opt/Qt/6.8.3/gcc_64/lib/` in Docker. Remove or replace missing libs. |
| linuxdeploy-plugin-qt may not fully support Qt6 | linuxdeploy-plugin-qt has Qt6 support since 2023. If issues, use `--plugin qt` flag which auto-detects. |
| Existing Docker image missing a system package needed by AppImage | Build will fail with clear error. Add missing package to `ubuntu_build_process_qt6.sh install_packages` and rebuild Docker image. |
| `build_qwt` uses `qmake` which may behave differently with `qmake6` | The Qt6 build script already uses `qmake6` for Qwt. Verify same approach works in AppImage context. |
