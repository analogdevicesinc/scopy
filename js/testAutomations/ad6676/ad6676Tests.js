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

// AD6676 Plugin Automated Tests
// Runs all Category A (fully automatable) API tests.
// Source: docs/tests/plugins/ad6676/ad6676_tests.rst

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: AD6676 Plugin Tests
TestFramework.init("AD6676 Plugin Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:10.48.69.131")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to AD6676 tool
if (!switchToTool("AD6676")) {
    printToConsole("ERROR: Cannot switch to AD6676 tool");
    exit(1);
}

// ============================================
// TST.AD6676.PLUGIN_LOADS
// ============================================
printToConsole("\n=== Plugin Loads ===\n");

TestFramework.runTest("TST.AD6676.PLUGIN_LOADS", function() {
    try {
        var tools = ad6676.getTools();
        printToConsole("  Tools found: " + tools);
        if (!tools || tools.length === 0) {
            printToConsole("  Error: getTools() returned empty list");
            return false;
        }
        var found = false;
        for (var i = 0; i < tools.length; i++) {
            if (tools[i].indexOf("AD6676") !== -1) {
                found = true;
                break;
            }
        }
        if (!found) {
            printToConsole("  Error: 'AD6676' not found in tools list");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// TST.AD6676.DEVICE_DETECTION
// ============================================
printToConsole("\n=== Device Detection and Display ===\n");

TestFramework.runTest("TST.AD6676.DEVICE_DETECTION", function() {
    try {
        var tools = ad6676.getTools();
        if (!tools || tools.length === 0) {
            printToConsole("  Error: getTools() returned empty list");
            return false;
        }
        var found = false;
        for (var i = 0; i < tools.length; i++) {
            if (tools[i].indexOf("AD6676") !== -1) {
                found = true;
                break;
            }
        }
        if (!found) {
            printToConsole("  Error: 'AD6676' not found in tools list");
            return false;
        }

        // Verify all 12 attribute getters return non-empty strings
        var adcFreq = ad6676.getAdcFrequency();
        var bw = ad6676.getBandwidth();
        var bwMarginLow = ad6676.getBwMarginLow();
        var bwMarginHigh = ad6676.getBwMarginHigh();
        var bwMarginIf = ad6676.getBwMarginIf();
        var ifFreq = ad6676.getIntermediateFrequency();
        var sampFreq = ad6676.getSamplingFrequency();
        var hwGain = ad6676.getHardwareGain();
        var scale = ad6676.getScale();
        var shufflerCtrl = ad6676.getShufflerControl();
        var shufflerThresh = ad6676.getShufflerThresh();
        var testMode = ad6676.getTestMode();

        printToConsole("  adc_frequency: " + adcFreq);
        printToConsole("  bandwidth: " + bw);
        printToConsole("  bw_margin_low: " + bwMarginLow);
        printToConsole("  bw_margin_high: " + bwMarginHigh);
        printToConsole("  bw_margin_if: " + bwMarginIf);
        printToConsole("  intermediate_frequency: " + ifFreq);
        printToConsole("  sampling_frequency: " + sampFreq);
        printToConsole("  hardwaregain: " + hwGain);
        printToConsole("  scale: " + scale);
        printToConsole("  shuffler_control: " + shufflerCtrl);
        printToConsole("  shuffler_thresh: " + shufflerThresh);
        printToConsole("  test_mode: " + testMode);

        var allNonEmpty = adcFreq && bw && bwMarginLow && bwMarginHigh && bwMarginIf &&
                          ifFreq && sampFreq && hwGain && scale &&
                          shufflerCtrl && shufflerThresh && testMode;
        if (!allNonEmpty) {
            printToConsole("  Error: One or more attribute getters returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// TST.AD6676.ADC_SETTINGS
// ============================================
printToConsole("\n=== ADC Frequency Configuration ===\n");

TestFramework.runTest("TST.AD6676.ADC_SETTINGS", function() {
    try {
        var original = ad6676.getAdcFrequency();
        printToConsole("  Original adc_frequency: " + original);

        // Step 1: Set a valid in-range value
        ad6676.setAdcFrequency("3000");
        msleep(500);
        var readBack = ad6676.getAdcFrequency();
        printToConsole("  Read back after set '3000': " + readBack);
        if (!readBack || readBack === "") {
            printToConsole("  Error: getAdcFrequency() returned empty string after set");
            ad6676.setAdcFrequency(original);
            msleep(500);
            return false;
        }

        // Step 2: Clamping test — set value below valid range (2925–3200 MHz)
        ad6676.setAdcFrequency("2000");
        msleep(500);
        var clamped = ad6676.getAdcFrequency();
        printToConsole("  Read back after out-of-range '2000': " + clamped);
        var clampedNum = parseFloat(clamped);
        if (isNaN(clampedNum) || clampedNum < 2925) {
            printToConsole("  Error: clamped value " + clamped + " is below minimum 2925");
            ad6676.setAdcFrequency(original);
            msleep(500);
            return false;
        }

        // Restore original
        ad6676.setAdcFrequency(original);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// TST.AD6676.BANDWIDTH_SETTINGS
// ============================================
printToConsole("\n=== Bandwidth Settings ===\n");

TestFramework.runTest("TST.AD6676.BANDWIDTH_SETTINGS", function() {
    try {
        var origBw = ad6676.getBandwidth();
        var origBwMarginLow = ad6676.getBwMarginLow();
        var origBwMarginHigh = ad6676.getBwMarginHigh();
        var origBwMarginIf = ad6676.getBwMarginIf();
        printToConsole("  Original bandwidth: " + origBw);
        printToConsole("  Original bw_margin_low: " + origBwMarginLow);
        printToConsole("  Original bw_margin_high: " + origBwMarginHigh);
        printToConsole("  Original bw_margin_if: " + origBwMarginIf);

        // Set and read back bandwidth
        ad6676.setBandwidth("75");
        msleep(500);
        var readBw = ad6676.getBandwidth();
        printToConsole("  Read back bandwidth after set '75': " + readBw);

        // Set and read back bw_margin_low
        ad6676.setBwMarginLow("5");
        msleep(500);
        var readBwMarginLow = ad6676.getBwMarginLow();
        printToConsole("  Read back bw_margin_low after set '5': " + readBwMarginLow);

        // Set and read back bw_margin_high
        ad6676.setBwMarginHigh("5");
        msleep(500);
        var readBwMarginHigh = ad6676.getBwMarginHigh();
        printToConsole("  Read back bw_margin_high after set '5': " + readBwMarginHigh);

        // Set and read back bw_margin_if
        ad6676.setBwMarginIf("3");
        msleep(500);
        var readBwMarginIf = ad6676.getBwMarginIf();
        printToConsole("  Read back bw_margin_if after set '3': " + readBwMarginIf);

        // Restore originals
        ad6676.setBandwidth(origBw);
        msleep(500);
        ad6676.setBwMarginLow(origBwMarginLow);
        msleep(500);
        ad6676.setBwMarginHigh(origBwMarginHigh);
        msleep(500);
        ad6676.setBwMarginIf(origBwMarginIf);
        msleep(500);

        if(!readBw || !readBwMarginLow || !readBwMarginHigh || !readBwMarginIf) return false;
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// TST.AD6676.RECEIVE_SETTINGS
// ============================================
printToConsole("\n=== Receive Settings ===\n");

TestFramework.runTest("TST.AD6676.RECEIVE_SETTINGS", function() {
    try {
        var origIf = ad6676.getIntermediateFrequency();
        var origGain = ad6676.getHardwareGain();
        var origScale = ad6676.getScale();
        printToConsole("  Original intermediate_frequency: " + origIf);
        printToConsole("  Original hardwaregain: " + origGain);
        printToConsole("  Original scale: " + origScale);

        // Set and read back intermediate frequency
        ad6676.setIntermediateFrequency("100");
        msleep(500);
        var readIf = ad6676.getIntermediateFrequency();
        printToConsole("  Read back intermediate_frequency after set '100': " + readIf);
        if (!readIf || readIf === "") {
            printToConsole("  Error: getIntermediateFrequency() returned empty string");
            ad6676.setIntermediateFrequency(origIf);
            msleep(500);
            return false;
        }

        // Verify sampling_frequency is read-only (getter only, no setter)
        var sampFreq = ad6676.getSamplingFrequency();
        printToConsole("  sampling_frequency (read-only): " + sampFreq);
        if (!sampFreq || sampFreq === "") {
            printToConsole("  Error: getSamplingFrequency() returned empty string");
            ad6676.setIntermediateFrequency(origIf);
            msleep(500);
            return false;
        }

        // Set and read back hardware gain
        ad6676.setHardwareGain("-10");
        msleep(500);
        var readGain = ad6676.getHardwareGain();
        printToConsole("  Read back hardwaregain after set '-10': " + readGain);
        if (!readGain || readGain === "") {
            printToConsole("  Error: getHardwareGain() returned empty string");
            ad6676.setIntermediateFrequency(origIf);
            ad6676.setHardwareGain(origGain);
            msleep(500);
            return false;
        }

        // Set and read back scale
        ad6676.setScale("6");
        msleep(500);
        var readScale = ad6676.getScale();
        printToConsole("  Read back scale after set '6': " + readScale);
        if (!readScale || readScale === "") {
            printToConsole("  Error: getScale() returned empty string");
        }

        // Restore originals
        ad6676.setIntermediateFrequency(origIf);
        msleep(500);
        ad6676.setHardwareGain(origGain);
        msleep(500);
        ad6676.setScale(origScale);
        msleep(500);

        if(!readIf || !sampFreq || !readGain || !readScale) return false;
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// TST.AD6676.SHUFFLER_SETTINGS
// ============================================
printToConsole("\n=== Shuffler Settings ===\n");

TestFramework.runTest("TST.AD6676.SHUFFLER_SETTINGS", function() {
    try {
        var origCtrl = ad6676.getShufflerControl();
        var origThresh = ad6676.getShufflerThresh();
        printToConsole("  Original shuffler_control: " + origCtrl);
        printToConsole("  Original shuffler_thresh: " + origThresh);

        // Set shuffler_control to "disable" (first/safe option)
        ad6676.setShufflerControl("disable");
        msleep(500);
        var readCtrl = ad6676.getShufflerControl();
        printToConsole("  Read back shuffler_control after set 'disable': " + readCtrl);
        if (!readCtrl || readCtrl === "") {
            printToConsole("  Error: getShufflerControl() returned empty string");
            ad6676.setShufflerControl(origCtrl);
            msleep(500);
            return false;
        }

        // Set shuffler_thresh
        ad6676.setShufflerThresh("8");
        msleep(500);
        var readThresh = ad6676.getShufflerThresh();
        printToConsole("  Read back shuffler_thresh after set '8': " + readThresh);
        if (!readThresh || readThresh === "") {
            printToConsole("  Error: getShufflerThresh() returned empty string");
        }

        // Restore originals
        ad6676.setShufflerControl(origCtrl);
        msleep(500);
        ad6676.setShufflerThresh(origThresh);
        msleep(500);

        if(!readCtrl || !readThresh) return false;
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// TST.AD6676.TEST_MODE
// ============================================
printToConsole("\n=== Test Mode Configuration ===\n");

TestFramework.runTest("TST.AD6676.TEST_MODE", function() {
    try {
        var original = ad6676.getTestMode();
        printToConsole("  Original test_mode: " + original);

        // Set to a known-safe test pattern
        ad6676.setTestMode("checkerboard");
        msleep(500);
        var readBack = ad6676.getTestMode();
        printToConsole("  Read back test_mode after set 'checkerboard': " + readBack);
        if (!readBack || readBack === "") {
            printToConsole("  Error: getTestMode() returned empty string");
            ad6676.setTestMode(original);
            msleep(500);
            return false;
        }

        // Restore to "off"
        ad6676.setTestMode("off");
        msleep(500);
        var restored = ad6676.getTestMode();
        printToConsole("  Read back test_mode after restore 'off': " + restored);

        // Restore original if it was not "off"
        if (original && original !== "off") {
            ad6676.setTestMode(original);
            msleep(500);
        }

        return readBack === "checkerboard";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// TST.AD6676.REFRESH
// ============================================
printToConsole("\n=== Refresh Functionality ===\n");

TestFramework.runTest("TST.AD6676.REFRESH", function() {
    try {
        ad6676.refresh();
        msleep(500);
        printToConsole("  refresh() completed without exception");

        // Verify values are still readable after refresh
        var adcFreq = ad6676.getAdcFrequency();
        var bw = ad6676.getBandwidth();
        var sampFreq = ad6676.getSamplingFrequency();
        printToConsole("  adc_frequency after refresh: " + adcFreq);
        printToConsole("  bandwidth after refresh: " + bw);
        printToConsole("  sampling_frequency after refresh: " + sampFreq);

        if (!adcFreq || !bw || !sampFreq) {
            printToConsole("  Error: One or more getters returned empty string after refresh");
            return false;
        }
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
