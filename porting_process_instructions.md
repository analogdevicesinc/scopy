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
 cmake -B build-qt6 -DCMAKE_PREFIX_PATH=/opt/Qt/6.7.0/gcc_64 -DENABLE_ALL_PACKAGES=ON -DCMAKE_BUILD_TYPE=Debug -DSCOPY_DEV_MODE=ON
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

**Status:** Changes applied, awaiting build verification.

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

**Status:** Code change applied, awaiting runtime verification.

### Analysis
QJSEngine/QJSValue API is almost entirely unchanged between Qt5 and Qt6. All 33 files using these APIs are compatible. No breaking API changes detected.

### Code fix applied:
`pluginbase/src/scopyjs.cpp:74` — Non-standard `for (each in o)` JS syntax → standard `for (var each in o)`

### M2K include paths (skipped):
8 M2K files use `#include <QtQml/QJSEngine>` — both forms work in Qt6. Deferred to M2K refactor.

### Runtime validation checklist:
- [ ] `QT_QPA_PLATFORM=offscreen ctest --output-on-failure` passes
- [ ] Scopy launches with X11 display
- [ ] JS scripting tool evaluates `print("hello")` and `1+1` correctly
- [ ] Plugins load in the UI

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
