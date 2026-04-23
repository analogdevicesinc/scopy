# Scopy Qt5 → Qt6 Porting Process

## Major Qt5 → Qt6 Breaking Changes Driving This Migration

1. **`setMargin(N)`** removed → use `setContentsMargins(N,N,N,N)` (408 occurrences)
2. **`QRegExp`** removed → use `QRegularExpression` (8 files)
3. **`QAction`** moved from `QtWidgets` to `QtGui`
4. **`QFontDatabase`** static-only API (no instances)
5. **`QProcess::start()`** overload removed → use `QProcess::startCommand()` or explicit args
6. **`qHash`** return type → `size_t` instead of `uint`
7. **`QKeyCombination`** replaces `int` key+modifier combos
8. **Signal overloads** removed (e.g. `QComboBox::currentIndexChanged(QString)`, `QSpinBox::valueChanged(QString)`)
9. **`QtConcurrent::run()`** signature changed
10. **`QFuture`/`QMutex`** API changes
11. **`SIGNAL()`/`SLOT()` macros** — still work but new-style connections preferred
12. **CMake targets** renamed: `Qt5::` → `Qt6::`, `KF5Archive` → `KF6Archive`
13. **`QOpenGLWidget`** moved to `OpenGLWidgets` module
14. **QML/JS engine** changes (QQmlEngine, property bindings)

---


# Setup the work env : 
On the host:
  xhost +local:docker

  Start the container:
  docker run -it --rm -v /home/imuthi/scopy:/home/runner/scopy -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY --net=host scopy-qt6-dev bash

  Inside the container (setup):
  git config --global --add safe.directory /home/runner/scopy
  pip3 install --break-system-packages aqtinstall
  python3 -m aqt install-qt --outputdir /opt/Qt linux desktop 6.7.0 -m qt3d qtscxml
  apt-get update && apt-get install -y libxcb-cursor0 libxcb-icccm4 libxcb-keysyms1 libxcb-shape0

  Build:
  cd /home/runner/scopy
  cmake -B build-qt6 -DCMAKE_PREFIX_PATH=/opt/Qt/6.7.0/gcc_64 -DENABLE_ALL_PACKAGES=ON
  cmake --build build-qt6 -j$(nproc)

  Run Scopy:
  ./build-qt6/scopy


## Subtask 1: Qt6 Docker Dev Environment

**Goal:** Docker image with Qt6 + all deps. Host Qt5 untouched.

**Status: COMPLETE** — `qmake6 --version` confirms Qt 6.7.0 in container.

### What existed
- `ci/ubuntu/docker_ubuntu/Dockerfile.qt6` — Ubuntu 24.04, runs `ubuntu_build_process_qt6.sh`
- `ci/ubuntu/ubuntu_build_process_qt6.sh` — Qt 6.7.0 via aqtinstall, KF6 branches, KDDock 2.2

### Fixes applied
1. `.dockerignore` only whitelisted the Qt5 script. Added Qt6 script:
```
# ci/ubuntu/.dockerignore
*
!ubuntu_build_process.sh
!ubuntu_build_process_qt6.sh    # ← added
```

2. ECM and KArchive repos don't have `kf6` branches — Qt6/KF6 code is on `master`:
```bash
# ci/ubuntu/ubuntu_build_process_qt6.sh
ECM_BRANCH=kf6       →  ECM_BRANCH=master
KARCHIVE_BRANCH=kf6  →  KARCHIVE_BRANCH=master
```

3. KDDockWidgets 2.2 defaults to Qt5 frontend — needs explicit Qt6 flag:
```bash
# ci/ubuntu/ubuntu_build_process_qt6.sh  build_kddock()
CURRENT_BUILD_CMAKE_OPTS=""  →  CURRENT_BUILD_CMAKE_OPTS="-DKDDockWidgets_QT6=ON"
```

4. karchive `master` requires Qt >= 6.8.0, but we use Qt 6.7.0. Use version-matched tags:
```bash
# ci/ubuntu/ubuntu_build_process_qt6.sh
ECM_BRANCH=master      →  ECM_BRANCH=v6.7.0
KARCHIVE_BRANCH=master →  KARCHIVE_BRANCH=v6.7.0
```

5. karchive v6.7.0 needs compression dev packages not in the Qt5 package list:
```bash
# ci/ubuntu/ubuntu_build_process_qt6.sh  install_packages()
# Added: libzstd-dev libbz2-dev liblzma-dev
```

### Build commands
```bash
cd /home/imuthi/scopy/ci/ubuntu
docker build -f docker_ubuntu/Dockerfile.qt6 -t scopy-qt6-dev .

# Verify
docker run --rm scopy-qt6-dev /opt/Qt/6.7.0/gcc_64/bin/qmake6 --version

# Dev shell with source mounted
docker run -it --rm \
  -v /home/imuthi/scopy:/home/runner/scopy \
  scopy-qt6-dev bash
```

---

## Subtask 2: CMake Build System Migration

