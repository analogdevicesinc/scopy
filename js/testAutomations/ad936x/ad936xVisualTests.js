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
// Source: docs/tests/plugins/ad936x/ad936x_tests.rst
// ============================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("AD936x Visual Validation Tests");

var VISUAL_DELAY = 3000; // 3 seconds for human observation

// Connect to device
if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// ============================================
// Test 1: Plugin Loads
// UID: TST.AD936X.PLUGIN_LOADS
// Description: Verify that the AD936x plugin loads in Scopy.
// VISUAL: Verify plugin is visible and accessible in the UI
// ============================================
printToConsole("\n=== Test 1: Plugin Loads (VISUAL) ===\n");

TestFramework.runTest("TST.AD936X.PLUGIN_LOADS", function() {
    try {
        // Step 1: Open Scopy application (already running)
        printToConsole("  Scopy application is running");
        printToConsole("  VISUAL CHECK: Verify Scopy launched without errors");
        msleep(VISUAL_DELAY);

        // Step 2: Open AD936x plugin
        if (!switchToTool("AD936X")) {
            printToConsole("  FAIL: Cannot switch to AD936X tool");
            return false;
        }

        var tools = ad936x.getTools();
        printToConsole("  AD936x tools found: " + tools);
        if (!tools || tools.length === 0) {
            printToConsole("  FAIL: No tools found for AD936x plugin");
            return false;
        }

        printToConsole("  VISUAL CHECK: Verify AD936x plugin loads and is accessible in the UI");
        msleep(VISUAL_DELAY);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2: Device Detection and Display
// UID: TST.AD936X.DEVICE_DETECTION_AND_DISPLAY
// Description: Verify that the AD936x plugin detects and displays
//   the connected AD936x device and its status panels.
// VISUAL: Verify device status panels are displayed correctly
// ============================================
printToConsole("\n=== Test 2: Device Detection and Display (VISUAL) ===\n");

TestFramework.runTest("TST.AD936X.DEVICE_DETECTION_AND_DISPLAY", function() {
    try {
        // Step 1: Open AD936x plugin
        if (!switchToTool("AD936X")) {
            printToConsole("  FAIL: Cannot switch to AD936X tool");
            return false;
        }

        var keys = ad936x.getWidgetKeys();
        printToConsole("  Widget keys available: " + keys.length);
        if (!keys || keys.length === 0) {
            printToConsole("  FAIL: No widgets detected - device may not be connected");
            return false;
        }

        // Display some key device attributes for verification
        var ensmMode = ad936x.getEnsmMode();
        var rxBw = ad936x.getRxRfBandwidth();
        var txBw = ad936x.getTxRfBandwidth();
        printToConsole("  ENSM Mode: " + ensmMode);
        printToConsole("  RX RF Bandwidth: " + rxBw);
        printToConsole("  TX RF Bandwidth: " + txBw);

        printToConsole("  VISUAL CHECK: Verify device status panels are displayed correctly");
        printToConsole("  VISUAL CHECK: Verify ENSM mode, RX/TX bandwidth values match the UI");
        msleep(VISUAL_DELAY);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3: Global Settings (Visual)
// UID: TST.AD936X.CHANGE_VALIDATE_GLOBAL_SETTINGS
// VISUAL: Watch ENSM mode and rate governor changes in the UI
// ============================================
printToConsole("\n=== Test 3: Change and Validate Global Settings (VISUAL) ===\n");

TestFramework.runTest("TST.AD936X.CHANGE_VALIDATE_GLOBAL_SETTINGS.VISUAL", function() {
    try {
        var origEnsm = ad936x.getEnsmMode();
        var origGov = ad936x.getTrxRateGovernor();

        // Step 1: Change ENSM mode
        printToConsole("  Setting ENSM mode to fdd...");
        ad936x.setEnsmMode("fdd");
        printToConsole("  VISUAL CHECK: Verify ENSM mode changes to FDD in the UI");
        msleep(VISUAL_DELAY);

        // Step 2: Change rate governor
        printToConsole("  Setting TRX rate governor to nominal...");
        ad936x.setTrxRateGovernor("nominal");
        printToConsole("  VISUAL CHECK: Verify rate governor changes in the UI");
        msleep(VISUAL_DELAY);

        // Restore
        ad936x.setEnsmMode(origEnsm);
        ad936x.setTrxRateGovernor(origGov);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: RX and TX Chain Configuration (Visual)
// UID: TST.AD936X.RX_TX_CHAIN_CONFIG
// VISUAL: Watch BW, sampling rate, and LO changes in the UI
// ============================================
printToConsole("\n=== Test 4: RX and TX Chain Configuration (VISUAL) ===\n");

TestFramework.runTest("TST.AD936X.RX_TX_CHAIN_CONFIG.VISUAL", function() {
    try {
        var origRxBw = ad936x.getRxRfBandwidth();
        var origRxSr = ad936x.getRxSamplingFrequency();
        var origTxBw = ad936x.getTxRfBandwidth();
        var origTxLo = ad936x.getTxLoFrequency();

        // Step 1: Change RX bandwidth and sampling rate
        printToConsole("  Setting RX RF bandwidth to 18 MHz...");
        ad936x.setRxRfBandwidth("18000000");
        printToConsole("  VISUAL CHECK: Verify RX bandwidth updated in UI");
        msleep(VISUAL_DELAY);

        printToConsole("  Setting RX sampling frequency to 25 MHz...");
        ad936x.setRxSamplingFrequency("25000000");
        printToConsole("  VISUAL CHECK: Verify RX sampling frequency updated in UI");
        msleep(VISUAL_DELAY);

        // Step 2: Change TX bandwidth and LO frequency
        printToConsole("  Setting TX RF bandwidth to 18 MHz...");
        ad936x.setTxRfBandwidth("18000000");
        printToConsole("  VISUAL CHECK: Verify TX bandwidth updated in UI");
        msleep(VISUAL_DELAY);

        printToConsole("  Setting TX LO frequency to 2.4 GHz...");
        ad936x.setTxLoFrequency("2400000000");
        printToConsole("  VISUAL CHECK: Verify TX LO frequency updated in UI");
        msleep(VISUAL_DELAY);

        // Restore
        ad936x.setRxRfBandwidth(origRxBw);
        ad936x.setRxSamplingFrequency(origRxSr);
        ad936x.setTxRfBandwidth(origTxBw);
        ad936x.setTxLoFrequency(origTxLo);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 5: Advanced Plugin Detection and Display (Visual)
// UID: TST.AD936X_ADVANCED.PLUGIN_DETECTION_AND_DISPLAY
// VISUAL: Verify Advanced plugin panels are displayed
// ============================================
printToConsole("\n=== Test 5: Advanced Plugin Detection and Display (VISUAL) ===\n");

TestFramework.runTest("TST.AD936X_ADVANCED.PLUGIN_DETECTION_AND_DISPLAY.VISUAL", function() {
    try {
        // Step 1: Open AD936x Advanced plugin
        if (!switchToTool("AD936X Advanced")) {
            printToConsole("  FAIL: Cannot switch to AD936X Advanced tool");
            return false;
        }

        printToConsole("  VISUAL CHECK: Verify AD936x Advanced plugin is loaded");
        msleep(VISUAL_DELAY);

        // Navigate through subtabs for visual verification
        ad936x_advanced.switchSubtab("ENSM/Mode/Clocks");
        printToConsole("  VISUAL CHECK: Verify ENSM/Mode/Clocks panel displays correctly");
        msleep(VISUAL_DELAY);

        ad936x_advanced.switchSubtab("Gain");
        printToConsole("  VISUAL CHECK: Verify Gain panel displays correctly");
        msleep(VISUAL_DELAY);

        ad936x_advanced.switchSubtab("RSSI");
        printToConsole("  VISUAL CHECK: Verify RSSI panel displays correctly");
        msleep(VISUAL_DELAY);

        ad936x_advanced.switchSubtab("TX Monitor");
        printToConsole("  VISUAL CHECK: Verify TX Monitor panel displays correctly");
        msleep(VISUAL_DELAY);

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
