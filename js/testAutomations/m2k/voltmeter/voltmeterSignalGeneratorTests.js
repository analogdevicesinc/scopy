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

// Voltmeter + Signal Generator Integration Tests
// Tests that use Signal Generator as AC source for voltmeter measurements
// Based on TST.DMM.* test specifications

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: Voltmeter + Signal Generator Integration Tests
TestFramework.init("Voltmeter + Signal Generator Tests");

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to Voltmeter tool first (primary tool)
if (!switchToTool("Voltmeter")) {
    printToConsole("ERROR: Cannot access Voltmeter");
    exit(1);
}

// Test 1: AC Voltage Measurement - Low Frequency Mode
TestFramework.runTest("TST.DMM.AC_LOW_FREQ", function() {
    try {
        // Generate AC signal with Signal Generator
        switchToTool("Signal Generator");
        siggen.mode[0] = 1; // Waveform mode
        siggen.waveform_type[0] = 0; // Sine
        siggen.waveform_amplitude[0] = 3; // 3Vpp
        siggen.waveform_frequency[0] = 100; // 100Hz (in low freq range)
        siggen.waveform_offset[0] = 0;
        siggen.enabled[0] = true;
        siggen.running = true;

        // Switch to Voltmeter
        switchToTool("Voltmeter");
        dmm.mode_ac_low_ch1 = true; // AC mode 20Hz-800Hz
        dmm.mode_ac_high_ch1 = false;
        dmm.running = true;
        msleep(2000); // Allow AC measurement to stabilize

        let measured = dmm.value_ch1;

        // For sine wave, RMS = Vpp / (2 * sqrt(2))
        let expectedRms = 3 / (2 * Math.sqrt(2));
        let pass = TestFramework.assertApproxEqual(measured, expectedRms, 0.2,
            "AC RMS measurement (low freq)");

        dmm.running = false;
        switchToTool("Signal Generator");
        siggen.running = false;
        siggen.enabled[0] = false;

        return pass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 2: AC Voltage Measurement - High Frequency Mode
TestFramework.runTest("TST.DMM.AC_HIGH_FREQ", function() {
    try {
        // Generate AC signal with Signal Generator
        switchToTool("Signal Generator");
        siggen.mode[0] = 1; // Waveform mode
        siggen.waveform_type[0] = 0; // Sine
        siggen.waveform_amplitude[0] = 2; // 2Vpp
        siggen.waveform_frequency[0] = 5000; // 5kHz (in high freq range)
        siggen.waveform_offset[0] = 0;
        siggen.enabled[0] = true;
        siggen.running = true;

        // Switch to Voltmeter
        switchToTool("Voltmeter");
        dmm.mode_ac_low_ch1 = false;
        dmm.mode_ac_high_ch1 = true; // AC mode 800Hz-40kHz
        dmm.running = true;
        msleep(2000); // Allow AC measurement to stabilize

        let measured = dmm.value_ch1;

        // For sine wave, RMS = Vpp / (2 * sqrt(2))
        let expectedRms = 2 / (2 * Math.sqrt(2));
        let pass = TestFramework.assertApproxEqual(measured, expectedRms, 0.2,
            "AC RMS measurement (high freq)");

        dmm.running = false;
        switchToTool("Signal Generator");
        siggen.running = false;
        siggen.enabled[0] = false;

        return pass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test 3: Histogram Feature
TestFramework.runTest("TST.DMM.HISTOGRAM", function() {
    try {
        // Generate varying signal
        switchToTool("Signal Generator");
        siggen.mode[0] = 1;
        siggen.waveform_type[0] = 2; // Triangle
        siggen.waveform_amplitude[0] = 4;
        siggen.waveform_frequency[0] = 10; // Slow for histogram
        siggen.enabled[0] = true;
        siggen.running = true;

        // Enable histogram
        switchToTool("Voltmeter");
        dmm.histogram_ch1 = true;
        dmm.running = true;
        msleep(2000);

        // Verify histogram is enabled
        let histogramEnabled = dmm.histogram_ch1;
        let pass1 = TestFramework.assertEqual(histogramEnabled, true,
            "Histogram enabled");

        // Disable histogram
        dmm.histogram_ch1 = false;
        msleep(500);

        histogramEnabled = dmm.histogram_ch1;
        let pass2 = TestFramework.assertEqual(histogramEnabled, false,
            "Histogram disabled");

        dmm.running = false;
        switchToTool("Signal Generator");
        siggen.running = false;
        siggen.enabled[0] = false;

        return pass1 && pass2;

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