# Scopy Qt5 → Qt6 Porting — Team Briefing

> **Note:** This was a proof-of-concept. The steps below describe what was discovered and attempted; they are not a finalized production plan.

---

# Part 1 — Brief Summary

## Step 1: Docker Dev Environment `[COMPLETE]`
- Created `Dockerfile.qt6` (Ubuntu 24.04) + `ubuntu_build_process_qt6.sh`
- Qt 6.7.0 via `aqtinstall`, KDDockWidgets 2.2 with `-DKDDockWidgets_QT6=ON`
- **Issue:** ECM/KArchive use `master` not `kf6`; KArchive `master` requires Qt ≥ 6.8 → pinned to `v6.7.0` tags
- **Issue:** `qt3d` and `qtscxml` modules not included by default → added explicitly
- Result: `qmake6 --version` confirms Qt 6.7.0

## Step 2: CMake Build System Migration `[COMPLETE]`
- 8 files: `Qt5` → `Qt6` in `find_package`, `KF5Archive` → `KF6Archive`, `Qt5::*` → `Qt6::*`
- New modules added: `OpenGLWidgets`, `StateMachine` (extracted from base in Qt 6)
- Qwt: `QWT_QT_VERSION qt5` → `qt6`
- Result: `cmake -B build-qt6` completes with Qt6

## Step 3: `setMargin()` Mass Replacement `[COMPLETE]`
- `setMargin(N)` removed in Qt6 → `setContentsMargins(N, N, N, N)`
- **397 occurrences** across **165 files** — sed one-liner
- **Issue:** Introduced duplicate consecutive calls in `channel_manager.cpp` (lines 80–81, 244–245) → tracked in Step 13

## Step 4: `QRegExp` → `QRegularExpression` `[COMPLETE]`
- 8 files migrated; zero `QRegExp` references remaining
- Key changes: `rx.indexIn(text)` → `rx.match(text)`, `rx.cap(N)` → `match.captured(N)`, `setFilterRegExp` → `setFilterFixedString`

## Step 5: Qt6 API Fixes — Compile Errors `[COMPLETE]`
- **22 individual fixes** applied; build reaches 100% with zero errors
- Notable: `QStyleOption::init()` → `initFrom()`, `QString(int)` → `QString::number()`, `qAsConst()` → `std::as_const()` (239 occurrences), `QFont::setWeight(75)` → `QFont::Bold`, Qt3D includes moved `Qt3DRender` → `Qt3DCore`, `QVariant::type()` → `typeId()`
- **Issue:** `QSignalTransition` moved to `StateMachine` module; not in default aqtinstall

## Step 6: Signal Overload Cleanup `[COMPLETE]`
- Removed overloads deleted in Qt6: `QComboBox::currentIndexChanged(QString)`, `QSpinBox::valueChanged(QString)`
- Fixed slots to receive `int` index and retrieve text via `itemText(idx)`
- Cleaned deprecation warnings: `QDropEvent::pos()`, `QFutureWatcher::setPaused/isPaused`, `QVariant::type()`, `Qt::AA_UseHighDpiPixmaps`

## Step 7: Threading & Concurrency `[COMPLETE]`
- `QtConcurrent::run()` argument order swapped (17 occurrences); added `(void)` casts for `[[nodiscard]]`
- `QFutureWatcher::setPaused` → `setSuspended`, `isPaused` → `isSuspending`
- `QSignalBlocker` temporaries → named variables (were silent bugs)
- `QMouseEvent::localPos()` → `position()`, `QEnterEvent::pos()` → `position().toPoint()`
- Result: 27/27 C++ unit tests pass

## Steps 8–9: SIGNAL/SLOT Modernization `[COMPLETE — core/gui/adc/dac]`
- Converted 84 old-style `SIGNAL()`/`SLOT()` connections across core, gui, adc, dac packages
- `symbol_controller.cpp` left in old-style: runtime polymorphic signal resolution required
- `deviceimpl.cpp` left in old-style: `dynamic_cast<QObject*>` prevents compile-time resolution

## Step 10: M2K Plugin `[INTENTIONALLY SKIPPED]`
- ~375 old-style connections across 39 files
- **Decision:** M2K is scheduled for refactor; apply new-style directly during refactor
- Old-style SIGNAL/SLOT **still works in Qt6** — no runtime impact

