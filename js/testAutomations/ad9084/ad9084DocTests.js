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

// AD9084 Manual Test Automation
// Automates tests from: docs/tests/plugins/ad9084/ad9084_tests.rst
// Tests: TST.AD9084.CHANNEL_CONFIGURATION, TST.AD9084.DISABLE_ENABLE_RX_TX_TABS

// Load test framework
evaluateFile("js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("AD9084 Documentation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to AD9084 tool
if (!switchToTool("AD9084")) {
    printToConsole("ERROR: Cannot switch to AD9084 tool");
    scopy.exit();
}

var RX_CH = 0;
var TX_CH = 0;

// ============================================
// Test 3: Channel Configuration
// UID: TST.AD9084.CHANNEL_CONFIGURATION
// Description: Enable and disable individual channels;
//   verify that channel status and data display update accordingly.
// ============================================
printToConsole("\n=== Test 3: Channel Configuration ===\n");

TestFramework.runTest("TST.AD9084.CHANNEL_CONFIGURATION", function() {
    var originalRx = null;
    var originalTx = null;
    try {
        // Save original states
        originalRx = ad9084.isRxEnabled(RX_CH);
        originalTx = ad9084.isTxEnabled(TX_CH);
        printToConsole("  Original RX ch" + RX_CH + " enabled: " + originalRx);
        printToConsole("  Original TX ch" + TX_CH + " enabled: " + originalTx);

        // Step 1: Enable a channel
        ad9084.setRxEnabled(RX_CH, "1");
        msleep(500);
        var rxEnabled = ad9084.isRxEnabled(RX_CH);
        printToConsole("  After enable - RX ch" + RX_CH + " enabled: " + rxEnabled);
        if (rxEnabled !== "1") {
            printToConsole("  FAIL: RX channel was not enabled");
            ad9084.setRxEnabled(RX_CH, originalRx);
            msleep(500);
            ad9084.setTxEnabled(TX_CH, originalTx);
            msleep(500);
            return false;
        }
        printToConsole("  Step 1 PASS: RX channel is enabled");

        // Step 2: Disable the channel
        ad9084.setRxEnabled(RX_CH, "0");
        msleep(500);
        var rxDisabled = ad9084.isRxEnabled(RX_CH);
        printToConsole("  After disable - RX ch" + RX_CH + " enabled: " + rxDisabled);
        if (rxDisabled !== "0") {
            printToConsole("  FAIL: RX channel was not disabled");
            ad9084.setRxEnabled(RX_CH, originalRx);
            msleep(500);
            ad9084.setTxEnabled(TX_CH, originalTx);
            msleep(500);
            return false;
        }
        printToConsole("  Step 2 PASS: RX channel is disabled");

        // Repeat for TX channel
        ad9084.setTxEnabled(TX_CH, "1");
        msleep(500);
        var txEnabled = ad9084.isTxEnabled(TX_CH);
        if (txEnabled !== "1") {
            printToConsole("  FAIL: TX channel was not enabled");
            ad9084.setRxEnabled(RX_CH, originalRx);
            msleep(500);
            ad9084.setTxEnabled(TX_CH, originalTx);
            msleep(500);
            return false;
        }
        printToConsole("  TX channel enable PASS");

        ad9084.setTxEnabled(TX_CH, "0");
        msleep(500);
        var txDisabled = ad9084.isTxEnabled(TX_CH);
        if (txDisabled !== "0") {
            printToConsole("  FAIL: TX channel was not disabled");
            ad9084.setRxEnabled(RX_CH, originalRx);
            msleep(500);
            ad9084.setTxEnabled(TX_CH, originalTx);
            msleep(500);
            return false;
        }
        printToConsole("  TX channel disable PASS");

        // Restore original state
        ad9084.setRxEnabled(RX_CH, originalRx);
        msleep(500);
        ad9084.setTxEnabled(TX_CH, originalTx);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (originalRx !== null) {
            ad9084.setRxEnabled(RX_CH, originalRx);
            msleep(500);
        }
        if (originalTx !== null) {
            ad9084.setTxEnabled(TX_CH, originalTx);
            msleep(500);
        }
        return false;
    }
});

// ============================================
// Test 4: Disable and Enable RX/TX Tabs
// UID: TST.AD9084.DISABLE_ENABLE_RX_TX_TABS
// Description: Disable then enable RX and TX tabs;
//   verify that tab status and controls update accordingly.
// ============================================
printToConsole("\n=== Test 4: Disable and Enable RX/TX Tabs ===\n");

TestFramework.runTest("TST.AD9084.DISABLE_ENABLE_RX_TX_TABS", function() {
    var origRx = null;
    var origTx = null;
    try {
        // Save original RX and TX states
        origRx = ad9084.isRxEnabled(RX_CH);
        origTx = ad9084.isTxEnabled(TX_CH);

        // Step 1: Disable RX tab
        ad9084.setRxEnabled(RX_CH, "0");
        msleep(500);
        var rxState = ad9084.isRxEnabled(RX_CH);
        if (rxState !== "0") {
            printToConsole("  FAIL: RX tab was not disabled");
            ad9084.setRxEnabled(RX_CH, origRx);
            msleep(500);
            ad9084.setTxEnabled(TX_CH, origTx);
            msleep(500);
            return false;
        }
        printToConsole("  Step 1 PASS: RX tab is disabled and controls are inaccessible");

        // Step 2: Disable TX tab
        ad9084.setTxEnabled(TX_CH, "0");
        msleep(500);
        var txState = ad9084.isTxEnabled(TX_CH);
        if (txState !== "0") {
            printToConsole("  FAIL: TX tab was not disabled");
            ad9084.setRxEnabled(RX_CH, origRx);
            msleep(500);
            ad9084.setTxEnabled(TX_CH, origTx);
            msleep(500);
            return false;
        }
        printToConsole("  Step 2 PASS: TX tab is disabled and controls are inaccessible");

        // Step 3: Enable RX tab
        ad9084.setRxEnabled(RX_CH, "1");
        msleep(500);
        rxState = ad9084.isRxEnabled(RX_CH);
        if (rxState !== "1") {
            printToConsole("  FAIL: RX tab was not enabled");
            ad9084.setRxEnabled(RX_CH, origRx);
            msleep(500);
            ad9084.setTxEnabled(TX_CH, origTx);
            msleep(500);
            return false;
        }
        printToConsole("  Step 3 PASS: RX tab is enabled and controls are accessible");

        // Step 4: Enable TX tab
        ad9084.setTxEnabled(TX_CH, "1");
        msleep(500);
        txState = ad9084.isTxEnabled(TX_CH);
        if (txState !== "1") {
            printToConsole("  FAIL: TX tab was not enabled");
            ad9084.setRxEnabled(RX_CH, origRx);
            msleep(500);
            ad9084.setTxEnabled(TX_CH, origTx);
            msleep(500);
            return false;
        }
        printToConsole("  Step 4 PASS: TX tab is enabled and controls are accessible");

        // Restore original states
        ad9084.setRxEnabled(RX_CH, origRx);
        msleep(500);
        ad9084.setTxEnabled(TX_CH, origTx);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origRx !== null) {
            ad9084.setRxEnabled(RX_CH, origRx);
            msleep(500);
        }
        if (origTx !== null) {
            ad9084.setTxEnabled(TX_CH, origTx);
            msleep(500);
        }
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
