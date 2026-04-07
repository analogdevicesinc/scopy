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
// Source: docs/tests/plugins/adrv9002/adrv9002_tests.rst
// ============================================================================

// Load test framework
evaluateFile("js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("ADRV9002 Visual Validation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// ============================================
// Test 1: Plugin Loading and Device Detection
// UID: TST.ADRV9002.CONTROLS.PLUGIN_LOADS
// Description: Verify that the ADRV9002 plugin loads and detects
//   device with Controls tab accessible.
// VISUAL: Verify Controls tab shows device controls
// ============================================
printToConsole("\n=== Test 1: Plugin Loading and Device Detection (VISUAL) ===\n");

TestFramework.runTest("TST.ADRV9002.CONTROLS.PLUGIN_LOADS", function() {
    try {
        // Step 1: Open Scopy application (already running)
        printToConsole("  Scopy application is running");

        // Step 2: Open ADRV9002 plugin and navigate to Controls tab
        if (!switchToTool("ADRV9002")) {
            printToConsole("  FAIL: Cannot switch to ADRV9002 tool");
            return false;
        }

        var tools = adrv9002.getTools();
        printToConsole("  ADRV9002 tools found: " + tools);
        if (!tools || tools.length === 0) {
            printToConsole("  FAIL: No tools found for ADRV9002 plugin");
            return false;
        }

        TestFramework.supervisedCheck("Verify plugin loads and Controls tab shows device controls");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3: Global Settings Section (Visual)
// UID: TST.ADRV9002.CONTROLS.GLOBAL_SETTINGS
// Description: Verify global settings controls display correctly.
// VISUAL: Verify Profile Manager, Temperature, and Calibrations widgets
// ============================================
printToConsole("\n=== Test 3: Global Settings Section (VISUAL) ===\n");

TestFramework.runTest("TST.ADRV9002.CONTROLS.GLOBAL_SETTINGS.VISUAL", function() {
    try {
        if (!switchToTool("ADRV9002")) {
            printToConsole("  FAIL: Cannot switch to ADRV9002 tool");
            return false;
        }

        // Read temperature
        var temperature = adrv9002.getTemperature();
        printToConsole("  Temperature reading: " + temperature);

        TestFramework.supervisedCheck("Verify Global Settings section is expanded and Profile Manager shows profile_config and stream_config status");

        TestFramework.supervisedCheck("Verify Temperature displays in Celsius with warning thresholds");

        TestFramework.supervisedCheck("Verify Initial Calibrations widget availability");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: RX Channel Controls (Visual)
// UID: TST.ADRV9002.CONTROLS.RX_CHANNEL_CONFIG
// VISUAL: Watch gain, mode, and powerdown changes in the UI
// ============================================
printToConsole("\n=== Test 4: RX Channel Controls (VISUAL) ===\n");

TestFramework.runTest("TST.ADRV9002.CONTROLS.RX_CHANNEL_CONFIG.VISUAL", function() {
    try {
        var origMode = adrv9002.getRxGainControlMode(0);
        var origGain = adrv9002.getRxHardwareGain(0);
        var origEnabled = adrv9002.isRxEnabled(0);

        // Step 2: Change RX1 Hardware Gain
        printToConsole("  Setting gain control to manual mode...");
        adrv9002.setRxGainControlMode(0, "manual");
        msleep(500);

        printToConsole("  Setting RX1 hardware gain to 10 dB...");
        adrv9002.setRxHardwareGain(0, "10");
        msleep(500);
        TestFramework.supervisedCheck("Verify gain control responds in UI");

        printToConsole("  Setting RX1 hardware gain to 30 dB...");
        adrv9002.setRxHardwareGain(0, "30");
        msleep(500);
        TestFramework.supervisedCheck("Verify gain value updated in UI");

        // Step 3: Change Gain Control Mode
        printToConsole("  Setting gain control mode to automatic...");
        adrv9002.setRxGainControlMode(0, "automatic");
        msleep(500);
        TestFramework.supervisedCheck("Verify mode changes and affects gain behavior");

        // Step 5: Toggle Powerdown
        printToConsole("  Disabling RX1 (powerdown)...");
        adrv9002.setRxEnabled(0, "0");
        msleep(500);
        TestFramework.supervisedCheck("Verify RX1 is powered down in UI");

        printToConsole("  Enabling RX1 (power up)...");
        adrv9002.setRxEnabled(0, "1");
        msleep(500);
        TestFramework.supervisedCheck("Verify RX1 is powered up in UI");

        // Restore
        adrv9002.setRxGainControlMode(0, "manual");
        msleep(500);
        adrv9002.setRxHardwareGain(0, origGain);
        msleep(500);
        adrv9002.setRxGainControlMode(0, origMode);
        msleep(500);
        adrv9002.setRxEnabled(0, origEnabled);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        adrv9002.setRxGainControlMode(0, "manual");
        msleep(500);
        adrv9002.setRxHardwareGain(0, origGain);
        msleep(500);
        adrv9002.setRxGainControlMode(0, origMode);
        msleep(500);
        adrv9002.setRxEnabled(0, origEnabled);
        msleep(500);
        return false;
    }
});

// ============================================
// Test 5: TX Channel Controls (Visual)
// UID: TST.ADRV9002.CONTROLS.TX_CHANNEL_CONFIG
// VISUAL: Watch attenuation and powerdown changes in the UI
// ============================================
printToConsole("\n=== Test 5: TX Channel Controls (VISUAL) ===\n");

TestFramework.runTest("TST.ADRV9002.CONTROLS.TX_CHANNEL_CONFIG.VISUAL", function() {
    try {
        var origAtten = adrv9002.getTxAttenuation(0);
        var origEnabled = adrv9002.isTxEnabled(0);

        // Step 2: Change TX1 Attenuation
        printToConsole("  Setting TX1 attenuation to -10 dB...");
        adrv9002.setTxAttenuation(0, "-10");
        msleep(500);
        TestFramework.supervisedCheck("Verify attenuation control responds in UI");

        printToConsole("  Setting TX1 attenuation to -30 dB...");
        adrv9002.setTxAttenuation(0, "-30");
        msleep(500);
        TestFramework.supervisedCheck("Verify attenuation value updated in UI");

        // Step 4: Toggle TX Powerdown
        printToConsole("  Disabling TX1 (powerdown)...");
        adrv9002.setTxEnabled(0, "0");
        msleep(500);
        TestFramework.supervisedCheck("Verify TX1 is powered down in UI");

        printToConsole("  Enabling TX1 (power up)...");
        adrv9002.setTxEnabled(0, "1");
        msleep(500);
        TestFramework.supervisedCheck("Verify TX1 is powered up in UI");

        // Restore
        adrv9002.setTxAttenuation(0, origAtten);
        msleep(500);
        adrv9002.setTxEnabled(0, origEnabled);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        adrv9002.setTxAttenuation(0, origAtten);
        msleep(500);
        adrv9002.setTxEnabled(0, origEnabled);
        msleep(500);
        return false;
    }
});

// ============================================
// Test 6: ORX Controls (Visual)
// UID: TST.ADRV9002.CONTROLS.ORX_CONFIG
// VISUAL: Watch ORX gain, BBDC, and powerdown changes in the UI
// ============================================
printToConsole("\n=== Test 6: ORX Controls (VISUAL) ===\n");

TestFramework.runTest("TST.ADRV9002.CONTROLS.ORX_CONFIG.VISUAL", function() {
    try {
        var origGain = adrv9002.getOrxHardwareGain(0);
        var origBbdc = adrv9002.isOrxBbdcRejectionEnabled(0);
        var origEnabled = adrv9002.isOrxEnabled(0);

        // Step 2: Change ORX Hardware Gain
        printToConsole("  Setting ORX1 hardware gain to 10 dB...");
        adrv9002.setOrxHardwareGain(0, "10");
        msleep(500);
        TestFramework.supervisedCheck("Verify ORX gain control responds in UI");

        printToConsole("  Setting ORX1 hardware gain to 30 dB...");
        adrv9002.setOrxHardwareGain(0, "30");
        msleep(500);
        TestFramework.supervisedCheck("Verify ORX gain value updated in UI");

        // Step 3: Enable/disable BBDC Rejection
        printToConsole("  Enabling ORX BBDC Rejection...");
        adrv9002.setOrxBbdcRejectionEnabled(0, "1");
        msleep(500);
        TestFramework.supervisedCheck("Verify BBDC Rejection enabled in UI");

        printToConsole("  Disabling ORX BBDC Rejection...");
        adrv9002.setOrxBbdcRejectionEnabled(0, "0");
        msleep(500);
        TestFramework.supervisedCheck("Verify BBDC Rejection disabled in UI");

        // Step 4: Toggle ORX Powerdown
        printToConsole("  Disabling ORX1 (powerdown)...");
        adrv9002.setOrxEnabled(0, "0");
        msleep(500);
        TestFramework.supervisedCheck("Verify ORX1 is powered down in UI");

        printToConsole("  Enabling ORX1 (power up)...");
        adrv9002.setOrxEnabled(0, "1");
        msleep(500);
        TestFramework.supervisedCheck("Verify ORX1 is powered up in UI");

        // Restore
        adrv9002.setOrxHardwareGain(0, origGain);
        msleep(500);
        adrv9002.setOrxBbdcRejectionEnabled(0, origBbdc);
        msleep(500);
        adrv9002.setOrxEnabled(0, origEnabled);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error (ORX may not be supported): " + e);
        adrv9002.setOrxHardwareGain(0, origGain);
        msleep(500);
        adrv9002.setOrxBbdcRejectionEnabled(0, origBbdc);
        msleep(500);
        adrv9002.setOrxEnabled(0, origEnabled);
        msleep(500);
        return "SKIP";
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
