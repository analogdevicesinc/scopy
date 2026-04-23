# Qt 5 → Qt 6 Official Porting Logic & Reference

This document summarizes the official Qt porting documentation relevant to the Scopy Qt6 migration.

---

## 1. Main Porting Guide

**Source:** https://doc.qt.io/qt-6/portingguide.html

**Golden rule:** Update to **Qt 5.15 first**, then migrate to Qt 6. Qt 5.15 already marks all APIs
that will be removed in Qt 6 as deprecated, minimizing surprises.

Enable compile-time deprecated-API warnings in your build:
```cmake
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)
```

### Seven key areas

| Area | Notes |
|---|---|
| Deprecated API removal | Catch all Qt 5.15 deprecations at compile time before switching |
| Module changes | ~25 modules removed or restructured (see §4) |
| Graphics testing | New rendering backend — test for visual regressions |
| High-DPI support | Default scale rounding: `Round` → `PassThrough` |
| Platform integration | Leverage native platform APIs |
| Tooling | Use Clazy for automated detection/fixing (see §2) |
| Build system | qmake → CMake (see §3) |

---

## 2. Clazy Porting Tool

**Source:** https://doc.qt.io/qt-6/porting-to-qt6-using-clazy.html

Clazy is a Clang compiler plugin that detects and auto-fixes Qt 5 → Qt 6 incompatibilities.

### Five dedicated Qt 6 checks

| Check | Purpose |
|---|---|
| `qt6-deprecated-api-fixes` | Replace deprecated Qt 5 APIs |
| `qt6-header-fixes` | Fix `#include` paths |
| `qt6-qhash-signature` | Update `qHash()` return type (`uint` → `size_t`) |
| `qt6-fwd-fixes` | Fix forward declarations |
| `missing-qobject-macro` | Detect missing `Q_OBJECT` |

### Workflow

```bash
# With CMake:
cmake -DCMAKE_CXX_COMPILER=clazy ...
export CLAZY_CHECKS="qt6-deprecated-api-fixes,qt6-header-fixes,qt6-qhash-signature,qt6-fwd-fixes"
export CLAZY_EXPORT_FIXES=ON
# Build → generates .yaml patch files
clang-apply-replacements <build-dir>
```

> **Important:** Run Clazy against Qt 5 code. The fixed code will only compile with Qt 6.
> Not all porting can be automated — manual review is always required.

---

## 3. Build System Migration (qmake → CMake)

**Source:** https://doc.qt.io/qt-6/qt6-buildsystem.html

Qt 6 uses **CMake** as its primary build system. qmake still works for applications but is
**required to be CMake** for custom plugins and libraries.

### Key differences from Qt 5 qmake

| Topic | Qt 5 (qmake) | Qt 6 (CMake) |
|---|---|---|
| Dependency resolution | pkg-config + compile-time tests | CMake package discovery |
| Config cache | `config.status` / `-redo` | `CMakeCache.txt` — delete to start fresh |
| Partial rebuilds | `make` in subdirectory | Must build targets from top-level directory |
| Cross-compilation | Host + target built together | Requires pre-installed matching Qt on host |
| Custom plugins/libs | qmake OK | Must use CMake |
| Qt targets | `Qt5::Core`, `Qt5::Widgets`, … | `Qt6::Core`, `Qt6::Widgets`, … |
| KDE Frameworks | `KF5Archive`, `KF5ECM` | `KF6Archive`, `KF6ECM` |

### Scopy-specific CMake migration done

- `CMakeLists.txt` — `Qt5` → `Qt6` in `find_package`
- `KF5Archive` → `KF6Archive` in root + `pkg-manager/CMakeLists.txt`
- `Qt5::*` → `Qt${QT_VERSION_MAJOR}::*` in package CMakeLists
- Added `OpenGLWidgets`, `StateMachine` modules (moved from base in Qt 6)
- Qwt: `QWT_QT_VERSION qt5` → `qt6`

---

## 4. What's New / Removed Modules in Qt 6.0

**Source:** https://doc.qt.io/qt-6/whatsnew60.html

### Removed from base (~25 modules)

| Category | Removed modules |
|---|---|
| Multimedia | Qt Multimedia, Qt Bluetooth, Qt NFC |
| Platform extras | Qt Android Extras, Qt Mac Extras, Qt Windows Extras, Qt X11 Extras |
| Dev tools | Qt Script, Qt Script Tools |
| Networking | Qt Serial Port, Qt WebSockets, Qt Positioning |
| Specialized | Qt Charts, Qt Data Visualization, Qt SCXML |

