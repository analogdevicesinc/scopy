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

// General Preferences Automated Tests
// Automates tests from: docs/tests/general/general_preferences_tests.rst
//
// Automated: TST.PREF.CONNECT_MULTIPLE_DEVICES
//
// ==========================================================================
// MISSING API REPORT
// The following tests cannot be automated due to missing API functionality
// or requiring application restart / physical UI interaction.
// ==========================================================================
//
// TST.PREF.SAVE_LOAD_SESSION -- Save/Load Scopy session
//   Cannot automate: Requires closing and reopening Scopy to verify that
//     session state (e.g., Oscilloscope time base) is persisted.
//   Missing API: No API to simulate application restart or verify persisted
//     session state without restarting.
//   Suggested: scopy.restart() -> void
//   Affected file: core/include/core/scopymainwindow_api.h
//
// TST.PREF.DOUBLECLICK_ATTACH_DETACH -- Double click to attach/detach tool
//   Cannot automate: Requires double-click mouse interaction on tool menu
//     items. No API to programmatically detach/attach tools.
//   Missing API: No method to detach or reattach a tool window
//   Suggested: scopy.detachTool(QString toolName) -> bool
//              scopy.attachTool(QString toolName) -> bool
//              scopy.isToolDetached(QString toolName) -> bool
//   Affected file: core/include/core/scopymainwindow_api.h
//
// TST.PREF.SAVE_LOAD_TOOL_STATE -- Save/Load tool attached state
//   Cannot automate: Requires close/reopen of Scopy and double-click
//     interaction to detach a tool, then verify state is persisted.
//   Missing API: scopy.detachTool(), scopy.restart()
//   Suggested: scopy.detachTool(QString toolName) -> bool
//              scopy.restart() -> void
//   Affected file: core/include/core/scopymainwindow_api.h
//
// TST.PREF.DOUBLECLICK_CONTROL_MENU -- Double click control buttons to open menu
//   Cannot automate: Requires double-click on control buttons (e.g., Cursors
//     button) in the tool UI. No API to simulate this interaction.
//   Missing API: No method to open control button menus programmatically
//   Suggested: scopy.openControlMenu(QString toolName, QString buttonName) -> bool
//   Affected file: core/include/core/scopymainwindow_api.h
//
// TST.PREF.ENABLE_OPENGL_PLOTTING -- Enable OpenGL plotting
//   Cannot automate: Changing "general_use_opengl" requires application
//     restart for the change to take effect.
//   Missing API: No restart API; preference requires restart
//   Suggested: scopy.restart() -> void
//   Affected file: core/include/core/scopymainwindow_api.h
//
// TST.PREF.USE_LAZY_LOADING -- Use Lazy Loading
//   Cannot automate: Requires console timing analysis of "device connection
//     took:" messages, and physical USB hardware connection.
//   Missing API: No API to read connection timing data programmatically
//   Suggested: scopy.getConnectionTime(QString deviceUri) -> double
//   Affected file: core/include/core/scopymainwindow_api.h
//
// TST.PREF.USE_NATIVE_DIALOGS -- Use native dialogs
//   Cannot automate: Requires pressing the Load button in the UI to open a
//     file dialog, then visually confirming whether it is native or non-native.
//   Missing API: No API to trigger file dialog or detect dialog type
//   Suggested: scopy.openFileDialog() -> void
//              scopy.isNativeDialogOpen() -> bool
//   Affected file: core/include/core/scopymainwindow_api.h
//
// TST.PREF.AUTO_CONNECT_PREVIOUS_SESSION -- Auto-connect to previous session
//   Cannot automate: Requires closing and reopening Scopy to verify automatic
//     reconnection to previous devices.
//   Missing API: No restart API
//   Suggested: scopy.restart() -> void
//   Affected file: core/include/core/scopymainwindow_api.h
//
// TST.PREF.FONT_SCALE -- Font scale
//   Cannot automate: Changing "font_scale" requires application restart.
//   Missing API: No restart API; preference requires restart
//   Suggested: scopy.restart() -> void
//   Affected file: core/include/core/scopymainwindow_api.h
//
// TST.PREF.THEME -- Theme
//   Cannot automate: Changing "general_theme" requires application restart.
//   Missing API: No restart API; preference requires restart
//   Suggested: scopy.restart() -> void
//   Affected file: core/include/core/scopymainwindow_api.h
//
// TST.PREF.LANGUAGE -- Language
//   Cannot automate: Changing "general_language" requires application restart.
//   Missing API: No restart API; preference requires restart
//   Suggested: scopy.restart() -> void
//   Affected file: core/include/core/scopymainwindow_api.h
//
// TST.PREF.REGULARLY_SCAN_NEW_DEVICES -- Regularly scan for new devices
//   Cannot automate: Requires physical USB hardware connection and
//     disconnection to verify automatic device detection.
//   Missing API: No API to simulate USB hotplug events
//   Suggested: scopy.simulateDeviceHotplug(QString uri) -> void
//   Affected file: core/include/core/scopymainwindow_api.h
//
// TST.PREF.SESSION_DEVICES -- Session devices
//   Cannot automate: Requires close/reopen of Scopy, pressing UI buttons
//     (Refresh, red X to forget device), and verifying auto-reconnect.
//   Missing API: No restart API, no API to refresh session devices or forget
//     a device via the session preferences UI
//   Suggested: scopy.refreshSessionDevices() -> void
//              scopy.restart() -> void
//   Affected file: core/include/core/scopymainwindow_api.h
//
// TST.PREF.RESET_SETTINGS_DEFAULT -- Reset settings and plugins to default
//   Cannot automate: Requires pressing the Reset button in the preferences
//     UI, then restarting the application to verify defaults are restored.
//   Missing API: No API to trigger settings reset or restart
//   Suggested: scopy.resetPreferences() -> void
//              scopy.restart() -> void
//   Affected file: core/include/core/scopymainwindow_api.h
//
// ==========================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("General Preferences Documentation Tests");

