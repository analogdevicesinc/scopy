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

// Digital I/O Loopback Tests
// Based on RST tests: TST.DIO.INDIVIDUAL, TST.DIO.GROUP
//
// Hardware Setup Required (Loopback):
//   DIO 0 → DIO 8
//   DIO 1 → DIO 9
//   DIO 2 → DIO 10
//   DIO 3 → DIO 11
//   DIO 4 → DIO 12
//   DIO 5 → DIO 13
//   DIO 6 → DIO 14
//   DIO 7 → DIO 15
//
// API Note: dio.group property has SCRIPTABLE=false, so group/individual
// mode cannot be set from JS. Group test simulates group behavior using
// individual pin operations.

evaluateFile("../js/testAutomations/common/testFramework.js");

var waitTime = 1000;

TestFramework.init("Digital I/O Loopback Tests");

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
// TST.DIO.INDIVIDUAL
// Check individual digital pin state
// ============================================================================
TestFramework.runTest("TST.DIO.INDIVIDUAL", function() {
    try {
        let allPass = true;

        // Step 1: Open the DigitalIO instrument
        printToConsole("  Step 1: DigitalIO instrument opened");
        dio.running = true;
        msleep(waitTime);

        // Step 2: Set DIO 0-7 and DIO 8-15 as individual pins
        // NOTE: dio.group has SCRIPTABLE=false, cannot set from JS. Default is individual.
        printToConsole("  Step 2: Individual mode (default, group API not scriptable)");

        // Step 3: Set DIO Channel 0 as output
        printToConsole("  Step 3: Set DIO 0 as output");
        dio.dir[0] = 1;
        msleep(waitTime);
        let dir0 = toNum(dio.dir[0]);
        if (dir0 !== 1) {
            printToConsole("    FAIL: DIO 0 direction not set to output");
            allPass = false;
        }

        // Step 4: Set DIO Channel 8 as input
        printToConsole("  Step 4: Set DIO 8 as input");
        dio.dir[8] = 0;
        msleep(waitTime);
        let dir8 = toNum(dio.dir[8]);
        if (dir8 !== 0) {
            printToConsole("    FAIL: DIO 8 direction not set to input");
            allPass = false;
        }

        // Step 5: Toggle DIO 0 and monitor DIO 8
        printToConsole("  Step 5: Testing loopback DIO 0 -> DIO 8");

        dio.out[0] = 1;
        msleep(waitTime);
        let gpi8_high = toNum(dio.gpi[8]);
        let pass5a = TestFramework.assertEqual(gpi8_high, 1,
            "DIO 0=1 -> DIO 8 reads 1");
        allPass = allPass && pass5a;

        dio.out[0] = 0;
        msleep(waitTime);
        let gpi8_low = toNum(dio.gpi[8]);
        let pass5b = TestFramework.assertEqual(gpi8_low, 0,
            "DIO 0=0 -> DIO 8 reads 0");
        allPass = allPass && pass5b;

        // Toggle multiple times
        for (let toggle = 0; toggle < 3; toggle++) {
            dio.out[0] = 1;
            msleep(waitTime);
            dio.out[0] = 0;
            msleep(waitTime);
        }

        // Step 6: Repeat with DIO 8 as output and DIO 0 as input
        // (RST says "DIO 1 as input" but loopback is DIO 0↔DIO 8)
        printToConsole("  Step 6: Testing reversed loopback DIO 8 -> DIO 0");

        dio.dir[8] = 1; // DIO 8 as output
        dio.dir[0] = 0; // DIO 0 as input
        msleep(waitTime);

        dio.out[8] = 1;
        msleep(waitTime);
        let gpi0_high = toNum(dio.gpi[0]);
        let pass6a = TestFramework.assertEqual(gpi0_high, 1,
            "DIO 8=1 -> DIO 0 reads 1");
        allPass = allPass && pass6a;

        dio.out[8] = 0;
        msleep(waitTime);
        let gpi0_low = toNum(dio.gpi[0]);
        let pass6b = TestFramework.assertEqual(gpi0_low, 0,
            "DIO 8=0 -> DIO 0 reads 0");
        allPass = allPass && pass6b;

        dio.running = false;
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// TST.DIO.GROUP
// Checking grouped digital pin states
// NOTE: dio.group has SCRIPTABLE=false, simulating group behavior with
// individual pin operations.
// ============================================================================
TestFramework.runTest("TST.DIO.GROUP", function() {
    try {
        let allPass = true;

        // Step 1: Open the DigitalIO instrument
        printToConsole("  Step 1: DigitalIO instrument opened");
        dio.running = true;
        msleep(waitTime);

        // Step 2: Set DIO 0-7 and DIO 8-15 as Group pins
        // NOTE: dio.group has SCRIPTABLE=false, simulating with individual pins
        printToConsole("  Step 2: Group mode (simulated, group API not scriptable)");

        // Step 3: Set DIO 0-7 as output and DIO 8-15 as input
        printToConsole("  Step 3: DIO 0-7 as output, DIO 8-15 as input");
        for (let i = 0; i < 8; i++) {
            dio.dir[i] = 1;     // Output
            dio.dir[i + 8] = 0; // Input
        }
        msleep(waitTime);

        // Step 4: Set DIO 0-7 to value 0
        printToConsole("  Step 4: Set DIO 0-7 to value 0");
        for (let i = 0; i < 8; i++) {
            dio.out[i] = 0;
        }
        msleep(waitTime);

        let readValue = 0;
        for (let i = 0; i < 8; i++) {
            readValue |= (toNum(dio.gpi[i + 8]) << i);
        }
        let pass4 = TestFramework.assertEqual(readValue, 0,
            "Group value 0: DIO 8-15 reads 0");
        allPass = allPass && pass4;

        // Step 5: Set DIO 0-7 to value 128
        printToConsole("  Step 5: Set DIO 0-7 to value 128");
        for (let i = 0; i < 8; i++) {
            dio.out[i] = (128 >> i) & 1;
        }
        msleep(waitTime);

        readValue = 0;
        for (let i = 0; i < 8; i++) {
            readValue |= (toNum(dio.gpi[i + 8]) << i);
        }
        let pass5 = TestFramework.assertEqual(readValue, 128,
            "Group value 128: DIO 8-15 reads 128");
        allPass = allPass && pass5;

        // Step 6: Set DIO 0-7 as input and DIO 8-15 as output
        printToConsole("  Step 6: Reversing - DIO 0-7 as input, DIO 8-15 as output");
        for (let i = 0; i < 8; i++) {
            dio.dir[i] = 0;     // Input
            dio.dir[i + 8] = 1; // Output
        }
        msleep(waitTime);

        // Step 7: Set DIO 8-15 to value 0
        printToConsole("  Step 7: Set DIO 8-15 to value 0");
        for (let i = 0; i < 8; i++) {
            dio.out[i + 8] = 0;
        }
        msleep(waitTime);

        readValue = 0;
        for (let i = 0; i < 8; i++) {
            readValue |= (toNum(dio.gpi[i]) << i);
        }
        let pass7 = TestFramework.assertEqual(readValue, 0,
            "Reversed group value 0: DIO 0-7 reads 0");
        allPass = allPass && pass7;

        // Step 8: Set DIO 8-15 to value 128
        printToConsole("  Step 8: Set DIO 8-15 to value 128");
        for (let i = 0; i < 8; i++) {
            dio.out[i + 8] = (128 >> i) & 1;
        }
        msleep(waitTime);

        readValue = 0;
        for (let i = 0; i < 8; i++) {
            readValue |= (toNum(dio.gpi[i]) << i);
        }
        let pass8 = TestFramework.assertEqual(readValue, 128,
            "Reversed group value 128: DIO 0-7 reads 128");
        allPass = allPass && pass8;

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