**Goal:** `cmake -B build-qt6` completes inside Docker container.

**Status: COMPLETE** — cmake configures successfully with Qt6.

### Changes made (8 files):
1. `CMakeLists.txt:65` — `Qt5` → `Qt6` in find_package
2. `CMakeLists.txt:75-80` — Removed Qt5 version check, simplified to status message
3. `CMakeLists.txt:241-258` — `KF5Archive` → `KF6Archive` (find + link)
4. `cmake/Modules/ScopyTest.cmake:30,37` — `Qt5` → `Qt${QT_VERSION_MAJOR}`
5. `packages/imu/plugins/imuanalyzer/CMakeLists.txt:115-121` — `Qt5::*` → `Qt${QT_VERSION_MAJOR}::*`
6. `packages/generic-plugins/plugins/regmap/CMakeLists.txt:130` — `Qt5::Test` → `Qt${QT_VERSION_MAJOR}::Test`
7. `pkg-manager/CMakeLists.txt:67-70` — `KF5Archive` → `KF6Archive`
8. `gui/CMakeLists.txt:69` + `m2k-gui/CMakeLists.txt:52` — `QWT_QT_VERSION qt5` → `qt6`
9. `common/include/common/common.h:28-33` — Removed Qt 5.14 compat shim

### Additional fix: Qt 3D module missing
aqtinstall doesn't include Qt 3D by default (needed by imu plugin). Added `-m qt3d`:
```bash
# ci/ubuntu/ubuntu_build_process_qt6.sh  install_qt()
aqt install-qt ... 6.7.0          →  aqt install-qt ... 6.7.0 -m qt3d
```

### Note: git safe.directory
Before running cmake inside container, must run:
```bash
git config --global --add safe.directory /home/runner/scopy
```
(Repo is mounted from host with different owner than container root)

### Verify inside Docker:
```bash
cd /home/runner/scopy
cmake -B build-qt6 -DCMAKE_PREFIX_PATH=/opt/Qt/6.7.0/gcc_64 -DENABLE_ALL_PACKAGES=ON
```

---

## Subtask 3: setMargin() Mass Replacement

**Status: COMPLETE** — 397 occurrences replaced across 165 files.

### Command used:
```bash
find /home/imuthi/scopy \( -path '*/build*' -o -path '*/.git' \) -prune -o \
  \( -name '*.cpp' -o -name '*.h' \) -print | \
  xargs sed -i -E 's/setMargin\(([^)]+)\)/setContentsMargins(\1, \1, \1, \1)/g'
```
Note: Must exclude build dirs (root-owned from Docker cmake run).

---

## Subtask 4: QRegExp → QRegularExpression

**Status: COMPLETE** — 8 files migrated, zero QRegExp references remaining.

### Key API mappings used:
- `QRegExp` → `QRegularExpression`, `QRegExpValidator` → `QRegularExpressionValidator`
- `rx.indexIn(text)` → `text.contains(rx)` or `rx.match(text)`
- `rx.cap(N)` → `match.captured(N)`
- `setFilterRegExp(QRegExp(s, FixedString))` → `setFilterFixedString(s)`

---

## Subtask 5: Qt6 API Fixes — Remaining Compile Errors

**Status: COMPLETE** — Build succeeds at 100% with zero errors (22 build fixes applied).

### Remaining deprecation warnings (non-blocking, for later cleanup):
- `QVariant::type()` → `typeId()` in `logicanalyzer_api.cpp`
- `QMetaType::typeName(int)` deprecated in `logicanalyzer_api.cpp`
- `Qt::AA_UseHighDpiPixmaps` no-op in Qt6 — remove from `main.cpp:119`
- `QDropEvent::pos()` → `position().toPoint()` in basemenu/basemenuitem
- `QFutureWatcher::setPaused/isPaused` → `setSuspended/isSuspended` in iiomanager
- `QtConcurrent::run` nodiscard warnings in M2K oscilloscope

### Runtime: X11 display from Docker
```bash
# On host:
xhost +local:docker

# Run container:
docker run -it --rm -v /home/imuthi/scopy:/home/runner/scopy -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY --net=host scopy-qt6-dev bash

# Inside container:
cd /home/runner/scopy/build-qt6 && ./scopy
```
Additional xcb packages needed for Qt6 runtime (added to build script):
```bash
libxcb-cursor0 libxcb-icccm4 libxcb-keysyms1 libxcb-shape0
```

### Category A: QOpenGLWidget module
Qt6 moved QOpenGLWidget to separate `OpenGLWidgets` module.
- `core/CMakeLists.txt`: added `OpenGLWidgets` to SCOPY_QT_COMPONENTS
- `imuanalyzer/CMakeLists.txt`: added `OpenGLWidgets` to find_package components

### Category B: enterEvent(QEvent*) → enterEvent(QEnterEvent*)
Qt6 changed parameter type. Used sed to replace across 18 files:
```bash
sed -i 's/enterEvent(QEvent \*/enterEvent(QEnterEvent */g'
```

