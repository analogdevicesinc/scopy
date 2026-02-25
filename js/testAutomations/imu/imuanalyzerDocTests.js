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

// IMU Analyzer Manual Test Automation - Documentation Tests
// Automates tests from: docs/tests/plugins/imuanalyzer/imuanalyzer_tests.rst
//
// Automated: TST.IMUANALYZER.PLUGIN_DETECTION (steps 1-2),
//            TST.IMUANALYZER.MEASUREMENT_MODE (API verification)
//
// ==========================================================================
// MISSING API REPORT
// The following tests cannot be automated due to missing API functionality.
// ==========================================================================
//
// TST.IMUANALYZER.VIEW_CONTROLS — Interactive 3D View Controls
//   Cannot automate: Steps 1 and 3 require mouse drag and scroll wheel
//     interaction in the 3D view. No API exists for simulating mouse events.
//   Missing API: Mouse/view simulation and camera state query methods
//   Suggested: imuanalyzer.simulateMouseDrag(dx, dy) -> void
//              imuanalyzer.simulateScroll(delta) -> void
//              imuanalyzer.getViewAngle() -> QString
//              imuanalyzer.getZoomLevel() -> double
//   Affected file: packages/imu/plugins/imuanalyzer/include/imuanalyzer/imuanalyzer_api.h
//
// TST.IMUANALYZER.PLUGIN_DETECTION (step 3) — Incompatible Device Check
//   Cannot automate: Requires connecting to a second incompatible device
//     (e.g., ADALM-PLUTO) to verify plugin does NOT appear.
//   Missing API: N/A — test environment requirement (second device needed)
//
// TST.IMUANALYZER.3D_VISUALIZATION (step 3) — Physical IMU Rotation
//   Cannot automate: Requires physically rotating the IMU hardware to verify
//     real-time 3D orientation tracking.
//   Missing API: N/A — physical hardware manipulation required
//
// ==========================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("IMU Analyzer Documentation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// ============================================
// Test 1: Plugin Detection and ADIS Compatibility (steps 1-2)
// UID: TST.IMUANALYZER.PLUGIN_DETECTION
// Description: Verify that the IMU Analyzer plugin is detected
//   and available for compatible ADIS devices.
// ============================================
printToConsole("\n=== Test 1: Plugin Detection and ADIS Compatibility ===\n");

