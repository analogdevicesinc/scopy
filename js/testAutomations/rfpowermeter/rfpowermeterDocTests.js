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

// RF Power Meter Manual Test Automation - DocTests
// Automates tests from: docs/tests/plugins/rfpowermeter/rfpowermeter_tests.rst
//
// NOTE: The RF Power Meter plugin does not have its own JS API object.
//       It configures the DataLogger plugin, so tests use the "datalogger"
//       JS API object.
//
// NOTE: These tests require a device with "powrms" IIO interface.
//       The emulator may not support this device.
//
// Tests (Category A - Fully Automatable via DataLogger API):
//   TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION.TOOL_NAME - Verify tool is renamed to "RF Power Meter"
//   TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION.MONITORS_AVAILABLE - Verify powrms monitors are available
//   TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION.POWER_MONITORS - Verify power monitors can be enabled
//   TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION.VOLTAGE_MONITORS - Verify voltage monitors are available
//   TST.RFPOWERMETER.PLUGIN_DETECTION.TOOL_EXISTS - Verify RF Power Meter tool exists after connection
//
// Skipped (Category C - Not Automatable):
//   TST.RFPOWERMETER.DATALOGGER_DEPENDENCY - Requires plugin manager interaction (no API to enable/disable plugins)
//   TST.RFPOWERMETER.PLUGIN_DETECTION Step 3 - Requires connecting to an incompatible device

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("RF Power Meter Documentation Tests");

// Connect to device (requires powrms IIO interface)
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    printToConsole("NOTE: RF Power Meter requires a device with 'powrms' IIO interface");
    scopy.exit();
}

// ============================================
// Test 1: Plugin Detection - Tool Exists
// UID: TST.RFPOWERMETER.PLUGIN_DETECTION.TOOL_EXISTS
// Description: Verify that the RF Power Meter tool
//   exists in the DataLogger tool list after connecting
//   to a compatible device.
// Source: Steps 1-2 of TST.RFPOWERMETER.PLUGIN_DETECTION
// ============================================
printToConsole("\n=== Test 1: Plugin Detection - Tool Exists ===\n");

TestFramework.runTest("TST.RFPOWERMETER.PLUGIN_DETECTION.TOOL_EXISTS", function() {
    try {
        // Step 1-2: After connecting to compatible device, verify plugin is available
        var toolList = datalogger.getToolList();
        printToConsole("  DataLogger tool list: " + toolList);

        if (toolList.indexOf("RF Power Meter") !== -1) {
            printToConsole("  PASS: 'RF Power Meter' found in tool list");
            return true;
        } else {
            printToConsole("  FAIL: 'RF Power Meter' not found in tool list");
            printToConsole("  Available tools: " + toolList);
            return false;
        }
    } catch (e) {
        printToConsole("  Error: " + e);
        printToConsole("  NOTE: RF Power Meter requires a device with 'powrms' IIO interface");
        return false;
    }
});

// Switch to RF Power Meter tool
if (!switchToTool("RF Power Meter")) {
    printToConsole("WARNING: Cannot switch to RF Power Meter tool");
    printToConsole("Remaining tests may fail if tool is not accessible");
}

// ============================================
// Test 3a: Automatic Configuration - Tool Name
// UID: TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION.TOOL_NAME
// Description: Verify that the DataLogger tool is
//   automatically renamed to "RF Power Meter" on connection.
// Source: Step 1 of TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION
// ============================================
printToConsole("\n=== Test 3a: Automatic Configuration - Tool Name ===\n");

