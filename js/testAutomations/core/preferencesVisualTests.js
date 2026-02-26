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
// Source: docs/tests/general/general_preferences_tests.rst
//
// Tests in this file:
//   TST.PREF.ENABLE_MENU_ANIMATIONS - Menu animations toggle
//   TST.PREF.ENABLE_STATUS_BAR      - Status bar visibility
//   TST.PREF.SHOW_GRID              - Grid display on plots
//   TST.PREF.SHOW_GRATICULE         - Graticule display on plots
//   TST.PREF.SHOW_PLOT_FPS          - Plot FPS overlay visibility
//   TST.PREF.PLOT_TARGET_FPS        - Plot target FPS capping
// ============================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("General Preferences Visual Validation Tests");

// Helper: get a preference value from the preferences map
function getPreferenceValue(prefName) {
    var prefs = scopy.getPreferences();
    return prefs[prefName];
}

// Connect to device
if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Determine which tool to use for plot-related tests
var tools = scopy.getTools();
var plotTool = null;
for (var i = 0; i < tools.length; i++) {
    if (tools[i] === "ADC - Time" || tools[i] === "Oscilloscope") {
        plotTool = tools[i];
        break;
    }
}

if (!plotTool) {
    printToConsole("ERROR: No plot-capable tool found (need ADC - Time or Oscilloscope)");
    printToConsole("  Available tools: " + tools.join(", "));
    TestFramework.disconnectFromDevice();
    scopy.exit();
}
printToConsole("  Using plot tool: " + plotTool);

// ============================================
// Test 6: Enable menu animations
// UID: TST.PREF.ENABLE_MENU_ANIMATIONS
// Description: Toggle visual animations for menu transitions.
// Preconditions: M2k.emu or any connected device
// ============================================
printToConsole("\n=== Test 6: Enable menu animations (SUPERVISED) ===\n");

TestFramework.runTest("TST.PREF.ENABLE_MENU_ANIMATIONS", function() {
    try {
        // Save original preference
        var originalValue = getPreferenceValue("general_use_animations");
        printToConsole("  Original value: " + originalValue);

        // Step 1: Set preference to enabled
        scopy.setPreference("general_use_animations", true);
        msleep(500);

        // Step 2: Switch to tool and let user toggle menus
        switchToTool(plotTool);

        if (!TestFramework.supervisedCheck(
            "Animations ENABLED: Toggle between menus in the " + plotTool + " tool.\n" +
            "  Verify: Opening menus should be animated (smooth transitions).")) {
            scopy.setPreference("general_use_animations", originalValue);
            return false;
        }

        // Step 3: Disable animations
        scopy.setPreference("general_use_animations", false);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Animations DISABLED: Toggle between menus in the " + plotTool + " tool.\n" +
            "  Verify: Opening menus should NOT have any animations (instant switch).")) {
            scopy.setPreference("general_use_animations", originalValue);
            return false;
        }

        // Restore original preference
        scopy.setPreference("general_use_animations", originalValue);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 7: Enable the status bar
// UID: TST.PREF.ENABLE_STATUS_BAR
// Description: Show status messages on the bottom of the app window.
// Preconditions: M2k.emu or any connected device
// ============================================
printToConsole("\n=== Test 7: Enable the status bar (SUPERVISED) ===\n");

TestFramework.runTest("TST.PREF.ENABLE_STATUS_BAR", function() {
    try {
        // Save original preference
        var originalValue = getPreferenceValue("general_show_status_bar");
        printToConsole("  Original value: " + originalValue);

        // Step 1: Set preference to enabled
        scopy.setPreference("general_show_status_bar", true);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Status bar ENABLED: Look at the bottom of the Scopy window.\n" +
            "  Verify: A status bar message area should be visible at the\n" +
            "  bottom of the application window.")) {
            scopy.setPreference("general_show_status_bar", originalValue);
            return false;
        }

        // Step 3: Disable status bar
        scopy.setPreference("general_show_status_bar", false);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Status bar DISABLED: Look at the bottom of the Scopy window.\n" +
            "  Verify: No status bar should be visible.")) {
            scopy.setPreference("general_show_status_bar", originalValue);
            return false;
        }

        // Restore original preference
        scopy.setPreference("general_show_status_bar", originalValue);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 8: Show Grid
// UID: TST.PREF.SHOW_GRID
// Description: Display grid lines for better visual referencing on plots.
// Preconditions: Pluto.emu or any device with ADC - Time tool
// ============================================
printToConsole("\n=== Test 8: Show Grid (SUPERVISED) ===\n");

TestFramework.runTest("TST.PREF.SHOW_GRID", function() {
    try {
        // Save original preference
        var originalValue = getPreferenceValue("show_grid");
        printToConsole("  Original value: " + originalValue);

        // Switch to plot tool
        switchToTool(plotTool);

        // Step 1: Set preference to enabled
        scopy.setPreference("show_grid", true);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Grid ENABLED: Look at the " + plotTool + " plot.\n" +
            "  Verify: A grid should be visible on the plot.")) {
            scopy.setPreference("show_grid", originalValue);
            return false;
        }

        // Step 3: Disable grid
        scopy.setPreference("show_grid", false);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Grid DISABLED: Look at the " + plotTool + " plot.\n" +
            "  Verify: No grid should be visible on the plot.")) {
            scopy.setPreference("show_grid", originalValue);
            return false;
        }

        // Restore original preference
        scopy.setPreference("show_grid", originalValue);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 9: Show Graticule
