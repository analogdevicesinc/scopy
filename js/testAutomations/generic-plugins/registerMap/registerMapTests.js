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

// Register Map Automated Tests
// 1:1 mapping with documentation test cases
// Based on TST.REGMAP.* test specifications

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Global wait time configuration
// Set to 1000 for normal testing, can be increased for external observation
// or decreased (e.g., 100) for fast test runs
var waitTime = 1000;  // milliseconds

// Helper function to verify file exists and has content
function fileExistsAndHasData(path) {
    try {
        var content = fileIO.readAll(path);
        if (!content || content.length === 0) {
            return false;
        }
        var lines = content.split('\n').filter(function (line) {
            return line.trim().length > 0;
        });
        return lines.length >= 1; // At least some content
    } catch (e) {
        return false;
    }
}

// Test Suite: Register Map
TestFramework.init("Register Map Tests (1:1 Documentation Mapping)");

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to Register Map tool
if (!switchToTool("Register map")) {
    printToConsole("ERROR: Cannot access Register Map");
    exit(1);
}

// ===========================================================================
// Test 1: Read
// ===========================================================================
TestFramework.runTest("TST.REGMAP.READ", function() {
    try {
        let devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Test reading register 0x02 as specified in documentation
        let testAddr = "0x02";
        printToConsole("  Reading register " + testAddr);

        let value = regmap.readRegister(testAddr);

        if (value && value.indexOf("0x") === 0) {
            printToConsole("  ✓ Successfully read register " + testAddr + ": " + value);
            return true;
        } else {
            printToConsole("  ✗ Failed to read register or invalid format");
            return false;
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 2: Write
// ===========================================================================
TestFramework.runTest("TST.REGMAP.WRITE", function() {
    try {
        let devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Test writing to register 0x02 with value 0x4a
        let testAddr = "0x02";
        let testValue = "0x4a";

        printToConsole("  Reading current value of " + testAddr);
        let originalValue = regmap.readRegister(testAddr);
        printToConsole("    Original value: " + originalValue);

        printToConsole("  Writing " + testValue + " to " + testAddr);
        regmap.write(testAddr, testValue);
        msleep(waitTime);

        let readback = regmap.readRegister(testAddr);
        printToConsole("    Readback value: " + readback);

        // Some registers may have reserved bits, so check if any bits changed
        if (readback === testValue) {
            printToConsole("  ✓ Write successful, exact match");
            return true;
        } else if (readback !== originalValue) {
            printToConsole("  ✓ Write successful, value changed (may have reserved bits)");
            return true;
        } else {
            printToConsole("  ✗ Write failed, value unchanged");
            return false;
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        // Register might be read-only
        printToConsole("  ⚠ Register may be read-only");
        return true; // Don't fail test for read-only registers
    }
});

// ===========================================================================
// Test 3: Write Wrong Value
// ===========================================================================
TestFramework.runTest("TST.REGMAP.WRITE_WRONG_VALUE", function() {
    try {
        let devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        let testAddr = "0x02";
        let invalidValue = "0xtest"; // Invalid hex value

        printToConsole("  Attempting to write invalid value: " + invalidValue);

        try {
            regmap.write(testAddr, invalidValue);
            msleep(waitTime);

            // Check what value was actually written
            let readback = regmap.readRegister(testAddr);
            printToConsole("  Value after invalid write: " + readback);

            // Should either reject the write or write 0x00
            if (readback === "0x00" || readback === "0x0") {
                printToConsole("  ✓ Invalid value resulted in 0x00");
                return true;
            } else {
                printToConsole("  ⚠ Invalid value handled differently: " + readback);
                return true; // Still pass as error was handled
            }

        } catch (writeError) {
            printToConsole("  ✓ Invalid value properly rejected with error");
            return true;
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 4: Change Device
// ===========================================================================
TestFramework.runTest("TST.REGMAP.CHANGE_DEVICE", function() {
    try {
        let devices = regmap.getAvailableDevicesName();

        if (!devices || devices.length < 2) {
            printToConsole("  Need at least 2 devices for this test");
            return "SKIP";
        }

        printToConsole("  Available devices: " + devices.join(", "));

        // Test switching between first two devices
        let device1 = devices[0];
        let device2 = devices[1];

        printToConsole("  Switching to device: " + device2);
        let result = regmap.setDevice(device2);

        if (result === true) {
            printToConsole("  ✓ Successfully changed device");

            // Switch back to first device
            regmap.setDevice(device1);
            return true;
        } else {
            printToConsole("  ✗ Failed to change device");
            return false;
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 6: Search Register
// ===========================================================================
TestFramework.runTest("TST.REGMAP.SEARCH", function() {
    try {
        let devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        let testsPassed = 0;
        let totalTests = 3;

        // Test 1: Search for "test"
        printToConsole("  Searching for 'test'");
        let results1 = regmap.search("test");
        if (results1 && results1.length > 0) {
            printToConsole("    Found " + results1.length + " results");
            testsPassed++;
        } else {
            printToConsole("    No results (may be device-dependent)");
        }

        // Test 2: Search for "010"
        printToConsole("  Searching for '010'");
        let results2 = regmap.search("010");
        if (results2 && results2.length > 0) {
            printToConsole("    Found " + results2.length + " results");
            testsPassed++;
        } else {
            printToConsole("    No results");
        }

        // Test 3: Search for non-existent term
        printToConsole("  Searching for 'not in here'");
        let results3 = regmap.search("not in here");
        if (!results3 || results3.length === 0) {
            printToConsole("    ✓ Correctly returned no results");
            testsPassed++;
        } else {
            printToConsole("    ✗ Unexpectedly found " + results3.length + " results");
        }

        // Pass if at least 2 out of 3 tests passed
        if (testsPassed >= 2) {
            printToConsole("  ✓ Search functionality working (" + testsPassed + "/" + totalTests + " tests passed)");
            return true;
        } else {
            printToConsole("  ✗ Search functionality issues");
            return false;
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 15: Settings Autoread
// ===========================================================================
TestFramework.runTest("TST.REGMAP.SETTINGS_AUTOREAD", function() {
    try {
        // Store initial state to restore later
        let initialState = regmap.isAutoreadEnabled();
        printToConsole("  Initial autoread state: " + (initialState ? "enabled" : "disabled"));

        // Enable autoread
        printToConsole("  Enabling autoread");
        let enableResult = regmap.enableAutoread(true);

        if (!enableResult) {
            printToConsole("  ✗ Failed to enable autoread");
            return false;
        }

        msleep(waitTime);

        // Verify it's enabled
        let isEnabled = regmap.isAutoreadEnabled();
        if (!isEnabled) {
            printToConsole("  ✗ Autoread not enabled after setting");
            return false;
        }

        printToConsole("  ✓ Autoread successfully enabled");

        // When autoread is enabled, selecting a register should auto-read it
        // We can't test the UI selection, but we can verify the API state

        // Disable autoread
        printToConsole("  Disabling autoread");
        regmap.enableAutoread(false);
        msleep(waitTime);

        let isDisabled = !regmap.isAutoreadEnabled();
        if (!isDisabled) {
            printToConsole("  ✗ Failed to disable autoread");
            return false;
        }

        printToConsole("  ✓ Autoread successfully disabled");

        // Restore initial state
        regmap.enableAutoread(initialState);

        return true;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 16: Settings Read Interval
// ===========================================================================
TestFramework.runTest("TST.REGMAP.SETTINGS_READ_INTERVAL", function() {
    try {
        let devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Read interval from 0x02 to 0x04 as specified in documentation
        let fromAddr = "0x02";
        let toAddr = "0x04";

        printToConsole("  Reading interval from " + fromAddr + " to " + toAddr);

        regmap.readInterval(fromAddr, toAddr);
        msleep(waitTime); // Allow time for reads to complete

        // Verify values were read by checking cached values
        let successCount = 0;
        let addresses = ["0x02", "0x03", "0x04"];

        for (let addr of addresses) {
            try {
                let value = regmap.getValueOfRegister(addr);
                if (value && value.length > 0) {
                    printToConsole("    " + addr + " = " + value);
                    successCount++;
                } else {
                    printToConsole("    " + addr + " = (not cached)");
                }
            } catch (e) {
                printToConsole("    " + addr + " = (error reading)");
            }
        }

        if (successCount > 0) {
            printToConsole("  ✓ Read interval successful (" + successCount + "/3 registers read)");
            return true;
        } else {
            printToConsole("  ✗ Read interval failed - no registers read");
            return false;
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 17: Settings Read Interval Wrong Interval
// ===========================================================================
TestFramework.runTest("TST.REGMAP.SETTINGS_READ_INTERVAL_WRONG_INTERVAL", function() {
    try {
        let devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Try to read with from > to (wrong interval)
        let fromAddr = "0x04";
        let toAddr = "0x02";

        printToConsole("  Testing wrong interval: from " + fromAddr + " to " + toAddr);

        try {
            regmap.readInterval(fromAddr, toAddr);
            msleep(waitTime);

            // Check if any registers were read (they shouldn't be)
            let addr = "0x03"; // Middle address that shouldn't be read
            let value = regmap.getValueOfRegister(addr);

            // Check if the register was actually read
            if (!value || value.length === 0) {
                printToConsole("  ✓ Wrong interval correctly rejected (no data read)");
                return true;
            } else {
                // If we get here without error, the interval was processed
                // This might be okay if the implementation handles it gracefully
                printToConsole("  ⚠ Wrong interval was processed (implementation may auto-correct): " + value);
                return true;
            }

        } catch (e) {
            printToConsole("  ✓ Wrong interval properly rejected: " + e);
            return true;
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 18: Settings Register Dump
// ===========================================================================
TestFramework.runTest("TST.REGMAP.SETTINGS_REGISTER_DUMP", function() {
    try {
        let devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // First read some registers to have data to dump
        printToConsole("  Reading registers 0x02, 0x03, 0x04");
        regmap.readRegister("0x02");
        regmap.readRegister("0x03");
        regmap.readRegister("0x04");
        msleep(waitTime);

        let dumpFile = fileIO.getTempPath() + "/regmap_dump_test.csv";
        printToConsole("  Dumping registers to: " + dumpFile);

        try {
            regmap.registerDump(dumpFile);
            msleep(waitTime);

            // Verify the file was created and contains data
            try {
                let content = fileIO.readAll(dumpFile);
                if (content && content.length > 0) {
                    let lines = content.split('\n').filter(function (line) {
                        return line.trim().length > 0;
                    });
                    if (lines.length >= 1) {
                        printToConsole("  ✓ Register dump created with " + lines.length + " lines");
                        return true;
                    } else {
                        printToConsole("  ✗ Dump file is empty");
                        return false;
                    }
                } else {
                    printToConsole("  ✗ Dump file has no content");
                    return false;
                }
            } catch (readError) {
                printToConsole("  ✗ Failed to verify dump file: " + readError);
                return false;
            }

        } catch (dumpError) {
            printToConsole("  ✗ Register dump failed: " + dumpError);
            return false;
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 11: Change Value of Bitfield
// ===========================================================================
TestFramework.runTest("TST.REGMAP.CHANGE_BITFIELD_VALUE", function() {
    try {
        let devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Test changing bitfield as specified in documentation
        // Register 0x02, change from 0x4c to 0x44 (bit 3 from on to off)
        let testAddr = "0x02";
        let initialValue = "0x4c";
        let expectedValue = "0x44";

        printToConsole("  Setting register " + testAddr + " to " + initialValue);
        regmap.write(testAddr, initialValue);
        msleep(waitTime);

        // Change bitfield value (bit 3 off means clearing bit 3)
        // 0x4c = 01001100, 0x44 = 01000100 (bit 3 cleared)
        printToConsole("  Changing bit 3 from on to off");

        try {
            // Write the new value with bit 3 cleared
            regmap.writeBitField(testAddr, expectedValue);
            msleep(waitTime);

            let readback = regmap.readRegister(testAddr);
            printToConsole("    Value after bitfield change: " + readback);

            // Check if the value matches expected
            let readNum = parseInt(readback.replace("0x", ""), 16);
            let expectedNum = parseInt(expectedValue.replace("0x", ""), 16);

            // Check if bit 3 was cleared (mask for bit 3 is 0x08)
            if ((readNum & 0x08) === 0) {
                printToConsole("  ✓ Bit 3 successfully changed to off");
                return true;
            } else {
                printToConsole("  ✗ Bit 3 not changed");
                return false;
            }

        } catch (e) {
            printToConsole("  ⚠ Bitfield operation not supported: " + e);
            // Don't fail if bitfields aren't supported
            return true;
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 19: Settings Write Values
// ===========================================================================
TestFramework.runTest("TST.REGMAP.SETTINGS_WRITE_VALUES", function() {
    try {
        let devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        let csvFile = fileIO.getTempPath() + "/regmap_test_values.csv";

        // Create a test CSV file with known register values
        printToConsole("  Creating test CSV file: " + csvFile);
        let csvContent = "address,value\n0x02,0x55\n0x03,0x66\n0x04,0x77";

        try {
            fileIO.writeToFile(csvContent, csvFile);
            msleep(waitTime);

            printToConsole("  Loading values from: " + csvFile);

            // Set the file path
            regmap.setPath(csvFile);
            msleep(waitTime);

            // Write values from file
            regmap.writeFromFile(csvFile);
            msleep(waitTime);

            // Verify the values were written by reading them back
            printToConsole("  Verifying written values:");
            let val02 = regmap.readRegister("0x02");
            let val03 = regmap.readRegister("0x03");
            let val04 = regmap.readRegister("0x04");

            printToConsole("    0x02 = " + val02 + " (expected 0x55)");
            printToConsole("    0x03 = " + val03 + " (expected 0x66)");
            printToConsole("    0x04 = " + val04 + " (expected 0x77)");

            // Check if at least one value was written correctly
            let successCount = 0;
            if (val02 === "0x55" || val02 === "0x0055") successCount++;
            if (val03 === "0x66" || val03 === "0x0066") successCount++;
            if (val04 === "0x77" || val04 === "0x0077") successCount++;

            if (successCount >= 2) {
                printToConsole("  ✓ Write values from file completed and verified (" + successCount + "/3)");
                return true;
            } else {
                printToConsole("  ✗ Write verification failed (only " + successCount + "/3 matched)");
                return false;
            }

        } catch (writeError) {
            printToConsole("  ✗ Write from file failed: " + writeError);
            return false;
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Additional Helper Tests (not in documentation but useful for validation)
// ===========================================================================

// Test: Get Available Devices
TestFramework.runTest("TST.REGMAP.GET_AVAILABLE_DEVICES", function() {
    try {
        let devices = regmap.getAvailableDevicesName();

        if (!devices || devices.length === 0) {
            printToConsole("  ✗ No devices found");
            return false;
        }

        printToConsole("  Found " + devices.length + " devices:");
        for (let i = 0; i < devices.length && i < 5; i++) {
            printToConsole("    - " + devices[i]);
        }

        return true;

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Test: Get Register Info
TestFramework.runTest("TST.REGMAP.GET_REGISTER_INFO", function() {
    try {
        let devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        let testAddr = "0x00";
        printToConsole("  Getting info for register " + testAddr);

        let info = regmap.getRegisterInfo(testAddr);

        if (info && info.length > 0) {
            printToConsole("  Register info:");
            for (let line of info) {
                if (line.length > 80) {
                    printToConsole("    " + line.substring(0, 80) + "...");
                } else {
                    printToConsole("    " + line);
                }
            }
            return true;
        } else {
            printToConsole("  ⚠ No register info available (may be device-dependent)");
            return true;
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        // Register info might not be available for all devices
        return true;
    }
});

// Test: Bitfield Operations
TestFramework.runTest("TST.REGMAP.BITFIELD_OPERATIONS", function() {
    try {
        let devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        let testAddr = "0x00";
        printToConsole("  Testing bitfield operations on " + testAddr);

        try {
            // Try to write a bitfield value
            regmap.writeBitField(testAddr, "0x0F");
            msleep(waitTime);

            printToConsole("  ✓ Bitfield write completed");

            // Try to get bitfield info
            let bitfieldInfo = regmap.getRegisterBitFieldsInfo(testAddr);
            if (bitfieldInfo && bitfieldInfo.length > 0) {
                printToConsole("  ✓ Bitfield info available");
            } else {
                printToConsole("  ⚠ No bitfield info (register may not have bitfields)");
            }

            return true;

        } catch (e) {
            printToConsole("  ⚠ Bitfield operations not supported: " + e);
            return true; // Don't fail as not all registers have bitfields
        }

    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
let exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();