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

// DAC Plugin Manual Test Automation - Documentation Tests
// Automates tests from: docs/tests/plugins/dac/dac_tests.rst
//
// Automated: TST.DAC.COMPAT, TST.DAC.BUFFER_FILE_SIZE
//
// ==========================================================================
// MISSING API REPORT
// The following tests cannot be automated due to missing API functionality.
// ==========================================================================
//
// TST.DAC.DDS_MODES — DDS modes
//   Cannot automate: No API to set DDS sub-mode (One CW Tone / Two CW Tones /
//     Independent I/Q Control). The DDS mode combo is internal to DdsDacAddon.
//   Missing API: No method to change the DDS sub-mode programmatically
//   Suggested: dac.setDdsMode(int deviceIndex, QString mode) -> void
//              dac.getDdsMode(int deviceIndex) -> QString
//   Affected file: packages/generic-plugins/plugins/dac/include/dac/dac_api.h
//
// TST.DAC.DDS_SINEWAVE — DDS mode sinewave generation
//   Cannot automate: No DDS sub-mode API (see above), requires Debugger and ADC
//     cross-plugin interaction, visual sinewave verification on ADC plot,
//     physical loopback cable between RX and TX.
//   Missing API: dac.setDdsMode(int, QString)
//   Suggested: dac.setDdsMode(int, QString) -> void
//   Affected file: packages/generic-plugins/plugins/dac/include/dac/dac_api.h
//
// TST.DAC.DDS_ATTRS — DDS mode attributes refresh
//   Cannot automate: No DDS sub-mode API, no refresh attributes button API,
//     requires Debugger cross-plugin to change IIO attributes directly.
//   Missing API: dac.setDdsMode(int, QString), dac.refreshAttributes(int)
//   Suggested: dac.refreshAttributes(int deviceIndex) -> void
//   Affected file: packages/generic-plugins/plugins/dac/include/dac/dac_api.h
//
// TST.DAC.TUTORIAL — Plugin tutorial
//   Cannot automate: Requires Info button UI interaction, Documentation button
//     opens external browser, Tutorial button opens walkthrough overlay.
//   Missing API: No API to trigger Info panel, Documentation link, or Tutorial
//   Suggested: dac.openInfoPanel() -> void
//              dac.openTutorial() -> void
//              dac.openDocumentation() -> void
//   Affected file: packages/generic-plugins/plugins/dac/include/dac/dac_api.h
//
// TST.DAC.CHANNEL_ATTRS — Channel attributes
//   Cannot automate: No API for individual channel IIO attribute read/write
//     (e.g., sampling_frequency). The DAC API lacks getWidgetKeys/readWidget/
//     writeWidget and has no channel attribute accessors.
//   Missing API: Channel attribute get/set methods
//   Suggested: dac.getChannelAttribute(int deviceIndex, QString channelUuid,
//                QString attrName) -> QString
//              dac.setChannelAttribute(int deviceIndex, QString channelUuid,
//                QString attrName, QString value) -> void
//   Affected file: packages/generic-plugins/plugins/dac/include/dac/dac_api.h
//
// ==========================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Helper: check if item exists in array
function arrayContains(arr, item) {
    for (var i = 0; i < arr.length; i++) {
        if (arr[i] === item) return true;
    }
    return false;
}

// Helper: check if any array item contains a substring
function arrayContainsSubstring(arr, substring) {
    for (var i = 0; i < arr.length; i++) {
        if (arr[i].indexOf(substring) !== -1) return true;
    }
    return false;
}

// ============================================================================
// CSV resource file paths
// The DAC plugin bundles CSV test files at build time. Adjust CSV_PATH below
// to match your build layout. Common options:
//   - Build dir relative: "packages/generic-plugins/plugins/dac/resources/dac-csv/"
//   - Source tree:         "../packages/generic-plugins/plugins/dac/resources/dac-csv/"
// ============================================================================
var CSV_PATH = "../packages/generic-plugins/plugins/dac/resources/dac-csv/";
var CSV_40K = CSV_PATH + "sine_complex_1_15360_sr_15360000_samps_40000_ampl_32767.csv";

