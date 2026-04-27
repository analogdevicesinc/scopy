# Scopy Qt5 to Qt6 Porting Plan

> **Date:** 2026-04-24 (updated 2026-04-27)
> **Branch:** `qt6_clean` (new clean branch from `main`; PoC reference: `qt6_migration`)
> **Status:** Planning (based on completed proof-of-concept)
> **Scope:** All packages except M2K (deferred to planned M2K refactor)
> **Qt Version:** Qt 6.8.3 (decided 2026-04-27)

---

## 1. Qt Version Decision

### 1.1 Chosen Version: Qt 6.8.3

| Property | Value |
|----------|-------|
| Version | 6.8.3 |
| Type | LTS (commercial support until Oct 2029) |
| aqtinstall arch | `linux_gcc_64` |
| aqtinstall modules needed | `qt3d qtscxml` (both verified available) |
| KArchive/ECM branch | `master` (works natively with Qt >= 6.8) |
| KDDockWidgets 2.2 | Compatible (supports Qt 6.2+) |
| PoC version | 6.7.0 (minor adjustments needed) |

### 1.2 Changes from PoC (6.7.0 → 6.8.3)

**Build script changes:**
- `ubuntu_build_process_qt6.sh` line 37: `QT=$QT_INSTALL_LOCATION/6.8.3/gcc_64`
- `ubuntu_build_process_qt6.sh` line 178: `aqt install-qt ... 6.8.3 -m qt3d qtscxml`
- aqtinstall arch: `linux_gcc_64` (changed from `gcc_64` used in 6.5/6.7)
- `ECM_BRANCH=v6.8.0` (was `master` — `master` bumped min Qt to 6.9.0, breaking the 6.8.3 build; KDE Frameworks has no `v6.8.3`, `v6.8.0` is the correct pinned tag requiring Qt ≥ 6.5.0)
- `KARCHIVE_BRANCH=v6.8.0` (same reason as ECM)

**Qt 6.8-specific changes that affect Scopy:**

| Change | Impact | Action |
|--------|--------|--------|
| Qt3D deprecated (maintained by KDAB) | `packages/imu/plugins/imuanalyzer/` uses Qt3D (1 plugin, 1 source file). Module still ships and compiles. | No action needed now. Monitor for future removal. |
| `QSignalSpy` no longer inherits `QObject` | `iioutil/test/tst_iiocommandqueue.cpp` has `Q_DECLARE_METATYPE(QSignalSpy *)`. `gr-util/test/tst_grblocks.cpp` uses QSignalSpy normally. | Verify tests still compile. `Q_DECLARE_METATYPE` doesn't require QObject inheritance — likely fine. |
| `QSignalSpy::signal()` behavior change | Tests use `count()` and `isValid()`, not `signal()`. | No impact. |

**No impact (verified):**
- `Qt::AA_ShareOpenGLContexts` — still supported in 6.8
- All required Qt modules — available via aqtinstall
- QDateTime timezone gap behavior — Scopy doesn't rely on gap resolution
- FFmpeg backend changes — Scopy doesn't use Qt Multimedia

---

## 2. Docker Development Environment

### 2.1 Goal
Provide a self-contained Qt6 build environment that does not touch the host Qt5 installation.

### 2.2 What Already Exists (from PoC)
- `ci/ubuntu/docker_ubuntu/Dockerfile.qt6` — Ubuntu 24.04 base
- `ci/ubuntu/ubuntu_build_process_qt6.sh` — Full dependency build script for Qt 6.7.0

### 2.3 Steps to Finalize

1. **Update Qt version** in `ubuntu_build_process_qt6.sh`:
   - Line 37: `QT=$QT_INSTALL_LOCATION/6.8.3/gcc_64`
   - Line 178: `sudo python3 -m aqt install-qt --outputdir $QT_INSTALL_LOCATION linux desktop 6.8.3 linux_gcc_64 -m qt3d qtscxml`
   - Both `qt3d` and `qtscxml` verified available for 6.8.3