// UID: TST.PREF.SHOW_GRATICULE
// Description: Display lines for axes centered on 0 points on plots.
// Preconditions: Pluto.emu or any device with ADC - Time tool
// ============================================
printToConsole("\n=== Test 9: Show Graticule (SUPERVISED) ===\n");

TestFramework.runTest("TST.PREF.SHOW_GRATICULE", function() {
    try {
        // Save original preference
        var originalValue = getPreferenceValue("show_graticule");
        printToConsole("  Original value: " + originalValue);

        // Switch to plot tool
        switchToTool(plotTool);

        // Step 1: Set preference to enabled
        scopy.setPreference("show_graticule", true);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Graticule ENABLED: Look at the " + plotTool + " plot.\n" +
            "  Verify: Horizontal and vertical measurement lines should be\n" +
            "  centered on the 0-axis points (both may not be visible at\n" +
            "  the same time).")) {
            scopy.setPreference("show_graticule", originalValue);
            return false;
        }

        // Step 3: Disable graticule
        scopy.setPreference("show_graticule", false);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Graticule DISABLED: Look at the " + plotTool + " plot.\n" +
            "  Verify: The lines should border the entire plot instead of\n" +
            "  centering on 0 points.")) {
            scopy.setPreference("show_graticule", originalValue);
            return false;
        }

        // Restore original preference
        scopy.setPreference("show_graticule", originalValue);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 19: Show plot FPS
// UID: TST.PREF.SHOW_PLOT_FPS
// Description: Show FPS on plots.
// Preconditions: M2k.emu or any device with plot tool
// ============================================
printToConsole("\n=== Test 19: Show plot FPS (SUPERVISED) ===\n");

TestFramework.runTest("TST.PREF.SHOW_PLOT_FPS", function() {
    try {
        // Save original preference
        var originalValue = getPreferenceValue("general_show_plot_fps");
        printToConsole("  Original value: " + originalValue);

        // Switch to plot tool and start acquisition
        switchToTool(plotTool);
        scopy.runTool(plotTool, true);
        msleep(1000);

        // Step 1: Set preference to enabled
        scopy.setPreference("general_show_plot_fps", true);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Show FPS ENABLED: Look at the " + plotTool + " plot while running.\n" +
            "  Verify: FPS text should be visible on the plot\n" +
            "  (typically red text showing frame rate information).")) {
            scopy.runTool(plotTool, false);
            scopy.setPreference("general_show_plot_fps", originalValue);
            return false;
        }

        // Step 4: Disable show FPS
        scopy.setPreference("general_show_plot_fps", false);
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Show FPS DISABLED: Look at the " + plotTool + " plot while running.\n" +
            "  Verify: No FPS text should be visible on the plot.")) {
            scopy.runTool(plotTool, false);
            scopy.setPreference("general_show_plot_fps", originalValue);
            return false;
        }

        // Stop acquisition and restore preference
        scopy.runTool(plotTool, false);
        msleep(500);
        scopy.setPreference("general_show_plot_fps", originalValue);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try { scopy.runTool(plotTool, false); } catch (e2) {}
        return false;
    }
});

// ============================================
// Test 20: Plot target FPS
// UID: TST.PREF.PLOT_TARGET_FPS
// Description: Manage the maximum FPS plots should run at.
// Preconditions: Pluto.emu or any device, "Show plot FPS" must be enabled
// ============================================
printToConsole("\n=== Test 20: Plot target FPS (SUPERVISED) ===\n");

TestFramework.runTest("TST.PREF.PLOT_TARGET_FPS", function() {
    try {
        // Save original preferences
        var originalFps = getPreferenceValue("general_plot_target_fps");
        var originalShowFps = getPreferenceValue("general_show_plot_fps");
        printToConsole("  Original target FPS: " + originalFps);
        printToConsole("  Original show FPS: " + originalShowFps);

        // Ensure show FPS is enabled for verification
        scopy.setPreference("general_show_plot_fps", true);
        msleep(500);

        // Switch to plot tool and start acquisition
        switchToTool(plotTool);
        scopy.runTool(plotTool, true);
        msleep(1000);

        // Test with a low FPS value (e.g., 10)
        scopy.setPreference("general_plot_target_fps", 10);
        msleep(2000);

        if (!TestFramework.supervisedCheck(
            "Target FPS set to 10: Look at the FPS overlay on the " + plotTool + " plot.\n" +
            "  Verify: The displayed FPS should be approximately 10 or below.")) {
            scopy.runTool(plotTool, false);
            scopy.setPreference("general_plot_target_fps", originalFps);
            scopy.setPreference("general_show_plot_fps", originalShowFps);
            return false;
        }

        // Test with a higher FPS value (e.g., 60)
        scopy.setPreference("general_plot_target_fps", 60);
        msleep(2000);

        if (!TestFramework.supervisedCheck(
            "Target FPS set to 60: Look at the FPS overlay on the " + plotTool + " plot.\n" +
            "  Verify: The displayed FPS should be higher than before,\n" +
            "  approximately 60 or below.")) {
            scopy.runTool(plotTool, false);
            scopy.setPreference("general_plot_target_fps", originalFps);
            scopy.setPreference("general_show_plot_fps", originalShowFps);
            return false;
        }

        // Stop acquisition and restore preferences
        scopy.runTool(plotTool, false);
        msleep(500);
        scopy.setPreference("general_plot_target_fps", originalFps);
        msleep(500);
        scopy.setPreference("general_show_plot_fps", originalShowFps);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try { scopy.runTool(plotTool, false); } catch (e2) {}
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
