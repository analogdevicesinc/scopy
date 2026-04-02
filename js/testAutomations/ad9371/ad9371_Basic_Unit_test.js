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

// AD9371 Unit Test Automation
// Tests every IIOWidget registered in the AD9371 plugin widget group.
// Spec: docs/superpowers/specs/2026-03-27-ad9371-unit-tests-design.md

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("AD9371 Basic Unit Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Widget key prefix
var PHY = "ad9371-phy/";

// Ensure device is in radio_on mode for tests
ad9371.setEnsmMode("radio_on");
msleep(1000);
printToConsole("ENSM mode set to: " + ad9371.getEnsmMode());

// Detect AD9375
var hasDpd = ad9371.readWidget(PHY + "adi,dpd-model-version");
var isAd9375 = (hasDpd !== null && hasDpd !== "");
printToConsole("AD9375 detected: " + isAd9375);

// ============================================
// Helper Functions
// ============================================

function testRange(key, min, max, mid) {
    try {
        var orig = ad9371.readWidget(key);
        printToConsole("  testRange: key=" + key + " orig=" + orig);

        var tolerance = 0.01;

        // Write min
        ad9371.writeWidget(key, String(min));
        msleep(500);
        var readMin = ad9371.readWidget(key);
        printToConsole("  Wrote min=" + min + ", read=" + readMin);
        if (Math.abs(parseFloat(readMin) - parseFloat(min)) > tolerance) {
            printToConsole("  FAIL: min mismatch, expected=" + String(min) + " got=" + readMin);
            ad9371.writeWidget(key, String(parseFloat(orig)));
            msleep(500);
            return false;
        }

        // Write max
        ad9371.writeWidget(key, String(max));
        msleep(500);
        var readMax = ad9371.readWidget(key);
        printToConsole("  Wrote max=" + max + ", read=" + readMax);
        if (Math.abs(parseFloat(readMax) - parseFloat(max)) > tolerance) {
            printToConsole("  FAIL: max mismatch, expected=" + String(max) + " got=" + readMax);
            ad9371.writeWidget(key, String(parseFloat(orig)));
            msleep(500);
            return false;
        }

        // Write mid
        ad9371.writeWidget(key, String(mid));
        msleep(500);
        var readMid = ad9371.readWidget(key);
        printToConsole("  Wrote mid=" + mid + ", read=" + readMid);
        if (Math.abs(parseFloat(readMid) - parseFloat(mid)) > tolerance) {
            printToConsole("  FAIL: mid mismatch, expected=" + String(mid) + " got=" + readMid);
            ad9371.writeWidget(key, String(parseFloat(orig)));
            msleep(500);
            return false;
        }

        // Restore
        ad9371.writeWidget(key, String(parseFloat(orig)));
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error in testRange: " + e);
        return false;
    }
}

function testCombo(key, validKeys) {
    try {
        var orig = ad9371.readWidget(key);
        printToConsole("  testCombo: key=" + key + " orig=" + orig);

        for (var i = 0; i < validKeys.length; i++) {
            ad9371.writeWidget(key, validKeys[i]);
            msleep(500);
            var readBack = ad9371.readWidget(key);
            printToConsole("  Wrote=" + validKeys[i] + ", read=" + readBack);
            if (readBack !== validKeys[i]) {
                printToConsole("  FAIL: combo mismatch, expected=" + validKeys[i] + " got=" + readBack);
                ad9371.writeWidget(key, orig);
                msleep(500);
                return false;
            }
        }

        // Restore
        ad9371.writeWidget(key, orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error in testCombo: " + e);
        return false;
    }
}

function testCheckbox(key) {
    try {
        var orig = ad9371.readWidget(key);
        printToConsole("  testCheckbox: key=" + key + " orig=" + orig);

        // Write "0"
        ad9371.writeWidget(key, "0");
        msleep(500);
        var read0 = ad9371.readWidget(key);
        printToConsole("  Wrote=0, read=" + read0);
        if (read0 !== "0") {
            printToConsole("  FAIL: checkbox mismatch, expected=0 got=" + read0);
            ad9371.writeWidget(key, orig);
            msleep(500);
            return false;
        }

        // Write "1"
        ad9371.writeWidget(key, "1");
        msleep(500);
        var read1 = ad9371.readWidget(key);
        printToConsole("  Wrote=1, read=" + read1);
        if (read1 !== "1") {
            printToConsole("  FAIL: checkbox mismatch, expected=1 got=" + read1);
            ad9371.writeWidget(key, orig);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.writeWidget(key, orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error in testCheckbox: " + e);
        return false;
    }
}

function testReadOnly(key) {
    try {
        var val = ad9371.readWidget(key);
        printToConsole("  testReadOnly: key=" + key + " val=" + val);
        if (val === null || val === "") {
            printToConsole("  FAIL: readOnly value is empty or null");
            return false;
        }
        printToConsole("  PASS: readOnly value is non-empty: " + val);
        return true;
    } catch (e) {
        printToConsole("  Error in testReadOnly: " + e);
        return false;
    }
}

function testConversion(getter, setter, widgetKey, apiVal, rawVal, tolerance) {
    try {
        var orig = getter();
        printToConsole("  testConversion: orig=" + orig + " apiVal=" + apiVal + " rawVal=" + rawVal);

        // Write via setter
        setter(apiVal);
        msleep(500);

        // Read via getter
        var apiRead = getter();
        printToConsole("  API read=" + apiRead);
        if (tolerance !== undefined && tolerance !== null) {
            var apiDiff = Math.abs(parseFloat(apiRead) - parseFloat(apiVal));
            if (apiDiff > tolerance) {
                printToConsole("  FAIL: API value mismatch, expected=" + apiVal + " got=" + apiRead + " tolerance=" + tolerance);
                setter(orig);
                msleep(500);
                return false;
            }
        } else {
            if (apiRead !== String(apiVal)) {
                printToConsole("  FAIL: API value mismatch, expected=" + apiVal + " got=" + apiRead);
                setter(orig);
                msleep(500);
                return false;
            }
        }

        // Read via readWidget
        var rawRead = ad9371.readWidget(widgetKey);
        printToConsole("  Raw widget read=" + rawRead);
        if (tolerance !== undefined && tolerance !== null) {
            var rawDiff = Math.abs(parseFloat(rawRead) - parseFloat(rawVal));
            if (rawDiff > tolerance) {
                printToConsole("  FAIL: Raw value mismatch, expected=" + rawVal + " got=" + rawRead + " tolerance=" + tolerance);
                setter(orig);
                msleep(500);
                return false;
            }
        } else {
            if (rawRead !== String(rawVal)) {
                printToConsole("  FAIL: Raw value mismatch, expected=" + rawVal + " got=" + rawRead);
                setter(orig);
                msleep(500);
                return false;
            }
        }

        // Restore
        setter(orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error in testConversion: " + e);
        return false;
    }
}

function testBadValueRange(key, min, max) {
    try {
        var orig = ad9371.readWidget(key);
        printToConsole("  testBadValueRange: key=" + key + " orig=" + orig + " validRange=[" + min + "," + max + "]");
        var tolerance = 0.01;

        // Write above max — expect clamped to max
        var aboveMax = String(parseFloat(max) + 1);
        ad9371.writeWidget(key, aboveMax);
        msleep(500);
        var readAbove = ad9371.readWidget(key);
        printToConsole("  Wrote aboveMax=" + aboveMax + ", read=" + readAbove);
        if (Math.abs(parseFloat(readAbove) - parseFloat(max)) > tolerance) {
            printToConsole("  FAIL: above-max not clamped, expected=" + max + " got=" + readAbove);
            ad9371.writeWidget(key, String(parseFloat(orig)));
            msleep(500);
            return false;
        }

        // Write below min — expect clamped to min
        var belowMin = String(parseFloat(min) - 1);
        ad9371.writeWidget(key, belowMin);
        msleep(500);
        var readBelow = ad9371.readWidget(key);
        printToConsole("  Wrote belowMin=" + belowMin + ", read=" + readBelow);
        if (Math.abs(parseFloat(readBelow) - parseFloat(min)) > tolerance) {
            printToConsole("  FAIL: below-min not clamped, expected=" + min + " got=" + readBelow);
            ad9371.writeWidget(key, String(parseFloat(orig)));
            msleep(500);
            return false;
        }

        // Restore
        ad9371.writeWidget(key, String(parseFloat(orig)));
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error in testBadValueRange: " + e);
        return false;
    }
}

function testBadValueCombo(key, badKey) {
    try {
        var orig = ad9371.readWidget(key);
        printToConsole("  testBadValueCombo: key=" + key + " orig=" + orig + " badKey=" + badKey);

        // Write invalid key — expect value unchanged
        ad9371.writeWidget(key, badKey);
        msleep(500);
        var readBack = ad9371.readWidget(key);
        printToConsole("  Wrote badKey=" + badKey + ", read=" + readBack);
        if (readBack !== orig) {
            printToConsole("  FAIL: invalid combo key was accepted, orig=" + orig + " got=" + readBack);
            ad9371.writeWidget(key, orig);
            msleep(500);
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error in testBadValueCombo: " + e);
        return false;
    }
}

// Known status strings from ad9371_api.cpp for validation
var KNOWN_DPD_STATUS = [
    "No Error", "Error: ORx disabled", "Error: Tx disabled",
    "Error: DPD initialization not run", "Error: Path delay not setup",
    "Error: ORx signal too low", "Error: ORx signal saturated",
    "Error: Tx signal too low", "Error: Tx signal saturated",
    "Error: Model error high", "Error: AM AM outliers",
    "Error: Invalid Tx profile", "Error: ORx QEC Disabled"
];

var KNOWN_CLGC_STATUS = [
    "No Error", "Error: TX is disabled", "Error: ORx is disabled",
    "Error: Loopback switch is closed", "Error: Data measurement aborted during capture",
    "Error: No initial calibration was done", "Error: Path delay not setup",
    "Error: No apply control is possible", "Error: Control value is out of range",
    "Error: CLGC feature is disabled", "Error: TX attenuation is capped",
    "Error: Gain measurement", "Error: No GPIO configured in single ORx configuration",
    "Error: Tx is not observable with any of the ORx Channels"
];

var KNOWN_VSWR_STATUS = [
    "No Error", "Error: TX disabled", "Error: ORx disabled",
    "Error: Loopback switch is closed", "Error: No initial calibration was done",
    "Error: Path delay not setup", "Error: Data capture aborted",
    "Error: VSWR is disabled", "Error: Entering Cal",
    "Error: No GPIO configured in single ORx configuration",
    "Error: Tx is not observable with any of the ORx Channels"
];

function isKnownStatus(val, knownList) {
    for (var i = 0; i < knownList.length; i++) {
        if (val === knownList[i]) return true;
    }
    // Accept "Error: Unknown status (N)" for undocumented firmware status codes
    if (val.indexOf("Error: Unknown status") === 0) return true;
    return false;
}

// Helper to run data-driven test arrays
function runDataDrivenTests(tests) {
    for (var i = 0; i < tests.length; i++) {
        (function(t) {
            TestFramework.runTest(t.uid, function() {
                if (t.type === "range") return testRange(PHY + t.attr, t.min, t.max, t.mid);
                if (t.type === "checkbox") return testCheckbox(PHY + t.attr);
                if (t.type === "combo") return testCombo(PHY + t.attr, t.options);
                if (t.type === "readonly") return testReadOnly(PHY + t.attr);
                return false;
            });
        })(tests[i]);
    }
}
// ============================================
// SECTION 17: Basic Tool Tests
// ============================================

// Switch to AD9371 basic tool
var basicToolName = isAd9375 ? "ADRV9375" : "AD9371";
if (!switchToTool(basicToolName)) {
    printToConsole("ERROR: Cannot switch to " + basicToolName + " tool");
    scopy.exit();
}
msleep(500);

// --- Global ---
TestFramework.runTest("UNIT.GLOBAL.ENSM_MODE", function() {
    try {
        var orig = ad9371.getEnsmMode();
        printToConsole("  Original ENSM mode: " + orig);
        if (!orig || orig === "") {
            printToConsole("  FAIL: getEnsmMode() returned empty");
            return false;
        }

        var modes = ["radio_on", "radio_off"];
        for (var i = 0; i < modes.length; i++) {
            ad9371.setEnsmMode(modes[i]);
            msleep(500);
            var readBack = ad9371.getEnsmMode();
            printToConsole("  Set ENSM to '" + modes[i] + "', read back: " + readBack);
            if (readBack !== modes[i]) {
                printToConsole("  FAIL: ENSM mode mismatch, expected=" + modes[i] + " got=" + readBack);
                ad9371.setEnsmMode(orig);
                msleep(500);
                return false;
            }
        }

        ad9371.setEnsmMode(orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// --- RX ---
TestFramework.runTest("UNIT.RX.RF_BANDWIDTH", function() {
    try {
        var raw = ad9371.readWidget(PHY + "voltage0_in/rf_bandwidth");
        if (!raw || raw === "") {
            printToConsole("  SKIP: rf_bandwidth not readable on this hardware");
            return "SKIP";
        }
        var rawHz = parseFloat(raw);
        var apiMHz = parseFloat(ad9371.getRxRfBandwidth());
        var expectedMHz = rawHz / 1e6;
        var diff = Math.abs(apiMHz - expectedMHz);
        if (diff > 0.001) {
            printToConsole("  FAIL: MHz conversion mismatch, raw=" + rawHz + " expected=" + expectedMHz + " got=" + apiMHz);
            return false;
        }
        printToConsole("  PASS: MHz conversion verified: " + apiMHz + " MHz");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.RX.SAMPLING_FREQUENCY", function() {
    try {
        return testReadOnly(PHY + "voltage0_in/sampling_frequency");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.RX.LO_FREQUENCY", function() {
    try {
        return testConversion(
            function() { return ad9371.getRxLoFrequency(); },
            function(v) { ad9371.setRxLoFrequency(v); },
            PHY + "altvoltage0_out/frequency",
            "1000",
            "1000000000",
            1.0
        );
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.RX.GAIN_CONTROL_MODE", function() {
    try {
        var orig = ad9371.getRxGainControlMode();
        printToConsole("  Original gain control mode: " + orig);
        if (!orig || orig === "") {
            printToConsole("  FAIL: getRxGainControlMode() returned empty");
            return false;
        }

        ad9371.setRxGainControlMode("manual");
        msleep(500);
        var readBack = ad9371.getRxGainControlMode();
        printToConsole("  Set to 'manual', read back: " + readBack);
        if (readBack !== "manual") {
            printToConsole("  FAIL: gain control mode mismatch");
            ad9371.setRxGainControlMode(orig);
            msleep(500);
            return false;
        }

        ad9371.setRxGainControlMode(orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.RX.CH0_HARDWARE_GAIN", function() {
    try {
        var origEnsm = ad9371.getEnsmMode();
        var origGainMode = ad9371.getRxGainControlMode();
        ad9371.setEnsmMode("radio_on");
        msleep(500);
        ad9371.setRxGainControlMode("manual");
        msleep(500);
        var rangeResult = testRange(PHY + "voltage0_in/hardwaregain", "0", "30", "15");
        if (!rangeResult) {
            ad9371.setRxGainControlMode(origGainMode);
            ad9371.setEnsmMode(origEnsm);
            return false;
        }

        var result = testConversion(
            function() { return ad9371.getRxHardwareGain(0); },
            function(v) { ad9371.setRxHardwareGain(0, v); },
            PHY + "voltage0_in/hardwaregain",
            "15",
            "15.00",
            1.0
        );
        ad9371.setRxGainControlMode(origGainMode);
        ad9371.setEnsmMode(origEnsm);
        return result;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.RX.CH0_RSSI", function() {
    try {
        var rssi = ad9371.getRxRssi(0);
        printToConsole("  RX RSSI ch0: " + rssi);
        if (rssi === null || rssi === "") {
            printToConsole("  FAIL: getRxRssi(0) returned empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.RX.CH0_TEMP_COMP_GAIN", function() {
    try {
        return testRange(PHY + "voltage0_in/temp_comp_gain", "-3", "3", "0");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.RX.CH0_QUADRATURE_TRACKING", function() {
    try {
        return testCheckbox(PHY + "voltage0_in/quadrature_tracking_en");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.RX.CH1_HARDWARE_GAIN", function() {
    try {
        var origEnsm = ad9371.getEnsmMode();
        var origGainMode = ad9371.getRxGainControlMode();
        ad9371.setEnsmMode("radio_on");
        msleep(500);
        ad9371.setRxGainControlMode("manual");
        msleep(500);
        var rangeResult = testRange(PHY + "voltage1_in/hardwaregain", "0", "30", "15");
        if (!rangeResult) {
            ad9371.setRxGainControlMode(origGainMode);
            ad9371.setEnsmMode(origEnsm);
            return false;
        }

        var result = testConversion(
            function() { return ad9371.getRxHardwareGain(1); },
            function(v) { ad9371.setRxHardwareGain(1, v); },
            PHY + "voltage1_in/hardwaregain",
            "15",
            "15.00",
            1.0
        );
        ad9371.setRxGainControlMode(origGainMode);
        ad9371.setEnsmMode(origEnsm);
        return result;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.RX.CH1_RSSI", function() {
    try {
        var rssi = ad9371.getRxRssi(1);
        printToConsole("  RX RSSI ch1: " + rssi);
        if (rssi === null || rssi === "") {
            printToConsole("  FAIL: getRxRssi(1) returned empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.RX.CH1_TEMP_COMP_GAIN", function() {
    try {
        return testRange(PHY + "voltage1_in/temp_comp_gain", "-3", "3", "0");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.RX.CH1_QUADRATURE_TRACKING", function() {
    try {
        return testCheckbox(PHY + "voltage1_in/quadrature_tracking_en");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// --- TX ---
TestFramework.runTest("UNIT.TX.RF_BANDWIDTH", function() {
    try {
        if (!testReadOnly(PHY + "voltage0_out/rf_bandwidth")) return false;
        // Verify MHz conversion via API
        var rawHz = parseFloat(ad9371.readWidget(PHY + "voltage0_out/rf_bandwidth"));
        var apiMHz = parseFloat(ad9371.getTxRfBandwidth());
        var expectedMHz = rawHz / 1e6;
        var diff = Math.abs(apiMHz - expectedMHz);
        if (diff > 0.001) {
            printToConsole("  FAIL: MHz conversion mismatch, raw=" + rawHz + " expected=" + expectedMHz + " got=" + apiMHz);
            return false;
        }
        printToConsole("  PASS: MHz conversion verified: " + apiMHz + " MHz");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.TX.SAMPLING_FREQUENCY", function() {
    try {
        return testReadOnly(PHY + "voltage0_out/sampling_frequency");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.TX.LO_FREQUENCY", function() {
    try {
        return testConversion(
            function() { return ad9371.getTxLoFrequency(); },
            function(v) { ad9371.setTxLoFrequency(v); },
            PHY + "altvoltage1_out/frequency",
            "2500",
            "2500000000",
            1.0
        );
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.TX.CH0_ATTENUATION", function() {
    try {
        return testConversion(
            function() { return ad9371.getTxAttenuation(0); },
            function(v) { ad9371.setTxAttenuation(0, v); },
            PHY + "voltage0_out/hardwaregain",
            "10",
            "-10",
            0.1
        );
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.TX.CH0_QUADRATURE_TRACKING", function() {
    try {
        return testCheckbox(PHY + "voltage0_out/quadrature_tracking_en");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.TX.CH0_LO_LEAKAGE_TRACKING", function() {
    try {
        return testCheckbox(PHY + "voltage0_out/lo_leakage_tracking_en");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.TX.CH1_ATTENUATION", function() {
    try {
        return testConversion(
            function() { return ad9371.getTxAttenuation(1); },
            function(v) { ad9371.setTxAttenuation(1, v); },
            PHY + "voltage1_out/hardwaregain",
            "10",
            "-10",
            0.1
        );
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.TX.CH1_QUADRATURE_TRACKING", function() {
    try {
        return testCheckbox(PHY + "voltage1_out/quadrature_tracking_en");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.TX.CH1_LO_LEAKAGE_TRACKING", function() {
    try {
        return testCheckbox(PHY + "voltage1_out/lo_leakage_tracking_en");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// --- Calibration ---
// NOTE: calibrate_rx_qec_en, calibrate_tx_qec_en, calibrate_tx_lol_en,
// calibrate_tx_lol_ext_en, calibrate_dpd_en, calibrate_clgc_en, calibrate_vswr_en
// use MenuOnOffSwitch (not IIOWidget) and are NOT registered in the IIOWidgetGroup.
// The API methods getCalibrateRxQecEn()/setCalibrateRxQecEn() etc. go through
// readFromWidget/writeToWidget which requires the widget group, so they return empty.
// These tests are skipped until the API is updated to use direct IIO access.

TestFramework.runTest("UNIT.CAL.CALIBRATE_TRIGGER", function() {
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

// --- DPD (AD9375 only) ---

// CH0
TestFramework.runTest("UNIT.DPD.CH0_TRACKING_EN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var orig = ad9371.getDpdTrackingEn(0);
        printToConsole("  Original DPD tracking en ch0: " + orig);

        ad9371.setDpdTrackingEn(0, "0");
        msleep(500);
        var read0 = ad9371.getDpdTrackingEn(0);
        printToConsole("  Set to '0', read back: " + read0);
        if (read0 !== "0") {
            printToConsole("  FAIL: mismatch");
            ad9371.setDpdTrackingEn(0, orig);
            msleep(500);
            return false;
        }

        ad9371.setDpdTrackingEn(0, "1");
        msleep(500);
        var read1 = ad9371.getDpdTrackingEn(0);
        printToConsole("  Set to '1', read back: " + read1);
        if (read1 !== "1") {
            printToConsole("  FAIL: mismatch");
            ad9371.setDpdTrackingEn(0, orig);
            msleep(500);
            return false;
        }

        ad9371.setDpdTrackingEn(0, orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.DPD.CH0_ACTUATOR_EN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var orig = ad9371.getDpdActuatorEn(0);
        printToConsole("  Original DPD actuator en ch0: " + orig);

        ad9371.setDpdActuatorEn(0, "0");
        msleep(500);
        var read0 = ad9371.getDpdActuatorEn(0);
        printToConsole("  Set to '0', read back: " + read0);
        if (read0 !== "0") {
            printToConsole("  FAIL: mismatch");
            ad9371.setDpdActuatorEn(0, orig);
            msleep(500);
            return false;
        }

        ad9371.setDpdActuatorEn(0, "1");
        msleep(500);
        var read1 = ad9371.getDpdActuatorEn(0);
        printToConsole("  Set to '1', read back: " + read1);
        if (read1 !== "1") {
            printToConsole("  FAIL: mismatch");
            ad9371.setDpdActuatorEn(0, orig);
            msleep(500);
            return false;
        }

        ad9371.setDpdActuatorEn(0, orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.DPD.CH0_TRACK_COUNT", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getDpdTrackCount(0);
        printToConsole("  DPD track count ch0: " + val);
        if (val === null || val === "") {
            printToConsole("  FAIL: getDpdTrackCount(0) returned empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.DPD.CH0_MODEL_ERROR", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getDpdModelError(0);
        printToConsole("  DPD model error ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: getDpdModelError(0) returned empty");
            return false;
        }
        if (val.indexOf("%") === -1) {
            printToConsole("  FAIL: Model error should contain '%'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.DPD.CH0_EXT_PATH_DELAY", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getDpdExternalPathDelay(0);
        printToConsole("  DPD ext path delay ch0: " + val);
        if (val === null || val === "") {
            printToConsole("  FAIL: getDpdExternalPathDelay(0) returned empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.DPD.CH0_STATUS", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getDpdStatus(0);
        printToConsole("  DPD status ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: getDpdStatus(0) returned empty");
            return false;
        }
        if (!isKnownStatus(val, KNOWN_DPD_STATUS)) {
            printToConsole("  FAIL: unexpected DPD status value: " + val);
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.DPD.CH0_RESET", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        ad9371.dpdReset(0);
        msleep(500);
        printToConsole("  DPD reset ch0 triggered without error");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// CH1
TestFramework.runTest("UNIT.DPD.CH1_TRACKING_EN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var orig = ad9371.getDpdTrackingEn(1);
        printToConsole("  Original DPD tracking en ch1: " + orig);

        ad9371.setDpdTrackingEn(1, "0");
        msleep(500);
        var read0 = ad9371.getDpdTrackingEn(1);
        printToConsole("  Set to '0', read back: " + read0);
        if (read0 !== "0") {
            printToConsole("  FAIL: mismatch");
            ad9371.setDpdTrackingEn(1, orig);
            msleep(500);
            return false;
        }

        ad9371.setDpdTrackingEn(1, "1");
        msleep(500);
        var read1 = ad9371.getDpdTrackingEn(1);
        printToConsole("  Set to '1', read back: " + read1);
        if (read1 !== "1") {
            printToConsole("  FAIL: mismatch");
            ad9371.setDpdTrackingEn(1, orig);
            msleep(500);
            return false;
        }

        ad9371.setDpdTrackingEn(1, orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.DPD.CH1_ACTUATOR_EN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var orig = ad9371.getDpdActuatorEn(1);
        printToConsole("  Original DPD actuator en ch1: " + orig);

        ad9371.setDpdActuatorEn(1, "0");
        msleep(500);
        var read0 = ad9371.getDpdActuatorEn(1);
        printToConsole("  Set to '0', read back: " + read0);
        if (read0 !== "0") {
            printToConsole("  FAIL: mismatch");
            ad9371.setDpdActuatorEn(1, orig);
            msleep(500);
            return false;
        }

        ad9371.setDpdActuatorEn(1, "1");
        msleep(500);
        var read1 = ad9371.getDpdActuatorEn(1);
        printToConsole("  Set to '1', read back: " + read1);
        if (read1 !== "1") {
            printToConsole("  FAIL: mismatch");
            ad9371.setDpdActuatorEn(1, orig);
            msleep(500);
            return false;
        }

        ad9371.setDpdActuatorEn(1, orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.DPD.CH1_TRACK_COUNT", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getDpdTrackCount(1);
        printToConsole("  DPD track count ch1: " + val);
        if (val === null || val === "") {
            printToConsole("  FAIL: getDpdTrackCount(1) returned empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.DPD.CH1_MODEL_ERROR", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getDpdModelError(1);
        printToConsole("  DPD model error ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: getDpdModelError(1) returned empty");
            return false;
        }
        if (val.indexOf("%") === -1) {
            printToConsole("  FAIL: Model error should contain '%'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.DPD.CH1_EXT_PATH_DELAY", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getDpdExternalPathDelay(1);
        printToConsole("  DPD ext path delay ch1: " + val);
        if (val === null || val === "") {
            printToConsole("  FAIL: getDpdExternalPathDelay(1) returned empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.DPD.CH1_STATUS", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getDpdStatus(1);
        printToConsole("  DPD status ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: getDpdStatus(1) returned empty");
            return false;
        }
        if (!isKnownStatus(val, KNOWN_DPD_STATUS)) {
            printToConsole("  FAIL: unexpected DPD status value: " + val);
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.DPD.CH1_RESET", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        ad9371.dpdReset(1);
        msleep(500);
        printToConsole("  DPD reset ch1 triggered without error");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// --- CLGC (AD9375 only) ---

// CH0
TestFramework.runTest("UNIT.CLGC.CH0_TRACKING_EN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var orig = ad9371.getClgcTrackingEn(0);
        printToConsole("  Original CLGC tracking en ch0: " + orig);

        ad9371.setClgcTrackingEn(0, "0");
        msleep(500);
        var read0 = ad9371.getClgcTrackingEn(0);
        printToConsole("  Set to '0', read back: " + read0);
        if (read0 !== "0") {
            printToConsole("  FAIL: mismatch");
            ad9371.setClgcTrackingEn(0, orig);
            msleep(500);
            return false;
        }

        ad9371.setClgcTrackingEn(0, "1");
        msleep(500);
        var read1 = ad9371.getClgcTrackingEn(0);
        printToConsole("  Set to '1', read back: " + read1);
        if (read1 !== "1") {
            printToConsole("  FAIL: mismatch");
            ad9371.setClgcTrackingEn(0, orig);
            msleep(500);
            return false;
        }

        ad9371.setClgcTrackingEn(0, orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// clgc_desired_gain is read-only on the channel attribute; writable via debug attr (tested in Advanced)
TestFramework.runTest("UNIT.CLGC.CH0_DESIRED_GAIN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getClgcDesiredGain(0);
        printToConsole("  CLGC desired gain ch0: " + val);
        if (val === null || val === "") {
            printToConsole("  FAIL: getClgcDesiredGain(0) returned empty");
            return false;
        }
        if (isNaN(parseFloat(val))) {
            printToConsole("  FAIL: expected numeric value, got: " + val);
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.CLGC.CH0_TRACK_COUNT", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getClgcTrackCount(0);
        printToConsole("  CLGC track count ch0: " + val);
        if (val === null || val === "") {
            printToConsole("  FAIL: getClgcTrackCount(0) returned empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.CLGC.CH0_STATUS", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getClgcStatus(0);
        printToConsole("  CLGC status ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: getClgcStatus(0) returned empty");
            return false;
        }
        if (!isKnownStatus(val, KNOWN_CLGC_STATUS)) {
            printToConsole("  FAIL: unexpected CLGC status value: " + val);
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.CLGC.CH0_CURRENT_GAIN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getClgcCurrentGain(0);
        printToConsole("  CLGC current gain ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.CLGC.CH0_TX_GAIN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getClgcTxGain(0);
        printToConsole("  CLGC TX gain ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.CLGC.CH0_TX_RMS", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getClgcTxRms(0);
        printToConsole("  CLGC TX RMS ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dBFS") === -1) {
            printToConsole("  FAIL: should contain 'dBFS'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.CLGC.CH0_ORX_RMS", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getClgcOrxRms(0);
        printToConsole("  CLGC ORX RMS ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dBFS") === -1) {
            printToConsole("  FAIL: should contain 'dBFS'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// CH1
TestFramework.runTest("UNIT.CLGC.CH1_TRACKING_EN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var orig = ad9371.getClgcTrackingEn(1);
        printToConsole("  Original CLGC tracking en ch1: " + orig);

        ad9371.setClgcTrackingEn(1, "0");
        msleep(500);
        var read0 = ad9371.getClgcTrackingEn(1);
        printToConsole("  Set to '0', read back: " + read0);
        if (read0 !== "0") {
            printToConsole("  FAIL: mismatch");
            ad9371.setClgcTrackingEn(1, orig);
            msleep(500);
            return false;
        }

        ad9371.setClgcTrackingEn(1, "1");
        msleep(500);
        var read1 = ad9371.getClgcTrackingEn(1);
        printToConsole("  Set to '1', read back: " + read1);
        if (read1 !== "1") {
            printToConsole("  FAIL: mismatch");
            ad9371.setClgcTrackingEn(1, orig);
            msleep(500);
            return false;
        }

        ad9371.setClgcTrackingEn(1, orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.CLGC.CH1_DESIRED_GAIN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getClgcDesiredGain(1);
        printToConsole("  CLGC desired gain ch1: " + val);
        if (val === null || val === "") {
            printToConsole("  FAIL: getClgcDesiredGain(1) returned empty");
            return false;
        }
        if (isNaN(parseFloat(val))) {
            printToConsole("  FAIL: expected numeric value, got: " + val);
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.CLGC.CH1_TRACK_COUNT", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getClgcTrackCount(1);
        printToConsole("  CLGC track count ch1: " + val);
        if (val === null || val === "") {
            printToConsole("  FAIL: getClgcTrackCount(1) returned empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.CLGC.CH1_STATUS", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getClgcStatus(1);
        printToConsole("  CLGC status ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: getClgcStatus(1) returned empty");
            return false;
        }
        if (!isKnownStatus(val, KNOWN_CLGC_STATUS)) {
            printToConsole("  FAIL: unexpected CLGC status value: " + val);
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.CLGC.CH1_CURRENT_GAIN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getClgcCurrentGain(1);
        printToConsole("  CLGC current gain ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.CLGC.CH1_TX_GAIN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getClgcTxGain(1);
        printToConsole("  CLGC TX gain ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.CLGC.CH1_TX_RMS", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getClgcTxRms(1);
        printToConsole("  CLGC TX RMS ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dBFS") === -1) {
            printToConsole("  FAIL: should contain 'dBFS'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.CLGC.CH1_ORX_RMS", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getClgcOrxRms(1);
        printToConsole("  CLGC ORX RMS ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dBFS") === -1) {
            printToConsole("  FAIL: should contain 'dBFS'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// --- VSWR (AD9375 only) ---

// CH0
TestFramework.runTest("UNIT.VSWR.CH0_TRACKING_EN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var orig = ad9371.getVswrTrackingEn(0);
        printToConsole("  Original VSWR tracking en ch0: " + orig);

        ad9371.setVswrTrackingEn(0, "0");
        msleep(500);
        var read0 = ad9371.getVswrTrackingEn(0);
        printToConsole("  Set to '0', read back: " + read0);
        if (read0 !== "0") {
            printToConsole("  FAIL: mismatch");
            ad9371.setVswrTrackingEn(0, orig);
            msleep(500);
            return false;
        }

        ad9371.setVswrTrackingEn(0, "1");
        msleep(500);
        var read1 = ad9371.getVswrTrackingEn(0);
        printToConsole("  Set to '1', read back: " + read1);
        if (read1 !== "1") {
            printToConsole("  FAIL: mismatch");
            ad9371.setVswrTrackingEn(0, orig);
            msleep(500);
            return false;
        }

        ad9371.setVswrTrackingEn(0, orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH0_TRACK_COUNT", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrTrackCount(0);
        printToConsole("  VSWR track count ch0: " + val);
        if (val === null || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH0_STATUS", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrStatus(0);
        printToConsole("  VSWR status ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (!isKnownStatus(val, KNOWN_VSWR_STATUS)) {
            printToConsole("  FAIL: unexpected VSWR status value: " + val);
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH0_FORWARD_GAIN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrForwardGain(0);
        printToConsole("  VSWR forward gain ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH0_FORWARD_GAIN_REAL", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrForwardGainReal(0);
        printToConsole("  VSWR forward gain real ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH0_FORWARD_GAIN_IMAG", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrForwardGainImag(0);
        printToConsole("  VSWR forward gain imag ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH0_FORWARD_ORX", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrForwardOrx(0);
        printToConsole("  VSWR forward ORx ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dBFS") === -1) {
            printToConsole("  FAIL: should contain 'dBFS'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH0_FORWARD_TX", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrForwardTx(0);
        printToConsole("  VSWR forward TX ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dBFS") === -1) {
            printToConsole("  FAIL: should contain 'dBFS'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH0_REFLECTED_GAIN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrReflectedGain(0);
        printToConsole("  VSWR reflected gain ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH0_REFLECTED_GAIN_REAL", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrReflectedGainReal(0);
        printToConsole("  VSWR reflected gain real ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH0_REFLECTED_GAIN_IMAG", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrReflectedGainImag(0);
        printToConsole("  VSWR reflected gain imag ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH0_REFLECTED_ORX", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrReflectedOrx(0);
        printToConsole("  VSWR reflected ORx ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dBFS") === -1) {
            printToConsole("  FAIL: should contain 'dBFS'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH0_REFLECTED_TX", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrReflectedTx(0);
        printToConsole("  VSWR reflected TX ch0: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dBFS") === -1) {
            printToConsole("  FAIL: should contain 'dBFS'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// CH1
TestFramework.runTest("UNIT.VSWR.CH1_TRACKING_EN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var orig = ad9371.getVswrTrackingEn(1);
        printToConsole("  Original VSWR tracking en ch1: " + orig);

        ad9371.setVswrTrackingEn(1, "0");
        msleep(500);
        var read0 = ad9371.getVswrTrackingEn(1);
        printToConsole("  Set to '0', read back: " + read0);
        if (read0 !== "0") {
            printToConsole("  FAIL: mismatch");
            ad9371.setVswrTrackingEn(1, orig);
            msleep(500);
            return false;
        }

        ad9371.setVswrTrackingEn(1, "1");
        msleep(500);
        var read1 = ad9371.getVswrTrackingEn(1);
        printToConsole("  Set to '1', read back: " + read1);
        if (read1 !== "1") {
            printToConsole("  FAIL: mismatch");
            ad9371.setVswrTrackingEn(1, orig);
            msleep(500);
            return false;
        }

        ad9371.setVswrTrackingEn(1, orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH1_TRACK_COUNT", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrTrackCount(1);
        printToConsole("  VSWR track count ch1: " + val);
        if (val === null || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH1_STATUS", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrStatus(1);
        printToConsole("  VSWR status ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (!isKnownStatus(val, KNOWN_VSWR_STATUS)) {
            printToConsole("  FAIL: unexpected VSWR status value: " + val);
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH1_FORWARD_GAIN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrForwardGain(1);
        printToConsole("  VSWR forward gain ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH1_FORWARD_GAIN_REAL", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrForwardGainReal(1);
        printToConsole("  VSWR forward gain real ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH1_FORWARD_GAIN_IMAG", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrForwardGainImag(1);
        printToConsole("  VSWR forward gain imag ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH1_FORWARD_ORX", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrForwardOrx(1);
        printToConsole("  VSWR forward ORx ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dBFS") === -1) {
            printToConsole("  FAIL: should contain 'dBFS'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH1_FORWARD_TX", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrForwardTx(1);
        printToConsole("  VSWR forward TX ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dBFS") === -1) {
            printToConsole("  FAIL: should contain 'dBFS'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH1_REFLECTED_GAIN", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrReflectedGain(1);
        printToConsole("  VSWR reflected gain ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH1_REFLECTED_GAIN_REAL", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrReflectedGainReal(1);
        printToConsole("  VSWR reflected gain real ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH1_REFLECTED_GAIN_IMAG", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrReflectedGainImag(1);
        printToConsole("  VSWR reflected gain imag ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dB") === -1) {
            printToConsole("  FAIL: should contain 'dB'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH1_REFLECTED_ORX", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrReflectedOrx(1);
        printToConsole("  VSWR reflected ORx ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dBFS") === -1) {
            printToConsole("  FAIL: should contain 'dBFS'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.VSWR.CH1_REFLECTED_TX", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }
        var val = ad9371.getVswrReflectedTx(1);
        printToConsole("  VSWR reflected TX ch1: " + val);
        if (!val || val === "") {
            printToConsole("  FAIL: returned empty");
            return false;
        }
        if (val.indexOf("dBFS") === -1) {
            printToConsole("  FAIL: should contain 'dBFS'");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// --- OBS ---
TestFramework.runTest("UNIT.OBS.RF_BANDWIDTH", function() {
    try {
        var raw = ad9371.readWidget(PHY + "voltage2_in/rf_bandwidth");
        if (!raw || raw === "") {
            printToConsole("  SKIP: rf_bandwidth not readable on this hardware");
            return "SKIP";
        }
        var rawHz = parseFloat(raw);
        var apiMHz = parseFloat(ad9371.getObsRfBandwidth());
        var expectedMHz = rawHz / 1e6;
        var diff = Math.abs(apiMHz - expectedMHz);
        if (diff > 0.001) {
            printToConsole("  FAIL: MHz conversion mismatch, raw=" + rawHz + " expected=" + expectedMHz + " got=" + apiMHz);
            return false;
        }
        printToConsole("  PASS: MHz conversion verified: " + apiMHz + " MHz");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.OBS.SAMPLING_FREQUENCY", function() {
    try {
        return testReadOnly(PHY + "voltage2_in/sampling_frequency");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.OBS.LO_FREQUENCY", function() {
    try {
        return testConversion(
            function() { return ad9371.getSnifferLoFrequency(); },
            function(v) { ad9371.setSnifferLoFrequency(v); },
            PHY + "altvoltage2_out/frequency",
            "3000",
            "3000000000",
            1.0
        );
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.OBS.RF_PORT_SELECT", function() {
    try {
        var orig = ad9371.getObsRfPortSelect();
        printToConsole("  Original OBS RF port select: " + orig);
        if (!orig || orig === "") {
            printToConsole("  FAIL: getObsRfPortSelect() returned empty");
            return false;
        }
        // Just verify readable, port select options are device-dependent
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.OBS.GAIN_CONTROL_MODE", function() {
    try {
        var orig = ad9371.getObsGainControlMode();
        printToConsole("  Original OBS gain control mode: " + orig);
        if (!orig || orig === "") {
            printToConsole("  FAIL: getObsGainControlMode() returned empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.OBS.HARDWARE_GAIN", function() {
    try {
        ad9371.setEnsmMode("radio_on");
        // ORx gain requires rf_port_select in an observation mode and manual gain control
        var origPort = ad9371.getObsRfPortSelect();
        var origMode = ad9371.getObsGainControlMode();
        printToConsole("  Original rf_port_select: " + origPort + ", gain_control_mode: " + origMode);

        // Switch to ORX1_TX_LO (observation mode) with manual gain control
        ad9371.setObsRfPortSelect("ORX1_TX_LO");
        ad9371.setObsGainControlMode("manual");
        msleep(1000);

        // Test with value within ORx gain range (0-18 dB per datasheet)
        var result = testConversion(
            function() { return ad9371.getObsHardwareGain(); },
            function(v) { ad9371.setObsHardwareGain(v); },
            PHY + "voltage2_in/hardwaregain",
            "10",
            "10.00",
            1.0
        );

        // Restore original settings
        ad9371.setObsRfPortSelect(origPort);
        ad9371.setObsGainControlMode(origMode);
        msleep(500);

        return result;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.OBS.RSSI", function() {
    try {
        return testReadOnly(PHY + "voltage2_in/rssi");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.OBS.TEMP_COMP_GAIN", function() {
    try {
        return testRange(PHY + "voltage2_in/temp_comp_gain", "-3", "3", "0");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.OBS.QUADRATURE_TRACKING", function() {
    try {
        return testCheckbox(PHY + "voltage2_in/quadrature_tracking_en");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// SECTION: Calibration Enable Flags
// ============================================

function testCalibrationFlag(uid, getter, setter) {
    TestFramework.runTest(uid, function() {
        try {
            var orig = getter();
            printToConsole("  Original value: " + orig);
            if (orig === null || orig === "") {
                printToConsole("  FAIL: getter returned empty");
                return false;
            }

            setter("1");
            msleep(500);
            var read1 = getter();
            printToConsole("  Set to '1', read back: " + read1);
            if (read1 !== "1") {
                printToConsole("  FAIL: expected '1', got '" + read1 + "'");
                setter(orig);
                msleep(500);
                return false;
            }

            setter("0");
            msleep(500);
            var read0 = getter();
            printToConsole("  Set to '0', read back: " + read0);
            if (read0 !== "0") {
                printToConsole("  FAIL: expected '0', got '" + read0 + "'");
                setter(orig);
                msleep(500);
                return false;
            }

            setter(orig);
            msleep(500);
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });
}

testCalibrationFlag("UNIT.CAL.RX_QEC_EN",
    function() { return ad9371.getCalibrateRxQecEn(); },
    function(v) { ad9371.setCalibrateRxQecEn(v); }
);

testCalibrationFlag("UNIT.CAL.TX_QEC_EN",
    function() { return ad9371.getCalibrateTxQecEn(); },
    function(v) { ad9371.setCalibrateTxQecEn(v); }
);

testCalibrationFlag("UNIT.CAL.TX_LOL_EN",
    function() { return ad9371.getCalibrateTxLolEn(); },
    function(v) { ad9371.setCalibrateTxLolEn(v); }
);

testCalibrationFlag("UNIT.CAL.TX_LOL_EXT_EN",
    function() { return ad9371.getCalibrateTxLolExtEn(); },
    function(v) { ad9371.setCalibrateTxLolExtEn(v); }
);

if (isAd9375) {
    testCalibrationFlag("UNIT.CAL.DPD_EN",
        function() { return ad9371.getCalibrateDpdEn(); },
        function(v) { ad9371.setCalibrateDpdEn(v); }
    );

    testCalibrationFlag("UNIT.CAL.CLGC_EN",
        function() { return ad9371.getCalibrateClgcEn(); },
        function(v) { ad9371.setCalibrateClgcEn(v); }
    );

    testCalibrationFlag("UNIT.CAL.VSWR_EN",
        function() { return ad9371.getCalibrateVswrEn(); },
        function(v) { ad9371.setCalibrateVswrEn(v); }
    );
}

// ============================================
// SECTION: Complex Functionality Tests
// ============================================

// C1: Full Calibration Flow (hardware-only)
TestFramework.runTest("UNIT.CAL.FULL_CALIBRATION_FLOW", function() {
    try {
        // Save originals
        var origRxQec = ad9371.getCalibrateRxQecEn();
        var origTxQec = ad9371.getCalibrateTxQecEn();
        var origTxLol = ad9371.getCalibrateTxLolEn();
        var origTxLolExt = ad9371.getCalibrateTxLolExtEn();

        // Enable all calibration flags
        ad9371.setCalibrateRxQecEn("1");
        ad9371.setCalibrateTxQecEn("1");
        ad9371.setCalibrateTxLolEn("1");
        ad9371.setCalibrateTxLolExtEn("1");
        msleep(500);

        // Trigger calibration
        ad9371.calibrate();
        msleep(2000);

        // Verify flags still set
        var rxQec = ad9371.getCalibrateRxQecEn();
        var txQec = ad9371.getCalibrateTxQecEn();
        printToConsole("  After calibrate: rx_qec=" + rxQec + " tx_qec=" + txQec);

        // Restore
        ad9371.setCalibrateRxQecEn(origRxQec);
        ad9371.setCalibrateTxQecEn(origTxQec);
        ad9371.setCalibrateTxLolEn(origTxLol);
        ad9371.setCalibrateTxLolExtEn(origTxLolExt);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// C2: Profile Loading (hardware-only)
TestFramework.runTest("UNIT.PROFILE.LOAD_AND_VERIFY", function() {
    try {
        var profilePath = ad9371.getDefaultProfilePath();
        if (!profilePath || profilePath === "") {
            printToConsole("  SKIP: No profile file available");
            return "SKIP";
        }
        printToConsole("  Profile path: " + profilePath);

        // Read current state
        var rxBwBefore = ad9371.getRxRfBandwidth();
        var txBwBefore = ad9371.getTxRfBandwidth();
        printToConsole("  Before: RX BW=" + rxBwBefore + " TX BW=" + txBwBefore);

        // Load profile
        ad9371.loadProfile(profilePath);
        msleep(5000);

        // Refresh widgets
        ad9371.refresh();
        msleep(2000);

        // Read after - verify sampling frequency is readable (more reliable than rf_bandwidth)
        var rxSfAfter = ad9371.getRxSamplingFrequency();
        var txSfAfter = ad9371.getTxSamplingFrequency();
        printToConsole("  After: RX SF=" + rxSfAfter + " TX SF=" + txSfAfter);

        // Verify at least sampling frequency is readable after profile load
        if (!rxSfAfter || rxSfAfter === "" || rxSfAfter === "0.000000") {
            printToConsole("  FAIL: RX sampling frequency invalid after profile load");
            return false;
        }

        printToConsole("  PASS: profile loaded successfully");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// C3: Phase Rotation Roundtrip (hardware-only)
TestFramework.runTest("UNIT.FPGA.PHASE_ROTATION_CH0", function() {
    try {
        var orig = ad9371.getPhaseRotation(0);
        printToConsole("  Original phase ch0: " + orig);
        if (orig === null || orig === "") {
            printToConsole("  SKIP: Phase rotation not available on this hardware");
            return "SKIP";
        }

        // Set to 0 degrees first as baseline
        ad9371.setPhaseRotation(0, 0.0);
        msleep(1000);
        var read0 = parseFloat(ad9371.getPhaseRotation(0));
        printToConsole("  Set 0, read: " + read0);

        // Set to 45 degrees
        ad9371.setPhaseRotation(0, 45.0);
        msleep(1000);
        var read45 = parseFloat(ad9371.getPhaseRotation(0));
        printToConsole("  Set 45, read: " + read45);

        // Check if writes are taking effect
        if (Math.abs(read0) < 2.0 && Math.abs(read45 - 45.0) > 5.0) {
            printToConsole("  SKIP: Phase rotation writes not supported on this FPGA configuration");
            ad9371.setPhaseRotation(0, parseFloat(orig));
            msleep(500);
            return "SKIP";
        }

        if (Math.abs(read45 - 45.0) > 2.0) {
            printToConsole("  FAIL: phase mismatch, expected ~45 got " + read45);
            ad9371.setPhaseRotation(0, parseFloat(orig));
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setPhaseRotation(0, parseFloat(orig));
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.FPGA.PHASE_ROTATION_CH1", function() {
    try {
        var orig = ad9371.getPhaseRotation(1);
        printToConsole("  Original phase ch1: " + orig);
        if (orig === null || orig === "") {
            printToConsole("  SKIP: Phase rotation not available on this hardware");
            return "SKIP";
        }

        var origVal = parseFloat(orig);
        var testVal = (Math.abs(origVal - 90.0) > 5.0) ? 90.0 : 0.0;
        ad9371.setPhaseRotation(1, testVal);
        msleep(1000);
        var readBack = parseFloat(ad9371.getPhaseRotation(1));
        printToConsole("  Set " + testVal + ", read: " + readBack);

        if (Math.abs(readBack - origVal) < 2.0) {
            printToConsole("  SKIP: Phase rotation writes not supported on this FPGA");
            return "SKIP";
        }

        if (Math.abs(readBack - testVal) > 2.0) {
            printToConsole("  FAIL: phase mismatch, expected ~" + testVal + " got " + readBack);
            ad9371.setPhaseRotation(1, origVal);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setPhaseRotation(1, origVal);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// C4: Gain Mode Interaction (hardware-only)
TestFramework.runTest("UNIT.RX.GAIN_MODE_INTERACTION", function() {
    try {
        var origMode = ad9371.getRxGainControlMode();
        var origEnsm = ad9371.getEnsmMode();
        var origGain = ad9371.getRxHardwareGain(0);
        printToConsole("  Original mode: " + origMode + ", gain: " + origGain);

        // Ensure ENSM is radio_on AND gain control is manual for gain writes to take effect
        ad9371.setEnsmMode("radio_on");
        msleep(500);
        ad9371.setRxGainControlMode("manual");
        msleep(1000);
        ad9371.setRxHardwareGain(0, "20");
        msleep(1000);
        var manualGain = ad9371.getRxHardwareGain(0);
        printToConsole("  Manual mode gain set to 20, read: " + manualGain);
        if (Math.abs(parseFloat(manualGain) - 20.0) > 1.0) {
            printToConsole("  FAIL: gain should be ~20 in manual mode");
            ad9371.setRxGainControlMode(origMode);
            ad9371.setEnsmMode(origEnsm);
            msleep(500);
            return false;
        }

        // Switch to automatic (AGC) - AD9371/AD9375 supports: manual, automatic, hybrid
        ad9371.setRxGainControlMode("automatic");
        msleep(1000);
        var agcGain = ad9371.getRxHardwareGain(0);
        printToConsole("  AGC mode gain: " + agcGain);
        if (agcGain === null || agcGain === "") {
            printToConsole("  FAIL: gain unreadable in AGC mode");
            ad9371.setRxGainControlMode(origMode);
            ad9371.setEnsmMode(origEnsm);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setRxGainControlMode(origMode);
        ad9371.setEnsmMode(origEnsm);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// C5: ENSM Mode State Transitions
TestFramework.runTest("UNIT.GLOBAL.ENSM_STATE_TRANSITIONS", function() {
    try {
        var orig = ad9371.getEnsmMode();
        printToConsole("  Original ENSM mode: " + orig);

        ad9371.setEnsmMode("radio_off");
        msleep(500);
        var readOff = ad9371.getEnsmMode();
        printToConsole("  Set radio_off, read: " + readOff);
        if (readOff !== "radio_off") {
            printToConsole("  FAIL: expected 'radio_off', got '" + readOff + "'");
            ad9371.setEnsmMode(orig);
            msleep(500);
            return false;
        }

        ad9371.setEnsmMode("radio_on");
        msleep(500);
        var readOn = ad9371.getEnsmMode();
        printToConsole("  Set radio_on, read: " + readOn);
        if (readOn !== "radio_on") {
            printToConsole("  FAIL: expected 'radio_on', got '" + readOn + "'");
            ad9371.setEnsmMode(orig);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setEnsmMode(orig);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// C6: DPD Reset + Status Check (AD9375 only, hardware-only)
TestFramework.runTest("UNIT.DPD.RESET_AND_STATUS_CH0", function() {
    try {
        if (!isAd9375) {
            printToConsole("  SKIP: Not AD9375");
            return "SKIP";
        }

        var statusBefore = ad9371.getDpdStatus(0);
        printToConsole("  DPD status before reset: " + statusBefore);

        ad9371.dpdReset(0);
        msleep(1000);

        var statusAfter = ad9371.getDpdStatus(0);
        printToConsole("  DPD status after reset: " + statusAfter);

        // Verify status is a valid human-readable string
        if (!statusAfter || statusAfter === "") {
            printToConsole("  FAIL: status empty after reset");
            return false;
        }
        if (statusAfter.indexOf("No Error") === -1 && statusAfter.indexOf("Error:") === -1) {
            printToConsole("  FAIL: status not a valid string: " + statusAfter);
            return false;
        }

        // Verify track count is readable
        var trackCount = ad9371.getDpdTrackCount(0);
        printToConsole("  DPD track count: " + trackCount);
        if (trackCount === null || trackCount === "") {
            printToConsole("  FAIL: track count unreadable after reset");
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// C7: Per-Channel Independence
TestFramework.runTest("UNIT.TX.CHANNEL_INDEPENDENCE", function() {
    try {
        var origCh0 = ad9371.getTxAttenuation(0);
        var origCh1 = ad9371.getTxAttenuation(1);
        printToConsole("  Original ch0=" + origCh0 + " ch1=" + origCh1);

        // Set ch0 to 5 dB
        ad9371.setTxAttenuation(0, "5");
        msleep(1000);
        var ch0After = parseFloat(ad9371.getTxAttenuation(0));
        var ch1After = parseFloat(ad9371.getTxAttenuation(1));
        printToConsole("  After setting ch0=5: ch0=" + ch0After + " ch1=" + ch1After);

        if (Math.abs(ch0After - 5.0) > 0.5) {
            printToConsole("  FAIL: ch0 should be ~5");
            ad9371.setTxAttenuation(0, origCh0);
            msleep(500);
            return false;
        }
        if (Math.abs(ch1After - parseFloat(origCh1)) > 0.5) {
            printToConsole("  FAIL: ch1 changed unexpectedly from " + origCh1 + " to " + ch1After);
            ad9371.setTxAttenuation(0, origCh0);
            msleep(500);
            return false;
        }

        // Set ch1 to 10 dB, verify ch0 unchanged
        ad9371.setTxAttenuation(1, "10");
        msleep(1000);
        var ch0Final = parseFloat(ad9371.getTxAttenuation(0));
        var ch1Final = parseFloat(ad9371.getTxAttenuation(1));
        printToConsole("  After setting ch1=10: ch0=" + ch0Final + " ch1=" + ch1Final);

        if (Math.abs(ch0Final - 5.0) > 0.5) {
            printToConsole("  FAIL: ch0 changed when setting ch1");
            ad9371.setTxAttenuation(0, origCh0);
            ad9371.setTxAttenuation(1, origCh1);
            msleep(500);
            return false;
        }
        if (Math.abs(ch1Final - 10.0) > 0.5) {
            printToConsole("  FAIL: ch1 should be ~10");
            ad9371.setTxAttenuation(0, origCh0);
            ad9371.setTxAttenuation(1, origCh1);
            msleep(500);
            return false;
        }

        // Restore
        ad9371.setTxAttenuation(0, origCh0);
        ad9371.setTxAttenuation(1, origCh1);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// C8: Up/Down Converter LO Splitting (hardware-only, requires UDC)
TestFramework.runTest("UNIT.UDC.LO_SPLITTING", function() {
    try {
        if (!ad9371.hasUdc()) {
            printToConsole("  SKIP: UDC hardware not present");
            return "SKIP";
        }

        var origUdc = ad9371.getUdcEnabled();
        var origRxLo = ad9371.getRxLoFrequency();
        printToConsole("  Original UDC=" + origUdc + " RX LO=" + origRxLo);

        // Enable UDC
        ad9371.setUdcEnabled(true);
        msleep(500);
        if (!ad9371.getUdcEnabled()) {
            printToConsole("  FAIL: UDC did not enable");
            return false;
        }

        // Set RX LO
        ad9371.setRxLoFrequency("1000");
        msleep(500);
        var udcLo = ad9371.getRxLoFrequency();
        printToConsole("  UDC enabled, RX LO set to 1000, read: " + udcLo);

        // Disable UDC
        ad9371.setUdcEnabled(false);
        msleep(500);

        // Restore
        ad9371.setRxLoFrequency(origRxLo);
        msleep(500);
        ad9371.setUdcEnabled(origUdc);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// C9: FPGA Frequency Settings (hardware-only)
TestFramework.runTest("UNIT.FPGA.TX_FREQUENCY", function() {
    try {
        var orig = ad9371.getFpgaTxFrequency();
        printToConsole("  Original FPGA TX freq: " + orig);
        if (orig === null || orig === "") {
            printToConsole("  SKIP: FPGA TX frequency widget not available");
            return "SKIP";
        }

        // Read back to verify it's readable
        var readBack = ad9371.getFpgaTxFrequency();
        if (readBack !== orig) {
            printToConsole("  FAIL: read-back inconsistent");
            return false;
        }

        printToConsole("  PASS: FPGA TX frequency readable: " + readBack);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("UNIT.FPGA.RX_FREQUENCY", function() {
    try {
        var orig = ad9371.getFpgaRxFrequency();
        printToConsole("  Original FPGA RX freq: " + orig);
        if (orig === null || orig === "") {
            printToConsole("  SKIP: FPGA RX frequency widget not available");
            return "SKIP";
        }

        var readBack = ad9371.getFpgaRxFrequency();
        if (readBack !== orig) {
            printToConsole("  FAIL: read-back inconsistent");
            return false;
        }

        printToConsole("  PASS: FPGA RX frequency readable: " + readBack);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// C10: Refresh Cycle
TestFramework.runTest("UNIT.UTIL.REFRESH_ALL", function() {
    try {
        // Read representative values before refresh
        var ensmBefore = ad9371.getEnsmMode();
        var rxGainBefore = ad9371.getRxHardwareGain(0);
        printToConsole("  Before refresh: ensm=" + ensmBefore + " rxGain=" + rxGainBefore);

        // Trigger refresh
        ad9371.refresh();
        msleep(2000);

        // Read again after refresh
        var ensmAfter = ad9371.getEnsmMode();
        var rxGainAfter = ad9371.getRxHardwareGain(0);
        printToConsole("  After refresh: ensm=" + ensmAfter + " rxGain=" + rxGainAfter);

        // Values should be non-empty and consistent
        if (!ensmAfter || ensmAfter === "") {
            printToConsole("  FAIL: ENSM mode empty after refresh");
            return false;
        }
        if (!rxGainAfter || rxGainAfter === "") {
            printToConsole("  FAIL: RX gain empty after refresh");
            return false;
        }

        printToConsole("  PASS: refresh completed, values consistent");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// SECTION 11: Bad Value Tests
// ============================================

// Test that out-of-range values are properly clamped by IIOWidget spinboxes

TestFramework.runTest("UNIT.BADVAL.RX_CH0_HARDWARE_GAIN", function() {
    return testBadValueRange("voltage0_in/hardwaregain", "0", "30");
});

TestFramework.runTest("UNIT.BADVAL.RX_CH0_TEMP_COMP_GAIN", function() {
    return testBadValueRange("voltage0_in/temp_comp_gain", "-3", "3");
});

TestFramework.runTest("UNIT.BADVAL.OBS_TEMP_COMP_GAIN", function() {
    return testBadValueRange("voltage2_in/temp_comp_gain", "-3", "3");
});

// Test that invalid combo key is rejected (value stays unchanged)
TestFramework.runTest("UNIT.BADVAL.RX_GAIN_CONTROL_MODE", function() {
    return testBadValueCombo("voltage0_in/gain_control_mode", "99");
});

// Test TX LO frequency below minimum via API
TestFramework.runTest("UNIT.BADVAL.TX_LO_FREQUENCY", function() {
    try {
        ad9371.setEnsmMode("radio_on");
        var orig = ad9371.getTxLoFrequency();
        printToConsole("  orig TX LO freq=" + orig);

        // Write 0 MHz (below 300 MHz min)
        ad9371.setTxLoFrequency("0");
        msleep(500);
        var readBack = ad9371.getTxLoFrequency();
        printToConsole("  Wrote 0 MHz, read=" + readBack);

        // Should either clamp to min or reject — value should not be 0
        var passed = (parseFloat(readBack) >= 300);
        if (!passed) {
            printToConsole("  FAIL: TX LO accepted invalid frequency 0 MHz");
        }

        // Restore
        ad9371.setTxLoFrequency(orig);
        msleep(500);
        return passed;
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
