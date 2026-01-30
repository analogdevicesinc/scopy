/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy Test Automation Framework
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

/**
 * Tool Switching Tests for Scopy
 *
 * Tests tool navigation, switching, and availability functionality
 * Critical for ensuring users can access all instruments and tools
 */

// Import framework modules
evaluateFile("../tests/framework/test_framework.js");
evaluateFile("../tests/framework/device_helpers.js");
evaluateFile("../tests/framework/core_helpers.js");
evaluateFile("../tests/framework/reporting.js");

// Test configuration
var TOOL_TEST_CONFIG = {
    deviceIP: "ip:192.168.2.1"
};

// Global test state
var connectedDeviceId = null;

/**
 * Test setup - connect to device
 */
function setupToolTests() {
    printToConsole("🔧 Setting up tool switching tests...");
    connectedDeviceId = connectToDevice(TOOL_TEST_CONFIG.deviceIP);
    return connectedDeviceId !== null;
}

/**
 * Test cleanup - disconnect device
 */
function cleanupToolTests() {
    if (connectedDeviceId) {
        disconnectFromDevice(connectedDeviceId);
        connectedDeviceId = null;
    }
}

/**
 * Test 1: Get Available Tools
 * Verifies that the system can enumerate available tools
 */
function testGetAvailableTools() {
    printToConsole("📋 Testing tool enumeration...");

    var tools = getAvailableTools(connectedDeviceId);
    if (!tools || tools.length === 0) {
        return "No tools available - device may not be properly connected";
    }

    printToConsole("✅ Found " + tools.length + " tools");
    return true;
}

/**
 * Test 2: Tool Switching
 * Tests switching between available tools
 */
function testToolSwitching() {
    printToConsole("🔄 Testing tool switching...");

    var tools = getAvailableTools(connectedDeviceId);
    if (!tools || tools.length === 0) {
        return "No tools available for switching test";
    }

    // Test switching to first tool
    if (!switchToTool(tools[0])) {
        return "Failed to switch to tool: " + tools[0];
    }

    // If there's a second tool, switch to it
    if (tools.length > 1) {
        if (!switchToTool(tools[1])) {
            return "Failed to switch to tool: " + tools[1];
        }
    }

    return true;
}

/**
 * Test 3: Tool Running Operations
 * Tests starting and stopping a tool that has run button
 */
function testToolRunningOperations() {
    printToConsole("▶️ Testing tool run/stop operations...");

    var tools = getAvailableTools(connectedDeviceId);
    if (!tools || tools.length === 0) {
        return "No tools available for run/stop test";
    }

    // Try first tool with run functionality
    var result = testToolRunning(tools[0]);
    if (result === true) {
        printToConsole("✅ Tool run/stop test successful");
    } else if (result.indexOf("no run button") !== -1) {
        printToConsole("ℹ️ Tool has no run button - skipping");
    } else {
        return result;
    }

    return true;
}


/**
 * Main test execution function
 */
function main() {
    printToConsole("=== Scopy Tool Switching Tests ===");

    // Initialize test framework
    initializeTestFramework(TOOL_TEST_CONFIG);

    // Setup tests
    if (!setupToolTests()) {
        printToConsole("❌ Test setup failed - cannot connect to device");
        scopy.exit(1);
        return;
    }

    try {
        // Define test cases
        var testCases = [
            new TestCase("TOOL.001", "Get Available Tools", testGetAvailableTools),
            new TestCase("TOOL.002", "Tool Switching", testToolSwitching),
            new TestCase("TOOL.003", "Tool Running Operations", testToolRunningOperations)
        ];

        var allPassed = true;
        var testResults = [];

        // Execute tests with proper dependency handling
        for (var i = 0; i < testCases.length; i++) {
            var testCase = testCases[i];
            printToConsole("\n" + "=".repeat(60));
            printToConsole("🔄 Running: " + testCase.testId + " - " + testCase.description);
            printToConsole("=".repeat(60));

            var testPassed = executeTest(testCase);
            testResults.push({
                testId: testCase.testId,
                description: testCase.description,
                passed: testPassed
            });

            if (!testPassed) {
                allPassed = false;

                // Stop execution if critical tool tests fail
                if (testCase.testId === "TOOL.001") {
                    printToConsole("🛑 CRITICAL: Tool enumeration failed - stopping all tests");
                    printToConsole("💡 Cannot test tool functionality if tools cannot be enumerated");
                    break;
                } else if (testCase.testId === "TOOL.002") {
                    printToConsole("🛑 CRITICAL: Basic tool switching failed - stopping critical tests");
                    printToConsole("💡 Tool switching is fundamental - other tests may not be reliable");
                    break;
                }
                // Other tests can continue even if they fail
            }
        }


        // Print results
        printToConsole("\n" + "=".repeat(80));
        printToConsole("📊 TOOL SWITCHING TESTS SUMMARY");
        printToConsole("=".repeat(80));

        for (var j = 0; j < testResults.length; j++) {
            var result = testResults[j];
            var icon = result.passed ? "✅" : "❌";
            printToConsole(icon + " " + result.testId + ": " + result.description);
        }

        printToConsole("\n🎯 Overall Result: " + (allPassed ? "ALL PASSED ✅" : "SOME FAILED ❌"));

        // Exit with appropriate code
        scopy.exit(allPassed ? 0 : 1);

    } finally {
        // Cleanup
        cleanupToolTests();
        cleanupAllConnections();
    }
}

// Execute tests
main();