---
name: unit-test-quality-checks
description: Unit test validation rules for Scopy plugin IIOWidget tests. Auto-loads when reviewing or writing `*_Unit_test.js` files.
---

# JS Unit Test Quality Check Rules

Apply these 7 validation categories when reviewing or generating JS unit test scripts for Scopy plugins.

---

## CRITICAL

### [U1] Widget Coverage

- Extract all widget keys from the plugin's widget factory source (IIOWidgetBuilder calls) or `getWidgetKeys()` return
- Extract all `Q_INVOKABLE` getter/setter pairs from the plugin's `*_api.h`
- Compare against widget keys and API methods exercised in the JS unit test files (from `readWidget()`, `writeWidget()`, `testRange()`, `testCombo()`, `testCheckbox()`, `testReadOnly()`, `testConversion()`, and `runDataDrivenTests()` calls)
- Flag any widget key that has no corresponding test
- Flag any `Q_INVOKABLE` getter/setter pair with no test exercising it
- Flag any test that references a widget key not in the discoverable set

### [U2] Helper Function Usage

- Tests for standard widget types MUST use canonical helper functions:
  - Range/spinbox widgets: `testRange()`
  - Combo/dropdown widgets: `testCombo()`
  - Checkbox/toggle widgets: `testCheckbox()`
  - Read-only widgets: `testReadOnly()`
  - API conversion tests: `testConversion()`
  - Bad value boundary tests: `testBadValueRange()`, `testBadValueCombo()`
- Data-driven sections with 3+ widgets of the same type MUST use `runDataDrivenTests()`
- Flag any test that manually writes/reads/compares widget values when a standard helper exists for that widget type
- **Exception:** complex tests (in the Complex section) are allowed custom logic

### [U3] Sleep After Setters

- Every `writeWidget()`, `set*()`, or any state-mutating API call (e.g., `calibrate()`, `dpdReset()`, `refresh()`, `loadProfile()`) must be followed **immediately** by `msleep(500)` on the next non-empty line
- `msleep(1000)` or longer is acceptable (minimum is `msleep(500)`)
- This applies both inside helper function definitions and in individual test bodies
- No state-mutating call may be left without a subsequent sleep

### [U4] State Restoration

- Every test that modifies device state must save the original value before changing it
- The original value must be restored in **all** exit paths: normal completion, early return, and catch block
- For tests that modify multiple values (e.g., ENSM mode + gain control mode), all modified values must be restored
- Complex tests modifying multiple widgets must restore every widget they changed

---

## WARNING

### [U5] Bad Value Tests

- Every range widget with a `testRange()` call SHOULD also have a `testBadValueRange()` call
- Every combo widget with a `testCombo()` call SHOULD also have a `testBadValueCombo()` call
- Report the coverage ratio (e.g., "12/15 range widgets have bad value tests, 80%")
- Warn if bad value test coverage drops below 80%

### [U6] Complex Test Isolation

- Complex multi-step tests must be in a clearly marked section (e.g., `// SECTION: Complex Functionality Tests`)
- Each complex test must have a comment documenting what it tests (e.g., `// C1: Full Calibration Flow`)
- Complex test UIDs must follow `UNIT.<SECTION>.<DESCRIPTIVE_NAME>` format
- Flag any undocumented complex test mixed into attribute test sections

---

## INFO

### [U7] File Structure

- File naming must follow the convention:
  - `<plugin>_Basic_Unit_test.js` — basic/main tool widget tests
  - `<plugin>_Advanced_Unit_test.js` — advanced tab widget tests
  - `<plugin>_Complex_Unit_test.js` — multi-step functionality tests
  - `<plugin>_Unit_test.js` — combined runner (or single file if only one)
- Each standalone file must end with the termination sequence in order:
  1. `TestFramework.disconnectFromDevice()`
  2. `TestFramework.printSummary()`
  3. `scopy.exit()`
- The combined runner file uses `evaluateFile()` to include sub-files and must NOT duplicate helper functions or test cases
- The GPL license header must be present at the top of every file
