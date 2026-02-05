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

// Signal Generator Automated Tests (Single-Tool Only)
// Tests that only use the Signal Generator without requiring other tools
// Based on TST.M2K.SG.* test specifications

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: Signal Generator
TestFramework.init("Signal Generator Tests");

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to Signal Generator tool
if (!switchToTool("Signal Generator")) {
    printToConsole("ERROR: Cannot access Signal Generator");
    exit(1);
}

// Test 1: Frequency Range Test
TestFramework.runTest("TST.M2K.SG.FREQUENCY_RANGE", function() {
    try {
        siggen.mode[0] = 1; // Waveform mode
        siggen.waveform_type[0] = 0; // Sine
        siggen.waveform_amplitude[0] = 2;
        siggen.enabled[0] = true;
        siggen.running = true;

        // Test various frequencies
        let frequencies = [0.001, 1, 100, 1000, 100000, 1000000, 5000000];
        let allPass = true;

        for (let freq of frequencies) {
            if (freq > 1000000) {
                printToConsole("  Testing " + (freq/1000000) + " MHz");
            } else if (freq > 1000) {
                printToConsole("  Testing " + (freq/1000) + " kHz");
            } else {
                printToConsole("  Testing " + freq + " Hz");
            }

            siggen.waveform_frequency[0] = freq;
            msleep(100);

            // Just verify setting was accepted (no measurement for very low/high freq)
            let readback = siggen.waveform_frequency[0];
            let pass = TestFramework.assertEqual(readback, freq, "Frequency set correctly");
            allPass = allPass && pass;
        }

        siggen.running = false;
        siggen.enabled[0] = false;
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 2: Amplitude Range Test
TestFramework.runTest("TST.M2K.SG.AMPLITUDE_RANGE", function() {
    try {
        siggen.mode[0] = 1; // Waveform mode
        siggen.waveform_type[0] = 0; // Sine
        siggen.waveform_frequency[0] = 1000;
        siggen.enabled[0] = true;
        siggen.running = true;

        // Test various amplitudes
        let amplitudes = [0.001, 0.01, 0.1, 0.5, 1, 2, 5, 10];
        let allPass = true;

        for (let amp of amplitudes) {
            printToConsole("  Testing " + amp + " Vpp");
            siggen.waveform_amplitude[0] = amp;
            msleep(100);

            // Verify setting was accepted
            let readback = siggen.waveform_amplitude[0];
            let pass = TestFramework.assertEqual(readback, amp, "Amplitude set correctly");
            allPass = allPass && pass;
        }

        siggen.running = false;
        siggen.enabled[0] = false;
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 3: Phase and Offset Test
TestFramework.runTest("TST.M2K.SG.PHASE_OFFSET", function() {
    try {
        siggen.mode[0] = 1; // Waveform mode
        siggen.waveform_type[0] = 0; // Sine
        siggen.waveform_amplitude[0] = 2;
        siggen.waveform_frequency[0] = 1000;
        siggen.enabled[0] = true;
        siggen.running = true;

        let allPass = true;

        // Test phase settings
        let phases = [0, 45, 90, 180, 270, 360];
        for (let phase of phases) {
            siggen.waveform_phase[0] = phase;
            msleep(100);
            let readback = siggen.waveform_phase[0];
            // Handle 360° = 0° equivalence
            let pass;
            if (phase === 360 && readback === 0) {
                pass = true; // 360° and 0° are equivalent
                printToConsole("  ✓ Phase 360° (returned as 0°)");
            } else {
                pass = TestFramework.assertEqual(readback, phase, "Phase " + phase + "°");
            }
            allPass = allPass && pass;
        }

        // Test offset settings
        let offsets = [-2, -1, 0, 1, 2];
        for (let offset of offsets) {
            siggen.waveform_offset[0] = offset;
            msleep(100);
            let readback = siggen.waveform_offset[0];
            let pass = TestFramework.assertEqual(readback, offset, "Offset " + offset + "V");
            allPass = allPass && pass;
        }

        siggen.running = false;
        siggen.enabled[0] = false;
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