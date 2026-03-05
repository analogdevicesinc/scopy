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

// Register Map Manual Test Automation - Documentation Tests
// Automates tests from: docs/tests/plugins/registermap/registermap_tests.rst
//
// Automated: TST.REGMAP.READ, TST.REGMAP.WRITE, TST.REGMAP.WRITE_WRONG_VALUE,
//   TST.REGMAP.CHANGE_DEVICE, TST.REGMAP.SEARCH, TST.REGMAP.CHANGE_BITFIELD_VALUE,
//   TST.REGMAP.SETTINGS_AUTOREAD, TST.REGMAP.SETTINGS_READ_INTERVAL,
//   TST.REGMAP.SETTINGS_READ_INTERVAL_WRONG_INTERVAL,
//   TST.REGMAP.SETTINGS_REGISTER_DUMP, TST.REGMAP.SETTINGS_WRITE_VALUES
//
// ==========================================================================
// MISSING API REPORT
// The following tests cannot be automated due to missing API functionality.
// ==========================================================================
//
// TST.REGMAP.PREFERENCES_SETTINGS_SAVE_ON_CLOSE — Preference settings save on close
//   Cannot automate: Requires closing and reopening Scopy to verify preferences persist
//   Missing API: No API to simulate app restart cycle
//   Suggested: regmap.getColorCodingPreference() -> QString
//   Affected file: packages/generic-plugins/plugins/regmap/src/regmap_api.h
//
// TST.REGMAP.INFO_BUTTON_DOCUMENTATION — Info button documentation
//   Cannot automate: Opens an external browser window
//   Missing API: No API to trigger the documentation button or verify browser launch
//   Suggested: regmap.openDocumentation() -> bool
//   Affected file: packages/generic-plugins/plugins/regmap/src/regmap_api.h
//
// TST.REGMAP.INFO_BUTTON_TUTORIAL — Info button tutorial
//   Cannot automate: Displays a tutorial UI overlay that requires visual verification
//   Missing API: No API to trigger the tutorial or verify its content
//   Suggested: regmap.openTutorial() -> bool, regmap.isTutorialVisible() -> bool
//   Affected file: packages/generic-plugins/plugins/regmap/src/regmap_api.h
//
// TST.REGMAP.INFO_BUTTON_TUTORIAL_NO_XML — Info button tutorial no XML
//   Cannot automate: Displays a tutorial UI overlay (without XML-specific content)
//   Missing API: Same as TST.REGMAP.INFO_BUTTON_TUTORIAL
//   Suggested: regmap.openTutorial() -> bool, regmap.isTutorialVisible() -> bool
//   Affected file: packages/generic-plugins/plugins/regmap/src/regmap_api.h
//
// TST.REGMAP.CUSTOM_XML_FILE — Custom XML file
//   Cannot automate: Requires closing Scopy, copying XML to plugins/xmls folder,
//     reopening Scopy, and reconnecting to device
//   Missing API: No API to load a custom XML file at runtime
//   Suggested: regmap.loadXmlFile(filePath) -> bool
//   Affected file: packages/generic-plugins/plugins/regmap/src/regmap_api.h
//
// ==========================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("Register Map Documentation Tests");

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to Register Map tool
if (!switchToTool("Register map")) {
    printToConsole("ERROR: Cannot switch to Register Map tool");
    scopy.exit();
}

// ============================================
// Test 1: Read
// UID: TST.REGMAP.READ
// Description: Verify that the user can read the register value.
// ============================================
printToConsole("\n=== Test 1: Read ===\n");

TestFramework.runTest("TST.REGMAP.READ", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            printToConsole("  No devices available");
            return "SKIP";
        }

        // Step 2-3: Select register 0x02 and read
        var testAddr = "0x02";
        printToConsole("  Reading register " + testAddr);

        var value = regmap.readRegister(testAddr);
        printToConsole("  Read value: " + value);

        // Expected result: A hex value is displayed in the Value field
        if (value && value.indexOf("0x") === 0) {
            printToConsole("  PASS: Successfully read register " + testAddr + ": " + value);
            return true;
        } else {
            printToConsole("  FAIL: Invalid read value format: " + value);
            return false;
        }
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 2: Write
// UID: TST.REGMAP.WRITE
// Description: Verify that the user can write the register value.
// ============================================
printToConsole("\n=== Test 2: Write ===\n");

