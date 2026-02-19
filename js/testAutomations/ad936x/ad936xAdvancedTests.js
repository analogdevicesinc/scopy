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

// AD936x Advanced Plugin Automated Tests
// Runs all API tests for the AD936X Advanced debug attributes

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: AD936x Advanced Plugin Tests
TestFramework.init("AD936x Advanced Plugin Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to AD936X Advanced tool
if (!switchToTool("AD936X Advanced")) {
    printToConsole("ERROR: Cannot switch to AD936X Advanced tool");
    exit(1);
}

// ============================================
// ENSM Mode
// ============================================
ad936x_advanced.switchSubtab("ENSM/Mode/Clocks");
msleep(500);
printToConsole("\n=== ENSM Mode ===\n");

TestFramework.runTest("TST.AD936X_ADV.FDD_MODE", function() {
    try {
        var original = ad936x_advanced.isFddModeEnabled();
        printToConsole("  Original FDD mode: " + original);
        ad936x_advanced.setFddModeEnabled("1");
        msleep(500);
        var readBack = ad936x_advanced.isFddModeEnabled();
        printToConsole("  Read back FDD mode: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setFddModeEnabled(original);
            msleep(500);
        }
        return readBack === "1";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.PIN_PULSE_MODE", function() {
    try {
        var original = ad936x_advanced.isEnsmPinPulseModeEnabled();
        printToConsole("  Original pin pulse mode: " + original);
        ad936x_advanced.setEnsmPinPulseModeEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isEnsmPinPulseModeEnabled();
        printToConsole("  Read back pin pulse mode: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setEnsmPinPulseModeEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.TXNRX_CONTROL", function() {
    try {
        var original = ad936x_advanced.isEnsmTxnrxControlEnabled();
        printToConsole("  Original TXNRX control: " + original);
        ad936x_advanced.setEnsmTxnrxControlEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isEnsmTxnrxControlEnabled();
        printToConsole("  Read back TXNRX control: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setEnsmTxnrxControlEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.TDD_DUAL_SYNTH", function() {
    try {
        var original = ad936x_advanced.isTddDualSynthModeEnabled();
        printToConsole("  Original TDD dual synth: " + original);
        ad936x_advanced.setTddDualSynthModeEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isTddDualSynthModeEnabled();
        printToConsole("  Read back TDD dual synth: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setTddDualSynthModeEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.TDD_FDD_VCO_TABLES", function() {
    try {
        var original = ad936x_advanced.isTddFddVcoTablesEnabled();
        printToConsole("  Original TDD FDD VCO tables: " + original);
        ad936x_advanced.setTddFddVcoTablesEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isTddFddVcoTablesEnabled();
        printToConsole("  Read back TDD FDD VCO tables: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setTddFddVcoTablesEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.TDD_SKIP_VCO_CAL", function() {
    try {
        var original = ad936x_advanced.isTddSkipVcoCalEnabled();
        printToConsole("  Original TDD skip VCO cal: " + original);
        ad936x_advanced.setTddSkipVcoCalEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isTddSkipVcoCalEnabled();
        printToConsole("  Read back TDD skip VCO cal: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setTddSkipVcoCalEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.UPDATE_TX_GAIN_ALERT", function() {
    try {
        var original = ad936x_advanced.isUpdateTxGainInAlertEnabled();
        printToConsole("  Original update TX gain in alert: " + original);
        ad936x_advanced.setUpdateTxGainInAlertEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isUpdateTxGainInAlertEnabled();
        printToConsole("  Read back update TX gain in alert: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setUpdateTxGainInAlertEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Mode
// ============================================
ad936x_advanced.switchSubtab("ENSM/Mode/Clocks");
msleep(500);
printToConsole("\n=== Mode ===\n");

TestFramework.runTest("TST.AD936X_ADV.RX_RF_PORT_INPUT", function() {
    try {
        var value = ad936x_advanced.getRxRfPortInputSelect();
        printToConsole("  RX RF port input select: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getRxRfPortInputSelect() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.TX_RF_PORT_INPUT", function() {
    try {
        var value = ad936x_advanced.getTxRfPortInputSelect();
        printToConsole("  TX RF port input select: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxRfPortInputSelect() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.RX1_RX2_PHASE_INV", function() {
    try {
        var original = ad936x_advanced.isRx1Rx2PhaseInversionEnabled();
        printToConsole("  Original RX1-RX2 phase inversion: " + original);
        ad936x_advanced.setRx1Rx2PhaseInversionEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isRx1Rx2PhaseInversionEnabled();
        printToConsole("  Read back RX1-RX2 phase inversion: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setRx1Rx2PhaseInversionEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Clocks
// ============================================
ad936x_advanced.switchSubtab("ENSM/Mode/Clocks");
msleep(500);
printToConsole("\n=== Clocks ===\n");

TestFramework.runTest("TST.AD936X_ADV.EXT_REFCLK", function() {
    try {
        var original = ad936x_advanced.isXoDisableUseExtRefclkEnabled();
        printToConsole("  Original ext refclk: " + original);
        ad936x_advanced.setXoDisableUseExtRefclkEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isXoDisableUseExtRefclkEnabled();
        printToConsole("  Read back ext refclk: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setXoDisableUseExtRefclkEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.EXT_RX_LO", function() {
    try {
        var original = ad936x_advanced.isExternalRxLoEnabled();
        printToConsole("  Original external RX LO: " + original);
        ad936x_advanced.setExternalRxLoEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isExternalRxLoEnabled();
        printToConsole("  Read back external RX LO: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setExternalRxLoEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.EXT_TX_LO", function() {
    try {
        var original = ad936x_advanced.isExternalTxLoEnabled();
        printToConsole("  Original external TX LO: " + original);
        ad936x_advanced.setExternalTxLoEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isExternalTxLoEnabled();
        printToConsole("  Read back external TX LO: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setExternalTxLoEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.CLK_OUTPUT_MODE", function() {
    try {
        var value = ad936x_advanced.getClkOutputModeSelect();
        printToConsole("  Clock output mode select: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getClkOutputModeSelect() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.RX_FASTLOCK_PINCTRL", function() {
    try {
        var original = ad936x_advanced.isRxFastlockPincontrolEnabled();
        printToConsole("  Original RX fastlock pincontrol: " + original);
        ad936x_advanced.setRxFastlockPincontrolEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isRxFastlockPincontrolEnabled();
        printToConsole("  Read back RX fastlock pincontrol: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setRxFastlockPincontrolEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.RX_FASTLOCK_DELAY", function() {
    try {
        var original = ad936x_advanced.getRxFastlockDelay();
        printToConsole("  Original RX fastlock delay: " + original);
        var testValue = "0";
        ad936x_advanced.setRxFastlockDelay(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getRxFastlockDelay();
        printToConsole("  Read back RX fastlock delay: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setRxFastlockDelay(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.TX_FASTLOCK_PINCTRL", function() {
    try {
        var original = ad936x_advanced.isTxFastlockPincontrolEnabled();
        printToConsole("  Original TX fastlock pincontrol: " + original);
        ad936x_advanced.setTxFastlockPincontrolEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isTxFastlockPincontrolEnabled();
        printToConsole("  Read back TX fastlock pincontrol: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setTxFastlockPincontrolEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.TX_FASTLOCK_DELAY", function() {
    try {
        var original = ad936x_advanced.getTxFastlockDelay();
        printToConsole("  Original TX fastlock delay: " + original);
        var testValue = "0";
        ad936x_advanced.setTxFastlockDelay(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getTxFastlockDelay();
        printToConsole("  Read back TX fastlock delay: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setTxFastlockDelay(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// eLNA
// ============================================
ad936x_advanced.switchSubtab("eLNA");
msleep(500);
printToConsole("\n=== eLNA ===\n");

TestFramework.runTest("TST.AD936X_ADV.ELNA_GAIN", function() {
    try {
        var original = ad936x_advanced.getElnaGain();
        printToConsole("  Original eLNA gain: " + original);
        var testValue = "0";
        ad936x_advanced.setElnaGain(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getElnaGain();
        printToConsole("  Read back eLNA gain: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setElnaGain(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.ELNA_BYPASS_LOSS", function() {
    try {
        var original = ad936x_advanced.getElnaBypassLoss();
        printToConsole("  Original eLNA bypass loss: " + original);
        var testValue = "0";
        ad936x_advanced.setElnaBypassLoss(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getElnaBypassLoss();
        printToConsole("  Read back eLNA bypass loss: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setElnaBypassLoss(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.ELNA_SETTLING_DELAY", function() {
    try {
        var original = ad936x_advanced.getElnaSettlingDelay();
        printToConsole("  Original eLNA settling delay: " + original);
        var testValue = "0";
        ad936x_advanced.setElnaSettlingDelay(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getElnaSettlingDelay();
        printToConsole("  Read back eLNA settling delay: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setElnaSettlingDelay(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.ELNA_RX1_GPO0", function() {
    try {
        var original = ad936x_advanced.isElnaRx1Gpo0ControlEnabled();
        printToConsole("  Original eLNA RX1 GPO0 control: " + original);
        ad936x_advanced.setElnaRx1Gpo0ControlEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isElnaRx1Gpo0ControlEnabled();
        printToConsole("  Read back eLNA RX1 GPO0 control: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setElnaRx1Gpo0ControlEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.ELNA_RX2_GPO1", function() {
    try {
        var original = ad936x_advanced.isElnaRx2Gpo1ControlEnabled();
        printToConsole("  Original eLNA RX2 GPO1 control: " + original);
        ad936x_advanced.setElnaRx2Gpo1ControlEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isElnaRx2Gpo1ControlEnabled();
        printToConsole("  Read back eLNA RX2 GPO1 control: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setElnaRx2Gpo1ControlEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.ELNA_GAINTABLE_ALL_IDX", function() {
    try {
        var original = ad936x_advanced.isElnaGaintableAllIndexEnabled();
        printToConsole("  Original eLNA gaintable all index: " + original);
        ad936x_advanced.setElnaGaintableAllIndexEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isElnaGaintableAllIndexEnabled();
        printToConsole("  Read back eLNA gaintable all index: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setElnaGaintableAllIndexEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// RSSI
// ============================================
ad936x_advanced.switchSubtab("RSSI");
msleep(500);
printToConsole("\n=== RSSI ===\n");

TestFramework.runTest("TST.AD936X_ADV.RSSI_DURATION", function() {
    try {
        var original = ad936x_advanced.getRssiDuration();
        printToConsole("  Original RSSI duration: " + original);
        var testValue = "1";
        ad936x_advanced.setRssiDuration(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getRssiDuration();
        printToConsole("  Read back RSSI duration: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setRssiDuration(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.RSSI_DELAY", function() {
    try {
        var original = ad936x_advanced.getRssiDelay();
        printToConsole("  Original RSSI delay: " + original);
        var testValue = "1";
        ad936x_advanced.setRssiDelay(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getRssiDelay();
        printToConsole("  Read back RSSI delay: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setRssiDelay(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.RSSI_WAIT", function() {
    try {
        var original = ad936x_advanced.getRssiWait();
        printToConsole("  Original RSSI wait: " + original);
        var testValue = "1";
        ad936x_advanced.setRssiWait(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getRssiWait();
        printToConsole("  Read back RSSI wait: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setRssiWait(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.RSSI_RESTART_MODE", function() {
    try {
        var value = ad936x_advanced.getRssiRestartMode();
        printToConsole("  RSSI restart mode: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getRssiRestartMode() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Gain Mode
// ============================================
ad936x_advanced.switchSubtab("GAIN");
msleep(500);
printToConsole("\n=== Gain Mode ===\n");

TestFramework.runTest("TST.AD936X_ADV.GC_RX1_MODE", function() {
    try {
        var value = ad936x_advanced.getGcRx1Mode();
        printToConsole("  GC RX1 mode: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getGcRx1Mode() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.GC_RX2_MODE", function() {
    try {
        var value = ad936x_advanced.getGcRx2Mode();
        printToConsole("  GC RX2 mode: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getGcRx2Mode() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.SPLIT_GAIN_TABLE", function() {
    try {
        var original = ad936x_advanced.isSplitGainTableModeEnabled();
        printToConsole("  Original split gain table: " + original);
        ad936x_advanced.setSplitGainTableModeEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isSplitGainTableModeEnabled();
        printToConsole("  Read back split gain table: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setSplitGainTableModeEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.GC_DEC_POW_DURATION", function() {
    try {
        var original = ad936x_advanced.getGcDecPowMeasurementDuration();
        printToConsole("  Original measurement duration: " + original);
        var testValue = "1";
        ad936x_advanced.setGcDecPowMeasurementDuration(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getGcDecPowMeasurementDuration();
        printToConsole("  Read back measurement duration: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setGcDecPowMeasurementDuration(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.AGC_DELAY_MARGIN", function() {
    try {
        var original = ad936x_advanced.getAgcAttackDelayExtraMargin();
        printToConsole("  Original AGC delay margin: " + original);
        var testValue = "1";
        ad936x_advanced.setAgcAttackDelayExtraMargin(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getAgcAttackDelayExtraMargin();
        printToConsole("  Read back AGC delay margin: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setAgcAttackDelayExtraMargin(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// AGC Thresholds
// ============================================
ad936x_advanced.switchSubtab("GAIN");
msleep(500);
printToConsole("\n=== AGC Thresholds ===\n");

TestFramework.runTest("TST.AD936X_ADV.AGC_OUTER_THRESH_HIGH", function() {
    try {
        var original = ad936x_advanced.getAgcOuterThreshHigh();
        printToConsole("  Original AGC outer thresh high: " + original);
        var testValue = "5";
        ad936x_advanced.setAgcOuterThreshHigh(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getAgcOuterThreshHigh();
        printToConsole("  Read back AGC outer thresh high: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setAgcOuterThreshHigh(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.AGC_INNER_THRESH_HIGH", function() {
    try {
        var original = ad936x_advanced.getAgcInnerThreshHigh();
        printToConsole("  Original AGC inner thresh high: " + original);
        var testValue = "5";
        ad936x_advanced.setAgcInnerThreshHigh(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getAgcInnerThreshHigh();
        printToConsole("  Read back AGC inner thresh high: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setAgcInnerThreshHigh(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.AGC_INNER_THRESH_LOW", function() {
    try {
        var original = ad936x_advanced.getAgcInnerThreshLow();
        printToConsole("  Original AGC inner thresh low: " + original);
        var testValue = "5";
        ad936x_advanced.setAgcInnerThreshLow(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getAgcInnerThreshLow();
        printToConsole("  Read back AGC inner thresh low: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setAgcInnerThreshLow(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.AGC_OUTER_THRESH_LOW", function() {
    try {
        var original = ad936x_advanced.getAgcOuterThreshLow();
        printToConsole("  Original AGC outer thresh low: " + original);
        var testValue = "5";
        ad936x_advanced.setAgcOuterThreshLow(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getAgcOuterThreshLow();
        printToConsole("  Read back AGC outer thresh low: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setAgcOuterThreshLow(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.AGC_GAIN_UPDATE_INTERVAL", function() {
    try {
        var original = ad936x_advanced.getAgcGainUpdateInterval();
        printToConsole("  Original AGC gain update interval: " + original);
        var testValue = "1";
        ad936x_advanced.setAgcGainUpdateInterval(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getAgcGainUpdateInterval();
        printToConsole("  Read back AGC gain update interval: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setAgcGainUpdateInterval(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// TX Monitor
// ============================================
ad936x_advanced.switchSubtab("TX MONITOR");
msleep(500);
printToConsole("\n=== TX Monitor ===\n");

TestFramework.runTest("TST.AD936X_ADV.TXMON1_FE_GAIN", function() {
    try {
        var value = ad936x_advanced.getTxmon1FrontEndGain();
        printToConsole("  TXMON1 front end gain: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxmon1FrontEndGain() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.TXMON2_FE_GAIN", function() {
    try {
        var value = ad936x_advanced.getTxmon2FrontEndGain();
        printToConsole("  TXMON2 front end gain: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getTxmon2FrontEndGain() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.TXMON_LOW_HIGH_THRESH", function() {
    try {
        var original = ad936x_advanced.getTxmonLowHighThresh();
        printToConsole("  Original TXMON low/high thresh: " + original);
        var testValue = "1";
        ad936x_advanced.setTxmonLowHighThresh(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getTxmonLowHighThresh();
        printToConsole("  Read back TXMON low/high thresh: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setTxmonLowHighThresh(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.TXMON_LOW_GAIN", function() {
    try {
        var original = ad936x_advanced.getTxmonLowGain();
        printToConsole("  Original TXMON low gain: " + original);
        var testValue = "1";
        ad936x_advanced.setTxmonLowGain(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getTxmonLowGain();
        printToConsole("  Read back TXMON low gain: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setTxmonLowGain(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.TXMON_HIGH_GAIN", function() {
    try {
        var original = ad936x_advanced.getTxmonHighGain();
        printToConsole("  Original TXMON high gain: " + original);
        var testValue = "1";
        ad936x_advanced.setTxmonHighGain(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getTxmonHighGain();
        printToConsole("  Read back TXMON high gain: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setTxmonHighGain(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.TXMON_DC_TRACKING", function() {
    try {
        var original = ad936x_advanced.isTxmonDcTrackingEnabled();
        printToConsole("  Original TXMON DC tracking: " + original);
        ad936x_advanced.setTxmonDcTrackingEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isTxmonDcTrackingEnabled();
        printToConsole("  Read back TXMON DC tracking: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setTxmonDcTrackingEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.TXMON_ONE_SHOT", function() {
    try {
        var original = ad936x_advanced.isTxmonOneShotModeEnabled();
        printToConsole("  Original TXMON one-shot mode: " + original);
        ad936x_advanced.setTxmonOneShotModeEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isTxmonOneShotModeEnabled();
        printToConsole("  Read back TXMON one-shot mode: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setTxmonOneShotModeEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Temp Sensor
// ============================================
ad936x_advanced.switchSubtab("MISC");
msleep(500);
printToConsole("\n=== Temp Sensor ===\n");

TestFramework.runTest("TST.AD936X_ADV.TEMP_MEAS_INTERVAL", function() {
    try {
        var original = ad936x_advanced.getTempSenseMeasurementInterval();
        printToConsole("  Original temp measurement interval: " + original);
        var testValue = "1000";
        ad936x_advanced.setTempSenseMeasurementInterval(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getTempSenseMeasurementInterval();
        printToConsole("  Read back temp measurement interval: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setTempSenseMeasurementInterval(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.TEMP_PERIODIC_MEAS", function() {
    try {
        var original = ad936x_advanced.isTempSensePeriodicMeasurementEnabled();
        printToConsole("  Original temp periodic measurement: " + original);
        ad936x_advanced.setTempSensePeriodicMeasurementEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isTempSensePeriodicMeasurementEnabled();
        printToConsole("  Read back temp periodic measurement: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setTempSensePeriodicMeasurementEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// MISC
// ============================================
ad936x_advanced.switchSubtab("MISC");
msleep(500);
printToConsole("\n=== MISC ===\n");

TestFramework.runTest("TST.AD936X_ADV.DC_OFFSET_ATTEN_HIGH", function() {
    try {
        var original = ad936x_advanced.getDcOffsetAttenuationHighRange();
        printToConsole("  Original DC offset attenuation high range: " + original);
        var testValue = "5";
        ad936x_advanced.setDcOffsetAttenuationHighRange(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getDcOffsetAttenuationHighRange();
        printToConsole("  Read back DC offset attenuation high range: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setDcOffsetAttenuationHighRange(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.DC_OFFSET_ATTEN_LOW", function() {
    try {
        var original = ad936x_advanced.getDcOffsetAttenuationLowRange();
        printToConsole("  Original DC offset attenuation low range: " + original);
        var testValue = "5";
        ad936x_advanced.setDcOffsetAttenuationLowRange(testValue);
        msleep(500);
        var readBack = ad936x_advanced.getDcOffsetAttenuationLowRange();
        printToConsole("  Read back DC offset attenuation low range: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setDcOffsetAttenuationLowRange(original);
            msleep(500);
        }
        return readBack === testValue;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.QEC_TRACKING_SLOW", function() {
    try {
        var original = ad936x_advanced.isQecTrackingSlowModeEnabled();
        printToConsole("  Original QEC tracking slow mode: " + original);
        ad936x_advanced.setQecTrackingSlowModeEnabled("0");
        msleep(500);
        var readBack = ad936x_advanced.isQecTrackingSlowModeEnabled();
        printToConsole("  Read back QEC tracking slow mode: " + readBack);
        if (original && original !== readBack) {
            ad936x_advanced.setQecTrackingSlowModeEnabled(original);
            msleep(500);
        }
        return readBack === "0";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// BIST
// ============================================
ad936x_advanced.switchSubtab("BIST");
msleep(500);
printToConsole("\n=== BIST ===\n");

TestFramework.runTest("TST.AD936X_ADV.BIST_PRBS", function() {
    try {
        var value = ad936x_advanced.getBistPrbs();
        printToConsole("  BIST PRBS: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getBistPrbs() returned empty string");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.AD936X_ADV.LOOPBACK", function() {
    try {
        var value = ad936x_advanced.getLoopback();
        printToConsole("  Loopback: " + value);
        if (!value || value === "") {
            printToConsole("  Error: getLoopback() returned empty string");
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

TestFramework.runTest("TST.AD936X_ADV.WIDGET_KEYS", function() {
    try {
        var keys = ad936x_advanced.getWidgetKeys();
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

TestFramework.runTest("TST.AD936X_ADV.WIDGET_RW", function() {
    try {
        var keys = ad936x_advanced.getWidgetKeys();
        if (!keys || keys.length === 0) {
            printToConsole("  Error: No widget keys available");
            return false;
        }
        var key = keys[0];
        var original = ad936x_advanced.readWidget(key);
        printToConsole("  Original value for '" + key + "': " + original);
        ad936x_advanced.writeWidget(key, original);
        msleep(500);
        var readBack = ad936x_advanced.readWidget(key);
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

TestFramework.runTest("TST.AD936X_ADV.REFRESH", function() {
    try {
        ad936x_advanced.refresh();
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
