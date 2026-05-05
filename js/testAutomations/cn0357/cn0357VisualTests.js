/*
 * Copyright (c) 2026 Analog Devices Inc.
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

// CN0357 Visual Test Automation
// Automates tests from: docs/tests/plugins/cn0357/cn0357_tests.rst
//
// Tests here require human visual verification (Category B — supervised):
//   TST.CN0357.ADC_UPDATE_RATE        (#2, step 3 — update rate observation)
//   TST.CN0357.FEEDBACK_SETTINGS     (#3, steps 4, 6 — UI element visibility)
//   TST.CN0357.SYSTEM_MEASUREMENTS   (#4, steps 2-3 — live update observation)
//   TST.CN0357.REFRESH_FUNCTION      (#6, toolbar button click)
//
// Each test sets up the state programmatically, then uses supervisedCheck()
// for the human to confirm the visual result.
// ============================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("CN0357 Visual Tests");

// Connect to device — ip:127.0.0.0 intentionally fails, triggering emulator fallback
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to CN0357 tool
if (!switchToTool("CN0357")) {
    printToConsole("ERROR: Cannot switch to CN0357 tool");
    scopy.exit();
}

// ============================================
// Test 2 Step 3: ADC Update Rate — Visual Verification
// UID: TST.CN0357.ADC_UPDATE_RATE
// Description: Verify that the Conversion (mV) reading updates at a rate
//              consistent with the selected sampling frequency.
// ============================================
printToConsole("\n=== Test 2 Step 3: ADC Update Rate — Visual ===\n");

TestFramework.runTest("TST.CN0357.ADC_UPDATE_RATE", function() {
    var origFreq = null;

    try {
        origFreq = cn0357.getSamplingFrequency();

        // Set to a slow rate so the user can observe the update cadence
        cn0357.setSamplingFrequency("16");
        msleep(500);

        printToConsole("  Sampling frequency set to 16 Hz");
        printToConsole("  Observe the Conversion (mV) display in the System section");

        var result = TestFramework.supervisedCheck(
            "Is the Conversion (mV) value updating at approximately 16 Hz (slow, steady updates)?"
        );

        // Now set to a fast rate for comparison
        cn0357.setSamplingFrequency("120");
        msleep(500);

        printToConsole("  Sampling frequency changed to 120 Hz");

        var result2 = TestFramework.supervisedCheck(
            "Is the Conversion (mV) value now updating noticeably faster (120 Hz)?"
        );

        // Restore
        cn0357.setSamplingFrequency(origFreq);
        msleep(500);

        return result && result2;
    } catch (e) {
        printToConsole("  Error: " + e);
        try {
            if (origFreq) { cn0357.setSamplingFrequency(origFreq); msleep(500); }
        } catch (e2) {}
        return false;
    }
});

// ============================================
// Test 3 Steps 4,6: Feedback Settings — Visibility
// UID: TST.CN0357.FEEDBACK_SETTINGS
// Description: Verify RDAC/Fixed Resistor UI elements show/hide on type switch.
// ============================================
printToConsole("\n=== Test 3 Steps 4,6: Feedback Settings — Visibility ===\n");

TestFramework.runTest("TST.CN0357.FEEDBACK_SETTINGS", function() {
    var origFeedbackType = null;

    try {
        origFeedbackType = cn0357.getFeedbackType();

        // Ensure we start in Rheostat mode
        cn0357.setFeedbackType("Rheostat");
        msleep(500);

        var result1 = TestFramework.supervisedCheck(
            "In Rheostat mode: Are the RDAC Value spinbox and Program Rheostat button visible?"
        );

        // Step 4: Switch to Fixed Resistor
        cn0357.setFeedbackType("Fixed Resistor");
        msleep(500);

        var result2 = TestFramework.supervisedCheck(
            "In Fixed Resistor mode: Are the RDAC Value spinbox and Program Rheostat button HIDDEN, and the Fixed Resistor spinbox VISIBLE?"
        );

        // Step 6: Switch back to Rheostat
        cn0357.setFeedbackType("Rheostat");
        msleep(500);

        var result3 = TestFramework.supervisedCheck(
            "Back in Rheostat mode: Is the Fixed Resistor spinbox HIDDEN, and are the RDAC Value spinbox and Program Rheostat button VISIBLE again?"
        );

        // Restore
        cn0357.setFeedbackType(origFeedbackType);
        msleep(500);

        return result1 && result2 && result3;
    } catch (e) {
        printToConsole("  Error: " + e);
        try {
            if (origFeedbackType) { cn0357.setFeedbackType(origFeedbackType); msleep(500); }
        } catch (e2) {}
        return false;
    }
});

// ============================================
// Test 4 Steps 2-3: System Measurements — Live Update
// UID: TST.CN0357.SYSTEM_MEASUREMENTS
// Description: Verify Concentration (ppm) and Conversion (mV) update periodically.
// ============================================
printToConsole("\n=== Test 4 Steps 2-3: System Measurements — Live Update ===\n");

TestFramework.runTest("TST.CN0357.SYSTEM_MEASUREMENTS", function() {
    try {
        printToConsole("  Observe the System section measurement displays");

        var result1 = TestFramework.supervisedCheck(
            "Is the Concentration (ppm) value updating periodically with a numeric value?"
        );

        var result2 = TestFramework.supervisedCheck(
            "Is the Conversion (mV) value updating periodically with a numeric millivolt reading?"
        );

        return result1 && result2;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 6: Refresh Button — Toolbar
// UID: TST.CN0357.REFRESH_FUNCTION
// Description: Verify toolbar refresh button re-reads all values.
// ============================================
printToConsole("\n=== Test 6: Refresh Button — Toolbar ===\n");

TestFramework.runTest("TST.CN0357.REFRESH_FUNCTION", function() {
    try {
        printToConsole("  Please click the refresh button in the CN0357 plugin toolbar");

        var result = TestFramework.supervisedCheck(
            "After clicking the refresh button, did all measurement and data values update/re-read from the device?"
        );

        return result;
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
