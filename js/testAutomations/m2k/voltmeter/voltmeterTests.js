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

// Voltmeter/DMM Automated Tests (Single-Tool Only)
// Tests that only use the Voltmeter without requiring other tools
// Based on TST.DMM.* test specifications

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: Voltmeter
TestFramework.init("Voltmeter Tests");

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to Voltmeter tool
if (!switchToTool("Voltmeter")) {
    printToConsole("ERROR: Cannot access Voltmeter");
    exit(1);
}

// Test 1: Mode Switching
TestFramework.runTest("TST.DMM.MODE_SWITCHING", function() {
    try {
        dmm.running = true;

        let allPass = true;

        // Test DC mode
        dmm.mode_ac_low_ch1 = false;
        dmm.mode_ac_high_ch1 = false;
        msleep(100);
        let dcMode = !dmm.mode_ac_low_ch1 && !dmm.mode_ac_high_ch1;
        let pass1 = TestFramework.assertEqual(dcMode, true, "DC mode set");
        allPass = allPass && pass1;

        // Test AC low mode
        dmm.mode_ac_low_ch1 = true;
        dmm.mode_ac_high_ch1 = false;
        msleep(100);
        let acLowMode = dmm.mode_ac_low_ch1 && !dmm.mode_ac_high_ch1;
        let pass2 = TestFramework.assertEqual(acLowMode, true, "AC low mode set");
        allPass = allPass && pass2;

        // Test AC high mode
        dmm.mode_ac_low_ch1 = false;
        dmm.mode_ac_high_ch1 = true;
        msleep(100);
        let acHighMode = !dmm.mode_ac_low_ch1 && dmm.mode_ac_high_ch1;
        let pass3 = TestFramework.assertEqual(acHighMode, true, "AC high mode set");
        allPass = allPass && pass3;

        dmm.running = false;
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 2: Start/Stop Control
TestFramework.runTest("TST.DMM.START_STOP", function() {
    try {
        // Test stopping
        dmm.running = false;
        msleep(100);
        let running = dmm.running;
        let pass1 = TestFramework.assertEqual(running, false, "DMM stopped");

        // Test starting
        dmm.running = true;
        msleep(100);
        running = dmm.running;
        let pass2 = TestFramework.assertEqual(running, true, "DMM started");

        dmm.running = false;
        return pass1 && pass2;

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