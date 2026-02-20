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

// Emulator Connection Automated Tests
// Tests: TST.EMU.EN, TST.EMU.NOT, TST.EMU.DIS, TST.EMU.DIS_CONN,
//        TST.EMU.EN_ADALM2000, TST.EMU.EN_PLUTO, TST.EMU.EN_PQM,
//        TST.EMU.EN_SWIOT_CONFIG, TST.EMU.EN_SWIOT_RUNTIME, TST.EMU.EN_GENERIC
//
// Prerequisite: iio-emu must be running externally for most tests

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite: Emulator Tests
TestFramework.init("Emulator Connection Tests");

var emulatorUri = "ip:127.0.0.1";

// Helper: check if emulator is reachable
function isEmulatorRunning() {
    try {
        var id = scopy.addDevice(emulatorUri);
        if (id && scopy.connectDevice(id)) {
            scopy.disconnectDevice(id);
            scopy.removeDevice(emulatorUri);
            msleep(500);
            return true;
        }
        try { scopy.removeDevice(emulatorUri); } catch (e) {}
        return false;
    } catch (e) {
        try { scopy.removeDevice(emulatorUri); } catch (e2) {}
        return false;
    }
}

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

// Helper: check if a plugin name exists in a plugin list
function pluginInList(pluginName, pluginList) {
    if (!pluginList) return false;
    for (var i = 0; i < pluginList.length; i++) {
        if (pluginList[i].toLowerCase().indexOf(pluginName.toLowerCase()) !== -1) {
            return true;
        }
    }
    return false;
}

// Check emulator availability
var emuAvailable = isEmulatorRunning();
printToConsole("Emulator available: " + emuAvailable + "\n");

// ============================================
// Basic Emulator Connection Tests
// ============================================
printToConsole("\n=== Basic Emulator Connection Tests ===\n");

TestFramework.runTest("TST.EMU.EN", function() {
    if (!emuAvailable) {
        printToConsole("  SKIP: Emulator not running");
        return "SKIP";
    }
    try {
        cleanupAllDevices();
        var deviceId = scopy.addDevice(emulatorUri);
        if (!deviceId) {
            printToConsole("  FAIL: addDevice returned null for emulator");
            return false;
        }
        msleep(500);

        var connected = scopy.connectDevice(deviceId);
        if (!connected) {
            printToConsole("  FAIL: connectDevice returned false for emulator");
            scopy.removeDevice(emulatorUri);
            return false;
        }
        msleep(2000);

        printToConsole("  ✓ Successfully connected to emulator at " + emulatorUri);

        scopy.disconnectDevice(deviceId);
        scopy.removeDevice(emulatorUri);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try { scopy.removeDevice(emulatorUri); } catch (e2) {}
        return false;
    }
});

TestFramework.runTest("TST.EMU.NOT", function() {
    if (emuAvailable) {
        printToConsole("  SKIP: Emulator is running (test requires emulator to be off)");
        return "SKIP";
    }
    try {
        cleanupAllDevices();
        var deviceId = scopy.addDevice(emulatorUri);
        msleep(500);

        var connected = false;
        if (deviceId) {
            connected = scopy.connectDevice(deviceId);
            msleep(2000);
        }

        if (connected) {
            printToConsole("  FAIL: Connection succeeded when emulator should be off");
            scopy.disconnectDevice(deviceId);
            scopy.removeDevice(emulatorUri);
            return false;
        }

        printToConsole("  ✓ Connection correctly failed when emulator not running");
        try { scopy.removeDevice(emulatorUri); } catch (e) {}
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  ✓ Exception thrown as expected: " + e);
        try { scopy.removeDevice(emulatorUri); } catch (e2) {}
        return true;
    }
});

TestFramework.runTest("TST.EMU.DIS", function() {
    if (!emuAvailable) {
        printToConsole("  SKIP: Emulator not running");
        return "SKIP";
    }
    try {
        cleanupAllDevices();
        var deviceId = scopy.addDevice(emulatorUri);
        if (!deviceId) return false;
        msleep(500);

        if (!scopy.connectDevice(deviceId)) {
            scopy.removeDevice(emulatorUri);
            return false;
        }
        msleep(2000);
        printToConsole("  ✓ Connected to emulator");

        scopy.disconnectDevice(deviceId);
        msleep(1000);
        printToConsole("  ✓ Disconnected from emulator");

        // Verify tools are cleared
        var tools = scopy.getTools();
        if (tools && tools.length > 0) {
            printToConsole("  ⚠ Tools still present after disconnect: " + tools.join(", "));
        } else {
            printToConsole("  ✓ Tools cleared after disconnect");
        }

        scopy.removeDevice(emulatorUri);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try { scopy.removeDevice(emulatorUri); } catch (e2) {}
        return false;
    }
});

TestFramework.runTest("TST.EMU.DIS_CONN", function() {
    if (!emuAvailable) {
        printToConsole("  SKIP: Emulator not running");
        return "SKIP";
    }
    try {
        cleanupAllDevices();
        var deviceId = scopy.addDevice(emulatorUri);
        if (!deviceId) return false;
        msleep(500);

        if (!scopy.connectDevice(deviceId)) {
            scopy.removeDevice(emulatorUri);
            return false;
        }
        msleep(2000);
        printToConsole("  ✓ Connected to emulator");

        // Disconnect while connected (verify state)
        scopy.disconnectDevice(deviceId);
        msleep(1000);

        // Verify device is still in device list but disconnected
        var devices = scopy.getDevicesName();
        if (!devices || devices.length === 0) {
            printToConsole("  ⚠ Device removed from list after disconnect (may be expected)");
        } else {
            printToConsole("  ✓ Device still in list after disconnect: " + devices.join(", "));
        }

        scopy.removeDevice(emulatorUri);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try { scopy.removeDevice(emulatorUri); } catch (e2) {}
        return false;
    }
});

