/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy Test Automation Framework
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

/**
 * DataLogger Plugin Tests for Scopy
 *
 * Tests core DataLogger functionality including channel management,
 * data collection, display modes, and file operations.
 * Based on docs/tests/plugins/datalogger/datalogger_tests.rst
 */

// Import framework modules
evaluateFile("../tests/framework/test_framework.js");
evaluateFile("../tests/framework/device_helpers.js");
evaluateFile("../tests/framework/core_helpers.js");
evaluateFile("../tests/framework/reporting.js");

// Test configuration - Simplified
var DATALOGGER_CONFIG = {
    device: "ip:192.168.2.1",
    channel: "xadc:temp0",
    channel2: "xadc:voltage0",
    waitTime: 2000
};

// Global test state
var connectedDeviceId = null;


/**
 * Simplified device setup and cleanup
 */
function setupDataLoggerTests() {
    connectedDeviceId = connectToDevice(DATALOGGER_CONFIG.device);
    return connectedDeviceId !== null;
}

function cleanupDataLoggerTests() {
    if (connectedDeviceId) {
        datalogger.setRunning(false);
        datalogger.disableMonitorOfTool("Data Logger ", DATALOGGER_CONFIG.channel);
        datalogger.disableMonitorOfTool("Data Logger ", DATALOGGER_CONFIG.channel2);
        disconnectFromDevice(connectedDeviceId);
        connectedDeviceId = null;
    }
}

/**
 * Test 1: Channel Enable/Disable Control
 */
function testDataLoggerChannelControl() {
    if (!scopy.switchTool("Data Logger ")) {
        return "Failed to switch to DataLogger";
    }
    safeWait(1000);

    var enableResult = datalogger.enableMonitorOfTool("Data Logger ", DATALOGGER_CONFIG.channel);
    if (enableResult !== "OK") {
        return "Failed to enable channel: " + enableResult;
    }

    datalogger.setRunning(true);
    safeWait(1000);
    datalogger.setRunning(false);

    var disableResult = datalogger.disableMonitorOfTool("Data Logger ", DATALOGGER_CONFIG.channel);
    if (disableResult !== "OK") {
        return "Failed to disable channel: " + disableResult;
    }

    printToConsole("Channel control test passed");
    return true;
}

/**
 * Test 2: Run/Stop Data Logging Operations
 */
function testDataLoggerRunStop() {
    if (!scopy.switchTool("Data Logger ")) {
        return "Failed to switch to DataLogger";
    }

    var enableResult = datalogger.enableMonitorOfTool("Data Logger ", DATALOGGER_CONFIG.channel);
    if (enableResult !== "OK") {
        return "Failed to enable channel: " + enableResult;
    }

    datalogger.setRunning(true);
    safeWait(DATALOGGER_CONFIG.waitTime);
    datalogger.setRunning(false);

    var disableResult = datalogger.disableMonitorOfTool("Data Logger ", DATALOGGER_CONFIG.channel);
    if (disableResult !== "OK") {
        return "Failed to disable channel: " + disableResult;
    }

    printToConsole("Run/Stop test passed");
    return true;
}

/**
 * Test 3: Data Management (Clear Data)
 */
function testDataLoggerClearData() {
    if (!scopy.switchTool("Data Logger ")) {
        return "Failed to switch to DataLogger";
    }

    var enableResult = datalogger.enableMonitorOfTool("Data Logger ", DATALOGGER_CONFIG.channel);
    if (enableResult !== "OK") {
        return "Failed to enable channel: " + enableResult;
    }

    datalogger.setRunning(true);
    safeWait(2000);
    datalogger.setRunning(false);
    datalogger.clearData();

    var disableResult = datalogger.disableMonitorOfTool("Data Logger ", DATALOGGER_CONFIG.channel);
    if (disableResult !== "OK") {
        return "Failed to disable channel: " + disableResult;
    }

    printToConsole("Clear data test passed");
    return true;
}

/**
 * Test 4: Display Mode Switching
 */
function testDataLoggerDisplayModes() {
    if (!scopy.switchTool("Data Logger ")) {
        return "Failed to switch to DataLogger";
    }

    var enableResult = datalogger.enableMonitorOfTool("Data Logger ", DATALOGGER_CONFIG.channel);
    if (enableResult !== "OK") {
        return "Failed to enable channel: " + enableResult;
    }

    datalogger.setRunning(true);
    safeWait(2000);

    datalogger.changeTool("Plot");
    safeWait(1000);
    datalogger.changeTool("Text");
    safeWait(1000);
    datalogger.changeTool("7 Segment");
    safeWait(1000);
    datalogger.changeTool("Plot");

    datalogger.setRunning(false);
    var disableResult = datalogger.disableMonitorOfTool("Data Logger ", DATALOGGER_CONFIG.channel);
    if (disableResult !== "OK") {
        return "Failed to disable channel: " + disableResult;
    }

    printToConsole("Display modes test passed");
    return true;
}

/**
 * Test 5: File Operations (Save/Load Data)
 */
function testDataLoggerFileOperations() {
    var testFilePath = "/tmp/datalogger_test.csv";

    if (!scopy.switchTool("Data Logger ")) {
        return "Failed to switch to DataLogger";
    }

    var enable1Result = datalogger.enableMonitorOfTool("Data Logger ", DATALOGGER_CONFIG.channel);
    if (enable1Result !== "OK") {
        return "Failed to enable first channel";
    }

    var enable2Result = datalogger.enableMonitorOfTool("Data Logger ", DATALOGGER_CONFIG.channel2);
    if (enable2Result !== "OK") {
        datalogger.disableMonitorOfTool("Data Logger ", DATALOGGER_CONFIG.channel);
        return "Failed to enable second channel";
    }

    datalogger.clearData();
    safeWait(500);

    datalogger.setRunning(true);
    datalogger.logAtPathForTool("Data Logger ", testFilePath);
    safeWait(3000);
    datalogger.setRunning(false);

    datalogger.importDataFromPathForTool("Data Logger ", testFilePath);
    safeWait(1000);

    datalogger.disableMonitorOfTool("Data Logger ", DATALOGGER_CONFIG.channel);
    datalogger.disableMonitorOfTool("Data Logger ", DATALOGGER_CONFIG.channel2);

    printToConsole("File operations test passed");
    return true;
}

/**
 * Main test execution function
 */
function main() {
    printToConsole("DataLogger Plugin Tests Starting...");

    if (!setupDataLoggerTests()) {
        printToConsole("Test setup failed - cannot connect to device");
        return;
    }

    var tests = [
        {name: "Channel Control", func: testDataLoggerChannelControl},
        {name: "Run/Stop", func: testDataLoggerRunStop},
        {name: "Clear Data", func: testDataLoggerClearData},
        {name: "Display Modes", func: testDataLoggerDisplayModes},
        {name: "File Operations", func: testDataLoggerFileOperations}
    ];

    var passCount = 0, failCount = 0;

    for (var i = 0; i < tests.length; i++) {
        var test = tests[i];
        var result = test.func();

        if (result === true) {
            passCount++;
        } else {
            failCount++;
            printToConsole(test.name + " failed: " + result);
        }
    }

    printToConsole("Results: " + passCount + " passed, " + failCount + " failed");
    cleanupDataLoggerTests();
}

// Execute tests
main();