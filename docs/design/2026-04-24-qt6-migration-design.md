# Scopy Qt6 Migration — Design Document

> **Date:** 2026-04-24 (updated 2026-04-27)
> **Branch:** `qt6_clean` (new, from `main`)
> **Reference:** `qt6_migration` (PoC branch, 14 commits ahead of main)
> **Scope:** All packages except M2K (deferred to planned M2K refactor)
> **Qt Version:** Qt 6.8.3 (decided 2026-04-27)

---

## 1. Strategy

**Clean Branch + Staged PRs:** Create a new branch `qt6_clean` from `main`. Rewrite the PoC changes as 7 clean, logically-grouped PRs merged sequentially into `qt6_clean`. Each PR is independently reviewable and targets a single concern. Merge `qt6_clean` to `main` only when fully validated.

```
main ─────────────────────────────────────────────────────────────► (untouched)
  \
   qt6_clean ──PR1──PR2──PR3──PR4──PR5──PR6──PR7──► (merge to main)
                │     │     │     │     │     │     │
              Docker  CI   CMake  sed  API  Sig  Runtime
```

**Key principles:**
- No unrelated commits — each PR has a clear, single-concern scope
- PoC commits serve as reference only — code is rewritten, not cherry-picked verbatim
- CI validates every PR after merge (PR 2 enables this)
- Git history is clean and bisectable

---

## 2. PR Breakdown

### PR 1: Docker Development Environment
**Scope:** CI infrastructure only — no source code changes.

| File | Action |
|------|--------|
| `ci/ubuntu/docker_ubuntu/Dockerfile.qt6` | MODIFY — update Qt version placeholder |
| `ci/ubuntu/ubuntu_build_process_qt6.sh` | MODIFY — parameterize version, update ECM/KArchive branches |
| `.dockerignore` | MODIFY — whitelist `ubuntu_build_process_qt6.sh` |

**Key changes:**
- Set `QT=6.8.3`, `ECM_BRANCH=v6.8.0`, `KARCHIVE_BRANCH=v6.8.0`
  - Note: initially set to `master`, but during Docker build karchive `master` failed with `Could not find Qt6Core >= 6.9.0` — master had moved ahead and bumped its minimum Qt requirement above 6.8.3. KDE Frameworks uses its own versioning (no `v6.8.3` exists); `v6.8.0` is the correct pinned tag — requires Qt ≥ 6.5.0, fully compatible with Qt 6.8.3.
- aqtinstall command: `python3 -m aqt install-qt --outputdir $QT_INSTALL_LOCATION linux desktop 6.8.3 linux_gcc_64 -m qt3d qtscxml`
  - Note: arch is `linux_gcc_64` (changed from `gcc_64` used in PoC/Qt 6.5)
  - Both `qt3d` and `qtscxml` verified available for 6.8.3
- Add missing runtime deps: `libxcb-cursor0 libxcb-icccm4 libxcb-keysyms1 libxcb-shape0`
- Add missing compression libs for KArchive: `libzstd-dev libbz2-dev liblzma-dev`
- Set `-DKDDockWidgets_QT6=ON` for KDDockWidgets
- Qt path in build script: `/opt/Qt/6.8.3/gcc_64`

  > **Note (debugged during Docker build):** We initially set the Qt path to `/opt/Qt/6.8.3/linux_gcc_64` based on the aqtinstall arch argument name. During the first Docker build attempt, the script failed at `build_qwt` with `qmake6: No such file or directory` (exit code 127). Investigation of aqtinstall's source (`QtRepoProperty.get_arch_dir_name`) revealed that on Linux, aqtinstall always installs Qt to the `gcc_64/` directory regardless of whether the arch argument is `linux_gcc_64` or `gcc_64`. The aqt command argument `linux_gcc_64` is correct (it selects the right Qt variant), but the on-disk path is always `gcc_64/`. Fix: keep `linux_gcc_64` in the aqt install command, change the QT path variable to `/opt/Qt/6.8.3/gcc_64`.

