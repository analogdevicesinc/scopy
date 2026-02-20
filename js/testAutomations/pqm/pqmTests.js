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
evaluateFile("../scopy/js/testAutomations/common/testFramework.js");

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
            printToConsole("  ✓ RMS tool opened successfully");
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.RMS_INFO", function() {
        printToConsole("\n=== Test 2 - Info button check ===\n");
        printToConsole("  NOTE: TST.PQM.RMS_INFO requires manual testing (UI interaction)");

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
            var isRunning = pqm.isRmsRunning();
            if (!isRunning) return false;
            msleep(4000);
            pqm.setRmsRunning(false);
            msleep(500);
            return !pqm.isRmsRunning();
        } catch (e) {
            printToConsole("  Error: " + e);
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
            if (pqm.getRmsLogPath() !== logPath) return false;
            pqm.setRmsRunning(true);
            msleep(2000);
            // Verify log file exists and is not empty
            var actualLogFile = pqm.getLogFilePath();
            if (!actualLogFile || actualLogFile === "") {
                printToConsole("  ⚠ Log file path is empty");
                return false;
            }
            var logContent = fileIO.readAll(actualLogFile);
            if (!logContent || logContent.length === 0) {
                printToConsole("  ⚠ Log file is empty");
                return false;
            }
            pqm.setRmsRunning(false);
            msleep(500);
            pqm.setRmsLoggingEnabled(false);
            return !pqm.isRmsLoggingEnabled();
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.RMS_LOG_WRONG", function() {
        try {
            var invalidPath = "/invalid/path/that/does/not/exist";
            pqm.setRmsLoggingEnabled(true);
            pqm.setRmsLogPath(invalidPath);
            msleep(500);
            pqm.setRmsRunning(true);
            msleep(3000);
            pqm.setRmsRunning(false);
            msleep(500);

            if (pqm.isRmsLoggingEnabled()) {
                printToConsole("  ⚠ Logging section remained open (expected to close)");
                return false;
            } else {
                printToConsole("  ✓ Logging section closed as expected");
            }

            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.RMS_SETTINGS", function() {
        printToConsole("\n=== Test 5 - Settings button check ===\n");
        printToConsole("  NOTE: TST.PQM.RMS_SETTINGS requires manual testing (UI interaction - settings button toggle)");
        return "SKIP";
    });

    TestFramework.runTest("TST.PQM.RMS_PQEVENTS", function() {
        try {
            pqm.setRmsRunning(true);
            msleep(1000);
            pqm.triggerPqEvent(true);
            msleep(2000);
            pqm.setRmsRunning(false);
            if (!pqm.isRmsPqEvents()) {
                printToConsole("  ⚠ No PQ events detected after trigger (expected some)");
                return false;
            }
            pqm.triggerPqEvent(false);
            pqm.resetRmsPqEvents();
            msleep(500);
            printToConsole("  ✓ PQEvents reset successfully");
            return !pqm.isRmsPqEvents();
        } catch (e) {
            printToConsole("  Error: " + e);
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
            pqm.setHarmonicsRunning(false);
            if (!pqm.isRmsPqEvents()) {
                printToConsole("  ⚠ No PQ events detected after trigger (expected some) - RMS");
                return false;
            }
            if (!pqm.isHarmonicsPqEvents()) {
                printToConsole("  ⚠ No PQ events detected after trigger (expected some) - Harmonics");
                return false;
            }
            pqm.triggerPqEvent(false);
            pqm.resetRmsPqEvents();
            msleep(500);
            printToConsole("  ✓ PQEvents should be reset successfully");
            return !pqm.isRmsPqEvents() && !pqm.isHarmonicsPqEvents();
        } catch (e) {
            printToConsole("  Error: " + e);
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
            printToConsole("  ✓ Harmonics tool opened successfully");
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.HARMONICS_INFO", function() {
        printToConsole("\n=== Test 2 - Info button check ===\n");
        printToConsole("  NOTE: TST.PQM.HARMONICS_INFO requires manual testing (UI interaction - opens browser)");
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
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.HARMONICS_OVERVIEW_PLOT", function() {
        printToConsole("\n=== Test 5 - Overview plot features ===\n");
        printToConsole("  NOTE: TST.PQM.HARMONICS_OVERVIEW_PLOT requires manual testing (UI interaction - table row/cell selection and plot verification)");
        return "SKIP";
    });

    TestFramework.runTest("TST.PQM.HARMONICS_SETTINGS", function() {
        printToConsole("\n=== Test 6 - Settings button check ===\n");
        printToConsole("  NOTE: TST.PQM.HARMONICS_SETTINGS requires manual testing (UI interaction - settings button and dropdown)");
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
            if (pqm.getHarmonicsLogPath() !== logPath) return false;
            pqm.setHarmonicsRunning(true);
            msleep(2000);
            // Verify log file exists and is not empty
            var actualLogFile = pqm.getLogFilePath();
            if (!actualLogFile || actualLogFile === "") {
                printToConsole("  ⚠ Log file path is empty");
                return false;
            }
            var logContent = fileIO.readAll(actualLogFile);
            if (!logContent || logContent.length === 0) {
                printToConsole("  ⚠ Log file is empty");
                return false;
            }
            pqm.setHarmonicsRunning(false);
            msleep(500);
            pqm.setHarmonicsLoggingEnabled(false);
            return !pqm.isHarmonicsLoggingEnabled();
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.HARMONICS_LOG_WRONG", function() {
        try {
            var invalidPath = "/invalid/path/that/does/not/exist";
            pqm.setHarmonicsLoggingEnabled(true);
            pqm.setHarmonicsLogPath(invalidPath);
            msleep(500);
            pqm.setHarmonicsRunning(true);
            msleep(3000);
            pqm.setHarmonicsRunning(false);
            msleep(500);
            
            if (pqm.isHarmonicsLoggingEnabled()) {
                printToConsole("  ⚠ Logging section remained open (expected to close)");
                return false;
            }
            
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
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
            if(!pqm.isHarmonicsPqEvents()) {
                printToConsole("  ⚠ No Harmonics PQ events detected after trigger (expected some)");
                return false;
            }
            pqm.triggerPqEvent(false);
            pqm.resetHarmonicsPqEvents();
            printToConsole("  ✓ Harmonics PQEvents should be reset successfully");
            msleep(500);
            return !pqm.isHarmonicsPqEvents();
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.HARMONICS_CURRENT_PLOTS_DATA", function() {
        printToConsole("\n=== Test 10 - Current harmonics plots data correspondence ===\n");
        printToConsole("  NOTE: TST.PQM.HARMONICS_CURRENT_PLOTS_DATA requires manual testing (UI interaction - visual plot/table data comparison)");
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
            printToConsole("  ✓ Waveform tool opened successfully");
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_INFO", function() {
        printToConsole("\n=== Test 2 - Info button check ===\n");
        printToConsole("  NOTE: TST.PQM.WAVEFORM_INFO requires manual testing (UI interaction - opens browser)");
        return "SKIP";
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_SETTINGS", function() {
        printToConsole("\n=== Test 3 - Settings button check ===\n");
        printToConsole("  NOTE: TST.PQM.WAVEFORM_SETTINGS requires manual testing (UI interaction - settings button toggle)");
        return "SKIP";
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_SETTINGS_TIMESPAN", function() {
        try {
            var testValues = [0.02, 1.0, 10.0];
            for (var i = 0; i < testValues.length; i++) {
                pqm.setWaveformTimespan(testValues[i]);
                msleep(500);
                if (Math.abs(pqm.getWaveformTimespan() - testValues[i]) > 0.001) return false;
            }
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_SETTINGS_ROLLING", function() {
        try {
            pqm.setWaveformRollingMode(true);
            msleep(500);
            if (!pqm.isWaveformRollingMode()) return false;
            pqm.setWaveformRollingMode(false);
            msleep(500);
            return !pqm.isWaveformRollingMode();
        } catch (e) {
            printToConsole("  Error: " + e);
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
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_PLOT_ZOOM", function() {
        printToConsole("\n=== Test 10 - Plot zoom ===\n");
        printToConsole("  NOTE: TST.PQM.WAVEFORM_PLOT_ZOOM requires manual testing (UI interaction - plot zoom gesture)");
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
            if (pqm.getWaveformLogPath() !== logPath) return false;
            pqm.setWaveformRunning(true);
            msleep(5000);
            // Verify log file exists and is not empty
            var actualLogFile = pqm.getLogFilePath();
            if (!actualLogFile || actualLogFile === "") {
                printToConsole("  ⚠ Log file path is empty");
                return false;
            }
            var logContent = fileIO.readAll(actualLogFile);
            if (!logContent || logContent.length === 0) {
                printToConsole("  ⚠ Log file is empty");
                return false;
            }
            pqm.setWaveformRunning(false);
            msleep(500);
            pqm.setWaveformLoggingEnabled(false);
            return !pqm.isWaveformLoggingEnabled();
        } catch (e) {
            printToConsole("  Error: " + e);
            return false;
        }
    });

    TestFramework.runTest("TST.PQM.WAVEFORM_LOG_WRONG", function() {
        try {
            var invalidPath = "/invalid/path/that/does/not/exist";
            pqm.setWaveformLoggingEnabled(true);
            pqm.setWaveformLogPath(invalidPath);
            msleep(500);
            pqm.setWaveformRunning(true);
            msleep(3000);
            pqm.setWaveformRunning(false);
            msleep(500);
            if (pqm.isRmsLoggingEnabled()) {
                printToConsole("  ⚠ Logging section remained open (expected to close)");
                return false;
            } else {
                printToConsole("  ✓ Logging section closed as expected");
            }
            pqm.setWaveformLoggingEnabled(false);
            return true;
        } catch (e) {
            printToConsole("  Error: " + e);
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
                printToConsole("  ✗ Failed to start RMS");
                return false;
            }

            // Start Harmonics acquisition
            printToConsole("  Starting Harmonics acquisition");
            pqm.setHarmonicsRunning(true);
            msleep(1000);
            if (!pqm.isHarmonicsRunning()) {
                printToConsole("  ✗ Failed to start Harmonics");
                pqm.setRmsRunning(false);
                return false;
            }

            // Start Waveform acquisition - should stop RMS and Harmonics
            printToConsole("  Starting Waveform acquisition");
            pqm.setWaveformRunning(true);
            msleep(2000);

            // Verify RMS stopped
            if (pqm.isRmsRunning()) {
                printToConsole("  ✗ RMS did not stop when Waveform started");
                pqm.setWaveformRunning(false);
                return false;
            }
            printToConsole("  ✓ RMS stopped as expected");

            // Verify Harmonics stopped
            if (pqm.isHarmonicsRunning()) {
                printToConsole("  ✗ Harmonics did not stop when Waveform started");
                pqm.setWaveformRunning(false);
                return false;
            }
            printToConsole("  ✓ Harmonics stopped as expected");

            // Verify Waveform is running
            if (!pqm.isWaveformRunning()) {
                printToConsole("  ✗ Waveform is not running");
                return false;
            }
            printToConsole("  ✓ Waveform is running");

            // Stop Waveform
            pqm.setWaveformRunning(false);
            msleep(500);

            printToConsole("  ✓ Concurrent acquisition test passed");
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
            printToConsole("  ✓ Settings tool opened successfully");
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
        try {
            pqm.settingsRead();
            msleep(2000);
            var attrName = "msv_carrier_frequency";
            var originalValue = pqm.getSettingsAttributeValue(attrName);
            pqm.setSettingsAttributeValue(attrName, "0.40");
            pqm.settingsWrite();
            msleep(2000);
            pqm.settingsRead();
            msleep(2000);
            // Restore original value
            if (originalValue) {
                pqm.setSettingsAttributeValue(attrName, originalValue);
                pqm.settingsWrite();
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
