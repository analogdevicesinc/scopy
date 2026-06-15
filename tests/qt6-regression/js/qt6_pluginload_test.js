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

// Qt6 Regression: Plugin Load and Switch Test
// Validates that all enabled plugins load and can be switched to under Qt6.
// Tier 1 (emulator-safe) — no hardware value assertions.

evaluateFile("js/testAutomations/common/testFramework.js");

TestFramework.init("Qt6 Plugin Load Regression Tests");

var testUri = "ip:127.0.0.1";

TestFramework.runTest("Connect to emulator", function() {
    var deviceId = scopy.addDevice(testUri);
    if (!deviceId) {
        return "Could not add device";
    }
    msleep(500);

    if (!scopy.connectDevice(deviceId)) {
        scopy.removeDevice(testUri);
        return "Could not connect to emulator";
    }
    msleep(2000);
    return true;
});

TestFramework.runTest("Get tools list is non-empty", function() {
    var tools = scopy.getTools();
    if (!tools || tools.length === 0) {
        return "No tools loaded";
    }
    printToConsole("Loaded tools: " + tools.join(", "));
    return true;
});

TestFramework.runTest("Switch to each tool", function() {
    var tools = scopy.getTools();
    if (!tools || tools.length === 0) {
        return "No tools to switch to";
    }

    var failures = [];
    for (var i = 0; i < tools.length; i++) {
        var toolName = tools[i];
        try {
            var switched = scopy.switchTool(toolName);
            msleep(1000);
            if (!switched) {
                failures.push(toolName);
            }
        } catch (e) {
            failures.push(toolName + " (exception: " + e + ")");
        }
    }

    if (failures.length > 0) {
        return "Failed to switch to: " + failures.join(", ");
    }
    return true;
});

TestFramework.runTest("Disconnect cleanly", function() {
    try {
        var devices = scopy.getDevicesName();
        if (devices && devices.length > 0) {
            for (var i = devices.length - 1; i >= 0; i--) {
                scopy.disconnectDevice(i);
                scopy.removeDevice(i);
            }
        }
    } catch (e) {
        return "Cleanup error: " + e;
    }
    msleep(500);
    return true;
});

var exitCode = TestFramework.printSummary();
scopy.exit();