**Qt 6.8-specific notes:**
- Qt3D is deprecated in Qt 6.8 (maintained by KDAB going forward) but still ships and compiles. Used only in `packages/imu/plugins/imuanalyzer/`. No action needed now.
- `QSignalSpy` no longer inherits `QObject` in Qt 6.8. Scopy's `Q_DECLARE_METATYPE(QSignalSpy *)` in `iioutil/test/tst_iiocommandqueue.cpp` should still compile — verify during PR 7 testing.

**Validation:** `docker build` succeeds, `/opt/Qt/6.8.3/gcc_64/bin/qmake6 --version` returns `Qt version 6.8.3`.

---

### PR 2: CI Pipeline
**Scope:** GitHub Actions workflows for automated Qt6 builds. Placed immediately after Docker so every subsequent PR is automatically validated.

| File | Action |
|------|--------|
| `ci/ubuntu/create_docker_image.sh` | MODIFY — add `ubuntu24_qt6()` function |
| `.github/workflows/ci-qt6.yml` | NEW — orchestrator, triggers on `qt6_clean` branch |
| `.github/workflows/ubuntubuild-qt6.yml` | NEW — reusable Ubuntu 24 Qt6 build job |

**Workflow design:**
- Trigger: push or PR to `qt6_clean` branch
- Container: `scopy2-ubuntu24-qt6:<tag>` (registry prefix to be added when image is published to Docker Hub)
- Build step: `ubuntu_build_process_qt6.sh build_scopy`
- Test step: `QT_QPA_PLATFORM=offscreen ctest --output-on-failure`

**Reference:** Existing `ci.yml` and `ubuntubuild.yml` as templates.

**Validation:** Workflow triggers and runs (build will fail until PR 3 is merged — this is expected). After PR 3+, CI should start passing.

---

### PR 3: CMake Build System Migration
**Scope:** Build system files only — `CMakeLists.txt` across all modules. No source code changes.

**Testing:** All cmake configuration and builds run inside the local Docker image `scopy2-ubuntu24-qt6:testing`.

#### Step 3.1: Root `CMakeLists.txt`

| Line | Current | Change To | Why |
|------|---------|-----------|-----|
| 65 | `find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets)` | `find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)` | Target Qt6 only (no dual-version support needed on this branch) |
| 75-80 | `if(Qt5Widgets_VERSION VERSION_LESS 5.15.2)` version check + status message using `Qt5Widgets_VERSION` | Replace with: `message(STATUS "Using Qt version: ${QT_VERSION}")` | Qt5-specific variable; Qt6 uses `QT_VERSION` directly |
| 246 | `find_package(KF5Archive REQUIRED)` | `find_package(KF6Archive REQUIRED)` | KDE Frameworks 6 with Qt6 |
| 247 | `if(KF5Archive_FOUND)` | `if(KF6Archive_FOUND)` | Match package name |
| 248 | `message(STATUS "Using KF5Archive")` | `message(STATUS "Using KF6Archive")` | Match package name |
| 262 | `target_link_libraries(... PUBLIC KF5::Archive)` | `target_link_libraries(... PUBLIC KF6::Archive)` | KF6 namespace |
| 261 | `if(KF5Archive_FOUND)` | `if(KF6Archive_FOUND)` | Match package name |

#### Step 3.2: `pkg-manager/CMakeLists.txt`

| Line | Current | Change To | Why |
|------|---------|-----------|-----|
| 67 | `find_package(KF5Archive QUIET)` | `find_package(KF6Archive QUIET)` | KDE Frameworks 6 |
| 68 | `if(KF5Archive_FOUND)` | `if(KF6Archive_FOUND)` | Match package name |
| 69 | `target_link_libraries(... PUBLIC KF5::Archive)` | `target_link_libraries(... PUBLIC KF6::Archive)` | KF6 namespace |

#### Step 3.3: `gui/CMakeLists.txt`

| Line | Current | Change To | Why |
|------|---------|-----------|-----|
| 69 | `set(QWT_QT_VERSION qt5)` | `set(QWT_QT_VERSION qt6)` | QWT Qt6 library naming |
| 110 | `find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Xml Svg REQUIRED)` | `find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Xml Svg StateMachine REQUIRED)` | `QSignalTransition` in `gui/src/menu_anim.cpp` moved to StateMachine module in Qt6 |

