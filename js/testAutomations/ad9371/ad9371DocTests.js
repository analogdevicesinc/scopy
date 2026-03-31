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

// AD9371 API Test Automation
// Automates tests from: docs/tests/plugins/ad9371/ad9371_api_tests.rst
//
// Missing API Report (Category C):
//   None — all RST test cases are automatable via the ad9371 API.

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("AD9371 API Documentation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to AD9371 tool
if (!switchToTool("AD9371")) {
    printToConsole("ERROR: Cannot switch to AD9371 tool");
    scopy.exit();
}

// ============================================
// Test 1: Plugin Loads
// UID: TST.AD9371.PLUGIN_LOADS
// ============================================
TestFramework.runTest("TST.AD9371.PLUGIN_LOADS", function() {
    try {
        var tools = ad9371.getTools();
        printToConsole("  Tools found: " + tools);
        if (!tools || tools.length < 2) {
            printToConsole("  FAIL: Expected at least 2 tools");
            return false;
        }
        var hasMain = false;
        var hasAdvanced = false;
        for (var i = 0; i < tools.length; i++) {
            if (tools[i] === "AD9371") hasMain = true;
            if (tools[i] === "AD9371 Advanced") hasAdvanced = true;
        }
        if (!hasMain || !hasAdvanced) {
            printToConsole("  FAIL: Missing AD9371 or AD9371 Advanced tool");
            return false;
        }
        printToConsole("  Both AD9371 and AD9371 Advanced tools are accessible");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2: API Object Registration
// UID: TST.AD9371.API_OBJECT_REGISTRATION
// ============================================
TestFramework.runTest("TST.AD9371.API_OBJECT_REGISTRATION", function() {
    try {
        var tools = ad9371.getTools();
        printToConsole("  ad9371.getTools() returned: " + tools);
        if (!tools || tools.length === 0) {
            printToConsole("  FAIL: getTools() returned empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3: Widget Keys Available
// UID: TST.AD9371.WIDGET_KEYS_AVAILABLE
// ============================================
TestFramework.runTest("TST.AD9371.WIDGET_KEYS_AVAILABLE", function() {
    try {
        var keys = ad9371.getWidgetKeys();
        printToConsole("  Widget keys count: " + keys.length);
        if (!keys || keys.length === 0) {
            printToConsole("  FAIL: getWidgetKeys() returned empty");
            return false;
        }

        var requiredKeys = [
            "ad9371-phy/ensm_mode",
            "ad9371-phy/voltage0_in/hardwaregain"
        ];
        var allFound = true;
        for (var i = 0; i < requiredKeys.length; i++) {
            var found = false;
            for (var j = 0; j < keys.length; j++) {
                if (keys[j] === requiredKeys[i]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                printToConsole("  FAIL: Missing key: " + requiredKeys[i]);
                allFound = false;
            } else {
                printToConsole("  Found key: " + requiredKeys[i]);
            }
        }
        return allFound;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: ENSM Mode Read-Write
// UID: TST.AD9371.ENSM_MODE_READ_WRITE
// ============================================
TestFramework.runTest("TST.AD9371.ENSM_MODE_READ_WRITE", function() {
    try {
        var origMode = ad9371.getEnsmMode();
        printToConsole("  Original ENSM mode: " + origMode);
        if (!origMode || origMode === "") {
            printToConsole("  FAIL: getEnsmMode() returned empty");
            return false;
        }

        // Select the opposite valid mode to avoid writing the same value
        var newMode = (origMode === "radio_on") ? "radio_off" : "radio_on";
        ad9371.setEnsmMode(newMode);
        msleep(500);
        var readBack = ad9371.getEnsmMode();
        printToConsole("  Set ENSM to '" + newMode + "', read back: " + readBack);
        if (readBack !== newMode) {
            printToConsole("  FAIL: ENSM mode did not change to '" + newMode + "'");
            ad9371.setEnsmMode(origMode);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setEnsmMode(origMode);
        msleep(500);
        printToConsole("  Restored ENSM mode to: " + origMode);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 5: RX LO Frequency MHz Conversion
// UID: TST.AD9371.RX_LO_FREQUENCY_CONVERSION
// ============================================
TestFramework.runTest("TST.AD9371.RX_LO_FREQUENCY_CONVERSION", function() {
    try {
        var origFreq = ad9371.getRxLoFrequency();
        printToConsole("  Original RX LO frequency: " + origFreq + " MHz");
        if (!origFreq || origFreq === "") {
            printToConsole("  FAIL: getRxLoFrequency() returned empty");
            return false;
        }

        // Verify it's in MHz range (not Hz)
        var freqVal = parseFloat(origFreq);
        if (freqVal > 1e6) {
            printToConsole("  FAIL: Value appears to be in Hz, not MHz: " + origFreq);
            return false;
        }

        ad9371.setRxLoFrequency("1000");
        msleep(500);
        var readBack = ad9371.getRxLoFrequency();
        printToConsole("  Set RX LO to 1000 MHz, read back: " + readBack);

        var readBackVal = parseFloat(readBack);
        if (Math.abs(readBackVal - 1000.0) > 1.0) {
            printToConsole("  FAIL: RX LO frequency readback mismatch");
            ad9371.setRxLoFrequency(origFreq);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setRxLoFrequency(origFreq);
        msleep(500);
        printToConsole("  Restored RX LO frequency");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 6: TX LO Frequency MHz Conversion
// UID: TST.AD9371.TX_LO_FREQUENCY_CONVERSION
// ============================================
TestFramework.runTest("TST.AD9371.TX_LO_FREQUENCY_CONVERSION", function() {
    try {
        var origFreq = ad9371.getTxLoFrequency();
        printToConsole("  Original TX LO frequency: " + origFreq + " MHz");

        ad9371.setTxLoFrequency("2500");
        msleep(500);
        var readBack = ad9371.getTxLoFrequency();
        printToConsole("  Set TX LO to 2500 MHz, read back: " + readBack);

        var readBackVal = parseFloat(readBack);
        if (Math.abs(readBackVal - 2500.0) > 1.0) {
            printToConsole("  FAIL: TX LO frequency readback mismatch");
            ad9371.setTxLoFrequency(origFreq);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setTxLoFrequency(origFreq);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 7: RX Hardware Gain Per-Channel
// UID: TST.AD9371.RX_HARDWARE_GAIN_PER_CHANNEL
// ============================================
TestFramework.runTest("TST.AD9371.RX_HARDWARE_GAIN_PER_CHANNEL", function() {
    try {
        // Channel 0
        var origGain0 = ad9371.getRxHardwareGain(0);
        printToConsole("  Original RX gain ch0: " + origGain0);

        ad9371.setRxHardwareGain(0, "15");
        msleep(500);
        var readBack0 = ad9371.getRxHardwareGain(0);
        printToConsole("  Set RX gain ch0 to 15, read back: " + readBack0);

        var val0 = parseFloat(readBack0);
        if (Math.abs(val0 - 15.0) > 1.0) {
            printToConsole("  FAIL: RX gain ch0 readback mismatch");
            ad9371.setRxHardwareGain(0, origGain0);
            msleep(500);
            return false;
        }

        // Restore ch0
        ad9371.setRxHardwareGain(0, origGain0);
        msleep(500);

        // Channel 1 (may not exist on non-2Rx2Tx)
        var origGain1 = ad9371.getRxHardwareGain(1);
        if (origGain1 && origGain1 !== "") {
            ad9371.setRxHardwareGain(1, "20");
            msleep(500);
            var readBack1 = ad9371.getRxHardwareGain(1);
            printToConsole("  Set RX gain ch1 to 20, read back: " + readBack1);

            var val1 = parseFloat(readBack1);
            if (Math.abs(val1 - 20.0) > 1.0) {
                printToConsole("  FAIL: RX gain ch1 readback mismatch");
                ad9371.setRxHardwareGain(1, origGain1);
                msleep(500);
                return false;
            }
            ad9371.setRxHardwareGain(1, origGain1);
            msleep(500);
        } else {
            printToConsole("  Channel 1 not available (non-2Rx2Tx), skipping");
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 8: TX Attenuation Negation Conversion
// UID: TST.AD9371.TX_ATTENUATION_NEGATION
// ============================================
TestFramework.runTest("TST.AD9371.TX_ATTENUATION_NEGATION", function() {
    try {
        var origAtten = ad9371.getTxAttenuation(0);
        printToConsole("  Original TX attenuation ch0: " + origAtten);

        ad9371.setTxAttenuation(0, "10");
        msleep(500);
        var readBack = ad9371.getTxAttenuation(0);
        printToConsole("  Set TX atten to 10, read back: " + readBack);

        var attenVal = parseFloat(readBack);
        if (Math.abs(attenVal - 10.0) > 0.1) {
            printToConsole("  FAIL: TX attenuation readback mismatch");
            ad9371.setTxAttenuation(0, origAtten);
            msleep(500);
            return false;
        }

        // Verify raw IIO value is negative
        var rawVal = ad9371.readWidget("ad9371-phy/voltage0_out/hardwaregain");
        printToConsole("  Raw IIO hardwaregain: " + rawVal);
        var rawNum = parseFloat(rawVal);
        if (rawNum >= 0) {
            printToConsole("  FAIL: Raw IIO value should be negative, got: " + rawVal);
            ad9371.setTxAttenuation(0, origAtten);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setTxAttenuation(0, origAtten);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 9: RX Gain Control Mode
// UID: TST.AD9371.RX_GAIN_CONTROL_MODE
// ============================================
TestFramework.runTest("TST.AD9371.RX_GAIN_CONTROL_MODE", function() {
    try {
        var origMode = ad9371.getRxGainControlMode();
        printToConsole("  Original RX gain control mode: " + origMode);
        if (!origMode || origMode === "") {
            printToConsole("  FAIL: getRxGainControlMode() returned empty");
            return false;
        }

        ad9371.setRxGainControlMode("manual");
        msleep(500);
        var readBack = ad9371.getRxGainControlMode();
        printToConsole("  Set to 'manual', read back: " + readBack);
        if (readBack !== "manual") {
            printToConsole("  FAIL: Gain control mode did not change to 'manual'");
            ad9371.setRxGainControlMode(origMode);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setRxGainControlMode(origMode);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 10: RX RSSI Read-Only
// UID: TST.AD9371.RX_RSSI_READ_ONLY
// ============================================
TestFramework.runTest("TST.AD9371.RX_RSSI_READ_ONLY", function() {
    try {
        var rssi = ad9371.getRxRssi(0);
        printToConsole("  RX RSSI ch0: " + rssi);
        if (!rssi || rssi === "") {
            printToConsole("  FAIL: getRxRssi(0) returned empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 11: Calibration Checkbox Read-Write
// UID: TST.AD9371.CALIBRATION_CHECKBOX_READ_WRITE
// ============================================
TestFramework.runTest("TST.AD9371.CALIBRATION_CHECKBOX_READ_WRITE", function() {
    try {
        // Test calibrate_rx_qec_en
        var origRxQec = ad9371.getCalibrateRxQecEn();
        printToConsole("  Original calibrate_rx_qec_en: " + origRxQec);

        ad9371.setCalibrateRxQecEn("1");
        msleep(500);
        var readBack = ad9371.getCalibrateRxQecEn();
        printToConsole("  Set to '1', read back: " + readBack);
        if (readBack !== "1") {
            printToConsole("  FAIL: calibrate_rx_qec_en did not set to '1'");
            ad9371.setCalibrateRxQecEn(origRxQec);
            msleep(500);
            return false;
        }

        // Test calibrate_tx_qec_en
        var origTxQec = ad9371.getCalibrateTxQecEn();
        ad9371.setCalibrateTxQecEn("1");
        msleep(500);
        var readBackTx = ad9371.getCalibrateTxQecEn();
        printToConsole("  Set calibrate_tx_qec_en to '1', read back: " + readBackTx);
        if (readBackTx !== "1") {
            printToConsole("  FAIL: calibrate_tx_qec_en did not set to '1'");
            ad9371.setCalibrateRxQecEn(origRxQec);
            msleep(500);
            ad9371.setCalibrateTxQecEn(origTxQec);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setCalibrateRxQecEn(origRxQec);
        msleep(500);
        ad9371.setCalibrateTxQecEn(origTxQec);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 12: Calibrate Trigger
// UID: TST.AD9371.CALIBRATE_TRIGGER
// ============================================
TestFramework.runTest("TST.AD9371.CALIBRATE_TRIGGER", function() {
    try {
        ad9371.calibrate();
        msleep(500);
        printToConsole("  Calibration triggered without error");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 13: RX Quadrature Tracking
// UID: TST.AD9371.RX_QUADRATURE_TRACKING
// ============================================
TestFramework.runTest("TST.AD9371.RX_QUADRATURE_TRACKING", function() {
    try {
        var origVal = ad9371.getRxQuadratureTracking(0);
        printToConsole("  Original RX quadrature tracking ch0: " + origVal);

        ad9371.setRxQuadratureTracking(0, "1");
        msleep(500);
        var readBack = ad9371.getRxQuadratureTracking(0);
        printToConsole("  Set to '1', read back: " + readBack);
        if (readBack !== "1") {
            printToConsole("  FAIL: quadrature_tracking_en did not set to '1'");
            ad9371.setRxQuadratureTracking(0, origVal);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setRxQuadratureTracking(0, origVal);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 14: TX LO Leakage Tracking
// UID: TST.AD9371.TX_LO_LEAKAGE_TRACKING
// ============================================
TestFramework.runTest("TST.AD9371.TX_LO_LEAKAGE_TRACKING", function() {
    try {
        var origVal = ad9371.getTxLoLeakageTracking(0);
        printToConsole("  Original TX LO leakage tracking ch0: " + origVal);

        ad9371.setTxLoLeakageTracking(0, "1");
        msleep(500);
        var readBack = ad9371.getTxLoLeakageTracking(0);
        printToConsole("  Set to '1', read back: " + readBack);
        if (readBack !== "1") {
            printToConsole("  FAIL: lo_leakage_tracking_en did not set to '1'");
            ad9371.setTxLoLeakageTracking(0, origVal);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setTxLoLeakageTracking(0, origVal);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 15: Observation RX Attributes
// UID: TST.AD9371.OBS_RX_ATTRIBUTES
// ============================================
TestFramework.runTest("TST.AD9371.OBS_RX_ATTRIBUTES", function() {
    try {
        // RF Bandwidth (read-only, MHz)
        var obsBw = ad9371.getObsRfBandwidth();
        printToConsole("  Obs RF Bandwidth: " + obsBw + " MHz");
        if (!obsBw || obsBw === "") {
            printToConsole("  FAIL: getObsRfBandwidth() returned empty");
            return false;
        }

        // Gain control mode
        var obsGainMode = ad9371.getObsGainControlMode();
        printToConsole("  Obs gain control mode: " + obsGainMode);
        if (!obsGainMode || obsGainMode === "") {
            printToConsole("  FAIL: getObsGainControlMode() returned empty");
            return false;
        }

        // Hardware gain write-readback
        var origGain = ad9371.getObsHardwareGain();
        ad9371.setObsHardwareGain("25");
        msleep(500);
        var readBackGain = ad9371.getObsHardwareGain();
        printToConsole("  Set obs HW gain to 25, read back: " + readBackGain);
        var gainVal = parseFloat(readBackGain);
        if (Math.abs(gainVal - 25.0) > 1.0) {
            printToConsole("  FAIL: Obs hardware gain readback mismatch");
            ad9371.setObsHardwareGain(origGain);
            msleep(500);
            return false;
        }
        ad9371.setObsHardwareGain(origGain);
        msleep(500);

        // RF port select
        var obsPort = ad9371.getObsRfPortSelect();
        printToConsole("  Obs RF port select: " + obsPort);
        if (!obsPort || obsPort === "") {
            printToConsole("  FAIL: getObsRfPortSelect() returned empty");
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 16: Sniffer LO Frequency MHz Conversion
// UID: TST.AD9371.SNIFFER_LO_FREQUENCY_CONVERSION
// ============================================
TestFramework.runTest("TST.AD9371.SNIFFER_LO_FREQUENCY_CONVERSION", function() {
    try {
        var origFreq = ad9371.getSnifferLoFrequency();
        printToConsole("  Original Sniffer LO frequency: " + origFreq + " MHz");

        ad9371.setSnifferLoFrequency("3000");
        msleep(500);
        var readBack = ad9371.getSnifferLoFrequency();
        printToConsole("  Set to 3000 MHz, read back: " + readBack);

        var readBackVal = parseFloat(readBack);
        if (Math.abs(readBackVal - 3000.0) > 1.0) {
            printToConsole("  FAIL: Sniffer LO frequency readback mismatch");
            ad9371.setSnifferLoFrequency(origFreq);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setSnifferLoFrequency(origFreq);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 17: DPD Tracking Enable (AD9375 Only)
// UID: TST.AD9371.DPD_TRACKING_ENABLE
// ============================================
TestFramework.runTest("TST.AD9371.DPD_TRACKING_ENABLE", function() {
    try {
        var origVal = ad9371.getDpdTrackingEn(0);
        if (!origVal && origVal !== "0") {
            printToConsole("  DPD not available (not AD9375), skipping");
            return "SKIP";
        }
        printToConsole("  Original DPD tracking en ch0: " + origVal);

        ad9371.setDpdTrackingEn(0, "1");
        msleep(500);
        var readBack = ad9371.getDpdTrackingEn(0);
        printToConsole("  Set to '1', read back: " + readBack);
        if (readBack !== "1") {
            printToConsole("  FAIL: dpd_tracking_en did not set to '1'");
            ad9371.setDpdTrackingEn(0, origVal);
            msleep(500);
            return false;
        }

        // Read track count
        var trackCount = ad9371.getDpdTrackCount(0);
        printToConsole("  DPD track count: " + trackCount);

        // Restore
        ad9371.setDpdTrackingEn(0, origVal);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 18: DPD Status String Map
// UID: TST.AD9371.DPD_STATUS_STRING_MAP
// ============================================
TestFramework.runTest("TST.AD9371.DPD_STATUS_STRING_MAP", function() {
    try {
        var status = ad9371.getDpdStatus(0);
        if (!status || status === "") {
            printToConsole("  DPD not available (not AD9375), skipping");
            return "SKIP";
        }
        printToConsole("  DPD status ch0: " + status);

        // Verify it's a human-readable string, not a raw integer
        var asInt = parseInt(status);
        if (status === String(asInt)) {
            printToConsole("  FAIL: Status is a raw integer, expected human-readable string");
            return false;
        }
        printToConsole("  Status is a human-readable string");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 19: DPD Model Error Scaling
// UID: TST.AD9371.DPD_MODEL_ERROR_SCALING
// ============================================
TestFramework.runTest("TST.AD9371.DPD_MODEL_ERROR_SCALING", function() {
    try {
        var modelError = ad9371.getDpdModelError(0);
        if (!modelError || modelError === "") {
            printToConsole("  DPD not available (not AD9375), skipping");
            return "SKIP";
        }
        printToConsole("  DPD model error ch0: " + modelError);

        // Verify it contains '%'
        if (modelError.indexOf("%") === -1) {
            printToConsole("  FAIL: Model error should contain '%' suffix");
            return false;
        }
        printToConsole("  Model error has correct % format");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 20: DPD Reset Trigger
// UID: TST.AD9371.DPD_RESET_TRIGGER
// ============================================
TestFramework.runTest("TST.AD9371.DPD_RESET_TRIGGER", function() {
    try {
        // Check if DPD is available first
        var status = ad9371.getDpdStatus(0);
        if (!status || status === "") {
            printToConsole("  DPD not available (not AD9375), skipping");
            return "SKIP";
        }

        ad9371.dpdReset(0);
        msleep(500);
        printToConsole("  DPD reset triggered without error");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 21: CLGC Desired Gain Scaling
// UID: TST.AD9371.CLGC_DESIRED_GAIN_SCALING
// ============================================
TestFramework.runTest("TST.AD9371.CLGC_DESIRED_GAIN_SCALING", function() {
    try {
        var origVal = ad9371.getClgcDesiredGain(0);
        if (!origVal && origVal !== "0") {
            printToConsole("  CLGC not available (not AD9375), skipping");
            return "SKIP";
        }
        printToConsole("  Original CLGC desired gain ch0: " + origVal);

        ad9371.setClgcDesiredGain(0, "0.50");
        msleep(500);
        var readBack = ad9371.getClgcDesiredGain(0);
        printToConsole("  Set to 0.50, read back: " + readBack);

        var readBackVal = parseFloat(readBack);
        if (Math.abs(readBackVal - 0.50) > 0.01) {
            printToConsole("  FAIL: CLGC desired gain readback mismatch");
            ad9371.setClgcDesiredGain(0, origVal);
            msleep(500);
            return false;
        }

        // Verify raw value is 50 (0.50 × 100)
        var rawVal = ad9371.readWidget("ad9371-phy/voltage0_out/clgc_desired_gain");
        printToConsole("  Raw IIO value: " + rawVal);
        var rawNum = parseFloat(rawVal);
        if (Math.abs(rawNum - 50.0) > 1.0) {
            printToConsole("  FAIL: Raw value should be ~50, got: " + rawVal);
            ad9371.setClgcDesiredGain(0, origVal);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setClgcDesiredGain(0, origVal);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 22: CLGC Read-Only Attributes
// UID: TST.AD9371.CLGC_READ_ONLY_ATTRIBUTES
// ============================================
TestFramework.runTest("TST.AD9371.CLGC_READ_ONLY_ATTRIBUTES", function() {
    try {
        var clgcStatus = ad9371.getClgcStatus(0);
        if (!clgcStatus || clgcStatus === "") {
            printToConsole("  CLGC not available (not AD9375), skipping");
            return "SKIP";
        }
        printToConsole("  CLGC status ch0: " + clgcStatus);

        var currentGain = ad9371.getClgcCurrentGain(0);
        printToConsole("  CLGC current gain ch0: " + currentGain);
        if (currentGain.indexOf("dB") === -1) {
            printToConsole("  FAIL: Current gain should end with 'dB'");
            return false;
        }

        var txGain = ad9371.getClgcTxGain(0);
        printToConsole("  CLGC TX gain ch0: " + txGain);
        if (txGain.indexOf("dB") === -1) {
            printToConsole("  FAIL: TX gain should end with 'dB'");
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 23: VSWR Tracking and Status
// UID: TST.AD9371.VSWR_TRACKING_AND_STATUS
// ============================================
TestFramework.runTest("TST.AD9371.VSWR_TRACKING_AND_STATUS", function() {
    try {
        var vswrEn = ad9371.getVswrTrackingEn(0);
        if (!vswrEn && vswrEn !== "0") {
            printToConsole("  VSWR not available (not AD9375), skipping");
            return "SKIP";
        }
        printToConsole("  VSWR tracking en ch0: " + vswrEn);

        var vswrStatus = ad9371.getVswrStatus(0);
        printToConsole("  VSWR status ch0: " + vswrStatus);
        if (!vswrStatus || vswrStatus === "") {
            printToConsole("  FAIL: getVswrStatus() returned empty");
            return false;
        }

        var forwardOrx = ad9371.getVswrForwardOrx(0);
        printToConsole("  VSWR forward ORx ch0: " + forwardOrx);
        if (forwardOrx.indexOf("dBFS") === -1) {
            printToConsole("  FAIL: Forward ORx should end with 'dBFS'");
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 24: Refresh All Widgets
// UID: TST.AD9371.REFRESH_ALL_WIDGETS
// ============================================
TestFramework.runTest("TST.AD9371.REFRESH_ALL_WIDGETS", function() {
    try {
        ad9371.refresh();
        msleep(500);
        printToConsole("  refresh() completed without error");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 26: Disconnect and Reconnect
// UID: TST.AD9371.DISCONNECT_RECONNECT
// ============================================
TestFramework.runTest("TST.AD9371.DISCONNECT_RECONNECT", function() {
    try {
        // Disconnect
        var deviceUri = TestFramework.deviceUri;
        var deviceId = TestFramework.deviceId;
        scopy.disconnectDevice(deviceId);
        msleep(2000);
        printToConsole("  Disconnected from device");

        // Reconnect
        if (!scopy.connectDevice(deviceId)) {
            printToConsole("  FAIL: Could not reconnect to device");
            return false;
        }
        msleep(2000);
        printToConsole("  Reconnected to device");

        // Verify API still works
        var tools = ad9371.getTools();
        printToConsole("  After reconnect, getTools(): " + tools);
        if (!tools || tools.length === 0) {
            printToConsole("  FAIL: API not functional after reconnect");
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 27: RX Sampling Frequency MHz Conversion
// UID: TST.AD9371.RX_SAMPLING_FREQUENCY_CONVERSION
// ============================================
TestFramework.runTest("TST.AD9371.RX_SAMPLING_FREQUENCY_CONVERSION", function() {
    try {
        var sampFreq = ad9371.getRxSamplingFrequency();
        printToConsole("  RX sampling frequency: " + sampFreq + " MHz");
        if (!sampFreq || sampFreq === "") {
            printToConsole("  FAIL: getRxSamplingFrequency() returned empty");
            return false;
        }

        // Verify it's in MHz range (not Hz)
        var freqVal = parseFloat(sampFreq);
        if (freqVal > 1e6) {
            printToConsole("  FAIL: Value appears to be in Hz, not MHz: " + sampFreq);
            return false;
        }
        printToConsole("  Value is in MHz range");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 28: Advanced Tool Widget Keys in Group
// UID: TST.AD9371.ADVANCED_WIDGET_KEYS_IN_GROUP
// ============================================
TestFramework.runTest("TST.AD9371.ADVANCED_WIDGET_KEYS_IN_GROUP", function() {
    try {
        var keys = ad9371.getWidgetKeys();
        printToConsole("  Total widget keys: " + keys.length);

        // Look for a debug attribute key from advanced tool (e.g., clk settings)
        var hasDebugKey = false;
        for (var i = 0; i < keys.length; i++) {
            if (keys[i].indexOf("adi,") !== -1) {
                hasDebugKey = true;
                printToConsole("  Found advanced debug key: " + keys[i]);
                break;
            }
        }

        if (!hasDebugKey) {
            printToConsole("  FAIL: No advanced tool debug attribute keys found in widget group");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Cleanup
// ============================================
TestFramework.disconnectFromDevice();
TestFramework.printSummary();
scopy.exit();
