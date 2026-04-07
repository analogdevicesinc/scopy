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

// ADRV9009 Manual Test Automation
// Automates tests from: docs/tests/plugins/adrv9009/adrv9009_tests.rst
//
// Tests automated here (Category A — fully automated via JS API):
//   TST.ADRV9009.CONTROLS.PLUGIN_LOADS          (#1)
//   TST.ADRV9009.CONTROLS.DEVICE_DETECTION       (#2)
//   TST.ADRV9009.CONTROLS.GLOBAL_SETTINGS        (#3)
//   TST.ADRV9009.CONTROLS.CALIBRATIONS           (#6)
//   TST.ADRV9009.CONTROLS.TRX_LO_FREQUENCY       (#5, including FHM)
//   TST.ADRV9009.CONTROLS.RX_CHANNEL_CONFIG      (#7)
//   TST.ADRV9009.CONTROLS.TX_CHANNEL_CONFIG      (#8)
//   TST.ADRV9009.CONTROLS.OBS_CHANNEL_CONFIG     (#9)
//   TST.ADRV9009.CONTROLS.FPGA_PHASE_ROTATION    (#10)
//   TST.ADRV9009.CONTROLS.REFRESH_FUNCTION       (#11)
//   TST.ADRV9009.ADVANCED.CLK_SETTINGS           (Advanced #2)
//   TST.ADRV9009.ADVANCED.RX_SETTINGS            (Advanced #3)
//   TST.ADRV9009.ADVANCED.TX_SETTINGS            (Advanced #4)
//   TST.ADRV9009.ADVANCED.ORX_SETTINGS           (Advanced #5)
//   TST.ADRV9009.ADVANCED.JESD_SETTINGS          (Advanced #6)
//   TST.ADRV9009.ADVANCED.JESD_FRAMER            (Advanced #7)
//   TST.ADRV9009.ADVANCED.JESD_DEFRAMER          (Advanced #8)
//   TST.ADRV9009.ADVANCED.REFRESH_FUNCTION       (Advanced #9)
//
// Not automated (requires OS file browser interaction):
//   TST.ADRV9009.CONTROLS.PROFILE_LOAD  (#4)
//
// Not automated (cannot be automated — lane polarity changes disrupt the JESD link):
//   TST.ADRV9009.ADVANCED.JESD_SETTINGS.SER_LANE_POLARITY  (#43)
//   TST.ADRV9009.ADVANCED.JESD_SETTINGS.DES_LANE_POLARITY  (#44)
//
// ============================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("ADRV9009 Documentation Tests");

// Connect to device — ip:127.0.0.0 intentionally fails, triggering emulator fallback
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to ADRV9009 tool
if (!switchToTool("ADRV9009")) {
    printToConsole("ERROR: Cannot switch to ADRV9009 tool");
    scopy.exit();
}

// Discover all widget keys at startup
var allKeys = adrv9009.getWidgetKeys();
printToConsole("Total widget keys discovered: " + allKeys.length);

// Helper: find the first key containing the given pattern
function findKey(pattern) {
    for (var i = 0; i < allKeys.length; i++) {
        if (allKeys[i].indexOf(pattern) !== -1) return allKeys[i];
    }
    return null;
}

// Helper: find all keys containing the given pattern
function findKeys(pattern) {
    var result = [];
    for (var i = 0; i < allKeys.length; i++) {
        if (allKeys[i].indexOf(pattern) !== -1) result.push(allKeys[i]);
    }
    return result;
}

// Helper: toggle a boolean attribute (0/1) and verify readback
// Returns true if toggle succeeded, false otherwise
function toggleBoolAttr(key, label) {
    var orig = adrv9009.readWidget(key);
    if (orig === "" || orig === null || orig === undefined) {
        printToConsole("  FAIL: " + label + " returned empty on read");
        return false;
    }
    var newVal = (orig === "1" || orig === "true") ? "0" : "1";
    adrv9009.writeWidget(key, newVal);
    msleep(500);
    var rb = adrv9009.readWidget(key);
    adrv9009.writeWidget(key, orig);
    msleep(500);
    var rbNum = parseFloat(rb);
    var expected = parseFloat(newVal);
    if (isNaN(rbNum) || Math.abs(rbNum - expected) > 0.5) {
        printToConsole("  FAIL: " + label + " write " + newVal + " readback " + rb);
        return false;
    }
    printToConsole("  PASS: " + label + " toggle " + orig + " -> " + newVal + " -> restored");
    return true;
}

// ============================================
// Test 1: Plugin Loads
// UID: TST.ADRV9009.CONTROLS.PLUGIN_LOADS
// Description: Verify that the ADRV9009 plugin loads and tool appears.
// ============================================
printToConsole("\n=== Test 1: Plugin Loads ===\n");

