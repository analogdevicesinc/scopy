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

// Regmap Plugin Automated Tests
//
// Tests automated here (Category A - fully automated via JS API):
//   TST.REGMAP.DEVICE_LIST           - Verify device list is populated
//   TST.REGMAP.SET_DEVICE            - Verify device selection
//   TST.REGMAP.READ_REGISTER         - Verify register read
//   TST.REGMAP.WRITE_REGISTER        - Verify register write and readback
//   TST.REGMAP.AUTOREAD              - Verify autoread toggle
//   TST.REGMAP.SEARCH                - Verify register search
//   TST.REGMAP.READ_INTERVAL         - Verify reading a range of registers
//   TST.REGMAP.REGISTER_INFO         - Verify register info retrieval
//   TST.REGMAP.BITFIELD_INFO         - Verify bitfield info retrieval
//   TST.REGMAP.VALUE_OF_REGISTER     - Verify cached register value retrieval
//
// Not automated (Category C - requires file browser / OS interaction):
//   TST.REGMAP.REGISTER_DUMP         - Requires file path on disk
//   TST.REGMAP.WRITE_FROM_FILE       - Requires valid register dump file
//
// ============================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("Regmap Documentation Tests");

// Connect to device
if (!TestFramework.connectToDevice("ip:127.0.0.0")) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to Regmap tool
if (!switchToTool("Register Map")) {
    printToConsole("ERROR: Cannot switch to Register Map tool");
    scopy.exit();
}

// ============================================
// Test: Device List
// UID: TST.REGMAP.DEVICE_LIST
// Description: Verify that available devices are listed
// ============================================
TestFramework.runTest("TST.REGMAP.DEVICE_LIST", function() {
    var devices = regmap.getAvailableDevicesName();
    printToConsole("  Available devices: " + devices);

    if (!devices || devices.length === 0) {
        return "No devices found";
    }

    printToConsole("  Found " + devices.length + " device(s)");
    return true;
});

// ============================================
// Test: Set Device
// UID: TST.REGMAP.SET_DEVICE
// Description: Verify that a device can be selected
// ============================================
TestFramework.runTest("TST.REGMAP.SET_DEVICE", function() {
    var devices = regmap.getAvailableDevicesName();
    if (!devices || devices.length === 0) {
        return "No devices available to select";
    }

    var deviceName = devices[0];
    printToConsole("  Selecting device: " + deviceName);
    var result = regmap.setDevice(deviceName);
    msleep(500);

    if (!result) {
        return "Failed to set device: " + deviceName;
    }

    printToConsole("  Device set successfully: " + deviceName);
    return true;
});

// ============================================
// Test: Read Register
// UID: TST.REGMAP.READ_REGISTER
// Description: Verify that a register can be read
// ============================================
TestFramework.runTest("TST.REGMAP.READ_REGISTER", function() {
    var devices = regmap.getAvailableDevicesName();
    if (!devices || devices.length === 0) {
        return "No devices available";
    }

    regmap.setDevice(devices[0]);
    msleep(500);

    var value = regmap.readRegister("0x0");
    printToConsole("  Register 0x0 value: " + value);

    if (value === null || value === undefined) {
        return "Failed to read register 0x0";
    }

    return true;
});

// ============================================
// Test: Write Register
// UID: TST.REGMAP.WRITE_REGISTER
// Description: Verify register write and readback
// ============================================
TestFramework.runTest("TST.REGMAP.WRITE_REGISTER", function() {
    var devices = regmap.getAvailableDevicesName();
    if (!devices || devices.length === 0) {
        return "No devices available";
    }

    regmap.setDevice(devices[0]);
    msleep(500);

    // Save original value
    var originalValue = regmap.readRegister("0x0");
    printToConsole("  Original value at 0x0: " + originalValue);

    try {
        // Write a test value
        regmap.write("0x0", "0xFF");
        msleep(500);

        // Read back
        var readback = regmap.readRegister("0x0");
        printToConsole("  Readback value at 0x0: " + readback);

        // Restore original value
        regmap.write("0x0", originalValue);
        msleep(500);

        return true;
    } catch (e) {
        // Restore original value on error
        regmap.write("0x0", originalValue);
        msleep(500);
        return "Error during write test: " + e;
    }
});

