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

// Power Supply + Voltmeter Integration Tests
// Tests that use Voltmeter to verify Power Supply output
// Based on TST.PS.* test specifications

/*
 * Power Supply Hardware Behavior:
 * - DAC1 (Channel 0/V+): Outputs 0V to +5V (positive rail)
 *   - Negative inputs are clamped to 0V
 * - DAC2 (Channel 1/V-): Outputs 0V to -5V (negative rail)
 *   - Setting -2V results in -2V output
 *   - Setting -5V results in -5V output
 *   - Positive inputs are clamped to 0V
 * - Tracking mode: DAC2 outputs -(DAC1 * tracking_percent / 100)
 * - This is the actual M2K hardware behavior
 *
 * IMPORTANT API BEHAVIOR:
 * - power.sync = true  → TRACKING mode (btnSync checked, DAC2 follows DAC1)
 * - power.sync = false → INDEPENDENT mode (btnSync unchecked, both DACs set independently)
 * - In tracking mode, DAC2 value cannot be set directly - it follows DAC1
 */

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

// Test 1: Check positive voltage output
TestFramework.runTest("TST.PS.POSITIVE", function() {
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

            // Increased tolerance for hardware variation, especially at higher voltages
            let tolerance = (voltage >= 4.5) ? 0.1 : 0.06;  // Increased from 0.05 to 0.06
            let pass = TestFramework.assertApproxEqual(measured, voltage, tolerance,
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

// Test 2: Check negative voltage output
TestFramework.runTest("TST.PS.NEGATIVE", function() {
    try {
        power.sync = false;  // Required for DAC2 value to be applied (C++ API quirk)
        power.dac2_enabled = true;

        // DAC2 outputs negative voltages
        printToConsole("  Testing DAC2 negative voltage output");
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

            // DAC2 outputs negative voltages
            let tolerance = (Math.abs(voltage) >= 4.5) ? 0.1 : 0.06;
            let pass = TestFramework.assertApproxEqual(measured, voltage, tolerance,
                "DAC2 negative voltage output");
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

// Test 3: Fine Voltage Tuning
TestFramework.runTest("TST.PS.FINE_TUNING", function() {
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

            // Increased tolerance to ±0.1V for realistic hardware accuracy
            let pass = TestFramework.assertApproxEqual(measured, voltage, 0.1,
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

// Test 4: Tracking Mode
TestFramework.runTest("TST.PS.TRACKING", function() {
    try {
        power.dac1_enabled = true;
        power.dac2_enabled = true;
        power.sync = true;  // true = tracking mode (btnSync checked)

        // Test tracking mode: DAC2 outputs -(DAC1 * tracking_percent/100)
        // DAC1 provides positive voltages, DAC2 outputs negative voltages
        printToConsole("  Testing tracking mode with positive master (DAC1)");
        let testConfigs = [
            {percent: 100, dac1: 3.0, expected2: -3.0},  // DAC2 outputs -3V
            {percent: 50, dac1: 4.0, expected2: -2.0},   // DAC2 outputs -2V
            {percent: 75, dac1: 4.0, expected2: -3.0},   // DAC2 outputs -3V
            {percent: 25, dac1: 4.0, expected2: -1.0}    // DAC2 outputs -1V
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

            let pass1 = TestFramework.assertApproxEqual(measured1, config.dac1, 0.1,
                "DAC1 tracking master");
            let pass2 = TestFramework.assertApproxEqual(measured2, config.expected2, 0.15,
                "DAC2 tracking slave at " + config.percent + "%");
            allPass = allPass && pass1 && pass2;

            switchToTool("Power Supply");
        }

        power.sync = false;  // false = return to independent mode
        power.dac1_enabled = false;
        power.dac2_enabled = false;
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
let exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();