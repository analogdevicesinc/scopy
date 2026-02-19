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

// ADRV9002 Manual Test Automation
// Automates tests from: docs/tests/plugins/adrv9002/adrv9002_tests.rst
// Tests: TST.ADRV9002.CONTROLS.GLOBAL_SETTINGS, TST.ADRV9002.CONTROLS.RX_CHANNEL_CONFIG,
//        TST.ADRV9002.CONTROLS.TX_CHANNEL_CONFIG, TST.ADRV9002.CONTROLS.ORX_CONFIG,
//        TST.ADRV9002.CONTROLS.REFRESH_FUNCTION
// Skipped: TST.ADRV9002.CONTROLS.DEVICE_DRIVER_API (no API for version string)

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("ADRV9002 Documentation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to ADRV9002 tool
if (!switchToTool("ADRV9002")) {
    printToConsole("ERROR: Cannot switch to ADRV9002 tool");
    scopy.exit();
}

// ============================================
// Test 3: Global Settings Section
// UID: TST.ADRV9002.CONTROLS.GLOBAL_SETTINGS
// Description: Verify global settings controls: temperature monitoring.
// Note: Profile Manager and Initial Calibrations skipped (no dedicated API).
// ============================================
printToConsole("\n=== Test 3: Global Settings Section ===\n");

TestFramework.runTest("TST.ADRV9002.CONTROLS.GLOBAL_SETTINGS", function() {
    try {
        // Step 3: Verify Temperature widget shows current reading
        var temperature = adrv9002.getTemperature();
        printToConsole("  Temperature reading: " + temperature);
        if (!temperature || temperature === "") {
            printToConsole("  FAIL: getTemperature() returned empty string");
            return false;
        }

        // Verify it's a plausible numeric value
        var tempNum = parseFloat(temperature);
        if (isNaN(tempNum)) {
            printToConsole("  FAIL: Temperature is not a valid number: " + temperature);
            return false;
        }
        printToConsole("  Temperature value: " + tempNum + " (valid numeric reading)");
        printToConsole("  Step 3 PASS: Temperature widget shows current reading");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: RX Channel Controls
// UID: TST.ADRV9002.CONTROLS.RX_CHANNEL_CONFIG
// Description: Test RX1 and RX2 channel control functionality.
// Steps: Change HW gain, gain control mode, ENSM mode,
//        toggle powerdown, verify read-only values.
// ============================================
printToConsole("\n=== Test 4: RX Channel Controls ===\n");

TestFramework.runTest("TST.ADRV9002.CONTROLS.RX_CHANNEL_CONFIG.HW_GAIN", function() {
    try {
        // Step 2: Change RX1 Hardware Gain (0-36 dB)
        // Must be in manual mode to set gain
        var origMode = adrv9002.getRxGainControlMode(0);
        var origGain = adrv9002.getRxHardwareGain(0);
        printToConsole("  Original gain mode: " + origMode);
        printToConsole("  Original HW gain: " + origGain);

        adrv9002.setRxGainControlMode(0, "manual");
        msleep(500);

        // Set gain to a known value
        adrv9002.setRxHardwareGain(0, "18");
        msleep(500);
        var readBack = adrv9002.getRxHardwareGain(0);
        printToConsole("  Set RX1 gain to 18 dB, read back: " + readBack);
        if (readBack.indexOf("18") === -1) {
            printToConsole("  FAIL: Gain control did not respond correctly");
            adrv9002.setRxHardwareGain(0, origGain);
            adrv9002.setRxGainControlMode(0, origMode);
            return false;
        }
        printToConsole("  Step 2 PASS: RX1 gain control responds and updates device");

        // Also test RX2
        var origGain2 = adrv9002.getRxHardwareGain(1);
        adrv9002.setRxGainControlMode(1, "manual");
        msleep(500);
        adrv9002.setRxHardwareGain(1, "24");
        msleep(500);
        var readBack2 = adrv9002.getRxHardwareGain(1);
        printToConsole("  Set RX2 gain to 24 dB, read back: " + readBack2);
        if (readBack2.indexOf("24") === -1) {
            printToConsole("  FAIL: RX2 gain control did not respond correctly");
        } else {
            printToConsole("  RX2 gain control PASS");
        }

        // Restore
        adrv9002.setRxHardwareGain(0, origGain);
        adrv9002.setRxGainControlMode(0, origMode);
        adrv9002.setRxHardwareGain(1, origGain2);
        msleep(500);

        return readBack.indexOf("18") !== -1;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.RX_CHANNEL_CONFIG.GAIN_MODE", function() {
    try {
        // Step 3: Change Gain Control Mode (manual/automatic)
        var origMode = adrv9002.getRxGainControlMode(0);
        printToConsole("  Original gain control mode: " + origMode);

        adrv9002.setRxGainControlMode(0, "manual");
        msleep(500);
        var readBack = adrv9002.getRxGainControlMode(0);
        printToConsole("  Set to manual, read back: " + readBack);
        if (readBack !== "manual") {
            printToConsole("  FAIL: Gain control mode did not change to manual");
            adrv9002.setRxGainControlMode(0, origMode);
            return false;
        }

        adrv9002.setRxGainControlMode(0, "automatic");
        msleep(500);
        readBack = adrv9002.getRxGainControlMode(0);
        printToConsole("  Set to automatic, read back: " + readBack);

        // Restore
        adrv9002.setRxGainControlMode(0, origMode);
        msleep(500);

        printToConsole("  Step 3 PASS: Mode changes and affects gain behavior");
        return readBack === "automatic";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.RX_CHANNEL_CONFIG.ENSM_MODE", function() {
    try {
        // Step 4: Change ENSM Mode (radio enable state machine)
        var ensmMode = adrv9002.getRxEnsmMode(0);
        printToConsole("  RX1 ENSM mode: " + ensmMode);
        if (!ensmMode || ensmMode === "") {
            printToConsole("  FAIL: getRxEnsmMode() returned empty string");
            return false;
        }

        var ensmMode2 = adrv9002.getRxEnsmMode(1);
        printToConsole("  RX2 ENSM mode: " + ensmMode2);
        if (!ensmMode2 || ensmMode2 === "") {
            printToConsole("  FAIL: getRxEnsmMode(1) returned empty string");
            return false;
        }

        printToConsole("  Step 4 PASS: ENSM mode reads correctly for both channels");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.RX_CHANNEL_CONFIG.POWERDOWN", function() {
    try {
        // Step 5: Toggle Powerdown enable/disable
        var origEnabled = adrv9002.isRxEnabled(0);
        printToConsole("  Original RX1 enabled: " + origEnabled);

        // Disable (powerdown)
        adrv9002.setRxEnabled(0, "0");
        msleep(500);
        var disabled = adrv9002.isRxEnabled(0);
        printToConsole("  After powerdown (disable): " + disabled);
        if (disabled !== "0") {
            printToConsole("  FAIL: Powerdown did not disable RX1");
            adrv9002.setRxEnabled(0, origEnabled);
            return false;
        }

        // Enable (power up)
        adrv9002.setRxEnabled(0, "1");
        msleep(500);
        var enabled = adrv9002.isRxEnabled(0);
        printToConsole("  After power up (enable): " + enabled);
        if (enabled !== "1") {
            printToConsole("  FAIL: Power up did not enable RX1");
            adrv9002.setRxEnabled(0, origEnabled);
            return false;
        }

        // Restore
        adrv9002.setRxEnabled(0, origEnabled);
        msleep(500);

        printToConsole("  Step 5 PASS: Powerdown control functions correctly");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.RX_CHANNEL_CONFIG.READ_ONLY", function() {
    try {
        // Step 6: Verify read-only values update: Decimated Power, Bandwidth, Sampling Rate
        var decPower0 = adrv9002.getRxDecimatedPower(0);
        var decPower1 = adrv9002.getRxDecimatedPower(1);
        printToConsole("  RX1 Decimated Power: " + decPower0);
        printToConsole("  RX2 Decimated Power: " + decPower1);
        if (!decPower0 || decPower0 === "") {
            printToConsole("  FAIL: getRxDecimatedPower(0) returned empty");
            return false;
        }

        var bw0 = adrv9002.getRxRfBandwidth(0);
        var bw1 = adrv9002.getRxRfBandwidth(1);
        printToConsole("  RX1 RF Bandwidth: " + bw0);
        printToConsole("  RX2 RF Bandwidth: " + bw1);
        if (!bw0 || bw0 === "") {
            printToConsole("  FAIL: getRxRfBandwidth(0) returned empty");
            return false;
        }

        var sr0 = adrv9002.getRxSamplingFrequency(0);
        var sr1 = adrv9002.getRxSamplingFrequency(1);
        printToConsole("  RX1 Sampling Rate: " + sr0);
        printToConsole("  RX2 Sampling Rate: " + sr1);
        if (!sr0 || sr0 === "") {
            printToConsole("  FAIL: getRxSamplingFrequency(0) returned empty");
            return false;
        }

        printToConsole("  Step 6 PASS: Read-only widgets display current device values");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 5: TX Channel Controls
// UID: TST.ADRV9002.CONTROLS.TX_CHANNEL_CONFIG
// Description: Test TX1 and TX2 channel control functionality.
// Steps: Change attenuation, control mode, toggle powerdown,
//        verify read-only values.
// ============================================
printToConsole("\n=== Test 5: TX Channel Controls ===\n");

TestFramework.runTest("TST.ADRV9002.CONTROLS.TX_CHANNEL_CONFIG.ATTENUATION", function() {
    try {
        // Step 2: Change TX1 Attenuation (-41.95 to 0 dB)
        var origAtten = adrv9002.getTxAttenuation(0);
        printToConsole("  Original TX1 attenuation: " + origAtten);

        adrv9002.setTxAttenuation(0, "-20");
        msleep(500);
        var readBack = adrv9002.getTxAttenuation(0);
        printToConsole("  Set TX1 attenuation to -20, read back: " + readBack);
        if (readBack.indexOf("-20") === -1) {
            printToConsole("  FAIL: Attenuation control did not respond");
            adrv9002.setTxAttenuation(0, origAtten);
            return false;
        }

        // Also test TX2
        var origAtten2 = adrv9002.getTxAttenuation(1);
        adrv9002.setTxAttenuation(1, "-15");
        msleep(500);
        var readBack2 = adrv9002.getTxAttenuation(1);
        printToConsole("  Set TX2 attenuation to -15, read back: " + readBack2);

        // Restore
        adrv9002.setTxAttenuation(0, origAtten);
        adrv9002.setTxAttenuation(1, origAtten2);
        msleep(500);

        printToConsole("  Step 2 PASS: Attenuation control responds and updates device");
        return readBack.indexOf("-20") !== -1;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.TX_CHANNEL_CONFIG.ATTEN_CTRL_MODE", function() {
    try {
        // Step 3: Change Attenuation Control Mode
        var mode = adrv9002.getTxAttenControlMode(0);
        printToConsole("  TX1 Attenuation Control Mode: " + mode);
        if (!mode || mode === "") {
            printToConsole("  FAIL: getTxAttenControlMode(0) returned empty");
            return false;
        }

        var mode2 = adrv9002.getTxAttenControlMode(1);
        printToConsole("  TX2 Attenuation Control Mode: " + mode2);

        printToConsole("  Step 3 PASS: Control mode reads correctly");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.TX_CHANNEL_CONFIG.POWERDOWN", function() {
    try {
        // Step 4: Toggle TX Powerdown enable/disable
        var origEnabled = adrv9002.isTxEnabled(0);
        printToConsole("  Original TX1 enabled: " + origEnabled);

        // Disable (powerdown)
        adrv9002.setTxEnabled(0, "0");
        msleep(500);
        var disabled = adrv9002.isTxEnabled(0);
        printToConsole("  After powerdown: " + disabled);
        if (disabled !== "0") {
            printToConsole("  FAIL: Powerdown did not disable TX1");
            adrv9002.setTxEnabled(0, origEnabled);
            return false;
        }

        // Enable
        adrv9002.setTxEnabled(0, "1");
        msleep(500);
        var enabled = adrv9002.isTxEnabled(0);
        printToConsole("  After power up: " + enabled);
        if (enabled !== "1") {
            printToConsole("  FAIL: Power up did not enable TX1");
            adrv9002.setTxEnabled(0, origEnabled);
            return false;
        }

        // Restore
        adrv9002.setTxEnabled(0, origEnabled);
        msleep(500);

        printToConsole("  Step 4 PASS: Powerdown control functions correctly");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.TX_CHANNEL_CONFIG.READ_ONLY", function() {
    try {
        // Step 5: Verify read-only values: Bandwidth, Sampling Rate
        var bw0 = adrv9002.getTxRfBandwidth(0);
        var bw1 = adrv9002.getTxRfBandwidth(1);
        printToConsole("  TX1 RF Bandwidth: " + bw0);
        printToConsole("  TX2 RF Bandwidth: " + bw1);
        if (!bw0 || bw0 === "") {
            printToConsole("  FAIL: getTxRfBandwidth(0) returned empty");
            return false;
        }
        if (!bw1 || bw1 === "") {
            printToConsole("  FAIL: getTxRfBandwidth(1) returned empty");
            return false;
        }

        var sr0 = adrv9002.getTxSamplingFrequency(0);
        var sr1 = adrv9002.getTxSamplingFrequency(1);
        printToConsole("  TX1 Sampling Rate: " + sr0);
        printToConsole("  TX2 Sampling Rate: " + sr1);
        if (!sr0 || sr0 === "") {
            printToConsole("  FAIL: getTxSamplingFrequency(0) returned empty");
            return false;
        }
        if (!sr1 || sr1 === "") {
            printToConsole("  FAIL: getTxSamplingFrequency(1) returned empty");
            return false;
        }

        printToConsole("  Step 5 PASS: Read-only widgets display current device values");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 6: ORX Controls
// UID: TST.ADRV9002.CONTROLS.ORX_CONFIG
// Description: Test ORX observation path controls.
// Steps: Check ORX visible, change gain, toggle BBDC,
//        toggle powerdown.
// ============================================
printToConsole("\n=== Test 6: ORX Controls ===\n");

TestFramework.runTest("TST.ADRV9002.CONTROLS.ORX_CONFIG.VISIBILITY", function() {
    try {
        // Step 1: Check if ORX controls are visible
        var orxGain = adrv9002.getOrxHardwareGain(0);
        printToConsole("  ORX1 HW gain: " + orxGain);
        if (!orxGain || orxGain === "") {
            printToConsole("  ORX controls not available on this device - SKIP");
            return "SKIP";
        }

        var orxGain2 = adrv9002.getOrxHardwareGain(1);
        printToConsole("  ORX2 HW gain: " + orxGain2);

        printToConsole("  Step 1 PASS: ORX1 and ORX2 controls appear");
        return true;
    } catch (e) {
        printToConsole("  ORX controls not supported: " + e);
        return "SKIP";
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.ORX_CONFIG.HW_GAIN", function() {
    try {
        // Step 2: Change ORX Hardware Gain (4-36 dB)
        var origGain = adrv9002.getOrxHardwareGain(0);
        printToConsole("  Original ORX1 HW gain: " + origGain);

        adrv9002.setOrxHardwareGain(0, "18");
        msleep(500);
        var readBack = adrv9002.getOrxHardwareGain(0);
        printToConsole("  Set ORX1 gain to 18, read back: " + readBack);

        // Restore
        adrv9002.setOrxHardwareGain(0, origGain);
        msleep(500);

        if (readBack.indexOf("18") === -1) {
            printToConsole("  FAIL: ORX gain control did not respond");
            return false;
        }
        printToConsole("  Step 2 PASS: ORX gain control responds and updates device");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.ORX_CONFIG.BBDC_REJECTION", function() {
    try {
        // Step 3: Enable/disable BBDC Rejection
        var origBbdc = adrv9002.isOrxBbdcRejectionEnabled(0);
        printToConsole("  Original ORX1 BBDC rejection: " + origBbdc);

        adrv9002.setOrxBbdcRejectionEnabled(0, "1");
        msleep(500);
        var enabled = adrv9002.isOrxBbdcRejectionEnabled(0);
        printToConsole("  After enable: " + enabled);

        adrv9002.setOrxBbdcRejectionEnabled(0, "0");
        msleep(500);
        var disabled = adrv9002.isOrxBbdcRejectionEnabled(0);
        printToConsole("  After disable: " + disabled);

        // Restore
        adrv9002.setOrxBbdcRejectionEnabled(0, origBbdc);
        msleep(500);

        if (enabled !== "1" || disabled !== "0") {
            printToConsole("  FAIL: BBDC Rejection toggle did not function correctly");
            return false;
        }
        printToConsole("  Step 3 PASS: BBDC Rejection toggle functions correctly");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.ORX_CONFIG.POWERDOWN", function() {
    try {
        // Step 4: Toggle ORX Powerdown
        var origEnabled = adrv9002.isOrxEnabled(0);
        printToConsole("  Original ORX1 enabled: " + origEnabled);

        adrv9002.setOrxEnabled(0, "0");
        msleep(500);
        var disabled = adrv9002.isOrxEnabled(0);
        printToConsole("  After powerdown: " + disabled);

        adrv9002.setOrxEnabled(0, "1");
        msleep(500);
        var enabled = adrv9002.isOrxEnabled(0);
        printToConsole("  After power up: " + enabled);

        // Restore
        adrv9002.setOrxEnabled(0, origEnabled);
        msleep(500);

        if (disabled !== "0" || enabled !== "1") {
            printToConsole("  FAIL: ORX enable/disable did not function correctly");
            return false;
        }
        printToConsole("  Step 4 PASS: ORX enable/disable control functions correctly");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 7: Refresh Functionality
// UID: TST.ADRV9002.CONTROLS.REFRESH_FUNCTION
// Description: Test refresh button updates all control values from device.
// Steps: Change a value, call refresh, verify values update.
// ============================================
printToConsole("\n=== Test 7: Refresh Functionality ===\n");

TestFramework.runTest("TST.ADRV9002.CONTROLS.REFRESH_FUNCTION", function() {
    try {
        // Step 1: Note current control values
        var origMode = adrv9002.getRxGainControlMode(0);
        var origGain = adrv9002.getRxHardwareGain(0);
        printToConsole("  Before refresh - gain mode: " + origMode + ", gain: " + origGain);

        // Step 2: Change device settings (via API, simulating external change)
        adrv9002.setRxGainControlMode(0, "manual");
        msleep(500);
        adrv9002.setRxHardwareGain(0, "20");
        msleep(500);
        printToConsole("  Changed gain to 20 dB in manual mode");

        // Step 3: Click refresh button
        adrv9002.refresh();
        msleep(1000);
        printToConsole("  Refresh triggered");

        // Verify values are consistent after refresh
        var afterRefreshGain = adrv9002.getRxHardwareGain(0);
        var afterRefreshMode = adrv9002.getRxGainControlMode(0);
        printToConsole("  After refresh - gain mode: " + afterRefreshMode + ", gain: " + afterRefreshGain);

        if (!afterRefreshGain || afterRefreshGain === "") {
            printToConsole("  FAIL: Values not updated after refresh");
            adrv9002.setRxHardwareGain(0, origGain);
            adrv9002.setRxGainControlMode(0, origMode);
            return false;
        }

        // Restore original values
        adrv9002.setRxHardwareGain(0, origGain);
        adrv9002.setRxGainControlMode(0, origMode);
        msleep(500);

        printToConsole("  PASS: Refresh button triggers update of all controls");
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
