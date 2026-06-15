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

// Qt6 Hardware Test — RegMap Plugin (ADALM-PLUTO / ad9361-phy)
// Tests register read/write, device selection, search, and autoread on real hardware.

evaluateFile("js/testAutomations/common/testFramework.js");

TestFramework.init("Qt6 HW RegMap Tests");

if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot connect to PLUTO at ip:192.168.2.1");
    scopy.exit();
}

if (!switchToTool("Register map")) {
    printToConsole("ERROR: Cannot switch to Register map tool");
    scopy.exit();
}

// ============================================
// Device Discovery
// ============================================
printToConsole("\n=== Device Discovery ===\n");

TestFramework.runTest("TST.HW.REGMAP.AVAILABLE_DEVICES", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        printToConsole("  Available devices: " + devices.length);
        for (var i = 0; i < devices.length; i++) {
            printToConsole("    [" + i + "] " + devices[i]);
        }
        if (!devices || devices.length === 0) {
            printToConsole("  Error: No devices found");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.REGMAP.SET_DEVICE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        var targetDevice = null;
        for (var i = 0; i < devices.length; i++) {
            if (devices[i].indexOf("ad9361") !== -1) {
                targetDevice = devices[i];
                break;
            }
        }
        if (!targetDevice) {
            printToConsole("  Warning: ad9361-phy not found, using first device");
            targetDevice = devices[0];
        }
        printToConsole("  Setting device to: " + targetDevice);
        var result = regmap.setDevice(targetDevice);
        msleep(1000);
        printToConsole("  setDevice returned: " + result);
        return result === true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Register Read
// ============================================
printToConsole("\n=== Register Read ===\n");

TestFramework.runTest("TST.HW.REGMAP.READ_REGISTER", function() {
    try {
        var value = regmap.readRegister("0x000");
        printToConsole("  Register 0x000 value: " + value);
        if (value === null || value === undefined) {
            printToConsole("  Error: readRegister returned null/undefined");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.REGMAP.GET_VALUE_OF_REGISTER", function() {
    try {
        var value = regmap.getValueOfRegister("0x000");
        printToConsole("  Register 0x000 value (getValueOfRegister): " + value);
        if (value === null || value === undefined) {
            printToConsole("  Error: getValueOfRegister returned null/undefined");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.REGMAP.GET_REGISTER_INFO", function() {
    try {
        var info = regmap.getRegisterInfo("0x000");
        printToConsole("  Register 0x000 info: " + info);
        if (!info) {
            printToConsole("  Error: getRegisterInfo returned null/empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Search
// ============================================
printToConsole("\n=== Search ===\n");

TestFramework.runTest("TST.HW.REGMAP.SEARCH", function() {
    try {
        var results = regmap.search("0x00");
        printToConsole("  Search '0x00' returned: " + results.length + " results");
        if (results.length > 0) {
            for (var i = 0; i < Math.min(results.length, 3); i++) {
                printToConsole("    [" + i + "] " + results[i]);
            }
        }
        return results.length > 0;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Autoread
// ============================================
printToConsole("\n=== Autoread ===\n");

TestFramework.runTest("TST.HW.REGMAP.AUTOREAD", function() {
    try {
        var initial = regmap.isAutoreadEnabled();
        printToConsole("  Initial autoread state: " + initial);

        regmap.enableAutoread(true);
        msleep(500);
        var enabled = regmap.isAutoreadEnabled();
        printToConsole("  After enable: " + enabled);

        regmap.enableAutoread(false);
        msleep(500);
        var disabled = regmap.isAutoreadEnabled();
        printToConsole("  After disable: " + disabled);

        // Restore
        if (initial === true || initial === "true") {
            regmap.enableAutoread(true);
        }

        return enabled === true && disabled === false;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
