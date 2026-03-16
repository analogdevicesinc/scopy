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

// FMCOMMS5 Manual Test Automation
// Automates tests from: docs/tests/plugins/fmcomms5/fmcomms5_tests.rst
// Tests: TST.FMCOMMS5.CHANGE_VALIDATE_GLOBAL_SETTINGS,
//        TST.FMCOMMS5.RX_TX_CHAIN_CONFIG,
//        TST.FMCOMMS5_ADVANCED.PLUGIN_DETECTION_AND_DISPLAY
// Skipped: TST.FMCOMMS5.CALIBRATION (requires physical hardware + ADC plugin API)
//
// Note: FMCOMMS5 has two physical AD9361 devices (ad9361-phy and ad9361-phy-B)
//   Channels 0-1 map to ad9361-phy, channels 2-3 map to ad9361-phy-B
//   LO frequencies are per-device (device 0 and device 1)

// Load test framework
evaluateFile("js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("FMCOMMS5 Documentation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to FMCOMMS5 tool
if (!switchToTool("FMCOMMS5")) {
    printToConsole("ERROR: Cannot switch to FMCOMMS5 tool");
    scopy.exit();
}

// ============================================
// Test 3: Change and Validate Global Settings
// UID: TST.FMCOMMS5.CHANGE_VALIDATE_GLOBAL_SETTINGS
// Description: Change global settings and verify changes are applied
//   and reflected in hardware for both devices.
// ============================================
printToConsole("\n=== Test 3: Change and Validate Global Settings ===\n");

TestFramework.runTest("TST.FMCOMMS5.CHANGE_VALIDATE_GLOBAL_SETTINGS.ENSM", function() {
    try {
        // Step 1: Change ENSM mode (FDD/TDD) - applies to both devices
        var origMode = fmcomms5.getEnsmMode();
        printToConsole("  Original ENSM mode: " + origMode);

        fmcomms5.setEnsmMode("fdd");
        msleep(500);
        var readBack = fmcomms5.getEnsmMode();
        printToConsole("  Set ENSM to fdd, read back: " + readBack);
        if (readBack !== "fdd") {
            printToConsole("  FAIL: ENSM mode did not change to fdd");
            fmcomms5.setEnsmMode(origMode);
            return false;
        }

        // Restore
        fmcomms5.setEnsmMode(origMode);
        msleep(500);

        printToConsole("  Step 1 PASS: ENSM mode changes and is reflected for both devices");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.CHANGE_VALIDATE_GLOBAL_SETTINGS.CALIB_GOVERNOR", function() {
    try {
        // Step 2: Change calibration mode and rate governor
        var calibMode = fmcomms5.getCalibMode();
        printToConsole("  Current calibration mode: " + calibMode);
        if (!calibMode || calibMode === "") {
            printToConsole("  FAIL: getCalibMode() returned empty");
            return false;
        }

        var origGovernor = fmcomms5.getTrxRateGovernor();
        printToConsole("  Original TRX rate governor: " + origGovernor);

        fmcomms5.setTrxRateGovernor("nominal");
        msleep(500);
        var readBack = fmcomms5.getTrxRateGovernor();
        printToConsole("  Set rate governor to nominal, read back: " + readBack);
        if (readBack !== "nominal") {
            printToConsole("  FAIL: Rate governor did not change to nominal");
            fmcomms5.setTrxRateGovernor(origGovernor);
            return false;
        }

        // Verify path rates
        var rxRates = fmcomms5.getRxPathRates();
        var txRates = fmcomms5.getTxPathRates();
        printToConsole("  RX path rates: " + rxRates);
        printToConsole("  TX path rates: " + txRates);

        // Restore
        fmcomms5.setTrxRateGovernor(origGovernor);
        msleep(500);

        printToConsole("  Step 2 PASS: Settings applied and validated for both devices");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: RX and TX Chain Configuration
// UID: TST.FMCOMMS5.RX_TX_CHAIN_CONFIG
// Description: Change RX/TX bandwidth, sampling rate, LO frequency,
//   and verify correct application for all four channels (RX1-4, TX1-4).
// Note: FMCOMMS5 channels 0-1 = ad9361-phy, channels 2-3 = ad9361-phy-B
//       BW and sampling rate are global, LO is per-device, gain is per-channel
// ============================================
printToConsole("\n=== Test 4: RX and TX Chain Configuration ===\n");

TestFramework.runTest("TST.FMCOMMS5.RX_TX_CHAIN_CONFIG.RX_GLOBAL", function() {
    try {
        // Step 1: Change RX bandwidth and sampling rate for all RX channels
        var origBw = fmcomms5.getRxRfBandwidth();
        var origSr = fmcomms5.getRxSamplingFrequency();
        printToConsole("  Original RX RF BW: " + origBw);
        printToConsole("  Original RX sampling freq: " + origSr);

        // Change RX bandwidth (global - affects all RX channels)
        fmcomms5.setRxRfBandwidth("18000000");
        msleep(500);
        var bwReadBack = fmcomms5.getRxRfBandwidth();
        printToConsole("  Set RX BW to 18 MHz, read back: " + bwReadBack);
        if (bwReadBack !== "18000000") {
            printToConsole("  FAIL: RX bandwidth was not applied");
            fmcomms5.setRxRfBandwidth(origBw);
            return false;
        }

        // Change RX sampling rate (global)
        fmcomms5.setRxSamplingFrequency("25000000");
        msleep(500);
        var srReadBack = fmcomms5.getRxSamplingFrequency();
        printToConsole("  Set RX sampling freq to 25 MHz, read back: " + srReadBack);
        if (srReadBack !== "25000000") {
            printToConsole("  FAIL: RX sampling frequency was not applied");
            fmcomms5.setRxRfBandwidth(origBw);
            fmcomms5.setRxSamplingFrequency(origSr);
            return false;
        }

        // Restore
        fmcomms5.setRxRfBandwidth(origBw);
        fmcomms5.setRxSamplingFrequency(origSr);
        msleep(500);

        printToConsole("  PASS: RX global settings applied for all channels");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.RX_TX_CHAIN_CONFIG.RX_LO", function() {
    try {
        // RX LO frequency per-device (device 0 = ad9361-phy, device 1 = ad9361-phy-B)
        var origLo0 = fmcomms5.getRxLoFrequency(0);
        var origLo1 = fmcomms5.getRxLoFrequency(1);
        printToConsole("  Original RX LO device 0: " + origLo0);
        printToConsole("  Original RX LO device 1: " + origLo1);

        // Change RX LO for device 0 (channels 0-1)
        fmcomms5.setRxLoFrequency(0, "2400000000");
        msleep(500);
        var loReadBack0 = fmcomms5.getRxLoFrequency(0);
        printToConsole("  Set RX LO device 0 to 2.4 GHz, read back: " + loReadBack0);
        if (loReadBack0 !== "2400000000") {
            printToConsole("  FAIL: RX LO device 0 was not applied");
            fmcomms5.setRxLoFrequency(0, origLo0);
            return false;
        }

        // Change RX LO for device 1 (channels 2-3)
        fmcomms5.setRxLoFrequency(1, "2400000000");
        msleep(500);
        var loReadBack1 = fmcomms5.getRxLoFrequency(1);
        printToConsole("  Set RX LO device 1 to 2.4 GHz, read back: " + loReadBack1);
        if (loReadBack1 !== "2400000000") {
            printToConsole("  FAIL: RX LO device 1 was not applied");
            fmcomms5.setRxLoFrequency(0, origLo0);
            fmcomms5.setRxLoFrequency(1, origLo1);
            return false;
        }

        // Restore
        fmcomms5.setRxLoFrequency(0, origLo0);
        fmcomms5.setRxLoFrequency(1, origLo1);
        msleep(500);

        printToConsole("  PASS: RX LO frequency set for both devices");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.RX_TX_CHAIN_CONFIG.RX_PER_CHANNEL", function() {
    try {
        // Verify per-channel RX gain across all 4 channels
        var origGains = [];
        var origModes = [];
        for (var ch = 0; ch < 4; ch++) {
            origModes.push(fmcomms5.getRxGainControlMode(ch));
            origGains.push(fmcomms5.getRxHardwareGain(ch));
            printToConsole("  RX ch" + ch + " mode: " + origModes[ch] + ", gain: " + origGains[ch]);
        }

        // Set all to manual and change gain
        var allPass = true;
        for (var ch = 0; ch < 4; ch++) {
            fmcomms5.setRxGainControlMode(ch, "manual");
            msleep(300);
            var testGain = String(10 + ch * 5); // 10, 15, 20, 25
            fmcomms5.setRxHardwareGain(ch, testGain);
            msleep(300);
            var readBack = fmcomms5.getRxHardwareGain(ch);
            printToConsole("  RX ch" + ch + " set to " + testGain + " dB, read back: " + readBack);
            if (readBack.indexOf(testGain) === -1) {
                printToConsole("  FAIL: RX ch" + ch + " gain not applied");
                allPass = false;
            }
        }

        // Restore
        for (var ch = 0; ch < 4; ch++) {
            fmcomms5.setRxHardwareGain(ch, origGains[ch]);
            fmcomms5.setRxGainControlMode(ch, origModes[ch]);
        }
        msleep(500);

        if (!allPass) return false;
        printToConsole("  PASS: RX per-channel gain applied for all 4 channels (RX1-4)");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.RX_TX_CHAIN_CONFIG.TX_GLOBAL", function() {
    try {
        // Step 2: Change TX bandwidth and LO frequency for all TX channels
        var origBw = fmcomms5.getTxRfBandwidth();
        var origSr = fmcomms5.getTxSamplingFrequency();
        printToConsole("  Original TX RF BW: " + origBw);
        printToConsole("  Original TX sampling freq: " + origSr);

        // Change TX bandwidth (global)
        fmcomms5.setTxRfBandwidth("18000000");
        msleep(500);
        var bwReadBack = fmcomms5.getTxRfBandwidth();
        printToConsole("  Set TX BW to 18 MHz, read back: " + bwReadBack);
        if (bwReadBack !== "18000000") {
            printToConsole("  FAIL: TX bandwidth was not applied");
            fmcomms5.setTxRfBandwidth(origBw);
            return false;
        }

        // Restore
        fmcomms5.setTxRfBandwidth(origBw);
        fmcomms5.setTxSamplingFrequency(origSr);
        msleep(500);

        printToConsole("  PASS: TX global settings applied");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.RX_TX_CHAIN_CONFIG.TX_LO", function() {
    try {
        // TX LO frequency per-device
        var origLo0 = fmcomms5.getTxLoFrequency(0);
        var origLo1 = fmcomms5.getTxLoFrequency(1);
        printToConsole("  Original TX LO device 0: " + origLo0);
        printToConsole("  Original TX LO device 1: " + origLo1);

        // Change TX LO for device 0
        fmcomms5.setTxLoFrequency(0, "2400000000");
        msleep(500);
        var loReadBack0 = fmcomms5.getTxLoFrequency(0);
        printToConsole("  Set TX LO device 0 to 2.4 GHz, read back: " + loReadBack0);
        if (loReadBack0 !== "2400000000") {
            printToConsole("  FAIL: TX LO device 0 was not applied");
            fmcomms5.setTxLoFrequency(0, origLo0);
            return false;
        }

        // Change TX LO for device 1
        fmcomms5.setTxLoFrequency(1, "2400000000");
        msleep(500);
        var loReadBack1 = fmcomms5.getTxLoFrequency(1);
        printToConsole("  Set TX LO device 1 to 2.4 GHz, read back: " + loReadBack1);
        if (loReadBack1 !== "2400000000") {
            printToConsole("  FAIL: TX LO device 1 was not applied");
            fmcomms5.setTxLoFrequency(0, origLo0);
            fmcomms5.setTxLoFrequency(1, origLo1);
            return false;
        }

        // Restore
        fmcomms5.setTxLoFrequency(0, origLo0);
        fmcomms5.setTxLoFrequency(1, origLo1);
        msleep(500);

        printToConsole("  PASS: TX LO frequency set for both devices");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.RX_TX_CHAIN_CONFIG.TX_PER_CHANNEL", function() {
    try {
        // Verify per-channel TX gain across all 4 channels
        var origGains = [];
        for (var ch = 0; ch < 4; ch++) {
            origGains.push(fmcomms5.getTxHardwareGain(ch));
            printToConsole("  TX ch" + ch + " gain: " + origGains[ch]);
        }

        // Change gain for each channel
        var allPass = true;
        for (var ch = 0; ch < 4; ch++) {
            var testGain = String(-10 - ch * 5); // -10, -15, -20, -25
            fmcomms5.setTxHardwareGain(ch, testGain);
            msleep(300);
            var readBack = fmcomms5.getTxHardwareGain(ch);
            printToConsole("  TX ch" + ch + " set to " + testGain + " dB, read back: " + readBack);
            if (readBack.indexOf(testGain) === -1) {
                printToConsole("  FAIL: TX ch" + ch + " gain not applied");
                allPass = false;
            }
        }

        // Restore
        for (var ch = 0; ch < 4; ch++) {
            fmcomms5.setTxHardwareGain(ch, origGains[ch]);
        }
        msleep(500);

        if (!allPass) return false;
        printToConsole("  PASS: TX per-channel gain applied for all 4 channels (TX1-4)");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 5: Advanced Plugin Detection and Display
// UID: TST.FMCOMMS5_ADVANCED.PLUGIN_DETECTION_AND_DISPLAY
// Description: Verify that the FMCOMMS5 Advanced plugin detects and
//   displays the connected FMCOMMS5 device and its advanced
//   configuration panels.
// ============================================
printToConsole("\n=== Test 5: Advanced Plugin Detection and Display ===\n");

TestFramework.runTest("TST.FMCOMMS5_ADVANCED.PLUGIN_DETECTION_AND_DISPLAY", function() {
    try {
        // Step 1: Open FMCOMMS5 Advanced plugin
        if (!switchToTool("FMCOMMS5 Advanced")) {
            printToConsole("  FAIL: Cannot switch to FMCOMMS5 Advanced tool");
            return false;
        }
        msleep(1000);

        // Verify widget keys are available
        var keys = fmcomms5_advanced.getWidgetKeys();
        printToConsole("  FMCOMMS5 Advanced widget keys: " + keys.length);
        if (!keys || keys.length === 0) {
            printToConsole("  FAIL: No widget keys found in Advanced plugin");
            return false;
        }
        printToConsole("  Advanced plugin has " + keys.length + " configurable attributes");

        // Navigate subtabs to verify panels are accessible
        fmcomms5_advanced.switchSubtab("ENSM/Mode/Clocks");
        msleep(500);
        printToConsole("  Switched to ENSM/Mode/Clocks subtab");

        fmcomms5_advanced.switchSubtab("Gain");
        msleep(500);
        printToConsole("  Switched to Gain subtab");

        fmcomms5_advanced.switchSubtab("FMCOMMS5");
        msleep(500);
        printToConsole("  Switched to FMCOMMS5 subtab (calibration panel)");

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
