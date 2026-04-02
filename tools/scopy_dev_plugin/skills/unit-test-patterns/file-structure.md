# Unit Test File Structure & Boilerplate

## File Naming Convention

| File | Purpose | When to create |
|------|---------|----------------|
| `<plugin>_Basic_Unit_test.js` | Main/basic tool widget tests | Always (if multiple files) |
| `<plugin>_Advanced_Unit_test.js` | Advanced tab widget tests | Plugin has advanced tool |
| `<plugin>_Complex_Unit_test.js` | Multi-step functionality tests | Plugin has complex scenarios |
| `<plugin>_Unit_test.js` | Combined runner OR single file | Always |

## Standalone File Template

Every standalone unit test file follows this structure:

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

// <Plugin> <Type> Unit Test Automation
// Tests <description of what this file covers>

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("<Plugin> <Type> Unit Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Widget key prefix
var PHY = "<device-name>/";

// Initial setup (e.g., set device to active mode)
<api>.setEnsmMode("radio_on");
msleep(1000);

// Feature detection (if needed)
var hasFeatureX = (<api>.readWidget(PHY + "feature_attribute") !== null);

// ============================================
// Helper Functions
// ============================================

// ... paste all standard helpers here ...
// testRange, testCombo, testCheckbox, testReadOnly,
// testConversion, testBadValueRange, testBadValueCombo,
// runDataDrivenTests

// ============================================
// SECTION: <Section Name>
// ============================================

// Switch to tool
switchToTool("<ToolName>");
msleep(500);

// --- <Subsection> ---
TestFramework.runTest("UNIT.<SECTION>.<ATTRIBUTE>", function() {
    // ... test logic ...
});

// ... more tests ...

// ============================================
// Cleanup
// ============================================
TestFramework.disconnectFromDevice();
TestFramework.printSummary();
scopy.exit();
```

## Combined Runner Template

When multiple sub-files exist, the combined runner is a thin wrapper:

```javascript
/*
 * Copyright (c) 2025 Analog Devices Inc.
 * ... (same license header) ...
 */

// <Plugin> Combined Unit Test Runner
// Runs all unit test sub-files in sequence

evaluateFile("../js/testAutomations/<plugin>/<plugin>_Basic_Unit_test.js");
evaluateFile("../js/testAutomations/<plugin>/<plugin>_Advanced_Unit_test.js");
evaluateFile("../js/testAutomations/<plugin>/<plugin>_Complex_Unit_test.js");
```

## UID Format

```
UNIT.<SECTION>.<ATTRIBUTE_NAME>
```

- `UNIT` — always the prefix for unit tests
- `SECTION` — uppercase section name (e.g., `RX`, `TX`, `OBS`, `CLK`, `DPD`, `CAL`, `GLOBAL`, `FPGA`, `UTIL`, `BAD`)
- `ATTRIBUTE_NAME` — uppercase snake_case derived from the attribute (e.g., `CH0_HARDWARE_GAIN`, `LO_FREQUENCY`)

Examples:
- `UNIT.RX.CH0_HARDWARE_GAIN`
- `UNIT.TX.LO_FREQUENCY`
- `UNIT.DPD.TRACKING_EN`
- `UNIT.BAD.RX_HARDWARE_GAIN_RANGE`
- `UNIT.CAL.FULL_CALIBRATION_FLOW`

## Widget Key Prefix Convention

```javascript
var PHY = "<device-name>/";  // e.g., "ad9371-phy/"
```

The device name comes from the EMU XML `<device>` tag's `id` attribute.

## Termination Sequence (non-negotiable)

Every standalone file must end with exactly:

```javascript
TestFramework.disconnectFromDevice();
TestFramework.printSummary();
scopy.exit();
```

In this exact order. No code after `scopy.exit()`.