## Step 11: QML/JS Engine `[COMPLETE]`
- QJSEngine/QJSValue API: almost entirely unchanged between Qt5 and Qt6
- Fix: non-standard `for (each in o)` → `for (var each in o)` in `scopyjs.cpp`
- Result: JS scripting tool functional, all plugins load

## Step 12: CI Pipeline `[PLANNED — not implemented]`
- Files identified; platform readiness assessed
- Ubuntu: ready (Dockerfile.qt6 + build script exist)
- AppImage / ARM / Windows / Flatpak: need Qt6-specific Docker images / toolchains

## Step 13: Remaining Deprecation Fixes `[PENDING]`
- `QMouseEvent::pos()` → `position().toPoint()` in 8 `gui/` files
- Duplicate `setContentsMargins` in `channel_manager.cpp` (sed artifact)

## Step 14: Clazy Verification Run `[COMPLETE]`
- Zero findings across 529 files on all 5 Qt6 porting checks
- M2K excluded intentionally

---

# Step 15: ## Known Issue: Qt::UniqueConnection + Lambda Crash

# Step 16: svg images need to have "fill" parameter or they won't work with the theme sistem 

# Part 2 — Detailed Breakdown

---

## Step 1: Docker Dev Environment

### Goal
Provide a Qt6 build environment without touching the host Qt5 installation.

### What was done
- Used existing `ci/ubuntu/docker_ubuntu/Dockerfile.qt6` (Ubuntu 24.04) and `ubuntu_build_process_qt6.sh`
- Qt 6.7.0 installed via `aqtinstall`
- KDDockWidgets 2.2 built from source with Qt6 support enabled

### Issues encountered

**Issue 1 — `.dockerignore` blocked Qt6 script**
`ci/ubuntu/.dockerignore` only whitelisted the Qt5 build script. Added `ubuntu_build_process_qt6.sh`.

**Issue 2 — Wrong branch names for ECM and KArchive**
Script used `kf6` branch; those repos use `master` for KF6 code.
```
ECM_BRANCH=kf6      →  ECM_BRANCH=master
KARCHIVE_BRANCH=kf6 →  KARCHIVE_BRANCH=master
```

**Issue 3 — KArchive `master` requires Qt ≥ 6.8**
We target Qt 6.7.0. Solution: pin to version-matched tags.
```
ECM_BRANCH=master      →  ECM_BRANCH=v6.7.0
KARCHIVE_BRANCH=master →  KARCHIVE_BRANCH=v6.7.0
```

**Issue 4 — KDDockWidgets defaults to Qt5 frontend**
```
CURRENT_BUILD_CMAKE_OPTS="" → CURRENT_BUILD_CMAKE_OPTS="-DKDDockWidgets_QT6=ON"
```

**Issue 5 — Missing compression packages for karchive v6.7.0**
Added: `libzstd-dev libbz2-dev liblzma-dev`

### Verification
```bash
docker run --rm scopy-qt6-dev /opt/Qt/6.7.0/gcc_64/bin/qmake6 --version
```

---

## Step 2: CMake Build System Migration

### Goal
`cmake -B build-qt6` completes inside the Docker container.

### What was done (8 files)

| File | Change |
|---|---|
| `CMakeLists.txt:65` | `Qt5` → `Qt6` in `find_package` |
| `CMakeLists.txt:75-80` | Removed Qt5 version check |
| `CMakeLists.txt:241-258` | `KF5Archive` → `KF6Archive` |
| `cmake/Modules/ScopyTest.cmake:30,37` | `Qt5` → `Qt${QT_VERSION_MAJOR}` |
| `packages/imu/.../CMakeLists.txt:115-121` | `Qt5::*` → `Qt${QT_VERSION_MAJOR}::*` |
| `packages/generic-plugins/.../CMakeLists.txt:130` | `Qt5::Test` → `Qt${QT_VERSION_MAJOR}::Test` |
| `pkg-manager/CMakeLists.txt:67-70` | `KF5Archive` → `KF6Archive` |
| `gui/CMakeLists.txt:69`, `m2k-gui/CMakeLists.txt:52` | `QWT_QT_VERSION qt5` → `qt6` |

