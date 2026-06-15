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

// Qt6 Hardware Test — IIO Explorer / Debugger Plugin (ADALM-PLUTO)
// Tests tree navigation, attribute read/write, watchlist, tabs, and search on real hardware.

evaluateFile("js/testAutomations/common/testFramework.js");

TestFramework.init("Qt6 HW IIO Explorer Tests");

if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot connect to PLUTO at ip:192.168.2.1");
    scopy.exit();
}

if (!switchToTool("Debugger")) {
    if (!switchToTool("IIO Explorer")) {
        printToConsole("ERROR: Cannot switch to Debugger/IIO Explorer tool");
        scopy.exit();
    }
}

// ============================================
// Tab Switching
// ============================================
printToConsole("\n=== Tab Switching ===\n");

TestFramework.runTest("TST.HW.IIOX.SHOW_ATTRIBUTES_TAB", function() {
    try {
        iioExplorer.showIIOAttributesTab();
        msleep(500);
        var tab = iioExplorer.getCurrentTab();
        printToConsole("  Current tab after showIIOAttributesTab: " + tab);
        return tab === 0;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.IIOX.SHOW_LOG_TAB", function() {
    try {
        iioExplorer.showLogTab();
        msleep(500);
        var tab = iioExplorer.getCurrentTab();
        printToConsole("  Current tab after showLogTab: " + tab);
        return tab === 1;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.IIOX.SHOW_CODEGEN_TAB", function() {
    try {
        iioExplorer.showCodeGeneratorTab();
        msleep(500);
        var tab = iioExplorer.getCurrentTab();
        printToConsole("  Current tab after showCodeGeneratorTab: " + tab);
        return tab === 2;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// Switch back to attributes tab for remaining tests
iioExplorer.showIIOAttributesTab();
msleep(500);

// ============================================
// Tree Navigation
// ============================================
printToConsole("\n=== Tree Navigation ===\n");

TestFramework.runTest("TST.HW.IIOX.SELECT_DEVICE", function() {
    try {
        var result = iioExplorer.selectItemByPath("context0/ad9361-phy");
        msleep(500);
        printToConsole("  selectItemByPath('context0/ad9361-phy'): " + result);
        var selectedPath = iioExplorer.getSelectedItemPath();
        printToConsole("  Selected item path: " + selectedPath);
        var selectedName = iioExplorer.getSelectedItemName();
        printToConsole("  Selected item name: " + selectedName);
        return result === true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.IIOX.EXPAND_COLLAPSE", function() {
    try {
        var expandResult = iioExplorer.expandItem("context0/ad9361-phy");
        msleep(500);
        var isExpanded = iioExplorer.isItemExpanded("context0/ad9361-phy");
        printToConsole("  Expanded ad9361-phy: " + expandResult + ", isExpanded: " + isExpanded);

        var childCount = iioExplorer.getChildCount("context0/ad9361-phy");
        printToConsole("  Child count: " + childCount);

        var childNames = iioExplorer.getChildNames("context0/ad9361-phy");
        printToConsole("  Child names: " + childNames);

        var collapseResult = iioExplorer.collapseItem("context0/ad9361-phy");
        msleep(500);
        var isCollapsed = !iioExplorer.isItemExpanded("context0/ad9361-phy");
        printToConsole("  Collapsed ad9361-phy: " + collapseResult + ", isCollapsed: " + isCollapsed);

        // Re-expand for subsequent tests
        iioExplorer.expandItem("context0/ad9361-phy");
        msleep(500);

        return expandResult === true && childCount > 0;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.IIOX.SELECT_CHANNEL", function() {
    try {
        var result = iioExplorer.selectItemByPath("context0/ad9361-phy/voltage0");
        msleep(500);
        printToConsole("  selectItemByPath('context0/ad9361-phy/voltage0'): " + result);
        var name = iioExplorer.getSelectedItemName();
        printToConsole("  Selected item name: " + name);
        var type = iioExplorer.getSelectedItemType();
        printToConsole("  Selected item type: " + type);
        return result === true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Attribute Read/Write
// ============================================
printToConsole("\n=== Attribute Read/Write ===\n");

TestFramework.runTest("TST.HW.IIOX.READ_ATTRIBUTE", function() {
    try {
        var value = iioExplorer.readAttributeValue("context0/ad9361-phy/temp0/input");
        printToConsole("  temp0/input value: " + value);
        if (value === null || value === undefined || value === "") {
            printToConsole("  Error: readAttributeValue returned empty");
            return false;
        }
        var numVal = parseInt(value);
        printToConsole("  Parsed temperature: " + numVal + " (millidegrees C)");
        return !isNaN(numVal);
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.IIOX.WRITE_READ_ATTRIBUTE", function() {
    try {
        var attrPath = "context0/ad9361-phy/voltage0/hardwaregain";
        var original = iioExplorer.readAttributeValue(attrPath);
        printToConsole("  Original hardwaregain: " + original);

        iioExplorer.writeAttributeValue(attrPath, "30.000000 dB");
        msleep(500);
        var readBack = iioExplorer.readAttributeValue(attrPath);
        printToConsole("  Read back hardwaregain: " + readBack);

        // Restore
        if (original && original !== readBack) {
            iioExplorer.writeAttributeValue(attrPath, original);
            msleep(500);
        }

        return readBack !== null && readBack !== "";
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.IIOX.TRIGGER_READ", function() {
    try {
        iioExplorer.triggerRead();
        msleep(500);
        printToConsole("  triggerRead() completed without crash");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Watchlist
// ============================================
printToConsole("\n=== Watchlist ===\n");

TestFramework.runTest("TST.HW.IIOX.WATCHLIST_ADD", function() {
    try {
        iioExplorer.clearWatchlist();
        msleep(500);

        var result = iioExplorer.addItemToWatchlistByPath("context0/ad9361-phy/temp0/input");
        msleep(500);
        printToConsole("  addItemToWatchlistByPath: " + result);

        var paths = iioExplorer.getWatchlistPaths();
        printToConsole("  Watchlist paths: " + paths);

        return paths.length > 0;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.IIOX.WATCHLIST_REFRESH", function() {
    try {
        iioExplorer.refreshWatchlist();
        msleep(500);
        printToConsole("  refreshWatchlist() completed without crash");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.IIOX.WATCHLIST_CLEAR", function() {
    try {
        iioExplorer.clearWatchlist();
        msleep(500);
        var paths = iioExplorer.getWatchlistPaths();
        printToConsole("  Watchlist paths after clear: " + paths);
        return paths.length === 0;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Search
// ============================================
printToConsole("\n=== Search ===\n");

TestFramework.runTest("TST.HW.IIOX.SEARCH", function() {
    try {
        iioExplorer.setSearchText("voltage");
        msleep(1000);
        var visibleCount = iioExplorer.getVisibleItemCount();
        printToConsole("  Search 'voltage' visible items: " + visibleCount);

        iioExplorer.setSearchText("");
        msleep(500);
        var allCount = iioExplorer.getVisibleItemCount();
        printToConsole("  Cleared search visible items: " + allCount);

        return visibleCount > 0;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Code Generator
// ============================================
printToConsole("\n=== Code Generator ===\n");

TestFramework.runTest("TST.HW.IIOX.CODEGEN", function() {
    try {
        iioExplorer.showCodeGeneratorTab();
        msleep(500);
        var code = iioExplorer.getGeneratedCode();
        printToConsole("  Generated code length: " + (code ? code.length : 0));
        if (code && code.length > 0) {
            printToConsole("  First 100 chars: " + code.substring(0, 100));
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Log
// ============================================
printToConsole("\n=== Log ===\n");

TestFramework.runTest("TST.HW.IIOX.LOG", function() {
    try {
        iioExplorer.showLogTab();
        msleep(500);
        var log = iioExplorer.getLogContent();
        printToConsole("  Log content length: " + (log ? log.length : 0));
        iioExplorer.clearLog();
        msleep(500);
        printToConsole("  Log cleared without crash");
        return true;
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