Also add `Qt${QT_VERSION_MAJOR}::StateMachine` to `target_link_libraries` for scopy-gui.

#### Step 3.4: `core/CMakeLists.txt`

| Line | Current | Change To | Why |
|------|---------|-----------|-----|
| 43 | `set(SCOPY_QT_COMPONENTS Core Widgets Concurrent Network)` | `set(SCOPY_QT_COMPONENTS Core Widgets Concurrent Network OpenGLWidgets)` | `QOpenGLWidget` in `core/src/scopymainwindow.cpp` moved to separate module in Qt6 |

#### Step 3.5: `packages/imu/plugins/imuanalyzer/CMakeLists.txt`

| Lines | Current | Change To | Why |
|-------|---------|-----------|-----|
| 109-110 | `Qt::Widgets`, `Qt::Core` | Keep (unversioned aliases work in Qt6) | Already compatible |
| 115-121 | Hardcoded `Qt5::Core`, `Qt5::Gui`, `Qt5::Widgets`, `Qt5::3DCore`, `Qt5::3DExtras`, `Qt5::3DRender`, `Qt5::3DInput` | Replace with `Qt${QT_VERSION_MAJOR}::Core`, `Qt${QT_VERSION_MAJOR}::Gui`, `Qt${QT_VERSION_MAJOR}::Widgets`, `Qt${QT_VERSION_MAJOR}::3DCore`, `Qt${QT_VERSION_MAJOR}::3DExtras`, `Qt${QT_VERSION_MAJOR}::3DRender`, `Qt${QT_VERSION_MAJOR}::3DInput` | Hardcoded Qt5 namespaces break Qt6 |

#### Step 3.6: `packages/generic-plugins/plugins/regmap/CMakeLists.txt`

| Line | Current | Change To | Why |
|------|---------|-----------|-----|
| 130 | `Qt5::Test` | `Qt${QT_VERSION_MAJOR}::Test` | Hardcoded Qt5 namespace |

#### Step 3.7: `packages/m2k/plugins/m2k/m2k-gui/CMakeLists.txt`

| Line | Current | Change To | Why |
|------|---------|-----------|-----|
| 52 | `set(QWT_QT_VERSION qt5)` | `set(QWT_QT_VERSION qt6)` | QWT Qt6 library naming |

> **Note:** M2K plugin is out of scope for source code changes but QWT version must still be updated for the build to find the correct QWT library.

#### Step 3.8: `common/include/common/common.h`

| Lines | Current | Change To | Why |
|-------|---------|-----------|-----|
| 28-33 | Qt 5.14 compat shim (`namespace Qt { endl, SkipEmptyParts }`) | Remove entirely | Qt6 is always ≥ 6.0, so the `QT_VERSION < 5.14` guard is never true |

#### Step 3.9: `cmake/Modules/FindQwt.cmake`

No changes needed — already includes both qt5 and qt6 search paths and library names.

#### Verification

Run inside the Docker container:
```bash
docker run -it --rm \
  -v /path/to/scopy:/home/runner/scopy \
  scopy2-ubuntu24-qt6:testing bash

# Inside container
git config --global --add safe.directory /home/runner/scopy
cd /home/runner/scopy
cmake -B build-qt6 -DCMAKE_PREFIX_PATH=/opt/Qt/6.8.3/gcc_64 -DENABLE_ALL_PACKAGES=ON
```

**Expected result:** cmake configuration completes successfully (compilation will fail until PR 4-6 are merged — this is expected at this stage).

**Verification checks:**
- `grep "Using Qt version: 6.8.3" build-qt6/CMakeCache.txt` or cmake output confirms Qt6 detected
- `grep "KF6Archive" build-qt6/CMakeCache.txt` confirms KF6 linkage
- No `Qt5::` references remain: `grep -rn "Qt5::" --include="CMakeLists.txt" . | grep -v m2k` returns zero hits (excluding m2k source changes which are deferred)

#### Debug Notes — Issues Found During First Build Test (2026-04-27)

