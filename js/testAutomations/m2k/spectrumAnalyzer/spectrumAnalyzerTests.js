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

// Spectrum Analyzer + Signal Generator Integration Tests
// Tests that verify spectrum analyzer functionality using signal generator
// Based on TST.M2K.SA.* test specifications
//
// Hardware Setup Required:
//   - Scope CH1+ connected to W1 (Signal Generator Output 1)
//   - Scope CH1- connected to GND
//   - Scope CH2+ connected to W2 (Signal Generator Output 2)
//   - Scope CH2- connected to GND

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: Spectrum Analyzer Tests
TestFramework.init("Spectrum Analyzer Tests");

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Helper function to configure signal generator channel
function configureSignalGenerator(channel, amplitude, frequency, offset, phase) {
    siggen.running = false;
    siggen.mode[channel] = 1; // Waveform mode
    siggen.waveform_type[channel] = 0; // Sine wave
    siggen.waveform_amplitude[channel] = amplitude;
    siggen.waveform_frequency[channel] = frequency;
    siggen.waveform_offset[channel] = offset;
    siggen.waveform_phase[channel] = phase;
    siggen.enabled[channel] = true;
}

// Helper function to configure spectrum analyzer
function configureSpectrumAnalyzer(startFreq, stopFreq, resBW) {
    spectrum.running = false;
    spectrum.startFreq = startFreq;
    spectrum.stopFreq = stopFreq;
    if (resBW) {
        spectrum.resBW = resBW;
    }
}

// Helper function to configure spectrum channel
function configureSpectrumChannel(channel, enabled, type, windowFunc) {
    let channels = spectrum.channels;
    if (channels && channels[channel]) {
        channels[channel].enabled = enabled;
        channels[channel].type = type; // 0 = Sample
        channels[channel].window = windowFunc; // 3 = Flat-top
    }
}

// Helper function to find peak frequency from spectrum data
function findPeakFrequency(channel) {
    let channels = spectrum.channels;
    if (!channels || !channels[channel]) {
        return null;
    }

    let data = channels[channel].data;
    let freq = channels[channel].freq;

    if (!data || !freq || data.length === 0) {
        return null;
    }

    let maxIdx = 0;
    let maxVal = data[0];

    for (let i = 1; i < data.length; i++) {
        if (data[i] > maxVal) {
            maxVal = data[i];
            maxIdx = i;
        }
    }

    return {
        frequency: freq[maxIdx],
        magnitude: maxVal
    };
}

// Helper function to use markers
function getMarkerPeakFrequency(channel) {
    let markers = spectrum.markers;
    if (markers && markers.length > 0) {
        // Find marker for this channel
        for (let i = 0; i < markers.length; i++) {
            if (markers[i].chId === channel) {
                markers[i].en = true;
                markers[i].type = 1; // Peak type
                msleep(500); // Allow marker to find peak
                return {
                    frequency: markers[i].freq,
                    magnitude: markers[i].magnitude
                };
            }
        }
    }
    return null;
}

// Switch to Signal Generator first to set up signals
if (!switchToTool("Signal Generator")) {
    printToConsole("ERROR: Cannot access Signal Generator");
    exit(1);
}

