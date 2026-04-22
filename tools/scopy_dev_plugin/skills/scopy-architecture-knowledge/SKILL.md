---
name: scopy-architecture-knowledge
description: Core Scopy architecture knowledge including plugin lifecycle, library dependencies, build system, and key design patterns. Loaded by clarify-task and design-task commands.
---

# Scopy Architecture Knowledge

## Plugin Lifecycle

Plugins inherit from `PluginBase` (which implements `Plugin` interface). Required overrides:

```cpp
bool compatible(QString param, QString category) override;  // Device detection (MUST NOT alter state)
bool onConnect() override;    // Setup when device connects
bool onDisconnect() override; // Cleanup when device disconnects
```

**Full lifecycle sequence:**
1. `DeviceFactory::build(param, category)` creates DeviceImpl
2. DeviceImpl calls `compatible(param, category)` on each plugin factory instance
3. For compatible plugins: `clone()` → `preload()` (NOT on UI thread)
4. `loadIcon()` → `loadPage()` → `loadConfigPage()` → `loadToolList()` → `loadPreferencesPage()` → `loadExtraButtons()` → `postload()`
5. On device connect: `onConnect()` — enable tools, create instruments, register APIs
6. On device disconnect: `onDisconnect()` — cleanup, delete APIs

**Plugin class skeleton:**
```cpp
class MyPlugin : public QObject, PluginBase {
    SCOPY_PLUGIN
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.adi.Scopy.PluginBase")
    Q_INTERFACES(scopy::PluginBase)

public:
    bool compatible(QString m_param, QString category) override;
    void loadToolList() override;
    bool loadIcon() override;
    bool loadPage() override;
    bool onConnect() override;
    bool onDisconnect() override;
    void initMetadata() override;
};
```

## Core Library Dependency Chain

Build order and what each library provides:

| Library | Provides | Key Dependencies |
|---------|----------|-----------------|
| `scopy-common` | Shared utilities, base classes | Qt Core/Widgets |
| `scopyiioutil` | IIO communication, PingTask, CyclicalTask | libiio, libserialport |
| `scopy-gui` | Qt widgets, UI components, Style system | scopy-common, QWT, Boost |
| `scopy-gr-util` | GNU Radio integration | scopy-iioutil, scopy-gui, Gnuradio libs, Genalyzer |
| `scopy-pluginbase` | Plugin framework, MessageBroker, Preferences, ScopyJS | scopy-common, scopyiioutil |
| `scopy-iio-widgets` | IIOWidgetBuilder, IIOWidget, IIOWidgetGroup | libiio, scopy-gui, scopyiioutil |
| `scopy-pkg-manager` | Runtime package installation | — |
| `scopy-core` | ScopyMainWindow, DeviceManager, PluginManager, DeviceFactory | all above, Python3, libsigrokdecode |

## Package Structure

Each package lives under `packages/<name>/` and contains:
```
packages/<name>/
├── manifest.json.cmakein    # Package metadata (id, title, version, category)
├── CMakeLists.txt           # Build config
├── plugins/                 # One or more plugins
│   └── <plugin>/
│       ├── include/<plugin>/<plugin>plugin.h
│       ├── src/<plugin>plugin.cpp
│       ├── CMakeLists.txt
│       ├── resources/
│       ├── test/
│       └── doc/
└── emu-xml/                 # Device emulation files
```

**Build flags:** `ENABLE_PACKAGE_<NAME>` (uppercase, dashes preserved). `ENABLE_ALL_PACKAGES=ON` enables all.

## ToolMenuEntry

Tools are registered in `loadToolList()` using the macro:
```cpp
#define SCOPY_NEW_TOOLMENUENTRY(id, name, icon)
// Creates: new ToolMenuEntry(id, name, icon, this->m_name, this->m_param, this)
```

Key properties: `id`, `name`, `icon`, `visible`, `enabled`, `running`, `attached`, `detachable`, `runEnabled`, `runBtnVisible`.

In `onConnect()`, each tool is enabled and given its widget:
```cpp
toolList[i]->setEnabled(true);
toolList[i]->setTool(instrumentWidget);
```

## MessageBroker (Pub/Sub Singleton)

```cpp
// Subscribe
MessageBroker::GetInstance()->subscribe(this, "topic_name");

// Publish
MessageBroker::GetInstance()->publish("topic_name", "message_data");

// Receive in plugin
void messageCallback(QString topic, QString message) override;
```

All plugins auto-subscribe to "broadcast" topic. The broker stores `QMap<QObject*, QSet<QString>>` for subscriptions.

## Preferences (Singleton)

```cpp
// Initialize with default
Preferences::init("plugin.setting_key", defaultValue);

// Read/Write
QVariant val = Preferences::get("plugin.setting_key");
Preferences::set("plugin.setting_key", newValue);

// React to changes
connect(Preferences::GetInstance(), &Preferences::preferenceChanged,
        [](QString key, QVariant val) { /* handle */ });
```

File-backed via QSettings. Call `save()`/`load()` for persistence.

## IIOWidgetBuilder (Fluent Builder)

```cpp
IIOWidgetBuilder builder(parentWidget);
auto widget = builder
    .device(iio_device)
    .channel(iio_channel)
    .attribute("voltage")
    .uiStrategy(IIOWidgetBuilder::RangeUi)
    .compactMode(true)
    .group(widgetGroup)
    .buildSingle();
```

**UI Strategies:** `EditableUi`, `ComboUi`, `SwitchUi`, `RangeUi`, `CheckBoxUi`, `TemperatureUi`
**Data Strategies:** `AttrData`, `TriggerData`, `DeviceAttrData`, `ContextAttrData`

Priority: Channel > Device > Context (most specific wins).

## Async Patterns

**CyclicalTask** — periodic thread execution:
```cpp
CyclicalTask *ct = new CyclicalTask(myThread, this);
ct->start(5000);  // Execute every 5 seconds
ct->stop();
```

**PingTask** — device connectivity check (subclass and implement `ping()`):
```cpp
class MyPingTask : public PingTask {
    bool ping() override { /* return true if device alive */ }
};
// Signals: pingSuccess(), pingFailed(), connectionLost(), forceDisconnect()
```

## DeviceFactory

```cpp
// Single static method — creates DeviceImpl with all compatible plugins
DeviceImpl *device = DeviceFactory::build(param, category, parent);
```

## ApiObject (JS Test Automation Base)

```cpp
class MyPluginApi : public ApiObject {
    Q_OBJECT
    friend class MyPlugin;
public:
    Q_INVOKABLE QString getValue();
    Q_INVOKABLE void setValue(const QString &val);
private:
    MyPlugin *m_plugin = nullptr;
};
```

Register in `onConnect()`: `ScopyJS::GetInstance()->registerApi(m_api)`
Delete at start of `onDisconnect()`: `delete m_api; m_api = nullptr;`