**Issue 1: `cmake/Modules/ScopyTest.cmake` — hardcoded `Qt5` references**
We ran `build_scopy` inside Docker and cmake failed with `Could not find a package configuration file provided by "Qt5"` at `ScopyTest.cmake:30`. Root cause: `ScopyTest.cmake` had `find_package(Qt5 COMPONENTS Test REQUIRED)` and `target_link_libraries(... Qt5::Test ...)` hardcoded instead of using `Qt${QT_VERSION_MAJOR}`. This file was missed in the initial PR 3 plan because the search only covered `CMakeLists.txt` files, not cmake modules in `cmake/Modules/`. Fix: replaced both occurrences with `Qt${QT_VERSION_MAJOR}` variants.

**Issue 2: Git safe directory — `ScopyAbout.cmake` fails on `string REPLACE`**
Same build test showed `CMake Error at cmake/Modules/ScopyAbout.cmake:63 (string): string sub-command REPLACE requires at least four arguments`. Root cause: when running inside Docker with a bind-mounted volume, git refuses to operate on the repo due to "dubious ownership" (container user `root` ≠ host user). The `ScopyAbout.cmake` module runs `git` commands to extract commit info, gets empty strings, and the `string(REPLACE ...)` call fails with too few arguments. Fix: added `git config --global --add safe.directory $SRC_DIR` at the start of the `build_scopy()` function in `ubuntu_build_process_qt6.sh`.

**Note:** `cmake/Modules/ScopyMacOS.cmake:41` also has a hardcoded `Qt5Gui_PLUGINS` reference, but this is macOS-only and not blocking the Linux build. Will be addressed when macOS support is ported.

---

### PR 4: Mass Automated Replacements
**Scope:** Source code only — safe, mechanical text replacements via sed. M2K package excluded (deferred to its own refactor).

**All sed commands exclude:** `build/`, `.git/`, `staging_ubuntu20/`, `staging_ubuntu/`, `packages/m2k/`
**File extensions:** `*.cpp`, `*.h`, `*.hpp`, `*.cc`, `*.mako`

#### Step 4.1: `setMargin(N)` → `setContentsMargins(N, N, N, N)`

| Detail | Value |
|--------|-------|
| Why | `QLayout::setMargin()` removed in Qt6, only `setContentsMargins()` exists |
| Occurrences | ~340 (non-m2k) |
| sed | `s/setMargin\(([^)]+)\)/setContentsMargins(\1, \1, \1, \1)/g` |

~57 files already use both `setMargin` and `setContentsMargins` on different objects. After sed, some files may have redundant consecutive calls on the same object — harmless (second overrides first).

#### Step 4.2: `qAsConst(` → `std::as_const(`

| Detail | Value |
|--------|-------|
| Why | `qAsConst` deprecated in Qt6, C++17 `std::as_const` is the replacement |
| Occurrences | ~210 (non-m2k, across .cpp/.h/.hpp/.cc/.mako) |
| sed | `s/qAsConst(/std::as_const(/g` |

#### Step 4.3: `<< endl` → `<< Qt::endl`

| Detail | Value |
|--------|-------|
| Why | Bare `endl` is ambiguous in Qt6 |
| Occurrences | 1 (non-m2k): `registermapvalues.cpp:98` |
| sed | `s/<< endl/<< Qt::endl/g` (single file) |

#### Step 4.4: `.midRef(` → `.mid(`

| Detail | Value |
|--------|-------|
| Why | `QString::midRef()` and `QStringRef` removed in Qt6 |
| Occurrences | 1 (non-m2k): `dacdatamodel.cpp:624` |
| sed | `s/\.midRef(/\.mid(/g` (single file) |

No `leftRef` or `rightRef` occurrences found.

#### Step 4.5: `enterEvent(QEvent *)` → `enterEvent(QEnterEvent *)`

| Detail | Value |
|--------|-------|
| Why | Qt6 changed parameter type; old signature silently stops being called |
| Occurrences | 8 (non-m2k): 4 headers + 4 cpp files |
| sed | `s/enterEvent\(QEvent \*([a-z]*)\)/enterEvent(QEnterEvent *\1)/g` |

