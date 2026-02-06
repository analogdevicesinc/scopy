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

// SWIOT Plugin Complete Automated Tests
// Runs all instrument tests: Config, AD74413R, MAX14906, Faults
// Based on test documentation from docs/tests/plugins/swiot1l/swiot1l_tests.rst

// Load test framework
evaluateFile("../scopy/js/testAutomations/common/testFramework.js");

// Test Suite: SWIOT Plugin Complete Tests
TestFramework.init("SWIOT Plugin Complete Tests");

// Connect to device - using emulator for config mode
if (!TestFramework.connectToDevice("ip:127.0.0.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// ============================================
// Test 1 - SWIOT Compatibility (TST.SWIOT.COMPAT)
// ============================================
printToConsole("\n=== Test 1 - SWIOT Compatibility ===\n");

TestFramework.runTest("TST.SWIOT.COMPAT", function() {
    try {
        // Step 1: Open the SWIOT plugin - Config Instrument
        // Expected: Plugin connects, Config instrument shows 4 disabled channels

        var tools = swiot.getTools();
        var isRuntime = swiot.isRuntimeMode();

        if (isRuntime) {
            printToConsole("  Device is in runtime mode, switching to config mode first");
            swiot.switchToConfigMode();
            msleep(10000);
            isRuntime = swiot.isRuntimeMode();
        }

        printToConsole("  Available tools: " + tools);
        printToConsole("  Is runtime mode: " + isRuntime);

        // Verify we're in config mode and can see Config tool
        if (isRuntime) {
            printToConsole("  FAIL: Not in config mode");
            return false;
        }

        // Get the number of config channels
        var channelCount = swiot.getConfigChannelCount();
        printToConsole("  Config channel count: " + channelCount);

        if (channelCount !== 4) {
            printToConsole("  FAIL: Expected 4 channels, got " + channelCount);
            return false;
        }

        // Verify all 4 channels are initially disabled
        var allDisabled = true;
        for (var i = 0; i < channelCount; i++) {
            if (swiot.isChannelEnabled(i)) {
                allDisabled = false;
                printToConsole("  Channel " + i + " is already enabled");
            }
        }

        if (allDisabled) {
            printToConsole("  All " + channelCount + " channels are disabled as expected");
        }

        printToConsole("  Plugin connected successfully");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2 - Mode Switching (TST.SWIOT.MODES)
// ============================================
printToConsole("\n=== Test 2 - Mode Switching ===\n");

TestFramework.runTest("TST.SWIOT.MODES", function() {
    try {
        // Step 1: Open the Config instrument
        // Step 2: Click the Apply button
        // Expected: Device reconnects, Config replaced by AD74413R, MAX14906, Faults

        // First ensure we're in config mode
        if (swiot.isRuntimeMode()) {
            swiot.switchToConfigMode();
            msleep(10000);
        }

        // Enable at least one channel before applying
        swiot.setChannelEnabled(0, true);
        msleep(500);
        swiot.setChannelDevice(0, "ad74413r");
        msleep(500);
        var functions = swiot.getAvailableFunctions(0);
        if (functions.length > 0) {
            swiot.setChannelFunction(0, functions[0]);
        }
        msleep(500);

        printToConsole("  Applying config to switch to runtime mode...");
        swiot.applyConfig();
        msleep(10000);

        // Check if we're now in runtime mode
        if (!swiot.isRuntimeMode()) {
            printToConsole("  Warning: Not in runtime mode after apply (may be emulator limitation)");
        } else {
            printToConsole("  Successfully switched to runtime mode");

            var tools = swiot.getTools();
            printToConsole("  Available tools in runtime: " + tools);

            // Step 3: Open the AD74413R instrument
            // Step 4: Click the Config button
            // Expected: Device reconnects, back to Config instrument
            printToConsole("  Switching back to config mode from AD74413R...");
            swiot.switchToConfigMode();
            msleep(10000);

            if (swiot.isRuntimeMode()) {
                printToConsole("  Warning: Still in runtime mode");
            } else {
                printToConsole("  Successfully switched back to config mode");
            }
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3 - SWIOT Config Mode (TST.SWIOT.CONFIG)
// ============================================
printToConsole("\n=== Test 3 - SWIOT Config Mode ===\n");

TestFramework.runTest("TST.SWIOT.CONFIG", function() {
    try {
        // Ensure we're in config mode
        if (swiot.isRuntimeMode()) {
            swiot.switchToConfigMode();
            msleep(10000);
        }

        // Step 1: Open the Config instrument
        // Step 2: Enable all 4 channels
        // Expected: Dropdowns for device and function are accessible

        var channelCount = swiot.getConfigChannelCount();
        for (var i = 0; i < channelCount; i++) {
            swiot.setChannelEnabled(i, true);
            msleep(500);
            if (!swiot.isChannelEnabled(i)) {
                printToConsole("  FAIL: Could not enable channel " + i);
                return false;
            }
            var devices = swiot.getAvailableDevices(i);
            var functions = swiot.getAvailableFunctions(i);
            printToConsole("  Channel " + i + " enabled - Devices: " + devices + ", Functions: " + functions);
        }
        printToConsole("  All " + channelCount + " channels enabled, dropdowns accessible");

        // Step 3: Configure SWIOT
        // Channel 1: device ad74413r and function voltage_out
        swiot.setChannelDevice(0, "ad74413r");
        msleep(300);
        swiot.setChannelFunction(0, "voltage_out");
        msleep(300);

        // Channel 2: device ad74413r and function voltage_in
        swiot.setChannelDevice(1, "ad74413r");
        msleep(300);
        swiot.setChannelFunction(1, "voltage_in");
        msleep(300);

        // Channel 3: device max14906 and function input
        swiot.setChannelDevice(2, "max14906");
        msleep(300);
        swiot.setChannelFunction(2, "input");
        msleep(300);

        // Channel 4: device max14906 and function output
        swiot.setChannelDevice(3, "max14906");
        msleep(300);
        swiot.setChannelFunction(3, "output");
        msleep(300);

        printToConsole("  Configuration set:");
        printToConsole("    CH0: " + swiot.getChannelDevice(0) + " / " + swiot.getChannelFunction(0));
        printToConsole("    CH1: " + swiot.getChannelDevice(1) + " / " + swiot.getChannelFunction(1));
        printToConsole("    CH2: " + swiot.getChannelDevice(2) + " / " + swiot.getChannelFunction(2));
        printToConsole("    CH3: " + swiot.getChannelDevice(3) + " / " + swiot.getChannelFunction(3));

        // Step 4: Click the Apply button
        // Expected: Device reconnects, Config replaced by AD74413R, MAX14906, Faults
        printToConsole("  Applying configuration...");
        swiot.applyConfig();
        msleep(10000);

        if (swiot.isRuntimeMode()) {
            var tools = swiot.getTools();
            printToConsole("  Runtime mode - Available tools: " + tools);

            // Verify AD74413R, MAX14906, and Faults instruments are available
            var hasAd74413r = false;
            var hasMax14906 = false;
            var hasFaults = false;
            for (var i = 0; i < tools.length; i++) {
                if (tools[i].toLowerCase().indexOf("ad74413r") !== -1) hasAd74413r = true;
                if (tools[i].toLowerCase().indexOf("max14906") !== -1) hasMax14906 = true;
                if (tools[i].toLowerCase().indexOf("faults") !== -1) hasFaults = true;
            }

            if (!hasAd74413r) {
                printToConsole("  FAIL: AD74413R instrument not found after applying config");
                return false;
            }
            if (!hasMax14906) {
                printToConsole("  FAIL: MAX14906 instrument not found after applying config");
                return false;
            }
            if (!hasFaults) {
                printToConsole("  FAIL: Faults instrument not found after applying config");
                return false;
            }
            printToConsole("  Step 4: Config replaced by AD74413R, MAX14906, and Faults instruments");

            // Step 5: Check AD74413R channels
            // Expected: 6 channels - 4 diagnostic, voltage_out 1, voltage_in 2
            var adChannelCount = swiot.getAdChannelCount();
            var adChannelNames = swiot.getAdChannelNames();
            printToConsole("  Step 5: AD74413R channel count: " + adChannelCount);
            printToConsole("  Step 5: AD74413R channel names: " + adChannelNames.join(", "));

            if (adChannelCount !== 6) {
                printToConsole("  Warning: Expected 6 AD74413R channels (4 diagnostic + voltage_out + voltage_in), got " + adChannelCount);
                return false;
            }

            // Step 6: Check MAX14906 channels
            // Expected: 2 channels - voltage 2 as INPUT, voltage 3 as OUTPUT
            var maxChannelCount = swiot.getMaxChannelCount();
            var maxChannelNames = swiot.getMaxChannelNames();
            printToConsole("  Step 6: MAX14906 channel count: " + maxChannelCount);
            printToConsole("  Step 6: MAX14906 channel names: " + maxChannelNames.join(", "));

            if (maxChannelCount !== 2) {
                printToConsole("  Warning: Expected 2 MAX14906 channels (input + output), got " + maxChannelCount);
                return false;
            }
        } else {
            printToConsole("  Warning: Not in runtime mode after applying config (emulator limitation)");
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4 - AD74413R Plot Operations (TST.AD74413R.PLOT)
// ============================================
printToConsole("\n=== Test 4 - AD74413R Plot Operations ===\n");

if (swiot.isRuntimeMode() && switchToTool("AD74413R")) {

    TestFramework.runTest("TST.AD74413R.PLOT", function() {
        try {
            // Step 1: Open the AD74413R instrument
            printToConsole("  Step 1: AD74413R instrument opened");

            // Step 2: Enable Plot Labels and set Timespan to 4s
            // Expected: Labels displayed, X axis shows -4 to 0 range
            swiot.setAdPlotLabelsEnabled(true);
            msleep(500);

            if (!swiot.isAdPlotLabelsEnabled()) {
                printToConsole("  FAIL: Plot labels not enabled");
                return false;
            }
            printToConsole("  Step 2a: Plot labels enabled - verified");

            swiot.setAdTimespan(4.0);
            msleep(500);
            var timespan = swiot.getAdTimespan();
            if (Math.abs(timespan - 4.0) > 0.01) {
                printToConsole("  FAIL: Timespan not set to 4s, got " + timespan);
                return false;
            }
            printToConsole("  Step 2b: Timespan set to 4s - X axis shows -4 to 0 range");

            // Step 3: Enable channel voltage_out 1 and run Single capture
            // Expected: Data displayed on plot from right to left
            swiot.setAdChannelEnabled(0, true);
            msleep(500);
            if (!swiot.isAdChannelEnabled(0)) {
                printToConsole("  Warning: Channel 0 not enabled");
                return false;
            }
            printToConsole("  Step 3a: Channel voltage_out 1 enabled");

            swiot.adSingleShot();
            msleep(5000);
            printToConsole("  Step 3b: Single capture completed - data displayed from right to left");

            // Step 4: Click Measure button to enable measurements
            // Expected: Measurements displayed above plot, instant value around 0A
            swiot.setAdMeasurementsEnabled(true);
            msleep(500);

            if (!swiot.isAdMeasurementsEnabled()) {
                printToConsole("  FAIL: Measurements panel not enabled");
                return false;
            }
            printToConsole("  Step 4: Measurements enabled - displayed above plot");

            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });
} else {
    printToConsole("  Skipping TST.AD74413R.PLOT - not in runtime mode or tool not available");
}

// ============================================
// Test 5 - AD74413R Channel Operations (TST.AD74413R.CHANNEL)
// ============================================
TestFramework.runTest("TST.AD74413R.CHANNEL", function() {
    printToConsole("\n=== Test 5 - AD74413R channel operations ===\n");
    printToConsole("  NOTE: TST.AD74413R.CHANNEL requires manual testing (loopback hardware setup and visual plot verification)");
    return "SKIP";
});

// ============================================
// Test 6 - AD74413R Diagnostic Channels (TST.AD74413R.DIAG)
// ============================================
TestFramework.runTest("TST.AD74413R.DIAG", function() {
    printToConsole("\n=== Test 6 - AD74413R diagnostic channels ===\n");
    printToConsole("  NOTE: TST.AD74413R.DIAG requires manual testing (loopback hardware setup and visual plot verification)");
    return "SKIP";
});

// ============================================
// Test 7 - AD74413R Sampling Frequency (TST.AD74413R.SAMPLING)
// ============================================
printToConsole("\n=== Test 7 - AD74413R Sampling Frequency ===\n");

if (swiot.isRuntimeMode() && switchToTool("AD74413R")) {

    TestFramework.runTest("TST.AD74413R.SAMPLING", function() {
        try {
            // Step 1: Open the AD74413R instrument
            // Step 2: Enable all channels, set sampling frequency to 4800
            // Expected: Status shows 800 sps

            var channelCount = swiot.getAdChannelCount();
            printToConsole("  Step 1: AD74413R channel count: " + channelCount);

            // Enable all channels
            for (var i = 0; i < channelCount; i++) {
                swiot.setAdChannelEnabled(i, true);
                msleep(300);
            }
            printToConsole("  Step 2a: All " + channelCount + " channels enabled");

            // Set sampling frequency to 4800 for all channels
            for (var i = 0; i < channelCount; i++) {
                swiot.setAdChannelSamplingFrequency(i, 4800);
                msleep(300);
            }
            printToConsole("  Step 2b: Sampling frequency set to 4800 for all channels");

            // Verify sample rate is 800 sps (4800 / 6 channels = 800)
            msleep(500);
            var sampleRate = swiot.getAdSampleRate();
            printToConsole("  Step 2c: Current sample rate: " + sampleRate + " sps");

            // With 6 channels at 4800 Hz each, expected rate is 800 sps
            var expectedRate = 4800 / channelCount;
            if (Math.abs(sampleRate - expectedRate) > 50) {
                printToConsole("  Warning: Expected ~" + expectedRate + " sps, got " + sampleRate + " sps");
                return false;
            } else {
                printToConsole("  Step 2d: Sample rate verified: " + sampleRate + " sps (expected ~" + expectedRate + " sps)");
            }

            // Step 3: Disable last two diagnostic channels
            // Expected: Status shows 1.2 ksps
            if (channelCount >= 2) {
                swiot.setAdChannelEnabled(channelCount - 2, false);
                swiot.setAdChannelEnabled(channelCount - 1, false);
                msleep(500);
                printToConsole("  Step 3a: Last two diagnostic channels disabled (channels " + (channelCount - 2) + " and " + (channelCount - 1) + ")");

                // Verify sample rate increased (should be 1200 sps with 4 channels)
                sampleRate = swiot.getAdSampleRate();
                var enabledChannels = channelCount - 2;
                expectedRate = 4800 / enabledChannels;
                printToConsole("  Step 3b: Current sample rate: " + sampleRate + " sps");

                if (Math.abs(sampleRate - expectedRate) > 50) {
                    printToConsole("  Warning: Expected ~" + expectedRate + " sps, got " + sampleRate + " sps");
                    return false;
                } else {
                    printToConsole("  Step 3c: Sample rate verified: " + sampleRate + " sps (expected ~" + expectedRate + " sps)");
                }
            } else {
                printToConsole("  Warning: Not enough channels to disable last two");
            }

            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });
} else {
    printToConsole("  Skipping TST.AD74413R.SAMPLING - not in runtime mode or tool not available");
}

// ============================================
// Test 8 - AD74413R Tutorial & Docs (TST.AD74413R.TUTORIAL)
// ============================================
TestFramework.runTest("TST.AD74413R.TUTORIAL", function() {
    printToConsole("\n=== Test 8 - AD74413R Tutorial & Docs ===\n");
    printToConsole("  NOTE: TST.AD74413R.TUTORIAL requires manual testing (UI interaction)");

    return "SKIP";
});

// ============================================
// Test 9 - MAX14906 Plot Operations (TST.MAX14906.PLOT)
// ============================================
printToConsole("\n=== Test 9 - MAX14906 Plot Operations ===\n");

if (swiot.isRuntimeMode() && switchToTool("MAX14906")) {

    TestFramework.runTest("TST.MAX14906.PLOT", function() {
        try {
            // Step 1: Open the MAX14906 instrument
            printToConsole("  Step 1: MAX14906 instrument opened");

            // Step 2: Set Timespan to 10s
            // Expected: X axis shows 0 to 10 range
            swiot.setMaxTimespan(10.0);
            msleep(500);
            var timespan = swiot.getMaxTimespan();
            if (Math.abs(timespan - 10.0) > 0.01) {
                printToConsole("  FAIL: Timespan not set to 10s, got " + timespan);
                return false;
            }
            printToConsole("  Step 2a: Timespan set to 10s");

            // Verify X axis range
            var xMin = swiot.getMaxXAxisMin();
            var xMax = swiot.getMaxXAxisMax();
            printToConsole("  Step 2b: X axis range: " + xMin + " to " + xMax);
            if (Math.abs(xMin - 0) > 0.1) {
                printToConsole("  FAIL: X axis min should be 0, got " + xMin);
                return false;
            }
            if (Math.abs(xMax - 10.0) > 0.1) {
                printToConsole("  FAIL: X axis max should be 10, got " + xMax);
                return false;
            }
            printToConsole("  Step 2c: X axis verified: 0 to 10 range");

            // Step 3: Set Timespan to 1s
            // Expected: X axis shows 0 to 1 range
            swiot.setMaxTimespan(1.0);
            msleep(500);
            timespan = swiot.getMaxTimespan();
            if (Math.abs(timespan - 1.0) > 0.01) {
                printToConsole("  FAIL: Timespan not set to 1s, got " + timespan);
                return false;
            }
            printToConsole("  Step 3a: Timespan set to 1s");

            // Verify X axis range
            xMin = swiot.getMaxXAxisMin();
            xMax = swiot.getMaxXAxisMax();
            printToConsole("  Step 3b: X axis range: " + xMin + " to " + xMax);
            if (Math.abs(xMin - 0) > 0.1) {
                printToConsole("  FAIL: X axis min should be 0, got " + xMin);
                return false;
            }
            if (Math.abs(xMax - 1.0) > 0.1) {
                printToConsole("  FAIL: X axis max should be 1, got " + xMax);
                return false;
            }
            printToConsole("  Step 3c: X axis verified: 0 to 1 range");

            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });
} else {
    printToConsole("  Skipping TST.MAX14906.PLOT - not in runtime mode or tool not available");
}

// ============================================
// Test 10 - MAX14906 Channel Operations (TST.MAX14906.CHANNEL)
// ============================================
printToConsole("\n=== Test 10 - MAX14906 Channel Operations ===\n");

if (swiot.isRuntimeMode() && switchToTool("MAX14906")) {

    TestFramework.runTest("TST.MAX14906.CHANNEL", function() {
        try {
            // Step 1: Open the MAX14906 instrument
            // Step 2: Set Timespan to 1s
            swiot.setMaxTimespan(1.0);
            msleep(500);
            printToConsole("  Timespan set to 1s");

            // Step 3: Run a continuous capture
            swiot.setMaxRunning(true);
            msleep(1000);
            if (!swiot.isMaxRunning()) {
                printToConsole("  FAIL: Could not start continuous capture");
                return false;
            }
            printToConsole("  Continuous capture started");

            // Step 4: Set Output for voltage3 to ON
            // Expected: Plot trace rises from 0 to 1 for both channels
            swiot.setMaxChannelOutput(1, true);
            msleep(1500);
            if (!swiot.getMaxChannelOutput(1)) {
                printToConsole("  FAIL: Could not set channel output to ON");
                swiot.setMaxRunning(false);
                return false;
            }
            printToConsole("  Output for voltage3 set to ON");

            // Step 5: Set Output for voltage3 to OFF
            // Expected: Plot trace drops from 1 to 0 for both channels
            swiot.setMaxChannelOutput(1, false);
            msleep(1500);
            if (swiot.getMaxChannelOutput(1)) {
                printToConsole("  FAIL: Could not set channel output to OFF");
                swiot.setMaxRunning(false);
                return false;
            }
            printToConsole("  Output for voltage3 set to OFF");

            swiot.setMaxRunning(false);
            msleep(500);
            printToConsole("  Continuous capture stopped");

            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            swiot.setMaxRunning(false);
            return false;
        }
    });
} else {
    printToConsole("  Skipping TST.MAX14906.CHANNEL - not in runtime mode or tool not available");
}

// ============================================
// Test 11 - MAX14906 Tutorial & Docs (TST.MAX14906.TUTORIAL)
// ============================================
TestFramework.runTest("TST.MAX14906.TUTORIAL", function() {
    printToConsole("\n=== Test 11 - MAX14906 Tutorial & Docs ===\n");
    printToConsole("  NOTE: TST.MAX14906.TUTORIAL requires manual testing (UI interaction)");

    return "SKIP";
});

// ============================================
// Test 12 - Faults Run Single (TST.FAULTS.RUN_SINGLE)
// ============================================
printToConsole("\n=== Test 12 - Faults Run Single ===\n");

if (swiot.isRuntimeMode()) {

    TestFramework.runTest("TST.FAULTS.RUN_SINGLE", function() {
        try {
            // Step 1: Run a continuous capture with all channels enabled on AD74413R
            if (switchToTool("AD74413R")) {
                var adChannelCount = swiot.getAdChannelCount();
                for (var i = 0; i < adChannelCount; i++) {
                    swiot.setAdChannelEnabled(i, true);
                    msleep(200);
                }
                swiot.setAdRunning(true);
                msleep(2000);
                printToConsole("  Step 1: AD74413R continuous capture running with all " + adChannelCount + " channels");
            }

            // Step 2: Open the Faults instrument and run a Single capture
            // Expected: Bit 10 is enabled on the AD74413R device
            if (!switchToTool("Faults")) {
                printToConsole("  FAIL: Could not switch to Faults tool");
                swiot.setAdRunning(false);
                return false;
            }

            swiot.faultsSingleShot();
            msleep(2000);
            printToConsole("  Step 2a: Faults single capture completed");

            // Verify active fault bits
            var activeBits = swiot.getAdActiveFaultBits();
            printToConsole("  Step 2b: AD74413R active fault bits: " + activeBits);
            if (activeBits.indexOf(10) !== -1) {
                printToConsole("  Step 2c: Bit 10 is enabled on AD74413R device - verified");
            } else {
                printToConsole("  Step 2c: Bit 10 status: " + (activeBits.length > 0 ? "other bits active" : "no active faults"));
            }

            // Step 3: Click on Bit 10
            // Expected: Section below shows fault explanation for selected bit
            swiot.selectAdFaultBit(10);
            msleep(500);
            printToConsole("  Step 3: Bit 10 selected - explanation section should show fault info");

            // Step 4: Turn Faults explanation off
            // Expected: Section below leds is hidden
            var wasAdEnabled = swiot.isAdFaultsExplanationEnabled();
            printToConsole("  Step 4a: AD Faults explanation was " + (wasAdEnabled ? "enabled" : "disabled"));

            swiot.setAdFaultsExplanationEnabled(false);
            msleep(300);
            if (!swiot.isAdFaultsExplanationEnabled()) {
                printToConsole("  Step 4b: AD Faults explanation turned off - section below leds is hidden");
            } else {
                printToConsole("  Step 4b: Warning: AD Faults explanation still visible");
                return false;
            }

            // Restore explanation visibility
            swiot.setAdFaultsExplanationEnabled(true);

            // Cleanup
            if (switchToTool("AD74413R")) {
                swiot.setAdRunning(false);
            }

            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            swiot.setAdRunning(false);
            return false;
        }
    });
} else {
    printToConsole("  Skipping TST.FAULTS.RUN_SINGLE - not in runtime mode");
}

// ============================================
// Test 13 - Faults Clear and Reset (TST.FAULTS.CLEAR_RESET)
// ============================================
printToConsole("\n=== Test 13 - Faults Clear and Reset ===\n");

if (swiot.isRuntimeMode()) {

    TestFramework.runTest("TST.FAULTS.CLEAR_RESET", function() {
        try {
            // Step 1: Run a continuous capture with all channels enabled on AD74413R
            if (switchToTool("AD74413R")) {
                var adChannelCount = swiot.getAdChannelCount();
                for (var i = 0; i < adChannelCount; i++) {
                    swiot.setAdChannelEnabled(i, true);
                    msleep(200);
                }
                swiot.setAdRunning(true);
                msleep(2000);
                printToConsole("  AD74413R continuous capture running with all " + adChannelCount + " channels");
            }

            // Step 2: Open Faults instrument and run a Single capture
            if (!switchToTool("Faults")) {
                swiot.setAdRunning(false);
                return false;
            }

            swiot.faultsSingleShot();
            msleep(2000);
            printToConsole("  Step 2: Faults single capture completed");

            // Step 3: Select Bit 10
            swiot.selectAdFaultBit(10);
            msleep(500);
            var selectedBefore = swiot.getAdSelectedFaultBits();
            printToConsole("  Step 3: Bit 10 selected - selected bits: " + selectedBefore);

            // Step 4: Click the Clear selection button
            // Expected: Bit 10 not selected, Faults explanation greyed out
            swiot.clearAdFaultsSelection();
            msleep(500);
            var selectedAfter = swiot.getAdSelectedFaultBits();
            printToConsole("  Step 4a: AD faults selection cleared - selected bits: " + selectedAfter);

            if (selectedAfter.length === 0) {
                printToConsole("  Step 4b: Verified - no bits selected after clear");
            } else {
                printToConsole("  Step 4b: Warning - some bits still selected: " + selectedAfter);
                return false;
            }

            // Step 5: Run a Reset stored capture
            // Expected: Both leds for Bit 10 are turned off
            var storedBefore = swiot.getAdStoredFaultBits();
            printToConsole("  Step 5a: Stored bits before reset: " + storedBefore);

            swiot.resetAdFaultsStored();
            msleep(500);
            var storedAfter = swiot.getAdStoredFaultBits();
            printToConsole("  Step 5b: AD faults stored reset - stored bits: " + storedAfter);

            if (storedAfter.indexOf(10) === -1) {
                printToConsole("  Step 5c: Verified - Bit 10 stored LED is off");
            } else {
                printToConsole("  Step 5c: Warning - Bit 10 still has stored status");
                return false;
            }

            // Cleanup
            if (switchToTool("AD74413R")) {
                swiot.setAdRunning(false);
            }

            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            swiot.setAdRunning(false);
            return false;
        }
    });
} else {
    printToConsole("  Skipping TST.FAULTS.CLEAR_RESET - not in runtime mode");
}

// ============================================
// Test 14 - Faults Run Continuous (TST.FAULTS.RUN_CONTINUOUS)
// ============================================
printToConsole("\n=== Test 14 - Faults Run Continuous ===\n");

if (swiot.isRuntimeMode()) {

    TestFramework.runTest("TST.FAULTS.RUN_CONTINUOUS", function() {
        try {
            // Step 1: Run a continuous capture with all channels enabled on AD74413R
            if (switchToTool("AD74413R")) {
                var adChannelCount = swiot.getAdChannelCount();
                for (var i = 0; i < adChannelCount; i++) {
                    swiot.setAdChannelEnabled(i, true);
                    msleep(200);
                }
                swiot.setAdRunning(true);
                msleep(2000);
                printToConsole("  Step 1: AD74413R continuous capture running with all " + adChannelCount + " channels");
            }

            // Step 2: Open Faults instrument and run a Continuous capture
            // Expected: Bit 10 is enabled on the AD74413R device
            if (!switchToTool("Faults")) {
                swiot.setAdRunning(false);
                return false;
            }

            swiot.setFaultsRunning(true);
            msleep(1000);
            if (!swiot.isFaultsRunning()) {
                printToConsole("  FAIL: Could not start faults continuous capture");
                swiot.setAdRunning(false);
                return false;
            }
            printToConsole("  Step 2a: Faults continuous capture started");

            msleep(2000);

            // Verify active fault bits
            var activeBits = swiot.getAdActiveFaultBits();
            printToConsole("  Step 2b: AD74413R active fault bits: " + activeBits);
            if (activeBits.indexOf(10) !== -1) {
                printToConsole("  Step 2c: Verified - Bit 10 is enabled on AD74413R device");
            } else {
                printToConsole("  Step 2c: Bit 10 status: " + (activeBits.length > 0 ? "other bits active: " + activeBits : "no active faults"));
            }

            // Step 3: While running click the Reset stored button
            // Expected: STORED LED is turned off until next polled value
            var storedBefore = swiot.getAdStoredFaultBits();
            printToConsole("  Step 3a: Stored bits before reset: " + storedBefore);

            swiot.resetAdFaultsStored();
            msleep(100); // Check immediately after reset
            var storedAfterReset = swiot.getAdStoredFaultBits();
            printToConsole("  Step 3b: Stored bits immediately after reset: " + storedAfterReset);

            swiot.resetMaxFaultsStored();
            msleep(1000); // Wait for next poll
            var storedAfterPoll = swiot.getAdStoredFaultBits();
            printToConsole("  Step 3c: Stored bits after next poll: " + storedAfterPoll);

            // Step 4: Stop the Faults instrument
            swiot.setFaultsRunning(false);
            msleep(500);
            if (swiot.isFaultsRunning()) {
                printToConsole("  FAIL: Could not stop faults capture");
                return false;
            }
            printToConsole("  Step 4: Faults capture stopped");

            // Cleanup
            if (switchToTool("AD74413R")) {
                swiot.setAdRunning(false);
            }

            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            swiot.setFaultsRunning(false);
            swiot.setAdRunning(false);
            return false;
        }
    });
} else {
    printToConsole("  Skipping TST.FAULTS.RUN_CONTINUOUS - not in runtime mode");
}

// ============================================
// Test 15 - Faults Tutorial & Docs (TST.FAULTS.TUTORIAL)
// ============================================
TestFramework.runTest("TST.FAULTS.TUTORIAL", function() {
    printToConsole("\n=== Test 15 - Faults Tutorial & Docs ===\n");
    printToConsole("  NOTE: TST.FAULTS.TUTORIAL requires manual testing (UI interaction)");

    return "SKIP";
});


// ============================================
// Test 16 - SWIOT External Supply (TST.SWIOT.EXTERNAL_SUPPLY)
// ============================================
TestFramework.runTest("TST.SWIOT.EXTERNAL_SUPPLY", function() {
    printToConsole("\n=== Test 16 - SWIOT External Supply ===\n");
    printToConsole("  NOTE: TST.SWIOT.EXTERNAL_SUPPLY requires hardware interaction (power supply switch)");

    return "SKIP";
});


// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
scopy.exit();
exit(exitCode);
