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

// Pattern Generator Automated Tests
// Based on RST test documentation for TST.M2K.PG.*
//
// API LIMITATIONS:
// The Pattern Generator API only exposes configuration properties
// (enabledChannels, channelNames, channelHeights, channelPosition,
// currentGroups, enabledPatterns, notes). It does NOT expose:
//   - running (cannot start/stop pattern generation)
//   - pattern parameters (frequency, duty cycle, phase, baud rate)
//   - output mode (PP/OD)
//   - print/export functionality
//
// As a result, only TST.M2K.PG.OTHER_FEATURES (steps 2-8) can be automated.
// All other RST tests (INDIVIDUAL_CHANNEL_OPERATION, GROUP_CHANNEL_OPERATION,
// SIMULTANEOUS_GROUP_AND_INDIVIDUAL_CHANNELS_OPERATION, and remaining
// OTHER_FEATURES steps) are blocked until the API is extended.

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: Pattern Generator
TestFramework.init("Pattern Generator Tests");

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to Pattern Generator tool
if (!switchToTool("Pattern Generator")) {
    printToConsole("ERROR: Cannot access Pattern Generator");
    exit(1);
}

// ============================================================================
// TST.M2K.PG.OTHER_FEATURES
// Steps 2-8: Channel naming and trace height
// Steps 9-13 (knobs) not automatable — requires UI knob interaction.
// Steps 14-27 (PP/OD output) not automatable — no running or output mode API.
// Steps 28-30 (print) not automatable — no print API.
// Step 31 (info button) not automatable — UI only.
// ============================================================================
TestFramework.runTest("TST.M2K.PG.OTHER_FEATURES", function() {
    try {
        let allPass = true;

        // Steps 2-3: Rename DIO0 to 'CH 0'
        let customNames = [];
        for (let i = 0; i < 16; i++) {
            customNames.push("DIO" + i);
        }
        customNames[0] = "CH 0";

        printToConsole("  Setting DIO0 name to 'CH 0'");
        pattern.channelNames = customNames;
        msleep(100);

        let actualNames = pattern.channelNames;
        if (actualNames && actualNames.length >= 1) {
            let pass = TestFramework.assertEqual(actualNames[0], "CH 0",
                "Channel 0 renamed to 'CH 0'");
            allPass = allPass && pass;
        } else {
            printToConsole("  ✗ Could not read channel names");
            allPass = false;
        }

        // Steps 4-5: Rename DIO1 to 'CH 1'
        customNames[1] = "CH 1";
        pattern.channelNames = customNames;
        msleep(100);

        actualNames = pattern.channelNames;
        if (actualNames && actualNames.length >= 2) {
            let pass = TestFramework.assertEqual(actualNames[1], "CH 1",
                "Channel 1 renamed to 'CH 1'");
            allPass = allPass && pass;
        }

        // Steps 6-7: Change trace height to 50
        let heights = [];
        for (let i = 0; i < 16; i++) {
            heights.push(50.0);
        }

        printToConsole("  Setting trace height to 50");
        pattern.channelHeights = heights;
        msleep(100);

        let actualHeights = pattern.channelHeights;
        if (actualHeights && actualHeights.length >= 1) {
            let pass = TestFramework.assertApproxEqual(actualHeights[0], 50.0, 1.0,
                "Trace height set to 50");
            allPass = allPass && pass;
        } else {
            printToConsole("  ✗ Could not read channel heights");
            allPass = false;
        }

        // Step 8: Change trace height to 10
        for (let i = 0; i < 16; i++) {
            heights[i] = 10.0;
        }

        printToConsole("  Setting trace height to 10");
        pattern.channelHeights = heights;
        msleep(100);

        actualHeights = pattern.channelHeights;
        if (actualHeights && actualHeights.length >= 1) {
            let pass = TestFramework.assertApproxEqual(actualHeights[0], 10.0, 1.0,
                "Trace height set to 10");
            allPass = allPass && pass;
        }

        // Steps 9-13: Knob testing — NOT AUTOMATABLE (requires UI knob interaction)
        printToConsole("  NOTE: Steps 9-13 (knob increment/decrement) not automatable");

        // Steps 14-27: PP/OD output — NOT AUTOMATABLE (no running or output mode API)
        printToConsole("  NOTE: Steps 14-27 (PP/OD output) not automatable - no running API");

        // Steps 28-30: Print — NOT AUTOMATABLE
        printToConsole("  NOTE: Steps 28-30 (print to PDF) not automatable");

        // Restore default names
        let defaultNames = [];
        for (let i = 0; i < 16; i++) {
            defaultNames.push("DIO" + i);
        }
        pattern.channelNames = defaultNames;

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
printToConsole(exitCode);
scopy.exit();
