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

// Regmap Plugin Visual Tests (Category B - Supervised)
//
// These tests require human visual verification via supervisedCheck().
//
// Tests:
//   TST.REGMAP.VISUAL.TOOL_LAYOUT        - Verify tool UI layout
//   TST.REGMAP.VISUAL.DEVICE_SWITCH       - Verify device switching updates UI
//   TST.REGMAP.VISUAL.REGISTER_DISPLAY    - Verify register values display correctly
//   TST.REGMAP.VISUAL.SEARCH_HIGHLIGHT    - Verify search highlights matching registers
//   TST.REGMAP.VISUAL.AUTOREAD_INDICATOR  - Verify autoread state indicator
//
// ============================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("Regmap Visual Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to Regmap tool
if (!switchToTool("Register Map")) {
    printToConsole("ERROR: Cannot switch to Register Map tool");
    scopy.exit();
}

// ============================================
// Test: Tool Layout
// UID: TST.REGMAP.VISUAL.TOOL_LAYOUT
// Description: Verify the Register Map tool UI layout
// ============================================
TestFramework.runTest("TST.REGMAP.VISUAL.TOOL_LAYOUT", function() {
    var devices = regmap.getAvailableDevicesName();
    if (!devices || devices.length === 0) {
        return "No devices available";
    }

    regmap.setDevice(devices[0]);
    msleep(500);

    var passed = TestFramework.supervisedCheck(
        "Verify Register Map tool layout: device selector, register list, " +
        "register details panel, and settings menu are all visible and properly arranged."
    );
    return passed ? true : "Visual check failed";
});

// ============================================
// Test: Device Switch
// UID: TST.REGMAP.VISUAL.DEVICE_SWITCH
// Description: Verify device switching updates the UI
// ============================================
TestFramework.runTest("TST.REGMAP.VISUAL.DEVICE_SWITCH", function() {
    var devices = regmap.getAvailableDevicesName();
    if (!devices || devices.length < 2) {
        printToConsole("  Skipping: need at least 2 devices to test switching");
        return "SKIP";
    }

    // Save original device
    regmap.setDevice(devices[0]);
    msleep(500);

    // Switch to second device
    regmap.setDevice(devices[1]);
    msleep(500);

    var passed = TestFramework.supervisedCheck(
        "Verify that the register map display updated to show registers for device: " + devices[1]
    );

    // Restore original device
    regmap.setDevice(devices[0]);
    msleep(500);

    return passed ? true : "Visual check failed";
});

// ============================================
// Test: Register Display
// UID: TST.REGMAP.VISUAL.REGISTER_DISPLAY
// Description: Verify register values display correctly after read
// ============================================
TestFramework.runTest("TST.REGMAP.VISUAL.REGISTER_DISPLAY", function() {
    var devices = regmap.getAvailableDevicesName();
    if (!devices || devices.length === 0) {
        return "No devices available";
    }

    regmap.setDevice(devices[0]);
    msleep(500);

    // Read a few registers to populate the display
    regmap.readInterval("0x0", "0x5");
    msleep(500);

    var passed = TestFramework.supervisedCheck(
        "Verify that registers 0x0 through 0x5 show their read values in the register map display."
    );
    return passed ? true : "Visual check failed";
});

// ============================================
// Test: Search Highlight
// UID: TST.REGMAP.VISUAL.SEARCH_HIGHLIGHT
// Description: Verify search highlights matching registers
// ============================================
TestFramework.runTest("TST.REGMAP.VISUAL.SEARCH_HIGHLIGHT", function() {
    var devices = regmap.getAvailableDevicesName();
    if (!devices || devices.length === 0) {
        return "No devices available";
    }

    regmap.setDevice(devices[0]);
    msleep(500);

    var results = regmap.search("0x0");
    msleep(500);

    var passed = TestFramework.supervisedCheck(
        "Verify that search results for '0x0' are highlighted or filtered in the register list. " +
        "Found " + results.length + " matching register(s)."
    );
    return passed ? true : "Visual check failed";
});

// ============================================
// Test: Autoread Indicator
// UID: TST.REGMAP.VISUAL.AUTOREAD_INDICATOR
// Description: Verify autoread state indicator updates
// ============================================
TestFramework.runTest("TST.REGMAP.VISUAL.AUTOREAD_INDICATOR", function() {
    var devices = regmap.getAvailableDevicesName();
    if (!devices || devices.length === 0) {
        return "No devices available";
    }

    regmap.setDevice(devices[0]);
    msleep(500);

    // Save original state
    var originalState = regmap.isAutoreadEnabled();

    try {
        // Enable autoread
        regmap.enableAutoread(true);
        msleep(500);

        var passed = TestFramework.supervisedCheck(
            "Verify that the autoread indicator shows 'enabled' state and register values are being updated automatically."
        );

        // Disable autoread
        regmap.enableAutoread(false);
        msleep(500);

        if (passed) {
            passed = TestFramework.supervisedCheck(
                "Verify that the autoread indicator now shows 'disabled' state and register values stop updating."
            );
        }

        // Restore original state
        regmap.enableAutoread(originalState);
        msleep(500);

        return passed ? true : "Visual check failed";
    } catch (e) {
        regmap.enableAutoread(originalState);
        msleep(500);
        return "Error during autoread visual test: " + e;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();
TestFramework.printSummary();
scopy.exit();
