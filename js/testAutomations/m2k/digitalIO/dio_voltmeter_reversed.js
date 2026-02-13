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

// Digital I/O Voltmeter/Power Supply Tests (Reversed)
// Based on RST test: TST.DIO.INDIVIDUAL.VOLTMETER (Step 10)
// Step 10: Repeat steps 5-9 using DIO 8 as output and DIO 1 as input
//
// Hardware Setup Required:
//   DIO 8 → 1+ (analog input for voltmeter)
//   DIO 1 → V+ (power supply positive output)
//   GND → 1-
//
// NOTE: No DIO loopback wires. DIO 8 and DIO 1 are each connected
// to separate instruments (voltmeter and power supply).

evaluateFile("../js/testAutomations/common/testFramework.js");

var waitTime = 1000;

TestFramework.init("Digital I/O Voltmeter Tests (Reversed)");

function toNum(value) {
    return value ? 1 : 0;
}

if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

if (!switchToTool("Digital I/O")) {
    printToConsole("ERROR: Cannot access Digital I/O");
    exit(1);
}

// ============================================================================
// TST.DIO.INDIVIDUAL.VOLTMETER (Step 10 - Reversed)
// Repeat steps 5-9 with DIO 8 as output and DIO 1 as input
// Steps 6-7 equivalent: DIO 8 output voltage measured by voltmeter via 1+
// Steps 8-9 equivalent: Power supply drives DIO 1 input via V+
// ============================================================================
TestFramework.runTest("TST.DIO.INDIVIDUAL.VOLTMETER", function() {
    try {
        let allPass = true;

        // Step 1: Open the DigitalIO instrument
        printToConsole("  Step 1: DigitalIO instrument opened");
        dio.running = true;

        // Step 2: Set DIO 0-7 and DIO 8-15 as individual pins
        printToConsole("  Step 2: Individual mode (default, group API not scriptable)");

        // Step 3: Set DIO 8 as output (reversed from original)
        printToConsole("  Step 3: Set DIO 8 as output");
        dio.dir[8] = 1;

        // Step 4: Set DIO 1 as input (reversed from original)
        printToConsole("  Step 4: Set DIO 1 as input");
        dio.dir[1] = 0;
        msleep(waitTime);

        // Step 5: Hardware connection (DIO 8 → 1+, DIO 1 → V+)
        printToConsole("  Step 5: Hardware: DIO 8 → 1+, DIO 1 → V+");

        // Step 6: Set DIO 8 to logic 0, measure with voltmeter
        printToConsole("  Step 6: Set DIO 8 to logic 0, measure voltage");
        dio.out[8] = 0;
        msleep(waitTime);

        switchToTool("Voltmeter");
        dmm.mode_ac_ch1 = false; // DC mode
        dmm.gainModes = [0, 0];  // +-25V range
        dmm.running = true;
        msleep(2000);

        let voltage_low = dmm.value_ch1;
        // Expected: voltage between -0.050V and 0.4V
        let pass6 = TestFramework.assertInRange(voltage_low, -0.05, 0.4,
            "DIO 8=0: Voltage " + voltage_low.toFixed(3) + "V (expect -0.05 to 0.4V)");
        allPass = allPass && pass6;

        // Step 7: Set DIO 8 to logic 1, measure voltage
        printToConsole("  Step 7: Set DIO 8 to logic 1, measure voltage");
        dmm.running = false;
        msleep(500);

        switchToTool("Digital I/O");
        dio.out[8] = 1;
        msleep(waitTime);

        switchToTool("Voltmeter");
        dmm.running = true;
        msleep(2000);

        let voltage_high = dmm.value_ch1;
        // Expected: voltage between 2.9V and 3.4V
        let pass7 = TestFramework.assertInRange(voltage_high, 2.9, 3.4,
            "DIO 8=1: Voltage " + voltage_high.toFixed(3) + "V (expect 2.9 to 3.4V)");
        allPass = allPass && pass7;

        dmm.running = false;
        msleep(500);

        // Step 8: Power supply 0.5V (between 0V and 0.8V), DIO 1 reads logic 0
        printToConsole("  Step 8: Power supply 0.5V, check DIO 1 reads logic 0");
        switchToTool("Power Supply");
        power.sync = false;
        power.dac1_value = 0.5;
        power.dac1_enabled = true;
        msleep(waitTime);

        switchToTool("Digital I/O");
        msleep(waitTime);

        let dio1_low = toNum(dio.gpi[1]);
        let pass8 = TestFramework.assertEqual(dio1_low, 0,
            "V+=0.5V: DIO 1 reads logic 0");
        allPass = allPass && pass8;

        // Step 9: Power supply 3.0V (between 2V and 3.3V), DIO 1 reads logic 1
        printToConsole("  Step 9: Power supply 3.0V, check DIO 1 reads logic 1");
        switchToTool("Power Supply");
        power.dac1_value = 3.0;
        msleep(waitTime);

        switchToTool("Digital I/O");
        msleep(waitTime);

        let dio1_high = toNum(dio.gpi[1]);
        let pass9 = TestFramework.assertEqual(dio1_high, 1,
            "V+=3.0V: DIO 1 reads logic 1");
        allPass = allPass && pass9;

        // Cleanup
        switchToTool("Power Supply");
        power.dac1_enabled = false;

        switchToTool("Digital I/O");
        dio.running = false;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.disconnectFromDevice();

let exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
