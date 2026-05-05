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

// AD6676 Basic Unit Tests
// Tests every IIOWidget attribute via readWidget/writeWidget and API getter/setter.
// Source: packages/ad6676/plugins/ad6676plugin/
// Device: axi-ad6676-hpc / channel: voltage0 (input)

evaluateFile("../js/testAutomations/common/testFramework.js");

TestFramework.init("AD6676 Basic Unit Tests");

if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

if (!switchToTool("AD6676")) {
    printToConsole("ERROR: Cannot switch to AD6676 tool");
    TestFramework.disconnectFromDevice();
    scopy.exit();
}

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

var PHY = "axi-ad6676-hpc/voltage0_in/";

// ---------------------------------------------------------------------------
// Standard helper functions
// ---------------------------------------------------------------------------

function testRange(uid, key, min, mid, max, tol) {
    TestFramework.runTest(uid, function() {
        var orig = ad6676.readWidget(key);
        try {
            // Write min
            ad6676.writeWidget(key, min.toString());
            msleep(500);
            var readMin = parseFloat(ad6676.readWidget(key));
            if (isNaN(readMin) || Math.abs(readMin - min) > tol) {
                printToConsole("  FAIL at min: wrote " + min + " read " + readMin);
                ad6676.writeWidget(key, orig);
                msleep(500);
                return "min value not within tolerance: wrote " + min + " read " + readMin;
            }

            // Write mid
            ad6676.writeWidget(key, mid.toString());
            msleep(500);
            var readMid = parseFloat(ad6676.readWidget(key));
            if (isNaN(readMid) || Math.abs(readMid - mid) > tol) {
                printToConsole("  FAIL at mid: wrote " + mid + " read " + readMid);
                ad6676.writeWidget(key, orig);
                msleep(500);
                return "mid value not within tolerance: wrote " + mid + " read " + readMid;
            }

            // Write max
            ad6676.writeWidget(key, max.toString());
            msleep(500);
            var readMax = parseFloat(ad6676.readWidget(key));
            if (isNaN(readMax) || Math.abs(readMax - max) > tol) {
                printToConsole("  FAIL at max: wrote " + max + " read " + readMax);
                ad6676.writeWidget(key, orig);
                msleep(500);
                return "max value not within tolerance: wrote " + max + " read " + readMax;
            }

            ad6676.writeWidget(key, orig);
            msleep(500);
            return true;
        } catch (e) {
            try { ad6676.writeWidget(key, orig); msleep(500); } catch (e2) {}
            return "exception: " + e;
        }
    });
}

function testBadValueRange(uid, key, min, max, belowMin, aboveMax) {
    TestFramework.runTest(uid, function() {
        var orig = ad6676.readWidget(key);
        try {
            // Write below min — expect clamped to >= min
            ad6676.writeWidget(key, belowMin.toString());
            msleep(500);
            var readBelow = parseFloat(ad6676.readWidget(key));
            if (!isNaN(readBelow) && readBelow < min) {
                printToConsole("  FAIL: below-min not clamped: wrote " + belowMin + " read " + readBelow + " min=" + min);
                ad6676.writeWidget(key, orig);
                msleep(500);
                return "below-min value accepted without clamping";
            }

            // Write above max — expect clamped to <= max
            ad6676.writeWidget(key, aboveMax.toString());
            msleep(500);
            var readAbove = parseFloat(ad6676.readWidget(key));
            if (!isNaN(readAbove) && readAbove > max) {
                printToConsole("  FAIL: above-max not clamped: wrote " + aboveMax + " read " + readAbove + " max=" + max);
                ad6676.writeWidget(key, orig);
                msleep(500);
                return "above-max value accepted without clamping";
            }

            ad6676.writeWidget(key, orig);
            msleep(500);
            return true;
        } catch (e) {
            try { ad6676.writeWidget(key, orig); msleep(500); } catch (e2) {}
            return "exception: " + e;
        }
    });
}

function testCombo(uid, key, options) {
    TestFramework.runTest(uid, function() {
        var orig = ad6676.readWidget(key);
        try {
            for (var i = 0; i < options.length; i++) {
                ad6676.writeWidget(key, options[i]);
                msleep(500);
                var readBack = ad6676.readWidget(key);
                if (readBack !== options[i]) {
                    printToConsole("  FAIL: wrote '" + options[i] + "' read '" + readBack + "'");
                    ad6676.writeWidget(key, orig);
                    msleep(500);
                    return "combo option mismatch: wrote '" + options[i] + "' read '" + readBack + "'";
                }
            }
            ad6676.writeWidget(key, orig);
            msleep(500);
            return true;
        } catch (e) {
            try { ad6676.writeWidget(key, orig); msleep(500); } catch (e2) {}
            return "exception: " + e;
        }
    });
}

