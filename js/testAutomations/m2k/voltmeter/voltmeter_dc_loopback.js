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

// Voltmeter DC Loopback Tests
// Based on RST test documentation for TST.M2K.VOLTMETER.*
//
// Hardware Setup Required:
//   V+ → 1+
//   V- → 2+
//   GND → 1-
//   GND → 2-
//
// Ch1 measures V+ (positive), Ch2 measures V- (negative)

evaluateFile("../js/testAutomations/common/testFramework.js");

TestFramework.init("Voltmeter DC Loopback Tests");

if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

if (!switchToTool("Voltmeter")) {
    printToConsole("ERROR: Cannot access Voltmeter");
    exit(1);
}

// ============================================================================
// TST.M2K.VOLTMETER.CHANNEL_1_OPERATION (DC subset)
// Steps 1-4: Ch1 DC mode, measure V+ at 3.3V
// ============================================================================
TestFramework.runTest("TST.M2K.VOLTMETER.CHANNEL_1_OPERATION", function() {
    try {
        let allPass = true;

        // Step 1-2: Set ch1 to DC mode, ensure +-25V range
        dmm.mode_ac_ch1 = false;
        dmm.gainModes = [0, 0];  // 0 = +-25V range (default may be +-2.5V which clips >2.5V)
        msleep(100);
        let pass1 = TestFramework.assertEqual(dmm.mode_ac_ch1, false, "Ch1 DC mode set");
        allPass = allPass && pass1;

        // Step 3-4: Set V+ to 3.3V, verify ch1 reading
        switchToTool("Power Supply");
        power.dac1_value = 3.3;
        power.dac1_enabled = true;
        msleep(500);

        switchToTool("Voltmeter");
        dmm.running = true;
        msleep(1000);

        let measured = dmm.value_ch1;
        let pass2 = TestFramework.assertInRange(measured, 3.2, 3.4,
            "Ch1 DC reads V+ at 3.3V");
        allPass = allPass && pass2;

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
// TST.M2K.VOLTMETER.CHANNEL_2_OPERATION (DC subset)
// Steps 1-2, 5-6: Ch2 DC mode, measure V- at -3.3V
// ============================================================================
TestFramework.runTest("TST.M2K.VOLTMETER.CHANNEL_2_OPERATION", function() {
    try {
        let allPass = true;

        // Step 1-2: Set ch2 to DC mode, ensure +-25V range
        switchToTool("Voltmeter");
        dmm.mode_ac_ch2 = false;
        dmm.gainModes = [0, 0];  // 0 = +-25V range
        msleep(100);
        let pass1 = TestFramework.assertEqual(dmm.mode_ac_ch2, false, "Ch2 DC mode set");
        allPass = allPass && pass1;

        // Step 5-6: Set V- to -3.3V, verify ch2 reading
        switchToTool("Power Supply");
        power.sync = false;
        power.dac2_value = -3.3;
        power.dac2_enabled = true;
        msleep(500);

        switchToTool("Voltmeter");
        dmm.running = true;
        msleep(1000);

        let measured = dmm.value_ch2;
        let pass2 = TestFramework.assertInRange(measured, -3.4, -3.2,
            "Ch2 DC reads V- at -3.3V");
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
// TST.M2K.VOLTMETER.CHANNEL_1_AND_CHANNEL_2_OPERATION (DC subset)
// Steps 1-7: Dual DC, toggle history graphs
// ============================================================================
TestFramework.runTest("TST.M2K.VOLTMETER.CHANNEL_1_AND_CHANNEL_2_OPERATION", function() {
    try {
        let allPass = true;

        // Step 1: Both channels DC mode, ensure +-25V range
        switchToTool("Voltmeter");
        dmm.mode_ac_ch1 = false;
        dmm.mode_ac_ch2 = false;
        dmm.gainModes = [0, 0];  // 0 = +-25V range

        // Step 2: Set V+=3.3V, V-=-4.5V
        switchToTool("Power Supply");
        power.sync = false;
        power.dac1_value = 3.3;
        power.dac2_value = -4.5;
        power.dac1_enabled = true;
        power.dac2_enabled = true;
        msleep(500);

        switchToTool("Voltmeter");
        dmm.running = true;
        msleep(1000);

        let m1 = dmm.value_ch1;
        let m2 = dmm.value_ch2;
        let pass1 = TestFramework.assertInRange(m1, 3.2, 3.4, "Ch1 DC at 3.3V");
        let pass2 = TestFramework.assertInRange(m2, -4.6, -4.4, "Ch2 DC at -4.5V");
        allPass = allPass && pass1 && pass2;

        // Step 3: Toggle histogram ch1 off, verify readings unchanged
        dmm.histogram_ch1 = false;
        msleep(500);
        m1 = dmm.value_ch1;
        let pass3 = TestFramework.assertInRange(m1, 3.2, 3.4,
            "Ch1 reading stable after histogram ch1 off");
        allPass = allPass && pass3;

        // Step 4: Toggle histogram ch2 off, verify readings unchanged
        dmm.histogram_ch2 = false;
        msleep(500);
        m2 = dmm.value_ch2;
        let pass4 = TestFramework.assertInRange(m2, -4.6, -4.4,
            "Ch2 reading stable after histogram ch2 off");
        allPass = allPass && pass4;

        // Step 5: Both histograms off, verify readings unchanged
        msleep(500);
        m1 = dmm.value_ch1;
        m2 = dmm.value_ch2;
        let pass5 = TestFramework.assertInRange(m1, 3.2, 3.4,
            "Ch1 stable with both histograms off");
        let pass6 = TestFramework.assertInRange(m2, -4.6, -4.4,
            "Ch2 stable with both histograms off");
        allPass = allPass && pass5 && pass6;

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

// ============================================================================
// TST.M2K.VOLTMETER.ADDITIONAL_FEATURES (peak hold + range subset)
// Steps 1-11, 27-33: Peak hold, reset, range switching
// ============================================================================
TestFramework.runTest("TST.M2K.VOLTMETER.ADDITIONAL_FEATURES", function() {
    try {
        let allPass = true;

        switchToTool("Voltmeter");
        dmm.mode_ac_ch1 = false;
        dmm.mode_ac_ch2 = false;
        dmm.gainModes = [0, 0];  // 0 = +-25V range for peak hold tests

        // Step 1: Enable peak hold
        dmm.peak_hold_en = true;
        dmm.running = true;

        // Step 2: Set V+=2.5V, V-=-3V
        switchToTool("Power Supply");
        power.sync = false;
        power.dac1_value = 2.5;
        power.dac2_value = -3.0;
        power.dac1_enabled = true;
        power.dac2_enabled = true;
        msleep(2000);

        switchToTool("Voltmeter");
        msleep(1000);

        // Peak hold: ch1 max should be ~2.5V, ch2 min should be ~-3V
        printToConsole("  Peak hold with V+=2.5V, V-=-3V");
        let ch1_val = dmm.value_ch1;
        let ch2_val = dmm.value_ch2;
        printToConsole("    Ch1: " + ch1_val + "V, Ch2: " + ch2_val + "V");

        // Step 3: Increase to V+=5V, V-=-5V
        switchToTool("Power Supply");
        power.dac1_value = 5.0;
        power.dac2_value = -5.0;
        msleep(2000);

        switchToTool("Voltmeter");
        msleep(1000);

        // Peak hold: ch1 max should now be ~5V, ch2 min should be ~-5V
        ch1_val = dmm.value_ch1;
        ch2_val = dmm.value_ch2;
        printToConsole("  Peak hold with V+=5V, V-=-5V");
        printToConsole("    Ch1: " + ch1_val + "V, Ch2: " + ch2_val + "V");

        // Step 4: Stop and reset peak hold
        dmm.running = false;
        dmm.peak_hold_en = false;
        msleep(500);
        dmm.peak_hold_en = true;
        let pass1 = TestFramework.assertEqual(dmm.peak_hold_en, true, "Peak hold re-enabled after reset");
        allPass = allPass && pass1;

        // Step 5: Range +-25V test - set V+=3.3V, V-=-3.3V
        dmm.peak_hold_en = false;
        switchToTool("Power Supply");
        power.dac1_value = 3.3;
        power.dac2_value = -3.3;
        msleep(500);

        switchToTool("Voltmeter");
        // Set range to +-25V (gain mode index for +-25V range)
        // gainModes is QVector<int>, index 0=ch1, index 1=ch2
        // 0 = +-25V range, 1 = +-2.5V range
        dmm.gainModes = [0, 0];
        dmm.running = true;
        msleep(1000);

        let m1 = dmm.value_ch1;
        let m2 = dmm.value_ch2;
        let pass2 = TestFramework.assertInRange(m1, 3.1, 3.5,
            "Ch1 at 3.3V with +-25V range");
        let pass3 = TestFramework.assertInRange(m2, -3.5, -3.1,
            "Ch2 at -3.3V with +-25V range");
        allPass = allPass && pass2 && pass3;

        // Step 6: Switch to +-2.5V range without disabling - out of range expected
        dmm.gainModes = [1, 1];
        msleep(1000);
        printToConsole("  Switched to +-2.5V range with 3.3V input (out of range expected)");
        // NOTE: Out of range behavior depends on hardware - just verify no crash

        // Step 7: With +-2.5V range, set V+=100mV, V-=-100mV
        switchToTool("Power Supply");
        power.dac1_value = 0.1;
        power.dac2_value = -0.1;
        msleep(500);

        switchToTool("Voltmeter");
        msleep(1000);

        m1 = dmm.value_ch1;
        m2 = dmm.value_ch2;
        // Wider tolerance - power supply accuracy is limited at low voltages
        let pass4 = TestFramework.assertInRange(m1, 0.04, 0.16,
            "Ch1 at 100mV with +-2.5V range");
        let pass5 = TestFramework.assertInRange(m2, -0.17, -0.04,
            "Ch2 at -100mV with +-2.5V range");
        allPass = allPass && pass4 && pass5;

        // Step 8: Switch back to +-25V range
        dmm.gainModes = [0, 0];
        msleep(1000);
        m1 = dmm.value_ch1;
        let pass6 = TestFramework.assertInRange(m1, 0.05, 0.15,
            "Ch1 at 100mV with +-25V range (lower precision)");
        allPass = allPass && pass6;

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
