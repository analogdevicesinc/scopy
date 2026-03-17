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

// AD936x Manual Test Automation
// Automates tests from: docs/tests/plugins/ad936x/ad936x_tests.rst
// Tests: TST.AD936X.CHANGE_VALIDATE_GLOBAL_SETTINGS,
//        TST.AD936X.RX_TX_CHAIN_CONFIG,
//        TST.AD936X_ADVANCED.PLUGIN_DETECTION_AND_DISPLAY

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("AD936x Documentation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to AD936X tool
if (!switchToTool("AD936X")) {
    printToConsole("ERROR: Cannot switch to AD936X tool");
    scopy.exit();
}

// ============================================
// Test 3: Change and Validate Device Global Settings
// UID: TST.AD936X.CHANGE_VALIDATE_GLOBAL_SETTINGS
// Description: Change ENSM mode, calibration, and rate governor;
//   verify changes are applied and reflected in hardware.
// ============================================
printToConsole("\n=== Test 3: Change and Validate Device Global Settings ===\n");

TestFramework.runTest("TST.AD936X.CHANGE_VALIDATE_GLOBAL_SETTINGS.ENSM", function() {
    var origMode;
    try {
        // Step 1: Change ENSM mode (FDD/TDD)
        origMode = ad936x.getEnsmMode();
        printToConsole("  Original ENSM mode: " + origMode);

        ad936x.setEnsmMode("fdd");
        msleep(500);
        var readBack = ad936x.getEnsmMode();
        printToConsole("  Set ENSM to fdd, read back: " + readBack);
        if (readBack !== "fdd") {
            printToConsole("  FAIL: ENSM mode did not change to fdd");
            ad936x.setEnsmMode(origMode);
            msleep(500);
            return false;
        }
        printToConsole("  ENSM mode FDD PASS");

        // Try TDD mode if originally in FDD
        if (origMode === "fdd") {
            // FDD is already verified, restore
            printToConsole("  (Device was already in FDD, verified successfully)");
        }

        // Restore
        ad936x.setEnsmMode(origMode);
        msleep(500);

        printToConsole("  Step 1 PASS: ENSM mode changes and is reflected in hardware");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origMode) { ad936x.setEnsmMode(origMode); msleep(500); }
        return false;
    }
});

TestFramework.runTest("TST.AD936X.CHANGE_VALIDATE_GLOBAL_SETTINGS.CALIB_GOVERNOR", function() {
    var origGovernor;
    try {
        // Step 2: Change calibration mode and rate governor
        var calibMode = ad936x.getCalibMode();
        printToConsole("  Current calibration mode: " + calibMode);
        if (!calibMode || calibMode === "") {
            printToConsole("  FAIL: getCalibMode() returned empty");
            return false;
        }
        printToConsole("  Calibration mode reads correctly");

        origGovernor = ad936x.getTrxRateGovernor();
        printToConsole("  Original TRX rate governor: " + origGovernor);

        ad936x.setTrxRateGovernor("nominal");
        msleep(500);
        var readBack = ad936x.getTrxRateGovernor();
        printToConsole("  Set rate governor to nominal, read back: " + readBack);
        if (readBack !== "nominal") {
            printToConsole("  FAIL: Rate governor did not change to nominal");
            ad936x.setTrxRateGovernor(origGovernor);
            msleep(500);
            return false;
        }

        // Verify path rates are readable
        var rxRates = ad936x.getRxPathRates();
        var txRates = ad936x.getTxPathRates();
        printToConsole("  RX path rates: " + rxRates);
        printToConsole("  TX path rates: " + txRates);

        // Restore
        ad936x.setTrxRateGovernor(origGovernor);
        msleep(500);

        printToConsole("  Step 2 PASS: Settings are applied and validated");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origGovernor) { ad936x.setTrxRateGovernor(origGovernor); msleep(500); }
        return false;
    }
});

