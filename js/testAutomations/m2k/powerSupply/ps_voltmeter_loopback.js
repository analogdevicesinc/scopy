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

// Power Supply + Voltmeter Loopback Tests
// Verifies power supply output using voltmeter instrument as measurement reference
//
// Hardware Setup:
//   V+ --> 1+  (positive power supply output to voltmeter channel 1 positive)
//   V- --> 2+  (negative power supply output to voltmeter channel 2 positive)
//
// RST UIDs covered:
//   TST.PS.POSITIVE    - Positive voltage output verification (steps 1-6)
//   TST.PS.NEGATIVE    - Negative voltage output verification (steps 1-6)
//   TST.PS.FINE_TUNING - SKIPPED (requires knob/button UI not available via API)
//   TST.PS.TRACKING    - Tracking mode and independent mode verification (steps 1-7)
//
// API Notes:
//   - power.dac1_enabled / power.dac2_enabled control hardware output
//   - PowerController_API has no 'running' Q_PROPERTY (unlike other M2K tool APIs)
//   - power.sync = false --> independent mode, power.sync = true --> tracking mode
//   - power.dac2_value can only be set when sync = false (independent mode)

evaluateFile("../js/testAutomations/common/testFramework.js");

TestFramework.init("Power Supply + Voltmeter Loopback Tests");

if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

if (!switchToTool("Power Supply")) {
    printToConsole("ERROR: Cannot access Power Supply");
    exit(1);
}

// Setup voltmeter: DC mode, +/-25V range for both channels
switchToTool("Voltmeter");
dmm.mode_ac_ch1 = false;
dmm.mode_ac_ch2 = false;
dmm.gainModes = [1, 1];  // 1 = +-25V range (0=Auto, 1=+-25V, 2=+-2.5V)
switchToTool("Power Supply");

// Helper: read voltage from voltmeter channel (1 or 2)
function readVoltmeterChannel(channel) {
    switchToTool("Voltmeter");
    dmm.running = true;
    msleep(1500);
    var value = (channel === 1) ? dmm.value_ch1 : dmm.value_ch2;
    dmm.running = false;
    switchToTool("Power Supply");
    return value;
}

// ============================================================================
// Test 1 - TST.PS.POSITIVE: Check positive voltage output
// ============================================================================
TestFramework.runTest("TST.PS.POSITIVE", function() {
    // Step 2: Set tracking ratio control to independent
    power.sync = false;

    // Step 3: Set the positive value to 3.3V and enable
    power.dac1_value = 3.3;
    power.dac1_enabled = true;
    msleep(500);

    // Step 4: Verify voltmeter reads 3.25V to 3.35V
    var v = readVoltmeterChannel(1);
    var pass = TestFramework.assertApproxEqual(v, 3.3, 0.05, "Step 4: DAC1 output at 3.3V");

    // Step 5: Change to 1.8V, verify 1.75V to 1.85V
    power.dac1_value = 1.8;
    msleep(500);
    v = readVoltmeterChannel(1);
    pass = TestFramework.assertApproxEqual(v, 1.8, 0.05, "Step 5: DAC1 output at 1.8V") && pass;

    // Step 6: Change to 5V, verify 4.95V to 5.05V
    power.dac1_value = 5.0;
    msleep(500);
    v = readVoltmeterChannel(1);
    pass = TestFramework.assertApproxEqual(v, 5.0, 0.05, "Step 6: DAC1 output at 5.0V") && pass;

    // Cleanup
    power.dac1_enabled = false;
    return pass;
});

// ============================================================================
// Test 2 - TST.PS.NEGATIVE: Check negative voltage output
// ============================================================================
TestFramework.runTest("TST.PS.NEGATIVE", function() {
    // Step 2: Set tracking ratio control to independent
    power.sync = false;

    // Step 3: Set the negative value to -3.3V and enable
    power.dac2_value = -3.3;
    power.dac2_enabled = true;
    msleep(500);

    // Step 4: Verify voltmeter reads -3.25V to -3.35V
    var v = readVoltmeterChannel(2);
    var pass = TestFramework.assertApproxEqual(v, -3.3, 0.05, "Step 4: DAC2 output at -3.3V");

    // Step 5: Change to -1.8V, verify -1.75V to -1.85V
    power.dac2_value = -1.8;
    msleep(500);
    v = readVoltmeterChannel(2);
    pass = TestFramework.assertApproxEqual(v, -1.8, 0.05, "Step 5: DAC2 output at -1.8V") && pass;

    // Step 6: Change to -5V, verify -4.95V to -5.05V
    power.dac2_value = -5.0;
    msleep(500);
    v = readVoltmeterChannel(2);
    pass = TestFramework.assertApproxEqual(v, -5.0, 0.05, "Step 6: DAC2 output at -5.0V") && pass;

    // Cleanup
    power.dac2_enabled = false;
    return pass;
});

// ============================================================================
// Test 3 - TST.PS.FINE_TUNING: Check fine tuning
// ============================================================================
TestFramework.runTest("TST.PS.FINE_TUNING", function() {
    // Steps 3-6 require setting knob intervals (+/-1V, +/-100mV) and using
    // +/- increment buttons. The Power Supply API does not expose knob interval
    // selection or +/- button interaction.
    // DAC voltage precision is already verified by TST.PS.POSITIVE and TST.PS.NEGATIVE.
    printToConsole("  SKIP: Knob interval and +/- button UI not available via API");
    return "SKIP";
});

// ============================================================================
// Test 4 - TST.PS.TRACKING: Check tracking mode
// ============================================================================
TestFramework.runTest("TST.PS.TRACKING", function() {
    // Step 2: Set tracking ratio control to tracking
    power.dac1_enabled = true;
    power.dac2_enabled = true;
    power.sync = true;

    // Step 3: Set the positive output to 5V, set tracking ratio to 50%
    power.dac1_value = 5.0;
    power.tracking_percent = 50;
    msleep(500);

    // Expected: negative output automatically set to -2.5V
    var v = readVoltmeterChannel(2);
    var pass = TestFramework.assertApproxEqual(v, -2.5, 0.05, "Step 3: Tracking 50% of 5V = -2.5V");

    // Step 4: Set tracking ratio to 100%
    power.tracking_percent = 100;
    msleep(500);

    // Expected: negative output automatically set to -5V
    v = readVoltmeterChannel(2);
    pass = TestFramework.assertApproxEqual(v, -5.0, 0.05, "Step 4: Tracking 100% of 5V = -5.0V") && pass;

    // Step 5: Set tracking ratio to 0%
    power.tracking_percent = 0;
    msleep(500);

    // Expected: negative output automatically set to 0mV
    v = readVoltmeterChannel(2);
    pass = TestFramework.assertApproxEqual(v, 0.0, 0.05, "Step 5: Tracking 0% of 5V = 0V") && pass;

    // Step 6: Set tracking ratio control to independent
    power.sync = false;

    // Step 7: Set the positive output to 5V and verify negative does not change
    // Note: switching to independent disables DAC2 (C++ sync_enabled behavior),
    // so we re-enable it before verifying
    power.dac2_enabled = true;
    msleep(500);

    var negBefore = readVoltmeterChannel(2);
    power.dac1_value = 5.0;
    msleep(500);
    var negAfter = readVoltmeterChannel(2);

    pass = TestFramework.assertApproxEqual(negAfter, negBefore, 0.1,
        "Step 7: Independent mode - negative unchanged after setting positive") && pass;

    // Cleanup
    power.dac1_enabled = false;
    power.dac2_enabled = false;
    return pass;
});

// Cleanup
TestFramework.disconnectFromDevice();

var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
