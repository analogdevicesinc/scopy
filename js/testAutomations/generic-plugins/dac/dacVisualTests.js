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
// ALL THE FOLLOWING TESTS REQUIRE SUPERVISED VISUAL VALIDATION
// These tests automate the steps from the manual test documentation but
// require a human observer to verify UI changes. After each visual check,
// the test will prompt the user to input 'y' (pass) or 'n' (fail).
// Source: docs/tests/plugins/dac/dac_tests.rst
//
// Tests in this file:
//   TST.DAC.BUFFER_LOAD           - Buffer mode file loading (EMU)
//   TST.DAC.BUFFER_CHANNEL_SELECT - Buffer mode channel selection (Device)
//   TST.DAC.BUFFER_CYCLIC         - Cyclic buffer mode (Device + loopback)
//   TST.DAC.REPEAT_DATA           - Repeat data option (Device, 4 TX channels)
// ============================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Helper: check if item exists in array
function arrayContains(arr, item) {
    for (var i = 0; i < arr.length; i++) {
        if (arr[i] === item) return true;
    }
    return false;
}

// ============================================================================
// CSV resource file paths
// Adjust CSV_PATH to match your build layout. Common options:
//   - Build dir relative: "packages/generic-plugins/plugins/dac/resources/dac-csv/"
//   - Source tree:         "../packages/generic-plugins/plugins/dac/resources/dac-csv/"
// ============================================================================
var CSV_PATH = "../packages/generic-plugins/plugins/dac/resources/dac-csv/";
var CSV_40K = CSV_PATH + "sine_complex_1_15360_sr_15360000_samps_40000_ampl_32767.csv";
var CSV_20K = CSV_PATH + "sine_complex_1_30720_sr_15360000_samps_20000_ampl_32767.csv";
var CSV_2CH = CSV_PATH + "sinewave_0.9_2ch.csv";
// Invalid file for negative testing (this is a Qt resource, use filesystem path)
var JSON_INVALID = "../packages/generic-plugins/plugins/dac/resources/tutorial_chapters.json";