Additional: removed Qt 5.14 compat shim from `common/include/common/common.h`.

### Issues encountered

**Issue — Qt 3D not in default aqtinstall**
Added `-m qt3d` to the install command.

**Issue — `git safe.directory` inside Docker**
Mounted repo has a different owner than container root user. Must run:
```bash
git config --global --add safe.directory /home/runner/scopy
```

### New modules required in Qt 6
- `OpenGLWidgets` — `QOpenGLWidget` moved out of `Widgets`
- `StateMachine` — `QSignalTransition` moved out of `Core`

---

## Step 3: `setMargin()` Mass Replacement

### Change
`QLayout::setMargin(N)` removed in Qt6. Replacement: `setContentsMargins(N, N, N, N)`.

### Scale
- **397 occurrences**, **165 files**
- Applied with a single sed pass (excluding build dirs and `.git`):

```bash
find /home/imuthi/scopy \( -path '*/build*' -o -path '*/.git' \) -prune -o \
  \( -name '*.cpp' -o -name '*.h' \) -print | \
  xargs sed -i -E 's/setMargin\(([^)]+)\)/setContentsMargins(\1, \1, \1, \1)/g'
```

### Issue
The sed pass introduced **duplicate consecutive `setContentsMargins` calls** in:
- `packages/m2k/plugins/m2k/m2k-gui/src/channel_manager.cpp` (lines 80–81, 244–245)

These must be cleaned up manually (Step 13, Fix B).

---

## Step 4: `QRegExp` → `QRegularExpression`

### Change
`QRegExp` and `QRegExpValidator` moved to Qt5Compat in Qt6. Must use `QRegularExpression`.

### API mapping applied

| Qt5 | Qt6 |
|---|---|
| `QRegExp rx(pattern)` | `QRegularExpression rx(pattern)` |
| `QRegExpValidator` | `QRegularExpressionValidator` |
| `rx.indexIn(text)` | `rx.match(text).hasMatch()` |
| `rx.cap(N)` | `match.captured(N)` |
| `setFilterRegExp(QRegExp(s, FixedString))` | `setFilterFixedString(s)` |
| `filterRegExp()` | `filterRegularExpression()` |

### Result
8 files migrated; zero `QRegExp` or `QRegExpValidator` references remaining.

---

## Step 5: Qt6 API Fixes — Compile Errors

### Summary
22 individual fixes applied to reach a clean build. Listed in order of discovery.

### Fix list

**`versionchecker.cpp:119` — `QList` constructor is now explicit**
```cpp
reply->ignoreSslErrors({QSslError::NoPeerCertificate});
→ reply->ignoreSslErrors(QList<QSslError>{QSslError(QSslError::NoPeerCertificate)});
```

**`qAsConst()` deprecated — 239 occurrences in 111 files**
```bash
sed -i 's/qAsConst(/std::as_const(/g'
```

**`gui/include/gui/utils.h:81` — `QStyleOption::init()` removed**
```cpp
opt.init(this); → opt.initFrom(this);
```

**`QSignalTransition` moved to `StateMachine` module**
Added `StateMachine` to `find_package` and `target_link_libraries` in `gui/CMakeLists.txt`. Also added `qtscxml` to aqtinstall (StateMachine ships with it).

**Qwt pulls in `<qopenglwidget.h>` which needs `OpenGLWidgets`**
Added to `gui/CMakeLists.txt`.

**`smallOnOffSwitch.cpp:177` — pre-existing bug exposed**
`leaveEvent` was calling `QCheckBox::enterEvent` — fixed to `leaveEvent`.

**`tutorialbuilder.h` — `QFile` forward-declared in Qt6 headers, needs explicit include**
```cpp
#include <QFile>  // added
```

**`menuwidget.cpp:87` — `QString(int)` ambiguous in Qt6**
```cpp
QString(uuid) → QString::number(uuid)
```

**`QFont::setWeight(int)` → enum**
```bash
sed -i 's/setWeight(75)/setWeight(QFont::Bold)/g'
```

