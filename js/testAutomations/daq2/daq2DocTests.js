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

// DAQ2 Plugin — Automated Doc Tests (Category A)
// Maps RST test cases from docs/tests/plugins/daq2/daq2_tests.rst to API calls.
// Device: axi-ad9680-hpc (AD9680 ADC)
// Emulator: iio-emu generic packages/daq2/emu-xml/daq2.xml at ip:127.0.0.1
// Run: cd build && ./scopy --script ../js/testAutomations/daq2/daq2DocTests.js

// Missing API Report
// ------------------
// Category B tests (visual checks, not in this file):
//   TST.DAQ2.ADC_SECTION_VISIBLE      — requires visual inspection of UI layout
//   TST.DAQ2.ADC_SAMPLING_FREQ_DISPLAYED — sampling_frequency label is a plain QLabel,
//                                          not in IIOWidgetGroup; no API getter
//   TST.DAQ2.DAC_SECTION_VISIBLE      — requires visual inspection of UI layout
//   TST.DAQ2.DAC_SAMPLING_FREQ_DISPLAYED — same as ADC sampling frequency

evaluateFile("js/testAutomations/common/testFramework.js");

var AD9680_TEST_MODES = [
    "off", "midscale_short", "pos_fullscale", "neg_fullscale",
    "checkerboard", "pn9", "pn23", "one_zero_toggle", "ramp"
];

// ============================================================
// Setup
// ============================================================

TestFramework.init("DAQ2 Doc Tests");

