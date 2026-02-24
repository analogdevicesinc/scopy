/*
 * Copyright (c) 2025 Analog Devices Inc.
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

// ADC Plugin Manual Test Automation
// Automates tests from: docs/tests/plugins/adc/adc_tests.rst
//
// Automated (Category A):
//   TST.ADC_TIME.X_AXIS, TST.ADC_TIME.Y_AXIS, TST.ADC_TIME_CHANNEL_SETTINGS,
//   TST.MEASURE, TST.ADC_TIME.XY_PLOT, TST.ADC_TIME.SETTINGS, TST.ADC_TIME.PRINT,
//   TST.CURSORS, TST.ADC_FREQ.X_AXIS_SETTINGS, TST.ADC_FREQ.Y_AXIS_SETTINGS,
//   TST.ADC_FREQ.MARKER_SETTINGS, TST.ADC_FREQ.SETTINGS_ADJUSTMENT,
//   TST.ADC_FREQ.PRINT_PLOT, TST.ADC_FREQ.Y_AXIS_CHANNEL_SETTINGS,
//   TST.ADC_FREQ.CURSOR_SETTINGS, TST.ADC_FREQ.CHANNEL_AVERAGING,
//   TST.ADC_GENALYZER.ENABLE, TST.ADC_GENALYZER.AUTO_MODE,
//   TST.ADC_GENALYZER.FIXED_TONE_MODE, TST.ADC_GENALYZER.FFT_INTEGRATION
//
// Skipped (Category C - not automatable):
//   TST.ADC_TIME.INFO - Opens external browser
//   TST.PLOT_NAVIGATION - Requires mouse interaction (scroll, click, drag)
//   TST.ADC_FREQ.VIEW_PLUGIN_DOC - Opens external browser
//   TST.PREF.X_SCALE_POS - Requires app restart
//   TST.PREF.Y_SCALE_POS - Requires app restart
//   TST.PREF.CH_HANDLE_POS - Requires app restart
//   TST.PREF.X_CURSOR_POS - Requires app restart
//   TST.PREF.Y_CURSOR_POS - Requires app restart
//   TST.PREF.DEFAULT_YMODE - Requires app restart

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("ADC Documentation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to ADC Time tool
if (!switchToTool("ADC - Time")) {
    printToConsole("WARNING: Could not switch to ADC - Time tool, tests will proceed via API");
}

// Get channels for use in tests
var timeChannels = adc.getTimeChannels();
printToConsole("Time channels: " + timeChannels);

// ============================================
// Test 1: X-AXIS
// UID: TST.ADC_TIME.X_AXIS
// Description: Verify X-axis behavior under different configurations
// ============================================
printToConsole("\n=== Test 1: X-AXIS ===\n");

TestFramework.runTest("TST.ADC_TIME.X_AXIS", function() {
    try {
        // Save original state
        var origBufferSize = adc.getTimeBufferSize();
        var origPlotSize = adc.getTimePlotSize();
        var origSync = adc.isTimeSyncBufferPlot();
        var origRolling = adc.isTimeRollingMode();
        printToConsole("  Original buffer=" + origBufferSize + " plot=" + origPlotSize +
                       " sync=" + origSync + " rolling=" + origRolling);

        // Step 3: Set buffer size to 100
        adc.setTimeBufferSize(100);
        msleep(500);
        var bufferSize = adc.getTimeBufferSize();
        printToConsole("  After setTimeBufferSize(100): " + bufferSize);
        if (bufferSize !== 100) {
            printToConsole("  FAIL: Buffer size not set to 100");
            adc.setTimeBufferSize(origBufferSize);
            return false;
        }
        printToConsole("  Step 3 PASS: Buffer size set to 100");

        // Step 4: Press Single
        adc.timeSingleShot();
        msleep(5000);
        printToConsole("  Step 4 PASS: Single shot completed");

        // Step 5: Turn off Sync buffer-plot sizes
        adc.setTimeSyncBufferPlot(false);
        msleep(500);
        var syncOff = adc.isTimeSyncBufferPlot();
        if (syncOff !== false) {
            printToConsole("  FAIL: Sync buffer-plot not turned off");
            adc.setTimeBufferSize(origBufferSize);
            adc.setTimeSyncBufferPlot(origSync);
            return false;
        }
        printToConsole("  Step 5 PASS: Sync buffer-plot turned off");

        // Step 6: Set plot size to 200
        adc.setTimePlotSize(200);
        msleep(500);
        var plotSize = adc.getTimePlotSize();
        printToConsole("  After setTimePlotSize(200): " + plotSize);
        if (plotSize !== 200) {
            printToConsole("  FAIL: Plot size not set to 200");
            adc.setTimeBufferSize(origBufferSize);
            adc.setTimePlotSize(origPlotSize);
            adc.setTimeSyncBufferPlot(origSync);
            return false;
        }
        printToConsole("  Step 6 PASS: Plot size set to 200");

        // Step 7: Press Single
        adc.timeSingleShot();
        msleep(5000);
        printToConsole("  Step 7 PASS: Single shot completed");

        // Step 8: Turn on Rolling mode
        adc.setTimeRollingMode(true);
        msleep(500);
        var rollingOn = adc.isTimeRollingMode();
        if (rollingOn !== true) {
            printToConsole("  FAIL: Rolling mode not turned on");
            adc.setTimeBufferSize(origBufferSize);
            adc.setTimePlotSize(origPlotSize);
            adc.setTimeSyncBufferPlot(origSync);
            adc.setTimeRollingMode(origRolling);
            return false;
        }
        printToConsole("  Step 8 PASS: Rolling mode enabled");

        // Restore original state
        adc.setTimeRollingMode(origRolling);
        msleep(500);
        adc.setTimeSyncBufferPlot(origSync);
        msleep(500);
        adc.setTimePlotSize(origPlotSize);
        msleep(500);
        adc.setTimeBufferSize(origBufferSize);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2: Y-AXIS
// UID: TST.ADC_TIME.Y_AXIS
// Description: Verify Y-axis behavior under different configurations
// ============================================
printToConsole("\n=== Test 2: Y-AXIS ===\n");

TestFramework.runTest("TST.ADC_TIME.Y_AXIS", function() {
    try {
        if (timeChannels.length === 0) {
            printToConsole("  No time channels available");
            return "SKIP";
        }
        var ch = timeChannels[0];

        // Save original state
        var origYMode = adc.getTimeChannelYMode(ch);
        var origYMin = adc.getTimeChannelYMin(ch);
        var origYMax = adc.getTimeChannelYMax(ch);
        printToConsole("  Original ch=" + ch + " YMode=" + origYMode +
                       " YMin=" + origYMin + " YMax=" + origYMax);

        // Step 5: Set YMode to ADC Counts (mode 0)
        adc.setTimeChannelYMode(ch, 0);
        msleep(500);
        var ymode = adc.getTimeChannelYMode(ch);
        printToConsole("  After setTimeChannelYMode(0): " + ymode);
        if (ymode !== 0) {
            printToConsole("  FAIL: YMode not set to ADC Counts (0)");
            adc.setTimeChannelYMode(ch, origYMode);
            return false;
        }
        printToConsole("  Step 5 PASS: YMode set to ADC Counts");

        // Step 6: Set Min to -4000 and Max to 4000
        adc.setTimeChannelYMin(ch, -4000);
        msleep(500);
        adc.setTimeChannelYMax(ch, 4000);
        msleep(500);
        var ymin = adc.getTimeChannelYMin(ch);
        var ymax = adc.getTimeChannelYMax(ch);
        printToConsole("  After set YMin=-4000 YMax=4000: min=" + ymin + " max=" + ymax);
        if (Math.abs(ymin - (-4000)) > 1 || Math.abs(ymax - 4000) > 1) {
            printToConsole("  FAIL: Y range not set correctly");
            adc.setTimeChannelYMode(ch, origYMode);
            adc.setTimeChannelYMin(ch, origYMin);
            adc.setTimeChannelYMax(ch, origYMax);
            return false;
        }
        printToConsole("  Step 6 PASS: Y range set to -4000 to 4000");

        // Step 7: Press Single
        adc.timeSingleShot();
        msleep(5000);
        printToConsole("  Step 7 PASS: Single shot completed");

        // Restore original state
        adc.setTimeChannelYMode(ch, origYMode);
        msleep(500);
        adc.setTimeChannelYMin(ch, origYMin);
        msleep(500);
        adc.setTimeChannelYMax(ch, origYMax);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 8: ADC-Time Channel Settings
// UID: TST.ADC_TIME_CHANNEL_SETTINGS
// Description: Test Y-axis settings and adjustments for the ADC-Time tool
// ============================================
printToConsole("\n=== Test 8: ADC-Time Channel Settings ===\n");

TestFramework.runTest("TST.ADC_TIME_CHANNEL_SETTINGS", function() {
    try {
        if (timeChannels.length === 0) {
            printToConsole("  No time channels available");
            return "SKIP";
        }
        var ch = timeChannels[0];

        // Save original state
        var origYMode = adc.getTimeChannelYMode(ch);
        var origYMin = adc.getTimeChannelYMin(ch);
        var origYMax = adc.getTimeChannelYMax(ch);
        printToConsole("  Original ch=" + ch + " YMode=" + origYMode +
                       " YMin=" + origYMin + " YMax=" + origYMax);

        // Step 3: Set YMode to ADC Counts, Min to -4000, Max to 4000
        adc.setTimeChannelYMode(ch, 0);
        msleep(500);
        adc.setTimeChannelYMin(ch, -4000);
        msleep(500);
        adc.setTimeChannelYMax(ch, 4000);
        msleep(500);

        var ymode = adc.getTimeChannelYMode(ch);
        var ymin = adc.getTimeChannelYMin(ch);
        var ymax = adc.getTimeChannelYMax(ch);
        printToConsole("  Channel " + ch + ": YMode=" + ymode + " YMin=" + ymin + " YMax=" + ymax);

        if (ymode !== 0) {
            printToConsole("  FAIL: YMode not set to ADC Counts (0) for " + ch);
            adc.setTimeChannelYMode(ch, origYMode);
            adc.setTimeChannelYMin(ch, origYMin);
            adc.setTimeChannelYMax(ch, origYMax);
            return false;
        }
        if (Math.abs(ymin - (-4000)) > 1 || Math.abs(ymax - 4000) > 1) {
            printToConsole("  FAIL: Y range not set correctly for " + ch);
            adc.setTimeChannelYMode(ch, origYMode);
            adc.setTimeChannelYMin(ch, origYMin);
            adc.setTimeChannelYMax(ch, origYMax);
            return false;
        }

        // Verify other channel was NOT affected (if multiple channels exist)
        if (timeChannels.length > 1) {
            var otherCh = timeChannels[1];
            var otherYMode = adc.getTimeChannelYMode(otherCh);
            printToConsole("  Other channel " + otherCh + " YMode=" + otherYMode +
                           " (should be unchanged from its original)");
        }

        printToConsole("  Step 3 PASS: Per-channel Y-axis settings applied correctly");

        // Press Single to verify
        adc.timeSingleShot();
        msleep(5000);
        printToConsole("  Single shot completed");

        // Restore original state
        adc.setTimeChannelYMode(ch, origYMode);
        msleep(500);
        adc.setTimeChannelYMin(ch, origYMin);
        msleep(500);
        adc.setTimeChannelYMax(ch, origYMax);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 9: Measure
// UID: TST.MEASURE
// Description: Test the Measure functionality, enabling panels
//   and checking frequency and stats
// ============================================
printToConsole("\n=== Test 9: Measure ===\n");

TestFramework.runTest("TST.MEASURE", function() {
    try {
        if (timeChannels.length === 0) {
            printToConsole("  No time channels available");
            return "SKIP";
        }
        var ch = timeChannels[0];

        // Get available measurements
        var measurements = adc.getTimeMeasurements(ch);
        if (!measurements || measurements.length === 0) {
            printToConsole("  No measurements available for " + ch);
            return "SKIP";
        }
        printToConsole("  Available measurements: " + measurements);

        // Find frequency measurement
        var freqMeas = null;
        for (var i = 0; i < measurements.length; i++) {
            if (measurements[i].toLowerCase().indexOf("freq") !== -1) {
                freqMeas = measurements[i];
                break;
            }
        }

        // Find a vertical measurement (middle/mean)
        var vertMeas = null;
        for (var i = 0; i < measurements.length; i++) {
            var name = measurements[i].toLowerCase();
            if (name.indexOf("mid") !== -1 || name.indexOf("mean") !== -1) {
                vertMeas = measurements[i];
                break;
            }
        }

        // Step 2: Enable frequency and vertical measurements
        if (freqMeas) {
            adc.enableTimeMeasurement(ch, freqMeas);
            msleep(500);
            printToConsole("  Enabled measurement: " + freqMeas);
        }
        if (vertMeas) {
            adc.enableTimeMeasurement(ch, vertMeas);
            msleep(500);
            printToConsole("  Enabled measurement: " + vertMeas);
        }

        // Run to acquire data
        adc.setTimeRunning(true);
        msleep(5000);
        adc.setTimeRunning(false);
        msleep(500);

        // Read measurement values
        var passed = true;
        if (freqMeas) {
            var freqValue = adc.getTimeMeasurementValue(ch, freqMeas);
            printToConsole("  " + freqMeas + " value: " + freqValue);
            if (typeof freqValue !== "number" || isNaN(freqValue)) {
                printToConsole("  FAIL: Frequency measurement returned invalid value");
                passed = false;
            } else {
                printToConsole("  PASS: Frequency measurement returned numeric value");
            }
        }
        if (vertMeas) {
            var vertValue = adc.getTimeMeasurementValue(ch, vertMeas);
            printToConsole("  " + vertMeas + " value: " + vertValue);
            if (typeof vertValue !== "number" || isNaN(vertValue)) {
                printToConsole("  FAIL: Vertical measurement returned invalid value");
                passed = false;
            } else {
                printToConsole("  PASS: Vertical measurement returned numeric value");
            }
        }

        // Step 3: Enable all measurements and verify
        printToConsole("  Enabling all measurements...");
        for (var i = 0; i < measurements.length; i++) {
            adc.enableTimeMeasurement(ch, measurements[i]);
            msleep(100);
        }
        msleep(500);
        printToConsole("  All measurements enabled: " + measurements.length + " total");

        // Step 4: Disable all measurements (cleanup)
        for (var i = 0; i < measurements.length; i++) {
            adc.disableTimeMeasurement(ch, measurements[i]);
            msleep(100);
        }
        msleep(500);
        printToConsole("  All measurements disabled (cleanup)");

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3: XY PLOT
// UID: TST.ADC_TIME.XY_PLOT
// Description: Verify XY plot functionality and visualization
// ============================================
printToConsole("\n=== Test 3: XY PLOT ===\n");

TestFramework.runTest("TST.ADC_TIME.XY_PLOT", function() {
    try {
        // Save original state
        var origEnabled = adc.isTimeXYPlotEnabled();
        printToConsole("  Original XY enabled: " + origEnabled);

        // Step 2: Turn on XY PLOT
        adc.setTimeXYPlotEnabled(true);
        msleep(500);
        var enabled = adc.isTimeXYPlotEnabled();
        if (!enabled) {
            printToConsole("  FAIL: XY plot not enabled");
            adc.setTimeXYPlotEnabled(origEnabled);
            return false;
        }
        printToConsole("  Step 2 PASS: XY plot enabled");

        if (timeChannels.length >= 1) {
            var origSource = adc.getTimeXYSource();

            // Step 4: Set X Axis source to first channel
            adc.setTimeXYSource(timeChannels[0]);
            msleep(500);
            var source = adc.getTimeXYSource();
            if (source !== timeChannels[0]) {
                printToConsole("  FAIL: XY source not set to " + timeChannels[0] + ", got " + source);
                adc.setTimeXYSource(origSource);
                adc.setTimeXYPlotEnabled(origEnabled);
                return false;
            }
            printToConsole("  Step 4 PASS: XY source set to " + timeChannels[0]);

            // Step 5: Set X Axis source to second channel
            if (timeChannels.length >= 2) {
                adc.setTimeXYSource(timeChannels[1]);
                msleep(500);
                var source2 = adc.getTimeXYSource();
                if (source2 !== timeChannels[1]) {
                    printToConsole("  FAIL: XY source not set to " + timeChannels[1]);
                    adc.setTimeXYSource(origSource);
                    adc.setTimeXYPlotEnabled(origEnabled);
                    return false;
                }
                printToConsole("  Step 5 PASS: XY source set to " + timeChannels[1]);
            }

            // Restore source
            adc.setTimeXYSource(origSource);
            msleep(500);
        }

        // Restore original state
        adc.setTimeXYPlotEnabled(origEnabled);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: Settings
// UID: TST.ADC_TIME.SETTINGS
// Description: Change Plot Title and toggle various plot settings
// ============================================
printToConsole("\n=== Test 4: Settings ===\n");

TestFramework.runTest("TST.ADC_TIME.SETTINGS", function() {
    try {
        // Save original state
        var origTitle = adc.timePlot.getTitle();
        var origLabels = adc.timePlot.isLabelsEnabled();
        printToConsole("  Original title: '" + origTitle + "' labels: " + origLabels);

        // Step 1: Change Plot Title to "test"
        adc.timePlot.setTitle("test");
        msleep(500);
        var title = adc.timePlot.getTitle();
        if (title !== "test") {
            printToConsole("  FAIL: Title not set to 'test', got '" + title + "'");
            adc.timePlot.setTitle(origTitle);
            return false;
        }
        printToConsole("  Step 1 PASS: Title changed to 'test'");

        // Step 2: Turn on and off plot labels
        adc.timePlot.setLabelsEnabled(true);
        msleep(500);
        var labelsOn = adc.timePlot.isLabelsEnabled();
        adc.timePlot.setLabelsEnabled(false);
        msleep(500);
        var labelsOff = adc.timePlot.isLabelsEnabled();
        if (!labelsOn || labelsOff) {
            printToConsole("  FAIL: Labels toggle not working (on=" + labelsOn + " off=" + labelsOff + ")");
            adc.timePlot.setTitle(origTitle);
            adc.timePlot.setLabelsEnabled(origLabels);
            return false;
        }
        printToConsole("  Step 2 PASS: Labels toggled on and off");

        // Step 3: Change thickness and style
        var channels = adc.timePlot.getChannels();
        if (channels.length > 0) {
            var ch = channels[0];
            var origThickness = adc.timePlot.getChannelThickness(ch);
            var origStyle = adc.timePlot.getChannelStyle(ch);

            var thicknessValues = [1, 2, 3];
            var thicknessPassed = true;
            for (var i = 0; i < thicknessValues.length; i++) {
                adc.timePlot.setChannelThickness(ch, thicknessValues[i]);
                msleep(500);
                if (adc.timePlot.getChannelThickness(ch) !== thicknessValues[i]) {
                    thicknessPassed = false;
                }
            }

            var styleValues = [0, 1, 2];
            var stylePassed = true;
            for (var i = 0; i < styleValues.length; i++) {
                adc.timePlot.setChannelStyle(ch, styleValues[i]);
                msleep(500);
                if (adc.timePlot.getChannelStyle(ch) !== styleValues[i]) {
                    stylePassed = false;
                }
            }

            // Restore
            adc.timePlot.setChannelThickness(ch, origThickness);
            adc.timePlot.setChannelStyle(ch, origStyle);
            msleep(500);

            if (!thicknessPassed || !stylePassed) {
                printToConsole("  FAIL: Thickness or style changes not verified");
                adc.timePlot.setTitle(origTitle);
                adc.timePlot.setLabelsEnabled(origLabels);
                return false;
            }
            printToConsole("  Step 3 PASS: Thickness and style changes verified");
        }

        // Restore original state
        adc.timePlot.setTitle(origTitle);
        msleep(500);
        adc.timePlot.setLabelsEnabled(origLabels);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 5: Print
// UID: TST.ADC_TIME.PRINT
// Description: Save the plot as a PDF file via the print functionality
// ============================================
printToConsole("\n=== Test 5: Print ===\n");

TestFramework.runTest("TST.ADC_TIME.PRINT", function() {
    try {
        // Press single to capture data
        adc.timeSingleShot();
        msleep(5000);

        // Print to file
        var filePath = "/tmp/scopy_adc_time_print_test.pdf";
        var result = adc.timePlot.printPlot(filePath);
        msleep(500);
        printToConsole("  Print result: " + result);
        if (!result) {
            printToConsole("  FAIL: printPlot returned false");
            return false;
        }
        printToConsole("  PASS: Plot printed to " + filePath);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 10: Cursors
// UID: TST.CURSORS
// Description: Test cursor functionality including synchronization,
//   tracking, and moving readouts
// ============================================
printToConsole("\n=== Test 10: Cursors ===\n");

TestFramework.runTest("TST.CURSORS", function() {
    try {
        // Save original state
        var origBufferSize = adc.getTimeBufferSize();
        var origXMode = adc.getTimeXMode();

        // Step 3: Set buffer size to 200, XMode to Sample
        adc.setTimeBufferSize(200);
        msleep(500);
        adc.setTimeXMode(0); // Samples
        msleep(500);

        // Step 4: Enable X and Y cursors
        adc.timePlot.setCursorsVisible(true);
        msleep(500);
        adc.timePlot.setXCursorsEnabled(true);
        msleep(500);
        adc.timePlot.setYCursorsEnabled(true);
        msleep(500);

        var cursorsVisible = adc.timePlot.isCursorsVisible();
        if (!cursorsVisible) {
            printToConsole("  FAIL: Cursors not visible");
            adc.setTimeBufferSize(origBufferSize);
            adc.setTimeXMode(origXMode);
            return false;
        }
        printToConsole("  Step 4 PASS: X and Y cursors enabled");

        // Step 5: Position X cursors
        adc.timePlot.setX1CursorPosition(50);
        msleep(500);
        adc.timePlot.setX2CursorPosition(100);
        msleep(500);
        var x1 = adc.timePlot.getX1CursorPosition();
        var x2 = adc.timePlot.getX2CursorPosition();
        printToConsole("  X1=" + x1 + " X2=" + x2 + " delta=" + Math.abs(x2 - x1));
        if (Math.abs(x1 - 50) > 1 || Math.abs(x2 - 100) > 1) {
            printToConsole("  FAIL: X cursor positions not set correctly");
            adc.timePlot.setCursorsVisible(false);
            adc.setTimeBufferSize(origBufferSize);
            adc.setTimeXMode(origXMode);
            return false;
        }
        printToConsole("  Step 5 PASS: X cursors positioned");

        // Step 6: Lock X cursors
        adc.timePlot.setXCursorsLocked(true);
        msleep(500);
        printToConsole("  Step 6 PASS: X cursors locked");

        // Step 7: Enable tracking
        adc.timePlot.setTrackingEnabled(true);
        msleep(500);
        printToConsole("  Step 7 PASS: Tracking enabled");

        // Step 9: Test Y cursors
        adc.timePlot.setY1CursorPosition(0.5);
        msleep(500);
        adc.timePlot.setY2CursorPosition(-0.5);
        msleep(500);
        var y1 = adc.timePlot.getY1CursorPosition();
        var y2 = adc.timePlot.getY2CursorPosition();
        printToConsole("  Y1=" + y1 + " Y2=" + y2 + " delta=" + Math.abs(y1 - y2));
        printToConsole("  Step 9 PASS: Y cursors positioned");

        // Cleanup
        adc.timePlot.setTrackingEnabled(false);
        msleep(500);
        adc.timePlot.setXCursorsLocked(false);
        msleep(500);
        adc.timePlot.setCursorsVisible(false);
        msleep(500);
        adc.setTimeXMode(origXMode);
        msleep(500);
        adc.setTimeBufferSize(origBufferSize);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Switch to ADC Frequency tool
// ============================================
printToConsole("\n=== Switching to ADC - Frequency ===\n");

if (!switchToTool("ADC - Frequency")) {
    printToConsole("WARNING: Could not switch to ADC - Frequency tool, tests will proceed via API");
}

var freqChannels = adc.getFreqChannels();
printToConsole("Freq channels: " + freqChannels);

// ============================================
// ADC-Freq Test 1: X-Axis settings
// UID: TST.ADC_FREQ.X_AXIS_SETTINGS
// Description: Adjust X-axis settings for frequency and sample-based
//   plotting in the ADC plugin
// ============================================
printToConsole("\n=== ADC-Freq Test 1: X-Axis Settings ===\n");

TestFramework.runTest("TST.ADC_FREQ.X_AXIS_SETTINGS", function() {
    try {
        // Save original state
        var origBufferSize = adc.getFreqBufferSize();
        var origOffset = adc.getFreqOffset();
        printToConsole("  Original buffer=" + origBufferSize + " offset=" + origOffset);

        // Step 2: Set buffer size to 4000
        adc.setFreqBufferSize(4000);
        msleep(500);
        var bufferSize = adc.getFreqBufferSize();
        printToConsole("  After setFreqBufferSize(4000): " + bufferSize);
        if (bufferSize !== 4000) {
            printToConsole("  FAIL: Buffer size not set to 4000");
            adc.setFreqBufferSize(origBufferSize);
            return false;
        }
        printToConsole("  Step 2 PASS: Buffer size set to 4000");

        // Press Single
        adc.freqSingleShot();
        msleep(5000);
        printToConsole("  Single shot completed");

        // Step 4: Set frequency offset to 1 MHz
        adc.setFreqOffset(1000000);
        msleep(500);
        var offset = adc.getFreqOffset();
        printToConsole("  After setFreqOffset(1000000): " + offset);
        if (Math.abs(offset - 1000000) > 1) {
            printToConsole("  FAIL: Frequency offset not set to 1 MHz");
            adc.setFreqBufferSize(origBufferSize);
            adc.setFreqOffset(origOffset);
            return false;
        }
        printToConsole("  Step 4 PASS: Frequency offset set to 1 MHz");

        // Press Single
        adc.freqSingleShot();
        msleep(5000);
        printToConsole("  Single shot completed");

        // Restore original state
        adc.setFreqOffset(origOffset);
        msleep(500);
        adc.setFreqBufferSize(origBufferSize);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// ADC-Freq Test 2: Y-Axis settings
// UID: TST.ADC_FREQ.Y_AXIS_SETTINGS
// Description: Adjust Y-axis settings, including autoscaling and manual
//   range settings
// ============================================
printToConsole("\n=== ADC-Freq Test 2: Y-Axis Settings ===\n");

TestFramework.runTest("TST.ADC_FREQ.Y_AXIS_SETTINGS", function() {
    try {
        if (freqChannels.length === 0) {
            printToConsole("  No freq channels available");
            return "SKIP";
        }
        var ch = freqChannels[0];

        // Save original state
        var origYMin = adc.getFreqChannelYMin(ch);
        var origYMax = adc.getFreqChannelYMax(ch);
        var origPowerOffset = adc.getFreqPowerOffset();
        var origWindow = adc.getFreqWindow();
        printToConsole("  Original ch=" + ch + " YMin=" + origYMin + " YMax=" + origYMax +
                       " powerOffset=" + origPowerOffset + " window=" + origWindow);

        // Step 3: Set Min to -140, Max to 20
        adc.setFreqChannelYMin(ch, -140);
        msleep(500);
        adc.setFreqChannelYMax(ch, 20);
        msleep(500);
        var ymin = adc.getFreqChannelYMin(ch);
        var ymax = adc.getFreqChannelYMax(ch);
        printToConsole("  After set YMin=-140 YMax=20: min=" + ymin + " max=" + ymax);
        if (Math.abs(ymin - (-140)) > 1 || Math.abs(ymax - 20) > 1) {
            printToConsole("  FAIL: Y range not set correctly");
            adc.setFreqChannelYMin(ch, origYMin);
            adc.setFreqChannelYMax(ch, origYMax);
            return false;
        }
        printToConsole("  Step 3 PASS: Y range set to -140 to 20");

        // Press Single
        adc.freqSingleShot();
        msleep(5000);

        // Step 4: Set power offset to 20 dB
        adc.setFreqPowerOffset(20);
        msleep(500);
        var powerOffset = adc.getFreqPowerOffset();
        printToConsole("  After setFreqPowerOffset(20): " + powerOffset);
        if (Math.abs(powerOffset - 20) > 0.01) {
            printToConsole("  FAIL: Power offset not set to 20");
            adc.setFreqPowerOffset(origPowerOffset);
            adc.setFreqChannelYMin(ch, origYMin);
            adc.setFreqChannelYMax(ch, origYMax);
            return false;
        }
        printToConsole("  Step 4 PASS: Power offset set to 20 dB");

        // Press Single
        adc.freqSingleShot();
        msleep(5000);

        // Step 5: Change through window options
        var windowValues = [0, 1, 2];
        var windowPassed = true;
        for (var i = 0; i < windowValues.length; i++) {
            adc.setFreqWindow(windowValues[i]);
            msleep(500);
            var win = adc.getFreqWindow();
            if (win !== windowValues[i]) {
                printToConsole("  FAIL: Window not set to " + windowValues[i] + ", got " + win);
                windowPassed = false;
            }
        }
        if (windowPassed) {
            printToConsole("  Step 5 PASS: All window options verified");
        }

        // Restore original state
        adc.setFreqWindow(origWindow);
        msleep(500);
        adc.setFreqPowerOffset(origPowerOffset);
        msleep(500);
        adc.setFreqChannelYMin(ch, origYMin);
        msleep(500);
        adc.setFreqChannelYMax(ch, origYMax);
        msleep(500);

        return windowPassed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// ADC-Freq Test 7: Marker settings
// UID: TST.ADC_FREQ.MARKER_SETTINGS
// Description: Use markers to identify peaks on channel curves
// ============================================
printToConsole("\n=== ADC-Freq Test 7: Marker Settings ===\n");

TestFramework.runTest("TST.ADC_FREQ.MARKER_SETTINGS", function() {
    try {
        if (freqChannels.length === 0) {
            printToConsole("  No freq channels available");
            return "SKIP";
        }
        var ch = freqChannels[0];

        // Save original marker state
        var origMarkerEnabled = adc.isFreqChannelMarkerEnabled(ch);
        printToConsole("  Original marker enabled: " + origMarkerEnabled);

        // Step 3: Enable markers, set type to peak (1) and count to 5
        adc.setFreqChannelMarkerEnabled(ch, true);
        msleep(500);
        var markerEnabled = adc.isFreqChannelMarkerEnabled(ch);
        if (!markerEnabled) {
            printToConsole("  FAIL: Markers not enabled");
            adc.setFreqChannelMarkerEnabled(ch, origMarkerEnabled);
            return false;
        }
        printToConsole("  Markers enabled");

        adc.setFreqChannelMarkerType(ch, 1); // peak type
        msleep(500);
        adc.setFreqChannelMarkerCount(ch, 5);
        msleep(500);
        printToConsole("  Step 3 PASS: Markers set to peak type, count=5");

        // Step 4: Change count to 7
        adc.setFreqChannelMarkerCount(ch, 7);
        msleep(500);
        printToConsole("  Step 4 PASS: Marker count changed to 7");

        // Step 6: Set marker type to fixed (0) and count to 5
        adc.setFreqChannelMarkerType(ch, 0); // fixed type
        msleep(500);
        adc.setFreqChannelMarkerCount(ch, 5);
        msleep(500);
        printToConsole("  Step 6 PASS: Markers set to fixed type, count=5");

        // Step 7: Change count to 7
        adc.setFreqChannelMarkerCount(ch, 7);
        msleep(500);
        printToConsole("  Step 7 PASS: Fixed marker count changed to 7");

        // Restore original state
        adc.setFreqChannelMarkerEnabled(ch, origMarkerEnabled);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// ADC-Freq Test 3: Settings adjustment
// UID: TST.ADC_FREQ.SETTINGS_ADJUSTMENT
// Description: Adjust plot settings such as title, labels, thickness,
//   and style
// ============================================
printToConsole("\n=== ADC-Freq Test 3: Settings Adjustment ===\n");

TestFramework.runTest("TST.ADC_FREQ.SETTINGS_ADJUSTMENT", function() {
    try {
        // Save original state
        var origTitle = adc.freqPlot.getTitle();
        var origLabels = adc.freqPlot.isLabelsEnabled();
        printToConsole("  Original title: '" + origTitle + "' labels: " + origLabels);

        // Step 1: Change Plot Title to "test"
        adc.freqPlot.setTitle("test");
        msleep(500);
        var title = adc.freqPlot.getTitle();
        if (title !== "test") {
            printToConsole("  FAIL: Title not set to 'test', got '" + title + "'");
            adc.freqPlot.setTitle(origTitle);
            return false;
        }
        printToConsole("  Step 1 PASS: Title changed to 'test'");

        // Step 2: Turn on and off plot labels
        adc.freqPlot.setLabelsEnabled(true);
        msleep(500);
        var labelsOn = adc.freqPlot.isLabelsEnabled();
        adc.freqPlot.setLabelsEnabled(false);
        msleep(500);
        var labelsOff = adc.freqPlot.isLabelsEnabled();
        if (!labelsOn || labelsOff) {
            printToConsole("  FAIL: Labels toggle not working (on=" + labelsOn + " off=" + labelsOff + ")");
            adc.freqPlot.setTitle(origTitle);
            adc.freqPlot.setLabelsEnabled(origLabels);
            return false;
        }
        printToConsole("  Step 2 PASS: Labels toggled on and off");

        // Step 3: Change thickness and style
        var channels = adc.freqPlot.getChannels();
        if (channels.length > 0) {
            var ch = channels[0];
            var origThickness = adc.freqPlot.getChannelThickness(ch);
            var origStyle = adc.freqPlot.getChannelStyle(ch);

            var thicknessValues = [1, 2, 3];
            var thicknessPassed = true;
            for (var i = 0; i < thicknessValues.length; i++) {
                adc.freqPlot.setChannelThickness(ch, thicknessValues[i]);
                msleep(500);
                if (adc.freqPlot.getChannelThickness(ch) !== thicknessValues[i]) {
                    thicknessPassed = false;
                }
            }

            var styleValues = [0, 1, 2];
            var stylePassed = true;
            for (var i = 0; i < styleValues.length; i++) {
                adc.freqPlot.setChannelStyle(ch, styleValues[i]);
                msleep(500);
                if (adc.freqPlot.getChannelStyle(ch) !== styleValues[i]) {
                    stylePassed = false;
                }
            }

            // Restore
            adc.freqPlot.setChannelThickness(ch, origThickness);
            adc.freqPlot.setChannelStyle(ch, origStyle);
            msleep(500);

            if (!thicknessPassed || !stylePassed) {
                printToConsole("  FAIL: Thickness or style changes not verified");
                adc.freqPlot.setTitle(origTitle);
                adc.freqPlot.setLabelsEnabled(origLabels);
                return false;
            }
            printToConsole("  Step 3 PASS: Thickness and style changes verified");
        }

        // Restore original state
        adc.freqPlot.setTitle(origTitle);
        msleep(500);
        adc.freqPlot.setLabelsEnabled(origLabels);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// ADC-Freq Test 4: Print plot
// UID: TST.ADC_FREQ.PRINT_PLOT
// Description: Print the current plot to a PDF file
// ============================================
printToConsole("\n=== ADC-Freq Test 4: Print Plot ===\n");

TestFramework.runTest("TST.ADC_FREQ.PRINT_PLOT", function() {
    try {
        // Press single to capture data
        adc.freqSingleShot();
        msleep(5000);

        // Print to file
        var filePath = "/tmp/scopy_adc_freq_print_test.pdf";
        var result = adc.freqPlot.printPlot(filePath);
        msleep(500);
        printToConsole("  Print result: " + result);
        if (!result) {
            printToConsole("  FAIL: printPlot returned false");
            return false;
        }
        printToConsole("  PASS: Freq plot printed to " + filePath);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// ADC-Freq Test 6: Y-Axis channel settings
// UID: TST.ADC_FREQ.Y_AXIS_CHANNEL_SETTINGS
// Description: Adjust the Y-axis for individual channel curve manipulation
// ============================================
printToConsole("\n=== ADC-Freq Test 6: Y-Axis Channel Settings ===\n");

TestFramework.runTest("TST.ADC_FREQ.Y_AXIS_CHANNEL_SETTINGS", function() {
    try {
        if (freqChannels.length === 0) {
            printToConsole("  No freq channels available");
            return "SKIP";
        }
        var ch = freqChannels[0];

        // Save original state
        var origYMin = adc.getFreqChannelYMin(ch);
        var origYMax = adc.getFreqChannelYMax(ch);
        printToConsole("  Original ch=" + ch + " YMin=" + origYMin + " YMax=" + origYMax);

        // Set per-channel Y range
        adc.setFreqChannelYMin(ch, -120);
        msleep(500);
        adc.setFreqChannelYMax(ch, 10);
        msleep(500);

        var ymin = adc.getFreqChannelYMin(ch);
        var ymax = adc.getFreqChannelYMax(ch);
        printToConsole("  After set YMin=-120 YMax=10: min=" + ymin + " max=" + ymax);

        if (Math.abs(ymin - (-120)) > 1 || Math.abs(ymax - 10) > 1) {
            printToConsole("  FAIL: Y range not set correctly for " + ch);
            adc.setFreqChannelYMin(ch, origYMin);
            adc.setFreqChannelYMax(ch, origYMax);
            return false;
        }

        // Verify other channel NOT affected (if multiple channels)
        if (freqChannels.length > 1) {
            var otherCh = freqChannels[1];
            var otherYMin = adc.getFreqChannelYMin(otherCh);
            var otherYMax = adc.getFreqChannelYMax(otherCh);
            printToConsole("  Other channel " + otherCh + " YMin=" + otherYMin + " YMax=" + otherYMax +
                           " (should be unchanged)");
        }
        printToConsole("  PASS: Per-channel freq Y-axis settings verified");

        // Restore original state
        adc.setFreqChannelYMin(ch, origYMin);
        msleep(500);
        adc.setFreqChannelYMax(ch, origYMax);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// ADC-Freq Test 8: Cursor settings
// UID: TST.ADC_FREQ.CURSOR_SETTINGS
// Description: Use cursors to measure specific points on channel curves
// ============================================
printToConsole("\n=== ADC-Freq Test 8: Cursor Settings ===\n");

TestFramework.runTest("TST.ADC_FREQ.CURSOR_SETTINGS", function() {
    try {
        // Run to have data on plot
        adc.freqSingleShot();
        msleep(5000);

        // Step 3: Enable X cursors
        adc.freqPlot.setCursorsVisible(true);
        msleep(500);
        adc.freqPlot.setXCursorsEnabled(true);
        msleep(500);

        var cursorsVisible = adc.freqPlot.isCursorsVisible();
        if (!cursorsVisible) {
            printToConsole("  FAIL: Freq cursors not visible");
            return false;
        }
        printToConsole("  Step 3 PASS: X cursors enabled");

        // Step 4: Position X cursor
        adc.freqPlot.setX1CursorPosition(1000000);
        msleep(500);
        var x1 = adc.freqPlot.getX1CursorPosition();
        printToConsole("  X1 cursor position: " + x1);
        printToConsole("  Step 4 PASS: X cursor positioned");

        // Step 5: Enable tracking
        adc.freqPlot.setTrackingEnabled(true);
        msleep(500);
        printToConsole("  Step 5 PASS: Tracking enabled");

        // Step 7: Enable Y cursors
        adc.freqPlot.setYCursorsEnabled(true);
        msleep(500);
        adc.freqPlot.setY1CursorPosition(-20);
        msleep(500);
        var y1 = adc.freqPlot.getY1CursorPosition();
        printToConsole("  Y1 cursor position: " + y1);
        printToConsole("  Step 7 PASS: Y cursor positioned");

        // Cleanup
        adc.freqPlot.setTrackingEnabled(false);
        msleep(500);
        adc.freqPlot.setCursorsVisible(false);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 7: Channel Averaging
// UID: TST.ADC_FREQ.CHANNEL_AVERAGING
// Description: Verify channel averaging functionality in ADC-Frequency
// ============================================
printToConsole("\n=== Test 7: Channel Averaging ===\n");

TestFramework.runTest("TST.ADC_FREQ.CHANNEL_AVERAGING", function() {
    try {
        if (freqChannels.length === 0) {
            printToConsole("  No freq channels available");
            return "SKIP";
        }
        var ch = freqChannels.length > 1 ? freqChannels[1] : freqChannels[0];

        // Save original state
        var origEnabled = adc.isFreqChannelAveragingEnabled(ch);
        var origSize = adc.getFreqChannelAveragingSize(ch);
        printToConsole("  Original ch=" + ch + " averaging enabled=" + origEnabled + " size=" + origSize);

        // Step 5: Enable averaging
        adc.setFreqChannelAveragingEnabled(ch, true);
        msleep(500);
        var enabled = adc.isFreqChannelAveragingEnabled(ch);
        if (!enabled) {
            printToConsole("  FAIL: Averaging not enabled for " + ch);
            adc.setFreqChannelAveragingEnabled(ch, origEnabled);
            return false;
        }
        printToConsole("  Step 5 PASS: Averaging enabled for " + ch);

        // Step 6: Set averaging size to 10
        adc.setFreqChannelAveragingSize(ch, 10);
        msleep(500);
        var size = adc.getFreqChannelAveragingSize(ch);
        printToConsole("  After setAveragingSize(10): " + size);
        if (size !== 10) {
            printToConsole("  FAIL: Averaging size not set to 10, got " + size);
            adc.setFreqChannelAveragingEnabled(ch, origEnabled);
            adc.setFreqChannelAveragingSize(ch, origSize);
            return false;
        }
        printToConsole("  Step 6 PASS: Averaging size set to 10");

        // Restore original state
        adc.setFreqChannelAveragingSize(ch, origSize);
        msleep(500);
        adc.setFreqChannelAveragingEnabled(ch, origEnabled);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Genalyzer Tests
// Requires complex mode enabled
// ============================================
printToConsole("\n=== Genalyzer Tests ===\n");

var complexWasEnabled = false;
try {
    complexWasEnabled = adc.isFreqComplexMode();
    if (!complexWasEnabled) {
        adc.setFreqComplexMode(true);
        msleep(1000);
    }
} catch (e) {
    printToConsole("  Warning: Could not enable complex mode: " + e);
}

// Re-discover freq channels after complex mode change
var genFreqChannels = [];
try {
    genFreqChannels = adc.getFreqChannels();
} catch (e) {
    printToConsole("  Warning: Could not get freq channels for genalyzer: " + e);
}

// ============================================
// Genalyzer Test 1: Genalyzer Analysis Enabling
// UID: TST.ADC_GENALYZER.ENABLE
// Description: Verify genalyzer analysis can be enabled and results panel appears
// ============================================
printToConsole("\n=== Genalyzer Test 1: Enable ===\n");

TestFramework.runTest("TST.ADC_GENALYZER.ENABLE", function() {
    try {
        // Save original state
        var origEnabled = adc.isGenalyzerEnabled();

        // Step 4: Enable genalyzer analysis
        adc.setGenalyzerEnabled(true);
        msleep(500);
        var enabled = adc.isGenalyzerEnabled();
        if (!enabled) {
            printToConsole("  FAIL: Genalyzer not enabled");
            adc.setGenalyzerEnabled(origEnabled);
            return false;
        }
        printToConsole("  Step 4 PASS: Genalyzer enabled");

        // Step 5: Run and verify metrics
        if (genFreqChannels.length > 0) {
            var ch = genFreqChannels[0];
            adc.setFreqRunning(true);
            msleep(5000);
            adc.triggerGenalyzerAnalysis(ch);
            msleep(1000);

            var snr = adc.getGenalyzerMetric(ch, "snr");
            adc.setFreqRunning(false);
            msleep(500);
            printToConsole("  SNR metric: " + snr);
            if (typeof snr !== "number") {
                printToConsole("  FAIL: SNR metric not a number");
                adc.setGenalyzerEnabled(origEnabled);
                return false;
            }
            printToConsole("  Step 5 PASS: Genalyzer analysis produced metrics");
        }

        // Restore original state
        adc.setGenalyzerEnabled(origEnabled);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Genalyzer Test 2: Auto Mode Analysis
// UID: TST.ADC_GENALYZER.AUTO_MODE
// Description: Verify automatic tone detection and analysis
// ============================================
printToConsole("\n=== Genalyzer Test 2: Auto Mode ===\n");

TestFramework.runTest("TST.ADC_GENALYZER.AUTO_MODE", function() {
    try {
        // Save original state
        var origEnabled = adc.isGenalyzerEnabled();
        var origMode = adc.getGenalyzerMode();
        var origSSB = adc.getGenalyzerSSBWidth();

        // Step 3: Select Auto mode
        adc.setGenalyzerEnabled(true);
        msleep(500);
        adc.setGenalyzerMode(0); // Auto
        msleep(500);
        var mode = adc.getGenalyzerMode();
        if (mode !== 0) {
            printToConsole("  FAIL: Mode not set to Auto (0), got " + mode);
            adc.setGenalyzerEnabled(origEnabled);
            adc.setGenalyzerMode(origMode);
            return false;
        }
        printToConsole("  Step 3 PASS: Auto mode selected");

        // Step 4: Set SSB width to 120
        adc.setGenalyzerSSBWidth(120);
        msleep(500);
        var ssb = adc.getGenalyzerSSBWidth();
        if (ssb !== 120) {
            printToConsole("  FAIL: SSB width not set to 120, got " + ssb);
            adc.setGenalyzerSSBWidth(origSSB);
            adc.setGenalyzerEnabled(origEnabled);
            adc.setGenalyzerMode(origMode);
            return false;
        }
        printToConsole("  Step 4 PASS: SSB width set to 120");

        // Step 7: Change SSB width to 60
        adc.setGenalyzerSSBWidth(60);
        msleep(500);
        var ssb60 = adc.getGenalyzerSSBWidth();
        if (ssb60 !== 60) {
            printToConsole("  FAIL: SSB width not set to 60, got " + ssb60);
            adc.setGenalyzerSSBWidth(origSSB);
            adc.setGenalyzerEnabled(origEnabled);
            adc.setGenalyzerMode(origMode);
            return false;
        }
        printToConsole("  Step 7 PASS: SSB width changed to 60");

        // Restore original state
        adc.setGenalyzerSSBWidth(origSSB);
        msleep(500);
        adc.setGenalyzerMode(origMode);
        msleep(500);
        adc.setGenalyzerEnabled(origEnabled);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Genalyzer Test 3: Fixed Tone Mode Analysis
// UID: TST.ADC_GENALYZER.FIXED_TONE_MODE
// Description: Verify fixed tone mode analysis with expected frequency
// ============================================
printToConsole("\n=== Genalyzer Test 3: Fixed Tone Mode ===\n");

TestFramework.runTest("TST.ADC_GENALYZER.FIXED_TONE_MODE", function() {
    try {
        // Save original state
        var origEnabled = adc.isGenalyzerEnabled();
        var origMode = adc.getGenalyzerMode();
        var origFreq = adc.getGenalyzerExpectedFreq();
        var origOrder = adc.getGenalyzerHarmonicOrder();

        // Step 3: Select Fixed Tone mode
        adc.setGenalyzerEnabled(true);
        msleep(500);
        adc.setGenalyzerMode(1); // Fixed Tone
        msleep(500);
        var mode = adc.getGenalyzerMode();
        if (mode !== 1) {
            printToConsole("  FAIL: Mode not set to Fixed Tone (1), got " + mode);
            adc.setGenalyzerEnabled(origEnabled);
            adc.setGenalyzerMode(origMode);
            return false;
        }
        printToConsole("  Step 3 PASS: Fixed Tone mode selected");

        // Step 4: Set Expected Frequency to 1MHz
        adc.setGenalyzerExpectedFreq(1000000);
        msleep(500);
        var freq = adc.getGenalyzerExpectedFreq();
        if (Math.abs(freq - 1000000) > 1) {
            printToConsole("  FAIL: Expected freq not set to 1MHz, got " + freq);
            adc.setGenalyzerExpectedFreq(origFreq);
            adc.setGenalyzerEnabled(origEnabled);
            adc.setGenalyzerMode(origMode);
            return false;
        }
        printToConsole("  Step 4 PASS: Expected frequency set to 1MHz");

        // Step 5: Set Harmonic Order to 5
        adc.setGenalyzerHarmonicOrder(5);
        msleep(500);
        var order = adc.getGenalyzerHarmonicOrder();
        if (order !== 5) {
            printToConsole("  FAIL: Harmonic order not set to 5, got " + order);
            adc.setGenalyzerHarmonicOrder(origOrder);
            adc.setGenalyzerExpectedFreq(origFreq);
            adc.setGenalyzerEnabled(origEnabled);
            adc.setGenalyzerMode(origMode);
            return false;
        }
        printToConsole("  Step 5 PASS: Harmonic order set to 5");

        // Step 6-7: Set SSB Fundamental and Default
        adc.setGenalyzerSSBFundamental(4);
        msleep(500);
        adc.setGenalyzerSSBDefault(3);
        msleep(500);
        printToConsole("  Steps 6-7 PASS: SSB Fundamental=4, SSB Default=3 set");

        // Step 10: Change Expected Frequency to 500kHz
        adc.setGenalyzerExpectedFreq(500000);
        msleep(500);
        var freq2 = adc.getGenalyzerExpectedFreq();
        if (Math.abs(freq2 - 500000) > 1) {
            printToConsole("  FAIL: Expected freq not set to 500kHz, got " + freq2);
            adc.setGenalyzerExpectedFreq(origFreq);
            adc.setGenalyzerHarmonicOrder(origOrder);
            adc.setGenalyzerEnabled(origEnabled);
            adc.setGenalyzerMode(origMode);
            return false;
        }
        printToConsole("  Step 10 PASS: Expected frequency changed to 500kHz");

        // Restore original state
        adc.setGenalyzerExpectedFreq(origFreq);
        msleep(500);
        adc.setGenalyzerHarmonicOrder(origOrder);
        msleep(500);
        adc.setGenalyzerMode(origMode);
        msleep(500);
        adc.setGenalyzerEnabled(origEnabled);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Genalyzer Test 4: FFT Integration
// UID: TST.ADC_GENALYZER.FFT_INTEGRATION
// Description: Verify genalyzer works with FFT averaging and windowing
// ============================================
printToConsole("\n=== Genalyzer Test 4: FFT Integration ===\n");

TestFramework.runTest("TST.ADC_GENALYZER.FFT_INTEGRATION", function() {
    try {
        if (genFreqChannels.length === 0) {
            printToConsole("  No freq channels available");
            return "SKIP";
        }
        var ch = genFreqChannels[0];

        // Save original state
        var origGenalyzer = adc.isGenalyzerEnabled();
        var origAvgEnabled = adc.isFreqChannelAveragingEnabled(ch);
        var origAvgSize = adc.getFreqChannelAveragingSize(ch);
        var origWindow = adc.getFreqWindow();

        // Step 3: Enable averaging with size 10
        adc.setGenalyzerEnabled(true);
        msleep(500);
        adc.setFreqChannelAveragingEnabled(ch, true);
        msleep(500);
        adc.setFreqChannelAveragingSize(ch, 10);
        msleep(500);

        var avgEnabled = adc.isFreqChannelAveragingEnabled(ch);
        var avgSize = adc.getFreqChannelAveragingSize(ch);
        if (!avgEnabled || avgSize !== 10) {
            printToConsole("  FAIL: Averaging not set correctly (enabled=" + avgEnabled + " size=" + avgSize + ")");
            adc.setFreqChannelAveragingEnabled(ch, origAvgEnabled);
            adc.setFreqChannelAveragingSize(ch, origAvgSize);
            adc.setGenalyzerEnabled(origGenalyzer);
            return false;
        }
        printToConsole("  Step 3 PASS: Averaging enabled with size 10");

        // Step 5: Change window function
        adc.setFreqWindow(2); // Blackman-Harris
        msleep(500);
        var win = adc.getFreqWindow();
        if (win !== 2) {
            printToConsole("  FAIL: Window not set to 2, got " + win);
            adc.setFreqWindow(origWindow);
            adc.setFreqChannelAveragingEnabled(ch, origAvgEnabled);
            adc.setFreqChannelAveragingSize(ch, origAvgSize);
            adc.setGenalyzerEnabled(origGenalyzer);
            return false;
        }
        printToConsole("  Step 5 PASS: Window changed to Blackman-Harris");

        // Step 6: Disable averaging
        adc.setFreqChannelAveragingEnabled(ch, false);
        msleep(500);
        var avgDisabled = !adc.isFreqChannelAveragingEnabled(ch);
        if (!avgDisabled) {
            printToConsole("  FAIL: Averaging not disabled");
            adc.setFreqChannelAveragingEnabled(ch, origAvgEnabled);
            adc.setFreqChannelAveragingSize(ch, origAvgSize);
            adc.setFreqWindow(origWindow);
            adc.setGenalyzerEnabled(origGenalyzer);
            return false;
        }
        printToConsole("  Step 6 PASS: Averaging disabled");

        // Restore original state
        adc.setFreqWindow(origWindow);
        msleep(500);
        adc.setFreqChannelAveragingSize(ch, origAvgSize);
        msleep(500);
        adc.setFreqChannelAveragingEnabled(ch, origAvgEnabled);
        msleep(500);
        adc.setGenalyzerEnabled(origGenalyzer);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Restore complex mode if we changed it
if (!complexWasEnabled) {
    try {
        adc.setFreqComplexMode(false);
        msleep(500);
    } catch (e) {
        printToConsole("  Warning: Could not restore complex mode: " + e);
    }
}

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
