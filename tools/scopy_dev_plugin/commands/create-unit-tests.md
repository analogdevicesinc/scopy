# /create-unit-tests — Create JS unit test scripts for IIOWidget coverage

You are creating JavaScript unit test scripts for a Scopy plugin that test every IIOWidget attribute via `readWidget`/`writeWidget` and API getter/setter methods.

**Plugin:** `$ARGUMENTS`

## Step 0: Load context

Use the Read tool to check if a port state file exists:
- Path: `tasks/$ARGUMENTS-port-state.md`
- If the file does not exist, note "No state file — will discover from source files directly." and continue.

## Prerequisites check

Before writing tests, verify this input exists:
1. **Plugin API class** — use the Glob tool to search for `*_api.h` in `scopy/packages/$ARGUMENTS/`

If the API class does not exist, stop and tell the user to run `/create-api $ARGUMENTS` first.

## Step 1: Discovery

Read these specific files to discover all testable widgets and API methods:

1. **Plugin API header** (all `Q_INVOKABLE` methods):
   - Use Glob: `scopy/packages/$ARGUMENTS/plugins/*/include/**/*_api.h`
   - Extract every getter/setter pair, standalone getters, and utility methods (`calibrate()`, `refresh()`, `loadProfile()`, etc.)

2. **Widget factory source** (all IIOWidgetBuilder calls):
   - Use Glob: `scopy/packages/$ARGUMENTS/plugins/*/src/**/*.cpp`
   - Identify files that use `IIOWidgetBuilder`
   - Extract: widget keys, UI strategy type (EditableUi, ComboUi, CheckBoxUi, RangeUi), range bounds, combo options, conversion functions

3. **EMU XML** (device structure and attribute defaults):
   - Use Glob: `scopy/packages/$ARGUMENTS/emu-xml/*.xml`
   - Extract: device name (for PHY prefix), channel structure, attribute names, default values, `_available` options

4. **Tool source files** (for tool names and advanced tool detection):
   - Search for `switchToTool()`, `getAdvancedTabs()`, `switchAdvancedTab()` in the API header
   - Check for `advanced/` subdirectory in the plugin source

5. **Existing test files** (avoid duplication):
   - Use Glob: `scopy/js/testAutomations/$ARGUMENTS/`

6. **Test framework API**:
   - `scopy/js/testAutomations/common/testFramework.js`

## Step 2: Classification — WAIT FOR APPROVAL

After reading all source material, present a structured classification report:

### Widget Key Prefix
```
var PHY = "<device-name>/";  // from EMU XML
```

### Basic Tool Widgets

| Section | Widget Key | Type | Min | Max | Mid | Options | Test Helper | UID |
|---------|-----------|------|-----|-----|-----|---------|-------------|-----|
| Global  | ensm_mode | combo | - | - | - | ["radio_on", "radio_off"] | testCombo + API | UNIT.GLOBAL.ENSM_MODE |
| RX      | voltage0_in/hardwaregain | range | 0 | 30 | 15 | - | testRange + testConversion | UNIT.RX.CH0_HARDWARE_GAIN |
| ...     | ... | ... | ... | ... | ... | ... | ... | ... |

### Advanced Tool Widgets (if applicable)

Group by tab name:

| Tab | Widget Key | Type | Min | Max | Mid | Options | Test Helper | UID |
|-----|-----------|------|-----|-----|-----|---------|-------------|-----|
| CLK Settings | adi,clocks-device-clock_khz | range | 30720 | 320000 | 122880 | - | testRange | UNIT.CLK.DEVICE_CLOCK_KHZ |
| ... | ... | ... | ... | ... | ... | ... | ... | ... |

### API-Only Methods (no widget, tested via getter/setter)

| Method | Test Type | UID |
|--------|-----------|-----|
| getRxRssi(channel) (readonly) | testReadOnly via API | UNIT.RX.CH0_RSSI |
| calibrate() | complex test | UNIT.CAL.CALIBRATE_TRIGGER |
| ... | ... | ... |

### Widget Counts
- Basic tool: X widgets
- Advanced tool: Y widgets across Z tabs
- API-only: W methods
- Total attribute test cases: N

### Proposed Complex Tests

Scan the API header for these method signature triggers and list matching complex tests:

