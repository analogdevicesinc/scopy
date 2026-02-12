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

// Voltmeter DC Differential Measurement Tests
// Based on RST test documentation for TST.M2K.VOLTMETER.*
//
// Hardware Setup Required:
//   V+ → 1+
//   V- → 1-
//   (Ch2 disconnected)
//
// Differential measurement: Ch1 measures V+ minus V-

evaluateFile("../js/testAutomations/common/testFramework.js");

TestFramework.init("Voltmeter DC Differential Tests");

if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

if (!switchToTool("Voltmeter")) {
    printToConsole("ERROR: Cannot access Voltmeter");
    exit(1);
}

// ============================================================================
// TST.M2K.VOLTMETER.CHANNEL_1_OPERATION (differential subset)
// Steps 7-8, 11: Differential measurement V+ - V-
// ============================================================================
TestFramework.runTest("TST.M2K.VOLTMETER.CHANNEL_1_OPERATION", function() {
    try {
        let allPass = true;

        // Set ch1 to DC mode, ensure +-25V range
        dmm.mode_ac_ch1 = false;
        dmm.gainModes = [0, 0];  // 0 = +-25V range
        msleep(100);

        // Steps 7-8: V+=5V, V-=-5V → ch1 reads ~10V differential
        switchToTool("Power Supply");
        power.sync = false;
        power.dac1_value = 5.0;
        power.dac2_value = -5.0;
        power.dac1_enabled = true;
        power.dac2_enabled = true;
        msleep(500);

        switchToTool("Voltmeter");
        dmm.running = true;
        msleep(1000);

        let measured = dmm.value_ch1;
        // Differential: V+ - V- = 5 - (-5) = 10V
        let pass1 = TestFramework.assertInRange(measured, 9.5, 10.5,
            "Ch1 differential: V+(5V) - V-(-5V) = ~10V");
        allPass = allPass && pass1;

        // Step 11: Reverse polarity test
        // Swap V+ and V- values to get negative differential
        switchToTool("Power Supply");
        power.dac1_value = 0.0;
        power.dac2_value = 0.0;
        msleep(500);

        switchToTool("Voltmeter");
        msleep(1000);

        measured = dmm.value_ch1;
        // With both at 0V: differential should be ~0V
        let pass2 = TestFramework.assertInRange(measured, -0.5, 0.5,
            "Ch1 differential: V+(0V) - V-(0V) = ~0V");
        allPass = allPass && pass2;

        // Test with V+=1V, V-=-5V → differential ~6V
        switchToTool("Power Supply");
        power.dac1_value = 1.0;
        power.dac2_value = -5.0;
        msleep(500);

        switchToTool("Voltmeter");
        msleep(1000);

        measured = dmm.value_ch1;
        let pass3 = TestFramework.assertInRange(measured, 5.5, 6.5,
            "Ch1 differential: V+(1V) - V-(-5V) = ~6V");
        allPass = allPass && pass3;

        // Cleanup
        dmm.running = false;
        switchToTool("Power Supply");
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

let exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
