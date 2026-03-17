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
// require a human observer to verify UI changes. Each step pauses for
// supervised human validation before continuing.
// Source: docs/tests/plugins/fmcomms5/fmcomms5_tests.rst
// Note: FMCOMMS5 has two physical AD9361 devices (ad9361-phy and ad9361-phy-B)
//   Channels 0-1 map to ad9361-phy, channels 2-3 map to ad9361-phy-B
// ============================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("FMCOMMS5 Visual Validation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// ============================================
// Test 1: Plugin Loads
// UID: TST.FMCOMMS5.PLUGIN_LOADS
// Description: Verify that the FMCOMMS5 plugin loads in Scopy.
// VISUAL: Verify plugin is visible and accessible
// ============================================
printToConsole("\n=== Test 1: Plugin Loads (VISUAL) ===\n");

TestFramework.runTest("TST.FMCOMMS5.PLUGIN_LOADS", function() {
    try {
        // Step 1: Open Scopy application (already running)
        printToConsole("  Scopy application is running");
        TestFramework.supervisedCheck("Verify Scopy launched without errors");

        // Step 2: Open FMCOMMS5 plugin
        if (!switchToTool("FMCOMMS5")) {
            printToConsole("  FAIL: Cannot switch to FMCOMMS5 tool");
            return false;
        }

        var tools = fmcomms5.getTools();
        printToConsole("  FMCOMMS5 tools found: " + tools);
        if (!tools || tools.length === 0) {
            printToConsole("  FAIL: No tools found for FMCOMMS5 plugin");
            return false;
        }

        TestFramework.supervisedCheck("Verify FMCOMMS5 plugin loads and is accessible in the UI");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2: Device Detection and Display
// UID: TST.FMCOMMS5.DEVICE_DETECTION_AND_DISPLAY
// Description: Verify that the FMCOMMS5 plugin detects and displays
//   both AD9361-phy and AD9361-phy-B devices and their status panels.
// VISUAL: Verify both devices' panels are displayed
// ============================================
printToConsole("\n=== Test 2: Device Detection and Display (VISUAL) ===\n");

TestFramework.runTest("TST.FMCOMMS5.DEVICE_DETECTION_AND_DISPLAY", function() {
    try {
        if (!switchToTool("FMCOMMS5")) {
            printToConsole("  FAIL: Cannot switch to FMCOMMS5 tool");
            return false;
        }

        var keys = fmcomms5.getWidgetKeys();
        printToConsole("  Widget keys available: " + keys.length);
        if (!keys || keys.length === 0) {
            printToConsole("  FAIL: No widgets detected");
            return false;
        }

        // Check for both devices by looking for widget keys from both
        var hasPhyA = false;
        var hasPhyB = false;
        for (var i = 0; i < keys.length; i++) {
            if (keys[i].indexOf("ad9361-phy/") !== -1 && keys[i].indexOf("ad9361-phy-B") === -1) {
                hasPhyA = true;
            }
            if (keys[i].indexOf("ad9361-phy-B") !== -1) {
                hasPhyB = true;
            }
        }
        printToConsole("  ad9361-phy detected: " + hasPhyA);
        printToConsole("  ad9361-phy-B detected: " + hasPhyB);

        // Show device attributes from both
        var ensmMode = fmcomms5.getEnsmMode();
        printToConsole("  ENSM Mode: " + ensmMode);

        // Read gains from all 4 channels
        for (var ch = 0; ch < 4; ch++) {
            var gain = fmcomms5.getRxHardwareGain(ch);
            printToConsole("  RX ch" + ch + " gain: " + gain +
                " (" + (ch < 2 ? "ad9361-phy" : "ad9361-phy-B") + ")");
        }

        TestFramework.supervisedCheck("Verify both AD9361-phy and AD9361-phy-B status panels are displayed");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3: Change and Validate Global Settings (Visual)
// UID: TST.FMCOMMS5.CHANGE_VALIDATE_GLOBAL_SETTINGS
// VISUAL: Watch ENSM and governor changes in the UI for both devices
// ============================================
printToConsole("\n=== Test 3: Global Settings (VISUAL) ===\n");

TestFramework.runTest("TST.FMCOMMS5.CHANGE_VALIDATE_GLOBAL_SETTINGS", function() {
    var origEnsm, origGov;
    try {
        origEnsm = fmcomms5.getEnsmMode();
        origGov = fmcomms5.getTrxRateGovernor();

        // Step 1: Change ENSM mode
        printToConsole("  Setting ENSM mode to fdd...");
        fmcomms5.setEnsmMode("fdd");
        msleep(500);
        TestFramework.supervisedCheck("Verify ENSM mode changes to FDD for both devices");

        // Step 2: Change rate governor
        printToConsole("  Setting TRX rate governor to nominal...");
        fmcomms5.setTrxRateGovernor("nominal");
        msleep(500);
        TestFramework.supervisedCheck("Verify rate governor changes for both devices");

        // Restore
        fmcomms5.setEnsmMode(origEnsm);
        msleep(500);
        fmcomms5.setTrxRateGovernor(origGov);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origEnsm) { fmcomms5.setEnsmMode(origEnsm); msleep(500); }
        if (origGov) { fmcomms5.setTrxRateGovernor(origGov); msleep(500); }
        return false;
    }
});

// ============================================
// Test 4: RX and TX Chain Configuration (Visual)
// UID: TST.FMCOMMS5.RX_TX_CHAIN_CONFIG
// VISUAL: Watch BW, LO, and gain changes for all 4 channels
// ============================================
printToConsole("\n=== Test 4: RX and TX Chain Configuration (VISUAL) ===\n");

TestFramework.runTest("TST.FMCOMMS5.RX_TX_CHAIN_CONFIG", function() {
    var origRxBw, origTxBw, origRxLo0, origRxLo1;
    try {
        origRxBw = fmcomms5.getRxRfBandwidth();
        origTxBw = fmcomms5.getTxRfBandwidth();
        origRxLo0 = fmcomms5.getRxLoFrequency(0);
        origRxLo1 = fmcomms5.getRxLoFrequency(1);

        // Step 1: Change RX bandwidth
        printToConsole("  Setting RX RF bandwidth to 18 MHz...");
        fmcomms5.setRxRfBandwidth("18000000");
        msleep(500);
        TestFramework.supervisedCheck("Verify RX bandwidth updated for all RX channels");

        // Change RX LO for both devices
        printToConsole("  Setting RX LO to 2.4 GHz for device 0 (channels 0-1)...");
        fmcomms5.setRxLoFrequency(0, "2400000000");
        msleep(500);
        TestFramework.supervisedCheck("Verify RX LO updated for device 0");

        printToConsole("  Setting RX LO to 2.4 GHz for device 1 (channels 2-3)...");
        fmcomms5.setRxLoFrequency(1, "2400000000");
        msleep(500);
        TestFramework.supervisedCheck("Verify RX LO updated for device 1");

        // Step 2: Change TX bandwidth
        printToConsole("  Setting TX RF bandwidth to 18 MHz...");
        fmcomms5.setTxRfBandwidth("18000000");
        msleep(500);
        TestFramework.supervisedCheck("Verify TX bandwidth updated for all TX channels");

        // Restore
        fmcomms5.setRxRfBandwidth(origRxBw);
        msleep(500);
        fmcomms5.setTxRfBandwidth(origTxBw);
        msleep(500);
        fmcomms5.setRxLoFrequency(0, origRxLo0);
        msleep(500);
        fmcomms5.setRxLoFrequency(1, origRxLo1);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origRxBw) { fmcomms5.setRxRfBandwidth(origRxBw); msleep(500); }
        if (origTxBw) { fmcomms5.setTxRfBandwidth(origTxBw); msleep(500); }
        if (origRxLo0) { fmcomms5.setRxLoFrequency(0, origRxLo0); msleep(500); }
        if (origRxLo1) { fmcomms5.setRxLoFrequency(1, origRxLo1); msleep(500); }
        return false;
    }
});

// ============================================
// Test 5: Advanced Plugin Detection and Display (Visual)
// UID: TST.FMCOMMS5_ADVANCED.PLUGIN_DETECTION_AND_DISPLAY
// VISUAL: Verify Advanced plugin panels are displayed
// ============================================
printToConsole("\n=== Test 5: Advanced Plugin Detection and Display (VISUAL) ===\n");

TestFramework.runTest("TST.FMCOMMS5_ADVANCED.PLUGIN_DETECTION_AND_DISPLAY", function() {
    try {
        if (!switchToTool("FMCOMMS5 Advanced")) {
            printToConsole("  FAIL: Cannot switch to FMCOMMS5 Advanced tool");
            return false;
        }

        TestFramework.supervisedCheck("Verify FMCOMMS5 Advanced plugin is loaded");

        // Navigate through subtabs
        fmcomms5_advanced.switchSubtab("ENSM/Mode/Clocks");
        msleep(500);
        TestFramework.supervisedCheck("Verify ENSM/Mode/Clocks panel displays correctly");

        fmcomms5_advanced.switchSubtab("Gain");
        msleep(500);
        TestFramework.supervisedCheck("Verify Gain panel displays correctly");

        fmcomms5_advanced.switchSubtab("FMCOMMS5");
        msleep(500);
        TestFramework.supervisedCheck("Verify FMCOMMS5 calibration panel displays correctly");

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
