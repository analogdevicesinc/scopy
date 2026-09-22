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

// CN0511 Manual Test Automation
// Automates tests from: docs/tests/plugins/cn0511/cn0511_tests.rst
// Tests: TST.CN0511.PLUGIN_LOADS,
//        TST.CN0511.SINGLE_TONE_SECTION_VISIBLE,
//        TST.CN0511.SET_FREQUENCY,
//        TST.CN0511.SET_AMPLITUDE,
//        TST.CN0511.DAC_AMP_ENABLE,
//        TST.CN0511.REFRESH_READS_FREQUENCY
//
// =============================================================================
// MISSING API REPORT — Category C (Not Automatable)
// =============================================================================
// TST.CN0511.VERIFY_FREQUENCY_WITH_AD936X (P2)
//   Reason: This test requires two physical boards simultaneously (CN0511 and
//   an AD936x receiver such as PlutoSDR or AD-FMCOMMS) with an SMA cable
//   connecting the CN0511 RF output to the AD936x RX input. The test framework
//   supports one device at a time and has no API to programmatically confirm RF
//   signal presence on the receiver. Step 3 also requires visual observation of
//   the AD936x signal display. Hardware setup and visual verification cannot be
//   automated.
// =============================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("CN0511 Documentation Tests");

// Connect to device (falls back to emulator at ip:127.0.0.1 automatically)
if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to CN0511 tool
if (!switchToTool("CN0511")) {
    printToConsole("ERROR: Cannot switch to CN0511 tool");
    scopy.exit();
}

// ============================================
// Test 1: Plugin Loads
// UID: TST.CN0511.PLUGIN_LOADS
// Description: Verify that the CN0511 plugin loads in Scopy and the tool
//   is accessible.
// ============================================
printToConsole("\n=== Test 1: Plugin Loads ===\n");

