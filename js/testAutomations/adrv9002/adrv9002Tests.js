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

// ADRV9002 Plugin Automated Tests
// Runs all API tests: Connection, Global, RX Chain, RX Tracking, TX Chain, TX Tracking, ORX, Widget Access, Utility

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: ADRV9002 Plugin Tests
TestFramework.init("ADRV9002 Plugin Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to ADRV9002 tool
if (!switchToTool("ADRV9002")) {
    printToConsole("ERROR: Cannot switch to ADRV9002 tool");
    exit(1);
}

// ============================================
// Connection & Tool Discovery
// ============================================
printToConsole("\n=== Connection & Tool Discovery ===\n");

TestFramework.runTest("TST.ADRV9002.TOOLS", function() {
    try {
        var tools = adrv9002.getTools();
        printToConsole("  Tools found: " + tools);
        if (!tools || tools.length === 0) {
            printToConsole("  Error: getTools() returned empty list");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Global
// ============================================
printToConsole("\n=== Global ===\n");

TestFramework.runTest("TST.ADRV9002.TEMPERATURE", function() {
    try {
        var value = adrv9002.getTemperature();
        printToConsole("  Temperature: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTemperature() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// RX Chain Tests
// ============================================
printToConsole("\n=== RX Chain Tests ===\n");

TestFramework.runTest("TST.ADRV9002.RX_GAIN_MODE", function() {
    try {
        var original = adrv9002.getRxGainControlMode(0);
        printToConsole("  Original RX gain control mode (ch0): " + original);
        adrv9002.setRxGainControlMode(0, "manual");
        msleep(500);
        var readBack = adrv9002.getRxGainControlMode(0);
        printToConsole("  Read back RX gain control mode (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setRxGainControlMode(0, original);
            msleep(500);
        }
        return readBack === "manual";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_HW_GAIN", function() {
    try {
        // Save original gain mode and gain
        var originalMode = adrv9002.getRxGainControlMode(0);
        var originalGain = adrv9002.getRxHardwareGain(0);
        printToConsole("  Original gain mode: " + originalMode);
        printToConsole("  Original HW gain: " + originalGain);

        // Set to manual mode first (required for writing gain)
        adrv9002.setRxGainControlMode(0, "manual");
        msleep(500);

        adrv9002.setRxHardwareGain(0, "10");
        msleep(500);
        var readBack = adrv9002.getRxHardwareGain(0);
        printToConsole("  Read back RX HW gain: " + readBack);

        // Restore originals
        if (originalGain) {
            adrv9002.setRxHardwareGain(0, originalGain);
            msleep(500);
        }
        if (originalMode && originalMode !== "manual") {
            adrv9002.setRxGainControlMode(0, originalMode);
            msleep(500);
        }

        return readBack.indexOf("10") !== -1;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_ENSM", function() {
    try {
        var value = adrv9002.getRxEnsmMode(0);
        printToConsole("  RX ENSM mode (ch0): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getRxEnsmMode(0) returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_ENABLED", function() {
    try {
        var original = adrv9002.isRxEnabled(0);
        printToConsole("  Original RX enabled (ch0): " + original);
        adrv9002.setRxEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isRxEnabled(0);
        printToConsole("  Read back RX enabled (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setRxEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_BBDC_REJECTION", function() {
    try {
        var original = adrv9002.isRxBbdcRejectionEnabled(0);
        printToConsole("  Original RX BBDC rejection (ch0): " + original);
        adrv9002.setRxBbdcRejectionEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isRxBbdcRejectionEnabled(0);
        printToConsole("  Read back RX BBDC rejection (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setRxBbdcRejectionEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_PORT_EN", function() {
    try {
        var value = adrv9002.getRxPortEnMode(0);
        printToConsole("  RX port en mode (ch0): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getRxPortEnMode(0) returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_DYN_ADC_SWITCH", function() {
    try {
        var original = adrv9002.isRxDynamicAdcSwitchEnabled(0);
        printToConsole("  Original RX dynamic ADC switch (ch0): " + original);
        adrv9002.setRxDynamicAdcSwitchEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isRxDynamicAdcSwitchEnabled(0);
        printToConsole("  Read back RX dynamic ADC switch (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setRxDynamicAdcSwitchEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_LO_FREQ", function() {
    try {
        var original = adrv9002.getRxLoFrequency(0);
        printToConsole("  Original RX LO frequency (ch0): " + original);
        adrv9002.setRxLoFrequency(0, "2400000000");
        msleep(500);
        var readBack = adrv9002.getRxLoFrequency(0);
        printToConsole("  Read back RX LO frequency (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setRxLoFrequency(0, original);
            msleep(500);
        }
        return readBack === "2400000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_NCO_FREQ", function() {
    try {
        var original = adrv9002.getRxNcoFrequency(0);
        printToConsole("  Original RX NCO frequency (ch0): " + original);
        adrv9002.setRxNcoFrequency(0, "1000");
        msleep(500);
        var readBack = adrv9002.getRxNcoFrequency(0);
        printToConsole("  Read back RX NCO frequency (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setRxNcoFrequency(0, original);
            msleep(500);
        }
        return readBack === "1000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_DIG_GAIN_MODE", function() {
    try {
        var original = adrv9002.getRxDigitalGainControlMode(0);
        printToConsole("  Original RX digital gain control mode (ch0): " + original);
        adrv9002.setRxDigitalGainControlMode(0, "automatic");
        msleep(500);
        var readBack = adrv9002.getRxDigitalGainControlMode(0);
        printToConsole("  Read back RX digital gain control mode (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setRxDigitalGainControlMode(0, original);
            msleep(500);
        }
        return readBack === "automatic";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_INTF_GAIN", function() {
    try {
        var original = adrv9002.getRxInterfaceGain(0);
        printToConsole("  Original RX interface gain (ch0): " + original);
        adrv9002.setRxInterfaceGain(0, "0dB");
        msleep(500);
        var readBack = adrv9002.getRxInterfaceGain(0);
        printToConsole("  Read back RX interface gain (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setRxInterfaceGain(0, original);
            msleep(500);
        }
        return readBack === "0dB";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_BBDC_LOOP_GAIN", function() {
    try {
        var original = adrv9002.getRxBbdcLoopGain(0);
        printToConsole("  Original RX BBDC loop gain (ch0): " + original);
        adrv9002.setRxBbdcLoopGain(0, "100");
        msleep(500);
        var readBack = adrv9002.getRxBbdcLoopGain(0);
        printToConsole("  Read back RX BBDC loop gain (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setRxBbdcLoopGain(0, original);
            msleep(500);
        }
        return readBack === "100";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_RF_BW", function() {
    try {
        var bw0 = adrv9002.getRxRfBandwidth(0);
        var bw1 = adrv9002.getRxRfBandwidth(1);
        printToConsole("  RX RF bandwidth ch0: " + bw0);
        printToConsole("  RX RF bandwidth ch1: " + bw1);
        if (!bw0 || bw0 === "") {
            printToConsole("  Error: getRxRfBandwidth(0) returned empty string");
            return false;
        }
        if (!bw1 || bw1 === "") {
            printToConsole("  Error: getRxRfBandwidth(1) returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_RSSI", function() {
    try {
        var rssi0 = adrv9002.getRxRssi(0);
        var rssi1 = adrv9002.getRxRssi(1);
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

// ============================================
// RX Read-Only Tests
// ============================================
printToConsole("\n=== RX Read-Only Tests ===\n");

TestFramework.runTest("TST.ADRV9002.RX_DEC_POWER", function() {
    try {
        var val0 = adrv9002.getRxDecimatedPower(0);
        var val1 = adrv9002.getRxDecimatedPower(1);
        printToConsole("  RX decimated power ch0: " + val0);
        printToConsole("  RX decimated power ch1: " + val1);
        if (!val0 || val0 === "") {
            printToConsole("  Error: getRxDecimatedPower(0) returned empty string");
            return false;
        }
        if (!val1 || val1 === "") {
            printToConsole("  Error: getRxDecimatedPower(1) returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_SAMPLING_FREQ", function() {
    try {
        var val0 = adrv9002.getRxSamplingFrequency(0);
        var val1 = adrv9002.getRxSamplingFrequency(1);
        printToConsole("  RX sampling frequency ch0: " + val0);
        printToConsole("  RX sampling frequency ch1: " + val1);
        if (!val0 || val0 === "") {
            printToConsole("  Error: getRxSamplingFrequency(0) returned empty string");
            return false;
        }
        if (!val1 || val1 === "") {
            printToConsole("  Error: getRxSamplingFrequency(1) returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// RX Tracking Tests
// ============================================
printToConsole("\n=== RX Tracking Tests ===\n");

TestFramework.runTest("TST.ADRV9002.RX_QUAD_FIC_TRACK", function() {
    try {
        var original = adrv9002.isRxQuadratureFicTrackingEnabled(0);
        printToConsole("  Original RX quadrature FIC tracking (ch0): " + original);
        adrv9002.setRxQuadratureFicTrackingEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isRxQuadratureFicTrackingEnabled(0);
        printToConsole("  Read back RX quadrature FIC tracking (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setRxQuadratureFicTrackingEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_AGC_TRACK", function() {
    try {
        var original = adrv9002.isRxAgcTrackingEnabled(0);
        printToConsole("  Original RX AGC tracking (ch0): " + original);
        adrv9002.setRxAgcTrackingEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isRxAgcTrackingEnabled(0);
        printToConsole("  Read back RX AGC tracking (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setRxAgcTrackingEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_BBDC_REJ_TRACK", function() {
    try {
        var original = adrv9002.isRxBbdcRejectionTrackingEnabled(0);
        printToConsole("  Original RX BBDC rejection tracking (ch0): " + original);
        adrv9002.setRxBbdcRejectionTrackingEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isRxBbdcRejectionTrackingEnabled(0);
        printToConsole("  Read back RX BBDC rejection tracking (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setRxBbdcRejectionTrackingEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_QUAD_POLY_TRACK", function() {
    try {
        var original = adrv9002.isRxQuadraturePolyTrackingEnabled(0);
        printToConsole("  Original RX quadrature poly tracking (ch0): " + original);
        adrv9002.setRxQuadraturePolyTrackingEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isRxQuadraturePolyTrackingEnabled(0);
        printToConsole("  Read back RX quadrature poly tracking (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setRxQuadraturePolyTrackingEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_HD_TRACK", function() {
    try {
        var original = adrv9002.isRxHdTrackingEnabled(0);
        printToConsole("  Original RX HD tracking (ch0): " + original);
        adrv9002.setRxHdTrackingEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isRxHdTrackingEnabled(0);
        printToConsole("  Read back RX HD tracking (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setRxHdTrackingEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.RX_RSSI_TRACK", function() {
    try {
        var original = adrv9002.isRxRssiTrackingEnabled(0);
        printToConsole("  Original RX RSSI tracking (ch0): " + original);
        adrv9002.setRxRssiTrackingEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isRxRssiTrackingEnabled(0);
        printToConsole("  Read back RX RSSI tracking (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setRxRssiTrackingEnabled(0, original);
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

TestFramework.runTest("TST.ADRV9002.TX_ATTEN", function() {
    try {
        var original = adrv9002.getTxAttenuation(0);
        printToConsole("  Original TX attenuation (ch0): " + original);
        adrv9002.setTxAttenuation(0, "-10");
        msleep(500);
        var readBack = adrv9002.getTxAttenuation(0);
        printToConsole("  Read back TX attenuation (ch0): " + readBack);
        // Restore original
        if (original) {
            adrv9002.setTxAttenuation(0, original);
            msleep(500);
        }
        return readBack.indexOf("-10") !== -1;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.TX_ATTEN_CTRL", function() {
    try {
        var value = adrv9002.getTxAttenControlMode(0);
        printToConsole("  TX atten control mode (ch0): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxAttenControlMode(0) returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.TX_NCO_FREQ", function() {
    try {
        var original = adrv9002.getTxNcoFrequency(0);
        printToConsole("  Original TX NCO frequency (ch0): " + original);
        adrv9002.setTxNcoFrequency(0, "1000");
        msleep(500);
        var readBack = adrv9002.getTxNcoFrequency(0);
        printToConsole("  Read back TX NCO frequency (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setTxNcoFrequency(0, original);
            msleep(500);
        }
        return readBack === "1000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.TX_LO_FREQ", function() {
    try {
        var original = adrv9002.getTxLoFrequency(0);
        printToConsole("  Original TX LO frequency (ch0): " + original);
        adrv9002.setTxLoFrequency(0, "2400000000");
        msleep(500);
        var readBack = adrv9002.getTxLoFrequency(0);
        printToConsole("  Read back TX LO frequency (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setTxLoFrequency(0, original);
            msleep(500);
        }
        return readBack === "2400000000";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.TX_ENSM", function() {
    try {
        var value = adrv9002.getTxEnsmMode(0);
        printToConsole("  TX ENSM mode (ch0): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxEnsmMode(0) returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.TX_ENABLED", function() {
    try {
        var original = adrv9002.isTxEnabled(0);
        printToConsole("  Original TX enabled (ch0): " + original);
        adrv9002.setTxEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isTxEnabled(0);
        printToConsole("  Read back TX enabled (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setTxEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.TX_PORT_EN", function() {
    try {
        var value = adrv9002.getTxPortEnMode(0);
        printToConsole("  TX port en mode (ch0): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxPortEnMode(0) returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.TX_RF_BW", function() {
    try {
        var bw0 = adrv9002.getTxRfBandwidth(0);
        var bw1 = adrv9002.getTxRfBandwidth(1);
        printToConsole("  TX RF bandwidth ch0: " + bw0);
        printToConsole("  TX RF bandwidth ch1: " + bw1);
        if (!bw0 || bw0 === "") {
            printToConsole("  Error: getTxRfBandwidth(0) returned empty string");
            return false;
        }
        if (!bw1 || bw1 === "") {
            printToConsole("  Error: getTxRfBandwidth(1) returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.TX_SAMPLING_FREQ", function() {
    try {
        var val0 = adrv9002.getTxSamplingFrequency(0);
        var val1 = adrv9002.getTxSamplingFrequency(1);
        printToConsole("  TX sampling frequency ch0: " + val0);
        printToConsole("  TX sampling frequency ch1: " + val1);
        if (!val0 || val0 === "") {
            printToConsole("  Error: getTxSamplingFrequency(0) returned empty string");
            return false;
        }
        if (!val1 || val1 === "") {
            printToConsole("  Error: getTxSamplingFrequency(1) returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// TX Tracking Tests
// ============================================
printToConsole("\n=== TX Tracking Tests ===\n");

TestFramework.runTest("TST.ADRV9002.TX_QUAD_TRACK", function() {
    try {
        var original = adrv9002.isTxQuadratureTrackingEnabled(0);
        printToConsole("  Original TX quadrature tracking (ch0): " + original);
        adrv9002.setTxQuadratureTrackingEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isTxQuadratureTrackingEnabled(0);
        printToConsole("  Read back TX quadrature tracking (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setTxQuadratureTrackingEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.TX_CL_GAIN_TRACK", function() {
    try {
        var original = adrv9002.isTxCloseLoopGainTrackingEnabled(0);
        printToConsole("  Original TX close loop gain tracking (ch0): " + original);
        adrv9002.setTxCloseLoopGainTrackingEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isTxCloseLoopGainTrackingEnabled(0);
        printToConsole("  Read back TX close loop gain tracking (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setTxCloseLoopGainTrackingEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.TX_PA_CORR_TRACK", function() {
    try {
        var original = adrv9002.isTxPaCorrectionTrackingEnabled(0);
        printToConsole("  Original TX PA correction tracking (ch0): " + original);
        adrv9002.setTxPaCorrectionTrackingEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isTxPaCorrectionTrackingEnabled(0);
        printToConsole("  Read back TX PA correction tracking (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setTxPaCorrectionTrackingEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.TX_LB_DELAY_TRACK", function() {
    try {
        var original = adrv9002.isTxLoopbackDelayTrackingEnabled(0);
        printToConsole("  Original TX loopback delay tracking (ch0): " + original);
        adrv9002.setTxLoopbackDelayTrackingEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isTxLoopbackDelayTrackingEnabled(0);
        printToConsole("  Read back TX loopback delay tracking (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setTxLoopbackDelayTrackingEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.TX_LO_LEAK_TRACK", function() {
    try {
        var original = adrv9002.isTxLoLeakageTrackingEnabled(0);
        printToConsole("  Original TX LO leakage tracking (ch0): " + original);
        adrv9002.setTxLoLeakageTrackingEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isTxLoLeakageTrackingEnabled(0);
        printToConsole("  Read back TX LO leakage tracking (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setTxLoLeakageTrackingEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// ORX Tests
// ============================================
printToConsole("\n=== ORX Tests ===\n");

TestFramework.runTest("TST.ADRV9002.ORX_HW_GAIN", function() {
    try {
        var original = adrv9002.getOrxHardwareGain(0);
        printToConsole("  Original ORX HW gain (ch0): " + original);
        adrv9002.setOrxHardwareGain(0, "10");
        msleep(500);
        var readBack = adrv9002.getOrxHardwareGain(0);
        printToConsole("  Read back ORX HW gain (ch0): " + readBack);
        // Restore original
        if (original) {
            adrv9002.setOrxHardwareGain(0, original);
            msleep(500);
        }
        return readBack.indexOf("10") !== -1;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.ORX_BBDC", function() {
    try {
        var original = adrv9002.isOrxBbdcRejectionEnabled(0);
        printToConsole("  Original ORX BBDC rejection (ch0): " + original);
        adrv9002.setOrxBbdcRejectionEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isOrxBbdcRejectionEnabled(0);
        printToConsole("  Read back ORX BBDC rejection (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setOrxBbdcRejectionEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.ORX_QUAD_POLY", function() {
    try {
        var original = adrv9002.isOrxQuadraturePolyTrackingEnabled(0);
        printToConsole("  Original ORX quadrature poly tracking (ch0): " + original);
        adrv9002.setOrxQuadraturePolyTrackingEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isOrxQuadraturePolyTrackingEnabled(0);
        printToConsole("  Read back ORX quadrature poly tracking (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setOrxQuadraturePolyTrackingEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.ORX_ENABLED", function() {
    try {
        var original = adrv9002.isOrxEnabled(0);
        printToConsole("  Original ORX enabled (ch0): " + original);
        adrv9002.setOrxEnabled(0, "1");
        msleep(500);
        var readBack = adrv9002.isOrxEnabled(0);
        printToConsole("  Read back ORX enabled (ch0): " + readBack);
        // Restore original
        if (original && original !== readBack) {
            adrv9002.setOrxEnabled(0, original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Generic Widget Access Tests
// ============================================
printToConsole("\n=== Generic Widget Access Tests ===\n");

TestFramework.runTest("TST.ADRV9002.WIDGET_KEYS", function() {
    try {
        var keys = adrv9002.getWidgetKeys();
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

TestFramework.runTest("TST.ADRV9002.WIDGET_RW", function() {
    try {
        var keys = adrv9002.getWidgetKeys();
        if (!keys || keys.length === 0) {
            printToConsole("  Error: No widget keys available");
            return false;
        }
        var key = keys[0];
        var original = adrv9002.readWidget(key);
        printToConsole("  Key: " + key);
        printToConsole("  Original value: " + original);
        adrv9002.writeWidget(key, original);
        msleep(500);
        var readBack = adrv9002.readWidget(key);
        printToConsole("  Read back value: " + readBack);
        return readBack === original;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Utility Tests
// ============================================
printToConsole("\n=== Utility Tests ===\n");

TestFramework.runTest("TST.ADRV9002.REFRESH", function() {
    try {
        adrv9002.refresh();
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