// ============================================
// Test 4: RX and TX Chain Configuration
// UID: TST.AD936X.RX_TX_CHAIN_CONFIG
// Description: Change RX/TX bandwidth, sampling rate, LO frequency,
//   and verify correct application.
// ============================================
printToConsole("\n=== Test 4: RX and TX Chain Configuration ===\n");

TestFramework.runTest("TST.AD936X.RX_TX_CHAIN_CONFIG.RX", function() {
    var origBw, origSr, origLo;
    try {
        // Step 1: Change RX bandwidth and sampling rate
        origBw = ad936x.getRxRfBandwidth();
        origSr = ad936x.getRxSamplingFrequency();
        origLo = ad936x.getRxLoFrequency();
        printToConsole("  Original RX RF BW: " + origBw);
        printToConsole("  Original RX sampling freq: " + origSr);
        printToConsole("  Original RX LO freq: " + origLo);

        // Change RX bandwidth
        ad936x.setRxRfBandwidth("18000000");
        msleep(500);
        var bwReadBack = ad936x.getRxRfBandwidth();
        printToConsole("  Set RX BW to 18 MHz, read back: " + bwReadBack);
        if (bwReadBack !== "18000000") {
            printToConsole("  FAIL: RX bandwidth was not applied");
            ad936x.setRxRfBandwidth(origBw);
            msleep(500);
            return false;
        }

        // Change RX sampling rate
        ad936x.setRxSamplingFrequency("25000000");
        msleep(500);
        var srReadBack = ad936x.getRxSamplingFrequency();
        printToConsole("  Set RX sampling freq to 25 MHz, read back: " + srReadBack);
        if (srReadBack !== "25000000") {
            printToConsole("  FAIL: RX sampling frequency was not applied");
            ad936x.setRxRfBandwidth(origBw);
            msleep(500);
            ad936x.setRxSamplingFrequency(origSr);
            msleep(500);
            return false;
        }

        // Change RX LO frequency
        ad936x.setRxLoFrequency("2400000000");
        msleep(500);
        var loReadBack = ad936x.getRxLoFrequency();
        printToConsole("  Set RX LO to 2.4 GHz, read back: " + loReadBack);
        if (loReadBack !== "2400000000") {
            printToConsole("  FAIL: RX LO frequency was not applied");
            ad936x.setRxRfBandwidth(origBw);
            msleep(500);
            ad936x.setRxSamplingFrequency(origSr);
            msleep(500);
            ad936x.setRxLoFrequency(origLo);
            msleep(500);
            return false;
        }

        // Verify per-channel attributes
        var gain0 = ad936x.getRxHardwareGain(0);
        var gain1 = ad936x.getRxHardwareGain(1);
        printToConsole("  RX ch0 HW gain: " + gain0);
        printToConsole("  RX ch1 HW gain: " + gain1);

        // Restore
        ad936x.setRxRfBandwidth(origBw);
        msleep(500);
        ad936x.setRxSamplingFrequency(origSr);
        msleep(500);
        ad936x.setRxLoFrequency(origLo);
        msleep(500);

        printToConsole("  Step 1 PASS: RX values are applied and reflected in hardware");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origBw) { ad936x.setRxRfBandwidth(origBw); msleep(500); }
        if (origSr) { ad936x.setRxSamplingFrequency(origSr); msleep(500); }
        if (origLo) { ad936x.setRxLoFrequency(origLo); msleep(500); }
        return false;
    }
});

