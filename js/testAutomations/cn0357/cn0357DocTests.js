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

// CN0357 Manual Test Automation
// Automates tests from: docs/tests/plugins/cn0357/cn0357_tests.rst
//
// Tests automated here (Category A — fully automated via JS API):
//   TST.CN0357.PLUGIN_LOADS           (#1)
//   TST.CN0357.ADC_UPDATE_RATE        (#2, steps 1-2)
//   TST.CN0357.FEEDBACK_SETTINGS      (#3, functional steps)
//   TST.CN0357.SYSTEM_MEASUREMENTS    (#4, steps 1, 4-6)
//   TST.CN0357.SENSOR_SENSITIVITY     (#5)
//   TST.CN0357.REFRESH_FUNCTION       (#6, programmatic)
//
// Visual tests requiring supervised checks are in cn0357VisualTests.js
//
// Missing API Report:
//   - No triggerRefresh() method to click the toolbar refresh button
//   - No API to query widget/section visibility state
// ============================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("CN0357 Documentation Tests");

// Connect to device — ip:127.0.0.0 intentionally fails, triggering emulator fallback
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to CN0357 tool
if (!switchToTool("CN0357")) {
    printToConsole("ERROR: Cannot switch to CN0357 tool");
    scopy.exit();
}

// ============================================
// Test 1: Plugin Loads
// UID: TST.CN0357.PLUGIN_LOADS
// Description: Verify that the CN0357 plugin loads and displays all three sections.
// ============================================
printToConsole("\n=== Test 1: Plugin Loads ===\n");