TestFramework.runTest("TST.IMUANALYZER.PLUGIN_DETECTION", function() {
    try {
        // Step 1: Verify plugin tool is in the tool list
        var tools = imuanalyzer.getTools();
        printToConsole("  Available tools: " + tools);
        var toolFound = false;
        for (var i = 0; i < tools.length; i++) {
            if (tools[i] === "IMUAnalyzer") {
                toolFound = true;
                break;
            }
        }
        if (!toolFound) {
            printToConsole("  FAIL: IMUAnalyzer not found in tool list");
            return false;
        }
        printToConsole("  Step 1 PASS: IMUAnalyzer found in tool list");

        // Step 2: Verify tool can be switched to (loads and is enabled)
        if (!switchToTool("IMUAnalyzer")) {
            printToConsole("  FAIL: Cannot switch to IMUAnalyzer tool");
            return false;
        }
        printToConsole("  Step 2 PASS: Successfully switched to IMUAnalyzer tool");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Switch to IMUAnalyzer tool for remaining tests
if (!switchToTool("IMUAnalyzer")) {
    printToConsole("ERROR: Cannot switch to IMUAnalyzer tool");
    scopy.exit();
}

// ============================================
// Test 4: Measurement Mode and Data Visualization (API verification)
// UID: TST.IMUANALYZER.MEASUREMENT_MODE
// Description: Verify that measurement mode toggle works via API
//   and sensor data values are readable when running.
// ============================================
printToConsole("\n=== Test 4: Measurement Mode - API Verification ===\n");

TestFramework.runTest("TST.IMUANALYZER.MEASUREMENT_MODE", function() {
    try {
        // Save original states
        var originalMeasure = imuanalyzer.isMeasureEnabled();
        var originalRunning = imuanalyzer.isRunning();
        printToConsole("  Original measure enabled: " + originalMeasure);
        printToConsole("  Original running: " + originalRunning);

        // Step 1: Enable measurement mode
        imuanalyzer.setMeasureEnabled(true);
        msleep(500);
        var measureEnabled = imuanalyzer.isMeasureEnabled();
        printToConsole("  After enable - measure enabled: " + measureEnabled);
        if (measureEnabled !== true) {
            printToConsole("  FAIL: Measurement mode was not enabled");
            imuanalyzer.setMeasureEnabled(originalMeasure);
            return false;
        }
        printToConsole("  Step 1 PASS: Measurement mode enabled");

        // Step 2: Start running to get sensor data
        imuanalyzer.setRunning(true);
        msleep(2000); // Allow time for data acquisition

        var running = imuanalyzer.isRunning();
        if (running !== true) {
            printToConsole("  FAIL: Data acquisition did not start");
            imuanalyzer.setMeasureEnabled(originalMeasure);
            return false;
        }
        printToConsole("  Step 2 PASS: Data acquisition is running");

        // Step 3: Read sensor data and verify values are returned
        var rotX = imuanalyzer.getRotationX();
        var rotY = imuanalyzer.getRotationY();
        var rotZ = imuanalyzer.getRotationZ();
        var accX = imuanalyzer.getAccelerationX();
        var accY = imuanalyzer.getAccelerationY();
        var accZ = imuanalyzer.getAccelerationZ();

        printToConsole("  Rotation  X=" + rotX + " Y=" + rotY + " Z=" + rotZ);
        printToConsole("  Acceleration X=" + accX + " Y=" + accY + " Z=" + accZ);

        // Verify sensor data returns are numbers (not undefined/null)
        if (typeof rotX !== "number" || typeof rotY !== "number" || typeof rotZ !== "number") {
            printToConsole("  FAIL: Rotation data is not numeric");
            imuanalyzer.setRunning(originalRunning);
            imuanalyzer.setMeasureEnabled(originalMeasure);
            return false;
        }
        if (typeof accX !== "number" || typeof accY !== "number" || typeof accZ !== "number") {
            printToConsole("  FAIL: Acceleration data is not numeric");
            imuanalyzer.setRunning(originalRunning);
            imuanalyzer.setMeasureEnabled(originalMeasure);
            return false;
        }
        printToConsole("  Step 3 PASS: Sensor data is readable and numeric");

        // Step 4: Toggle measurement mode off and verify
        imuanalyzer.setMeasureEnabled(false);
        msleep(500);
        var measureDisabled = imuanalyzer.isMeasureEnabled();
        if (measureDisabled !== false) {
            printToConsole("  FAIL: Measurement mode was not disabled");
            imuanalyzer.setRunning(originalRunning);
            imuanalyzer.setMeasureEnabled(originalMeasure);
            return false;
        }
        printToConsole("  Step 4 PASS: Measurement mode disabled");

        // Step 5: Re-enable and verify data continues
        imuanalyzer.setMeasureEnabled(true);
        msleep(500);
        var reEnabled = imuanalyzer.isMeasureEnabled();
        if (reEnabled !== true) {
            printToConsole("  FAIL: Measurement mode was not re-enabled");
            imuanalyzer.setRunning(originalRunning);
            imuanalyzer.setMeasureEnabled(originalMeasure);
            return false;
        }

        // Verify data is still updating after re-enable
        var rotX2 = imuanalyzer.getRotationX();
        if (typeof rotX2 !== "number") {
            printToConsole("  FAIL: Sensor data not available after re-enable");
            imuanalyzer.setRunning(originalRunning);
            imuanalyzer.setMeasureEnabled(originalMeasure);
            return false;
        }
        printToConsole("  Step 5 PASS: Measurement re-enabled, data continues");

        // Restore original states
        imuanalyzer.setRunning(originalRunning);
        msleep(500);
        imuanalyzer.setMeasureEnabled(originalMeasure);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        imuanalyzer.setRunning(false);
        imuanalyzer.setMeasureEnabled(false);
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