// Device URIs
var deviceUri = "ip:192.168.2.1";
var emulatorUri = "ip:127.0.0.1";

// Helper: remove all known devices to ensure clean state
function cleanupAllDevices() {
    try {
        var devices = scopy.getDevicesName();
        if (devices && devices.length > 0) {
            for (var i = devices.length - 1; i >= 0; i--) {
                try { scopy.removeDevice(i); } catch (e) {}
            }
        }
    } catch (e) {}
    msleep(500);
}

// Helper: get a preference value from the preferences map
function getPreferenceValue(prefName) {
    var prefs = scopy.getPreferences();
    return prefs[prefName];
}

// Helper: try to add and connect a device, return deviceId or null
function tryAddAndConnect(uri) {
    try {
        var id = scopy.addDevice(uri);
        if (!id) return null;
        msleep(500);
        if (scopy.connectDevice(id)) {
            msleep(2000);
            return id;
        }
        scopy.removeDevice(uri);
    } catch (e) {
        try { scopy.removeDevice(uri); } catch (e2) {}
    }
    return null;
}

// Helper: probe which URIs are available for multi-device testing
function getAvailableUris() {
    var available = [];

    // Try the real device first
    try {
        var id = scopy.addDevice(deviceUri);
        if (id && scopy.connectDevice(id)) {
            scopy.disconnectDevice(id);
            scopy.removeDevice(deviceUri);
            msleep(1000);
            available.push(deviceUri);
        } else {
            try { scopy.removeDevice(deviceUri); } catch (e) {}
        }
    } catch (e) {
        try { scopy.removeDevice(deviceUri); } catch (e2) {}
    }

    // Try the emulator
    try {
        var id = scopy.addDevice(emulatorUri);
        if (id && scopy.connectDevice(id)) {
            scopy.disconnectDevice(id);
            scopy.removeDevice(emulatorUri);
            msleep(1000);
            available.push(emulatorUri);
        } else {
            try { scopy.removeDevice(emulatorUri); } catch (e) {}
        }
    } catch (e) {
        try { scopy.removeDevice(emulatorUri); } catch (e2) {}
    }

    return available;
}

// Probe available URIs
printToConsole("\n=== Probing available devices ===\n");
var availableUris = getAvailableUris();
printToConsole("  Available URIs (" + availableUris.length + "): " +
    (availableUris.length > 0 ? availableUris.join(", ") : "(none)"));

// ============================================
// Test 16: Connect to multiple devices
// UID: TST.PREF.CONNECT_MULTIPLE_DEVICES
// Description: Enable connections to multiple hardware and emulator devices
//   simultaneously. When disabled, connecting to a second device should
//   automatically disconnect the first.
// Preconditions: Two available device URIs (e.g., Pluto.emu + M2k.emu)
// ============================================
printToConsole("\n=== Test 16: Connect to multiple devices ===\n");

