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
// require a human observer to verify UI changes.
// Source: docs/tests/plugins/ad6676/ad6676_tests.rst
// ============================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("AD6676 Visual Validation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to AD6676 tool
if (!switchToTool("AD6676")) {
    printToConsole("ERROR: Cannot switch to AD6676 tool");
    scopy.exit();
}

// ============================================
// TST.AD6676.DYNAMIC_BANDWIDTH_RANGE
// UID: TST.AD6676.DYNAMIC_BANDWIDTH_RANGE
// Description: Verify that changing ADC frequency updates the valid
//   bandwidth range displayed in the UI (min/max spinbox limits).
// VISUAL: Verify bandwidth range min/max updated in the UI
// ============================================
printToConsole("\n=== Dynamic Bandwidth Range Update (VISUAL) ===\n");

TestFramework.runTest("TST.AD6676.DYNAMIC_BANDWIDTH_RANGE", function() {
    try {
        var origAdcFreq = ad6676.getAdcFrequency();
        printToConsole("  Original adc_frequency: " + origAdcFreq);

        // Set ADC frequency to 3000 MHz to trigger bandwidth range update
        printToConsole("  Setting adc_frequency to 3000...");
        ad6676.setAdcFrequency("3000");
        msleep(500);

        var readBack = ad6676.getAdcFrequency();
        printToConsole("  Read back adc_frequency: " + readBack);

        // Visual check: bandwidth range should update to 15–150 MHz
        TestFramework.supervisedCheck(
            "Verify bandwidth range min/max updated to 15-150 MHz in the UI after setting ADC frequency to 3000 MHz"
        );

        // Restore original ADC frequency
        ad6676.setAdcFrequency(origAdcFreq);
        msleep(500);
        printToConsole("  Restored adc_frequency to: " + origAdcFreq);

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