2. **Update KArchive/ECM branches**:
   - `ECM_BRANCH=v6.8.0` (pinned — `master` requires Qt ≥ 6.9.0; `v6.8.0` is Qt 6.8.3 compatible)
   - `KARCHIVE_BRANCH=v6.8.0` (same reason)

3. **Build the Docker image:**
   ```bash
   cd ci/ubuntu
   docker build -f docker_ubuntu/Dockerfile.qt6 -t scopy-qt6-dev .
   ```

4. **Verify:**
   ```bash
   docker run --rm scopy-qt6-dev /opt/Qt/6.8.3/gcc_64/bin/qmake6 --version
   ```

### 2.4 Running the Dev Environment

```bash
# On host — allow Docker to access X11
xhost +local:docker

# Start container with source mounted
docker run -it --rm \
  -v /path/to/scopy:/home/runner/scopy \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e DISPLAY=$DISPLAY \
  --net=host scopy-qt6-dev bash

# Inside container — required one-time setup
git config --global --add safe.directory /home/runner/scopy

# Build
cd /home/runner/scopy
cmake -B build-qt6 -DCMAKE_PREFIX_PATH=/opt/Qt/6.8.3/gcc_64 -DENABLE_ALL_PACKAGES=ON
cmake --build build-qt6 -j$(nproc)

# Run (requires X11 display)
./build-qt6/scopy
```

### 2.5 Known Issues & Solutions

| Issue | Solution |
|-------|----------|
| `.dockerignore` blocks Qt6 script | Add `ubuntu_build_process_qt6.sh` to whitelist |
| `git safe.directory` error in container | Run `git config --global --add safe.directory /home/runner/scopy` |
| KArchive `master` needs Qt >= 6.8 | Pin to version-matched tags if using Qt < 6.8 |
| KDDockWidgets defaults to Qt5 | Set `-DKDDockWidgets_QT6=ON` |
| Missing compression libs for KArchive | Install `libzstd-dev libbz2-dev liblzma-dev` |
| Qt6 runtime needs xcb packages | Install `libxcb-cursor0 libxcb-icccm4 libxcb-keysyms1 libxcb-shape0` |

---

## 3. CI Pipeline (Ubuntu Only — First Phase)

### 3.1 Goal
Automated Qt6 build on every push to `qt6_migration` branch. Uses the Docker image from Step 2.

### 3.2 Files to Create/Modify

| File | Action |
|------|--------|
| `ci/ubuntu/create_docker_image.sh` | Add `ubuntu24_qt6()` function using `Dockerfile.qt6` |
| `.github/workflows/ci-qt6.yml` | Qt6 orchestrator workflow, triggers on `qt6_migration` branch |
| `.github/workflows/ubuntubuild-qt6.yml` | Ubuntu 24 Qt6 build job (reusable workflow) |

### 3.3 Workflow Design

**Trigger:** Push or PR to `qt6_migration` branch

**Docker image:** `scopy2-ubuntu24-qt6:<tag>` (registry prefix to be added when image is published to Docker Hub)

**Key difference from Qt5 workflow:**
```yaml
container: scopy2-ubuntu24-qt6:${{ inputs.docker_tag }}
steps:
  - run: ubuntu_build_process_qt6.sh build_scopy
```

### 3.4 Steps

1. Push the finalized Docker image to the container registry
2. Create `ci-qt6.yml` orchestrator (model after existing `ci.yml`)
3. Create `ubuntubuild-qt6.yml` (model after existing `ubuntubuild.yml`, reference `ubuntu_build_process_qt6.sh`)
4. Test on a push to the `qt6_migration` branch
5. Add unit test execution: `QT_QPA_PLATFORM=offscreen ctest --output-on-failure`

### 3.5 Future Platforms (Out of Scope for Now)

| Platform | Blocker |
|----------|---------|
| AppImage x86_64 | Needs Qt6 Docker image with AppImage tooling |
| AppImage ARM | Needs Qt6 cross-compilation sysroot |
| Windows/MinGW | Needs Qt6 MSYS2 toolchain |
| Flatpak | Needs `org.kde.Sdk//6.x` runtime |

---

