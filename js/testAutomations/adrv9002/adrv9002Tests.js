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

// ADRV9002 Automated Tests
// Covers: TST.ADRV9002.CONTROLS.*

evaluateFile("../js/testAutomations/common/testFramework.js");

function isFiniteNumber(value) {
    return typeof value === "number" && isFinite(value);
}

function isNonEmptyString(value) {
    return typeof value === "string" && value.length > 0;
}

function pauseAction() {
    msleep(1000);
}

TestFramework.init("ADRV9002 Tests");

if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

if (!switchToToolAny(["ADRV9002"])) {
    printToConsole("ERROR: Cannot access ADRV9002 tool");
    exit(1);
}
if (!waitForApi("adrv9002", 10000)) {
    printToConsole("ERROR: adrv9002 API not available");
    exit(1);
}

TestFramework.runTest("TST.ADRV9002.CONTROLS.PLUGIN_LOADS", function() {
    try {
        adrv9002.show();
        pauseAction();
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.DEVICE_DRIVER_API", function() {
    try {
        let version = adrv9002.getDriverVersion();
        if (!isNonEmptyString(version)) {
            return false;
        }
        printToConsole("  Driver version: " + version);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.GLOBAL_SETTINGS", function() {
    try {
        let profile = adrv9002.getProfileConfig();
        let stream = adrv9002.getStreamConfig();
        let tempC = adrv9002.getTemperatureC();
        let initCal = adrv9002.isInitialCalibrationsSupported();

        if (!isNonEmptyString(profile) || !isNonEmptyString(stream)) {
            return TestFramework.skip("Profile or stream config not available");
        }
        if (!isFiniteNumber(tempC)) {
            return TestFramework.skip("Temperature not available");
        }
        if (typeof initCal !== "boolean") {
            return TestFramework.skip("Initial calibration support not available");
        }

        printToConsole("  Profile: " + profile);
        printToConsole("  Stream: " + stream);
        printToConsole("  Temp C: " + tempC);
        printToConsole("  Initial cal supported: " + initCal);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.RX_CHANNEL_CONFIG", function() {
    try {
        let chan = 0;
        let gain = adrv9002.getRxGain(chan);
        let gainMode = adrv9002.getRxGainMode(chan);
        let ensm = adrv9002.getRxEnsmMode(chan);
        let powerdown = adrv9002.getRxPowerdown(chan);

        if (!isFiniteNumber(gain) || !isNonEmptyString(gainMode) || !isNonEmptyString(ensm)) {
            return TestFramework.skip("RX channel attributes not available");
        }

        let ok = true;
        ok = adrv9002.setRxGain(chan, gain) && ok;
        pauseAction();
        ok = adrv9002.setRxGainMode(chan, gainMode) && ok;
        pauseAction();
        ok = adrv9002.setRxEnsmMode(chan, ensm) && ok;
        pauseAction();
        ok = adrv9002.setRxPowerdown(chan, powerdown) && ok;
        pauseAction();

        if (!ok) {
            return false;
        }

        let pass = true;
        pass = TestFramework.assertApproxEqual(adrv9002.getRxGain(chan), gain, 0.1, "RX gain roundtrip") && pass;
        pass = TestFramework.assertEqual(adrv9002.getRxGainMode(chan), gainMode, "RX gain mode roundtrip") && pass;
        pass = TestFramework.assertEqual(adrv9002.getRxEnsmMode(chan), ensm, "RX ENSM roundtrip") && pass;
        pass = TestFramework.assertEqual(adrv9002.getRxPowerdown(chan), powerdown, "RX powerdown roundtrip") && pass;
        let decPower = adrv9002.getRxDecimatedPower(chan);
        let bw = adrv9002.getRxBandwidth(chan);
        let rate = adrv9002.getRxSampleRate(chan);

        if (!isFiniteNumber(decPower) || !isFiniteNumber(bw) || !isFiniteNumber(rate)) {
            return TestFramework.skip("RX read-only values not available");
        }

        printToConsole("  Decimated power: " + decPower);
        printToConsole("  RX bandwidth: " + bw);
        printToConsole("  RX sample rate: " + rate);
        return pass;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.TX_CHANNEL_CONFIG", function() {
    try {
        let chan = 0;
        let atten = adrv9002.getTxAtten(chan);
        let attenMode = adrv9002.getTxAttenMode(chan);
        let powerdown = adrv9002.getTxPowerdown(chan);

        if (!isFiniteNumber(atten) || !isNonEmptyString(attenMode)) {
            return TestFramework.skip("TX channel attributes not available");
        }

        let ok = true;
        ok = adrv9002.setTxAtten(chan, atten) && ok;
        pauseAction();
        ok = adrv9002.setTxAttenMode(chan, attenMode) && ok;
        pauseAction();
        ok = adrv9002.setTxPowerdown(chan, powerdown) && ok;
        pauseAction();

        if (!ok) {
            return false;
        }

        let pass = true;
        pass = TestFramework.assertApproxEqual(adrv9002.getTxAtten(chan), atten, 0.1, "TX atten roundtrip") && pass;
        pass = TestFramework.assertEqual(adrv9002.getTxAttenMode(chan), attenMode, "TX atten mode roundtrip") && pass;
        pass = TestFramework.assertEqual(adrv9002.getTxPowerdown(chan), powerdown, "TX powerdown roundtrip") && pass;
        let bw = adrv9002.getTxBandwidth(chan);
        let rate = adrv9002.getTxSampleRate(chan);

        if (!isFiniteNumber(bw) || !isFiniteNumber(rate)) {
            return TestFramework.skip("TX read-only values not available");
        }

        printToConsole("  TX bandwidth: " + bw);
        printToConsole("  TX sample rate: " + rate);
        return pass;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.ORX_CONFIG", function() {
    try {
        let chan = 0;
        let gain = adrv9002.getOrxGain(chan);
        if (!isFiniteNumber(gain)) {
            return TestFramework.skip("ORX gain not available");
        }

        let powerdown = adrv9002.getOrxPowerdown(chan);
        let bbdc = adrv9002.getBbdcRejection(chan);

        let ok = true;
        ok = adrv9002.setOrxGain(chan, gain) && ok;
        pauseAction();
        ok = adrv9002.setOrxPowerdown(chan, powerdown) && ok;
        pauseAction();
        ok = adrv9002.setBbdcRejection(chan, bbdc) && ok;
        pauseAction();

        if (!ok) {
            return false;
        }

        let pass = true;
        pass = TestFramework.assertApproxEqual(adrv9002.getOrxGain(chan), gain, 0.1, "ORX gain roundtrip") && pass;
        pass = TestFramework.assertEqual(adrv9002.getOrxPowerdown(chan), powerdown, "ORX powerdown roundtrip") && pass;
        pass = TestFramework.assertEqual(adrv9002.getBbdcRejection(chan), bbdc, "BBDC rejection roundtrip") && pass;
        return pass;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADRV9002.CONTROLS.REFRESH_FUNCTION", function() {
    try {
        adrv9002.refresh();
        pauseAction();
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.disconnectFromDevice();
let exitCode = TestFramework.printSummary();
exit(exitCode);
