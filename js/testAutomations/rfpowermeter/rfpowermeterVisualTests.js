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
// ALL THE FOLLOWING TESTS REQUIRE SUPERVISED VISUAL VALIDATION
// These tests automate the steps from the manual test documentation but
// require a human observer to verify UI changes. After each visual check,
// the test will prompt the user to input 'y' (pass) or 'n' (fail).
// Source: docs/tests/plugins/rfpowermeter/rfpowermeter_tests.rst
//
// NOTE: The RF Power Meter plugin does not have its own JS API object.
//       It configures the DataLogger plugin, so tests use the "datalogger"
//       JS API object.
//
// NOTE: These tests require a device with "powrms" IIO interface.
//       The emulator may not support this device.
//
// Tests:
//   TST.RFPOWERMETER.PLUGIN_DETECTION - Plugin detection and compatibility (Test 1)
//   TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION - Automatic DataLogger configuration (Test 3)
//   TST.RFPOWERMETER.FREQUENCY_CONTROL - Frequency control widget (Test 4)
//
// Skipped (Category C - Not Automatable):
//   TST.RFPOWERMETER.DATALOGGER_DEPENDENCY - Requires disabling/enabling plugins
//     via plugin manager (no JS API available)
// ============================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("RF Power Meter Visual Validation Tests");

// Connect to device (requires powrms IIO interface)
if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    printToConsole("NOTE: RF Power Meter requires a device with 'powrms' IIO interface");
    scopy.exit();
}

// ============================================
// Test 1: Plugin Detection and Compatibility
// UID: TST.RFPOWERMETER.PLUGIN_DETECTION
// Description: Verify that the RF Power Meter plugin
//   is detected and only activates for compatible devices.
// Preconditions: RF Power Meter plugin is installed,
//   Data Logger plugin is enabled.
// ============================================
printToConsole("\n=== Test 1: Plugin Detection and Compatibility (SUPERVISED) ===\n");

