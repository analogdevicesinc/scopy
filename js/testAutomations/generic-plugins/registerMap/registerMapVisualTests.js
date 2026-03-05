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
// ALL THE FOLLOWING TESTS REQUIRE SUPERVISED VISUAL VALIDATION
// These tests automate the steps from the manual test documentation but
// require a human observer to verify UI changes. After each visual check,
// the test will prompt the user to input 'y' (pass) or 'n' (fail).
// Source: docs/tests/plugins/registermap/registermap_tests.rst
// ============================================================================

// Load test framework
evaluateFile("../js/testAutomations/common/testFramework.js");

// Test Suite
TestFramework.init("Register Map Visual Validation Tests");

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
// Test 5: Show Register Map Table
// UID: TST.REGMAP.SHOW_TABLE
// Description: Verify that the user can see the register map table.
// ============================================
printToConsole("\n=== Test 5: Show Register Map Table (SUPERVISED) ===\n");

TestFramework.runTest("TST.REGMAP.SHOW_TABLE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            printToConsole("  No devices available");
            return "SKIP";
        }

        printToConsole("  Available devices: " + devices.join(", "));

        // Step 1: Open Register Map plugin (already open)
        // Expected: The register map table is displayed in the plugin
        if (!TestFramework.supervisedCheck(
            "Verify the register map table is displayed in the plugin")) {
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 7: Simple Register Value Updated on Read
// UID: TST.REGMAP.SIMPLE_REGISTER_VALUE_UPDATED_ON_READ
// Description: Verify that the user can see the register value in the
//   table updated on read.
// ============================================
printToConsole("\n=== Test 7: Simple Register Value Updated on Read (SUPERVISED) ===\n");

TestFramework.runTest("TST.REGMAP.SIMPLE_REGISTER_VALUE_UPDATED_ON_READ", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Steps 2-3: Select register 0x02 and read
        printToConsole("  Reading register 0x02...");
        var value = regmap.readRegister("0x02");
        msleep(500);
        printToConsole("  Read value: " + value);

        // Expected: The value in the table is updated to the new value
        if (!TestFramework.supervisedCheck(
            "Verify the value in the register table is updated to: " + value)) {
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 8: Simple Register Value Updated on Write
// UID: TST.REGMAP.SIMPLE_REGISTER_VALUE_UPDATED_ON_WRITE
// Description: Verify that the user can see the register value in the
//   table updated on write.
// ============================================
printToConsole("\n=== Test 8: Simple Register Value Updated on Write (SUPERVISED) ===\n");

TestFramework.runTest("TST.REGMAP.SIMPLE_REGISTER_VALUE_UPDATED_ON_WRITE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Save original value
        var original = regmap.readRegister("0x02");
        printToConsole("  Original value of 0x02: " + original);

        // Steps 3-5: Read, change to 0x4a, write
        printToConsole("  Writing 0x4a to register 0x02...");
        regmap.write("0x02", "0x4a");
        msleep(500);

        // Expected: The value in the table is updated to the new value
        if (!TestFramework.supervisedCheck(
            "Verify the value in the register table for 0x02 is updated to 0x4a")) {
            regmap.write("0x02", original);
            msleep(500);
            return false;
        }

        // Restore original value
        regmap.write("0x02", original);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 9: Detailed Register Value Updated on Read
// UID: TST.REGMAP.DETAILED_REGISTER_VALUE_UPDATED_ON_READ
// Description: Verify that the detailed register bitfields value is
//   updated on read.
// ============================================
printToConsole("\n=== Test 9: Detailed Register Value Updated on Read (SUPERVISED) ===\n");

TestFramework.runTest("TST.REGMAP.DETAILED_REGISTER_VALUE_UPDATED_ON_READ", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Steps 2-3: Select register 0x02 and read
        printToConsole("  Reading register 0x02...");
        var value = regmap.readRegister("0x02");
        msleep(500);
        printToConsole("  Read value: " + value);

        var bitfields = regmap.getRegisterBitFieldsInfo("0x02");
        if (bitfields && bitfields.length > 0) {
            printToConsole("  Bitfields found: " + bitfields.length);
        }

        // Expected: The detailed register bitfields value is updated
        if (!TestFramework.supervisedCheck(
            "Verify the detailed register bitfields are updated to reflect value: " + value)) {
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 10: Detailed Register Value Updated on Write
// UID: TST.REGMAP.DETAILED_REGISTER_VALUE_UPDATED_ON_WRITE
// Description: Verify that the detailed register bitfields value is
//   updated on write.
// ============================================
printToConsole("\n=== Test 10: Detailed Register Value Updated on Write (SUPERVISED) ===\n");

TestFramework.runTest("TST.REGMAP.DETAILED_REGISTER_VALUE_UPDATED_ON_WRITE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Save original value
        var original = regmap.readRegister("0x02");
        printToConsole("  Original value of 0x02: " + original);

        // Steps 3-5: Read, change to 0x4a, write
        printToConsole("  Writing 0x4a to register 0x02...");
        regmap.write("0x02", "0x4a");
        msleep(500);

        // Expected: The detailed register bitfields value is updated
        if (!TestFramework.supervisedCheck(
            "Verify the detailed register bitfields are updated to reflect value 0x4a")) {
            regmap.write("0x02", original);
            msleep(500);
            return false;
        }

        // Restore original value
        regmap.write("0x02", original);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 12: Change Value Field Updates Detailed Register Bitfields
// UID: TST.REGMAP.CHANGE_VALUE_FIELD
// Description: Verify that when value of the Value field is changed
//   the detailed register bitfields are updated.
// ============================================
printToConsole("\n=== Test 12: Change Value Field Updates Bitfields (SUPERVISED) ===\n");

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

        if (!TestFramework.supervisedCheck(
            "Verify bitfield 'reg002_b3' is ON (bit 3 is set in 0x4c)")) {
            regmap.write("0x02", original);
            msleep(500);
            return false;
        }

        // Step 4: Change value to 0x44 (bit 3 cleared)
        printToConsole("  Changing value to 0x44 (clearing bit 3)...");
        regmap.write("0x02", "0x44");
        msleep(500);

        // Expected: The detailed bitfield "reg002_b3" value is changed from on to off
        if (!TestFramework.supervisedCheck(
            "Verify bitfield 'reg002_b3' changed from ON to OFF (bit 3 cleared in 0x44)")) {
            regmap.write("0x02", original);
            msleep(500);
            return false;
        }

        // Restore original value
        regmap.write("0x02", original);
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 13: Change Selected Register
// UID: TST.REGMAP.CHANGE_SELECTED_REGISTER
// Description: Verify that when the register is changed the detailed
//   register bitfields are updated.
// ============================================
printToConsole("\n=== Test 13: Change Selected Register (SUPERVISED) ===\n");

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

        // Expected: Register 0x20 is displayed and selected
        if (!TestFramework.supervisedCheck(
            "Verify register 0x20 is displayed and selected in the table")) {
            return false;
        }

        // Step 3: Change to register 0x21 (simulates + button)
        printToConsole("  Reading register 0x21...");
        regmap.readRegister("0x21");
        msleep(500);

        // Expected: Register 0x21 is displayed and selected
        if (!TestFramework.supervisedCheck(
            "Verify register 0x21 is now displayed and selected in the table")) {
            return false;
        }

        // Step 4: Change back to register 0x20 (simulates - button)
        printToConsole("  Reading register 0x20 again...");
        regmap.readRegister("0x20");
        msleep(500);

        // Expected: Register 0x20 is selected, address picker shows 0x20
        if (!TestFramework.supervisedCheck(
            "Verify register 0x20 is selected again and the address picker shows 0x20")) {
            return false;
        }

        // Step 5: Click on register 0x21 in the table
        // Note: Table click is UI-only, simulating via readRegister
        printToConsole("  Reading register 0x21 (simulating table click)...");
        regmap.readRegister("0x21");
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Verify register 0x21 is selected and the address picker updated to 0x21")) {
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 14: Toggle Detailed Register Visible
// UID: TST.REGMAP.TOGGLE_DETAILED_REGISTER_VISIBLE
// Description: Verify that the user can toggle the detailed register
//   visibility.
// Note: No API exists for toggling the detailed register panel.
//   The test reads a register to populate the view, then asks the
//   user to manually press the toggle button.
// ============================================
printToConsole("\n=== Test 14: Toggle Detailed Register Visible (SUPERVISED) ===\n");

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

        if (!TestFramework.supervisedCheck(
            "Verify the detailed register view is visible with bitfield data. " +
            "Then press the 'Toggle detailed register' button to HIDE it.")) {
            return false;
        }

        // Expected: Detailed register is hidden
        if (!TestFramework.supervisedCheck(
            "Verify the detailed register section is now HIDDEN. " +
            "Then press the 'Toggle detailed register' button again to SHOW it.")) {
            return false;
        }

        // Expected: Detailed register is visible again
        if (!TestFramework.supervisedCheck(
            "Verify the detailed register section is VISIBLE again")) {
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 20: Settings Dump Register and Write Values Disabled
// UID: TST.REGMAP.SETTINGS_DUMP_REGIS_AND_WRITE_VALUES_DISABLED
// Description: Verify that the Register dump and Write values buttons
//   are disabled when the file path is not selected.
// ============================================
printToConsole("\n=== Test 20: Dump/Write Buttons Disabled Without Path (SUPERVISED) ===\n");

TestFramework.runTest("TST.REGMAP.SETTINGS_DUMP_REGIS_AND_WRITE_VALUES_DISABLED", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Step 3-4: Verify buttons disabled when no path set
        printToConsole("  Clearing file path...");
        regmap.setPath("");
        msleep(500);

        // Expected: Register dump and Write values buttons are disabled
        if (!TestFramework.supervisedCheck(
            "Open Settings. Verify 'Register dump' and 'Write values' buttons are DISABLED " +
            "(no file path is set)")) {
            return false;
        }

        // Step 5: Select a file path to enable buttons
        var testPath = fileIO.getTempPath() + "/regmap_visual_test.csv";
        printToConsole("  Setting file path to: " + testPath);
        regmap.setPath(testPath);
        msleep(500);

        // Expected: Both buttons are now enabled
        if (!TestFramework.supervisedCheck(
            "Verify 'Register dump' and 'Write values' buttons are now ENABLED")) {
            regmap.setPath("");
            msleep(500);
            return false;
        }

        // Restore: clear path
        regmap.setPath("");
        msleep(500);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 21: Preferences Color Coding Background Color
// UID: TST.REGMAP.PREFERENCES_COLOR_CODING_BACKGROUND_COLOR
// Description: Verify that the background color of the register in the
//   table is changing to reflect the value of the register.
// ============================================
printToConsole("\n=== Test 21: Color Coding Background Color (SUPERVISED) ===\n");

TestFramework.runTest("TST.REGMAP.PREFERENCES_COLOR_CODING_BACKGROUND_COLOR", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Steps 1-3: User must set Preferences > RegmapPlugin > background color coding
        if (!TestFramework.supervisedCheck(
            "Open Preferences > RegmapPlugin tab. Select 'Register background and " +
            "bitfield background' from the color coding dropdown. Then return to Register Map.")) {
            return false;
        }

        // Steps 5-6: Select register 0x02 and read
        printToConsole("  Reading register 0x02...");
        regmap.readRegister("0x02");
        msleep(500);

        // Expected: Background colors reflect register/bitfield values
        if (!TestFramework.supervisedCheck(
            "Verify the background color of register 0x02 reflects its value AND " +
            "the background color of bitfields reflects their values")) {
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 22: Preferences Color Coding Text Color
// UID: TST.REGMAP.PREFERENCES_COLOR_CODING_TEXT_COLOR
// Description: Verify that the text color of the register in the table
//   is changing to reflect the value of the register.
// ============================================
printToConsole("\n=== Test 22: Color Coding Text Color (SUPERVISED) ===\n");

TestFramework.runTest("TST.REGMAP.PREFERENCES_COLOR_CODING_TEXT_COLOR", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Steps 1-3: User must set Preferences > RegmapPlugin > text color coding
        if (!TestFramework.supervisedCheck(
            "Open Preferences > RegmapPlugin tab. Select 'Register text and " +
            "bitfield text' from the color coding dropdown. Then return to Register Map.")) {
            return false;
        }

        // Steps 5-6: Select register 0x02 and read
        printToConsole("  Reading register 0x02...");
        regmap.readRegister("0x02");
        msleep(500);

        // Expected: Text colors reflect register/bitfield values
        if (!TestFramework.supervisedCheck(
            "Verify the text color of register 0x02 reflects its value AND " +
            "the text color of bitfields reflects their values")) {
            return false;
        }

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 23: Preferences Color for Value Range
// UID: TST.REGMAP.PREFERENCES_COLORS_VALUE_RANGE
// Description: Verify that the colors used for the values are correct.
// ============================================
printToConsole("\n=== Test 23: Color Coding Value Range (SUPERVISED) ===\n");

TestFramework.runTest("TST.REGMAP.PREFERENCES_COLORS_VALUE_RANGE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Steps 1-3: Ensure text color coding is set
        if (!TestFramework.supervisedCheck(
            "Ensure Preferences > RegmapPlugin > 'Register text and bitfield text' " +
            "is selected. Then return to Register Map.")) {
            return false;
        }

        // Save original values
        var orig02 = regmap.readRegister("0x02");
        var orig03 = regmap.readRegister("0x03");
        printToConsole("  Original values: 0x02=" + orig02 + ", 0x03=" + orig03);

        // Steps 6-7: Write 0x6f to register 0x02 and read
        printToConsole("  Writing 0x6f to register 0x02...");
        regmap.write("0x02", "0x6f");
        msleep(500);
        regmap.readRegister("0x02");
        msleep(500);

        // Steps 8-10: Write 0x6f to register 0x03 and read
        printToConsole("  Writing 0x6f to register 0x03...");
        regmap.write("0x03", "0x6f");
        msleep(500);
        regmap.readRegister("0x03");
        msleep(500);

        // Expected: Bitfields with same value have same text color
        if (!TestFramework.supervisedCheck(
            "Verify that bitfields with the same value in registers 0x02 and 0x03 " +
            "have the same text color")) {
            regmap.write("0x02", orig02);
            msleep(500);
            regmap.write("0x03", orig03);
            msleep(500);
            return false;
        }

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

// ============================================
// Test 24: Preferences Color Coding Dynamic Change
// UID: TST.REGMAP.PREFERENCES_COLOR_CODING_DYNAMIC_CHANGE
// Description: Verify that the color coding is changing dynamically
//   when preferences are updated.
// ============================================
printToConsole("\n=== Test 24: Color Coding Dynamic Change (SUPERVISED) ===\n");

TestFramework.runTest("TST.REGMAP.PREFERENCES_COLOR_CODING_DYNAMIC_CHANGE", function() {
    try {
        var devices = regmap.getAvailableDevicesName();
        if (!devices || devices.length === 0) {
            return "SKIP";
        }

        // Steps 2-3: Read a register to have a value displayed
        printToConsole("  Reading register 0x02...");
        regmap.readRegister("0x02");
        msleep(500);

        if (!TestFramework.supervisedCheck(
            "Note the current appearance of register 0x02 (no color coding or current coding). " +
            "Then open Preferences > RegmapPlugin tab and select " +
            "'Register background and bitfield background' from the dropdown.")) {
            return false;
        }

        // Step 7: Switch back to Register Map
        // Expected: Background color dynamically updated
        if (!TestFramework.supervisedCheck(
            "Return to Register Map. Verify the background color of register 0x02 " +
            "dynamically updated to reflect its value")) {
            return false;
        }

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