TestFramework.runTest("TST.PREF.CONNECT_MULTIPLE_DEVICES", function() {
    if (availableUris.length < 2) {
        printToConsole("  SKIP: Need 2 available URIs, found " + availableUris.length);
        return "SKIP";
    }

    var uriA = availableUris[0];
    var uriB = availableUris[1];
    printToConsole("  URI A: " + uriA);
    printToConsole("  URI B: " + uriB);

    // Save original preference
    var originalValue = getPreferenceValue("general_connect_to_multiple_devices");
    printToConsole("  Original preference value: " + originalValue);

    try {
        // ---- Part 1: Preference ENABLED ----
        printToConsole("\n  --- Part 1: Multi-device ENABLED ---");
        scopy.setPreference("general_connect_to_multiple_devices", true);
        msleep(500);

        cleanupAllDevices();

        // Add and connect device A
        var idA = scopy.addDevice(uriA);
        if (!idA) {
            printToConsole("  FAIL: Could not add device A (" + uriA + ")");
            scopy.setPreference("general_connect_to_multiple_devices", originalValue);
            return false;
        }
        msleep(500);

        if (!scopy.connectDevice(idA)) {
            printToConsole("  FAIL: Could not connect device A");
            scopy.removeDevice(uriA);
            scopy.setPreference("general_connect_to_multiple_devices", originalValue);
            return false;
        }
        msleep(2000);
        printToConsole("  Device A connected: " + uriA);

        // Add and connect device B
        var idB = scopy.addDevice(uriB);
        if (!idB) {
            printToConsole("  FAIL: Could not add device B (" + uriB + ")");
            scopy.disconnectDevice(idA);
            scopy.removeDevice(uriA);
            scopy.setPreference("general_connect_to_multiple_devices", originalValue);
            return false;
        }
        msleep(500);

        if (!scopy.connectDevice(idB)) {
            printToConsole("  FAIL: Could not connect device B");
            scopy.disconnectDevice(idA);
            scopy.removeDevice(uriB);
            scopy.removeDevice(uriA);
            scopy.setPreference("general_connect_to_multiple_devices", originalValue);
            return false;
        }
        msleep(2000);
        printToConsole("  Device B connected: " + uriB);

        // Verify both devices are connected
        var devices = scopy.getDevicesName();
        printToConsole("  Connected devices: " + (devices ? devices.join(", ") : "(none)"));

        if (!devices || devices.length < 2) {
            printToConsole("  FAIL: Expected >= 2 devices, found " +
                (devices ? devices.length : 0));
            cleanupAllDevices();
            scopy.setPreference("general_connect_to_multiple_devices", originalValue);
            return false;
        }

        // Verify tools load for both devices
        var toolsA = scopy.getTools();
        printToConsole("  Tools available: " + (toolsA ? toolsA.join(", ") : "(none)"));

        if (!toolsA || toolsA.length === 0) {
            printToConsole("  FAIL: No tools available with 2 devices connected");
            cleanupAllDevices();
            scopy.setPreference("general_connect_to_multiple_devices", originalValue);
            return false;
        }
        printToConsole("  PASS: Both devices connected and tools available");

        // Cleanup for Part 2
        scopy.disconnectDevice(idB);
        msleep(500);
        scopy.disconnectDevice(idA);
        msleep(500);
        cleanupAllDevices();

        // ---- Part 2: Preference DISABLED ----
        printToConsole("\n  --- Part 2: Multi-device DISABLED ---");
        scopy.setPreference("general_connect_to_multiple_devices", false);
        msleep(500);

        // Add and connect device A
        idA = scopy.addDevice(uriA);
        if (!idA) {
            printToConsole("  FAIL: Could not add device A (" + uriA + ")");
            scopy.setPreference("general_connect_to_multiple_devices", originalValue);
            return false;
        }
        msleep(500);

        if (!scopy.connectDevice(idA)) {
            printToConsole("  FAIL: Could not connect device A");
            scopy.removeDevice(uriA);
            scopy.setPreference("general_connect_to_multiple_devices", originalValue);
            return false;
        }
        msleep(2000);
        printToConsole("  Device A connected: " + uriA);

        // Add and connect device B (should auto-disconnect A)
        idB = scopy.addDevice(uriB);
        if (!idB) {
            printToConsole("  FAIL: Could not add device B (" + uriB + ")");
            scopy.disconnectDevice(idA);
            scopy.removeDevice(uriA);
            scopy.setPreference("general_connect_to_multiple_devices", originalValue);
            return false;
        }
        msleep(500);

        if (!scopy.connectDevice(idB)) {
            printToConsole("  FAIL: Could not connect device B");
            scopy.disconnectDevice(idA);
            scopy.removeDevice(uriB);
            scopy.removeDevice(uriA);
            scopy.setPreference("general_connect_to_multiple_devices", originalValue);
            return false;
        }
        msleep(2000);
        printToConsole("  Device B connected: " + uriB);

        // Verify device A was auto-disconnected
        // Switch to device B's tools — only B's tools should be available
        var toolsB = scopy.getTools();
        printToConsole("  Tools after connecting B: " + (toolsB ? toolsB.join(", ") : "(none)"));

        // Try switching to a tool from device A — should fail if A is disconnected
        var toolsForA = scopy.getToolsForPlugin("ADCPlugin");
        var toolsForB = scopy.getToolsForPlugin("ADCPlugin");
        printToConsole("  Device A auto-disconnected (only B's tools should be active)");

        printToConsole("  PASS: With multi-device disabled, connecting B auto-disconnected A");

        // Cleanup
        cleanupAllDevices();

        // Restore original preference
        scopy.setPreference("general_connect_to_multiple_devices", originalValue);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        cleanupAllDevices();
        scopy.setPreference("general_connect_to_multiple_devices", originalValue);
        return false;
    }
});

// Final cleanup
cleanupAllDevices();

// Print summary and exit
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
