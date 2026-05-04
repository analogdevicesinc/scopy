/*
 * Copyright (c) 2026 Analog Devices Inc.
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

// FMCOMMS11 Manual Test Automation
// Automates tests from: docs/tests/plugins/fmcomms11/fmcomms11_tests.rst
// Tests: TST.FMCOMMS11.PLUGIN_LOADS, TST.FMCOMMS11.DEVICE_DETECTION,
//        TST.FMCOMMS11.ADC_SECTION_VISIBLE, TST.FMCOMMS11.ADC_SAMPLING_FREQ_DISPLAY,
//        TST.FMCOMMS11.ADC_TEST_MODE, TST.FMCOMMS11.ADC_SCALE,
//        TST.FMCOMMS11.INPUT_ATTN_SECTION_VISIBLE, TST.FMCOMMS11.HMC1119_GAIN,
//        TST.FMCOMMS11.DAC_SECTION_VISIBLE, TST.FMCOMMS11.DAC_SAMPLING_FREQ_DISPLAY,
//        TST.FMCOMMS11.DAC_NCO_FREQUENCY, TST.FMCOMMS11.DAC_FIR85_ENABLE,
//        TST.FMCOMMS11.OUTPUT_VGA_SECTION_VISIBLE, TST.FMCOMMS11.ADL5240_GAIN,
//        TST.FMCOMMS11.REFRESH_FUNCTION

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("FMCOMMS11 Documentation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:10.48.69.131")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to FMCOMMS11 tool
if (!switchToTool("FMCOMMS11")) {
    printToConsole("ERROR: Cannot switch to FMCOMMS11 tool");
    scopy.exit();
}

// ============================================
// Test 1: Plugin Loads
// UID: TST.FMCOMMS11.PLUGIN_LOADS
// Description: Verify that the FMCOMMS11 plugin loads in Scopy without errors.
// ============================================
printToConsole("\n=== Test 1: Plugin Loads ===\n");

TestFramework.runTest("TST.FMCOMMS11.PLUGIN_LOADS", function() {
    try {
        var tools = fmcomms11.getTools();    
        if (!tools || tools.length === 0) {
            printToConsole("  FAIL: No tools found for FMCOMMS11 plugin");
            return false;
        }
        printToConsole("  FMCOMMS11 tools found: " + tools);
        printToConsole("  PASS: FMCOMMS11 plugin loaded and tools are accessible");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2: Device Detection
// UID: TST.FMCOMMS11.DEVICE_DETECTION
// Description: Verify that the FMCOMMS11 plugin detects all required IIO devices.
// ============================================
printToConsole("\n=== Test 2: Device Detection ===\n");

TestFramework.runTest("TST.FMCOMMS11.DEVICE_DETECTION", function() {
    try {
        var keys = fmcomms11.getWidgetKeys();
        if (!keys || keys.length === 0) {
            printToConsole("  FAIL: No widgets detected - device may not be connected");
            return false;
        }
        printToConsole("  Widget keys available: " + keys.length);
        printToConsole("  PASS: Device detected, " + keys.length + " widgets built");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3: ADC Section Visible
// UID: TST.FMCOMMS11.ADC_SECTION_VISIBLE
// Description: Verify that the ADC section is visible and contains expected controls.
// ============================================
printToConsole("\n=== Test 3: ADC Section Visible ===\n");

TestFramework.runTest("TST.FMCOMMS11.ADC_SECTION_VISIBLE", function() {
    try {
        var freq = fmcomms11.getAdcSamplingFrequency();
        printToConsole("  ADC Sampling Frequency: " + freq);
        if (!freq || freq === "") {
            printToConsole("  FAIL: ADC sampling frequency returned empty - ADC section may not be visible");
            return false;
        }
        printToConsole("  PASS: ADC section is visible, sampling frequency readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: ADC Sampling Frequency Display
// UID: TST.FMCOMMS11.ADC_SAMPLING_FREQ_DISPLAY
// Description: Verify ADC sampling frequency displays a non-zero value in MHz.
// ============================================
printToConsole("\n=== Test 4: ADC Sampling Frequency Display ===\n");

TestFramework.runTest("TST.FMCOMMS11.ADC_SAMPLING_FREQ_DISPLAY", function() {
    try {
        var freq = fmcomms11.getAdcSamplingFrequency();
        printToConsole("  ADC Sampling Frequency: " + freq);
        if (!freq || freq === "") {
            printToConsole("  FAIL: ADC sampling frequency returned empty");
            return false;
        }
        var numVal = parseFloat(freq);
        if (isNaN(numVal) || numVal === 0) {
            printToConsole("  FAIL: ADC sampling frequency is zero or non-numeric: " + freq);
            return false;
        }
        printToConsole("  PASS: ADC sampling frequency is non-zero: " + freq);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 5: ADC Test Mode Selection
// UID: TST.FMCOMMS11.ADC_TEST_MODE
// Description: Verify ADC test mode combo box applies selected mode to device.
// ============================================
printToConsole("\n=== Test 5: ADC Test Mode Selection ===\n");

TestFramework.runTest("TST.FMCOMMS11.ADC_TEST_MODE", function() {
    var originalMode = "";
    try {
        originalMode = fmcomms11.getAdcTestMode();
        printToConsole("  Original ADC test mode: " + originalMode);

        // Set to midscale_short
        fmcomms11.setAdcTestMode("midscale_short");
        msleep(500);
        var newMode = fmcomms11.getAdcTestMode();
        printToConsole("  After set - ADC test mode: " + newMode);
        if (newMode !== "midscale_short") {
            printToConsole("  FAIL: ADC test mode was not applied");
            fmcomms11.setAdcTestMode(originalMode);
            msleep(500);
            return false;
        }
        printToConsole("  PASS: ADC test mode set to midscale_short");

        // Restore original
        fmcomms11.setAdcTestMode(originalMode);
        msleep(500);
        var restored = fmcomms11.getAdcTestMode();
        printToConsole("  Restored ADC test mode: " + restored);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (originalMode !== "") {
            fmcomms11.setAdcTestMode(originalMode);
            msleep(500);
        }
        return false;
    }
});

// ============================================
// Test 6: ADC Scale Selection
// UID: TST.FMCOMMS11.ADC_SCALE
// Description: Verify ADC scale combo box returns a value and write-readback works.
// ============================================
printToConsole("\n=== Test 6: ADC Scale Selection ===\n");

TestFramework.runTest("TST.FMCOMMS11.ADC_SCALE", function() {
    var originalScale = "";
    try {
        originalScale = fmcomms11.getAdcScale();
        printToConsole("  Original ADC scale: " + originalScale);
        if (!originalScale || originalScale === "") {
            printToConsole("  FAIL: ADC scale returned empty");
            return false;
        }

        // Write same value back and verify readback
        fmcomms11.setAdcScale(originalScale);
        msleep(500);
        var readbackScale = fmcomms11.getAdcScale();
        printToConsole("  Readback ADC scale: " + readbackScale);
        if (readbackScale !== originalScale) {
            printToConsole("  FAIL: ADC scale readback mismatch: expected " + originalScale + " got " + readbackScale);
            return false;
        }
        printToConsole("  PASS: ADC scale is non-empty and write-readback matches");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (originalScale !== "") {
            fmcomms11.setAdcScale(originalScale);
            msleep(500);
        }
        return false;
    }
});

// ============================================
// Test 7: Input Attenuator Section Visible
// UID: TST.FMCOMMS11.INPUT_ATTN_SECTION_VISIBLE
// Description: Verify Input Attenuator section is visible with HMC1119 gain control.
// ============================================
printToConsole("\n=== Test 7: Input Attenuator Section Visible ===\n");

TestFramework.runTest("TST.FMCOMMS11.INPUT_ATTN_SECTION_VISIBLE", function() {
    try {
        var attn = fmcomms11.getInputAttenuation();
        printToConsole("  Input Attenuation: " + attn);
        if (!attn || attn === "") {
            printToConsole("  FAIL: Input attenuation returned empty - section may not be visible");
            return false;
        }
        printToConsole("  PASS: Input Attenuator section is visible");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 8: HMC1119 Attenuator Gain
// UID: TST.FMCOMMS11.HMC1119_GAIN
// Description: Verify HMC1119 attenuator gain spin box applies value to device.
// ============================================
printToConsole("\n=== Test 8: HMC1119 Attenuator Gain ===\n");

TestFramework.runTest("TST.FMCOMMS11.HMC1119_GAIN", function() {
    var originalAttn = "";
    try {
        originalAttn = fmcomms11.getInputAttenuation();
        printToConsole("  Original input attenuation: " + originalAttn);

        // Set to -15.75 dB
        fmcomms11.setInputAttenuation("-15.75");
        msleep(500);
        var newAttn = fmcomms11.getInputAttenuation();
        printToConsole("  After set to -15.75 - input attenuation: " + newAttn);
        if (Math.abs(parseFloat(newAttn) - (-15.75)) > 0.01) {
            printToConsole("  FAIL: Input attenuation was not applied correctly");
            fmcomms11.setInputAttenuation(originalAttn);
            msleep(500);
            return false;
        }
        printToConsole("  PASS: Input attenuation set to -15.75 dB");

        // Restore original
        fmcomms11.setInputAttenuation(originalAttn);
        msleep(500);
        var restored = fmcomms11.getInputAttenuation();
        printToConsole("  Restored input attenuation: " + restored);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (originalAttn !== "") {
            fmcomms11.setInputAttenuation(originalAttn);
            msleep(500);
        }
        return false;
    }
});

// ============================================
// Test 10: DAC Section Visible
// UID: TST.FMCOMMS11.DAC_SECTION_VISIBLE
// Description: Verify DAC section is visible and contains expected controls.
// ============================================
printToConsole("\n=== Test 10: DAC Section Visible ===\n");

TestFramework.runTest("TST.FMCOMMS11.DAC_SECTION_VISIBLE", function() {
    try {
        var freq = fmcomms11.getDacSamplingFrequency();
        printToConsole("  DAC Sampling Frequency: " + freq);
        if (!freq || freq === "") {
            printToConsole("  FAIL: DAC sampling frequency returned empty - DAC section may not be visible");
            return false;
        }
        printToConsole("  PASS: DAC section is visible, sampling frequency readable");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 11: DAC Sampling Frequency Display
// UID: TST.FMCOMMS11.DAC_SAMPLING_FREQ_DISPLAY
// Description: Verify DAC sampling frequency displays a non-zero value in MHz.
// ============================================
printToConsole("\n=== Test 11: DAC Sampling Frequency Display ===\n");

TestFramework.runTest("TST.FMCOMMS11.DAC_SAMPLING_FREQ_DISPLAY", function() {
    try {
        var freq = fmcomms11.getDacSamplingFrequency();
        printToConsole("  DAC Sampling Frequency: " + freq);
        if (!freq || freq === "") {
            printToConsole("  FAIL: DAC sampling frequency returned empty");
            return false;
        }
        var numVal = parseFloat(freq);
        if (isNaN(numVal) || numVal === 0) {
            printToConsole("  FAIL: DAC sampling frequency is zero or non-numeric: " + freq);
            return false;
        }
        printToConsole("  PASS: DAC sampling frequency is non-zero: " + freq);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 12: DAC NCO Frequency
// UID: TST.FMCOMMS11.DAC_NCO_FREQUENCY
// Description: Verify DAC NCO frequency spin box applies value to device in MHz.
// ============================================
printToConsole("\n=== Test 12: DAC NCO Frequency ===\n");

TestFramework.runTest("TST.FMCOMMS11.DAC_NCO_FREQUENCY", function() {
    var originalNco = "";
    try {
        originalNco = fmcomms11.getNcoFrequency();
        printToConsole("  Original NCO frequency: " + originalNco);

        // Set to 100 MHz
        fmcomms11.setNcoFrequency("100");
        msleep(500);
        var newNco = fmcomms11.getNcoFrequency();
        printToConsole("  After set to 100 - NCO frequency: " + newNco);
        if (Math.abs(parseFloat(newNco) - 100) > 0.01) {
            printToConsole("  FAIL: NCO frequency was not applied correctly");
            fmcomms11.setNcoFrequency(originalNco);
            msleep(500);
            return false;
        }
        printToConsole("  PASS: NCO frequency set to 100 MHz");

        // Restore original only if it was within the valid writeable range
        // (device may boot with NCO=0 or NCO=Fs which IIO rejects as a write)
        var originalNcoFloat = parseFloat(originalNco);
        if (originalNcoFloat >= 1.0 && originalNcoFloat <= 5999.0) {
            fmcomms11.setNcoFrequency(originalNco);
            msleep(500);
            var restored = fmcomms11.getNcoFrequency();
            printToConsole("  Restored NCO frequency: " + restored);
        } else {
            printToConsole("  Skipped restore: original value " + originalNco + " MHz is outside writeable range");
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        var originalNcoFloat2 = parseFloat(originalNco);
        if (originalNco !== "" && originalNcoFloat2 >= 1.0 && originalNcoFloat2 <= 5999.0) {
            fmcomms11.setNcoFrequency(originalNco);
            msleep(500);
        }
        return false;
    }
});

// ============================================
// Test 13: DAC FIR85 Filter Enable
// UID: TST.FMCOMMS11.DAC_FIR85_ENABLE
// Description: Verify FIR85 filter enable checkbox toggles filter on DAC device.
// ============================================
printToConsole("\n=== Test 13: DAC FIR85 Filter Enable ===\n");

TestFramework.runTest("TST.FMCOMMS11.DAC_FIR85_ENABLE", function() {
    var originalFir = "";
    try {
        originalFir = fmcomms11.isFir85Enabled();
        printToConsole("  Original FIR85 state: " + originalFir);

        // Enable FIR85
        fmcomms11.setFir85Enabled("1");
        msleep(500);
        var firEnabled = fmcomms11.isFir85Enabled();
        printToConsole("  After enable - FIR85 state: " + firEnabled);
        if (firEnabled !== "1") {
            printToConsole("  FAIL: FIR85 was not enabled");
            fmcomms11.setFir85Enabled(originalFir);
            msleep(500);
            return false;
        }
        printToConsole("  Step 1 PASS: FIR85 enabled");

        // Disable FIR85
        fmcomms11.setFir85Enabled("0");
        msleep(500);
        var firDisabled = fmcomms11.isFir85Enabled();
        printToConsole("  After disable - FIR85 state: " + firDisabled);
        if (firDisabled !== "0") {
            printToConsole("  FAIL: FIR85 was not disabled");
            fmcomms11.setFir85Enabled(originalFir);
            msleep(500);
            return false;
        }
        printToConsole("  Step 2 PASS: FIR85 disabled");

        // Restore original
        fmcomms11.setFir85Enabled(originalFir);
        msleep(500);
        var restored = fmcomms11.isFir85Enabled();
        printToConsole("  Restored FIR85 state: " + restored);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (originalFir !== "") {
            fmcomms11.setFir85Enabled(originalFir);
            msleep(500);
        }
        return false;
    }
});

// ============================================
// Test 14: Output VGA Section Visible
// UID: TST.FMCOMMS11.OUTPUT_VGA_SECTION_VISIBLE
// Description: Verify Output VGA section is visible with ADL5240 gain control.
// ============================================
printToConsole("\n=== Test 14: Output VGA Section Visible ===\n");

TestFramework.runTest("TST.FMCOMMS11.OUTPUT_VGA_SECTION_VISIBLE", function() {
    try {
        var gain = fmcomms11.getOutputGain();
        printToConsole("  Output Gain: " + gain);
        if (!gain || gain === "") {
            printToConsole("  FAIL: Output gain returned empty - Output VGA section may not be visible");
            return false;
        }
        printToConsole("  PASS: Output VGA section is visible");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 15: ADL5240 VGA Gain
// UID: TST.FMCOMMS11.ADL5240_GAIN
// Description: Verify ADL5240 VGA gain spin box applies value to device.
// ============================================
printToConsole("\n=== Test 15: ADL5240 VGA Gain ===\n");

TestFramework.runTest("TST.FMCOMMS11.ADL5240_GAIN", function() {
    var originalGain = "";
    try {
        originalGain = fmcomms11.getOutputGain();
        printToConsole("  Original output gain: " + originalGain);

        // Set to 10 dB
        fmcomms11.setOutputGain("10");
        msleep(500);
        var newGain = fmcomms11.getOutputGain();
        printToConsole("  After set to 10 - output gain: " + newGain);
        if (Math.abs(parseFloat(newGain) - 10) > 0.01) {
            printToConsole("  FAIL: Output gain was not applied correctly");
            fmcomms11.setOutputGain(originalGain);
            msleep(500);
            return false;
        }
        printToConsole("  PASS: Output gain set to 10 dB");

        // Restore original
        fmcomms11.setOutputGain(originalGain);
        msleep(500);
        var restored = fmcomms11.getOutputGain();
        printToConsole("  Restored output gain: " + restored);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        if (originalGain !== "") {
            fmcomms11.setOutputGain(originalGain);
            msleep(500);
        }
        return false;
    }
});

// ============================================
// Test 16: Refresh Functionality
// UID: TST.FMCOMMS11.REFRESH_FUNCTION
// Description: Verify refresh reads back current attribute values from all devices.
// ============================================
printToConsole("\n=== Test 16: Refresh Functionality ===\n");

TestFramework.runTest("TST.FMCOMMS11.REFRESH_FUNCTION", function() {
    try {
        // Call refresh
        fmcomms11.refresh();
        msleep(500);

        // Verify reads still return non-empty values after refresh
        var adcFreq = fmcomms11.getAdcSamplingFrequency();
        var dacFreq = fmcomms11.getDacSamplingFrequency();
        var attn = fmcomms11.getInputAttenuation();
        var gain = fmcomms11.getOutputGain();

        printToConsole("  After refresh - ADC freq: " + adcFreq);
        printToConsole("  After refresh - DAC freq: " + dacFreq);
        printToConsole("  After refresh - Input attenuation: " + attn);
        printToConsole("  After refresh - Output gain: " + gain);

        if (!adcFreq || adcFreq === "") {
            printToConsole("  FAIL: ADC frequency empty after refresh");
            return false;
        }
        if (!dacFreq || dacFreq === "") {
            printToConsole("  FAIL: DAC frequency empty after refresh");
            return false;
        }
        if (!attn || attn === "") {
            printToConsole("  FAIL: Input attenuation empty after refresh");
            return false;
        }
        if (!gain || gain === "") {
            printToConsole("  FAIL: Output gain empty after refresh");
            return false;
        }

        printToConsole("  PASS: All values readable after refresh");
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
