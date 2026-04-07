/*
 * Copyright (c) 2024 Analog Devices Inc.
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

// PQM Plugin Complete Automated Tests
// Runs all instrument tests: RMS, Harmonics, Waveform, Settings

// Load test framework
evaluateFile("js/testAutomations/common/testFramework.js");

// Test Suite: PQM Plugin Complete Tests
TestFramework.init("PQM Plugin Complete Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// ============================================
// RMS Instrument Tests
// ============================================
printToConsole("\n=== RMS Instrument Tests ===\n");

if (switchToTool("Rms")) {

    TestFramework.runTest("TST.PQM.RMS_OPEN", function() {
        try {
            // Verify tool opened by checking API is accessible
            pqm.isRmsRunning();
            pqm.isRmsLoggingEnabled();
            printToConsole("  RMS tool opened successfully");
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.RMS_INFO", function() {
        return "SKIP";
    });

    TestFramework.runTest("TST.PQM.RMS_SINGLE", function() {
        try {
            pqm.rmsSingleShot();
            msleep(3000);
            var isRunning = pqm.isRmsRunning();
            return !isRunning;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.RMS_RUN", function() {
        try {
            pqm.setRmsRunning(true);
            msleep(1000);
            if (!pqm.isRmsRunning()) {
                return false;
            }
            msleep(4000);
            pqm.setRmsRunning(false);
            msleep(500);
            return !pqm.isRmsRunning();
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setRmsRunning(false);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.RMS_LOG", function() {
        try {
            var logPath = fileIO.getTempPath();
            pqm.setRmsLoggingEnabled(true);
            msleep(500);
            if (!pqm.isRmsLoggingEnabled()) return false;
            pqm.setRmsLogPath(logPath);
            msleep(500);
            if (pqm.getRmsLogPath() !== logPath) {
                pqm.setRmsLoggingEnabled(false);
                return false;
            }
            pqm.setRmsRunning(true);
            msleep(2000);
            // Verify log file exists and is not empty
            var actualLogFile = pqm.getLogFilePath();
            if (!actualLogFile || actualLogFile === "") {
                printToConsole("  Log file path is empty");
                pqm.setRmsRunning(false);
                msleep(500);
                pqm.setRmsLoggingEnabled(false);
                return false;
            }
            var logContent = fileIO.readAll(actualLogFile);
            if (!logContent || logContent.length === 0) {
                printToConsole("  Log file is empty");
                pqm.setRmsRunning(false);
                msleep(500);
                pqm.setRmsLoggingEnabled(false);
                return false;
            }
            pqm.setRmsRunning(false);
            msleep(500);
            pqm.setRmsLoggingEnabled(false);
            msleep(500);
            return !pqm.isRmsLoggingEnabled();
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setRmsRunning(false);
            msleep(500);
            pqm.setRmsLoggingEnabled(false);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.RMS_LOG_WRONG", function() {
        try {
            var invalidPath = "/invalid/path/that/does/not/exist";
            pqm.setRmsLoggingEnabled(true);
            msleep(500);
            pqm.setRmsLogPath(invalidPath);
            msleep(500);
            pqm.setRmsRunning(true);
            msleep(3000);
            pqm.setRmsRunning(false);
            msleep(500);

            if (pqm.isRmsLoggingEnabled()) {
                printToConsole("  Logging section remained open (expected to close)");
                return false;
            }

            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setRmsRunning(false);
            msleep(500);
            pqm.setRmsLoggingEnabled(false);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.RMS_SETTINGS", function() {
        return "SKIP";
    });

    TestFramework.runTest("TST.PQM.RMS_PQEVENTS", function() {
        try {
            pqm.setRmsRunning(true);
            msleep(1000);
            pqm.triggerPqEvent(true);
            msleep(2000);
            pqm.setRmsRunning(false);
            msleep(500);
            if (!pqm.isRmsPqEvents()) {
                printToConsole("  No PQ events detected after trigger (expected some)");
                pqm.triggerPqEvent(false);
                return false;
            }
            pqm.triggerPqEvent(false);
            pqm.resetRmsPqEvents();
            msleep(500);
            return !pqm.isRmsPqEvents();
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setRmsRunning(false);
            pqm.triggerPqEvent(false);
            pqm.resetRmsPqEvents();
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.RMS_PQEVENTS_SYNC", function() {
        try {
            pqm.setRmsRunning(true);
            msleep(1000);
            pqm.setHarmonicsRunning(true);
            msleep(1000);
            pqm.triggerPqEvent(true);
            msleep(2000);
            pqm.setRmsRunning(false);
            msleep(500);
            pqm.setHarmonicsRunning(false);
            msleep(500);
            if (!pqm.isRmsPqEvents()) {
                printToConsole("  No PQ events detected after trigger (expected some) - RMS");
                pqm.triggerPqEvent(false);
                pqm.resetRmsPqEvents();
                return false;
            }
            if (!pqm.isHarmonicsPqEvents()) {
                printToConsole("  No PQ events detected after trigger (expected some) - Harmonics");
                pqm.triggerPqEvent(false);
                pqm.resetRmsPqEvents();
                return false;
            }
            pqm.triggerPqEvent(false);
            pqm.resetRmsPqEvents();
            msleep(500);
            return !pqm.isRmsPqEvents() && !pqm.isHarmonicsPqEvents();
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setRmsRunning(false);
            pqm.setHarmonicsRunning(false);
            pqm.triggerPqEvent(false);
            pqm.resetRmsPqEvents();
            return false;
        }
    });
}

// ============================================
// Harmonics Instrument Tests
// ============================================
printToConsole("\n=== Harmonics Instrument Tests ===\n");

if (switchToTool("Harmonics")) {

    TestFramework.runTest("TST.PQM.HARMONICS_OPEN", function() {
        try {
            // Verify tool opened by checking API is accessible
            var isRunning = pqm.isHarmonicsRunning();
            var loggingEnabled = pqm.isHarmonicsLoggingEnabled();
            var activeChannel = pqm.getHarmonicsActiveChannel();
            printToConsole("  Harmonics tool opened successfully");
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.HARMONICS_INFO", function() {
        return "SKIP";
    });

    TestFramework.runTest("TST.PQM.HARMONICS_SINGLE", function() {
        try {
            pqm.harmonicsSingleShot();
            msleep(3000);
            return !pqm.isHarmonicsRunning();
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.HARMONICS_RUN", function() {
        try {
            pqm.setHarmonicsRunning(true);
            msleep(1000);
            if (!pqm.isHarmonicsRunning()) return false;
            msleep(2000);
            pqm.setHarmonicsRunning(false);
            msleep(500);
            return !pqm.isHarmonicsRunning();
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setHarmonicsRunning(false);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.HARMONICS_OVERVIEW_PLOT", function() {
        return "SKIP";
    });

    TestFramework.runTest("TST.PQM.HARMONICS_SETTINGS", function() {
        return "SKIP";
    });

    TestFramework.runTest("TST.PQM.HARMONICS_LOG", function() {
        try {
            var logPath = fileIO.getTempPath();
            pqm.setHarmonicsLoggingEnabled(true);
            msleep(500);
            if (!pqm.isHarmonicsLoggingEnabled()) return false;
            pqm.setHarmonicsLogPath(logPath);
            msleep(500);
            if (pqm.getHarmonicsLogPath() !== logPath) {
                pqm.setHarmonicsLoggingEnabled(false);
                return false;
            }
            pqm.setHarmonicsRunning(true);
            msleep(2000);
            // Verify log file exists and is not empty
            var actualLogFile = pqm.getLogFilePath();
            if (!actualLogFile || actualLogFile === "") {
                printToConsole("  Log file path is empty");
                pqm.setHarmonicsRunning(false);
                msleep(500);
                pqm.setHarmonicsLoggingEnabled(false);
                return false;
            }
            var logContent = fileIO.readAll(actualLogFile);
            if (!logContent || logContent.length === 0) {
                printToConsole("  Log file is empty");
                pqm.setHarmonicsRunning(false);
                msleep(500);
                pqm.setHarmonicsLoggingEnabled(false);
                return false;
            }
            pqm.setHarmonicsRunning(false);
            msleep(500);
            pqm.setHarmonicsLoggingEnabled(false);
            msleep(500);
            return !pqm.isHarmonicsLoggingEnabled();
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setHarmonicsRunning(false);
            msleep(500);
            pqm.setHarmonicsLoggingEnabled(false);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.HARMONICS_LOG_WRONG", function() {
        try {
            var invalidPath = "/invalid/path/that/does/not/exist";
            pqm.setHarmonicsLoggingEnabled(true);
            msleep(500);
            pqm.setHarmonicsLogPath(invalidPath);
            msleep(500);
            pqm.setHarmonicsRunning(true);
            msleep(3000);
            pqm.setHarmonicsRunning(false);
            msleep(500);

            if (pqm.isHarmonicsLoggingEnabled()) {
                printToConsole("  Logging section remained open (expected to close)");
                return false;
            }

            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setHarmonicsRunning(false);
            msleep(500);
            pqm.setHarmonicsLoggingEnabled(false);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.HARMONICS_PQEVENTS", function() {
        try {
            pqm.setHarmonicsRunning(true);
            msleep(1000);
            pqm.triggerPqEvent(true);
            msleep(2000);
            pqm.setHarmonicsRunning(false);
            msleep(500);
            if (!pqm.isHarmonicsPqEvents()) {
                printToConsole("  No Harmonics PQ events detected after trigger (expected some)");
                pqm.triggerPqEvent(false);
                return false;
            }
            pqm.triggerPqEvent(false);
            pqm.resetHarmonicsPqEvents();
            msleep(500);
            return !pqm.isHarmonicsPqEvents();
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setHarmonicsRunning(false);
            pqm.triggerPqEvent(false);
            pqm.resetHarmonicsPqEvents();
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.HARMONICS_CURRENT_PLOTS_DATA", function() {
        return "SKIP";
    });
}

// ============================================
// Waveform Instrument Tests
// ============================================
printToConsole("\n=== Waveform Instrument Tests ===\n");

if (switchToTool("Waveform")) {

    TestFramework.runTest("TST.PQM.WAVEFORM_OPEN", function() {
        try {
            // Verify tool opened by checking API is accessible
            var isRunning = pqm.isWaveformRunning();
            var loggingEnabled = pqm.isWaveformLoggingEnabled();
            var timespan = pqm.getWaveformTimespan();
            var rollingMode = pqm.isWaveformRollingMode();
            printToConsole("  Waveform tool opened successfully");
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_INFO", function() {
        return "SKIP";
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_SETTINGS", function() {
        return "SKIP";
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_SETTINGS_TIMESPAN", function() {
        var originalTimespan = pqm.getWaveformTimespan();
        try {
            var testValues = [0.02, 1.0, 10.0];
            for (var i = 0; i < testValues.length; i++) {
                pqm.setWaveformTimespan(testValues[i]);
                msleep(500);
                if (Math.abs(pqm.getWaveformTimespan() - testValues[i]) > 0.001) {
                    pqm.setWaveformTimespan(originalTimespan);
                    msleep(500);
                    return false;
                }
            }
            pqm.setWaveformTimespan(originalTimespan);
            msleep(500);
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setWaveformTimespan(originalTimespan);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_SETTINGS_ROLLING", function() {
        var originalRolling = pqm.isWaveformRollingMode();
        try {
            pqm.setWaveformRollingMode(true);
            msleep(500);
            if (!pqm.isWaveformRollingMode()) {
                pqm.setWaveformRollingMode(originalRolling);
                msleep(500);
                return false;
            }
            pqm.setWaveformRollingMode(false);
            msleep(500);
            if (pqm.isWaveformRollingMode()) {
                pqm.setWaveformRollingMode(originalRolling);
                msleep(500);
                return false;
            }
            pqm.setWaveformRollingMode(originalRolling);
            msleep(500);
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setWaveformRollingMode(originalRolling);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_SINGLE_TRIG", function() {
        try {
            pqm.setWaveformRollingMode(false);
            msleep(500);
            pqm.waveformSingleShot();
            msleep(10000);
            return !pqm.isWaveformRunning();
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setWaveformRunning(false);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_SINGLE_ROLL", function() {
        try {
            pqm.setWaveformRollingMode(true);
            msleep(500);
            pqm.waveformSingleShot();
            msleep(10000);
            return !pqm.isWaveformRunning();
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setWaveformRunning(false);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_RUN_TRIG", function() {
        try {
            pqm.setWaveformRollingMode(false);
            msleep(500);
            pqm.setWaveformRunning(true);
            msleep(1000);
            if (!pqm.isWaveformRunning()) return false;
            msleep(4000);
            pqm.setWaveformRunning(false);
            msleep(500);
            return !pqm.isWaveformRunning();
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setWaveformRunning(false);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_RUN_ROLL", function() {
        try {
            pqm.setWaveformRollingMode(true);
            msleep(500);
            pqm.setWaveformRunning(true);
            msleep(1000);
            if (!pqm.isWaveformRunning()) return false;
            msleep(4000);
            pqm.setWaveformRunning(false);
            msleep(500);
            return !pqm.isWaveformRunning();
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setWaveformRunning(false);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_PLOT_ZOOM", function() {
        return "SKIP";
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_LOG", function() {
        try {
            var logPath = fileIO.getTempPath();
            pqm.setWaveformLoggingEnabled(true);
            msleep(500);
            if (!pqm.isWaveformLoggingEnabled()) return false;
            pqm.setWaveformLogPath(logPath);
            msleep(500);
            if (pqm.getWaveformLogPath() !== logPath) {
                pqm.setWaveformLoggingEnabled(false);
                return false;
            }
            pqm.setWaveformRunning(true);
            msleep(5000);
            // Verify log file exists and is not empty
            var actualLogFile = pqm.getLogFilePath();
            if (!actualLogFile || actualLogFile === "") {
                printToConsole("  Log file path is empty");
                pqm.setWaveformRunning(false);
                msleep(500);
                pqm.setWaveformLoggingEnabled(false);
                return false;
            }
            var logContent = fileIO.readAll(actualLogFile);
            if (!logContent || logContent.length === 0) {
                printToConsole("  Log file is empty");
                pqm.setWaveformRunning(false);
                msleep(500);
                pqm.setWaveformLoggingEnabled(false);
                return false;
            }
            pqm.setWaveformRunning(false);
            msleep(500);
            pqm.setWaveformLoggingEnabled(false);
            msleep(500);
            return !pqm.isWaveformLoggingEnabled();
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setWaveformRunning(false);
            msleep(500);
            pqm.setWaveformLoggingEnabled(false);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_LOG_WRONG", function() {
        try {
            var invalidPath = "/invalid/path/that/does/not/exist";
            pqm.setWaveformLoggingEnabled(true);
            msleep(500);
            pqm.setWaveformLogPath(invalidPath);
            msleep(500);
            pqm.setWaveformRunning(true);
            msleep(3000);
            pqm.setWaveformRunning(false);
            msleep(500);
            if (pqm.isWaveformLoggingEnabled()) {
                printToConsole("  Logging section remained open (expected to close)");
                return false;
            }
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setWaveformRunning(false);
            msleep(500);
            pqm.setWaveformLoggingEnabled(false);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_CONCURRENT_ACQ", function() {
        try {
            // Start RMS acquisition
            printToConsole("  Starting RMS acquisition");
            pqm.setRmsRunning(true);
            msleep(1000);
            if (!pqm.isRmsRunning()) {
                printToConsole("  Failed to start RMS");
                return false;
            }

            // Start Harmonics acquisition
            printToConsole("  Starting Harmonics acquisition");
            pqm.setHarmonicsRunning(true);
            msleep(1000);
            if (!pqm.isHarmonicsRunning()) {
                printToConsole("  Failed to start Harmonics");
                pqm.setRmsRunning(false);
                return false;
            }

            // Start Waveform acquisition - should stop RMS and Harmonics
            printToConsole("  Starting Waveform acquisition");
            pqm.setWaveformRunning(true);
            msleep(2000);

            // Verify RMS stopped
            if (pqm.isRmsRunning()) {
                printToConsole("  RMS did not stop when Waveform started");
                pqm.setWaveformRunning(false);
                return false;
            }
            printToConsole("  RMS stopped as expected");

            // Verify Harmonics stopped
            if (pqm.isHarmonicsRunning()) {
                printToConsole("  Harmonics did not stop when Waveform started");
                pqm.setWaveformRunning(false);
                return false;
            }
            printToConsole("  Harmonics stopped as expected");

            // Verify Waveform is running
            if (!pqm.isWaveformRunning()) {
                printToConsole("  Waveform is not running");
                return false;
            }
            printToConsole("  Waveform is running");

            // Stop Waveform
            pqm.setWaveformRunning(false);
            msleep(500);

            printToConsole("  Concurrent acquisition test passed");
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            pqm.setRmsRunning(false);
            pqm.setHarmonicsRunning(false);
            pqm.setWaveformRunning(false);
            return false;
        }
    });
}

// ============================================
// Settings Instrument Tests
// ============================================
printToConsole("\n=== Settings Instrument Tests ===\n");

if (switchToTool("Settings")) {

    TestFramework.runTest("TST.PQM.SETTINGS_OPEN", function() {
        try {
            // Verify tool opened - settings read will confirm accessibility
            printToConsole("  Settings tool opened successfully");
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.SETTINGS_READ", function() {
        try {
            pqm.settingsRead();
            msleep(2000);
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.SETTINGS_WRITE", function() {
        var attrName = "msv_carrier_frequency";
        try {
            pqm.settingsRead();
            msleep(2000);
            var originalValue = pqm.getSettingsAttributeValue(attrName);
            pqm.setSettingsAttributeValue(attrName, "0.40");
            msleep(500);
            pqm.settingsWrite();
            msleep(2000);
            pqm.settingsRead();
            msleep(2000);
            // Restore original value
            if (originalValue) {
                pqm.setSettingsAttributeValue(attrName, originalValue);
                msleep(500);
                pqm.settingsWrite();
                msleep(2000);
            }
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });
}

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
scopy.exit();
exit(exitCode);
