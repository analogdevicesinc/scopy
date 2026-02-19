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

// AD9084 Plugin Automated Tests
// Runs all API tests: Connection, Tools, RX Chain, TX Chain, Widget Access, Utility

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: AD9084 Plugin Tests
TestFramework.init("AD9084 Plugin Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to AD9084 tool
if (!switchToTool("AD9084")) {
    printToConsole("ERROR: Cannot switch to AD9084 tool");
    scopy.exit();
}

// Use channel 0 for RX and TX tests by default
// Actual channel indices depend on hardware config; discover via getWidgetKeys()
var RX_CH = 0;
var TX_CH = 0;

// ============================================
// Connection & Tool Discovery
// ============================================
printToConsole("\n=== Connection & Tool Discovery ===\n");

TestFramework.runTest("TST.AD9084.TOOLS", function() {
    try {
        var tools = ad9084.getTools();
        printToConsole("  Tools found: " + tools);
        if (!tools || tools.length === 0) {
            printToConsole("  Error: getTools() returned empty list");
            return false;
        }
        var found = false;
        for (var i = 0; i < tools.length; i++) {
            if (tools[i].indexOf("AD9084") !== -1 || tools[i].indexOf("ad9084") !== -1) {
                found = true;
                break;
            }
        }
        if (!found) {
            printToConsole("  Warning: No tool containing 'AD9084' found in list, but tools are present");
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.WIDGET_KEYS", function() {
    try {
        var keys = ad9084.getWidgetKeys();
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

// ============================================
// RX Chain Tests
// ============================================
printToConsole("\n=== RX Chain Tests ===\n");

TestFramework.runTest("TST.AD9084.RX_ADC_FREQ", function() {
    try {
        var value = ad9084.getRxAdcFrequency(RX_CH);
        printToConsole("  RX ADC frequency (ch" + RX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getRxAdcFrequency() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.RX_CHN_NCO_FREQ", function() {
    try {
        var original = ad9084.getRxChannelNcoFrequency(RX_CH);
        printToConsole("  Original RX channel NCO frequency (ch" + RX_CH + "): " + original);
        if (!original || original === "") {
            printToConsole("  Error: getRxChannelNcoFrequency() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.RX_CHN_NCO_PHASE", function() {
    try {
        var value = ad9084.getRxChannelNcoPhase(RX_CH);
        printToConsole("  RX channel NCO phase (ch" + RX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getRxChannelNcoPhase() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.RX_MAIN_NCO_FREQ", function() {
    try {
        var value = ad9084.getRxMainNcoFrequency(RX_CH);
        printToConsole("  RX main NCO frequency (ch" + RX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getRxMainNcoFrequency() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.RX_MAIN_NCO_PHASE", function() {
    try {
        var value = ad9084.getRxMainNcoPhase(RX_CH);
        printToConsole("  RX main NCO phase (ch" + RX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getRxMainNcoPhase() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.RX_TEST_MODE", function() {
    try {
        var value = ad9084.getRxTestMode(RX_CH);
        printToConsole("  RX test mode (ch" + RX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getRxTestMode() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.RX_NYQUIST_ZONE", function() {
    try {
        var value = ad9084.getRxNyquistZone(RX_CH);
        printToConsole("  RX Nyquist zone (ch" + RX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getRxNyquistZone() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.RX_LOOPBACK", function() {
    try {
        var value = ad9084.getRxLoopback(RX_CH);
        printToConsole("  RX loopback (ch" + RX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getRxLoopback() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.RX_CFIR_EN", function() {
    try {
        var value = ad9084.isRxCfirEnabled(RX_CH);
        printToConsole("  RX CFIR enabled (ch" + RX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: isRxCfirEnabled() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.RX_CFIR_PROFILE", function() {
    try {
        var value = ad9084.getRxCfirProfileSel(RX_CH);
        printToConsole("  RX CFIR profile sel (ch" + RX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getRxCfirProfileSel() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.RX_EN", function() {
    try {
        var value = ad9084.isRxEnabled(RX_CH);
        printToConsole("  RX enabled (ch" + RX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: isRxEnabled() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// TX Chain Tests
// ============================================
printToConsole("\n=== TX Chain Tests ===\n");

TestFramework.runTest("TST.AD9084.TX_DAC_FREQ", function() {
    try {
        var value = ad9084.getTxDacFrequency(TX_CH);
        printToConsole("  TX DAC frequency (ch" + TX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxDacFrequency() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.TX_CHN_NCO_FREQ", function() {
    try {
        var value = ad9084.getTxChannelNcoFrequency(TX_CH);
        printToConsole("  TX channel NCO frequency (ch" + TX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxChannelNcoFrequency() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.TX_CHN_NCO_PHASE", function() {
    try {
        var value = ad9084.getTxChannelNcoPhase(TX_CH);
        printToConsole("  TX channel NCO phase (ch" + TX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxChannelNcoPhase() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.TX_MAIN_NCO_FREQ", function() {
    try {
        var value = ad9084.getTxMainNcoFrequency(TX_CH);
        printToConsole("  TX main NCO frequency (ch" + TX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxMainNcoFrequency() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.TX_MAIN_NCO_PHASE", function() {
    try {
        var value = ad9084.getTxMainNcoPhase(TX_CH);
        printToConsole("  TX main NCO phase (ch" + TX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxMainNcoPhase() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.TX_CHN_NCO_GAIN_SCALE", function() {
    try {
        var value = ad9084.getTxChannelNcoGainScale(TX_CH);
        printToConsole("  TX channel NCO gain scale (ch" + TX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxChannelNcoGainScale() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.TX_CHN_NCO_TEST_TONE_SCALE", function() {
    try {
        var value = ad9084.getTxChannelNcoTestToneScale(TX_CH);
        printToConsole("  TX channel NCO test tone scale (ch" + TX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxChannelNcoTestToneScale() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.TX_CHN_NCO_TEST_TONE_EN", function() {
    try {
        var value = ad9084.isTxChannelNcoTestToneEnabled(TX_CH);
        printToConsole("  TX channel NCO test tone enabled (ch" + TX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: isTxChannelNcoTestToneEnabled() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.TX_MAIN_NCO_TEST_TONE_SCALE", function() {
    try {
        var value = ad9084.getTxMainNcoTestToneScale(TX_CH);
        printToConsole("  TX main NCO test tone scale (ch" + TX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxMainNcoTestToneScale() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.TX_MAIN_NCO_TEST_TONE_EN", function() {
    try {
        var value = ad9084.isTxMainNcoTestToneEnabled(TX_CH);
        printToConsole("  TX main NCO test tone enabled (ch" + TX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: isTxMainNcoTestToneEnabled() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.TX_CFIR_EN", function() {
    try {
        var value = ad9084.isTxCfirEnabled(TX_CH);
        printToConsole("  TX CFIR enabled (ch" + TX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: isTxCfirEnabled() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.TX_CFIR_PROFILE", function() {
    try {
        var value = ad9084.getTxCfirProfileSel(TX_CH);
        printToConsole("  TX CFIR profile sel (ch" + TX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxCfirProfileSel() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD9084.TX_EN", function() {
    try {
        var value = ad9084.isTxEnabled(TX_CH);
        printToConsole("  TX enabled (ch" + TX_CH + "): " + value);
        if (!value || value === "") {
            printToConsole("  Error: isTxEnabled() returned empty string");
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

TestFramework.runTest("TST.AD9084.WIDGET_RW", function() {
    try {
        var keys = ad9084.getWidgetKeys();
        if (!keys || keys.length === 0) {
            printToConsole("  Error: No widget keys available");
            return false;
        }
        // Read the first available widget key
        var key = keys[0];
        var value = ad9084.readWidget(key);
        printToConsole("  Read widget '" + key + "': " + value);
        if (!value || value === "") {
            printToConsole("  Error: readWidget() returned empty string for key: " + key);
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Utility Tests
// ============================================
printToConsole("\n=== Utility Tests ===\n");

TestFramework.runTest("TST.AD9084.REFRESH", function() {
    try {
        ad9084.refresh();
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