Also add `#include <QEnterEvent>` to 4 header files:
- `gui/include/gui/buffer_previewer.hpp`
- `gui/include/gui/basemenuitem.h`
- `gui/include/gui/smallOnOffSwitch.h`
- `core/include/core/toolmenuitem.h`

#### Step 4.6: `setWeight(75)` → `setWeight(QFont::Bold)` — SKIPPED

All occurrences are in m2k files (1 in .cpp, 4 in .cc). No-op for this PR.

#### Step 4.7: Run formatter

`./tools/format.sh` — fixes line lengths broken by `setContentsMargins(N, N, N, N)` expansion.

#### Step 4.8: Docker build test

Build inside Docker to verify. Compilation will still fail (PR 5 API fixes needed), but fewer errors than before PR 4.

#### Verification

These greps should all return 0 hits (excluding m2k and build dirs):
```
grep -rn 'setMargin(' ... (excluding setContentsMargins matches)
grep -rn 'qAsConst(' ...
grep -rn 'enterEvent(QEvent' ...
grep -rn '.midRef(' ...
```

---

### PR 5: API Migration (Manual Fixes)
**Scope:** Manual C++ fixes for Qt6 API changes. M2K excluded. Each step is small and isolated.

**Note (2026-04-27):** After codebase exploration, several categories from the original plan were found to have zero occurrences outside M2K: `qRegisterMetaTypeStreamOperators`, Qt3D include moves, `powercontrol.ui` signal fix, `QString != NULL`, `QString(int)` disambiguation, implicit `QString→QFileInfo`. These are removed from the plan. `QMouseEvent::pos()` is deprecated but still compiles — deferred to cleanup.

#### Step 5.1: QSslError explicit list — `pluginbase/src/versionchecker.cpp:119`

| Detail | Value |
|--------|-------|
| Why | `QList<T>` constructor from initializer list became `explicit` in Qt6 |
| Fix | `{QSslError::NoPeerCertificate}` → `QList<QSslError>{QSslError::NoPeerCertificate}` |

This was the first error encountered — blocked the build at 5%.

#### Step 5.2: QRegExp → QRegularExpression (4 files)

| File | Change |
|------|--------|
| `gr-util/src/griiodevicesource.cpp:122-123` | `QString::remove(QRegExp(...))` → `remove(QRegularExpression(...))` |
| `gui/src/spinbox_a.cpp` (6 locations) | Full migration: `QRegExpValidator` → `QRegularExpressionValidator`, `indexIn()`+`cap()` → `match()`+`captured()` |
| `core/src/iiotabwidget.cpp:195-199,406-407` | `QRegExp` + `QRegExpValidator` for IP/serial validation |
| `packages/swiot/.../bufferlogic.cpp:56` | `QRegExp` for number extraction |

#### Step 5.3: filterRegExp → filterRegularExpression (2 files)

| File | Change |
|------|--------|
| `.../iiosortfilterproxymodel.cpp:35` | `filterRegExp().pattern()` → `filterRegularExpression().pattern()` |
| `.../iioexplorerinstrument.cpp:174,445` | `setFilterRegExp(QRegExp(...))` → `setFilterRegularExpression(QRegularExpression(...))` |

#### Step 5.4: QStyleOption::init() → initFrom()

| File | Change |
|------|--------|
| `gui/include/gui/utils.h:81` | `opt.init(this)` → `opt.initFrom(this)` in `QWIDGET_PAINT_EVENT_HELPER` macro |

#### Step 5.5: QVariant::type() → typeId()

| File | Change |
|------|--------|
| `core/src/pkggui/pkgpreviewpage.cpp:59` | `variant.type() == QVariant::List` → `variant.typeId() == QMetaType::QVariantList` |
| `core/src/pkggui/pkggridwidget.cpp:83` | `var.type() == QVariant::String` → `var.typeId() == QMetaType::QString` |

#### Step 5.6: Remove `Qt::AA_UseHighDpiPixmaps`

| File | Change |
|------|--------|
| `main.cpp:119` | Delete `QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps)` — always-on in Qt6 |

#### Step 5.7: QtConcurrent::run() argument order (9 files)

Qt6 removed `run(object, &method, args...)`. New syntax: `run(&method, object, args...)`.

