# /create-api — Generate JavaScript API class for a Scopy plugin

You are creating a JavaScript API class for a Scopy plugin so it can be controlled from test automation scripts.

**Plugin:** `$ARGUMENTS`

## Step 0: Load context

Use the Read tool to check if a port state file exists:
- Path: `tasks/$ARGUMENTS-port-state.md`
- If the file does not exist, note "No state file — will discover from source files directly." and continue.

## Step 1: Discovery

Locate the plugin's source files by checking these paths in priority order:

**If state file exists**, use it as the inventory: extract the plugin class name, namespace, tool list IDs, and section/attribute inventory.

**If no state file**, discover directly:
- Use the Glob tool to find the plugin header: `scopy/packages/$ARGUMENTS/plugins/*/include/*/$ARGUMENTS*plugin.h`
- Use the Glob tool to find the plugin source: `scopy/packages/$ARGUMENTS/plugins/*/src/$ARGUMENTS*plugin.cpp`
- Read tool class files in the same directory
- Read `manifest.json.cmakein` for device name and display name
- Read CMakeLists.txt for library and export macro naming

**Also read the tool implementation** (`<plugin>.cpp` in `src/`) to catalogue every
IIOWidget's conversion setup. For each widget, note:
- `setDataToUIConversion` lambda → determines what the **getter** must do (e.g. ÷1e6 for Hz→MHz)
- `setUItoDataConversion` lambda → determines what the **setter** must do (e.g. ×1e6 for MHz→Hz)
- IIO attribute suffix patterns (e.g. hardwaregain returns "-10.000000 dB" — suffix must be stripped in getter)
- Combo widget available-attribute name → extract valid enum string list for setter validation

Also check if an API already exists (use the Glob tool to search for `*_api.h` in the plugin's package dir). If found, report it to the user and stop.

## Step 2: Design — WAIT FOR APPROVAL

After reading the plugin header, source, and instrument classes, present:

1. **API pattern chosen** (Pattern A: tool-based, Pattern B: IIOWidgetGroup-based, or both)
2. **Complete method list** grouped by instrument/category:
   - `getTools()` always included
   - getter/setter pairs for each widget member
   - run-control methods
   - IIOWidget access methods (if applicable)

   For each getter/setter pair, also document a **conversion row**:
   | Method | IIO unit | API unit | Getter conversion | Setter conversion |
3. **Missing Functionality Report** — things that CAN'T be exposed without base-class changes
4. **Files to create/modify**:
   - `<plugin>_api.h` (new)
   - `<plugin>_api.cpp` (new)
   - Plugin header (add friend + `m_api` member + `initApi()`)
   - Plugin source (add `initApi()` impl + call in `onConnect()` + delete in `onDisconnect()`)
   - CMakeLists.txt (only if it uses explicit file lists — check first)

**Wait for user approval before writing any code.**

## Step 3: Implement

Follow the API patterns described in the `scopy-api-patterns` knowledge skill:
- Use `friend class` for private member access (do NOT add public getters to instrument classes)
- Null-check every instrument pointer
- Call `initApi()` as the last line of `onConnect()` before `return true`
- Delete `m_api` at the BEGINNING of `onDisconnect()` before any other cleanup
- Set the JS object name with `setObjectName("pluginname")` — lowercase, short
- Register with `ScopyJS::GetInstance()->registerApi(m_api)`
- End the `.cpp` file with `#include "moc_<plugin>_api.cpp"`

## IIOWidget Conversion Rules

`IIOWidget::writeAsync()` writes directly to the IIO data strategy — it does **NOT** apply
`m_UItoDS`. Similarly, `read().first` returns the raw IIO value, not the UI-displayed value.
Getters and setters MUST replicate the conversion logic from `setDataToUIConversion` /
`setUItoDataConversion` lambdas found in the tool class.

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

## Step 4: Validate

- [ ] API class inherits from `ApiObject`
- [ ] All `Q_INVOKABLE` methods null-check before accessing members
- [ ] `friend class` declared in plugin header
- [ ] `initApi()` called at end of `onConnect()`, `delete m_api` at start of `onDisconnect()`
- [ ] `.cpp` ends with `#include "moc_<plugin>_api.cpp"`
- [ ] JS object name is lowercase and consistent with similar plugins
- [ ] No modifications to instrument, widget, or utility classes

## Step 5: Update state file (if it exists)

```markdown
## Status
- Phase: API_COMPLETE
```

## Rules

- Do NOT modify any instrument, widget, or utility class — only the plugin header/source
- Do NOT create API methods that require new public methods in base classes
- Do NOT add friend declarations unless private member access is actually needed
- Keep the JS object name lowercase and consistent with similar plugins
- Every method must null-check before accessing members