function testBadValueCombo(uid, key, invalidOption) {
    TestFramework.runTest(uid, function() {
        var orig = ad6676.readWidget(key);
        try {
            ad6676.writeWidget(key, invalidOption);
            msleep(500);
            var readBack = ad6676.readWidget(key);
            // Invalid option should be rejected — value should remain unchanged or be a valid option
            if (readBack === invalidOption) {
                printToConsole("  FAIL: invalid option '" + invalidOption + "' was accepted");
                ad6676.writeWidget(key, orig);
                msleep(500);
                return "invalid combo option was accepted: " + invalidOption;
            }
            ad6676.writeWidget(key, orig);
            msleep(500);
            return true;
        } catch (e) {
            try { ad6676.writeWidget(key, orig); msleep(500); } catch (e2) {}
            return "exception: " + e;
        }
    });
}

function testReadOnly(uid, key) {
    TestFramework.runTest(uid, function() {
        try {
            var val = ad6676.readWidget(key);
            if (!val || val === "") {
                return "readOnly widget returned empty string";
            }
            printToConsole("  Value: " + val);
            return true;
        } catch (e) {
            return "exception: " + e;
        }
    });
}

function testConversion(uid, apiSetter, widgetKey, setVal, expectedRawFn, tol) {
    TestFramework.runTest(uid, function() {
        var origRaw = ad6676.readWidget(widgetKey);
        try {
            apiSetter(setVal);
            msleep(500);
            var widgetRaw = parseFloat(ad6676.readWidget(widgetKey));
            var expectedRaw = expectedRawFn(setVal);
            if (isNaN(widgetRaw) || Math.abs(widgetRaw - expectedRaw) > tol) {
                printToConsole("  FAIL: API set=" + setVal + " widgetRaw=" + widgetRaw + " expectedRaw=" + expectedRaw);
                ad6676.writeWidget(widgetKey, origRaw);
                msleep(500);
                return "conversion mismatch: widgetRaw=" + widgetRaw + " expectedRaw=" + expectedRaw;
            }
            ad6676.writeWidget(widgetKey, origRaw);
            msleep(500);
            return true;
        } catch (e) {
            try { ad6676.writeWidget(widgetKey, origRaw); msleep(500); } catch (e2) {}
            return "exception: " + e;
        }
    });
}

function runDataDrivenTests(tests) {
    for (var i = 0; i < tests.length; i++) {
        var t = tests[i];
        if (t.type === "range") {
            testRange(t.uid, t.key, t.min, t.mid, t.max, t.tol);
        } else if (t.type === "rangebad") {
            testBadValueRange(t.uid, t.key, t.min, t.max, t.belowMin, t.aboveMax);
        } else if (t.type === "combo") {
            testCombo(t.uid, t.key, t.options);
        } else if (t.type === "combobad") {
            testBadValueCombo(t.uid, t.key, t.invalidOption);
        } else if (t.type === "readonly") {
            testReadOnly(t.uid, t.key);
        }
    }
}

// ---------------------------------------------------------------------------
// UNIT.UTIL — Utility API checks
// ---------------------------------------------------------------------------

TestFramework.runTest("UNIT.UTIL.GET_TOOLS", function() {
    try {
        var tools = ad6676.getTools();
        if (!tools || tools.length === 0) {
            return "getTools() returned empty list";
        }
        printToConsole("  Tools: " + tools);
        return true;
    } catch (e) {
        return "exception: " + e;
    }
});

TestFramework.runTest("UNIT.UTIL.GET_WIDGET_KEYS", function() {
    try {
        var keys = ad6676.getWidgetKeys();
        if (!keys || keys.length === 0) {
            return "getWidgetKeys() returned empty list";
        }
        printToConsole("  Widget keys (" + keys.length + "): " + keys);
        return true;
    } catch (e) {
        return "exception: " + e;
    }
});

// ---------------------------------------------------------------------------
// UNIT.ADC — ADC Settings
// ---------------------------------------------------------------------------

// adc_frequency: widget stores raw Hz; range [2925000000, 3200000000]
testRange(
    "UNIT.ADC.ADC_FREQUENCY",
    PHY + "adc_frequency",
    2925000000, 3062000000, 3200000000,
    1000 // 1 kHz tolerance on raw Hz
);

testBadValueRange(
    "UNIT.ADC.ADC_FREQUENCY_BAD",
    PHY + "adc_frequency",
    2925000000, 3200000000,
    2000000000, // below min
    4000000000  // above max
);

// Conversion: setAdcFrequency("3000") [MHz] → widget raw ≈ 3000000000 Hz
testConversion(
    "UNIT.ADC.ADC_FREQUENCY_CONV",
    function(v) { ad6676.setAdcFrequency(v); },
    PHY + "adc_frequency",
    "3000",
    function(mhzStr) { return parseFloat(mhzStr) * 1e6; },
    1000
);