| File | Occurrences |
|------|-------------|
| `fmcomms5calibration.cpp:73` | 1 |
| `profilegeneratorwidget.cpp:715,718,747` | 3 |
| `iiomanager.cpp:78` | 1 |
| `dacdatamodel.cpp:350` | 1 |
| `pqmdatalogger.cpp:142` | 1 |
| `acquisitionmanager.cpp:136,284` | 2 |

#### Step 5.8: Missing `OpenGLWidgets` Qt module in CMakeLists (3 files)

**Found during build test (2026-04-27):** Build failed at 13% on `plotwidget.cpp` — QWT's `qwt_plot_opengl_canvas.h` includes `<qopenglwidget.h>` which in Qt6 moved from `QtWidgets` to the separate `QtOpenGLWidgets` module. Without the module in `find_package` and `target_link_libraries`, the include path is missing.

| File | Change |
|------|--------|
| `gui/CMakeLists.txt:110,155` | Added `OpenGLWidgets` to `find_package` and `target_link_libraries` — gui uses `QwtPlotOpenGLCanvas` which inherits `QOpenGLWidget` |
| `packages/imu/plugins/imuanalyzer/CMakeLists.txt:69,122` | Added `OpenGLWidgets` to `find_package` and `target_link_libraries` — `scenerenderer.hpp` includes `QOpenGLWidget` and `QOpenGLFunctions` |

**Why:** In Qt5, `QOpenGLWidget` lived in `QtWidgets`. In Qt6, it was extracted into `QtOpenGLWidgets` — a separate module that must be explicitly found and linked. `core/CMakeLists.txt` already had it (added in PR 3), but gui and imuanalyzer did not.

#### Step 5.9: `leaveEvent` calling `enterEvent` type mismatch — `smallOnOffSwitch.cpp:177`

**Found during build test (2026-04-27):** Build failed at 15% — `leaveEvent` was calling `QCheckBox::enterEvent(event)` passing a `QEvent*`, but Qt6's `enterEvent` takes `QEnterEvent*`. This was a pre-existing bug: `leaveEvent` should call the parent `leaveEvent`, not `enterEvent`. PR 4's sed correctly changed `enterEvent` signatures but this call site was inside `leaveEvent`, so it was not a signature — it was a call to the wrong function entirely.

**Fix:** `QCheckBox::enterEvent(event)` → `QCheckBox::leaveEvent(event)` (bug fix, not just type change)

#### Step 5.10: Incomplete `QFile` type — `tutorialbuilder.h:83`

**Found during build test (2026-04-27):** `QFile m_jsonFile` member has incomplete type. In Qt5, `QFile` was transitively included through header chains. In Qt6, `qevent.h` only forward-declares `QFile`, so headers using `QFile` as a value type need an explicit `#include <QFile>`.

**Fix:** Added `#include <QFile>` to `gui/include/gui/tutorialbuilder.h`

#### Verification

Build in Docker — compilation should progress past the 15% failure point. Any new errors found during the build will be documented and fixed iteratively.

---

### PR 6: Signal/Slot Modernization & Threading
**Scope:** Signal/slot syntax migration and concurrency fixes.

**Signal/Slot changes (~84 connections from PoC):**
- Old-style `SIGNAL()/SLOT()` → new-style `&Class::method` in core, gui, adc, dac, pluginbase, iio-widgets
- **Keep old-style intentionally** in: `gui/src/symbol_controller.cpp`, `core/src/deviceimpl.cpp` (runtime polymorphic dispatch)
- **Skip entirely:** M2K plugin (deferred to M2K refactor)
- Remaining ~19 files with old-style SIGNAL() (excluding m2k/staging) must be reviewed and modernized where appropriate

**Removed signal overloads:**
- `QComboBox::currentIndexChanged(QString)` → use `int` parameter + `itemText(idx)`
- `QSpinBox::valueChanged(QString)` → use `int` parameter

**Threading fixes:**
- `QtConcurrent::run()` — add `(void)` cast for fire-and-forget (`[[nodiscard]]`)
- `QFutureWatcher::setPaused/isPaused` → `setSuspended/isSuspending`
- `QSignalBlocker` temporaries → named variables (3 silent bugs)
- `QMouseEvent::localPos()` → `position()`, `QEnterEvent::pos()` → `position().toPoint()`