### Category C: QVariant::type() → typeId()
`QVariant::type()` and `QVariant::Type` removed in Qt6.
- `pkggridwidget.cpp`: `var.type() == QVariant::String` → `var.typeId() == QMetaType::QString`
- `pkgpreviewpage.cpp`: `variant.type() == QVariant::List` → `variant.typeId() == QMetaType::QVariantList`
- `enum.cpp`: `v.first.type() == QVariant::Double` → `v.first.typeId() == QMetaType::Double`
- `int.cpp` (3 places): `QVariant::Type type = variant.type()` → `int type = variant.typeId()`

### Subtask 5.1: Iterative build error fixes

**Build error 1**: `versionchecker.cpp:119` — Qt6 `QList` constructor is explicit
```cpp
reply->ignoreSslErrors({QSslError::NoPeerCertificate});
→  reply->ignoreSslErrors(QList<QSslError>{QSslError(QSslError::NoPeerCertificate)});
```

**Mass fix**: `qAsConst()` deprecated → `std::as_const()` (239 occurrences, 111 files)
```bash
sed -i 's/qAsConst(/std::as_const(/g'
```

**Build error 2**: `gui/include/gui/utils.h:81` — `QStyleOption::init()` removed in Qt6
```cpp
opt.init(this);  →  opt.initFrom(this);
```
Single macro used by ~12 widget classes.

**Build error 3**: `QSignalTransition` moved to `StateMachine` module in Qt6
```cmake
# gui/CMakeLists.txt
find_package(Qt... COMPONENTS Widgets Xml Svg REQUIRED)
→ find_package(Qt... COMPONENTS Widgets Xml Svg StateMachine REQUIRED)
# Also added Qt::StateMachine to target_link_libraries
```
StateMachine module not in base aqtinstall — added `qtscxml` to install_qt():
```bash
aqt install-qt ... 6.7.0 -m qt3d qtscxml
```

**Build error 4**: Qwt includes `<qopenglwidget.h>` which is in `OpenGLWidgets` module
```cmake
# gui/CMakeLists.txt — added OpenGLWidgets to find_package and target_link_libraries
```

**Build error 5**: `smallOnOffSwitch.cpp:177` — pre-existing bug: `leaveEvent` called `enterEvent`
```cpp
QCheckBox::enterEvent(event);  →  QCheckBox::leaveEvent(event);
```

**Build error 6**: `tutorialbuilder.h` — `QFile` forward-declared in Qt6 headers, needs explicit include
```cpp
#include <QFile>  // added
```

**Build error 7**: `menuwidget.cpp:87` — `QString(int)` ambiguous in Qt6 (int→QChar has multiple candidates)
```cpp
QString(uuid)  →  QString::number(uuid)
```

**Build error 9**: `QFont::setWeight(int)` → `QFont::Weight` enum in Qt6
```bash
# Qt5's 75 = Bold. 5 occurrences in m2k display plots
sed -i 's/setWeight(75)/setWeight(QFont::Bold)/g'
```

**Build error 10**: `logdatatofile.cpp:61` — QString* compared to QString without dereference
```cpp
if(currentFileHeader != fileHeader)  →  if(*currentFileHeader != fileHeader)
```

**Build error 11**: `logdatatofile.cpp:239` — `splitRef()` removed in Qt6
```cpp
splitRef("/")  →  split("/")
```

**Build error 12**: `scopymainwindow_api.cpp:367` — `QString != NULL` ambiguous in Qt6
```cpp
if(prefName != NULL)  →  if(!prefName.isNull())
```

**Build error 13**: `QtConcurrent::run()` signature changed in Qt6 (17 occurrences)
```cpp
# Qt5: QtConcurrent::run(obj, &Class::method, args...)
# Qt6: QtConcurrent::run(&Class::method, obj, args...)
# Fixed with sed — swap first two arguments
```

**Build error 18**: More `QString(int/bool)` ambiguous conversions — use `QString::number()`
- `fftplotmanagersettings.cpp` (3), `timeplotmanagersettings.cpp` (3), `jsonformatedelement.cpp` (3 bools)

**Build error 20**: `qRegisterMetaTypeStreamOperators` removed in Qt6 (auto-registered now)
- `logicanalyzer_api.h`: removed 7 calls, kept `qRegisterMetaType` calls
- `pattern_generator_api.h`: removed 4 calls, kept `qRegisterMetaType` calls

**Build error 19**: `filterRegExp()` removed → `filterRegularExpression()`
- `iiosortfilterproxymodel.cpp:35`

**Build error 21**: `endl` not in scope for QTextStream — use `Qt::endl`
```bash
sed -i 's/<< endl/<< Qt::endl/g'
```

**Build error 22**: `powercontrol.ui` `setNum(int)` ambiguous (QLabel overload)
- Removed connection from `.ui`, added manual connect with `qOverload<int>(&QLabel::setNum)` in `power_controller.cpp`

