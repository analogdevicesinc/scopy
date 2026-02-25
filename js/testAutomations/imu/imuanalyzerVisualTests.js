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

// ============================================================================
// ALL THE FOLLOWING TESTS REQUIRE SUPERVISED VISUAL VALIDATION
// These tests automate the steps from the manual test documentation but
// require a human observer to verify UI changes. After each visual check,
// the test will prompt the user to input 'y' (pass) or 'n' (fail).
// Source: docs/tests/plugins/imuanalyzer/imuanalyzer_tests.rst
// ============================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("IMU Analyzer Visual Validation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to IMUAnalyzer tool
if (!switchToTool("IMUAnalyzer")) {
    printToConsole("ERROR: Cannot switch to IMUAnalyzer tool");
    scopy.exit();
}

// ============================================
// Test 1: Plugin Detection and ADIS Compatibility (Visual)
// UID: TST.IMUANALYZER.PLUGIN_DETECTION
// Description: Verify that the IMU Analyzer plugin is visually
//   detected and enabled in the tool list for compatible ADIS devices.
// ============================================
printToConsole("\n=== Test 1: Plugin Detection (SUPERVISED) ===\n");

TestFramework.runTest("TST.IMUANALYZER.PLUGIN_DETECTION.VISUAL", function() {
    try {
        // Step 1: Verify plugin is in tool list (API check)
        var tools = imuanalyzer.getTools();
        printToConsole("  Available tools: " + tools);

        // Step 2: Supervised visual check — plugin visible and enabled in UI
        if (!TestFramework.supervisedCheck(
            "Verify that the IMUAnalyzer plugin is visible in the device " +
            "tool list on the left panel and is enabled (not grayed out).")) {
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2: 3D Visualization and Scene Rendering
// UID: TST.IMUANALYZER.3D_VISUALIZATION
// Description: Verify that 3D visualization engine works correctly
//   and displays IMU orientation in real-time.
// ============================================
printToConsole("\n=== Test 2: 3D Visualization and Scene Rendering (SUPERVISED) ===\n");

TestFramework.runTest("TST.IMUANALYZER.3D_VISUALIZATION", function() {
    try {
        // Save original state
        var originalRunning = imuanalyzer.isRunning();

        // Step 1: Verify tool opens with tabbed interface and 3D view
        printToConsole("  Step 1: Verifying tool opens with 3D view...");

        if (!TestFramework.supervisedCheck(
            "Verify the IMU Analyzer tool has opened with a tabbed interface. " +
            "The '3D View' tab should be active and visible. " +
            "The 3D scene should be rendered without errors (no black screen).")) {
            imuanalyzer.setRunning(originalRunning);
            return false;
        }

        // Step 2: Start data acquisition
        printToConsole("  Step 2: Starting data acquisition...");
        imuanalyzer.setRunning(true);
        msleep(2000);

        var running = imuanalyzer.isRunning();
        printToConsole("  Running state: " + running);

        if (!TestFramework.supervisedCheck(
            "Verify the 3D object appears in the scene representing IMU orientation. " +
            "Real-time updates should begin with no rendering errors or black screen.")) {
            imuanalyzer.setRunning(originalRunning);
            return false;
        }

        // Step 3: Physical rotation check — skip with note
        printToConsole("  Step 3: Physical IMU rotation — requires hardware manipulation");
        printToConsole("  NOTE: Skipping physical rotation check. If hardware is available,");
        printToConsole("  manually rotate the IMU device and observe the 3D visualization.");

        if (!TestFramework.supervisedCheck(
            "If IMU hardware is available and can be rotated: verify the 3D " +
            "visualization updates in real-time to match the physical orientation, " +
            "with smooth animation, no lag, and clearly visible color-coded axes. " +
            "If hardware cannot be rotated, confirm the 3D view is stable and " +
            "responsive, then type 'y' to continue.")) {
            imuanalyzer.setRunning(originalRunning);
            return false;
        }

        // Restore original state
        imuanalyzer.setRunning(originalRunning);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        imuanalyzer.setRunning(false);
        return false;
    }
});

// ============================================
// Test 4: Measurement Mode and Data Visualization (Visual)
// UID: TST.IMUANALYZER.MEASUREMENT_MODE
// Description: Verify that measurement mode provides accurate sensor
//   data display with proper UI panel visibility and units.
// ============================================
printToConsole("\n=== Test 4: Measurement Mode - Visual Verification (SUPERVISED) ===\n");

TestFramework.runTest("TST.IMUANALYZER.MEASUREMENT_MODE.VISUAL", function() {
    try {
        // Save original states
        var originalMeasure = imuanalyzer.isMeasureEnabled();
        var originalRunning = imuanalyzer.isRunning();

        // Step 1: Enable measurement mode
        printToConsole("  Step 1: Enabling measurement mode...");
        imuanalyzer.setMeasureEnabled(true);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Verify the Measure button is in active/toggled state. " +
            "A data visualization panel should appear showing numerical " +
            "data displays with real-time updates.")) {
            imuanalyzer.setMeasureEnabled(originalMeasure);
            return false;
        }

        // Step 2: Start running and verify sensor data display
        printToConsole("  Step 2: Starting data acquisition to verify sensor display...");
        imuanalyzer.setRunning(true);
        msleep(2000);

        // Read and print sensor values for reference
        var rotX = imuanalyzer.getRotationX();
        var rotY = imuanalyzer.getRotationY();
        var rotZ = imuanalyzer.getRotationZ();
        var accX = imuanalyzer.getAccelerationX();
        var accY = imuanalyzer.getAccelerationY();
        var accZ = imuanalyzer.getAccelerationZ();
        printToConsole("  Rotation  X=" + rotX + " Y=" + rotY + " Z=" + rotZ);
        printToConsole("  Acceleration X=" + accX + " Y=" + accY + " Z=" + accZ);

        if (!TestFramework.supervisedCheck(
            "Verify the sensor data display shows: " +
            "rotation data (pitch, roll, yaw) with appropriate units, " +
            "position data calculated from acceleration integration, " +
            "and temperature reading showing current sensor temperature. " +
            "All values should update in real-time.")) {
            imuanalyzer.setRunning(originalRunning);
            imuanalyzer.setMeasureEnabled(originalMeasure);
            return false;
        }

        // Step 3: Toggle measurement mode off and on
        printToConsole("  Step 3: Toggling measurement mode off...");
        imuanalyzer.setMeasureEnabled(false);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Verify the measurement data panel has hidden/disappeared " +
            "after disabling measurement mode.")) {
            imuanalyzer.setRunning(originalRunning);
            imuanalyzer.setMeasureEnabled(originalMeasure);
            return false;
        }

        printToConsole("  Re-enabling measurement mode...");
        imuanalyzer.setMeasureEnabled(true);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Verify the measurement data panel has reappeared after " +
            "re-enabling measurement mode and data continues updating correctly.")) {
            imuanalyzer.setRunning(originalRunning);
            imuanalyzer.setMeasureEnabled(originalMeasure);
            return false;
        }

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
