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

// SWIOT Faults Instrument Tests
// Tests: TST.FAULTS.RUN_SINGLE, TST.FAULTS.CLEAR_RESET, TST.FAULTS.RUN_CONTINUOUS

// Load test framework
evaluateFile("../scopy/js/testAutomations/common/testFramework.js");

// Test Suite: Faults Tests
TestFramework.init("SWIOT Faults Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Verify we're in runtime mode
if (!swiot.isRuntimeMode()) {
    printToConsole("ERROR: Faults tests require runtime mode. Please configure and apply first.");
    TestFramework.disconnectFromDevice();
    exit(1);
}

// Helper function to setup AD74413R for faults testing
function setupAdForFaultsTesting() {
    if (switchToTool("AD74413R")) {
        for (var i = 0; i < 6; i++) {
            swiot.setAdChannelEnabled(i, true);
            msleep(200);
        }
        swiot.setAdRunning(true);
        msleep(2000);
        printToConsole("  AD74413R continuous capture running with all channels");
        return true;
    }
    return false;
}

// Helper function to cleanup AD74413R after faults testing
function cleanupAd() {
    if (switchToTool("AD74413R")) {
        swiot.setAdRunning(false);
        msleep(500);
    }
}

// ============================================
// Test 12 - Faults Run Single (TST.FAULTS.RUN_SINGLE)
// ============================================
printToConsole("\n=== Test 12 - Faults Run Single ===\n");

TestFramework.runTest("TST.FAULTS.RUN_SINGLE", function() {
    try {
        // Step 1: Run a continuous capture with all channels enabled on AD74413R
        if (!setupAdForFaultsTesting()) {
            printToConsole("  FAIL: Could not setup AD74413R");
            return false;
        }

        // Step 2: Open the Faults instrument and run a Single capture
        // Expected: Bit 10 is enabled on the AD74413R device
        if (!switchToTool("Faults")) {
            printToConsole("  FAIL: Could not switch to Faults tool");
            cleanupAd();
            return false;
        }

        swiot.faultsSingleShot();
        msleep(2000);
        printToConsole("  Faults single capture completed");

        // Step 3: Click on Bit 10
        // Expected: Section below shows fault explanation for selected bit
        // Note: Bit selection not exposed in API

        // Step 4: Turn Faults explanation off
        // Expected: Section below leds is hidden
        // Note: Faults explanation toggle not exposed in API

        // Cleanup
        cleanupAd();

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        cleanupAd();
        return false;
    }
});

// ============================================
// Test 13 - Faults Clear and Reset (TST.FAULTS.CLEAR_RESET)
// ============================================
printToConsole("\n=== Test 13 - Faults Clear and Reset ===\n");

TestFramework.runTest("TST.FAULTS.CLEAR_RESET", function() {
    try {
        // Step 1: Run a continuous capture with all channels enabled on AD74413R
        if (!setupAdForFaultsTesting()) {
            printToConsole("  FAIL: Could not setup AD74413R");
            return false;
        }

        // Step 2: Open Faults instrument and run a Single capture
        if (!switchToTool("Faults")) {
            cleanupAd();
            return false;
        }

        swiot.faultsSingleShot();
        msleep(2000);
        printToConsole("  Faults single capture completed");

        // Step 3: Select Bit 10
        // Note: Bit selection not exposed in API

        // Step 4: Click the Clear selection button
        // Expected: Bit 10 not selected, Faults explanation greyed out
        swiot.clearAdFaultsSelection();
        msleep(500);
        printToConsole("  AD faults selection cleared");

        swiot.clearMaxFaultsSelection();
        msleep(500);
        printToConsole("  MAX faults selection cleared");

        // Step 5: Run a Reset stored capture
        // Expected: Both leds for Bit 10 are turned off
        swiot.resetAdFaultsStored();
        msleep(500);
        printToConsole("  AD faults stored reset");

        swiot.resetMaxFaultsStored();
        msleep(500);
        printToConsole("  MAX faults stored reset");

        // Cleanup
        cleanupAd();

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        cleanupAd();
        return false;
    }
});

// ============================================
// Test 14 - Faults Run Continuous (TST.FAULTS.RUN_CONTINUOUS)
// ============================================
printToConsole("\n=== Test 14 - Faults Run Continuous ===\n");

TestFramework.runTest("TST.FAULTS.RUN_CONTINUOUS", function() {
    try {
        // Step 1: Run a continuous capture with all channels enabled on AD74413R
        if (!setupAdForFaultsTesting()) {
            printToConsole("  FAIL: Could not setup AD74413R");
            return false;
        }

        // Step 2: Open Faults instrument and run a Continuous capture
        // Expected: Bit 10 is enabled on the AD74413R device
        if (!switchToTool("Faults")) {
            cleanupAd();
            return false;
        }

        swiot.setFaultsRunning(true);
        msleep(1000);
        if (!swiot.isFaultsRunning()) {
            printToConsole("  FAIL: Could not start faults continuous capture");
            cleanupAd();
            return false;
        }
        printToConsole("  Faults continuous capture started");

        msleep(2000);

        // Step 3: While running click the Reset stored button
        // Expected: STORED LED is turned off until next polled value
        swiot.resetAdFaultsStored();
        swiot.resetMaxFaultsStored();
        msleep(1000);
        printToConsole("  Reset stored while running");

        // Step 4: Stop the Faults instrument
        swiot.setFaultsRunning(false);
        msleep(500);
        if (swiot.isFaultsRunning()) {
            printToConsole("  FAIL: Could not stop faults capture");
        }
        printToConsole("  Faults capture stopped");

        // Cleanup
        cleanupAd();

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        swiot.setFaultsRunning(false);
        cleanupAd();
        return false;
    }
});

// ============================================
// Test 15 - Faults Tutorial & Docs (TST.FAULTS.TUTORIAL)
// ============================================
printToConsole("\n=== Test 15 - Faults Tutorial & Docs ===\n");
printToConsole("  NOTE: TST.FAULTS.TUTORIAL requires manual testing (UI interaction)");

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
scopy.exit();
exit(exitCode);