TestFramework.runTest("TST.CN0511.PLUGIN_LOADS", function() {
    try {
        // Step 1: Verify CN0511 tool appears in the tool list
        var tools = cn0511.getTools();
        printToConsole("  Available tools: " + tools);
        if (!tools || tools.indexOf("CN0511") === -1) {
            printToConsole("  FAIL: CN0511 not found in tool list");
            return false;
        }
        printToConsole("  CN0511 found in tool list");

        // Step 2: Verify tool is loaded by reading a known attribute
        var freq = cn0511.getFrequency();
        if (freq === null || freq === undefined || freq === "") {
            printToConsole("  FAIL: CN0511 tool did not return frequency (tool may not be loaded)");
            return false;
        }
        printToConsole("  CN0511 tool loaded, current frequency: " + freq + " MHz");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2: Single Tone Mode Section Visible
// UID: TST.CN0511.SINGLE_TONE_SECTION_VISIBLE
// Description: Verify that the Single Tone Mode section displays the
//   Frequency spinbox, Amplitude spinbox, and Enter button.
// ============================================
printToConsole("\n=== Test 2: Single Tone Mode Section Visible ===\n");

TestFramework.runTest("TST.CN0511.SINGLE_TONE_SECTION_VISIBLE", function() {
    try {
        // Verify Frequency spinbox is accessible with default value 4500 MHz
        var freq = cn0511.getFrequency();
        printToConsole("  Frequency spinbox value: " + freq);
        if (!TestFramework.assertEqual(freq, "4500", "Frequency default value is 4500 MHz")) {
            return false;
        }

        // Verify Amplitude spinbox is accessible with default value 0 dBm
        var amp = cn0511.getAmplitude();
        printToConsole("  Amplitude spinbox value: " + amp);
        if (!TestFramework.assertEqual(amp, "0", "Amplitude default value is 0 dBm")) {
            return false;
        }

        printToConsole("  Single Tone Mode section accessible with correct default values");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3: Set Output Frequency
// UID: TST.CN0511.SET_FREQUENCY
// Description: Verify that setting a new frequency value and clicking Enter
//   writes the frequency to the AD9166 DAC without errors.
// ============================================
printToConsole("\n=== Test 3: Set Output Frequency ===\n");

TestFramework.runTest("TST.CN0511.SET_FREQUENCY", function() {
    var origFreq = cn0511.getFrequency();
    printToConsole("  Original frequency: " + origFreq + " MHz");

    try {
        // Step 1: Set Frequency to 2000 MHz
        cn0511.setFrequency("2000");
        msleep(500);
        var readBack = cn0511.getFrequency();
        printToConsole("  Set frequency to 2000 MHz, spinbox reads: " + readBack);
        if (!TestFramework.assertEqual(readBack, "2000", "Frequency spinbox accepted value 2000")) {
            cn0511.setFrequency(origFreq);
            msleep(500);
            return false;
        }

        // Step 2: Apply calibration (click Enter)
        cn0511.applyCalibration();
        msleep(1000);
        printToConsole("  Calibration applied (Enter clicked), no exception thrown");

        // Restore original frequency
        cn0511.setFrequency(origFreq);
        msleep(500);
        cn0511.applyCalibration();
        msleep(1000);
        printToConsole("  Original frequency restored: " + origFreq + " MHz");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try {
            cn0511.setFrequency(origFreq);
            msleep(500);
            cn0511.applyCalibration();
        } catch (e2) {}
        return false;
    }
});

// ============================================
// Test 4: Set Output Amplitude
// UID: TST.CN0511.SET_AMPLITUDE
// Description: Verify that setting a new amplitude value and clicking Enter
//   writes the amplitude to the AD9166 DAC without errors.
// ============================================
printToConsole("\n=== Test 4: Set Output Amplitude ===\n");

TestFramework.runTest("TST.CN0511.SET_AMPLITUDE", function() {
    var origAmp = cn0511.getAmplitude();
    printToConsole("  Original amplitude: " + origAmp + " dBm");

    try {
        // Step 1: Set Amplitude to -20 dBm
        cn0511.setAmplitude("-20");
        msleep(500);
        var readBack = cn0511.getAmplitude();
        printToConsole("  Set amplitude to -20 dBm, spinbox reads: " + readBack);
        if (!TestFramework.assertEqual(readBack, "-20", "Amplitude spinbox accepted value -20")) {
            cn0511.setAmplitude(origAmp);
            msleep(500);
            return false;
        }

        // Step 2: Apply calibration (click Enter)
        cn0511.applyCalibration();
        msleep(1000);
        printToConsole("  Calibration applied (Enter clicked), no exception thrown");

        // Restore original amplitude
        cn0511.setAmplitude(origAmp);
        msleep(500);
        cn0511.applyCalibration();
        msleep(1000);
        printToConsole("  Original amplitude restored: " + origAmp + " dBm");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try {
            cn0511.setAmplitude(origAmp);
            msleep(500);
            cn0511.applyCalibration();
        } catch (e2) {}
        return false;
    }
});

// ============================================
// Test 5: DAC Amplifier Enable
// UID: TST.CN0511.DAC_AMP_ENABLE
// Description: Verify that the DAC Amplifier section is visible and that
//   toggling the Enable checkbox writes the en attribute to the ad9166-amp
//   device.
// ============================================
printToConsole("\n=== Test 5: DAC Amplifier Enable ===\n");

TestFramework.runTest("TST.CN0511.DAC_AMP_ENABLE", function() {
    // Step 1: Verify DAC Amplifier section is accessible
    var origVal = cn0511.isDacAmpEnabled();
    printToConsole("  DAC Amplifier Enable current value: " + origVal);
    if (origVal === null || origVal === undefined || origVal === "") {
        printToConsole("  FAIL: DAC Amplifier Enable attribute not accessible");
        return false;
    }
    printToConsole("  DAC Amplifier section accessible");

    try {
        // Step 2: Toggle Enable checkbox to enabled
        cn0511.setDacAmpEnabled("1");
        msleep(500);
        cn0511.refresh();
        msleep(500);
        var readBack = cn0511.isDacAmpEnabled();
        printToConsole("  Set DAC amp enable to 1, read back: " + readBack);
        if (!TestFramework.assertEqual(readBack, "1", "DAC amp enable set to 1")) {
            cn0511.setDacAmpEnabled(origVal);
            msleep(500);
            return false;
        }

        // Step 3: Toggle Enable checkbox to disabled
        cn0511.setDacAmpEnabled("0");
        msleep(500);
        cn0511.refresh();
        msleep(500);
        readBack = cn0511.isDacAmpEnabled();
        printToConsole("  Set DAC amp enable to 0, read back: " + readBack);
        if (!TestFramework.assertEqual(readBack, "0", "DAC amp enable set to 0")) {
            cn0511.setDacAmpEnabled(origVal);
            msleep(500);
            return false;
        }

        // Restore original value
        cn0511.setDacAmpEnabled(origVal);
        msleep(500);
        printToConsole("  DAC amp enable restored to: " + origVal);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try {
            cn0511.setDacAmpEnabled(origVal);
            msleep(500);
        } catch (e2) {}
        return false;
    }
});

// ============================================
// Test 6: Refresh Reads NCO Frequency
// UID: TST.CN0511.REFRESH_READS_FREQUENCY
// Description: Verify that clicking the Refresh button reads the current NCO
//   frequency from the device and updates the Frequency spinbox.
// ============================================
printToConsole("\n=== Test 6: Refresh Reads NCO Frequency ===\n");

TestFramework.runTest("TST.CN0511.REFRESH_READS_FREQUENCY", function() {
    var origFreq = cn0511.getFrequency();
    printToConsole("  Original frequency: " + origFreq + " MHz");

    try {
        // Step 1: Set Frequency to 3000 MHz and apply calibration
        cn0511.setFrequency("3000");
        msleep(500);
        cn0511.applyCalibration();
        msleep(2000); // Wait for async calibration to complete and write nco_frequency

        printToConsole("  Applied calibration at 3000 MHz");

        // Step 2: Stage a different value in the spinbox without applying
        cn0511.setFrequency("5000");
        msleep(500);
        var stagedVal = cn0511.getFrequency();
        printToConsole("  Staged frequency in spinbox: " + stagedVal + " MHz");
        if (!TestFramework.assertEqual(stagedVal, "5000", "Spinbox shows staged value 5000")) {
            cn0511.setFrequency(origFreq);
            msleep(500);
            return false;
        }

        // Step 3: Click Refresh — spinbox should update to device value (3000 MHz)
        cn0511.refresh();
        msleep(1000); // Wait for async NCO frequency readback

        var afterRefresh = cn0511.getFrequency();
        printToConsole("  After refresh, frequency spinbox shows: " + afterRefresh + " MHz");
        if (!TestFramework.assertEqual(afterRefresh, "3000",
                "After refresh, spinbox updated to device value (3000 MHz)")) {
            cn0511.setFrequency(origFreq);
            msleep(500);
            cn0511.applyCalibration();
            return false;
        }

        // Restore original frequency
        cn0511.setFrequency(origFreq);
        msleep(500);
        cn0511.applyCalibration();
        msleep(2000);
        printToConsole("  Original frequency restored: " + origFreq + " MHz");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try {
            cn0511.setFrequency(origFreq);
            msleep(500);
            cn0511.applyCalibration();
        } catch (e2) {}
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
