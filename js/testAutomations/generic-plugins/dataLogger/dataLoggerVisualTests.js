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
// require a human observer to verify UI changes. Each step includes a
// supervisedCheck() call to allow the human to confirm pass/fail.
// Source: docs/tests/plugins/datalogger/datalogger_tests.rst
//
// Not automatable (Category C - skipped entirely):
//   TST.DATALOGGER.PRINT_COLLECTED_DATA - requires print dialog and file save
//   TST.DATALOGGER.INFO_BUTTON_DOCUMENTATION - opens external browser
//   TST.DATALOGGER.INFO_BUTTON_TUTORIAL - displays tutorial UI overlay
//   TST.DATALOGGER.SET_VALID_X_AXIS_DELTA_VALUE - no X-axis delta API
//   TST.DATALOGGER.HANDLE_INVALID_X_AXIS_DELTA_VALUE_INPUT - no X-axis delta API
//   TST.DATALOGGER.ADJUST_X_AXIS_DELTA_VALUE_USING_BUTTONS - UI +/- buttons only
//   TST.DATALOGGER.TOGGLE_X_AXIS_UTC_TIME_DISPLAY - no UTC toggle API
//   TST.DATALOGGER.TOGGLE_X_AXIS_LIVE_PLOTTING - no live plotting toggle API
//   TST.DATALOGGER.TOGGLE_Y_AXIS_AUTOSCALE - no autoscale toggle API
//   TST.DATALOGGER.ADJUST_CURVE_THICKNESS - no curve thickness API
//   TST.DATALOGGER.CHANGE_CURVE_STYLE - no curve style API
//   TST.DATALOGGER.ADJUST_PLOT_DISPLAY_SETTINGS - no buffer preview/axis label API
//   TST.DATALOGGER.CHOOSE_FILE_FOR_DATA_LOGGING - requires file dialog
//   TST.DATALOGGER.USE_KDOCKS - requires drag/drop docking interaction
//   TST.DATALOGGER.USE_CHANNEL_SCALING - no channel scaling API
// ============================================================================

// Load test framework
evaluateFile("js/testAutomations/common/testFramework.js");

// Helper function to parse newline-separated string into array
function parseNewlineSeparatedString(str) {
    if (!str || str.length === 0) {
        return [];
    }
    return str.split('\n').filter(function (item) {
        return item && item.trim().length > 0;
    });
}

// Helper function to check if item in array
function arrayContains(arr, item) {
    for (var i = 0; i < arr.length; i++) {
        if (arr[i] === item) return true;
    }
    return false;
}

// Test Suite
TestFramework.init("Data Logger Visual Validation Tests");

var toolName = "Data Logger ";

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to Data Logger tool
if (!switchToTool("Data Logger ")) {
    printToConsole("ERROR: Cannot access Data Logger tool");
    scopy.exit();
}

// ===========================================================================
// Test 6: Remove a Created Tool
// UID: TST.DATALOGGER.REMOVE_A_CREATED_TOOL
// Description: Remove the tool and check if the tool is removed.
// VISUAL: Verify the X button removes the created tool
// Note: removeTool() is not exposed in the API. This test creates a tool
//   via API and requires manual click on the X button.
// ===========================================================================
printToConsole("\n=== Test 6: Remove a Created Tool (VISUAL) ===\n");

