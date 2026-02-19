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

// AD936x Plugin Automated Tests
// Runs all API tests: Connection, Global Settings, RX Chain, TX Chain, Widget Access, Utility

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: AD936x Plugin Tests
TestFramework.init("AD936x Plugin Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to AD936x tool
if (!switchToTool("AD936X")) {
    printToConsole("ERROR: Cannot switch to AD936X tool");
    exit(1);
}

// ============================================
// Connection & Tool Discovery
// ============================================
printToConsole("\n=== Connection & Tool Discovery ===\n");

TestFramework.runTest("TST.AD936X.TOOLS", function() {
    try {
        var tools = ad936x.getTools();
        printToConsole("  Tools found: " + tools);
        if (!tools || tools.length === 0) {
            printToConsole("  Error: getTools() returned empty list");
            return false;
        }
        var found = false;
        for (var i = 0; i < tools.length; i++) {
            if (tools[i].indexOf("AD936X") !== -1 || tools[i].indexOf("AD936x") !== -1 || tools[i].indexOf("ad936x") !== -1) {
                found = true;
                break;
            }
        }
        if (!found) {
            printToConsole("  Warning: No tool containing 'AD936X' found in list, but tools are present");
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.IS_FMCOMMS5", function() {
    try {
        var result = ad936x.isFmcomms5();
        printToConsole("  isFmcomms5: " + result);
        if (typeof result !== "boolean") {
            printToConsole("  Error: isFmcomms5() did not return a boolean");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Global Device Settings
// ============================================
printToConsole("\n=== Global Device Settings ===\n");

TestFramework.runTest("TST.AD936X.ENSM_MODE", function() {
    try {
        var original = ad936x.getEnsmMode();
        printToConsole("  Original ENSM mode: " + original);
        ad936x.setEnsmMode("fdd");
        msleep(500);
        var readBack = ad936x.getEnsmMode();
        printToConsole("  Read back ENSM mode: " + readBack);
        // Restore original
        if (original && original !== readBack) {
            ad936x.setEnsmMode(original);
            msleep(500);
        }
        return readBack === "fdd";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.CALIB_MODE", function() {
    try {
        var value = ad936x.getCalibMode();
        printToConsole("  Calib mode: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getCalibMode() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.TRX_RATE_GOVERNOR", function() {
    try {
        var original = ad936x.getTrxRateGovernor();
        printToConsole("  Original TRX rate governor: " + original);
        ad936x.setTrxRateGovernor("nominal");
        msleep(500);
        var readBack = ad936x.getTrxRateGovernor();
        printToConsole("  Read back TRX rate governor: " + readBack);
        // Restore original
        if (original && original !== readBack) {
            ad936x.setTrxRateGovernor(original);
            msleep(500);
        }
        return readBack === "nominal";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.RX_PATH_RATES", function() {
    try {
        var value = ad936x.getRxPathRates();
        printToConsole("  RX path rates: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getRxPathRates() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.TX_PATH_RATES", function() {
    try {
        var value = ad936x.getTxPathRates();
        printToConsole("  TX path rates: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxPathRates() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.XO_CORRECTION", function() {
    try {
        var original = ad936x.getXoCorrection();
        printToConsole("  Original XO correction: " + original);
        var testValue = "40000000";
        ad936x.setXoCorrection(testValue);
        msleep(500);
        var readBack = ad936x.getXoCorrection();
        printToConsole("  Read back XO correction: " + readBack);
        // Restore original
        if (original && original !== readBack) {
            ad936x.setXoCorrection(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// RX Chain Tests
// ============================================
printToConsole("\n=== RX Chain Tests ===\n");

TestFramework.runTest("TST.AD936X.RX_RF_BW", function() {
    try {
        var original = ad936x.getRxRfBandwidth();
        printToConsole("  Original RX RF bandwidth: " + original);
        ad936x.setRxRfBandwidth("18000000");
        msleep(500);
        var readBack = ad936x.getRxRfBandwidth();
        printToConsole("  Read back RX RF bandwidth: " + readBack);
        // Restore original
        if (original && original !== readBack) {
            ad936x.setRxRfBandwidth(original);
            msleep(500);
        }
        return readBack === "18000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.RX_SAMPLING_FREQ", function() {
    try {
        var original = ad936x.getRxSamplingFrequency();
        printToConsole("  Original RX sampling frequency: " + original);
        ad936x.setRxSamplingFrequency("25000000");
        msleep(500);
        var readBack = ad936x.getRxSamplingFrequency();
        printToConsole("  Read back RX sampling frequency: " + readBack);
        // Restore original
        if (original && original !== readBack) {
            ad936x.setRxSamplingFrequency(original);
            msleep(500);
        }
        return readBack === "25000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.RX_RF_PORT", function() {
    try {
        var value = ad936x.getRxRfPortSelect();
        printToConsole("  RX RF port: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getRxRfPortSelect() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.RX_LO_FREQ", function() {
    try {
        var original = ad936x.getRxLoFrequency();
        printToConsole("  Original RX LO frequency: " + original);
        ad936x.setRxLoFrequency("2400000000");
        msleep(500);
        var readBack = ad936x.getRxLoFrequency();
        printToConsole("  Read back RX LO frequency: " + readBack);
        // Restore original
        if (original && original !== readBack) {
            ad936x.setRxLoFrequency(original);
            msleep(500);
        }
        return readBack === "2400000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.RX_GAIN_MODE", function() {
    try {
        var original = ad936x.getRxGainControlMode(0);
        printToConsole("  Original RX gain control mode (ch0): " + original);
        ad936x.setRxGainControlMode(0, "manual");
        msleep(500);
        var readBack = ad936x.getRxGainControlMode(0);
        printToConsole("  Read back RX gain control mode (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            ad936x.setRxGainControlMode(0, original);
            msleep(500);
        }
        return readBack === "manual";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.RX_HW_GAIN", function() {
    try {
        // Save original gain mode and gain
        var originalMode = ad936x.getRxGainControlMode(0);
        var originalGain = ad936x.getRxHardwareGain(0);
        printToConsole("  Original gain mode: " + originalMode);
        printToConsole("  Original HW gain: " + originalGain);

        // Set to manual mode first (required for writing gain)
        ad936x.setRxGainControlMode(0, "manual");
        msleep(500);

        ad936x.setRxHardwareGain(0, "30");
        msleep(500);
        var readBack = ad936x.getRxHardwareGain(0);
        printToConsole("  Read back RX HW gain: " + readBack);

        // Restore originals
        if (originalGain) {
            ad936x.setRxHardwareGain(0, originalGain);
            msleep(500);
        }
        if (originalMode && originalMode !== "manual") {
            ad936x.setRxGainControlMode(0, originalMode);
            msleep(500);
        }

        return readBack.indexOf("30") !== -1;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.RX_RSSI", function() {
    try {
        var rssi0 = ad936x.getRxRssi(0);
        var rssi1 = ad936x.getRxRssi(1);
        printToConsole("  RX RSSI ch0: " + rssi0);
        printToConsole("  RX RSSI ch1: " + rssi1);
        if (!rssi0 || rssi0 === "") {
            printToConsole("  Error: getRxRssi(0) returned empty string");
            return false;
        }
        if (!rssi1 || rssi1 === "") {
            printToConsole("  Error: getRxRssi(1) returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.RX_QUAD_TRACK", function() {
    try {
        var original = ad936x.isQuadratureTrackingEnabled();
        printToConsole("  Original quadrature tracking: " + original);
        ad936x.setQuadratureTrackingEnabled("1");
        msleep(500);
        var readBack = ad936x.isQuadratureTrackingEnabled();
        printToConsole("  Read back quadrature tracking: " + readBack);
        // Restore original
        if (original && original !== readBack) {
            ad936x.setQuadratureTrackingEnabled(original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.RX_RF_DC_TRACK", function() {
    try {
        var original = ad936x.isRfDcOffsetTrackingEnabled();
        printToConsole("  Original RF DC offset tracking: " + original);
        ad936x.setRfDcOffsetTrackingEnabled("1");
        msleep(500);
        var readBack = ad936x.isRfDcOffsetTrackingEnabled();
        printToConsole("  Read back RF DC offset tracking: " + readBack);
        // Restore original
        if (original && original !== readBack) {
            ad936x.setRfDcOffsetTrackingEnabled(original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.RX_BB_DC_TRACK", function() {
    try {
        var original = ad936x.isBbDcOffsetTrackingEnabled();
        printToConsole("  Original BB DC offset tracking: " + original);
        ad936x.setBbDcOffsetTrackingEnabled("1");
        msleep(500);
        var readBack = ad936x.isBbDcOffsetTrackingEnabled();
        printToConsole("  Read back BB DC offset tracking: " + readBack);
        // Restore original
        if (original && original !== readBack) {
            ad936x.setBbDcOffsetTrackingEnabled(original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// TX Chain Tests
// ============================================
printToConsole("\n=== TX Chain Tests ===\n");

TestFramework.runTest("TST.AD936X.TX_RF_BW", function() {
    try {
        var original = ad936x.getTxRfBandwidth();
        printToConsole("  Original TX RF bandwidth: " + original);
        ad936x.setTxRfBandwidth("18000000");
        msleep(500);
        var readBack = ad936x.getTxRfBandwidth();
        printToConsole("  Read back TX RF bandwidth: " + readBack);
        // Restore original
        if (original && original !== readBack) {
            ad936x.setTxRfBandwidth(original);
            msleep(500);
        }
        return readBack === "18000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.TX_SAMPLING_FREQ", function() {
    try {
        var original = ad936x.getTxSamplingFrequency();
        printToConsole("  Original TX sampling frequency: " + original);
        ad936x.setTxSamplingFrequency("25000000");
        msleep(500);
        var readBack = ad936x.getTxSamplingFrequency();
        printToConsole("  Read back TX sampling frequency: " + readBack);
        // Restore original
        if (original && original !== readBack) {
            ad936x.setTxSamplingFrequency(original);
            msleep(500);
        }
        return readBack === "25000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.TX_RF_PORT", function() {
    try {
        var value = ad936x.getTxRfPortSelect();
        printToConsole("  TX RF port: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxRfPortSelect() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.TX_LO_FREQ", function() {
    try {
        var original = ad936x.getTxLoFrequency();
        printToConsole("  Original TX LO frequency: " + original);
        ad936x.setTxLoFrequency("2400000000");
        msleep(500);
        var readBack = ad936x.getTxLoFrequency();
        printToConsole("  Read back TX LO frequency: " + readBack);
        // Restore original
        if (original && original !== readBack) {
            ad936x.setTxLoFrequency(original);
            msleep(500);
        }
        return readBack === "2400000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.TX_HW_GAIN", function() {
    try {
        var originalGain = ad936x.getTxHardwareGain(0);
        printToConsole("  Original TX HW gain: " + originalGain);
        ad936x.setTxHardwareGain(0, "-10");
        msleep(500);
        var readBack = ad936x.getTxHardwareGain(0);
        printToConsole("  Read back TX HW gain: " + readBack);
        // Restore original
        if (originalGain) {
            ad936x.setTxHardwareGain(0, originalGain);
            msleep(500);
        }
        return readBack.indexOf("-10") !== -1;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.TX_RSSI", function() {
    try {
        var rssi0 = ad936x.getTxRssi(0);
        var rssi1 = ad936x.getTxRssi(1);
        printToConsole("  TX RSSI ch0: " + rssi0);
        printToConsole("  TX RSSI ch1: " + rssi1);
        if (!rssi0 || rssi0 === "") {
            printToConsole("  Error: getTxRssi(0) returned empty string");
            return false;
        }
        if (!rssi1 || rssi1 === "") {
            printToConsole("  Error: getTxRssi(1) returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Generic Widget Access Tests
// ============================================
printToConsole("\n=== Generic Widget Access Tests ===\n");

TestFramework.runTest("TST.AD936X.WIDGET_KEYS", function() {
    try {
        var keys = ad936x.getWidgetKeys();
        printToConsole("  Widget keys count: " + keys.length);
        if (!keys || keys.length === 0) {
            printToConsole("  Error: getWidgetKeys() returned empty list");
            return false;
        }
        for (var i = 0; i < Math.min(keys.length, 5); i++) {
            printToConsole("  Key[" + i + "]: " + keys[i]);
        }
        if (keys.length > 5) {
            printToConsole("  ... and " + (keys.length - 5) + " more");
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.WIDGET_RW", function() {
    try {
        var key = "ad9361-phy//ensm_mode";
        var original = ad936x.readWidget(key);
        printToConsole("  Original value for '" + key + "': " + original);
        ad936x.writeWidget(key, "fdd");
        msleep(500);
        var readBack = ad936x.readWidget(key);
        printToConsole("  Read back value: " + readBack);
        // Restore original
        if (original && original !== readBack) {
            ad936x.writeWidget(key, original);
            msleep(500);
        }
        return readBack === "fdd";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Utility Tests
// ============================================
printToConsole("\n=== Utility Tests ===\n");

TestFramework.runTest("TST.AD936X.REFRESH", function() {
    try {
        ad936x.refresh();
        msleep(500);
        printToConsole("  refresh() completed without exception");
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

