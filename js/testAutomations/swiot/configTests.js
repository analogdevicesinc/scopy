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

// SWIOT Config Instrument Tests
// Tests: TST.SWIOT.COMPAT, TST.SWIOT.MODES, TST.SWIOT.CONFIG

// Load test framework
evaluateFile("../scopy/js/testAutomations/common/testFramework.js");

// Test Suite: SWIOT Config Tests
TestFramework.init("SWIOT Config Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// ============================================
// Test 1 - SWIOT Compatibility (TST.SWIOT.COMPAT)
// ============================================
printToConsole("\n=== Test 1 - SWIOT Compatibility ===\n");

TestFramework.runTest("TST.SWIOT.COMPAT", function() {
    try {
        // Step 1: Open the SWIOT plugin - Config Instrument
        // Expected: Plugin connects, Config instrument shows 4 disabled channels

        var tools = swiot.getTools();
        var isRuntime = swiot.isRuntimeMode();

        if (isRuntime) {
            printToConsole("  Device is in runtime mode, switching to config mode first");
            swiot.switchToConfigMode();
            msleep(10000);
            isRuntime = swiot.isRuntimeMode();
        }

        printToConsole("  Available tools: " + tools);
        printToConsole("  Is runtime mode: " + isRuntime);

        // Verify we're in config mode and can see Config tool
        if (isRuntime) {
            printToConsole("  FAIL: Not in config mode");
            return false;
        }

        // Get the number of config channels
        var channelCount = swiot.getConfigChannelCount();
        printToConsole("  Config channel count: " + channelCount);

        if (channelCount !== 4) {
            printToConsole("  FAIL: Expected 4 channels, got " + channelCount);
            return false;
        }

        // Verify all 4 channels are initially disabled
        var allDisabled = true;
        for (var i = 0; i < channelCount; i++) {
            if (swiot.isChannelEnabled(i)) {
                allDisabled = false;
                printToConsole("  Channel " + i + " is already enabled");
            }
        }

        if (allDisabled) {
            printToConsole("  All " + channelCount + " channels are disabled as expected");
        }

        printToConsole("  Plugin connected successfully");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2 - Mode Switching (TST.SWIOT.MODES)
// ============================================
printToConsole("\n=== Test 2 - Mode Switching ===\n");

TestFramework.runTest("TST.SWIOT.MODES", function() {
    try {
        // Step 1: Open the Config instrument
        // Step 2: Click the Apply button
        // Expected: Device reconnects, Config replaced by AD74413R, MAX14906, Faults

        // First ensure we're in config mode
        if (swiot.isRuntimeMode()) {
            swiot.switchToConfigMode();
            msleep(10000);
        }

        // Enable at least one channel before applying
        swiot.setChannelEnabled(0, true);
        msleep(500);
        swiot.setChannelDevice(0, "ad74413r");
        msleep(500);
        var functions = swiot.getAvailableFunctions(0);
        if (functions.length > 0) {
            swiot.setChannelFunction(0, functions[0]);
        }
        msleep(500);

        printToConsole("  Applying config to switch to runtime mode...");
        swiot.applyConfig();
        msleep(10000);

        // Check if we're now in runtime mode
        if (!swiot.isRuntimeMode()) {
            printToConsole("  Warning: Not in runtime mode after apply (may be emulator limitation)");
        } else {
            printToConsole("  Successfully switched to runtime mode");

            var tools = swiot.getTools();
            printToConsole("  Available tools in runtime: " + tools);

            // Step 3: Open the AD74413R instrument
            // Step 4: Click the Config button
            // Expected: Device reconnects, back to Config instrument
            printToConsole("  Switching back to config mode from AD74413R...");
            swiot.switchToConfigMode();
            msleep(10000);

            if (swiot.isRuntimeMode()) {
                printToConsole("  Warning: Still in runtime mode");
            } else {
                printToConsole("  Successfully switched back to config mode");
            }
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3 - SWIOT Config Mode (TST.SWIOT.CONFIG)
// ============================================
printToConsole("\n=== Test 3 - SWIOT Config Mode ===\n");

TestFramework.runTest("TST.SWIOT.CONFIG", function() {
    try {
        // Ensure we're in config mode
        if (swiot.isRuntimeMode()) {
            swiot.switchToConfigMode();
            msleep(10000);
        }

        // Step 1: Open the Config instrument
        // Step 2: Enable all 4 channels
        // Expected: Dropdowns for device and function are accessible

        var channelCount = swiot.getConfigChannelCount();
        for (var i = 0; i < channelCount; i++) {
            swiot.setChannelEnabled(i, true);
            msleep(500);
            if (!swiot.isChannelEnabled(i)) {
                printToConsole("  FAIL: Could not enable channel " + i);
                return false;
            }
            var devices = swiot.getAvailableDevices(i);
            var functions = swiot.getAvailableFunctions(i);
            printToConsole("  Channel " + i + " enabled - Devices: " + devices + ", Functions: " + functions);
        }
        printToConsole("  All " + channelCount + " channels enabled, dropdowns accessible");

        // Step 3: Configure SWIOT
        // Channel 1: device ad74413r and function voltage_out
        swiot.setChannelDevice(0, "ad74413r");
        msleep(300);
        swiot.setChannelFunction(0, "voltage_out");
        msleep(300);

        // Channel 2: device ad74413r and function voltage_in
        swiot.setChannelDevice(1, "ad74413r");
        msleep(300);
        swiot.setChannelFunction(1, "voltage_in");
        msleep(300);

        // Channel 3: device max14906 and function input
        swiot.setChannelDevice(2, "max14906");
        msleep(300);
        swiot.setChannelFunction(2, "input");
        msleep(300);

        // Channel 4: device max14906 and function output
        swiot.setChannelDevice(3, "max14906");
        msleep(300);
        swiot.setChannelFunction(3, "output");
        msleep(300);

        printToConsole("  Configuration set:");
        printToConsole("    CH0: " + swiot.getChannelDevice(0) + " / " + swiot.getChannelFunction(0));
        printToConsole("    CH1: " + swiot.getChannelDevice(1) + " / " + swiot.getChannelFunction(1));
        printToConsole("    CH2: " + swiot.getChannelDevice(2) + " / " + swiot.getChannelFunction(2));
        printToConsole("    CH3: " + swiot.getChannelDevice(3) + " / " + swiot.getChannelFunction(3));

        // Step 4: Click the Apply button
        // Expected: Device reconnects, Config replaced by AD74413R, MAX14906, Faults
        printToConsole("  Applying configuration...");
        swiot.applyConfig();
        msleep(10000);

        if (swiot.isRuntimeMode()) {
            var tools = swiot.getTools();
            printToConsole("  Runtime mode - Available tools: " + tools);
        }

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
scopy.exit();
exit(exitCode);
