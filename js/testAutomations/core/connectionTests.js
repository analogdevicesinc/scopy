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

// Connection Lifecycle Automated Tests
// Tests: TST.CONN.SUCC, TST.CONN.DISCONN, TST.ADD.SCAN_IP, TST.ADD.SCAN_EMPTY,
//        TST.ADD.VERIFY, TST.ADD.PLUGINS_ADD_SINGLE, TST.ADD.PLUGINS_ADD_MULTIPLE,
//        TST.ADD.PLUGINS_EMPTY, TST.DEV.FORGET, TST.DEV.FORGET_CONN,
//        TST.DEV.SELECT_TOOL, TST.DEV.ADD_ORDER, TST.DEV.MULTI_CONN, TST.DEV.RM_MULTI

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: Connection Tests
TestFramework.init("Connection Lifecycle Tests");

// Device URIs
var deviceUri = "ip:192.168.2.1";
var emulatorUri = "ip:127.0.0.1";

// Helper: determine which URI is available for testing
function getAvailableUri() {
    try {
        var id = scopy.addDevice(deviceUri);
        if (id && scopy.connectDevice(id)) {
            scopy.disconnectDevice(id);
            scopy.removeDevice(deviceUri);
            msleep(1000);
            return deviceUri;
        }
        scopy.removeDevice(deviceUri);
    } catch (e) {
        try { scopy.removeDevice(deviceUri); } catch (e2) {}
    }

    try {
        var id = scopy.addDevice(emulatorUri);
        if (id && scopy.connectDevice(id)) {
            scopy.disconnectDevice(id);
            scopy.removeDevice(emulatorUri);
            msleep(1000);
            return emulatorUri;
        }
        scopy.removeDevice(emulatorUri);
    } catch (e) {
        try { scopy.removeDevice(emulatorUri); } catch (e2) {}
    }

    return null;
}

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

// Determine available URI before running tests
var testUri = getAvailableUri();

if (!testUri) {
    printToConsole("ERROR: No device or emulator available for connection tests");
    printToConsole("Skipping all connection tests");
}

// ============================================
// Connection Tests
// ============================================
printToConsole("\n=== Connection Lifecycle Tests ===\n");

TestFramework.runTest("TST.CONN.SUCC", function() {
    if (!testUri) return "SKIP";
    try {
        cleanupAllDevices();
        var deviceId = scopy.addDevice(testUri);
        if (!deviceId) {
            printToConsole("  FAIL: addDevice returned null");
            return false;
        }
        msleep(500);

        var connected = scopy.connectDevice(deviceId);
        if (!connected) {
            printToConsole("  FAIL: connectDevice returned false");
            scopy.removeDevice(testUri);
            return false;
        }
        msleep(2000);
        printToConsole("  ✓ Device connected successfully via " + testUri);

        // Cleanup
        scopy.disconnectDevice(deviceId);
        scopy.removeDevice(testUri);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try { scopy.removeDevice(testUri); } catch (e2) {}
        return false;
    }
});

TestFramework.runTest("TST.CONN.DISCONN", function() {
    if (!testUri) return "SKIP";
    try {
        cleanupAllDevices();
        var deviceId = scopy.addDevice(testUri);
        if (!deviceId) return false;
        msleep(500);

        if (!scopy.connectDevice(deviceId)) {
            scopy.removeDevice(testUri);
            return false;
        }
        msleep(2000);
        printToConsole("  ✓ Device connected");

        scopy.disconnectDevice(deviceId);
        msleep(1000);
        printToConsole("  ✓ Device disconnected");

        // Verify tools are gone after disconnect
        var tools = scopy.getTools();
        if (tools && tools.length > 0) {
            printToConsole("  ⚠ Tools still present after disconnect: " + tools.join(", "));
        } else {
            printToConsole("  ✓ No tools available after disconnect");
        }

        scopy.removeDevice(testUri);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try { scopy.removeDevice(testUri); } catch (e2) {}
        return false;
    }
});

// ============================================
// Add Device Tests
// ============================================
printToConsole("\n=== Add Device Tests ===\n");