### Compatibility layers provided

- **Qt 5 Core Compatibility APIs** — backward-compat for essential legacy code
- **Qt 5 Compat: Graphical Effects** — transition layer for graphical effects

### New in Qt 6.0

- **Qt Shader Tools** — compute/graphics shader support
- **Qt StateMachine** — extracted from Qt Core into its own module
- Build system now requires **CMake + Ninja** (building Qt itself from source)

---

## 5. Qt Core Breaking Changes

**Source:** https://doc.qt.io/qt-6/qtcore-changes-qt6.html

### Containers

| Class | Change |
|---|---|
| `QHash` / `QMultiHash` / `QSet` | `qHash()` return type: `uint` → `size_t` |
| `QHash` | References to elements no longer stable during growth/removal |
| `QHash::insertMulti()` | Removed — use `QMultiHash` |
| `QVector` | Now a typedef for `QList`; unified implementation |
| `QList` / `QVector` | Size type: `int` → `qsizetype` |

> **Scopy status:** ~175 instances of `for(int i = 0; i < list.size(); i++)` remain. Accepted as warning-only (no functional impact on 64-bit). `QHash` element stability not an issue (no code mutates hash while holding references). See Gap Analysis in `porting_process_instructions.md`.

### Strings

| Old | New |
|---|---|
| `QStringRef` | Moved to Qt5Compat; use `QStringView` (pass by value) |
| `QString::splitRef()` | Removed; use `split()` |
| `QString::midRef()` / `leftRef()` / `rightRef()` | Removed; use `mid()` / `left()` / `right()` |
| `QTextStream::setCodec()` | Removed; use `setEncoding()` |
| `endl` (QTextStream) | Use `Qt::endl` |

### Type system

| Old | New |
|---|---|
| `QVariant::type()` | Use `typeId()` |
| `QVariant::Type` enum | Use `QMetaType::Type` |
| `QVariant` comparison operators | Removed (use explicit comparisons) |
| `QVariant` as `QMap` key | No longer supported |
| `QRegExp` | Moved to Qt5Compat; use `QRegularExpression` |
| `QRegExpValidator` | Use `QRegularExpressionValidator` |

### Regular expressions — API mapping

| Qt 5 (`QRegExp`) | Qt 6 (`QRegularExpression`) |
|---|---|
| `rx.indexIn(text)` | `text.contains(rx)` or `rx.match(text)` |
| `rx.cap(N)` | `match.captured(N)` |
| `setFilterRegExp(QRegExp(s, FixedString))` | `setFilterFixedString(s)` |
| `filterRegExp()` | `filterRegularExpression()` |

### Other notable changes

| Old | New |
|---|---|
| `QRecursiveMutex` inherits `QMutex` | No longer inherits |
| `QFuture` implicit conversions | Removed; use explicit methods |
| `QProcess::pid()` | `processId()` |
| `QEvent` copy constructor/assignment | Made protected; use `clone()` |
| `QCoreApplication::quit()` | Graceful shutdown (was immediate) |
| `qAsConst()` | `std::as_const()` |
| `qRegisterMetaTypeStreamOperators` | Removed (auto-registered now) |

---

## 6. Qt Widgets & GUI Changes

### enterEvent parameter type changed
```cpp
// Qt 5
void enterEvent(QEvent *event) override;

// Qt 6
void enterEvent(QEnterEvent *event) override;
```

### QStyleOption initialization
```cpp
// Qt 5
opt.init(this);

// Qt 6
opt.initFrom(this);
```

### QDropEvent position
```cpp
// Qt 5
event->pos()

// Qt 6
event->position().toPoint()
```

### QMouseEvent local position
```cpp
// Qt 5
event->localPos()

// Qt 6
event->position()
```

### QMouseEvent::pos() (deprecated)
```cpp
// Qt 5 / still compiles but deprecated in Qt 6
event->pos()

// Qt 6
event->position().toPoint()
```
> **Scopy status:** `localPos()` fixed in Subtask 7. `pos()` still present in 8 gui/ files — tracked as **Subtask 13** in `porting_process_instructions.md`.

### QFont weight
```cpp
// Qt 5
font.setWeight(75);  // int

// Qt 6
font.setWeight(QFont::Bold);  // enum
```

