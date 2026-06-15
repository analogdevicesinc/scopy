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

// Qt6 Regression: JS Engine Validation Test
// Validates that QJSEngine correctly registers and exposes API objects under Qt6.
// Tier 1 (emulator-safe) — tests engine-level functionality, not hardware values.

evaluateFile("js/testAutomations/common/testFramework.js");

TestFramework.init("Qt6 JS Engine Regression Tests");

TestFramework.runTest("printToConsole works", function() {
    try {
        printToConsole("JS engine printToConsole test");
        return true;
    } catch (e) {
        return "printToConsole threw: " + e;
    }
});

TestFramework.runTest("scopy global object accessible", function() {
    if (typeof scopy === "undefined") {
        return "scopy object is undefined";
    }
    return true;
});

TestFramework.runTest("scopy.addDevice is callable", function() {
    if (typeof scopy.addDevice !== "function") {
        return "scopy.addDevice is not a function";
    }
    return true;
});

TestFramework.runTest("scopy.removeDevice is callable", function() {
    if (typeof scopy.removeDevice !== "function") {
        return "scopy.removeDevice is not a function";
    }
    return true;
});

TestFramework.runTest("scopy.getTools callable without crash", function() {
    try {
        var tools = scopy.getTools();
        return true;
    } catch (e) {
        return "getTools() threw: " + e;
    }
});

TestFramework.runTest("msleep function works", function() {
    try {
        msleep(100);
        return true;
    } catch (e) {
        return "msleep threw: " + e;
    }
});

TestFramework.runTest("evaluateFile loads test framework", function() {
    if (typeof TestFramework === "undefined") {
        return "TestFramework not available after evaluateFile";
    }
    if (typeof TestFramework.runTest !== "function") {
        return "TestFramework.runTest not a function";
    }
    return true;
});

TestFramework.runTest("Invalid JS does not crash", function() {
    try {
        var result = eval("var x = {{{");
        return "Expected error but got result";
    } catch (e) {
        // Error is expected — engine handles it gracefully
        return true;
    }
});

var exitCode = TestFramework.printSummary();
scopy.exit();
