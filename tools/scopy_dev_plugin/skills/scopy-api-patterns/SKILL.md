---
name: scopy-api-patterns
description: API class conventions for Scopy plugins including ApiObject inheritance, Q_INVOKABLE patterns, and IIOWidgetGroup access. Auto-loads when writing API classes or implementing test automation interfaces.
---

# Scopy API Class Patterns

## API Class Structure

Every plugin API class follows this pattern:

```cpp
#include "scopy-<plugin>_export.h"
#include <pluginbase/apiobject.h>

namespace scopy::<plugin> {

class <Plugin>Plugin;  // Forward declaration

class SCOPY_<PLUGIN>_EXPORT <Plugin>_API : public ApiObject
{
    Q_OBJECT
public:
    explicit <Plugin>_API(<Plugin>Plugin *plugin);
    ~<Plugin>_API();

    // Tool management — always included
    Q_INVOKABLE QStringList getTools();

    // Typed getters/setters for key attributes
    Q_INVOKABLE QString getAttribute();
    Q_INVOKABLE void setAttribute(const QString &value);

    // Generic widget access for all registered widgets
    Q_INVOKABLE QStringList getWidgetKeys();
    Q_INVOKABLE QString readWidget(const QString &key);
    Q_INVOKABLE void writeWidget(const QString &key, const QString &value);

    // Utility
    Q_INVOKABLE void refresh();

private:
    QString readFromWidget(const QString &key);
    void writeToWidget(const QString &key, const QString &value);

    <Plugin>Plugin *m_plugin;
};

} // namespace scopy::<plugin>
```

## Friend Class Access Pattern

The API class accesses private plugin members via `friend class`:

```cpp
// In plugin header (<plugin>plugin.h):
class <Plugin>Plugin : public QObject, PluginBase {
    // ... existing code ...
    friend class <Plugin>_API;

private:
    <Plugin>_API *m_api = nullptr;
    void initApi();
};
```

**Never add public getters** to plugin or instrument classes just for the API. Use `friend` instead.

## Plugin Integration

### initApi() implementation

```cpp
void <Plugin>Plugin::initApi()
{
    m_api = new <Plugin>_API(this);
    m_api->setObjectName("<plugin>");  // lowercase, short
    ScopyJS::GetInstance()->registerApi(m_api);
}
```

### Lifecycle placement

```cpp
bool <Plugin>Plugin::onConnect()
{
    // ... existing setup code ...
    initApi();  // LAST line before return
    return true;
}

bool <Plugin>Plugin::onDisconnect()
{
    if(m_api) { delete m_api; m_api = nullptr; }  // FIRST line
    // ... existing cleanup code ...
    return true;
}
```

## Private Helper Methods

```cpp
QString <Plugin>_API::readFromWidget(const QString &key)
{
    if(!m_plugin->m_widgetGroup) {
        qWarning(CAT_<PLUGIN>_API) << "Widget manager not available";
        return QString();
    }
    IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
    if(!widget) {
        qWarning(CAT_<PLUGIN>_API) << "Widget not found for key:" << key;
        return QString();
    }
    QPair<QString, QString> result = widget->read();
    return result.first;
}

void <Plugin>_API::writeToWidget(const QString &key, const QString &value)
{
    if(!m_plugin->m_widgetGroup) {
        qWarning(CAT_<PLUGIN>_API) << "Widget manager not available";
        return;
    }
    IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
    if(!widget) {
        qWarning(CAT_<PLUGIN>_API) << "Widget not found for key:" << key;
        return;
    }
    widget->writeAsync(value);
}
```

## Getter/Setter Naming

| Attribute Type | Getter | Setter |
|---|---|---|
| Frequency | `getRxLoFrequency()` | `setRxLoFrequency(value)` |
| Mode/Enum | `getEnsmMode()` | `setEnsmMode(mode)` |
| Boolean enable | `isTrackingEnabled()` | `setTrackingEnabled(value)` |
| Per-channel | `getRxGain(int channel)` | `setRxGain(int channel, value)` |
| Read-only | `getRxRssi(int channel)` | *(no setter)* |

## Widget Key Format

Keys follow the pattern: `<device>/<channel>_<direction>/<attribute>`

Examples:
- `ad9361-phy/ensm_mode` (device-level attribute)
- `ad9361-phy/voltage0_in/rf_bandwidth` (channel attribute)
- `ad9361-phy/altvoltage0_out/frequency` (LO frequency)

## Rules

- Every `Q_INVOKABLE` method must null-check `m_plugin` members before access
- `.cpp` must end with `#include "moc_<plugin>_api.cpp"`
- Use `Q_LOGGING_CATEGORY(CAT_<PLUGIN>_API, "<Plugin>_API")` for warnings
- Return `QString()` (empty) on error, never crash
- All getter return types are `QString` (JS compatibility)
- Setter parameters use `const QString &`

## IIOWidget Conversion Rules

`IIOWidget::writeAsync()` writes directly to the IIO data strategy — it does **NOT** apply
`m_UItoDS`. Similarly, `read().first` returns the raw IIO value, not the UI-displayed value.
Getters and setters MUST replicate the conversion logic from `setDataToUIConversion` /
`setUItoDataConversion` lambdas found in the tool class. Before writing API methods, read the
tool implementation (`<plugin>.cpp`) and catalogue every widget's conversion lambdas.

**Frequency (Hz ↔ MHz):**
```cpp
// Getter: divide raw Hz string by 1e6
QString getXxx() {
    QString raw = readFromWidget(key);
    if(raw.isEmpty()) return raw;
    return QString::number(raw.toDouble() / 1e6, 'f', 3);
}
// Setter: multiply MHz value by 1e6 before writing
void setXxx(const QString &val) {
    writeToWidget(key, QString::number(val.toDouble() * 1e6, 'f', 0));
}
```

**IIO unit suffix (e.g. " dB" appended by IIO driver to hardwaregain):**
```cpp
// Getter: strip the trailing suffix before returning
QString getXxx() {
    QString raw = readFromWidget(key);
    int idx = raw.indexOf(" dB");
    if(idx != -1) raw = raw.left(idx).trimmed();
    return raw;
}
// Setter: write numeric string only (IIO accepts without suffix)
void setXxx(const QString &val) { writeToWidget(key, val); }
```

**dBFS ↔ linear scale (e.g. IIO stores 0.5 linear, UI shows 6 dBFS):**
```cpp
#include <cmath>
// Getter: linear → dBFS (round to nearest integer)
QString getXxx() {
    QString raw = readFromWidget(key);
    if(raw.isEmpty()) return raw;
    double linear = raw.toDouble();
    if(linear <= 0.0) return QString("0");
    return QString::number(static_cast<int>(20.0 * std::log10(1.0 / linear) + 0.5));
}
// Setter: dBFS → linear
void setXxx(const QString &val) {
    double linear = std::pow(10.0, -val.toDouble() / 20.0);
    writeToWidget(key, QString::number(linear, 'g', 10));
}
```

**Combo/enum widgets — validate before writing:**
```cpp
void setXxx(const QString &val)
{
    static const QStringList options = {"opt1", "opt2", "opt3"};
    if(!options.contains(val)) {
        qWarning(CAT_...) << "Invalid value:" << val << "Valid:" << options;
        return;
    }
    writeToWidget(key, val);
}
```

> **Note:** If any attribute uses `std::log10` or `std::pow` (dBFS↔linear scale conversions),
> add `#include <cmath>` to the `.cpp` file.
