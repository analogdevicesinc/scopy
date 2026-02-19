/*
 * Copyright (c) 2026 Analog Devices Inc.
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

// SWIOT Plugin Supervised Automated Tests
// These tests require loopback hardware and visual supervision of plot signals.
// Based on test documentation from docs/tests/plugins/swiot1l/swiot1l_tests.rst

// Load test framework
evaluateFile("../scopy/js/testAutomations/common/testFramework.js");

// Test Suite: SWIOT Supervised Tests
TestFramework.init("SWIOT Plugin Supervised Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Helper: Stop AD74413R capture and disable all enabled channels
function cleanupAd74413r() {
    swiot.setAdRunning(false);
    var channelCount = swiot.getAdChannelCount();
    for (var i = 0; i < channelCount; i++) {
        if (swiot.isAdChannelEnabled(i)) {
            swiot.setAdPlotChannelEnabled(i, false);
        }
    }
    msleep(500);
}

// ============================================
// Test 5 - AD74413R Channel Operations (TST.AD74413R.CHANNEL)
// ============================================
printToConsole("\n=== Test 5 - AD74413R Channel Operations ===\n");

if (swiot.isRuntimeMode() && switchToTool("AD74413R")) {

    TestFramework.runTest("TST.AD74413R.CHANNEL", function() {
        try {
            // Step 1: Open the AD74413R instrument
            printToConsole("  Step 1: AD74413R instrument opened");

            // Print available widget keys for reference
            var keys = swiot.getAdWidgetKeys();
            printToConsole("  Available widget keys: " + keys);

            // Step 2: Enable Plot Labels and set Timespan to 2s
            swiot.setAdPlotLabelsEnabled(true);
            msleep(500);
            swiot.setAdTimespan(2.0);
            msleep(500);
            if (!TestFramework.assertApproxEqual(swiot.getAdTimespan(), 2.0, 0.01, "Timespan set to 2s")) {
                cleanupAd74413r();
                return false;
            }

            // Step 3: Enable voltage_out 1 and voltage_in 2 channels
            swiot.setAdPlotChannelEnabled(0, true);
            msleep(300);
            swiot.setAdPlotChannelEnabled(1, true);
            msleep(300);
            if (!swiot.isAdChannelEnabled(0) || !swiot.isAdChannelEnabled(1)) {
                printToConsole("  FAIL: Could not enable channels 0 and 1");
                cleanupAd74413r();
                return false;
            }
            printToConsole("  Step 3: Channels voltage_out 1 and voltage_in 2 enabled");

            // Step 4: Set sampling_frequency to 1200 on voltage_out 1
            swiot.setAdChannelSamplingFrequency(0, 1200);
            msleep(1000);
            var sampFreq = swiot.getAdChannelSamplingFrequency(0);
            if (!TestFramework.assertApproxEqual(sampFreq, 1200, 50, "Sampling frequency set to 1200")) {
                cleanupAd74413r();
                return false;
            }
            var sampleRate = swiot.getAdSampleRate();
            printToConsole("  Step 4: Sample rate: " + sampleRate + " sps");

            // Step 5: Set YMin to -1A and YMax to 10A for voltage_out 1
            swiot.setAdChannelYMin(0, -1.0);
            msleep(300);
            swiot.setAdChannelYMax(0, 10.0);
            msleep(300);
            if (!TestFramework.assertApproxEqual(swiot.getAdChannelYMin(0), -1.0, 0.01, "Channel 0 YMin set to -1")) {
                cleanupAd74413r();
                return false;
            }
            if (!TestFramework.assertApproxEqual(swiot.getAdChannelYMax(0), 10.0, 0.01, "Channel 0 YMax set to 10")) {
                cleanupAd74413r();
                return false;
            }

            // Step 6: Set YMin to 0V and YMax to 20V for voltage_in 2
            swiot.setAdChannelYMin(1, 0.0);
            msleep(300);
            swiot.setAdChannelYMax(1, 20.0);
            msleep(300);
            if (!TestFramework.assertApproxEqual(swiot.getAdChannelYMin(1), 0.0, 0.01, "Channel 1 YMin set to 0")) {
                cleanupAd74413r();
                return false;
            }
            if (!TestFramework.assertApproxEqual(swiot.getAdChannelYMax(1), 20.0, 0.01, "Channel 1 YMax set to 20")) {
                cleanupAd74413r();
                return false;
            }

            // Step 7: Set the RAW output value to 8192 on voltage_out 1
            // Key format: device/channelId_direction/attribute
            var rawKey = "ad74413r/voltage0_out/raw";
            swiot.setAdWidgetValue(rawKey, "8192");
            msleep(1000);

            // Expected: Value is automatically clamped to 8191, converted value shows ~10.9V
            var rawValue = swiot.getAdWidgetValue(rawKey);
            printToConsole("  Step 7: RAW value after writing 8192: " + rawValue);
            if (!TestFramework.assertApproxEqual(parseInt(rawValue), 8191, 1, "RAW clamped to 8191")) {
                cleanupAd74413r();
                return false;
            }

            // Step 8: Run a Single capture
            // SUPERVISE: voltage_in 2 instant value should show ~10V
            swiot.adSingleShot();
            msleep(5000);
            printToConsole("  Step 8: Single capture completed");
            printToConsole("  SUPERVISE: Verify voltage_in 2 instant value shows ~10V");

            // Step 9: Run a Continuous capture
            // SUPERVISE: voltage_in 2 instant value should show ~10V
            swiot.setAdRunning(true);
            msleep(3000);
            if (!swiot.isAdRunning()) {
                printToConsole("  FAIL: Could not start continuous capture");
                cleanupAd74413r();
                return false;
            }
            printToConsole("  Step 9: Continuous capture running");
            printToConsole("  SUPERVISE: Verify voltage_in 2 instant value shows ~10V");

            // Step 10: While running change the RAW value to 4096
            // SUPERVISE: signal drops to ~5.5V on plot, instant value matches
            swiot.setAdWidgetValue(rawKey, "4096");
            msleep(2000);
            rawValue = swiot.getAdWidgetValue(rawKey);
            if (!TestFramework.assertApproxEqual(parseInt(rawValue), 4096, 1, "RAW set to 4096")) {
                cleanupAd74413r();
                return false;
            }
            printToConsole("  Step 10: RAW set to 4096 - below field shows ~5.5V");
            printToConsole("  SUPERVISE: Verify voltage_in 2 signal drops to ~5.5V on plot");

            msleep(5000);
            // Step 11: While running change the RAW value to -1
            // SUPERVISE: signal drops to 0V on plot, instant value matches
            swiot.setAdWidgetValue(rawKey, "-1");
            msleep(2000);
            rawValue = swiot.getAdWidgetValue(rawKey);
            if (!TestFramework.assertApproxEqual(parseInt(rawValue), 0, 0, "RAW clamped to 0")) {
                cleanupAd74413r();
                return false;
            }
            printToConsole("  Step 11: RAW clamped to 0 - below field shows 0V");
            printToConsole("  SUPERVISE: Verify voltage_in 2 signal drops to 0V on plot");

            // Cleanup
            cleanupAd74413r();
            printToConsole("  Cleanup: capture stopped, channels disabled");

            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            cleanupAd74413r();
            return false;
        }
    });
} else {
    printToConsole("  Skipping TST.AD74413R.CHANNEL - not in runtime mode or tool not available");
}

// ============================================
// Test 6 - AD74413R Diagnostic Channels (TST.AD74413R.DIAG)
// ============================================
printToConsole("\n=== Test 6 - AD74413R Diagnostic Channels ===\n");

if (swiot.isRuntimeMode() && switchToTool("AD74413R")) {

    TestFramework.runTest("TST.AD74413R.DIAG", function() {
        try {
            // Step 1: Open the AD74413R instrument
            printToConsole("  Step 1: AD74413R instrument opened");

            // Step 2: Enable Plot Labels and set Timespan to 2s
            swiot.setAdPlotLabelsEnabled(true);
            msleep(500);
            swiot.setAdTimespan(2.0);
            msleep(500);
            if (!TestFramework.assertApproxEqual(swiot.getAdTimespan(), 2.0, 0.01, "Timespan set to 2s")) {
                cleanupAd74413r();
                return false;
            }

            // Step 3: Enable voltage_out 1, voltage_in 2 and diagnostic 5 channels
            swiot.setAdPlotChannelEnabled(0, true);
            msleep(300);
            swiot.setAdPlotChannelEnabled(1, true);
            msleep(300);
            swiot.setAdPlotChannelEnabled(2, true);
            msleep(1000);
            if (!swiot.isAdChannelEnabled(0) || !swiot.isAdChannelEnabled(1) || !swiot.isAdChannelEnabled(4)) {
                printToConsole("  FAIL: Could not enable channels 0, 1 and 4");
                cleanupAd74413r();
                return false;
            }
            printToConsole("  Step 3: Channels voltage_out 1, voltage_in 2 and diagnostic 5 enabled");

            // Step 4: Set diag_function to sensel_b on diagnostic 5
            var diagFuncKey = "ad74413r/voltage4_in/diag_function";
            swiot.setAdWidgetValue(diagFuncKey, "sensel_b");
            msleep(1000);
            var diagFunc = swiot.getAdWidgetValue(diagFuncKey);
            printToConsole("  Step 4: diag_function set to: " + diagFunc);
            if (!TestFramework.assertEqual(diagFunc, "sensel_b", "diag_function set to sensel_b")) {
                cleanupAd74413r();
                return false;
            }

            // Step 5: Set the RAW output value to 8192 on voltage_out 1
            var rawKey = "ad74413r/voltage0_out/raw";
            swiot.setAdWidgetValue(rawKey, "8192");
            msleep(1000);
            var rawValue = swiot.getAdWidgetValue(rawKey);
            printToConsole("  Step 5: RAW value after writing 8192: " + rawValue);

            // Step 6: Run a Continuous capture
            // SUPERVISE: diagnostic 5 shows ~10V, same as voltage_in 2
            swiot.setAdRunning(true);
            msleep(3000);
            if (!swiot.isAdRunning()) {
                printToConsole("  FAIL: Could not start continuous capture");
                cleanupAd74413r();
                return false;
            }
            printToConsole("  Step 6: Continuous capture running");
            printToConsole("  SUPERVISE: Verify diagnostic 5 instant value shows ~10V, same as voltage_in 2");

            // Step 7: While running change the RAW value to 4096
            // SUPERVISE: Both diagnostic 5 and voltage_in 2 drop to ~5.5V
            swiot.setAdWidgetValue(rawKey, "4096");
            msleep(2000);
            rawValue = swiot.getAdWidgetValue(rawKey);
            if (!TestFramework.assertApproxEqual(parseInt(rawValue), 4096, 1, "RAW set to 4096")) {
                cleanupAd74413r();
                return false;
            }
            printToConsole("  Step 7: RAW set to 4096");
            printToConsole("  SUPERVISE: Verify both diagnostic 5 and voltage_in 2 drop to ~5.5V");

            // Step 8: While running change the RAW value to 2000
            // SUPERVISE: Both diagnostic 5 and voltage_in 2 drop to ~2.7V
            swiot.setAdWidgetValue(rawKey, "2000");
            msleep(2000);
            rawValue = swiot.getAdWidgetValue(rawKey);
            if (!TestFramework.assertApproxEqual(parseInt(rawValue), 2000, 1, "RAW set to 2000")) {
                cleanupAd74413r();
                return false;
            }
            printToConsole("  Step 8: RAW set to 2000");
            printToConsole("  SUPERVISE: Verify both diagnostic 5 and voltage_in 2 drop to ~2.7V");

            // Cleanup
            cleanupAd74413r();
            printToConsole("  Cleanup: capture stopped, channels disabled");

            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            cleanupAd74413r();
            return false;
        }
    });
} else {
    printToConsole("  Skipping TST.AD74413R.DIAG - not in runtime mode or tool not available");
}

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
scopy.exit();
exit(exitCode);
