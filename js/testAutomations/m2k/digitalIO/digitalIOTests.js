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

// Digital I/O Automated Tests
// Based on TST.DIO.* test specifications

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: Digital I/O
TestFramework.init("Digital I/O Tests");

// Helper function: Convert boolean API returns to numeric values (0/1)
// The Digital I/O API returns boolean values but tests expect numeric
function toNum(value) {
    return value ? 1 : 0;
}

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to Digital I/O tool
if (!switchToTool("Digital I/O")) {
    printToConsole("ERROR: Cannot access Digital I/O");
    exit(1);
}

// Test 1: Pin Direction Configuration
TestFramework.runTest("TST.DIO.PIN_DIRECTION", function() {
    try {
        dio.running = true;
        let allPass = true;

        // Test setting all pins as outputs
        printToConsole("  Setting all pins as outputs");
        for (let i = 0; i < 16; i++) {
            dio.dir[i] = 1; // Output
        }
        msleep(100);

        // Verify direction settings
        for (let i = 0; i < 16; i++) {
            let dir = toNum(dio.dir[i]);
            if (dir !== 1) {
                printToConsole("  ✗ Pin " + i + " direction incorrect: " + dir);
                allPass = false;
            }
        }

        // Test setting all pins as inputs
        printToConsole("  Setting all pins as inputs");
        for (let i = 0; i < 16; i++) {
            dio.dir[i] = 0; // Input
        }
        msleep(100);

        // Verify direction settings
        for (let i = 0; i < 16; i++) {
            let dir = toNum(dio.dir[i]);
            if (dir !== 0) {
                printToConsole("  ✗ Pin " + i + " direction incorrect: " + dir);
                allPass = false;
            }
        }

        // Test mixed configuration
        printToConsole("  Setting mixed input/output configuration");
        for (let i = 0; i < 16; i++) {
            dio.dir[i] = i % 2; // Alternating 0,1,0,1...
        }
        msleep(100);

        for (let i = 0; i < 16; i++) {
            let expected = i % 2;
            let dir = toNum(dio.dir[i]);
            if (dir !== expected) {
                printToConsole("  ✗ Pin " + i + " direction incorrect: " + dir);
                allPass = false;
            }
        }

        if (allPass) {
            printToConsole("  ✓ All pin directions configured correctly");
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 2: Output Pin Control
TestFramework.runTest("TST.DIO.OUTPUT_CONTROL", function() {
    try {
        dio.running = true;
        let allPass = true;

        // Configure all pins as outputs
        for (let i = 0; i < 16; i++) {
            dio.dir[i] = 1; // Output
        }
        msleep(100);

        // Test setting all high
        printToConsole("  Setting all outputs HIGH");
        for (let i = 0; i < 16; i++) {
            dio.out[i] = 1;
        }
        msleep(100);

        for (let i = 0; i < 16; i++) {
            let val = toNum(dio.out[i]);
            if (val !== 1) {
                printToConsole("  ✗ Pin " + i + " output not HIGH: " + val);
                allPass = false;
            }
        }

        // Test setting all low
        printToConsole("  Setting all outputs LOW");
        for (let i = 0; i < 16; i++) {
            dio.out[i] = 0;
        }
        msleep(100);

        for (let i = 0; i < 16; i++) {
            let val = toNum(dio.out[i]);
            if (val !== 0) {
                printToConsole("  ✗ Pin " + i + " output not LOW: " + val);
                allPass = false;
            }
        }

        // Test alternating pattern
        printToConsole("  Setting alternating pattern (0101...)");
        for (let i = 0; i < 16; i++) {
            dio.out[i] = i % 2;
        }
        msleep(100);

        for (let i = 0; i < 16; i++) {
            let expected = i % 2;
            let val = toNum(dio.out[i]);
            if (val !== expected) {
                printToConsole("  ✗ Pin " + i + " incorrect: " + val);
                allPass = false;
            }
        }

        if (allPass) {
            printToConsole("  ✓ All output controls working correctly");
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 3: Input Pin Reading (requires loopback)
TestFramework.runTest("TST.DIO.INPUT_READING", function() {
    try {
        dio.running = true;

        printToConsole("  Note: This test requires loopback connections");
        printToConsole("  Connect DIO0→DIO8, DIO1→DIO9, etc.");

        // Configure lower 8 as outputs, upper 8 as inputs
        for (let i = 0; i < 8; i++) {
            dio.dir[i] = 1; // Output
            dio.dir[i + 8] = 0; // Input
        }
        msleep(100);

        let allPass = true;

        // Test pattern 1: All high
        printToConsole("  Testing HIGH signal propagation");
        for (let i = 0; i < 8; i++) {
            dio.out[i] = 1;
        }
        msleep(200);

        for (let i = 0; i < 8; i++) {
            let inputVal = toNum(dio.gpi[i + 8]);
            let pass = TestFramework.assertEqual(inputVal, 1,
                "DIO" + (i+8) + " reads HIGH from DIO" + i);
            allPass = allPass && pass;
        }

        // Test pattern 2: All low
        printToConsole("  Testing LOW signal propagation");
        for (let i = 0; i < 8; i++) {
            dio.out[i] = 0;
        }
        msleep(200);

        for (let i = 0; i < 8; i++) {
            let inputVal = toNum(dio.gpi[i + 8]);
            let pass = TestFramework.assertEqual(inputVal, 0,
                "DIO" + (i+8) + " reads LOW from DIO" + i);
            allPass = allPass && pass;
        }

        // Test pattern 3: Alternating
        printToConsole("  Testing alternating pattern");
        for (let i = 0; i < 8; i++) {
            dio.out[i] = i % 2;
        }
        msleep(200);

        for (let i = 0; i < 8; i++) {
            let expected = i % 2;
            let inputVal = toNum(dio.gpi[i + 8]);
            let pass = TestFramework.assertEqual(inputVal, expected,
                "DIO" + (i+8) + " reads pattern from DIO" + i);
            allPass = allPass && pass;
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 4: Binary Counter Pattern
TestFramework.runTest("TST.DIO.BINARY_COUNTER", function() {
    try {
        dio.running = true;

        // Configure lower 8 pins as outputs
        for (let i = 0; i < 8; i++) {
            dio.dir[i] = 1;
        }
        msleep(100);

        printToConsole("  Running 8-bit binary counter (0-255)");
        let allPass = true;

        // Test some key values
        let testValues = [0, 1, 2, 4, 8, 16, 32, 64, 128, 255, 170, 85];

        for (let value of testValues) {
            printToConsole("  Testing binary value: " + value);

            // Set binary pattern
            for (let bit = 0; bit < 8; bit++) {
                dio.out[bit] = (value >> bit) & 1;
            }
            msleep(50);

            // Verify pattern
            let readback = 0;
            for (let bit = 0; bit < 8; bit++) {
                readback |= (toNum(dio.out[bit]) << bit);
            }

            let pass = TestFramework.assertEqual(readback, value,
                "Binary pattern verification");
            allPass = allPass && pass;
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 5: Walking Ones Pattern
TestFramework.runTest("TST.DIO.WALKING_ONES", function() {
    try {
        dio.running = true;

        // Configure all pins as outputs
        for (let i = 0; i < 16; i++) {
            dio.dir[i] = 1;
        }
        msleep(100);

        printToConsole("  Running walking ones pattern");
        let allPass = true;

        for (let position = 0; position < 16; position++) {
            // Clear all pins
            for (let i = 0; i < 16; i++) {
                dio.out[i] = 0;
            }

            // Set only one pin high
            dio.out[position] = 1;
            msleep(50);

            // Verify only one pin is high
            let highCount = 0;
            let highPin = -1;
            for (let i = 0; i < 16; i++) {
                if (toNum(dio.out[i]) === 1) {
                    highCount++;
                    highPin = i;
                }
            }

            if (highCount !== 1 || highPin !== position) {
                printToConsole("  ✗ Walking ones failed at position " + position);
                allPass = false;
            }
        }

        if (allPass) {
            printToConsole("  ✓ Walking ones pattern completed successfully");
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 6: Walking Zeros Pattern
TestFramework.runTest("TST.DIO.WALKING_ZEROS", function() {
    try {
        dio.running = true;

        // Configure all pins as outputs
        for (let i = 0; i < 16; i++) {
            dio.dir[i] = 1;
        }
        msleep(100);

        printToConsole("  Running walking zeros pattern");
        let allPass = true;

        for (let position = 0; position < 16; position++) {
            // Set all pins high
            for (let i = 0; i < 16; i++) {
                dio.out[i] = 1;
            }

            // Clear only one pin
            dio.out[position] = 0;
            msleep(50);

            // Verify only one pin is low
            let lowCount = 0;
            let lowPin = -1;
            for (let i = 0; i < 16; i++) {
                if (toNum(dio.out[i]) === 0) {
                    lowCount++;
                    lowPin = i;
                }
            }

            if (lowCount !== 1 || lowPin !== position) {
                printToConsole("  ✗ Walking zeros failed at position " + position);
                allPass = false;
            }
        }

        if (allPass) {
            printToConsole("  ✓ Walking zeros pattern completed successfully");
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 7: Rapid Toggle Test
TestFramework.runTest("TST.DIO.RAPID_TOGGLE", function() {
    try {
        dio.running = true;

        // Configure pin 0 as output
        dio.dir[0] = 1;
        msleep(100);

        printToConsole("  Rapid toggle test on DIO0");
        let toggleCount = 100;
        let startTime = Date.now();

        for (let i = 0; i < toggleCount; i++) {
            dio.out[0] = i % 2;
        }

        let duration = Date.now() - startTime;
        let toggleRate = (toggleCount * 1000) / duration;

        printToConsole("  Completed " + toggleCount + " toggles in " + duration + "ms");
        printToConsole("  Toggle rate: " + toggleRate.toFixed(0) + " toggles/sec");

        // Verify final state
        let finalState = toNum(dio.out[0]);
        let expectedState = (toggleCount - 1) % 2;
        let pass = TestFramework.assertEqual(finalState, expectedState,
            "Final state verification");

        return pass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 8: Group Operations
TestFramework.runTest("TST.DIO.GROUP_OPERATIONS", function() {
    try {
        dio.running = true;

        // Configure all as outputs
        for (let i = 0; i < 16; i++) {
            dio.dir[i] = 1;
        }
        msleep(100);

        let allPass = true;

        // Test 1: Lower byte high, upper byte low
        printToConsole("  Setting lower byte HIGH, upper byte LOW");
        for (let i = 0; i < 8; i++) {
            dio.out[i] = 1;
            dio.out[i + 8] = 0;
        }
        msleep(100);

        for (let i = 0; i < 8; i++) {
            if (toNum(dio.out[i]) !== 1 || toNum(dio.out[i + 8]) !== 0) {
                printToConsole("  ✗ Group operation 1 failed");
                allPass = false;
                break;
            }
        }

        // Test 2: Lower byte low, upper byte high
        printToConsole("  Setting lower byte LOW, upper byte HIGH");
        for (let i = 0; i < 8; i++) {
            dio.out[i] = 0;
            dio.out[i + 8] = 1;
        }
        msleep(100);

        for (let i = 0; i < 8; i++) {
            if (toNum(dio.out[i]) !== 0 || toNum(dio.out[i + 8]) !== 1) {
                printToConsole("  ✗ Group operation 2 failed");
                allPass = false;
                break;
            }
        }

        // Test 3: Nibble patterns
        printToConsole("  Setting nibble patterns");
        for (let i = 0; i < 4; i++) {
            dio.out[i] = 1;         // Nibble 0: HIGH
            dio.out[i + 4] = 0;     // Nibble 1: LOW
            dio.out[i + 8] = 1;     // Nibble 2: HIGH
            dio.out[i + 12] = 0;    // Nibble 3: LOW
        }
        msleep(100);

        for (let i = 0; i < 4; i++) {
            if (toNum(dio.out[i]) !== 1 || toNum(dio.out[i + 4]) !== 0 ||
                toNum(dio.out[i + 8]) !== 1 || toNum(dio.out[i + 12]) !== 0) {
                printToConsole("  ✗ Nibble pattern failed");
                allPass = false;
                break;
            }
        }

        if (allPass) {
            printToConsole("  ✓ All group operations successful");
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 9: Start/Stop Control
TestFramework.runTest("TST.DIO.START_STOP", function() {
    try {
        let allPass = true;

        // Test stopping
        printToConsole("  Stopping Digital I/O");
        dio.running = false;
        msleep(100);

        let running = dio.running;
        let pass1 = TestFramework.assertEqual(running, false, "DIO stopped");
        allPass = allPass && pass1;

        // Test starting
        printToConsole("  Starting Digital I/O");
        dio.running = true;
        msleep(100);

        running = dio.running;
        let pass2 = TestFramework.assertEqual(running, true, "DIO started");
        allPass = allPass && pass2;

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
exit(exitCode);