| # | Pattern | API Trigger | UID |
|---|---------|-------------|-----|
| C1 | Calibration Flow | `calibrate()` found | UNIT.CAL.FULL_CALIBRATION_FLOW |
| C2 | Profile Loading | `loadProfile()` found | UNIT.PROFILE.LOAD_AND_VERIFY |
| C3 | Gain Mode Interaction | `getXxxGainControlMode()` + `setXxxHardwareGain()` found | UNIT.RX.GAIN_MODE_INTERACTION |
| C4 | State Transitions | `getEnsmMode()`/`setEnsmMode()` found | UNIT.GLOBAL.ENSM_STATE_TRANSITIONS |
| C5 | DPD Operations | `dpdReset()` + `getDpdStatus()` found | UNIT.DPD.RESET_AND_STATUS_CH0 |
| C6 | Channel Independence | Setter with `int channel`, 2+ channels | UNIT.TX.CHANNEL_INDEPENDENCE |
| C7 | Phase Rotation | `getPhaseRotation()`/`setPhaseRotation()` found | UNIT.FPGA.PHASE_ROTATION_CH0 |
| C8 | Frequency Tuning | Hz-to-MHz conversion in getter/setter | UNIT.RX.LO_FREQUENCY |
| C9 | UDC LO Splitting | `hasUdc()`/`getUdcEnabled()` found | UNIT.UDC.LO_SPLITTING |
| C10 | Refresh Cycle | `refresh()` found | UNIT.UTIL.REFRESH_ALL |

Only list patterns where the API trigger was actually found.

### Proposed File Structure

Apply adaptive splitting:
- Always: `<plugin>_Basic_Unit_test.js` (or `<plugin>_Unit_test.js` if single file)
- If advanced tool detected: `+ <plugin>_Advanced_Unit_test.js`
- If complex tests approved: `+ <plugin>_Complex_Unit_test.js`
- If multiple files: `+ <plugin>_Unit_test.js` (combined runner)

### Variant Detection

If the plugin supports device variants (e.g., AD9371 vs AD9375), describe:
- How to detect the variant at runtime
- Which widgets/tests need skip guards

**Wait for user approval before writing any JavaScript.**

## Step 3: Interactive Complex Test Discovery

After presenting the plan, ask the user:
1. "Which complex tests should I include?" (present the matched list)
2. "Are there any plugin-specific complex scenarios not in the standard patterns?"
3. "Are there variant-specific features that need skip guards?"

## Step 4: Generate Files

Generate each file following the `unit-test-patterns` skill. Use the `file-structure.md` pattern for boilerplate.

**File locations:**
- `scopy/js/testAutomations/$ARGUMENTS/<plugin>_Basic_Unit_test.js`
- `scopy/js/testAutomations/$ARGUMENTS/<plugin>_Advanced_Unit_test.js` (if applicable)
- `scopy/js/testAutomations/$ARGUMENTS/<plugin>_Complex_Unit_test.js` (if applicable)
- `scopy/js/testAutomations/$ARGUMENTS/<plugin>_Unit_test.js` (combined runner or single file)

**Critical generation rules (non-negotiable):**

1. Every `writeWidget()` and setter call is followed by `msleep(500)`
2. Every test saves original value before modifying, and restores it in ALL code paths (normal, early return, catch)
3. Standard widget types use canonical helper functions (`testRange`, `testCombo`, `testCheckbox`, `testReadOnly`, `testConversion`)
4. Sections with 3+ widgets of the same type use `runDataDrivenTests()` with test descriptor arrays
5. Every range widget gets both `testRange()` and `testBadValueRange()` tests
6. Every combo widget gets both `testCombo()` and `testBadValueCombo()` tests
7. API getter/setter pairs with unit conversion get `testConversion()` tests
8. Files end with `TestFramework.disconnectFromDevice()`, `TestFramework.printSummary()`, `scopy.exit()`
9. UID format: `UNIT.<SECTION>.<ATTRIBUTE_NAME>` (uppercase, dots as separators)
10. Never invent API methods — only use what's in the `*_api.h` header
11. Never invent widget keys — only use keys discoverable from source code or EMU XML
12. Variant-specific tests wrapped in skip guards (e.g., `if (!isAd9375) return "SKIP"`)

## Step 5: Validate

Run the `unit-test-quality-checks` skill rules (U1-U7) against the generated files:
- [ ] [U1] Every discoverable widget has a test
- [ ] [U2] Standard helpers used (no ad-hoc logic for standard types)
- [ ] [U3] Every setter has `msleep(500)` after it
- [ ] [U4] Original values saved and restored in all code paths
- [ ] [U5] Bad value tests present for range and combo widgets
- [ ] [U6] Complex tests isolated in marked section
- [ ] [U7] File naming, license header, termination sequence correct

## Step 6: Update state file (if it exists)

```markdown
## Status
- Phase: UNIT_TESTS_COMPLETE
```

## Rules

- Do NOT modify any C++ source code
- Do NOT invent API methods that don't exist in the `*_api.h` header
- Do NOT invent widget keys — discover them from source code and EMU XML
- Getter return values are always strings — compare with `===` against string values
- Every test must restore original hardware state
- Use `"SKIP"` return for features not available on current hardware variant
