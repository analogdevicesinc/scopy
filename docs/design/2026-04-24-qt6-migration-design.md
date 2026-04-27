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
**Scope:** Build system files only — `CMakeLists.txt` across all modules.

| File | Change |
|------|--------|
| `CMakeLists.txt` (root) | `find_package(QT NAMES Qt6 ...)`, add `OpenGLWidgets`, `StateMachine` |
| `pkg-manager/CMakeLists.txt` | `KF5Archive` → `KF6Archive` |
| `gui/CMakeLists.txt` | Add `Qt6::OpenGLWidgets`, `Qt6::StateMachine`; `QWT_QT_VERSION qt6` |
| `packages/*/CMakeLists.txt` | `Qt5::*` → `Qt${QT_VERSION_MAJOR}::*` |
| `cmake/Modules/` | Any custom Find modules that reference Qt5 explicitly |
| `common/include/common/common.h` | Remove Qt 5.14 compat shim |

**Reference:** PoC commit `eb9d183db` (first build with Qt6).

**Validation:** `cmake -B build-qt6 -DCMAKE_PREFIX_PATH=/opt/Qt/6.8.3/gcc_64` succeeds (configuration stage).

---

### PR 4: Mass Automated Replacements
**Scope:** Source code only — safe, mechanical text replacements via sed.

| Replacement | Estimated Files | sed Pattern |
|-------------|----------------|-------------|
| `setMargin(N)` → `setContentsMargins(N,N,N,N)` | ~165 | `s/setMargin\(([^)]+)\)/setContentsMargins(\1, \1, \1, \1)/g` |
| `qAsConst(` → `std::as_const(` | ~111 | `s/qAsConst(/std::as_const(/g` |
| `<< endl` → `<< Qt::endl` | ~2 | `s/<< endl/<< Qt::endl/g` |
| `.midRef(` → `.mid(` (and `leftRef`, `rightRef`) | few | `s/\.midRef(/\.mid(/g` |
| `setWeight(75)` → `setWeight(QFont::Bold)` | ~5 | manual review |
| `enterEvent(QEvent *` → `enterEvent(QEnterEvent *` | varies | `s/enterEvent(QEvent \*/enterEvent(QEnterEvent */g` |

**All sed commands exclude:** `build/`, `.git/`, `m2k/`, `staging_ubuntu20/`

**Post-sed review:**
- Check for duplicate `setContentsMargins` in `channel_manager.cpp` (known artifact)
- Run `tools/format.sh` to fix formatting issues introduced by sed

**Validation:** `grep -r "setMargin\|qAsConst\|midRef\|leftRef\|rightRef"` returns zero hits outside excluded dirs.

---

### PR 5: API Migration (Manual Fixes)
**Scope:** 15 categories of manual C++ fixes, each small and isolated.

| Category | Files | Change |
|----------|-------|--------|
| `QRegExp` → `QRegularExpression` | 8 | Replace class + include |
| `QStyleOption::init()` → `initFrom()` | ~12 widgets | One macro update |
| `QString(int)` → `QString::number(int)` | ~10 | Disambiguate constructor |
| `QVariant::type()` → `typeId()` | varies | Type system update |
| `QtConcurrent::run()` arg order | 17 | Swap first two arguments |
| Implicit `QString→QFileInfo` | varies | Add explicit constructor |
| `QList` explicit constructor | varies | Add `QList<T>{...}` |
| Remove `qRegisterMetaTypeStreamOperators` | varies | Delete calls (auto-registered in Qt6) |
| `filterRegExp()` → `filterRegularExpression()` | varies | Rename |
| Qt3D includes path change | varies | `Qt3DRender/QGeometry` → `Qt3DCore/QGeometry` |
| Add explicit `#include <QFile>` | varies | Qt6 only forward-declares |
| `powercontrol.ui` signal fix | 1 | Move `setNum` to code with `qOverload` |
| `QString != NULL` → `!str.isNull()` | varies | Null comparison |
| `QMouseEvent::pos()` → `position().toPoint()` | 8 gui files | Deprecation fix |
| Remove `Qt::AA_UseHighDpiPixmaps` | 1 | No-op in Qt6 |

**Reference:** PoC commits `bce251e46`, `2432fb2d2`, `220cfc694`.

**Validation:** Full compilation succeeds (may still have linker errors until PR 6).

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