**Qt::UniqueConnection + lambda fix (regmap, 2 sites):**
- Extract lambda to named slot (Qt6 asserts on `Qt::UniqueConnection` with lambdas)

**Reference:** PoC commits `e9c59fae6`, `2b762284b`, `6bf693d02`.

**Validation:** Full compilation + linkage succeeds.

---

### PR 7: Runtime Fixes & Final Verification
**Scope:** JS engine fixes, runtime validation, deprecation cleanup.

| Fix | Detail |
|-----|--------|
| JS syntax: `for (each in o)` → `for (var each in o)` | `scopyjs.cpp` — Qt6 V4 engine rejects non-standard syntax |
| Tutorial builder nullptr check | `core/src/tutorialbuilder.cpp` |
| UI tool template fix | Core UI fixes from PoC |
| Sed artifact cleanup | Remove duplicate `setContentsMargins` if any remain |

**Verification checklist:**
- [ ] `QT_QPA_PLATFORM=offscreen ctest --output-on-failure` — all 27 tests pass
- [ ] Launch Scopy with X11 — all plugins load
- [ ] JS scripting tool — `print("hello")` works
- [ ] Navigate all tool menus — no crashes or rendering issues
- [ ] Clazy with 5 Qt6 porting checks — zero findings
- [ ] Review known risks (Section 5)

---

## 3. Existing Tools & Patterns Report

### Reusable (use directly)
| Tool | Location | Purpose |
|------|----------|---------|
| `ubuntu_build_process_qt6.sh` | `ci/ubuntu/` | Full dependency build — parameterize version |
| `Dockerfile.qt6` | `ci/ubuntu/docker_ubuntu/` | Docker base image |
| `run_clazy.sh` | `porting_process/` | Post-migration verification |
| `format.sh` | `tools/` | Enforce formatting after sed passes |
| Existing CI workflows | `.github/workflows/` | Templates for Qt6 workflow |

### Adaptable (modify slightly)
| Item | Change Needed |
|------|---------------|
| `ubuntu_build_process_qt6.sh` | Parameterize Qt version, update ECM/KArchive branches |
| `Dockerfile.qt6` | Update base packages for chosen Qt version |
| `ci.yml` / `ubuntubuild.yml` | Model Qt6 variants after these |

### Missing (build from scratch)
| Item | Complexity |
|------|-----------|
| `.github/workflows/ci-qt6.yml` | Simple — model after `ci.yml` |
| `.github/workflows/ubuntubuild-qt6.yml` | Simple — model after `ubuntubuild.yml` |

---

## 4. Testing Strategy

### Unit Tests
- Existing Qt unit tests (`ctest`) must pass under Qt6 — no new tests needed for migration
- Run with `QT_QPA_PLATFORM=offscreen` in CI (headless)
- 27 tests expected to pass (from PoC verification)

### Clazy Verification
Run after all source PRs (PR 4-7) are merged:
```bash
apt-get install -y clazy
bash porting_process/run_clazy.sh
```
5 checks: `qt6-deprecated-api-fixes`, `qt6-header-fixes`, `qt6-qhash-signature`, `qt6-fwd-fixes`, `missing-qobject-macro`

Expected: zero findings (confirmed in PoC across 529 files).

### Runtime Validation
- Launch Scopy with X11 display → all plugins load
- JS scripting tool → `print("hello")` and basic evaluation work
- Navigate all tool menus → no crashes or rendering issues

### Compile-Time Guard (optional, post-merge)
Add to root `CMakeLists.txt` to catch future deprecated API usage:
```cmake
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)
```

---

## 5. Known Risks

### `disconnect()` with nullptr slot — 3 files
Lambda connections cannot be disconnected by `disconnect(sender, &Signal, receiver, nullptr)` in Qt6. Requires storing `QMetaObject::Connection` handles.
- `gui/src/widgets/plotlegend.cpp:115-117`
- `gui/src/widgets/menuplotchannelcurvestylecontrol.cpp:123`
- `gui/src/cursorcontroller.cpp:193-194, 241-244`

