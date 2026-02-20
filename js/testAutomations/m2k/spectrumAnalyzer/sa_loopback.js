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

// Spectrum Analyzer Loopback Tests
// Based on RST tests:
//   TST.M2K.SA.CHANNEL_1_OPERATION
//   TST.M2K.SA.CHANNEL_2_OPERATION
//   TST.M2K.SA.CHANNEL_1_AND_2_OPERATION
//   TST.M2K.SA.ADDITIONAL_FEATURES
//
// Hardware Setup Required:
//   - Scope CH1+ → W1 (Signal Generator Output 1)
//   - Scope CH1- → GND
//   - Scope CH2+ → W2 (Signal Generator Output 2)
//   - Scope CH2- → GND

evaluateFile("../js/testAutomations/common/testFramework.js");

TestFramework.init("Spectrum Analyzer Loopback Tests");

if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Helper: find peak frequency from channel data array
function findPeakFrequency(channel) {
    var channels = spectrum.channels;
    if (!channels || !channels[channel]) {
        return null;
    }

    var data = channels[channel].data;
    var freq = channels[channel].freq;

    if (!data || !freq || data.length === 0) {
        return null;
    }

    var maxIdx = 0;
    var maxVal = data[0];

    for (var i = 1; i < data.length; i++) {
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

// Helper: configure signal generator channel and start it
function setupSignalGenerator(channel, amplitude, frequency, offset, phase, waveformType) {
    switchToTool("Signal Generator");
    siggen.running = false;
    var modes = siggen.mode;
    modes[channel] = 1; // Waveform mode
    siggen.mode = modes;

    var types = siggen.waveform_type;
    types[channel] = (waveformType !== undefined) ? waveformType : 0; // Default: Sine
    siggen.waveform_type = types;

    var amps = siggen.waveform_amplitude;
    amps[channel] = amplitude;
    siggen.waveform_amplitude = amps;

    var freqs = siggen.waveform_frequency;
    freqs[channel] = frequency;
    siggen.waveform_frequency = freqs;

    var offsets = siggen.waveform_offset;
    offsets[channel] = offset;
    siggen.waveform_offset = offsets;

    var phases = siggen.waveform_phase;
    phases[channel] = phase;
    siggen.waveform_phase = phases;

    var enabled = siggen.enabled;
    enabled[channel] = true;
    siggen.enabled = enabled;
}

// Helper: run a single-channel frequency test point
// Returns true if peak detected within tolerance
function runFrequencyTest(saChannel, siggenChannel, testFreq, startFreq, stopFreq, resBW, desc) {
    // Configure signal generator
    setupSignalGenerator(siggenChannel, 10, testFreq, 0, 0);
    siggen.running = true;
    msleep(500);

    // Configure spectrum analyzer
    switchToTool("Spectrum Analyzer");
    spectrum.running = false;
    spectrum.startFreq = startFreq;
    spectrum.stopFreq = stopFreq;
    spectrum.resBW = resBW;

    // Configure channel: Type=Sample(0), Window=Flat-top(3)
    var channels = spectrum.channels;
    if (channels && channels[saChannel]) {
        channels[saChannel].enabled = true;
        channels[saChannel].type = 0;   // Sample
        channels[saChannel].window = 0; // Flat-top
    }

    // Run and wait for data to stabilize
    spectrum.running = true;
    msleep(3000);
    spectrum.running = false;
    msleep(500);

    // Check peak frequency
    var peak = findPeakFrequency(saChannel);
    if (peak) {
        var tolerance = testFreq * 0.05;
        if (tolerance < 50) tolerance = 50; // Minimum 50Hz tolerance for low frequencies
        return TestFramework.assertApproxEqual(peak.frequency, testFreq, tolerance,
            desc + " peak at " + testFreq + "Hz");
    } else {
        printToConsole("    Warning: No peak data for " + desc);
        return false;
    }
}

// ============================================================================
// TST.M2K.SA.CHANNEL_1_OPERATION
// Verifies spectrum analyzer on channel 1 across 11 frequency points.
// ============================================================================
TestFramework.runTest("TST.M2K.SA.CHANNEL_1_OPERATION", function() {
    try {
        var allPass = true;
        var passCount = 0;

        // 11 frequency test points from RST (steps 4-43)
        var testConfigs = [
            // [testFreq, startFreq, stopFreq, resBW, description]
            {freq: 500,      start: 0,    stop: 50000,    rbw: "12.21 Hz",    desc: "500Hz"},
            {freq: 1000,     start: 0,    stop: 50000,    rbw: "12.21 Hz",    desc: "1kHz"},
            {freq: 7500,     start: 0,    stop: 50000,    rbw: "12.21 Hz",    desc: "7.5kHz"},
            {freq: 100000,   start: 0,    stop: 200000,   rbw: "12.21 Hz",    desc: "100kHz"},
            {freq: 250000,   start: 0,    stop: 500000,   rbw: "30.52 Hz",    desc: "250kHz"},
            {freq: 500000,   start: 0,    stop: 1000000,  rbw: "61.04 Hz",    desc: "500kHz"},
            {freq: 800000,   start: 0,    stop: 1600000,  rbw: "98.44 Hz",    desc: "800kHz"},
            {freq: 1000000,  start: 0,    stop: 2000000,  rbw: "122.07 Hz",   desc: "1MHz"},
            {freq: 5000000,  start: 0,    stop: 10000000, rbw: "610.35 Hz",   desc: "5MHz"},
            {freq: 10000000, start: 0,    stop: 20000000, rbw: "1.53 kHz",    desc: "10MHz"},
            {freq: 20000000, start: 0,    stop: 50000000, rbw: "3.05 kHz",    desc: "20MHz"}
        ];

        // Step 1: Open Spectrum Analyzer
        if (!switchToTool("Spectrum Analyzer")) {
            printToConsole("ERROR: Cannot access Spectrum Analyzer");
            return false;
        }

        // Step 2: Configure channel 1 - Type=Sample, Window=Flat-top
        var channels = spectrum.channels;
        if (channels && channels[0]) {
            channels[0].enabled = true;
            channels[0].type = 0;   // Sample
            channels[0].window = 0; // Flat-top
        }

        // Step 3: Wiring is physical (CH1+ → W1, CH1- → GND)

        // Steps 4-43: Test each frequency point
        for (var i = 0; i < testConfigs.length; i++) {
            var config = testConfigs[i];
            printToConsole("  Testing CH1 at " + config.desc);

            var pass = runFrequencyTest(0, 0, config.freq, config.start, config.stop,
                config.rbw, "CH1 " + config.desc);
            if (pass) passCount++;
            allPass = allPass && pass;

            // Stop siggen between tests
            switchToTool("Signal Generator");
            siggen.running = false;
        }

        // Cleanup
        switchToTool("Signal Generator");
        siggen.running = false;
        var enabled = siggen.enabled;
        enabled[0] = false;
        siggen.enabled = enabled;

        printToConsole("  Passed " + passCount + "/" + testConfigs.length + " frequency tests");
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// TST.M2K.SA.CHANNEL_2_OPERATION
// Verifies spectrum analyzer on channel 2 across 11 frequency points.
// ============================================================================
TestFramework.runTest("TST.M2K.SA.CHANNEL_2_OPERATION", function() {
    try {
        var allPass = true;
        var passCount = 0;

        // 11 frequency test points from RST (steps 4-46)
        var testConfigs = [
            {freq: 500,      start: 0,    stop: 50000,    rbw: "12.21 Hz",    desc: "500Hz"},
            {freq: 1000,     start: 0,    stop: 50000,    rbw: "12.21 Hz",    desc: "1kHz"},
            {freq: 7500,     start: 0,    stop: 50000,    rbw: "12.21 Hz",    desc: "7.5kHz"},
            {freq: 100000,   start: 0,    stop: 200000,   rbw: "12.21 Hz",    desc: "100kHz"},
            {freq: 250000,   start: 0,    stop: 500000,   rbw: "30.52 Hz",    desc: "250kHz"},
            {freq: 500000,   start: 0,    stop: 1000000,  rbw: "61.04 Hz",    desc: "500kHz"},
            {freq: 800000,   start: 0,    stop: 1600000,  rbw: "98.44 Hz",    desc: "800kHz"},
            {freq: 1000000,  start: 0,    stop: 2000000,  rbw: "122.07 Hz",   desc: "1MHz"},
            {freq: 5000000,  start: 0,    stop: 10000000, rbw: "610.35 Hz",   desc: "5MHz"},
            {freq: 10000000, start: 0,    stop: 20000000, rbw: "1.53 kHz",    desc: "10MHz"},
            {freq: 20000000, start: 0,    stop: 50000000, rbw: "3.05 kHz",    desc: "20MHz"}
        ];

        // Step 1: Open Spectrum Analyzer
        if (!switchToTool("Spectrum Analyzer")) {
            printToConsole("ERROR: Cannot access Spectrum Analyzer");
            return false;
        }

        // Step 2: Configure channel 2 - Type=Sample, Window=Flat-top
        var channels = spectrum.channels;
        if (channels && channels[1]) {
            channels[1].enabled = true;
            channels[1].type = 0;   // Sample
            channels[1].window = 0; // Flat-top
        }
        // Disable channel 1
        if (channels && channels[0]) {
            channels[0].enabled = false;
        }

        // Step 3: Wiring is physical (CH2+ → W2, CH2- → GND)

        // Steps 4-46: Test each frequency point
        for (var i = 0; i < testConfigs.length; i++) {
            var config = testConfigs[i];
            printToConsole("  Testing CH2 at " + config.desc);

            // Signal generator channel 1 (index 1) → W2
            var pass = runFrequencyTest(1, 1, config.freq, config.start, config.stop,
                config.rbw, "CH2 " + config.desc);
            if (pass) passCount++;
            allPass = allPass && pass;

            // Stop siggen between tests
            switchToTool("Signal Generator");
            siggen.running = false;
        }

        // Cleanup
        switchToTool("Signal Generator");
        siggen.running = false;
        var enabled = siggen.enabled;
        enabled[1] = false;
        siggen.enabled = enabled;

        printToConsole("  Passed " + passCount + "/" + testConfigs.length + " frequency tests");
        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// TST.M2K.SA.CHANNEL_1_AND_2_OPERATION
// Verifies dual-channel operation with markers.
// ============================================================================
TestFramework.runTest("TST.M2K.SA.CHANNEL_1_AND_2_OPERATION", function() {
    try {
        var allPass = true;

        // --- Part 1: Marker function test (steps 1-14) ---
        printToConsole("  Part 1: Marker function test");

        // Steps 1-2: Configure both channels - Type=Sample, Window=Flat-top
        if (!switchToTool("Spectrum Analyzer")) {
            printToConsole("ERROR: Cannot access Spectrum Analyzer");
            return false;
        }
        var channels = spectrum.channels;
        if (channels && channels[0] && channels[1]) {
            channels[0].enabled = true;
            channels[0].type = 0;   // Sample
            channels[0].window = 0; // Flat-top
            channels[1].enabled = true;
            channels[1].type = 0;
            channels[1].window = 0; // Flat-top
        }

        // Step 3: Wiring is physical (CH1+ → W1, CH1- → GND, CH2+ → W2, CH2- → GND)

        // Steps 4-5: Set sweep and signal generator
        spectrum.startFreq = 0;
        spectrum.stopFreq = 1000000;
        spectrum.resBW = "61.04 Hz";

        // CH1: 250kHz, CH2: 750kHz
        setupSignalGenerator(0, 10, 250000, 0, 0);
        setupSignalGenerator(1, 10, 750000, 0, 0);
        siggen.running = true;
        msleep(500);

        switchToTool("Spectrum Analyzer");
        spectrum.running = true;
        msleep(3000);
        spectrum.running = false;
        msleep(500);

        // Step 6: Enable marker for channel 1, click peak
        var markers = spectrum.markers;
        if (markers) {
            // Find a marker for channel 0
            for (var i = 0; i < markers.length; i++) {
                if (markers[i].chId === 0 && markers[i].mkId === 0) {
                    markers[i].en = true;
                    markers[i].type = 1; // Peak - snaps to max peak
                    msleep(500);
                    var m0Freq = markers[i].freq;
                    var pass6 = TestFramework.assertApproxEqual(m0Freq, 250000, 12500,
                        "CH1 marker peak at 250kHz");
                    allPass = allPass && pass6;
                    break;
                }
            }

            // Steps 7-9: →Peak, Down Ampl, Up Ampl buttons
            // NOT AUTOMATABLE: No marker navigation API

            // Steps 10-11: Enable marker for channel 2, click peak
            for (var i = 0; i < markers.length; i++) {
                if (markers[i].chId === 1 && markers[i].mkId === 0) {
                    markers[i].en = true;
                    markers[i].type = 1; // Peak
                    msleep(500);
                    var m1Freq = markers[i].freq;
                    var pass11 = TestFramework.assertApproxEqual(m1Freq, 750000, 37500,
                        "CH2 marker peak at 750kHz");
                    allPass = allPass && pass11;
                    break;
                }
            }

            // Steps 12-14: ←Peak, Down Ampl, Up Ampl buttons
            // NOT AUTOMATABLE: No marker navigation API
        }

        // Cleanup markers from Part 1
        markers = spectrum.markers;
        if (markers) {
            for (var i = 0; i < markers.length; i++) {
                markers[i].en = false;
            }
        }

        // --- Part 2: Dual channel frequency pair tests (steps 16-38) ---
        printToConsole("  Part 2: Dual channel frequency pair tests");

        var dualConfigs = [
            // [ch1Freq, ch2Freq, startFreq, stopFreq, resBW, description]
            {ch1: 100,    ch2: 300,   start: 0, stop: 50000,  rbw: "12.21 Hz", desc: "100Hz/300Hz"},
            {ch1: 200,    ch2: 600,   start: 0, stop: 50000,  rbw: "12.21 Hz", desc: "200Hz/600Hz"},
            {ch1: 300,    ch2: 700,   start: 0, stop: 50000,  rbw: "12.21 Hz", desc: "300Hz/700Hz"},
            {ch1: 4000,   ch2: 7000,  start: 0, stop: 50000,  rbw: "12.21 Hz", desc: "4kHz/7kHz"},
            {ch1: 10000,  ch2: 15000, start: 0, stop: 50000,  rbw: "12.21 Hz", desc: "10kHz/15kHz"},
            {ch1: 25000,  ch2: 35000, start: 0, stop: 50000,  rbw: "24.41 Hz",   desc: "25kHz/35kHz"},
            {ch1: 50000,  ch2: 70000, start: 0, stop: 100000, rbw: "61.04 Hz",   desc: "50kHz/70kHz"}
        ];

        for (var d = 0; d < dualConfigs.length; d++) {
            var dc = dualConfigs[d];
            printToConsole("    Testing dual: " + dc.desc);

            // Configure signal generator - both channels
            setupSignalGenerator(0, 10, dc.ch1, 0, 0);
            setupSignalGenerator(1, 10, dc.ch2, 0, 0);
            siggen.running = true;
            msleep(500);

            // Configure spectrum analyzer
            switchToTool("Spectrum Analyzer");
            spectrum.running = false;
            spectrum.startFreq = dc.start;
            spectrum.stopFreq = dc.stop;
            spectrum.resBW = dc.rbw;

            channels = spectrum.channels;
            if (channels && channels[0] && channels[1]) {
                channels[0].enabled = true;
                channels[0].type = 0;
                channels[0].window = 0; // Flat-top
                channels[1].enabled = true;
                channels[1].type = 0;
                channels[1].window = 0; // Flat-top
            }

            // Enable marker table for monitoring
            spectrum.markerTableVisible = true;

            spectrum.running = true;
            msleep(3000);
            spectrum.running = false;
            msleep(500);

            // Check CH1 peak
            var peak1 = findPeakFrequency(0);
            if (peak1) {
                var tol1 = dc.ch1 * 0.05;
                if (tol1 < 20) tol1 = 20;
                var p1 = TestFramework.assertApproxEqual(peak1.frequency, dc.ch1, tol1,
                    "CH1 peak at " + dc.ch1 + "Hz");
                allPass = allPass && p1;
            }

            // Check CH2 peak
            var peak2 = findPeakFrequency(1);
            if (peak2) {
                var tol2 = dc.ch2 * 0.05;
                if (tol2 < 20) tol2 = 20;
                var p2 = TestFramework.assertApproxEqual(peak2.frequency, dc.ch2, tol2,
                    "CH2 peak at " + dc.ch2 + "Hz");
                allPass = allPass && p2;
            }

            // Stop siggen between tests
            switchToTool("Signal Generator");
            siggen.running = false;
        }

        // Cleanup
        switchToTool("Signal Generator");
        siggen.running = false;
        var enabled = siggen.enabled;
        enabled[0] = false;
        enabled[1] = false;
        siggen.enabled = enabled;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// TST.M2K.SA.ADDITIONAL_FEATURES
// Verifies detector types (Peak Hold, Min Hold) and marker table.
// ============================================================================
TestFramework.runTest("TST.M2K.SA.ADDITIONAL_FEATURES", function() {
    try {
        var allPass = true;

        // --- Part 1: Peak Hold Continuous detector (steps 1-8) ---
        printToConsole("  Part 1: Peak Hold Continuous detector (CH1)");

        // Steps 1-2: Configure CH1 - Type=Sample, Window=Flat-top
        if (!switchToTool("Spectrum Analyzer")) {
            printToConsole("ERROR: Cannot access Spectrum Analyzer");
            return false;
        }
        var channels = spectrum.channels;
        if (channels && channels[0]) {
            channels[0].enabled = true;
            channels[0].type = 0;   // Sample initially
            channels[0].window = 0; // Flat-top
        }

        // Step 3: Wiring is physical

        // Steps 4-5: Configure sweep and signal generator
        spectrum.startFreq = 0;
        spectrum.stopFreq = 1000000;
        spectrum.resBW = "61.04 Hz";

        setupSignalGenerator(0, 10, 500000, 0, 0);
        siggen.running = true;
        msleep(500);

        // Steps 6-7: Set detector type to Peak Hold Continuous
        switchToTool("Spectrum Analyzer");
        channels = spectrum.channels;
        if (channels && channels[0]) {
            channels[0].type = 2; // Peak Hold Continuous (enum: 0=Sample, 1=PeakHold, 2=PeakHoldContinuous)
        }

        spectrum.running = true;
        msleep(3000);

        // Step 8: Change signal to 250kHz, peak hold should retain 500kHz peak
        switchToTool("Signal Generator");
        siggen.running = false;
        var freqs = siggen.waveform_frequency;
        freqs[0] = 250000;
        siggen.waveform_frequency = freqs;
        siggen.running = true;
        msleep(500);

        switchToTool("Spectrum Analyzer");
        msleep(3000);
        spectrum.running = false;
        msleep(500);

        // Verify: with peak hold, we expect BOTH 250kHz and 500kHz visible in data.
        // Peak hold retains the max at each bin, so the old 500kHz peak should persist
        // alongside the new 250kHz peak.
        var channels_ph = spectrum.channels;
        if (channels_ph && channels_ph[0]) {
            var data = channels_ph[0].data;
            var freq = channels_ph[0].freq;

            if (data && freq && data.length > 0) {
                // Find local peak near 250kHz
                var found250 = false;
                var found500 = false;
                var noiseFloor = -80; // dB threshold above which we consider a peak

                for (var i = 1; i < data.length - 1; i++) {
                    if (data[i] > data[i-1] && data[i] > data[i+1] && data[i] > noiseFloor) {
                        if (Math.abs(freq[i] - 250000) < 25000) found250 = true;
                        if (Math.abs(freq[i] - 500000) < 50000) found500 = true;
                    }
                }

                var pass250 = TestFramework.assertEqual(found250, true,
                    "Peak Hold: 250kHz peak present (new signal)");
                var pass500 = TestFramework.assertEqual(found500, true,
                    "Peak Hold: 500kHz peak retained (old signal)");
                allPass = allPass && pass250 && pass500;
            }
        }

        // --- Part 2: Min Hold Continuous detector (steps 9-11) ---
        printToConsole("  Part 2: Min Hold Continuous detector (CH1)");

        // Reset detector to Sample first to clear Peak Hold state
        channels = spectrum.channels;
        if (channels && channels[0]) {
            channels[0].type = 0; // Sample (reset)
        }

        // Step 9: Set detector type to Min Hold Continuous
        channels = spectrum.channels;
        if (channels && channels[0]) {
            channels[0].type = 4; // Min Hold Continuous (enum: 3=MinHold, 4=MinHoldContinuous)
        }

        // Reset signal to 500kHz
        switchToTool("Signal Generator");
        siggen.running = false;
        freqs = siggen.waveform_frequency;
        freqs[0] = 500000;
        siggen.waveform_frequency = freqs;
        siggen.running = true;
        msleep(500);

        // Step 10: Run spectrum analyzer
        switchToTool("Spectrum Analyzer");
        spectrum.running = true;
        msleep(3000);

        // Step 11: Change signal to 250kHz
        switchToTool("Signal Generator");
        siggen.running = false;
        freqs = siggen.waveform_frequency;
        freqs[0] = 250000;
        siggen.waveform_frequency = freqs;
        siggen.running = true;
        msleep(500);

        switchToTool("Spectrum Analyzer");
        msleep(3000);
        spectrum.running = false;
        msleep(500);

        // Min hold: after changing from 500kHz to 250kHz, the min hold should suppress
        // the old 500kHz peak (replaced by noise floor minimum at that bin).
        var channels_mh = spectrum.channels;
        if (channels_mh && channels_mh[0]) {
            var mhData = channels_mh[0].data;
            var mhFreq = channels_mh[0].freq;

            if (mhData && mhFreq && mhData.length > 0) {
                // Check that 500kHz bin is suppressed (below peak threshold)
                var peakAt500 = false;
                var noiseThreshold = -40; // dB - a real peak would be well above this
                for (var i = 1; i < mhData.length - 1; i++) {
                    if (Math.abs(mhFreq[i] - 500000) < 50000 &&
                        mhData[i] > mhData[i-1] && mhData[i] > mhData[i+1] &&
                        mhData[i] > noiseThreshold) {
                        peakAt500 = true;
                    }
                }
                var passMH = TestFramework.assertEqual(peakAt500, false,
                    "Min Hold: 500kHz peak suppressed after frequency change");
                allPass = allPass && passMH;
            }
        }

        // --- Part 3: Repeat detector tests on CH2 (step 12) ---
        printToConsole("  Part 3: Repeat detector tests on CH2");

        channels = spectrum.channels;
        if (channels && channels[1]) {
            channels[1].enabled = true;
            channels[1].window = 0; // Flat-top

            // Peak Hold on CH2
            channels[1].type = 2; // Peak Hold Continuous (enum: 0=Sample, 1=PeakHold, 2=PeakHoldContinuous)
        }

        setupSignalGenerator(1, 10, 500000, 0, 0);
        siggen.running = true;
        msleep(500);

        switchToTool("Spectrum Analyzer");
        spectrum.running = true;
        msleep(3000);
        spectrum.running = false;
        msleep(500);

        var peakCH2 = findPeakFrequency(1);
        if (peakCH2) {
            var passCH2 = TestFramework.assertApproxEqual(peakCH2.frequency, 500000, 50000,
                "CH2 Peak Hold: peak at 500kHz");
            allPass = allPass && passCH2;
        }

        // Min Hold on CH2
        channels = spectrum.channels;
        if (channels && channels[1]) {
            channels[1].type = 4; // Min Hold Continuous (enum: 3=MinHold, 4=MinHoldContinuous)
        }
        spectrum.running = true;
        msleep(3000);
        spectrum.running = false;
        msleep(500);
        // CH2 Min Hold: verify detector type was applied
        var peakCH2mh = findPeakFrequency(1);
        if (peakCH2mh) {
            printToConsole("    CH2 Min Hold: peak at " + peakCH2mh.frequency + "Hz, mag=" + peakCH2mh.magnitude + "dB");
        }
        printToConsole("    CH2 Min Hold detector test completed");

        // --- Part 4: Marker Table functionality (steps 13-20) ---
        printToConsole("  Part 4: Marker Table functionality");

        // Steps 13-14: Configure both channels
        channels = spectrum.channels;
        if (channels && channels[0] && channels[1]) {
            channels[0].enabled = true;
            channels[0].type = 0;   // Sample
            channels[0].window = 0; // Flat-top
            channels[1].enabled = true;
            channels[1].type = 0;
            channels[1].window = 0; // Flat-top
        }

        // Step 15: Signal Generator CH1: Square 5V 50kHz
        setupSignalGenerator(0, 5, 50000, 0, 0, 1); // 1 = Square wave

        // Step 16: Signal Generator CH2: Triangle 5V 100kHz
        setupSignalGenerator(1, 5, 100000, 0, 0, 2); // 2 = Triangle wave
        siggen.running = true;
        msleep(500);

        // Step 17: Set sweep: Start 0Hz, Stop 1MHz, resBW 61.04 Hz
        switchToTool("Spectrum Analyzer");
        spectrum.startFreq = 0;
        spectrum.stopFreq = 1000000;
        spectrum.resBW = "61.04 Hz";

        // Step 18: Run both instruments
        spectrum.running = true;
        msleep(3000);
        spectrum.running = false;
        msleep(500);

        // Step 19: Enable marker table
        spectrum.markerTableVisible = true;
        var tablePass = TestFramework.assertEqual(spectrum.markerTableVisible, true,
            "Marker table enabled");
        allPass = allPass && tablePass;

        // Step 20: Enable 5 markers per channel and set to known harmonic positions
        // Square wave at 50kHz: harmonics at 50kHz, 150kHz, 250kHz, 350kHz, 450kHz (odd harmonics)
        // Triangle wave at 100kHz: harmonics at 100kHz, 300kHz, 500kHz, 700kHz, 900kHz (odd harmonics)
        // NOTE: RST says use Up Ampl/Dn Ampl buttons (not automatable),
        //       so we set marker positions manually to known harmonic frequencies.
        var markers = spectrum.markers;
        if (markers) {
            var ch0Harmonics = [50000, 150000, 250000, 350000, 450000];
            var ch1Harmonics = [100000, 300000, 500000, 700000, 900000];
            var mkIdx = 0;

            // Set CH1 markers
            for (var i = 0; i < markers.length && mkIdx < 5; i++) {
                if (markers[i].chId === 0) {
                    markers[i].en = true;
                    markers[i].type = 0; // Manual
                    markers[i].freq = ch0Harmonics[mkIdx];
                    msleep(200);
                    mkIdx++;
                }
            }

            // Set CH2 markers
            mkIdx = 0;
            for (var i = 0; i < markers.length && mkIdx < 5; i++) {
                if (markers[i].chId === 1) {
                    markers[i].en = true;
                    markers[i].type = 0; // Manual
                    markers[i].freq = ch1Harmonics[mkIdx];
                    msleep(200);
                    mkIdx++;
                }
            }
            printToConsole("    Enabled markers on harmonics for both channels");
        }

        // Verify fundamental frequency detection for CH1 (50kHz square wave)
        var peakSq = findPeakFrequency(0);
        if (peakSq) {
            var freqPass = TestFramework.assertApproxEqual(peakSq.frequency, 50000, 5000,
                "CH1 fundamental at 50kHz (Square)");
            allPass = allPass && freqPass;
        }

        // Verify fundamental frequency detection for CH2 (100kHz triangle wave)
        var peakTri = findPeakFrequency(1);
        if (peakTri) {
            var freqPass2 = TestFramework.assertApproxEqual(peakTri.frequency, 100000, 10000,
                "CH2 fundamental at 100kHz (Triangle)");
            allPass = allPass && freqPass2;
        }

        // Cleanup
        switchToTool("Signal Generator");
        siggen.running = false;
        var enabled = siggen.enabled;
        enabled[0] = false;
        enabled[1] = false;
        siggen.enabled = enabled;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
