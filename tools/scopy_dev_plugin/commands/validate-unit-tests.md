# /validate-unit-tests — Validate JS unit test scripts for a Scopy plugin

You are validating the JavaScript unit test scripts for the Scopy plugin: `$ARGUMENTS`

The `unit-test-quality-checks` skill rules (checks U1–U7) govern this analysis.

## Step 1: Discover files

Use the Glob tool to locate:
- Unit test files: `js/testAutomations/$ARGUMENTS/*_Unit_test.js`, `js/testAutomations/$ARGUMENTS/*_Basic_Unit_test.js`, `js/testAutomations/$ARGUMENTS/*_Advanced_Unit_test.js`, `js/testAutomations/$ARGUMENTS/*_Complex_Unit_test.js`
- Plugin API header: `scopy/packages/$ARGUMENTS/plugins/*/include/**/*_api.h`
- Widget factory sources: `scopy/packages/$ARGUMENTS/plugins/*/src/**/*.cpp` (files using `IIOWidgetBuilder`)
- EMU XML: `scopy/packages/$ARGUMENTS/emu-xml/*.xml`
- Test framework: `js/testAutomations/common/testFramework.js`

If no JS unit test files are found, report "No unit test files found for `$ARGUMENTS`" and stop.

Read **all** discovered files before starting analysis.

## Step 2: Build expected widget set

From the source files, build the complete set of testable widgets and API methods:

1. **From widget factory sources**: Extract all IIOWidgetBuilder calls to get widget keys and their types (EditableUi → range, ComboUi → combo, CheckBoxUi → checkbox, read-only patterns)
2. **From API header**: Extract all `Q_INVOKABLE` getter/setter pairs and standalone getters
3. **From EMU XML**: Extract device name prefix, channel structure, attribute names

This is the "expected" set — every item should have at least one test.

## Step 3: Build actual test set

From the JS unit test files, extract:

1. **Widget keys tested**: All keys passed to `readWidget()`, `writeWidget()`, `testRange()`, `testCombo()`, `testCheckbox()`, `testReadOnly()`, `testConversion()`, `testBadValueRange()`, `testBadValueCombo()`, and `runDataDrivenTests()` calls
2. **API methods exercised**: All `<apiObject>.<method>()` calls
3. **Test UIDs**: All `TestFramework.runTest("<UID>", ...)` UIDs

## Step 4: Run checks U1–U7

### CRITICAL

**[U1] Widget Coverage**
- Compare expected widget set against actual test set
- Flag any widget key with no corresponding test
- Flag any `Q_INVOKABLE` getter/setter pair with no test exercising it
- Flag any test referencing a widget key not in the expected set
- Report coverage: `X/Y widgets tested (Z%)`

**[U2] Helper Function Usage**
- Scan each `TestFramework.runTest()` body
- For non-complex tests: if the body manually writes/reads/compares widget values when a standard helper exists for that widget type, flag it
- Verify `runDataDrivenTests()` is used for sections with 3+ widgets of the same type
- Complex tests (in sections marked `// SECTION: Complex`) are exempt

**[U3] Sleep After Setters**
- Identify every `writeWidget()`, `set*()`, or state-mutating call (`calibrate()`, `dpdReset()`, `refresh()`, `loadProfile()`)
- Check that the very next non-empty line is `msleep(500)` or longer
- Flag any setter not followed immediately by msleep
- Check both in helper definitions and in individual test bodies

**[U4] State Restoration**
- Identify every test that modifies state (calls a setter or writeWidget)
- Check that it saves the original value before modification
- Check that it restores the original value in: normal completion, early return, and catch block
- Flag any test that modifies state without full restoration

### WARNING

**[U5] Bad Value Tests**
- Count range widgets with `testRange()` calls vs those with `testBadValueRange()` calls
- Count combo widgets with `testCombo()` calls vs those with `testBadValueCombo()` calls
- Report coverage ratio for each
- Warn if bad value test coverage drops below 80%

**[U6] Complex Test Isolation**
- Check that complex multi-step tests are in a clearly marked section
- Check each complex test has a descriptive comment (e.g., `// C1: Full Calibration Flow`)
- Check complex test UIDs follow `UNIT.<SECTION>.<DESCRIPTIVE_NAME>` format
- Flag any undocumented complex logic mixed into attribute test sections

### INFO

**[U7] File Structure**
- Verify file naming convention (`*_Basic_Unit_test.js`, `*_Advanced_Unit_test.js`, etc.)
- Verify GPL license header present in every file
- Verify termination sequence: `disconnectFromDevice()` → `printSummary()` → `scopy.exit()`
- Verify combined runner uses `evaluateFile()` and does not duplicate helpers or tests
- Report file count and structure

## Step 5: Generate report

```
## Unit Test Validation Report: $ARGUMENTS

### Summary
| Check | Severity | Result |
|-------|----------|--------|
| [U1] Widget Coverage | CRITICAL | PASS/FAIL |
| [U2] Helper Function Usage | CRITICAL | PASS/FAIL |
| [U3] Sleep After Setters | CRITICAL | PASS/FAIL |
| [U4] State Restoration | CRITICAL | PASS/FAIL |
| [U5] Bad Value Tests | WARNING | PASS/WARN |
| [U6] Complex Test Isolation | WARNING | PASS/WARN |
| [U7] File Structure | INFO | PASS/INFO |

### Critical Issues
**[U1] Missing widget coverage**
`voltage0_in/rf_bandwidth` — no test found in any unit test file.
> **Fix:** Add a testReadOnly() call for this widget.

**[U3] Missing sleep after setter**
`ad9371_Basic_Unit_test.js:142` — `writeWidget()` not followed by `msleep(500)`.
> **Fix:** Add `msleep(500);` on the next line.

### Warnings
**[U5] Bad value test coverage**
- Range widgets: 12/15 have testBadValueRange() (80%)
- Combo widgets: 3/5 have testBadValueCombo() (60%) — below 80% threshold

### Info
**[U7] File structure**
- Files found: <plugin>_Basic_Unit_test.js, <plugin>_Advanced_Unit_test.js, <plugin>_Unit_test.js
- License headers: OK
- Termination sequence: OK

### Widget Coverage Detail
| Widget Key | Type | Has Test | Has Bad Value Test |
|-----------|------|----------|-------------------|
| voltage0_in/hardwaregain | range | YES | YES |
| voltage0_in/rf_bandwidth | readonly | NO | N/A |
| ensm_mode | combo | YES | YES |
| ... | ... | ... | ... |
Coverage: X/Y widgets tested (Z%)

### API Method Coverage
| Method | Has Test |
|--------|----------|
| getRxHardwareGain(channel) | YES |
| setRxHardwareGain(channel, val) | YES |
| getRxRssi(channel) | NO |
| ... | ... |
Coverage: X/Y methods tested (Z%)

### Verdict
[PASS/FAIL] — [one sentence summary]
```

PASS = zero critical issues. FAIL = one or more critical issues.
