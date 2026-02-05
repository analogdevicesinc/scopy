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
 */

// AD9084 Automated Tests
// Covers: TST.AD9084.*

evaluateFile("../js/testAutomations/common/testFramework.js");

function isFiniteNumber(value) {
    return typeof value === "number" && isFinite(value);
}

function pauseAction() {
    msleep(1000);
}

TestFramework.init("AD9084 Tests");

if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

if (!switchToToolAny(["AD9084", "AD9088"])) {
    printToConsole("ERROR: Cannot access AD9084 tool");
    exit(1);
}
if (!waitForApi("ad9084", 10000)) {
    printToConsole("ERROR: ad9084 API not available");
    exit(1);
}

TestFramework.runTest("TST.AD9084.PLUGIN_LOADS", function() {
    try {
        ad9084.show();
        pauseAction();
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.DEVICE_DETECTION_AND_DISPLAY", function() {
    try {
        let nco = ad9084.getNco();
        if (!isFiniteNumber(nco)) {
            let rate = ad9084.getSampleRate();
            if (!isFiniteNumber(rate)) {
                return TestFramework.skip("No readable device attributes");
            }
            printToConsole("  Sample rate: " + rate);
            return true;
        }
        printToConsole("  NCO: " + nco);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.CHANNEL_CONFIGURATION", function() {
    try {
        let idx = 0;
        let initial = ad9084.getEnableChannel(idx);
        ad9084.setEnableChannel(idx, !initial);
        pauseAction();
        let toggled = ad9084.getEnableChannel(idx);
        ad9084.setEnableChannel(idx, initial);
        pauseAction();
        let restored = ad9084.getEnableChannel(idx);

        let pass = true;
        pass = TestFramework.assertEqual(toggled, !initial, "Channel toggled") && pass;
        pass = TestFramework.assertEqual(restored, initial, "Channel restored") && pass;
        return pass;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.DISABLE_ENABLE_RX_TX_TABS", function() {
    try {
        ad9084.setRxTabEnabled(false);
        pauseAction();
        ad9084.setTxTabEnabled(false);
        pauseAction();
        let rxDisabled = ad9084.getRxTabEnabled();
        let txDisabled = ad9084.getTxTabEnabled();

        ad9084.setRxTabEnabled(true);
        pauseAction();
        ad9084.setTxTabEnabled(true);
        pauseAction();
        let rxEnabled = ad9084.getRxTabEnabled();
        let txEnabled = ad9084.getTxTabEnabled();

        let pass = true;
        pass = TestFramework.assertEqual(rxDisabled, false, "RX tab disabled") && pass;
        pass = TestFramework.assertEqual(txDisabled, false, "TX tab disabled") && pass;
        pass = TestFramework.assertEqual(rxEnabled, true, "RX tab enabled") && pass;
        pass = TestFramework.assertEqual(txEnabled, true, "TX tab enabled") && pass;
        return pass;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.disconnectFromDevice();
let exitCode = TestFramework.printSummary();
exit(exitCode);