if (!TestFramework.connectToDevice("ip:127.0.0.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    TestFramework.disconnectFromDevice();
    TestFramework.printSummary();
    scopy.exit();
}

if (!switchToTool("DAQ2")) {
    printToConsole("WARNING: Could not switch to DAQ2 tool, tests will proceed via API");
}

// ============================================================
// TST.DAQ2.PLUGIN_LOADS
// ============================================================

TestFramework.runTest("TST.DAQ2.PLUGIN_LOADS", function() {
    var tools = daq2.getTools();
    printToConsole("  Tools: " + tools);
    if (!tools || tools.length === 0) {
        return "getTools() returned empty — DAQ2 plugin not loaded";
    }

    var keys = daq2.getWidgetKeys();
    printToConsole("  Widget keys: " + keys);
    var expected = [
        "axi-ad9680-hpc/voltage0_in/test_mode",
        "axi-ad9680-hpc/voltage1_in/test_mode"
    ];
    for (var i = 0; i < expected.length; i++) {
        var found = false;
        for (var j = 0; j < keys.length; j++) {
            if (keys[j] === expected[i]) { found = true; break; }
        }
        if (!found) {
            return "Expected widget key not found: " + expected[i];
        }
        printToConsole("  ✓ key present: " + expected[i]);
    }
    return true;
});

// ============================================================
// TST.DAQ2.ADC_CH0_TEST_MODE_WRITE_READBACK
// ============================================================

TestFramework.runTest("TST.DAQ2.ADC_CH0_TEST_MODE_WRITE_READBACK", function() {
    var original = daq2.getAdcCh0TestMode();
    printToConsole("  Original Ch0 test_mode: " + original);

    var testValue = (original === "pn9") ? "off" : "pn9";
    try {
        daq2.setAdcCh0TestMode(testValue);
        msleep(500);
        var readBack = daq2.getAdcCh0TestMode();
        printToConsole("  Written: '" + testValue + "'  Read back: '" + readBack + "'");

        daq2.setAdcCh0TestMode(original);
        msleep(500);

        if (readBack !== testValue) {
            return "Readback mismatch: wrote '" + testValue + "', got '" + readBack + "'";
        }
        printToConsole("  ✓ Write-readback passed");
        return true;
    } catch (e) {
        daq2.setAdcCh0TestMode(original);
        msleep(500);
        throw e;
    }
});

// ============================================================
// TST.DAQ2.ADC_CH1_TEST_MODE_WRITE_READBACK
// ============================================================

TestFramework.runTest("TST.DAQ2.ADC_CH1_TEST_MODE_WRITE_READBACK", function() {
    var original = daq2.getAdcCh1TestMode();
    printToConsole("  Original Ch1 test_mode: " + original);

    var testValue = (original === "pn23") ? "off" : "pn23";
    try {
        daq2.setAdcCh1TestMode(testValue);
        msleep(500);
        var readBack = daq2.getAdcCh1TestMode();
        printToConsole("  Written: '" + testValue + "'  Read back: '" + readBack + "'");

        daq2.setAdcCh1TestMode(original);
        msleep(500);

        if (readBack !== testValue) {
            return "Readback mismatch: wrote '" + testValue + "', got '" + readBack + "'";
        }
        printToConsole("  ✓ Write-readback passed");
        return true;
    } catch (e) {
        daq2.setAdcCh1TestMode(original);
        msleep(500);
        throw e;
    }
});

// ============================================================
// TST.DAQ2.REFRESH_UPDATES_WIDGETS
// ============================================================

TestFramework.runTest("TST.DAQ2.REFRESH_UPDATES_WIDGETS", function() {
    var ch0Before = daq2.getAdcCh0TestMode();
    var ch1Before = daq2.getAdcCh1TestMode();
    printToConsole("  Before refresh — ch0: '" + ch0Before + "'  ch1: '" + ch1Before + "'");

    if (!ch0Before || ch0Before === "") {
        return "Ch0 test_mode empty before refresh";
    }
    if (!ch1Before || ch1Before === "") {
        return "Ch1 test_mode empty before refresh";
    }

    daq2.refresh();
    msleep(1000);

    var ch0After = daq2.getAdcCh0TestMode();
    var ch1After = daq2.getAdcCh1TestMode();
    printToConsole("  After refresh  — ch0: '" + ch0After + "'  ch1: '" + ch1After + "'");

    if (!ch0After || ch0After === "") {
        return "Ch0 test_mode empty after refresh";
    }
    if (!ch1After || ch1After === "") {
        return "Ch1 test_mode empty after refresh";
    }
    printToConsole("  ✓ Both channels readable after refresh");
    return true;
});

// ============================================================
// TST.DAQ2.ADC_CH0_TEST_MODE_ALL_OPTIONS
// ============================================================

TestFramework.runTest("TST.DAQ2.ADC_CH0_TEST_MODE_ALL_OPTIONS", function() {
    var original = daq2.getAdcCh0TestMode();
    printToConsole("  Original Ch0 test_mode: " + original);
    try {
        for (var i = 0; i < AD9680_TEST_MODES.length; i++) {
            var opt = AD9680_TEST_MODES[i];
            daq2.setAdcCh0TestMode(opt);
            msleep(500);
            var readBack = daq2.getAdcCh0TestMode();
            if (readBack !== opt) {
                daq2.setAdcCh0TestMode(original);
                msleep(500);
                return "Option '" + opt + "' failed: got '" + readBack + "'";
            }
            printToConsole("  ✓ '" + opt + "'");
        }
        daq2.setAdcCh0TestMode(original);
        msleep(500);
        return true;
    } catch (e) {
        daq2.setAdcCh0TestMode(original);
        msleep(500);
        throw e;
    }
});

// ============================================================
// TST.DAQ2.ADC_CH1_TEST_MODE_ALL_OPTIONS
// ============================================================

TestFramework.runTest("TST.DAQ2.ADC_CH1_TEST_MODE_ALL_OPTIONS", function() {
    var original = daq2.getAdcCh1TestMode();
    printToConsole("  Original Ch1 test_mode: " + original);
    try {
        for (var i = 0; i < AD9680_TEST_MODES.length; i++) {
            var opt = AD9680_TEST_MODES[i];
            daq2.setAdcCh1TestMode(opt);
            msleep(500);
            var readBack = daq2.getAdcCh1TestMode();
            if (readBack !== opt) {
                daq2.setAdcCh1TestMode(original);
                msleep(500);
                return "Option '" + opt + "' failed: got '" + readBack + "'";
            }
            printToConsole("  ✓ '" + opt + "'");
        }
        daq2.setAdcCh1TestMode(original);
        msleep(500);
        return true;
    } catch (e) {
        daq2.setAdcCh1TestMode(original);
        msleep(500);
        throw e;
    }
});

// ============================================================
// Cleanup
// ============================================================

TestFramework.disconnectFromDevice();
TestFramework.printSummary();
scopy.exit();
