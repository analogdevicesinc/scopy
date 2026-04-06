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

// AD9371 Advanced Documentation Test Automation
// Automates tests from: docs/tests/plugins/ad9371/ad9371_advanced_tests.rst
//
// Category B tests (visual) not included:
//   TST.AD9371.ADVANCED_REFRESH_BUTTON — requires visual confirmation of animation
//
// "VISIBLE" section tests are verified functionally: switchAdvancedTab() + readWidget().
// DPD/CLGC/VSWR tests are skipped (return "SKIP") when not on AD9375 hardware.

evaluateFile("js/testAutomations/common/testFramework.js");

TestFramework.init("AD9371 Advanced Documentation Tests");

if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

if (!switchToTool("AD9371 Advanced")) {
    printToConsole("ERROR: Cannot switch to AD9371 Advanced tool");
    scopy.exit();
}

var PHY = "ad9371-phy/";

// Helper: read a widget and verify non-empty
function verifyReadable(key) {
    var val = ad9371.readWidget(key);
    if (!val && val !== "0") {
        printToConsole("  FAIL: readWidget('" + key + "') returned empty");
        return null;
    }
    printToConsole("  Read " + key + " = " + val);
    return val;
}

// ============================================
// Test 1: Advanced Plugin Loads
// UID: TST.AD9371.ADVANCED_PLUGIN_LOADS
// ============================================
TestFramework.runTest("TST.AD9371.ADVANCED_PLUGIN_LOADS", function() {
    try {
        var tools = ad9371.getTools();
        printToConsole("  Tools found: " + tools);
        if (!tools || tools.length === 0) {
            printToConsole("  FAIL: getTools() returned empty");
            return false;
        }
        var hasAdvanced = false;
        for (var i = 0; i < tools.length; i++) {
            if (tools[i] === "AD9371 Advanced") hasAdvanced = true;
        }
        if (!hasAdvanced) {
            printToConsole("  FAIL: AD9371 Advanced tool not found");
            return false;
        }

        var tabs = ad9371.getAdvancedTabs();
        printToConsole("  Advanced tabs: " + tabs);
        if (!tabs || tabs.length === 0) {
            printToConsole("  FAIL: getAdvancedTabs() returned empty");
            return false;
        }
        printToConsole("  AD9371 Advanced plugin accessible with " + tabs.length + " tabs");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2: Clock Settings Section Visible
// UID: TST.AD9371.ADVANCED_CLK_SETTINGS_VISIBLE
// ============================================
ad9371.switchAdvancedTab("CLK Settings");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_CLK_SETTINGS_VISIBLE", function() {
    try {
        if (verifyReadable(PHY + "adi,clocks-device-clock_khz") === null) return false;
        if (verifyReadable(PHY + "adi,clocks-clk-pll-vco-freq_khz") === null) return false;
        printToConsole("  Clock Settings panel accessible and widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3: Calibrations Section Visible
// UID: TST.AD9371.ADVANCED_CALIBRATIONS_VISIBLE
// ============================================
ad9371.switchAdvancedTab("Calibrations");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_CALIBRATIONS_VISIBLE", function() {
    try {
        var txQec = ad9371.getCalibrateTxQecEn();
        if (!txQec && txQec !== "0") {
            printToConsole("  FAIL: getCalibrateTxQecEn() returned empty");
            return false;
        }
        printToConsole("  TX QEC Cal: " + txQec);

        var txLol = ad9371.getCalibrateTxLolEn();
        if (!txLol && txLol !== "0") {
            printToConsole("  FAIL: getCalibrateTxLolEn() returned empty");
            return false;
        }
        printToConsole("  TX LOL Cal: " + txLol);

        printToConsole("  Calibrations panel accessible and widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: Advanced Calibration Mask Write
// UID: TST.AD9371.ADVANCED_CALIBRATIONS_MASK_WRITE
// ============================================
TestFramework.runTest("TST.AD9371.ADVANCED_CALIBRATIONS_MASK_WRITE", function() {
    var origVal = null;
    try {
        origVal = ad9371.getCalibrateTxQecEn();
        if (!origVal && origVal !== "0") {
            printToConsole("  FAIL: Cannot read calibrate_tx_qec_en");
            return false;
        }
        printToConsole("  Original TX QEC Cal: " + origVal);

        var newVal = (origVal === "1") ? "0" : "1";
        printToConsole("  Writing TX QEC Cal: " + newVal);
        ad9371.setCalibrateTxQecEn(newVal);
        msleep(500);

        var readBack = ad9371.getCalibrateTxQecEn();
        printToConsole("  Readback: " + readBack);

        if (readBack !== newVal) {
            ad9371.setCalibrateTxQecEn(origVal);
            msleep(500);
            printToConsole("  FAIL: Expected " + newVal + ", got " + readBack);
            return false;
        }

        ad9371.refresh();
        msleep(500);

        var afterRefresh = ad9371.getCalibrateTxQecEn();
        printToConsole("  After refresh: " + afterRefresh);

        // Restore
        ad9371.setCalibrateTxQecEn(origVal);
        msleep(500);

        if (afterRefresh !== newVal) {
            printToConsole("  FAIL: Value not persisted after refresh. Expected " + newVal + ", got " + afterRefresh);
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (origVal !== null) {
            try { ad9371.setCalibrateTxQecEn(origVal); msleep(500); } catch (e2) {}
        }
        return false;
    }
});

// ============================================
// Test 5: TX Settings Section Visible
// UID: TST.AD9371.ADVANCED_TX_SETTINGS_VISIBLE
// ============================================
ad9371.switchAdvancedTab("TX Settings");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_TX_SETTINGS_VISIBLE", function() {
    try {
        if (verifyReadable(PHY + "adi,tx-settings-tx-channels-enable") === null) return false;
        if (verifyReadable(PHY + "adi,tx-profile-rf-bandwidth_hz") === null) return false;
        printToConsole("  TX Settings panel accessible and widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 6: RX Settings Section Visible
// UID: TST.AD9371.ADVANCED_RX_SETTINGS_VISIBLE
// ============================================
ad9371.switchAdvancedTab("RX Settings");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_RX_SETTINGS_VISIBLE", function() {
    try {
        if (verifyReadable(PHY + "adi,rx-settings-rx-channels-enable") === null) return false;
        if (verifyReadable(PHY + "adi,rx-profile-rf-bandwidth_hz") === null) return false;
        printToConsole("  RX Settings panel accessible and widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 7: Observation Settings Section Visible
// UID: TST.AD9371.ADVANCED_OBS_SETTINGS_VISIBLE
// ============================================
ad9371.switchAdvancedTab("OBS Settings");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_OBS_SETTINGS_VISIBLE", function() {
    try {
        if (verifyReadable(PHY + "adi,obs-settings-obs-rx-lo-source") === null) return false;
        if (verifyReadable(PHY + "adi,obs-profile-rf-bandwidth_hz") === null) return false;
        printToConsole("  OBS Settings panel accessible and widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 8: Gain Setup Section Visible
// UID: TST.AD9371.ADVANCED_GAIN_SETUP_VISIBLE
// ============================================
ad9371.switchAdvancedTab("GAIN Setup");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_GAIN_SETUP_VISIBLE", function() {
    try {
        if (verifyReadable(PHY + "adi,rx-gain-mode") === null) return false;
        if (verifyReadable(PHY + "adi,rx1-gain-index") === null) return false;
        printToConsole("  Gain Setup panel accessible and widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 9: AGC Setup Section Visible
// UID: TST.AD9371.ADVANCED_AGC_SETUP_VISIBLE
// ============================================
ad9371.switchAdvancedTab("AGC Setup");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_AGC_SETUP_VISIBLE", function() {
    try {
        if (verifyReadable(PHY + "adi,rx-peak-agc-apd-high-thresh") === null) return false;
        if (verifyReadable(PHY + "adi,rx-agc-conf-agc-rx1-max-gain-index") === null) return false;
        printToConsole("  AGC Setup panel accessible and widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 10: ARM GPIO Section Visible
// UID: TST.AD9371.ADVANCED_ARM_GPIO_VISIBLE
// ============================================
ad9371.switchAdvancedTab("ARM GPIO");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_ARM_GPIO_VISIBLE", function() {
    try {
        if (verifyReadable(PHY + "adi,arm-gpio-use-rx2-enable-pin") === null) return false;
        if (verifyReadable(PHY + "adi,arm-gpio-orx-trigger-pin") === null) return false;
        printToConsole("  ARM GPIO panel accessible and widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 11: GPIO Section Visible
// UID: TST.AD9371.ADVANCED_GPIO_VISIBLE
// ============================================
ad9371.switchAdvancedTab("GPIO");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_GPIO_VISIBLE", function() {
    try {
        if (verifyReadable(PHY + "adi,gpio-3v3-src-ctrl3_0") === null) return false;
        if (verifyReadable(PHY + "adi,gpio-src-ctrl3_0") === null) return false;
        printToConsole("  GPIO panel accessible and widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 12: AUX DAC Section Visible
// UID: TST.AD9371.ADVANCED_AUX_DAC_VISIBLE
// ============================================
ad9371.switchAdvancedTab("AUX DAC");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_AUX_DAC_VISIBLE", function() {
    try {
        if (verifyReadable(PHY + "adi,aux-dac-value0") === null) return false;
        if (verifyReadable(PHY + "adi,aux-dac-slope0") === null) return false;
        printToConsole("  AUX DAC panel accessible and widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 13: JESD Framer Section Visible
// UID: TST.AD9371.ADVANCED_JESD_FRAMER_VISIBLE
// ============================================
ad9371.switchAdvancedTab("JESD Framer");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_JESD_FRAMER_VISIBLE", function() {
    try {
        if (verifyReadable(PHY + "adi,jesd204-rx-framer-bank-id") === null) return false;
        if (verifyReadable(PHY + "adi,jesd204-rx-framer-m") === null) return false;
        printToConsole("  JESD Framer panel accessible and widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 14: JESD Deframer Section Visible
// UID: TST.AD9371.ADVANCED_JESD_DEFRAMER_VISIBLE
// ============================================
ad9371.switchAdvancedTab("JESD Deframer");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_JESD_DEFRAMER_VISIBLE", function() {
    try {
        if (verifyReadable(PHY + "adi,jesd204-deframer-bank-id") === null) return false;
        if (verifyReadable(PHY + "adi,jesd204-deframer-m") === null) return false;
        printToConsole("  JESD Deframer panel accessible and widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 15: BIST Section Visible
// UID: TST.AD9371.ADVANCED_BIST_VISIBLE
// ============================================
ad9371.switchAdvancedTab("BIST");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_BIST_VISIBLE", function() {
    try {
        if (verifyReadable(PHY + "loopback_tx_rx") === null) return false;
        if (verifyReadable(PHY + "bist_prbs_rx") === null) return false;
        printToConsole("  BIST panel accessible and widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 16: DPD Settings Section Visible (AD9375 only)
// UID: TST.AD9371.ADVANCED_DPD_SETTINGS_VISIBLE
// ============================================
ad9371.switchAdvancedTab("DPD Settings");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_DPD_SETTINGS_VISIBLE", function() {
    try {
        var modelVer = ad9371.readWidget(PHY + "adi,dpd-model-version");
        if (!modelVer && modelVer !== "0") {
            printToConsole("  DPD not available (not AD9375), skipping");
            return "SKIP";
        }
        printToConsole("  DPD model version: " + modelVer);
        if (verifyReadable(PHY + "adi,dpd-num-weights") === null) return false;
        printToConsole("  DPD Settings panel accessible and widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 17: CLGC Settings Section Visible (AD9375 only)
// UID: TST.AD9371.ADVANCED_CLGC_SETTINGS_VISIBLE
// ============================================
ad9371.switchAdvancedTab("CLGC Settings");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_CLGC_SETTINGS_VISIBLE", function() {
    try {
        var desiredGain = ad9371.readWidget(PHY + "adi,clgc-tx1-desired-gain");
        if (!desiredGain && desiredGain !== "0") {
            printToConsole("  CLGC not available (not AD9375), skipping");
            return "SKIP";
        }
        printToConsole("  CLGC tx1 desired gain: " + desiredGain);
        if (verifyReadable(PHY + "adi,clgc-tx1-control-ratio") === null) return false;
        printToConsole("  CLGC Settings panel accessible and widgets readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 18: VSWR Settings Section Visible (AD9375 only)
// UID: TST.AD9371.ADVANCED_VSWR_SETTINGS_VISIBLE
// ============================================
ad9371.switchAdvancedTab("VSWR Settings");
msleep(500);
TestFramework.runTest("TST.AD9371.ADVANCED_VSWR_SETTINGS_VISIBLE", function() {
    try {
        var delayOffset = ad9371.readWidget(PHY + "adi,vswr-additional-delay-offset");
        if (!delayOffset && delayOffset !== "0") {
            printToConsole("  VSWR not available (not AD9375), skipping");
            return "SKIP";
        }
        printToConsole("  VSWR additional delay offset: " + delayOffset);
        if (verifyReadable(PHY + "adi,vswr-tx1-vswr-switch-delay_us") === null) return false;
        printToConsole("  VSWR Settings panel accessible and widgets readable");
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
