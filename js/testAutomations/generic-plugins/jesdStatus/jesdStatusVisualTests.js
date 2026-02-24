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
// Source: docs/tests/plugins/jesd/jesd_status_tests.rst
// ============================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("JESD STATUS Visual Validation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// ============================================
// Test 1: Plugin Loads and Displays JESD Device
// UID: TST.JESD_STATUS.PLUGIN_LOADS_AND_DISPLAYS_DEVICE
// Description: Verify that the JESD STATUS tool loads and displays the
//   connected JESD204 Link Layer Device and its status panels.
// ============================================
printToConsole("\n=== Test 1: Plugin Loads and Displays JESD Device (SUPERVISED) ===\n");

TestFramework.runTest("TST.JESD_STATUS.PLUGIN_LOADS_AND_DISPLAYS_DEVICE", function() {
    try {
        // Step 1: Open JESD STATUS tool
        printToConsole("  Step 1: Opening JESD STATUS tool...");
        if (!switchToTool("JESD STATUS")) {
            printToConsole("  FAIL: Cannot switch to JESD STATUS tool");
            return false;
        }

        // Verify programmatic data is available
        var tools = jesdstatus.getTools();
        printToConsole("  Tools found: " + tools);
        if (!tools || tools.length === 0) {
            printToConsole("  FAIL: No tools found for JESD STATUS plugin");
            return false;
        }

        var devices = jesdstatus.getDevices();
        printToConsole("  JESD devices found: " + devices);
        if (!devices || devices.length === 0) {
            printToConsole("  FAIL: No JESD devices detected");
            return false;
        }

        var selectedDevice = jesdstatus.getSelectedDevice();
        printToConsole("  Selected device: " + selectedDevice);

        // Read link status to verify data flow
        jesdstatus.refresh();
        msleep(500);

        var linkState = jesdstatus.getLinkState();
        var linkStatus = jesdstatus.getLinkStatus();
        printToConsole("  Link State: " + linkState);
        printToConsole("  Link Status: " + linkStatus);

        // Supervised visual check
        if (!TestFramework.supervisedCheck(
            "Verify the JESD STATUS tool displays the JESD204 Link Layer " +
            "Device (e.g., axi-jesd204-rx) and its status panels are visible.")) {
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 6: UI/UX and Color Coding
// UID: TST.JESD_STATUS.UI_UX_COLOR_CODING
// Description: Verify that status fields use correct color coding
//   and the UI is clear and responsive.
// ============================================
printToConsole("\n=== Test 6: UI/UX and Color Coding (SUPERVISED) ===\n");

TestFramework.runTest("TST.JESD_STATUS.UI_UX_COLOR_CODING", function() {
    try {
        // Ensure we are on the JESD STATUS tool
        if (!switchToTool("JESD STATUS")) {
            printToConsole("  FAIL: Cannot switch to JESD STATUS tool");
            return false;
        }

        // Refresh to get latest data
        jesdstatus.refresh();
        msleep(500);

        // Read current status values to provide context for visual inspection
        var linkState = jesdstatus.getLinkState();
        var linkStatus = jesdstatus.getLinkStatus();
        var syncState = jesdstatus.getSyncState();
        var sysrefCaptured = jesdstatus.getSysrefCaptured();
        var sysrefAlignError = jesdstatus.getSysrefAlignmentError();

        printToConsole("  Current link state: " + linkState);
        printToConsole("  Current link status: " + linkStatus);
        printToConsole("  Current sync state: " + syncState);
        printToConsole("  Current SYSREF captured: " + sysrefCaptured);
        printToConsole("  Current SYSREF alignment error: " + sysrefAlignError);

        // Print lane info for context
        var laneCount = jesdstatus.getLaneCount();
        printToConsole("  Lane count: " + laneCount);
        for (var lane = 0; lane < laneCount; lane++) {
            var errors = jesdstatus.getLaneErrors(lane);
            printToConsole("  Lane " + lane + " errors: " + errors);
        }

        // Supervised visual check
        if (!TestFramework.supervisedCheck(
            "Verify that status fields use correct color coding: " +
            "green for OK/enabled states, red for errors/disabled states. " +
            "Check that the UI is clear and responsive.")) {
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
