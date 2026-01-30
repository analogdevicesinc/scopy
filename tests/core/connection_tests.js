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
 * Simple Connection Tests for Scopy
 *
 * Tests basic device connection functionality with single responsibility principle
 * Each test does exactly one thing and builds on previous test success
 */

// Import framework modules
evaluateFile("../tests/framework/test_framework.js");
evaluateFile("../tests/framework/device_helpers.js");
evaluateFile("../tests/framework/core_helpers.js");
evaluateFile("../tests/framework/reporting.js");

// Test configuration
var CONNECTION_TEST_CONFIG = {
    deviceIP: "ip:192.168.2.1",
    calibrationWait: 3000
};

/**
 * Test 1: Connection Only
 * ONLY tests if we can connect to device - does NOT disconnect
 */
function testConnectionOnly() {
    printToConsole("🔗 Testing basic connection only...");

    var deviceId = connectToDevice(CONNECTION_TEST_CONFIG.deviceIP);
    if (!deviceId) {
        return "Failed to connect to device";
    }

    // Verify connection worked by checking tools
    if (!verifyDeviceConnection(deviceId)) {
        return "Connection verification failed - no tools available";
    }

    printToConsole("✅ Connection successful - device ready for other tests");
    printToConsole("📱 Device ID: " + deviceId);

    // DO NOT DISCONNECT - leave device connected for next test
    return true;
}

/**
 * Test 2: Disconnection Only
 * ONLY tests if we can disconnect - assumes device is already connected
 */
function testDisconnectionOnly() {
    printToConsole("🔌 Testing basic disconnection only...");

    // This test assumes device is connected from Test 1
    if (!deviceState.currentDevice) {
        return "No device connected to disconnect - connection test must have failed";
    }

    var deviceId = deviceState.currentDevice.id;
    printToConsole("📱 Disconnecting device: " + deviceId);

    var success = disconnectFromDevice(deviceId, true); // Disconnect and remove
    if (!success) {
        return "Disconnection failed";
    }

    printToConsole("✅ Disconnection successful");
    return true;
}

/**
 * Test 3: Invalid Connection Handling
 * Tests that invalid connections fail cleanly without hanging
 */
function testInvalidConnectionHandling() {
    printToConsole("⏱️ Testing invalid connection handling...");

    // Test with obviously invalid IP that should fail quickly
    var invalidIP = "ip:192.168.255.255";
    var startTime = new Date();

    var deviceId = connectToDevice(invalidIP, 0); // No calibration wait
    var duration = new Date() - startTime;

    // Should fail quickly and not hang
    if (deviceId !== null) {
        // Unexpected success - clean up
        disconnectFromDevice(deviceId, true);
        return "Expected connection to invalid IP to fail, but it succeeded";
    }

    // Verify it failed within reasonable time (less than 30 seconds)
    if (duration > 30000) {
        return "Connection timeout took too long: " + duration + "ms (expected < 30s)";
    }

    printToConsole("✅ Invalid connection failed cleanly in " + (duration/1000).toFixed(1) + "s");
    return true;
}

/**
 * Main test execution function
 */
function main() {
    printToConsole("=== Scopy Simple Connection Tests ===");

    // Initialize test framework
    initializeTestFramework(CONNECTION_TEST_CONFIG);

    // Validate system prerequisites
    var prereqResult = validateSystemPrerequisites();
    if (prereqResult !== true) {
        printToConsole("❌ Prerequisites failed: " + prereqResult);
        scopy.exit(1);
        return;
    }

    // Define test cases in dependency order
    var testCases = [
        new TestCase("CONN.001", "Connection Only", testConnectionOnly),
        new TestCase("CONN.002", "Disconnection Only", testDisconnectionOnly),
        new TestCase("CONN.003", "Invalid Connection Handling", testInvalidConnectionHandling)
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

            // Stop execution if critical connection tests fail
            if (testCase.testId === "CONN.001") {
                printToConsole("🛑 CRITICAL: Basic connection failed - stopping all tests");
                printToConsole("💡 Cannot test other functionality if device connection doesn't work");
                break;
            } else if (testCase.testId === "CONN.002") {
                printToConsole("🛑 CRITICAL: Basic disconnection failed - stopping critical tests");
                printToConsole("💡 Device may be left in inconsistent state");
                break;
            }
            // Test 3 (invalid connection) can fail without stopping others
        }
    }


    // Print results
    printToConsole("\n" + "=".repeat(80));
    printToConsole("📊 SIMPLE CONNECTION TESTS SUMMARY");
    printToConsole("=".repeat(80));

    for (var j = 0; j < testResults.length; j++) {
        var result = testResults[j];
        var icon = result.passed ? "✅" : "❌";
        printToConsole(icon + " " + result.testId + ": " + result.description);
    }

    printToConsole("\n🎯 Overall Result: " + (allPassed ? "ALL PASSED ✅" : "SOME FAILED ❌"));

    // Clean up any remaining connections
    cleanupAllConnections();

    // Exit with appropriate code
    scopy.exit(allPassed ? 0 : 1);
}

// Execute tests
main();