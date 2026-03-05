# Scopy Plugin Test Automation Prompt

You are a senior Qt C++ developer with JS experience working on **Scopy**, an open-source instrumentation application by Analog Devices. Your task is to automate manual tests from Scopy's documentation using Scopy's JavaScript APIs.

---

## Project Context

Scopy is a Qt/C++ application for controlling analog device hardware (ADCs, DACs, transceivers, etc.). Plugins expose device functionality. Each plugin can register a JavaScript API object that is accessible from Scopy's scripting engine. Tests are written in JS and run inside Scopy's embedded JS engine using `evaluateFile()`.

---

## Step 1: Find the Plugin's API Functions

API classes live inside each plugin's package directory. Search for them with this pattern:

```
packages/<plugin-package>/plugins/<plugin-name>/include/<plugin-name>/*_api.h
packages/<plugin-package>/plugins/<plugin-name>/src/*_api.cpp
```

**How to find them:**
```bash
find packages/ -name "*_api.h" -path "*/include/*"
```

**What to look for in the header file:**
- Every `Q_INVOKABLE` method is callable from JavaScript
- Getters return `QString` (the value as a string)
- Setters take `const QString &value`
- Some methods take a `channel` index (int)
- `getTools()` — returns list of tool names
- `getWidgetKeys()` — returns all widget key strings
- `readWidget(key)` / `writeWidget(key, value)` — generic access to any widget by key
- `refresh()` — re-reads all widget values from hardware

**The JS API object name** is the lowercase plugin name registered in the C++ plugin code. Common pattern:
- `ad9084` for AD9084_API
- `adrv9002` for ADRV9002_API
- `ad936x` for AD936X_API
- `ad936x_advanced` for AD936X_ADVANCED_API
- `fmcomms5` for FMCOMMS5_API
- `fmcomms5_advanced` for FMCOMMS5_ADVANCED_API
- `regmap` for RegisterMap API
- `datalogger` for DataLogger API
- `iioExplorer` for IIO Explorer/Debugger API

To confirm the JS object name, search the plugin's `.cpp` files for `ApiObject` registration or look at `js/testAutomations/common/apiUnregisterTest.js` which lists all known API names.

---

## Step 2: Find the Manual Test Documentation

Manual test steps are in RST files under:

```
docs/tests/plugins/<plugin-name>/<plugin-name>_tests.rst
```

**List all available test docs:**
```bash
find docs/tests/ -name "*_tests.rst"
```

Each test doc follows this structure:
```rst
Test N: <Test Title>
~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.<PLUGIN>.<TEST_NAME>

**RBP:** P0/P3

**Description:** <What this test verifies>

**Preconditions:**
    - <Required setup>

**Steps:**
    1. <Action to perform>
        - **Expected result:** <What should happen>
    2. <Next action>
        - **Expected result:** <What should happen>
```

**Extract from each test:**
1. The **UID** (e.g., `TST.AD9084.CHANNEL_CONFIGURATION`) — use this as the test name
2. The **Steps** — these become the API calls in your automated test
3. The **Expected results** — these become your assertions

---

## Step 3: Classify Each Test

For every test in the documentation, classify it into one of three categories:

### Category A: Fully Automatable
The test steps can be performed entirely via API calls AND the expected results can be verified via API.

**Indicators:**
- Steps say "change value X" → map to `setX(value)` API call
- Expected result says "value is updated" → verify with `getX()` and compare
- Steps say "enable/disable" → map to `setEnabled("1"/"0")` and verify with `isEnabled()`
- Steps say "read value" → call getter and check non-empty

**These go into: `<plugin>DocTests.js`**

### Category B: Visual Check Required
The test steps can be performed via API but the expected result requires human observation of the UI.

**Indicators:**
- Expected result says "is displayed in the UI"
- Expected result says "plugin loads and is accessible"
- Expected result says "status panels are displayed"
- Expected result says "tab is disabled/enabled" (UI state)
- Steps involve observing colors, layouts, or visual feedback

**These go into: `<plugin>VisualTests.js`**

### Category C: Not Automatable
No API exists for the required action, or the test requires external tools/hardware interaction that can't be scripted.

**Indicators:**
- Steps require file dialogs, drag-and-drop, or mouse interaction with no API equivalent
- Steps require external hardware manipulation (e.g., physically connecting cables)
- Steps require interacting with the OS (e.g., opening a browser)
- No getter/setter exists for the attribute being tested
- Steps require restarting Scopy

**These are skipped — document them as a comment at the top of the DocTests file.**