**`logdatatofile.cpp:61` — QString* compared without dereference**
```cpp
if(currentFileHeader != fileHeader) → if(*currentFileHeader != fileHeader)
```

**`logdatatofile.cpp:239` — `splitRef()` removed**
```cpp
splitRef("/") → split("/")
```

**`scopymainwindow_api.cpp:367` — `QString != NULL` ambiguous**
```cpp
if(prefName != NULL) → if(!prefName.isNull())
```

**`QtConcurrent::run()` argument order — 17 occurrences**
```cpp
// Qt5
QtConcurrent::run(obj, &Class::method, args...);
// Qt6
QtConcurrent::run(&Class::method, obj, args...);
```

**`pkgmanager.cpp:151`, `pkgutil.cpp:234` — implicit `QString→QFileInfo` removed**
```cpp
return pkgPath; → return QFileInfo(pkgPath);
files.append(it.next()); → files.append(QFileInfo(it.next()));
```

**`fmcomms5calibration.cpp:73` — template deduction fails for 0-arg member function**
```cpp
QtConcurrent::run(&Class::method, this);
→ QtConcurrent::run([this]() { method(); });
```

**`extprocplotinfo.h:88` — `id < 0` where `id` is QString**
```cpp
id < 0 → id.toInt() < 0
```

**Qt3D include paths changed**
```cpp
#include <Qt3DRender/QGeometry>  → #include <Qt3DCore/QGeometry>
#include <Qt3DRender/QAttribute> → #include <Qt3DCore/QAttribute>
```

**`midRef()`/`leftRef()`/`rightRef()` removed**
```bash
sed -i 's/\.midRef(/\.mid(/g; s/\.leftRef(/\.left(/g; s/\.rightRef(/\.right(/g'
```

**`QString(int/bool)` ambiguous — additional sites**
`fftplotmanagersettings.cpp` (3), `timeplotmanagersettings.cpp` (3), `jsonformatedelement.cpp` (3 bools) → `QString::number()`

**`qRegisterMetaTypeStreamOperators` removed (auto-registered in Qt6)**
Removed 7 calls in `logicanalyzer_api.h`, 4 calls in `pattern_generator_api.h`.

**`filterRegExp()` → `filterRegularExpression()`**
`iiosortfilterproxymodel.cpp:35`

**`endl` not in scope for QTextStream**
```bash
sed -i 's/<< endl/<< Qt::endl/g'
```

**`powercontrol.ui` — `setNum(int)` overload ambiguous**
Removed connection from `.ui`; added manual `qOverload<int>(&QLabel::setNum)` connect in `power_controller.cpp`.

**`QVariant::type()` / `QVariant::Type` removed**
Multiple files: `var.type() == QVariant::String` → `var.typeId() == QMetaType::QString`, etc.

### Runtime — X11 display from Docker
Additional xcb packages required for Qt6 runtime:
```bash
libxcb-cursor0 libxcb-icccm4 libxcb-keysyms1 libxcb-shape0
```

---

## Step 6: Signal Overload Cleanup

### Removed overloads
Qt6 removed `QString` overloads of:
- `QComboBox::currentIndexChanged(const QString &text)`
- `QSpinBox::valueChanged(const QString &text)`

**`patterns.cpp`** — `SIGNAL(activated(QString))` → `SIGNAL(activated(int))` (4 places)

**`spectrum_analyzer.hpp/.cpp`** — 3 slots `on_*_currentIndexChanged(const QString&)` → `(int idx)`, body updated to `itemText(idx)`.

### Deprecation warning fixes in this step
- `main.cpp:119` — removed `Qt::AA_UseHighDpiPixmaps` (no-op in Qt6, always on)
- `basemenu.cpp:126`, `basemenuitem.cpp:184` — `event->pos()` → `event->position().toPoint()`
- `iiomanager.cpp:66,77` — `setPaused/isPaused` → `setSuspended/isSuspending`
- `logicanalyzer_api.cpp:253` — `p_val.type()` → `p_val.typeId()`
- `logicanalyzer_api.cpp:349-385` — `QMetaType::typeName(X)` → `QMetaType(X).name()`

---

## Step 7: Threading & Concurrency

### Key changes

