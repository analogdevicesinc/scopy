---
name: scopy-component-relationships
description: How Scopy libraries depend on each other, plugin commonalities, MessageBroker topics, shared widgets, style system flow, and build system relationships. Loaded when determining component interactions or change impact.
---

# Scopy Component Relationships

## Library Dependency Graph

```
                    ┌──────────────┐
                    │  scopy-core  │
                    └──────┬───────┘
           ┌───────────────┼───────────────────┐
           │               │                   │
    ┌──────▼───────┐ ┌─────▼──────┐ ┌──────────▼──────────┐
    │scopy-pluginbase│ │ scopy-gui │ │ scopy-pkg-manager   │
    └──────┬───────┘ └─────┬──────┘ └─────────────────────┘
           │               │
    ┌──────▼───────────────▼──────┐
    │       scopy-common          │
    └──────┬──────────────────────┘
           │
    ┌──────▼───────┐    ┌────────────────────┐
    │ scopyiioutil  │◄───│  scopy-iio-widgets │
    └──────────────┘    └────────────────────┘
                              │
                        ┌─────▼──────┐
                        │scopy-gr-util│
                        └────────────┘
```

**Detailed link dependencies:**

| Library | Links Against |
|---------|--------------|
| `scopy-common` | Qt Core/Widgets/Concurrent |
| `scopyiioutil` | Qt Widgets, libiio, libserialport |
| `scopy-gui` | scopy-common, scopy-pluginbase, QWT, Boost, Qt Widgets/Xml/Svg |
| `scopy-gr-util` | scopy-iioutil, scopy-gui, scopy-iio-widgets, Gnuradio (runtime/analog/blocks/fft/filter/pmt/iio), Genalyzer |
| `scopy-pluginbase` | scopy-common, scopyiioutil, Qt Core/Widgets/Concurrent/Qml, libiio |
| `scopy-iio-widgets` | scopy-gui, scopyiioutil, libiio, Qt Core/Widgets |
| `scopy-pkg-manager` | Qt Core |
| `scopy-core` | all above, Python3, libsigrokdecode, Qt Core/Widgets/Concurrent/Network |

## Plugin Pattern Families

### Device Plugins (hardware-specific)
**Packages:** ad936x, ad9371, adrv9002, adrv9009, apollo-ad9084, cn0511, daq2, fmcomms11, swiot, pqmon, imu, rfpowermeter

**Common patterns:**
- `compatible()` checks IIO context for specific device names
- Use IIOWidgetBuilder extensively for attribute controls
- Create instrument/tool classes per device feature
- Often have gain widgets, frequency controls, calibration sections
- Use CyclicalTask/PingTask for device monitoring
- API class exposes Q_INVOKABLE methods for test automation

### Generic Plugins
**Package:** generic-plugins
**Plugins:** ADC, DAC, Datalogger, Debugger, Regmap, JESD Status

**Common patterns:**
- Work with any IIO device (not device-specific compatible())
- ADC/DAC use GNU Radio pipeline (gr-util dependency)
- Debugger/Regmap are register-level tools
- Datalogger handles data recording across devices

### Utility Plugins
**Package:** extproc (external processor), test-plugins

**Patterns:**
- extproc: External process management, plot management
- test-plugins: Development/testing utilities (default OFF)

## MessageBroker Topics

The MessageBroker is a singleton pub/sub system. All plugins auto-subscribe to "broadcast".

**Known topic conventions:**
- Topic names typically match plugin class names (e.g., "TestPlugin", "TestPlugin2")
- "broadcast" — default topic, all plugins receive
- Device-specific topics for cross-plugin coordination

**Usage pattern:**
```cpp
// Publisher plugin
MessageBroker::GetInstance()->publish("MyPlugin", "calibration_complete");

// Subscriber plugin (in constructor or init)
MessageBroker::GetInstance()->subscribe(this, "MyPlugin");

// Receive
void messageCallback(QString topic, QString message) override {
    if(topic == "MyPlugin" && message == "calibration_complete") { /* react */ }
}
```

## Shared IIOWidget Patterns

IIOWidgetBuilder is used across most device plugins. Common widget configurations:

| Widget Type | Use Case | Plugins Using It |
|-------------|----------|-----------------|
| `RangeUi` | Numeric attributes (frequency, gain) | ad936x, adrv9002, adrv9009, generic ADC/DAC |
| `ComboUi` | Enumerated attributes (mode, filter) | ad936x, adrv9002, swiot, generic |
| `CheckBoxUi` | Boolean attributes (enable/disable) | swiot, pqmon |
| `EditableUi` | Free-form text attributes | debugger, regmap |
| `TemperatureUi` | Temperature with thresholds | imu, rfpowermeter |

**IIOWidgetGroup** — groups widgets for batch read/write operations. Most plugins create one group per tool/section.

## Style System Flow

```
JSON theme files → Style::getAttribute() / Style::getColor()
                 → Style::setStyle(widget, property)
                 → Style::setBackgroundColor(widget, color)
```

**Key theme color categories:**

| Category | Values | Purpose |
|----------|--------|---------|
| Background | `background_primary`, `background_subtle`, `background_secondary`, `background_plot` | Container backgrounds |
| Content | `content_default`, `content_subtle`, `content_silent`, `content_inverse` | Text and icons |
| Status | `content_success`, `content_error`, `content_busy` | Status indicators |
| Interactive | `interactive_primary_*`, `interactive_subtle_*`, `interactive_secondary_*` | Buttons, controls (idle/hover/pressed/disabled) |
| Special | `interactive_accent_idle`, `danger_default`, `interactive_focus` | Accent and danger states |

**Common style properties:**
```cpp
Style::setStyle(widget, style::properties::label::menuBig);
Style::setStyle(widget, style::properties::widget::border_interactive);
Style::setStyle(widget, style::properties::widget::basicBackground, true, true);
Style::setBackgroundColor(widget, json::theme::background_primary);
```

## Build System Relationships

**Package enable flags:**
- `ENABLE_PACKAGE_<NAME>` — uppercase name with dashes preserved
- `ENABLE_ALL_PACKAGES=ON` — enables everything
- Default ON for all except: `TEST-PLUGINS` (OFF), `EXTPROC` (OFF)

**Plugin enable flags (within a package):**
- `ENABLE_PLUGIN_<NAME>` — individual plugin control
- Defined by `add_plugins()` function in PackageUtils.cmake

**Available packages (16 total):**
ad936x, ad9371, adrv9002, adrv9009, apollo-ad9084, cn0511, daq2, extproc, fmcomms11, generic-plugins, imu, m2k, pqmon, rfpowermeter, swiot, test-plugins

## Cross-Cutting Concerns

### Settings Persistence
- Plugin-level: `saveSettings(QSettings&)` / `loadSettings(QSettings&)`
- Global: `Preferences::get/set()` singleton
- Both backed by QSettings (INI file format)

### Device Connection State
- DeviceImpl manages plugin lifecycle
- `m_connectedPlugins` tracks which plugins are active
- PingTask monitors device health
- `connectionLost()` signal triggers reconnection/warning UI

### ScopyJS (JavaScript Engine)
- Enabled by default (`ENABLE_SCOPYJS=ON`)
- API objects registered via `ScopyJS::GetInstance()->registerApi()`
- Used for test automation and scripting
- Each plugin can expose one API object
