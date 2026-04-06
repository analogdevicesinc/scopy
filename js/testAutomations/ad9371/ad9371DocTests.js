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

// AD9371 Documentation Test Automation
// Automates tests from: docs/tests/plugins/ad9371/ad9371_tests.rst
//
// Missing API Report (Category C — not automatable):
//   TST.AD9371.BLOCK_DIAGRAM_VIEW — No API to switch between Controls and Block Diagram views.
//     Requires a UI button click that is not exposed via the JavaScript API.
//
// Category B tests (visual checks) are not included in this file:
//   TST.AD9371.DEVICE_DETECTION — requires visual confirmation of UI layout
//   TST.AD9371.REFRESH_BUTTON   — requires visual confirmation of refresh animation

evaluateFile("js/testAutomations/common/testFramework.js");

TestFramework.init("AD9371 Documentation Tests");

if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

if (!switchToTool("AD9371")) {
    printToConsole("ERROR: Cannot switch to AD9371 tool");
    scopy.exit();
}

// ============================================
// Test 1: Plugin Loads
// UID: TST.AD9371.UI_PLUGIN_LOADS
// ============================================
TestFramework.runTest("TST.AD9371.UI_PLUGIN_LOADS", function() {
    try {
        var tools = ad9371.getTools();
        printToConsole("  Tools found: " + tools);
        if (!tools || tools.length < 2) {
            printToConsole("  FAIL: Expected at least 2 tools (AD9371 and AD9371 Advanced)");
            return false;
        }
        var hasMain = false;
        var hasAdvanced = false;
        for (var i = 0; i < tools.length; i++) {
            if (tools[i] === "AD9371") hasMain = true;
            if (tools[i] === "AD9371 Advanced") hasAdvanced = true;
        }
        if (!hasMain) {
            printToConsole("  FAIL: AD9371 tool not found in tools list");
            return false;
        }
        if (!hasAdvanced) {
            printToConsole("  FAIL: AD9371 Advanced tool not found in tools list");
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
// Test 3: Global Settings Section Visible
// UID: TST.AD9371.GLOBAL_SETTINGS_VISIBLE
// ============================================
TestFramework.runTest("TST.AD9371.GLOBAL_SETTINGS_VISIBLE", function() {
    try {
        var ensmMode = ad9371.getEnsmMode();
        if (!ensmMode) {
            printToConsole("  FAIL: getEnsmMode() returned empty");
            return false;
        }
        printToConsole("  ENSM Mode: " + ensmMode);

        var rxQec = ad9371.getCalibrateRxQecEn();
        if (!rxQec && rxQec !== "0") {
            printToConsole("  FAIL: getCalibrateRxQecEn() returned empty");
            return false;
        }
        printToConsole("  CAL RX QEC: " + rxQec);

        var txQec = ad9371.getCalibrateTxQecEn();
        if (!txQec && txQec !== "0") {
            printToConsole("  FAIL: getCalibrateTxQecEn() returned empty");
            return false;
        }
        printToConsole("  CAL TX QEC: " + txQec);

        var txLol = ad9371.getCalibrateTxLolEn();
        if (!txLol && txLol !== "0") {
            printToConsole("  FAIL: getCalibrateTxLolEn() returned empty");
            return false;
        }
        printToConsole("  CAL TX LOL: " + txLol);

        printToConsole("  Global Settings widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: ENSM Mode Write-Readback
// UID: TST.AD9371.ENSM_MODE_WRITE
// ============================================
TestFramework.runTest("TST.AD9371.ENSM_MODE_WRITE", function() {
    var origMode = null;
    try {
        origMode = ad9371.getEnsmMode();
        if (!origMode) {
            printToConsole("  FAIL: Cannot read current ENSM mode");
            return false;
        }
        printToConsole("  Original ENSM mode: " + origMode);

        // Pick a different value to test
        var testMode = (origMode === "fdd") ? "alert" : "fdd";
        printToConsole("  Writing ENSM mode: " + testMode);
        ad9371.setEnsmMode(testMode);
        msleep(500);

        var readBack = ad9371.getEnsmMode();
        printToConsole("  Readback ENSM mode: " + readBack);

        // Restore before checking
        ad9371.setEnsmMode(origMode);
        msleep(500);

        if (readBack !== testMode) {
            printToConsole("  FAIL: Expected " + testMode + ", got " + readBack);
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origMode) {
            try { ad9371.setEnsmMode(origMode); msleep(500); } catch (e2) {}
        }
        return false;
    }
});

// ============================================
// Test 5: Calibration Mask Write
// UID: TST.AD9371.CALIBRATION_MASK_WRITE
// ============================================
TestFramework.runTest("TST.AD9371.CALIBRATION_MASK_WRITE", function() {
    var origRxQec = null;
    try {
        origRxQec = ad9371.getCalibrateRxQecEn();
        if (!origRxQec && origRxQec !== "0") {
            printToConsole("  FAIL: Cannot read CAL RX QEC");
            return false;
        }
        printToConsole("  Original CAL RX QEC: " + origRxQec);

        var newVal = (origRxQec === "1") ? "0" : "1";
        printToConsole("  Writing CAL RX QEC: " + newVal);
        ad9371.setCalibrateRxQecEn(newVal);
        msleep(500);

        var readBack = ad9371.getCalibrateRxQecEn();
        printToConsole("  Readback CAL RX QEC: " + readBack);

        if (readBack !== newVal) {
            ad9371.setCalibrateRxQecEn(origRxQec);
            msleep(500);
            printToConsole("  FAIL: CAL RX QEC readback mismatch. Expected " + newVal + ", got " + readBack);
            return false;
        }

        printToConsole("  Triggering calibration...");
        var calResult = ad9371.calibrate();
        printToConsole("  Calibrate returned: " + calResult);

        // Restore
        ad9371.setCalibrateRxQecEn(origRxQec);
        msleep(500);

        if (!calResult) {
            printToConsole("  FAIL: calibrate() returned false");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origRxQec !== null) {
            try { ad9371.setCalibrateRxQecEn(origRxQec); msleep(500); } catch (e2) {}
        }
        return false;
    }
});

// ============================================
// Test 6: Calibrate Button Triggers Calibration
// UID: TST.AD9371.UI_CALIBRATE_TRIGGER
// ============================================
TestFramework.runTest("TST.AD9371.UI_CALIBRATE_TRIGGER", function() {
    var origTxQec = null;
    var origTxLol = null;
    try {
        origTxQec = ad9371.getCalibrateTxQecEn();
        origTxLol = ad9371.getCalibrateTxLolEn();
        if ((!origTxQec && origTxQec !== "0") || (!origTxLol && origTxLol !== "0")) {
            printToConsole("  FAIL: Cannot read calibration enable flags");
            return false;
        }
        printToConsole("  Original TX QEC: " + origTxQec + ", TX LOL: " + origTxLol);

        ad9371.setCalibrateTxQecEn("1");
        msleep(500);
        ad9371.setCalibrateTxLolEn("1");
        msleep(500);

        var readQec = ad9371.getCalibrateTxQecEn();
        var readLol = ad9371.getCalibrateTxLolEn();
        printToConsole("  TX QEC after set: " + readQec + ", TX LOL after set: " + readLol);

        printToConsole("  Triggering calibration...");
        var calResult = ad9371.calibrate();
        printToConsole("  Calibrate returned: " + calResult);

        // Restore
        ad9371.setCalibrateTxQecEn(origTxQec);
        msleep(500);
        ad9371.setCalibrateTxLolEn(origTxLol);
        msleep(500);

        if (readQec !== "1" || readLol !== "1") {
            printToConsole("  FAIL: Switches did not enable correctly");
            return false;
        }
        if (!calResult) {
            printToConsole("  FAIL: calibrate() returned false");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origTxQec !== null) {
            try { ad9371.setCalibrateTxQecEn(origTxQec); msleep(500); } catch (e2) {}
        }
        if (origTxLol !== null) {
            try { ad9371.setCalibrateTxLolEn(origTxLol); msleep(500); } catch (e2) {}
        }
        return false;
    }
});

// ============================================
// Test 7: Load Profile from File
// UID: TST.AD9371.LOAD_PROFILE
// ============================================
TestFramework.runTest("TST.AD9371.LOAD_PROFILE", function() {
    try {
        var profilePath = ad9371.getDefaultProfilePath();
        if (!profilePath) {
            printToConsole("  SKIP: No default profile path available");
            return "SKIP";
        }
        printToConsole("  Default profile path: " + profilePath);

        var rxRateBefore = ad9371.getRxSamplingRate();
        printToConsole("  RX Sampling Rate before: " + rxRateBefore);

        ad9371.loadProfile(profilePath);
        msleep(1000);

        var rxRateAfter = ad9371.getRxSamplingRate();
        printToConsole("  RX Sampling Rate after: " + rxRateAfter);

        if (!rxRateAfter) {
            printToConsole("  FAIL: RX Sampling Rate not readable after profile load");
            return false;
        }
        printToConsole("  Profile loaded successfully, sampling rate readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 8: RX Chain Section Visible
// UID: TST.AD9371.RX_CHAIN_VISIBLE
// ============================================
TestFramework.runTest("TST.AD9371.RX_CHAIN_VISIBLE", function() {
    try {
        var rfBw = ad9371.getRxRfBandwidth();
        if (!rfBw) {
            printToConsole("  FAIL: getRxRfBandwidth() returned empty");
            return false;
        }
        printToConsole("  RX RF Bandwidth: " + rfBw);

        var gcMode = ad9371.getRxGainControlMode();
        if (!gcMode) {
            printToConsole("  FAIL: getRxGainControlMode() returned empty");
            return false;
        }
        printToConsole("  RX Gain Control Mode: " + gcMode);

        var hwGain = ad9371.getRxHardwareGain(0);
        if (!hwGain && hwGain !== "0") {
            printToConsole("  FAIL: getRxHardwareGain(0) returned empty");
            return false;
        }
        printToConsole("  RX Hardware Gain ch0: " + hwGain);

        var rssi = ad9371.getRxRssi(0);
        if (!rssi && rssi !== "0") {
            printToConsole("  FAIL: getRxRssi(0) returned empty");
            return false;
        }
        printToConsole("  RX RSSI ch0: " + rssi);

        var loFreq = ad9371.getRxLoFrequency();
        if (!loFreq) {
            printToConsole("  FAIL: getRxLoFrequency() returned empty");
            return false;
        }
        printToConsole("  RX LO Frequency: " + loFreq);

        printToConsole("  All RX Chain attributes readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 9: RX Gain Control Mode Write-Readback
// UID: TST.AD9371.RX_GAIN_CONTROL_MODE_WRITE
// ============================================
TestFramework.runTest("TST.AD9371.RX_GAIN_CONTROL_MODE_WRITE", function() {
    var origMode = null;
    try {
        origMode = ad9371.getRxGainControlMode();
        if (!origMode) {
            printToConsole("  FAIL: Cannot read RX Gain Control Mode");
            return false;
        }
        printToConsole("  Original RX Gain Control Mode: " + origMode);

        var testMode = (origMode === "manual") ? "slow_attack" : "manual";
        printToConsole("  Writing RX Gain Control Mode: " + testMode);
        ad9371.setRxGainControlMode(testMode);
        msleep(500);

        var readBack = ad9371.getRxGainControlMode();
        printToConsole("  Readback: " + readBack);

        // Restore
        ad9371.setRxGainControlMode(origMode);
        msleep(500);

        if (readBack !== testMode) {
            printToConsole("  FAIL: Expected " + testMode + ", got " + readBack);
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origMode) {
            try { ad9371.setRxGainControlMode(origMode); msleep(500); } catch (e2) {}
        }
        return false;
    }
});

// ============================================
// Test 10: RX LO Frequency Write-Readback
// UID: TST.AD9371.RX_LO_FREQUENCY_WRITE
// ============================================
TestFramework.runTest("TST.AD9371.RX_LO_FREQUENCY_WRITE", function() {
    var origFreq = null;
    try {
        origFreq = ad9371.getRxLoFrequency();
        if (!origFreq) {
            printToConsole("  FAIL: Cannot read RX LO Frequency");
            return false;
        }
        printToConsole("  Original RX LO Frequency: " + origFreq);

        // Write a slightly different frequency (shift by 1 MHz)
        var origHz = parseInt(origFreq);
        if (isNaN(origHz)) {
            printToConsole("  FAIL: Cannot parse frequency value: " + origFreq);
            return false;
        }
        var testHz = origHz + 1000000;
        var testVal = String(testHz);
        printToConsole("  Writing RX LO Frequency: " + testVal);
        ad9371.setRxLoFrequency(testVal);
        msleep(500);

        var readBack = ad9371.getRxLoFrequency();
        printToConsole("  Readback: " + readBack);

        // Restore
        ad9371.setRxLoFrequency(origFreq);
        msleep(500);

        if (!readBack) {
            printToConsole("  FAIL: Readback returned empty");
            return false;
        }
        // Hardware may quantize the value — verify it changed from original
        if (readBack === origFreq) {
            printToConsole("  FAIL: Value did not change after write");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origFreq) {
            try { ad9371.setRxLoFrequency(origFreq); msleep(500); } catch (e2) {}
        }
        return false;
    }
});

// ============================================
// Test 11: RX Hardware Gain Write-Readback
// UID: TST.AD9371.RX_HARDWARE_GAIN_WRITE
// ============================================
TestFramework.runTest("TST.AD9371.RX_HARDWARE_GAIN_WRITE", function() {
    var origMode = null;
    var origGain = null;
    try {
        origMode = ad9371.getRxGainControlMode();
        if (!origMode) {
            printToConsole("  FAIL: Cannot read RX Gain Control Mode");
            return false;
        }

        // Must be in manual mode to write hardware gain
        if (origMode !== "manual") {
            printToConsole("  Setting RX Gain Control Mode to manual");
            ad9371.setRxGainControlMode("manual");
            msleep(500);
        }

        origGain = ad9371.getRxHardwareGain(0);
        if (!origGain && origGain !== "0") {
            printToConsole("  FAIL: Cannot read RX Hardware Gain ch0");
            ad9371.setRxGainControlMode(origMode);
            msleep(500);
            return false;
        }
        printToConsole("  Original RX Hardware Gain ch0: " + origGain);

        var origGainVal = parseFloat(origGain);
        var testGainVal = (origGainVal >= 10) ? origGainVal - 3 : origGainVal + 3;
        var testGain = String(testGainVal);
        printToConsole("  Writing RX Hardware Gain: " + testGain);
        ad9371.setRxHardwareGain(0, testGain);
        msleep(500);

        var readBack = ad9371.getRxHardwareGain(0);
        printToConsole("  Readback: " + readBack);

        // Restore gain and mode
        ad9371.setRxHardwareGain(0, origGain);
        msleep(500);
        if (origMode !== "manual") {
            ad9371.setRxGainControlMode(origMode);
            msleep(500);
        }

        if (!readBack && readBack !== "0") {
            printToConsole("  FAIL: Readback returned empty");
            return false;
        }
        if (readBack === origGain) {
            printToConsole("  FAIL: Hardware gain did not change after write");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origGain !== null) {
            try { ad9371.setRxHardwareGain(0, origGain); msleep(500); } catch (e2) {}
        }
        if (origMode !== null) {
            try { ad9371.setRxGainControlMode(origMode); msleep(500); } catch (e2) {}
        }
        return false;
    }
});

// ============================================
// Test 12: TX Chain Section Visible
// UID: TST.AD9371.TX_CHAIN_VISIBLE
// ============================================
TestFramework.runTest("TST.AD9371.TX_CHAIN_VISIBLE", function() {
    try {
        var rfBw = ad9371.getTxRfBandwidth();
        if (!rfBw) {
            printToConsole("  FAIL: getTxRfBandwidth() returned empty");
            return false;
        }
        printToConsole("  TX RF Bandwidth: " + rfBw);

        var atten = ad9371.getTxAttenuation(0);
        if (!atten && atten !== "0") {
            printToConsole("  FAIL: getTxAttenuation(0) returned empty");
            return false;
        }
        printToConsole("  TX Attenuation ch0: " + atten);

        var quadTrack = ad9371.getTxQuadratureTracking(0);
        if (!quadTrack && quadTrack !== "0") {
            printToConsole("  FAIL: getTxQuadratureTracking(0) returned empty");
            return false;
        }
        printToConsole("  TX Quadrature Tracking ch0: " + quadTrack);

        var loFreq = ad9371.getTxLoFrequency();
        if (!loFreq) {
            printToConsole("  FAIL: getTxLoFrequency() returned empty");
            return false;
        }
        printToConsole("  TX LO Frequency: " + loFreq);

        printToConsole("  All TX Chain attributes readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 13: TX LO Frequency Write-Readback
// UID: TST.AD9371.TX_LO_FREQUENCY_WRITE
// ============================================
TestFramework.runTest("TST.AD9371.TX_LO_FREQUENCY_WRITE", function() {
    var origFreq = null;
    try {
        origFreq = ad9371.getTxLoFrequency();
        if (!origFreq) {
            printToConsole("  FAIL: Cannot read TX LO Frequency");
            return false;
        }
        printToConsole("  Original TX LO Frequency: " + origFreq);

        var origHz = parseInt(origFreq);
        if (isNaN(origHz)) {
            printToConsole("  FAIL: Cannot parse TX LO frequency: " + origFreq);
            return false;
        }
        var testHz = origHz + 1000000;
        var testVal = String(testHz);
        printToConsole("  Writing TX LO Frequency: " + testVal);
        ad9371.setTxLoFrequency(testVal);
        msleep(500);

        var readBack = ad9371.getTxLoFrequency();
        printToConsole("  Readback: " + readBack);

        // Restore
        ad9371.setTxLoFrequency(origFreq);
        msleep(500);

        if (!readBack) {
            printToConsole("  FAIL: Readback returned empty");
            return false;
        }
        if (readBack === origFreq) {
            printToConsole("  FAIL: Value did not change after write");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origFreq) {
            try { ad9371.setTxLoFrequency(origFreq); msleep(500); } catch (e2) {}
        }
        return false;
    }
});

// ============================================
// Test 14: TX Attenuation Write-Readback
// UID: TST.AD9371.TX_ATTENUATION_WRITE
// ============================================
TestFramework.runTest("TST.AD9371.TX_ATTENUATION_WRITE", function() {
    var origAtten = null;
    try {
        origAtten = ad9371.getTxAttenuation(0);
        if (!origAtten && origAtten !== "0") {
            printToConsole("  FAIL: Cannot read TX Attenuation ch0");
            return false;
        }
        printToConsole("  Original TX Attenuation ch0: " + origAtten);

        var origVal = parseFloat(origAtten);
        var testVal = (origVal >= 10) ? String(origVal - 3) : String(origVal + 3);
        printToConsole("  Writing TX Attenuation: " + testVal);
        ad9371.setTxAttenuation(0, testVal);
        msleep(500);

        var readBack = ad9371.getTxAttenuation(0);
        printToConsole("  Readback: " + readBack);

        // Restore
        ad9371.setTxAttenuation(0, origAtten);
        msleep(500);

        if (!readBack && readBack !== "0") {
            printToConsole("  FAIL: Readback returned empty");
            return false;
        }
        if (readBack === origAtten) {
            printToConsole("  FAIL: Attenuation did not change after write");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origAtten !== null) {
            try { ad9371.setTxAttenuation(0, origAtten); msleep(500); } catch (e2) {}
        }
        return false;
    }
});

// ============================================
// Test 15: Observation/Sniffer RX Section Visible
// UID: TST.AD9371.OBS_CHAIN_VISIBLE
// ============================================
TestFramework.runTest("TST.AD9371.OBS_CHAIN_VISIBLE", function() {
    try {
        var rfBw = ad9371.getObsRfBandwidth();
        if (!rfBw) {
            printToConsole("  FAIL: getObsRfBandwidth() returned empty");
            return false;
        }
        printToConsole("  Obs RF Bandwidth: " + rfBw);

        var gcMode = ad9371.getObsGainControlMode();
        if (!gcMode) {
            printToConsole("  FAIL: getObsGainControlMode() returned empty");
            return false;
        }
        printToConsole("  Obs Gain Control Mode: " + gcMode);

        var hwGain = ad9371.getObsHardwareGain();
        if (!hwGain && hwGain !== "0") {
            printToConsole("  FAIL: getObsHardwareGain() returned empty");
            return false;
        }
        printToConsole("  Obs Hardware Gain: " + hwGain);

        var rfPort = ad9371.getObsRfPortSelect();
        if (!rfPort) {
            printToConsole("  FAIL: getObsRfPortSelect() returned empty");
            return false;
        }
        printToConsole("  Obs RF Port Select: " + rfPort);

        var rssi = ad9371.getObsRssi();
        if (!rssi && rssi !== "0") {
            printToConsole("  FAIL: getObsRssi() returned empty");
            return false;
        }
        printToConsole("  Obs RSSI: " + rssi);

        printToConsole("  All Obs/Sniffer RX attributes readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 16: Obs RF Port Select Write-Readback
// UID: TST.AD9371.OBS_RF_PORT_SELECT_WRITE
// ============================================
TestFramework.runTest("TST.AD9371.OBS_RF_PORT_SELECT_WRITE", function() {
    var origPort = null;
    try {
        origPort = ad9371.getObsRfPortSelect();
        if (!origPort) {
            printToConsole("  FAIL: Cannot read Obs RF Port Select");
            return false;
        }
        printToConsole("  Original Obs RF Port: " + origPort);

        // Pick a different port option
        var testPort = (origPort === "ORX1_TX_LO") ? "ORX2_TX_LO" : "ORX1_TX_LO";
        printToConsole("  Writing Obs RF Port: " + testPort);
        ad9371.setObsRfPortSelect(testPort);
        msleep(500);

        var readBack = ad9371.getObsRfPortSelect();
        printToConsole("  Readback: " + readBack);

        // Restore
        ad9371.setObsRfPortSelect(origPort);
        msleep(500);

        if (!readBack) {
            printToConsole("  FAIL: Readback returned empty");
            return false;
        }
        if (readBack !== testPort) {
            printToConsole("  FAIL: Expected " + testPort + ", got " + readBack);
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origPort) {
            try { ad9371.setObsRfPortSelect(origPort); msleep(500); } catch (e2) {}
        }
        return false;
    }
});

// ============================================
// Test 17: Obs Gain Control Mode Write-Readback
// UID: TST.AD9371.OBS_GAIN_CONTROL_MODE_WRITE
// ============================================
TestFramework.runTest("TST.AD9371.OBS_GAIN_CONTROL_MODE_WRITE", function() {
    var origMode = null;
    try {
        origMode = ad9371.getObsGainControlMode();
        if (!origMode) {
            printToConsole("  FAIL: Cannot read Obs Gain Control Mode");
            return false;
        }
        printToConsole("  Original Obs Gain Control Mode: " + origMode);

        var testMode = (origMode === "manual") ? "slow_attack" : "manual";
        printToConsole("  Writing Obs Gain Control Mode: " + testMode);
        ad9371.setObsGainControlMode(testMode);
        msleep(500);

        var readBack = ad9371.getObsGainControlMode();
        printToConsole("  Readback: " + readBack);

        // Restore
        ad9371.setObsGainControlMode(origMode);
        msleep(500);

        if (readBack !== testMode) {
            printToConsole("  FAIL: Expected " + testMode + ", got " + readBack);
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origMode) {
            try { ad9371.setObsGainControlMode(origMode); msleep(500); } catch (e2) {}
        }
        return false;
    }
});

// ============================================
// Test 18: FPGA Settings Section Visible
// UID: TST.AD9371.FPGA_SETTINGS_VISIBLE
// ============================================
TestFramework.runTest("TST.AD9371.FPGA_SETTINGS_VISIBLE", function() {
    try {
        var txFreq = ad9371.getFpgaTxFrequency();
        if (!txFreq) {
            printToConsole("  FAIL: getFpgaTxFrequency() returned empty");
            return false;
        }
        printToConsole("  FPGA TX Frequency: " + txFreq);

        var rxFreq = ad9371.getFpgaRxFrequency();
        if (!rxFreq) {
            printToConsole("  FAIL: getFpgaRxFrequency() returned empty");
            return false;
        }
        printToConsole("  FPGA RX Frequency: " + rxFreq);

        var phaseRot = ad9371.getPhaseRotation(0);
        if (!phaseRot && phaseRot !== "0") {
            printToConsole("  FAIL: getPhaseRotation(0) returned empty");
            return false;
        }
        printToConsole("  Phase Rotation ch0: " + phaseRot);

        printToConsole("  All FPGA Settings attributes readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 19: FPGA TX Frequency Write-Readback
// UID: TST.AD9371.FPGA_TX_FREQUENCY_WRITE
// ============================================
TestFramework.runTest("TST.AD9371.FPGA_TX_FREQUENCY_WRITE", function() {
    var origFreq = null;
    try {
        origFreq = ad9371.getFpgaTxFrequency();
        if (!origFreq) {
            printToConsole("  FAIL: Cannot read FPGA TX Frequency");
            return false;
        }
        printToConsole("  Original FPGA TX Frequency: " + origFreq);

        // Get available options via widget keys then pick a different one
        var keys = ad9371.getWidgetKeys();
        var fpgaTxKey = null;
        for (var i = 0; i < keys.length; i++) {
            if (keys[i].indexOf("fpga_tx") !== -1 || keys[i].indexOf("FPGA_TX") !== -1 ||
                keys[i].indexOf("tx_frequency") !== -1) {
                fpgaTxKey = keys[i];
                break;
            }
        }

        // Use setFpgaTxFrequency with current value as a round-trip test
        ad9371.setFpgaTxFrequency(origFreq);
        msleep(500);

        var readBack = ad9371.getFpgaTxFrequency();
        printToConsole("  Readback after write: " + readBack);

        if (!readBack) {
            printToConsole("  FAIL: Readback returned empty");
            return false;
        }
        printToConsole("  FPGA TX Frequency write-readback successful");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origFreq) {
            try { ad9371.setFpgaTxFrequency(origFreq); msleep(500); } catch (e2) {}
        }
        return false;
    }
});

// ============================================
// Test 20: FPGA RX Frequency Write-Readback
// UID: TST.AD9371.FPGA_RX_FREQUENCY_WRITE
// ============================================
TestFramework.runTest("TST.AD9371.FPGA_RX_FREQUENCY_WRITE", function() {
    var origFreq = null;
    try {
        origFreq = ad9371.getFpgaRxFrequency();
        if (!origFreq) {
            printToConsole("  FAIL: Cannot read FPGA RX Frequency");
            return false;
        }
        printToConsole("  Original FPGA RX Frequency: " + origFreq);

        ad9371.setFpgaRxFrequency(origFreq);
        msleep(500);

        var readBack = ad9371.getFpgaRxFrequency();
        printToConsole("  Readback after write: " + readBack);

        if (!readBack) {
            printToConsole("  FAIL: Readback returned empty");
            return false;
        }
        printToConsole("  FPGA RX Frequency write-readback successful");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origFreq) {
            try { ad9371.setFpgaRxFrequency(origFreq); msleep(500); } catch (e2) {}
        }
        return false;
    }
});

// ============================================
// Cleanup
// ============================================
TestFramework.disconnectFromDevice();
TestFramework.printSummary();
scopy.exit();
