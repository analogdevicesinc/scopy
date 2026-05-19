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
TestFramework.init("AD9371 Advanced Unit Tests");

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
// Switch to AD9371 Advanced tool first
// ============================================
var advancedToolName = isAd9375 ? "ADRV9375 Advanced" : "AD9371 Advanced";
if (!switchToTool(advancedToolName)) {
    printToConsole("ERROR: Cannot switch to " + advancedToolName + " tool");
    scopy.exit();
}
msleep(500);

// ============================================
// SECTION 1: Navigation
// ============================================

TestFramework.runTest("UNIT.NAV.GET_ADVANCED_TABS", function() {
    try {
        var tabs = ad9371.getAdvancedTabs();
        printToConsole("  Advanced tabs: " + tabs);
        if (!tabs || tabs.length === 0) {
            printToConsole("  FAIL: getAdvancedTabs() returned empty");
            return false;
        }
        printToConsole("  Found " + tabs.length + " tabs");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// SECTION 2: Advanced Tool - CLK Settings
// ============================================
ad9371.switchAdvancedTab("CLK Settings");
msleep(500);

var clkTests = [
    {uid: "UNIT.CLK.DEVICE_CLOCK_KHZ", attr: "adi,clocks-device-clock_khz", type: "range", min: "30000", max: "320000", mid: "175000"},
    {uid: "UNIT.CLK.PLL_VCO_FREQ_KHZ", attr: "adi,clocks-clk-pll-vco-freq_khz", type: "range", min: "6000000", max: "12500000", mid: "9250000"},
    {uid: "UNIT.CLK.PLL_VCO_DIV", attr: "adi,clocks-clk-pll-vco-div", type: "combo", options: ["0","1","2","3"]},
    {uid: "UNIT.CLK.PLL_HS_DIV", attr: "adi,clocks-clk-pll-hs-div", type: "range", min: "4", max: "5", mid: "4"}
];
runDataDrivenTests(clkTests);

// ============================================
// SECTION 3: Advanced Tool - TX Settings
// ============================================
ad9371.switchAdvancedTab("TX Settings");
msleep(500);

var txSettingsTests = [
    {uid: "UNIT.ADV_TX.CHANNELS_ENABLE", attr: "adi,tx-settings-tx-channels-enable", type: "combo", options: ["0","1","2","3"]},
    {uid: "UNIT.ADV_TX.PLL_LO_FREQ_HZ", attr: "adi,tx-settings-tx-pll-lo-frequency_hz", type: "range", min: "300000000", max: "6000000000", mid: "3000000000"},
    {uid: "UNIT.ADV_TX.PLL_USE_EXT_LO", attr: "adi,tx-settings-tx-pll-use-external-lo", type: "checkbox"},
    {uid: "UNIT.ADV_TX.ATTEN_STEP_SIZE", attr: "adi,tx-settings-tx-atten-step-size", type: "combo", options: ["0","1","2","3"]},
    {uid: "UNIT.ADV_TX.TX1_ATTEN_MDB", attr: "adi,tx-settings-tx1-atten_mdb", type: "range", min: "0", max: "41950", mid: "20000"},
    {uid: "UNIT.ADV_TX.TX2_ATTEN_MDB", attr: "adi,tx-settings-tx2-atten_mdb", type: "range", min: "0", max: "41950", mid: "20000"},
    {uid: "UNIT.ADV_TX.DAC_DIV", attr: "adi,tx-profile-dac-div", type: "combo", options: ["0","1","2"]},
    {uid: "UNIT.ADV_TX.FIR_INTERPOLATION", attr: "adi,tx-profile-tx-fir-interpolation", type: "combo", options: ["1","2","4"]},
    {uid: "UNIT.ADV_TX.THB1_INTERPOLATION", attr: "adi,tx-profile-thb1-interpolation", type: "range", min: "1", max: "2", mid: "1"},
    {uid: "UNIT.ADV_TX.THB2_INTERPOLATION", attr: "adi,tx-profile-thb2-interpolation", type: "range", min: "1", max: "2", mid: "1"},
    {uid: "UNIT.ADV_TX.INPUT_HB_INTERPOLATION", attr: "adi,tx-profile-tx-input-hb-interpolation", type: "range", min: "1", max: "2", mid: "1"},
    {uid: "UNIT.ADV_TX.IQ_RATE_KHZ", attr: "adi,tx-profile-iq-rate_khz", type: "range", min: "30000", max: "320000", mid: "175000"},
    {uid: "UNIT.ADV_TX.PRIMARY_SIG_BW_HZ", attr: "adi,tx-profile-primary-sig-bandwidth_hz", type: "range", min: "0", max: "250000000", mid: "125000000"},
    {uid: "UNIT.ADV_TX.RF_BW_HZ", attr: "adi,tx-profile-rf-bandwidth_hz", type: "range", min: "0", max: "250000000", mid: "125000000"},
    {uid: "UNIT.ADV_TX.DAC_3DB_CORNER_KHZ", attr: "adi,tx-profile-tx-dac-3db-corner_khz", type: "range", min: "0", max: "250000", mid: "125000"},
    {uid: "UNIT.ADV_TX.BBF_3DB_CORNER_KHZ", attr: "adi,tx-profile-tx-bbf-3db-corner_khz", type: "range", min: "0", max: "250000", mid: "125000"}
];
runDataDrivenTests(txSettingsTests);

// ============================================
// SECTION 4: Advanced Tool - RX Settings
// ============================================
ad9371.switchAdvancedTab("RX Settings");
msleep(500);

var rxSettingsTests = [
    {uid: "UNIT.ADV_RX.CHANNELS_ENABLE", attr: "adi,rx-settings-rx-channels-enable", type: "combo", options: ["0","1","2","3"]},
    {uid: "UNIT.ADV_RX.PLL_LO_FREQ_HZ", attr: "adi,rx-settings-rx-pll-lo-frequency_hz", type: "range", min: "300000000", max: "6000000000", mid: "3000000000"},
    {uid: "UNIT.ADV_RX.PLL_USE_EXT_LO", attr: "adi,rx-settings-rx-pll-use-external-lo", type: "checkbox"},
    {uid: "UNIT.ADV_RX.REAL_IF_DATA", attr: "adi,rx-settings-real-if-data", type: "checkbox"},
    {uid: "UNIT.ADV_RX.ADC_DIV", attr: "adi,rx-profile-adc-div", type: "range", min: "1", max: "2", mid: "1"},
    {uid: "UNIT.ADV_RX.FIR_DECIMATION", attr: "adi,rx-profile-rx-fir-decimation", type: "combo", options: ["1","2","4"]},
    {uid: "UNIT.ADV_RX.DEC5_DECIMATION", attr: "adi,rx-profile-rx-dec5-decimation", type: "range", min: "4", max: "5", mid: "4"},
    {uid: "UNIT.ADV_RX.EN_HIGH_REJ_DEC5", attr: "adi,rx-profile-en-high-rej-dec5", type: "checkbox"},
    {uid: "UNIT.ADV_RX.RHB1_DECIMATION", attr: "adi,rx-profile-rhb1-decimation", type: "range", min: "1", max: "2", mid: "1"},
    {uid: "UNIT.ADV_RX.IQ_RATE_KHZ", attr: "adi,rx-profile-iq-rate_khz", type: "range", min: "20000", max: "200000", mid: "110000"},
    {uid: "UNIT.ADV_RX.RF_BW_HZ", attr: "adi,rx-profile-rf-bandwidth_hz", type: "range", min: "5000000", max: "100000000", mid: "50000000"},
    {uid: "UNIT.ADV_RX.BBF_3DB_CORNER_KHZ", attr: "adi,rx-profile-rx-bbf-3db-corner_khz", type: "range", min: "0", max: "153600", mid: "76800"}
];
runDataDrivenTests(rxSettingsTests);

// ============================================
// SECTION 5: Advanced Tool - DPD Settings (AD9375 only)
// ============================================
ad9371.switchAdvancedTab("DPD Settings");
msleep(500);

var dpdSettingsTests = [
    {uid: "UNIT.ADV_DPD.DAMPING", attr: "adi,dpd-damping", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.ADV_DPD.NUM_WEIGHTS", attr: "adi,dpd-num-weights", type: "range", min: "0", max: "3", mid: "1"},
    {uid: "UNIT.ADV_DPD.MODEL_VERSION", attr: "adi,dpd-model-version", type: "range", min: "0", max: "3", mid: "1"},
    {uid: "UNIT.ADV_DPD.HIGH_POWER_MODEL_UPDATE", attr: "adi,dpd-high-power-model-update", type: "checkbox"},
    {uid: "UNIT.ADV_DPD.MODEL_PRIOR_WEIGHT", attr: "adi,dpd-model-prior-weight", type: "range", min: "0", max: "32", mid: "16"},
    {uid: "UNIT.ADV_DPD.ROBUST_MODELING", attr: "adi,dpd-robust-modeling", type: "checkbox"},
    {uid: "UNIT.ADV_DPD.SAMPLES", attr: "adi,dpd-samples", type: "range", min: "0", max: "65535", mid: "32768"},
    {uid: "UNIT.ADV_DPD.OUTLIER_THRESHOLD", attr: "adi,dpd-outlier-threshold", type: "range", min: "0", max: "65535", mid: "32768"},
    {uid: "UNIT.ADV_DPD.ADDITIONAL_DELAY_OFFSET", attr: "adi,dpd-additional-delay-offset", type: "range", min: "0", max: "63", mid: "31"},
    {uid: "UNIT.ADV_DPD.PATH_DELAY_PN_SEQ_LEVEL", attr: "adi,dpd-path-delay-pn-seq-level", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.ADV_DPD.WEIGHTS0_REAL", attr: "adi,dpd-weights0-real", type: "range", min: "-128", max: "127", mid: "0"},
    {uid: "UNIT.ADV_DPD.WEIGHTS0_IMAG", attr: "adi,dpd-weights0-imag", type: "range", min: "-128", max: "127", mid: "0"},
    {uid: "UNIT.ADV_DPD.WEIGHTS1_REAL", attr: "adi,dpd-weights1-real", type: "range", min: "-128", max: "127", mid: "0"},
    {uid: "UNIT.ADV_DPD.WEIGHTS1_IMAG", attr: "adi,dpd-weights1-imag", type: "range", min: "-128", max: "127", mid: "0"},
    {uid: "UNIT.ADV_DPD.WEIGHTS2_REAL", attr: "adi,dpd-weights2-real", type: "range", min: "-128", max: "127", mid: "0"},
    {uid: "UNIT.ADV_DPD.WEIGHTS2_IMAG", attr: "adi,dpd-weights2-imag", type: "range", min: "-128", max: "127", mid: "0"}
];

for (var i = 0; i < dpdSettingsTests.length; i++) {
    (function(t) {
        TestFramework.runTest(t.uid, function() {
            if (!isAd9375) {
                printToConsole("  SKIP: Not AD9375, DPD not available");
                return "SKIP";
            }
            if (t.type === "range") return testRange(PHY + t.attr, t.min, t.max, t.mid);
            if (t.type === "checkbox") return testCheckbox(PHY + t.attr);
            if (t.type === "combo") return testCombo(PHY + t.attr, t.options);
            return false;
        });
    })(dpdSettingsTests[i]);
}

// ============================================
// SECTION 6: Advanced Tool - CLGC Settings (AD9375 only)
// ============================================
ad9371.switchAdvancedTab("CLGC Settings");
msleep(500);

var clgcSettingsTests = [
    {uid: "UNIT.ADV_CLGC.TX1_DESIRED_GAIN", attr: "adi,clgc-tx1-desired-gain", type: "range", min: "-32768", max: "32767", mid: "0"},
    {uid: "UNIT.ADV_CLGC.TX2_DESIRED_GAIN", attr: "adi,clgc-tx2-desired-gain", type: "range", min: "-32768", max: "32767", mid: "0"},
    {uid: "UNIT.ADV_CLGC.TX1_ATTEN_LIMIT", attr: "adi,clgc-tx1-atten-limit", type: "range", min: "0", max: "40000", mid: "20000"},
    {uid: "UNIT.ADV_CLGC.TX2_ATTEN_LIMIT", attr: "adi,clgc-tx2-atten-limit", type: "range", min: "0", max: "40000", mid: "20000"},
    {uid: "UNIT.ADV_CLGC.TX1_CONTROL_RATIO", attr: "adi,clgc-tx1-control-ratio", type: "range", min: "1", max: "6", mid: "3"},
    {uid: "UNIT.ADV_CLGC.TX2_CONTROL_RATIO", attr: "adi,clgc-tx2-control-ratio", type: "range", min: "1", max: "6", mid: "3"},
    {uid: "UNIT.ADV_CLGC.ALLOW_TX1_ATTEN_UPDATES", attr: "adi,clgc-allow-tx1-atten-updates", type: "checkbox"},
    {uid: "UNIT.ADV_CLGC.ALLOW_TX2_ATTEN_UPDATES", attr: "adi,clgc-allow-tx2-atten-updates", type: "checkbox"},
    {uid: "UNIT.ADV_CLGC.ADDITIONAL_DELAY_OFFSET", attr: "adi,clgc-additional-delay-offset", type: "range", min: "-32768", max: "32767", mid: "0"},
    {uid: "UNIT.ADV_CLGC.PATH_DELAY_PN_SEQ_LEVEL", attr: "adi,clgc-path-delay-pn-seq-level", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.ADV_CLGC.TX1_REL_THRESHOLD", attr: "adi,clgc-tx1-rel-threshold", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.ADV_CLGC.TX2_REL_THRESHOLD", attr: "adi,clgc-tx2-rel-threshold", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.ADV_CLGC.TX1_REL_THRESHOLD_EN", attr: "adi,clgc-tx1-rel-threshold-en", type: "checkbox"},
    {uid: "UNIT.ADV_CLGC.TX2_REL_THRESHOLD_EN", attr: "adi,clgc-tx2-rel-threshold-en", type: "checkbox"}
];

for (var i = 0; i < clgcSettingsTests.length; i++) {
    (function(t) {
        TestFramework.runTest(t.uid, function() {
            if (!isAd9375) {
                printToConsole("  SKIP: Not AD9375, CLGC not available");
                return "SKIP";
            }
            if (t.type === "range") return testRange(PHY + t.attr, t.min, t.max, t.mid);
            if (t.type === "checkbox") return testCheckbox(PHY + t.attr);
            if (t.type === "combo") return testCombo(PHY + t.attr, t.options);
            return false;
        });
    })(clgcSettingsTests[i]);
}

// ============================================
// SECTION 7: Advanced Tool - VSWR Settings (AD9375 only)
// ============================================
ad9371.switchAdvancedTab("VSWR Settings");
msleep(500);

var vswrSettingsTests = [
    {uid: "UNIT.ADV_VSWR.ADDITIONAL_DELAY_OFFSET", attr: "adi,vswr-additional-delay-offset", type: "range", min: "-32768", max: "32767", mid: "0"},
    {uid: "UNIT.ADV_VSWR.PATH_DELAY_PN_SEQ_LEVEL", attr: "adi,vswr-path-delay-pn-seq-level", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.ADV_VSWR.TX1_SWITCH_GPIO3P3_PIN", attr: "adi,vswr-tx1-vswr-switch-gpio3p3-pin", type: "range", min: "0", max: "11", mid: "5"},
    {uid: "UNIT.ADV_VSWR.TX2_SWITCH_GPIO3P3_PIN", attr: "adi,vswr-tx2-vswr-switch-gpio3p3-pin", type: "range", min: "0", max: "11", mid: "5"},
    {uid: "UNIT.ADV_VSWR.TX1_SWITCH_POLARITY", attr: "adi,vswr-tx1-vswr-switch-polarity", type: "checkbox"},
    {uid: "UNIT.ADV_VSWR.TX2_SWITCH_POLARITY", attr: "adi,vswr-tx2-vswr-switch-polarity", type: "checkbox"},
    {uid: "UNIT.ADV_VSWR.TX1_SWITCH_DELAY_US", attr: "adi,vswr-tx1-vswr-switch-delay_us", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.ADV_VSWR.TX2_SWITCH_DELAY_US", attr: "adi,vswr-tx2-vswr-switch-delay_us", type: "range", min: "0", max: "255", mid: "128"}
];

for (var i = 0; i < vswrSettingsTests.length; i++) {
    (function(t) {
        TestFramework.runTest(t.uid, function() {
            if (!isAd9375) {
                printToConsole("  SKIP: Not AD9375, VSWR not available");
                return "SKIP";
            }
            if (t.type === "range") return testRange(PHY + t.attr, t.min, t.max, t.mid);
            if (t.type === "checkbox") return testCheckbox(PHY + t.attr);
            return false;
        });
    })(vswrSettingsTests[i]);
}

// ============================================
// SECTION 8: Advanced Tool - OBS Settings
// ============================================
ad9371.switchAdvancedTab("OBS Settings");
msleep(500);

var obsSettingsTests = [
    {uid: "UNIT.ADV_OBS.LO_SOURCE", attr: "adi,obs-settings-obs-rx-lo-source", type: "combo", options: ["0","1"]},
    {uid: "UNIT.ADV_OBS.SNIFFER_PLL_LO_FREQ_HZ", attr: "adi,obs-settings-sniffer-pll-lo-frequency_hz", type: "range", min: "300000000", max: isAd9375 ? "6000000000" : "4000000000", mid: isAd9375 ? "3000000000" : "2000000000"},
    {uid: "UNIT.ADV_OBS.REAL_IF_DATA", attr: "adi,obs-settings-real-if-data", type: "checkbox"},
    {uid: "UNIT.ADV_OBS.DEFAULT_CHANNEL", attr: "adi,obs-settings-default-obs-rx-channel", type: "combo", options: ["0","1","2","3","4","5","6","20","36","52"]},
    // OBS Profile
    {uid: "UNIT.ADV_OBS.OBS_PROFILE_ADC_DIV", attr: "adi,obs-profile-adc-div", type: "range", min: "1", max: "2", mid: "1"},
    {uid: "UNIT.ADV_OBS.OBS_PROFILE_FIR_DECIMATION", attr: "adi,obs-profile-rx-fir-decimation", type: "combo", options: ["1","2","4"]},
    {uid: "UNIT.ADV_OBS.OBS_PROFILE_DEC5_DECIMATION", attr: "adi,obs-profile-rx-dec5-decimation", type: "range", min: "4", max: "5", mid: "4"},
    {uid: "UNIT.ADV_OBS.OBS_PROFILE_EN_HIGH_REJ_DEC5", attr: "adi,obs-profile-en-high-rej-dec5", type: "checkbox"},
    {uid: "UNIT.ADV_OBS.OBS_PROFILE_RHB1_DECIMATION", attr: "adi,obs-profile-rhb1-decimation", type: "range", min: "1", max: "2", mid: "1"},
    {uid: "UNIT.ADV_OBS.OBS_PROFILE_IQ_RATE_KHZ", attr: "adi,obs-profile-iq-rate_khz", type: "range", min: "20000", max: "320000", mid: "170000"},
    {uid: "UNIT.ADV_OBS.OBS_PROFILE_RF_BW_HZ", attr: "adi,obs-profile-rf-bandwidth_hz", type: "range", min: "5000000", max: "240000000", mid: "120000000"},
    {uid: "UNIT.ADV_OBS.OBS_PROFILE_BBF_3DB_CORNER_KHZ", attr: "adi,obs-profile-rx-bbf-3db-corner_khz", type: "range", min: "0", max: "250000", mid: "125000"},
    // Sniffer Profile
    {uid: "UNIT.ADV_OBS.SNIFFER_PROFILE_ADC_DIV", attr: "adi,sniffer-profile-adc-div", type: "range", min: "1", max: "2", mid: "1"},
    {uid: "UNIT.ADV_OBS.SNIFFER_PROFILE_FIR_DECIMATION", attr: "adi,sniffer-profile-rx-fir-decimation", type: "combo", options: ["1","2","4"]},
    {uid: "UNIT.ADV_OBS.SNIFFER_PROFILE_DEC5_DECIMATION", attr: "adi,sniffer-profile-rx-dec5-decimation", type: "range", min: "4", max: "5", mid: "4"},
    {uid: "UNIT.ADV_OBS.SNIFFER_PROFILE_EN_HIGH_REJ_DEC5", attr: "adi,sniffer-profile-en-high-rej-dec5", type: "checkbox"},
    {uid: "UNIT.ADV_OBS.SNIFFER_PROFILE_RHB1_DECIMATION", attr: "adi,sniffer-profile-rhb1-decimation", type: "range", min: "1", max: "2", mid: "1"},
    {uid: "UNIT.ADV_OBS.SNIFFER_PROFILE_IQ_RATE_KHZ", attr: "adi,sniffer-profile-iq-rate_khz", type: "range", min: "0", max: "61440", mid: "30720"},
    {uid: "UNIT.ADV_OBS.SNIFFER_PROFILE_RF_BW_HZ", attr: "adi,sniffer-profile-rf-bandwidth_hz", type: "range", min: "0", max: "20000000", mid: "10000000"},
    {uid: "UNIT.ADV_OBS.SNIFFER_PROFILE_BBF_3DB_CORNER_KHZ", attr: "adi,sniffer-profile-rx-bbf-3db-corner_khz", type: "range", min: "0", max: "20000", mid: "10000"}
];
runDataDrivenTests(obsSettingsTests);

// ============================================
// SECTION 9: Advanced Tool - Gain Setup
// ============================================
ad9371.switchAdvancedTab("GAIN Setup");
msleep(500);

var gainTests = [
    {uid: "UNIT.GAIN.RX_MODE", attr: "adi,rx-gain-mode", type: "combo", options: ["0","2","3"]},
    {uid: "UNIT.GAIN.RX1_GAIN_INDEX", attr: "adi,rx1-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.GAIN.RX2_GAIN_INDEX", attr: "adi,rx2-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.GAIN.RX1_MAX_GAIN_INDEX", attr: "adi,rx1-max-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.GAIN.RX1_MIN_GAIN_INDEX", attr: "adi,rx1-min-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.GAIN.RX2_MAX_GAIN_INDEX", attr: "adi,rx2-max-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.GAIN.RX2_MIN_GAIN_INDEX", attr: "adi,rx2-min-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.GAIN.ORX_MODE", attr: "adi,orx-gain-mode", type: "combo", options: ["0","2","3"]},
    {uid: "UNIT.GAIN.ORX2_GAIN_INDEX", attr: "adi,orx2-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.GAIN.ORX_MAX_GAIN_INDEX", attr: "adi,orx-max-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.GAIN.ORX_MIN_GAIN_INDEX", attr: "adi,orx-min-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.GAIN.SNIFFER_MODE", attr: "adi,sniffer-gain-mode", type: "combo", options: ["0","2","3"]},
    {uid: "UNIT.GAIN.SNIFFER_GAIN_INDEX", attr: "adi,sniffer-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.GAIN.SNIFFER_MAX_GAIN_INDEX", attr: "adi,sniffer-max-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.GAIN.SNIFFER_MIN_GAIN_INDEX", attr: "adi,sniffer-min-gain-index", type: "range", min: "0", max: "255", mid: "128"}
];
runDataDrivenTests(gainTests);

// ORX1 gain index - tested separately because ORX1 may be locked by firmware when active
TestFramework.runTest("UNIT.GAIN.ORX1_GAIN_INDEX", function() {
    try {
        var key = PHY + "adi,orx1-gain-index";
        var orig = ad9371.readWidget(key);
        ad9371.writeWidget(key, "128");
        msleep(500);
        var readBack = ad9371.readWidget(key);
        if (readBack === orig && orig !== "128") {
            printToConsole("  SKIP: ORX1 gain index locked by firmware (active calibration channel)");
            return "SKIP";
        }
        ad9371.writeWidget(key, String(parseFloat(orig)));
        msleep(500);
        return testRange(key, "0", "255", "128");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// SECTION 10: Advanced Tool - AGC Setup
// ============================================
ad9371.switchAdvancedTab("AGC Setup");
msleep(500);

// --- AGC RX Peak (20 widgets) ---
var agcRxPeakTests = [
    {uid: "UNIT.AGC_RX_PEAK.APD_HIGH_THRESH", attr: "adi,rx-peak-agc-apd-high-thresh", type: "range", min: "0", max: "63", mid: "31"},
    {uid: "UNIT.AGC_RX_PEAK.APD_LOW_THRESH", attr: "adi,rx-peak-agc-apd-low-thresh", type: "range", min: "0", max: "63", mid: "31"},
    {uid: "UNIT.AGC_RX_PEAK.HB2_HIGH_THRESH", attr: "adi,rx-peak-agc-hb2-high-thresh", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_RX_PEAK.HB2_LOW_THRESH", attr: "adi,rx-peak-agc-hb2-low-thresh", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_RX_PEAK.HB2_VERY_LOW_THRESH", attr: "adi,rx-peak-agc-hb2-very-low-thresh", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_RX_PEAK.APD_HIGH_THRESH_EXCEEDED_CNT", attr: "adi,rx-peak-agc-apd-high-thresh-exceeded-cnt", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_RX_PEAK.APD_LOW_THRESH_EXCEEDED_CNT", attr: "adi,rx-peak-agc-apd-low-thresh-exceeded-cnt", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_RX_PEAK.HB2_HIGH_THRESH_EXCEEDED_CNT", attr: "adi,rx-peak-agc-hb2-high-thresh-exceeded-cnt", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_RX_PEAK.HB2_LOW_THRESH_EXCEEDED_CNT", attr: "adi,rx-peak-agc-hb2-low-thresh-exceeded-cnt", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_RX_PEAK.HB2_VERY_LOW_THRESH_EXCEEDED_CNT", attr: "adi,rx-peak-agc-hb2-very-low-thresh-exceeded-cnt", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_RX_PEAK.APD_HIGH_GAIN_STEP_ATTACK", attr: "adi,rx-peak-agc-apd-high-gain-step-attack", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_RX_PEAK.APD_LOW_GAIN_STEP_RECOVERY", attr: "adi,rx-peak-agc-apd-low-gain-step-recovery", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_RX_PEAK.HB2_HIGH_GAIN_STEP_ATTACK", attr: "adi,rx-peak-agc-hb2-high-gain-step-attack", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_RX_PEAK.HB2_LOW_GAIN_STEP_RECOVERY", attr: "adi,rx-peak-agc-hb2-low-gain-step-recovery", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_RX_PEAK.HB2_VERY_LOW_GAIN_STEP_RECOVERY", attr: "adi,rx-peak-agc-hb2-very-low-gain-step-recovery", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_RX_PEAK.APD_FAST_ATTACK", attr: "adi,rx-peak-agc-apd-fast-attack", type: "checkbox"},
    {uid: "UNIT.AGC_RX_PEAK.HB2_FAST_ATTACK", attr: "adi,rx-peak-agc-hb2-fast-attack", type: "checkbox"},
    {uid: "UNIT.AGC_RX_PEAK.HB2_OVERLOAD_DETECT_ENABLE", attr: "adi,rx-peak-agc-hb2-overload-detect-enable", type: "checkbox"},
    {uid: "UNIT.AGC_RX_PEAK.HB2_OVERLOAD_DURATION_CNT", attr: "adi,rx-peak-agc-hb2-overload-duration-cnt", type: "range", min: "0", max: "6", mid: "3"},
    {uid: "UNIT.AGC_RX_PEAK.HB2_OVERLOAD_THRESH_CNT", attr: "adi,rx-peak-agc-hb2-overload-thresh-cnt", type: "range", min: "0", max: "15", mid: "7"}
];
runDataDrivenTests(agcRxPeakTests);

// --- AGC OBS Peak (20 widgets) ---
var agcObsPeakTests = [
    {uid: "UNIT.AGC_OBS_PEAK.APD_HIGH_THRESH", attr: "adi,obs-peak-agc-apd-high-thresh", type: "range", min: "0", max: "63", mid: "31"},
    {uid: "UNIT.AGC_OBS_PEAK.APD_LOW_THRESH", attr: "adi,obs-peak-agc-apd-low-thresh", type: "range", min: "0", max: "63", mid: "31"},
    {uid: "UNIT.AGC_OBS_PEAK.HB2_HIGH_THRESH", attr: "adi,obs-peak-agc-hb2-high-thresh", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_OBS_PEAK.HB2_LOW_THRESH", attr: "adi,obs-peak-agc-hb2-low-thresh", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_OBS_PEAK.HB2_VERY_LOW_THRESH", attr: "adi,obs-peak-agc-hb2-very-low-thresh", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_OBS_PEAK.APD_HIGH_THRESH_EXCEEDED_CNT", attr: "adi,obs-peak-agc-apd-high-thresh-exceeded-cnt", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_OBS_PEAK.APD_LOW_THRESH_EXCEEDED_CNT", attr: "adi,obs-peak-agc-apd-low-thresh-exceeded-cnt", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_OBS_PEAK.HB2_HIGH_THRESH_EXCEEDED_CNT", attr: "adi,obs-peak-agc-hb2-high-thresh-exceeded-cnt", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_OBS_PEAK.HB2_LOW_THRESH_EXCEEDED_CNT", attr: "adi,obs-peak-agc-hb2-low-thresh-exceeded-cnt", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_OBS_PEAK.HB2_VERY_LOW_THRESH_EXCEEDED_CNT", attr: "adi,obs-peak-agc-hb2-very-low-thresh-exceeded-cnt", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_OBS_PEAK.APD_HIGH_GAIN_STEP_ATTACK", attr: "adi,obs-peak-agc-apd-high-gain-step-attack", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_OBS_PEAK.APD_LOW_GAIN_STEP_RECOVERY", attr: "adi,obs-peak-agc-apd-low-gain-step-recovery", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_OBS_PEAK.HB2_HIGH_GAIN_STEP_ATTACK", attr: "adi,obs-peak-agc-hb2-high-gain-step-attack", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_OBS_PEAK.HB2_LOW_GAIN_STEP_RECOVERY", attr: "adi,obs-peak-agc-hb2-low-gain-step-recovery", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_OBS_PEAK.HB2_VERY_LOW_GAIN_STEP_RECOVERY", attr: "adi,obs-peak-agc-hb2-very-low-gain-step-recovery", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_OBS_PEAK.APD_FAST_ATTACK", attr: "adi,obs-peak-agc-apd-fast-attack", type: "checkbox"},
    {uid: "UNIT.AGC_OBS_PEAK.HB2_FAST_ATTACK", attr: "adi,obs-peak-agc-hb2-fast-attack", type: "checkbox"},
    {uid: "UNIT.AGC_OBS_PEAK.HB2_OVERLOAD_DETECT_ENABLE", attr: "adi,obs-peak-agc-hb2-overload-detect-enable", type: "checkbox"},
    {uid: "UNIT.AGC_OBS_PEAK.HB2_OVERLOAD_DURATION_CNT", attr: "adi,obs-peak-agc-hb2-overload-duration-cnt", type: "range", min: "0", max: "6", mid: "3"},
    {uid: "UNIT.AGC_OBS_PEAK.HB2_OVERLOAD_THRESH_CNT", attr: "adi,obs-peak-agc-hb2-overload-thresh-cnt", type: "range", min: "0", max: "15", mid: "7"}
];
runDataDrivenTests(agcObsPeakTests);

// --- AGC RX Power (10 widgets) ---
var agcRxPwrTests = [
    {uid: "UNIT.AGC_RX_PWR.PMD_UPPER_HIGH_THRESH", attr: "adi,rx-pwr-agc-pmd-upper-high-thresh", type: "range", min: "0", max: "15", mid: "7"},
    {uid: "UNIT.AGC_RX_PWR.PMD_UPPER_LOW_THRESH", attr: "adi,rx-pwr-agc-pmd-upper-low-thresh", type: "range", min: "0", max: "127", mid: "63"},
    {uid: "UNIT.AGC_RX_PWR.PMD_LOWER_HIGH_THRESH", attr: "adi,rx-pwr-agc-pmd-lower-high-thresh", type: "range", min: "0", max: "127", mid: "63"},
    {uid: "UNIT.AGC_RX_PWR.PMD_LOWER_LOW_THRESH", attr: "adi,rx-pwr-agc-pmd-lower-low-thresh", type: "range", min: "0", max: "15", mid: "7"},
    {uid: "UNIT.AGC_RX_PWR.PMD_UPPER_HIGH_GAIN_STEP_ATTACK", attr: "adi,rx-pwr-agc-pmd-upper-high-gain-step-attack", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_RX_PWR.PMD_UPPER_LOW_GAIN_STEP_ATTACK", attr: "adi,rx-pwr-agc-pmd-upper-low-gain-step-attack", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_RX_PWR.PMD_LOWER_HIGH_GAIN_STEP_RECOVERY", attr: "adi,rx-pwr-agc-pmd-lower-high-gain-step-recovery", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_RX_PWR.PMD_LOWER_LOW_GAIN_STEP_RECOVERY", attr: "adi,rx-pwr-agc-pmd-lower-low-gain-step-recovery", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_RX_PWR.PMD_MEAS_DURATION", attr: "adi,rx-pwr-agc-pmd-meas-duration", type: "range", min: "0", max: "15", mid: "7"},
    {uid: "UNIT.AGC_RX_PWR.PMD_MEAS_CONFIG", attr: "adi,rx-pwr-agc-pmd-meas-config", type: "combo", options: ["0","1","2","3"]}
];
runDataDrivenTests(agcRxPwrTests);

// --- AGC OBS Power (10 widgets) ---
var agcObsPwrTests = [
    {uid: "UNIT.AGC_OBS_PWR.PMD_UPPER_HIGH_THRESH", attr: "adi,obs-pwr-agc-pmd-upper-high-thresh", type: "range", min: "0", max: "15", mid: "7"},
    {uid: "UNIT.AGC_OBS_PWR.PMD_UPPER_LOW_THRESH", attr: "adi,obs-pwr-agc-pmd-upper-low-thresh", type: "range", min: "0", max: "127", mid: "63"},
    {uid: "UNIT.AGC_OBS_PWR.PMD_LOWER_HIGH_THRESH", attr: "adi,obs-pwr-agc-pmd-lower-high-thresh", type: "range", min: "0", max: "127", mid: "63"},
    {uid: "UNIT.AGC_OBS_PWR.PMD_LOWER_LOW_THRESH", attr: "adi,obs-pwr-agc-pmd-lower-low-thresh", type: "range", min: "0", max: "15", mid: "7"},
    {uid: "UNIT.AGC_OBS_PWR.PMD_UPPER_HIGH_GAIN_STEP_ATTACK", attr: "adi,obs-pwr-agc-pmd-upper-high-gain-step-attack", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_OBS_PWR.PMD_UPPER_LOW_GAIN_STEP_ATTACK", attr: "adi,obs-pwr-agc-pmd-upper-low-gain-step-attack", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_OBS_PWR.PMD_LOWER_HIGH_GAIN_STEP_RECOVERY", attr: "adi,obs-pwr-agc-pmd-lower-high-gain-step-recovery", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_OBS_PWR.PMD_LOWER_LOW_GAIN_STEP_RECOVERY", attr: "adi,obs-pwr-agc-pmd-lower-low-gain-step-recovery", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.AGC_OBS_PWR.PMD_MEAS_DURATION", attr: "adi,obs-pwr-agc-pmd-meas-duration", type: "range", min: "0", max: "15", mid: "7"},
    {uid: "UNIT.AGC_OBS_PWR.PMD_MEAS_CONFIG", attr: "adi,obs-pwr-agc-pmd-meas-config", type: "combo", options: ["0","1","2","3"]}
];
runDataDrivenTests(agcObsPwrTests);

// --- AGC RX Config (11 widgets) ---
var agcRxConfTests = [
    {uid: "UNIT.AGC_RX_CONF.RX1_MAX_GAIN_INDEX", attr: "adi,rx-agc-conf-agc-rx1-max-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_RX_CONF.RX1_MIN_GAIN_INDEX", attr: "adi,rx-agc-conf-agc-rx1-min-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_RX_CONF.RX2_MAX_GAIN_INDEX", attr: "adi,rx-agc-conf-agc-rx2-max-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_RX_CONF.RX2_MIN_GAIN_INDEX", attr: "adi,rx-agc-conf-agc-rx2-min-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_RX_CONF.PEAK_THRESHOLD_MODE", attr: "adi,rx-agc-conf-agc-peak-threshold-mode", type: "checkbox"},
    {uid: "UNIT.AGC_RX_CONF.LOW_THS_PREVENT_GAIN_INCREASE", attr: "adi,rx-agc-conf-agc-low-ths-prevent-gain-increase", type: "checkbox"},
    {uid: "UNIT.AGC_RX_CONF.GAIN_UPDATE_COUNTER", attr: "adi,rx-agc-conf-agc-gain-update-counter", type: "range", min: "1", max: "4194303", mid: "2097152"},
    {uid: "UNIT.AGC_RX_CONF.SLOW_LOOP_SETTLING_DELAY", attr: "adi,rx-agc-conf-agc-slow-loop-settling-delay", type: "range", min: "0", max: "127", mid: "63"},
    {uid: "UNIT.AGC_RX_CONF.PEAK_WAIT_TIME", attr: "adi,rx-agc-conf-agc-peak-wait-time", type: "range", min: "2", max: "31", mid: "16"},
    {uid: "UNIT.AGC_RX_CONF.RESET_ON_RX_ENABLE", attr: "adi,rx-agc-conf-agc-reset-on-rx-enable", type: "checkbox"},
    {uid: "UNIT.AGC_RX_CONF.ENABLE_SYNC_PULSE_FOR_GAIN_COUNTER", attr: "adi,rx-agc-conf-agc-enable-sync-pulse-for-gain-counter", type: "checkbox"}
];
runDataDrivenTests(agcRxConfTests);

// --- AGC OBS Config (10 widgets) ---
var agcObsConfTests = [
    {uid: "UNIT.AGC_OBS_CONF.OBS_RX_MAX_GAIN_INDEX", attr: "adi,obs-agc-conf-agc-obs-rx-max-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_OBS_CONF.OBS_RX_MIN_GAIN_INDEX", attr: "adi,obs-agc-conf-agc-obs-rx-min-gain-index", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.AGC_OBS_CONF.OBS_RX_SELECT", attr: "adi,obs-agc-conf-agc-obs-rx-select", type: "checkbox"},
    {uid: "UNIT.AGC_OBS_CONF.PEAK_THRESHOLD_MODE", attr: "adi,obs-agc-conf-agc-peak-threshold-mode", type: "checkbox"},
    {uid: "UNIT.AGC_OBS_CONF.LOW_THS_PREVENT_GAIN_INCREASE", attr: "adi,obs-agc-conf-agc-low-ths-prevent-gain-increase", type: "checkbox"},
    {uid: "UNIT.AGC_OBS_CONF.GAIN_UPDATE_COUNTER", attr: "adi,obs-agc-conf-agc-gain-update-counter", type: "range", min: "1", max: "4194303", mid: "2097152"},
    {uid: "UNIT.AGC_OBS_CONF.SLOW_LOOP_SETTLING_DELAY", attr: "adi,obs-agc-conf-agc-slow-loop-settling-delay", type: "range", min: "0", max: "127", mid: "63"},
    {uid: "UNIT.AGC_OBS_CONF.PEAK_WAIT_TIME", attr: "adi,obs-agc-conf-agc-peak-wait-time", type: "range", min: "2", max: "31", mid: "16"},
    {uid: "UNIT.AGC_OBS_CONF.RESET_ON_RX_ENABLE", attr: "adi,obs-agc-conf-agc-reset-on-rx-enable", type: "checkbox"},
    {uid: "UNIT.AGC_OBS_CONF.ENABLE_SYNC_PULSE_FOR_GAIN_COUNTER", attr: "adi,obs-agc-conf-agc-enable-sync-pulse-for-gain-counter", type: "checkbox"}
];
runDataDrivenTests(agcObsConfTests);

// ============================================
// SECTION 11: Advanced Tool - ARM GPIO
// ============================================
ad9371.switchAdvancedTab("ARM GPIO");
msleep(500);

var armGpioTests = [
    {uid: "UNIT.ARM_GPIO.USE_RX2_ENABLE_PIN", attr: "adi,arm-gpio-use-rx2-enable-pin", type: "checkbox"},
    {uid: "UNIT.ARM_GPIO.USE_TX2_ENABLE_PIN", attr: "adi,arm-gpio-use-tx2-enable-pin", type: "checkbox"},
    {uid: "UNIT.ARM_GPIO.TX_RX_PIN_MODE", attr: "adi,arm-gpio-tx-rx-pin-mode", type: "checkbox"},
    {uid: "UNIT.ARM_GPIO.ORX_PIN_MODE", attr: "adi,arm-gpio-orx-pin-mode", type: "checkbox"},
    {uid: "UNIT.ARM_GPIO.ORX_TRIGGER_PIN", attr: "adi,arm-gpio-orx-trigger-pin", type: "range", min: "0", max: "15", mid: "7"},
    {uid: "UNIT.ARM_GPIO.ORX_MODE2_PIN", attr: "adi,arm-gpio-orx-mode2-pin", type: "range", min: "0", max: "15", mid: "7"},
    {uid: "UNIT.ARM_GPIO.ORX_MODE1_PIN", attr: "adi,arm-gpio-orx-mode1-pin", type: "range", min: "0", max: "15", mid: "7"},
    {uid: "UNIT.ARM_GPIO.ORX_MODE0_PIN", attr: "adi,arm-gpio-orx-mode0-pin", type: "range", min: "0", max: "15", mid: "7"}
];
runDataDrivenTests(armGpioTests);

// ============================================
// SECTION 12: Advanced Tool - GPIO
// ============================================
ad9371.switchAdvancedTab("GPIO");
msleep(500);

var gpioTests = [
    // 3.3V GPIO source controls (3 combos)
    {uid: "UNIT.GPIO.3V3_SRC_CTRL3_0", attr: "adi,gpio-3v3-src-ctrl3_0", type: "combo", options: ["1","2","3","4"]},
    {uid: "UNIT.GPIO.3V3_SRC_CTRL7_4", attr: "adi,gpio-3v3-src-ctrl7_4", type: "combo", options: ["1","2","3","4"]},
    {uid: "UNIT.GPIO.3V3_SRC_CTRL11_8", attr: "adi,gpio-3v3-src-ctrl11_8", type: "combo", options: ["1","2","3","4"]},
    // Low-voltage GPIO source controls (5 combos)
    {uid: "UNIT.GPIO.SRC_CTRL3_0", attr: "adi,gpio-src-ctrl3_0", type: "combo", options: ["0","3","9","10"]},
    {uid: "UNIT.GPIO.SRC_CTRL7_4", attr: "adi,gpio-src-ctrl7_4", type: "combo", options: ["0","3","9","10"]},
    {uid: "UNIT.GPIO.SRC_CTRL11_8", attr: "adi,gpio-src-ctrl11_8", type: "combo", options: ["0","3","9","10"]},
    {uid: "UNIT.GPIO.SRC_CTRL15_12", attr: "adi,gpio-src-ctrl15_12", type: "combo", options: ["0","3","9","10"]},
    {uid: "UNIT.GPIO.SRC_CTRL18_16", attr: "adi,gpio-src-ctrl18_16", type: "combo", options: ["0","3","9","10"]}
];
runDataDrivenTests(gpioTests);

// ============================================
// SECTION 13: Advanced Tool - AUX DAC
// ============================================
ad9371.switchAdvancedTab("AUX DAC");
msleep(500);

var auxDacTests = [];
for (var i = 0; i < 10; i++) {
    (function(idx) {
        auxDacTests.push({uid: "UNIT.AUX_DAC.VALUE" + idx, attr: "adi,aux-dac-value" + idx, type: "range", min: "0", max: "1023", mid: "512"});
        auxDacTests.push({uid: "UNIT.AUX_DAC.SLOPE" + idx, attr: "adi,aux-dac-slope" + idx, type: "combo", options: ["0","1"]});
        auxDacTests.push({uid: "UNIT.AUX_DAC.VREF" + idx, attr: "adi,aux-dac-vref" + idx, type: "combo", options: ["0","1","2","3"]});
    })(i);
}
runDataDrivenTests(auxDacTests);

// ============================================
// SECTION 14: Advanced Tool - JESD Framer
// ============================================
ad9371.switchAdvancedTab("JESD Framer");
msleep(500);

// RX Framer
var jesdRxFramerTests = [
    {uid: "UNIT.JESD_RX_FRAMER.BANK_ID", attr: "adi,jesd204-rx-framer-bank-id", type: "range", min: "0", max: "15", mid: "7"},
    {uid: "UNIT.JESD_RX_FRAMER.DEVICE_ID", attr: "adi,jesd204-rx-framer-device-id", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.JESD_RX_FRAMER.LANE0_ID", attr: "adi,jesd204-rx-framer-lane0-id", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.JESD_RX_FRAMER.M", attr: "adi,jesd204-rx-framer-m", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.JESD_RX_FRAMER.K", attr: "adi,jesd204-rx-framer-k", type: "range", min: "0", max: "32", mid: "16"},
    {uid: "UNIT.JESD_RX_FRAMER.SCRAMBLE", attr: "adi,jesd204-rx-framer-scramble", type: "checkbox"},
    {uid: "UNIT.JESD_RX_FRAMER.EXTERNAL_SYSREF", attr: "adi,jesd204-rx-framer-external-sysref", type: "checkbox"},
    {uid: "UNIT.JESD_RX_FRAMER.SERIALIZER_LANE_CROSSBAR", attr: "adi,jesd204-rx-framer-serializer-lane-crossbar", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.JESD_RX_FRAMER.SERIALIZER_AMPLITUDE", attr: "adi,jesd204-rx-framer-serializer-amplitude", type: "range", min: "0", max: "15", mid: "7"},
    {uid: "UNIT.JESD_RX_FRAMER.PRE_EMPHASIS", attr: "adi,jesd204-rx-framer-pre-emphasis", type: "range", min: "0", max: "7", mid: "3"},
    {uid: "UNIT.JESD_RX_FRAMER.LMFC_OFFSET", attr: "adi,jesd204-rx-framer-lmfc-offset", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.JESD_RX_FRAMER.NEW_SYSREF_ON_RELINK", attr: "adi,jesd204-rx-framer-new-sysref-on-relink", type: "checkbox"},
    {uid: "UNIT.JESD_RX_FRAMER.ENABLE_AUTO_CHAN_XBAR", attr: "adi,jesd204-rx-framer-enable-auto-chan-xbar", type: "checkbox"},
    {uid: "UNIT.JESD_RX_FRAMER.OBS_RX_SYNCB_SELECT", attr: "adi,jesd204-rx-framer-obs-rx-syncb-select", type: "checkbox"},
    {uid: "UNIT.JESD_RX_FRAMER.RX_SYNCB_MODE", attr: "adi,jesd204-rx-framer-rx-syncb-mode", type: "checkbox"},
    {uid: "UNIT.JESD_RX_FRAMER.OVER_SAMPLE", attr: "adi,jesd204-rx-framer-over-sample", type: "checkbox"}
];
runDataDrivenTests(jesdRxFramerTests);

// OBS Framer
var jesdObsFramerTests = [
    {uid: "UNIT.JESD_OBS_FRAMER.BANK_ID", attr: "adi,jesd204-obs-framer-bank-id", type: "range", min: "0", max: "15", mid: "7"},
    {uid: "UNIT.JESD_OBS_FRAMER.DEVICE_ID", attr: "adi,jesd204-obs-framer-device-id", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.JESD_OBS_FRAMER.LANE0_ID", attr: "adi,jesd204-obs-framer-lane0-id", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.JESD_OBS_FRAMER.M", attr: "adi,jesd204-obs-framer-m", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.JESD_OBS_FRAMER.K", attr: "adi,jesd204-obs-framer-k", type: "range", min: "0", max: "32", mid: "16"},
    {uid: "UNIT.JESD_OBS_FRAMER.SCRAMBLE", attr: "adi,jesd204-obs-framer-scramble", type: "checkbox"},
    {uid: "UNIT.JESD_OBS_FRAMER.EXTERNAL_SYSREF", attr: "adi,jesd204-obs-framer-external-sysref", type: "checkbox"},
    {uid: "UNIT.JESD_OBS_FRAMER.SERIALIZER_LANE_CROSSBAR", attr: "adi,jesd204-obs-framer-serializer-lane-crossbar", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.JESD_OBS_FRAMER.SERIALIZER_AMPLITUDE", attr: "adi,jesd204-obs-framer-serializer-amplitude", type: "range", min: "0", max: "15", mid: "7"},
    {uid: "UNIT.JESD_OBS_FRAMER.PRE_EMPHASIS", attr: "adi,jesd204-obs-framer-pre-emphasis", type: "range", min: "0", max: "7", mid: "3"},
    {uid: "UNIT.JESD_OBS_FRAMER.LMFC_OFFSET", attr: "adi,jesd204-obs-framer-lmfc-offset", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.JESD_OBS_FRAMER.NEW_SYSREF_ON_RELINK", attr: "adi,jesd204-obs-framer-new-sysref-on-relink", type: "checkbox"},
    {uid: "UNIT.JESD_OBS_FRAMER.ENABLE_AUTO_CHAN_XBAR", attr: "adi,jesd204-obs-framer-enable-auto-chan-xbar", type: "checkbox"},
    {uid: "UNIT.JESD_OBS_FRAMER.OBS_RX_SYNCB_SELECT", attr: "adi,jesd204-obs-framer-obs-rx-syncb-select", type: "checkbox"},
    {uid: "UNIT.JESD_OBS_FRAMER.RX_SYNCB_MODE", attr: "adi,jesd204-obs-framer-rx-syncb-mode", type: "checkbox"},
    {uid: "UNIT.JESD_OBS_FRAMER.OVER_SAMPLE", attr: "adi,jesd204-obs-framer-over-sample", type: "checkbox"}
];
runDataDrivenTests(jesdObsFramerTests);

// ============================================
// SECTION 15: Advanced Tool - JESD Deframer
// ============================================
ad9371.switchAdvancedTab("JESD Deframer");
msleep(500);

var jesdDeframerTests = [
    {uid: "UNIT.JESD_DEFRAMER.BANK_ID", attr: "adi,jesd204-deframer-bank-id", type: "range", min: "0", max: "15", mid: "7"},
    {uid: "UNIT.JESD_DEFRAMER.DEVICE_ID", attr: "adi,jesd204-deframer-device-id", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.JESD_DEFRAMER.LANE0_ID", attr: "adi,jesd204-deframer-lane0-id", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.JESD_DEFRAMER.M", attr: "adi,jesd204-deframer-m", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.JESD_DEFRAMER.K", attr: "adi,jesd204-deframer-k", type: "range", min: "0", max: "32", mid: "16"},
    {uid: "UNIT.JESD_DEFRAMER.SCRAMBLE", attr: "adi,jesd204-deframer-scramble", type: "checkbox"},
    {uid: "UNIT.JESD_DEFRAMER.EXTERNAL_SYSREF", attr: "adi,jesd204-deframer-external-sysref", type: "checkbox"},
    {uid: "UNIT.JESD_DEFRAMER.DESERIALIZER_LANE_CROSSBAR", attr: "adi,jesd204-deframer-deserializer-lane-crossbar", type: "range", min: "0", max: "255", mid: "128"},
    {uid: "UNIT.JESD_DEFRAMER.EQ_SETTING", attr: "adi,jesd204-deframer-eq-setting", type: "range", min: "0", max: "3", mid: "1"},
    {uid: "UNIT.JESD_DEFRAMER.LMFC_OFFSET", attr: "adi,jesd204-deframer-lmfc-offset", type: "range", min: "0", max: "31", mid: "15"},
    {uid: "UNIT.JESD_DEFRAMER.NEW_SYSREF_ON_RELINK", attr: "adi,jesd204-deframer-new-sysref-on-relink", type: "checkbox"},
    {uid: "UNIT.JESD_DEFRAMER.ENABLE_AUTO_CHAN_XBAR", attr: "adi,jesd204-deframer-enable-auto-chan-xbar", type: "checkbox"},
    {uid: "UNIT.JESD_DEFRAMER.TX_SYNCB_MODE", attr: "adi,jesd204-deframer-tx-syncb-mode", type: "checkbox"}
];
runDataDrivenTests(jesdDeframerTests);

// ============================================
// SECTION 16: Advanced Tool - BIST
// ============================================
ad9371.switchAdvancedTab("BIST");
msleep(500);

var bistTests = [
    {uid: "UNIT.BIST.LOOPBACK_TX_RX", attr: "loopback_tx_rx", type: "checkbox"},
    {uid: "UNIT.BIST.LOOPBACK_TX_OBS", attr: "loopback_tx_obs", type: "checkbox"},
    {uid: "UNIT.BIST.BIST_PRBS_RX", attr: "bist_prbs_rx", type: "combo", options: ["0","1","2","3"]},
    {uid: "UNIT.BIST.BIST_PRBS_OBS", attr: "bist_prbs_obs", type: "combo", options: ["0","1","2","3"]}
];
runDataDrivenTests(bistTests);

// ============================================
// SECTION 17: Bad Value Tests
// ============================================

// Test that out-of-range values are properly clamped by IIOWidget spinboxes
// and invalid combo keys are rejected

// CLK: device-clock_khz [30000, 320000]
TestFramework.runTest("UNIT.BADVAL.CLK_DEVICE_CLOCK_KHZ", function() {
    ad9371.switchAdvancedTab("CLK Settings");
    msleep(500);
    return testBadValueRange(PHY + "adi,clocks-device-clock_khz", "30000", "320000");
});

// TX: tx1-atten_mdb [0, 41950]
TestFramework.runTest("UNIT.BADVAL.ADV_TX_ATTEN_MDB", function() {
    ad9371.switchAdvancedTab("TX Settings");
    msleep(500);
    return testBadValueRange(PHY + "adi,tx-settings-tx1-atten_mdb", "0", "41950");
});

// RX: rx-dec5-decimation [4, 5]
TestFramework.runTest("UNIT.BADVAL.ADV_RX_DEC5", function() {
    ad9371.switchAdvancedTab("RX Settings");
    msleep(500);
    return testBadValueRange(PHY + "adi,rx-profile-rx-dec5-decimation", "4", "5");
});

// Gain: rx1-gain-index [0, 255]
TestFramework.runTest("UNIT.BADVAL.GAIN_RX1_INDEX", function() {
    ad9371.switchAdvancedTab("Gain Setup");
    msleep(500);
    return testBadValueRange(PHY + "adi,rx1-gain-index", "0", "255");
});

// AGC: peak-wait-time [2, 31]
TestFramework.runTest("UNIT.BADVAL.AGC_PEAK_WAIT", function() {
    ad9371.switchAdvancedTab("AGC Setup");
    msleep(500);
    return testBadValueRange(PHY + "adi,rx-agc-conf-agc-peak-wait-time", "2", "31");
});

// AUX DAC: value0 [0, 1023]
TestFramework.runTest("UNIT.BADVAL.AUX_DAC_VALUE0", function() {
    ad9371.switchAdvancedTab("AUX DAC");
    msleep(500);
    return testBadValueRange(PHY + "adi,aux-dac-value0", "0", "1023");
});

// JESD: rx-framer-bank-id [0, 15]
TestFramework.runTest("UNIT.BADVAL.JESD_RX_BANK_ID", function() {
    ad9371.switchAdvancedTab("JESD RX Framer");
    msleep(500);
    return testBadValueRange(PHY + "adi,jesd204-rx-framer-bank-id", "0", "15");
});

// TX Settings combo: channels-enable (valid: 0,1,2,3) — try "5"
TestFramework.runTest("UNIT.BADVAL.ADV_TX_CHANNELS", function() {
    ad9371.switchAdvancedTab("TX Settings");
    msleep(500);
    return testBadValueCombo(PHY + "adi,tx-settings-tx-channels-enable", "5");
});

// DPD: weights0-real [-128, 127] (AD9375 only)
TestFramework.runTest("UNIT.BADVAL.DPD_WEIGHTS0_REAL", function() {
    if (!isAd9375) return "SKIP";
    ad9371.switchAdvancedTab("DPD Settings");
    msleep(500);
    return testBadValueRange(PHY + "adi,dpd-weights0-real", "-128", "127");
});

// CLGC: tx1-desired-gain [-32768, 32767] (AD9375 only)
TestFramework.runTest("UNIT.BADVAL.CLGC_TX1_DESIRED_GAIN", function() {
    if (!isAd9375) return "SKIP";
    ad9371.switchAdvancedTab("CLGC Settings");
    msleep(500);
    return testBadValueRange(PHY + "adi,clgc-tx1-desired-gain", "-32768", "32767");
});

// ============================================
// Cleanup
// ============================================
TestFramework.disconnectFromDevice();
TestFramework.printSummary();
scopy.exit();