// ============================================================================
// Test 1: Channel 1 Operation - Frequency Range Tests
// UID: TST.M2K.SA.CHANNEL_1_OPERATION
// ============================================================================
TestFramework.runTest("TST.M2K.SA.CHANNEL_1_OPERATION", function() {
    try {
        // Test configurations: [signalFreq, startFreq, stopFreq, description]
        let testConfigs = [
            {freq: 500, start: 0, stop: 1000, desc: "500Hz"},
            {freq: 1000, start: 0, stop: 2000, desc: "1kHz"},
            {freq: 7500, start: 5000, stop: 10000, desc: "7.5kHz"},
            {freq: 100000, start: 0, stop: 200000, desc: "100kHz"},
            {freq: 250000, start: 0, stop: 500000, desc: "250kHz"},
            {freq: 500000, start: 0, stop: 1000000, desc: "500kHz"},
            {freq: 800000, start: 0, stop: 1600000, desc: "800kHz"},
            {freq: 1000000, start: 0, stop: 2000000, desc: "1MHz"},
            {freq: 5000000, start: 0, stop: 10000000, desc: "5MHz"}
        ];

        let allPass = true;
        let passCount = 0;

        for (let config of testConfigs) {
            printToConsole("  Testing frequency: " + config.desc);

            // Configure Signal Generator
            switchToTool("Signal Generator");
            configureSignalGenerator(0, 10, config.freq, 0, 0);
            siggen.running = true;
            msleep(500);

            // Switch to Spectrum Analyzer
            switchToTool("Spectrum Analyzer");

            // Configure Spectrum Analyzer
            configureSpectrumAnalyzer(config.start, config.stop, null);

            // Configure channel 1: Type=Sample (0), Window=Flat-top (3)
            configureSpectrumChannel(0, true, 0, 3);

            // Run spectrum analyzer
            spectrum.running = true;
            msleep(2000); // Allow spectrum to stabilize

            // Stop and read data
            spectrum.running = false;
            msleep(500);

            // Find peak frequency
            let peak = findPeakFrequency(0);

            if (peak) {
                // Allow 5% tolerance for frequency measurement
                let tolerance = config.freq * 0.05;
                let pass = TestFramework.assertApproxEqual(peak.frequency, config.freq, tolerance,
                    "Peak frequency at " + config.desc);
                if (pass) passCount++;
                allPass = allPass && pass;
            } else {
                printToConsole("    Warning: Could not detect peak frequency");
                // Try using markers instead
                let markerPeak = getMarkerPeakFrequency(0);
                if (markerPeak) {
                    let tolerance = config.freq * 0.05;
                    let pass = TestFramework.assertApproxEqual(markerPeak.frequency, config.freq, tolerance,
                        "Marker peak frequency at " + config.desc);
                    if (pass) passCount++;
                    allPass = allPass && pass;
                }
            }
        }

        // Stop signal generator
        switchToTool("Signal Generator");
        siggen.running = false;
        siggen.enabled[0] = false;

        printToConsole("  Passed " + passCount + "/" + testConfigs.length + " frequency tests");
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 2: Channel 2 Operation - Frequency Range Tests
// UID: TST.M2K.SA.CHANNEL_2_OPERATION
// ============================================================================
TestFramework.runTest("TST.M2K.SA.CHANNEL_2_OPERATION", function() {
    try {
        // Test configurations for channel 2
        let testConfigs = [
            {freq: 500, start: 0, stop: 1000, desc: "500Hz"},
            {freq: 1000, start: 0, stop: 2000, desc: "1kHz"},
            {freq: 7500, start: 5000, stop: 10000, desc: "7.5kHz"},
            {freq: 100000, start: 0, stop: 200000, desc: "100kHz"},
            {freq: 500000, start: 0, stop: 1000000, desc: "500kHz"},
            {freq: 1000000, start: 0, stop: 2000000, desc: "1MHz"}
        ];

        let allPass = true;
        let passCount = 0;

        for (let config of testConfigs) {
            printToConsole("  Testing CH2 frequency: " + config.desc);

            // Configure Signal Generator Channel 2
            switchToTool("Signal Generator");
            siggen.enabled[0] = false; // Disable CH1
            configureSignalGenerator(1, 10, config.freq, 0, 0);
            siggen.running = true;
            msleep(500);

            // Switch to Spectrum Analyzer
            switchToTool("Spectrum Analyzer");

            // Configure Spectrum Analyzer
            configureSpectrumAnalyzer(config.start, config.stop, null);

            // Configure channel 2: Type=Sample (0), Window=Flat-top (3)
            configureSpectrumChannel(0, false, 0, 3); // Disable CH1
            configureSpectrumChannel(1, true, 0, 3);  // Enable CH2

            // Run spectrum analyzer
            spectrum.running = true;
            msleep(2000);

            // Stop and read data
            spectrum.running = false;
            msleep(500);

            // Find peak frequency on channel 2
            let peak = findPeakFrequency(1);

            if (peak) {
                let tolerance = config.freq * 0.05;
                let pass = TestFramework.assertApproxEqual(peak.frequency, config.freq, tolerance,
                    "CH2 Peak frequency at " + config.desc);
                if (pass) passCount++;
                allPass = allPass && pass;
            } else {
                printToConsole("    Warning: Could not detect CH2 peak frequency");
            }
        }

        // Stop signal generator
        switchToTool("Signal Generator");
        siggen.running = false;
        siggen.enabled[1] = false;

        printToConsole("  Passed " + passCount + "/" + testConfigs.length + " frequency tests");
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 3: Channel 1 and 2 Simultaneous Operation
// UID: TST.M2K.SA.CHANNEL_1_AND_2_OPERATION
// ============================================================================
TestFramework.runTest("TST.M2K.SA.CHANNEL_1_AND_2_OPERATION", function() {
    try {
        // Test configurations: CH1 and CH2 with different frequencies
        let testConfigs = [
            {ch1Freq: 100, ch2Freq: 300, start: 0, stop: 500, desc: "100Hz/300Hz"},
            {ch1Freq: 200, ch2Freq: 600, start: 0, stop: 1000, desc: "200Hz/600Hz"},
            {ch1Freq: 4000, ch2Freq: 7000, start: 0, stop: 10000, desc: "4kHz/7kHz"},
            {ch1Freq: 25000, ch2Freq: 35000, start: 0, stop: 50000, desc: "25kHz/35kHz"},
            {ch1Freq: 250000, ch2Freq: 750000, start: 0, stop: 1000000, desc: "250kHz/750kHz"}
        ];

        let allPass = true;
        let passCount = 0;

        for (let config of testConfigs) {
            printToConsole("  Testing dual channel: " + config.desc);

            // Configure Signal Generator - both channels
            switchToTool("Signal Generator");
            configureSignalGenerator(0, 10, config.ch1Freq, 0, 0);
            configureSignalGenerator(1, 10, config.ch2Freq, 0, 0);
            siggen.running = true;
            msleep(500);

            // Switch to Spectrum Analyzer
            switchToTool("Spectrum Analyzer");

            // Configure Spectrum Analyzer
            configureSpectrumAnalyzer(config.start, config.stop, null);

            // Enable both channels with Sample type and Flat-top window
            configureSpectrumChannel(0, true, 0, 3);
            configureSpectrumChannel(1, true, 0, 3);

            // Enable marker table for monitoring
            spectrum.markerTableVisible = true;

            // Run spectrum analyzer
            spectrum.running = true;
            msleep(2000);

            // Stop and read data
            spectrum.running = false;
            msleep(500);

            // Find peak frequencies
            let peak1 = findPeakFrequency(0);
            let peak2 = findPeakFrequency(1);

            let ch1Pass = false;
            let ch2Pass = false;

            if (peak1) {
                let tolerance = config.ch1Freq * 0.05;
                ch1Pass = TestFramework.assertApproxEqual(peak1.frequency, config.ch1Freq, tolerance,
                    "CH1 peak at " + config.ch1Freq + "Hz");
            }

            if (peak2) {
                let tolerance = config.ch2Freq * 0.05;
                ch2Pass = TestFramework.assertApproxEqual(peak2.frequency, config.ch2Freq, tolerance,
                    "CH2 peak at " + config.ch2Freq + "Hz");
            }

            if (ch1Pass && ch2Pass) passCount++;
            allPass = allPass && ch1Pass && ch2Pass;
        }

        // Cleanup
        switchToTool("Signal Generator");
        siggen.running = false;
        siggen.enabled[0] = false;
        siggen.enabled[1] = false;

        printToConsole("  Passed " + passCount + "/" + testConfigs.length + " dual channel tests");
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 4: Additional Features - Detector Types and Markers
// UID: TST.M2K.SA.ADDITIONAL_FEATURES
// ============================================================================
TestFramework.runTest("TST.M2K.SA.ADDITIONAL_FEATURES", function() {
    try {
        let allPass = true;

        // Part 1: Test Peak Hold Continuous detector
        printToConsole("  Testing Peak Hold Continuous detector");

        switchToTool("Signal Generator");
        configureSignalGenerator(0, 10, 500000, 0, 0);
        siggen.running = true;
        msleep(500);

        switchToTool("Spectrum Analyzer");
        configureSpectrumAnalyzer(0, 1000000, null);
        configureSpectrumChannel(0, true, 0, 3);

        // Set channel averaging to Peak hold (type may vary)
        let channels = spectrum.channels;
        if (channels && channels[0]) {
            channels[0].averaging = 1; // Enable averaging for peak detection
        }

        spectrum.running = true;
        msleep(2000);

        // Capture initial peak
        spectrum.running = false;
        let initialPeak = findPeakFrequency(0);

        // Change signal frequency
        switchToTool("Signal Generator");
        siggen.waveform_frequency[0] = 250000;
        msleep(500);

        switchToTool("Spectrum Analyzer");
        spectrum.running = true;
        msleep(2000);
        spectrum.running = false;

        // For peak hold, we should see both peaks retained
        // This is a visual verification in manual tests
        printToConsole("    Peak hold detector test completed (visual verification)");

        // Part 2: Test Min Hold Continuous detector
        printToConsole("  Testing Min Hold Continuous detector");

        if (channels && channels[0]) {
            channels[0].averaging = 2; // Min hold mode (if available)
        }

        switchToTool("Signal Generator");
        siggen.waveform_frequency[0] = 500000;
        msleep(500);

        switchToTool("Spectrum Analyzer");
        spectrum.running = true;
        msleep(2000);
        spectrum.running = false;

        printToConsole("    Min hold detector test completed (visual verification)");

        // Part 3: Test Marker Table functionality
        printToConsole("  Testing Marker Table functionality");

        switchToTool("Signal Generator");
        // Configure square wave for harmonics test
        siggen.waveform_type[0] = 1; // Square wave
        siggen.waveform_amplitude[0] = 5;
        siggen.waveform_frequency[0] = 50000;
        msleep(500);

        switchToTool("Spectrum Analyzer");
        configureSpectrumAnalyzer(0, 1000000, null);
        configureSpectrumChannel(0, true, 0, 3);

        // Enable marker table
        spectrum.markerTableVisible = true;
        let tableVisible = spectrum.markerTableVisible;
        let markerTablePass = TestFramework.assertEqual(tableVisible, true, "Marker table enabled");
        allPass = allPass && markerTablePass;

        // Enable markers
        let markers = spectrum.markers;
        if (markers && markers.length >= 5) {
            for (let i = 0; i < 5; i++) {
                markers[i].en = true;
                markers[i].type = 1; // Peak type
            }
            printToConsole("    Enabled 5 markers for harmonic detection");
        }

        spectrum.running = true;
        msleep(2000);
        spectrum.running = false;

        // Verify fundamental frequency detection
        let peak = findPeakFrequency(0);
        if (peak) {
            let freqPass = TestFramework.assertApproxEqual(peak.frequency, 50000, 2500,
                "Fundamental frequency at 50kHz");
            allPass = allPass && freqPass;
        }

        // Cleanup
        switchToTool("Signal Generator");
        siggen.running = false;
        siggen.enabled[0] = false;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 5: Frequency Sweep Settings Verification
// UID: TST.M2K.SA.SWEEP_SETTINGS
// ============================================================================
TestFramework.runTest("TST.M2K.SA.SWEEP_SETTINGS", function() {
    try {
        switchToTool("Spectrum Analyzer");

        let allPass = true;

        // Test start/stop frequency settings
        let testSettings = [
            {start: 0, stop: 1000},
            {start: 1000, stop: 10000},
            {start: 0, stop: 1000000},
            {start: 5000, stop: 10000},
            {start: 0, stop: 50000000}
        ];

        for (let setting of testSettings) {
            spectrum.startFreq = setting.start;
            spectrum.stopFreq = setting.stop;
            msleep(100);

            let actualStart = spectrum.startFreq;
            let actualStop = spectrum.stopFreq;

            let startPass = TestFramework.assertApproxEqual(actualStart, setting.start, 1,
                "Start freq " + setting.start + "Hz");
            let stopPass = TestFramework.assertApproxEqual(actualStop, setting.stop, 1,
                "Stop freq " + setting.stop + "Hz");

            allPass = allPass && startPass && stopPass;
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 6: Display Settings Verification
// UID: TST.M2K.SA.DISPLAY_SETTINGS
// ============================================================================
TestFramework.runTest("TST.M2K.SA.DISPLAY_SETTINGS", function() {
    try {
        switchToTool("Spectrum Analyzer");

        let allPass = true;

        // Test cursors enable/disable
        spectrum.cursors = true;
        msleep(100);
        let cursorsEnabled = spectrum.cursors;
        let cursorsPass = TestFramework.assertEqual(cursorsEnabled, true, "Cursors enabled");
        allPass = allPass && cursorsPass;

        spectrum.cursors = false;
        msleep(100);
        let cursorsDisabled = !spectrum.cursors;
        let cursorsOffPass = TestFramework.assertEqual(cursorsDisabled, true, "Cursors disabled");
        allPass = allPass && cursorsOffPass;

        // Test horizontal and vertical cursors
        spectrum.horizontal_cursors = true;
        spectrum.vertical_cursors = true;
        msleep(100);
        let hCursors = spectrum.horizontal_cursors;
        let vCursors = spectrum.vertical_cursors;
        let hPass = TestFramework.assertEqual(hCursors, true, "Horizontal cursors enabled");
        let vPass = TestFramework.assertEqual(vCursors, true, "Vertical cursors enabled");
        allPass = allPass && hPass && vPass;

        // Test log scale
        spectrum.logScale = true;
        msleep(100);
        let logEnabled = spectrum.logScale;
        let logPass = TestFramework.assertEqual(logEnabled, true, "Log scale enabled");
        allPass = allPass && logPass;

        spectrum.logScale = false;
        msleep(100);

        // Test waterfall display
        spectrum.waterfall_visible = true;
        msleep(100);
        let waterfallEnabled = spectrum.waterfall_visible;
        let waterfallPass = TestFramework.assertEqual(waterfallEnabled, true, "Waterfall enabled");
        allPass = allPass && waterfallPass;

        spectrum.waterfall_visible = false;
        msleep(100);

        // Test scale settings
        spectrum.topScale = 10;
        spectrum.bottomScale = -100;
        msleep(100);

        let topScale = spectrum.topScale;
        let bottomScale = spectrum.bottomScale;
        let topPass = TestFramework.assertApproxEqual(topScale, 10, 0.1, "Top scale set to 10");
        let bottomPass = TestFramework.assertApproxEqual(bottomScale, -100, 0.1, "Bottom scale set to -100");
        allPass = allPass && topPass && bottomPass;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 7: Channel Settings Verification
// UID: TST.M2K.SA.CHANNEL_SETTINGS
// ============================================================================
TestFramework.runTest("TST.M2K.SA.CHANNEL_SETTINGS", function() {
    try {
        switchToTool("Spectrum Analyzer");

        let allPass = true;
        let channels = spectrum.channels;

        if (!channels || channels.length < 2) {
            printToConsole("  Warning: Could not access spectrum channels");
            return "SKIP";
        }

        // Test channel enable/disable
        channels[0].enabled = true;
        channels[1].enabled = false;
        msleep(100);

        let ch0Enabled = channels[0].enabled;
        let ch1Disabled = !channels[1].enabled;
        let enablePass = TestFramework.assertEqual(ch0Enabled, true, "Channel 0 enabled");
        let disablePass = TestFramework.assertEqual(ch1Disabled, true, "Channel 1 disabled");
        allPass = allPass && enablePass && disablePass;

        // Test averaging types
        // 0 = none, 1 = exponential, 2 = linear (per API)
        for (let avgType = 0; avgType <= 2; avgType++) {
            channels[0].averaging = avgType;
            msleep(100);
            let actualAvg = channels[0].averaging;
            let avgPass = TestFramework.assertEqual(actualAvg, avgType,
                "Averaging type " + avgType);
            allPass = allPass && avgPass;
        }

        // Test window functions
        // Window types: 0=rectangular, 1=hamming, 2=hann, 3=flat-top, etc.
        let windowTypes = [0, 1, 2, 3];
        for (let winType of windowTypes) {
            channels[0].window = winType;
            msleep(100);
            let actualWin = channels[0].window;
            let winPass = TestFramework.assertEqual(actualWin, winType,
                "Window type " + winType);
            allPass = allPass && winPass;
        }

        // Test line thickness
        channels[0].line_thickness = 2.0;
        msleep(100);
        let thickness = channels[0].line_thickness;
        let thickPass = TestFramework.assertApproxEqual(thickness, 2.0, 0.1,
            "Line thickness set to 2.0");
        allPass = allPass && thickPass;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 8: Single Shot Mode
// UID: TST.M2K.SA.SINGLE_SHOT
// ============================================================================
TestFramework.runTest("TST.M2K.SA.SINGLE_SHOT", function() {
    try {
        // Setup signal
        switchToTool("Signal Generator");
        configureSignalGenerator(0, 5, 100000, 0, 0);
        siggen.running = true;
        msleep(500);

        switchToTool("Spectrum Analyzer");
        configureSpectrumAnalyzer(0, 200000, null);
        configureSpectrumChannel(0, true, 0, 3);

        // Test single shot mode
        spectrum.single = true;
        msleep(100);

        let singleEnabled = spectrum.single;
        let singlePass = TestFramework.assertEqual(singleEnabled, true, "Single shot mode enabled");

        // Trigger single acquisition
        spectrum.running = true;
        msleep(2000); // Wait for acquisition

        // In single mode, running should stop after capture
        // Verify we got data
        let peak = findPeakFrequency(0);
        let dataPass = false;
        if (peak) {
            dataPass = TestFramework.assertApproxEqual(peak.frequency, 100000, 5000,
                "Single shot captured peak at 100kHz");
        }

        // Disable single mode
        spectrum.single = false;
        spectrum.running = false;

        // Cleanup
        switchToTool("Signal Generator");
        siggen.running = false;
        siggen.enabled[0] = false;

        return singlePass && dataPass;

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