**Build error 16**: `scenerenderer.hpp:47` — Qt3D classes moved in Qt6
```cpp
Qt3DRender/QGeometry   →  Qt3DCore/QGeometry
Qt3DRender/QAttribute  →  Qt3DCore/QAttribute
```

**Build error 17**: `midRef()`/`leftRef()`/`rightRef()` removed in Qt6
```bash
sed -i 's/\.midRef(/\.mid(/g; s/\.leftRef(/\.left(/g; s/\.rightRef(/\.right(/g'
```

**Build error 15**: `fmcomms5calibration.cpp:73` — Qt6 template deduction fails for member function pointer without args
```cpp
QtConcurrent::run(&Class::method, this);  →  QtConcurrent::run([this]() { method(); });
```

**Build error 14**: `extprocplotinfo.h:88` — `id < 0` where id is QString
```cpp
id < 0  →  id.toInt() < 0
```

**Build error 8**: Implicit `QString→QFileInfo` conversion removed in Qt6
```cpp
# pkgmanager.cpp:151
return pkgPath;  →  return QFileInfo(pkgPath);
# pkgutil.cpp:234
files.append(it.next());  →  files.append(QFileInfo(it.next()));
```

---

## Subtask 6: Signal Overload Cleanup

**Status: COMPLETE** — Build 100%, zero errors.

### Signal overload fixes:
1. `patterns.cpp:1336-1338,3621` — `SIGNAL(activated(QString))` → `SIGNAL(activated(int))` (4 places)
2. `spectrum_analyzer.hpp/.cpp` — `on_*_currentIndexChanged(const QString&)` → `(int idx)` (3 slots)
   - Body updated to get text via `itemText(idx)`

### Deprecation warning fixes:
3. `main.cpp:119` — Removed `Qt::AA_UseHighDpiPixmaps` (no-op in Qt6)
4. `basemenu.cpp:126`, `basemenuitem.cpp:184` — `event->pos()` → `event->position().toPoint()`
5. `iiomanager.cpp:66,77` — `setPaused/isPaused` → `setSuspended/isSuspending`
6. `logicanalyzer_api.cpp:253` — `p_val.type()` → `p_val.typeId()`
7. `logicanalyzer_api.cpp:349-385` — `QMetaType::typeName(X)` → `QMetaType(X).name()`

---

## Subtask 7: Threading & Concurrency + Deprecation Warning Fixes

**Status: COMPLETE** — Verified by 27/27 C++ unit tests passing (see `porting_debug/02_functionality_issues.md`).

### Fixes applied:
1. `qAsConst` → `std::as_const` in `.cc` files (6 occurrences missed by earlier sed)
2. `QSignalBlocker` temporaries → assigned to named variables (3 places in spectrum_analyzer.cpp — these were bugs)
3. `runInHwThreadPool` macro — added `(void)` cast to suppress nodiscard
4. Fire-and-forget `QtConcurrent::run` — added `(void)` cast (4 places)
5. `QTranslator::load` — added return value check with warning
6. `QEnterEvent::pos()` → `position().toPoint()` in axishandle.cpp
7. `QMouseEvent::localPos()` → `position()` in dropdown_switch_list.cpp
8. `QVariant::type()` → `typeId()` in decoder.cpp and enum.cpp (3 casts)
9. `commandqueue.cpp` — added `(void)` to QtConcurrent::run

---

## Subtask 8: SIGNAL()/SLOT() modernization — core & GUI

### Batch: GUI source files (2026-03-18)

Converted old-style `SIGNAL()`/`SLOT()` connections to new-style `&Class::method` syntax:

1. **`gui/src/spinbox_a.cpp`** — 6 connects: `QComboBox::currentIndexChanged`, `QLineEdit::editingFinished`, `QPushButton::clicked` (x2), `SpinBoxA::valueChanged`→`CompletionCircle::setValueDouble`, `CompletionCircle::toggled`→`SpinBoxA::setFineMode`
2. **`gui/src/symbol.cpp`** — 10 connects/disconnects: `QwtScaleWidget::scaleDivChanged` (6 in constructor/setMobileAxis/setFixedAxis), `Symbol::positionChanged`/`pixelPositionChanged` forwarding via `qOverload` in VertDebugSymbol and HorizDebugSymbol constructors. Added `#include <qwt_scale_widget.h>`.
3. **`gui/src/symbol_controller.cpp`** — 4 connects/disconnects: **NOT converted** because `attachSymbol`/`detachSymbol` take `Symbol*` but connect to `positionChanged(double)` which only exists on derived classes. Old-style string-based connection is required for runtime signal resolution.
4. **`gui/src/menu_anim.cpp`** — 2 connects: `CustomAnimation::finished`→`MenuAnim::openAnimFinished`/`closeAnimFinished`
5. **`gui/src/widgets/semiexclusivebuttongroup.cpp`** — 1 connect: `QButtonGroup::buttonClicked`→`SemiExclusiveButtonGroup::buttonClicked`
6. **`gui/src/widgets/menucontrolbutton.cpp`** — 1 connect: `QCheckBox::toggled`→`QWidget::setVisible`