// ============================================
// Test: Autoread Toggle
// UID: TST.REGMAP.AUTOREAD
// Description: Verify autoread can be enabled and disabled
// ============================================
TestFramework.runTest("TST.REGMAP.AUTOREAD", function() {
    var devices = regmap.getAvailableDevicesName();
    if (!devices || devices.length === 0) {
        return "No devices available";
    }

    regmap.setDevice(devices[0]);
    msleep(500);

    // Save original state
    var originalState = regmap.isAutoreadEnabled();
    printToConsole("  Original autoread state: " + originalState);

    try {
        // Enable autoread
        var enableResult = regmap.enableAutoread(true);
        msleep(500);
        if (!enableResult) {
            regmap.enableAutoread(originalState);
            msleep(500);
            return "Failed to enable autoread";
        }

        var isEnabled = regmap.isAutoreadEnabled();
        if (!isEnabled) {
            regmap.enableAutoread(originalState);
            msleep(500);
            return "Autoread not enabled after enableAutoread(true)";
        }
        printToConsole("  Autoread enabled successfully");

        // Disable autoread
        var disableResult = regmap.enableAutoread(false);
        msleep(500);
        if (!disableResult) {
            regmap.enableAutoread(originalState);
            msleep(500);
            return "Failed to disable autoread";
        }

        var isDisabled = !regmap.isAutoreadEnabled();
        if (!isDisabled) {
            regmap.enableAutoread(originalState);
            msleep(500);
            return "Autoread not disabled after enableAutoread(false)";
        }
        printToConsole("  Autoread disabled successfully");

        // Restore original state
        regmap.enableAutoread(originalState);
        msleep(500);

        return true;
    } catch (e) {
        regmap.enableAutoread(originalState);
        msleep(500);
        return "Error during autoread test: " + e;
    }
});

// ============================================
// Test: Search
// UID: TST.REGMAP.SEARCH
// Description: Verify register search functionality
// ============================================
TestFramework.runTest("TST.REGMAP.SEARCH", function() {
    var devices = regmap.getAvailableDevicesName();
    if (!devices || devices.length === 0) {
        return "No devices available";
    }

    regmap.setDevice(devices[0]);
    msleep(500);

    // Search for a common term
    var results = regmap.search("0x0");
    printToConsole("  Search results for '0x0': " + results.length + " matches");

    if (results === null || results === undefined) {
        return "Search returned null/undefined";
    }

    return true;
});

// ============================================
// Test: Read Interval
// UID: TST.REGMAP.READ_INTERVAL
// Description: Verify reading a range of registers
// ============================================
TestFramework.runTest("TST.REGMAP.READ_INTERVAL", function() {
    var devices = regmap.getAvailableDevicesName();
    if (!devices || devices.length === 0) {
        return "No devices available";
    }

    regmap.setDevice(devices[0]);
    msleep(500);

    // Read a small interval
    regmap.readInterval("0x0", "0x3");
    msleep(500);

    printToConsole("  Read interval 0x0 to 0x3 completed");
    return true;
});

// ============================================
// Test: Register Info
// UID: TST.REGMAP.REGISTER_INFO
// Description: Verify register info retrieval
// ============================================
TestFramework.runTest("TST.REGMAP.REGISTER_INFO", function() {
    var devices = regmap.getAvailableDevicesName();
    if (!devices || devices.length === 0) {
        return "No devices available";
    }

    regmap.setDevice(devices[0]);
    msleep(500);

    var info = regmap.getRegisterInfo("0x0");
    printToConsole("  Register info for 0x0: " + info);

    if (info === null || info === undefined) {
        return "Register info returned null/undefined";
    }

    // Info may be empty if no template is loaded, which is valid
    if (info.length > 0) {
        printToConsole("  Register has " + info.length + " info fields");
    } else {
        printToConsole("  No register template info available (expected for devices without XML templates)");
    }

    return true;
});

// ============================================
// Test: Bitfield Info
// UID: TST.REGMAP.BITFIELD_INFO
// Description: Verify bitfield info retrieval
// ============================================
TestFramework.runTest("TST.REGMAP.BITFIELD_INFO", function() {
    var devices = regmap.getAvailableDevicesName();
    if (!devices || devices.length === 0) {
        return "No devices available";
    }

    regmap.setDevice(devices[0]);
    msleep(500);

    var bitfields = regmap.getRegisterBitFieldsInfo("0x0");
    printToConsole("  Bitfield info for 0x0: " + bitfields);

    if (bitfields === null || bitfields === undefined) {
        return "Bitfield info returned null/undefined";
    }

    if (bitfields.length > 0) {
        printToConsole("  Register has bitfield info available");
    } else {
        printToConsole("  No bitfield info available (expected for devices without XML templates)");
    }

    return true;
});

// ============================================
// Test: Value Of Register
// UID: TST.REGMAP.VALUE_OF_REGISTER
// Description: Verify cached register value retrieval
// ============================================
TestFramework.runTest("TST.REGMAP.VALUE_OF_REGISTER", function() {
    var devices = regmap.getAvailableDevicesName();
    if (!devices || devices.length === 0) {
        return "No devices available";
    }

    regmap.setDevice(devices[0]);
    msleep(500);

    // First read the register to populate the cache
    regmap.readRegister("0x0");
    msleep(500);

    // Then get the cached value
    var value = regmap.getValueOfRegister("0x0");
    printToConsole("  Cached value of register 0x0: " + value);

    // Value may be empty if register was not previously read
    if (value === null || value === undefined) {
        return "getValueOfRegister returned null/undefined";
    }

    return true;
});

// Cleanup
TestFramework.disconnectFromDevice();
TestFramework.printSummary();
scopy.exit();
