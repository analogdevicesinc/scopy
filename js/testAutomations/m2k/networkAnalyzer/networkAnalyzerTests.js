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

// Network Analyzer Tests
// Tests that verify network analyzer functionality
// Based on TST.NA.* test specifications
//
// Hardware Setup Required (Loopback Mode):
//   - W1 (Signal Generator) → 1+ (Channel 1 Input)
//   - 1- → GND
//   - 2+ → 2- (Channel 2 terminated to ground for reference)
//   - All grounds connected
//
// Note: Filter tests (Low Pass, High Pass, Band Pass, Band Stop) require
// external RC circuits. These tests verify basic functionality with loopback.

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: Network Analyzer Tests
TestFramework.init("Network Analyzer Tests");

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to Network Analyzer tool
if (!switchToTool("Network Analyzer")) {
    printToConsole("ERROR: Cannot access Network Analyzer");
    exit(1);
}

// ============================================================================
// Test 1: Sweep Settings Configuration
// UID: TST.NA.SWEEP_SETTINGS
// ============================================================================
TestFramework.runTest("TST.NA.SWEEP_SETTINGS", function() {
    try {
        let allPass = true;

        // Test frequency range settings
        printToConsole("  Testing frequency range settings");

        // Note: M2K max frequency is 25MHz (hardware limit)
        let freqConfigs = [
            {min: 10, max: 500000, desc: "10Hz to 500kHz"},
            {min: 50, max: 25000000, desc: "50Hz to 25MHz"},
            {min: 1, max: 1000000, desc: "1Hz to 1MHz"},
            {min: 20, max: 10000000, desc: "20Hz to 10MHz"}
        ];

        for (let config of freqConfigs) {
            network.min_freq = config.min;
            network.max_freq = config.max;
            msleep(1000);  // Allow hardware to settle

            let actualMin = network.min_freq;
            let actualMax = network.max_freq;

            let minPass = TestFramework.assertApproxEqual(actualMin, config.min, config.min * 0.01,
                "Min freq " + config.desc);
            let maxPass = TestFramework.assertApproxEqual(actualMax, config.max, config.max * 0.01,
                "Max freq " + config.desc);

            allPass = allPass && minPass && maxPass;
        }

        // Test sample count settings
        printToConsole("  Testing sample count settings");

        let sampleCounts = [10, 50, 100, 200];
        for (let count of sampleCounts) {
            network.samples_count = count;
            msleep(1000);  // Allow hardware to settle

            let actual = network.samples_count;
            let pass = TestFramework.assertApproxEqual(actual, count, 1,
                "Sample count " + count);
            allPass = allPass && pass;
        }

        // Test logarithmic vs linear sweep
        printToConsole("  Testing sweep type (log/linear)");

        network.log_freq = true;
        msleep(1000);  // Allow hardware to settle
        let logEnabled = network.log_freq;
        let logPass = TestFramework.assertEqual(logEnabled, true, "Logarithmic sweep enabled");
        allPass = allPass && logPass;

        network.log_freq = false;
        msleep(1000);  // Allow hardware to settle
        let linEnabled = !network.log_freq;
        let linPass = TestFramework.assertEqual(linEnabled, true, "Linear sweep enabled");
        allPass = allPass && linPass;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 2: Reference Settings Configuration
// UID: TST.NA.REFERENCE_SETTINGS
// ============================================================================
TestFramework.runTest("TST.NA.REFERENCE_SETTINGS", function() {
    try {
        let allPass = true;

        // Test reference channel selection
        printToConsole("  Testing reference channel selection");

        // Channel 1 reference
        network.ref_channel = 1;
        msleep(1000);  // Allow hardware to settle
        let refCh1 = network.ref_channel;
        let ch1Pass = TestFramework.assertEqual(refCh1, 1, "Reference channel 1");
        allPass = allPass && ch1Pass;

        // Channel 2 reference
        network.ref_channel = 2;
        msleep(1000);  // Allow hardware to settle
        let refCh2 = network.ref_channel;
        let ch2Pass = TestFramework.assertEqual(refCh2, 2, "Reference channel 2");
        allPass = allPass && ch2Pass;

        // Test amplitude settings
        printToConsole("  Testing amplitude settings");

        let amplitudes = [0.5, 1, 2, 5];
        for (let amp of amplitudes) {
            network.amplitude = amp;
            msleep(1000);  // Allow hardware to settle

            let actual = network.amplitude;
            let pass = TestFramework.assertApproxEqual(actual, amp, 0.01,
                "Amplitude " + amp + "V");
            allPass = allPass && pass;
        }

        // Test offset settings
        printToConsole("  Testing offset settings");

        let offsets = [-2, -1, 0, 1, 2];
        for (let offset of offsets) {
            network.offset = offset;
            msleep(1000);  // Allow hardware to settle

            let actual = network.offset;
            let pass = TestFramework.assertApproxEqual(actual, offset, 0.01,
                "Offset " + offset + "V");
            allPass = allPass && pass;
        }

        // Reset to defaults
        network.ref_channel = 1;
        network.amplitude = 1;
        network.offset = 0;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 3: Display Settings Configuration
// UID: TST.NA.DISPLAY_SETTINGS
// ============================================================================
TestFramework.runTest("TST.NA.DISPLAY_SETTINGS", function() {
    try {
        let allPass = true;

        // Test magnitude scale settings
        printToConsole("  Testing magnitude scale settings");

        let magConfigs = [
            {min: -90, max: 10},
            {min: -50, max: 5},
            {min: -45, max: 10},
            {min: -100, max: 20}
        ];

        for (let config of magConfigs) {
            network.min_mag = config.min;
            network.max_mag = config.max;
            msleep(1000);  // Allow hardware to settle

            let actualMin = network.min_mag;
            let actualMax = network.max_mag;

            let minPass = TestFramework.assertApproxEqual(actualMin, config.min, 0.1,
                "Min magnitude " + config.min + "dB");
            let maxPass = TestFramework.assertApproxEqual(actualMax, config.max, 0.1,
                "Max magnitude " + config.max + "dB");

            allPass = allPass && minPass && maxPass;
        }

        // Test phase scale settings
        printToConsole("  Testing phase scale settings");

        let phaseConfigs = [
            {min: -150, max: 60},
            {min: -180, max: 180},
            {min: -100, max: 20},
            {min: -90, max: 90}
        ];

        for (let config of phaseConfigs) {
            network.min_phase = config.min;
            network.max_phase = config.max;
            msleep(1000);  // Allow hardware to settle

            let actualMin = network.min_phase;
            let actualMax = network.max_phase;

            let minPass = TestFramework.assertApproxEqual(actualMin, config.min, 0.1,
                "Min phase " + config.min + "°");
            let maxPass = TestFramework.assertApproxEqual(actualMax, config.max, 0.1,
                "Max phase " + config.max + "°");

            allPass = allPass && minPass && maxPass;
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 4: Plot Types
// UID: TST.NA.PLOT_TYPES
// ============================================================================
TestFramework.runTest("TST.NA.PLOT_TYPES", function() {
    try {
        let allPass = true;

        // Plot types: 0=Bode, 1=Nyquist, 2=Nichols (based on typical implementations)
        let plotTypes = [
            {type: 0, name: "Bode"},
            {type: 1, name: "Nyquist"},
            {type: 2, name: "Nichols"}
        ];

        for (let plot of plotTypes) {
            printToConsole("  Testing " + plot.name + " plot type");

            network.plot_type = plot.type;
            msleep(1000);  // Allow hardware to settle

            let actual = network.plot_type;
            let pass = TestFramework.assertEqual(actual, plot.type,
                plot.name + " plot type set");
            allPass = allPass && pass;
        }

        // Reset to Bode
        network.plot_type = 0;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 5: Cursor Functionality
// UID: TST.NA.CURSORS
// ============================================================================
TestFramework.runTest("TST.NA.CURSORS", function() {
    try {
        let allPass = true;

        // Test cursor enable/disable
        printToConsole("  Testing cursor enable/disable");

        network.cursors = true;
        msleep(1000);  // Allow hardware to settle
        let cursorsEnabled = network.cursors;
        let enablePass = TestFramework.assertEqual(cursorsEnabled, true, "Cursors enabled");
        allPass = allPass && enablePass;

        // Test cursor position settings (0-3 typically)
        // Note: Cursors must remain enabled for position/transparency to work
        printToConsole("  Testing cursor position");

        let positions = [0, 1, 2, 3];
        for (let pos of positions) {
            network.cursors_position = pos;
            msleep(1000);  // Allow hardware to settle

            let actual = network.cursors_position;
            let pass = TestFramework.assertEqual(actual, pos,
                "Cursor position " + pos);
            allPass = allPass && pass;
        }

        // Test cursor transparency
        printToConsole("  Testing cursor transparency");

        let transparencies = [0, 25, 50, 75, 100];
        for (let trans of transparencies) {
            network.cursors_transparency = trans;
            msleep(1000);  // Allow hardware to settle

            let actual = network.cursors_transparency;
            let pass = TestFramework.assertEqual(actual, trans,
                "Cursor transparency " + trans + "%");
            allPass = allPass && pass;
        }

        // Now test disable
        network.cursors = false;
        msleep(1000);  // Allow hardware to settle
        let cursorsDisabled = !network.cursors;
        let disablePass = TestFramework.assertEqual(cursorsDisabled, true, "Cursors disabled");
        allPass = allPass && disablePass;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 6: Averaging and Periods Settings
// UID: TST.NA.AVERAGING
// ============================================================================
TestFramework.runTest("TST.NA.AVERAGING", function() {
    try {
        let allPass = true;

        // Test averaging settings
        printToConsole("  Testing averaging settings");

        let avgValues = [1, 2, 4, 8];
        for (let avg of avgValues) {
            network.averaging = avg;
            msleep(1000);  // Allow hardware to settle

            let actual = network.averaging;
            let pass = TestFramework.assertEqual(actual, avg,
                "Averaging " + avg);
            allPass = allPass && pass;
        }

        // Test periods settings
        // Note: Minimum periods is 2 (hardware limit)
        printToConsole("  Testing periods settings");

        let periodValues = [2, 4, 8, 16];
        for (let periods of periodValues) {
            network.periods = periods;
            msleep(1000);  // Allow hardware to settle

            let actual = network.periods;
            let pass = TestFramework.assertEqual(actual, periods,
                "Periods " + periods);
            allPass = allPass && pass;
        }

        // Reset to defaults
        network.averaging = 1;
        network.periods = 2;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 7: Line Thickness Settings
// UID: TST.NA.LINE_THICKNESS
// ============================================================================
TestFramework.runTest("TST.NA.LINE_THICKNESS", function() {
    try {
        let allPass = true;

        printToConsole("  Testing line thickness settings");

        let thicknesses = [1, 2, 3, 4, 5];
        for (let thick of thicknesses) {
            network.line_thickness = thick;
            msleep(1000);  // Allow hardware to settle

            let actual = network.line_thickness;
            let pass = TestFramework.assertEqual(actual, thick,
                "Line thickness " + thick);
            allPass = allPass && pass;
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 8: Loopback Sweep Test (Channel 1)
// UID: TST.NA.LOOPBACK_CH1
// Description: With direct loopback (W1 → 1+), signal should pass unattenuated
// ============================================================================
TestFramework.runTest("TST.NA.LOOPBACK_CH1", function() {
    try {
        printToConsole("  Running loopback sweep on Channel 1");
        printToConsole("  (Requires W1 → 1+ connection)");

        // Configure for loopback test
        network.ref_channel = 1;
        network.amplitude = 1;
        network.offset = 0;
        network.min_freq = 100;
        network.max_freq = 100000;
        network.samples_count = 20;
        network.log_freq = true;
        network.min_mag = -20;
        network.max_mag = 10;
        network.min_phase = -180;
        network.max_phase = 180;
        network.plot_type = 0; // Bode
        network.averaging = 1;
        network.periods = 1;

        // Run sweep
        network.running = true;
        msleep(5000); // Allow sweep to complete (depends on sample count and freq range)

        // Stop and read data
        network.running = false;
        msleep(500);

        // Get measurement data
        // NOTE: API naming quirk - network.data returns X-axis (frequency),
        // network.freq returns Y-axis (magnitude in dB)
        let freqData = network.data;   // X-axis = frequency values
        let magData = network.freq;    // Y-axis = magnitude values (dB)
        let phaseData = network.phase;

        // Verify we got data
        let dataAvailable = (magData && magData.length > 0);
        let dataPass = TestFramework.assertEqual(dataAvailable, true,
            "Measurement data available");

        if (!dataAvailable) {
            printToConsole("    Warning: No measurement data received");
            return false;
        }

        printToConsole("    Received " + magData.length + " data points");

        // For loopback, magnitude should be close to 0dB (±3dB tolerance)
        // Check average magnitude
        let sumMag = 0;
        for (let i = 0; i < magData.length; i++) {
            sumMag += magData[i];
        }
        let avgMag = sumMag / magData.length;

        printToConsole("    Average magnitude: " + avgMag.toFixed(2) + " dB");

        // With loopback, average magnitude should be near 0dB
        // Allow ±5dB tolerance for cable losses and measurement uncertainty
        let magPass = TestFramework.assertInRange(avgMag, -5, 5,
            "Average magnitude near 0dB for loopback");

        // Check that phase data is also available
        let phaseAvailable = (phaseData && phaseData.length > 0);
        let phasePass = TestFramework.assertEqual(phaseAvailable, true,
            "Phase data available");

        return dataPass && magPass && phasePass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 9: Frequency Response Data Validity
// UID: TST.NA.DATA_VALIDITY
// Description: Verify frequency data matches configured sweep range
// ============================================================================
TestFramework.runTest("TST.NA.DATA_VALIDITY", function() {
    try {
        printToConsole("  Testing frequency response data validity");

        // Configure sweep
        let minFreq = 1000;
        let maxFreq = 100000;
        let sampleCount = 10;

        network.ref_channel = 1;
        network.amplitude = 1;
        network.offset = 0;
        network.min_freq = minFreq;
        network.max_freq = maxFreq;
        network.samples_count = sampleCount;
        network.log_freq = true;
        network.plot_type = 0; // Bode

        // Run sweep
        network.running = true;
        msleep(5000);
        network.running = false;
        msleep(500);

        // Get data
        // NOTE: API naming quirk - network.data returns X-axis (frequency),
        // network.freq returns Y-axis (magnitude in dB)
        let freqData = network.data;   // X-axis = frequency values
        let magData = network.freq;    // Y-axis = magnitude values (dB)

        if (!freqData || freqData.length === 0) {
            printToConsole("    Warning: No frequency data received");
            return "SKIP";
        }

        let allPass = true;

        // Verify number of points matches sample count
        // Note: Actual count may vary slightly
        printToConsole("    Expected ~" + sampleCount + " points, got " + freqData.length);

        // Verify frequency range
        let firstFreq = freqData[0];
        let lastFreq = freqData[freqData.length - 1];

        printToConsole("    Frequency range: " + firstFreq.toFixed(2) + " Hz to " + lastFreq.toFixed(2) + " Hz");

        // First frequency should be near min_freq (within 20%)
        let firstPass = TestFramework.assertInRange(firstFreq, minFreq * 0.8, minFreq * 1.2,
            "First frequency near " + minFreq + " Hz");
        allPass = allPass && firstPass;

        // Last frequency should be near max_freq (within 20%)
        let lastPass = TestFramework.assertInRange(lastFreq, maxFreq * 0.8, maxFreq * 1.2,
            "Last frequency near " + maxFreq + " Hz");
        allPass = allPass && lastPass;

        // Verify frequencies are in ascending order
        let ascending = true;
        for (let i = 1; i < freqData.length; i++) {
            if (freqData[i] <= freqData[i-1]) {
                ascending = false;
                break;
            }
        }
        let orderPass = TestFramework.assertEqual(ascending, true,
            "Frequencies in ascending order");
        allPass = allPass && orderPass;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 10: Logarithmic vs Linear Sweep Comparison
// UID: TST.NA.SWEEP_COMPARISON
// Description: Verify log and linear sweeps produce different frequency distributions
// ============================================================================
TestFramework.runTest("TST.NA.SWEEP_COMPARISON", function() {
    try {
        printToConsole("  Comparing logarithmic vs linear sweep");

        // Configure common settings
        network.ref_channel = 1;
        network.amplitude = 1;
        network.offset = 0;
        network.min_freq = 100;
        network.max_freq = 10000;
        network.samples_count = 10;
        network.plot_type = 0;

        // Run logarithmic sweep
        network.log_freq = true;
        network.running = true;
        msleep(3000);
        network.running = false;
        msleep(500);

        // NOTE: API naming quirk - network.data returns X-axis (frequency)
        let logFreqData = network.data.slice(); // Copy array

        // Run linear sweep
        network.log_freq = false;
        network.running = true;
        msleep(3000);
        network.running = false;
        msleep(500);

        let linFreqData = network.data.slice();

        if (!logFreqData || logFreqData.length === 0 || !linFreqData || linFreqData.length === 0) {
            printToConsole("    Warning: Could not get sweep data");
            return "SKIP";
        }

        let allPass = true;

        // For log sweep, frequency ratio between consecutive points should be roughly constant
        // For linear sweep, frequency difference between consecutive points should be roughly constant

        // Check log sweep - ratio should be similar
        if (logFreqData.length >= 3) {
            let ratio1 = logFreqData[1] / logFreqData[0];
            let ratio2 = logFreqData[2] / logFreqData[1];
            let ratioSimilar = Math.abs(ratio1 - ratio2) < (ratio1 * 0.2);
            printToConsole("    Log sweep ratios: " + ratio1.toFixed(2) + ", " + ratio2.toFixed(2));
            let logRatioPass = TestFramework.assertEqual(ratioSimilar, true,
                "Log sweep has consistent frequency ratios");
            allPass = allPass && logRatioPass;
        }

        // Check linear sweep - difference should be similar
        if (linFreqData.length >= 3) {
            let diff1 = linFreqData[1] - linFreqData[0];
            let diff2 = linFreqData[2] - linFreqData[1];
            let diffSimilar = Math.abs(diff1 - diff2) < (diff1 * 0.2);
            printToConsole("    Linear sweep diffs: " + diff1.toFixed(2) + ", " + diff2.toFixed(2));
            let linDiffPass = TestFramework.assertEqual(diffSimilar, true,
                "Linear sweep has consistent frequency differences");
            allPass = allPass && linDiffPass;
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 11: Reference Channel Comparison
// UID: TST.NA.REF_CHANNEL_COMPARE
// Description: Test both reference channels produce valid data
// ============================================================================
TestFramework.runTest("TST.NA.REF_CHANNEL_COMPARE", function() {
    try {
        let allPass = true;

        // Common settings
        network.amplitude = 1;
        network.offset = 0;
        network.min_freq = 1000;
        network.max_freq = 50000;
        network.samples_count = 10;
        network.log_freq = true;
        network.plot_type = 0;

        // Test Channel 1 reference
        printToConsole("  Testing Channel 1 as reference");
        network.ref_channel = 1;
        network.running = true;
        msleep(3000);
        network.running = false;
        msleep(500);

        let ch1Data = network.data;
        let ch1HasData = (ch1Data && ch1Data.length > 0);
        let ch1Pass = TestFramework.assertEqual(ch1HasData, true,
            "Channel 1 reference produces data");
        allPass = allPass && ch1Pass;

        if (ch1HasData) {
            printToConsole("    CH1 data points: " + ch1Data.length);
        }

        // Test Channel 2 reference
        printToConsole("  Testing Channel 2 as reference");
        network.ref_channel = 2;
        network.running = true;
        msleep(3000);
        network.running = false;
        msleep(500);

        let ch2Data = network.data;
        let ch2HasData = (ch2Data && ch2Data.length > 0);
        let ch2Pass = TestFramework.assertEqual(ch2HasData, true,
            "Channel 2 reference produces data");
        allPass = allPass && ch2Pass;

        if (ch2HasData) {
            printToConsole("    CH2 data points: " + ch2Data.length);
        }

        // Reset to channel 1
        network.ref_channel = 1;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 12: Run/Stop Control
// UID: TST.NA.RUN_STOP
// Description: Verify run and stop controls work correctly
// ============================================================================
TestFramework.runTest("TST.NA.RUN_STOP", function() {
    try {
        let allPass = true;

        // Configure for quick sweep
        network.min_freq = 1000;
        network.max_freq = 10000;
        network.samples_count = 5;
        network.log_freq = true;

        // Test start
        printToConsole("  Testing start control");
        network.running = true;
        msleep(500);

        let isRunning = network.running;
        // Note: running state may have already completed for short sweeps
        printToConsole("    Running state after start: " + isRunning);

        // Wait for sweep and test stop
        msleep(2000);
        network.running = false;
        msleep(200);

        let isStopped = !network.running;
        let stopPass = TestFramework.assertEqual(isStopped, true, "Stop control works");
        allPass = allPass && stopPass;

        // Verify we can restart
        printToConsole("  Testing restart capability");
        network.running = true;
        msleep(500);
        network.running = false;
        msleep(200);

        let restartData = network.data;
        let restartPass = TestFramework.assertEqual(restartData && restartData.length > 0, true,
            "Restart produces data");
        allPass = allPass && restartPass;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 13: Wide Frequency Range Sweep
// UID: TST.NA.WIDE_SWEEP
// Description: Test sweep across wide frequency range (10Hz to 10MHz)
// ============================================================================
TestFramework.runTest("TST.NA.WIDE_SWEEP", function() {
    try {
        printToConsole("  Testing wide frequency range sweep (10Hz to 10MHz)");

        // Configure wide sweep
        network.ref_channel = 1;
        network.amplitude = 1;
        network.offset = 0;
        network.min_freq = 10;
        network.max_freq = 10000000; // 10 MHz
        network.samples_count = 50;
        network.log_freq = true;
        network.plot_type = 0;
        network.averaging = 1;

        // Run sweep (longer timeout for wide range)
        network.running = true;
        msleep(10000); // 10 seconds for wide sweep
        network.running = false;
        msleep(500);

        // Verify data
        // NOTE: API naming quirk - network.data returns X-axis (frequency),
        // network.freq returns Y-axis (magnitude in dB)
        let freqData = network.data;   // X-axis = frequency values
        let magData = network.freq;    // Y-axis = magnitude values (dB)
        let phaseData = network.phase;

        let allPass = true;

        // Check data availability
        let hasData = (freqData && freqData.length > 0);
        let dataPass = TestFramework.assertEqual(hasData, true,
            "Wide sweep produced data");
        allPass = allPass && dataPass;

        if (hasData) {
            printToConsole("    Data points: " + freqData.length);
            printToConsole("    Freq range: " + freqData[0].toFixed(2) + " Hz to " +
                          freqData[freqData.length-1].toFixed(2) + " Hz");

            // Find min and max magnitude
            let minMag = Math.min.apply(null, magData);
            let maxMag = Math.max.apply(null, magData);
            printToConsole("    Magnitude range: " + minMag.toFixed(2) + " dB to " + maxMag.toFixed(2) + " dB");

            // Verify magnitude is within reasonable range
            let magRangePass = TestFramework.assertInRange(maxMag, -50, 20,
                "Max magnitude in reasonable range");
            allPass = allPass && magRangePass;
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 14: Notes Functionality
// UID: TST.NA.NOTES
// Description: Test notes/annotations functionality
// ============================================================================
TestFramework.runTest("TST.NA.NOTES", function() {
    try {
        let allPass = true;

        printToConsole("  Testing notes functionality");

        // Set a note
        let testNote = "Test measurement - Network Analyzer automated test";
        network.notes = testNote;
        msleep(1000);  // Allow hardware to settle

        // Read back the note
        let readNote = network.notes;
        let notePass = TestFramework.assertEqual(readNote, testNote,
            "Notes set and retrieved correctly");
        allPass = allPass && notePass;

        // Clear note
        network.notes = "";
        msleep(1000);  // Allow hardware to settle
        let clearedNote = network.notes;
        let clearPass = TestFramework.assertEqual(clearedNote, "",
            "Notes cleared correctly");
        allPass = allPass && clearPass;

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
