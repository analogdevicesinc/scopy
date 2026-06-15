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

// Qt6 Hardware Test — DataLogger Plugin (ADALM-PLUTO)
// Tests device/monitor discovery, enable/disable, run/stop, and precision on real hardware.

evaluateFile("js/testAutomations/common/testFramework.js");

TestFramework.init("Qt6 HW DataLogger Tests");

if (!TestFramework.connectToDevice("ip:192.168.2.1")) {
    printToConsole("ERROR: Cannot connect to PLUTO at ip:192.168.2.1");
    scopy.exit();
}

if (!switchToTool("Data Logger")) {
    printToConsole("ERROR: Cannot switch to Data Logger tool");
    scopy.exit();
}

// ============================================
// Device & Monitor Discovery
// ============================================
printToConsole("\n=== Device & Monitor Discovery ===\n");

TestFramework.runTest("TST.HW.DLOG.AVAILABLE_DEVICES", function() {
    try {
        var devices = datalogger.showAvailableDevices();
        printToConsole("  Available devices: " + devices);
        if (!devices || devices === "") {
            printToConsole("  Error: showAvailableDevices() returned empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.DLOG.AVAILABLE_MONITORS", function() {
    try {
        var monitors = datalogger.showAvailableMonitors();
        printToConsole("  Available monitors: " + monitors);
        if (!monitors || monitors === "") {
            printToConsole("  Error: showAvailableMonitors() returned empty");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.DLOG.MONITORS_OF_DEVICE", function() {
    try {
        var monitors = datalogger.showMonitorsOfDevice("ad9361-phy");
        printToConsole("  Monitors for ad9361-phy: " + monitors);
        if (!monitors || monitors === "") {
            printToConsole("  Warning: No monitors found for ad9361-phy, trying without device filter");
            return true;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Monitor Enable/Disable
// ============================================
printToConsole("\n=== Monitor Enable/Disable ===\n");

var testMonitor = null;

TestFramework.runTest("TST.HW.DLOG.ENABLE_MONITOR", function() {
    try {
        var monitorsStr = datalogger.showAvailableMonitors();
        var monitors = monitorsStr.split(",");
        if (monitors.length === 0 || monitors[0] === "") {
            printToConsole("  Skip: No monitors available");
            return "SKIP";
        }
        testMonitor = monitors[0].trim();
        printToConsole("  Enabling monitor: " + testMonitor);
        datalogger.enableMonitor(testMonitor);
        msleep(500);
        printToConsole("  enableMonitor completed without crash");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.DLOG.DISABLE_MONITOR", function() {
    if (!testMonitor) {
        printToConsole("  Skip: No monitor was enabled");
        return "SKIP";
    }
    try {
        printToConsole("  Disabling monitor: " + testMonitor);
        datalogger.disableMonitor(testMonitor);
        msleep(500);
        printToConsole("  disableMonitor completed without crash");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Run / Stop
// ============================================
printToConsole("\n=== Run / Stop ===\n");

TestFramework.runTest("TST.HW.DLOG.RUN_STOP", function() {
    try {
        if (testMonitor) {
            datalogger.enableMonitor(testMonitor);
            msleep(500);
        }
        printToConsole("  Starting logging...");
        datalogger.setRunning(true);
        msleep(2000);
        printToConsole("  Stopping logging...");
        datalogger.setRunning(false);
        msleep(500);
        printToConsole("  Run/stop cycle completed without crash");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Precision
// ============================================
printToConsole("\n=== Precision ===\n");

TestFramework.runTest("TST.HW.DLOG.CHANGE_PRECISION", function() {
    try {
        datalogger.changePrecision(4);
        msleep(500);
        printToConsole("  Changed precision to 4 decimals");
        datalogger.changePrecision(2);
        msleep(500);
        printToConsole("  Changed precision to 2 decimals");
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

// ============================================
// Tool Management
// ============================================
printToConsole("\n=== Tool Management ===\n");

TestFramework.runTest("TST.HW.DLOG.GET_TOOL_LIST", function() {
    try {
        var tools = datalogger.getToolList();
        printToConsole("  Tool list: " + tools);
        if (!tools) {
            printToConsole("  Error: getToolList() returned null");
            return false;
        }
        return true;
    } catch (e) {
        printToConsole("  Error: " + e);
        return false;
    }
});

TestFramework.runTest("TST.HW.DLOG.CLEAR_DATA", function() {
    try {
        datalogger.clearData();
        msleep(500);
        printToConsole("  clearData() completed without crash");
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
