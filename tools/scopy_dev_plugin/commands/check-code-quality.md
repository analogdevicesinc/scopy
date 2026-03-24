# Code Quality Analysis for Scopy Package

You are performing static code quality analysis on the Scopy package: `$ARGUMENTS`

## Step 1: Discover Source Files

Use the Glob tool to find all `.h`, `.hpp`, and `.cpp` files in `scopy/packages/$ARGUMENTS/`.

**Exclude** these generated/build files:
- `*_export.h`
- `*_config.h`
- `moc_*.cpp`
- `ui_*.h`
- Anything under `build*/` directories

Read ALL discovered files before starting analysis. Cross-file analysis is essential for lifecycle and dead code checks.

## Step 2: Analyze Against Check Categories

Analyze every file against these 7 categories, ordered by severity:

### CRITICAL Severity

**[C1] Logic Errors**
- Always-true/false conditions
- Wrong operators (`=` vs `==`, `&` vs `&&`, `|` vs `||`)
- Missing `break` in switch-case (unless fallthrough is intentional and commented)
- Off-by-one errors in loops/indexing
- Unreachable code after return/break/continue
- Integer overflow/truncation in arithmetic

**[C2] Possible Bugs**
- Null pointer dereference — especially after IIO calls like `iio_context_find_device()`, `iio_device_find_channel()`, `iio_device_find_attr()` which can return NULL
- Use-after-delete or use-after-deleteLater
- Uninitialized member variables (check constructor initializer lists)
- Raw pointer set to dangling state after delete (missing `= nullptr`)
- Ignored return values from IIO functions that indicate errors
- Double-free or double-delete scenarios
- Race conditions with signals/slots across threads

**[C3] Plugin Lifecycle**
- **onConnect/onDisconnect asymmetry**: every resource acquired in `onConnect()` or `init()` must be released in `onDisconnect()` or `deinit()`
  - Every `new` needs a corresponding `delete` (unless Qt parent-child ownership handles it)
  - Every `open()` needs `close()`
  - Every `connect()` (signal) needs `disconnect()` (unless the object is deleted, which auto-disconnects)
  - IIOWidgetGroup creation must have matching cleanup
  - API objects registered via `registerApiObject()` should be unregistered
- Check that `onDisconnect()` properly reverses `onConnect()`

### WARNING Severity

**[W1] Qt Issues**
- Missing `Q_OBJECT` macro in QObject-derived classes that use signals/slots
- Old-style `SIGNAL()`/`SLOT()` string-based connections (prefer new-style `&Class::method`)
- Missing parent parameter in QObject/QWidget constructors (potential memory leak)
- Missing virtual destructor on non-QObject base classes used polymorphically
- `QTimer::singleShot` with raw `this` pointer without preventing dangling calls

**[W2] Dead Code**
- Private methods declared but never called within the class
- Member variables declared but never read
- Commented-out code blocks (3+ consecutive lines)
- Empty non-virtual function bodies
- Unused local variables
- Unused `#include` of project headers

### INFO Severity

**[I1] Naming Conventions**
- Member variables should use `m_` prefix
- Methods should be `camelCase()`
- Classes should be `PascalCase`
- File names should be lowercase
- Use `Q_SLOTS` not `slots`, `Q_SIGNALS` not `signals`

**[I2] Include Hygiene**
- Includes that appear unused
- Headers that could use forward declarations instead
- Missing include guards or `#pragma once`

## Step 3: Generate Report

### Rules for Reporting
- **Be conservative**: if unsure, downgrade severity
- **Be Qt-ownership-aware**: don't flag missing `delete` when Qt parent handles it
- **Be specific**: every finding must include file path, line number, and fix suggestion
- **No false positives**: only report issues you are confident about

### Report Format

```
## Code Quality Report: $ARGUMENTS

### Summary
| Category | Critical | Warning | Info |
|----------|----------|---------|------|
| Logic Errors | X | - | - |
| Possible Bugs | X | - | - |
| Plugin Lifecycle | X | - | - |
| Qt Issues | - | X | - |
| Dead Code | - | X | - |
| Naming Conventions | - | - | X |
| Include Hygiene | - | - | X |
| **Total** | **X** | **X** | **X** |

### Critical Issues
**[C2] Null pointer not checked after IIO call**
`src/plugin.cpp:142` — `iio_context_find_device()` return value used without null check.
> **Fix:** Add null check after the call.

### Warnings
...

### Info
...

### Verdict
[PASS/FAIL] — [one sentence summary]
```

PASS = zero critical issues. FAIL = one or more critical issues.
