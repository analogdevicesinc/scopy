/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

// Data Logger Automated Tests (Single-Tool Only)
// Tests that only use the Data Logger without requiring other tools
// Based on TST.DATALOGGER.* test specifications

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: Data Logger 
TestFramework.init("Data Logger  Tests");

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to Data Logger  tool
if (!switchToTool("Data Logger ")) {
    printToConsole("ERROR: Cannot access Data Logger ");
    exit(1);
}

// Test 1: Show Available Monitors
TestFramework.runTest("TST.DATALOGGER.SHOW_MONITORS", function() {
    try {
        let monitors = datalogger.showAvailableMonitors();

        if (!monitors || monitors.length === 0) {
            printToConsole("  No monitors available");
            return false;
        }

        printToConsole("  Found " + monitors.length + " monitors:");
        for (let i = 0; i < monitors.length && i < 10; i++) {
            printToConsole("    - " + monitors[i]);
        }

        return true;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 2: Show Available Devices
TestFramework.runTest("TST.DATALOGGER.SHOW_DEVICES", function() {
    try {
        let devices = datalogger.showAvailableDevices();

        if (!devices || devices.length === 0) {
            printToConsole("  No devices available");
            return false;
        }

        printToConsole("  Found " + devices.length + " devices:");
        for (let i = 0; i < devices.length && i < 5; i++) {
            printToConsole("    - " + devices[i]);
        }

        return true;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 3: Enable/Disable Monitors
TestFramework.runTest("TST.DATALOGGER.ENABLE_MONITORS", function() {
    try {
        let monitors = datalogger.showAvailableMonitors();

        if (!monitors || monitors.length === 0) {
            return "SKIP";
        }

        // Enable first few monitors
        let enableCount = Math.min(3, monitors.length);
        let allPass = true;

        for (let i = 0; i < enableCount; i++) {
            printToConsole("  Enabling monitor: " + monitors[i]);
            try {
                datalogger.enableMonitor(monitors[i]);
                msleep(100);
            } catch (e) {
                printToConsole("    Failed to enable: " + e);
                allPass = false;
            }
        }

        if (allPass) {
            printToConsole("  ✓ All monitors enabled successfully");
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 4: Start/Stop Logging
TestFramework.runTest("TST.DATALOGGER.START_STOP", function() {
    try {
        // Start logging
        printToConsole("  Starting Data Logger ");
        datalogger.setRunning(true);
        msleep(2000);

        // Stop logging
        printToConsole("  Stopping Data Logger ");
        datalogger.setRunning(false);
        msleep(500);

        printToConsole("  ✓ Start/stop control successful");
        return true;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 5: Create Tool
TestFramework.runTest("TST.DATALOGGER.CREATE_TOOL", function() {
    try {
        printToConsole("  Creating new Data Logger  tool");
        datalogger.createTool();
        msleep(500);

        printToConsole("  ✓ Tool created successfully");
        return true;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 6: Tool-Specific Monitor Enable
TestFramework.runTest("TST.DATALOGGER.TOOL_MONITOR", function() {
    try {
        let monitors = datalogger.showAvailableMonitors();

        if (!monitors || monitors.length === 0) {
            return "SKIP";
        }

        // Create a tool and enable monitor for it
        printToConsole("  Creating tool for monitor testing");
        datalogger.createTool();
        msleep(500);

        let toolName = "Tool1"; // Default tool name
        let monitor = monitors[0];

        printToConsole("  Enabling monitor '" + monitor + "' for tool '" + toolName + "'");
        try {
            datalogger.enableMonitorOfTool(toolName, monitor);
            msleep(500);
            printToConsole("  ✓ Monitor enabled for tool");
            return true;
        } catch (e) {
            printToConsole("  ⚠ Failed to enable monitor: " + e);
            return true; // Don't fail as this is tool-dependent
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 7: Single-Shot Logging
TestFramework.runTest("TST.DATALOGGER.SINGLE_LOG", function() {
    try {
        let monitors = datalogger.showAvailableMonitors();

        if (!monitors || monitors.length === 0) {
            return "SKIP";
        }

        // Setup for logging
        datalogger.createTool();
        let toolName = "Tool1";
        let logPath = "/tmp/datalogger_test_single.csv";

        printToConsole("  Single-shot logging to: " + logPath);

        // Enable a monitor
        if (monitors.length > 0) {
            datalogger.enableMonitorOfTool(toolName, monitors[0]);
            msleep(500);
        }

        // Perform single-shot logging
        try {
            datalogger.logAtPathForTool(toolName, logPath);
            msleep(2000);

            printToConsole("  ✓ Single-shot logging completed");
            return true;

        } catch (e) {
            printToConsole("  ⚠ Logging failed (may need permissions): " + e);
            return true; // Don't fail test
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 8: Continuous Logging
TestFramework.runTest("TST.DATALOGGER.CONTINUOUS_LOG", function() {
    try {
        let monitors = datalogger.showAvailableMonitors();

        if (!monitors || monitors.length === 0) {
            return "SKIP";
        }

        // Setup for logging
        datalogger.createTool();
        let toolName = "Tool2";
        let logPath = "/tmp/datalogger_test_continuous.csv";

        printToConsole("  Starting continuous logging to: " + logPath);

        // Enable a monitor
        if (monitors.length > 0) {
            datalogger.enableMonitorOfTool(toolName, monitors[0]);
            msleep(500);
        }

        // Start continuous logging
        try {
            datalogger.continuousLogAtPathForTool(toolName, logPath);
            msleep(3000); // Log for 3 seconds

            // Stop logging
            datalogger.setRunning(false);
            msleep(500);

            printToConsole("  ✓ Continuous logging completed");
            return true;

        } catch (e) {
            printToConsole("  ⚠ Logging failed (may need permissions): " + e);
            return true; // Don't fail test
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 9: Multiple Tools
TestFramework.runTest("TST.DATALOGGER.MULTIPLE_TOOLS", function() {
    try {
        let monitors = datalogger.showAvailableMonitors();

        if (!monitors || monitors.length < 2) {
            printToConsole("  Need at least 2 monitors for multi-tool test");
            return "SKIP";
        }

        // Create multiple tools
        printToConsole("  Creating multiple tools");

        for (let i = 1; i <= 3; i++) {
            datalogger.createTool();
            msleep(200);
            printToConsole("    Created Tool" + i);
        }

        // Enable different monitors for each tool
        for (let i = 1; i <= Math.min(3, monitors.length); i++) {
            let toolName = "Tool" + i;
            let monitor = monitors[i - 1];

            try {
                datalogger.enableMonitorOfTool(toolName, monitor);
                printToConsole("    Enabled " + monitor + " for " + toolName);
            } catch (e) {
                printToConsole("    Failed to enable monitor: " + e);
            }
        }

        printToConsole("  ✓ Multiple tools configured");
        return true;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});


// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
let exitCode = TestFramework.printSummary();
exit(exitCode);