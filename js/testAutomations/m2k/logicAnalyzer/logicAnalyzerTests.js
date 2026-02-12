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

// Logic Analyzer Automated Tests
// Based on RST test documentation for TST.LOGIC.*
//
// API LIMITATIONS:
// The Logic Analyzer API only exposes configuration properties (sampleRate,
// bufferSize, delay, enabledChannels, channelNames, channelHeights, cursors,
// decoders, groups, notes). It does NOT expose:
//   - running (cannot start/stop captures)
//   - trigger configuration (cannot set edge conditions)
//   - decoder output readback (cannot verify decoded data)
//   - export/print functionality
//
// As a result, only TST.LOGIC.CHN_VISUAL (steps 3-4) can be automated.
// All other RST tests (CHN_TRIGGER, EXT_CHN_TRIGGER, OSC_EXT_TRIGGER,
// TRIGGER_MODES, CLOCK_SIGNAL, PARALLEL_DECODER, SPI_DECODER, UART_DECODER,
// PWM_DECODER, EXPORT, PRINT_PLOT, DECODER_TABLE) are blocked until the API
// is extended.

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
// TST.LOGIC.CHN_VISUAL
// Steps 3-4: Channel name and trace height
// Step 5 (color to transparent) is not automatable — no color API exposed.
// ============================================================================
TestFramework.runTest("TST.LOGIC.CHN_VISUAL", function() {
    try {
        let allPass = true;

        // Step 3: Change channel name to "D0"
        let customNames = [];
        for (let i = 0; i < 16; i++) {
            customNames.push("DIO" + i);
        }
        customNames[0] = "D0";

        printToConsole("  Setting DIO0 name to 'D0'");
        logic.channelNames = customNames;
        msleep(1000);

        let actualNames = logic.channelNames;
        if (actualNames && actualNames.length >= 1) {
            let pass = TestFramework.assertEqual(actualNames[0], "D0",
                "Channel 0 name set to 'D0'");
            allPass = allPass && pass;
        } else {
            printToConsole("  ✗ Could not read channel names");
            allPass = false;
        }

        // Step 4: Change trace height to 50
        let heights = [];
        for (let i = 0; i < 16; i++) {
            heights.push(50.0);
        }

        printToConsole("  Setting trace height to 50");
        logic.channelHeights = heights;
        msleep(1000);

        let actualHeights = logic.channelHeights;
        if (actualHeights && actualHeights.length >= 1) {
            let pass = TestFramework.assertApproxEqual(actualHeights[0], 50.0, 1.0,
                "Channel 0 trace height set to 50");
            allPass = allPass && pass;
        } else {
            printToConsole("  ✗ Could not read channel heights");
            allPass = false;
        }

        // Step 5: Change color to transparent — NOT AUTOMATABLE (no color API)
        printToConsole("  NOTE: Step 5 (color to transparent) not automatable - no color API");

        // Restore default name
        customNames[0] = "DIO0";
        logic.channelNames = customNames;

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
