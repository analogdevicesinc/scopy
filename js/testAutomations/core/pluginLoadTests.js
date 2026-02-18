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

// Plugin Load Detection and Switching Automated Tests
//
// This test suite connects to both physical device and emulator and verifies
// that plugins load and can be switched to without errors. For each device, it:
// 1. Connects to the device once
// 2. Gets the list of loaded tools/plugins
// 3. Prints "PLUGIN_NAME load test PASS" for each loaded plugin
// 4. Tests switching to each plugin using scopy.switchTool()
// 5. Prints "PLUGIN_NAME switch test PASS/FAIL" for each switch attempt
// 6. Disconnects and cleans up

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: Plugin Load Tests
TestFramework.init("Plugin Load Detection Tests");


// Helper: remove all devices
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

// Helper: connect to a device and test all loaded plugins
function testAllPluginsOnDevice(testUri, deviceLabel) {
    printToConsole("\n--- Testing plugins on " + deviceLabel + " (" + testUri + ") ---");
    cleanupAllDevices();

    try {
        // Phase 1: Connect and detect loaded plugins
        var deviceId = scopy.addDevice(testUri);
        if (!deviceId) {
            printToConsole("FAIL: Could not add device " + deviceLabel);
            return;
        }
        msleep(500);

        if (!scopy.connectDevice(deviceId)) {
            printToConsole("FAIL: Could not connect to " + deviceLabel);
            scopy.removeDevice(testUri);
            return;
        }
        msleep(2000);

        // Get all tools/plugins that loaded after connection
        var loadedTools = scopy.getTools();

        if (!loadedTools || loadedTools.length === 0) {
            printToConsole("No plugins loaded on " + deviceLabel);
            scopy.disconnectDevice(deviceId);
            scopy.removeDevice(testUri);
            msleep(500);
            return;
        }

        printToConsole("Loaded plugins: " + loadedTools.join(", "));
        printToConsole("");

        // Print PASS for each loaded plugin
        for (var i = 0; i < loadedTools.length; i++) {
            printToConsole(loadedTools[i] + " load test PASS");
        }

        // Phase 2: Test switching to each plugin
        printToConsole("\n--- Testing plugin switching on " + deviceLabel + " ---");

        for (var i = 0; i < loadedTools.length; i++) {
            var toolName = loadedTools[i];
            var switched = scopy.switchTool(toolName);
            msleep(2000);  // Allow tool to initialize

            if (!switched) {
                printToConsole(toolName + " switch test FAIL");
            } else {
                printToConsole(toolName + " switch test PASS");
            }
        }

        // Disconnect and cleanup
        scopy.disconnectDevice(deviceId);
        scopy.removeDevice(testUri);
        msleep(500);
    } catch (e) {
        printToConsole("Error testing " + deviceLabel + ": " + e);
        cleanupAllDevices();
    }
}

var emulatorUri = "ip:127.0.0.1";
var deviceUri = "ip:192.168.2.1";

// ============================================
// Plugin Load Tests
// ============================================
printToConsole("\n=== Plugin Load Detection Tests ===\n");

// Test all plugins on physical device
testAllPluginsOnDevice(deviceUri, "Physical Device");

// Test all plugins on emulator
testAllPluginsOnDevice(emulatorUri, "Emulator");

// Final cleanup
cleanupAllDevices();

printToConsole("\n=== Plugin Load Tests Complete ===");
scopy.exit();
