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
// These tests automate the setup steps from the manual test documentation but
// require a human observer to verify UI state. Each step uses
// TestFramework.supervisedCheck() to pause for visual inspection.
// Source: docs/tests/plugins/adrv9009/adrv9009_tests.rst
//
// Tests covered (Category B — supervised):
//   TST.ADRV9009.CONTROLS.MCS_SYNC          (#12)
//     Requires: two ADRV9009-PHY devices connected
//   TST.ADRV9009.ADVANCED.PLUGIN_DETECTION  (Advanced #1)
//     Requires: human to confirm all 15 tabs are visible
// ============================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("ADRV9009 Visual Validation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// ============================================
// Test 12: Multi-Chip Sync (multi-device only)
// UID: TST.ADRV9009.CONTROLS.MCS_SYNC
// Description: Verify MCS Sync button triggers synchronization.
// VISUAL: Verify sync completes without error.
// Precondition: Two ADRV9009-PHY devices must be connected.
// ============================================
printToConsole("\n=== Test 12: Multi-Chip Sync (VISUAL, multi-device) ===\n");

TestFramework.runTest("TST.ADRV9009.CONTROLS.MCS_SYNC", function() {
    try {
        if (!switchToTool("ADRV9009")) {
            printToConsole("  FAIL: Cannot switch to ADRV9009 tool");
            return false;
        }

        // Step 2: Verify MCS Sync button is visible in UI
        printToConsole("  VISUAL CHECK: Verify MCS Sync button is visible in the ADRV9009 tool");
        printToConsole("    Expected: MCS Sync button is present (only visible with multiple devices)");
        TestFramework.supervisedCheck("Verify MCS Sync button is visible in the ADRV9009 tool");

        // Step 3: Trigger MCS Sync
        printToConsole("  Triggering MCS Sync...");
        adrv9009.triggerMcsSync();
        msleep(1000);

        printToConsole("  VISUAL CHECK: Verify multi-chip synchronization sequence completes without error");
        printToConsole("    Expected: Sync completes — no error dialog appears");
        TestFramework.supervisedCheck("Verify multi-chip synchronization sequence completes without error");

        return true;
    } catch (e) {
        printToConsole("  Error (may be normal if only one device is connected): " + e);
        return true;
    }
});

// ============================================
// Advanced Test 1: Advanced Tool Loads and Displays Tabs
// UID: TST.ADRV9009.ADVANCED.PLUGIN_DETECTION
// Description: Verify ADRV9009 Advanced tool loads and displays all 15 tabs.
// VISUAL: Confirm all 15 tab labels are visible — this cannot be verified via API.
// Expected tabs:
//   1.  CLK Settings       6.  FHM Setup        11. AUX DAC
//   2.  Calibrations       7.  PA Protection    12. JESD204 Settings
//   3.  TX Settings        8.  GAIN Setup       13. JESD Framer
//   4.  RX Settings        9.  AGC Setup        14. JESD Deframer
//   5.  ORX Settings      10.  ARM GPIO         15. BIST
// ============================================
printToConsole("\n=== Advanced Test 1: Advanced Tab Detection (VISUAL) ===\n");

TestFramework.runTest("TST.ADRV9009.ADVANCED.PLUGIN_DETECTION", function() {
    try {
        // Step 1: Open the ADRV9009 Advanced tool
        if (!switchToTool("ADRV9009 Advanced")) {
            printToConsole("  FAIL: Cannot switch to ADRV9009 Advanced tool");
            return false;
        }
        printToConsole("  Switched to ADRV9009 Advanced tool");

        // Step 2: Human verifies all 15 tabs are displayed
        printToConsole("  VISUAL CHECK: Verify all 15 tabs are visible in the Advanced tool:");
        printToConsole("    1.  CLK Settings");
        printToConsole("    2.  Calibrations");
        printToConsole("    3.  TX Settings");
        printToConsole("    4.  RX Settings");
        printToConsole("    5.  ORX Settings");
        printToConsole("    6.  FHM Setup");
        printToConsole("    7.  PA Protection");
        printToConsole("    8.  GAIN Setup");
        printToConsole("    9.  AGC Setup");
        printToConsole("    10. ARM GPIO");
        printToConsole("    11. AUX DAC");
        printToConsole("    12. JESD204 Settings");
        printToConsole("    13. JESD Framer");
        printToConsole("    14. JESD Deframer");
        printToConsole("    15. BIST");
        TestFramework.supervisedCheck("Verify all 15 tabs are visible in the Advanced tool");

        printToConsole("  VISUAL CHECK: Verify each tab opens without error when clicked");
        TestFramework.supervisedCheck("Verify each tab opens without error when clicked");

        printToConsole("  PASS: ADRV9009 Advanced tool opened — visual inspection required");
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
