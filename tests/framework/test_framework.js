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
 * Scopy Test Automation Framework
 *
 * Core testing utilities for Scopy automated testing.
 * Provides test execution, validation, reporting, and error handling.
 */

// Global test state
var testFramework = {
    currentTest: null,
    testResults: [],
    totalTests: 0,
    passedTests: 0,
    failedTests: 0,
    startTime: null,
    config: {
        deviceIP: "ip:192.168.2.1",
        defaultTimeout: 5000,
        calibrationWait: 3000,
        measurementWait: 2000,
        toolSwitchWait: 1000
    }
};

/**
 * Test result structure
 */
function TestResult(testId, description, status, message, duration, details) {
    this.testId = testId;
    this.description = description;
    this.status = status; // 'PASS', 'FAIL', 'SKIP'
    this.message = message || '';
    this.duration = duration || 0;
    this.details = details || {};
    this.timestamp = new Date().toISOString();
}

/**
 * Test case structure
 */
function TestCase(testId, description, testFunction, timeout, dependencies) {
    this.testId = testId;
    this.description = description;
    this.testFunction = testFunction;
    this.timeout = timeout || testFramework.config.defaultTimeout;
    this.dependencies = dependencies || [];
}

/**
 * Initialize test framework
 */
function initializeTestFramework(config) {
    if (config) {
        Object.assign(testFramework.config, config);
    }
    testFramework.startTime = new Date();
    testFramework.testResults = [];
    testFramework.totalTests = 0;
    testFramework.passedTests = 0;
    testFramework.failedTests = 0;

    printToConsole("=== Scopy Test Framework Initialized ===");
    printToConsole("Device IP: " + testFramework.config.deviceIP);
    printToConsole("Start Time: " + testFramework.startTime.toISOString());
}

/**
 * Execute a single test case
 */
function executeTest(testCase) {
    testFramework.currentTest = testCase;
    testFramework.totalTests++;

    var startTime = new Date();
    printToConsole("\n🔄 Running: " + testCase.testId + " - " + testCase.description);

    try {
        // Execute the test function
        var result = testCase.testFunction();
        var duration = new Date() - startTime;

        if (result === true || result === undefined) {
            // Test passed
            var testResult = new TestResult(
                testCase.testId,
                testCase.description,
                'PASS',
                'Test completed successfully',
                duration
            );
            testFramework.testResults.push(testResult);
            testFramework.passedTests++;
            printToConsole("✅ PASS: " + testCase.testId);
            return true;

        } else {
            // Test failed with custom message
            var testResult = new TestResult(
                testCase.testId,
                testCase.description,
                'FAIL',
                result || 'Test returned false',
                duration
            );
            testFramework.testResults.push(testResult);
            testFramework.failedTests++;
            printToConsole("❌ FAIL: " + testCase.testId + " - " + testResult.message);
            return false;
        }

    } catch (error) {
        var duration = new Date() - startTime;
        var testResult = new TestResult(
            testCase.testId,
            testCase.description,
            'FAIL',
            'Exception: ' + error.toString(),
            duration,
            { exception: error.toString() }
        );
        testFramework.testResults.push(testResult);
        testFramework.failedTests++;
        printToConsole("💥 EXCEPTION: " + testCase.testId + " - " + error.toString());
        return false;
    } finally {
        testFramework.currentTest = null;
    }
}


/**
 * Generate final test report
 */
function generateTestReport() {
    var totalDuration = new Date() - testFramework.startTime;

    printToConsole("\n" + "=".repeat(80));
    printToConsole("📈 FINAL TEST REPORT");
    printToConsole("=".repeat(80));
    printToConsole("Total Tests Executed: " + testFramework.totalTests);
    printToConsole("Passed: " + testFramework.passedTests + " ✅");
    printToConsole("Failed: " + testFramework.failedTests + " ❌");
    printToConsole("Overall Success Rate: " + ((testFramework.passedTests / testFramework.totalTests) * 100).toFixed(1) + "%");
    printToConsole("Total Duration: " + (totalDuration / 1000).toFixed(2) + "s");

    if (testFramework.failedTests > 0) {
        printToConsole("\n❌ FAILED TESTS:");
        for (var i = 0; i < testFramework.testResults.length; i++) {
            var result = testFramework.testResults[i];
            if (result.status === 'FAIL') {
                printToConsole("  - " + result.testId + ": " + result.message);
            }
        }
    }

    printToConsole("=".repeat(80));

    return {
        totalTests: testFramework.totalTests,
        passedTests: testFramework.passedTests,
        failedTests: testFramework.failedTests,
        successRate: (testFramework.passedTests / testFramework.totalTests) * 100,
        duration: totalDuration,
        results: testFramework.testResults
    };
}

/**
 * Assertion functions
 */
function assertTrue(condition, message) {
    if (!condition) {
        throw new Error(message || "Assertion failed: expected true");
    }
}

function assertFalse(condition, message) {
    if (condition) {
        throw new Error(message || "Assertion failed: expected false");
    }
}

function assertEqual(actual, expected, message) {
    if (actual !== expected) {
        throw new Error(message || "Assertion failed: expected " + expected + " but got " + actual);
    }
}

function assertNotEqual(actual, notExpected, message) {
    if (actual === notExpected) {
        throw new Error(message || "Assertion failed: expected not to equal " + notExpected);
    }
}

function assertInRange(value, min, max, message) {
    if (value < min || value > max) {
        throw new Error(message || "Assertion failed: " + value + " not in range [" + min + ", " + max + "]");
    }
}

/**
 * Utility functions
 */
function waitForCondition(conditionFn, timeout, checkInterval) {
    timeout = timeout || testFramework.config.defaultTimeout;
    checkInterval = checkInterval || 100;

    var startTime = new Date();
    while ((new Date() - startTime) < timeout) {
        if (conditionFn()) {
            return true;
        }
        msleep(checkInterval);
    }
    return false;
}

function safeWait(milliseconds) {
    if (milliseconds > 0) {
        msleep(milliseconds);
    }
}

/**
 * Test configuration helpers
 */
function setTestConfig(key, value) {
    testFramework.config[key] = value;
}

function getTestConfig(key) {
    return testFramework.config[key];
}

// Export for other modules
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        TestCase: TestCase,
        TestResult: TestResult,
        initializeTestFramework: initializeTestFramework,
        executeTest: executeTest,
        executeTestSuite: executeTestSuite,
        generateTestReport: generateTestReport,
        assertTrue: assertTrue,
        assertFalse: assertFalse,
        assertEqual: assertEqual,
        assertNotEqual: assertNotEqual,
        assertInRange: assertInRange,
        waitForCondition: waitForCondition,
        safeWait: safeWait,
        setTestConfig: setTestConfig,
        getTestConfig: getTestConfig
    };
}