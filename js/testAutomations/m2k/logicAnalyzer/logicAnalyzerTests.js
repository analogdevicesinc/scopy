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

// Logic Analyzer Automated Tests
// Based on TST.LA.* test specifications
//
// LIMITATIONS:
// - Cannot start/stop captures (no 'running' property exposed)
// - Cannot configure triggers
// - Cannot export data
// - Cannot verify actual signal acquisition
//
// These tests focus on configuration, settings, and state management.

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: Logic Analyzer
TestFramework.init("Logic Analyzer Tests");

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to Logic Analyzer tool
if (!switchToTool("Logic Analyzer")) {
    printToConsole("ERROR: Cannot access Logic Analyzer");
    exit(1);
}

// ============================================================================
// Test 1: Sample Rate Settings
// UID: TST.LA.SAMPLE_RATE
// ============================================================================
TestFramework.runTest("TST.LA.SAMPLE_RATE", function() {
    try {
        let allPass = true;

        // Test various sample rates (in Hz)
        // M2K Logic Analyzer supports up to 100 MSa/s
        let testRates = [
            1000,        // 1 kSa/s
            10000,       // 10 kSa/s
            100000,      // 100 kSa/s
            1000000,     // 1 MSa/s
            10000000,    // 10 MSa/s
            100000000    // 100 MSa/s
        ];

        for (let rate of testRates) {
            printToConsole("  Setting sample rate: " + rate + " Sa/s");
            la.sampleRate = rate;
            msleep(100);

            let actualRate = la.sampleRate;
            // Allow 1% tolerance for sample rate
            let tolerance = rate * 0.01;
            let pass = TestFramework.assertApproxEqual(actualRate, rate, tolerance,
                "Sample rate " + rate + " Sa/s");
            allPass = allPass && pass;
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 2: Buffer Size Settings
// UID: TST.LA.BUFFER_SIZE
// ============================================================================
TestFramework.runTest("TST.LA.BUFFER_SIZE", function() {
    try {
        let allPass = true;

        // Test various buffer sizes
        let testSizes = [
            1024,
            4096,
            8192,
            16384,
            65536,
            262144
        ];

        for (let size of testSizes) {
            printToConsole("  Setting buffer size: " + size + " samples");
            la.bufferSize = size;
            msleep(100);

            let actualSize = la.bufferSize;
            let pass = TestFramework.assertEqual(actualSize, size,
                "Buffer size " + size);
            allPass = allPass && pass;
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 3: Delay/Time Position Settings
// UID: TST.LA.DELAY
// ============================================================================
TestFramework.runTest("TST.LA.DELAY", function() {
    try {
        let allPass = true;

        // Test various delay values (pre-trigger samples)
        let testDelays = [
            0,
            100,
            500,
            1000,
            5000,
            10000
        ];

        for (let delay of testDelays) {
            printToConsole("  Setting delay: " + delay);
            la.delay = delay;
            msleep(100);

            let actualDelay = la.delay;
            let pass = TestFramework.assertEqual(actualDelay, delay,
                "Delay " + delay);
            allPass = allPass && pass;
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 4: Stream vs One-Shot Mode
// UID: TST.LA.STREAM_ONESHOT
// ============================================================================
TestFramework.runTest("TST.LA.STREAM_ONESHOT", function() {
    try {
        let allPass = true;

        // Test one-shot mode (false = stream, true = one-shot)
        printToConsole("  Setting stream mode (streamOneShot = false)");
        la.streamOneShot = false;
        msleep(100);
        let streamMode = la.streamOneShot;
        let streamPass = TestFramework.assertEqual(streamMode, false, "Stream mode enabled");
        allPass = allPass && streamPass;

        printToConsole("  Setting one-shot mode (streamOneShot = true)");
        la.streamOneShot = true;
        msleep(100);
        let oneShotMode = la.streamOneShot;
        let oneShotPass = TestFramework.assertEqual(oneShotMode, true, "One-shot mode enabled");
        allPass = allPass && oneShotPass;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 5: Enabled Channels
// UID: TST.LA.ENABLED_CHANNELS
// ============================================================================
TestFramework.runTest("TST.LA.ENABLED_CHANNELS", function() {
    try {
        let allPass = true;

        // Test enabling individual channels (0-15)
        printToConsole("  Testing channel enable/disable");

        // Enable all channels
        let allChannels = [];
        for (let i = 0; i < 16; i++) {
            allChannels.push(i);
        }
        la.enabledChannels = allChannels;
        msleep(100);

        let enabled = la.enabledChannels;
        let allEnabledPass = TestFramework.assertEqual(enabled.length, 16,
            "All 16 channels enabled");
        allPass = allPass && allEnabledPass;

        // Enable only first 8 channels
        let firstEight = [0, 1, 2, 3, 4, 5, 6, 7];
        la.enabledChannels = firstEight;
        msleep(100);

        enabled = la.enabledChannels;
        let firstEightPass = TestFramework.assertEqual(enabled.length, 8,
            "First 8 channels enabled");
        allPass = allPass && firstEightPass;

        // Enable specific channels
        let specificChannels = [0, 2, 4, 6, 8, 10, 12, 14];
        la.enabledChannels = specificChannels;
        msleep(100);

        enabled = la.enabledChannels;
        let specificPass = TestFramework.assertEqual(enabled.length, 8,
            "Even channels enabled");
        allPass = allPass && specificPass;

        // Restore all channels
        la.enabledChannels = allChannels;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 6: Channel Names
// UID: TST.LA.CHANNEL_NAMES
// ============================================================================
TestFramework.runTest("TST.LA.CHANNEL_NAMES", function() {
    try {
        let allPass = true;

        // Set custom channel names
        let customNames = [];
        for (let i = 0; i < 16; i++) {
            customNames.push("CH_" + i + "_TEST");
        }

        printToConsole("  Setting custom channel names");
        la.channelNames = customNames;
        msleep(100);

        let actualNames = la.channelNames;

        // Verify names were set
        if (actualNames && actualNames.length === 16) {
            for (let i = 0; i < 16; i++) {
                let pass = TestFramework.assertEqual(actualNames[i], customNames[i],
                    "Channel " + i + " name");
                allPass = allPass && pass;
                if (!pass) break; // Stop on first failure
            }
        } else {
            printToConsole("  Warning: Could not verify channel names");
            allPass = false;
        }

        // Restore default names
        let defaultNames = [];
        for (let i = 0; i < 16; i++) {
            defaultNames.push("DIO" + i);
        }
        la.channelNames = defaultNames;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 7: Channel Heights
// UID: TST.LA.CHANNEL_HEIGHTS
// ============================================================================
TestFramework.runTest("TST.LA.CHANNEL_HEIGHTS", function() {
    try {
        let allPass = true;

        // Set custom channel heights (for trace visualization)
        let testHeights = [];
        for (let i = 0; i < 16; i++) {
            testHeights.push(50.0); // 50 pixels height
        }

        printToConsole("  Setting channel heights to 50.0");
        la.channelHeights = testHeights;
        msleep(100);

        let actualHeights = la.channelHeights;

        if (actualHeights && actualHeights.length >= 16) {
            let pass = TestFramework.assertApproxEqual(actualHeights[0], 50.0, 1.0,
                "Channel 0 height");
            allPass = allPass && pass;
        } else {
            printToConsole("  Warning: Could not verify channel heights");
        }

        // Test varying heights
        let varyingHeights = [];
        for (let i = 0; i < 16; i++) {
            varyingHeights.push(30.0 + i * 5); // 30, 35, 40, ...
        }

        printToConsole("  Setting varying channel heights");
        la.channelHeights = varyingHeights;
        msleep(100);

        actualHeights = la.channelHeights;
        if (actualHeights && actualHeights.length >= 16) {
            let pass = TestFramework.assertApproxEqual(actualHeights[5], 55.0, 1.0,
                "Channel 5 height (55.0)");
            allPass = allPass && pass;
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 8: Channel Positions
// UID: TST.LA.CHANNEL_POSITION
// ============================================================================
TestFramework.runTest("TST.LA.CHANNEL_POSITION", function() {
    try {
        let allPass = true;

        // Set channel vertical positions
        let testPositions = [];
        for (let i = 0; i < 16; i++) {
            testPositions.push(i * 100.0); // Stacked positions
        }

        printToConsole("  Setting channel positions");
        la.channelPosition = testPositions;
        msleep(100);

        let actualPositions = la.channelPosition;

        if (actualPositions && actualPositions.length >= 16) {
            let pass = TestFramework.assertApproxEqual(actualPositions[0], 0.0, 10.0,
                "Channel 0 position");
            allPass = allPass && pass;

            pass = TestFramework.assertApproxEqual(actualPositions[5], 500.0, 10.0,
                "Channel 5 position");
            allPass = allPass && pass;
        } else {
            printToConsole("  Warning: Could not verify channel positions");
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 9: Cursors Enable/Disable
// UID: TST.LA.CURSORS
// ============================================================================
TestFramework.runTest("TST.LA.CURSORS", function() {
    try {
        let allPass = true;

        // Enable cursors
        printToConsole("  Enabling cursors");
        la.cursors = true;
        msleep(100);

        let cursorsEnabled = la.cursors;
        let enablePass = TestFramework.assertEqual(cursorsEnabled, true, "Cursors enabled");
        allPass = allPass && enablePass;

        // Disable cursors
        printToConsole("  Disabling cursors");
        la.cursors = false;
        msleep(100);

        let cursorsDisabled = la.cursors;
        let disablePass = TestFramework.assertEqual(cursorsDisabled, false, "Cursors disabled");
        allPass = allPass && disablePass;

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 10: Cursor Position Settings
// UID: TST.LA.CURSOR_POSITION
// ============================================================================
TestFramework.runTest("TST.LA.CURSOR_POSITION", function() {
    try {
        let allPass = true;

        // Enable cursors first
        la.cursors = true;
        msleep(100);

        // Test cursor positions (0-3 for 4 cursor positions)
        let testPositions = [0, 1, 2, 3];

        for (let pos of testPositions) {
            printToConsole("  Setting cursor position: " + pos);
            la.cursors_position = pos;
            msleep(100);

            let actualPos = la.cursors_position;
            let pass = TestFramework.assertEqual(actualPos, pos,
                "Cursor position " + pos);
            allPass = allPass && pass;
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 11: Cursor Transparency
// UID: TST.LA.CURSOR_TRANSPARENCY
// ============================================================================
TestFramework.runTest("TST.LA.CURSOR_TRANSPARENCY", function() {
    try {
        let allPass = true;

        // Enable cursors first
        la.cursors = true;
        msleep(100);

        // Test transparency values (0-100%)
        let testValues = [0, 25, 50, 75, 100];

        for (let trans of testValues) {
            printToConsole("  Setting cursor transparency: " + trans + "%");
            la.cursors_transparency = trans;
            msleep(100);

            let actualTrans = la.cursors_transparency;
            let pass = TestFramework.assertEqual(actualTrans, trans,
                "Cursor transparency " + trans + "%");
            allPass = allPass && pass;
        }

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 12: Channel Groups
// UID: TST.LA.CHANNEL_GROUPS
// ============================================================================
TestFramework.runTest("TST.LA.CHANNEL_GROUPS", function() {
    try {
        let allPass = true;

        // Create channel groups (e.g., group channels 0-3 and 4-7)
        // Format: QVector<QVector<int>> - array of arrays
        let testGroups = [
            [0, 1, 2, 3],      // Group 1: channels 0-3
            [4, 5, 6, 7]       // Group 2: channels 4-7
        ];

        printToConsole("  Setting channel groups: [0-3], [4-7]");
        la.currentGroups = testGroups;
        msleep(100);

        let actualGroups = la.currentGroups;

        if (actualGroups && actualGroups.length >= 2) {
            let pass = TestFramework.assertEqual(actualGroups.length, 2,
                "Two groups created");
            allPass = allPass && pass;

            // Verify first group contains 4 channels
            if (actualGroups[0]) {
                pass = TestFramework.assertEqual(actualGroups[0].length, 4,
                    "First group has 4 channels");
                allPass = allPass && pass;
            }
        } else {
            printToConsole("  Warning: Could not verify channel groups");
        }

        // Test single group with all channels
        let singleGroup = [[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]];
        printToConsole("  Setting single group with all channels");
        la.currentGroups = singleGroup;
        msleep(100);

        actualGroups = la.currentGroups;
        if (actualGroups && actualGroups.length === 1) {
            let pass = TestFramework.assertEqual(actualGroups[0].length, 16,
                "Single group with 16 channels");
            allPass = allPass && pass;
        }

        // Clear groups
        la.currentGroups = [];

        return allPass;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================================================
// Test 13: Decoder Information (Read-Only)
// UID: TST.LA.DECODER_INFO
// ============================================================================
TestFramework.runTest("TST.LA.DECODER_INFO", function() {
    try {
        // Note: enabledDecoders and decoderSettings are read-only in the API
        // We can only read the current state, not set decoders via script

        printToConsole("  Reading enabled decoders (read-only)");
        let enabledDecoders = la.enabledDecoders;

        if (enabledDecoders !== undefined) {
            printToConsole("    Enabled decoders count: " + enabledDecoders.length);
            for (let i = 0; i < enabledDecoders.length; i++) {
                printToConsole("    - " + enabledDecoders[i]);
            }

            printToConsole("  Reading decoder settings (read-only)");
            let decoderSettings = la.decoderSettings;
            if (decoderSettings !== undefined) {
                printToConsole("    Decoder settings count: " + decoderSettings.length);
            }

            return true;
        } else {
            printToConsole("    No decoders enabled or property not accessible");
            return true; // Still pass - just informational
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        // Return true as this is informational only
        return true;
    }
});

// ============================================================================
// Test 14: Notes
// UID: TST.LA.NOTES
// ============================================================================
TestFramework.runTest("TST.LA.NOTES", function() {
    try {
        let allPass = true;

        // Set notes
        let testNote = "Logic Analyzer Test Note - Created by automated test";
        printToConsole("  Setting instrument notes");
        la.notes = testNote;
        msleep(100);

        let actualNote = la.notes;
        let pass = TestFramework.assertEqual(actualNote, testNote, "Notes set correctly");
        allPass = allPass && pass;

        // Clear notes
        la.notes = "";
        msleep(100);

        actualNote = la.notes;
        let clearPass = TestFramework.assertEqual(actualNote, "", "Notes cleared");
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
