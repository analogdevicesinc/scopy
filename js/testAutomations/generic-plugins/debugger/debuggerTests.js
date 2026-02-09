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

// Debugger Plugin Automated Tests
// Tests: TST.DBG.EXPLR.LOAD, TST.DBG.EXPLR.NAV, TST.DBG.EXPLR.FILTER, TST.DBG.EXPLR.WATCH, TST.DBG.EXPLR.READ_ALL, TST.DBG.EXPLR.LOG, TST.DBG.CODEGEN

// Load test framework
evaluateFile("../scopy/js/testAutomations/common/testFramework.js");

// Test Suite: Debugger Tests
TestFramework.init("Debugger Plugin Tests");

// Device URI for testing
var deviceUri = "ip:192.168.2.1";

// ============================================
// Test 1 - Loading the Debugger (TST.DBG.EXPLR.LOAD)
// ============================================
printToConsole("\n=== Test 1 - Loading the Debugger ===\n");

TestFramework.runTest("TST.DBG.EXPLR.LOAD", function() {
    try {
        // Step 1: Verify compatible plugins for the device URI
        // This simulates clicking "Verify" button after adding URI
        printToConsole("  Step 1: Checking compatible plugins for " + deviceUri);
        var compatiblePlugins = scopy.getPlugins(deviceUri, "iio");

        if (compatiblePlugins === null || compatiblePlugins.length === 0) {
            printToConsole("  FAIL: No compatible plugins found for " + deviceUri);
            return false;
        }
        printToConsole("  Compatible plugins: " + compatiblePlugins.join(", "));

        // Step 2: Verify Debugger is in the list of compatible plugins
        var hasDebugger = false;
        for (var i = 0; i < compatiblePlugins.length; i++) {
            if (compatiblePlugins[i].toLowerCase().indexOf("debugger") !== -1) {
                hasDebugger = true;
                break;
            }
        }

        if (!hasDebugger) {
            printToConsole("  FAIL: Debugger plugin not found in compatible plugins list");
            return false;
        }
        printToConsole("  Step 2: Debugger plugin found in compatible plugins list");

        // Step 3: Add and connect to device, verify Debugger appears in tool menu
        printToConsole("  Step 3: Adding and connecting to device...");
        var deviceId = scopy.addDevice(deviceUri);
        if (!deviceId) {
            printToConsole("  FAIL: Could not add device");
            return false;
        }
        msleep(500);

        if (!scopy.connectDevice(deviceId)) {
            printToConsole("  FAIL: Could not connect to device");
            scopy.removeDevice(deviceUri);
            return false;
        }
        msleep(2000); // Wait for plugins to load

        var tools = scopy.getTools();
        printToConsole("  Available tools after connect: " + tools.join(", "));

        var debuggerInTools = false;
        for (var i = 0; i < tools.length; i++) {
            if (tools[i].toLowerCase().indexOf("debugger") !== -1) {
                debuggerInTools = true;
                break;
            }
        }

        if (!debuggerInTools) {
            printToConsole("  FAIL: Debugger not found in tool menu after connecting");
            scopy.disconnectDevice(deviceId);
            scopy.removeDevice(deviceUri);
            return false;
        }
        printToConsole("  Step 3: Debugger appears in tool menu after connecting");

        // Step 4: Disconnect and verify Debugger disappears from tool menu
        printToConsole("  Step 4: Disconnecting device...");
        scopy.disconnectDevice(deviceId);
        msleep(1000);

        tools = scopy.getTools();
        printToConsole("  Available tools after disconnect: " + (tools.length > 0 ? tools.join(", ") : "(none)"));

        debuggerInTools = false;
        for (var i = 0; i < tools.length; i++) {
            if (tools[i].toLowerCase().indexOf("debugger") !== -1) {
                debuggerInTools = true;
                break;
            }
        }

        if (debuggerInTools) {
            printToConsole("  FAIL: Debugger still in tool menu after disconnecting");
            scopy.removeDevice(deviceUri);
            return false;
        }
        printToConsole("  Step 4: Debugger disappeared from tool menu after disconnecting");

        // Step 5: Reconnect and verify Debugger reappears
        printToConsole("  Step 5: Reconnecting device...");
        if (!scopy.connectDevice(deviceId)) {
            printToConsole("  FAIL: Could not reconnect to device");
            scopy.removeDevice(deviceUri);
            return false;
        }
        msleep(2000);

        tools = scopy.getTools();
        printToConsole("  Available tools after reconnect: " + tools.join(", "));

        debuggerInTools = false;
        for (var i = 0; i < tools.length; i++) {
            if (tools[i].toLowerCase().indexOf("debugger") !== -1) {
                debuggerInTools = true;
                break;
            }
        }

        if (!debuggerInTools) {
            printToConsole("  FAIL: Debugger not found in tool menu after reconnecting");
            scopy.disconnectDevice(deviceId);
            scopy.removeDevice(deviceUri);
            return false;
        }
        printToConsole("  Step 5: Debugger reappeared in tool menu after reconnecting");

        // Step 6: Disconnect again and verify Debugger disappears
        printToConsole("  Step 6: Disconnecting device again...");
        scopy.disconnectDevice(deviceId);
        msleep(1000);

        tools = scopy.getTools();
        printToConsole("  Available tools after final disconnect: " + (tools.length > 0 ? tools.join(", ") : "(none)"));

        debuggerInTools = false;
        for (var i = 0; i < tools.length; i++) {
            if (tools[i].toLowerCase().indexOf("debugger") !== -1) {
                debuggerInTools = true;
                break;
            }
        }

        if (debuggerInTools) {
            printToConsole("  FAIL: Debugger still in tool menu after final disconnect");
            scopy.removeDevice(deviceUri);
            return false;
        }
        printToConsole("  Step 6: Debugger disappeared from tool menu after final disconnect");

        // Cleanup: Remove device
        scopy.removeDevice(deviceUri);
        printToConsole("  Cleanup: Device removed");
        msleep(2000);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Setup for remaining tests - Connect to device
// ============================================
printToConsole("\n=== Setting up for remaining tests ===\n");

// Connect to device for the remaining tests
if (!TestFramework.connectToDevice(deviceUri)) {
    printToConsole("ERROR: Cannot proceed without device connection");
    exit(1);
}

// Switch to Debugger tool
if (!switchToTool("Debugger")) {
    printToConsole("ERROR: Could not switch to Debugger tool");
    TestFramework.disconnectFromDevice();
    exit(1);
}

// Wait for debugger to initialize
msleep(1000);

// ============================================
// Test 2 - Navigation Tree Operations (TST.DBG.EXPLR.NAV)
// ============================================
printToConsole("\n=== Test 2 - Navigation Tree Operations ===\n");

TestFramework.runTest("TST.DBG.EXPLR.NAV", function() {
    try {
        // Step 1: Verify navigation tree exists and has devices
        var visibleCount = iioExplorer.getVisibleItemCount();
        if (visibleCount === 0) {
            printToConsole("  FAIL: Navigation tree is empty, no devices displayed");
            return false;
        }
        printToConsole("  Step 1: Navigation tree has " + visibleCount + " visible items");

        // Step 2: Click on context element and verify details are shown
        var contextPath = "context0";
        if (!iioExplorer.selectItemByPath(contextPath)) {
            printToConsole("  FAIL: Could not select context element");
            return false;
        }
        msleep(1000);

        var selectedName = iioExplorer.getSelectedItemName();
        var selectedType = iioExplorer.getSelectedItemType();
        if (!selectedName || !selectedType) {
            printToConsole("  FAIL: Details not displayed for context element");
            return false;
        }
        printToConsole("  Step 2: Selected context - Name: " + selectedName + ", Type: " + selectedType);

        // Step 3: Click on device element and verify details
        var devicePath = "context0/ad9361-phy";
        if (!iioExplorer.selectItemByPath(devicePath)) {
            printToConsole("  FAIL: Could not select device element");
            return false;
        }
        msleep(1000);

        selectedName = iioExplorer.getSelectedItemName();
        selectedType = iioExplorer.getSelectedItemType();
        printToConsole("  Step 3: Selected device - Name: " + selectedName + ", Type: " + selectedType);

        // Step 4: Expand device (simulate double-click) to show channels
        // First collapse to ensure we start from collapsed state
        iioExplorer.collapseItem(devicePath);
        msleep(1000);

        var isExpandedBefore = iioExplorer.isItemExpanded(devicePath);
        printToConsole("  Step 4a: Device expanded before: " + isExpandedBefore);

        if (!iioExplorer.expandItem(devicePath)) {
            printToConsole("  FAIL: Could not expand device");
            return false;
        }
        msleep(1000);

        var isExpandedAfter = iioExplorer.isItemExpanded(devicePath);
        if (!isExpandedAfter) {
            printToConsole("  FAIL: Device not expanded after expandItem call");
            return false;
        }
        printToConsole("  Step 4b: Device expanded after: " + isExpandedAfter);

        // Verify channels are visible (check child count)
        var channelCount = iioExplorer.getChildCount(devicePath);
        if (channelCount === 0) {
            printToConsole("  FAIL: No channels found under device");
            return false;
        }
        var channelNames = iioExplorer.getChildNames(devicePath);
        printToConsole("  Step 4c: Device has " + channelCount + " children (channels)");
        printToConsole("  Step 4d: Channel names: " + channelNames.slice(0, 5).join(", ") + (channelCount > 5 ? "..." : ""));

        // Step 5: Click on a channel from the list
        var channelPath = "context0/ad9361-phy/voltage0";
        if (!iioExplorer.selectItemByPath(channelPath)) {
            printToConsole("  FAIL: Could not select channel");
            return false;
        }
        msleep(1000);

        selectedName = iioExplorer.getSelectedItemName();
        selectedType = iioExplorer.getSelectedItemType();
        printToConsole("  Step 5: Selected channel - Name: " + selectedName + ", Type: " + selectedType);

        // Step 6: Click on another channel
        var channel2Path = "context0/ad9361-phy/voltage1";
        if (!iioExplorer.selectItemByPath(channel2Path)) {
            printToConsole("  Warning: Could not select voltage1 channel, trying another");
            channel2Path = "context0/ad9361-phy/altvoltage0";
            if (!iioExplorer.selectItemByPath(channel2Path)) {
                printToConsole("  FAIL: Could not select any other channel");
                return false;
            }
        }
        msleep(1000);

        selectedName = iioExplorer.getSelectedItemName();
        selectedType = iioExplorer.getSelectedItemType();
        printToConsole("  Step 6: Selected another channel - Name: " + selectedName + ", Type: " + selectedType);

        // Step 7: Expand channel to show attributes
        iioExplorer.collapseItem(channelPath);
        msleep(1000);

        if (!iioExplorer.expandItem(channelPath)) {
            printToConsole("  FAIL: Could not expand channel");
            return false;
        }
        msleep(300);

        var attrCount = iioExplorer.getChildCount(channelPath);
        if (attrCount === 0) {
            printToConsole("  FAIL: No attributes found under channel");
            return false;
        }
        var attrNames = iioExplorer.getChildNames(channelPath);
        printToConsole("  Step 7: Channel has " + attrCount + " attributes");
        printToConsole("  Step 7b: Attribute names: " + attrNames.slice(0, 5).join(", ") + (attrCount > 5 ? "..." : ""));

        // Step 8: Click on an attribute
        var attrPath = "context0/ad9361-phy/voltage0/hardwaregain";
        if (!iioExplorer.selectItemByPath(attrPath)) {
            // Try another attribute if hardwaregain doesn't exist
            attrPath = channelPath + "/" + attrNames[0];
            if (!iioExplorer.selectItemByPath(attrPath)) {
                printToConsole("  FAIL: Could not select any attribute");
                return false;
            }
        }
        msleep(1000);

        selectedName = iioExplorer.getSelectedItemName();
        selectedType = iioExplorer.getSelectedItemType();
        printToConsole("  Step 8: Selected attribute - Name: " + selectedName + ", Type: " + selectedType);

        // Step 9: Click on another attribute
        if (attrNames.length > 1) {
            var attr2Path = channelPath + "/" + attrNames[1];
            if (iioExplorer.selectItemByPath(attr2Path)) {
                msleep(300);
                selectedName = iioExplorer.getSelectedItemName();
                selectedType = iioExplorer.getSelectedItemType();
                printToConsole("  Step 9: Selected another attribute - Name: " + selectedName + ", Type: " + selectedType);
            }
        }

        // Step 10: Collapse channel (simulate double-click)
        if (!iioExplorer.collapseItem(channelPath)) {
            printToConsole("  FAIL: Could not collapse channel");
            return false;
        }
        msleep(1000);

        var isChannelExpanded = iioExplorer.isItemExpanded(channelPath);
        if (isChannelExpanded) {
            printToConsole("  FAIL: Channel still expanded after collapse");
            return false;
        }
        printToConsole("  Step 10: Channel collapsed successfully");

        // Step 11: Collapse device (simulate double-click)
        if (!iioExplorer.collapseItem(devicePath)) {
            printToConsole("  FAIL: Could not collapse device");
            return false;
        }
        msleep(300);

        var isDeviceExpanded = iioExplorer.isItemExpanded(devicePath);
        if (isDeviceExpanded) {
            printToConsole("  FAIL: Device still expanded after collapse");
            return false;
        }
        printToConsole("  Step 11: Device collapsed successfully");

        // Cleanup: expand context back to normal state
        iioExplorer.expandItem(contextPath);
        msleep(200);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Test 3 - Filter Bar Operations (TST.DBG.EXPLR.FILTER)
// ============================================
printToConsole("\n=== Test 3 - Filter Bar Operations ===\n");

TestFramework.runTest("TST.DBG.EXPLR.FILTER", function() {
    try {
        // Step 1: Verify the filter bar has placeholder text "Type to filter"
        var placeholderText = iioExplorer.getSearchPlaceholderText();
        printToConsole("  Step 1: Filter bar placeholder text: '" + placeholderText + "'");

        // Note: The actual placeholder may vary, but it should contain "filter" or similar
        if (placeholderText.toLowerCase().indexOf("filter") === -1 &&
            placeholderText.toLowerCase().indexOf("search") === -1 &&
            placeholderText.toLowerCase().indexOf("type") === -1) {
            printToConsole("  Warning: Placeholder text may not be as expected");
        }

        // Step 2: Clear filter and get initial item count
        iioExplorer.setSearchText("");
        msleep(1000);

        var initialCount = iioExplorer.getVisibleItemCount();
        if (initialCount === 0) {
            printToConsole("  FAIL: Navigation tree is empty before filtering");
            return false;
        }
        printToConsole("  Step 2: Initial visible item count: " + initialCount);

        // Step 3: Type a string that is NOT present in the navigation tree
        var nonExistentString = "xyz_nonexistent_item_12345";
        iioExplorer.setSearchText(nonExistentString);
        msleep(1000);

        var countAfterNonExistent = iioExplorer.getVisibleItemCount();
        if (countAfterNonExistent !== 0) {
            printToConsole("  FAIL: Expected 0 visible items after filtering with non-existent string, got " + countAfterNonExistent);
            return false;
        }
        printToConsole("  Step 3: Filtered with '" + nonExistentString + "' - visible items: " + countAfterNonExistent + " (expected 0)");

        // Step 4: Clear the filter bar
        iioExplorer.setSearchText("");
        msleep(1000);

        var countAfterClear = iioExplorer.getVisibleItemCount();
        if (countAfterClear !== initialCount) {
            printToConsole("  Warning: Item count after clearing filter (" + countAfterClear + ") differs from initial count (" + initialCount + ")");
        }
        if (countAfterClear === 0) {
            printToConsole("  FAIL: Navigation tree is empty after clearing filter");
            return false;
        }
        printToConsole("  Step 4: Cleared filter - visible items restored: " + countAfterClear);

        // Step 5: Type a string that IS present in the navigation tree (device name)
        var existingString1 = "ad9361";
        iioExplorer.setSearchText(existingString1);
        msleep(1000);

        var countAfterExisting1 = iioExplorer.getVisibleItemCount();
        if (countAfterExisting1 === 0) {
            printToConsole("  FAIL: Expected some visible items after filtering with '" + existingString1 + "', got 0");
            return false;
        }
        if (countAfterExisting1 >= initialCount) {
            printToConsole("  Warning: Filter did not reduce item count. Before: " + initialCount + ", After: " + countAfterExisting1);
        }
        printToConsole("  Step 5: Filtered with '" + existingString1 + "' - visible items: " + countAfterExisting1);

        // Step 6: Clear and repeat with another string (channel name)
        iioExplorer.setSearchText("");
        msleep(1000);

        var existingString2 = "voltage";
        iioExplorer.setSearchText(existingString2);
        msleep(1000);

        var countAfterExisting2 = iioExplorer.getVisibleItemCount();
        if (countAfterExisting2 === 0) {
            printToConsole("  FAIL: Expected some visible items after filtering with '" + existingString2 + "', got 0");
            return false;
        }
        printToConsole("  Step 6: Filtered with '" + existingString2 + "' - visible items: " + countAfterExisting2);

        // Step 7: Clear and repeat with an attribute name
        iioExplorer.setSearchText("");
        msleep(1000);

        var existingString3 = "hardwaregain";
        iioExplorer.setSearchText(existingString3);
        msleep(1000);

        var countAfterExisting3 = iioExplorer.getVisibleItemCount();
        if (countAfterExisting3 === 0) {
            printToConsole("  FAIL: Expected some visible items after filtering with '" + existingString3 + "', got 0");
            return false;
        }
        printToConsole("  Step 7: Filtered with '" + existingString3 + "' - visible items: " + countAfterExisting3);

        // Cleanup: Clear the filter
        iioExplorer.setSearchText("");
        msleep(1000);

        var finalCount = iioExplorer.getVisibleItemCount();
        printToConsole("  Cleanup: Filter cleared - final visible items: " + finalCount);

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        iioExplorer.setSearchText("");
        return false;
    }
});

// ============================================
// Test 4 - Title Bar (TST.DBG.EXPLR.TITLE)
// ============================================
printToConsole("\n=== Test 4 - Title Bar ===\n");

TestFramework.runTest("TST.DBG.EXPLR.TITLE", function() {
    printToConsole("\n=== Test 4 - The title bar updates when selecting different elements ===\n");
    printToConsole("  NOTE: TST.DBG.EXPLR.TITLE requires manual testing (UI interaction - title bar visual verification)");
    return "SKIP";
});

// ============================================
// Test 5 - Watchlist Operations (TST.DBG.EXPLR.WATCH)
// ============================================
printToConsole("\n=== Test 5 - Watchlist Operations ===\n");

TestFramework.runTest("TST.DBG.EXPLR.WATCH", function() {
    try {
        // Step 1: Clear any existing watchlist items
        iioExplorer.clearWatchlist();
        msleep(1000);

        var paths = iioExplorer.getWatchlistPaths();
        if (paths.length !== 0) {
            printToConsole("  FAIL: Watchlist not cleared, has " + paths.length + " items");
            return false;
        }
        printToConsole("  Step 1: Watchlist cleared - table should have 4 columns (Name, Value, Type, Path) plus X button column");

        // Step 2: Select an element and verify title bar updates
        // hardwaregain cannot be modified,must use another attribute
        var testPath1 = "context0/ad9361-phy/calib_mode";
        if (!iioExplorer.selectItemByPath(testPath1)) {
            printToConsole("  FAIL: Could not select item: " + testPath1);
            return false;
        }
        msleep(1000);
        printToConsole("  Step 2: Selected element - title bar should show 'hardwaregain' with green plus sign");

        // Step 3: Add to watchlist using addCurrentItemToWatchlist (simulates clicking green plus)
        if (!iioExplorer.addCurrentItemToWatchlist()) {
            printToConsole("  FAIL: Could not add current item to watchlist");
            return false;
        }
        msleep(300);

        paths = iioExplorer.getWatchlistPaths();
        if (paths.length !== 1) {
            printToConsole("  FAIL: Expected 1 item in watchlist, got " + paths.length);
            return false;
        }
        printToConsole("  Step 3: Added first element to watchlist - circle should change to X, element in first row");

        // Step 4: Add another element to watchlist
        var testPath2 = "context0/ad9361-phy/voltage0/gain_control_mode";
        if (!iioExplorer.addItemToWatchlistByPath(testPath2)) {
            printToConsole("  FAIL: Could not add item by path: " + testPath2);
            return false;
        }
        msleep(300);

        paths = iioExplorer.getWatchlistPaths();
        if (paths.length !== 2) {
            printToConsole("  FAIL: Expected 2 items in watchlist, got " + paths.length);
            return false;
        }
        printToConsole("  Step 4: Added second element to watchlist - element in second row");

        // Step 5: Modify value of an element from the watch list
        var originalValue = iioExplorer.readAttributeValue(testPath1);
        printToConsole("  Step 5: Original value of " + testPath1 + ": " + originalValue);

        var newValue = (originalValue === "manual") ? "auto" : "manual";
        if (!iioExplorer.writeWatchlistAttributeValue(testPath1, newValue)) {
            printToConsole("  FAIL: Could not write watchlist attribute value");
            return false;
        }
        msleep(500);

        // Refresh to ensure UI updates
        iioExplorer.refreshWatchlist();
        msleep(300);

        // Read back to verify value was written
        var readValue = iioExplorer.readAttributeValue(testPath1);
        printToConsole("  Step 5: Wrote new value: " + newValue + ", read back: " + readValue);
        printToConsole("  Step 5: Value should be updated in table and details view panel");

        // Restore original value
        iioExplorer.writeWatchlistAttributeValue(testPath1, originalValue);
        msleep(300);
        printToConsole("  Step 5: Restored original value: " + originalValue);

        // Step 6: Remove element from watchlist (simulates clicking red X)
        if (!iioExplorer.removeItemFromWatchlistByPath(testPath1)) {
            printToConsole("  FAIL: Could not remove item: " + testPath1);
            return false;
        }
        msleep(300);

        paths = iioExplorer.getWatchlistPaths();
        if (paths.length !== 1) {
            printToConsole("  FAIL: Expected 1 item in watchlist after removal, got " + paths.length);
            return false;
        }
        printToConsole("  Step 6: Removed element via X button - element removed from watchlist");

        // Step 7: Remove last element (simulates clicking X from last column)
        if (!iioExplorer.removeItemFromWatchlistByPath(testPath2)) {
            printToConsole("  FAIL: Could not remove item: " + testPath2);
            return false;
        }
        msleep(300);

        paths = iioExplorer.getWatchlistPaths();
        if (paths.length !== 0) {
            printToConsole("  FAIL: Expected 0 items in watchlist after removal, got " + paths.length);
            return false;
        }
        printToConsole("  Step 7: Removed last element - watchlist is empty");

        // Cleanup
        iioExplorer.clearWatchlist();
        msleep(300);
        printToConsole("  Cleanup: Watchlist cleared");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        iioExplorer.clearWatchlist();
        return false;
    }
});

// ============================================
// Test 6 - Details View (TST.DBG.EXPLR.DETAILS)
// ============================================
printToConsole("\n=== Test 6 - Details View ===\n");

TestFramework.runTest("TST.DBG.EXPLR.DETAILS", function() {
    printToConsole("\n=== Test 6 - The details view from the debugger work correctly ===\n");
    printToConsole("  NOTE: TST.DBG.EXPLR.DETAILS requires manual testing (UI interaction - progress bar, GUI/IIO view visual verification)");
    return "SKIP";
});

// ============================================
// Test 7 - Read All Operations (TST.DBG.EXPLR.READ_ALL)
// ============================================
printToConsole("\n=== Test 7 - Read All Operations ===\n");

TestFramework.runTest("TST.DBG.EXPLR.READ_ALL", function() {
    try {
        // Setup: Clear watchlist and prepare test paths
        iioExplorer.clearWatchlist();
        msleep(300);

        var deviceAttrPath = "context0/ad9361-phy/calib_mode";
        var channelAttrPath = "context0/ad9361-phy/voltage0/gain_control_mode";

        // Step 1: Add a few elements to the watchlist (device and channel attributes)
        if (!iioExplorer.addItemToWatchlistByPath(deviceAttrPath)) {
            printToConsole("  FAIL: Could not add device attribute to watchlist: " + deviceAttrPath);
            return false;
        }
        msleep(1000);
        printToConsole("  Step 1a: Added device attribute to watchlist: " + deviceAttrPath);

        if (!iioExplorer.addItemToWatchlistByPath(channelAttrPath)) {
            printToConsole("  FAIL: Could not add channel attribute to watchlist: " + channelAttrPath);
            return false;
        }
        msleep(1000);
        printToConsole("  Step 1b: Added channel attribute to watchlist: " + channelAttrPath);

        var paths = iioExplorer.getWatchlistPaths();
        if (paths.length !== 2) {
            printToConsole("  FAIL: Expected 2 items in watchlist, got " + paths.length);
            return false;
        }

        // Step 2: Select an element from the navigation tree (one with a few attributes)
        var selectedElementPath = "context0/ad9361-phy/voltage0";
        if (!iioExplorer.selectItemByPath(selectedElementPath)) {
            printToConsole("  FAIL: Could not select element: " + selectedElementPath);
            return false;
        }
        msleep(1000);
        printToConsole("  Step 2: Selected element from tree: " + selectedElementPath);

        // Read initial values from details view and watchlist
        var initialChannelValue = iioExplorer.readAttributeValue(channelAttrPath);
        printToConsole("  Initial channel attribute value: " + initialChannelValue);

        // Step 3: Click the read all button - nothing should change as all attributes are already read
        iioExplorer.triggerRead();
        msleep(1000);
        printToConsole("  Step 3: Triggered read all");

        var valueAfterFirstRead = iioExplorer.readAttributeValue(channelAttrPath);
        if (valueAfterFirstRead !== initialChannelValue) {
            printToConsole("  Warning: Value changed after first read all (initial: " + initialChannelValue + ", after: " + valueAfterFirstRead + ")");
        } else {
            printToConsole("  Step 3: Verified - values unchanged after read all (attributes already read)");
        }

        // Step 4: Simulate external change using iio_attr command
        // Note: In real testing, this would be done with: iio_attr -u ip:192.168.2.1 -c ad9361-phy voltage0 gain_control_mode <new_value>
        // Here we use writeAttributeValue to simulate an external hardware change
        var newValue = (initialChannelValue === "manual") ? "slow_attack" : "manual";
        printToConsole("  Step 4: Simulating external change via iio_attr (using writeAttributeValue)");
        printToConsole("    Command equivalent: iio_attr -u ip:192.168.2.1 -c ad9361-phy voltage0 gain_control_mode " + newValue);

        if (!iioExplorer.writeAttributeValue(channelAttrPath, newValue)) {
            printToConsole("  FAIL: Could not write attribute value");
            return false;
        }
        msleep(1000);
        printToConsole("  Step 4: Changed attribute from '" + initialChannelValue + "' to '" + newValue + "'");

        // Step 5: Click the read all button again
        iioExplorer.triggerRead();
        msleep(1000);
        printToConsole("  Step 5: Triggered read all again");

        // Verify value is updated in the details view
        var valueAfterSecondRead = iioExplorer.readAttributeValue(channelAttrPath);
        if (valueAfterSecondRead !== newValue) {
            printToConsole("  FAIL: Details view value not updated. Expected: " + newValue + ", Got: " + valueAfterSecondRead);
            return false;
        }
        printToConsole("  Step 5a: Details view updated correctly: " + valueAfterSecondRead);

        // Verify value is updated in the watchlist
        iioExplorer.refreshWatchlist();
        msleep(500);
        var watchlistValue = iioExplorer.readAttributeValue(channelAttrPath);
        if (watchlistValue !== newValue) {
            printToConsole("  FAIL: Watchlist value not updated. Expected: " + newValue + ", Got: " + watchlistValue);
            return false;
        }
        printToConsole("  Step 5b: Watchlist updated correctly: " + watchlistValue);

        // Step 6: Check the IIO View (switch to IIO tab in details view)
        iioExplorer.setDetailsViewTab(1); // 1 = IIO View
        msleep(1000);
        var currentDetailsTab = iioExplorer.getDetailsViewTab();
        if (currentDetailsTab !== 1) {
            printToConsole("  Warning: Could not switch to IIO View tab");
        } else {
            printToConsole("  Step 6: Switched to IIO View tab");
            // The IIO View should also show the updated value
            var iioViewValue = iioExplorer.readAttributeValue(channelAttrPath);
            if (iioViewValue !== newValue) {
                printToConsole("  FAIL: IIO View value not updated. Expected: " + newValue + ", Got: " + iioViewValue);
                return false;
            }
            printToConsole("  Step 6: IIO View updated correctly: " + iioViewValue);
        }

        // Restore original value
        iioExplorer.writeAttributeValue(channelAttrPath, initialChannelValue);
        msleep(1000);
        printToConsole("  Cleanup: Restored original value: " + initialChannelValue);

        // Reset details view to GUI tab
        iioExplorer.setDetailsViewTab(0);
        msleep(200);

        // Cleanup
        iioExplorer.clearWatchlist();

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        iioExplorer.setDetailsViewTab(0);
        iioExplorer.clearWatchlist();
        return false;
    }
});

// ============================================
// Test 8 - Log Operations (TST.DBG.EXPLR.LOG)
// ============================================
printToConsole("\n=== Test 8 - Log Operations ===\n");

TestFramework.runTest("TST.DBG.EXPLR.LOG", function() {
    try {
        // Step 1: Clear the log
        iioExplorer.clearLog();
        msleep(300);
        printToConsole("  Cleared log");

        // Step 2: Verify log is empty or minimal
        var logContent = iioExplorer.getLogContent();
        var initialLogLength = logContent.length;
        printToConsole("  Initial log length: " + initialLogLength);

        // Step 3: Switch to IIO Attributes tab
        iioExplorer.showIIOAttributesTab();
        msleep(1000);

        var currentTab = iioExplorer.getCurrentTab();
        if (currentTab !== 0) {
            printToConsole("  FAIL: Expected tab 0 (IIO Attributes), got " + currentTab);
            return false;
        }
        printToConsole("  Switched to IIO Attributes tab");

        // Step 4: Perform a read operation (generates log entry)
        var testPath = "context0/ad9361-phy/voltage0/hardwaregain";
        var value = iioExplorer.readAttributeValue(testPath);
        msleep(1000);
        printToConsole("  Read attribute: " + testPath + " = " + value);

        // Step 5: Perform a write operation (generates log entry)
        // First read current value of gain_control_mode
        var gainModePath = "context0/ad9361-phy/voltage0/gain_control_mode";
        var originalGainMode = iioExplorer.readAttributeValue(gainModePath);
        msleep(1000);
        printToConsole("  Original gain_control_mode: " + originalGainMode);

        // Write a value
        var newGainMode = (originalGainMode === "manual") ? "slow_attack" : "manual";
        iioExplorer.writeAttributeValue(gainModePath, newGainMode);
        msleep(1000);
        printToConsole("  Wrote gain_control_mode: " + newGainMode);

        // Step 6: Switch to Log tab
        iioExplorer.showLogTab();
        msleep(1000);

        currentTab = iioExplorer.getCurrentTab();
        if (currentTab !== 1) {
            printToConsole("  FAIL: Expected tab 1 (Log), got " + currentTab);
            return false;
        }
        printToConsole("  Switched to Log tab");

        // Step 7: Verify log contains entries
        logContent = iioExplorer.getLogContent();
        if (logContent.length <= initialLogLength) {
            printToConsole(logContent.length + "<= " + initialLogLength);
            printToConsole("  FAIL: Log should have new entries after operations");
            return false;
        }
        printToConsole("  Log has new entries (length: " + logContent.length + ")");

        // Step 8: Verify log contains expected content
        if (logContent.indexOf("gain_control_mode") === -1) {
            printToConsole("  Warning: Log does not contain 'gain_control_mode'");
        } else {
            printToConsole("  Log contains 'gain_control_mode' entry");
        }

        // Step 9: Restore original value
        iioExplorer.writeAttributeValue(gainModePath, originalGainMode);
        msleep(1000);
        printToConsole("  Restored gain_control_mode to: " + originalGainMode);

        // Step 10: Switch back to IIO Attributes tab
        iioExplorer.showIIOAttributesTab();
        msleep(300);
        printToConsole("  Switched back to IIO Attributes tab");

        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        iioExplorer.showIIOAttributesTab();
        return false;
    }
});

// ============================================
// Test 9 - Code Generator (TST.DBG.CODEGEN)
// ============================================
printToConsole("\n=== Test 9 - Code Generator ===\n");

TestFramework.runTest("TST.DBG.CODEGEN", function() {
    try {
        // Step 1: Clear watchlist
        iioExplorer.clearWatchlist();
        msleep(300);
        printToConsole("  Cleared watchlist");

        // Step 2: Add items to watchlist (generates code)
        var devicePath = "context0/ad9361-phy";
        if (!iioExplorer.selectItemByPath(devicePath)) {
            printToConsole("  FAIL: Could not select device: " + devicePath);
            return false;
        }
        msleep(1000);

        if (!iioExplorer.addCurrentItemToWatchlist()) {
            printToConsole("  FAIL: Could not add device to watchlist");
            return false;
        }
        msleep(1000);
        printToConsole("  Added device to watchlist: " + devicePath);

        // Step 3: Add a channel attribute
        var attrPath = "context0/ad9361-phy/voltage0/hardwaregain";
        if (!iioExplorer.addItemToWatchlistByPath(attrPath)) {
            printToConsole("  FAIL: Could not add attribute to watchlist: " + attrPath);
            return false;
        }
        msleep(300);
        printToConsole("  Added attribute to watchlist: " + attrPath);

        // Step 4: Switch to Code Generator tab
        iioExplorer.showCodeGeneratorTab();
        msleep(1000);

        var currentTab = iioExplorer.getCurrentTab();
        if (currentTab !== 2) {
            printToConsole("  FAIL: Expected tab 2 (Code Generator), got " + currentTab);
            return false;
        }
        printToConsole("  Switched to Code Generator tab");

        // Step 5: Get generated code
        var code = iioExplorer.getGeneratedCode();
        if (code === null || code === undefined || code === "") {
            printToConsole("  FAIL: Generated code is empty");
            return false;
        }
        printToConsole("  Generated code length: " + code.length + " characters");

        // Step 6: Verify code contains expected elements
        var expectedElements = [
            "#include <iio.h>",
            "iio_create_context",
            "ad9361-phy",
            "iio_context_find_device"
        ];

        var allFound = true;
        for (var i = 0; i < expectedElements.length; i++) {
            if (code.indexOf(expectedElements[i]) === -1) {
                printToConsole("  Warning: Code does not contain: " + expectedElements[i]);
                allFound = false;
            } else {
                printToConsole("  Code contains: " + expectedElements[i]);
            }
        }

        // Step 7: Save generated code to file (cross-platform temp path)
        var tempDir = fileIO.getTempPath();
        var savePath = tempDir + "/debugger_test_generated_code.cpp";
        printToConsole("  Using temp directory: " + tempDir);

        if (!iioExplorer.saveGeneratedCode(savePath)) {
            printToConsole("  FAIL: Could not save generated code to " + savePath);
            return false;
        }
        printToConsole("  Saved generated code to: " + savePath);

        // Step 8: Verify file exists and size > 0
        msleep(1000); // Allow file system to sync
        var fileContent = fileIO.readAll(savePath);

        if (fileContent === null || fileContent === undefined) {
            printToConsole("  FAIL: Could not read saved file - file may not exist");
            return false;
        }

        if (fileContent.length === 0) {
            printToConsole("  FAIL: Saved file is empty (size = 0)");
            return false;
        }

        printToConsole("  Verified file exists with size: " + fileContent.length + " bytes");

        // Step 9: Verify saved content matches generated code
        if (fileContent === code) {
            printToConsole("  Verified saved content matches generated code");
        } else {
            printToConsole("  Warning: Saved content differs from generated code");
            printToConsole("    Generated length: " + code.length);
            printToConsole("    Saved length: " + fileContent.length);
        }

        // Step 10: Switch back to IIO Attributes tab
        iioExplorer.showIIOAttributesTab();
        msleep(300);
        printToConsole("  Switched back to IIO Attributes tab");

        // Cleanup
        iioExplorer.clearWatchlist();

        return allFound;
    } catch (e) {
        printToConsole("  Error: " + e);
        iioExplorer.showIIOAttributesTab();
        iioExplorer.clearWatchlist();
        return false;
    }
});

// ============================================
// Debugger Preferences Tests
// ============================================
printToConsole("\n=== Debugger Preferences Tests ===\n");

TestFramework.runTest("TST.DBG.PREF.CHANGE_VERSION", function() {
    printToConsole("\n=== Pref Test 1 - Control debugger version ===\n");
    printToConsole("  NOTE: TST.DBG.PREF.CHANGE_VERSION requires manual testing (requires Scopy restart)");
    return "SKIP";
});

TestFramework.runTest("TST.DBG.PREF.INCLUDE_DEBUG_ATTRIBUTES", function() {
    printToConsole("\n=== Pref Test 2 - Include debug attributes in IIO Explorer ===\n");
    printToConsole("  NOTE: TST.DBG.PREF.INCLUDE_DEBUG_ATTRIBUTES requires manual testing (requires Scopy restart)");
    return "SKIP";
});

// Cleanup
TestFramework.disconnectFromDevice();

// Print summary and exit
var exitCode = TestFramework.printSummary();
scopy.exit();
exit(exitCode);