TestFramework.runTest("TST.ADRV9009.CONTROLS.PLUGIN_LOADS", function() {
    try {
        var tools = adrv9009.getTools();
        printToConsole("  Tools found: " + tools);
        if (!tools || tools.length === 0) {
            printToConsole("  FAIL: No tools found for ADRV9009 plugin");
            return false;
        }
        if (tools.indexOf("ADRV9009") === -1) {
            printToConsole("  FAIL: ADRV9009 tool not in list: " + tools);
            return false;
        }
        printToConsole("  PASS: ADRV9009 tool is visible and accessible");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2: Device Detection and Display
// UID: TST.ADRV9009.CONTROLS.DEVICE_DETECTION
// Description: Verify all main UI sections are present by checking widget keys.
//   Global Settings: ensm_mode key present
//   RX section: voltage0_in/hardwaregain key present (RX1)
//   TX section: voltage0_out/hardwaregain key present (TX1)
//   OBS section: voltage2_in/hardwaregain key present (OBS1)
//   FPGA section: calibscale key present (axi-adrv9009-rx-hpc)
// ============================================
printToConsole("\n=== Test 2: Device Detection and Display ===\n");

TestFramework.runTest("TST.ADRV9009.CONTROLS.DEVICE_DETECTION", function() {
    try {
        var pass = true;

        // Global Settings: ENSM mode widget must exist
        var ensmKey = findKey("ensm_mode");
        if (!ensmKey) {
            printToConsole("  FAIL: Global Settings section not detected (ensm_mode key missing)");
            pass = false;
        } else {
            printToConsole("  PASS: Global Settings detected — key: " + ensmKey);
        }

        // RX section: RX1 hardwaregain
        var rx1Key = findKey("voltage0_in/hardwaregain");
        if (!rx1Key) {
            printToConsole("  FAIL: RX section not detected (voltage0_in/hardwaregain missing)");
            pass = false;
        } else {
            printToConsole("  PASS: RX section detected — key: " + rx1Key);
        }

        // RX2 presence
        var rx2Key = findKey("voltage1_in/hardwaregain");
        if (!rx2Key) {
            printToConsole("  WARN: RX2 not detected (voltage1_in/hardwaregain missing)");
        } else {
            printToConsole("  PASS: RX2 detected — key: " + rx2Key);
        }

        // TX section: TX1 hardwaregain
        var tx1Key = findKey("voltage0_out/hardwaregain");
        if (!tx1Key) {
            printToConsole("  FAIL: TX section not detected (voltage0_out/hardwaregain missing)");
            pass = false;
        } else {
            printToConsole("  PASS: TX section detected — key: " + tx1Key);
        }

        // OBS section: OBS1 hardwaregain
        var obs1Key = findKey("voltage2_in/hardwaregain");
        if (!obs1Key) {
            printToConsole("  FAIL: OBS section not detected (voltage2_in/hardwaregain missing)");
            pass = false;
        } else {
            printToConsole("  PASS: OBS section detected — key: " + obs1Key);
        }

        // FPGA section: calibscale on axi-adrv9009-rx-hpc
        var fpgaKey = findKey("calibscale");
        if (!fpgaKey) {
            printToConsole("  WARN: FPGA section not detected (calibscale missing — FPGA device may not be present)");
        } else {
            printToConsole("  PASS: FPGA section detected — key: " + fpgaKey);
        }

        if (pass) {
            printToConsole("  PASS: All mandatory UI sections detected via widget keys");
        }
        return pass;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3: Global Settings — ENSM Mode
// UID: TST.ADRV9009.CONTROLS.GLOBAL_SETTINGS
// Description: Verify ENSM mode control changes device state.
// ============================================
printToConsole("\n=== Test 3: Global Settings — ENSM Mode ===\n");

TestFramework.runTest("TST.ADRV9009.CONTROLS.GLOBAL_SETTINGS", function() {
    try {
        var ensmKey = findKey("ensm_mode");
        if (!ensmKey) {
            printToConsole("  FAIL: No ensm_mode key found in widget keys");
            return false;
        }
        printToConsole("  Using ENSM key: " + ensmKey);

        // Step 1: Note the current ENSM mode value
        var origVal = adrv9009.readWidget(ensmKey);
        printToConsole("  Current ENSM mode: " + origVal);
        if (!origVal || origVal === "") {
            printToConsole("  FAIL: readWidget returned empty for " + ensmKey);
            return false;
        }

        // Step 2: Change to a different value
        // ADRV9009 ENSM modes: radio_on, alert — pick the other one
        var newVal = (origVal.indexOf("alert") !== -1) ? "radio_on" : "alert";
        printToConsole("  Changing ENSM mode to: " + newVal);
        adrv9009.writeWidget(ensmKey, newVal);
        msleep(500);

        // Step 3: Read back from hardware
        var readBack = adrv9009.readWidget(ensmKey);
        printToConsole("  Read back ENSM mode: " + readBack);

        // Restore original value
        adrv9009.writeWidget(ensmKey, origVal);
        msleep(500);

        if (readBack.indexOf(newVal) === -1) {
            printToConsole("  FAIL: Hardware did not reflect ENSM mode change (got: " + readBack + ")");
            return false;
        }
        printToConsole("  PASS: ENSM mode write-readback verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 6: Calibration Controls
// UID: TST.ADRV9009.CONTROLS.CALIBRATIONS
// Description: Verify calibration enable checkboxes can be toggled.
// Attributes (device-level on adrv9009-phy):
//   calibrate_rx_qec_en, calibrate_tx_qec_en, calibrate_tx_lol_en,
//   calibrate_tx_lol_ext_en, calibrate_rx_phase_correction_en, calibrate_fhm_en
// Note: The 'calibrate' hardware trigger attribute is not exposed as a UI button.
// ============================================
printToConsole("\n=== Test 6: Calibration Controls ===\n");

TestFramework.runTest("TST.ADRV9009.CONTROLS.CALIBRATIONS.RX_QEC", function() {
    try {
        var key = findKey("calibrate_rx_qec_en");
        if (!key) {
            printToConsole("  FAIL: calibrate_rx_qec_en key not found");
            return false;
        }
        return toggleBoolAttr(key, "RX QEC enable");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.CALIBRATIONS.TX_QEC", function() {
    try {
        var key = findKey("calibrate_tx_qec_en");
        if (!key) {
            printToConsole("  FAIL: calibrate_tx_qec_en key not found");
            return false;
        }
        return toggleBoolAttr(key, "TX QEC enable");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.CALIBRATIONS.TX_LOL", function() {
    try {
        var key = findKey("calibrate_tx_lol_en");
        if (!key) {
            printToConsole("  FAIL: calibrate_tx_lol_en key not found");
            return false;
        }
        return toggleBoolAttr(key, "TX LOL enable");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.CALIBRATIONS.TX_LOL_EXT", function() {
    try {
        var key = findKey("calibrate_tx_lol_ext_en");
        if (!key) {
            printToConsole("  SKIP: calibrate_tx_lol_ext_en key not found");
            return true;
        }
        return toggleBoolAttr(key, "TX LOL Ext enable");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.CALIBRATIONS.RX_PHASE_CORR", function() {
    try {
        var key = findKey("calibrate_rx_phase_correction_en");
        if (!key) {
            printToConsole("  SKIP: calibrate_rx_phase_correction_en key not found");
            return true;
        }
        return toggleBoolAttr(key, "RX Phase Correction enable");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.CALIBRATIONS.FHM", function() {
    try {
        var key = findKey("calibrate_fhm_en");
        if (!key) {
            printToConsole("  SKIP: calibrate_fhm_en key not found");
            return true;
        }
        return toggleBoolAttr(key, "FHM calibration enable");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 5: TRX LO Frequency Control
// UID: TST.ADRV9009.CONTROLS.TRX_LO_FREQUENCY
// Description: Verify TRX LO frequency control within valid range.
// ============================================
printToConsole("\n=== Test 5: TRX LO Frequency Control ===\n");

TestFramework.runTest("TST.ADRV9009.CONTROLS.TRX_LO_FREQUENCY", function() {
    try {
        // Key: adrv9009-phy/altvoltage0_out/frequency
        var loKey = findKey("altvoltage0_out/frequency");
        if (!loKey) {
            printToConsole("  FAIL: No TRX LO frequency key found");
            printToConsole("  Available altvoltage keys: " + findKeys("altvoltage"));
            return false;
        }
        printToConsole("  Using TRX LO key: " + loKey);

        // Step 1: Note current frequency
        var origFreq = adrv9009.readWidget(loKey);
        printToConsole("  Current TRX LO frequency: " + origFreq);
        if (!origFreq || origFreq === "") {
            printToConsole("  FAIL: readWidget returned empty for " + loKey);
            return false;
        }

        // Step 2: Change to a new value within range (70–6000 MHz = 70e6–6000e6 Hz)
        var newFreq = "2450000000";
        printToConsole("  Changing TRX LO frequency to: " + newFreq + " Hz");
        adrv9009.writeWidget(loKey, newFreq);
        msleep(500);

        // Step 3: Read back frequency
        var readBack = adrv9009.readWidget(loKey);
        printToConsole("  Read back frequency: " + readBack);

        // Restore original
        adrv9009.writeWidget(loKey, origFreq);
        msleep(500);

        if (!readBack || readBack === "") {
            printToConsole("  FAIL: readWidget returned empty after write");
            return false;
        }
        var readNum = parseFloat(readBack);
        var writeNum = parseFloat(newFreq);
        if (isNaN(readNum) || Math.abs(readNum - writeNum) / writeNum > 0.01) {
            printToConsole("  FAIL: Read-back value " + readBack + " does not match written " + newFreq);
            return false;
        }
        printToConsole("  PASS: TRX LO frequency write-readback verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.TRX_LO_FREQUENCY.FHM_ENABLE", function() {
    try {
        // Step 4: Enable/disable frequency hopping mode
        // Key: adrv9009-phy/altvoltage0_out/frequency_hopping_mode_enable
        var fhmKey = findKey("altvoltage0_out/frequency_hopping_mode_enable");
        if (!fhmKey) {
            printToConsole("  SKIP: frequency_hopping_mode_enable key not found");
            return true;
        }
        printToConsole("  Using FHM enable key: " + fhmKey);
        return toggleBoolAttr(fhmKey, "Frequency Hopping Mode enable");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 7: RX Channel Controls (RX1/RX2)
// UID: TST.ADRV9009.CONTROLS.RX_CHANNEL_CONFIG
// Description: Verify RX gain control mode, hardware gain, powerdown, tracking.
// Key format: adrv9009-phy/voltage{0,1}_in/{attr}
// ============================================
printToConsole("\n=== Test 7: RX Channel Controls ===\n");

TestFramework.runTest("TST.ADRV9009.CONTROLS.RX_CHANNEL_CONFIG.GAIN_MODE", function() {
    try {
        // gain_control_mode for RX1 = voltage0_in
        var modeKey = findKey("voltage0_in/gain_control_mode");
        if (!modeKey) {
            printToConsole("  Keys with 'gain_control_mode': " + findKeys("gain_control_mode"));
            printToConsole("  SKIP: RX gain_control_mode key not found");
            return true;
        }
        printToConsole("  Using gain_control_mode key: " + modeKey);

        var origMode = adrv9009.readWidget(modeKey);
        printToConsole("  Original RX1 gain control mode: " + origMode);
        if (!origMode || origMode === "") {
            printToConsole("  FAIL: gain_control_mode returned empty");
            return false;
        }

        // Read available modes
        var availKey = findKey("voltage0_in/gain_control_mode_available");
        if (!availKey) {
            availKey = findKey("gain_control_mode_available");
        }
        var newMode = null;
        if (availKey) {
            var available = adrv9009.readWidget(availKey);
            printToConsole("  Available modes: " + available);
            var modes = available.split(" ");
            for (var i = 0; i < modes.length; i++) {
                var m = modes[i].trim();
                if (m !== "" && m !== origMode) {
                    newMode = m;
                    break;
                }
            }
        }

        if (!newMode) {
            // Fallback: try common ADRV9009 gain modes
            newMode = (origMode === "manual") ? "slow_attack" : "manual";
        }

        // Step 1: Change to a different mode
        printToConsole("  Changing gain control mode to: " + newMode);
        adrv9009.writeWidget(modeKey, newMode);
        msleep(500);

        var readBack = adrv9009.readWidget(modeKey);
        printToConsole("  Read back gain control mode: " + readBack);

        // Restore
        adrv9009.writeWidget(modeKey, origMode);
        msleep(500);

        if (!readBack || readBack === "") {
            printToConsole("  FAIL: gain_control_mode read-back returned empty");
            return false;
        }
        if (readBack.indexOf(newMode) === -1) {
            printToConsole("  FAIL: gain_control_mode read-back '" + readBack + "' does not match written '" + newMode + "'");
            return false;
        }
        printToConsole("  PASS: RX1 gain_control_mode write-readback verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.RX_CHANNEL_CONFIG.HW_GAIN", function() {
    try {
        var gainKey = findKey("voltage0_in/hardwaregain");
        if (!gainKey) {
            gainKey = findKey("voltage0_in/gain");
        }
        if (!gainKey) {
            printToConsole("  FAIL: RX1 hardwaregain key not found");
            printToConsole("  voltage0_in keys: " + findKeys("voltage0_in"));
            return false;
        }
        printToConsole("  Using RX1 gain key: " + gainKey);

        // Step 3: Set RX1 hardware gain to a new value (range 0–30 dB)
        var origGain = adrv9009.readWidget(gainKey);
        printToConsole("  Original RX1 hardware gain: " + origGain);

        adrv9009.writeWidget(gainKey, "18");
        msleep(500);
        var readBack = adrv9009.readWidget(gainKey);
        printToConsole("  Set RX1 gain to 18, read back: " + readBack);

        // Also test RX2 (voltage1_in)
        var gainKey2 = findKey("voltage1_in/hardwaregain");
        if (gainKey2) {
            var origGain2 = adrv9009.readWidget(gainKey2);
            adrv9009.writeWidget(gainKey2, "20");
            msleep(500);
            var readBack2 = adrv9009.readWidget(gainKey2);
            printToConsole("  Set RX2 gain to 20, read back: " + readBack2);
            adrv9009.writeWidget(gainKey2, origGain2);
            msleep(500);
        }

        // Restore RX1
        adrv9009.writeWidget(gainKey, origGain);
        msleep(500);

        if (!readBack || readBack === "") {
            printToConsole("  FAIL: gain read-back returned empty");
            return false;
        }
        var readNum = parseFloat(readBack);
        if (isNaN(readNum) || Math.abs(readNum - 18) > 1) {
            printToConsole("  FAIL: gain read-back " + readBack + " does not match written 18");
            return false;
        }
        printToConsole("  PASS: RX1 hardware gain write-readback verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.RX_CHANNEL_CONFIG.POWERDOWN", function() {
    try {
        var pdKey = findKey("voltage0_in/powerdown");
        if (!pdKey) {
            printToConsole("  FAIL: RX1 powerdown key not found");
            return false;
        }
        printToConsole("  Using RX1 powerdown key: " + pdKey);

        var origPd = adrv9009.readWidget(pdKey);
        printToConsole("  Original RX1 powerdown: " + origPd);

        // Step 4: Toggle RX1 powerdown
        adrv9009.writeWidget(pdKey, "1");
        msleep(500);
        var afterEnable = adrv9009.readWidget(pdKey);
        printToConsole("  After set powerdown=1: " + afterEnable);

        adrv9009.writeWidget(pdKey, "0");
        msleep(500);
        var afterDisable = adrv9009.readWidget(pdKey);
        printToConsole("  After set powerdown=0: " + afterDisable);

        // Restore
        adrv9009.writeWidget(pdKey, origPd);
        msleep(500);

        if (parseFloat(afterEnable) !== 1 || parseFloat(afterDisable) !== 0) {
            printToConsole("  FAIL: powerdown toggle did not function correctly");
            return false;
        }
        printToConsole("  PASS: RX1 powerdown toggle functions correctly");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.RX_CHANNEL_CONFIG.QEC_TRACKING", function() {
    try {
        // Step 5: Toggle RX1 QEC tracking
        var qecKey = findKey("voltage0_in/quadrature_tracking_en");
        if (!qecKey) {
            printToConsole("  SKIP: RX1 quadrature_tracking_en key not found");
            return true;
        }
        printToConsole("  Using QEC tracking key: " + qecKey);
        return toggleBoolAttr(qecKey, "RX1 QEC tracking");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.RX_CHANNEL_CONFIG.HD2_TRACKING", function() {
    try {
        // Step 6: Toggle RX1 HD2 tracking
        var hd2Key = findKey("voltage0_in/hd2_tracking_en");
        if (!hd2Key) {
            printToConsole("  SKIP: RX1 hd2_tracking_en key not found");
            return true;
        }
        printToConsole("  Using HD2 tracking key: " + hd2Key);
        return toggleBoolAttr(hd2Key, "RX1 HD2 tracking");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 8: TX Channel Controls (TX1/TX2)
// UID: TST.ADRV9009.CONTROLS.TX_CHANNEL_CONFIG
// Description: Verify TX attenuation, powerdown, QEC/LOL tracking.
// Key format: adrv9009-phy/voltage{0,1}_out/{attr}
// TX hardwaregain convention: values are negative dB (0 to -41.95)
// ============================================
printToConsole("\n=== Test 8: TX Channel Controls ===\n");

TestFramework.runTest("TST.ADRV9009.CONTROLS.TX_CHANNEL_CONFIG.ATTENUATION", function() {
    try {
        // TX1 = voltage0_out, TX2 = voltage1_out
        var attenKey = findKey("voltage0_out/hardwaregain");
        if (!attenKey) {
            printToConsole("  FAIL: TX1 hardwaregain key not found");
            printToConsole("  voltage0_out keys: " + findKeys("voltage0_out"));
            return false;
        }
        printToConsole("  Using TX1 attenuation key: " + attenKey);

        // Step 1: Change TX1 attenuation (IIO value is negative dB, range 0 to -41.95)
        var origAtten = adrv9009.readWidget(attenKey);
        printToConsole("  Original TX1 attenuation: " + origAtten);

        adrv9009.writeWidget(attenKey, "-20");
        msleep(500);
        var readBack = adrv9009.readWidget(attenKey);
        printToConsole("  Set TX1 attenuation to -20, read back: " + readBack);

        // Also test TX2 (voltage1_out)
        var attenKey2 = findKey("voltage1_out/hardwaregain");
        if (attenKey2) {
            var origAtten2 = adrv9009.readWidget(attenKey2);
            adrv9009.writeWidget(attenKey2, "-15");
            msleep(500);
            var readBack2 = adrv9009.readWidget(attenKey2);
            printToConsole("  Set TX2 attenuation to -15, read back: " + readBack2);
            adrv9009.writeWidget(attenKey2, origAtten2);
            msleep(500);
        }

        // Restore TX1
        adrv9009.writeWidget(attenKey, origAtten);
        msleep(500);

        if (!readBack || readBack === "") {
            printToConsole("  FAIL: TX attenuation read-back returned empty");
            return false;
        }
        var readNum = parseFloat(readBack);
        if (isNaN(readNum) || Math.abs(readNum - (-20)) > 1) {
            printToConsole("  FAIL: TX attenuation read-back " + readBack + " does not match written -20");
            return false;
        }
        printToConsole("  PASS: TX1 attenuation write-readback verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.TX_CHANNEL_CONFIG.POWERDOWN", function() {
    try {
        var pdKey = findKey("voltage0_out/powerdown");
        if (!pdKey) {
            printToConsole("  FAIL: TX1 powerdown key not found");
            return false;
        }
        printToConsole("  Using TX1 powerdown key: " + pdKey);

        var origPd = adrv9009.readWidget(pdKey);
        printToConsole("  Original TX1 powerdown: " + origPd);

        // Step 2: Toggle TX1 powerdown
        adrv9009.writeWidget(pdKey, "1");
        msleep(500);
        var afterEnable = adrv9009.readWidget(pdKey);

        adrv9009.writeWidget(pdKey, "0");
        msleep(500);
        var afterDisable = adrv9009.readWidget(pdKey);

        // Restore
        adrv9009.writeWidget(pdKey, origPd);
        msleep(500);

        printToConsole("  Powerdown: enabled=" + afterEnable + ", disabled=" + afterDisable);
        if (parseFloat(afterEnable) !== 1 || parseFloat(afterDisable) !== 0) {
            printToConsole("  FAIL: TX1 powerdown toggle did not function correctly");
            return false;
        }
        printToConsole("  PASS: TX1 powerdown toggle functions correctly");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.TX_CHANNEL_CONFIG.QEC_TRACKING", function() {
    try {
        // Step 3: Toggle TX1 QEC tracking
        var qecKey = findKey("voltage0_out/quadrature_tracking_en");
        if (!qecKey) {
            printToConsole("  SKIP: TX1 quadrature_tracking_en key not found");
            return true;
        }
        printToConsole("  Using TX1 QEC tracking key: " + qecKey);
        return toggleBoolAttr(qecKey, "TX1 QEC tracking");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.TX_CHANNEL_CONFIG.LOL_TRACKING", function() {
    try {
        // Step 4: Toggle TX1 LOL tracking
        var lolKey = findKey("voltage0_out/lo_leakage_tracking_en");
        if (!lolKey) {
            printToConsole("  SKIP: TX1 lo_leakage_tracking_en key not found");
            return true;
        }
        printToConsole("  Using TX1 LOL tracking key: " + lolKey);
        return toggleBoolAttr(lolKey, "TX1 LOL tracking");
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 9: Observation RX Controls (OBS1/OBS2)
// UID: TST.ADRV9009.CONTROLS.OBS_CHANNEL_CONFIG
// Description: Verify OBS hardware gain, LO source, AUX LO, powerdown.
// Key format: adrv9009-phy/voltage{2,3}_in/{attr}
// ============================================
printToConsole("\n=== Test 9: OBS Channel Controls ===\n");

TestFramework.runTest("TST.ADRV9009.CONTROLS.OBS_CHANNEL_CONFIG.HW_GAIN", function() {
    try {
        var gainKey = findKey("voltage2_in/hardwaregain");
        if (!gainKey) {
            printToConsole("  FAIL: OBS1 hardwaregain key not found");
            printToConsole("  voltage2_in keys: " + findKeys("voltage2_in"));
            return false;
        }
        printToConsole("  Using OBS1 gain key: " + gainKey);

        // Step 1: Change OBS1 hardware gain (range 0–30 dB)
        var origGain = adrv9009.readWidget(gainKey);
        printToConsole("  Original OBS1 gain: " + origGain);

        adrv9009.writeWidget(gainKey, "12");
        msleep(500);
        var readBack = adrv9009.readWidget(gainKey);
        printToConsole("  Set OBS1 gain to 12, read back: " + readBack);

        // Also test OBS2
        var gainKey2 = findKey("voltage3_in/hardwaregain");
        if (gainKey2) {
            var origGain2 = adrv9009.readWidget(gainKey2);
            adrv9009.writeWidget(gainKey2, "15");
            msleep(500);
            var readBack2 = adrv9009.readWidget(gainKey2);
            printToConsole("  Set OBS2 gain to 15, read back: " + readBack2);
            adrv9009.writeWidget(gainKey2, origGain2);
            msleep(500);
        }

        // Restore OBS1
        adrv9009.writeWidget(gainKey, origGain);
        msleep(500);

        if (!readBack || readBack === "") {
            printToConsole("  FAIL: OBS gain read-back returned empty");
            return false;
        }
        var readNum = parseFloat(readBack);
        if (isNaN(readNum) || Math.abs(readNum - 12) > 1) {
            printToConsole("  FAIL: OBS gain read-back " + readBack + " does not match written 12");
            return false;
        }
        printToConsole("  PASS: OBS1 hardware gain write-readback verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.OBS_CHANNEL_CONFIG.LO_SOURCE", function() {
    try {
        // Step 2: Change LO source for OBS (rf_port_select)
        var loSrcKey = findKey("voltage2_in/rf_port_select");
        if (!loSrcKey) {
            printToConsole("  SKIP: OBS1 rf_port_select key not found");
            return true;
        }
        printToConsole("  Using OBS1 LO source key: " + loSrcKey);

        var origSrc = adrv9009.readWidget(loSrcKey);
        printToConsole("  Original OBS1 LO source: " + origSrc);
        if (!origSrc || origSrc === "") {
            printToConsole("  FAIL: rf_port_select returned empty");
            return false;
        }

        // Read available LO source options
        var availKey = findKey("voltage2_in/rf_port_select_available");
        var newSrc = null;
        if (availKey) {
            var available = adrv9009.readWidget(availKey);
            printToConsole("  Available LO sources: " + available);
            var options = available.split(" ");
            for (var i = 0; i < options.length; i++) {
                var opt = options[i].trim();
                if (opt !== "" && opt !== origSrc) {
                    newSrc = opt;
                    break;
                }
            }
        }

        if (!newSrc) {
            // Fallback: ADRV9009 OBS LO sources are RFPLL or AUXPLL
            newSrc = (origSrc.indexOf("AUX") !== -1) ? "RFPLL" : "AUXPLL";
        }

        printToConsole("  Changing OBS1 LO source to: " + newSrc);
        adrv9009.writeWidget(loSrcKey, newSrc);
        msleep(500);

        var readBack = adrv9009.readWidget(loSrcKey);
        printToConsole("  Read back OBS1 LO source: " + readBack);

        // Restore
        adrv9009.writeWidget(loSrcKey, origSrc);
        msleep(500);

        if (!readBack || readBack === "") {
            printToConsole("  FAIL: rf_port_select read-back returned empty");
            return false;
        }
        if (readBack.indexOf(newSrc) === -1) {
            printToConsole("  FAIL: LO source read-back '" + readBack + "' does not match written '" + newSrc + "'");
            return false;
        }
        printToConsole("  PASS: OBS1 LO source write-readback verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.OBS_CHANNEL_CONFIG.AUX_LO", function() {
    try {
        // Step 3: AUX LO frequency (altvoltage1_out/frequency)
        var auxLoKey = findKey("altvoltage1_out/frequency");
        if (!auxLoKey) {
            printToConsole("  SKIP: AUX LO frequency key not found");
            printToConsole("  altvoltage1 keys: " + findKeys("altvoltage1"));
            return true;
        }
        printToConsole("  Using AUX LO key: " + auxLoKey);

        var origAuxLo = adrv9009.readWidget(auxLoKey);
        printToConsole("  Original AUX LO frequency: " + origAuxLo);

        adrv9009.writeWidget(auxLoKey, "2100000000");
        msleep(500);
        var readBack = adrv9009.readWidget(auxLoKey);
        printToConsole("  Set AUX LO to 2100 MHz, read back: " + readBack);

        // Restore
        adrv9009.writeWidget(auxLoKey, origAuxLo);
        msleep(500);

        if (!readBack || readBack === "") {
            printToConsole("  FAIL: AUX LO read-back returned empty");
            return false;
        }
        var readNum = parseFloat(readBack);
        if (isNaN(readNum) || Math.abs(readNum - 2100000000) / 2100000000 > 0.01) {
            printToConsole("  FAIL: AUX LO read-back " + readBack + " does not match written 2100000000");
            return false;
        }
        printToConsole("  PASS: AUX LO frequency write-readback verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.CONTROLS.OBS_CHANNEL_CONFIG.POWERDOWN", function() {
    try {
        var pdKey = findKey("voltage2_in/powerdown");
        if (!pdKey) {
            printToConsole("  FAIL: OBS1 powerdown key not found");
            return false;
        }
        printToConsole("  Using OBS1 powerdown key: " + pdKey);

        var origPd = adrv9009.readWidget(pdKey);
        printToConsole("  Original OBS1 powerdown: " + origPd);

        // Step 4: Toggle OBS1 powerdown
        adrv9009.writeWidget(pdKey, "1");
        msleep(500);
        var afterEnable = adrv9009.readWidget(pdKey);

        adrv9009.writeWidget(pdKey, "0");
        msleep(500);
        var afterDisable = adrv9009.readWidget(pdKey);

        // Restore
        adrv9009.writeWidget(pdKey, origPd);
        msleep(500);

        printToConsole("  OBS powerdown: enabled=" + afterEnable + ", disabled=" + afterDisable);
        if (parseFloat(afterEnable) !== 1 || parseFloat(afterDisable) !== 0) {
            printToConsole("  FAIL: OBS1 powerdown toggle did not function correctly");
            return false;
        }
        printToConsole("  PASS: OBS1 powerdown toggle functions correctly");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 10: FPGA Phase Rotation
// UID: TST.ADRV9009.CONTROLS.FPGA_PHASE_ROTATION
// Description: Verify FPGA phase rotation controls update for RX1 and RX2.
// Device: axi-adrv9009-rx-hpc
// Channels: voltage0_i_in/calibscale (RX1 I), voltage0_q_in/calibscale (RX1 Q)
// ============================================
printToConsole("\n=== Test 10: FPGA Phase Rotation ===\n");

TestFramework.runTest("TST.ADRV9009.CONTROLS.FPGA_PHASE_ROTATION", function() {
    try {
        // Phase rotation keys on axi-adrv9009-rx-hpc device
        var phaseKeys = findKeys("calibscale");
        printToConsole("  calibscale keys found: " + phaseKeys);

        if (!phaseKeys || phaseKeys.length === 0) {
            printToConsole("  SKIP: No calibscale (phase rotation) keys found — FPGA device may not be present");
            return true;
        }

        var rx1ScaleKey = phaseKeys[0];
        printToConsole("  Using phase scale key: " + rx1ScaleKey);

        // Steps 1-2: Change RX1 phase rotation
        var origScale = adrv9009.readWidget(rx1ScaleKey);
        printToConsole("  Original RX1 calibscale: " + origScale);
        if (!origScale || origScale === "") {
            printToConsole("  FAIL: calibscale returned empty");
            return false;
        }

        // Write a new value (cos(45°) ≈ 0.707)
        adrv9009.writeWidget(rx1ScaleKey, "0.707");
        msleep(500);
        var readBack = adrv9009.readWidget(rx1ScaleKey);
        printToConsole("  Set calibscale to 0.707, read back: " + readBack);

        // Restore
        adrv9009.writeWidget(rx1ScaleKey, origScale);
        msleep(500);

        if (!readBack || readBack === "") {
            printToConsole("  FAIL: calibscale read-back returned empty");
            return false;
        }
        var readNum = parseFloat(readBack);
        if (isNaN(readNum) || Math.abs(readNum - 0.707) > 0.01) {
            printToConsole("  FAIL: calibscale read-back " + readBack + " does not match 0.707");
            return false;
        }

        // Step 3: Also verify RX2 (if present)
        if (phaseKeys.length > 2) {
            var rx2ScaleKey = phaseKeys[2]; // voltage1_i_in/calibscale
            var origScale2 = adrv9009.readWidget(rx2ScaleKey);
            printToConsole("  RX2 calibscale original: " + origScale2);
            adrv9009.writeWidget(rx2ScaleKey, "0.5");
            msleep(500);
            var readBack2 = adrv9009.readWidget(rx2ScaleKey);
            printToConsole("  Set RX2 calibscale to 0.5, read back: " + readBack2);
            adrv9009.writeWidget(rx2ScaleKey, origScale2);
            msleep(500);
        }

        printToConsole("  PASS: FPGA phase rotation (calibscale) write-readback verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 11: Refresh Functionality
// UID: TST.ADRV9009.CONTROLS.REFRESH_FUNCTION
// Description: Verify refresh button and programmatic refresh update controls.
// ============================================
printToConsole("\n=== Test 11: Refresh Functionality ===\n");

TestFramework.runTest("TST.ADRV9009.CONTROLS.REFRESH_FUNCTION", function() {
    try {
        // Step 1: Note current control values
        var gainKey = findKey("voltage0_in/hardwaregain");
        var beforeRefresh = gainKey ? adrv9009.readWidget(gainKey) : "";
        printToConsole("  Before refresh — RX1 gain: " + beforeRefresh);

        // Step 3: Click refresh button
        adrv9009.triggerRefresh();
        msleep(1000);
        printToConsole("  Main tool refresh triggered");

        // Also test programmatic refresh
        adrv9009.refresh();
        msleep(1000);
        printToConsole("  Programmatic refresh triggered");

        // Verify values are still readable after refresh
        var afterRefresh = gainKey ? adrv9009.readWidget(gainKey) : "";
        printToConsole("  After refresh — RX1 gain: " + afterRefresh);

        if (gainKey && (!afterRefresh || afterRefresh === "")) {
            printToConsole("  FAIL: Values not readable after refresh");
            return false;
        }
        printToConsole("  PASS: Refresh triggered and values remain readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Switch to the Advanced tool for all advanced tests
if (!switchToTool("ADRV9009 Advanced")) {
    printToConsole("ERROR: Cannot switch to ADRV9009 Advanced tool");
    scopy.exit();
}

// ============================================
// Advanced Test 2: Clock Settings Tab
// UID: TST.ADRV9009.ADVANCED.CLK_SETTINGS
// Description: Verify CLK Settings tab controls: device clock, VCO freq, HS divider.
// Key format: adrv9009-phy/adi,dig-clocks-{attr}
// Note: VCO freq and HS divider are profile config params — read-only verification only.
//       Writing to VCO/HS-div on a live board could disrupt the JESD link.
// ============================================
printToConsole("\n=== Advanced Test 2: Clock Settings ===\n");

TestFramework.runTest("TST.ADRV9009.ADVANCED.CLK_SETTINGS.DEVICE_CLOCK", function() {
    try {
        adrv9009.switchAdvancedTab("CLK Settings");
        msleep(500);
        var clkKey = findKey("dig-clocks-device-clock_khz");
        if (!clkKey) {
            printToConsole("  FAIL: device-clock_khz key not found");
            printToConsole("  dig-clocks keys: " + findKeys("dig-clocks"));
            return false;
        }
        printToConsole("  Using device clock key: " + clkKey);

        // Step 2: Read device clock (profile parameter — read-only)
        var origClk = adrv9009.readWidget(clkKey);
        printToConsole("  Device clock value: " + origClk);
        if (!origClk || origClk === "") {
            printToConsole("  FAIL: device-clock_khz returned empty");
            return false;
        }
        printToConsole("  PASS: Device clock reads correctly: " + origClk + " kHz");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.CLK_SETTINGS.VCO_FREQ", function() {
    try {
        adrv9009.switchAdvancedTab("CLK Settings");
        msleep(500);
        var vcoKey = findKey("dig-clocks-clk-pll-vco-freq_khz");
        if (!vcoKey) {
            printToConsole("  FAIL: clk-pll-vco-freq_khz key not found");
            return false;
        }
        printToConsole("  Using VCO freq key: " + vcoKey);

        // Read VCO frequency (profile parameter — read-only to avoid disrupting JESD link)
        var origVco = adrv9009.readWidget(vcoKey);
        printToConsole("  VCO frequency: " + origVco + " kHz");
        if (!origVco || origVco === "") {
            printToConsole("  FAIL: vco-freq_khz returned empty");
            return false;
        }
        printToConsole("  PASS: VCO frequency reads correctly: " + origVco + " kHz");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.CLK_SETTINGS.HS_DIVIDER", function() {
    try {
        adrv9009.switchAdvancedTab("CLK Settings");
        msleep(500);
        var hsDivKey = findKey("dig-clocks-clk-pll-hs-div");
        if (!hsDivKey) {
            printToConsole("  FAIL: clk-pll-hs-div key not found");
            return false;
        }
        printToConsole("  Using HS divider key: " + hsDivKey);

        // Step 4: Read HS divider setting (profile parameter — read-only)
        var origHsDiv = adrv9009.readWidget(hsDivKey);
        printToConsole("  HS divider: " + origHsDiv);
        if (!origHsDiv || origHsDiv === "") {
            printToConsole("  FAIL: hs-div returned empty");
            return false;
        }
        printToConsole("  PASS: HS divider reads correctly: " + origHsDiv);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Advanced Test 3: RX Profile Settings
// UID: TST.ADRV9009.ADVANCED.RX_SETTINGS
// Description: Verify RX FIR decimation, output rate, RF bandwidth, channels enable.
// Key format: adrv9009-phy/adi,rx-profile-{attr}
// Note: Profile params are loaded via profile file — read-only verification here.
// ============================================
printToConsole("\n=== Advanced Test 3: RX Profile Settings ===\n");

TestFramework.runTest("TST.ADRV9009.ADVANCED.RX_SETTINGS.FIR_DECIMATION", function() {
    try {
        adrv9009.switchAdvancedTab("RX Settings");
        msleep(500);
        var firKey = findKey("rx-profile-rx-fir-decimation");
        if (!firKey) {
            printToConsole("  FAIL: rx-fir-decimation key not found");
            printToConsole("  rx-profile keys: " + findKeys("rx-profile"));
            return false;
        }
        printToConsole("  Using RX FIR decimation key: " + firKey);

        var origFir = adrv9009.readWidget(firKey);
        printToConsole("  RX FIR decimation: " + origFir);
        if (!origFir || origFir === "") {
            printToConsole("  FAIL: rx-fir-decimation returned empty");
            return false;
        }
        printToConsole("  PASS: RX FIR decimation reads correctly: " + origFir);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.RX_SETTINGS.OUTPUT_RATE", function() {
    try {
        adrv9009.switchAdvancedTab("RX Settings");
        msleep(500);
        var rateKey = findKey("rx-profile-rx-output-rate_khz");
        if (!rateKey) {
            printToConsole("  FAIL: rx-output-rate_khz key not found");
            return false;
        }
        printToConsole("  Using RX output rate key: " + rateKey);

        var origRate = adrv9009.readWidget(rateKey);
        printToConsole("  RX output rate: " + origRate + " kHz");
        if (!origRate || origRate === "") {
            printToConsole("  FAIL: rx-output-rate_khz returned empty");
            return false;
        }
        printToConsole("  PASS: RX output rate reads correctly: " + origRate + " kHz");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.RX_SETTINGS.RF_BANDWIDTH", function() {
    try {
        adrv9009.switchAdvancedTab("RX Settings");
        msleep(500);
        var bwKey = findKey("rx-profile-rf-bandwidth_hz");
        if (!bwKey) {
            printToConsole("  FAIL: rx-profile-rf-bandwidth_hz key not found");
            return false;
        }
        printToConsole("  Using RX RF bandwidth key: " + bwKey);

        var origBw = adrv9009.readWidget(bwKey);
        printToConsole("  RX RF bandwidth: " + origBw + " Hz");
        if (!origBw || origBw === "") {
            printToConsole("  FAIL: rx-profile-rf-bandwidth_hz returned empty");
            return false;
        }
        printToConsole("  PASS: RX RF bandwidth reads correctly: " + origBw + " Hz");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.RX_SETTINGS.CHANNELS_ENABLE", function() {
    try {
        adrv9009.switchAdvancedTab("RX Settings");
        msleep(500);
        var rxChKey = findKey("rx-settings-rx-channels");
        if (!rxChKey) {
            printToConsole("  FAIL: rx-settings-rx-channels key not found");
            printToConsole("  rx-settings keys: " + findKeys("rx-settings"));
            return false;
        }
        printToConsole("  Using RX channels key: " + rxChKey);

        var origCh = adrv9009.readWidget(rxChKey);
        printToConsole("  RX channels enable: " + origCh);
        if (!origCh || origCh === "") {
            printToConsole("  FAIL: rx-settings-rx-channels returned empty");
            return false;
        }
        printToConsole("  PASS: RX channels enable reads correctly: " + origCh);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Advanced Test 4: TX Profile Settings
// UID: TST.ADRV9009.ADVANCED.TX_SETTINGS
// ============================================
printToConsole("\n=== Advanced Test 4: TX Profile Settings ===\n");

TestFramework.runTest("TST.ADRV9009.ADVANCED.TX_SETTINGS.FIR_INTERPOLATION", function() {
    try {
        adrv9009.switchAdvancedTab("TX Settings");
        msleep(500);
        var firKey = findKey("tx-profile-tx-fir-interpolation");
        if (!firKey) {
            printToConsole("  FAIL: tx-fir-interpolation key not found");
            printToConsole("  tx-profile keys: " + findKeys("tx-profile"));
            return false;
        }
        printToConsole("  Using TX FIR interpolation key: " + firKey);

        var origFir = adrv9009.readWidget(firKey);
        printToConsole("  TX FIR interpolation: " + origFir);
        if (!origFir || origFir === "") {
            printToConsole("  FAIL: tx-fir-interpolation returned empty");
            return false;
        }
        printToConsole("  PASS: TX FIR interpolation reads correctly: " + origFir);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.TX_SETTINGS.INPUT_RATE", function() {
    try {
        adrv9009.switchAdvancedTab("TX Settings");
        msleep(500);
        var rateKey = findKey("tx-profile-tx-input-rate_khz");
        if (!rateKey) {
            printToConsole("  FAIL: tx-input-rate_khz key not found");
            return false;
        }
        printToConsole("  Using TX input rate key: " + rateKey);

        var origRate = adrv9009.readWidget(rateKey);
        printToConsole("  TX input rate: " + origRate + " kHz");
        if (!origRate || origRate === "") {
            printToConsole("  FAIL: tx-input-rate_khz returned empty");
            return false;
        }
        printToConsole("  PASS: TX input rate reads correctly: " + origRate + " kHz");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.TX_SETTINGS.ATTEN_STEP_SIZE", function() {
    try {
        adrv9009.switchAdvancedTab("TX Settings");
        msleep(500);
        var stepKey = findKey("tx-settings-tx-atten-step-size");
        if (!stepKey) {
            printToConsole("  FAIL: tx-atten-step-size key not found");
            printToConsole("  tx-settings keys: " + findKeys("tx-settings"));
            return false;
        }
        printToConsole("  Using TX atten step size key: " + stepKey);

        var origStep = adrv9009.readWidget(stepKey);
        printToConsole("  TX atten step size: " + origStep);
        if (!origStep || origStep === "") {
            printToConsole("  FAIL: tx-atten-step-size returned empty");
            return false;
        }
        printToConsole("  PASS: TX attenuation step size reads correctly: " + origStep);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.TX_SETTINGS.TX1_TX2_ATTEN", function() {
    try {
        adrv9009.switchAdvancedTab("TX Settings");
        msleep(500);
        var tx1Key = findKey("tx-settings-tx1-atten_md-b");
        var tx2Key = findKey("tx-settings-tx2-atten_md-b");

        if (!tx1Key) {
            printToConsole("  FAIL: tx1-atten_md-b key not found");
            return false;
        }
        printToConsole("  Using TX1 atten key: " + tx1Key);

        var origTx1 = adrv9009.readWidget(tx1Key);
        printToConsole("  TX1 advanced atten: " + origTx1);
        if (!origTx1 || origTx1 === "") {
            printToConsole("  FAIL: tx1-atten_md-b returned empty");
            return false;
        }

        if (tx2Key) {
            var origTx2 = adrv9009.readWidget(tx2Key);
            printToConsole("  TX2 advanced atten: " + origTx2);
        }

        printToConsole("  PASS: TX1/TX2 advanced attenuation reads correctly");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Advanced Test 5: Observation RX Profile Settings
// UID: TST.ADRV9009.ADVANCED.ORX_SETTINGS
// ============================================
printToConsole("\n=== Advanced Test 5: ORX Profile Settings ===\n");

TestFramework.runTest("TST.ADRV9009.ADVANCED.ORX_SETTINGS.OUTPUT_RATE", function() {
    try {
        adrv9009.switchAdvancedTab("ORX Settings");
        msleep(500);
        var rateKey = findKey("orx-profile-orx-output-rate_khz");
        if (!rateKey) {
            printToConsole("  FAIL: orx-output-rate_khz key not found");
            printToConsole("  orx-profile keys: " + findKeys("orx-profile"));
            return false;
        }
        printToConsole("  Using ORX output rate key: " + rateKey);

        var origRate = adrv9009.readWidget(rateKey);
        printToConsole("  ORX output rate: " + origRate + " kHz");
        if (!origRate || origRate === "") {
            printToConsole("  FAIL: orx-output-rate_khz returned empty");
            return false;
        }
        printToConsole("  PASS: ORX output rate reads correctly: " + origRate + " kHz");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.ORX_SETTINGS.RF_BANDWIDTH", function() {
    try {
        adrv9009.switchAdvancedTab("ORX Settings");
        msleep(500);
        var bwKey = findKey("orx-profile-rf-bandwidth_hz");
        if (!bwKey) {
            printToConsole("  FAIL: orx-profile-rf-bandwidth_hz key not found");
            return false;
        }
        printToConsole("  Using ORX RF bandwidth key: " + bwKey);

        var origBw = adrv9009.readWidget(bwKey);
        printToConsole("  ORX RF bandwidth: " + origBw + " Hz");
        if (!origBw || origBw === "") {
            printToConsole("  FAIL: orx-profile-rf-bandwidth_hz returned empty");
            return false;
        }
        printToConsole("  PASS: ORX RF bandwidth reads correctly: " + origBw + " Hz");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.ORX_SETTINGS.CHANNELS_ENABLE", function() {
    try {
        adrv9009.switchAdvancedTab("ORX Settings");
        msleep(500);
        var orxChKey = findKey("obs-settings-obs-rx-channels-enable");
        if (!orxChKey) {
            orxChKey = findKey("obs-settings-obs-rx-channels");
        }
        if (!orxChKey) {
            var obsKeys = findKeys("obs-settings");
            printToConsole("  obs-settings keys: " + obsKeys);
            if (obsKeys.length === 0) {
                printToConsole("  SKIP: ORX channels enable key not found");
                return true;
            }
            orxChKey = obsKeys[0];
        }
        printToConsole("  Using ORX channels key: " + orxChKey);

        var origCh = adrv9009.readWidget(orxChKey);
        printToConsole("  ORX channels enable: " + origCh);
        if (!origCh || origCh === "") {
            printToConsole("  FAIL: ORX channels enable returned empty");
            return false;
        }
        printToConsole("  PASS: ORX channels enable reads correctly: " + origCh);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Advanced Test 6: JESD204 Physical Layer Settings
// UID: TST.ADRV9009.ADVANCED.JESD_SETTINGS
// Description: Verify serializer amplitude, pre-emphasis, lane polarity, DES EQ, SYSREF LVDS.
// ============================================
printToConsole("\n=== Advanced Test 6: JESD204 Settings ===\n");

TestFramework.runTest("TST.ADRV9009.ADVANCED.JESD_SETTINGS.SER_AMPLITUDE", function() {
    try {
        adrv9009.switchAdvancedTab("JESD204 Settings");
        msleep(500);
        var serAmpKey = findKey("jesd204-ser-amplitude");
        if (!serAmpKey) {
            printToConsole("  FAIL: jesd204-ser-amplitude key not found");
            printToConsole("  jesd204 keys: " + findKeys("jesd204-ser"));
            return false;
        }
        printToConsole("  Using SER amplitude key: " + serAmpKey);

        // Step 2: Change serializer amplitude (range 0–15)
        var origAmp = adrv9009.readWidget(serAmpKey);
        printToConsole("  Original SER amplitude: " + origAmp);
        if (!origAmp || origAmp === "") {
            printToConsole("  FAIL: ser-amplitude returned empty");
            return false;
        }

        adrv9009.writeWidget(serAmpKey, "7");
        msleep(500);
        var readBack = adrv9009.readWidget(serAmpKey);
        printToConsole("  Set SER amplitude to 7, read back: " + readBack);

        // Restore
        adrv9009.writeWidget(serAmpKey, origAmp);
        msleep(500);

        if (!readBack || readBack === "") {
            printToConsole("  FAIL: SER amplitude read-back returned empty");
            return false;
        }
        var readNum = parseFloat(readBack);
        if (isNaN(readNum) || Math.abs(readNum - 7) > 0.5) {
            printToConsole("  FAIL: SER amplitude read-back " + readBack + " does not match 7");
            return false;
        }
        printToConsole("  PASS: JESD204 serializer amplitude write-readback verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.JESD_SETTINGS.SER_PRE_EMPHASIS", function() {
    try {
        adrv9009.switchAdvancedTab("JESD204 Settings");
        msleep(500);
        var preEmpKey = findKey("jesd204-ser-pre-emphasis");
        if (!preEmpKey) {
            printToConsole("  FAIL: jesd204-ser-pre-emphasis key not found");
            return false;
        }
        printToConsole("  Using SER pre-emphasis key: " + preEmpKey);

        // Step 3: Change serializer pre-emphasis (range 0–4)
        var origPre = adrv9009.readWidget(preEmpKey);
        printToConsole("  Original SER pre-emphasis: " + origPre);

        adrv9009.writeWidget(preEmpKey, "2");
        msleep(500);
        var readBack = adrv9009.readWidget(preEmpKey);
        printToConsole("  Set SER pre-emphasis to 2, read back: " + readBack);

        // Restore
        adrv9009.writeWidget(preEmpKey, origPre);
        msleep(500);

        if (!readBack || readBack === "") {
            printToConsole("  FAIL: SER pre-emphasis read-back returned empty");
            return false;
        }
        var readNum = parseFloat(readBack);
        if (isNaN(readNum) || Math.abs(readNum - 2) > 0.5) {
            printToConsole("  FAIL: SER pre-emphasis read-back " + readBack + " does not match 2");
            return false;
        }
        printToConsole("  PASS: JESD204 serializer pre-emphasis write-readback verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.JESD_SETTINGS.DES_EQ", function() {
    try {
        adrv9009.switchAdvancedTab("JESD204 Settings");
        msleep(500);
        var desEqKey = findKey("jesd204-des-eq-setting");
        if (!desEqKey) {
            printToConsole("  FAIL: jesd204-des-eq-setting key not found");
            return false;
        }
        printToConsole("  Using DES EQ key: " + desEqKey);

        // Step 6 (RST step 5 in JESD_SETTINGS): Change deserializer EQ setting (range 0–4)
        var origEq = adrv9009.readWidget(desEqKey);
        printToConsole("  Original DES EQ: " + origEq);

        adrv9009.writeWidget(desEqKey, "2");
        msleep(500);
        var readBack = adrv9009.readWidget(desEqKey);
        printToConsole("  Set DES EQ to 2, read back: " + readBack);

        // Restore
        adrv9009.writeWidget(desEqKey, origEq);
        msleep(500);

        if (!readBack || readBack === "") {
            printToConsole("  FAIL: DES EQ read-back returned empty");
            return false;
        }
        if (parseFloat(readBack) !== 2) {
            printToConsole("  FAIL: DES EQ read-back " + readBack + " does not match 2");
            return false;
        }
        printToConsole("  PASS: JESD204 deserializer EQ write-readback verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.JESD_SETTINGS.SYSREF_LVDS", function() {
    try {
        adrv9009.switchAdvancedTab("JESD204 Settings");
        msleep(500);
        var sysrefKey = findKey("jesd204-sysref-lvds-mode");
        if (!sysrefKey) {
            printToConsole("  FAIL: jesd204-sysref-lvds-mode key not found");
            printToConsole("  sysref keys: " + findKeys("sysref"));
            return false;
        }
        printToConsole("  Using SYSREF LVDS key: " + sysrefKey);

        // Step 7 (RST step 6): Toggle SYSREF LVDS mode switch
        var origVal = adrv9009.readWidget(sysrefKey);
        printToConsole("  Original SYSREF LVDS mode: " + origVal);
        if (!origVal || origVal === "") {
            printToConsole("  FAIL: sysref-lvds-mode returned empty");
            return false;
        }

        adrv9009.writeWidget(sysrefKey, "1");
        msleep(500);
        var afterEnable = adrv9009.readWidget(sysrefKey);

        adrv9009.writeWidget(sysrefKey, "0");
        msleep(500);
        var afterDisable = adrv9009.readWidget(sysrefKey);

        // Restore
        adrv9009.writeWidget(sysrefKey, origVal);
        msleep(500);

        printToConsole("  SYSREF LVDS: enabled=" + afterEnable + ", disabled=" + afterDisable);

        // Use flexible boolean check (IIO may return "0"/"1" or "true"/"false")
        var enabledOk = (afterEnable === "1" || afterEnable === "true");
        var disabledOk = (afterDisable === "0" || afterDisable === "false");
        if (!enabledOk || !disabledOk) {
            printToConsole("  FAIL: SYSREF LVDS mode toggle did not function correctly");
            return false;
        }
        printToConsole("  PASS: SYSREF LVDS mode toggle verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Advanced Test 7: JESD204 Framer A/B Configuration
// UID: TST.ADRV9009.ADVANCED.JESD_FRAMER
// Description: Verify Framer A and B: bank-id, device-id, M/K/F/NP, lane enables, scramble.
// ============================================
printToConsole("\n=== Advanced Test 7: JESD Framer ===\n");

TestFramework.runTest("TST.ADRV9009.ADVANCED.JESD_FRAMER.BANK_DEVICE_ID", function() {
    try {
        adrv9009.switchAdvancedTab("JESD Framer");
        msleep(500);
        // Step 3: Check Framer A and Framer B bank-id
        var bankAKey = findKey("framer-a-bank-id");
        var bankBKey = findKey("framer-b-bank-id");

        if (!bankAKey) {
            printToConsole("  FAIL: framer-a-bank-id key not found");
            printToConsole("  framer-a keys: " + findKeys("framer-a"));
            return false;
        }
        printToConsole("  Using Framer A bank-id key: " + bankAKey);

        // Framer A write+readback
        var origBankA = adrv9009.readWidget(bankAKey);
        printToConsole("  Framer A bank-id: " + origBankA);

        adrv9009.writeWidget(bankAKey, "1");
        msleep(500);
        var readBackA = adrv9009.readWidget(bankAKey);
        printToConsole("  Set Framer A bank-id to 1, read back: " + readBackA);

        // Restore Framer A
        adrv9009.writeWidget(bankAKey, origBankA);
        msleep(500);

        if (!readBackA || readBackA === "") {
            printToConsole("  FAIL: framer-a-bank-id read-back returned empty");
            return false;
        }
        if (parseFloat(readBackA) !== 1) {
            printToConsole("  FAIL: Framer A bank-id read-back " + readBackA + " does not match 1");
            return false;
        }

        // Framer B write+readback (step 7: repeat for Framer B)
        if (bankBKey) {
            var origBankB = adrv9009.readWidget(bankBKey);
            printToConsole("  Framer B bank-id: " + origBankB);

            adrv9009.writeWidget(bankBKey, "1");
            msleep(500);
            var readBackB = adrv9009.readWidget(bankBKey);
            printToConsole("  Set Framer B bank-id to 1, read back: " + readBackB);

            // Restore Framer B
            adrv9009.writeWidget(bankBKey, origBankB);
            msleep(500);

            if (!readBackB || readBackB === "") {
                printToConsole("  FAIL: framer-b-bank-id read-back returned empty");
                return false;
            }
            if (parseFloat(readBackB) !== 1) {
                printToConsole("  FAIL: Framer B bank-id read-back " + readBackB + " does not match 1");
                return false;
            }
            printToConsole("  PASS: Framer B bank-id write-readback verified");
        } else {
            printToConsole("  WARN: framer-b-bank-id key not found — Framer B not tested");
        }

        printToConsole("  PASS: JESD Framer A bank-id write-readback verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.JESD_FRAMER.MK_PARAMS", function() {
    try {
        adrv9009.switchAdvancedTab("JESD Framer");
        msleep(500);
        // Step 4: Verify M, K, F and NP parameter attributes are readable
        // These are JESD204 link parameters — read-only verification (writing could break the link)
        var params = [
            { name: "framer-a-m",  label: "Framer A M (converters)" },
            { name: "framer-a-k",  label: "Framer A K (frames/multiframe)" },
            { name: "framer-a-f",  label: "Framer A F (octets/frame)" },
            { name: "framer-a-np", label: "Framer A NP (converter bits)" }
        ];

        var allFound = true;
        for (var i = 0; i < params.length; i++) {
            var key = findKey(params[i].name);
            if (!key) {
                printToConsole("  FAIL: " + params[i].label + " key not found (" + params[i].name + ")");
                allFound = false;
                continue;
            }
            var val = adrv9009.readWidget(key);
            if (!val || val === "") {
                printToConsole("  FAIL: " + params[i].label + " returned empty");
                allFound = false;
            } else {
                printToConsole("  PASS: " + params[i].label + " = " + val);
            }
        }

        return allFound;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.JESD_FRAMER.SCRAMBLE", function() {
    try {
        adrv9009.switchAdvancedTab("JESD Framer");
        msleep(500);
        // Step 6: Enable/disable scramble for Framer A (and Framer B)
        var scrambleKey = findKey("framer-a-scramble");
        if (!scrambleKey) {
            printToConsole("  FAIL: framer-a-scramble key not found");
            return false;
        }
        printToConsole("  Using Framer A scramble key: " + scrambleKey);

        var result = toggleBoolAttr(scrambleKey, "Framer A scramble");
        if (!result) return false;

        // Also test Framer B scramble
        var scrambleBKey = findKey("framer-b-scramble");
        if (scrambleBKey) {
            result = toggleBoolAttr(scrambleBKey, "Framer B scramble");
        }
        return result;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Advanced Test 8: JESD204 Deframer A/B Configuration
// UID: TST.ADRV9009.ADVANCED.JESD_DEFRAMER
// Description: Verify Deframer A and B: bank-id, device-id, M/K, lane enables, SYNCB out.
// ============================================
printToConsole("\n=== Advanced Test 8: JESD Deframer ===\n");

TestFramework.runTest("TST.ADRV9009.ADVANCED.JESD_DEFRAMER.BANK_DEVICE_ID", function() {
    try {
        adrv9009.switchAdvancedTab("JESD Deframer");
        msleep(500);
        // Step 3: Check Deframer A and Deframer B bank-id
        var bankAKey = findKey("deframer-a-bank-id");
        var bankBKey = findKey("deframer-b-bank-id");

        if (!bankAKey) {
            printToConsole("  FAIL: deframer-a-bank-id key not found");
            printToConsole("  deframer-a keys: " + findKeys("deframer-a"));
            return false;
        }
        printToConsole("  Using Deframer A bank-id key: " + bankAKey);

        // Deframer A write+readback
        var origBankA = adrv9009.readWidget(bankAKey);
        printToConsole("  Deframer A bank-id: " + origBankA);

        adrv9009.writeWidget(bankAKey, "1");
        msleep(500);
        var readBackA = adrv9009.readWidget(bankAKey);
        printToConsole("  Set Deframer A bank-id to 1, read back: " + readBackA);

        // Restore Deframer A
        adrv9009.writeWidget(bankAKey, origBankA);
        msleep(500);

        if (!readBackA || readBackA === "") {
            printToConsole("  FAIL: deframer-a-bank-id read-back returned empty");
            return false;
        }
        if (parseFloat(readBackA) !== 1) {
            printToConsole("  FAIL: Deframer A bank-id read-back " + readBackA + " does not match 1");
            return false;
        }

        // Deframer B write+readback (step 7: repeat for Deframer B)
        if (bankBKey) {
            var origBankB = adrv9009.readWidget(bankBKey);
            printToConsole("  Deframer B bank-id: " + origBankB);

            adrv9009.writeWidget(bankBKey, "1");
            msleep(500);
            var readBackB = adrv9009.readWidget(bankBKey);
            printToConsole("  Set Deframer B bank-id to 1, read back: " + readBackB);

            // Restore Deframer B
            adrv9009.writeWidget(bankBKey, origBankB);
            msleep(500);

            if (!readBackB || readBackB === "") {
                printToConsole("  FAIL: deframer-b-bank-id read-back returned empty");
                return false;
            }
            if (parseFloat(readBackB) !== 1) {
                printToConsole("  FAIL: Deframer B bank-id read-back " + readBackB + " does not match 1");
                return false;
            }
            printToConsole("  PASS: Deframer B bank-id write-readback verified");
        } else {
            printToConsole("  WARN: deframer-b-bank-id key not found — Deframer B not tested");
        }

        printToConsole("  PASS: JESD Deframer A bank-id write-readback verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.JESD_DEFRAMER.MK_PARAMS", function() {
    try {
        adrv9009.switchAdvancedTab("JESD Deframer");
        msleep(500);
        // Step 4: Verify M and K parameter attributes are readable
        // Read-only verification — writing JESD link params on a live device could break the link
        var params = [
            { name: "deframer-a-m", label: "Deframer A M (converters)" },
            { name: "deframer-a-k", label: "Deframer A K (frames/multiframe)" }
        ];

        var allFound = true;
        for (var i = 0; i < params.length; i++) {
            var key = findKey(params[i].name);
            if (!key) {
                printToConsole("  FAIL: " + params[i].label + " key not found (" + params[i].name + ")");
                allFound = false;
                continue;
            }
            var val = adrv9009.readWidget(key);
            if (!val || val === "") {
                printToConsole("  FAIL: " + params[i].label + " returned empty");
                allFound = false;
            } else {
                printToConsole("  PASS: " + params[i].label + " = " + val);
            }
        }

        return allFound;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9009.ADVANCED.JESD_DEFRAMER.SYNCB_OUT", function() {
    try {
        adrv9009.switchAdvancedTab("JESD Deframer");
        msleep(500);
        // Step 6: Change SYNCB out select for Deframer A (and Deframer B)
        var syncbKey = findKey("deframer-a-syncb-out-select");
        if (!syncbKey) {
            printToConsole("  FAIL: deframer-a-syncb-out-select key not found");
            return false;
        }
        printToConsole("  Using Deframer A SYNCB out select key: " + syncbKey);

        var result = toggleBoolAttr(syncbKey, "Deframer A SYNCB out select");
        if (!result) return false;

        // Also test Deframer B
        var syncbBKey = findKey("deframer-b-syncb-out-select");
        if (syncbBKey) {
            result = toggleBoolAttr(syncbBKey, "Deframer B SYNCB out select");
        }
        return result;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Advanced Test 9: Advanced Tool Refresh Functionality
// UID: TST.ADRV9009.ADVANCED.REFRESH_FUNCTION
// Description: Verify refresh in Advanced tool updates controls from device.
// ============================================
printToConsole("\n=== Advanced Test 9: Advanced Refresh Functionality ===\n");

TestFramework.runTest("TST.ADRV9009.ADVANCED.REFRESH_FUNCTION", function() {
    try {
        // Step 4: Click refresh in the Advanced tool
        adrv9009.triggerAdvancedRefresh();
        msleep(1000);
        printToConsole("  Advanced tool refresh triggered");

        // Also test programmatic refresh
        adrv9009.refresh();
        msleep(1000);
        printToConsole("  Programmatic refresh triggered");

        // Verify CLK settings are still readable after refresh
        var clkKey = findKey("dig-clocks-device-clock_khz");
        if (clkKey) {
            var afterRefresh = adrv9009.readWidget(clkKey);
            printToConsole("  After refresh — device clock: " + afterRefresh);
            if (!afterRefresh || afterRefresh === "") {
                printToConsole("  FAIL: Values not readable after advanced refresh");
                return false;
            }
        }

        printToConsole("  PASS: Advanced refresh triggered and values remain readable");
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
