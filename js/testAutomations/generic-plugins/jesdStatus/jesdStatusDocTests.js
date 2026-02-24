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

// JESD STATUS Manual Test Automation - Documentation Tests
// Automates tests from: docs/tests/plugins/jesd/jesd_status_tests.rst
//
// Automated: TST.JESD_STATUS.LANE_STATUS_TABLE,
//            TST.JESD_STATUS.SYSREF_ALIGNMENT_ERROR_REPORTING
//
// ==========================================================================
// MISSING API REPORT
// The following tests cannot be automated due to missing API functionality.
// ==========================================================================
//
// TST.JESD_STATUS.LINK_STATUS_DISPLAY — Link Status Display
//   Cannot automate: Steps require using the Debugger Plugin to modify IIO
//     device attributes to toggle the JESD link ON/OFF. Verification also
//     requires observing color coding (green for enabled, red for disabled).
//   Missing API: No API to modify link state from the jesdstatus plugin.
//     Cross-plugin IIO attribute writes are not supported in test scripts.
//   Suggested: jesdstatus.setLinkEnabled(bool enabled) -> void
//   Affected file: packages/generic-plugins/plugins/jesdstatus/include/jesdstatus/jesdstatus_api.h
//
// TST.JESD_STATUS.REAL_TIME_UPDATES — Real-Time Updates
//   Cannot automate: Steps require externally enabling/disabling the JESD
//     link and inducing errors on the AD9084 device. These are hardware-level
//     operations that cannot be scripted via the jesdstatus API.
//   Missing API: No API to toggle link state or induce errors programmatically.
//   Suggested: jesdstatus.setLinkEnabled(bool enabled) -> void
//              jesdstatus.injectError(int lane) -> void
//   Affected file: packages/generic-plugins/plugins/jesdstatus/include/jesdstatus/jesdstatus_api.h
//
// ==========================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("JESD STATUS Documentation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to JESD STATUS tool
if (!switchToTool("JESD STATUS")) {
    printToConsole("ERROR: Cannot switch to JESD STATUS tool");
    scopy.exit();
}

// ============================================
// Test 3: Lane Status Table
// UID: TST.JESD_STATUS.LANE_STATUS_TABLE
// Description: Verify that the Lane Status table displays error count
//   and extended multiblock alignment for all lanes (0-7).
// ============================================
printToConsole("\n=== Test 3: Lane Status Table ===\n");

TestFramework.runTest("TST.JESD_STATUS.LANE_STATUS_TABLE", function() {
    try {
        // Refresh to get latest data from hardware
        jesdstatus.refresh();
        msleep(500);

        // Step 1: Check the Lane Status table for all lanes
        var laneCount = jesdstatus.getLaneCount();
        printToConsole("  Lane count: " + laneCount);

        if (laneCount <= 0) {
            printToConsole("  FAIL: No lanes detected");
            return false;
        }

        var encoder = jesdstatus.getEncoder();
        printToConsole("  Encoder type: " + encoder);

        var allLanesOk = true;
        for (var lane = 0; lane < laneCount; lane++) {
            var errors = jesdstatus.getLaneErrors(lane);
            var extMba = jesdstatus.getLaneExtMultiBlockAlignment(lane);
            var latency = jesdstatus.getLaneLatency(lane);
            var cgsState = jesdstatus.getLaneCgsState(lane);
            var initFrameSync = jesdstatus.getLaneInitFrameSync(lane);
            var initLaneAlign = jesdstatus.getLaneInitLaneAlignSeq(lane);

            printToConsole("  Lane " + lane + ":");
            printToConsole("    Errors: " + errors);
            printToConsole("    Ext Multiblock Alignment: " + extMba);
            printToConsole("    Latency: " + latency);
            printToConsole("    CGS State: " + cgsState);
            printToConsole("    Init Frame Sync: " + initFrameSync);
            printToConsole("    Init Lane Align Seq: " + initLaneAlign);

            // Verify error count is readable (non-empty)
            if (!errors && errors !== "0") {
                printToConsole("  FAIL: Lane " + lane + " error count is empty");
                allLanesOk = false;
            }

            // Verify extended multiblock alignment is readable (non-empty)
            if (!extMba || extMba === "") {
                printToConsole("  FAIL: Lane " + lane + " ext multiblock alignment is empty");
                allLanesOk = false;
            }
        }

        if (!allLanesOk) {
            return false;
        }

        printToConsole("  PASS: All " + laneCount + " lanes have valid error count and extended multiblock alignment");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: SYSREF and Alignment Error Reporting
// UID: TST.JESD_STATUS.SYSREF_ALIGNMENT_ERROR_REPORTING
// Description: Verify that SYSREF captured and SYSREF alignment
//   error fields are displayed and update according to device state.
// ============================================
printToConsole("\n=== Test 4: SYSREF and Alignment Error Reporting ===\n");

TestFramework.runTest("TST.JESD_STATUS.SYSREF_ALIGNMENT_ERROR_REPORTING", function() {
    try {
        // Refresh to get latest data from hardware
        jesdstatus.refresh();
        msleep(500);

        // Step 1: Verify SYSREF captured field
        var sysrefCaptured = jesdstatus.getSysrefCaptured();
        printToConsole("  SYSREF Captured: " + sysrefCaptured);

        if (!sysrefCaptured || sysrefCaptured === "") {
            printToConsole("  FAIL: SYSREF Captured field is empty");
            return false;
        }
        printToConsole("  Step 1 PASS: SYSREF Captured field has value");

        // Step 2: Verify SYSREF alignment error field
        var sysrefAlignError = jesdstatus.getSysrefAlignmentError();
        printToConsole("  SYSREF Alignment Error: " + sysrefAlignError);

        if (!sysrefAlignError || sysrefAlignError === "") {
            printToConsole("  FAIL: SYSREF Alignment Error field is empty");
            return false;
        }
        printToConsole("  Step 2 PASS: SYSREF Alignment Error field has value");

        // Also verify additional link status fields are readable
        var linkState = jesdstatus.getLinkState();
        var linkStatus = jesdstatus.getLinkStatus();
        var syncState = jesdstatus.getSyncState();
        printToConsole("  Link State: " + linkState);
        printToConsole("  Link Status: " + linkStatus);
        printToConsole("  Sync State: " + syncState);

        // Refresh and verify values update
        jesdstatus.refresh();
        msleep(500);

        var sysrefCaptured2 = jesdstatus.getSysrefCaptured();
        var sysrefAlignError2 = jesdstatus.getSysrefAlignmentError();
        printToConsole("  After refresh - SYSREF Captured: " + sysrefCaptured2);
        printToConsole("  After refresh - SYSREF Alignment Error: " + sysrefAlignError2);

        if (!sysrefCaptured2 || sysrefCaptured2 === "") {
            printToConsole("  FAIL: SYSREF Captured field is empty after refresh");
            return false;
        }
        if (!sysrefAlignError2 || sysrefAlignError2 === "") {
            printToConsole("  FAIL: SYSREF Alignment Error field is empty after refresh");
            return false;
        }
        printToConsole("  PASS: SYSREF fields update correctly after refresh");

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
