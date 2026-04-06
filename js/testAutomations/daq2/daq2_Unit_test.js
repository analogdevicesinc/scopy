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

// DAQ2 Plugin Unit Tests
// Tests every IIOWidget attribute and API getter/setter method via readWidget/writeWidget.
// Device: axi-ad9680-hpc (ADC, AD9680)
// Emulator: iio-emu generic daq2.xml at ip:127.0.0.1
// Run: ./scopy --script js/testAutomations/daq2/daq2_Unit_test.js

// Load test framework
evaluateFile("js/testAutomations/common/testFramework.js");

// ============================================
// Standard helper functions
// ============================================

// Valid test_mode options (from AD9680_TEST_MODES in daq2_api.cpp)
var AD9680_TEST_MODES = [
    "off", "midscale_short", "pos_fullscale", "neg_fullscale",
    "checkerboard", "pn9", "pn23", "one_zero_toggle", "ramp"
];

/**
 * testCombo — Write each valid option to a combo widget via writeWidget,
 * read back and verify exact match. Restores original value when done.
 * @param {string} key     - Widget key (e.g. "axi-ad9680-hpc/voltage0_in/test_mode")
 * @param {Array}  options - Valid string options to cycle through
 * @returns true on full pass, error string on first mismatch
 */
function testCombo(key, options) {
    var original = daq2.readWidget(key);
    printToConsole("  Original value: " + original);
    try {
        for (var i = 0; i < options.length; i++) {
            var opt = options[i];
            daq2.writeWidget(key, opt);
            msleep(500);
            var readBack = daq2.readWidget(key);
            if (readBack !== opt) {
                printToConsole("  FAIL option '" + opt + "': got '" + readBack + "'");
                daq2.writeWidget(key, original);
                msleep(500);
                return "Combo roundtrip failed for option '" + opt + "': got '" + readBack + "'";
            }
            printToConsole("  ✓ option '" + opt + "'");
        }
    } catch (e) {
        daq2.writeWidget(key, original);
        msleep(500);
        throw e;
    }
    daq2.writeWidget(key, original);
    msleep(500);
    return true;
}

/**
 * testBadValueCombo — Attempt to write an invalid value via the typed API setter.
 * The setter validates against AD9680_TEST_MODES and silently rejects unknown values.
 * Verifies that the widget value is unchanged after the rejected write.
 * @param {string} getterFn - API getter function name (e.g. "getAdcCh0TestMode")
 * @param {string} setterFn - API setter function name (e.g. "setAdcCh0TestMode")
 * @returns true if value is unchanged after bad write, error string otherwise
 */
function testBadValueCombo(getterFn, setterFn) {
    var original = daq2[getterFn]();
    printToConsole("  Original value: " + original);
    daq2[setterFn]("invalid_test_mode_xyz");
    msleep(500);
    var afterBad = daq2[getterFn]();
    printToConsole("  Value after invalid write: " + afterBad);
    if (afterBad !== original) {
        return "Bad value was accepted: value changed from '" + original + "' to '" + afterBad + "'";
    }
    printToConsole("  ✓ Invalid value correctly rejected");
    return true;
}

// ============================================
// Test suite setup
// ============================================

TestFramework.init("DAQ2 Plugin Unit Tests");

if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    TestFramework.disconnectFromDevice();
    TestFramework.printSummary();
    scopy.exit();
}

if (!switchToTool("DAQ2")) {
    printToConsole("WARNING: Could not switch to DAQ2 tool, tests will proceed via API");
}

// ============================================
// Utility Tests
// ============================================
printToConsole("\n=== Utility Tests ===\n");

TestFramework.runTest("UNIT.UTIL.GET_TOOLS", function() {
    var tools = daq2.getTools();
    printToConsole("  Tools: " + tools);
    if (!tools || tools.length === 0) {
        return "getTools() returned empty list";
    }
    return true;
});

TestFramework.runTest("UNIT.UTIL.GET_WIDGET_KEYS", function() {
    var keys = daq2.getWidgetKeys();
    printToConsole("  Widget keys (" + keys.length + "): " + keys);
    if (!keys || keys.length === 0) {
        return "getWidgetKeys() returned empty list";
    }
    var expectedKeys = [
        "axi-ad9680-hpc/voltage0_in/test_mode",
        "axi-ad9680-hpc/voltage1_in/test_mode"
    ];
    for (var i = 0; i < expectedKeys.length; i++) {
        var found = false;
        for (var j = 0; j < keys.length; j++) {
            if (keys[j] === expectedKeys[i]) { found = true; break; }
        }
        if (!found) {
            return "Expected key '" + expectedKeys[i] + "' not found in widget keys";
        }
        printToConsole("  ✓ found '" + expectedKeys[i] + "'");
    }
    return true;
});

