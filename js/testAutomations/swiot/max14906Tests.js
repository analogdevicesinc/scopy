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

// SWIOT MAX14906 Instrument Tests
// Tests: TST.MAX14906.PLOT, TST.MAX14906.CHANNEL

// Load test framework
evaluateFile("../scopy/js/testAutomations/common/testFramework.js");

// Test Suite: MAX14906 Tests
TestFramework.init("SWIOT MAX14906 Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Verify we're in runtime mode
if (!swiot.isRuntimeMode()) {
    printToConsole("ERROR: MAX14906 tests require runtime mode. Please configure and apply first.");
    TestFramework.disconnectFromDevice();
    exit(1);
}

// Switch to MAX14906 tool
if (!switchToTool("MAX14906")) {
    printToConsole("ERROR: Could not switch to MAX14906 tool");
    TestFramework.disconnectFromDevice();
    exit(1);
}

// ============================================
// Test 9 - MAX14906 Plot Operations (TST.MAX14906.PLOT)
// ============================================
printToConsole("\n=== Test 9 - MAX14906 Plot Operations ===\n");

TestFramework.runTest("TST.MAX14906.PLOT", function() {
    try {
        // Step 1: Open the MAX14906 instrument
        printToConsole("  MAX14906 instrument opened");

        // Step 2: Set Timespan to 10s
        // Expected: X axis shows 0 to 10 range
        swiot.setMaxTimespan(10.0);
        msleep(500);
        var timespan = swiot.getMaxTimespan();
        if (Math.abs(timespan - 10.0) > 0.01) {
            printToConsole("  FAIL: Timespan not set to 10s, got " + timespan);
            return false;
        }
        printToConsole("  Timespan set to 10s");

        // Step 3: Set Timespan to 1s
        // Expected: X axis shows 0 to 1 range
        swiot.setMaxTimespan(1.0);
        msleep(500);
        timespan = swiot.getMaxTimespan();
        if (Math.abs(timespan - 1.0) > 0.01) {
            printToConsole("  FAIL: Timespan not set to 1s, got " + timespan);
            return false;
        }
        printToConsole("  Timespan set to 1s");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 10 - MAX14906 Channel Operations (TST.MAX14906.CHANNEL)
// ============================================
printToConsole("\n=== Test 10 - MAX14906 Channel Operations ===\n");

TestFramework.runTest("TST.MAX14906.CHANNEL", function() {
    try {
        // Step 1: Open the MAX14906 instrument
        // Step 2: Set Timespan to 1s
        swiot.setMaxTimespan(1.0);
        msleep(500);
        printToConsole("  Timespan set to 1s");

        // Step 3: Run a continuous capture
        swiot.setMaxRunning(true);
        msleep(1000);
        if (!swiot.isMaxRunning()) {
            printToConsole("  FAIL: Could not start continuous capture");
            return false;
        }
        printToConsole("  Continuous capture started");

        // Step 4: Set Output for voltage3 to ON
        // Expected: Plot trace rises from 0 to 1 for both channels
        swiot.setMaxChannelOutput(1, true);
        msleep(1500);
        if (!swiot.getMaxChannelOutput(1)) {
            printToConsole("  FAIL: Could not set channel output to ON");
            swiot.setMaxRunning(false);
            return false;
        }
        printToConsole("  Output for voltage3 set to ON");

        // Step 5: Set Output for voltage3 to OFF
        // Expected: Plot trace drops from 1 to 0 for both channels
        swiot.setMaxChannelOutput(1, false);
        msleep(1500);
        if (swiot.getMaxChannelOutput(1)) {
            printToConsole("  FAIL: Could not set channel output to OFF");
            swiot.setMaxRunning(false);
            return false;
        }
        printToConsole("  Output for voltage3 set to OFF");

        swiot.setMaxRunning(false);
        msleep(500);
        printToConsole("  Continuous capture stopped");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        swiot.setMaxRunning(false);
        return false;
    }
});

// ============================================
// Test 11 - MAX14906 Tutorial & Docs (TST.MAX14906.TUTORIAL)
// ============================================
printToConsole("\n=== Test 11 - MAX14906 Tutorial & Docs ===\n");
printToConsole("  NOTE: TST.MAX14906.TUTORIAL requires manual testing (UI interaction)");

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
scopy.exit();
exit(exitCode);
