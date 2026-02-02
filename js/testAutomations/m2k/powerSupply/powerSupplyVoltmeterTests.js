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

// Power Supply + Voltmeter Integration Tests
// Tests that use Voltmeter to verify Power Supply output
// Based on TST.PS.* test specifications

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: Power Supply + Voltmeter Integration Tests
TestFramework.init("Power Supply + Voltmeter Tests");

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to Power Supply tool
if (!switchToTool("Power Supply")) {
    printToConsole("ERROR: Cannot access Power Supply");
    exit(1);
}

// Test 1: Positive Voltage Range - DAC1
TestFramework.runTest("TST.PS.POSITIVE_DAC1", function() {
    try {
        power.dac1_enabled = true;
        power.running = true;

        let testVoltages = [0.1, 0.5, 1.0, 2.0, 3.3, 4.5, 5.0];
        let allPass = true;

        for (let voltage of testVoltages) {
            printToConsole("  Setting DAC1 to " + voltage + "V");
            power.dac1_value = voltage;
            msleep(500);

            // Verify with voltmeter
            switchToTool("Voltmeter");
            dmm.running = true;
            msleep(1000);
            let measured = dmm.value_ch1;
            dmm.running = false;

            let pass = TestFramework.assertApproxEqual(measured, voltage, 0.05,
                "DAC1 output verification");
            allPass = allPass && pass;

            switchToTool("Power Supply");
        }

        power.dac1_enabled = false;
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 2: Negative Voltage Range - DAC1
TestFramework.runTest("TST.PS.NEGATIVE_DAC1", function() {
    try {
        power.dac1_enabled = true;
        power.running = true;

        let testVoltages = [-0.1, -0.5, -1.0, -2.0, -3.3, -4.5, -5.0];
        let allPass = true;

        for (let voltage of testVoltages) {
            printToConsole("  Setting DAC1 to " + voltage + "V");
            power.dac1_value = voltage;
            msleep(500);

            // Verify with voltmeter
            switchToTool("Voltmeter");
            dmm.running = true;
            msleep(1000);
            let measured = dmm.value_ch1;
            dmm.running = false;

            let pass = TestFramework.assertApproxEqual(measured, voltage, 0.05,
                "DAC1 negative voltage");
            allPass = allPass && pass;

            switchToTool("Power Supply");
        }

        power.dac1_enabled = false;
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 3: Positive Voltage Range - DAC2
TestFramework.runTest("TST.PS.POSITIVE_DAC2", function() {
    try {
        power.dac2_enabled = true;
        power.running = true;

        let testVoltages = [0.1, 0.5, 1.0, 2.0, 3.3, 4.5, 5.0];
        let allPass = true;

        for (let voltage of testVoltages) {
            printToConsole("  Setting DAC2 to " + voltage + "V");
            power.dac2_value = voltage;
            msleep(500);

            // Verify with voltmeter
            switchToTool("Voltmeter");
            dmm.running = true;
            msleep(1000);
            let measured = dmm.value_ch2;
            dmm.running = false;

            let pass = TestFramework.assertApproxEqual(measured, voltage, 0.05,
                "DAC2 output verification");
            allPass = allPass && pass;

            switchToTool("Power Supply");
        }

        power.dac2_enabled = false;
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 4: Negative Voltage Range - DAC2
TestFramework.runTest("TST.PS.NEGATIVE_DAC2", function() {
    try {
        power.dac2_enabled = true;
        power.running = true;

        let testVoltages = [-0.1, -0.5, -1.0, -2.0, -3.3, -4.5, -5.0];
        let allPass = true;

        for (let voltage of testVoltages) {
            printToConsole("  Setting DAC2 to " + voltage + "V");
            power.dac2_value = voltage;
            msleep(500);

            // Verify with voltmeter
            switchToTool("Voltmeter");
            dmm.running = true;
            msleep(1000);
            let measured = dmm.value_ch2;
            dmm.running = false;

            let pass = TestFramework.assertApproxEqual(measured, voltage, 0.05,
                "DAC2 negative voltage");
            allPass = allPass && pass;

            switchToTool("Power Supply");
        }

        power.dac2_enabled = false;
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 5: Fine Voltage Tuning - DAC1
TestFramework.runTest("TST.PS.FINE_TUNING_DAC1", function() {
    try {
        power.dac1_enabled = true;
        power.running = true;

        // Test fine voltage steps around 3.3V
        let baseVoltage = 3.3;
        let steps = [-0.1, -0.05, -0.01, 0, 0.01, 0.05, 0.1];
        let allPass = true;

        for (let step of steps) {
            let voltage = baseVoltage + step;
            printToConsole("  Fine tuning to " + voltage.toFixed(3) + "V");
            power.dac1_value = voltage;
            msleep(500);

            // Verify with voltmeter
            switchToTool("Voltmeter");
            dmm.running = true;
            msleep(1000);
            let measured = dmm.value_ch1;
            dmm.running = false;

            let pass = TestFramework.assertApproxEqual(measured, voltage, 0.02,
                "Fine tuning verification");
            allPass = allPass && pass;

            switchToTool("Power Supply");
        }

        power.dac1_enabled = false;
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 6: Tracking Mode - Positive
TestFramework.runTest("TST.PS.TRACKING_POSITIVE", function() {
    try {
        power.dac1_enabled = true;
        power.dac2_enabled = true;
        power.sync = true;
        power.tracking_enabled = true;
        power.running = true;

        // Test different tracking percentages
        let testConfigs = [
            {percent: 100, dac1: 3.0, expected2: 3.0},
            {percent: 50, dac1: 4.0, expected2: 2.0},
            {percent: 75, dac1: 4.0, expected2: 3.0},
            {percent: 25, dac1: 4.0, expected2: 1.0}
        ];

        let allPass = true;

        for (let config of testConfigs) {
            printToConsole("  Testing tracking at " + config.percent + "%");
            power.tracking_percent = config.percent;
            power.dac1_value = config.dac1;
            msleep(1000);

            // Verify both channels with voltmeter
            switchToTool("Voltmeter");
            dmm.running = true;
            msleep(1000);
            let measured1 = dmm.value_ch1;
            let measured2 = dmm.value_ch2;
            dmm.running = false;

            let pass1 = TestFramework.assertApproxEqual(measured1, config.dac1, 0.05,
                "DAC1 tracking master");
            let pass2 = TestFramework.assertApproxEqual(measured2, config.expected2, 0.05,
                "DAC2 tracking slave at " + config.percent + "%");
            allPass = allPass && pass1 && pass2;

            switchToTool("Power Supply");
        }

        power.sync = false;
        power.tracking_enabled = false;
        power.dac1_enabled = false;
        power.dac2_enabled = false;
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 7: Tracking Mode - Negative
TestFramework.runTest("TST.PS.TRACKING_NEGATIVE", function() {
    try {
        power.dac1_enabled = true;
        power.dac2_enabled = true;
        power.sync = true;
        power.tracking_enabled = true;
        power.running = true;

        // Test tracking with negative voltages
        power.tracking_percent = 100;
        let testVoltages = [-1.0, -2.0, -3.0];
        let allPass = true;

        for (let voltage of testVoltages) {
            printToConsole("  Testing tracking at " + voltage + "V");
            power.dac1_value = voltage;
            msleep(1000);

            // Verify both channels
            switchToTool("Voltmeter");
            dmm.running = true;
            msleep(1000);
            let measured1 = dmm.value_ch1;
            let measured2 = dmm.value_ch2;
            dmm.running = false;

            let pass1 = TestFramework.assertApproxEqual(measured1, voltage, 0.05,
                "DAC1 negative tracking");
            let pass2 = TestFramework.assertApproxEqual(measured2, voltage, 0.05,
                "DAC2 negative tracking");
            allPass = allPass && pass1 && pass2;

            switchToTool("Power Supply");
        }

        power.sync = false;
        power.tracking_enabled = false;
        power.dac1_enabled = false;
        power.dac2_enabled = false;
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 8: Independent Channel Operation
TestFramework.runTest("TST.PS.INDEPENDENT_CHANNELS", function() {
    try {
        power.dac1_enabled = true;
        power.dac2_enabled = true;
        power.sync = false;
        power.running = true;

        // Set different voltages on each channel
        printToConsole("  Setting DAC1=2.5V, DAC2=-1.5V");
        power.dac1_value = 2.5;
        power.dac2_value = -1.5;
        msleep(1000);

        // Verify both channels independently
        switchToTool("Voltmeter");
        dmm.running = true;
        msleep(1000);
        let measured1 = dmm.value_ch1;
        let measured2 = dmm.value_ch2;
        dmm.running = false;

        let pass1 = TestFramework.assertApproxEqual(measured1, 2.5, 0.05,
            "DAC1 independent");
        let pass2 = TestFramework.assertApproxEqual(measured2, -1.5, 0.05,
            "DAC2 independent");

        switchToTool("Power Supply");
        power.dac1_enabled = false;
        power.dac2_enabled = false;

        return pass1 && pass2;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 9: Voltage Limits
TestFramework.runTest("TST.PS.VOLTAGE_LIMITS", function() {
    try {
        power.dac1_enabled = true;
        power.running = true;

        let allPass = true;

        // Test maximum positive
        printToConsole("  Testing maximum positive: 5V");
        power.dac1_value = 5.0;
        msleep(500);
        let readback = power.dac1_value;
        let pass1 = TestFramework.assertEqual(readback, 5.0, "Max positive limit");
        allPass = allPass && pass1;

        // Test maximum negative
        printToConsole("  Testing maximum negative: -5V");
        power.dac1_value = -5.0;
        msleep(500);
        readback = power.dac1_value;
        let pass2 = TestFramework.assertEqual(readback, -5.0, "Max negative limit");
        allPass = allPass && pass2;

        // Test out of range (should clamp)
        printToConsole("  Testing out of range values");
        power.dac1_value = 6.0; // Should clamp to 5.0
        msleep(100);
        readback = power.dac1_value;
        let pass3 = TestFramework.assertInRange(readback, 4.9, 5.1,
            "Positive clamping");
        allPass = allPass && pass3;

        power.dac1_value = -6.0; // Should clamp to -5.0
        msleep(100);
        readback = power.dac1_value;
        let pass4 = TestFramework.assertInRange(readback, -5.1, -4.9,
            "Negative clamping");
        allPass = allPass && pass4;

        power.dac1_enabled = false;
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 10: Enable/Disable Channels
TestFramework.runTest("TST.PS.ENABLE_DISABLE", function() {
    try {
        power.running = true;
        let allPass = true;

        // Test DAC1 enable/disable
        printToConsole("  Testing DAC1 enable/disable");
        power.dac1_value = 3.0;
        power.dac1_enabled = true;
        msleep(500);

        switchToTool("Voltmeter");
        dmm.running = true;
        msleep(1000);
        let measured = dmm.value_ch1;
        let pass1 = TestFramework.assertApproxEqual(measured, 3.0, 0.1,
            "DAC1 enabled output");
        dmm.running = false;

        switchToTool("Power Supply");
        power.dac1_enabled = false;
        msleep(500);

        switchToTool("Voltmeter");
        dmm.running = true;
        msleep(1000);
        measured = dmm.value_ch1;
        let pass2 = TestFramework.assertApproxEqual(measured, 0.0, 0.1,
            "DAC1 disabled output");
        dmm.running = false;

        // Test DAC2 enable/disable
        printToConsole("  Testing DAC2 enable/disable");
        switchToTool("Power Supply");
        power.dac2_value = -2.0;
        power.dac2_enabled = true;
        msleep(500);

        switchToTool("Voltmeter");
        dmm.running = true;
        msleep(1000);
        measured = dmm.value_ch2;
        let pass3 = TestFramework.assertApproxEqual(measured, -2.0, 0.1,
            "DAC2 enabled output");
        dmm.running = false;

        switchToTool("Power Supply");
        power.dac2_enabled = false;
        msleep(500);

        switchToTool("Voltmeter");
        dmm.running = true;
        msleep(1000);
        measured = dmm.value_ch2;
        let pass4 = TestFramework.assertApproxEqual(measured, 0.0, 0.1,
            "DAC2 disabled output");
        dmm.running = false;

        switchToTool("Power Supply");
        return pass1 && pass2 && pass3 && pass4;

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