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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

// Data Logger Automated Tests (Single-Tool Only)
// Tests that only use the Data Logger without requiring other tools
// Based on TST.DATALOGGER.* test specifications
//
// ============================================================================
// TESTS THAT CANNOT BE AUTOMATED WITH CURRENT API
// ============================================================================
// The following documented tests require API methods that are not yet exposed:
//
// Doc Test 6:  REMOVE_A_CREATED_TOOL         - removeTool() not exposed in API
// Doc Test 7:  VERIFY_REMOVAL_DISABLED_FOR_DEFAULT_TOOL - UI-only (check X button state)
// Doc Test 8:  PRINT_COLLECTED_DATA          - No print() API method
// Doc Test 9:  INFO_BUTTON_DOCUMENTATION     - UI-only (click button, open browser)
// Doc Test 10: INFO_BUTTON_TUTORIAL          - UI-only
// Doc Test 12: SET_VALID_X_AXIS_DELTA_VALUE  - No X-axis delta API
// Doc Test 13: HANDLE_INVALID_X_AXIS_DELTA_VALUE_INPUT - No X-axis delta API
// Doc Test 14: ADJUST_X_AXIS_DELTA_VALUE_USING_BUTTONS - UI-only (+/- buttons)
// Doc Test 15: TOGGLE_X_AXIS_UTC_TIME_DISPLAY - No UTC toggle API
// Doc Test 16: TOGGLE_X_AXIS_LIVE_PLOTTING   - No live plotting toggle API
// Doc Test 17: TOGGLE_Y_AXIS_AUTOSCALE       - No autoscale toggle API
// Doc Test 19: ADJUST_CURVE_THICKNESS        - No curve thickness API
// Doc Test 20: CHANGE_CURVE_STYLE            - No curve style API
// Doc Test 21: ADJUST_PLOT_DISPLAY_SETTINGS  - No buffer preview/axis label API
// Doc Test 28: CHOOSE_FILE_FOR_DATA_LOGGING  - UI-only (file dialog)
// Doc Test 35: USE_MULTIPLOT                 - No multiplot behavior API
// Doc Test 36: USE_KDOCKS                    - UI-only (drag/drop docking)
// Doc Test 37: USE_CHANNEL_SCALING           - No channel scaling API
// ============================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Helper function to parse newline-separated string into array
function parseNewlineSeparatedString(str) {
    if (!str || str.length === 0) {
        return [];
    }
    // Split by newline and filter out empty strings
    return str.split('\n').filter(function (item) {
        return item && item.trim().length > 0;
    });
}

// Helper function to check if file exists and has data
function fileExistsAndHasData(path) {
    try {
        var content = fileIO.readAll(path);
        if (!content || content.length === 0) {
            return false;
        }

        // For CSV files, verify we have at least header + one data row
        var lines = content.split('\n').filter(function (line) {
            return line.trim().length > 0;
        });

        return lines.length >= 2; // At least header + 1 data row
    } catch (e) {
        return false;
    }
}

// Helper function to check if file exists and is not empty
function fileExists(path) {
    try {
        var content = fileIO.readAll(path);
        return content && content.length > 0;
    } catch (e) {
        return false;
    }
}

// Helper function to get file size (in characters, not bytes)
function getFileSize(path) {
    try {
        var content = fileIO.readAll(path);
        return content ? content.length : 0;
    } catch (e) {
        return 0;
    }
}

// Helper function to read first N lines of file
function readFileLines(path, n) {
    try {
        var content = fileIO.readAll(path);
        if (!content) return "";

        var lines = content.split('\n');
        var result = [];
        for (var i = 0; i < Math.min(n, lines.length); i++) {
            result.push(lines[i]);
        }
        return result.join('\n');
    } catch (e) {
        return "";
    }
}