TestFramework.runTest("TST.DATALOGGER.REMOVE_A_CREATED_TOOL", function() {
    try {
        // Get initial tool list
        var initialToolsStr = datalogger.getToolList();
        var initialTools = parseNewlineSeparatedString(initialToolsStr);
        printToConsole("  Initial tools: " + initialTools.join(", "));

        // Create a new tool
        printToConsole("  Creating new Data Logger tool...");
        var result = datalogger.createTool();
        msleep(500);

        if (result !== "Tool created") {
            printToConsole("  FAIL: Tool creation failed: " + result);
            return false;
        }

        // Find the new tool name
        var afterCreateStr = datalogger.getToolList();
        var afterCreateTools = parseNewlineSeparatedString(afterCreateStr);
        var newToolName = null;
        for (var i = 0; i < afterCreateTools.length; i++) {
            if (!arrayContains(initialTools, afterCreateTools[i])) {
                newToolName = afterCreateTools[i];
                break;
            }
        }

        printToConsole("  New tool created: " + newToolName);

        var check1 = TestFramework.supervisedCheck("Verify the new tool has an X button next to the + button");
        if (!check1) return false;

        var check2 = TestFramework.supervisedCheck("Click the X button to remove tool '" + newToolName + "'. Verify the tool is removed from the tools list");
        return check2;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 7: Verify Removal Disabled for Default Tool
// UID: TST.DATALOGGER.VERIFY_REMOVAL_DISABLED_FOR_DEFAULT_TOOL
// Description: Check if the remove tool button is disabled for the
//   default (first) tool.
// VISUAL: Verify X button is only on new tools, not on the default tool
// ===========================================================================
printToConsole("\n=== Test 7: Verify Removal Disabled for Default Tool (VISUAL) ===\n");

TestFramework.runTest("TST.DATALOGGER.VERIFY_REMOVAL_DISABLED_FOR_DEFAULT_TOOL", function() {
    try {
        // Create a new tool to compare
        printToConsole("  Creating new Data Logger tool...");
        var result = datalogger.createTool();
        msleep(500);

        if (result !== "Tool created") {
            printToConsole("  FAIL: Tool creation failed: " + result);
            return false;
        }

        var check1 = TestFramework.supervisedCheck("Verify the NEW tool has an X button available");
        if (!check1) return false;

        var check2 = TestFramework.supervisedCheck("Switch to the original/default Data Logger tool. Verify the DEFAULT tool does NOT have an X button");
        return check2;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 18: Set Y-Axis Minimum and Maximum Values
// UID: TST.DATALOGGER.SET_Y_AXIS_MIN_MAX_VALUES
// Description: Change the Y-Axis min and max value and check if the
//   Y-Axis displays data with the new min and max value.
// VISUAL: Verify Y-axis range updates in the plot
// ===========================================================================
printToConsole("\n=== Test 18: Set Y-Axis Min/Max Values (VISUAL) ===\n");

TestFramework.runTest("TST.DATALOGGER.SET_Y_AXIS_MIN_MAX_VALUES", function() {
    try {
        var monitorsStr = datalogger.showAvailableMonitors();
        var monitors = parseNewlineSeparatedString(monitorsStr);
        if (monitors.length === 0) {
            return "SKIP";
        }

        // Enable a monitor and start running
        datalogger.enableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        printToConsole("  Starting data acquisition...");
        datalogger.setRunning(true);
        msleep(2000);

        // Step 5-6: Set Y-axis min to 0
        printToConsole("  Setting Y-axis min to 0...");
        datalogger.setMinYAxis(0);
        msleep(500);

        var check1 = TestFramework.supervisedCheck("Verify Y-axis bottom value is now '0'");

        // Step 7: Set Y-axis max to 2
        printToConsole("  Setting Y-axis max to 2...");
        datalogger.setMaxYAxis(2);
        msleep(500);

        var check2 = TestFramework.supervisedCheck("Verify Y-axis top value is now '2'");

        // Test wider range
        printToConsole("  Setting Y-axis range to -10 to 10...");
        datalogger.setMinYAxis(-10);
        msleep(500);
        datalogger.setMaxYAxis(10);
        msleep(500);

        var check3 = TestFramework.supervisedCheck("Verify Y-axis displays range -10 to 10");

        // Cleanup - restore state
        datalogger.setRunning(false);
        msleep(500);
        datalogger.disableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        return check1 && check2 && check3;
    } catch (e) {
        printToConsole("  Error: " + e);
        datalogger.setRunning(false);
        return false;
    }
});

// ===========================================================================
// Test 22: Verify Plot Display Method
// UID: TST.DATALOGGER.VERIFY_PLOT_DISPLAY_METHOD
// Description: Verify that the application displays data in "Plot" mode.
// VISUAL: Verify data is displayed as a plot/graph
// ===========================================================================
printToConsole("\n=== Test 22: Verify Plot Display Method (VISUAL) ===\n");

TestFramework.runTest("TST.DATALOGGER.VERIFY_PLOT_DISPLAY_METHOD.VISUAL", function() {
    try {
        var monitorsStr = datalogger.showAvailableMonitors();
        var monitors = parseNewlineSeparatedString(monitorsStr);
        if (monitors.length === 0) {
            return "SKIP";
        }

        // Enable monitor and start running
        datalogger.enableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        printToConsole("  Starting data acquisition...");
        datalogger.setRunning(true);
        msleep(2000);

        // Switch to plot mode
        printToConsole("  Switching to Plot display mode...");
        datalogger.setDisplayMode(toolName, 0);
        msleep(500);

        var passed = TestFramework.supervisedCheck("Verify data is displayed in Plot mode (graph with curves)");

        // Cleanup - restore state
        datalogger.setRunning(false);
        msleep(500);
        datalogger.disableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        datalogger.setRunning(false);
        return false;
    }
});

// ===========================================================================
// Test 23: Verify Plain Text Display Method
// UID: TST.DATALOGGER.VERIFY_PLAIN_TEXT_DISPLAY_METHOD
// Description: Verify that the application displays data in "Plain Text" mode.
// VISUAL: Verify data is displayed as plain text values
// ===========================================================================
printToConsole("\n=== Test 23: Verify Plain Text Display Method (VISUAL) ===\n");

TestFramework.runTest("TST.DATALOGGER.VERIFY_PLAIN_TEXT_DISPLAY_METHOD.VISUAL", function() {
    try {
        var monitorsStr = datalogger.showAvailableMonitors();
        var monitors = parseNewlineSeparatedString(monitorsStr);
        if (monitors.length === 0) {
            return "SKIP";
        }

        datalogger.enableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        printToConsole("  Starting data acquisition...");
        datalogger.setRunning(true);
        msleep(2000);

        // Switch to text mode
        printToConsole("  Switching to Plain Text display mode...");
        datalogger.setDisplayMode(toolName, 1);
        msleep(500);

        var passed = TestFramework.supervisedCheck("Verify data is displayed in Plain Text mode (text values)");

        // Restore to plot mode and cleanup
        datalogger.setDisplayMode(toolName, 0);
        msleep(500);
        datalogger.setRunning(false);
        msleep(500);
        datalogger.disableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        datalogger.setRunning(false);
        return false;
    }
});

// ===========================================================================
// Test 24: Verify 7 Segment Display Method
// UID: TST.DATALOGGER.VERIFY_7_SEGMENT_DISPLAY_METHOD
// Description: Verify that the application displays data in "7 Segment" mode.
// VISUAL: Verify data is displayed in 7-segment style
// ===========================================================================
printToConsole("\n=== Test 24: Verify 7 Segment Display Method (VISUAL) ===\n");

TestFramework.runTest("TST.DATALOGGER.VERIFY_7_SEGMENT_DISPLAY_METHOD.VISUAL", function() {
    try {
        var monitorsStr = datalogger.showAvailableMonitors();
        var monitors = parseNewlineSeparatedString(monitorsStr);
        if (monitors.length === 0) {
            return "SKIP";
        }

        datalogger.enableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        printToConsole("  Starting data acquisition...");
        datalogger.setRunning(true);
        msleep(2000);

        // Switch to 7 segment mode
        printToConsole("  Switching to 7 Segment display mode...");
        datalogger.setDisplayMode(toolName, 2);
        msleep(500);

        var passed = TestFramework.supervisedCheck("Verify data is displayed in 7 Segment mode (large digit display)");

        // Restore to plot mode and cleanup
        datalogger.setDisplayMode(toolName, 0);
        msleep(500);
        datalogger.setRunning(false);
        msleep(500);
        datalogger.disableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        datalogger.setRunning(false);
        return false;
    }
});

// ===========================================================================
// Test 25: Toggle Between Display Methods
// UID: TST.DATALOGGER.TOGGLE_BETWEEN_DISPLAY_METHODS
// Description: Verify that the application toggles between display methods.
// VISUAL: Verify each mode transition displays data correctly
// ===========================================================================
printToConsole("\n=== Test 25: Toggle Between Display Methods (VISUAL) ===\n");

TestFramework.runTest("TST.DATALOGGER.TOGGLE_BETWEEN_DISPLAY_METHODS.VISUAL", function() {
    try {
        var monitorsStr = datalogger.showAvailableMonitors();
        var monitors = parseNewlineSeparatedString(monitorsStr);
        if (monitors.length === 0) {
            return "SKIP";
        }

        datalogger.enableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        printToConsole("  Starting data acquisition...");
        datalogger.setRunning(true);
        msleep(2000);

        // Step 4: Switch to Text
        printToConsole("  Switching to Text mode...");
        datalogger.setDisplayMode(toolName, 1);
        msleep(500);

        var check1 = TestFramework.supervisedCheck("Verify data is displayed in Plain Text mode");

        // Step 5: Switch to 7 Segment
        printToConsole("  Switching to 7 Segment mode...");
        datalogger.setDisplayMode(toolName, 2);
        msleep(500);

        var check2 = TestFramework.supervisedCheck("Verify data is displayed in 7 Segment mode");

        // Step 6: Switch back to Plot
        printToConsole("  Switching back to Plot mode...");
        datalogger.setDisplayMode(toolName, 0);
        msleep(500);

        var check3 = TestFramework.supervisedCheck("Verify data is displayed in Plot mode");

        // Cleanup - restore state
        datalogger.setRunning(false);
        msleep(500);
        datalogger.disableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        return check1 && check2 && check3;
    } catch (e) {
        printToConsole("  Error: " + e);
        datalogger.setRunning(false);
        return false;
    }
});

// ===========================================================================
// Test 26: Set 7 Segment Display Precision
// UID: TST.DATALOGGER.SET_7_SEGMENT_DISPLAY_PRECISION
// Description: Change the 7 Segment precision and check if it updates.
// VISUAL: Verify the number of decimal points changes in 7-segment display
// ===========================================================================
printToConsole("\n=== Test 26: Set 7 Segment Display Precision (VISUAL) ===\n");

TestFramework.runTest("TST.DATALOGGER.SET_7_SEGMENT_DISPLAY_PRECISION.VISUAL", function() {
    try {
        var monitorsStr = datalogger.showAvailableMonitors();
        var monitors = parseNewlineSeparatedString(monitorsStr);
        if (monitors.length === 0) {
            return "SKIP";
        }

        datalogger.enableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        printToConsole("  Starting data acquisition...");
        datalogger.setRunning(true);
        msleep(2000);

        // Switch to 7 segment mode
        datalogger.setDisplayMode(toolName, 2);
        msleep(500);

        // Set precision to 2 decimals
        printToConsole("  Setting precision to 2 decimals...");
        datalogger.changePrecision(2);
        msleep(500);

        var check1 = TestFramework.supervisedCheck("Verify 7 Segment displays values with 2 decimal points");

        // Set precision to 5 decimals
        printToConsole("  Setting precision to 5 decimals...");
        datalogger.changePrecision(5);
        msleep(500);

        var check2 = TestFramework.supervisedCheck("Verify 7 Segment displays values with 5 decimal points");

        // Restore defaults
        datalogger.changePrecision(3);
        msleep(500);
        datalogger.setDisplayMode(toolName, 0);
        msleep(500);
        datalogger.setRunning(false);
        msleep(500);
        datalogger.disableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        return check1 && check2;
    } catch (e) {
        printToConsole("  Error: " + e);
        datalogger.setRunning(false);
        return false;
    }
});

// ===========================================================================
// Test 27: Toggle 7 Segment Min/Max Display
// UID: TST.DATALOGGER.TOGGLE_7_SEGMENT_MIN_MAX_DISPLAY
// Description: Toggle 7 Segment min/max and verify display changes.
// VISUAL: Verify min/max values appear/disappear in 7-segment display
// ===========================================================================
printToConsole("\n=== Test 27: Toggle 7 Segment Min/Max Display (VISUAL) ===\n");

TestFramework.runTest("TST.DATALOGGER.TOGGLE_7_SEGMENT_MIN_MAX_DISPLAY.VISUAL", function() {
    try {
        var monitorsStr = datalogger.showAvailableMonitors();
        var monitors = parseNewlineSeparatedString(monitorsStr);
        if (monitors.length === 0) {
            return "SKIP";
        }

        datalogger.enableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        printToConsole("  Starting data acquisition...");
        datalogger.setRunning(true);
        msleep(2000);

        // Switch to 7 segment mode
        datalogger.setDisplayMode(toolName, 2);
        msleep(500);

        // Disable min/max
        printToConsole("  Disabling Min/Max display...");
        datalogger.setMinMax(false);
        msleep(500);

        var check1 = TestFramework.supervisedCheck("Verify 7 Segment displays data WITHOUT min/max values");

        // Enable min/max
        printToConsole("  Enabling Min/Max display...");
        datalogger.setMinMax(true);
        msleep(500);

        var check2 = TestFramework.supervisedCheck("Verify 7 Segment displays data WITH min/max values");

        // Restore state
        datalogger.setDisplayMode(toolName, 0);
        msleep(500);
        datalogger.setRunning(false);
        msleep(500);
        datalogger.disableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        return check1 && check2;
    } catch (e) {
        printToConsole("  Error: " + e);
        datalogger.setRunning(false);
        return false;
    }
});

// ===========================================================================
// Test 32: Set Maximum Channel Data Storage
// UID: TST.DATALOGGER.SET_MAXIMUM_CHANNEL_DATA_STORAGE
// Description: Verify that the user can set the maximum channel data storage
//   via Preferences.
// VISUAL: Verify the preference is applied in the DataLoggerPlugin tab
// ===========================================================================
printToConsole("\n=== Test 32: Set Maximum Channel Data Storage (VISUAL) ===\n");

TestFramework.runTest("TST.DATALOGGER.SET_MAXIMUM_CHANNEL_DATA_STORAGE.VISUAL", function() {
    try {
        // Set the preference via API
        printToConsole("  Setting data storage size to '1 Mb'...");
        scopy.setPreference("dataloggerplugin_data_storage_size", "1 Mb");
        msleep(500);

        var passed = TestFramework.supervisedCheck("Open Preferences > DataLoggerPlugin tab. Verify 'Maximum data stored for each monitor' is set to '1 Mb'");

        // Restore default
        scopy.setPreference("dataloggerplugin_data_storage_size", "10 Kb");
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        // Restore default on error
        scopy.setPreference("dataloggerplugin_data_storage_size", "10 Kb");
        return false;
    }
});

// ===========================================================================
// Test 33: Set Data Logger Read Interval
// UID: TST.DATALOGGER.SET_DATA_LOGGER_READ_INTERVAL
// Description: Verify that the user can set the read interval and that
//   data collection rate changes accordingly.
// VISUAL: Verify data is collected at the new interval rate
// ===========================================================================
printToConsole("\n=== Test 33: Set Data Logger Read Interval (VISUAL) ===\n");

TestFramework.runTest("TST.DATALOGGER.SET_DATA_LOGGER_READ_INTERVAL.VISUAL", function() {
    try {
        var monitorsStr = datalogger.showAvailableMonitors();
        var monitors = parseNewlineSeparatedString(monitorsStr);
        if (monitors.length === 0) {
            return "SKIP";
        }

        // Enable monitor and start
        datalogger.enableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        printToConsole("  Starting data acquisition with default interval...");
        datalogger.setRunning(true);
        msleep(2000);

        var check1 = TestFramework.supervisedCheck("Note the current data collection rate (default: 1 second interval)");

        // Change read interval to 2 seconds
        printToConsole("  Setting read interval to 2000 ms...");
        scopy.setPreference("dataloggerplugin_read_interval", "2000");
        msleep(500);

        var check2 = TestFramework.supervisedCheck("Verify data is now collected every 2 seconds (slower rate)");

        // Restore default
        scopy.setPreference("dataloggerplugin_read_interval", "1000");
        msleep(500);
        datalogger.setRunning(false);
        msleep(500);
        datalogger.disableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        return check1 && check2;
    } catch (e) {
        printToConsole("  Error: " + e);
        scopy.setPreference("dataloggerplugin_read_interval", "1000");
        datalogger.setRunning(false);
        return false;
    }
});

// ===========================================================================
// Test 34: Set X-Axis Date Time Format
// UID: TST.DATALOGGER.SET_X_AXIS_DATE_TIME_FORMAT
// Description: Verify that the user can set the date time format for the
//   X Axis via Preferences.
// VISUAL: Verify X-axis labels change to the new format
// ===========================================================================
printToConsole("\n=== Test 34: Set X-Axis Date Time Format (VISUAL) ===\n");

TestFramework.runTest("TST.DATALOGGER.SET_X_AXIS_DATE_TIME_FORMAT.VISUAL", function() {
    try {
        var monitorsStr = datalogger.showAvailableMonitors();
        var monitors = parseNewlineSeparatedString(monitorsStr);
        if (monitors.length === 0) {
            return "SKIP";
        }

        // Enable monitor and start
        datalogger.enableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        printToConsole("  Starting data acquisition...");
        datalogger.setRunning(true);
        msleep(2000);

        var check1 = TestFramework.supervisedCheck("Note the current X-axis date/time format (default: hh:mm:ss)");

        // Change format to mm:ss
        printToConsole("  Setting date time format to 'mm:ss'...");
        scopy.setPreference("dataloggerplugin_date_time_format", "mm:ss");
        msleep(500);

        var check2 = TestFramework.supervisedCheck("Verify X-axis now displays time in 'mm:ss' format");

        // Restore default
        scopy.setPreference("dataloggerplugin_date_time_format", "hh:mm:ss");
        msleep(500);
        datalogger.setRunning(false);
        msleep(500);
        datalogger.disableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        return check1 && check2;
    } catch (e) {
        printToConsole("  Error: " + e);
        scopy.setPreference("dataloggerplugin_date_time_format", "hh:mm:ss");
        datalogger.setRunning(false);
        return false;
    }
});

// ===========================================================================
// Test 35: Use Multiplot Feature
// UID: TST.DATALOGGER.USE_MULTIPLOT
// Description: Verify that the user can enable multiplot in preferences,
//   create multiple plots, and assign channels to different plots.
// VISUAL: Verify multiple plots are displayed and can be managed
// ===========================================================================
printToConsole("\n=== Test 35: Use Multiplot Feature (VISUAL) ===\n");

TestFramework.runTest("TST.DATALOGGER.USE_MULTIPLOT", function() {
    try {
        // Enable multiplot preference
        printToConsole("  Enabling multiplot preference...");
        scopy.setPreference("dataloggerplugin_add_remove_plot", "true");
        msleep(500);

        var check1 = TestFramework.supervisedCheck("Verify multiplot is enabled in Preferences > DataLoggerPlugin");

        var monitorsStr = datalogger.showAvailableMonitors();
        var monitors = parseNewlineSeparatedString(monitorsStr);
        if (monitors.length === 0) {
            // Restore and skip
            scopy.setPreference("dataloggerplugin_add_remove_plot", "false");
            msleep(500);
            return "SKIP";
        }

        // Enable a monitor and start
        datalogger.enableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        printToConsole("  Starting data acquisition...");
        datalogger.setRunning(true);
        msleep(2000);

        var check2 = TestFramework.supervisedCheck("Open Settings and press 'Add Plot' button. Assign a second channel to the new plot. Verify two plots are visible with data from assigned channels");

        var check3 = TestFramework.supervisedCheck("Remove the second plot. Verify only the first plot remains and its data is unaffected");

        // Restore state
        datalogger.setRunning(false);
        msleep(500);
        scopy.setPreference("dataloggerplugin_add_remove_plot", "false");
        msleep(500);
        datalogger.disableMonitorOfTool(toolName, monitors[0]);
        msleep(500);

        return check1 && check2 && check3;
    } catch (e) {
        printToConsole("  Error: " + e);
        datalogger.setRunning(false);
        scopy.setPreference("dataloggerplugin_add_remove_plot", "false");
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
TestFramework.printSummary();
scopy.exit();
