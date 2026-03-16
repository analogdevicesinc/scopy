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

// ============================================================================
// ALL THE FOLLOWING TESTS REQUIRE VISUAL VALIDATION
// These tests automate the steps from the manual test documentation but
// require a human observer to verify UI changes. Each step includes a 3-second
// pause to allow visual inspection of the application state.
// Source: docs/tests/plugins/ad9084/ad9084_tests.rst
// ============================================================================

// Load test framework
evaluateFile("js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("AD9084 Visual Validation Tests");

var VISUAL_DELAY = 3000; // 3 seconds for human observation

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// ============================================
// Test 1: Plugin Loads
// UID: TST.AD9084.PLUGIN_LOADS
// Description: Verify that the AD9084 plugin loads in Scopy
//   and is accessible in the UI.
// VISUAL: Verify AD9084 plugin is visible and accessible in the UI
// ============================================
printToConsole("\n=== Test 1: Plugin Loads (VISUAL) ===\n");

TestFramework.runTest("TST.AD9084.PLUGIN_LOADS", function() {
    try {
        // Step 1: Open Scopy application (already running)
        printToConsole("  Scopy application is running");

        // Step 2: Open AD9084 plugin
        if (!switchToTool("AD9084")) {
            printToConsole("  FAIL: Cannot switch to AD9084 tool");
            return false;
        }

        var tools = ad9084.getTools();
        printToConsole("  AD9084 tools found: " + tools);
        if (!tools || tools.length === 0) {
            printToConsole("  FAIL: No tools found for AD9084 plugin");
            return false;
        }

        printToConsole("  VISUAL CHECK: Verify AD9084 plugin is visible and accessible in the UI");
        msleep(VISUAL_DELAY);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2: Device Detection and Display
// UID: TST.AD9084.DEVICE_DETECTION_AND_DISPLAY
// Description: Verify that the plugin detects and displays
//   the connected AD9084 device and its status panels.
// VISUAL: Verify device status panels are displayed correctly
// ============================================
printToConsole("\n=== Test 2: Device Detection and Display (VISUAL) ===\n");

TestFramework.runTest("TST.AD9084.DEVICE_DETECTION_AND_DISPLAY", function() {
    try {
        // Step 1: Open AD9084 plugin
        if (!switchToTool("AD9084")) {
            printToConsole("  FAIL: Cannot switch to AD9084 tool");
            return false;
        }

        var keys = ad9084.getWidgetKeys();
        printToConsole("  Widget keys available: " + keys.length);
        if (!keys || keys.length === 0) {
            printToConsole("  FAIL: No widgets detected - device may not be connected");
            return false;
        }

        // Print some widget keys for reference
        for (var i = 0; i < Math.min(keys.length, 10); i++) {
            printToConsole("  Widget: " + keys[i]);
        }
        if (keys.length > 10) {
            printToConsole("  ... and " + (keys.length - 10) + " more widgets");
        }

        printToConsole("  VISUAL CHECK: Verify device status panels are displayed correctly");
        msleep(VISUAL_DELAY);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3: Channel Configuration (Visual)
// UID: TST.AD9084.CHANNEL_CONFIGURATION
// Description: Enable and disable individual channels;
//   verify that channel status and data display update accordingly.
// VISUAL: Watch channel status and data display update in the UI
// ============================================
printToConsole("\n=== Test 3: Channel Configuration (VISUAL) ===\n");

TestFramework.runTest("TST.AD9084.CHANNEL_CONFIGURATION.VISUAL", function() {
    try {
        var ch = 0;
        var originalRx = ad9084.isRxEnabled(ch);
        var originalTx = ad9084.isTxEnabled(ch);

        // Step 1: Enable RX channel
        printToConsole("  Enabling RX channel " + ch + "...");
        ad9084.setRxEnabled(ch, "1");
        printToConsole("  VISUAL CHECK: Verify RX channel " + ch + " is enabled in the UI");
        msleep(VISUAL_DELAY);

        // Step 2: Disable RX channel
        printToConsole("  Disabling RX channel " + ch + "...");
        ad9084.setRxEnabled(ch, "0");
        printToConsole("  VISUAL CHECK: Verify RX channel " + ch + " is disabled in the UI");
        msleep(VISUAL_DELAY);

        // Step 3: Enable TX channel
        printToConsole("  Enabling TX channel " + ch + "...");
        ad9084.setTxEnabled(ch, "1");
        printToConsole("  VISUAL CHECK: Verify TX channel " + ch + " is enabled in the UI");
        msleep(VISUAL_DELAY);

        // Step 4: Disable TX channel
        printToConsole("  Disabling TX channel " + ch + "...");
        ad9084.setTxEnabled(ch, "0");
        printToConsole("  VISUAL CHECK: Verify TX channel " + ch + " is disabled in the UI");
        msleep(VISUAL_DELAY);

        // Restore original states
        ad9084.setRxEnabled(ch, originalRx);
        ad9084.setTxEnabled(ch, originalTx);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: Disable and Enable RX/TX Tabs (Visual)
// UID: TST.AD9084.DISABLE_ENABLE_RX_TX_TABS
// Description: Disable then enable RX and TX tabs;
//   verify that tab status and controls update accordingly.
// VISUAL: Watch tab enable/disable state changes in the UI
// ============================================
printToConsole("\n=== Test 4: Disable and Enable RX/TX Tabs (VISUAL) ===\n");

TestFramework.runTest("TST.AD9084.DISABLE_ENABLE_RX_TX_TABS.VISUAL", function() {
    try {
        var ch = 0;
        var origRx = ad9084.isRxEnabled(ch);
        var origTx = ad9084.isTxEnabled(ch);

        // Step 1: Disable RX tab
        printToConsole("  Disabling RX tab (channel " + ch + ")...");
        ad9084.setRxEnabled(ch, "0");
        printToConsole("  VISUAL CHECK: Verify RX tab is disabled and controls are inaccessible");
        msleep(VISUAL_DELAY);

        // Step 2: Disable TX tab
        printToConsole("  Disabling TX tab (channel " + ch + ")...");
        ad9084.setTxEnabled(ch, "0");
        printToConsole("  VISUAL CHECK: Verify TX tab is disabled and controls are inaccessible");
        msleep(VISUAL_DELAY);

        // Step 3: Enable RX tab
        printToConsole("  Enabling RX tab (channel " + ch + ")...");
        ad9084.setRxEnabled(ch, "1");
        printToConsole("  VISUAL CHECK: Verify RX tab is enabled and controls are accessible");
        msleep(VISUAL_DELAY);

        // Step 4: Enable TX tab
        printToConsole("  Enabling TX tab (channel " + ch + ")...");
        ad9084.setTxEnabled(ch, "1");
        printToConsole("  VISUAL CHECK: Verify TX tab is enabled and controls are accessible");
        msleep(VISUAL_DELAY);

        // Restore original states
        ad9084.setRxEnabled(ch, origRx);
        ad9084.setTxEnabled(ch, origTx);
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