// Test Suite
TestFramework.init("DAC Visual Validation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to DAC tool
if (!switchToTool("DAC")) {
    printToConsole("ERROR: Cannot switch to DAC tool");
    scopy.exit();
}

// ============================================
// Test 2: Buffer mode file loading
// UID: TST.DAC.BUFFER_LOAD
// Description: Verify that the DAC plugin can load files in buffer mode.
//   Checks file size, Run button state, DATA CONFIGURATION section, and
//   log entries after loading valid and invalid files.
// Preconditions: AdalmPluto.Emu
// ============================================
printToConsole("\n=== Test 2: Buffer mode file loading (SUPERVISED) ===\n");

TestFramework.runTest("TST.DAC.BUFFER_LOAD", function() {
    try {
        // Save original state
        var originalMode = dac.getDeviceMode(0);

        // Step 2: Switch to Buffer mode
        dac.setDeviceMode(0, "Buffer");
        msleep(500);

        // Steps 3-4: Load the 40k sample CSV file
        printToConsole("  Step 3-4: Loading 40k sample CSV file...");
        dac.loadFile(0, CSV_40K);
        msleep(1000);

        // Verify file size via API
        var fileSize = dac.getFileSize(0);
        printToConsole("  File size reported by API: " + fileSize);

        if (fileSize <= 0) {
            printToConsole("  FAIL: File size is 0 or negative after loading");
            dac.setDeviceMode(0, originalMode);
            msleep(500);
            return false;
        }
        printToConsole("  PASS: File loaded, size = " + fileSize);

        // Supervised check for visual elements
        if (!TestFramework.supervisedCheck(
            "After loading the CSV file, verify ALL of the following:\n" +
            "  1. The 'File Size' field shows ~40 ksamples\n" +
            "  2. The Run button is enabled\n" +
            "  3. A 'DATA CONFIGURATION' section appeared\n" +
            "  4. The log shows: 'Successfully loaded: [filename]'")) {
            dac.setDeviceMode(0, originalMode);
            msleep(500);
            return false;
        }

        // Step 5: Run, then stop to reload with a different file
        //   Toggle run off→on around data changes to ensure proper signal
        //   propagation and UI consistency (Qt queued connections).
        printToConsole("  Step 5: Starting DAC run...");
        dac.setDacRunning(true);
        msleep(1000);

        printToConsole("  Stopping DAC to load 2-channel sinewave file...");
        dac.setDacRunning(false);
        msleep(500);
        dac.loadFile(0, CSV_2CH);
        msleep(1000);
        dac.setDacRunning(true);
        msleep(1000);

        var fileSize2 = dac.getFileSize(0);
        printToConsole("  File size after loading 2ch file: " + fileSize2);

        if (!TestFramework.supervisedCheck(
            "Verify the 'File Size' field now shows ~8444 samples")) {
            dac.setDacRunning(false);
            msleep(500);
            dac.setDeviceMode(0, originalMode);
            msleep(500);
            return false;
        }

        // Step 6: Stop, load another file, restart
        printToConsole("  Step 6: Stopping DAC to load 20k sample CSV file...");
        dac.setDacRunning(false);
        msleep(500);
        dac.loadFile(0, CSV_20K);
        msleep(1000);
        dac.setDacRunning(true);
        msleep(1000);

        var fileSize3 = dac.getFileSize(0);
        printToConsole("  File size after loading 20k file: " + fileSize3);

        if (!TestFramework.supervisedCheck(
            "Verify the 'File Size' field now shows ~20 ksamples")) {
            dac.setDacRunning(false);
            msleep(500);
            dac.setDeviceMode(0, originalMode);
            msleep(500);
            return false;
        }

        // Stop DAC before invalid file test
        dac.setDacRunning(false);
        msleep(500);

        // Steps 7-8: Load an invalid (JSON) file
        printToConsole("  Steps 7-8: Loading invalid JSON file...");
        dac.loadFile(0, JSON_INVALID);
        msleep(1000);

        if (!TestFramework.supervisedCheck(
            "After loading the JSON file, verify ALL of the following:\n" +
            "  1. The file was NOT loaded (no data configuration)\n" +
            "  2. The Run button is disabled\n" +
            "  3. The log shows: 'Failed to load: [filename]'")) {
            dac.setDeviceMode(0, originalMode);
            msleep(500);
            return false;
        }

        // Restore original state
        dac.setDeviceMode(0, originalMode);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3: Buffer mode channel selection
// UID: TST.DAC.BUFFER_CHANNEL_SELECT
// Description: Verify channel selection behavior during buffer operation.
//   Tests enabling/disabling channels while running and verifying the DAC
//   responds correctly (continues running or stops).
// Preconditions: AdalmPluto.Device (physical device required)
// ============================================
printToConsole("\n=== Test 3: Buffer mode channel selection (SUPERVISED) ===\n");

TestFramework.runTest("TST.DAC.BUFFER_CHANNEL_SELECT", function() {
    try {
        // Save original state
        var originalMode = dac.getDeviceMode(0);

        // Steps 1-2: Switch to Buffer mode
        dac.setDeviceMode(0, "Buffer");
        msleep(500);

        // Steps 3-4: Load the 40k sample CSV file
        printToConsole("  Loading 40k sample CSV file...");
        dac.loadFile(0, CSV_40K);
        msleep(1000);

        // Step 5: Check enabled channels
        var channels = dac.getBufferChannels(0);
        printToConsole("  Buffer channels: " + channels);

        var enabledCount = 0;
        for (var i = 0; i < channels.length; i++) {
            var enabled = dac.isBufferChannelEnabled(0, channels[i]);
            printToConsole("    " + channels[i] + ": " + (enabled ? "enabled" : "disabled"));
            if (enabled) enabledCount++;
        }
        printToConsole("  Enabled channels: " + enabledCount + "/" + channels.length);

        if (!TestFramework.supervisedCheck(
            "Verify that both channels are enabled in the channels list")) {
            dac.setDeviceMode(0, originalMode);
            msleep(500);
            return false;
        }

        // Step 6: Press Run
        //   After loadFile, the DacInstrument receives data via Qt signals.
        //   We emit runToggled(true) which the DacInstrument connects to
        //   via dacplugin.cpp:133. The running(bool) signal from DacInstrument
        //   feeds back to ToolMenuEntry::setRunning to update the button.
        printToConsole("  Starting DAC...");
        dac.setDacRunning(true);
        msleep(1000);

        var isRunning = dac.isDacRunning();
        printToConsole("  DAC running: " + isRunning);

        if (!isRunning) {
            printToConsole("  FAIL: DAC did not start (isDacRunning = false)");
            dac.setDeviceMode(0, originalMode);
            msleep(500);
            return false;
        }

        if (!TestFramework.supervisedCheck(
            "Verify:\n" +
            "  1. The 'Run' button changed to 'Stop'\n" +
            "  2. Log shows: 'Pushed 40000 samples, 160000 bytes (1/1 buffers)'")) {
            dac.setDacRunning(false);
            msleep(500);
            dac.setDeviceMode(0, originalMode);
            msleep(500);
            return false;
        }

        // Step 7: Disable one channel while running
        //   Channel changes while running cause the DAC to re-push data
        //   with the new channel configuration. Toggle off→on to ensure
        //   the new channel state is picked up cleanly.
        if (channels.length >= 2) {
            printToConsole("  Disabling channel: " + channels[1]);
            dac.setDacRunning(false);
            msleep(500);
            dac.setBufferChannelEnabled(0, channels[1], false);
            msleep(500);
            dac.setDacRunning(true);
            msleep(1000);

            var stillRunning = dac.isDacRunning();
            printToConsole("  DAC still running after disabling one channel: " + stillRunning);

            if (!TestFramework.supervisedCheck(
                "Verify:\n" +
                "  1. The plugin keeps running\n" +
                "  2. Log shows: 'Pushed 40000 samples, 80000 bytes (1/1 buffers)'")) {
                dac.setBufferChannelEnabled(0, channels[1], true);
                msleep(500);
                dac.setDacRunning(false);
                msleep(500);
                dac.setDeviceMode(0, originalMode);
                msleep(500);
                return false;
            }

            // Step 8: Disable the remaining channel
            printToConsole("  Disabling channel: " + channels[0]);
            dac.setDacRunning(false);
            msleep(500);
            dac.setBufferChannelEnabled(0, channels[0], false);
            msleep(500);
            dac.setDacRunning(true);
            msleep(1000);

            var stoppedRunning = dac.isDacRunning();
            printToConsole("  DAC running after disabling all channels: " + stoppedRunning);

            if (!TestFramework.supervisedCheck(
                "Verify:\n" +
                "  1. The plugin stopped running (button shows 'Run' again)\n" +
                "  2. Log shows: 'Unable to create buffer, no channel enabled'")) {
                // Restore channels
                dac.setBufferChannelEnabled(0, channels[0], true);
                msleep(500);
                dac.setBufferChannelEnabled(0, channels[1], true);
                msleep(500);
                dac.setDacRunning(false);
                msleep(500);
                dac.setDeviceMode(0, originalMode);
                msleep(500);
                return false;
            }

            // Restore channels
            dac.setBufferChannelEnabled(0, channels[0], true);
            msleep(500);
            dac.setBufferChannelEnabled(0, channels[1], true);
            msleep(500);
        } else {
            printToConsole("  SKIP: Not enough channels for disable test (need >= 2, have " + channels.length + ")");
        }

        // Restore original state
        dac.setDacRunning(false);
        msleep(500);
        dac.setDeviceMode(0, originalMode);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        dac.setDacRunning(false);
        return false;
    }
});

// ============================================
// Test 5: Cyclic buffer mode
// UID: TST.DAC.BUFFER_CYCLIC
// Description: Verify DAC buffer generation in cyclic mode by transmitting
//   a signal and observing it on the ADC via a loopback cable.
//   Uses iioExplorer API for LO frequency setup and adc API for ADC control.
// Preconditions:
//   - AdalmPluto.Device (physical device)
//   - Loopback cable between RX and TX
//   - Debugger and ADC plugins available
// ============================================
printToConsole("\n=== Test 5: Cyclic buffer mode (SUPERVISED) ===\n");

// IIO attribute paths for ad9361-phy LO channels.
// Adjust IIO_CONTEXT if your device tree uses a different root name.
var IIO_CONTEXT = "context0";
var AD9361_TX_LO_FREQ = IIO_CONTEXT + "/ad9361-phy/altvoltage1/frequency";
var AD9361_RX_LO_FREQ = IIO_CONTEXT + "/ad9361-phy/altvoltage0/frequency";
var LO_FREQUENCY = "2450000000";

TestFramework.runTest("TST.DAC.BUFFER_CYCLIC", function() {
    try {
        // Save original state
        var originalMode = dac.getDeviceMode(0);

        // Steps 1-4: Load file in DAC Buffer mode
        printToConsole("  Switching to Buffer mode...");
        dac.setDeviceMode(0, "Buffer");
        msleep(500);

        // Save original cyclic state and ensure cyclic mode is enabled for this test
        var originalCyclic = dac.isCyclic(0);
        dac.setCyclic(0, true);
        msleep(500);

        printToConsole("  Loading 40k sample CSV file...");
        dac.loadFile(0, CSV_40K);
        msleep(1000);

        // Step 5: Press Run
        printToConsole("  Starting DAC...");
        dac.setDacRunning(true);
        msleep(1000);

        var isRunning = dac.isDacRunning();
        printToConsole("  DAC running: " + isRunning);

        if (!isRunning) {
            printToConsole("  FAIL: DAC did not start (isDacRunning = false)");
            dac.setCyclic(0, originalCyclic);
            msleep(500);
            dac.setDeviceMode(0, originalMode);
            msleep(500);
            return false;
        }

        // Steps 6-7: Configure LO frequencies via iioExplorer API
        //   Save originals, then write 2.45 GHz for TX_LO and RX_LO.
        printToConsole("  Reading original LO frequencies...");
        var origTxLoFreq = iioExplorer.readAttributeValue(AD9361_TX_LO_FREQ);
        var origRxLoFreq = iioExplorer.readAttributeValue(AD9361_RX_LO_FREQ);
        printToConsole("    TX_LO original: " + origTxLoFreq);
        printToConsole("    RX_LO original: " + origRxLoFreq);

        printToConsole("  Setting TX_LO frequency to " + LO_FREQUENCY + "...");
        iioExplorer.writeAttributeValue(AD9361_TX_LO_FREQ, LO_FREQUENCY);
        msleep(500);

        printToConsole("  Setting RX_LO frequency to " + LO_FREQUENCY + "...");
        iioExplorer.writeAttributeValue(AD9361_RX_LO_FREQ, LO_FREQUENCY);
        msleep(500);

        // Verify write
        var txLoResult = iioExplorer.readAttributeValue(AD9361_TX_LO_FREQ);
        var rxLoResult = iioExplorer.readAttributeValue(AD9361_RX_LO_FREQ);
        printToConsole("    TX_LO after write: " + txLoResult);
        printToConsole("    RX_LO after write: " + rxLoResult);

        // Steps 8-10: Configure and start ADC via adc API
        printToConsole("  Saving original ADC buffer size...");
        var origAdcBufferSize = adc.getTimeBufferSize();
        printToConsole("    ADC buffer size original: " + origAdcBufferSize);

        printToConsole("  Setting ADC buffer size to 4096...");
        adc.setTimeBufferSize(4096);
        msleep(500);

        printToConsole("  Starting ADC...");
        adc.setTimeRunning(true);
        msleep(1000);

        var adcRunning = adc.isTimeRunning();
        printToConsole("  ADC running: " + adcRunning);

        // Switch to ADC - Time tool for visual inspection
        switchToTool("ADC - Time");
        msleep(500);

        // Step 11: Check ADC plot for sinewave
        if (!TestFramework.supervisedCheck(
            "On the ADC plot: verify a sinewave is visible.\n" +
            "The signal should be a clear sinusoidal waveform.")) {
            // Restore everything
            adc.setTimeRunning(false);
            msleep(500);
            adc.setTimeBufferSize(origAdcBufferSize);
            msleep(500);
            iioExplorer.writeAttributeValue(AD9361_TX_LO_FREQ, origTxLoFreq);
            msleep(500);
            iioExplorer.writeAttributeValue(AD9361_RX_LO_FREQ, origRxLoFreq);
            msleep(500);
            switchToTool("DAC");
            dac.setDacRunning(false);
            msleep(500);
            dac.setCyclic(0, originalCyclic);
            msleep(500);
            dac.setDeviceMode(0, originalMode);
            msleep(500);
            return false;
        }

        // Step 12: Stop DAC and verify sinewave disappears
        printToConsole("  Stopping DAC...");
        switchToTool("DAC");
        msleep(500);
        dac.setDacRunning(false);
        msleep(1000);

        // Switch back to ADC to observe the result
        switchToTool("ADC - Time");
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "After stopping the DAC: verify the sinewave no longer\n" +
            "appears on the ADC plot (signal should be noise/flat).")) {
            adc.setTimeRunning(false);
            msleep(500);
            adc.setTimeBufferSize(origAdcBufferSize);
            msleep(500);
            iioExplorer.writeAttributeValue(AD9361_TX_LO_FREQ, origTxLoFreq);
            msleep(500);
            iioExplorer.writeAttributeValue(AD9361_RX_LO_FREQ, origRxLoFreq);
            msleep(500);
            switchToTool("DAC");
            dac.setCyclic(0, originalCyclic);
            msleep(500);
            dac.setDeviceMode(0, originalMode);
            msleep(500);
            return false;
        }

        // Restore all cross-plugin state
        printToConsole("  Restoring ADC and Debugger state...");
        adc.setTimeRunning(false);
        msleep(500);
        adc.setTimeBufferSize(origAdcBufferSize);
        msleep(500);
        iioExplorer.writeAttributeValue(AD9361_TX_LO_FREQ, origTxLoFreq);
        msleep(500);
        iioExplorer.writeAttributeValue(AD9361_RX_LO_FREQ, origRxLoFreq);
        msleep(500);

        // Restore DAC state
        switchToTool("DAC");
        msleep(500);
        dac.setCyclic(0, originalCyclic);
        msleep(500);
        dac.setDeviceMode(0, originalMode);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        dac.setDacRunning(false);
        return false;
    }
});

// ============================================
// Test 11: Repeat data option
// UID: TST.DAC.REPEAT_DATA
// Description: Verify the DAC plugin repeat data option. Data sets loaded
//   from the file should be repeated to fill in all enabled channels.
// Preconditions:
//   - AdalmPluto.Device (physical device)
//   - Enable 2 TX channels on your Pluto device (4 voltage channels total)
// ============================================
printToConsole("\n=== Test 11: Repeat data option (SUPERVISED) ===\n");

TestFramework.runTest("TST.DAC.REPEAT_DATA", function() {
    try {
        // Save original state
        var originalMode = dac.getDeviceMode(0);

        // Steps 1-2: Switch to Buffer mode
        dac.setDeviceMode(0, "Buffer");
        msleep(500);

        // Steps 3-4: Load the 2-channel sinewave file
        printToConsole("  Loading 2-channel sinewave file...");
        dac.loadFile(0, CSV_2CH);
        msleep(1000);

        var fileSize = dac.getFileSize(0);
        printToConsole("  File size: " + fileSize);

        // Check available channels
        var channels = dac.getBufferChannels(0);
        printToConsole("  Available channels (" + channels.length + "): " + channels);

        // Verify first 2 channels are enabled
        var enabledChannels = [];
        for (var i = 0; i < channels.length; i++) {
            if (dac.isBufferChannelEnabled(0, channels[i])) {
                enabledChannels.push(channels[i]);
            }
        }
        printToConsole("  Enabled channels: " + enabledChannels.length);

        if (!TestFramework.supervisedCheck(
            "Verify that the first 2 channels are enabled after loading the file")) {
            dac.setDeviceMode(0, originalMode);
            msleep(500);
            return false;
        }

        if (channels.length < 4) {
            printToConsole("  NOTE: This test requires 4 TX channels (2 TX chains).");
            printToConsole("  Only " + channels.length + " channels available.");
            printToConsole("  Skipping steps that require 4 channels.");

            // Test repeat data with available channels
            // Step 6: Set repeat data and run
            var originalRepeat = dac.isRepeatFileBuffer(0);
            dac.setRepeatFileBuffer(0, true);
            msleep(500);

            printToConsole("  Running with Repeat data = true...");
            dac.setDacRunning(true);
            msleep(1000);

            var runState = dac.isDacRunning();
            printToConsole("  DAC running: " + runState);

            if (!TestFramework.supervisedCheck(
                "Verify the log shows the DAC pushing samples successfully\n" +
                "(e.g., 'Pushed X samples, Y bytes (1/1 buffers)')")) {
                dac.setDacRunning(false);
                msleep(500);
                dac.setRepeatFileBuffer(0, originalRepeat);
                msleep(500);
                dac.setDeviceMode(0, originalMode);
                msleep(500);
                return false;
            }

            // Restore
            dac.setDacRunning(false);
            msleep(500);
            dac.setRepeatFileBuffer(0, originalRepeat);
            msleep(500);
            dac.setDeviceMode(0, originalMode);
            msleep(500);

            return true;
        }

        // Step 5: Enable the remaining 2 channels
        printToConsole("  Enabling all channels...");
        for (var j = 0; j < channels.length; j++) {
            dac.setBufferChannelEnabled(0, channels[j], true);
            msleep(500);
        }

        // Save original repeat state
        var originalRepeat = dac.isRepeatFileBuffer(0);

        // Step 6: Set Repeat data to true and Run
        //   All setting changes happen while stopped, then toggle run on.
        printToConsole("  Setting Repeat data = true and running...");
        dac.setRepeatFileBuffer(0, true);
        msleep(500);
        dac.setDacRunning(true);
        msleep(1000);

        printToConsole("  DAC running: " + dac.isDacRunning());

        if (!TestFramework.supervisedCheck(
            "Verify the log shows:\n" +
            "  'Pushed 8444 samples, 67552 bytes (1/1 buffers)'")) {
            dac.setDacRunning(false);
            msleep(500);
            dac.setRepeatFileBuffer(0, originalRepeat);
            msleep(500);
            dac.setDeviceMode(0, originalMode);
            msleep(500);
            return false;
        }

        // Step 7: Stop → change repeat to false → start
        printToConsole("  Stopping DAC, setting Repeat data = false, restarting...");
        dac.setDacRunning(false);
        msleep(500);
        dac.setRepeatFileBuffer(0, false);
        msleep(500);
        dac.setDacRunning(true);
        msleep(1000);

        printToConsole("  DAC running: " + dac.isDacRunning());

        if (!TestFramework.supervisedCheck(
            "Verify the log shows:\n" +
            "  'Not enough data columns for all enabled channels'")) {
            dac.setDacRunning(false);
            msleep(500);
            dac.setRepeatFileBuffer(0, originalRepeat);
            msleep(500);
            dac.setDeviceMode(0, originalMode);
            msleep(500);
            return false;
        }

        // Step 8: Stop → disable last 2 channels → start
        printToConsole("  Stopping DAC, disabling last 2 channels...");
        dac.setDacRunning(false);
        msleep(500);
        dac.setBufferChannelEnabled(0, channels[2], false);
        msleep(500);
        dac.setBufferChannelEnabled(0, channels[3], false);
        msleep(500);

        printToConsole("  Running with only first 2 channels...");
        dac.setDacRunning(true);
        msleep(1000);

        printToConsole("  DAC running: " + dac.isDacRunning());

        if (!TestFramework.supervisedCheck(
            "Verify the log shows:\n" +
            "  'Pushed 8444 samples, 33776 bytes (1/1 buffers)'")) {
            dac.setDacRunning(false);
            msleep(500);
            // Restore all channels
            for (var k = 0; k < channels.length; k++) {
                dac.setBufferChannelEnabled(0, channels[k], true);
                msleep(500);
            }
            dac.setRepeatFileBuffer(0, originalRepeat);
            msleep(500);
            dac.setDeviceMode(0, originalMode);
            msleep(500);
            return false;
        }

        // Restore all state
        dac.setDacRunning(false);
        msleep(500);
        for (var m = 0; m < channels.length; m++) {
            dac.setBufferChannelEnabled(0, channels[m], true);
            msleep(500);
        }
        dac.setRepeatFileBuffer(0, originalRepeat);
        msleep(500);
        dac.setDeviceMode(0, originalMode);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        dac.setDacRunning(false);
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
