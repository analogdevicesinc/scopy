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
 * Configuration Management Tests for Scopy
 *
 * Tests save/load configuration functionality and settings persistence
 * Based on docs/tests/general/general_preferences_tests.rst
 */

// Import framework modules
evaluateFile("../tests/framework/test_framework.js");
evaluateFile("../tests/framework/device_helpers.js");
evaluateFile("../tests/framework/core_helpers.js");
evaluateFile("../tests/framework/reporting.js");

// Test configuration
var CONFIG_TEST_CONFIG = {
    deviceIP: "ip:192.168.2.1"
};

// Global test state
var connectedDeviceId = null;

/**
 * Test setup - connect to device
 */
function setupConfigurationTests() {
    printToConsole("⚙️ Setting up configuration tests...");
    connectedDeviceId = connectToDevice(CONFIG_TEST_CONFIG.deviceIP);
    return connectedDeviceId !== null;
}

/**
 * Test cleanup - disconnect device
 */
function cleanupConfigurationTests() {
    if (connectedDeviceId) {
        disconnectFromDevice(connectedDeviceId);
        connectedDeviceId = null;
    }
}

/**
 * Test 1: Configuration Save/Load Cycle
 * Tests that configurations can be saved and loaded successfully
 */
function testConfigurationSaveLoad() {
    printToConsole("🔄 Testing configuration save/load...");

    var fileName = "test_config";

    // Save configuration
    var saveResult = testSaveConfiguration(fileName);
    if (typeof saveResult !== 'string') {
        return "Configuration save failed: " + saveResult;
    }

    // Load configuration
    var loadResult = testLoadConfiguration(fileName);
    if (loadResult !== true) {
        return "Configuration load failed: " + loadResult;
    }

    return true;
}

/**
 * Test 2: Configuration with Tool Settings
 * Tests saving/loading configuration after switching tools
 */
function testConfigurationWithTool() {
    printToConsole("🔧 Testing configuration with tool...");

    var tools = getAvailableTools(connectedDeviceId);
    if (!tools || tools.length === 0) {
        printToConsole("ℹ️ No tools available - skipping tool configuration test");
        return true;
    }

    // Switch to first tool and save
    if (!switchToTool(tools[0])) {
        return "Failed to switch to tool";
    }

    var saveResult = testSaveConfiguration("test_with_tool");
    if (typeof saveResult !== 'string') {
        return "Failed to save configuration with tool: " + saveResult;
    }

    // Load the configuration
    var loadResult = testLoadConfiguration("test_with_tool");
    if (loadResult !== true) {
        return "Failed to load configuration with tool: " + loadResult;
    }

    return true;
}

/**
 * Test 3: Configuration Error Handling
 * Tests loading non-existent configuration
 */
function testConfigurationErrorHandling() {
    printToConsole("⚠️ Testing configuration error handling...");

    // Test loading non-existent configuration
    var loadResult = testLoadConfiguration("non_existent_config");

    // Should fail gracefully (return error message, not crash)
    if (loadResult === true) {
        return "Loading non-existent configuration should have failed";
    }

    printToConsole("✅ Non-existent configuration properly failed");
    return true;
}

/**
 * Main test execution function
 */
function main() {
    printToConsole("=== Scopy Configuration Management Tests ===");

    // Initialize test framework
    initializeTestFramework(CONFIG_TEST_CONFIG);

    // Setup tests
    if (!setupConfigurationTests()) {
        printToConsole("❌ Test setup failed - cannot connect to device");
        scopy.exit(1);
        return;
    }

    try {
        // Define test cases
        var testCases = [
            new TestCase("CONFIG.001", "Configuration Save/Load", testConfigurationSaveLoad),
            new TestCase("CONFIG.002", "Configuration with Tool", testConfigurationWithTool),
            new TestCase("CONFIG.003", "Configuration Error Handling", testConfigurationErrorHandling)
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
                // Configuration tests can continue even if some fail
                printToConsole("⚠️ Test failed but continuing with remaining tests");
            }
        }


        // Print results
        printToConsole("\n" + "=".repeat(80));
        printToConsole("📊 CONFIGURATION TESTS SUMMARY");
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
        cleanupConfigurationTests();
        cleanupAllConnections();
    }
}

// Execute tests
main();