// ---------------------------------------------------------------------------
// UNIT.BW — Bandwidth Settings
// bw_margin_low/high/if share the same range shape — use runDataDrivenTests
// ---------------------------------------------------------------------------

runDataDrivenTests([
    {
        uid: "UNIT.BW.BW_MARGIN_LOW",
        key: PHY + "bw_margin_low",
        type: "range", min: 0, mid: 15, max: 30, tol: 0.5
    },
    {
        uid: "UNIT.BW.BW_MARGIN_HIGH",
        key: PHY + "bw_margin_high",
        type: "range", min: 0, mid: 15, max: 30, tol: 0.5
    },
    {
        uid: "UNIT.BW.BW_MARGIN_IF",
        key: PHY + "bw_margin_if",
        type: "range", min: -30, mid: 0, max: 30, tol: 0.5
    },
    {
        uid: "UNIT.BW.BW_MARGIN_LOW_BAD",
        key: PHY + "bw_margin_low",
        type: "rangebad", min: 0, max: 30, belowMin: -5, aboveMax: 35
    },
    {
        uid: "UNIT.BW.BW_MARGIN_HIGH_BAD",
        key: PHY + "bw_margin_high",
        type: "rangebad", min: 0, max: 30, belowMin: -5, aboveMax: 35
    },
    {
        uid: "UNIT.BW.BW_MARGIN_IF_BAD",
        key: PHY + "bw_margin_if",
        type: "rangebad", min: -30, max: 30, belowMin: -50, aboveMax: 50
    }
]);

// bandwidth: widget stores raw Hz; initial static range [14625000, 160000000]
testRange(
    "UNIT.BW.BANDWIDTH",
    PHY + "bandwidth",
    14625000, 75000000, 160000000,
    1000
);

testBadValueRange(
    "UNIT.BW.BANDWIDTH_BAD",
    PHY + "bandwidth",
    14625000, 160000000,
    1000000,   // below min
    500000000  // above max
);

// Conversion: setBandwidth("75") [MHz] → widget raw ≈ 75000000 Hz
testConversion(
    "UNIT.BW.BANDWIDTH_CONV",
    function(v) { ad6676.setBandwidth(v); },
    PHY + "bandwidth",
    "75",
    function(mhzStr) { return parseFloat(mhzStr) * 1e6; },
    1000
);

// ---------------------------------------------------------------------------
// UNIT.RX — Receive Settings
// ---------------------------------------------------------------------------

// intermediate_frequency: widget stores raw Hz; range [70000000, 450000000]
testRange(
    "UNIT.RX.INTERMEDIATE_FREQUENCY",
    PHY + "intermediate_frequency",
    70000000, 260000000, 450000000,
    1000
);

testBadValueRange(
    "UNIT.RX.INTERMEDIATE_FREQUENCY_BAD",
    PHY + "intermediate_frequency",
    70000000, 450000000,
    1000000,    // below min
    999000000   // above max
);

// Conversion: setIntermediateFrequency("250") [MHz] → widget raw ≈ 250000000 Hz
testConversion(
    "UNIT.RX.INTERMEDIATE_FREQUENCY_CONV",
    function(v) { ad6676.setIntermediateFrequency(v); },
    PHY + "intermediate_frequency",
    "250",
    function(mhzStr) { return parseFloat(mhzStr) * 1e6; },
    1000
);

// sampling_frequency: read-only
testReadOnly(
    "UNIT.RX.SAMPLING_FREQUENCY",
    PHY + "sampling_frequency"
);

// hardwaregain: widget raw is a number string (e.g. "-12.000000 dB" from IIO, but widget write takes just number)
// Range [-27, 0] dB
testRange(
    "UNIT.RX.HARDWARE_GAIN",
    PHY + "hardwaregain",
    -27, -14, 0,
    0.5
);

testBadValueRange(
    "UNIT.RX.HARDWARE_GAIN_BAD",
    PHY + "hardwaregain",
    -27, 0,
    -50, // below min
    10   // above max
);

