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

// AD9371 Advanced Tool Test Automation
// Automates tests from: docs/tests/plugins/ad9371/ad9371_advanced_api_tests.rst
//
// Tests all 16 advanced sections using the generic widget access API:
//   ad9371.readWidget(key), ad9371.writeWidget(key, value)
//
// Widget keys for debug attributes follow: "ad9371-phy/<attr_name>"

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("AD9371 Advanced API Documentation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to AD9371 Advanced tool for visual tracking
if (!switchToTool("AD9371 Advanced")) {
    printToConsole("ERROR: Cannot switch to AD9371 Advanced tool");
    scopy.exit();
}

// --- Helper Functions ---

// Prefix for all debug attribute keys
var PHY = "ad9371-phy/";

// Read a widget key and verify non-empty
function verifyReadable(key) {
    var val = ad9371.readWidget(key);
    if (!val && val !== "0") {
        printToConsole("  FAIL: readWidget('" + key + "') returned empty");
        return null;
    }
    printToConsole("  Read " + key + " = " + val);
    return val;
}

// Write a value, readback, verify match, restore original
function testWriteReadback(key, testValue) {
    var orig = ad9371.readWidget(key);
    if (!orig && orig !== "0") {
        printToConsole("  FAIL: Cannot read original value for " + key);
        return false;
    }
    printToConsole("  Original " + key + " = " + orig);
    ad9371.writeWidget(key, testValue);
    msleep(500);
    var readBack = ad9371.readWidget(key);
    printToConsole("  Write " + testValue + ", readback = " + readBack);

    // Restore before checking result
    ad9371.writeWidget(key, orig);
    msleep(500);

    if (readBack !== testValue) {
        printToConsole("  FAIL: Expected " + testValue + ", got " + readBack);
        return false;
    }
    return true;
}

// Verify a checkbox value is "0" or "1"
function verifyCheckbox(key) {
    var val = verifyReadable(key);
    if (val === null) return false;
    if (val !== "0" && val !== "1") {
        printToConsole("  FAIL: Checkbox " + key + " = " + val + ", expected 0 or 1");
        return false;
    }
    return true;
}

// ============================================
// Test 1: CLK Settings
// UID: TST.AD9371.ADV_CLK_SETTINGS
// ============================================
ad9371.switchAdvancedTab("CLK Settings");
msleep(500);
TestFramework.runTest("TST.AD9371.ADV_CLK_SETTINGS", function() {
    try {
        // Read device clock
        if (verifyReadable(PHY + "adi,clocks-device-clock_khz") === null) return false;

        // Read VCO freq
        if (verifyReadable(PHY + "adi,clocks-clk-pll-vco-freq_khz") === null) return false;

        // Read combo (raw numeric value)
        if (verifyReadable(PHY + "adi,clocks-clk-pll-vco-div") === null) return false;

        // Write-readback CLK PLL HS DIV (range [4, 5])
        if (!testWriteReadback(PHY + "adi,clocks-clk-pll-hs-div", "5")) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2: TX Settings
// UID: TST.AD9371.ADV_TX_SETTINGS
// ============================================
ad9371.switchAdvancedTab("TX Settings");
msleep(500);
TestFramework.runTest("TST.AD9371.ADV_TX_SETTINGS", function() {
    try {
        // Read TX channels enable combo
        if (verifyReadable(PHY + "adi,tx-settings-tx-channels-enable") === null) return false;

        // Verify external LO checkbox
        if (!verifyCheckbox(PHY + "adi,tx-settings-tx-pll-use-external-lo")) return false;

        // Write-readback TX1 attenuation (range [0, 41950])
        if (!testWriteReadback(PHY + "adi,tx-settings-tx1-atten_mdb", "1000")) return false;

        // Read TX profile attrs
        if (verifyReadable(PHY + "adi,tx-profile-dac-div") === null) return false;
        if (verifyReadable(PHY + "adi,tx-profile-iq-rate_khz") === null) return false;
        if (verifyReadable(PHY + "adi,tx-profile-rf-bandwidth_hz") === null) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3: RX Settings
// UID: TST.AD9371.ADV_RX_SETTINGS
// ============================================
ad9371.switchAdvancedTab("RX Settings");
msleep(500);
TestFramework.runTest("TST.AD9371.ADV_RX_SETTINGS", function() {
    try {
        // Read RX channels enable combo
        if (verifyReadable(PHY + "adi,rx-settings-rx-channels-enable") === null) return false;

        // Verify checkboxes
        if (!verifyCheckbox(PHY + "adi,rx-settings-rx-pll-use-external-lo")) return false;
        if (!verifyCheckbox(PHY + "adi,rx-settings-real-if-data")) return false;

        // Read RX profile attrs
        if (verifyReadable(PHY + "adi,rx-profile-adc-div") === null) return false;
        if (verifyReadable(PHY + "adi,rx-profile-iq-rate_khz") === null) return false;
        if (verifyReadable(PHY + "adi,rx-profile-rf-bandwidth_hz") === null) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: DPD Settings (AD9375 only)
// UID: TST.AD9371.ADV_DPD_SETTINGS
// ============================================
ad9371.switchAdvancedTab("DPD Settings");
msleep(500);
TestFramework.runTest("TST.AD9371.ADV_DPD_SETTINGS", function() {
    try {
        var modelVer = ad9371.readWidget(PHY + "adi,dpd-model-version");
        if (!modelVer && modelVer !== "0") {
            printToConsole("  DPD not available (not AD9375), skipping");
            return "SKIP";
        }

        // Read DPD attrs
        if (verifyReadable(PHY + "adi,dpd-num-weights") === null) return false;
        if (verifyReadable(PHY + "adi,dpd-samples") === null) return false;

        // Verify checkbox
        if (!verifyCheckbox(PHY + "adi,dpd-high-power-model-update")) return false;
        if (!verifyCheckbox(PHY + "adi,dpd-robust-modeling")) return false;

        // Write-readback DPD damping (range [0, 255])
        if (!testWriteReadback(PHY + "adi,dpd-damping", "128")) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 5: CLGC Settings (AD9375 only)
// UID: TST.AD9371.ADV_CLGC_SETTINGS
// ============================================
ad9371.switchAdvancedTab("CLGC Settings");
msleep(500);
TestFramework.runTest("TST.AD9371.ADV_CLGC_SETTINGS", function() {
    try {
        var desiredGain = ad9371.readWidget(PHY + "adi,clgc-tx1-desired-gain");
        if (!desiredGain && desiredGain !== "0") {
            printToConsole("  CLGC not available (not AD9375), skipping");
            return "SKIP";
        }
        printToConsole("  Read adi,clgc-tx1-desired-gain = " + desiredGain);

        // Read more attrs
        if (verifyReadable(PHY + "adi,clgc-tx1-control-ratio") === null) return false;
        if (verifyReadable(PHY + "adi,clgc-tx1-rel-threshold") === null) return false;

        // Verify checkboxes
        if (!verifyCheckbox(PHY + "adi,clgc-allow-tx1-atten-updates")) return false;
        if (!verifyCheckbox(PHY + "adi,clgc-tx1-rel-threshold-en")) return false;

        // Write-readback CLGC TX1 atten limit (range [0, 40000])
        if (!testWriteReadback(PHY + "adi,clgc-tx1-atten-limit", "20000")) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 6: VSWR Settings (AD9375 only)
// UID: TST.AD9371.ADV_VSWR_SETTINGS
// ============================================
ad9371.switchAdvancedTab("VSWR Settings");
msleep(500);
TestFramework.runTest("TST.AD9371.ADV_VSWR_SETTINGS", function() {
    try {
        var delayOffset = ad9371.readWidget(PHY + "adi,vswr-additional-delay-offset");
        if (!delayOffset && delayOffset !== "0") {
            printToConsole("  VSWR not available (not AD9375), skipping");
            return "SKIP";
        }
        printToConsole("  Read adi,vswr-additional-delay-offset = " + delayOffset);

        // Read more attrs
        if (verifyReadable(PHY + "adi,vswr-tx1-vswr-switch-delay_us") === null) return false;
        if (verifyReadable(PHY + "adi,vswr-tx1-vswr-switch-gpio3p3-pin") === null) return false;

        // Verify checkbox
        if (!verifyCheckbox(PHY + "adi,vswr-tx1-vswr-switch-polarity")) return false;

        // Write-readback VSWR path delay PN seq level (range [0, 255])
        if (!testWriteReadback(PHY + "adi,vswr-path-delay-pn-seq-level", "100")) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 7: OBS Settings
// UID: TST.AD9371.ADV_OBS_SETTINGS
// ============================================
ad9371.switchAdvancedTab("OBS Settings");
msleep(500);
TestFramework.runTest("TST.AD9371.ADV_OBS_SETTINGS", function() {
    try {
        // Read OBS config combos
        if (verifyReadable(PHY + "adi,obs-settings-obs-rx-lo-source") === null) return false;
        if (verifyReadable(PHY + "adi,obs-settings-default-obs-rx-channel") === null) return false;

        // Verify checkbox
        if (!verifyCheckbox(PHY + "adi,obs-settings-real-if-data")) return false;

        // Read sniffer PLL freq
        if (verifyReadable(PHY + "adi,obs-settings-sniffer-pll-lo-frequency_hz") === null) return false;

        // Read OBS profile attrs
        if (verifyReadable(PHY + "adi,obs-profile-iq-rate_khz") === null) return false;
        if (verifyReadable(PHY + "adi,obs-profile-rf-bandwidth_hz") === null) return false;

        // Read sniffer profile attrs
        if (verifyReadable(PHY + "adi,sniffer-profile-iq-rate_khz") === null) return false;
        if (verifyReadable(PHY + "adi,sniffer-profile-rf-bandwidth_hz") === null) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 8: Gain Setup
// UID: TST.AD9371.ADV_GAIN_SETUP
// ============================================
ad9371.switchAdvancedTab("GAIN Setup");
msleep(500);
TestFramework.runTest("TST.AD9371.ADV_GAIN_SETUP", function() {
    try {
        // Read gain mode combos (0=MGC, 2=AGC, 3=HYBRID)
        if (verifyReadable(PHY + "adi,rx-gain-mode") === null) return false;
        if (verifyReadable(PHY + "adi,orx-gain-mode") === null) return false;
        if (verifyReadable(PHY + "adi,sniffer-gain-mode") === null) return false;

        // Read gain indices
        if (verifyReadable(PHY + "adi,rx1-gain-index") === null) return false;
        if (verifyReadable(PHY + "adi,orx1-gain-index") === null) return false;
        if (verifyReadable(PHY + "adi,sniffer-gain-index") === null) return false;

        // Read min/max
        if (verifyReadable(PHY + "adi,rx1-max-gain-index") === null) return false;
        if (verifyReadable(PHY + "adi,rx1-min-gain-index") === null) return false;

        // Write-readback RX1 gain index (range [0, 255])
        if (!testWriteReadback(PHY + "adi,rx1-gain-index", "200")) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 9: AGC RX Peak Detector
// UID: TST.AD9371.ADV_AGC_RX_PEAK
// ============================================
ad9371.switchAdvancedTab("AGC Setup");
msleep(500);
TestFramework.runTest("TST.AD9371.ADV_AGC_RX_PEAK", function() {
    try {
        // Read peak AGC thresholds
        if (verifyReadable(PHY + "adi,rx-peak-agc-apd-high-thresh") === null) return false;
        if (verifyReadable(PHY + "adi,rx-peak-agc-apd-low-thresh") === null) return false;
        if (verifyReadable(PHY + "adi,rx-peak-agc-hb2-high-thresh") === null) return false;

        // Verify checkbox
        if (!verifyCheckbox(PHY + "adi,rx-peak-agc-apd-fast-attack")) return false;
        if (!verifyCheckbox(PHY + "adi,rx-peak-agc-hb2-fast-attack")) return false;

        // Write-readback HB2 low thresh (range [0, 255])
        if (!testWriteReadback(PHY + "adi,rx-peak-agc-hb2-low-thresh", "128")) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 10: AGC RX Power Measurement
// UID: TST.AD9371.ADV_AGC_RX_POWER
// ============================================
TestFramework.runTest("TST.AD9371.ADV_AGC_RX_POWER", function() {
    try {
        // Read power AGC thresholds
        if (verifyReadable(PHY + "adi,rx-pwr-agc-pmd-upper-high-thresh") === null) return false;
        if (verifyReadable(PHY + "adi,rx-pwr-agc-pmd-upper-low-thresh") === null) return false;
        if (verifyReadable(PHY + "adi,rx-pwr-agc-pmd-lower-high-thresh") === null) return false;
        if (verifyReadable(PHY + "adi,rx-pwr-agc-pmd-lower-low-thresh") === null) return false;

        // Read measurement config combo
        if (verifyReadable(PHY + "adi,rx-pwr-agc-pmd-meas-config") === null) return false;

        // Read measurement duration
        if (verifyReadable(PHY + "adi,rx-pwr-agc-pmd-meas-duration") === null) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 11: AGC RX Config
// UID: TST.AD9371.ADV_AGC_RX_CONFIG
// ============================================
TestFramework.runTest("TST.AD9371.ADV_AGC_RX_CONFIG", function() {
    try {
        // Read gain index ranges
        if (verifyReadable(PHY + "adi,rx-agc-conf-agc-rx1-max-gain-index") === null) return false;
        if (verifyReadable(PHY + "adi,rx-agc-conf-agc-rx1-min-gain-index") === null) return false;

        // Verify checkboxes
        if (!verifyCheckbox(PHY + "adi,rx-agc-conf-agc-peak-threshold-mode")) return false;
        if (!verifyCheckbox(PHY + "adi,rx-agc-conf-agc-low-ths-prevent-gain-increase")) return false;
        if (!verifyCheckbox(PHY + "adi,rx-agc-conf-agc-reset-on-rx-enable")) return false;

        // Read counter/delay
        if (verifyReadable(PHY + "adi,rx-agc-conf-agc-gain-update-counter") === null) return false;
        if (verifyReadable(PHY + "adi,rx-agc-conf-agc-peak-wait-time") === null) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 12: AGC OBS (Peak + Power + Config)
// UID: TST.AD9371.ADV_AGC_OBS
// ============================================
TestFramework.runTest("TST.AD9371.ADV_AGC_OBS", function() {
    try {
        // OBS Peak AGC
        if (verifyReadable(PHY + "adi,obs-peak-agc-apd-high-thresh") === null) return false;
        if (verifyReadable(PHY + "adi,obs-peak-agc-hb2-high-thresh") === null) return false;

        // OBS Power AGC
        if (verifyReadable(PHY + "adi,obs-pwr-agc-pmd-upper-high-thresh") === null) return false;
        if (verifyReadable(PHY + "adi,obs-pwr-agc-pmd-meas-config") === null) return false;

        // OBS AGC Config
        if (verifyReadable(PHY + "adi,obs-agc-conf-agc-obs-rx-max-gain-index") === null) return false;
        if (!verifyCheckbox(PHY + "adi,obs-agc-conf-agc-obs-rx-select")) return false;
        if (!verifyCheckbox(PHY + "adi,obs-agc-conf-agc-peak-threshold-mode")) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 13: ARM GPIO
// UID: TST.AD9371.ADV_ARM_GPIO
// ============================================
ad9371.switchAdvancedTab("ARM GPIO");
msleep(500);
TestFramework.runTest("TST.AD9371.ADV_ARM_GPIO", function() {
    try {
        // Verify checkboxes
        if (!verifyCheckbox(PHY + "adi,arm-gpio-use-rx2-enable-pin")) return false;
        if (!verifyCheckbox(PHY + "adi,arm-gpio-use-tx2-enable-pin")) return false;
        if (!verifyCheckbox(PHY + "adi,arm-gpio-tx-rx-pin-mode")) return false;
        if (!verifyCheckbox(PHY + "adi,arm-gpio-orx-pin-mode")) return false;

        // Read range attrs
        if (verifyReadable(PHY + "adi,arm-gpio-orx-trigger-pin") === null) return false;
        if (verifyReadable(PHY + "adi,arm-gpio-orx-mode2-pin") === null) return false;
        if (verifyReadable(PHY + "adi,arm-gpio-orx-mode1-pin") === null) return false;
        if (verifyReadable(PHY + "adi,arm-gpio-orx-mode0-pin") === null) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 14: GPIO
// UID: TST.AD9371.ADV_GPIO
// ============================================
ad9371.switchAdvancedTab("GPIO");
msleep(500);
TestFramework.runTest("TST.AD9371.ADV_GPIO", function() {
    try {
        // Read GPIO 3.3V source control combos
        if (verifyReadable(PHY + "adi,gpio-3v3-src-ctrl3_0") === null) return false;
        if (verifyReadable(PHY + "adi,gpio-3v3-src-ctrl7_4") === null) return false;
        if (verifyReadable(PHY + "adi,gpio-3v3-src-ctrl11_8") === null) return false;

        // Read GPIO low-voltage source control combos
        if (verifyReadable(PHY + "adi,gpio-src-ctrl3_0") === null) return false;
        if (verifyReadable(PHY + "adi,gpio-src-ctrl7_4") === null) return false;
        if (verifyReadable(PHY + "adi,gpio-src-ctrl11_8") === null) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 15: AUX DAC
// UID: TST.AD9371.ADV_AUX_DAC
// ============================================
ad9371.switchAdvancedTab("AUX DAC");
msleep(500);
TestFramework.runTest("TST.AD9371.ADV_AUX_DAC", function() {
    try {
        // Read DAC value (range [0, 1023])
        if (verifyReadable(PHY + "adi,aux-dac-value0") === null) return false;
        if (verifyReadable(PHY + "adi,aux-dac-value1") === null) return false;

        // Read DAC slope combo
        if (verifyReadable(PHY + "adi,aux-dac-slope0") === null) return false;

        // Read DAC vref combo
        if (verifyReadable(PHY + "adi,aux-dac-vref0") === null) return false;

        // Write-readback DAC value0 (range [0, 1023])
        if (!testWriteReadback(PHY + "adi,aux-dac-value0", "512")) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 16: JESD Framer (RX + OBS)
// UID: TST.AD9371.ADV_JESD_FRAMER
// ============================================
ad9371.switchAdvancedTab("JESD Framer");
msleep(500);
TestFramework.runTest("TST.AD9371.ADV_JESD_FRAMER", function() {
    try {
        // RX Framer attrs
        if (verifyReadable(PHY + "adi,jesd204-rx-framer-bank-id") === null) return false;
        if (verifyReadable(PHY + "adi,jesd204-rx-framer-device-id") === null) return false;
        if (verifyReadable(PHY + "adi,jesd204-rx-framer-m") === null) return false;
        if (verifyReadable(PHY + "adi,jesd204-rx-framer-k") === null) return false;
        if (!verifyCheckbox(PHY + "adi,jesd204-rx-framer-scramble")) return false;
        if (!verifyCheckbox(PHY + "adi,jesd204-rx-framer-external-sysref")) return false;

        // OBS Framer attrs
        if (verifyReadable(PHY + "adi,jesd204-obs-framer-bank-id") === null) return false;
        if (verifyReadable(PHY + "adi,jesd204-obs-framer-device-id") === null) return false;
        if (!verifyCheckbox(PHY + "adi,jesd204-obs-framer-scramble")) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 17: JESD Deframer
// UID: TST.AD9371.ADV_JESD_DEFRAMER
// ============================================
ad9371.switchAdvancedTab("JESD Deframer");
msleep(500);
TestFramework.runTest("TST.AD9371.ADV_JESD_DEFRAMER", function() {
    try {
        // Deframer attrs
        if (verifyReadable(PHY + "adi,jesd204-deframer-bank-id") === null) return false;
        if (verifyReadable(PHY + "adi,jesd204-deframer-device-id") === null) return false;
        if (verifyReadable(PHY + "adi,jesd204-deframer-m") === null) return false;
        if (verifyReadable(PHY + "adi,jesd204-deframer-k") === null) return false;
        if (!verifyCheckbox(PHY + "adi,jesd204-deframer-scramble")) return false;
        if (!verifyCheckbox(PHY + "adi,jesd204-deframer-external-sysref")) return false;

        // Read more attrs
        if (verifyReadable(PHY + "adi,jesd204-deframer-eq-setting") === null) return false;
        if (verifyReadable(PHY + "adi,jesd204-deframer-lmfc-offset") === null) return false;
        if (!verifyCheckbox(PHY + "adi,jesd204-deframer-new-sysref-on-relink")) return false;

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 18: BIST
// UID: TST.AD9371.ADV_BIST
// ============================================
ad9371.switchAdvancedTab("BIST");
msleep(500);
TestFramework.runTest("TST.AD9371.ADV_BIST", function() {
    try {
        // Verify loopback checkboxes
        if (!verifyCheckbox(PHY + "loopback_tx_rx")) return false;
        if (!verifyCheckbox(PHY + "loopback_tx_obs")) return false;

        // Read PRBS combos
        if (verifyReadable(PHY + "bist_prbs_rx") === null) return false;
        if (verifyReadable(PHY + "bist_prbs_obs") === null) return false;

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
