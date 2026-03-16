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

// ============================================================================
// ALL THE FOLLOWING TESTS REQUIRE VISUAL VALIDATION
// These tests automate the steps from the manual test documentation but
// require a human observer to verify UI changes. Each step includes a 3-second
// pause to allow visual inspection of the application state.
// Source: docs/tests/plugins/registermap/registermap_tests.rst
//
// Not automatable (Category C - skipped entirely):
//   TST.REGMAP.PREFERENCES_SETTINGS_SAVE_ON_CLOSE - requires restarting Scopy
//   TST.REGMAP.INFO_BUTTON_DOCUMENTATION - opens external browser
//   TST.REGMAP.INFO_BUTTON_TUTORIAL - displays tutorial UI overlay
//   TST.REGMAP.INFO_BUTTON_TUTORIAL_NO_XML - displays tutorial UI overlay
//   TST.REGMAP.CUSTOM_XML_FILE - requires restarting Scopy and copying files
// ============================================================================

// Load test framework
evaluateFile("js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("Register Map Visual Validation Tests");

var VISUAL_DELAY = 3000; // 3 seconds for human observation

// Connect to device
if (!TestFramework.connectToDevice()) {
    printToConsole("ERROR: Cannot proceed without device connection");
    scopy.exit();
}

// Switch to Register Map tool
if (!switchToTool("Register map")) {
    printToConsole("ERROR: Cannot access Register Map tool");
    scopy.exit();
}

// ===========================================================================
// Test 5: Show register map table
// UID: TST.REGMAP.SHOW_TABLE
// Description: Verify that the user can see the register map table.
// Precondition: XML file of the device is present in the system
// VISUAL: Verify the register map table is displayed in the plugin
// ===========================================================================
printToConsole("\n=== Test 5: Show Register Map Table (VISUAL) ===\n");

TestFramework.runTest("TST.REGMAP.SHOW_TABLE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            printToConsole("  No devices available");
            return "SKIP";
        }

        printToConsole("  Available devices: " + devices.join(", "));
        printToConsole("  VISUAL CHECK: Verify the register map table is displayed in the plugin");
        msleep(VISUAL_DELAY);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 7: Simple Register value updated on read
// UID: TST.REGMAP.SIMPLE_REGISTER_VALUE_UPDATED_ON_READ
// Description: Verify that the user can see the register value in the
//   table updated on read.
// VISUAL: Verify the value in the table is updated to the new value
// ===========================================================================
printToConsole("\n=== Test 7: Simple Register Value Updated on Read (VISUAL) ===\n");

TestFramework.runTest("TST.REGMAP.SIMPLE_REGISTER_VALUE_UPDATED_ON_READ", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        printToConsole("  Reading register 0x02...");
        var value = regmap.readRegister("0x02");
        printToConsole("  Read value: " + value);

        printToConsole("  VISUAL CHECK: Verify the value in the register table is updated to: " + value);
        msleep(VISUAL_DELAY);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 8: Simple Register value updated on write
// UID: TST.REGMAP.SIMPLE_REGISTER_VALUE_UPDATED_ON_WRITE
// Description: Verify that the user can see the register value in the
//   table updated on write.
// VISUAL: Verify the value in the table is updated to the new value
// ===========================================================================
printToConsole("\n=== Test 8: Simple Register Value Updated on Write (VISUAL) ===\n");

TestFramework.runTest("TST.REGMAP.SIMPLE_REGISTER_VALUE_UPDATED_ON_WRITE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Save original value
        var original = regmap.readRegister("0x02");
        printToConsole("  Original value of 0x02: " + original);

        // Write new value
        printToConsole("  Writing 0x4a to register 0x02...");
        regmap.write("0x02", "0x4a");
        msleep(500);

        printToConsole("  VISUAL CHECK: Verify the value in the register table is updated to 0x4a");
        msleep(VISUAL_DELAY);

        // Restore original value
        regmap.write("0x02", original);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 9: Detailed Register value updated on read
// UID: TST.REGMAP.DETAILED_REGISTER_VALUE_UPDATED_ON_READ
// Description: Verify that the detailed register bitfields value is
//   updated on read.
// VISUAL: Verify the detailed register bitfields are updated
// ===========================================================================
printToConsole("\n=== Test 9: Detailed Register Value Updated on Read (VISUAL) ===\n");

TestFramework.runTest("TST.REGMAP.DETAILED_REGISTER_VALUE_UPDATED_ON_READ", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        printToConsole("  Reading register 0x02...");
        var value = regmap.readRegister("0x02");
        printToConsole("  Read value: " + value);

        // Show bitfield info if available
        var bitfields = regmap.getRegisterBitFieldsInfo("0x02");
        if (bitfields && bitfields.length > 0) {
            printToConsole("  Bitfields found: " + bitfields.length);
        }

        printToConsole("  VISUAL CHECK: Verify the detailed register bitfields are updated to reflect value: " + value);
        msleep(VISUAL_DELAY);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 10: Detailed Register value updated on write
// UID: TST.REGMAP.DETAILED_REGISTER_VALUE_UPDATED_ON_WRITE
// Description: Verify that the detailed register bitfields value is
//   updated on write.
// VISUAL: Verify the detailed register bitfields are updated
// ===========================================================================
printToConsole("\n=== Test 10: Detailed Register Value Updated on Write (VISUAL) ===\n");

TestFramework.runTest("TST.REGMAP.DETAILED_REGISTER_VALUE_UPDATED_ON_WRITE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Save original value
        var original = regmap.readRegister("0x02");
        printToConsole("  Original value of 0x02: " + original);

        // Write new value
        printToConsole("  Writing 0x4a to register 0x02...");
        regmap.write("0x02", "0x4a");
        msleep(500);

        printToConsole("  VISUAL CHECK: Verify the detailed register bitfields are updated to reflect value 0x4a");
        msleep(VISUAL_DELAY);

        // Restore original value
        regmap.write("0x02", original);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 12: Change Value field updates detailed register bitfields
// UID: TST.REGMAP.CHANGE_VALUE_FIELD
// Description: Verify that when the Value field is changed the detailed
//   register bitfields are updated.
// VISUAL: Verify bitfield "reg002_b3" changes from on to off
// ===========================================================================
printToConsole("\n=== Test 12: Change Value Field Updates Bitfields (VISUAL) ===\n");

TestFramework.runTest("TST.REGMAP.CHANGE_VALUE_FIELD", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Save original value
        var original = regmap.readRegister("0x02");
        printToConsole("  Original value of 0x02: " + original);

        // Step 3: Set value to 0x4c
        printToConsole("  Setting register 0x02 to 0x4c...");
        regmap.write("0x02", "0x4c");
        msleep(500);

        printToConsole("  VISUAL CHECK: Verify bitfield 'reg002_b3' is ON (bit 3 set in 0x4c)");
        msleep(VISUAL_DELAY);

        // Step 4: Change value to 0x44
        printToConsole("  Changing value to 0x44 (bit 3 cleared)...");
        regmap.write("0x02", "0x44");
        msleep(500);

        printToConsole("  VISUAL CHECK: Verify bitfield 'reg002_b3' changed from ON to OFF");
        msleep(VISUAL_DELAY);

        // Restore original value
        regmap.write("0x02", original);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 13: Change selected register
// UID: TST.REGMAP.CHANGE_SELECTED_REGISTER
// Description: Verify that when the register is changed the detailed
//   register bitfields are updated.
// VISUAL: Verify register selection changes in the table and address picker
// Note: +/- buttons and table click are UI-only interactions. This test
//   reads different registers to trigger selection changes.
// ===========================================================================
printToConsole("\n=== Test 13: Change Selected Register (VISUAL) ===\n");

TestFramework.runTest("TST.REGMAP.CHANGE_SELECTED_REGISTER", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Step 2: Select register 0x20
        printToConsole("  Reading register 0x20...");
        regmap.readRegister("0x20");
        msleep(500);

        printToConsole("  VISUAL CHECK: Verify register 0x20 is displayed and selected in the table");
        msleep(VISUAL_DELAY);

        // Step 3: Change to register 0x21 (simulates +button)
        printToConsole("  Reading register 0x21...");
        regmap.readRegister("0x21");
        msleep(500);

        printToConsole("  VISUAL CHECK: Verify register 0x21 is now displayed and selected in the table");
        msleep(VISUAL_DELAY);

        // Step 4: Change back to register 0x20 (simulates -button)
        printToConsole("  Reading register 0x20 again...");
        regmap.readRegister("0x20");
        msleep(500);

        printToConsole("  VISUAL CHECK: Verify register 0x20 is selected again and address picker shows 0x20");
        msleep(VISUAL_DELAY);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 14: Toggle detailed register visible
// UID: TST.REGMAP.TOGGLE_DETAILED_REGISTER_VISIBLE
// Description: Verify that the user can toggle the detailed register
//   visibility.
// VISUAL: Verify the detailed register section hides and shows
// Note: No API exists for toggling the detailed register panel. This test
//   reads a register to ensure the detailed view has data, then requires
//   manual interaction to toggle visibility.
// ===========================================================================
printToConsole("\n=== Test 14: Toggle Detailed Register Visible (VISUAL) ===\n");

TestFramework.runTest("TST.REGMAP.TOGGLE_DETAILED_REGISTER_VISIBLE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Read a register to populate the detailed view
        printToConsole("  Reading register 0x02 to populate detailed view...");
        regmap.readRegister("0x02");
        msleep(500);

        printToConsole("  VISUAL CHECK: Verify the detailed register view is visible with bitfield data");
        msleep(VISUAL_DELAY);

        printToConsole("  ACTION REQUIRED: Press the 'Toggle detailed register' button");
        printToConsole("  VISUAL CHECK: Verify the detailed register section is now hidden");
        msleep(VISUAL_DELAY);

        printToConsole("  ACTION REQUIRED: Press the 'Toggle detailed register' button again");
        printToConsole("  VISUAL CHECK: Verify the detailed register section is visible again");
        msleep(VISUAL_DELAY);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 20: Settings Dump register and write values buttons disabled
// UID: TST.REGMAP.SETTINGS_DUMP_REGIS_AND_WRITE_VALUES_DISABLED
// Description: Verify that the Register dump and Write values buttons
//   are disabled when the file path is not selected.
// VISUAL: Verify buttons are disabled/enabled based on file path
// ===========================================================================
printToConsole("\n=== Test 20: Dump/Write Buttons Disabled Without Path (VISUAL) ===\n");

TestFramework.runTest("TST.REGMAP.SETTINGS_DUMP_REGIS_AND_WRITE_VALUES_DISABLED", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Clear file path to ensure buttons should be disabled
        printToConsole("  Clearing file path...");
        regmap.setPath("");
        msleep(500);

        printToConsole("  VISUAL CHECK: Verify 'Register dump' button is disabled (no file path set)");
        msleep(VISUAL_DELAY);

        printToConsole("  VISUAL CHECK: Verify 'Write values' button is disabled (no file path set)");
        msleep(VISUAL_DELAY);

        // Set a file path to enable buttons
        var testPath = fileIO.getTempPath() + "/regmap_visual_test.csv";
        printToConsole("  Setting file path to: " + testPath);
        regmap.setPath(testPath);
        msleep(500);

        printToConsole("  VISUAL CHECK: Verify 'Register dump' and 'Write values' buttons are now enabled");
        msleep(VISUAL_DELAY);

        // Clear path again to restore
        regmap.setPath("");
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 21: Preferences color coding background color
// UID: TST.REGMAP.PREFERENCES_COLOR_CODING_BACKGROUND_COLOR
// Description: Verify that the background color of the register in the
//   table is changing to reflect the value of the register.
// VISUAL: Verify background colors change after reading a register
// Note: Color coding preferences must be set manually via Preferences >
//   RegmapPlugin > "Register background and bitfield background"
// ===========================================================================
printToConsole("\n=== Test 21: Color Coding Background Color (VISUAL) ===\n");

TestFramework.runTest("TST.REGMAP.PREFERENCES_COLOR_CODING_BACKGROUND_COLOR", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        printToConsole("  ACTION REQUIRED: Open Preferences > RegmapPlugin tab");
        printToConsole("  ACTION REQUIRED: Select 'Register background and bitfield background' from dropdown");
        msleep(VISUAL_DELAY);

        printToConsole("  Reading register 0x02...");
        regmap.readRegister("0x02");
        msleep(500);

        printToConsole("  VISUAL CHECK: Verify the background color of register 0x02 reflects its value");
        printToConsole("  VISUAL CHECK: Verify the background color of bitfields reflects their values");
        msleep(VISUAL_DELAY);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 22: Preferences color coding text color
// UID: TST.REGMAP.PREFERENCES_COLOR_CODING_TEXT_COLOR
// Description: Verify that the text color of the register in the table
//   is changing to reflect the value of the register.
// VISUAL: Verify text colors change after reading a register
// Note: Color coding preferences must be set manually via Preferences >
//   RegmapPlugin > "Register text and bitfield text"
// ===========================================================================
printToConsole("\n=== Test 22: Color Coding Text Color (VISUAL) ===\n");

TestFramework.runTest("TST.REGMAP.PREFERENCES_COLOR_CODING_TEXT_COLOR", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        printToConsole("  ACTION REQUIRED: Open Preferences > RegmapPlugin tab");
        printToConsole("  ACTION REQUIRED: Select 'Register text and bitfield text' from dropdown");
        msleep(VISUAL_DELAY);

        printToConsole("  Reading register 0x02...");
        regmap.readRegister("0x02");
        msleep(500);

        printToConsole("  VISUAL CHECK: Verify the text color of register 0x02 reflects its value");
        printToConsole("  VISUAL CHECK: Verify the text color of bitfields reflects their values");
        msleep(VISUAL_DELAY);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 23: Preferences color for value range
// UID: TST.REGMAP.PREFERENCES_COLORS_VALUE_RANGE
// Description: Verify that registers with the same value have the same
//   color coding applied.
// VISUAL: Verify bitfields with same values have same colors
// ===========================================================================
printToConsole("\n=== Test 23: Color Coding Value Range (VISUAL) ===\n");

TestFramework.runTest("TST.REGMAP.PREFERENCES_COLORS_VALUE_RANGE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        printToConsole("  ACTION REQUIRED: Ensure Preferences > RegmapPlugin > 'Register text and bitfield text' is selected");
        msleep(VISUAL_DELAY);

        // Save original values
        var orig02 = regmap.readRegister("0x02");
        var orig03 = regmap.readRegister("0x03");
        printToConsole("  Original 0x02: " + orig02 + ", 0x03: " + orig03);

        // Write same value to both registers
        printToConsole("  Writing 0x6f to register 0x02...");
        regmap.write("0x02", "0x6f");
        msleep(500);

        regmap.readRegister("0x02");
        msleep(500);

        printToConsole("  Writing 0x6f to register 0x03...");
        regmap.write("0x03", "0x6f");
        msleep(500);

        regmap.readRegister("0x03");
        msleep(500);

        printToConsole("  VISUAL CHECK: Verify bitfields with the same value have the same text color");
        msleep(VISUAL_DELAY);

        // Restore original values
        regmap.write("0x02", orig02);
        msleep(500);
        regmap.write("0x03", orig03);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ===========================================================================
// Test 24: Preferences color coding dynamic change
// UID: TST.REGMAP.PREFERENCES_COLOR_CODING_DYNAMIC_CHANGE
// Description: Verify that the color coding is changing dynamically
//   when preferences are updated.
// VISUAL: Verify colors update in real-time when preference changes
// ===========================================================================
printToConsole("\n=== Test 24: Color Coding Dynamic Change (VISUAL) ===\n");

TestFramework.runTest("TST.REGMAP.PREFERENCES_COLOR_CODING_DYNAMIC_CHANGE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Step 2-3: Read a register to have a value displayed
        printToConsole("  Reading register 0x02...");
        regmap.readRegister("0x02");
        msleep(500);

        printToConsole("  VISUAL CHECK: Note the current appearance of register 0x02 (no color coding)");
        msleep(VISUAL_DELAY);

        // Step 4-6: User must change preferences
        printToConsole("  ACTION REQUIRED: Open Preferences > RegmapPlugin tab");
        printToConsole("  ACTION REQUIRED: Select 'Register background and bitfield background' from dropdown");
        msleep(VISUAL_DELAY);

        // Step 7: Switch back to Register Map
        printToConsole("  VISUAL CHECK: Verify the background color of register 0x02 dynamically updated to reflect its value");
        msleep(VISUAL_DELAY);

        return true;
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