---

## Step 4: Write the Tests

### File Structure

For each plugin, create two files:

```
js/testAutomations/<plugin-dir>/<plugin>DocTests.js      — Fully automated (Category A)
js/testAutomations/<plugin-dir>/<plugin>VisualTests.js    — Visual validation (Category B)
```

Existing directory structure for reference:
```
js/testAutomations/
├── common/
│   ├── testFramework.js          ← shared framework (ALWAYS load this)
│   └── apiUnregisterTest.js
├── core/
│   ├── connectionTests.js
│   └── ...
├── ad9084/
│   ├── ad9084Tests.js            ← API validation tests (already exist)
│   ├── ad9084DocTests.js         ← doc-matching automated tests
│   └── ad9084VisualTests.js      ← visual validation tests
├── ad936x/
│   ├── ad936xTests.js
│   ├── ad936xDocTests.js
│   ├── ad936xVisualTests.js
│   ├── fmcomms5DocTests.js
│   └── fmcomms5VisualTests.js
├── adrv9002/
│   ├── adrv9002Tests.js
│   ├── adrv9002DocTests.js
│   └── adrv9002VisualTests.js
├── generic-plugins/
│   ├── dataLogger/dataLoggerTests.js
│   ├── debugger/debuggerTests.js
│   └── registerMap/registerMapTests.js
├── m2k/
├── pqm/
└── swiot/
```

### DocTests.js Template (Category A — Fully Automated)

```javascript
/*
 * Copyright (c) 2025 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

// <PLUGIN_NAME> Manual Test Automation
// Automates tests from: docs/tests/plugins/<plugin>/<plugin>_tests.rst
// Tests: <list UIDs of automated tests>
// Skipped: <list UIDs of skipped tests with reason>

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("<PLUGIN_NAME> Documentation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:<DEVICE_IP>")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to plugin tool
if (!switchToTool("<TOOL_NAME>")) {
    printToConsole("ERROR: Cannot switch to <TOOL_NAME> tool");
    scopy.exit();
}

// ============================================
// Test N: <Test Title from docs>
// UID: TST.<PLUGIN>.<TEST_NAME>
// Description: <copy from docs>
// ============================================
printToConsole("\n=== Test N: <Test Title> ===\n");

TestFramework.runTest("TST.<PLUGIN>.<TEST_NAME>", function() {
    try {
        // Save original state (ALWAYS do this before modifying)
        var original = <api_object>.get<Attribute>(<channel>);
        printToConsole("  Original value: " + original);

        // Step 1: Perform action from docs
        <api_object>.set<Attribute>(<channel>, "<new_value>");
        msleep(500);

        // Verify expected result from docs
        var readBack = <api_object>.get<Attribute>(<channel>);
        printToConsole("  After change: " + readBack);
        if (readBack !== "<expected_value>") {
            printToConsole("  FAIL: <describe what went wrong>");
            // Restore before returning
            <api_object>.set<Attribute>(<channel>, original);
            return false;
        }
        printToConsole("  Step 1 PASS: <expected result from docs>");

        // Restore original state (ALWAYS restore)
        <api_object>.set<Attribute>(<channel>, original);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
```

### VisualTests.js Template (Category B — Visual Validation)

```javascript
// ... same copyright header ...

// ============================================================================
// ALL THE FOLLOWING TESTS REQUIRE VISUAL VALIDATION
// These tests automate the steps from the manual test documentation but
// require a human observer to verify UI changes. Each step includes a 3-second
// pause to allow visual inspection of the application state.
// Source: docs/tests/plugins/<plugin>/<plugin>_tests.rst
// ============================================================================

evaluateFile("../js/testAutomations/common/testFramework.js");

TestFramework.init("<PLUGIN_NAME> Visual Validation Tests");

var VISUAL_DELAY = 3000; // 3 seconds for human observation

if (!TestFramework.connectToDevice("ip:<DEVICE_IP>")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

TestFramework.runTest("TST.<PLUGIN>.<TEST_NAME>", function() {
    try {
        // Step from docs
        printToConsole("  <Describe what is happening>...");
        <api_object>.set<Something>(<value>);
        printToConsole("  VISUAL CHECK: <What human should verify in UI>");
        msleep(VISUAL_DELAY);

        // Restore original values
        // ...

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.disconnectFromDevice();
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
```

---

## Step 5: Test Patterns and Rules

### Available Framework Functions

