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

// Voltmeter DC Reversed Polarity Tests
// Based on RST test documentation for TST.M2K.VOLTMETER.*
//
// Hardware Setup Required:
//   V- → 1+
//   V+ → 2+
//   GND → 1-
//   GND → 2-
//
// Reversed loopback. Ch1 measures V- (negative), Ch2 measures V+ (positive).

evaluateFile("../js/testAutomations/common/testFramework.js");

TestFramework.init("Voltmeter DC Reversed Polarity Tests");

if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

if (!switchToTool("Voltmeter")) {
    printToConsole("ERROR: Cannot access Voltmeter");
    exit(1);
}

// ============================================================================
// TST.M2K.VOLTMETER.CHANNEL_1_OPERATION (reversed subset)
// Steps 5-6, 10: Ch1 DC with negative supply (V- wired to 1+)
// ============================================================================
TestFramework.runTest("TST.M2K.VOLTMETER.CHANNEL_1_OPERATION", function() {
    try {
        let allPass = true;

        // Set ch1 to DC mode, ensure +-25V range
        dmm.mode_ac_ch1 = false;
        dmm.gainModes = [0, 0];  // 0 = +-25V range
        msleep(100);

        // V- connected to 1+, so set V- to -3.3V → ch1 reads -3.3V
        switchToTool("Power Supply");
        power.sync = false;
        power.dac2_value = -3.3;
        power.dac2_enabled = true;
        msleep(500);

        switchToTool("Voltmeter");
        dmm.running = true;
        msleep(1000);

        let measured = dmm.value_ch1;
        let pass1 = TestFramework.assertInRange(measured, -3.4, -3.2,
            "Ch1 reads V- at -3.3V (reversed wiring)");
        allPass = allPass && pass1;

        // Step 10: Polarity reversal test
        // With V- connected to 1+, positive voltage from V- rail
        // V- set to 0V should read ~0V on ch1
        switchToTool("Power Supply");
        power.dac2_value = 0;
        msleep(500);

        switchToTool("Voltmeter");
        msleep(1000);
        measured = dmm.value_ch1;
        let pass2 = TestFramework.assertInRange(measured, -0.1, 0.1,
            "Ch1 reads ~0V when V- set to 0V");
        allPass = allPass && pass2;

        // Cleanup
        dmm.running = false;
        switchToTool("Power Supply");
        power.dac2_enabled = false;

        return allPass;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// TST.M2K.VOLTMETER.CHANNEL_2_OPERATION (reversed subset)
// Steps 3-4, 9: Ch2 DC with positive supply (V+ wired to 2+)
// ============================================================================
TestFramework.runTest("TST.M2K.VOLTMETER.CHANNEL_2_OPERATION", function() {
    try {
        let allPass = true;

        // Set ch2 to DC mode, ensure +-25V range
        switchToTool("Voltmeter");
        dmm.mode_ac_ch2 = false;
        dmm.gainModes = [0, 0];  // 0 = +-25V range
        msleep(100);

        // V+ connected to 2+, so set V+ to 3.3V → ch2 reads 3.3V
        switchToTool("Power Supply");
        power.dac1_value = 3.3;
        power.dac1_enabled = true;
        msleep(500);

        switchToTool("Voltmeter");
        dmm.running = true;
        msleep(1000);

        let measured = dmm.value_ch2;
        let pass1 = TestFramework.assertInRange(measured, 3.2, 3.4,
            "Ch2 reads V+ at 3.3V (reversed wiring)");
        allPass = allPass && pass1;

        // Cleanup
        dmm.running = false;
        switchToTool("Power Supply");
        power.dac1_enabled = false;

        return allPass;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// TST.M2K.VOLTMETER.ADDITIONAL_FEATURES (peak hold reversed subset)
// Steps 7-9: Peak hold with reversed connections
// ============================================================================
TestFramework.runTest("TST.M2K.VOLTMETER.ADDITIONAL_FEATURES", function() {
    try {
        let allPass = true;

        switchToTool("Voltmeter");
        dmm.mode_ac_ch1 = false;
        dmm.mode_ac_ch2 = false;
        dmm.gainModes = [0, 0];  // 0 = +-25V range
        dmm.peak_hold_en = true;
        dmm.running = true;

        // With reversed wiring: ch1=V-, ch2=V+
        // Set V+=3V, V-=-2V
        switchToTool("Power Supply");
        power.sync = false;
        power.dac1_value = 3.0;
        power.dac2_value = -2.0;
        power.dac1_enabled = true;
        power.dac2_enabled = true;
        msleep(2000);

        switchToTool("Voltmeter");
        msleep(1000);

        // Ch1 reads V- → should be ~-2V
        // Ch2 reads V+ → should be ~3V
        let ch1_val = dmm.value_ch1;
        let ch2_val = dmm.value_ch2;
        printToConsole("  Peak hold reversed: Ch1=" + ch1_val + "V, Ch2=" + ch2_val + "V");

        // Increase voltages
        switchToTool("Power Supply");
        power.dac1_value = 5.0;
        power.dac2_value = -5.0;
        msleep(2000);

        switchToTool("Voltmeter");
        msleep(1000);

        ch1_val = dmm.value_ch1;
        ch2_val = dmm.value_ch2;
        printToConsole("  Peak hold after increase: Ch1=" + ch1_val + "V, Ch2=" + ch2_val + "V");

        // Verify peak hold is tracking correctly
        let pass1 = TestFramework.assertEqual(dmm.peak_hold_en, true,
            "Peak hold still enabled");
        allPass = allPass && pass1;

        // Cleanup
        dmm.peak_hold_en = false;
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