### High-DPI
- `Qt::AA_UseHighDpiPixmaps` — removed (always on in Qt 6, attribute is a no-op)
- Default scale factor rounding changed from `Round` to `PassThrough`

---

## 7. Qt Concurrent Changes

**QtConcurrent::run() argument order changed:**
```cpp
// Qt 5
QtConcurrent::run(obj, &Class::method, arg1, arg2);

// Qt 6
QtConcurrent::run(&Class::method, obj, arg1, arg2);
```

When template deduction fails (e.g. no-argument member function):
```cpp
// Qt 6 workaround
QtConcurrent::run([this]() { method(); });
```

`QtConcurrent::run()` now has `[[nodiscard]]` — suppress with `(void)` cast if intentionally fire-and-forget.

---

## 8. QFutureWatcher API

```cpp
// Qt 5
watcher.setPaused(true);
watcher.isPaused();

// Qt 6
watcher.setSuspended(true);
watcher.isSuspending();
```

---

## 9. Signal/Slot Modernization

Old-style `SIGNAL()`/`SLOT()` macros **still work in Qt 6** but new-style is preferred for new code.

| Aspect | Old-style SIGNAL/SLOT | New-style &Class::method |
|---|---|---|
| Errors detected | Runtime (silent fail) | Compile time |
| Overloads | Handled by signature string | Requires `QOverload<type>::of(&Class::method)` |
| Polymorphic types | Works (runtime lookup) | May require old-style or lambdas |
| Performance | Slightly slower | Faster (compile-time resolved) |
| Deprecated in Qt 6? | No — fully supported | Recommended for new code |

### Removed signal overloads

```cpp
// Qt 5 — these overloads existed:
QComboBox::currentIndexChanged(const QString &text)  // removed in Qt 6
QSpinBox::valueChanged(const QString &text)          // removed in Qt 6

// Qt 6 — int-only overload remains:
QComboBox::currentIndexChanged(int index)
QSpinBox::valueChanged(int value)
// Retrieve text via: comboBox->itemText(index)
```

### Overloaded slots — disambiguation
```cpp
// When connecting to an overloaded slot:
connect(src, &Source::signal, this, QOverload<int>::of(&MyClass::slot));
```

---

## 10. Qt 3D Module Changes

```cpp
// Qt 5
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QAttribute>

// Qt 6
#include <Qt3DCore/QGeometry>
#include <Qt3DCore/QAttribute>
```

---

## 11. OpenGL Module Changes

```cpp
// Qt 5 — QOpenGLWidget was in QtWidgets
find_package(Qt5 COMPONENTS Widgets)

// Qt 6 — QOpenGLWidget moved to separate module
find_package(Qt6 COMPONENTS Widgets OpenGLWidgets)
target_link_libraries(... Qt6::OpenGLWidgets)
```

---

## 12. QML / JS Engine

QJSEngine/QJSValue API is **almost entirely unchanged** between Qt5 and Qt6.

Non-standard JS syntax no longer accepted by Qt 6's V4 engine:
```js
// Qt 5 (non-standard, worked in V4)
for (each in obj) { ... }

// Qt 6 (standard)
for (var each in obj) { ... }
```

---

## 13. Miscellaneous API Changes

| Old | New | Notes |
|---|---|---|
| `QString(int)` constructor | `QString::number(int)` | Ambiguous in Qt 6 |
| `QString != NULL` | `!str.isNull()` | Ambiguous comparison removed |
| `QFileInfo` from `QString` | Explicit `QFileInfo(str)` | Implicit conversion removed |
| `QVariant::type() == QVariant::String` | `var.typeId() == QMetaType::QString` | |
| `QMetaType::typeName(int)` | `QMetaType(int).name()` | |
| `QSignalBlocker` temporaries | Assign to named variable | Unnamed temporaries destroyed immediately |

---

## Sources

| Document | URL |
|---|---|
| Qt 5 to Qt 6 Porting Guide | https://doc.qt.io/qt-6/portingguide.html |
| Porting with Clazy | https://doc.qt.io/qt-6/porting-to-qt6-using-clazy.html |
| Qt 6 Build System | https://doc.qt.io/qt-6/qt6-buildsystem.html |
| What's New in Qt 6.0 | https://doc.qt.io/qt-6/whatsnew60.html |
| Qt Core Changes for Qt 6 | https://doc.qt.io/qt-6/qtcore-changes-qt6.html |