## 4. Qt Official Documentation Reference

### 4.1 Essential Reading

| Document | URL | Purpose |
|----------|-----|---------|
| Main Porting Guide | https://doc.qt.io/qt-6/portingguide.html | Overview of all breaking changes |
| Qt Core Changes | https://doc.qt.io/qt-6/qtcore-changes-qt6.html | Container, string, type system changes |
| Clazy Porting Tool | https://doc.qt.io/qt-6/porting-to-qt6-using-clazy.html | Automated detection of Qt5 incompatibilities |
| Build System Migration | https://doc.qt.io/qt-6/qt6-buildsystem.html | CMake target changes |
| What's New in Qt 6.0 | https://doc.qt.io/qt-6/whatsnew60.html | Removed/restructured modules |

### 4.2 Golden Rule from Qt Documentation

> **Update to Qt 5.15 first, then migrate to Qt 6.** Qt 5.15 marks all APIs that will be removed in Qt 6 as deprecated.

Scopy is already on Qt 5.15 — this prerequisite is met.

### 4.3 Recommended Compile-Time Check

Add to root `CMakeLists.txt` after the port is complete to catch future deprecated API usage:
```cmake
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)
```

---

## 5. Step-by-Step Porting Process

### Phase 1: Environment Setup

- [ ] **1.1** Decide on Qt6 LTS version (see Debate Points)
- [ ] **1.2** Update `ubuntu_build_process_qt6.sh` with final version number
- [ ] **1.3** Update KArchive/ECM branch tags to match chosen Qt version
- [ ] **1.4** Build and verify Docker image
- [ ] **1.5** Verify all dependencies build successfully inside container

### Phase 2: CMake Build System Migration

- [ ] **2.1** `find_package(Qt5 ...)` -> `find_package(Qt6 ...)` in root `CMakeLists.txt`
- [ ] **2.2** `KF5Archive` -> `KF6Archive` in root + `pkg-manager/CMakeLists.txt`
- [ ] **2.3** `Qt5::*` -> `Qt${QT_VERSION_MAJOR}::*` in package CMakeLists files
- [ ] **2.4** Add new required modules: `OpenGLWidgets`, `StateMachine`
- [ ] **2.5** `QWT_QT_VERSION qt5` -> `qt6` in gui + m2k-gui CMakeLists
- [ ] **2.6** Remove Qt 5.14 compat shim from `common/include/common/common.h`
- [ ] **2.7** Verify `cmake -B build-qt6` completes successfully

**Already done in PoC:** All items above. Review and apply from PoC commits.

### Phase 3: Mass Automated Replacements

These are safe to apply via sed across the entire codebase (excluding `build/` and `.git/`):

- [ ] **3.1** `setMargin(N)` -> `setContentsMargins(N, N, N, N)` (~397 occurrences)
- [ ] **3.2** `qAsConst(` -> `std::as_const(` (~239 occurrences)
- [ ] **3.3** `<< endl` -> `<< Qt::endl`
- [ ] **3.4** `.midRef(` / `.leftRef(` / `.rightRef(` -> `.mid(` / `.left(` / `.right(`
- [ ] **3.5** `setWeight(75)` -> `setWeight(QFont::Bold)`
- [ ] **3.6** `enterEvent(QEvent *` -> `enterEvent(QEnterEvent *`
- [ ] **3.7** Review for sed artifacts (e.g., duplicate `setContentsMargins` in `channel_manager.cpp`)

### Phase 4: API Migration (Manual Fixes)

Each fix is small and isolated. Apply in order of discovery during compilation:

- [ ] **4.1** `QRegExp` -> `QRegularExpression` (8 files)
- [ ] **4.2** `QStyleOption::init()` -> `initFrom()`
- [ ] **4.3** `QString(int)` -> `QString::number(int)` (ambiguous constructor)
- [ ] **4.4** `QVariant::type()` -> `typeId()`, `QVariant::Type` -> `QMetaType::Type`
- [ ] **4.5** `QtConcurrent::run()` argument order swap (17 occurrences)
- [ ] **4.6** Implicit `QString->QFileInfo` conversion — add explicit `QFileInfo()` constructor
- [ ] **4.7** `QList` explicit constructor — `{...}` -> `QList<T>{...}`
- [ ] **4.8** `qRegisterMetaTypeStreamOperators` calls — remove (auto-registered in Qt6)
- [ ] **4.9** `filterRegExp()` -> `filterRegularExpression()`
- [ ] **4.10** Qt3D includes: `Qt3DRender/QGeometry` -> `Qt3DCore/QGeometry`
- [ ] **4.11** `QSignalTransition` — add `StateMachine` module to CMake
- [ ] **4.12** `QOpenGLWidget` — add `OpenGLWidgets` module to CMake
- [ ] **4.13** Add explicit `#include <QFile>` where Qt6 only forward-declares
- [ ] **4.14** `powercontrol.ui` — move `setNum` connection to code with `qOverload`
- [ ] **4.15** `QString != NULL` -> `!str.isNull()`

### Phase 5: Signal/Slot & Overload Fixes

- [ ] **5.1** Fix removed signal overloads: `QComboBox::currentIndexChanged(QString)`, `QSpinBox::valueChanged(QString)` -> use `int` parameter + `itemText(idx)`
- [ ] **5.2** Modernize SIGNAL()/SLOT() in core, gui, adc, dac packages (84 connections — done in PoC)
- [ ] **5.3** Leave `symbol_controller.cpp` and `deviceimpl.cpp` in old-style (intentional — runtime polymorphic dispatch)
- [ ] **5.4** Skip M2K plugin (deferred to M2K refactor)

### Phase 6: Threading & Concurrency

- [ ] **6.1** `QtConcurrent::run()` — add `(void)` cast for fire-and-forget calls (`[[nodiscard]]`)
- [ ] **6.2** `QFutureWatcher::setPaused/isPaused` -> `setSuspended/isSuspending`
- [ ] **6.3** `QSignalBlocker` temporaries -> assign to named variables (silent bug fix)
- [ ] **6.4** `QMouseEvent::localPos()` -> `position()`, `QEnterEvent::pos()` -> `position().toPoint()`

### Phase 7: JS Engine & Runtime Validation

- [ ] **7.1** Fix non-standard JS syntax: `for (each in o)` -> `for (var each in o)` in `scopyjs.cpp`
- [ ] **7.2** Run unit tests: `QT_QPA_PLATFORM=offscreen ctest --output-on-failure` (expect 27/27 pass)
- [ ] **7.3** Launch Scopy with X11 display — verify all plugins load
- [ ] **7.4** Test JS scripting tool — verify `print("hello")` and basic evaluation work

### Phase 8: Deprecation Cleanup

- [ ] **8.1** `QMouseEvent::pos()` -> `position().toPoint()` in 8 gui files
- [ ] **8.2** Clean up duplicate `setContentsMargins` from sed artifacts
- [ ] **8.3** Remove `Qt::AA_UseHighDpiPixmaps` (no-op in Qt6)

### Phase 9: Verification

- [ ] **9.1** Run Clazy with 5 Qt6 porting checks (use `porting_process/run_clazy.sh`)
- [ ] **9.2** Verify zero findings across all files (excluding M2K)
- [ ] **9.3** Full runtime test — launch, navigate tools, test JS scripting
- [ ] **9.4** Review known risks (see Section 6)

### Phase 10: CI Pipeline

- [ ] **10.1** Push finalized Docker image to container registry
- [ ] **10.2** Create `ci-qt6.yml` orchestrator workflow
- [ ] **10.3** Create `ubuntubuild-qt6.yml` reusable workflow
- [ ] **10.4** Verify CI passes on push to `qt6_migration`

---

## 6. Known Issues & Solutions

### 6.1 Issues Encountered and Fixed in PoC