**`QtConcurrent::run()` is now `[[nodiscard]]`**
Fire-and-forget sites need a `(void)` cast. 4 places in runInHwThreadPool, 4 more scattered.

**`QFutureWatcher` pause API renamed**
```cpp
watcher.setPaused(true);  → watcher.setSuspended(true);
watcher.isPaused();       → watcher.isSuspending();
```

**`QSignalBlocker` temporaries destroyed immediately**
```cpp
// Qt5 — worked accidentally due to lifetime extension
QSignalBlocker(spinBox);
// Qt6 — must name it
auto blocker = QSignalBlocker(spinBox);
```
3 such cases in `spectrum_analyzer.cpp` — these were latent bugs.

**Mouse/enter event position APIs**
- `QEnterEvent::pos()` → `position().toPoint()` — `axishandle.cpp`
- `QMouseEvent::localPos()` → `position()` — `dropdown_switch_list.cpp`

**`QTranslator::load` return value**
Now should be checked; added warning log.

### Verification
```bash
QT_QPA_PLATFORM=offscreen ctest --output-on-failure
# Result: 27/27 tests passed
```

---

## Steps 8–9: SIGNAL/SLOT Modernization

### Why
Old-style `SIGNAL()`/`SLOT()` macros are string-based and fail silently at runtime. New-style `&Class::method` fails at compile time.

### What was converted
**GUI (Step 8):** `spinbox_a.cpp`, `symbol.cpp`, `menu_anim.cpp`, `semiexclusivebuttongroup.cpp`, `menucontrolbutton.cpp` — 24 connections.

**Core (Step 8):** `scanbuttoncontroller.cpp`, `devicebrowser.cpp`, `scopyaboutpage.cpp`, `iiotabwidget.cpp`, `scopyhomepage.cpp`, `scopymainwindow.cpp`, `devicemanager.cpp` — ~35 connections.

**ADC/DAC plugins (Step 9):** `bufferdacaddon.cpp`, `grtimesinkcomponent.cpp`, `grfftsinkcomponent.cpp`, `adctimeinstrumentcontroller.cpp`, `adcfftinstrumentcontroller.cpp`, `adcinstrumentcontroller.cpp` — 25 connections.

### Left in old-style (intentional)
- `symbol_controller.cpp` — signal resolution requires runtime polymorphic dispatch
- `deviceimpl.cpp` — all 18 connections use `dynamic_cast<QObject*>(p)` as receiver

### Notes
- `QOverload<>::of()` used where slots are overloaded
- Lambdas used where signal/slot parameter counts differ
- `disconnect(sender, &Signal, nullptr, nullptr)` pattern used for bulk disconnect

---

## Step 10: M2K Plugin — Intentionally Skipped

### Scale
~375 old-style connections across 39 files.

### Decision
M2K is scheduled for a near-term refactor. Attempting the conversion now:
1. Exposed many edge cases: protected slots, overloaded Qwt signals, polymorphic types
2. Required reverting ~10% of attempted conversions back to old-style anyway
3. The refactor will apply new-style directly — no point in converting twice

### Impact of not converting
- Old-style SIGNAL/SLOT **fully works in Qt6** — no functional regression
- Only risk: silent failures on typos (existing risk, not introduced by Qt6)

---

## Step 11: QML/JS Engine

### Assessment
`QJSEngine`/`QJSValue` API is almost entirely unchanged between Qt5 and Qt6. All 33 files using these APIs were found compatible with no changes.

### Fix applied
`pluginbase/src/scopyjs.cpp:74` — non-standard for-each syntax rejected by Qt6's V4 engine:
```js
for (each in obj) { ... }   // Qt5 — non-standard, worked
→ for (var each in obj) { ... }  // Qt6 — standard
```

### M2K includes (deferred)
8 M2K files use `#include <QtQml/QJSEngine>` — both `<QtQml/...>` and `<QJSEngine>` work in Qt6. Deferred to M2K refactor.

### Verification
- Scopy launches with X11 display
- JS scripting tool evaluates `print("hello")` and `1+1` correctly
- All plugins load with no console errors

---

## Step 12: CI Pipeline

### Status: Planned, not implemented

### Files to create/modify