// scale: widget stores linear (e.g. "1.000000"); UI shows dBFS [0–12]
// testRange via widget raw (linear values): min=0 dBFS→1.0, mid=6 dBFS→~0.5, max=12 dBFS→~0.25
// Write via API (dBFS), read via API, verify round-trip
TestFramework.runTest("UNIT.RX.SCALE", function() {
    var origRaw = ad6676.readWidget(PHY + "scale");
    try {
        // Test dBFS 0 → linear ≈ 1.0
        ad6676.setScale("0");
        msleep(500);
        var read0 = ad6676.getScale();
        if (read0 !== "0") {
            ad6676.writeWidget(PHY + "scale", origRaw);
            msleep(500);
            return "scale 0 dBFS round-trip failed: got " + read0;
        }

        // Test dBFS 6 → getScale() should return "6"
        ad6676.setScale("6");
        msleep(500);
        var read6 = ad6676.getScale();
        if (read6 !== "6") {
            ad6676.writeWidget(PHY + "scale", origRaw);
            msleep(500);
            return "scale 6 dBFS round-trip failed: got " + read6;
        }

        // Test dBFS 12 → getScale() should return "12"
        ad6676.setScale("12");
        msleep(500);
        var read12 = ad6676.getScale();
        if (read12 !== "12") {
            ad6676.writeWidget(PHY + "scale", origRaw);
            msleep(500);
            return "scale 12 dBFS round-trip failed: got " + read12;
        }

        ad6676.writeWidget(PHY + "scale", origRaw);
        msleep(500);
        return true;
    } catch (e) {
        try { ad6676.writeWidget(PHY + "scale", origRaw); msleep(500); } catch (e2) {}
        return "exception: " + e;
    }
});

// UNIT.RX.SCALE_BAD: out-of-range dBFS values tested via API (readWidget returns linear,
// so testBadValueRange cannot be used directly for scale's mixed domain)
TestFramework.runTest("UNIT.RX.SCALE_BAD", function() {
    var origScale = ad6676.getScale(); // dBFS string
    try {
        // Write below min (< 0 dBFS) — expect clamped to >= 0
        ad6676.setScale("-5");
        msleep(500);
        var readBelow = parseInt(ad6676.getScale());
        if (!isNaN(readBelow) && readBelow < 0) {
            ad6676.setScale(origScale);
            msleep(500);
            return "below-min scale not clamped: got " + readBelow + " dBFS (expected >= 0)";
        }
        printToConsole("  below-min (-5 dBFS) clamped to: " + readBelow + " dBFS");

        // Write above max (> 12 dBFS) — expect clamped to <= 12
        ad6676.setScale("20");
        msleep(500);
        var readAbove = parseInt(ad6676.getScale());
        if (!isNaN(readAbove) && readAbove > 12) {
            ad6676.setScale(origScale);
            msleep(500);
            return "above-max scale not clamped: got " + readAbove + " dBFS (expected <= 12)";
        }
        printToConsole("  above-max (20 dBFS) clamped to: " + readAbove + " dBFS");

        ad6676.setScale(origScale);
        msleep(500);
        return true;
    } catch (e) {
        try { ad6676.setScale(origScale); msleep(500); } catch (e2) {}
        return "exception: " + e;
    }
});

// Conversion: setScale("6") [dBFS] → widget raw linear ≈ pow(10, -6/20) ≈ 0.5012
testConversion(
    "UNIT.RX.SCALE_CONV",
    function(v) { ad6676.setScale(v); },
    PHY + "scale",
    "6",
    function(dbfsStr) { return Math.pow(10, -parseFloat(dbfsStr) / 20.0); },
    0.01 // tolerance on linear value
);

// ---------------------------------------------------------------------------
// UNIT.SHUFFLER — Shuffler Settings
// ---------------------------------------------------------------------------

testCombo(
    "UNIT.SHUFFLER.SHUFFLER_CONTROL",
    PHY + "shuffler_control",
    ["disable", "fadc", "fadc/2", "fadc/3", "fadc/4"]
);

testBadValueCombo(
    "UNIT.SHUFFLER.SHUFFLER_CONTROL_BAD",
    PHY + "shuffler_control",
    "invalid_option"
);

testCombo(
    "UNIT.SHUFFLER.SHUFFLER_THRESH",
    PHY + "shuffler_thresh",
    ["0", "1", "2", "3", "4", "5", "6", "7", "8"]
);

testBadValueCombo(
    "UNIT.SHUFFLER.SHUFFLER_THRESH_BAD",
    PHY + "shuffler_thresh",
    "99"
);

// ---------------------------------------------------------------------------
// UNIT.TEST — Test Settings
// ---------------------------------------------------------------------------

testCombo(
    "UNIT.TEST.TEST_MODE",
    PHY + "test_mode",
    ["off", "checkerboard", "one_zero_toggle", "pn23", "pn9",
     "rep_user_pat", "sing_user_pat", "ramp", "mod_rpat", "jspat", "jtspat"]
);

testBadValueCombo(
    "UNIT.TEST.TEST_MODE_BAD",
    PHY + "test_mode",
    "not_a_mode"
);

// ---------------------------------------------------------------------------
// Cleanup
// ---------------------------------------------------------------------------

TestFramework.disconnectFromDevice();
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
