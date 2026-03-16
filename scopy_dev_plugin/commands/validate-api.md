# /validate-api — Validate API class for a Scopy plugin

You are validating the JavaScript API class implementation for the Scopy plugin: `$ARGUMENTS`

The `api-quality-checks` skill rules (checks A1–A7) govern this analysis.

## Step 1: Discover files

Use the Glob tool to locate:
- Plugin API header: `scopy/packages/$ARGUMENTS/plugins/*/include/**/*_api.h`
- Plugin API source: `scopy/packages/$ARGUMENTS/plugins/*/src/*_api.cpp`
- Plugin header: `scopy/packages/$ARGUMENTS/plugins/*/include/**/*plugin.h`
- Plugin source: `scopy/packages/$ARGUMENTS/plugins/*/src/*plugin.cpp`
- Tool implementation: `scopy/packages/$ARGUMENTS/plugins/*/src/$ARGUMENTS*.cpp` (the tool class, not the plugin)

If no `*_api.h` is found, report "No API class found for `$ARGUMENTS`" and stop.

Read **all** discovered files before starting analysis.

## Step 2: Run checks A1–A7

Apply every rule from the `api-quality-checks` skill:

### CRITICAL

**[A1] Inheritance & Structure**
- Inherits from `ApiObject`?
- `Q_OBJECT` present?
- Export macro (`SCOPY_<PLUGIN>_EXPORT`) on class declaration?
- Constructor takes `<Plugin>Plugin *`?

**[A2] Plugin Integration**
- `friend class <Plugin>_API` in plugin header?
- `m_api` declared and initialised to `nullptr`?
- `initApi()` declared in plugin header?
- `initApi()` is the last call in `onConnect()` before `return true`?
- `delete m_api; m_api = nullptr;` is the first statement in `onDisconnect()`?
- `ScopyJS::GetInstance()->registerApi(m_api)` inside `initApi()`?
- `setObjectName()` uses a lowercase short name?
- `.cpp` ends with `#include "moc_<plugin>_api.cpp"`?

**[A3] Null Safety**
- Every `Q_INVOKABLE` method null-checks before accessing members?
- `readFromWidget()` / `writeToWidget()` null-check `m_widgetGroup`?

**[A5] Conversion Correctness**
Cross-reference tool's IIOWidget lambdas (`setDataToUIConversion` / `setUItoDataConversion`):
- Hz attributes: getter ÷1e6, setter ×1e6?
- Suffix attributes: getter strips suffix?
- dBFS↔linear: correct `log10`/`pow` formulas?
- Combo attributes: setter validates against static `QStringList`?

### WARNING

**[A4] API Coverage**
- `getTools()` present?
- Writable widgets have getter + setter; read-only have getter only?
- Generic `readWidget()` / `writeWidget()` present?

**[A6] Private Helper Pattern**
- `readFromWidget()` and `writeToWidget()` declared as private?
- Both null-check `m_plugin->m_widgetGroup`?

### INFO

**[A7] Naming & Types**
- Getters return `QString`, setters take `const QString &`?
- No public getters added to instrument classes?
- Naming follows `get<Attribute>()` / `set<Attribute>()` convention?

## Step 3: Generate report

```
## API Validation Report: $ARGUMENTS

### Summary
| Check | Severity | Result |
|-------|----------|--------|
| [A1] Inheritance & Structure | CRITICAL | PASS/FAIL |
| [A2] Plugin Integration | CRITICAL | PASS/FAIL |
| [A3] Null Safety | CRITICAL | PASS/FAIL |
| [A5] Conversion Correctness | CRITICAL | PASS/FAIL |
| [A4] API Coverage | WARNING | PASS/WARN |
| [A6] Private Helper Pattern | WARNING | PASS/WARN |
| [A7] Naming & Types | INFO | PASS/INFO |

### Critical Issues
**[A2] initApi() not last in onConnect()**
`src/myplugin.cpp:87` — `initApi()` is called before widget setup completes.
> **Fix:** Move `initApi()` to the line immediately before `return true`.

### Warnings
...

### Info
...

### Verdict
[PASS/FAIL] — [one sentence summary]
```

PASS = zero critical issues. FAIL = one or more critical issues.