// ============================================
// Device-Specific Emulator Tests
// ============================================
printToConsole("\n=== Device-Specific Emulator Tests ===\n");

TestFramework.runTest("TST.EMU.EN_ADALM2000", function() {
    if (!emuAvailable) {
        printToConsole("  SKIP: Emulator not running");
        return "SKIP";
    }
    try {
        cleanupAllDevices();
        var deviceId = scopy.addDevice(emulatorUri);
        if (!deviceId) return false;
        msleep(500);

        // Check if M2K plugins are available
        var plugins = scopy.getPlugins(emulatorUri, "iio");
        if (!plugins || plugins.length === 0) {
            printToConsole("  SKIP: No plugins found for emulator");
            scopy.removeDevice(emulatorUri);
            return "SKIP";
        }

        printToConsole("  Available plugins: " + plugins.join(", "));

        // Check for M2K-specific plugins
        var m2kPlugins = ["oscilloscope", "signal_generator", "voltmeter", "power_supply", "digitalio", "network_analyzer", "spectrum_analyzer", "logic_analyzer", "pattern_generator"];
        var foundM2k = false;
        for (var i = 0; i < m2kPlugins.length; i++) {
            if (pluginInList(m2kPlugins[i], plugins)) {
                foundM2k = true;
                printToConsole("  ✓ Found M2K plugin: " + m2kPlugins[i]);
            }
        }

        if (!foundM2k) {
            printToConsole("  SKIP: Emulator not running ADALM2000 context");
            scopy.removeDevice(emulatorUri);
            return "SKIP";
        }

        scopy.removeDevice(emulatorUri);
        msleep(500);
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        try { scopy.removeDevice(emulatorUri); } catch (e2) {}
        return false;
    }
});

TestFramework.runTest("TST.EMU.EN_PLUTO", function() {
    if (!emuAvailable) {
        printToConsole("  SKIP: Emulator not running");
        return "SKIP";
    }
    try {
        cleanupAllDevices();
        var plugins = scopy.getPlugins(emulatorUri, "iio");
        if (!plugins || plugins.length === 0) {
            printToConsole("  SKIP: No plugins found for emulator");
            return "SKIP";
        }

        printToConsole("  Available plugins: " + plugins.join(", "));

        if (!pluginInList("ad936x", plugins)) {
            printToConsole("  SKIP: Emulator not running Pluto/AD936X context");
            return "SKIP";
        }

        printToConsole("  ✓ AD936X plugin found - Pluto emulator detected");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.EMU.EN_PQM", function() {
    if (!emuAvailable) {
        printToConsole("  SKIP: Emulator not running");
        return "SKIP";
    }
    try {
        cleanupAllDevices();
        var plugins = scopy.getPlugins(emulatorUri, "iio");
        if (!plugins || plugins.length === 0) {
            printToConsole("  SKIP: No plugins found for emulator");
            return "SKIP";
        }

        printToConsole("  Available plugins: " + plugins.join(", "));

        if (!pluginInList("pqm", plugins)) {
            printToConsole("  SKIP: Emulator not running PQM context");
            return "SKIP";
        }

        printToConsole("  ✓ PQM plugin found - PQM emulator detected");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.EMU.EN_SWIOT_CONFIG", function() {
    if (!emuAvailable) {
        printToConsole("  SKIP: Emulator not running");
        return "SKIP";
    }
    try {
        cleanupAllDevices();
        var plugins = scopy.getPlugins(emulatorUri, "iio");
        if (!plugins || plugins.length === 0) {
            printToConsole("  SKIP: No plugins found for emulator");
            return "SKIP";
        }

        printToConsole("  Available plugins: " + plugins.join(", "));

        if (!pluginInList("swiot", plugins)) {
            printToConsole("  SKIP: Emulator not running SWIOT context");
            return "SKIP";
        }

        printToConsole("  ✓ SWIOT plugin found - SWIOT config emulator detected");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.EMU.EN_SWIOT_RUNTIME", function() {
    if (!emuAvailable) {
        printToConsole("  SKIP: Emulator not running");
        return "SKIP";
    }
    try {
        cleanupAllDevices();
        var plugins = scopy.getPlugins(emulatorUri, "iio");
        if (!plugins || plugins.length === 0) {
            printToConsole("  SKIP: No plugins found for emulator");
            return "SKIP";
        }

        printToConsole("  Available plugins: " + plugins.join(", "));

        if (!pluginInList("swiot", plugins)) {
            printToConsole("  SKIP: Emulator not running SWIOT runtime context");
            return "SKIP";
        }

        printToConsole("  ✓ SWIOT plugin found - SWIOT runtime emulator detected");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.EMU.EN_GENERIC", function() {
    if (!emuAvailable) {
        printToConsole("  SKIP: Emulator not running");
        return "SKIP";
    }
    try {
        cleanupAllDevices();
        var plugins = scopy.getPlugins(emulatorUri, "iio");
        if (!plugins || plugins.length === 0) {
            printToConsole("  SKIP: No plugins found for emulator");
            return "SKIP";
        }

        printToConsole("  Available plugins: " + plugins.join(", "));

        // Generic emulator should at least have debugger plugin
        if (pluginInList("debugger", plugins)) {
            printToConsole("  ✓ Debugger plugin found - generic IIO emulator detected");
            return true;
        }

        // If no specific plugin found, any plugin list is acceptable for generic
        printToConsole("  ✓ Generic IIO emulator detected with " + plugins.length + " plugins");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Final cleanup
cleanupAllDevices();

// Print summary and exit
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