### Batch 2: Core source files (2026-03-18)

**Files converted:**
1. **`core/src/scanbuttoncontroller.cpp`** — 1 connect: `QCheckBox::toggled`→`ScanButtonController::enableScan`
2. **`core/src/devicebrowser.cpp`** — 3 connects: `QPushButton::clicked`→`DeviceBrowser::forwardRequestDeviceWithDirection`, `DeviceBrowser::requestDevice`→`DeviceBrowser::updateSelectedDeviceIdx`
3. **`core/src/scopyaboutpage.cpp`** — 3 connects: `QPushButton::clicked`→`QTextBrowser::home/backward/forward`
4. **`core/src/iiotabwidget.cpp`** — 2 connects: `QPushButton::clicked`→`IioTabWidget::futureScan/futureSerialScan`
5. **`core/src/scopyhomepage.cpp`** — 5 signal forwards: `HomepageControls::goLeft/goRight`→`DeviceBrowser::prevDevice/nextDevice`, `DeviceBrowser::requestDevice`→`InfoPageStack::slideInKey` and `ScopyHomePage::requestDevice`, `ScopyHomePage::deviceAddedToUi`→`ScopyHomeAddPage::deviceAddedToUi`. Added includes for `infopagestack.h` and `homepage_controls.h`. Used lambda for `requestDevice` signal-to-signal with param count mismatch (2→1).
6. **`core/src/scopymainwindow.cpp`** — ~19 connects/disconnects: `BrowseMenu::requestLoad/requestSave` (used `QOverload<>` for overloaded `load()`/`save()`), `ScannedIIOContextCollector::foundDevice/lostDevice`→`DeviceManager::createDevice/removeDevice`, `ScopyHomePage::requestDevice`→`requestTools`, `DeviceManager::deviceAdded/deviceRemoveStarted/deviceConnecting/deviceConnected/deviceDisconnected/requestDevice`→various slots on `scc`, `hp`, `m_toolMenuManager`, `PluginManager::startLoadPlugin`↔`ScopySplashscreen::setMessage`, `Preferences::preferenceChanged`→`handlePreferences`
7. **`core/src/devicemanager.cpp`** — 3 connects + 9 disconnects in `connectDeviceToManager`/`disconnectDeviceFromManager`: `DeviceImpl::requestedRestart/toolListChanged/requestTool`. Used `QOverload<>::of(&DeviceManager::restartDevice)` for overloaded slot. First 6 disconnects used `disconnect(d, &Signal, nullptr, nullptr)` pattern.
8. **`core/src/deviceimpl.cpp`** — All 18 old-style SIGNAL/SLOT uses involve `dynamic_cast<QObject*>(p)` — left unchanged as required.

## Subtask 9: SIGNAL()/SLOT() modernization — active plugins

### Batch 1: generic-plugins (dac + adc) — 2026-03-18

Converted 24 old-style SIGNAL()/SLOT() connections to new-style `&Class::member` syntax across 6 files:

1. **`packages/generic-plugins/plugins/dac/src/bufferdacaddon.cpp`** — 3 connects: `DataBuffer::loadFinished/loadFailed/dataUpdated`→`BufferDacAddon::onLoadFinished/onLoadFailed/dataReload` (kept `Qt::QueuedConnection`)
2. **`packages/generic-plugins/plugins/adc/src/time/grtimesinkcomponent.cpp`** — 5 connects + 5 disconnects: `GRTopBlock::builtSignalPaths/teardownSignalPaths/started/aboutToStop/forceStop`→`GRTimeSinkComponent::connectSignalPaths/tearDownSignalPaths/ready/finish/requestForceStop`
3. **`packages/generic-plugins/plugins/adc/src/freq/grfftsinkcomponent.cpp`** — 5 connects + 5 disconnects: same pattern as #2 but for `GRFFTSinkComponent`
4. **`packages/generic-plugins/plugins/adc/src/adctimeinstrumentcontroller.cpp`** — 5 connects: `GRTimeSinkComponent::arm/disarm/ready/finish/requestForceStop`→`ADCInstrumentController::onStart/onStop/startUpdates/stopUpdates/stop`
5. **`packages/generic-plugins/plugins/adc/src/adcfftinstrumentcontroller.cpp`** — 5 connects: same pattern as #4 but sender is `GRFFTSinkComponent`
6. **`packages/generic-plugins/plugins/adc/src/adcinstrumentcontroller.cpp`** — 1 connect: `Preferences::preferenceChanged`→`ADCInstrumentController::handlePreferences`

## Subtask 10: SIGNAL()/SLOT() Modernization — M2K Legacy

**Status: SKIPPED** — M2K plugin will be refactored soon with Qt6 best practices applied directly.