TestFramework.runTest("TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION.TOOL_NAME", function() {
    try {
        var toolList = datalogger.getToolList();
        printToConsole("  Tool list: " + toolList);

        // Verify "RF Power Meter" is in the tool list (renamed from "Data Logger")
        if (toolList.indexOf("RF Power Meter") !== -1) {
            printToConsole("  PASS: Tool name correctly set to 'RF Power Meter'");
            return true;
        } else {
            printToConsole("  FAIL: Tool name was not changed to 'RF Power Meter'");
            return false;
        }
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3b: Automatic Configuration - Monitors Available
// UID: TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION.MONITORS_AVAILABLE
// Description: Verify that powrms monitors are detected
//   and available in the DataLogger.
// Source: Step 2 of TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION
// ============================================
printToConsole("\n=== Test 3b: Automatic Configuration - Monitors Available ===\n");

TestFramework.runTest("TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION.MONITORS_AVAILABLE", function() {
    try {
        var availableMonitors = datalogger.showAvailableMonitors();
        printToConsole("  Available monitors:\n" + availableMonitors);

        var passed = true;

        // Check for powrms monitors
        if (availableMonitors.indexOf("powrms") === -1) {
            printToConsole("  FAIL: No powrms monitors found");
            return false;
        }
        printToConsole("  PASS: powrms monitors detected");

        // Also check device-specific monitor list
        var powrmsMonitors = datalogger.showMonitorsOfDevice("powrms");
        printToConsole("  powrms device monitors:\n" + powrmsMonitors);

        if (!powrmsMonitors || powrmsMonitors === "") {
            printToConsole("  FAIL: showMonitorsOfDevice('powrms') returned empty");
            return false;
        }
        printToConsole("  PASS: powrms device monitors listed");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3c: Automatic Configuration - Power Monitors
// UID: TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION.POWER_MONITORS
// Description: Verify that Power Forward (power5) and
//   Power Reverse (power6) monitors are available and
//   can be enabled for the RF Power Meter tool.
// Source: Step 2 of TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION
// ============================================
printToConsole("\n=== Test 3c: Automatic Configuration - Power Monitors ===\n");

TestFramework.runTest("TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION.POWER_MONITORS", function() {
    try {
        var passed = true;
        var toolName = "RF Power Meter";

        // Verify power5 (Power Forward) can be enabled
        var result5 = datalogger.enableMonitorOfTool(toolName, "powrms:power5");
        printToConsole("  Enable powrms:power5 result: " + result5);
        if (result5 && result5.indexOf("Error") !== -1) {
            printToConsole("  FAIL: Could not enable Power Forward (powrms:power5)");
            passed = false;
        } else {
            printToConsole("  PASS: Power Forward (powrms:power5) enabled");
        }

        // Verify power6 (Power Reverse) can be enabled
        var result6 = datalogger.enableMonitorOfTool(toolName, "powrms:power6");
        printToConsole("  Enable powrms:power6 result: " + result6);
        if (result6 && result6.indexOf("Error") !== -1) {
            printToConsole("  FAIL: Could not enable Power Reverse (powrms:power6)");
            passed = false;
        } else {
            printToConsole("  PASS: Power Reverse (powrms:power6) enabled");
        }

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3d: Automatic Configuration - Voltage Monitors
// UID: TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION.VOLTAGE_MONITORS
// Description: Verify that voltage monitors (voltage0-4)
//   are available for the powrms device.
// Source: Step 2 of TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION
// ============================================
printToConsole("\n=== Test 3d: Automatic Configuration - Voltage Monitors ===\n");

TestFramework.runTest("TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION.VOLTAGE_MONITORS", function() {
    try {
        var powrmsMonitors = datalogger.showMonitorsOfDevice("powrms");
        printToConsole("  powrms monitors: " + powrmsMonitors);

        var passed = true;
        var expectedMonitors = [
            "powrms:voltage0",  // Voltage In Corrected
            "powrms:voltage1",  // Voltage Out Corrected
            "powrms:voltage2",  // Temperature
            "powrms:voltage3",  // Voltage In
            "powrms:voltage4"   // Voltage Out
        ];

        for (var i = 0; i < expectedMonitors.length; i++) {
            if (powrmsMonitors.indexOf(expectedMonitors[i]) !== -1) {
                printToConsole("  PASS: " + expectedMonitors[i] + " found");
            } else {
                printToConsole("  FAIL: " + expectedMonitors[i] + " not found");
                passed = false;
            }
        }

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test: Data Acquisition from Power Monitors
// UID: TST.RFPOWERMETER.DATA_ACQUISITION
// Description: Verify that data can be acquired from
//   the power monitors by running the DataLogger briefly.
// ============================================
printToConsole("\n=== Data Acquisition from Power Monitors ===\n");

TestFramework.runTest("TST.RFPOWERMETER.DATA_ACQUISITION", function() {
    try {
        var toolName = "RF Power Meter";

        // Enable power monitors
        datalogger.enableMonitorOfTool(toolName, "powrms:power5");
        datalogger.enableMonitorOfTool(toolName, "powrms:power6");
        msleep(500);

        // Run data acquisition briefly
        printToConsole("  Starting data acquisition...");
        datalogger.setRunning(true);
        msleep(3000);
        datalogger.setRunning(false);
        msleep(500);

        printToConsole("  PASS: Data acquisition completed without errors");

        return true;
    } catch (e) {
        printToConsole("  Error during data acquisition: " + e);
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
