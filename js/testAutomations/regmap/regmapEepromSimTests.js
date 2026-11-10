/*
 * Copyright (c) 2026 Analog Devices Inc.
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

// Register Map - Simulated dongleDevice tests (regmap tool configuration POC)
//
// The eprom_test package must be installed (-DENABLE_PACKAGE_EPROM_TEST=ON). At startup DongleDeviceManager
// adds the simulated dongleDevice "dongle:sim0". Its register map starts empty. The EEPROM configuration tool
// (EpromTestPlugin, JS API "eprom_test") reads the board EEPROM on Sync and configures regmap on Configure:
//   eprom1 - template + CSV read/write (eprom1.csv)
//   eprom2 - template only, no read/write
//   eprom3 - CSV read/write only, no template (eprom3.csv)
//
// Tests automated here (Category A - fully automated via JS API):
//   TST.REGMAP.DONGLE_EMPTY_TOOL        - After connect the register map has no device
//   TST.REGMAP.DONGLE_SYNC              - Sync reads the board EEPROM, the structure lists eprom1..3
//   TST.REGMAP.DONGLE_CONFIGURE         - Configure fills the register map with eprom1..3
//   TST.REGMAP.EEPROM_SIM_DEVICE        - The register map shows exactly the three configured devices
//   TST.REGMAP.EEPROM_CSV_READ          - Read on eprom1 returns the CSV value
//   TST.REGMAP.EEPROM_CSV_WRITE         - Write on eprom1 changes the CSV and reads back
//   TST.REGMAP.EEPROM_TEMPLATE_ONLY     - eprom2: template works, read/write is refused
//   TST.REGMAP.EEPROM_READ_WRITE_ONLY   - eprom3: CSV read/write without a template
//   TST.REGMAP.EEPROM_MISSING_ADDRESS   - An address missing from the CSV gives no value
//   TST.REGMAP.DONGLE_SWITCH_DEVICE     - Switch device (board 2), Sync, Configure: only eprom4, still connected
//   TST.REGMAP.DONGLE_EPROM4_READ_WRITE - eprom4 loads its JSON template and reads and writes its own CSV (eprom4.csv)
//   TST.REGMAP.DONGLE_SWITCH_BACK       - Switch back to board 1, Sync, Configure: eprom1..3 again
//
// The status bar message of TST.REGMAP.EEPROM_MISSING_ADDRESS and the hidden controls of
// TST.REGMAP.EEPROM_TEMPLATE_ONLY are checked by hand (see registermap_tests.rst).
//
// ============================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

var SIM_URI = "dongle:sim0";
var EXPECTED_DEVICES = ["eprom1", "eprom2", "eprom3"];

// Values shipped in packages/eprom_test/resources/eprom_test/ (*.csv and the 5 register templates)
var POC_REG_NAME = "POC_REG";
var EPROM1_POC_REG_ADDR = "0x4"; // 5th register of eprom1_regmap.xml
var EPROM2_POC_REG_ADDR = "0x4"; // 5th register of eprom2_regmap.xml
var EPROM1_READ_ADDR = EPROM1_POC_REG_ADDR;
var EPROM1_READ_VALUE = 0x9e;
var EPROM1_WRITE_ADDR = "0x1";
var EPROM1_WRITE_VALUE = 0x33;
var EPROM1_MISSING_ADDR = "0x8";
var EPROM3_READ_ADDR = "0x0";
var EPROM3_READ_VALUE = 0x00090062;
var EPROM3_WRITE_ADDR = "0x4";
var EPROM3_WRITE_VALUE = 0x12;
// Board 2 (eeprom_board2.json): one device, eprom4
var BOARD1 = "board 1";
var BOARD2 = "board 2";
var BOARD2_DEVICES = ["eprom4"];
var EPROM4_POC_REG_ADDR = "0x4"; // 5th register of eprom4.regmap.json
var EPROM4_JSON_MARKER = "(JSON template)"; // in every register description of eprom4.regmap.json
var EPROM4_POC_REG_VALUE = 0x4b;
var EPROM4_WRITE_ADDR = "0x1";
var EPROM4_WRITE_VALUE = 0x55;

// Reads an address and returns the cached value as a number, or null if nothing was read
function readValue(addr) {
    regmap.readRegister(addr);
    msleep(500);
    var text = regmap.getValueOfRegister(addr);
    if (!text) {
        return null;
    }
    return parseInt(text, 16);
}

function toHex(value) {
    return "0x" + value.toString(16);
}

// The simulated device is added at startup, it is not added by the test
function connectToSimulatedDevice() {
    var simId = "";
    for (var i = 0; i < 20 && simId === ""; i++) {
        var ids = scopy.getDevicesName();
        for (var j = 0; j < ids.length; j++) {
            if (ids[j].indexOf(SIM_URI) !== -1) {
                simId = ids[j];
            }
        }
        if (simId === "") {
            msleep(500);
        }
    }
    if (simId === "") {
        printToConsole("ERROR: simulated device " + SIM_URI + " not found, is the eprom_test package installed?");
        return false;
    }

    printToConsole("Connecting to device: " + simId);
    if (!scopy.connectDevice(simId)) {
        return false;
    }
    msleep(2000);
    // let TestFramework.disconnectFromDevice() disconnect it
    TestFramework.deviceId = simId;
    TestFramework.deviceUri = SIM_URI;
    TestFramework.isConnected = true;
    printToConsole("✓ Successfully connected to device\n");
    return true;
}

// Returns an error text if the register map doesn't list exactly the given devices, or "" if it does
function checkDevices(expected) {
    var devices = regmap.getAvailableDevicesName();
    printToConsole("  Available devices: " + devices);
    if (!devices || devices.length !== expected.length) {
        return "Expected " + expected.length + " devices (" + expected + "), got: " + devices;
    }
    for (var i = 0; i < expected.length; i++) {
        if (devices.indexOf(expected[i]) === -1) {
            return "Missing device " + expected[i];
        }
    }
    return "";
}

function checkExpectedDevices() {
    return checkDevices(EXPECTED_DEVICES);
}

// Returns an error text if the synced structure doesn't list exactly the given devices, or "" if it does
function checkStructure(expected, notExpected) {
    var structure = eprom_test.getStructure();
    printToConsole("  Synced structure:\n" + structure);
    for (var i = 0; i < expected.length; i++) {
        if (structure.indexOf("\"" + expected[i] + "\"") === -1) {
            return "Missing " + expected[i] + " in the synced structure";
        }
    }
    for (var j = 0; j < notExpected.length; j++) {
        if (structure.indexOf("\"" + notExpected[j] + "\"") !== -1) {
            return notExpected[j] + " should not be in the synced structure";
        }
    }
    return "";
}

// Test Suite
TestFramework.init("Regmap Simulated dongleDevice Tests");

if (!connectToSimulatedDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// ============================================
// Test: Empty register map before Configure
// UID: TST.REGMAP.DONGLE_EMPTY_TOOL
// Description: After connect the register map tool is there but has no device
// ============================================
TestFramework.runTest("TST.REGMAP.DONGLE_EMPTY_TOOL", function() {
    var devices = regmap.getAvailableDevicesName();
    printToConsole("  Available devices: " + devices);
    if (devices && devices.length !== 0) {
        return "Expected no device before Configure, got: " + devices;
    }
    if (eprom_test.getStructure() !== "") {
        return "Expected no synced structure before Sync";
    }
    return true;
});

// ============================================
// Test: Sync
// UID: TST.REGMAP.DONGLE_SYNC
// Description: Sync reads the board EEPROM and the structure lists eprom1, eprom2 and eprom3
// ============================================
TestFramework.runTest("TST.REGMAP.DONGLE_SYNC", function() {
    var synced = eprom_test.sync();
    msleep(500);
    if (!synced) {
        return "Sync failed";
    }
    var structure = eprom_test.getStructure();
    printToConsole("  Synced structure:\n" + structure);
    for (var i = 0; i < EXPECTED_DEVICES.length; i++) {
        if (structure.indexOf("\"" + EXPECTED_DEVICES[i] + "\"") === -1) {
            return "Missing " + EXPECTED_DEVICES[i] + " in the synced structure";
        }
    }
    // Sync alone configures nothing
    if (regmap.getAvailableDevicesName().length !== 0) {
        return "Sync must not configure the register map";
    }
    return true;
});

// ============================================
// Test: Configure
// UID: TST.REGMAP.DONGLE_CONFIGURE
// Description: Configure fills the register map with the synced devices
// ============================================
TestFramework.runTest("TST.REGMAP.DONGLE_CONFIGURE", function() {
    var configured = eprom_test.configure();
    msleep(500);
    if (!configured) {
        return "Configure failed";
    }
    var error = checkExpectedDevices();
    return error === "" ? true : error;
});

// ============================================
// Test: Simulated EEPROM device
// UID: TST.REGMAP.EEPROM_SIM_DEVICE
// Description: After Configure the register map shows exactly the three configured devices
// ============================================
TestFramework.runTest("TST.REGMAP.EEPROM_SIM_DEVICE", function() {
    var error = checkExpectedDevices();
    return error === "" ? true : error;
});

// ============================================
// Test: CSV read
// UID: TST.REGMAP.EEPROM_CSV_READ
// Description: eprom1 uses the POC template, and a read of POC_REG returns the value from eprom1.csv
// ============================================
TestFramework.runTest("TST.REGMAP.EEPROM_CSV_READ", function() {
    var deviceSet = regmap.setDevice("eprom1");
    msleep(500);
    if (!deviceSet) {
        return "Failed to set device eprom1";
    }

    var info = regmap.getRegisterInfo(EPROM1_POC_REG_ADDR);
    printToConsole("  Register info for " + EPROM1_POC_REG_ADDR + ": " + info);
    if (info.indexOf("Name:" + POC_REG_NAME) === -1) {
        return "Expected " + POC_REG_NAME + " at " + EPROM1_POC_REG_ADDR + ", the POC template is not used";
    }

    var value = readValue(EPROM1_READ_ADDR);
    printToConsole("  " + EPROM1_READ_ADDR + " = " + (value === null ? "not read" : toHex(value)));
    if (value !== EPROM1_READ_VALUE) {
        return "Expected " + toHex(EPROM1_READ_VALUE) + " at " + EPROM1_READ_ADDR;
    }
    return true;
});

// ============================================
// Test: CSV write
// UID: TST.REGMAP.EEPROM_CSV_WRITE
// Description: A write on eprom1 goes to the CSV and the read-back shows the new value
// ============================================
TestFramework.runTest("TST.REGMAP.EEPROM_CSV_WRITE", function() {
    var deviceSet = regmap.setDevice("eprom1");
    msleep(500);
    if (!deviceSet) {
        return "Failed to set device eprom1";
    }

    // Save original value
    var original = readValue(EPROM1_WRITE_ADDR);
    if (original === null) {
        return "Could not read the original value at " + EPROM1_WRITE_ADDR;
    }
    printToConsole("  Original value at " + EPROM1_WRITE_ADDR + ": " + toHex(original));

    try {
        regmap.write(EPROM1_WRITE_ADDR, toHex(EPROM1_WRITE_VALUE));
        msleep(500);

        // the write strategy triggers the read-back, so the cache already holds the new value
        var readback = parseInt(regmap.getValueOfRegister(EPROM1_WRITE_ADDR), 16);
        printToConsole("  Readback value at " + EPROM1_WRITE_ADDR + ": " + toHex(readback));

        // Restore original value
        regmap.write(EPROM1_WRITE_ADDR, toHex(original));
        msleep(500);

        if (readback !== EPROM1_WRITE_VALUE) {
            return "Expected readback " + toHex(EPROM1_WRITE_VALUE);
        }
        var restored = readValue(EPROM1_WRITE_ADDR);
        if (restored !== original) {
            return "Original value not restored";
        }
        return true;
    } catch (e) {
        // Restore original value on error
        regmap.write(EPROM1_WRITE_ADDR, toHex(original));
        msleep(500);
        return "Error during write test: " + e;
    }
});

// ============================================
// Test: Template only
// UID: TST.REGMAP.EEPROM_TEMPLATE_ONLY
// Description: eprom2 has a template and no read/write; read/write calls are refused without a crash
// ============================================
TestFramework.runTest("TST.REGMAP.EEPROM_TEMPLATE_ONLY", function() {
    var deviceSet = regmap.setDevice("eprom2");
    msleep(500);
    if (!deviceSet) {
        return "Failed to set device eprom2";
    }

    // regmap search compares lower case names, so the search text must be lower case too
    var matches = regmap.search(POC_REG_NAME.toLowerCase());
    printToConsole("  Search results for '" + POC_REG_NAME.toLowerCase() + "': " + matches.length + " matches");
    if (matches.length === 0) {
        return "Search found nothing, the POC template is missing";
    }

    var info = regmap.getRegisterInfo(EPROM2_POC_REG_ADDR);
    printToConsole("  Register info for " + EPROM2_POC_REG_ADDR + ": " + info);
    if (info.indexOf("Name:" + POC_REG_NAME) === -1) {
        return "Expected " + POC_REG_NAME + " at " + EPROM2_POC_REG_ADDR;
    }

    // no read/write: every call is refused, and nothing gets a value
    var readText = regmap.readRegister("0x0");
    msleep(500);
    regmap.write("0x0", "0x1");
    msleep(500);
    regmap.readInterval("0x0", "0x4");
    msleep(500);
    if (readText !== "") {
        return "readRegister should be refused, got: " + readText;
    }
    if (regmap.getValueOfRegister("0x0") !== "") {
        return "A value was read on a template only device";
    }
    return true;
});

// ============================================
// Test: Read/write only
// UID: TST.REGMAP.EEPROM_READ_WRITE_ONLY
// Description: eprom3 has no template and reads/writes its own CSV
// ============================================
TestFramework.runTest("TST.REGMAP.EEPROM_READ_WRITE_ONLY", function() {
    var deviceSet = regmap.setDevice("eprom3");
    msleep(500);
    if (!deviceSet) {
        return "Failed to set device eprom3";
    }

    if (regmap.search("0x0").length !== 0) {
        return "Search should find nothing without a template";
    }

    var value = readValue(EPROM3_READ_ADDR);
    printToConsole("  " + EPROM3_READ_ADDR + " = " + (value === null ? "not read" : toHex(value)));
    if (value !== EPROM3_READ_VALUE) {
        return "Expected " + toHex(EPROM3_READ_VALUE) + " at " + EPROM3_READ_ADDR;
    }

    // Save original value
    var original = readValue(EPROM3_WRITE_ADDR);
    if (original === null) {
        return "Could not read the original value at " + EPROM3_WRITE_ADDR;
    }

    try {
        regmap.write(EPROM3_WRITE_ADDR, toHex(EPROM3_WRITE_VALUE));
        msleep(500);
        var readback = parseInt(regmap.getValueOfRegister(EPROM3_WRITE_ADDR), 16);
        printToConsole("  Readback value at " + EPROM3_WRITE_ADDR + ": " + toHex(readback));

        // Restore original value
        regmap.write(EPROM3_WRITE_ADDR, toHex(original));
        msleep(500);

        if (readback !== EPROM3_WRITE_VALUE) {
            return "Expected readback " + toHex(EPROM3_WRITE_VALUE);
        }
        return true;
    } catch (e) {
        // Restore original value on error
        regmap.write(EPROM3_WRITE_ADDR, toHex(original));
        msleep(500);
        return "Error during write test: " + e;
    }
});

// ============================================
// Test: Missing address
// UID: TST.REGMAP.EEPROM_MISSING_ADDRESS
// Description: Reading an address that is not in the CSV gives no value (and an error in the status bar)
// ============================================
TestFramework.runTest("TST.REGMAP.EEPROM_MISSING_ADDRESS", function() {
    var deviceSet = regmap.setDevice("eprom1");
    msleep(500);
    if (!deviceSet) {
        return "Failed to set device eprom1";
    }

    var value = readValue(EPROM1_MISSING_ADDR);
    printToConsole("  " + EPROM1_MISSING_ADDR + " = " + (value === null ? "not read" : toHex(value)));
    if (value !== null) {
        return "Address " + EPROM1_MISSING_ADDR + " is not in the CSV but returned " + toHex(value);
    }
    return true;
});

// Puts board 1 back behind the dongle and configures the register map with it again
function restoreBoard1() {
    if (eprom_test.getBoard() !== BOARD1) {
        eprom_test.switchDevice();
        msleep(500);
    }
    eprom_test.sync();
    msleep(500);
    eprom_test.configure();
    msleep(500);
}

// ============================================
// Test: Switch device
// UID: TST.REGMAP.DONGLE_SWITCH_DEVICE
// Description: The board behind the dongle is replaced while connected; Sync reads board 2 and Configure
// reconfigures the register map with eprom4 only, without disconnecting
// ============================================
TestFramework.runTest("TST.REGMAP.DONGLE_SWITCH_DEVICE", function() {
    try {
        eprom_test.switchDevice();
        msleep(500);
        var board = eprom_test.getBoard();
        printToConsole("  Board after switch: " + board);
        if (board !== BOARD2) {
            restoreBoard1();
            return "Expected " + BOARD2 + " after switchDevice, got: " + board;
        }
        // the old configuration is stale but stays applied until Configure
        if (eprom_test.getStructure() !== "") {
            restoreBoard1();
            return "The synced structure must be cleared by switchDevice";
        }
        if (eprom_test.configure()) {
            restoreBoard1();
            return "Configure must be refused before Sync";
        }
        msleep(500);
        var error = checkExpectedDevices();
        if (error !== "") {
            restoreBoard1();
            return "Before Configure the register map must keep board 1: " + error;
        }

        if (!eprom_test.sync()) {
            restoreBoard1();
            return "Sync of board 2 failed";
        }
        msleep(500);
        error = checkStructure(BOARD2_DEVICES, EXPECTED_DEVICES);
        if (error !== "") {
            restoreBoard1();
            return error;
        }

        if (!eprom_test.configure()) {
            restoreBoard1();
            return "Configure of board 2 failed";
        }
        msleep(500);
        // same session: the register map API answers without a reconnect
        error = checkDevices(BOARD2_DEVICES);
        if (error !== "") {
            restoreBoard1();
            return error;
        }
        return true;
    } catch (e) {
        restoreBoard1();
        return "Error during switch test: " + e;
    }
});

// ============================================
// Test: eprom4 read/write
// UID: TST.REGMAP.DONGLE_EPROM4_READ_WRITE
// Description: eprom4 (board 2) uses its own JSON template and reads and writes its own CSV
// ============================================
TestFramework.runTest("TST.REGMAP.DONGLE_EPROM4_READ_WRITE", function() {
    var deviceSet = regmap.setDevice("eprom4");
    msleep(500);
    if (!deviceSet) {
        return "Failed to set device eprom4";
    }

    var info = regmap.getRegisterInfo(EPROM4_POC_REG_ADDR);
    printToConsole("  Register info for " + EPROM4_POC_REG_ADDR + ": " + info);
    if (info.indexOf("Name:" + POC_REG_NAME) === -1) {
        return "Expected " + POC_REG_NAME + " at " + EPROM4_POC_REG_ADDR + ", the eprom4 template is not used";
    }
    if (info.join("|").indexOf(EPROM4_JSON_MARKER) === -1) {
        return "eprom4 is not described by eprom4.regmap.json: " + info;
    }
    var value = readValue(EPROM4_POC_REG_ADDR);
    printToConsole("  " + EPROM4_POC_REG_ADDR + " = " + (value === null ? "not read" : toHex(value)));
    if (value !== EPROM4_POC_REG_VALUE) {
        return "Expected " + toHex(EPROM4_POC_REG_VALUE) + " at " + EPROM4_POC_REG_ADDR;
    }

    // Save original value
    var original = readValue(EPROM4_WRITE_ADDR);
    if (original === null) {
        return "Could not read the original value at " + EPROM4_WRITE_ADDR;
    }
    printToConsole("  Original value at " + EPROM4_WRITE_ADDR + ": " + toHex(original));

    try {
        regmap.write(EPROM4_WRITE_ADDR, toHex(EPROM4_WRITE_VALUE));
        msleep(500);
        var readback = parseInt(regmap.getValueOfRegister(EPROM4_WRITE_ADDR), 16);
        printToConsole("  Readback value at " + EPROM4_WRITE_ADDR + ": " + toHex(readback));

        // Restore original value
        regmap.write(EPROM4_WRITE_ADDR, toHex(original));
        msleep(500);

        if (readback !== EPROM4_WRITE_VALUE) {
            return "Expected readback " + toHex(EPROM4_WRITE_VALUE);
        }
        if (readValue(EPROM4_WRITE_ADDR) !== original) {
            return "Original value not restored";
        }
        return true;
    } catch (e) {
        // Restore original value on error
        regmap.write(EPROM4_WRITE_ADDR, toHex(original));
        msleep(500);
        return "Error during write test: " + e;
    }
});

// ============================================
// Test: Switch back
// UID: TST.REGMAP.DONGLE_SWITCH_BACK
// Description: Board 1 goes back behind the dongle; Sync + Configure bring back eprom1..3 without disconnecting
// ============================================
TestFramework.runTest("TST.REGMAP.DONGLE_SWITCH_BACK", function() {
    try {
        restoreBoard1();
        var board = eprom_test.getBoard();
        printToConsole("  Board after switch back: " + board);
        if (board !== BOARD1) {
            return "Expected " + BOARD1 + ", got: " + board;
        }
        var error = checkStructure(EXPECTED_DEVICES, BOARD2_DEVICES);
        if (error !== "") {
            return error;
        }
        error = checkExpectedDevices();
        return error === "" ? true : error;
    } catch (e) {
        return "Error during switch back test: " + e;
    }
});

TestFramework.disconnectFromDevice();
TestFramework.printSummary();
scopy.exit();