| # | Issue | Root Cause | Solution | Files Affected |
|---|-------|-----------|----------|----------------|
| 1 | `setMargin()` not found | Removed in Qt6 | `setContentsMargins(N,N,N,N)` | 165 files |
| 2 | `QRegExp` not found | Moved to Qt5Compat | Use `QRegularExpression` | 8 files |
| 3 | `qAsConst()` deprecated | Use C++ stdlib | `std::as_const()` | 111 files |
| 4 | `QStyleOption::init()` removed | API change | `initFrom()` | 1 macro, ~12 widgets |
| 5 | `QSignalTransition` not found | Extracted to own module | Add `StateMachine` to CMake + aqtinstall `qtscxml` | gui/CMakeLists.txt |
| 6 | `QOpenGLWidget` not found | Extracted to own module | Add `OpenGLWidgets` to CMake | gui/CMakeLists.txt |
| 7 | `QString(int)` ambiguous | Constructor change | `QString::number()` | ~10 files |
| 8 | `QtConcurrent::run()` wrong arg order | API change | Swap first two arguments | 17 sites |
| 9 | `QFont::setWeight(int)` removed | Changed to enum | `QFont::Bold` etc. | 5 sites |
| 10 | `splitRef()`/`midRef()` etc. removed | Simplified API | Use `split()`/`mid()` | sed pass |
| 11 | Signal overloads removed | `QString` overloads dropped | Use `int` + `itemText()` | spectrum_analyzer, patterns |
| 12 | `QSignalBlocker` temporaries | Destroyed immediately in Qt6 | Assign to named variable | 3 sites (silent bugs) |
| 13 | `for (each in o)` JS syntax | Non-standard, V4 rejects | `for (var each in o)` | scopyjs.cpp |
| 14 | KArchive `master` needs Qt >= 6.8 | Version mismatch | Pin to `v6.x.y` tags | build script |
| 15 | `Qt::UniqueConnection` + lambda crash | Qt6 enforces assert | Extract lambda to named slot | regmap (2 sites) |
| 16 | `.QWidget` CSS selector breaks with `Q_OBJECT` | `className()` changes | Don't add `Q_OBJECT` if not needed, or fix `Style::setBackgroundColor()` | style.cpp |
| 17 | SVG files missing `fill` attribute | Theme system can't apply colors | Add solid `fill` color to SVG files | Various SVG resources |

### 6.2 Known Risks (Not Blocking)

**`disconnect()` with nullptr slot — 3 files:**
Lambda connections cannot be disconnected by `disconnect(sender, &Signal, receiver, nullptr)` in Qt6. Requires storing `QMetaObject::Connection` handles.
- `gui/src/widgets/plotlegend.cpp:115-117`
- `gui/src/widgets/menuplotchannelcurvestylecontrol.cpp:123`
- `gui/src/cursorcontroller.cpp:193-194, 241-244`

**Action:** Verify at runtime. Refactor to stored connection handles if signals fire unexpectedly.

**`int` vs `qsizetype` — ~175 instances:**
`QList::size()` returns `qsizetype` in Qt6 (was `int`). Functionally correct on 64-bit. Generates `-Wconversion` warnings if enabled. Accepted as-is.

**`Style::setBackgroundColor()` fragility:**
The `.QWidget` CSS class selector in `gui/src/style.cpp:302-309` only matches widgets without `Q_OBJECT`. Any future `Q_OBJECT` addition to a QWidget subclass using this method will break styling. Should be fixed to use `widget->metaObject()->className()` dynamically.

### 6.3 Items Verified as Non-Issues

| Qt6 Breaking Change | Scopy Status |
|---------------------|-------------|
| `QStringRef` usage | Zero occurrences |
| `QRecursiveMutex` inherits `QMutex` | Zero occurrences |
| `QVariant` as `QMap`/`QHash` key | Zero occurrences |
| `QQmlEngine::setContextProperty()` | Zero occurrences |
| `QEvent` copy constructor | No custom QEvent subclasses |
| `QProcess::start()` single-string overload | All sites use 2-arg form |
| `QAction` moved Widgets->Gui | Transitive CMake link resolves it |
| `QFontDatabase` static-only | Already uses static calls |

---

## 7. Tooling Reference

### 7.1 Clazy Verification

Run after all fixes to confirm zero remaining Qt5 incompatibilities:

