---
name: api-quality-checks
description: API class validation rules for Scopy plugins. Auto-loads when reviewing or editing `*_api.h` / `*_api.cpp` files or running API quality checks.
---

# API Quality Check Rules

Apply these 7 validation categories when reviewing or generating API classes for Scopy plugins.

---

## CRITICAL

### [A1] Inheritance & Structure

- Class inherits from `ApiObject`
- `Q_OBJECT` macro is present in the class body
- Export macro (`SCOPY_<PLUGIN>_EXPORT`) is on the class declaration
- Constructor takes `<Plugin>Plugin *` as its sole argument

### [A2] Plugin Integration

- `friend class <Plugin>_API` is declared in the plugin header
- `m_api` is declared as `<Plugin>_API *m_api = nullptr;` in the plugin header
- `initApi()` is declared in the plugin header
- `initApi()` is the **last call** in `onConnect()` before `return true`
- `delete m_api; m_api = nullptr;` is the **first statement** in `onDisconnect()`
- `ScopyJS::GetInstance()->registerApi(m_api)` is called inside `initApi()`
- `m_api->setObjectName("<name>")` uses a lowercase, short name consistent with similar plugins
- The `.cpp` file ends with `#include "moc_<plugin>_api.cpp"`

### [A3] Null Safety

- Every `Q_INVOKABLE` method null-checks plugin/instrument members before accessing them
- `readFromWidget()` / `writeToWidget()` null-check `m_widgetGroup` before use

### [A5] Conversion Correctness

Cross-reference the tool's IIOWidget lambdas to verify:

- **Hz attributes**: getter divides raw value ÷1e6; setter multiplies by ×1e6
- **Suffix attributes** (e.g. `" dB"`): getter strips the suffix before returning
- **dBFS↔linear**: getter applies `20*log10(1/linear)` rounded to int; setter applies `pow(10, -val/20)`
- **Combo attributes**: setter validates the input against a static `QStringList` before calling `writeToWidget`

---

## WARNING

### [A4] API Coverage

- `getTools()` method is present
- Every **writable** IIOWidget has a getter **and** a setter; every **read-only** widget has only a getter
- Generic `readWidget()` / `writeWidget()` helpers are present

### [A6] Private Helper Pattern

- `readFromWidget(const QString &key)` and `writeToWidget(const QString &key, const QString &value)` are declared as private helpers
- Both helpers null-check `m_plugin->m_widgetGroup` before accessing it

---

## INFO

### [A7] Naming & Types

- Getters return `QString`; setter parameters use `const QString &`
- No public getters were added to instrument classes just for API access — use `friend class` instead
- Naming follows the convention: `get<Attribute>()` / `set<Attribute>()`
