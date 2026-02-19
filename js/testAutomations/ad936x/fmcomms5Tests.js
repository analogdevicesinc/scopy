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

// FMCOMMS5 Plugin Automated Tests
// Runs all API tests: Global Settings, RX Chain, TX Chain, Per-Channel, LO, Widget Access, Utility

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: FMCOMMS5 Plugin Tests
TestFramework.init("FMCOMMS5 Plugin Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to FMCOMMS5 tool
if (!switchToTool("FMCOMMS5")) {
    printToConsole("ERROR: Cannot switch to FMCOMMS5 tool");
    scopy.exit();
}

// ============================================
// Global Device Settings
// ============================================
printToConsole("\n=== Global Device Settings ===\n");

TestFramework.runTest("TST.FMCOMMS5.ENSM_MODE", function() {
    try {
        var original = fmcomms5.getEnsmMode();
        printToConsole("  Original ENSM mode: " + original);
        fmcomms5.setEnsmMode("fdd");
        msleep(500);
        var readBack = fmcomms5.getEnsmMode();
        printToConsole("  Read back ENSM mode: " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setEnsmMode(original);
            msleep(500);
        }
        return readBack === "fdd";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.CALIB_MODE", function() {
    try {
        var value = fmcomms5.getCalibMode();
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

TestFramework.runTest("TST.FMCOMMS5.TRX_RATE_GOVERNOR", function() {
    try {
        var original = fmcomms5.getTrxRateGovernor();
        printToConsole("  Original TRX rate governor: " + original);
        fmcomms5.setTrxRateGovernor("nominal");
        msleep(500);
        var readBack = fmcomms5.getTrxRateGovernor();
        printToConsole("  Read back TRX rate governor: " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setTrxRateGovernor(original);
            msleep(500);
        }
        return readBack === "nominal";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.RX_PATH_RATES", function() {
    try {
        var value = fmcomms5.getRxPathRates();
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

TestFramework.runTest("TST.FMCOMMS5.TX_PATH_RATES", function() {
    try {
        var value = fmcomms5.getTxPathRates();
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

TestFramework.runTest("TST.FMCOMMS5.XO_CORRECTION", function() {
    try {
        var original = fmcomms5.getXoCorrection();
        printToConsole("  Original XO correction: " + original);
        var testValue = "40000000";
        fmcomms5.setXoCorrection(testValue);
        msleep(500);
        var readBack = fmcomms5.getXoCorrection();
        printToConsole("  Read back XO correction: " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setXoCorrection(original);
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

TestFramework.runTest("TST.FMCOMMS5.RX_RF_BW", function() {
    try {
        var original = fmcomms5.getRxRfBandwidth();
        printToConsole("  Original RX RF bandwidth: " + original);
        fmcomms5.setRxRfBandwidth("18000000");
        msleep(500);
        var readBack = fmcomms5.getRxRfBandwidth();
        printToConsole("  Read back RX RF bandwidth: " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setRxRfBandwidth(original);
            msleep(500);
        }
        return readBack === "18000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.RX_SAMPLING_FREQ", function() {
    try {
        var original = fmcomms5.getRxSamplingFrequency();
        printToConsole("  Original RX sampling frequency: " + original);
        fmcomms5.setRxSamplingFrequency("25000000");
        msleep(500);
        var readBack = fmcomms5.getRxSamplingFrequency();
        printToConsole("  Read back RX sampling frequency: " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setRxSamplingFrequency(original);
            msleep(500);
        }
        return readBack === "25000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.RX_RF_PORT", function() {
    try {
        var value = fmcomms5.getRxRfPortSelect();
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

TestFramework.runTest("TST.FMCOMMS5.RX_QUAD_TRACK", function() {
    try {
        var original = fmcomms5.isQuadratureTrackingEnabled();
        printToConsole("  Original quadrature tracking: " + original);
        fmcomms5.setQuadratureTrackingEnabled("1");
        msleep(500);
        var readBack = fmcomms5.isQuadratureTrackingEnabled();
        printToConsole("  Read back quadrature tracking: " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setQuadratureTrackingEnabled(original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.RX_RF_DC_TRACK", function() {
    try {
        var original = fmcomms5.isRfDcOffsetTrackingEnabled();
        printToConsole("  Original RF DC offset tracking: " + original);
        fmcomms5.setRfDcOffsetTrackingEnabled("1");
        msleep(500);
        var readBack = fmcomms5.isRfDcOffsetTrackingEnabled();
        printToConsole("  Read back RF DC offset tracking: " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setRfDcOffsetTrackingEnabled(original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.RX_BB_DC_TRACK", function() {
    try {
        var original = fmcomms5.isBbDcOffsetTrackingEnabled();
        printToConsole("  Original BB DC offset tracking: " + original);
        fmcomms5.setBbDcOffsetTrackingEnabled("1");
        msleep(500);
        var readBack = fmcomms5.isBbDcOffsetTrackingEnabled();
        printToConsole("  Read back BB DC offset tracking: " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setBbDcOffsetTrackingEnabled(original);
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

TestFramework.runTest("TST.FMCOMMS5.TX_RF_BW", function() {
    try {
        var original = fmcomms5.getTxRfBandwidth();
        printToConsole("  Original TX RF bandwidth: " + original);
        fmcomms5.setTxRfBandwidth("18000000");
        msleep(500);
        var readBack = fmcomms5.getTxRfBandwidth();
        printToConsole("  Read back TX RF bandwidth: " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setTxRfBandwidth(original);
            msleep(500);
        }
        return readBack === "18000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.TX_SAMPLING_FREQ", function() {
    try {
        var original = fmcomms5.getTxSamplingFrequency();
        printToConsole("  Original TX sampling frequency: " + original);
        fmcomms5.setTxSamplingFrequency("25000000");
        msleep(500);
        var readBack = fmcomms5.getTxSamplingFrequency();
        printToConsole("  Read back TX sampling frequency: " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setTxSamplingFrequency(original);
            msleep(500);
        }
        return readBack === "25000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.TX_RF_PORT", function() {
    try {
        var value = fmcomms5.getTxRfPortSelect();
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

// ============================================
// Per-Channel RX Tests (4 channels: 0-3)
// ============================================
printToConsole("\n=== Per-Channel RX Tests ===\n");

TestFramework.runTest("TST.FMCOMMS5.RX_HW_GAIN_CH0", function() {
    try {
        var originalMode = fmcomms5.getRxGainControlMode(0);
        var originalGain = fmcomms5.getRxHardwareGain(0);
        printToConsole("  Original gain mode (ch0): " + originalMode);
        printToConsole("  Original HW gain (ch0): " + originalGain);
        fmcomms5.setRxGainControlMode(0, "manual");
        msleep(500);
        fmcomms5.setRxHardwareGain(0, "30");
        msleep(500);
        var readBack = fmcomms5.getRxHardwareGain(0);
        printToConsole("  Read back RX HW gain (ch0): " + readBack);
        if (originalGain) {
            fmcomms5.setRxHardwareGain(0, originalGain);
            msleep(500);
        }
        if (originalMode && originalMode !== "manual") {
            fmcomms5.setRxGainControlMode(0, originalMode);
            msleep(500);
        }
        return readBack.indexOf("30") !== -1;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.RX_HW_GAIN_CH2", function() {
    try {
        var originalMode = fmcomms5.getRxGainControlMode(2);
        var originalGain = fmcomms5.getRxHardwareGain(2);
        printToConsole("  Original gain mode (ch2): " + originalMode);
        printToConsole("  Original HW gain (ch2): " + originalGain);
        fmcomms5.setRxGainControlMode(2, "manual");
        msleep(500);
        fmcomms5.setRxHardwareGain(2, "30");
        msleep(500);
        var readBack = fmcomms5.getRxHardwareGain(2);
        printToConsole("  Read back RX HW gain (ch2): " + readBack);
        if (originalGain) {
            fmcomms5.setRxHardwareGain(2, originalGain);
            msleep(500);
        }
        if (originalMode && originalMode !== "manual") {
            fmcomms5.setRxGainControlMode(2, originalMode);
            msleep(500);
        }
        return readBack.indexOf("30") !== -1;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.RX_GAIN_MODE_CH0", function() {
    try {
        var original = fmcomms5.getRxGainControlMode(0);
        printToConsole("  Original RX gain control mode (ch0): " + original);
        fmcomms5.setRxGainControlMode(0, "manual");
        msleep(500);
        var readBack = fmcomms5.getRxGainControlMode(0);
        printToConsole("  Read back RX gain control mode (ch0): " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setRxGainControlMode(0, original);
            msleep(500);
        }
        return readBack === "manual";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.RX_GAIN_MODE_CH2", function() {
    try {
        var original = fmcomms5.getRxGainControlMode(2);
        printToConsole("  Original RX gain control mode (ch2): " + original);
        fmcomms5.setRxGainControlMode(2, "manual");
        msleep(500);
        var readBack = fmcomms5.getRxGainControlMode(2);
        printToConsole("  Read back RX gain control mode (ch2): " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setRxGainControlMode(2, original);
            msleep(500);
        }
        return readBack === "manual";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.RX_RSSI", function() {
    try {
        var rssi0 = fmcomms5.getRxRssi(0);
        var rssi1 = fmcomms5.getRxRssi(1);
        var rssi2 = fmcomms5.getRxRssi(2);
        var rssi3 = fmcomms5.getRxRssi(3);
        printToConsole("  RX RSSI ch0: " + rssi0);
        printToConsole("  RX RSSI ch1: " + rssi1);
        printToConsole("  RX RSSI ch2: " + rssi2);
        printToConsole("  RX RSSI ch3: " + rssi3);
        if (!rssi0 || rssi0 === "") {
            printToConsole("  Error: getRxRssi(0) returned empty string");
            return false;
        }
        if (!rssi1 || rssi1 === "") {
            printToConsole("  Error: getRxRssi(1) returned empty string");
            return false;
        }
        if (!rssi2 || rssi2 === "") {
            printToConsole("  Error: getRxRssi(2) returned empty string");
            return false;
        }
        if (!rssi3 || rssi3 === "") {
            printToConsole("  Error: getRxRssi(3) returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Per-Channel TX Tests (4 channels: 0-3)
// ============================================
printToConsole("\n=== Per-Channel TX Tests ===\n");

TestFramework.runTest("TST.FMCOMMS5.TX_HW_GAIN_CH0", function() {
    try {
        var originalGain = fmcomms5.getTxHardwareGain(0);
        printToConsole("  Original TX HW gain (ch0): " + originalGain);
        fmcomms5.setTxHardwareGain(0, "-10");
        msleep(500);
        var readBack = fmcomms5.getTxHardwareGain(0);
        printToConsole("  Read back TX HW gain (ch0): " + readBack);
        if (originalGain) {
            fmcomms5.setTxHardwareGain(0, originalGain);
            msleep(500);
        }
        return readBack.indexOf("-10") !== -1;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.TX_HW_GAIN_CH2", function() {
    try {
        var originalGain = fmcomms5.getTxHardwareGain(2);
        printToConsole("  Original TX HW gain (ch2): " + originalGain);
        fmcomms5.setTxHardwareGain(2, "-10");
        msleep(500);
        var readBack = fmcomms5.getTxHardwareGain(2);
        printToConsole("  Read back TX HW gain (ch2): " + readBack);
        if (originalGain) {
            fmcomms5.setTxHardwareGain(2, originalGain);
            msleep(500);
        }
        return readBack.indexOf("-10") !== -1;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.TX_RSSI", function() {
    try {
        var rssi0 = fmcomms5.getTxRssi(0);
        var rssi1 = fmcomms5.getTxRssi(1);
        var rssi2 = fmcomms5.getTxRssi(2);
        var rssi3 = fmcomms5.getTxRssi(3);
        printToConsole("  TX RSSI ch0: " + rssi0);
        printToConsole("  TX RSSI ch1: " + rssi1);
        printToConsole("  TX RSSI ch2: " + rssi2);
        printToConsole("  TX RSSI ch3: " + rssi3);
        if (!rssi0 || rssi0 === "") {
            printToConsole("  Error: getTxRssi(0) returned empty string");
            return false;
        }
        if (!rssi1 || rssi1 === "") {
            printToConsole("  Error: getTxRssi(1) returned empty string");
            return false;
        }
        if (!rssi2 || rssi2 === "") {
            printToConsole("  Error: getTxRssi(2) returned empty string");
            return false;
        }
        if (!rssi3 || rssi3 === "") {
            printToConsole("  Error: getTxRssi(3) returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// LO Frequencies (2 devices: 0, 1)
// ============================================
printToConsole("\n=== LO Frequencies ===\n");

TestFramework.runTest("TST.FMCOMMS5.RX_LO_DEV0", function() {
    try {
        var original = fmcomms5.getRxLoFrequency(0);
        printToConsole("  Original RX LO frequency (dev0): " + original);
        fmcomms5.setRxLoFrequency(0, "2400000000");
        msleep(500);
        var readBack = fmcomms5.getRxLoFrequency(0);
        printToConsole("  Read back RX LO frequency (dev0): " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setRxLoFrequency(0, original);
            msleep(500);
        }
        return readBack === "2400000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.RX_LO_DEV1", function() {
    try {
        var original = fmcomms5.getRxLoFrequency(1);
        printToConsole("  Original RX LO frequency (dev1): " + original);
        fmcomms5.setRxLoFrequency(1, "2400000000");
        msleep(500);
        var readBack = fmcomms5.getRxLoFrequency(1);
        printToConsole("  Read back RX LO frequency (dev1): " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setRxLoFrequency(1, original);
            msleep(500);
        }
        return readBack === "2400000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.TX_LO_DEV0", function() {
    try {
        var original = fmcomms5.getTxLoFrequency(0);
        printToConsole("  Original TX LO frequency (dev0): " + original);
        fmcomms5.setTxLoFrequency(0, "2400000000");
        msleep(500);
        var readBack = fmcomms5.getTxLoFrequency(0);
        printToConsole("  Read back TX LO frequency (dev0): " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setTxLoFrequency(0, original);
            msleep(500);
        }
        return readBack === "2400000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.FMCOMMS5.TX_LO_DEV1", function() {
    try {
        var original = fmcomms5.getTxLoFrequency(1);
        printToConsole("  Original TX LO frequency (dev1): " + original);
        fmcomms5.setTxLoFrequency(1, "2400000000");
        msleep(500);
        var readBack = fmcomms5.getTxLoFrequency(1);
        printToConsole("  Read back TX LO frequency (dev1): " + readBack);
        if (original && original !== readBack) {
            fmcomms5.setTxLoFrequency(1, original);
            msleep(500);
        }
        return readBack === "2400000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Generic Widget Access Tests
// ============================================
printToConsole("\n=== Generic Widget Access Tests ===\n");

TestFramework.runTest("TST.FMCOMMS5.WIDGET_KEYS", function() {
    try {
        var keys = fmcomms5.getWidgetKeys();
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

TestFramework.runTest("TST.FMCOMMS5.WIDGET_RW", function() {
    try {
        var key = "ad9361-phy//ensm_mode";
        var original = fmcomms5.readWidget(key);
        printToConsole("  Original value for '" + key + "': " + original);
        fmcomms5.writeWidget(key, "fdd");
        msleep(500);
        var readBack = fmcomms5.readWidget(key);
        printToConsole("  Read back value: " + readBack);
        if (original && original !== readBack) {
            fmcomms5.writeWidget(key, original);
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

TestFramework.runTest("TST.FMCOMMS5.REFRESH", function() {
    try {
        fmcomms5.refresh();
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