```javascript
// Framework setup
evaluateFile("../js/testAutomations/common/testFramework.js");
TestFramework.init("Suite Name");
TestFramework.connectToDevice("ip:127.0.0.0");  // emulator
TestFramework.connectToDevice("ip:192.168.2.1"); // real device
TestFramework.disconnectFromDevice();
TestFramework.printSummary();

// Running tests
TestFramework.runTest("TST.UID.NAME", function() {
    // return true = PASS
    // return false = FAIL
    // return "SKIP" = SKIP (feature not available)
});

// Assertions
TestFramework.assertEqual(actual, expected, "message");
TestFramework.assertInRange(value, min, max, "message");
TestFramework.assertApproxEqual(actual, expected, tolerance, "message");

// Navigation
switchToTool("ToolName");  // switches to a tool, returns true/false

// Timing
msleep(500);   // 500ms for async API operations
msleep(2000);  // 2s for tool switching
msleep(3000);  // 3s for visual validation pauses

// Output
printToConsole("message");

// App control
scopy.exit();
```

### Critical Rules

1. **ALWAYS save and restore original values** — every setter call must be preceded by reading the original value, and the original must be restored at the end (even in error paths).

2. **Use `msleep(500)` after every setter call** — API operations are asynchronous and need time to propagate to the device.

3. **Use doc UIDs as test names** — the first argument to `TestFramework.runTest()` must be the UID from the documentation (e.g., `"TST.AD9084.CHANNEL_CONFIGURATION"`). For sub-tests, append with a dot (e.g., `"TST.ADRV9002.CONTROLS.RX_CHANNEL_CONFIG.HW_GAIN"`).

4. **All getter return values are strings** — even numeric values come back as strings. Compare with `===` against string values (e.g., `readBack === "18000000"`). For partial matches use `readBack.indexOf("18") !== -1`.

5. **Test for non-empty on read-only attributes** — if the attribute can't be written, just verify the getter returns a non-empty string:
   ```javascript
   var value = api.getSomething();
   if (!value || value === "") return false;
   ```

6. **For write-read tests, follow this pattern:**
   ```javascript
   var original = api.getValue();          // save
   api.setValue("newValue");               // write
   msleep(500);                           // wait
   var readBack = api.getValue();         // read back
   // assert readBack matches expected
   api.setValue(original);                 // restore
   ```

7. **Use `return "SKIP"` for optional features** — if a feature might not exist on all devices (e.g., ORX on ADRV9002), wrap it in try/catch and return "SKIP".

8. **Visual tests get `VISUAL CHECK:` markers** — always prefix visual validation messages with `"VISUAL CHECK: "` so they're easily searchable in output logs.

9. **Do NOT use `exit(1)`** — use `scopy.exit()` for clean shutdown.

10. **Generic widget fallback** — if there's no dedicated API method for an attribute but you know the widget key, use:
    ```javascript
    var keys = api.getWidgetKeys();        // discover all keys
    var value = api.readWidget(keyName);   // read by key
    api.writeWidget(keyName, newValue);    // write by key
    ```

---

## Step 6: Checklist Before Submitting

- [ ] Read the `*_api.h` header to know all available API methods
- [ ] Read the `*_tests.rst` doc to know all manual test steps
- [ ] Classified every doc test as A (automated), B (visual), or C (not automatable)
- [ ] Created `<plugin>DocTests.js` for Category A tests
- [ ] Created `<plugin>VisualTests.js` for Category B tests with banner at top
- [ ] Listed skipped tests (Category C) as comments at top of DocTests file
- [ ] Every test uses the doc UID as its name
- [ ] Every test saves and restores original values
- [ ] Every setter is followed by `msleep(500)`
- [ ] Visual tests use `VISUAL_DELAY = 3000` and `"VISUAL CHECK: "` markers
- [ ] Tests end with `TestFramework.disconnectFromDevice()`, `TestFramework.printSummary()`, and `scopy.exit()`
- [ ] File has the GPLv3 copyright header (copy from existing test files)

---

## Quick Reference: Existing Examples

Study these files for patterns:
- **DocTests example:** `js/testAutomations/ad9084/ad9084DocTests.js`
- **VisualTests example:** `js/testAutomations/adrv9002/adrv9002VisualTests.js`
- **Multi-channel example (4 channels, 2 devices):** `js/testAutomations/ad936x/fmcomms5DocTests.js`
- **API validation example:** `js/testAutomations/ad9084/ad9084Tests.js`
- **Generic plugin example:** `js/testAutomations/generic-plugins/dataLogger/dataLoggerTests.js`
- **Test framework:** `js/testAutomations/common/testFramework.js`