// Test Suite
TestFramework.init("DAC Documentation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to DAC tool
if (!switchToTool("DAC")) {
    printToConsole("ERROR: Cannot switch to DAC tool");
    scopy.exit();
}

// ============================================
// Test 1: Compatibility with device
// UID: TST.DAC.COMPAT
// Description: Verify that the DAC plugin is compatible with the selected
//   device and that the plugin is able to correctly parse it.
// ============================================
printToConsole("\n=== Test 1: Compatibility with device ===\n");

TestFramework.runTest("TST.DAC.COMPAT", function() {
    try {
        // Save original device mode
        var originalMode = dac.getDeviceMode(0);
        printToConsole("  Original device mode: " + originalMode);

        // Step 1: Verify DAC tool is accessible
        var tools = dac.getTools();
        printToConsole("  Available tools: " + tools);
        if (!arrayContains(tools, "DAC")) {
            printToConsole("  FAIL: DAC tool not found in tool list");
            return false;
        }
        printToConsole("  PASS: DAC tool found in tool list");

        // Step 2: Switch to Buffer mode and check channels list
        dac.setDeviceMode(0, "Buffer");
        msleep(500);

        var channels = dac.getBufferChannels(0);
        printToConsole("  Buffer channels (" + channels.length + "): " + channels);

        var hasVoltage0 = arrayContainsSubstring(channels, "voltage0");
        var hasVoltage1 = arrayContainsSubstring(channels, "voltage1");

        if (!hasVoltage0 || !hasVoltage1) {
            printToConsole("  FAIL: Expected channels containing voltage0 and voltage1");
            printToConsole("    voltage0 found: " + hasVoltage0);
            printToConsole("    voltage1 found: " + hasVoltage1);
            dac.setDeviceMode(0, originalMode);
            return false;
        }
        printToConsole("  PASS: Buffer channels contain voltage0 and voltage1");

        // Step 3: Switch to DDS mode and check TX section
        dac.setDeviceMode(0, "DDS");
        msleep(500);

        var txList = dac.getDdsTxList(0);
        printToConsole("  DDS TX list (" + txList.length + "): " + txList);

        if (txList.length < 1) {
            printToConsole("  FAIL: No TX entries found in DDS mode");
            dac.setDeviceMode(0, originalMode);
            return false;
        }
        printToConsole("  PASS: DDS TX section has " + txList.length + " entry/entries");

        // Restore original mode
        dac.setDeviceMode(0, originalMode);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: Buffer mode file size validation
// UID: TST.DAC.BUFFER_FILE_SIZE
// Description: Verify that the DAC plugin validates the file size in buffer
//   mode. Tests clamping to max, setting lower values, and increment/decrement.
//   Note: Steps 8-9 (physical +/- buttons) are simulated via setFileSize().
// Preconditions: AdalmPluto.Device (physical device required)
// ============================================
printToConsole("\n=== Test 4: Buffer mode file size validation ===\n");

TestFramework.runTest("TST.DAC.BUFFER_FILE_SIZE", function() {
    try {
        // Save original state
        var originalMode = dac.getDeviceMode(0);

        // Step 2: Switch to Buffer mode
        dac.setDeviceMode(0, "Buffer");
        msleep(500);

        // Steps 3-4: Load the 40k sample CSV file
        printToConsole("  Loading CSV file...");
        dac.loadFile(0, CSV_40K);
        msleep(1000);

        // Step 5: Check file size after loading
        var loadedSize = dac.getFileSize(0);
        printToConsole("  File size after loading: " + loadedSize);

        if (loadedSize <= 0) {
            printToConsole("  FAIL: File size is 0 or negative after loading");
            dac.setDeviceMode(0, originalMode);
            return false;
        }
        printToConsole("  PASS: File size populated: " + loadedSize);

        // Step 6: Set file size higher than actual — should be clamped to max
        var higherValue = loadedSize + 10000;
        printToConsole("  Setting file size to " + higherValue + " (above max)...");
        dac.setFileSize(0, higherValue);
        msleep(500);

        var clampedSize = dac.getFileSize(0);
        printToConsole("  After setting higher: " + clampedSize);

        if (clampedSize > loadedSize) {
            printToConsole("  FAIL: File size (" + clampedSize + ") exceeded loaded file size (" + loadedSize + ")");
            dac.setFileSize(0, loadedSize);
            dac.setDeviceMode(0, originalMode);
            return false;
        }
        printToConsole("  PASS: File size clamped to max (" + clampedSize + ")");

        // Step 7: Set file size lower than actual
        var lowerValue = loadedSize * 0.75;
        printToConsole("  Setting file size to " + lowerValue + " (below max)...");
        dac.setFileSize(0, lowerValue);
        msleep(500);

        var reducedSize = dac.getFileSize(0);
        printToConsole("  After setting lower: " + reducedSize);

        if (reducedSize >= loadedSize) {
            printToConsole("  FAIL: File size was not reduced (got " + reducedSize + ", expected < " + loadedSize + ")");
            dac.setFileSize(0, loadedSize);
            dac.setDeviceMode(0, originalMode);
            return false;
        }
        printToConsole("  PASS: File size reduced to " + reducedSize);

        // Steps 8-9: Simulate +/- button (increment then decrement)
        var beforeIncrement = dac.getFileSize(0);
        var incrementedValue = beforeIncrement + 1000;
        printToConsole("  Simulating '+' button: " + beforeIncrement + " -> " + incrementedValue);
        dac.setFileSize(0, incrementedValue);
        msleep(500);

        var afterIncrement = dac.getFileSize(0);
        printToConsole("  After increment: " + afterIncrement);

        if (afterIncrement <= beforeIncrement) {
            printToConsole("  FAIL: File size did not increase after increment");
            dac.setFileSize(0, loadedSize);
            dac.setDeviceMode(0, originalMode);
            return false;
        }
        printToConsole("  PASS: File size incremented to " + afterIncrement);

        printToConsole("  Simulating '-' button: " + afterIncrement + " -> " + beforeIncrement);
        dac.setFileSize(0, beforeIncrement);
        msleep(500);

        var afterDecrement = dac.getFileSize(0);
        printToConsole("  After decrement: " + afterDecrement);

        if (afterDecrement > afterIncrement) {
            printToConsole("  FAIL: File size did not decrease after decrement");
            dac.setFileSize(0, loadedSize);
            dac.setDeviceMode(0, originalMode);
            return false;
        }
        printToConsole("  PASS: File size decremented to " + afterDecrement);

        // Restore original state
        dac.setFileSize(0, loadedSize);
        msleep(500);
        dac.setDeviceMode(0, originalMode);
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