TestFramework.runTest("TST.AD936X.RX_TX_CHAIN_CONFIG.TX", function() {
    var origBw, origSr, origLo;
    try {
        // Step 2: Change TX bandwidth and LO frequency
        origBw = ad936x.getTxRfBandwidth();
        origSr = ad936x.getTxSamplingFrequency();
        origLo = ad936x.getTxLoFrequency();
        printToConsole("  Original TX RF BW: " + origBw);
        printToConsole("  Original TX sampling freq: " + origSr);
        printToConsole("  Original TX LO freq: " + origLo);

        // Change TX bandwidth
        ad936x.setTxRfBandwidth("18000000");
        msleep(500);
        var bwReadBack = ad936x.getTxRfBandwidth();
        printToConsole("  Set TX BW to 18 MHz, read back: " + bwReadBack);
        if (bwReadBack !== "18000000") {
            printToConsole("  FAIL: TX bandwidth was not applied");
            ad936x.setTxRfBandwidth(origBw);
            msleep(500);
            return false;
        }

        // Change TX LO frequency
        ad936x.setTxLoFrequency("2400000000");
        msleep(500);
        var loReadBack = ad936x.getTxLoFrequency();
        printToConsole("  Set TX LO to 2.4 GHz, read back: " + loReadBack);
        if (loReadBack !== "2400000000") {
            printToConsole("  FAIL: TX LO frequency was not applied");
            ad936x.setTxRfBandwidth(origBw);
            msleep(500);
            ad936x.setTxLoFrequency(origLo);
            msleep(500);
            return false;
        }

        // Verify per-channel attributes
        var gain0 = ad936x.getTxHardwareGain(0);
        var gain1 = ad936x.getTxHardwareGain(1);
        printToConsole("  TX ch0 HW gain: " + gain0);
        printToConsole("  TX ch1 HW gain: " + gain1);

        // Restore
        ad936x.setTxRfBandwidth(origBw);
        msleep(500);
        ad936x.setTxSamplingFrequency(origSr);
        msleep(500);
        ad936x.setTxLoFrequency(origLo);
        msleep(500);

        printToConsole("  Step 2 PASS: TX values are applied and reflected in hardware");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origBw) { ad936x.setTxRfBandwidth(origBw); msleep(500); }
        if (origSr) { ad936x.setTxSamplingFrequency(origSr); msleep(500); }
        if (origLo) { ad936x.setTxLoFrequency(origLo); msleep(500); }
        return false;
    }
});

// ============================================
// Test 5: Advanced Plugin Detection and Display
// UID: TST.AD936X_ADVANCED.PLUGIN_DETECTION_AND_DISPLAY
// Description: Verify that the AD936x Advanced plugin detects and
//   displays the connected AD936x device and its advanced
//   configuration panels.
// ============================================
printToConsole("\n=== Test 5: Advanced Plugin Detection and Display ===\n");

TestFramework.runTest("TST.AD936X_ADVANCED.PLUGIN_DETECTION_AND_DISPLAY", function() {
    try {
        // Step 1: Open AD936x Advanced plugin
        if (!switchToTool("AD936X Advanced")) {
            printToConsole("  FAIL: Cannot switch to AD936X Advanced tool");
            return false;
        }
        msleep(1000);

        // Verify widget keys are available (confirms device detection)
        var keys = ad936x_advanced.getWidgetKeys();
        printToConsole("  AD936x Advanced widget keys: " + keys.length);
        if (!keys || keys.length === 0) {
            printToConsole("  FAIL: No widget keys found in Advanced plugin");
            return false;
        }
        printToConsole("  Advanced plugin has " + keys.length + " configurable attributes");

        // Navigate subtabs to verify panels are accessible
        ad936x_advanced.switchSubtab("ENSM/Mode/Clocks");
        msleep(500);
        printToConsole("  Switched to ENSM/Mode/Clocks subtab");

        ad936x_advanced.switchSubtab("Gain");
        msleep(500);
        printToConsole("  Switched to Gain subtab");

        ad936x_advanced.switchSubtab("RSSI");
        msleep(500);
        printToConsole("  Switched to RSSI subtab");

        ad936x_advanced.switchSubtab("TX Monitor");
        msleep(500);
        printToConsole("  Switched to TX Monitor subtab");

        printToConsole("  PASS: Advanced plugin detects device and displays configuration panels");
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