```bash
# Inside Docker container
apt-get install -y clazy
bash /home/runner/scopy/porting_process/run_clazy.sh
```

**5 checks run:** `qt6-deprecated-api-fixes`, `qt6-header-fixes`, `qt6-qhash-signature`, `qt6-fwd-fixes`, `missing-qobject-macro`

**Expected result:** Zero findings (confirmed in PoC across 529 files)

### 7.2 Compile-Time Deprecation Guard

After porting is complete, optionally add to root `CMakeLists.txt`:
```cmake
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)
```
This catches any future use of Qt 5.15-deprecated APIs at compile time.

### 7.3 Useful sed Commands (from PoC)

All commands exclude `build/`, `.git/`, and M2K directories:

```bash
# setMargin -> setContentsMargins
find . \( -path '*/build*' -o -path '*/.git' -o -path '*/m2k/*' \) -prune -o \
  \( -name '*.cpp' -o -name '*.h' \) -print | \
  xargs sed -i -E 's/setMargin\(([^)]+)\)/setContentsMargins(\1, \1, \1, \1)/g'

# qAsConst -> std::as_const
sed -i 's/qAsConst(/std::as_const(/g'

# endl -> Qt::endl (QTextStream)
sed -i 's/<< endl/<< Qt::endl/g'

# String view methods
sed -i 's/\.midRef(/\.mid(/g; s/\.leftRef(/\.left(/g; s/\.rightRef(/\.right(/g'

# enterEvent parameter type
sed -i 's/enterEvent(QEvent \*/enterEvent(QEnterEvent */g'
```

---

## 8. M2K Plugin — Deferred

The M2K plugin (`packages/m2k/`) is **out of scope** for this porting effort.

- ~375 old-style SIGNAL/SLOT connections across 39 files
- Old-style `SIGNAL()`/`SLOT()` macros **fully work in Qt6** — no functional regression
- M2K is scheduled for a near-term refactor where Qt6 best practices will be applied directly
- The PoC confirmed M2K compiles and runs under Qt6 without any SIGNAL/SLOT conversion

---

## 9. Estimated Effort Summary

| Phase | Effort | Notes |
|-------|--------|-------|
| 1. Environment Setup | 1 day | Mostly done — update version and rebuild |
| 2. CMake Migration | 0.5 day | Done in PoC — cherry-pick and verify |
| 3. Mass Replacements | 0.5 day | sed commands ready, review artifacts |
| 4. API Migration | 2-3 days | 15 categories of manual fixes |
| 5. Signal/Slot | 1-2 days | 84 connections done in PoC, verify |
| 6. Threading | 0.5 day | Small scope, done in PoC |
| 7. JS Engine & Runtime | 0.5 day | Minimal changes needed |
| 8. Deprecation Cleanup | 0.5 day | 8 files + artifact cleanup |
| 9. Verification | 1 day | Clazy + runtime testing |
| 10. CI Pipeline | 1-2 days | Docker push + workflow creation |

**Total estimated:** ~8-11 days (much of Phase 2-8 can be cherry-picked from PoC commits)

---

## 10. References

| Resource | URL |
|----------|-----|
| Qt 5 to Qt 6 Porting Guide | https://doc.qt.io/qt-6/portingguide.html |
| Qt Core Changes for Qt 6 | https://doc.qt.io/qt-6/qtcore-changes-qt6.html |
| Porting with Clazy | https://doc.qt.io/qt-6/porting-to-qt6-using-clazy.html |
| Qt 6 Build System | https://doc.qt.io/qt-6/qt6-buildsystem.html |
| What's New in Qt 6.0 | https://doc.qt.io/qt-6/whatsnew60.html |
| Qt 6 Releases & LTS Info | https://doc.qt.io/qt-6/qt-releases.html |
| PoC Briefing (this branch) | `porting_process/porting_process_breafing.md` |
| PoC Detailed Instructions | `porting_process/porting_process_instructions.md` |
| PoC Qt Logic Reference | `porting_process/porting_logic.md` |
| Clazy Run Script | `porting_process/run_clazy.sh` |