**Action:** Verify at runtime. Refactor to stored connection handles if signals fire unexpectedly.

### `int` vs `qsizetype` — ~175 instances
`QList::size()` returns `qsizetype` in Qt6 (was `int`). Generates `-Wconversion` warnings if enabled. Accepted as-is — functionally correct on 64-bit.

### `Style::setBackgroundColor()` fragility
The `.QWidget` CSS class selector in `gui/src/style.cpp:302-309` only matches widgets without `Q_OBJECT`. Any future `Q_OBJECT` addition to a QWidget subclass using this method will break styling. Should be fixed to use `widget->metaObject()->className()` dynamically.

### M2K Plugin — Out of Scope
~375 old-style SIGNAL/SLOT connections across 39 files. Old-style macros fully work in Qt6 — no functional regression. M2K is scheduled for a near-term refactor.

---

## 6. Implementation Order

| Order | PR | Depends On | Purpose |
|-------|----|-----------:|---------|
| 1 | Docker Environment | Qt version decision | Build environment |
| 2 | CI Pipeline | PR 1 (Docker image) | Automated validation for all subsequent PRs |
| 3 | CMake Migration | PR 1 (build env) | Build system compiles with Qt6 |
| 4 | Mass Replacements | PR 3 (cmake) | Mechanical text fixes |
| 5 | API Migration | PR 4 | Manual C++ fixes |
| 6 | Signal/Slot + Threading | PR 5 | Modernize connections, fix concurrency |
| 7 | Runtime Fixes + Verify | PR 6 | Final fixes and full validation |

**Qt version decided:** Qt 6.8.3. PR 1 is unblocked.

**Estimated effort:** ~5-7 days (risk-buffered: 6-9 days). See time estimates section for per-PR breakdown.

---

## 7. Qt 6.8.3 — Key Build Parameters

All concrete values for use in the build script and Docker setup:

| Parameter | Value |
|-----------|-------|
| Qt version | `6.8.3` |
| aqtinstall arch | `linux_gcc_64` |
| aqtinstall modules | `qt3d qtscxml` |
| Qt install path | `/opt/Qt/6.8.3/gcc_64` (aqtinstall always uses `gcc_64/` on Linux) |
| `ECM_BRANCH` | `v6.8.0` (pinned from `master` — KDE Frameworks `master` bumped min Qt to 6.9.0, breaking our 6.8.3 build; `v6.8.0` requires Qt ≥ 6.5.0 and is the last release in the 6.8.x series) |
| `KARCHIVE_BRANCH` | `v6.8.0` (same reason as ECM; KDE Frameworks has no `v6.8.3` — their versioning is independent of Qt's) |
| `KDDOCK_BRANCH` | `2.2` (unchanged, supports Qt 6.2+) |
| `cmake --prefix-path` | `/opt/Qt/6.8.3/gcc_64` |

**Qt 6.8-specific considerations:**
- Qt3D deprecated (KDAB maintains) — still ships, still compiles, no action needed
- `QSignalSpy` no longer inherits `QObject` — verify `tst_iiocommandqueue.cpp` and `tst_grblocks.cpp` compile in PR 7
- `Qt::AA_ShareOpenGLContexts` — still supported, no change needed in `main.cpp`

---

## 8. References

| Resource | Location |
|----------|----------|
| PoC branch | `qt6_migration` (14 commits, 495 files changed) |
| Qt 5→6 Porting Guide | https://doc.qt.io/qt-6/portingguide.html |
| Qt Core Changes | https://doc.qt.io/qt-6/qtcore-changes-qt6.html |
| Clazy Porting Tool | https://doc.qt.io/qt-6/porting-to-qt6-using-clazy.html |
| PoC Briefing | `porting_process/porting_process_breafing.md` |
| PoC Detailed Instructions | `porting_process/porting_process_instructions.md` |
| PoC Qt Logic Reference | `porting_process/porting_logic.md` |
| Clazy Run Script | `porting_process/run_clazy.sh` |
| Useful sed Commands | `docs/design/2026-04-24-qt6-porting-plan.md` Section 7.3 |