// ============================================
// ADC Ch0 Test Mode (axi-ad9680-hpc voltage0_in)
// ============================================
printToConsole("\n=== ADC Ch0 Test Mode ===\n");

TestFramework.runTest("UNIT.ADC.CH0_TEST_MODE", function() {
    return testCombo("axi-ad9680-hpc/voltage0_in/test_mode", AD9680_TEST_MODES);
});

TestFramework.runTest("UNIT.ADC.CH0_TEST_MODE_API", function() {
    var original = daq2.getAdcCh0TestMode();
    printToConsole("  Original via API: " + original);
    var testOpt = (original === "pn9") ? "off" : "pn9";
    try {
        daq2.setAdcCh0TestMode(testOpt);
        msleep(500);
        var readBack = daq2.getAdcCh0TestMode();
        printToConsole("  Written: '" + testOpt + "'  Read back: '" + readBack + "'");
        daq2.setAdcCh0TestMode(original);
        msleep(500);
        if (readBack !== testOpt) {
            return "API setter/getter mismatch: wrote '" + testOpt + "', got '" + readBack + "'";
        }
        return true;
    } catch (e) {
        daq2.setAdcCh0TestMode(original);
        msleep(500);
        throw e;
    }
});

TestFramework.runTest("UNIT.ADC.CH0_TEST_MODE_BAD_VALUE", function() {
    return testBadValueCombo("getAdcCh0TestMode", "setAdcCh0TestMode");
});

// ============================================
// ADC Ch1 Test Mode (axi-ad9680-hpc voltage1_in)
// ============================================
printToConsole("\n=== ADC Ch1 Test Mode ===\n");

TestFramework.runTest("UNIT.ADC.CH1_TEST_MODE", function() {
    return testCombo("axi-ad9680-hpc/voltage1_in/test_mode", AD9680_TEST_MODES);
});

TestFramework.runTest("UNIT.ADC.CH1_TEST_MODE_API", function() {
    var original = daq2.getAdcCh1TestMode();
    printToConsole("  Original via API: " + original);
    var testOpt = (original === "pn9") ? "off" : "pn9";
    try {
        daq2.setAdcCh1TestMode(testOpt);
        msleep(500);
        var readBack = daq2.getAdcCh1TestMode();
        printToConsole("  Written: '" + testOpt + "'  Read back: '" + readBack + "'");
        daq2.setAdcCh1TestMode(original);
        msleep(500);
        if (readBack !== testOpt) {
            return "API setter/getter mismatch: wrote '" + testOpt + "', got '" + readBack + "'";
        }
        return true;
    } catch (e) {
        daq2.setAdcCh1TestMode(original);
        msleep(500);
        throw e;
    }
});

TestFramework.runTest("UNIT.ADC.CH1_TEST_MODE_BAD_VALUE", function() {
    return testBadValueCombo("getAdcCh1TestMode", "setAdcCh1TestMode");
});

// ============================================
// Complex Tests
// ============================================
printToConsole("\n=== Complex Tests ===\n");

// C1 — Refresh Cycle
// Calls refresh() and verifies both widget values are still readable afterward.
TestFramework.runTest("UNIT.UTIL.REFRESH_CYCLE", function() {
    var ch0Before = daq2.readWidget("axi-ad9680-hpc/voltage0_in/test_mode");
    var ch1Before = daq2.readWidget("axi-ad9680-hpc/voltage1_in/test_mode");
    printToConsole("  Before refresh — ch0: '" + ch0Before + "'  ch1: '" + ch1Before + "'");

    daq2.refresh();
    msleep(1000);

    var ch0After = daq2.readWidget("axi-ad9680-hpc/voltage0_in/test_mode");
    var ch1After = daq2.readWidget("axi-ad9680-hpc/voltage1_in/test_mode");
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

// ============================================
// Cleanup
// ============================================
TestFramework.disconnectFromDevice();
TestFramework.printSummary();
scopy.exit();
