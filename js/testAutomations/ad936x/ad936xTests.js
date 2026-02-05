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

// AD936X Automated Tests
// Covers: TST.AD936X.* and TST.AD936X_ADVANCED.*

evaluateFile("../js/testAutomations/common/testFramework.js");

function isFiniteNumber(value) {
    return typeof value === "number" && isFinite(value);
}

const UI_PAUSE_MS = 1000;

function pauseAction() {
    msleep(UI_PAUSE_MS);
}

TestFramework.init("AD936X Tests");

if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

if (!switchToToolAny(["AD936X", "FMCOMMS5"])) {
    printToConsole("ERROR: Cannot access AD936X tool");
    exit(1);
}
pauseAction();
var ad = resolveApi("ad936x", 10000);
if (!ad) {
    printToConsole("ERROR: ad936x API not available");
    exit(1);
}
if (typeof ad.show === "function") {
    ad.show();
    pauseAction();
}

TestFramework.runTest("TST.AD936X.PLUGIN_LOADS", function() {
    try {
        if (!ad || typeof ad.getSampleRate !== "function") {
            return "ad936x API not available";
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.DEVICE_DETECTION_AND_DISPLAY", function() {
    try {
        if (!ad) {
            return "ad936x API not available";
        }
        let sampleRate = ad.getSampleRate();
        if (!isFiniteNumber(sampleRate)) {
            return TestFramework.skip("Sample rate not available");
        }
        printToConsole("  Sample rate: " + sampleRate);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.CHANGE_VALIDATE_GLOBAL_SETTINGS", function() {
    try {
        if (!ad) {
            return "ad936x API not available";
        }
        let ensm = ad.getEnsmMode();
        let calib = ad.getCalibrationMode();
        let rateGov = ad.getRateGovernor();

        if (!ensm || !calib || !rateGov) {
            return TestFramework.skip("Required attributes not available");
        }

        let ok1 = ad.setEnsmMode(ensm);
        pauseAction();
        let ok2 = ad.setCalibrationMode(calib);
        pauseAction();
        let ok3 = ad.setRateGovernor(rateGov);
        pauseAction();

        if (!ok1 || !ok2 || !ok3) {
            return false;
        }

        let pass = true;
        pass = TestFramework.assertEqual(ad.getEnsmMode(), ensm, "ENSM mode roundtrip") && pass;
        pass = TestFramework.assertEqual(ad.getCalibrationMode(), calib, "Calibration mode roundtrip") && pass;
        pass = TestFramework.assertEqual(ad.getRateGovernor(), rateGov, "Rate governor roundtrip") && pass;
        return pass;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X.RX_TX_CHAIN_CONFIG", function() {
    try {
        if (!ad) {
            return "ad936x API not available";
        }
        let rxBw = ad.getRxBandwidth();
        let txBw = ad.getTxBandwidth();
        let rate = ad.getSampleRate();
        let txLo = ad.getTxLo();

        if (!isFiniteNumber(rxBw) || !isFiniteNumber(txBw) || !isFiniteNumber(rate) || !isFiniteNumber(txLo)) {
            return TestFramework.skip("RX/TX chain attributes not available");
        }

        let ok = true;
        ok = ad.setRxBandwidth(rxBw) && ok;
        pauseAction();
        ok = ad.setTxBandwidth(txBw) && ok;
        pauseAction();
        ok = ad.setSampleRate(rate) && ok;
        pauseAction();
        ok = ad.setTxLo(txLo) && ok;
        pauseAction();

        if (!ok) {
            return false;
        }

        let pass = true;
        pass = TestFramework.assertApproxEqual(ad.getRxBandwidth(), rxBw, 1, "RX bandwidth roundtrip") && pass;
        pass = TestFramework.assertApproxEqual(ad.getTxBandwidth(), txBw, 1, "TX bandwidth roundtrip") && pass;
        pass = TestFramework.assertApproxEqual(ad.getSampleRate(), rate, 1, "Sample rate roundtrip") && pass;
        pass = TestFramework.assertApproxEqual(ad.getTxLo(), txLo, 1, "TX LO roundtrip") && pass;
        return pass;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADVANCED.PLUGIN_DETECTION_AND_DISPLAY", function() {
    try {
        if (!switchToToolAny(["AD936X Advanced", "FMCOMMS5 Advanced"])) {
            return TestFramework.skip("Advanced tool not available");
        }
        pauseAction();
        let adv = resolveApi("ad936x_adv", 5000);
        if (!adv) {
            return "ad936x_adv API not available";
        }
        if (typeof adv.show === "function") {
            adv.show();
            pauseAction();
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.disconnectFromDevice();
let exitCode = TestFramework.printSummary();
exit(exitCode);