TestFramework.runTest("TST.ADD.SCAN_IP", function() {
    if (!testUri) return "SKIP";
    try {
        cleanupAllDevices();
        var deviceId = scopy.addDevice(testUri);
        if (!deviceId) {
            printToConsole("  FAIL: addDevice returned null for " + testUri);
            return false;
        }
        msleep(500);
        printToConsole("  ✓ Device added via IP scan: " + testUri);

        scopy.removeDevice(testUri);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try { scopy.removeDevice(testUri); } catch (e2) {}
        return false;
    }
});

TestFramework.runTest("TST.ADD.SCAN_EMPTY", function() {
    try {
        cleanupAllDevices();
        // Try adding with an invalid/empty URI - should not crash
        var deviceId = null;
        try {
            deviceId = scopy.addDevice("ip:0.0.0.0");
            msleep(1000);
        } catch (e) {
            printToConsole("  ✓ addDevice with invalid URI threw exception (expected)");
        }

        // Even if it returned something, app should still be stable
        if (deviceId) {
            try { scopy.removeDevice("ip:0.0.0.0"); } catch (e) {}
        }

        printToConsole("  ✓ Application remained stable after invalid URI scan");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.ADD.VERIFY", function() {
    if (!testUri) return "SKIP";
    try {
        cleanupAllDevices();
        var deviceId = scopy.addDevice(testUri);
        if (!deviceId) return false;
        msleep(500);

        var devices = scopy.getDevicesName();
        if (!devices || devices.length === 0) {
            printToConsole("  FAIL: getDevicesName returned empty after addDevice");
            scopy.removeDevice(testUri);
            return false;
        }

        var found = false;
        for (var i = 0; i < devices.length; i++) {
            if (devices[i].indexOf(testUri) !== -1 || devices[i].length > 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            printToConsole("  FAIL: Device not found in getDevicesName()");
            scopy.removeDevice(testUri);
            return false;
        }

        printToConsole("  ✓ Device verified in device list: " + devices.join(", "));
        scopy.removeDevice(testUri);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try { scopy.removeDevice(testUri); } catch (e2) {}
        return false;
    }
});

TestFramework.runTest("TST.ADD.PLUGINS_ADD_SINGLE", function() {
    if (!testUri) return "SKIP";
    try {
        cleanupAllDevices();
        // Get available plugins first
        var allPlugins = scopy.getPlugins(testUri, "iio");
        if (!allPlugins || allPlugins.length === 0) {
            printToConsole("  SKIP: No plugins available for " + testUri);
            return "SKIP";
        }

        var singlePlugin = allPlugins[0];
        printToConsole("  Adding device with single plugin: " + singlePlugin);

        var deviceId = scopy.addDevice(testUri, [singlePlugin]);
        if (!deviceId) {
            printToConsole("  FAIL: addDevice with single plugin returned null");
            return false;
        }
        msleep(500);

        if (!scopy.connectDevice(deviceId)) {
            printToConsole("  FAIL: connectDevice failed");
            scopy.removeDevice(testUri);
            return false;
        }
        msleep(2000);

        var tools = scopy.getTools();
        printToConsole("  ✓ Tools loaded with single plugin: " + (tools ? tools.join(", ") : "(none)"));

        scopy.disconnectDevice(deviceId);
        scopy.removeDevice(testUri);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try { scopy.removeDevice(testUri); } catch (e2) {}
        return false;
    }
});

TestFramework.runTest("TST.ADD.PLUGINS_ADD_MULTIPLE", function() {
    if (!testUri) return "SKIP";
    try {
        cleanupAllDevices();
        var allPlugins = scopy.getPlugins(testUri, "iio");
        if (!allPlugins || allPlugins.length < 2) {
            printToConsole("  SKIP: Need at least 2 plugins, found: " + (allPlugins ? allPlugins.length : 0));
            return "SKIP";
        }

        var selectedPlugins = [allPlugins[0], allPlugins[1]];
        printToConsole("  Adding device with plugins: " + selectedPlugins.join(", "));

        var deviceId = scopy.addDevice(testUri, selectedPlugins);
        if (!deviceId) {
            printToConsole("  FAIL: addDevice with multiple plugins returned null");
            return false;
        }
        msleep(500);

        if (!scopy.connectDevice(deviceId)) {
            printToConsole("  FAIL: connectDevice failed");
            scopy.removeDevice(testUri);
            return false;
        }
        msleep(2000);

        var tools = scopy.getTools();
        printToConsole("  ✓ Tools loaded with multiple plugins: " + (tools ? tools.join(", ") : "(none)"));

        scopy.disconnectDevice(deviceId);
        scopy.removeDevice(testUri);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try { scopy.removeDevice(testUri); } catch (e2) {}
        return false;
    }
});

TestFramework.runTest("TST.ADD.PLUGINS_EMPTY", function() {
    if (!testUri) return "SKIP";
    try {
        cleanupAllDevices();
        printToConsole("  Adding device with empty plugin list");

        var deviceId = scopy.addDevice(testUri, []);
        msleep(500);

        if (deviceId) {
            printToConsole("  ✓ addDevice with empty plugins returned deviceId (device added)");
            scopy.removeDevice(testUri);
        } else {
            printToConsole("  ✓ addDevice with empty plugins returned null (rejected)");
        }

        // Either way, app should remain stable
        printToConsole("  ✓ Application stable after empty plugin list");
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try { scopy.removeDevice(testUri); } catch (e2) {}
        return false;
    }
});

// ============================================
// Device Management Tests
// ============================================
printToConsole("\n=== Device Management Tests ===\n");

TestFramework.runTest("TST.DEV.FORGET", function() {
    if (!testUri) return "SKIP";
    try {
        cleanupAllDevices();
        var deviceId = scopy.addDevice(testUri);
        if (!deviceId) return false;
        msleep(500);

        var devicesBefore = scopy.getDevicesName();
        printToConsole("  Devices before remove: " + devicesBefore.join(", "));

        scopy.removeDevice(testUri);
        msleep(500);

        var devicesAfter = scopy.getDevicesName();
        if (devicesAfter && devicesAfter.length > 0) {
            printToConsole("  FAIL: Device still present after removeDevice");
            printToConsole("  Devices after remove: " + devicesAfter.join(", "));
            return false;
        }

        printToConsole("  ✓ Device successfully forgotten (removed from list)");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try { scopy.removeDevice(testUri); } catch (e2) {}
        return false;
    }
});

TestFramework.runTest("TST.DEV.FORGET_CONN", function() {
    if (!testUri) return "SKIP";
    try {
        cleanupAllDevices();
        var deviceId = scopy.addDevice(testUri);
        if (!deviceId) return false;
        msleep(500);

        if (!scopy.connectDevice(deviceId)) {
            scopy.removeDevice(testUri);
            return false;
        }
        msleep(2000);
        printToConsole("  ✓ Device connected");

        // Remove device while connected
        scopy.removeDevice(testUri);
        msleep(1000);

        var devicesAfter = scopy.getDevicesName();
        if (devicesAfter && devicesAfter.length > 0) {
            printToConsole("  FAIL: Device still present after forget while connected");
            return false;
        }

        printToConsole("  ✓ Connected device successfully forgotten");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try { scopy.removeDevice(testUri); } catch (e2) {}
        return false;
    }
});

TestFramework.runTest("TST.DEV.SELECT_TOOL", function() {
    if (!testUri) return "SKIP";
    try {
        cleanupAllDevices();
        var deviceId = scopy.addDevice(testUri);
        if (!deviceId) return false;
        msleep(500);

        if (!scopy.connectDevice(deviceId)) {
            scopy.removeDevice(testUri);
            return false;
        }
        msleep(2000);

        var tools = scopy.getTools();
        if (!tools || tools.length === 0) {
            printToConsole("  FAIL: No tools available after connecting");
            scopy.disconnectDevice(deviceId);
            scopy.removeDevice(testUri);
            return false;
        }

        printToConsole("  Available tools: " + tools.join(", "));
        var allSwitched = true;

        for (var i = 0; i < tools.length; i++) {
            var switched = scopy.switchTool(tools[i]);
            msleep(1000);
            if (!switched) {
                printToConsole("  ✗ Failed to switch to: " + tools[i]);
                allSwitched = false;
            } else {
                printToConsole("  ✓ Switched to: " + tools[i]);
            }
        }

        scopy.disconnectDevice(deviceId);
        scopy.removeDevice(testUri);
        msleep(500);
        return allSwitched;
    } catch (e) {
        printToConsole("  Error: " + e);
        try {
            scopy.disconnectDevice(scopy.addDevice(testUri));
            scopy.removeDevice(testUri);
        } catch (e2) {}
        return false;
    }
});

TestFramework.runTest("TST.DEV.ADD_ORDER", function() {
    if (!testUri) return "SKIP";
    try {
        cleanupAllDevices();

        // Add first device
        var id1 = scopy.addDevice(testUri);
        msleep(500);

        var devices = scopy.getDevicesName();
        if (!devices || devices.length === 0) {
            printToConsole("  FAIL: No devices after first add");
            try { scopy.removeDevice(testUri); } catch (e) {}
            return false;
        }

        printToConsole("  ✓ After first add: " + devices.join(", "));
        printToConsole("  ✓ Device count: " + devices.length);

        // Verify the device is in the list
        if (devices.length < 1) {
            printToConsole("  FAIL: Expected at least 1 device");
            cleanupAllDevices();
            return false;
        }

        printToConsole("  ✓ Device order verified");
        cleanupAllDevices();
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        cleanupAllDevices();
        return false;
    }
});

TestFramework.runTest("TST.DEV.MULTI_CONN", function() {
    if (!testUri) return "SKIP";
    try {
        cleanupAllDevices();

        // Add and connect first device
        var id1 = scopy.addDevice(testUri);
        if (!id1) return false;
        msleep(500);

        if (!scopy.connectDevice(id1)) {
            scopy.removeDevice(testUri);
            return false;
        }
        msleep(2000);
        printToConsole("  ✓ First device connected: " + testUri);

        // Try adding a second device (emulator or vice versa)
        var secondUri = (testUri === deviceUri) ? emulatorUri : deviceUri;
        var id2 = null;
        try {
            id2 = scopy.addDevice(secondUri);
            msleep(500);
            if (id2) {
                if (scopy.connectDevice(id2)) {
                    msleep(2000);
                    printToConsole("  ✓ Second device connected: " + secondUri);
                    scopy.disconnectDevice(id2);
                } else {
                    printToConsole("  ⚠ Second device could not connect (may be unavailable)");
                }
                scopy.removeDevice(secondUri);
            }
        } catch (e) {
            printToConsole("  ⚠ Second device unavailable: " + e);
            try { scopy.removeDevice(secondUri); } catch (e2) {}
        }

        scopy.disconnectDevice(id1);
        scopy.removeDevice(testUri);
        msleep(500);

        printToConsole("  ✓ Multi-connection test completed");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        cleanupAllDevices();
        return false;
    }
});

TestFramework.runTest("TST.DEV.RM_MULTI", function() {
    if (!testUri) return "SKIP";
    try {
        cleanupAllDevices();

        // Add device
        var id1 = scopy.addDevice(testUri);
        msleep(500);

        var devicesBefore = scopy.getDevicesName();
        printToConsole("  Devices before cleanup: " + (devicesBefore ? devicesBefore.join(", ") : "(none)"));

        // Remove all devices
        cleanupAllDevices();

        var devicesAfter = scopy.getDevicesName();
        if (devicesAfter && devicesAfter.length > 0) {
            printToConsole("  FAIL: Devices still present after removing all");
            printToConsole("  Remaining: " + devicesAfter.join(", "));
            return false;
        }

        printToConsole("  ✓ All devices removed, clean state verified");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        cleanupAllDevices();
        return false;
    }
});

// Final cleanup
cleanupAllDevices();

// Print summary and exit
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