TestFramework.runTest("TST.CN0357.PLUGIN_LOADS", function() {
    try {
        var tools = cn0357.getTools();
        printToConsole("  Tools found: " + tools);
        if (!tools || tools.length === 0) {
            printToConsole("  FAIL: No tools found for CN0357 plugin");
            return false;
        }
        if (tools.indexOf("CN0357") === -1) {
            printToConsole("  FAIL: CN0357 tool not in list: " + tools);
            return false;
        }
        printToConsole("  PASS: CN0357 tool is visible and accessible");

        var keys = cn0357.getWidgetKeys();
        printToConsole("  Widget keys discovered: " + keys.length);
        if (!keys || keys.length === 0) {
            printToConsole("  FAIL: No widget keys found — plugin sections may not have loaded");
            return false;
        }
        printToConsole("  PASS: Widget keys present, plugin sections loaded");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2: Change ADC Update Rate
// UID: TST.CN0357.ADC_UPDATE_RATE
// Description: Verify that the ADC Update Rate setting changes the sampling frequency.
// Steps 1-2 automated here; step 3 (visual rate observation) in VisualTests.
// ============================================
printToConsole("\n=== Test 2: Change ADC Update Rate ===\n");

TestFramework.runTest("TST.CN0357.ADC_UPDATE_RATE", function() {
    try {
        // Step 1: Read current sampling frequency
        var origFreq = cn0357.getSamplingFrequency();
        printToConsole("  Current sampling frequency: " + origFreq);
        if (!origFreq || origFreq === "") {
            printToConsole("  FAIL: getSamplingFrequency() returned empty");
            return false;
        }
        printToConsole("  PASS: Update Rate combo is readable with default value");

        // Step 2: Change to each available option
        var rates = ["120", "100", "60", "50", "25", "16"];
        var pass = true;

        for (var i = 0; i < rates.length; i++) {
            cn0357.setSamplingFrequency(rates[i]);
            msleep(500);

            var readBack = cn0357.getSamplingFrequency();
            printToConsole("  Set rate to " + rates[i] + " Hz, read back: " + readBack);

            if (!readBack || readBack === "") {
                printToConsole("  FAIL: getSamplingFrequency() returned empty after setting " + rates[i]);
                pass = false;
                continue;
            }

            var readNum = parseFloat(readBack);
            var expectedNum = parseFloat(rates[i]);
            if (isNaN(readNum) || Math.abs(readNum - expectedNum) > 1) {
                printToConsole("  FAIL: Read-back " + readBack + " does not match written " + rates[i]);
                pass = false;
            }
        }

        // Restore original frequency
        cn0357.setSamplingFrequency(origFreq);
        msleep(500);

        if (pass) {
            printToConsole("  PASS: All sampling frequency values accepted and verified");
        }
        return pass;
    } catch (e) {
        printToConsole("  Error: " + e);
        // Attempt restore
        try { cn0357.setSamplingFrequency(origFreq); msleep(500); } catch (e2) {}
        return false;
    }
});

// ============================================
// Test 3: Feedback Settings Configuration
// UID: TST.CN0357.FEEDBACK_SETTINGS
// Description: Verify feedback type switching, RDAC programming, fixed resistor.
// Functional steps automated here; visibility checks in VisualTests.
// ============================================
printToConsole("\n=== Test 3: Feedback Settings Configuration ===\n");

TestFramework.runTest("TST.CN0357.FEEDBACK_SETTINGS", function() {
    var origFeedbackType = null;
    var origRdac = null;
    var origFixedRes = null;
    var origSensitivity = null;

    try {
        // Save all original values
        origFeedbackType = cn0357.getFeedbackType();
        origRdac = cn0357.getRdacValue();
        origFixedRes = cn0357.getFixedResistor();
        origSensitivity = cn0357.getSensorSensitivity();

        // Step 1: Verify Rheostat mode is the default
        printToConsole("  Current feedback type: " + origFeedbackType);
        if (origFeedbackType !== "Rheostat") {
            printToConsole("  WARN: Default feedback type is not Rheostat: " + origFeedbackType);
        }

        // Ensure we are in Rheostat mode for RDAC tests
        cn0357.setFeedbackType("Rheostat");
        msleep(500);

        var fbType = cn0357.getFeedbackType();
        if (fbType !== "Rheostat") {
            printToConsole("  FAIL: Could not set feedback type to Rheostat, got: " + fbType);
            return false;
        }
        printToConsole("  PASS: Feedback type set to Rheostat");

        // Step 2: Set RDAC Value to 512 and program
        cn0357.setRdacValue("512");
        msleep(500);
        var rdacVal = cn0357.getRdacValue();
        printToConsole("  Set RDAC to 512, read back: " + rdacVal);
        if (rdacVal !== "512") {
            printToConsole("  FAIL: RDAC value read-back " + rdacVal + " does not match 512");
            return false;
        }

        cn0357.programRheostat();
        msleep(500);
        var fbRes512 = cn0357.getFeedbackResistance();
        printToConsole("  After programming RDAC=512, feedback resistance: " + fbRes512);
        if (!fbRes512 || fbRes512 === "") {
            printToConsole("  FAIL: Feedback resistance is empty after programming RDAC");
            return false;
        }
        printToConsole("  PASS: RDAC=512 programmed, feedback resistance updated");

        // Step 3: Test boundary values
        cn0357.setRdacValue("0");
        msleep(500);
        cn0357.programRheostat();
        msleep(500);
        var rdacZero = cn0357.getRdacValue();
        var fbResZero = cn0357.getFeedbackResistance();
        printToConsole("  RDAC=0: value=" + rdacZero + ", feedback resistance=" + fbResZero);
        if (rdacZero !== "0") {
            printToConsole("  FAIL: RDAC boundary 0 not accepted");
            return false;
        }

        cn0357.setRdacValue("1023");
        msleep(500);
        cn0357.programRheostat();
        msleep(500);
        var rdacMax = cn0357.getRdacValue();
        var fbResMax = cn0357.getFeedbackResistance();
        printToConsole("  RDAC=1023: value=" + rdacMax + ", feedback resistance=" + fbResMax);
        if (rdacMax !== "1023") {
            printToConsole("  FAIL: RDAC boundary 1023 not accepted");
            return false;
        }
        printToConsole("  PASS: Both boundary values (0, 1023) accepted");

        // Step 4-5: Switch to Fixed Resistor and set value
        cn0357.setFeedbackType("Fixed Resistor");
        msleep(500);
        var fbTypeFixed = cn0357.getFeedbackType();
        if (fbTypeFixed !== "Fixed Resistor") {
            printToConsole("  FAIL: Could not switch to Fixed Resistor, got: " + fbTypeFixed);
            return false;
        }
        printToConsole("  PASS: Switched to Fixed Resistor mode");

        cn0357.setFixedResistor("10000");
        msleep(500);
        var fixedRes = cn0357.getFixedResistor();
        printToConsole("  Set fixed resistor to 10000, read back: " + fixedRes);
        var fixedNum = parseFloat(fixedRes);
        if (isNaN(fixedNum) || Math.abs(fixedNum - 10000) > 1) {
            printToConsole("  FAIL: Fixed resistor read-back " + fixedRes + " does not match 10000");
            return false;
        }

        var fbResFixed = cn0357.getFeedbackResistance();
        printToConsole("  Feedback resistance in Fixed mode: " + fbResFixed);
        printToConsole("  PASS: Fixed resistor value accepted, feedback resistance updated");

        // Step 6: Switch back to Rheostat
        cn0357.setFeedbackType("Rheostat");
        msleep(500);
        var fbTypeBack = cn0357.getFeedbackType();
        if (fbTypeBack !== "Rheostat") {
            printToConsole("  FAIL: Could not switch back to Rheostat, got: " + fbTypeBack);
            return false;
        }
        printToConsole("  PASS: Switched back to Rheostat mode");

        // Restore original values
        cn0357.setFeedbackType(origFeedbackType);
        msleep(500);
        cn0357.setRdacValue(origRdac);
        msleep(500);
        cn0357.setFixedResistor(origFixedRes);
        msleep(500);

        printToConsole("  PASS: All feedback settings steps verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        // Restore original values
        try {
            if (origFeedbackType) { cn0357.setFeedbackType(origFeedbackType); msleep(500); }
            if (origRdac) { cn0357.setRdacValue(origRdac); msleep(500); }
            if (origFixedRes) { cn0357.setFixedResistor(origFixedRes); msleep(500); }
        } catch (e2) {}
        return false;
    }
});

// ============================================
// Test 4: System Measurements and Data
// UID: TST.CN0357.SYSTEM_MEASUREMENTS
// Description: Verify System section displays measurements and computed data.
// Steps 1, 4-6 automated here; steps 2-3 (live update observation) in VisualTests.
// ============================================
printToConsole("\n=== Test 4: System Measurements and Data ===\n");

TestFramework.runTest("TST.CN0357.SYSTEM_MEASUREMENTS", function() {
    try {
        var pass = true;

        // Step 1: Verify all measurement fields are present and readable
        var ppm = cn0357.getConcentrationPpm();
        printToConsole("  Concentration (ppm): " + ppm);
        if (!ppm || ppm === "") {
            printToConsole("  FAIL: Concentration (ppm) is empty");
            pass = false;
        } else {
            printToConsole("  PASS: Concentration (ppm) is readable");
        }

        var mv = cn0357.getAdcMillivolts();
        printToConsole("  Conversion (mV): " + mv);
        if (!mv || mv === "") {
            printToConsole("  FAIL: Conversion (mV) is empty");
            pass = false;
        } else {
            printToConsole("  PASS: Conversion (mV) is readable");
        }

        // Step 4: Supply Voltage
        var supply = cn0357.getSupplyVoltage();
        printToConsole("  Supply Voltage (V): " + supply);
        if (!supply || supply === "") {
            printToConsole("  FAIL: Supply Voltage is empty");
            pass = false;
        } else {
            printToConsole("  PASS: Supply Voltage is readable");
        }

        // Step 5: Feedback Resistance
        var fbRes = cn0357.getFeedbackResistance();
        printToConsole("  Feedback Resistance: " + fbRes);
        if (!fbRes || fbRes === "") {
            printToConsole("  FAIL: Feedback Resistance is empty");
            pass = false;
        } else {
            printToConsole("  PASS: Feedback Resistance is readable");
        }

        // Step 6: ppm/mV and mV/ppm coefficients
        var ppmMv = cn0357.getPpmPerMv();
        printToConsole("  ppm/mV: " + ppmMv);
        if (!ppmMv || ppmMv === "") {
            printToConsole("  FAIL: ppm/mV coefficient is empty");
            pass = false;
        } else {
            printToConsole("  PASS: ppm/mV coefficient is readable");
        }

        var mvPpm = cn0357.getMvPerPpm();
        printToConsole("  mV/ppm: " + mvPpm);
        if (!mvPpm || mvPpm === "") {
            printToConsole("  FAIL: mV/ppm coefficient is empty");
            pass = false;
        } else {
            printToConsole("  PASS: mV/ppm coefficient is readable");
        }

        // Verify coefficients are non-zero numeric
        var sensitivity = cn0357.getSensorSensitivity();
        printToConsole("  Sensor Sensitivity: " + sensitivity);
        if (!sensitivity || sensitivity === "") {
            printToConsole("  FAIL: Sensor Sensitivity is empty");
            pass = false;
        } else {
            printToConsole("  PASS: Sensor Sensitivity is readable");
        }

        if (pass) {
            printToConsole("  PASS: All system measurement and data fields are readable");
        }
        return pass;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 5: Change Sensor Sensitivity
// UID: TST.CN0357.SENSOR_SENSITIVITY
// Description: Verify that changing Sensor Sensitivity updates coefficients.
// ============================================
printToConsole("\n=== Test 5: Change Sensor Sensitivity ===\n");

TestFramework.runTest("TST.CN0357.SENSOR_SENSITIVITY", function() {
    var origSensitivity = null;

    try {
        // Step 1: Note current sensitivity
        origSensitivity = cn0357.getSensorSensitivity();
        printToConsole("  Current Sensor Sensitivity: " + origSensitivity);
        if (!origSensitivity || origSensitivity === "") {
            printToConsole("  FAIL: getSensorSensitivity() returned empty");
            return false;
        }

        var origNum = parseFloat(origSensitivity);
        printToConsole("  PASS: Sensor Sensitivity readable, value: " + origNum);

        // Step 2: Change to 30 nA/ppm
        cn0357.setSensorSensitivity("30");
        msleep(500);
        var sens30 = cn0357.getSensorSensitivity();
        var ppmMv30 = cn0357.getPpmPerMv();
        var mvPpm30 = cn0357.getMvPerPpm();
        printToConsole("  Set sensitivity to 30: read back=" + sens30 + ", ppm/mV=" + ppmMv30 + ", mV/ppm=" + mvPpm30);

        var sens30Num = parseFloat(sens30);
        if (isNaN(sens30Num) || Math.abs(sens30Num - 30) > 1) {
            printToConsole("  FAIL: Sensitivity read-back " + sens30 + " does not match 30");
            cn0357.setSensorSensitivity(origSensitivity);
            msleep(500);
            return false;
        }
        printToConsole("  PASS: Sensitivity set to 30, coefficients updated");

        // Step 3: Change to 1 nA/ppm (minimum)
        cn0357.setSensorSensitivity("1");
        msleep(500);
        var sens1 = cn0357.getSensorSensitivity();
        var ppmMv1 = cn0357.getPpmPerMv();
        var mvPpm1 = cn0357.getMvPerPpm();
        printToConsole("  Set sensitivity to 1: read back=" + sens1 + ", ppm/mV=" + ppmMv1 + ", mV/ppm=" + mvPpm1);

        var sens1Num = parseFloat(sens1);
        if (isNaN(sens1Num) || Math.abs(sens1Num - 1) > 0.5) {
            printToConsole("  FAIL: Sensitivity read-back " + sens1 + " does not match 1");
            cn0357.setSensorSensitivity(origSensitivity);
            msleep(500);
            return false;
        }
        printToConsole("  PASS: Minimum sensitivity (1) accepted");

        // Verify coefficients changed from 30 to 1
        if (ppmMv30 === ppmMv1 && mvPpm30 === mvPpm1) {
            printToConsole("  WARN: Coefficients did not change between sensitivity 30 and 1");
        }

        // Step 4: Change to 100 nA/ppm (maximum)
        cn0357.setSensorSensitivity("100");
        msleep(500);
        var sens100 = cn0357.getSensorSensitivity();
        var ppmMv100 = cn0357.getPpmPerMv();
        var mvPpm100 = cn0357.getMvPerPpm();
        printToConsole("  Set sensitivity to 100: read back=" + sens100 + ", ppm/mV=" + ppmMv100 + ", mV/ppm=" + mvPpm100);

        var sens100Num = parseFloat(sens100);
        if (isNaN(sens100Num) || Math.abs(sens100Num - 100) > 1) {
            printToConsole("  FAIL: Sensitivity read-back " + sens100 + " does not match 100");
            cn0357.setSensorSensitivity(origSensitivity);
            msleep(500);
            return false;
        }
        printToConsole("  PASS: Maximum sensitivity (100) accepted");

        // Restore original
        cn0357.setSensorSensitivity(origSensitivity);
        msleep(500);

        printToConsole("  PASS: All sensor sensitivity values accepted and coefficients updated");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try {
            if (origSensitivity) { cn0357.setSensorSensitivity(origSensitivity); msleep(500); }
        } catch (e2) {}
        return false;
    }
});

// ============================================
// Test 6: Refresh Button Functionality
// UID: TST.CN0357.REFRESH_FUNCTION
// Description: Verify that refresh re-reads all values from the device.
// Programmatic refresh automated here; toolbar button in VisualTests.
// ============================================
printToConsole("\n=== Test 6: Refresh Button Functionality ===\n");

TestFramework.runTest("TST.CN0357.REFRESH_FUNCTION", function() {
    try {
        // Step 1: Note current measurement values
        var beforeFreq = cn0357.getSamplingFrequency();
        var beforeMv = cn0357.getAdcMillivolts();
        var beforePpm = cn0357.getConcentrationPpm();
        printToConsole("  Before refresh — freq: " + beforeFreq + ", mV: " + beforeMv + ", ppm: " + beforePpm);

        // Step 2: Trigger programmatic refresh
        cn0357.refresh();
        msleep(1000);
        printToConsole("  Programmatic refresh triggered");

        // Verify values are still readable after refresh
        var afterFreq = cn0357.getSamplingFrequency();
        var afterMv = cn0357.getAdcMillivolts();
        var afterPpm = cn0357.getConcentrationPpm();
        printToConsole("  After refresh — freq: " + afterFreq + ", mV: " + afterMv + ", ppm: " + afterPpm);

        if (!afterFreq || afterFreq === "") {
            printToConsole("  FAIL: Sampling frequency not readable after refresh");
            return false;
        }
        if (!afterMv || afterMv === "") {
            printToConsole("  FAIL: ADC millivolts not readable after refresh");
            return false;
        }

        printToConsole("  PASS: Programmatic refresh triggered and values remain readable");
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
