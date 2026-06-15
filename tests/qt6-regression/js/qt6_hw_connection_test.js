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

// Qt6 Hardware Test — Connection & Plugin Load (ADALM-PLUTO)
// Verifies device connection, plugin loading, and tool switching on real hardware.

evaluateFile("js/testAutomations/common/testFramework.js");

TestFramework.init("Qt6 HW Connection & Plugin Load Tests");

// Connect to PLUTO
if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot connect to PLUTO at ip:192.168.2.1");
    scopy.exit();
}

// ============================================
// Connection Tests
// ============================================
printToConsole("\n=== Connection Tests ===\n");

TestFramework.runTest("TST.HW.CONN.DEVICE_CONNECTED", function() {
    if (!TestFramework.isConnected) {
        printToConsole("  Error: Not connected");
        return false;
    }
    printToConsole("  Connected to: " + TestFramework.deviceUri);
    return true;
});

// ============================================
// Tool Discovery Tests
// ============================================
printToConsole("\n=== Tool Discovery Tests ===\n");

TestFramework.runTest("TST.HW.CONN.GET_TOOLS", function() {
    try {
        var tools = scopy.getTools();
        printToConsole("  Total tools loaded: " + tools.length);
        for (var i = 0; i < tools.length; i++) {
            printToConsole("    [" + i + "] " + tools[i]);
        }
        if (!tools || tools.length === 0) {
            printToConsole("  Error: No tools loaded");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.CONN.EXPECTED_PLUGINS", function() {
    try {
        var tools = scopy.getTools();
        var toolStr = tools.join(",").toLowerCase();

        var expected = ["adc", "dac", "ad936x"];
        var allFound = true;
        for (var i = 0; i < expected.length; i++) {
            var found = false;
            for (var j = 0; j < tools.length; j++) {
                if (tools[j].toLowerCase().indexOf(expected[i]) !== -1) {
                    found = true;
                    break;
                }
            }
            if (found) {
                printToConsole("  Found: " + expected[i]);
            } else {
                printToConsole("  MISSING: " + expected[i]);
                allFound = false;
            }
        }
        return allFound;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Tool Switching Tests — switch to each tool without crash
// ============================================
printToConsole("\n=== Tool Switching Tests ===\n");

TestFramework.runTest("TST.HW.CONN.SWITCH_ALL_TOOLS", function() {
    try {
        var tools = scopy.getTools();
        var allOk = true;
        for (var i = 0; i < tools.length; i++) {
            var toolName = tools[i];
            printToConsole("  Switching to: " + toolName);
            try {
                var result = scopy.switchTool(toolName);
                msleep(2000);
                if (!result) {
                    printToConsole("    WARNING: switchTool returned false for " + toolName);
                } else {
                    printToConsole("    OK");
                }
            } catch (e) {
                printToConsole("    ERROR switching to " + toolName + ": " + e);
                allOk = false;
            }
        }
        return allOk;
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
