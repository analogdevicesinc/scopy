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

// Test Automation Framework for Scopy
// Common utilities and test management

var TestFramework = {
    // Test statistics
    totalTests: 0,
    passedTests: 0,
    failedTests: 0,
    skippedTests: 0,
    currentTestSuite: "",
    testResults: [],

    // Device configuration
    deviceUri: "ip:192.168.2.1",  // Default M2K IP, can be overridden
    deviceId: null,
    isConnected: false,

    // Initialize test framework
    init: function(suiteName) {
        this.currentTestSuite = suiteName;
        this.totalTests = 0;
        this.passedTests = 0;
        this.failedTests = 0;
        this.skippedTests = 0;
        this.testResults = [];

        printToConsole("=====================================");
        printToConsole("Test Suite: " + suiteName);
        printToConsole("Start Time: " + new Date().toISOString());
        printToConsole("=====================================\n");
    },

    // Connect to device with automatic fallback to emulator
    connectToDevice: function(uri) {
        // If specific URI provided, use it
        if (uri) {
            this.deviceUri = uri;
        }

        // Try real device first (ip:192.168.2.1)
        try {
            printToConsole("Connecting to M2K device: " + this.deviceUri);
            this.deviceId = scopy.addDevice(this.deviceUri);

            if (this.deviceId && scopy.connectDevice(this.deviceId)) {
                this.isConnected = true;
                printToConsole("✓ Successfully connected to M2K device\n");
                msleep(2000);
                return true;
            }
        } catch (e) {
            // Clean up failed attempt
            if (this.deviceId) {
                try { scopy.removeDevice(this.deviceId); } catch (e2) {}
            }
        }

        // Fallback to emulator (ip:127.0.0.1)
        printToConsole("M2K not found, trying emulator...");
        this.deviceUri = "ip:127.0.0.1";

        try {
            this.deviceId = scopy.addDevice(this.deviceUri);

            if (this.deviceId && scopy.connectDevice(this.deviceId)) {
                this.isConnected = true;
                printToConsole("✓ Successfully connected to emulator\n");
                msleep(2000);
                return true;
            }
        } catch (e) {
            printToConsole("✗ Failed to connect to both device and emulator");
            this.isConnected = false;
            return false;
        }
    },

    // Disconnect from device
    disconnectFromDevice: function() {
        if (this.isConnected && this.deviceId) {
            try {
                scopy.disconnectDevice(this.deviceId);
                scopy.removeDevice(this.deviceId);
                this.isConnected = false;
                printToConsole("✓ Disconnected from device\n");
            } catch (e) {
                printToConsole("Warning: Error disconnecting: " + e);
            }
        }
    },

    // Run a single test
    runTest: function(testName, testFunction) {
        this.totalTests++;
        printToConsole("Running Test UID: " + testName);
        printToConsole("Test #" + this.totalTests + " in suite");
        printToConsole("-".repeat(40));

        try {
            let startTime = Date.now();
            let result = testFunction();
            let duration = Date.now() - startTime;

            if (result === true) {
                this.passedTests++;
                printToConsole("✓ PASS (" + duration + "ms)\n");
                this.testResults.push({
                    name: testName,
                    status: "PASS",
                    duration: duration
                });
            } else if (result === "SKIP") {
                this.skippedTests++;
                printToConsole("⊘ SKIPPED\n");
                this.testResults.push({
                    name: testName,
                    status: "SKIP",
                    duration: 0
                });
            } else {
                this.failedTests++;
                printToConsole("✗ FAIL (" + duration + "ms)");
                if (result && typeof result === "string") {
                    printToConsole("  Reason: " + result);
                }
                printToConsole("");
                this.testResults.push({
                    name: testName,
                    status: "FAIL",
                    duration: duration,
                    reason: result
                });
            }
        } catch (e) {
            this.failedTests++;
            printToConsole("✗ ERROR: " + e + "\n");
            this.testResults.push({
                name: testName,
                status: "ERROR",
                error: e.toString()
            });
        }
    },

    // Assert functions
    assertEqual: function(actual, expected, message) {
        if (actual === expected) {
            printToConsole("  ✓ " + (message || "Values are equal"));
            return true;
        } else {
            printToConsole("  ✗ " + (message || "Values not equal"));
            printToConsole("    Expected: " + expected);
            printToConsole("    Actual: " + actual);
            return false;
        }
    },

    assertInRange: function(value, min, max, message) {
        if (value >= min && value <= max) {
            printToConsole("  ✓ " + (message || "Value in range"));
            printToConsole("    Value: " + value + " (range: " + min + " to " + max + ")");
            return true;
        } else {
            printToConsole("  ✗ " + (message || "Value out of range"));
            printToConsole("    Value: " + value + " (expected: " + min + " to " + max + ")");
            return false;
        }
    },

    assertApproxEqual: function(actual, expected, tolerance, message) {
        let diff = Math.abs(actual - expected);
        if (diff <= tolerance) {
            printToConsole("  ✓ " + (message || "Values approximately equal"));
            printToConsole("    Value: " + actual + " ≈ " + expected + " (±" + tolerance + ")");
            return true;
        } else {
            printToConsole("  ✗ " + (message || "Values not approximately equal"));
            printToConsole("    Expected: " + expected + " ±" + tolerance);
            printToConsole("    Actual: " + actual + " (diff: " + diff + ")");
            return false;
        }
    },

    // Print test summary
    printSummary: function() {
        printToConsole("\n=====================================");
        printToConsole("Test Summary: " + this.currentTestSuite);
        printToConsole("=====================================");
        printToConsole("Total Tests:   " + this.totalTests);
        printToConsole("Passed:        " + this.passedTests + " (" +
                      Math.round(this.passedTests / this.totalTests * 100) + "%)");
        printToConsole("Failed:        " + this.failedTests);
        printToConsole("Skipped:       " + this.skippedTests);
        printToConsole("End Time:      " + new Date().toISOString());
        printToConsole("=====================================\n");

        // Return exit code
        return this.failedTests === 0 ? 0 : 1;
    },

    // Save test results to file
    saveResults: function(filePath) {
        try {
            let results = {
                suite: this.currentTestSuite,
                timestamp: new Date().toISOString(),
                summary: {
                    total: this.totalTests,
                    passed: this.passedTests,
                    failed: this.failedTests,
                    skipped: this.skippedTests
                },
                tests: this.testResults
            };

            // Note: Actual file writing would require additional API
            printToConsole("Test results would be saved to: " + filePath);
            printToConsole(JSON.stringify(results, null, 2));

        } catch (e) {
            printToConsole("Error saving results: " + e);
        }
    }
};

// Helper function for switching tools with verification
function switchToTool(toolName) {
    try {
        let result = scopy.switchTool(toolName);
        if (!result) {
            throw new Error("Failed to switch to " + toolName);
        }
        printToConsole("  Switched to " + toolName);
        msleep(2000); // Allow tool to initialize
        return true;
    } catch (e) {
        printToConsole("  ERROR: Failed to switch tool: " + e);
        return false;
    }
}

// Export for use in other test files
if (typeof module !== 'undefined' && module.exports) {
    module.exports = TestFramework;
}