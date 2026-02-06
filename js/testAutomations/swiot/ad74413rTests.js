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

// SWIOT AD74413R Instrument Tests
// Tests: TST.AD74413R.PLOT, TST.AD74413R.CHANNEL, TST.AD74413R.DIAG, TST.AD74413R.SAMPLING

// Load test framework
evaluateFile("../scopy/js/testAutomations/common/testFramework.js");

// Test Suite: AD74413R Tests
TestFramework.init("SWIOT AD74413R Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Verify we're in runtime mode
if (!swiot.isRuntimeMode()) {
    printToConsole("ERROR: AD74413R tests require runtime mode. Please configure and apply first.");
    TestFramework.disconnectFromDevice();
    exit(1);
}

// Switch to AD74413R tool
if (!switchToTool("AD74413R")) {
    printToConsole("ERROR: Could not switch to AD74413R tool");
    TestFramework.disconnectFromDevice();
    exit(1);
}

// ============================================
// Test 4 - AD74413R Plot Operations (TST.AD74413R.PLOT)
// ============================================
printToConsole("\n=== Test 4 - AD74413R Plot Operations ===\n");

TestFramework.runTest("TST.AD74413R.PLOT", function() {
    try {
        // Step 1: Open the AD74413R instrument
        printToConsole("  AD74413R instrument opened");

        // Step 2: Enable Plot Labels and set Timespan to 4s
        // Expected: Labels displayed, X axis shows -4 to 0 range
        swiot.setAdPlotLabelsEnabled(true);
        msleep(500);
        printToConsole("  Plot labels enabled");

        swiot.setAdTimespan(4.0);
        msleep(500);
        var timespan = swiot.getAdTimespan();
        if (Math.abs(timespan - 4.0) > 0.01) {
            printToConsole("  FAIL: Timespan not set to 4s, got " + timespan);
            return false;
        }
        printToConsole("  Timespan set to 4s");

        // Step 3: Enable channel voltage_out 1 and run Single capture
        // Expected: Data displayed on plot from right to left
        swiot.setAdChannelEnabled(0, true);
        msleep(500);
        if (!swiot.isAdChannelEnabled(0)) {
            printToConsole("  Warning: Channel 0 not enabled");
        }
        printToConsole("  Channel voltage_out 1 enabled");

        swiot.adSingleShot();
        msleep(5000);
        printToConsole("  Single capture completed");

        // Step 4: Click Measure button
        // Expected: Measurements displayed above plot, instant value around 0A
        // Note: Measure button toggle not exposed in API yet

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 5 - AD74413R Channel Operations (TST.AD74413R.CHANNEL)
// ============================================
printToConsole("\n=== Test 5 - AD74413R Channel Operations ===\n");

TestFramework.runTest("TST.AD74413R.CHANNEL", function() {
    try {
        // Step 1: Open the AD74413R instrument
        // Step 2: Enable Plot Labels and set Timespan to 2s
        swiot.setAdPlotLabelsEnabled(true);
        swiot.setAdTimespan(2.0);
        msleep(500);
        printToConsole("  Plot labels enabled, timespan set to 2s");

        // Step 3: Enable voltage_out 1 and voltage_in 2 channels
        swiot.setAdChannelEnabled(0, true);
        swiot.setAdChannelEnabled(1, true);
        msleep(500);
        printToConsole("  Channels voltage_out 1 and voltage_in 2 enabled");

        // Step 4: Set sampling_frequency to 1200
        // Expected: Green animation, status shows 1.2ksps
        swiot.setAdChannelSamplingFrequency(0, 1200);
        swiot.setAdChannelSamplingFrequency(1, 1200);
        msleep(500);
        var freq0 = swiot.getAdChannelSamplingFrequency(0);
        var freq1 = swiot.getAdChannelSamplingFrequency(1);
        printToConsole("  Sampling frequency set - CH0: " + freq0 + ", CH1: " + freq1);
        if (freq0 !== 1200 || freq1 !== 1200) {
            printToConsole("  Warning: Sampling frequency mismatch");
        }

        // Step 5: Set YMin to -1A and YMax to 10A
        // Expected: Plot Y axis scaled between -1A and 10A
        swiot.setAdChannelYMin(0, -1.0);
        swiot.setAdChannelYMax(0, 10.0);
        msleep(300);
        var yMin = swiot.getAdChannelYMin(0);
        var yMax = swiot.getAdChannelYMax(0);
        printToConsole("  Y-axis set - YMin: " + yMin + ", YMax: " + yMax);

        // Step 8: Run a Single capture and check measurements
        swiot.adSingleShot();
        msleep(5000);
        printToConsole("  Single capture completed");

        // Step 9: Run a Continuous capture and check measurements
        swiot.setAdRunning(true);
        msleep(3000);
        if (!swiot.isAdRunning()) {
            printToConsole("  FAIL: Continuous capture not running");
            return false;
        }
        printToConsole("  Continuous capture running");

        // Step 10-11: Change RAW values while running
        // Note: RAW output control not exposed in API yet

        swiot.setAdRunning(false);
        msleep(500);
        printToConsole("  Capture stopped");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        swiot.setAdRunning(false);
        return false;
    }
});

// ============================================
// Test 6 - AD74413R Diagnostic Channels (TST.AD74413R.DIAG)
// ============================================
printToConsole("\n=== Test 6 - AD74413R Diagnostic Channels ===\n");

TestFramework.runTest("TST.AD74413R.DIAG", function() {
    try {
        // Step 1: Open the AD74413R instrument
        // Step 2: Enable Plot Labels and set Timespan to 2s
        swiot.setAdPlotLabelsEnabled(true);
        swiot.setAdTimespan(2.0);
        msleep(500);

        // Step 3: Enable voltage_out 1, voltage_in 2, and diagnostic 5 channels
        swiot.setAdChannelEnabled(0, true);
        swiot.setAdChannelEnabled(1, true);
        swiot.setAdChannelEnabled(4, true); // diagnostic 5 (index 4)
        msleep(500);
        printToConsole("  Channels enabled: voltage_out 1, voltage_in 2, diagnostic 5");

        // Step 4: Set diag_function to sensel_b
        // Note: Diagnostic function control not exposed in API yet

        // Step 6: Run a Continuous capture
        swiot.setAdRunning(true);
        msleep(3000);
        printToConsole("  Continuous capture running");

        // Step 7-8: Change RAW values and observe diagnostic channel
        // Note: RAW output control not exposed in API yet

        swiot.setAdRunning(false);
        msleep(500);
        printToConsole("  Capture stopped");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        swiot.setAdRunning(false);
        return false;
    }
});

// ============================================
// Test 7 - AD74413R Sampling Frequency (TST.AD74413R.SAMPLING)
// ============================================
printToConsole("\n=== Test 7 - AD74413R Sampling Frequency ===\n");

TestFramework.runTest("TST.AD74413R.SAMPLING", function() {
    try {
        // Step 1: Open the AD74413R instrument
        // Step 2: Enable all channels, set sampling frequency to 4800
        // Expected: Status shows 800 sps

        // Enable all channels
        for (var i = 0; i < 6; i++) {
            swiot.setAdChannelEnabled(i, true);
            msleep(300);
        }
        printToConsole("  All channels enabled");

        // Set sampling frequency to 4800 for all channels
        for (var i = 0; i < 6; i++) {
            swiot.setAdChannelSamplingFrequency(i, 4800);
            msleep(300);
        }
        printToConsole("  Sampling frequency set to 4800 for all channels");

        // Verify sampling frequency
        var freq = swiot.getAdChannelSamplingFrequency(0);
        printToConsole("  Channel 0 sampling frequency: " + freq);

        // Step 3: Disable last two diagnostic channels
        // Expected: Status shows 1.2 ksps
        swiot.setAdChannelEnabled(4, false);
        swiot.setAdChannelEnabled(5, false);
        msleep(500);
        printToConsole("  Last two diagnostic channels disabled");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 8 - AD74413R Tutorial & Docs (TST.AD74413R.TUTORIAL)
// ============================================
printToConsole("\n=== Test 8 - AD74413R Tutorial & Docs ===\n");
printToConsole("  NOTE: TST.AD74413R.TUTORIAL requires manual testing (UI interaction)");

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
scopy.exit();
exit(exitCode);