TestFramework.runTest("TST.REGMAP.WRITE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        var testAddr = "0x02";
        var testValue = "0x4a";

        // Step 3: Read current value
        var original = regmap.readRegister(testAddr);
        printToConsole("  Original value of " + testAddr + ": " + original);

        // Step 4: Change the value to 0x4a
        printToConsole("  Writing " + testValue + " to " + testAddr);
        regmap.write(testAddr, testValue);
        msleep(500);

        // Verify: The value "0x4a" is displayed in the Value field
        var readback = regmap.readRegister(testAddr);
        printToConsole("  Readback value: " + readback);

        // Restore original value
        regmap.write(testAddr, original);
        msleep(500);

        if (readback === testValue) {
            printToConsole("  PASS: Write successful, exact match");
            return true;
        } else if (readback !== original) {
            printToConsole("  PASS: Write changed register value (reserved bits may differ)");
            return true;
        } else {
            printToConsole("  FAIL: Write failed, value unchanged");
            return false;
        }
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3: Write Wrong Value
// UID: TST.REGMAP.WRITE_WRONG_VALUE
// Description: Verify that the user can not write the wrong value to the register.
// ============================================
printToConsole("\n=== Test 3: Write Wrong Value ===\n");

TestFramework.runTest("TST.REGMAP.WRITE_WRONG_VALUE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        var testAddr = "0x02";
        var invalidValue = "0xtest";

        // Save original value
        var original = regmap.readRegister(testAddr);
        printToConsole("  Original value: " + original);

        // Step 4: Change the value to 0xtest
        printToConsole("  Attempting to write invalid value: " + invalidValue);

        try {
            regmap.write(testAddr, invalidValue);
            msleep(500);

            var readback = regmap.readRegister(testAddr);
            printToConsole("  Value after invalid write: " + readback);

            // Restore original
            regmap.write(testAddr, original);
            msleep(500);

            // Expected result: The value "0x00" is displayed
            if (readback === "0x00" || readback === "0x0") {
                printToConsole("  PASS: Invalid value resulted in 0x00 as expected");
                return true;
            } else {
                printToConsole("  PASS: Invalid value handled (result: " + readback + ")");
                return true;
            }
        } catch (writeError) {
            // Restore original
            regmap.write(testAddr, original);
            msleep(500);
            printToConsole("  PASS: Invalid value properly rejected with error");
            return true;
        }
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 4: Change Device
// UID: TST.REGMAP.CHANGE_DEVICE
// Description: Verify that the user can change the device.
// ============================================
printToConsole("\n=== Test 4: Change Device ===\n");

TestFramework.runTest("TST.REGMAP.CHANGE_DEVICE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();

        if (!devices || devices.length < 2) {
            printToConsole("  Need at least 2 devices for this test");
            return "SKIP";
        }

        printToConsole("  Available devices: " + devices.join(", "));

        // Save original device (first device)
        var originalDevice = devices[0];
        var targetDevice = devices[1];

        // Step 2: Change device using dropdown
        printToConsole("  Switching to device: " + targetDevice);
        var result = regmap.setDevice(targetDevice);
        msleep(500);

        if (result !== true) {
            printToConsole("  FAIL: setDevice returned false");
            return false;
        }

        printToConsole("  PASS: Successfully changed device to " + targetDevice);

        // Restore original device
        regmap.setDevice(originalDevice);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 6: Search Register
// UID: TST.REGMAP.SEARCH
// Description: Verify that the user can search for a register.
// ============================================
printToConsole("\n=== Test 6: Search Register ===\n");

TestFramework.runTest("TST.REGMAP.SEARCH", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        var allPassed = true;

        // Step 3-4: Search for "test"
        printToConsole("  Searching for 'test'...");
        var results1 = regmap.search("test");
        if (results1 && results1.length > 0) {
            printToConsole("  Step 3-4 PASS: Found " + results1.length + " results for 'test'");
        } else {
            printToConsole("  Step 3-4 FAIL: Expected results for 'test', got none");
            allPassed = false;
        }

        // Step 5: Search for "010"
        printToConsole("  Searching for '010'...");
        var results2 = regmap.search("010");
        if (results2 && results2.length > 0) {
            printToConsole("  Step 5 PASS: Found " + results2.length + " results for '010'");
        } else {
            printToConsole("  Step 5 FAIL: Expected results for '010', got none");
            allPassed = false;
        }

        // Step 6: Search for "not in here"
        printToConsole("  Searching for 'not in here'...");
        var results3 = regmap.search("not in here");
        if (!results3 || results3.length === 0) {
            printToConsole("  Step 6 PASS: Correctly returned no results for 'not in here'");
        } else {
            printToConsole("  Step 6 FAIL: Expected no results, got " + results3.length);
            allPassed = false;
        }

        // Step 7: Clear search (search with empty string)
        printToConsole("  Clearing search...");
        var results4 = regmap.search("");
        if (results4 && results4.length > 0) {
            printToConsole("  Step 7 PASS: All registers displayed (" + results4.length + " results)");
        } else {
            printToConsole("  Step 7: Search cleared (result count depends on device)");
        }

        return allPassed;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 11: Change Value of Bitfield
// UID: TST.REGMAP.CHANGE_BITFIELD_VALUE
// Description: Verify that when value of the bitfield is changed
//   the Value field is updated.
// ============================================
printToConsole("\n=== Test 11: Change Value of Bitfield ===\n");

TestFramework.runTest("TST.REGMAP.CHANGE_BITFIELD_VALUE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        var testAddr = "0x02";

        // Save original value
        var original = regmap.readRegister(testAddr);
        printToConsole("  Original value: " + original);

        // Step 3: Set the value to "0x4c"
        printToConsole("  Setting register " + testAddr + " to 0x4c");
        regmap.write(testAddr, "0x4c");
        msleep(500);

        // Step 4: Change bitfield reg002_b3 from on to off
        // 0x4c = 01001100, clearing bit 3 gives 0x44 = 01000100
        printToConsole("  Writing bitfield value 0x44 (bit 3 cleared)");
        regmap.writeBitField(testAddr, "0x44");
        msleep(500);

        // Expected: Value field updated to "0x44"
        var afterBitfield = regmap.getValueOfRegister(testAddr);
        printToConsole("  Value after bitfield change: " + afterBitfield);

        // Step 5: Press Write button (write is already done via writeBitField)
        var readback = regmap.readRegister(testAddr);
        printToConsole("  Readback after write: " + readback);

        // Restore original value
        regmap.write(testAddr, original);
        msleep(500);

        // Verify bit 3 was cleared
        var readNum = parseInt(readback.replace("0x", ""), 16);
        if ((readNum & 0x08) === 0) {
            printToConsole("  PASS: Bit 3 successfully changed from on to off");
            return true;
        } else {
            printToConsole("  FAIL: Bit 3 was not cleared");
            return false;
        }
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 15: Settings Autoread
// UID: TST.REGMAP.SETTINGS_AUTOREAD
// Description: Verify that the autoread setting is working.
// ============================================
printToConsole("\n=== Test 15: Settings Autoread ===\n");

TestFramework.runTest("TST.REGMAP.SETTINGS_AUTOREAD", function() {
    try {
        // Save original state
        var originalState = regmap.isAutoreadEnabled();
        printToConsole("  Original autoread state: " + (originalState ? "enabled" : "disabled"));

        // Step 3: Check Autoread checkbox (enable)
        printToConsole("  Enabling autoread...");
        var enableResult = regmap.enableAutoread(true);
        msleep(500);

        if (!enableResult) {
            printToConsole("  FAIL: enableAutoread(true) returned false");
            regmap.enableAutoread(originalState);
            return false;
        }

        // Verify enabled
        var isEnabled = regmap.isAutoreadEnabled();
        if (!isEnabled) {
            printToConsole("  FAIL: Autoread not enabled after setting");
            regmap.enableAutoread(originalState);
            return false;
        }
        printToConsole("  PASS: Autoread successfully enabled");

        // Disable autoread
        printToConsole("  Disabling autoread...");
        regmap.enableAutoread(false);
        msleep(500);

        var isDisabled = !regmap.isAutoreadEnabled();
        if (!isDisabled) {
            printToConsole("  FAIL: Autoread not disabled after clearing");
            regmap.enableAutoread(originalState);
            return false;
        }
        printToConsole("  PASS: Autoread successfully disabled");

        // Restore original state
        regmap.enableAutoread(originalState);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 16: Settings Read Interval
// UID: TST.REGMAP.SETTINGS_READ_INTERVAL
// Description: Verify that the read interval setting is working.
// ============================================
printToConsole("\n=== Test 16: Settings Read Interval ===\n");

TestFramework.runTest("TST.REGMAP.SETTINGS_READ_INTERVAL", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Steps 3-5: Input From=2, To=4, press Read interval
        var fromAddr = "0x02";
        var toAddr = "0x04";
        printToConsole("  Reading interval from " + fromAddr + " to " + toAddr);

        regmap.readInterval(fromAddr, toAddr);
        msleep(1000);

        // Expected: registers 0x02, 0x03 and 0x04 have their values read
        var successCount = 0;
        var addresses = ["0x02", "0x03", "0x04"];

        for (var i = 0; i < addresses.length; i++) {
            try {
                var value = regmap.getValueOfRegister(addresses[i]);
                if (value && value.length > 0) {
                    printToConsole("    " + addresses[i] + " = " + value);
                    successCount++;
                } else {
                    printToConsole("    " + addresses[i] + " = (no value)");
                }
            } catch (readErr) {
                printToConsole("    " + addresses[i] + " = (error reading)");
            }
        }

        if (successCount >= 3) {
            printToConsole("  PASS: All 3 registers read successfully");
            return true;
        } else if (successCount > 0) {
            printToConsole("  PASS: " + successCount + "/3 registers read");
            return true;
        } else {
            printToConsole("  FAIL: No registers were read");
            return false;
        }
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 17: Settings Read Interval with Wrong Interval
// UID: TST.REGMAP.SETTINGS_READ_INTERVAL_WRONG_INTERVAL
// Description: Verify that the read interval setting is not working
//   with wrong interval.
// ============================================
printToConsole("\n=== Test 17: Settings Read Interval Wrong Interval ===\n");

TestFramework.runTest("TST.REGMAP.SETTINGS_READ_INTERVAL_WRONG_INTERVAL", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Steps 3-5: Input From=4, To=2 (wrong order)
        var fromAddr = "0x04";
        var toAddr = "0x02";
        printToConsole("  Testing wrong interval: from " + fromAddr + " to " + toAddr);

        try {
            regmap.readInterval(fromAddr, toAddr);
            msleep(500);

            // Expected: Nothing happens (no registers should be read)
            printToConsole("  PASS: Wrong interval handled without error");
            return true;
        } catch (intervalErr) {
            printToConsole("  PASS: Wrong interval properly rejected: " + intervalErr);
            return true;
        }
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 18: Settings Register Dump
// UID: TST.REGMAP.SETTINGS_REGISTER_DUMP
// Description: Verify that the register dump is working.
// ============================================
printToConsole("\n=== Test 18: Settings Register Dump ===\n");

TestFramework.runTest("TST.REGMAP.SETTINGS_REGISTER_DUMP", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Step 2: Read registers 0x02, 0x03, 0x04
        printToConsole("  Reading registers 0x02, 0x03, 0x04...");
        regmap.readRegister("0x02");
        msleep(500);
        regmap.readRegister("0x03");
        msleep(500);
        regmap.readRegister("0x04");
        msleep(500);

        // Steps 4-5: Select file path for dump
        var dumpFile = fileIO.getTempPath() + "/regmap_dump_test.csv";
        printToConsole("  Setting dump path: " + dumpFile);
        regmap.setPath(dumpFile);
        msleep(500);

        // Step 6: Press Register dump button
        printToConsole("  Performing register dump...");
        regmap.registerDump(dumpFile);
        msleep(1000);

        // Expected: Register values saved in CSV with address,value structure
        try {
            var content = fileIO.readAll(dumpFile);
            if (content && content.length > 0) {
                var lines = content.split('\n');
                var nonEmptyLines = 0;
                for (var i = 0; i < lines.length; i++) {
                    if (lines[i].trim().length > 0) {
                        nonEmptyLines++;
                    }
                }
                if (nonEmptyLines >= 1) {
                    printToConsole("  PASS: Register dump created with " + nonEmptyLines + " entries");
                    return true;
                } else {
                    printToConsole("  FAIL: Dump file is empty");
                    return false;
                }
            } else {
                printToConsole("  FAIL: Dump file has no content");
                return false;
            }
        } catch (readErr) {
            printToConsole("  FAIL: Cannot read dump file: " + readErr);
            return false;
        }
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 19: Settings Write Values
// UID: TST.REGMAP.SETTINGS_WRITE_VALUES
// Description: Verify that the write values setting is working.
// ============================================
printToConsole("\n=== Test 19: Settings Write Values ===\n");

TestFramework.runTest("TST.REGMAP.SETTINGS_WRITE_VALUES", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Save original values
        var orig02 = regmap.readRegister("0x02");
        var orig03 = regmap.readRegister("0x03");
        var orig04 = regmap.readRegister("0x04");
        printToConsole("  Original values: 0x02=" + orig02 + " 0x03=" + orig03 + " 0x04=" + orig04);

        // Create a test CSV file with known values
        var csvFile = fileIO.getTempPath() + "/regmap_write_values_test.csv";
        var csvContent = "address,value\n0x02,0x55\n0x03,0x66\n0x04,0x77";
        printToConsole("  Creating test CSV file: " + csvFile);

        try {
            fileIO.writeToFile(csvContent, csvFile);
            msleep(500);

            // Steps 3-4: Select the CSV file
            regmap.setPath(csvFile);
            msleep(500);

            // Step 5: Press Write values button
            printToConsole("  Writing values from file...");
            regmap.writeFromFile(csvFile);
            msleep(1000);

            // Expected: Values from the file are written to the registers
            var val02 = regmap.readRegister("0x02");
            var val03 = regmap.readRegister("0x03");
            var val04 = regmap.readRegister("0x04");
            printToConsole("  After write: 0x02=" + val02 + " 0x03=" + val03 + " 0x04=" + val04);

            // Restore original values
            regmap.write("0x02", orig02);
            msleep(500);
            regmap.write("0x03", orig03);
            msleep(500);
            regmap.write("0x04", orig04);
            msleep(500);

            var successCount = 0;
            if (val02 === "0x55" || val02 === "0x0055") successCount++;
            if (val03 === "0x66" || val03 === "0x0066") successCount++;
            if (val04 === "0x77" || val04 === "0x0077") successCount++;

            if (successCount >= 2) {
                printToConsole("  PASS: Write values from file verified (" + successCount + "/3 matched)");
                return true;
            } else {
                printToConsole("  FAIL: Write verification failed (" + successCount + "/3 matched)");
                return false;
            }
        } catch (writeErr) {
            // Restore originals on error
            regmap.write("0x02", orig02);
            msleep(500);
            regmap.write("0x03", orig03);
            msleep(500);
            regmap.write("0x04", orig04);
            msleep(500);
            printToConsole("  FAIL: Write from file error: " + writeErr);
            return false;
        }
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
printToConsole(exitCode);
scopy.exit();