### Why skipped
The M2K plugin (`packages/m2k/`) contains ~375 old-style SIGNAL/SLOT connections across 39 files. An initial conversion attempt was made but reverted because:
1. The M2K plugin is scheduled for a near-term refactor
2. New Qt6 best practices (new-style connects, modern signal patterns) will be applied during that refactor
3. The conversion exposed many edge cases (protected slots, overloaded Qwt signals, polymorphic types) that required reverting ~10% of conversions to old-style anyway

### Impact of keeping old-style SIGNAL/SLOT in Qt6
- **Functional**: Old-style `SIGNAL()`/`SLOT()` macros **still work in Qt6**. They are string-based and resolved at runtime. No compile errors, no runtime crashes.
- **Performance**: Negligible. String-based lookup is slightly slower than compile-time resolution, but the difference is unmeasurable for signal/slot connections.
- **Safety**: The main downside is that typos or signature mismatches in old-style connections fail **silently at runtime** (the connection just doesn't fire) rather than producing a compile error. This is an existing risk, not a new one introduced by Qt6.
- **Deprecation**: Old-style macros are NOT deprecated in Qt6. They are fully supported and will continue to work. Qt documentation recommends new-style for new code but does not mandate migration.

### Recommendation
When the M2K refactor happens, convert directly to new-style `&Class::signal` syntax. No action needed now.

---

## Subtask 11: QML/JS Engine & Final Runtime Validation

**Status: COMPLETE** — All runtime checklist items verified (see `porting_debug/02_functionality_issues.md`).

### Analysis
QJSEngine/QJSValue API is almost entirely unchanged between Qt5 and Qt6. All 33 files using these APIs are compatible. No breaking API changes detected.

### Code fix applied:
`pluginbase/src/scopyjs.cpp:74` — Non-standard `for (each in o)` JS syntax → standard `for (var each in o)`

### M2K include paths (skipped):
8 M2K files use `#include <QtQml/QJSEngine>` — both forms work in Qt6. Deferred to M2K refactor.

### Runtime validation checklist:
- [x] `QT_QPA_PLATFORM=offscreen ctest --output-on-failure` passes — 27/27 tests passed
- [x] Scopy launches with X11 display — confirmed via Docker + xhost
- [x] JS scripting tool evaluates `print("hello")` and `1+1` correctly — pluginLoadTests.js passed
- [x] Plugins load in the UI — all plugins detected and loaded, no console errors

---

## Subtask 12: CI Pipeline Update — PLANNED (not yet implemented)

**Strategy:** Qt6 CI on `qt6_migration` branch only. Main branch stays Qt5.

### Files to create/modify:
| File | Action |
|------|--------|
| `ci/ubuntu/create_docker_image.sh` | Add `ubuntu24_qt6()` using `Dockerfile.qt6` |
| `.github/workflows/ci-qt6.yml` | Qt6 orchestrator, triggers on `qt6_migration` branch |
| `.github/workflows/ubuntubuild-qt6.yml` | Ubuntu 24 Qt6 build (`scopy2-ubuntu24-qt6` image) |
| `.github/workflows/appimage-x86_64-qt6.yml` | Stub — needs Qt6 AppImage Docker image |
| `.github/workflows/mingwbuild-qt6.yml` | Stub — needs Qt6 MinGW Docker image |
| `.github/workflows/linuxflatpakbuild-qt6.yml` | Stub — needs `org.kde.Sdk//6.x` |

### Platform readiness:
- **Ubuntu**: Ready (Dockerfile.qt6 + build script exist)
- **AppImage x86_64**: Needs Qt6 Docker image
- **AppImage ARM**: Needs Qt6 cross-compilation sysroot
- **Windows/MinGW**: Needs Qt6 MSYS2 toolchain
- **Flatpak**: Needs org.kde.Sdk//6.x runtime

### Key difference from Qt5 Ubuntu workflow:
```yaml
container: cristianbindea/scopy2-ubuntu24-qt6:${{ inputs.docker_tag }}
run: ubuntu_build_process_qt6.sh build_scopy  # instead of ubuntu_build_process.sh
```

---

## Subtask 13: Remaining Deprecation Fixes — QMouseEvent::pos() + Redundant setContentsMargins

**Status: PENDING**

### Fix A: QMouseEvent::pos() → position().toPoint() in 8 gui/ files

`QMouseEvent::pos()` is deprecated in Qt6; use `position().toPoint()` instead.
These were flagged in `porting_debug/01_code_review_issues.md` (Issue 2) but not yet applied.

**Affected files:**
- `gui/src/basemenuitem.cpp`
- `gui/src/buffer_previewer.cpp`
- `gui/src/plotzoomer.cpp`
- `gui/src/symbol_controller.cpp`
- `gui/src/axishandle.cpp` (partially done for `QEnterEvent` in Subtask 7, check remaining mouse events)
- `gui/src/completion_circle.cpp`
- `gui/src/hoverwidget.cpp`
- `gui/src/basictracker.cpp`

**Fix command (run on each file individually after review):**
```bash
sed -i 's/event->pos()/event->position().toPoint()/g' <file>
```

**Note:** `symbol_controller.cpp` uses old-style string connections intentionally (runtime polymorphic signal resolution) — verify the pos() call there is a QMouseEvent, not a QDropEvent (already fixed).

### Fix B: Redundant setContentsMargins in channel_manager.cpp

The sed migration (Subtask 3) introduced duplicate consecutive `setContentsMargins` calls at lines 80-81 and 244-245 in:
- `packages/m2k/plugins/m2k/m2k-gui/src/channel_manager.cpp`

Fix: manually remove the second redundant call at each location (keep the pre-existing one).

---

## Subtask 14: Clazy Qt6 Verification Run

**Status: COMPLETE** — 2026-03-19. Zero findings across 529 files. Migration verified clean.

### Goal
Run Clazy's 5 dedicated Qt6 porting checks against the Scopy codebase as a final verification pass using `clazy-standalone` + the existing `build-qt6/compile_commands.json`. No rebuild needed. Expected result: zero findings = migration confirmed complete.

### Approach: clazy-standalone with existing compile_commands.json

`build-qt6/compile_commands.json` already exists (2.1 MB, 3,145 entries). `clazy-standalone` can consume it directly — no recompilation required.

**5 checks to run:**
| Check | What it catches |
|---|---|
| `qt6-deprecated-api-fixes` | Deprecated Qt5 APIs still in use |
| `qt6-header-fixes` | Wrong `#include` paths for Qt6 |
| `qt6-qhash-signature` | `qHash()` returning `uint` instead of `size_t` |
| `qt6-fwd-fixes` | Bad forward declarations |
| `missing-qobject-macro` | Classes missing `Q_OBJECT` |

M2K package is excluded (intentionally not ported, tracked separately).

### How to run (inside Docker container)

Use the ready-made script: `porting_process/run_clazy.sh`

```bash
# Step 0 (on host): start Docker
xhost +local:docker
docker run -it --rm \
  -v /home/imuthi/scopy:/home/runner/scopy \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -e DISPLAY=$DISPLAY --net=host scopy-qt6-dev bash

# Step 1 (inside container): one-time setup
git config --global --add safe.directory /home/runner/scopy
apt-get update && apt-get install -y clazy

# Step 2: run the script
bash /home/runner/scopy/porting_process/run_clazy.sh
```

Output files (written to `porting_process/`):
- `clazy_qt6_report.txt` — full clazy output
- `clazy_qt6_findings.txt` — filtered warnings only (the one that matters)

### Interpreting results
- **0 lines in `clazy_qt6_findings.txt`** — migration verified complete → mark COMPLETE
- **Lines present** — each line is a missed porting item; fix the code and re-run

### Reference
Official documentation: https://doc.qt.io/qt-6/porting-to-qt6-using-clazy.html

---

## Gap Analysis vs Official Qt Documentation

*Completed 2026-03-19. Compares this document against the official porting guide summary in `porting_logic.md`.*

### Items verified clean — no fix needed (searched codebase, zero occurrences)

| Qt6 Breaking Change | Search result |
|---|---|
| `QStringRef` usage | Zero occurrences ✅ |
| `QRecursiveMutex` (no longer inherits QMutex) | Zero occurrences ✅ |
| `QMutex::Recursive` constructor arg (removed) | Zero occurrences ✅ |
| `QVariant` as `QMap`/`QHash` key | Zero occurrences ✅ |
| `QQmlEngine::setContextProperty()` (deprecated) | Zero occurrences ✅ |
| `QEvent` copy constructor/assignment (now protected) | No custom QEvent subclasses ✅ |

### Items listed in header but no explicit fix subtask — verified correct

| Item | How it was already correct |
|---|---|
| `QAction` moved QtWidgets→QtGui | CMake links `Qt::Widgets` which transitively provides `Qt::Gui`; `#include <QAction>` resolves correctly ✅ |
| `QFontDatabase` static-only | `gui/src/style.cpp` uses `QFontDatabase::addApplicationFont()` — static call, no instance created ✅ |
| `QProcess::start()` single-string overload removed | All 5 usage sites use 2-arg form (`program`, `QStringList`) or `startDetached()` ✅ |
| `QKeyCombination` replaces int key+modifier | Code uses `QKeySequence::Copy` enum with `matches()` — correct modern pattern ✅ |
| High-DPI `AA_UseHighDpiPixmaps` removed | Removed in Subtask 6; Qt6 `PassThrough` rounding default used — no explicit policy needed ✅ |
| `QQmlEngine` usage | Only in `packages/m2k/.../src/old/`; uses `QQmlEngine::setObjectOwnership()` (static, Qt6-compatible) ✅ |

### Known risks (not breaking, but flagged for awareness)

**disconnect() with nullptr slot (3 files):**
In Qt6, `disconnect(sender, &Signal, receiver, nullptr)` has subtly changed semantics for lambda connections. Lambda connections cannot be disconnected by receiver+nullptr — they require storing the `QMetaObject::Connection` handle.

Affected files:
- `gui/src/widgets/plotlegend.cpp:115-117`
- `gui/src/widgets/menuplotchannelcurvestylecontrol.cpp:123`
- `gui/src/cursorcontroller.cpp:193-194, 241-244`

**Action:** Verify these disconnections work correctly at runtime. If signals fire unexpectedly after disconnect, refactor to store and use `QMetaObject::Connection` handles.

**int vs qsizetype for list/vector sizes (~175 instances):**
Qt6 changed `QList::size()` / `count()` return type from `int` to `qsizetype`. Assigning to `int` is still functionally correct on all 64-bit platforms but generates `-Wconversion` warnings if that flag is enabled.

**Decision:** Accept as-is. Address if `-Wconversion` is ever promoted to an error in the build.

### Optional tooling enhancements (not implemented)

**`QT_DISABLE_DEPRECATED_UP_TO` macro:**
The official Qt porting guide recommends adding this to CMake to catch any future deprecated API usage at compile time:
```cmake
# In root CMakeLists.txt — catches Qt 5.15 deprecations
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)
```
Not currently added. Optional — add when the codebase is fully warning-clean.

---

## Known Issue: Q_OBJECT + Style::setBackgroundColor() Interaction

**Status: DOCUMENTED** — 2026-03-24. Root cause identified and workaround applied.

### Problem

Adding `Q_OBJECT` to any `QWidget` subclass that uses `Style::setBackgroundColor()` (with default `extend_to_children=false`) will break its background styling, causing visual regressions (wrong background color, altered margins/spacing).

### Root cause

`Style::setBackgroundColor()` at `gui/src/style.cpp:302-309` generates a hardcoded CSS class selector:

```cpp
widget->setStyleSheet(widget->styleSheet() + "\n.QWidget { background-color: " + color + "; }");
```

The `.QWidget` selector only matches widgets whose `metaObject()->className()` returns `"QWidget"`:

- **Without `Q_OBJECT`**: `className()` returns `"QWidget"` (the base class) → selector **matches** → background applied correctly.
- **With `Q_OBJECT`**: `className()` returns the actual class name (e.g. `"ToolTemplate"`) → selector **does not match** → background silently not applied.

### Symptom

When `Q_OBJECT` was added to `ToolTemplate` (commit `220cfc694`), the ToolTemplate lost its `background_subtle` background color. This caused the widget to inherit different styles, resulting in visible margin/spacing changes compared to the correct appearance on the `qt6_port` branch.

### Affected code

- `Style::setBackgroundColor()` — `gui/src/style.cpp:302-309` (the root cause)
- `menu_anim.cpp:55` — also hardcodes `.QWidget` in a stylesheet string
- Any future QWidget subclass that calls `Style::setBackgroundColor()` with `extend_to_children=false` (the default)

### Workaround applied

Removed `Q_OBJECT` from `ToolTemplate` since it doesn't need signals/slots. The visual regression is resolved.

### Future guidance

Before adding `Q_OBJECT` to a QWidget subclass, check if it or its children use `Style::setBackgroundColor()`. If so, either:
1. Skip `Q_OBJECT` if the class doesn't need signals/slots
2. Fix `Style::setBackgroundColor()` to use `widget->metaObject()->className()` instead of hardcoded `"QWidget"`


---

## Known Issue: Qt::UniqueConnection + Lambda Crash

**Status: FIXED** — 2026-03-24. Two occurrences fixed in regmap.

### Problem

In Qt6, using `Qt::UniqueConnection` with a **lambda** causes an assert failure and crash:
```
ASSERT failure in QObject::connect: "Unique connection requires the slot to be a pointer to a member function of a QObject subclass"
```

Qt5 silently ignored the `Qt::UniqueConnection` flag for lambdas. Qt6 enforces it with a hard assert.

**Qt6 docs**: https://doc.qt.io/qt-6/qobject.html#connect-4

### Fix applied

`packages/generic-plugins/plugins/regmap/src/registermaptool.cpp` — Extracted lambda bodies into private slots (`startActiveDeviceTutorial()`, `startActiveDeviceSimpleTutorial()`) so `Qt::UniqueConnection` works with member function pointers.

### Future guidance

Search for `Qt::UniqueConnection` in any code being ported. If the slot argument is a lambda, either:
1. Extract the lambda into a named member slot (preferred — preserves `UniqueConnection` semantics)
2. Remove `Qt::UniqueConnection` and manage connection lifetime manually via stored `QMetaObject::Connection` handles

---

# Known issue with style

Some svg files use : 

`````
     style="fill-opacity:1;stroke-width:1.413463;opacity:1"
```` 

Setting colors like this breaks the svg file and we can't apply the theme colors to it 

One solution is to add a solid color as "fill" in the svg file:


````````
  fill="#676769"
````````