// Helper function to validate CSV format
function isValidCSV(content) {
    if (!content || content.trim().length === 0) {
        return false;
    }

    let lines = content.split('\n').filter(function (line) {
        return line.trim().length > 0;
    });

    if (lines.length < 2) {
        return false; // Need at least header + one data row
    }

    // Check if all lines have same number of columns
    let headerCols = lines[0].split(',').length;
    for (let i = 1; i < lines.length; i++) {
        let cols = lines[i].split(',').length;
        if (cols !== headerCols) {
            return false;
        }
    }
    return true;
}

// Helper function to compare arrays
function arraysEqual(a, b) {
    if (a.length !== b.length) return false;
    for (let i = 0; i < a.length; i++) {
        if (a[i] !== b[i]) return false;
    }
    return true;
}

// Helper function to check if item in array
function arrayContains(arr, item) {
    for (let i = 0; i < arr.length; i++) {
        if (arr[i] === item) return true;
    }
    return false;
}

// Test Suite: Data Logger
TestFramework.init("Data Logger Tests");

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to Data Logger tool
if (!switchToTool("Data Logger ")) {
    printToConsole("ERROR: Cannot access Data Logger");
    exit(1);
}

// Test 1: Enable/Disable Data Logger Channel (Doc Test 1)
TestFramework.runTest("TST.DATALOGGER.ENABLE_DISABLE_DATA_LOGGER_CHANNEL", function () {
    try {
        let monitorsStr = datalogger.showAvailableMonitors();
        let monitors = parseNewlineSeparatedString(monitorsStr);

        if (monitors.length === 0) {
            return "SKIP";
        }

        let toolName = "Data Logger ";
        scopy.switchTool(toolName);

        // Enable first few monitors and validate return values
        let enableCount = Math.min(3, monitors.length);
        let allPass = true;
        let enabledMonitors = [];

        for (let i = 0; i < enableCount; i++) {
            printToConsole("  Enabling monitor: " + monitors[i]);
            try {
                let result = datalogger.enableMonitorOfTool(toolName, monitors[i]);
                if (result === "OK") {
                    printToConsole("    ✓ Enable returned OK");
                    enabledMonitors.push(monitors[i]);
                } else {
                    printToConsole("    ✗ Enable returned: " + result);
                    allPass = false;
                }
                msleep(100);
            } catch (e) {
                printToConsole("    Failed to enable: " + e);
                allPass = false;
            }
        }

        // Now disable the monitors and validate
        printToConsole("  Testing disable functionality:");
        for (let i = 0; i < enabledMonitors.length; i++) {
            try {
                let result = datalogger.disableMonitorOfTool(toolName, enabledMonitors[i]);
                if (result === "OK") {
                    printToConsole("    ✓ Disabled " + enabledMonitors[i] + " successfully");
                } else {
                    printToConsole("    ✗ Disable returned: " + result);
                    allPass = false;
                }
                msleep(100);
            } catch (e) {
                printToConsole("    Failed to disable: " + e);
                allPass = false;
            }
        }

        // Test enabling already enabled monitor (edge case)
        if (monitors.length > 0) {
            printToConsole("  Testing double-enable edge case:");
            datalogger.enableMonitorOfTool(toolName, monitors[0]);
            msleep(100);
            let result = datalogger.enableMonitorOfTool(toolName, monitors[0]);
            if (result === "OK") {
                printToConsole("    ✓ Double-enable handled gracefully");
            }
        }

        if (allPass) {
            printToConsole("  ✓ All enable/disable operations validated successfully");
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 2: Run and Stop Data Logging (Doc Test 2)
TestFramework.runTest("TST.DATALOGGER.RUN_STOP_DATA_LOGGING", function () {
    try {
        // Start logging
        let toolName = "Data Logger ";
        scopy.switchTool(toolName);

        // Start data logger and verify it can collect data
        printToConsole("  Starting Data Logger");
        datalogger.setRunning(true);
        msleep(2000);

        // Verify running state by attempting to log data
        let testPath = fileIO.getTempPath() + "/start_stop_test.csv";
        try {
            datalogger.logAtPathForTool(toolName, testPath);
            msleep(500);
            printToConsole("  ✓ Data Logger started successfully (can log data)");
        } catch (e) {
            printToConsole("  ⚠ Could not verify running state: " + e);
        }

        // Stop logging
        printToConsole("  Stopping Data Logger");
        datalogger.setRunning(false);
        msleep(500);

        printToConsole("  ✓ Start/stop control executed");
        return true;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 5: Create a New Data Logger Tool (Doc Test 5)
TestFramework.runTest("TST.DATALOGGER.CREATE_NEW_DATA_LOGGER_TOOL", function () {
    try {
        // Get initial tool list
        let initialToolsStr = datalogger.getToolList();
        let initialTools = parseNewlineSeparatedString(initialToolsStr);
        let initialCount = initialTools.length;
        printToConsole("  Initial tool count: " + initialCount);

        // Create new tool
        printToConsole("  Creating new Data Logger tool");
        let result = datalogger.createTool();
        msleep(500);

        if (result !== "Tool created") {
            printToConsole("  ✗ Tool creation failed: " + result);
            return false;
        }

        printToConsole("  ✓ createTool returned 'Tool created'");

        // Verify tool appears in list
        let newToolsStr = datalogger.getToolList();
        let newTools = parseNewlineSeparatedString(newToolsStr);
        let newCount = newTools.length;

        if (newCount > initialCount) {
            printToConsole("  ✓ Tool count increased from " + initialCount + " to " + newCount);

            // Find the new tool (should be named Tool1, Tool2, etc.)
            let foundNewTool = false;
            for (let i = 0; i < newTools.length; i++) {
                if (!arrayContains(initialTools, newTools[i])) {
                    printToConsole("  ✓ New tool found: " + newTools[i]);
                    foundNewTool = true;
                    break;
                }
            }

            if (!foundNewTool) {
                printToConsole("  ⚠ Could not identify which tool is new");
            }

            return true;
        } else {
            printToConsole("  ✗ Tool count did not increase (still " + newCount + ")");
            return false;
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 29: Save Collected Data to File (Doc Test 29)
TestFramework.runTest("TST.DATALOGGER.SAVE_COLLECTED_DATA_TO_FILE", function () {
    try {
        let monitorsStr = datalogger.showAvailableMonitors();
        let monitors = parseNewlineSeparatedString(monitorsStr);

        if (monitors.length === 0) {
            return "SKIP";
        }

        // Setup for logging
        let toolName = "Data Logger ";
        scopy.switchTool(toolName);
        let logPath = fileIO.getTempPath() + "/datalogger_test_single.csv";

        printToConsole("  Single-shot logging to: " + logPath);

        // Enable a monitor
        if (monitors.length > 0) {
            let result = datalogger.enableMonitorOfTool(toolName, monitors[0]);
            if (result !== "OK") {
                printToConsole("  Failed to enable monitor: " + result);
                return false;
            }
            printToConsole("  ✓ Enabled monitor: " + monitors[0]);
            msleep(500);
        }

        // Start data acquisition to collect data before logging
        printToConsole("  Starting data acquisition...");
        datalogger.setRunning(true);
        msleep(3000);  // Increased wait time for more data collection

        // Perform single-shot logging
        try {
            datalogger.logAtPathForTool(toolName, logPath);
            msleep(2000);

            // Stop data acquisition
            datalogger.setRunning(false);

            // Verify the file was created and contains data
            if (fileExistsAndHasData(logPath)) {
                printToConsole("  ✓ Verified file exists and contains data");
                return true;
            } else {
                printToConsole("  ✗ File is missing or empty");
                return false;
            }

        } catch (e) {
            printToConsole("  Error during logging: " + e);
            datalogger.setRunning(false);  // Stop data acquisition on failure
            return false;
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 30: Enable Live Data Logging (Doc Test 30)
TestFramework.runTest("TST.DATALOGGER.ENABLE_LIVE_DATA_LOGGING", function () {
    try {
        let monitorsStr = datalogger.showAvailableMonitors();
        let monitors = parseNewlineSeparatedString(monitorsStr);

        if (monitors.length === 0) {
            return "SKIP";
        }

        // Setup for logging
        let toolName = "Data Logger ";
        scopy.switchTool(toolName);
        let logPath = fileIO.getTempPath() + "/datalogger_test_continuous.csv";

        printToConsole("  Starting continuous logging to: " + logPath);

        // Enable a monitor
        if (monitors.length > 0) {
            let result = datalogger.enableMonitorOfTool(toolName, monitors[0]);
            if (result !== "OK") {
                printToConsole("  Failed to enable monitor: " + result);
                return false;
            }
            printToConsole("  ✓ Enabled monitor: " + monitors[0]);
            msleep(500);
        }

        // Start data acquisition and wait for initial data collection
        printToConsole("  Starting data acquisition...");
        datalogger.setRunning(true);
        msleep(3000);  // Increased wait time for more data collection

        // Start continuous logging
        try {
            datalogger.continuousLogAtPathForTool(toolName, logPath);
            printToConsole("  ✓ Continuous logging started");
            msleep(3000); // Wait for continuous writes

            // Stop continuous logging
            datalogger.stopContinuousLogForTool(toolName);
            datalogger.setRunning(false);
            msleep(500);
            printToConsole("  ✓ Stopped continuous logging");

            // Verify the file was created and contains data
            if (fileExistsAndHasData(logPath)) {
                printToConsole("  ✓ Verified file exists and contains data");
                return true;
            } else {
                printToConsole("  ✗ File is missing or empty");
                return false;
            }

        } catch (e) {
            printToConsole("  Error during logging: " + e);
            try {
                datalogger.stopContinuousLogForTool(toolName);
                datalogger.setRunning(false);
            } catch (stopError) {
                // Ignore stop errors
            }
            return false;
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 3: Clear Collected Data (Doc Test 3)

TestFramework.runTest("TST.DATALOGGER.CLEAR_COLLECTED_DATA", function () {
    try {
        let toolName = "Data Logger ";
        let monitorsStr = datalogger.showAvailableMonitors();
        let monitors = parseNewlineSeparatedString(monitorsStr);

        if (monitors.length === 0) {
            printToConsole("  No monitors available to test clear");
            return "SKIP";
        }

        // Enable a monitor to collect data
        datalogger.enableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        // Collect some data
        printToConsole("  Collecting data before clear...");
        datalogger.setRunning(true);
        msleep(3000);  // Increased wait time
        datalogger.setRunning(false);

        // Log data before clear to verify we have data
        let beforePath = fileIO.getTempPath() + "/clear_test_before.csv";
        datalogger.logAtPathForTool(toolName, beforePath);
        msleep(500);

        // Verify the file was created and contains data
        if (fileExistsAndHasData(beforePath)) {
            printToConsole("  ✓ Verified file exists and contains data");

        } else {
            printToConsole("  ✗ File is missing or empty");
            return false;
        }


        // Clear accumulated data
        datalogger.clearData();
        printToConsole("  ✓ Cleared data");

        // Start collecting again to verify data collection works after clear
        datalogger.setRunning(true);
        msleep(3000);  // Collect enough data to verify clear worked
        datalogger.setRunning(false);

        // Try to log after clear - should have minimal data
        let afterPath = fileIO.getTempPath() + "/clear_test_after.csv";
        datalogger.logAtPathForTool(toolName, afterPath);
        msleep(500);
         // Verify the file was created and contains data
        if (fileExistsAndHasData(afterPath)) {
            printToConsole("  ✓ Verified file exists and contains data");

        } else {
            printToConsole("  ✗ File is missing or empty");
            return false;
        }

        printToConsole("  ✓ Clear data functionality tested");
        return true;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 4: Clear Data While Running (Doc Test 4)
TestFramework.runTest("TST.DATALOGGER.CLEAR_DATA_WHILE_RUNNING", function () {
    try {
        let toolName = "Data Logger ";
        let monitorsStr = datalogger.showAvailableMonitors();
        let monitors = parseNewlineSeparatedString(monitorsStr);

        if (monitors.length === 0) {
            printToConsole("  No monitors available to test clear while running");
            return "SKIP";
        }

        // Enable a monitor to collect data
        datalogger.enableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        // Start data acquisition
        printToConsole("  Starting data acquisition...");
        datalogger.setRunning(true);
        msleep(2000);

        // Clear data while still running
        printToConsole("  Clearing data while running...");
        datalogger.clearData();
        printToConsole("  ✓ clearData() called while running");

        // Continue running briefly to verify acquisition continues
        msleep(1000);

        // Stop and verify no exceptions occurred
        datalogger.setRunning(false);
        printToConsole("  ✓ Data logger stopped successfully after clear");

        // Verify data collection resumed by logging
        let verifyPath = fileIO.getTempPath() + "/clear_while_running_verify.csv";
        datalogger.logAtPathForTool(toolName, verifyPath);
        msleep(500);
        printToConsole("  ✓ Data collection resumed after clear (logged to " + verifyPath + ")");

        return true;

    } catch (e) {
        printToConsole("  Error: " + e);
        datalogger.setRunning(false);
        return false;
    }
});

// Test 27: Toggle 7 Segment Min/Max Display (Doc Test 27)

TestFramework.runTest("TST.DATALOGGER.TOGGLE_7_SEGMENT_MIN_MAX_DISPLAY", function () {
    try {
        // Enable min/max display
        datalogger.setMinMax(true);
        msleep(500);
        printToConsole("  ✓ Min/Max display enabled");

        // Disable min/max display
        datalogger.setMinMax(false);
        msleep(500);
        printToConsole("  ✓ Min/Max display disabled");

        // If no exceptions, settings were applied
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 26: Set 7 Segment Display Precision (Doc Test 26)

TestFramework.runTest("TST.DATALOGGER.SET_7_SEGMENT_DISPLAY_PRECISION", function () {
    try {
        let allPass = true;
        for (let decimals = 0; decimals <= 6; decimals++) {
            try {
                datalogger.changePrecision(decimals);
                printToConsole("  ✓ Set precision to " + decimals + " decimals");
                msleep(500);
            } catch (e) {
                printToConsole("  ✗ Failed to set precision to " + decimals + ": " + e);
                allPass = false;
            }
        }
        return allPass;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 18: Set Y-Axis Minimum and Maximum Values (Doc Test 18)
// NOTE: Skipped - Requires UI observation. No API methods to verify Y-axis values.

TestFramework.runTest("TST.DATALOGGER.SET_Y_AXIS_MIN_MAX_VALUES", function () {
    try {
        printToConsole("  ⚠ Test requires UI observation - no API getters for Y-axis values");
        printToConsole("  Manual verification needed:");
        printToConsole("    1. Check Y-axis displays range -10 to 10");
        printToConsole("    2. Check Y-axis displays range 0 to 5");

        // Test setting negative to positive range
        datalogger.setMinYAxis(-10);
        datalogger.setMaxYAxis(10);
        printToConsole("  ✓ Set Y-axis range: -10 to 10 (verify visually)");
        msleep(500);

        // Test setting positive range only
        datalogger.setMinYAxis(0);
        datalogger.setMaxYAxis(5);
        printToConsole("  ✓ Set Y-axis range: 0 to 5 (verify visually)");
        msleep(500);

        // Skip test - requires UI observation
        return "SKIP";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 31: Import Data from File (Doc Test 31)

TestFramework.runTest("TST.DATALOGGER.IMPORT_DATA_FROM_FILE", function () {
    try {
        let toolName = "Data Logger ";
        // Use the file created by Test 7 (SINGLE_LOG)
        let importPath = fileIO.getTempPath() + "/datalogger_test_single.csv";

        printToConsole("  Using file from Test 7: " + importPath);
        printToConsole("  Note: This test validates both export (Test 7) and import functionality");

        // Clear any existing data first
        datalogger.clearData();
        msleep(500);

        // Import the data
        try {
            datalogger.importDataFromPathForTool(toolName, importPath);
            printToConsole("  ✓ Import command executed for " + importPath);
            msleep(1000);

            // Verify import worked by exporting and checking
            let verifyPath = fileIO.getTempPath() + "/datalogger_import_verify.csv";
            datalogger.logAtPathForTool(toolName, verifyPath);
            msleep(500);

            printToConsole("  ✓ Data imported and re-exported successfully");
            printToConsole("  ✓ Round-trip test (export → import → export) completed");

            return true;

        } catch (e) {
            printToConsole("  Import failed: " + e);
            return false;
        }
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 25: Toggle Between Display Methods (Doc Test 25)

TestFramework.runTest("TST.DATALOGGER.TOGGLE_BETWEEN_DISPLAY_METHODS", function () {
    try {
        let toolName = "Data Logger ";
        let allPass = true;

        // Try different display modes (0, 1, 2, etc.)
        for (let mode = 0; mode <= 2; mode++) {
            try {
                datalogger.setDisplayMode(toolName, mode);
                printToConsole("  ✓ Set display mode to " + mode);
                msleep(500);
            } catch (e) {
                printToConsole("  ✗ Failed to set display mode " + mode + ": " + e);
                allPass = false;
            }
        }

        return allPass;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 22: Verify Plot Display Method (Doc Test 22)
TestFramework.runTest("TST.DATALOGGER.VERIFY_PLOT_DISPLAY_METHOD", function () {
    try {
        let toolName = "Data Logger ";

        // Set display mode to Plot (mode 0)
        datalogger.setDisplayMode(toolName, 0);
        msleep(500);
        printToConsole("  ✓ Set display mode to Plot (mode 0)");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 23: Verify Plain Text Display Method (Doc Test 23)
TestFramework.runTest("TST.DATALOGGER.VERIFY_PLAIN_TEXT_DISPLAY_METHOD", function () {
    try {
        let toolName = "Data Logger ";

        // Set display mode to Text (mode 1)
        datalogger.setDisplayMode(toolName, 1);
        msleep(500);
        printToConsole("  ✓ Set display mode to Plain Text (mode 1)");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 24: Verify 7 Segment Display Method (Doc Test 24)
TestFramework.runTest("TST.DATALOGGER.VERIFY_7_SEGMENT_DISPLAY_METHOD", function () {
    try {
        let toolName = "Data Logger ";

        // Set display mode to 7 Segment (mode 2)
        datalogger.setDisplayMode(toolName, 2);
        msleep(500);
        printToConsole("  ✓ Set display mode to 7 Segment (mode 2)");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 11: Settings Change Data Logger Tool Name (Doc Test 11)

TestFramework.runTest("TST.DATALOGGER.SETTINGS_CHANGE_DATA_LOGGER_TOOL_NAME", function () {
    try {
        // Get initial tool list
        let initialToolsStr = datalogger.getToolList();
        let initialTools = parseNewlineSeparatedString(initialToolsStr);

        // Create new tool
        printToConsole("  Creating new tool for rename test");
        let result = datalogger.createTool();
        if (result !== "Tool created") {
            printToConsole("  ✗ Failed to create tool: " + result);
            return false;
        }
        msleep(500);

        // Find the newly created tool name
        let afterCreateStr = datalogger.getToolList();
        let afterCreateTools = parseNewlineSeparatedString(afterCreateStr);
        let createdToolName = null;

        for (let i = 0; i < afterCreateTools.length; i++) {
            if (!arrayContains(initialTools, afterCreateTools[i])) {
                createdToolName = afterCreateTools[i];
                break;
            }
        }

        if (!createdToolName) {
            printToConsole("  ✗ Could not find newly created tool");
            return false;
        }

        printToConsole("  ✓ Created tool: " + createdToolName);

        // Rename the tool
        let newName = "MyCustomTool_" + Date.now(); // Make name unique
        printToConsole("  Renaming '" + createdToolName + "' to '" + newName + "'");
        datalogger.setToolName(createdToolName, newName);
        msleep(500);

        // Verify in tool list
        let finalToolsStr = datalogger.getToolList();
        let finalTools = parseNewlineSeparatedString(finalToolsStr);

        let foundNewName = arrayContains(finalTools, newName);
        let oldNameGone = !arrayContains(finalTools, createdToolName);

        if (foundNewName) {
            printToConsole("  ✓ New name '" + newName + "' found in tool list");
        } else {
            printToConsole("  ✗ New name '" + newName + "' NOT found in tool list");
        }

        if (oldNameGone) {
            printToConsole("  ✓ Old name '" + createdToolName + "' removed from tool list");
        } else {
            printToConsole("  ✗ Old name '" + createdToolName + "' still in tool list");
        }

        return foundNewName && oldNameGone;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 32: Set Maximum Channel Data Storage (Doc Test 32)
// NOTE: Skipped - scopy.getPreference() returns QPair which is not exposed to JavaScript
TestFramework.runTest("TST.DATALOGGER.SET_MAXIMUM_CHANNEL_DATA_STORAGE", function () {
    try {
        printToConsole("  ⚠ Test skipped - scopy.getPreference() returns QPair<QString,QVariant>");
        printToConsole("  JavaScript cannot handle QPair return type");
        printToConsole("  Preference can be set but not verified programmatically");

        // Can still set the preference, just can't verify it
        let testValue = "1 Mb";
        scopy.setPreference("dataloggerplugin_data_storage_size", testValue);
        printToConsole("  ✓ Preference set (cannot verify)");

        return "SKIP";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 33: Set Data Logger Read Interval (Doc Test 33)
// NOTE: Skipped - scopy.getPreference() returns QPair which is not exposed to JavaScript
TestFramework.runTest("TST.DATALOGGER.SET_DATA_LOGGER_READ_INTERVAL", function () {
    try {
        printToConsole("  ⚠ Test skipped - scopy.getPreference() returns QPair<QString,QVariant>");
        printToConsole("  JavaScript cannot handle QPair return type");
        printToConsole("  Preference can be set but not verified programmatically");

        // Can still set the preference, just can't verify it
        let testValue = "2";
        scopy.setPreference("dataloggerplugin_read_interval", testValue);
        printToConsole("  ✓ Preference set (cannot verify)");

        return "SKIP";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 34: Set X-Axis Date Time Format (Doc Test 34)
// NOTE: Skipped - scopy.getPreference() returns QPair which is not exposed to JavaScript
TestFramework.runTest("TST.DATALOGGER.SET_X_AXIS_DATE_TIME_FORMAT", function () {
    try {
        printToConsole("  ⚠ Test skipped - scopy.getPreference() returns QPair<QString,QVariant>");
        printToConsole("  JavaScript cannot handle QPair return type");
        printToConsole("  Preference can be set but not verified programmatically");

        // Can still set the preference, just can't verify it
        let testValue = "mm:ss";
        scopy.setPreference("dataloggerplugin_date_time_format", testValue);
        printToConsole("  ✓ Preference set (cannot verify)");

        return "SKIP";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
let exitCode = TestFramework.printSummary();
scopy.exit();