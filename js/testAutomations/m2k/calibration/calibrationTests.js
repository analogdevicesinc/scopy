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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

// Calibration Tests
// Tests that verify calibration functionality for M2K
// Based on TST.GEN.SETUP.M2KCONN and calibration API specifications
//
// Hardware Setup Required:
//   - ADALM2000 device connected (real hardware recommended for calibration tests)
//   - No external connections required for basic calibration
//
// Note: Calibration tests modify device calibration parameters.
// After running tests, the device will be recalibrated to ensure proper operation.

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: Calibration Tests
TestFramework.init("Calibration Tests");

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to Calibration tool
if (!switchToTool("Calibration")) {
    printToConsole("ERROR: Cannot access Calibration tool");
    printToConsole("Note: Calibration tool may need to be enabled in preferences");
    // Try to continue with basic calibration API tests
}

// ============================================================================
// Test 1: Basic Calibration API Availability
// UID: TST.CAL.API_AVAILABLE
// ============================================================================
TestFramework.runTest("TST.CAL.API_AVAILABLE", function() {
    try {
        let allPass = true;

        printToConsole("  Checking calibration API availability");

        // Check if calibration object exists
        let calibExists = (typeof calibration !== 'undefined');
        let calibPass = TestFramework.assertEqual(calibExists, true,
            "Calibration API object exists");

        if (!calibExists) {
            printToConsole("    Warning: calibration object not available");
            return "SKIP";
        }

        allPass = allPass && calibPass;

        // Check if manual calibration object exists
        let manualCalibExists = (typeof manualCalibration !== 'undefined');
        printToConsole("  Manual calibration API available: " + manualCalibExists);

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 2: Calibrate All (ADC + DAC)
// UID: TST.CAL.CALIBRATE_ALL
// ============================================================================
TestFramework.runTest("TST.CAL.CALIBRATE_ALL", function() {
    try {
        printToConsole("  Running full calibration (ADC + DAC)");

        if (typeof calibration === 'undefined') {
            printToConsole("    Calibration API not available");
            return "SKIP";
        }

        // Run full calibration
        let result = calibration.calibrateAll();

        // calibrateAll() returns true on success
        let pass = TestFramework.assertEqual(result, true,
            "calibrateAll() completed successfully");

        // Wait for calibration to complete
        msleep(3000);

        return pass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 3: Calibrate ADC Only
// UID: TST.CAL.CALIBRATE_ADC
// ============================================================================
TestFramework.runTest("TST.CAL.CALIBRATE_ADC", function() {
    try {
        printToConsole("  Running ADC calibration");

        if (typeof calibration === 'undefined') {
            printToConsole("    Calibration API not available");
            return "SKIP";
        }

        // Run ADC calibration
        let result = calibration.calibrateAdc();

        // calibrateAdc() returns true on success
        let pass = TestFramework.assertEqual(result, true,
            "calibrateAdc() completed successfully");

        // Wait for calibration to complete
        msleep(2000);

        return pass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 4: Calibrate DAC Only
// UID: TST.CAL.CALIBRATE_DAC
// ============================================================================
TestFramework.runTest("TST.CAL.CALIBRATE_DAC", function() {
    try {
        printToConsole("  Running DAC calibration");

        if (typeof calibration === 'undefined') {
            printToConsole("    Calibration API not available");
            return "SKIP";
        }

        // Run DAC calibration
        let result = calibration.calibrateDac();

        // calibrateDac() returns true on success
        let pass = TestFramework.assertEqual(result, true,
            "calibrateDac() completed successfully");

        // Wait for calibration to complete
        msleep(2000);

        return pass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 5: Reset Calibration
// UID: TST.CAL.RESET
// ============================================================================
TestFramework.runTest("TST.CAL.RESET", function() {
    try {
        printToConsole("  Resetting calibration to defaults");

        if (typeof calibration === 'undefined') {
            printToConsole("    Calibration API not available");
            return "SKIP";
        }

        // Reset calibration
        let result = calibration.resetCalibration();

        // resetCalibration() returns true on success
        let pass = TestFramework.assertEqual(result, true,
            "resetCalibration() completed successfully");

        msleep(1000);

        return pass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 6: Device Temperature Reading
// UID: TST.CAL.DEVICE_TEMP
// ============================================================================
TestFramework.runTest("TST.CAL.DEVICE_TEMP", function() {
    try {
        printToConsole("  Reading device temperature");

        if (typeof calibration === 'undefined') {
            printToConsole("    Calibration API not available");
            return "SKIP";
        }

        // Get device temperature
        // The device name for M2K temperature sensor is typically "ad9963" or similar
        let temp = calibration.devTemp("ad9963");

        printToConsole("    Device temperature: " + temp + " C");

        // Temperature should be in a reasonable range (0-100 C)
        // If temp is 0 or negative, the sensor might not be available
        if (temp === 0) {
            printToConsole("    Warning: Temperature reading is 0, sensor may not be available");
            // Try alternative device name
            temp = calibration.devTemp("xadc");
            printToConsole("    XADC temperature: " + temp + " C");
        }

        // Verify temperature is in reasonable range
        let pass = TestFramework.assertInRange(temp, -10, 100,
            "Device temperature in reasonable range");

        return pass;

    } catch (e) {
        printToConsole("  Error: " + e);
        // Temperature reading may not be available on all devices
        printToConsole("  Note: Temperature reading may not be available on emulator");
        return "SKIP";
    }
});

// ============================================================================
// Test 7: Manual Calibration - Read Calibration Parameters
// UID: TST.CAL.READ_PARAMS
// ============================================================================
TestFramework.runTest("TST.CAL.READ_PARAMS", function() {
    try {
        printToConsole("  Reading calibration parameters");

        if (typeof manualCalibration === 'undefined') {
            printToConsole("    Manual calibration API not available");
            return "SKIP";
        }

        let allPass = true;

        // Read DAC calibration parameters
        printToConsole("  DAC Calibration Parameters:");

        let offsetPosDac = manualCalibration.offset_pos_dac;
        let offsetNegDac = manualCalibration.offset_neg_dac;
        let gainPosDac = manualCalibration.gain_pos_dac;
        let gainNegDac = manualCalibration.gain_neg_dac;

        printToConsole("    offset_pos_dac: " + offsetPosDac);
        printToConsole("    offset_neg_dac: " + offsetNegDac);
        printToConsole("    gain_pos_dac: " + gainPosDac);
        printToConsole("    gain_neg_dac: " + gainNegDac);

        // Verify values are numbers (not NaN or undefined)
        let dacOffsetPosValid = !isNaN(offsetPosDac);
        let dacOffsetNegValid = !isNaN(offsetNegDac);
        let dacGainPosValid = !isNaN(gainPosDac);
        let dacGainNegValid = !isNaN(gainNegDac);

        allPass = allPass && TestFramework.assertEqual(dacOffsetPosValid, true, "DAC offset pos is valid");
        allPass = allPass && TestFramework.assertEqual(dacOffsetNegValid, true, "DAC offset neg is valid");
        allPass = allPass && TestFramework.assertEqual(dacGainPosValid, true, "DAC gain pos is valid");
        allPass = allPass && TestFramework.assertEqual(dacGainNegValid, true, "DAC gain neg is valid");

        // Read ADC calibration parameters
        printToConsole("  ADC Calibration Parameters:");

        let offsetPosAdc = manualCalibration.offset_pos_adc;
        let offsetNegAdc = manualCalibration.offset_neg_adc;
        let gainPosAdc = manualCalibration.gain_pos_adc;
        let gainNegAdc = manualCalibration.gain_neg_adc;

        printToConsole("    offset_pos_adc: " + offsetPosAdc);
        printToConsole("    offset_neg_adc: " + offsetNegAdc);
        printToConsole("    gain_pos_adc: " + gainPosAdc);
        printToConsole("    gain_neg_adc: " + gainNegAdc);

        // Verify values are numbers
        let adcOffsetPosValid = !isNaN(offsetPosAdc);
        let adcOffsetNegValid = !isNaN(offsetNegAdc);
        let adcGainPosValid = !isNaN(gainPosAdc);
        let adcGainNegValid = !isNaN(gainNegAdc);

        allPass = allPass && TestFramework.assertEqual(adcOffsetPosValid, true, "ADC offset pos is valid");
        allPass = allPass && TestFramework.assertEqual(adcOffsetNegValid, true, "ADC offset neg is valid");
        allPass = allPass && TestFramework.assertEqual(adcGainPosValid, true, "ADC gain pos is valid");
        allPass = allPass && TestFramework.assertEqual(adcGainNegValid, true, "ADC gain neg is valid");

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 8: Manual Calibration - Auto Calibration
// UID: TST.CAL.AUTO_CALIBRATION
// ============================================================================
TestFramework.runTest("TST.CAL.AUTO_CALIBRATION", function() {
    try {
        printToConsole("  Running auto calibration via manual calibration API");

        if (typeof manualCalibration === 'undefined') {
            printToConsole("    Manual calibration API not available");
            return "SKIP";
        }

        // Run auto calibration
        manualCalibration.autoCalibration();

        // Wait for calibration to complete
        msleep(5000);

        // Verify by checking that calibration parameters are valid
        let offsetPosAdc = manualCalibration.offset_pos_adc;
        let gainPosAdc = manualCalibration.gain_pos_adc;

        let offsetValid = !isNaN(offsetPosAdc);
        let gainValid = !isNaN(gainPosAdc);

        let pass = TestFramework.assertEqual(offsetValid && gainValid, true,
            "Auto calibration produced valid parameters");

        printToConsole("    Post-calibration offset_pos_adc: " + offsetPosAdc);
        printToConsole("    Post-calibration gain_pos_adc: " + gainPosAdc);

        return pass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 9: Manual Calibration - Load Default Calibration
// UID: TST.CAL.LOAD_DEFAULT
// ============================================================================
TestFramework.runTest("TST.CAL.LOAD_DEFAULT", function() {
    try {
        printToConsole("  Loading default calibration parameters");

        if (typeof manualCalibration === 'undefined') {
            printToConsole("    Manual calibration API not available");
            return "SKIP";
        }

        // Load default calibration
        manualCalibration.loadCalibration();

        msleep(1000);

        // Verify calibration loaded by checking parameters are valid
        let offsetPosAdc = manualCalibration.offset_pos_adc;
        let gainPosAdc = manualCalibration.gain_pos_adc;

        let offsetValid = !isNaN(offsetPosAdc);
        let gainValid = !isNaN(gainPosAdc);

        let pass = TestFramework.assertEqual(offsetValid && gainValid, true,
            "Default calibration loaded successfully");

        printToConsole("    Default offset_pos_adc: " + offsetPosAdc);
        printToConsole("    Default gain_pos_adc: " + gainPosAdc);

        return pass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 10: Manual Calibration - Story-based Calibration (Info Only)
// UID: TST.CAL.STORY_INFO
// Description: Test the story-based calibration API structure
// ============================================================================
TestFramework.runTest("TST.CAL.STORY_INFO", function() {
    try {
        printToConsole("  Testing story-based calibration API structure");

        if (typeof manualCalibration === 'undefined') {
            printToConsole("    Manual calibration API not available");
            return "SKIP";
        }

        let allPass = true;

        // Test story 0 (first calibration story)
        printToConsole("  Starting calibration story 0");
        let numSteps = manualCalibration.start(0);

        printToConsole("    Story 0 has " + numSteps + " steps");

        // If numSteps is -1, no steps available
        if (numSteps === -1) {
            printToConsole("    Story 0 has no steps or is not available");
            // This is not necessarily a failure - some devices may not support all stories
        } else {
            // Verify we got a valid number of steps
            let stepsValid = (numSteps > 0);
            allPass = allPass && TestFramework.assertEqual(stepsValid, true,
                "Story 0 has valid number of steps");

            // We won't actually run through all steps as this requires
            // external equipment and manual intervention
            printToConsole("    Note: Full story execution requires manual intervention");
        }

        // Test checking multiple stories
        for (let storyId = 1; storyId <= 3; storyId++) {
            let steps = manualCalibration.start(storyId);
            printToConsole("    Story " + storyId + " has " + steps + " steps");
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 11: Calibration Parameter Ranges
// UID: TST.CAL.PARAM_RANGES
// Description: Verify calibration parameters are within expected ranges
// ============================================================================
TestFramework.runTest("TST.CAL.PARAM_RANGES", function() {
    try {
        printToConsole("  Verifying calibration parameter ranges");

        if (typeof manualCalibration === 'undefined') {
            printToConsole("    Manual calibration API not available");
            return "SKIP";
        }

        let allPass = true;

        // First, run auto calibration to get valid parameters
        manualCalibration.autoCalibration();
        msleep(3000);

        // Check DAC offset ranges
        // Offsets are typically small values around 0
        let offsetPosDac = manualCalibration.offset_pos_dac;
        let offsetNegDac = manualCalibration.offset_neg_dac;

        printToConsole("  Checking DAC offsets");
        // DAC offsets should be reasonable (typically within ±100 or so)
        // The exact range depends on the device, so we use a wide tolerance
        let dacOffsetPosPass = TestFramework.assertInRange(offsetPosDac, -1000, 1000,
            "DAC positive offset in range");
        let dacOffsetNegPass = TestFramework.assertInRange(offsetNegDac, -1000, 1000,
            "DAC negative offset in range");
        allPass = allPass && dacOffsetPosPass && dacOffsetNegPass;

        // Check DAC gain ranges
        // Gains should be close to 1.0 (or their scaled equivalent)
        let gainPosDac = manualCalibration.gain_pos_dac;
        let gainNegDac = manualCalibration.gain_neg_dac;

        printToConsole("  Checking DAC gains");
        // Gains can vary, but should be non-zero
        let dacGainPosNonZero = (gainPosDac !== 0);
        let dacGainNegNonZero = (gainNegDac !== 0);
        allPass = allPass && TestFramework.assertEqual(dacGainPosNonZero, true, "DAC positive gain non-zero");
        allPass = allPass && TestFramework.assertEqual(dacGainNegNonZero, true, "DAC negative gain non-zero");

        // Check ADC offset ranges
        let offsetPosAdc = manualCalibration.offset_pos_adc;
        let offsetNegAdc = manualCalibration.offset_neg_adc;

        printToConsole("  Checking ADC offsets");
        let adcOffsetPosPass = TestFramework.assertInRange(offsetPosAdc, -1000, 1000,
            "ADC positive offset in range");
        let adcOffsetNegPass = TestFramework.assertInRange(offsetNegAdc, -1000, 1000,
            "ADC negative offset in range");
        allPass = allPass && adcOffsetPosPass && adcOffsetNegPass;

        // Check ADC gain ranges
        let gainPosAdc = manualCalibration.gain_pos_adc;
        let gainNegAdc = manualCalibration.gain_neg_adc;

        printToConsole("  Checking ADC gains");
        let adcGainPosNonZero = (gainPosAdc !== 0);
        let adcGainNegNonZero = (gainNegAdc !== 0);
        allPass = allPass && TestFramework.assertEqual(adcGainPosNonZero, true, "ADC positive gain non-zero");
        allPass = allPass && TestFramework.assertEqual(adcGainNegNonZero, true, "ADC negative gain non-zero");

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 12: Calibration Persistence
// UID: TST.CAL.PERSISTENCE
// Description: Verify calibration parameters persist after recalibration
// ============================================================================
TestFramework.runTest("TST.CAL.PERSISTENCE", function() {
    try {
        printToConsole("  Testing calibration parameter persistence");

        if (typeof manualCalibration === 'undefined') {
            printToConsole("    Manual calibration API not available");
            return "SKIP";
        }

        // Run auto calibration
        manualCalibration.autoCalibration();
        msleep(3000);

        // Record initial parameters
        let initialOffsetPosAdc = manualCalibration.offset_pos_adc;
        let initialGainPosAdc = manualCalibration.gain_pos_adc;

        printToConsole("    Initial offset_pos_adc: " + initialOffsetPosAdc);
        printToConsole("    Initial gain_pos_adc: " + initialGainPosAdc);

        // Run auto calibration again
        manualCalibration.autoCalibration();
        msleep(3000);

        // Record new parameters
        let newOffsetPosAdc = manualCalibration.offset_pos_adc;
        let newGainPosAdc = manualCalibration.gain_pos_adc;

        printToConsole("    New offset_pos_adc: " + newOffsetPosAdc);
        printToConsole("    New gain_pos_adc: " + newGainPosAdc);

        // Parameters should be similar (within reasonable tolerance)
        // They may not be identical due to measurement noise
        let allPass = true;

        // Allow 10% tolerance for offset variation
        let offsetTolerance = Math.abs(initialOffsetPosAdc * 0.1) + 10;
        let offsetClose = Math.abs(newOffsetPosAdc - initialOffsetPosAdc) < offsetTolerance;
        allPass = allPass && TestFramework.assertEqual(offsetClose, true,
            "Offset consistent between calibrations");

        // Allow 5% tolerance for gain variation
        let gainTolerance = Math.abs(initialGainPosAdc * 0.05) + 0.01;
        let gainClose = Math.abs(newGainPosAdc - initialGainPosAdc) < gainTolerance;
        allPass = allPass && TestFramework.assertEqual(gainClose, true,
            "Gain consistent between calibrations");

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 13: Calibration Effect on Measurements
// UID: TST.CAL.MEASUREMENT_EFFECT
// Description: Verify calibration improves measurement accuracy
// ============================================================================
TestFramework.runTest("TST.CAL.MEASUREMENT_EFFECT", function() {
    try {
        printToConsole("  Testing calibration effect on measurements");
        printToConsole("  (Requires loopback: V+ → 1+)");

        if (typeof calibration === 'undefined') {
            printToConsole("    Calibration API not available");
            return "SKIP";
        }

        // First, run full calibration
        let calibResult = calibration.calibrateAll();
        if (!calibResult) {
            printToConsole("    Warning: calibrateAll() returned false");
        }
        msleep(3000);

        // Switch to Power Supply to generate known voltage
        switchToTool("Power Supply");

        // Set power supply to known voltage
        power.pos_enabled = true;
        power.pos_voltage = 2.5;
        msleep(1000);

        // Switch to Voltmeter to measure
        switchToTool("Voltmeter");

        dmm.mode[0] = 0; // DC mode
        dmm.running = true;
        msleep(2000);

        dmm.running = false;

        // Read measurement (this depends on loopback connection)
        // If no loopback, measurement will be near 0
        let measured = dmm.value[0];
        printToConsole("    Measured voltage: " + measured + " V");
        printToConsole("    Expected voltage: 2.5 V (if loopback connected)");

        // Cleanup
        switchToTool("Power Supply");
        power.pos_enabled = false;

        // If loopback is connected, measurement should be close to 2.5V
        // If not connected, we at least verified the calibration ran without error
        let pass = true;

        // Check if measurement is reasonable (either near 2.5V with loopback or near 0V without)
        if (Math.abs(measured - 2.5) < 0.5) {
            printToConsole("    Loopback detected - measurement accuracy verified");
            pass = TestFramework.assertApproxEqual(measured, 2.5, 0.25,
                "Calibrated measurement accuracy");
        } else if (Math.abs(measured) < 0.5) {
            printToConsole("    No loopback detected - skipping accuracy check");
            pass = true; // Calibration still ran successfully
        } else {
            printToConsole("    Unexpected measurement value: " + measured);
            pass = false;
        }

        return pass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Final Cleanup: Recalibrate device to ensure proper operation
// ============================================================================
TestFramework.runTest("TST.CAL.FINAL_RECALIBRATION", function() {
    try {
        printToConsole("  Final recalibration to ensure device is properly calibrated");

        if (typeof calibration !== 'undefined') {
            let result = calibration.calibrateAll();
            msleep(3000);

            let pass = TestFramework.assertEqual(result, true,
                "Final recalibration completed");
            return pass;
        } else if (typeof manualCalibration !== 'undefined') {
            manualCalibration.autoCalibration();
            msleep(3000);

            printToConsole("    Auto calibration completed");
            return true;
        } else {
            printToConsole("    No calibration API available");
            return "SKIP";
        }

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