TestFramework.runTest("TST.RFPOWERMETER.PLUGIN_DETECTION", function() {
    try {
        // Step 1: Connected to compatible RF Power Meter device
        printToConsole("  Step 1: Connected to device with powrms IIO interface");
        var toolList = datalogger.getToolList();
        printToConsole("  DataLogger tools: " + toolList);

        if (toolList.indexOf("RF Power Meter") === -1) {
            printToConsole("  FAIL: RF Power Meter not detected for this device");
            printToConsole("  Ensure the connected device has 'powrms' IIO interface");
            return false;
        }

        if (!TestFramework.supervisedCheck(
            "Verify RF Power Meter plugin is automatically detected and available")) {
            return false;
        }

        // Step 2: Verify plugin appears in the device plugin list
        printToConsole("  Step 2: Verifying plugin in device plugin list...");

        if (!TestFramework.supervisedCheck(
            "Verify RF Power Meter plugin is listed as active/compatible " +
            "in the device plugin list on the left panel")) {
            return false;
        }

        // Step 3: Test with incompatible device (manual step - cannot be automated)
        printToConsole("  Step 3: MANUAL STEP - To verify incompatibility:");
        printToConsole("    - Disconnect this device");
        printToConsole("    - Connect to a device WITHOUT 'powrms' interface (e.g., ADALM-PLUTO)");
        printToConsole("    - Expected: RF Power Meter plugin does not appear");
        printToConsole("  (This step cannot be automated - requires different hardware)");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3: Automatic Data Logger Configuration
// UID: TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION
// Description: Verify that RF Power Meter automatically
//   configures Data Logger with correct settings.
// Preconditions: Connected to compatible device,
//   DataLogger dependency is passed.
// ============================================
printToConsole("\n=== Test 3: Automatic Data Logger Configuration (SUPERVISED) ===\n");

TestFramework.runTest("TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION", function() {
    try {
        // Switch to RF Power Meter tool
        if (!switchToTool("RF Power Meter")) {
            printToConsole("  FAIL: Cannot switch to RF Power Meter tool");
            return false;
        }

        // Step 1: Verify tool name and display mode
        printToConsole("  Step 1: Verifying tool name and display mode...");

        if (!TestFramework.supervisedCheck(
            "Verify tool name is 'RF Power Meter' (not 'Data Logger')")) {
            return false;
        }

        if (!TestFramework.supervisedCheck(
            "Verify display mode is set to seven-segment display " +
            "(large numeric readouts should be visible, not plot or text mode)")) {
            return false;
        }

        // Step 2: Verify monitor configuration
        printToConsole("  Step 2: Verifying monitor configuration...");

        // Enable power monitors to make them visible
        datalogger.enableMonitorOfTool("RF Power Meter", "powrms:power5");
        msleep(500);
        datalogger.enableMonitorOfTool("RF Power Meter", "powrms:power6");
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Verify 'Power Forward' (powrms:power5) monitor is enabled " +
            "and configured with 'dBm' units on the seven-segment display")) {
            return false;
        }

        if (!TestFramework.supervisedCheck(
            "Verify 'Power Reverse' (powrms:power6) monitor is enabled " +
            "and configured with 'dBm' units on the seven-segment display")) {
            return false;
        }

        // Start acquisition briefly to show live values
        printToConsole("  Starting data acquisition for visual verification...");
        datalogger.setRunning(true);
        msleep(3000);

        if (!TestFramework.supervisedCheck(
            "Verify power readings are displayed in dBm units " +
            "on the seven-segment display with live updates")) {
            datalogger.setRunning(false);
            msleep(500);
            return false;
        }

        datalogger.setRunning(false);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: Frequency Control Widget
// UID: TST.RFPOWERMETER.FREQUENCY_CONTROL
// Description: Verify that frequency control widget is
//   properly integrated and functional.
// Preconditions: Connected to compatible device,
//   automatic configuration is passed.
// ============================================
printToConsole("\n=== Test 4: Frequency Control Widget (SUPERVISED) ===\n");

TestFramework.runTest("TST.RFPOWERMETER.FREQUENCY_CONTROL", function() {
    try {
        // Ensure we're on the RF Power Meter tool
        if (!switchToTool("RF Power Meter")) {
            printToConsole("  FAIL: Cannot switch to RF Power Meter tool");
            return false;
        }

        // Step 1: Locate frequency control widget
        printToConsole("  Step 1: Locating frequency control widget...");

        if (!TestFramework.supervisedCheck(
            "Verify frequency control widget is visible in the seven-segment " +
            "display area (should show 'frequency_MHz' attribute control)")) {
            return false;
        }

        // Step 2: Modify frequency setting
        printToConsole("  Step 2: Frequency modification...");
        printToConsole("    Please manually modify the frequency value using the widget.");

        if (!TestFramework.supervisedCheck(
            "After modifying the frequency value: verify that the frequency " +
            "value can be changed and changes are applied to the device")) {
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test: Display Modes Verification
// UID: TST.RFPOWERMETER.DISPLAY_MODES
// Description: Verify that display mode switching works
//   correctly for the RF Power Meter tool.
// ============================================
printToConsole("\n=== Display Modes Verification (SUPERVISED) ===\n");

TestFramework.runTest("TST.RFPOWERMETER.DISPLAY_MODES", function() {
    try {
        if (!switchToTool("RF Power Meter")) {
            printToConsole("  FAIL: Cannot switch to RF Power Meter tool");
            return false;
        }

        // Save original display mode (seven-segment = 2 is default for RF Power Meter)
        var originalMode = 2;

        // Ensure monitors are enabled
        datalogger.enableMonitorOfTool("RF Power Meter", "powrms:power5");
        datalogger.enableMonitorOfTool("RF Power Meter", "powrms:power6");
        msleep(500);

        // Seven Segment mode (should be default for RF Power Meter)
        printToConsole("  Switching to Seven Segment display mode...");
        datalogger.setDisplayMode("RF Power Meter", 2);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Verify large numeric readouts are displayed " +
            "with Power Forward and Power Reverse values in dBm")) {
            datalogger.setDisplayMode("RF Power Meter", originalMode);
            msleep(500);
            return false;
        }

        // Plot mode
        printToConsole("  Switching to Plot display mode...");
        datalogger.setDisplayMode("RF Power Meter", 0);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Verify plot view shows power monitor traces")) {
            datalogger.setDisplayMode("RF Power Meter", originalMode);
            msleep(500);
            return false;
        }

        // Text mode
        printToConsole("  Switching to Text display mode...");
        datalogger.setDisplayMode("RF Power Meter", 1);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Verify text view shows monitor values as text")) {
            datalogger.setDisplayMode("RF Power Meter", originalMode);
            msleep(500);
            return false;
        }

        // Restore to Seven Segment (default for RF Power Meter)
        printToConsole("  Restoring to Seven Segment display mode...");
        datalogger.setDisplayMode("RF Power Meter", originalMode);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Verify seven-segment display is restored")) {
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        datalogger.setDisplayMode("RF Power Meter", 2);
        msleep(500);
        return false;
    }
});

// ============================================
// Test: Live Data Monitoring
// UID: TST.RFPOWERMETER.LIVE_MONITORING
// Description: Verify live data monitoring on the
//   seven-segment display with all available monitors.
// ============================================
printToConsole("\n=== Live Data Monitoring (SUPERVISED) ===\n");

TestFramework.runTest("TST.RFPOWERMETER.LIVE_MONITORING", function() {
    try {
        if (!switchToTool("RF Power Meter")) {
            printToConsole("  FAIL: Cannot switch to RF Power Meter tool");
            return false;
        }

        // Ensure seven-segment mode
        datalogger.setDisplayMode("RF Power Meter", 2);
        msleep(500);

        // Enable all power and voltage monitors
        var monitors = [
            { id: "powrms:power5", name: "Power Forward (dBm)" },
            { id: "powrms:power6", name: "Power Reverse (dBm)" },
            { id: "powrms:voltage0", name: "Voltage In Corrected (mV)" },
            { id: "powrms:voltage1", name: "Voltage Out Corrected (mV)" },
            { id: "powrms:voltage2", name: "Temperature (mV)" }
        ];

        for (var i = 0; i < monitors.length; i++) {
            datalogger.enableMonitorOfTool("RF Power Meter", monitors[i].id);
            printToConsole("  Enabled: " + monitors[i].name);
        }
        msleep(500);

        // Start acquisition
        printToConsole("  Starting data acquisition...");
        datalogger.setRunning(true);
        msleep(2000);

        var monitorList = "";
        for (var j = 0; j < monitors.length; j++) {
            monitorList += monitors[j].name;
            if (j < monitors.length - 1) monitorList += ", ";
        }

        if (!TestFramework.supervisedCheck(
            "Verify the following monitors display live values: " + monitorList)) {
            datalogger.setRunning(false);
            msleep(500);
            datalogger.disableMonitorOfTool("RF Power Meter", "powrms:voltage0");
            datalogger.disableMonitorOfTool("RF Power Meter", "powrms:voltage1");
            datalogger.disableMonitorOfTool("RF Power Meter", "powrms:voltage2");
            msleep(500);
            return false;
        }

        if (!TestFramework.supervisedCheck(
            "Verify values are updating in real-time on the seven-segment display")) {
            datalogger.setRunning(false);
            msleep(500);
            datalogger.disableMonitorOfTool("RF Power Meter", "powrms:voltage0");
            datalogger.disableMonitorOfTool("RF Power Meter", "powrms:voltage1");
            datalogger.disableMonitorOfTool("RF Power Meter", "powrms:voltage2");
            msleep(500);
            return false;
        }

        // Stop acquisition
        datalogger.setRunning(false);
        msleep(500);

        // Disable extra monitors, keep only power monitors
        datalogger.disableMonitorOfTool("RF Power Meter", "powrms:voltage0");
        datalogger.disableMonitorOfTool("RF Power Meter", "powrms:voltage1");
        datalogger.disableMonitorOfTool("RF Power Meter", "powrms:voltage2");
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        datalogger.setRunning(false);
        msleep(500);
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
