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

// ============================================================================
// ALL THE FOLLOWING TESTS REQUIRE VISUAL VALIDATION
// These tests automate the steps from the manual test documentation but
// require a human observer to verify UI changes. After each visual check,
// the script prompts for user feedback (y/n) to determine pass/fail.
// Source: docs/tests/plugins/adc/adc_tests.rst
// ============================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("ADC Visual Validation Tests");

// Helper: prompt user for visual verification
// Returns true if user confirms (y), false if user rejects (n)
function visualCheck(message) {
    printToConsole("  VISUAL CHECK: " + message);
    var response = readFromConsole("  Pass? (y/n): ");
    if (response === "n" || response === "N") {
        printToConsole("  FAIL: Visual check rejected by user");
        return false;
    }
    return true;
}

// Connect to device
if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to ADC Time tool
if (!switchToTool("ADC - Time")) {
    printToConsole("WARNING: Could not switch to ADC - Time tool");
}

var timeChannels = adc.getTimeChannels();
printToConsole("Time channels: " + timeChannels);

// ============================================
// Test 1: X-AXIS (Visual)
// UID: TST.ADC_TIME.X_AXIS
// ============================================
printToConsole("\n=== Test 1: X-AXIS (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_TIME.X_AXIS", function() {
    try {
        var passed = true;
        var origBufferSize = adc.getTimeBufferSize();
        var origPlotSize = adc.getTimePlotSize();
        var origSync = adc.isTimeSyncBufferPlot();
        var origRolling = adc.isTimeRollingMode();

        printToConsole("  Setting buffer size to 100...");
        adc.setTimeBufferSize(100);
        msleep(500);
        adc.timeSingleShot();
        msleep(5000);
        if (!visualCheck("Plot labels should go up to 100, channel curves should span the whole plot")) passed = false;

        printToConsole("  Turning off sync, setting plot size to 200...");
        adc.setTimeSyncBufferPlot(false);
        msleep(500);
        adc.setTimePlotSize(200);
        msleep(500);
        adc.timeSingleShot();
        msleep(5000);
        if (!visualCheck("Plot labels should go up to 200, channel curves should span the whole plot")) passed = false;

        printToConsole("  Enabling rolling mode...");
        adc.setTimeRollingMode(true);
        msleep(500);
        if (!visualCheck("Plot labels should invert, going from 200 to 0")) passed = false;

        adc.setTimeRollingMode(origRolling);
        msleep(500);
        adc.setTimeSyncBufferPlot(origSync);
        msleep(500);
        adc.setTimePlotSize(origPlotSize);
        msleep(500);
        adc.setTimeBufferSize(origBufferSize);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2: Y-AXIS (Visual)
// UID: TST.ADC_TIME.Y_AXIS
// ============================================
printToConsole("\n=== Test 2: Y-AXIS (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_TIME.Y_AXIS", function() {
    try {
        if (timeChannels.length === 0) {
            printToConsole("  No time channels available");
            return "SKIP";
        }
        var passed = true;
        var ch = timeChannels[0];
        var origYMode = adc.getTimeChannelYMode(ch);
        var origYMin = adc.getTimeChannelYMin(ch);
        var origYMax = adc.getTimeChannelYMax(ch);

        printToConsole("  Setting YMode to ADC Counts, range=-4000 to 4000...");
        adc.setTimeChannelYMode(ch, 0);
        msleep(500);
        adc.setTimeChannelYMin(ch, -4000);
        msleep(500);
        adc.setTimeChannelYMax(ch, 4000);
        msleep(500);
        adc.timeSingleShot();
        msleep(5000);
        if (!visualCheck("Y-axis labels should range from -4000 to 4000, curves should vertically adjust")) passed = false;

        adc.setTimeChannelYMode(ch, origYMode);
        msleep(500);
        adc.setTimeChannelYMin(ch, origYMin);
        msleep(500);
        adc.setTimeChannelYMax(ch, origYMax);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 8: ADC-Time Channel Settings (Visual)
// UID: TST.ADC_TIME_CHANNEL_SETTINGS
// ============================================
printToConsole("\n=== Test 8: ADC-Time Channel Settings (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_TIME_CHANNEL_SETTINGS", function() {
    try {
        if (timeChannels.length === 0) {
            printToConsole("  No time channels available");
            return "SKIP";
        }
        var passed = true;
        var ch = timeChannels[0];
        var origYMode = adc.getTimeChannelYMode(ch);
        var origYMin = adc.getTimeChannelYMin(ch);
        var origYMax = adc.getTimeChannelYMax(ch);

        printToConsole("  Setting " + ch + " YMode=ADC Counts, range=-4000 to 4000...");
        adc.setTimeChannelYMode(ch, 0);
        msleep(500);
        adc.setTimeChannelYMin(ch, -4000);
        msleep(500);
        adc.setTimeChannelYMax(ch, 4000);
        msleep(500);
        adc.timeSingleShot();
        msleep(5000);
        if (!visualCheck("Only " + ch + " curve should vertically adjust, other channels unchanged")) passed = false;

        adc.setTimeChannelYMode(ch, origYMode);
        msleep(500);
        adc.setTimeChannelYMin(ch, origYMin);
        msleep(500);
        adc.setTimeChannelYMax(ch, origYMax);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3: XY PLOT (Visual)
// UID: TST.ADC_TIME.XY_PLOT
// ============================================
printToConsole("\n=== Test 3: XY PLOT (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_TIME.XY_PLOT", function() {
    try {
        var passed = true;
        var origEnabled = adc.isTimeXYPlotEnabled();

        printToConsole("  Enabling XY plot...");
        adc.setTimeXYPlotEnabled(true);
        msleep(500);
        if (!visualCheck("A new plot should appear on the right")) passed = false;

        if (timeChannels.length >= 1) {
            var origSource = adc.getTimeXYSource();

            printToConsole("  Setting XY source to " + timeChannels[0] + "...");
            adc.setTimeXYSource(timeChannels[0]);
            msleep(500);
            if (!visualCheck("A curve should appear on the XY plot")) passed = false;

            if (timeChannels.length >= 2) {
                printToConsole("  Setting XY source to " + timeChannels[1] + "...");
                adc.setTimeXYSource(timeChannels[1]);
                msleep(500);
                if (!visualCheck("A different curve should appear on the XY plot")) passed = false;
            }

            adc.setTimeXYSource(origSource);
            msleep(500);
        }

        adc.setTimeXYPlotEnabled(origEnabled);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: Settings (Visual)
// UID: TST.ADC_TIME.SETTINGS
// ============================================
printToConsole("\n=== Test 4: Settings (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_TIME.SETTINGS", function() {
    try {
        var passed = true;
        var origTitle = adc.timePlot.getTitle();
        var origLabels = adc.timePlot.isLabelsEnabled();

        printToConsole("  Setting title to 'test'...");
        adc.timePlot.setTitle("test");
        msleep(500);
        if (!visualCheck("Text in top left should change to 'test'")) passed = false;

        printToConsole("  Enabling plot labels...");
        adc.timePlot.setLabelsEnabled(true);
        msleep(500);
        if (!visualCheck("Labels on X and Y axis should appear")) passed = false;

        printToConsole("  Disabling plot labels...");
        adc.timePlot.setLabelsEnabled(false);
        msleep(500);
        if (!visualCheck("Labels should disappear")) passed = false;

        var channels = adc.timePlot.getChannels();
        if (channels.length > 0) {
            var ch = channels[0];
            var origThickness = adc.timePlot.getChannelThickness(ch);
            var origStyle = adc.timePlot.getChannelStyle(ch);

            printToConsole("  Setting thickness to 3...");
            adc.timePlot.setChannelThickness(ch, 3);
            msleep(500);
            if (!visualCheck("Channel curve should appear thicker")) passed = false;

            printToConsole("  Setting style to 1 (dashed)...");
            adc.timePlot.setChannelStyle(ch, 1);
            msleep(500);
            if (!visualCheck("Channel curve should appear dashed")) passed = false;

            adc.timePlot.setChannelThickness(ch, origThickness);
            adc.timePlot.setChannelStyle(ch, origStyle);
            msleep(500);
        }

        adc.timePlot.setTitle(origTitle);
        msleep(500);
        adc.timePlot.setLabelsEnabled(origLabels);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 10: Cursors (Visual)
// UID: TST.CURSORS
// ============================================
printToConsole("\n=== Test 10: Cursors (VISUAL) ===\n");

TestFramework.runTest("TST.CURSORS", function() {
    try {
        var passed = true;
        var origBufferSize = adc.getTimeBufferSize();
        var origXMode = adc.getTimeXMode();

        adc.setTimeBufferSize(200);
        msleep(500);
        adc.setTimeXMode(0);
        msleep(500);
        adc.timeSingleShot();
        msleep(5000);

        printToConsole("  Enabling X and Y cursors...");
        adc.timePlot.setCursorsVisible(true);
        msleep(500);
        adc.timePlot.setXCursorsEnabled(true);
        msleep(500);
        adc.timePlot.setYCursorsEnabled(true);
        msleep(500);
        if (!visualCheck("X and Y cursors should be visible on plot")) passed = false;

        printToConsole("  Positioning X cursors at 50 and 100...");
        adc.timePlot.setX1CursorPosition(50);
        msleep(500);
        adc.timePlot.setX2CursorPosition(100);
        msleep(500);
        if (!visualCheck("X cursors should be at samples 50 and 100")) passed = false;

        printToConsole("  Locking X cursors...");
        adc.timePlot.setXCursorsLocked(true);
        msleep(500);
        if (!visualCheck("Cursors should be locked (move together)")) passed = false;

        printToConsole("  Enabling tracking...");
        adc.timePlot.setTrackingEnabled(true);
        msleep(500);
        if (!visualCheck("Markers should appear on channel curve synced with X cursors")) passed = false;

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

        return passed;
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
    printToConsole("WARNING: Could not switch to ADC - Frequency tool");
}

var freqChannels = adc.getFreqChannels();
printToConsole("Freq channels: " + freqChannels);

// ============================================
// ADC-Freq Test 1: X-Axis Settings (Visual)
// UID: TST.ADC_FREQ.X_AXIS_SETTINGS
// ============================================
printToConsole("\n=== ADC-Freq Test 1: X-Axis Settings (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_FREQ.X_AXIS_SETTINGS", function() {
    try {
        var passed = true;
        var origBufferSize = adc.getFreqBufferSize();
        var origOffset = adc.getFreqOffset();

        printToConsole("  Setting buffer size to 4000...");
        adc.setFreqBufferSize(4000);
        msleep(500);
        adc.freqSingleShot();
        msleep(5000);
        if (!visualCheck("Plot labels should go up to 2000, channel curves should span the whole plot")) passed = false;

        printToConsole("  Setting frequency offset to 1 MHz...");
        adc.setFreqOffset(1000000);
        msleep(500);
        adc.freqSingleShot();
        msleep(5000);
        if (!visualCheck("Curves should move 1 MHz to the left")) passed = false;

        adc.setFreqOffset(origOffset);
        msleep(500);
        adc.setFreqBufferSize(origBufferSize);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// ADC-Freq Test 2: Y-Axis Settings (Visual)
// UID: TST.ADC_FREQ.Y_AXIS_SETTINGS
// ============================================
printToConsole("\n=== ADC-Freq Test 2: Y-Axis Settings (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_FREQ.Y_AXIS_SETTINGS", function() {
    try {
        if (freqChannels.length === 0) {
            printToConsole("  No freq channels available");
            return "SKIP";
        }
        var passed = true;
        var ch = freqChannels[0];
        var origYMin = adc.getFreqChannelYMin(ch);
        var origYMax = adc.getFreqChannelYMax(ch);
        var origPowerOffset = adc.getFreqPowerOffset();
        var origWindow = adc.getFreqWindow();

        printToConsole("  Setting Y range to -140 to 20...");
        adc.setFreqChannelYMin(ch, -140);
        msleep(500);
        adc.setFreqChannelYMax(ch, 20);
        msleep(500);
        adc.freqSingleShot();
        msleep(5000);
        if (!visualCheck("Y-axis labels should be from -140 to 20, curves should vertically adjust")) passed = false;

        printToConsole("  Setting power offset to 20 dB...");
        adc.setFreqPowerOffset(20);
        msleep(500);
        adc.freqSingleShot();
        msleep(5000);
        if (!visualCheck("Curves should move 20 dB up")) passed = false;

        printToConsole("  Cycling through window options...");
        adc.setFreqRunning(true);
        msleep(2000);

        adc.setFreqWindow(0);
        msleep(500);
        if (!visualCheck("Window 0 applied - curves should change slightly")) passed = false;

        adc.setFreqWindow(1);
        msleep(500);
        if (!visualCheck("Window 1 applied - curves should change slightly")) passed = false;

        adc.setFreqWindow(2);
        msleep(500);
        if (!visualCheck("Window 2 applied - curves should change slightly")) passed = false;

        adc.setFreqRunning(false);
        msleep(500);

        adc.setFreqWindow(origWindow);
        msleep(500);
        adc.setFreqPowerOffset(origPowerOffset);
        msleep(500);
        adc.setFreqChannelYMin(ch, origYMin);
        msleep(500);
        adc.setFreqChannelYMax(ch, origYMax);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// ADC-Freq Test 7: Marker Settings (Visual)
// UID: TST.ADC_FREQ.MARKER_SETTINGS
// ============================================
printToConsole("\n=== ADC-Freq Test 7: Marker Settings (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_FREQ.MARKER_SETTINGS", function() {
    try {
        if (freqChannels.length === 0) {
            printToConsole("  No freq channels available");
            return "SKIP";
        }
        var passed = true;
        var ch = freqChannels[0];
        var origMarkerEnabled = adc.isFreqChannelMarkerEnabled(ch);

        adc.setFreqRunning(true);
        msleep(2000);

        printToConsole("  Enabling markers: peak type, count=5...");
        adc.setFreqChannelMarkerEnabled(ch, true);
        msleep(500);
        adc.setFreqChannelMarkerType(ch, 1);
        msleep(500);
        adc.setFreqChannelMarkerCount(ch, 5);
        msleep(500);
        if (!visualCheck("5 markers should appear on " + ch + " curve at the highest peaks")) passed = false;

        printToConsole("  Changing marker count to 7...");
        adc.setFreqChannelMarkerCount(ch, 7);
        msleep(500);
        if (!visualCheck("7 markers should appear on " + ch + " curve at the highest peaks")) passed = false;

        printToConsole("  Setting markers to fixed type, count=5...");
        adc.setFreqChannelMarkerType(ch, 0);
        msleep(500);
        adc.setFreqChannelMarkerCount(ch, 5);
        msleep(500);
        if (!visualCheck("5 movable cursors should appear on the plot")) passed = false;

        printToConsole("  Changing fixed marker count to 7...");
        adc.setFreqChannelMarkerCount(ch, 7);
        msleep(500);
        if (!visualCheck("7 movable cursors should appear on the plot")) passed = false;

        adc.setFreqRunning(false);
        msleep(500);
        adc.setFreqChannelMarkerEnabled(ch, origMarkerEnabled);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// ADC-Freq Test 3: Settings Adjustment (Visual)
// UID: TST.ADC_FREQ.SETTINGS_ADJUSTMENT
// ============================================
printToConsole("\n=== ADC-Freq Test 3: Settings Adjustment (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_FREQ.SETTINGS_ADJUSTMENT", function() {
    try {
        var passed = true;
        var origTitle = adc.freqPlot.getTitle();
        var origLabels = adc.freqPlot.isLabelsEnabled();

        printToConsole("  Setting freq plot title to 'test'...");
        adc.freqPlot.setTitle("test");
        msleep(500);
        if (!visualCheck("Text in the top left should change to 'test'")) passed = false;

        printToConsole("  Enabling plot labels...");
        adc.freqPlot.setLabelsEnabled(true);
        msleep(500);
        if (!visualCheck("Labels on X and Y axis should appear")) passed = false;

        printToConsole("  Disabling plot labels...");
        adc.freqPlot.setLabelsEnabled(false);
        msleep(500);
        if (!visualCheck("Labels should disappear")) passed = false;

        var channels = adc.freqPlot.getChannels();
        if (channels.length > 0) {
            var ch = channels[0];
            var origThickness = adc.freqPlot.getChannelThickness(ch);
            var origStyle = adc.freqPlot.getChannelStyle(ch);

            printToConsole("  Setting thickness to 3...");
            adc.freqPlot.setChannelThickness(ch, 3);
            msleep(500);
            if (!visualCheck("Channel curve should appear thicker")) passed = false;

            printToConsole("  Setting style to 1 (dashed)...");
            adc.freqPlot.setChannelStyle(ch, 1);
            msleep(500);
            if (!visualCheck("Channel curve should appear dashed")) passed = false;

            adc.freqPlot.setChannelThickness(ch, origThickness);
            adc.freqPlot.setChannelStyle(ch, origStyle);
            msleep(500);
        }

        adc.freqPlot.setTitle(origTitle);
        msleep(500);
        adc.freqPlot.setLabelsEnabled(origLabels);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// ADC-Freq Test 6: Y-Axis Channel Settings (Visual)
// UID: TST.ADC_FREQ.Y_AXIS_CHANNEL_SETTINGS
// ============================================
printToConsole("\n=== ADC-Freq Test 6: Y-Axis Channel Settings (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_FREQ.Y_AXIS_CHANNEL_SETTINGS", function() {
    try {
        if (freqChannels.length === 0) {
            printToConsole("  No freq channels available");
            return "SKIP";
        }
        var passed = true;
        var ch = freqChannels[0];
        var origYMin = adc.getFreqChannelYMin(ch);
        var origYMax = adc.getFreqChannelYMax(ch);

        printToConsole("  Setting " + ch + " Y range to -120 to 10...");
        adc.setFreqChannelYMin(ch, -120);
        msleep(500);
        adc.setFreqChannelYMax(ch, 10);
        msleep(500);
        adc.freqSingleShot();
        msleep(5000);
        if (!visualCheck("Only " + ch + " curve should vertically adjust")) passed = false;

        adc.setFreqChannelYMin(ch, origYMin);
        msleep(500);
        adc.setFreqChannelYMax(ch, origYMax);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// ADC-Freq Test 8: Cursor Settings (Visual)
// UID: TST.ADC_FREQ.CURSOR_SETTINGS
// ============================================
printToConsole("\n=== ADC-Freq Test 8: Cursor Settings (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_FREQ.CURSOR_SETTINGS", function() {
    try {
        var passed = true;
        adc.freqSingleShot();
        msleep(5000);

        printToConsole("  Enabling freq cursors...");
        adc.freqPlot.setCursorsVisible(true);
        msleep(500);
        adc.freqPlot.setXCursorsEnabled(true);
        msleep(500);
        if (!visualCheck("X cursors should be visible on freq plot")) passed = false;

        printToConsole("  Positioning X1 cursor at 1 MHz...");
        adc.freqPlot.setX1CursorPosition(1000000);
        msleep(500);
        if (!visualCheck("Cursor should be at 1 MHz position")) passed = false;

        printToConsole("  Enabling tracking...");
        adc.freqPlot.setTrackingEnabled(true);
        msleep(500);
        if (!visualCheck("Markers should appear on channel curve synced with cursor")) passed = false;

        printToConsole("  Enabling Y cursors...");
        adc.freqPlot.setYCursorsEnabled(true);
        msleep(500);
        adc.freqPlot.setY1CursorPosition(-20);
        msleep(500);
        if (!visualCheck("Y cursor should be at -20 dB position")) passed = false;

        adc.freqPlot.setTrackingEnabled(false);
        msleep(500);
        adc.freqPlot.setCursorsVisible(false);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Channel Averaging (Visual)
// UID: TST.ADC_FREQ.CHANNEL_AVERAGING
// ============================================
printToConsole("\n=== Channel Averaging (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_FREQ.CHANNEL_AVERAGING", function() {
    try {
        if (freqChannels.length === 0) {
            printToConsole("  No freq channels available");
            return "SKIP";
        }
        var passed = true;
        var ch = freqChannels.length > 1 ? freqChannels[1] : freqChannels[0];
        var origEnabled = adc.isFreqChannelAveragingEnabled(ch);
        var origSize = adc.getFreqChannelAveragingSize(ch);

        printToConsole("  Enabling averaging for " + ch + " with size 10...");
        adc.setFreqChannelAveragingEnabled(ch, true);
        msleep(500);
        adc.setFreqChannelAveragingSize(ch, 10);
        msleep(500);

        adc.setFreqRunning(true);
        msleep(5000);
        if (!visualCheck(ch + " curve should appear smoother due to averaging")) passed = false;

        adc.setFreqRunning(false);
        msleep(500);
        adc.setFreqChannelAveragingSize(ch, origSize);
        msleep(500);
        adc.setFreqChannelAveragingEnabled(ch, origEnabled);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Genalyzer Visual Tests
// Requires complex mode
// ============================================
printToConsole("\n=== Genalyzer Visual Tests ===\n");

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

// ============================================
// Genalyzer Test 1: Enable (Visual)
// UID: TST.ADC_GENALYZER.ENABLE
// ============================================
printToConsole("\n=== Genalyzer Test 1: Enable (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_GENALYZER.ENABLE", function() {
    try {
        var passed = true;
        var origEnabled = adc.isGenalyzerEnabled();

        printToConsole("  Enabling genalyzer analysis...");
        adc.setGenalyzerEnabled(true);
        msleep(500);
        if (!visualCheck("A genalyzer results panel should appear with channel tabs")) passed = false;

        printToConsole("  Starting acquisition...");
        adc.setFreqRunning(true);
        msleep(5000);
        if (!visualCheck("Analysis results should populate with metrics (SNR, SINAD, SFDR, THD)")) passed = false;

        adc.setFreqRunning(false);
        msleep(500);
        adc.setGenalyzerEnabled(origEnabled);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Genalyzer Test 2: Auto Mode (Visual)
// UID: TST.ADC_GENALYZER.AUTO_MODE
// ============================================
printToConsole("\n=== Genalyzer Test 2: Auto Mode (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_GENALYZER.AUTO_MODE", function() {
    try {
        var passed = true;
        var origEnabled = adc.isGenalyzerEnabled();
        var origMode = adc.getGenalyzerMode();
        var origSSB = adc.getGenalyzerSSBWidth();

        printToConsole("  Enabling genalyzer in Auto mode...");
        adc.setGenalyzerEnabled(true);
        msleep(500);
        adc.setGenalyzerMode(0);
        msleep(500);
        adc.setGenalyzerSSBWidth(120);
        msleep(500);

        adc.setFreqRunning(true);
        msleep(5000);
        if (!visualCheck("Genalyzer should auto-detect tone and display A:freq metric")) passed = false;

        printToConsole("  Changing SSB width to 60...");
        adc.setGenalyzerSSBWidth(60);
        msleep(500);
        if (!visualCheck("signal_nbins metric should update")) passed = false;

        adc.setFreqRunning(false);
        msleep(500);
        adc.setGenalyzerSSBWidth(origSSB);
        msleep(500);
        adc.setGenalyzerMode(origMode);
        msleep(500);
        adc.setGenalyzerEnabled(origEnabled);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Genalyzer Test 3: Fixed Tone Mode (Visual)
// UID: TST.ADC_GENALYZER.FIXED_TONE_MODE
// ============================================
printToConsole("\n=== Genalyzer Test 3: Fixed Tone Mode (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_GENALYZER.FIXED_TONE_MODE", function() {
    try {
        var passed = true;
        var origEnabled = adc.isGenalyzerEnabled();
        var origMode = adc.getGenalyzerMode();
        var origFreq = adc.getGenalyzerExpectedFreq();
        var origOrder = adc.getGenalyzerHarmonicOrder();

        printToConsole("  Enabling genalyzer in Fixed Tone mode...");
        adc.setGenalyzerEnabled(true);
        msleep(500);
        adc.setGenalyzerMode(1);
        msleep(500);
        adc.setGenalyzerExpectedFreq(1000000);
        msleep(500);
        adc.setGenalyzerHarmonicOrder(5);
        msleep(500);
        adc.setGenalyzerSSBFundamental(4);
        msleep(500);
        adc.setGenalyzerSSBDefault(3);
        msleep(500);

        adc.setFreqRunning(true);
        msleep(5000);
        if (!visualCheck("A:freq metric should match 1MHz, A:mag_dbfs should show carrier level")) passed = false;

        printToConsole("  Changing expected frequency to 500kHz...");
        adc.setGenalyzerExpectedFreq(500000);
        msleep(1000);
        if (!visualCheck("Analysis should retarget to 500kHz with different results")) passed = false;

        adc.setFreqRunning(false);
        msleep(500);
        adc.setGenalyzerExpectedFreq(origFreq);
        msleep(500);
        adc.setGenalyzerHarmonicOrder(origOrder);
        msleep(500);
        adc.setGenalyzerMode(origMode);
        msleep(500);
        adc.setGenalyzerEnabled(origEnabled);
        msleep(500);

        return passed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Genalyzer Test 4: FFT Integration (Visual)
// UID: TST.ADC_GENALYZER.FFT_INTEGRATION
// ============================================
printToConsole("\n=== Genalyzer Test 4: FFT Integration (VISUAL) ===\n");

TestFramework.runTest("TST.ADC_GENALYZER.FFT_INTEGRATION", function() {
    try {
        var genFreqChannels = adc.getFreqChannels();
        if (genFreqChannels.length === 0) {
            printToConsole("  No freq channels available");
            return "SKIP";
        }
        var passed = true;
        var ch = genFreqChannels[0];
        var origGenalyzer = adc.isGenalyzerEnabled();
        var origAvgEnabled = adc.isFreqChannelAveragingEnabled(ch);
        var origAvgSize = adc.getFreqChannelAveragingSize(ch);
        var origWindow = adc.getFreqWindow();

        printToConsole("  Enabling genalyzer with averaging size 10...");
        adc.setGenalyzerEnabled(true);
        msleep(500);
        adc.setFreqChannelAveragingEnabled(ch, true);
        msleep(500);
        adc.setFreqChannelAveragingSize(ch, 10);
        msleep(500);

        adc.setFreqRunning(true);
        msleep(5000);
        if (!visualCheck("Genalyzer results should be stable due to averaging")) passed = false;

        printToConsole("  Changing window to Blackman-Harris...");
        adc.setFreqWindow(2);
        msleep(1000);
        if (!visualCheck("Genalyzer results should update with new window function")) passed = false;

        printToConsole("  Disabling averaging...");
        adc.setFreqChannelAveragingEnabled(ch, false);
        msleep(1000);
        if (!visualCheck("Results should be less stable without averaging")) passed = false;

        adc.setFreqRunning(false);
        msleep(500);
        adc.setFreqWindow(origWindow);
        msleep(500);
        adc.setFreqChannelAveragingSize(ch, origAvgSize);
        msleep(500);
        adc.setFreqChannelAveragingEnabled(ch, origAvgEnabled);
        msleep(500);
        adc.setGenalyzerEnabled(origGenalyzer);
        msleep(500);

        return passed;
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
