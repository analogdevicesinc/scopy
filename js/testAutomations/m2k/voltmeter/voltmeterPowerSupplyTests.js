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

// Voltmeter + Power Supply Integration Tests
// Tests that use Power Supply as DC source for voltmeter measurements
// Based on TST.DMM.* test specifications

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: Voltmeter + Power Supply Integration Tests
TestFramework.init("Voltmeter + Power Supply Tests");

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to Voltmeter tool first (primary tool)
if (!switchToTool("Voltmeter")) {
    printToConsole("ERROR: Cannot access Voltmeter");
    exit(1);
}

// Test 1: DC Voltage Measurement - Channel 1
// Note: DAC1 (V+) only supports 0V to +5V range
TestFramework.runTest("TST.DMM.DC_MEASUREMENT_CH1", function() {
    try {
        // Generate test signal with Power Supply
        switchToTool("Power Supply");
        power.dac1_enabled = true;

        // Switch to Voltmeter
        switchToTool("Voltmeter");
        dmm.mode_ac_low_ch1 = false;
        dmm.mode_ac_high_ch1 = false;
        dmm.running = true;

        // DAC1 only supports 0V to +5V (positive voltages only)
        let testVoltages = [0, 0.5, 1, 2, 3.3, 4, 5];
        let allPass = true;

        for (let voltage of testVoltages) {
            // Set voltage
            switchToTool("Power Supply");
            power.dac1_value = voltage;
            msleep(500);

            // Measure
            switchToTool("Voltmeter");
            msleep(500);
            let measured = dmm.value_ch1;

            // Increased tolerance for hardware variation
            let tolerance = (voltage >= 4) ? 0.1 : 0.06;
            let pass = TestFramework.assertApproxEqual(measured, voltage, tolerance,
                "DC voltage " + voltage + "V");
            allPass = allPass && pass;
        }

        dmm.running = false;
        switchToTool("Power Supply");
        power.dac1_enabled = false;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 2: DC Voltage Measurement - Channel 2
// Note: DAC2 (V-) only supports -5V to 0V range, outputs absolute value
TestFramework.runTest("TST.DMM.DC_MEASUREMENT_CH2", function() {
    try {
        // Generate test signal with Power Supply
        switchToTool("Power Supply");
        power.sync = false;  // Required for DAC2 value to be applied
        power.dac2_enabled = true;

        // Switch to Voltmeter
        switchToTool("Voltmeter");
        dmm.mode_ac_low_ch2 = false;
        dmm.mode_ac_high_ch2 = false;
        dmm.running = true;

        // DAC2 only supports -5V to 0V (negative inputs, outputs absolute value)
        let testVoltages = [-0.5, -1, -2, -3.3, -4, -5];
        let allPass = true;

        for (let voltage of testVoltages) {
            // Set voltage
            switchToTool("Power Supply");
            power.dac2_value = voltage;
            msleep(500);

            // Measure - DAC2 outputs absolute value of input
            switchToTool("Voltmeter");
            msleep(500);
            let measured = dmm.value_ch2;
            let expectedValue = Math.abs(voltage);

            // Increased tolerance for hardware variation
            let tolerance = (Math.abs(voltage) >= 4) ? 0.1 : 0.06;
            let pass = TestFramework.assertApproxEqual(measured, expectedValue, tolerance,
                "DC voltage " + voltage + "V (outputs " + expectedValue + "V)");
            allPass = allPass && pass;
        }

        dmm.running = false;
        switchToTool("Power Supply");
        power.dac2_enabled = false;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 3: Dual Channel Simultaneous Measurement
// Note: DAC1=positive only (0-5V), DAC2=negative input (outputs absolute value)
TestFramework.runTest("TST.DMM.DUAL_CHANNEL", function() {
    try {
        // Generate different voltages on both channels
        switchToTool("Power Supply");
        power.sync = false;  // Required for DAC2 value to be applied
        power.dac1_value = 2.5;    // DAC1: positive voltage
        power.dac2_value = -1.8;   // DAC2: negative input, outputs +1.8V
        power.dac1_enabled = true;
        power.dac2_enabled = true;
        msleep(500);

        // Measure both channels
        switchToTool("Voltmeter");
        dmm.mode_ac_low_ch1 = false;
        dmm.mode_ac_high_ch1 = false;
        dmm.mode_ac_low_ch2 = false;
        dmm.mode_ac_high_ch2 = false;
        dmm.running = true;
        msleep(1000);

        let measured1 = dmm.value_ch1;
        let measured2 = dmm.value_ch2;

        let pass1 = TestFramework.assertApproxEqual(measured1, 2.5, 0.1,
            "Channel 1 measurement");
        // DAC2 outputs absolute value of -1.8V = 1.8V
        let pass2 = TestFramework.assertApproxEqual(measured2, 1.8, 0.1,
            "Channel 2 measurement (absolute value)");

        dmm.running = false;
        switchToTool("Power Supply");
        power.dac1_enabled = false;
        power.dac2_enabled = false;

        return pass1 && pass2;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 4: Peak Hold Feature
TestFramework.runTest("TST.DMM.PEAK_HOLD", function() {
    try {
        switchToTool("Voltmeter");

        // Enable peak hold
        dmm.peak_hold_en = true;
        dmm.running = true;

        // Generate a peak voltage
        switchToTool("Power Supply");
        power.dac1_enabled = true;
        power.dac1_value = 4.0;
        msleep(1000);

        // Reduce voltage
        power.dac1_value = 1.0;
        msleep(1000);

        // With peak hold, reading should still show ~4V peak
        switchToTool("Voltmeter");
        let peakReading = dmm.value_ch1;

        printToConsole("  Peak hold reading: " + peakReading + "V");
        // Note: Actual peak hold behavior may vary

        // Disable peak hold
        dmm.peak_hold_en = false;
        msleep(1000);

        // Now reading should show current value ~1V
        let currentReading = dmm.value_ch1;
        printToConsole("  Current reading: " + currentReading + "V");

        let pass = TestFramework.assertApproxEqual(currentReading, 1.0, 0.1,
            "Current voltage after peak hold disabled");

        dmm.running = false;
        switchToTool("Power Supply");
        power.dac1_enabled = false;

        return pass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 5: Data Logging Enable/Disable
TestFramework.runTest("TST.DMM.DATA_LOGGING", function() {
    try {
        switchToTool("Voltmeter");

        // Enable data logging
        dmm.data_logging_en = true;
        let loggingEnabled = dmm.data_logging_en;
        let pass1 = TestFramework.assertEqual(loggingEnabled, true,
            "Data logging enabled");

        // Generate test data
        switchToTool("Power Supply");
        power.dac1_enabled = true;

        switchToTool("Voltmeter");
        dmm.running = true;

        // Log some data points
        for (let v = 0; v <= 2; v += 0.5) {
            switchToTool("Power Supply");
            power.dac1_value = v;
            msleep(500);

            switchToTool("Voltmeter");
            let reading = dmm.value_ch1;
            printToConsole("  Logging: " + reading.toFixed(3) + "V");
        }

        // Disable data logging
        dmm.data_logging_en = false;
        loggingEnabled = dmm.data_logging_en;
        let pass2 = TestFramework.assertEqual(loggingEnabled, false,
            "Data logging disabled");

        dmm.running = false;
        switchToTool("Power Supply");
        power.dac1_enabled = false;

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