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

// AD6676 Complex Unit Tests
// Multi-step scenarios testing functionality beyond single-widget read/write.
//   C1: UNIT.UTIL.REFRESH_CYCLE     — refresh() re-reads all widgets
//   C2: UNIT.BW.BANDWIDTH_DYNAMIC_RANGE — bandwidth clamping vs live ADC freq

evaluateFile("../js/testAutomations/common/testFramework.js");

TestFramework.init("AD6676 Complex Unit Tests");

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
// C1: UNIT.UTIL.REFRESH_CYCLE
// Verifies that refresh() completes without error and all getters return
// non-empty values afterwards.
// ---------------------------------------------------------------------------

TestFramework.runTest("UNIT.UTIL.REFRESH_CYCLE", function() {
    try {
        ad6676.refresh();
        msleep(500);

        var checks = [
            { name: "adcFrequency",          val: ad6676.getAdcFrequency() },
            { name: "bandwidth",             val: ad6676.getBandwidth() },
            { name: "bwMarginLow",           val: ad6676.getBwMarginLow() },
            { name: "bwMarginHigh",          val: ad6676.getBwMarginHigh() },
            { name: "bwMarginIf",            val: ad6676.getBwMarginIf() },
            { name: "intermediateFrequency", val: ad6676.getIntermediateFrequency() },
            { name: "samplingFrequency",     val: ad6676.getSamplingFrequency() },
            { name: "hardwareGain",          val: ad6676.getHardwareGain() },
            { name: "scale",                 val: ad6676.getScale() },
            { name: "shufflerControl",       val: ad6676.getShufflerControl() },
            { name: "shufflerThresh",        val: ad6676.getShufflerThresh() },
            { name: "testMode",              val: ad6676.getTestMode() }
        ];

        for (var i = 0; i < checks.length; i++) {
            printToConsole("  " + checks[i].name + ": " + checks[i].val);
            if (!checks[i].val || checks[i].val === "") {
                return "getter returned empty after refresh: " + checks[i].name;
            }
        }
        return true;
    } catch (e) {
        return "exception: " + e;
    }
});

// ---------------------------------------------------------------------------
// C2: UNIT.BW.BANDWIDTH_DYNAMIC_RANGE
// Reads the current ADC frequency and computes the valid bandwidth window
// (0.5%–5% of ADC freq in MHz), then verifies that:
//   (a) a mid-range value round-trips correctly
//   (b) a below-minimum value is clamped up to >= bwMin
//   (c) an above-maximum value is clamped down to <= bwMax
// ---------------------------------------------------------------------------

TestFramework.runTest("UNIT.BW.BANDWIDTH_DYNAMIC_RANGE", function() {
    var origBw = ad6676.getBandwidth(); // MHz string from API
    try {
        // Step 1: Read current ADC frequency (MHz) and derive bandwidth window
        var adcFreqMhz = parseFloat(ad6676.getAdcFrequency());
        if (isNaN(adcFreqMhz) || adcFreqMhz <= 0) {
            return "getAdcFrequency() returned invalid value: " + ad6676.getAdcFrequency();
        }

        var bwMin = 0.005 * adcFreqMhz; // 0.5% of ADC freq
        var bwMax = 0.05  * adcFreqMhz; // 5.0% of ADC freq
        var bwMid = (bwMin + bwMax) / 2.0;

        printToConsole("  ADC freq: " + adcFreqMhz + " MHz");
        printToConsole("  Bandwidth window: [" + bwMin.toFixed(3) + ", " + bwMax.toFixed(3) + "] MHz (mid=" + bwMid.toFixed(3) + ")");

        // Step 2: Write mid-range value — expect readback within [bwMin, bwMax]
        ad6676.setBandwidth(bwMid.toFixed(6));
        msleep(500);
        var readMid = parseFloat(ad6676.getBandwidth());
        printToConsole("  Wrote bwMid=" + bwMid.toFixed(3) + " readBack=" + readMid);
        if (isNaN(readMid) || readMid < bwMin - 0.1 || readMid > bwMax + 0.1) {
            ad6676.setBandwidth(origBw);
            msleep(500);
            return "mid-range value out of expected window: wrote " + bwMid.toFixed(3) + " read " + readMid;
        }

        // Step 3: Write below minimum — expect clamped up to >= bwMin
        var belowMin = (bwMin * 0.5).toFixed(6);
        ad6676.setBandwidth(belowMin);
        msleep(500);
        var readBelow = parseFloat(ad6676.getBandwidth());
        printToConsole("  Wrote belowMin=" + belowMin + " readBack=" + readBelow + " (expect >= " + bwMin.toFixed(3) + ")");
        if (!isNaN(readBelow) && readBelow < bwMin - 0.1) {
            ad6676.setBandwidth(origBw);
            msleep(500);
            return "below-min not clamped: wrote " + belowMin + " read " + readBelow + " bwMin=" + bwMin.toFixed(3);
        }

        // Step 4: Write above maximum — expect clamped down to <= bwMax
        var aboveMax = (bwMax * 2.0).toFixed(6);
        ad6676.setBandwidth(aboveMax);
        msleep(500);
        var readAbove = parseFloat(ad6676.getBandwidth());
        printToConsole("  Wrote aboveMax=" + aboveMax + " readBack=" + readAbove + " (expect <= " + bwMax.toFixed(3) + ")");
        if (!isNaN(readAbove) && readAbove > bwMax + 0.1) {
            ad6676.setBandwidth(origBw);
            msleep(500);
            return "above-max not clamped: wrote " + aboveMax + " read " + readAbove + " bwMax=" + bwMax.toFixed(3);
        }

        // Restore
        ad6676.setBandwidth(origBw);
        msleep(500);
        return true;
    } catch (e) {
        try { ad6676.setBandwidth(origBw); msleep(500); } catch (e2) {}
        return "exception: " + e;
    }
});

// ---------------------------------------------------------------------------
// Cleanup
// ---------------------------------------------------------------------------

TestFramework.disconnectFromDevice();
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