| File | Action |
|---|---|
| `ci/ubuntu/create_docker_image.sh` | Add `ubuntu24_qt6()` using `Dockerfile.qt6` |
| `.github/workflows/ci-qt6.yml` | Qt6 orchestrator, triggers on `qt6_migration` branch |
| `.github/workflows/ubuntubuild-qt6.yml` | Ubuntu 24 Qt6 build |
| `.github/workflows/appimage-x86_64-qt6.yml` | Stub — needs Qt6 AppImage Docker image |
| `.github/workflows/mingwbuild-qt6.yml` | Stub — needs Qt6 MinGW Docker image |
| `.github/workflows/linuxflatpakbuild-qt6.yml` | Stub — needs `org.kde.Sdk//6.x` |

### Platform readiness

| Platform | Status |
|---|---|
| Ubuntu x86_64 | Ready — Dockerfile.qt6 + build script exist |
| AppImage x86_64 | Needs Qt6 Docker image |
| AppImage ARM | Needs Qt6 cross-compilation sysroot |
| Windows/MinGW | Needs Qt6 MSYS2 toolchain |
| Flatpak | Needs `org.kde.Sdk//6.x` runtime |

---

## Step 13: Remaining Deprecation Fixes

### Fix A — `QMouseEvent::pos()` → `position().toPoint()`
Deprecated in Qt6; should be updated in these 8 files:
- `gui/src/basemenuitem.cpp`
- `gui/src/buffer_previewer.cpp`
- `gui/src/plotzoomer.cpp`
- `gui/src/symbol_controller.cpp`
- `gui/src/axishandle.cpp` (partially done in Step 7)
- `gui/src/completion_circle.cpp`
- `gui/src/hoverwidget.cpp`
- `gui/src/basictracker.cpp`

### Fix B — Duplicate `setContentsMargins` in `channel_manager.cpp`
The Step 3 sed pass introduced consecutive duplicate calls at lines 80–81 and 244–245.
Manual removal: keep the first call, delete the second.

---

## Step 14: Clazy Verification Run

### Goal
Use Clazy's 5 dedicated Qt6 porting checks as a final pass. Consumes the existing `build-qt6/compile_commands.json` — no rebuild needed.

### Checks run

| Check | Purpose |
|---|---|
| `qt6-deprecated-api-fixes` | Deprecated Qt5 APIs still in use |
| `qt6-header-fixes` | Wrong `#include` paths for Qt6 |
| `qt6-qhash-signature` | `qHash()` returning `uint` instead of `size_t` |
| `qt6-fwd-fixes` | Bad forward declarations |
| `missing-qobject-macro` | Classes missing `Q_OBJECT` |

### Result
**Zero findings across 529 files.** M2K excluded intentionally.

---

## Known Risks (not blocking, flagged for awareness)

### `disconnect()` with `nullptr` slot — 3 files
In Qt6, `disconnect(sender, &Signal, receiver, nullptr)` cannot disconnect **lambda** connections — these require storing the `QMetaObject::Connection` handle.

Affected:
- `gui/src/widgets/plotlegend.cpp:115-117`
- `gui/src/widgets/menuplotchannelcurvestylecontrol.cpp:123`
- `gui/src/cursorcontroller.cpp:193-194, 241-244`

Action: verify at runtime; refactor to `QMetaObject::Connection` handles if signals fire unexpectedly after disconnect.

### `int` vs `qsizetype` for container sizes — ~175 instances
Qt6 changed `QList::size()` return type from `int` to `qsizetype`. Functionally correct on 64-bit; generates `-Wconversion` warnings if that flag is enabled. Accepted as-is.

### Items verified as non-issues (searched, zero occurrences)

| Change | Result |
|---|---|
| `QStringRef` usage | Zero occurrences |
| `QRecursiveMutex` inheriting `QMutex` | Zero occurrences |
| `QVariant` as `QMap`/`QHash` key | Zero occurrences |
| `QQmlEngine::setContextProperty()` (deprecated) | Zero occurrences |
| `QEvent` copy constructor (now protected) | No custom QEvent subclasses |
| `QProcess::start()` single-string overload | All sites use 2-arg form |
| `QKeyCombination` | Code uses `QKeySequence::matches()` — correct |
