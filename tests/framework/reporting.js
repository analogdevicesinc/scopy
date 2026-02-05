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
 * Test Reporting and Formatting Utilities for Scopy Test Automation
 *
 * Provides structured test result reporting, formatting, and output utilities
 */

/**
 * Generate detailed test report with formatting
 */
function generateDetailedTestReport(testResults, suiteName) {
    var report = {
        suiteName: suiteName || "Test Suite",
        timestamp: new Date().toISOString(),
        summary: {},
        tests: [],
        metadata: {}
    };

    if (!testResults || !testResults.results) {
        report.summary = {
            totalTests: 0,
            passed: 0,
            failed: 0,
            successRate: 0,
            duration: 0
        };
        return report;
    }

    // Calculate summary statistics
    var results = testResults.results || [];
    var passedTests = results.filter(function(test) { return test.status === 'PASS'; });
    var failedTests = results.filter(function(test) { return test.status === 'FAIL'; });

    report.summary = {
        totalTests: results.length,
        passed: passedTests.length,
        failed: failedTests.length,
        successRate: results.length > 0 ? (passedTests.length / results.length) * 100 : 0,
        duration: testResults.duration || 0
    };

    // Format individual test results
    report.tests = results.map(function(test) {
        return {
            testId: test.testId,
            description: test.description,
            status: test.status,
            message: test.message,
            duration: test.duration,
            timestamp: test.timestamp,
            details: test.details || {}
        };
    });

    // Add metadata
    report.metadata = {
        deviceInfo: getCurrentDeviceInfo(),
        toolInfo: getCurrentToolInfo(),
        environment: {
            timestamp: new Date().toISOString(),
            testFrameworkVersion: "1.0.0"
        }
    };

    return report;
}

/**
 * Print formatted test summary to console
 */
function printTestSummary(testResults, suiteName) {
    var summary = testResults.summary || testResults;

    printToConsole("\n" + "=".repeat(60));
    printToConsole("📊 TEST SUMMARY: " + (suiteName || "Test Suite"));
    printToConsole("=".repeat(60));

    printToConsole("📋 Total Tests: " + summary.totalTests);
    printToConsole("✅ Passed: " + summary.passed);
    printToConsole("❌ Failed: " + summary.failed);
    printToConsole("📈 Success Rate: " + (summary.successRate || 0).toFixed(1) + "%");

    if (summary.duration) {
        printToConsole("⏱️ Duration: " + (summary.duration / 1000).toFixed(2) + "s");
    }

    printToConsole("=".repeat(60));
}

/**
 * Print detailed test results to console
 */
function printDetailedResults(testResults) {
    if (!testResults.tests) return;

    printToConsole("\n📝 DETAILED TEST RESULTS:");
    printToConsole("-".repeat(60));

    for (var i = 0; i < testResults.tests.length; i++) {
        var test = testResults.tests[i];
        var icon = test.status === 'PASS' ? '✅' : '❌';
        var duration = test.duration ? ' (' + (test.duration / 1000).toFixed(2) + 's)' : '';

        printToConsole(icon + " " + test.testId + ": " + test.description + duration);

        if (test.status === 'FAIL' && test.message) {
            printToConsole("    └─ " + test.message);
        }
    }

    printToConsole("-".repeat(60));
}

/**
 * Print failed tests summary
 */
function printFailedTestsSummary(testResults) {
    var failedTests = testResults.tests.filter(function(test) {
        return test.status === 'FAIL';
    });

    if (failedTests.length === 0) {
        printToConsole("🎉 No failed tests!");
        return;
    }

    printToConsole("\n❌ FAILED TESTS SUMMARY:");
    printToConsole("-".repeat(40));

    for (var i = 0; i < failedTests.length; i++) {
        var test = failedTests[i];
        printToConsole((i + 1) + ". " + test.testId);
        printToConsole("   Description: " + test.description);
        printToConsole("   Error: " + test.message);
        if (test.details && test.details.exception) {
            printToConsole("   Exception: " + test.details.exception);
        }
        printToConsole("");
    }
}

/**
 * Generate CSV format report
 */
function generateCSVReport(testResults) {
    var csv = "TestID,Description,Status,Message,Duration,Timestamp\n";

    if (testResults.tests) {
        for (var i = 0; i < testResults.tests.length; i++) {
            var test = testResults.tests[i];
            var row = [
                test.testId,
                '"' + test.description.replace(/"/g, '""') + '"',
                test.status,
                '"' + (test.message || '').replace(/"/g, '""') + '"',
                test.duration || 0,
                test.timestamp || ''
            ].join(',');
            csv += row + "\n";
        }
    }

    return csv;
}

/**
 * Generate JUnit XML format report
 */
function generateJUnitXMLReport(testResults, suiteName) {
    suiteName = suiteName || "ScopyTestSuite";
    var summary = testResults.summary || {};

    var xml = '<?xml version="1.0" encoding="UTF-8"?>\n';
    xml += '<testsuite name="' + suiteName + '" ';
    xml += 'tests="' + (summary.totalTests || 0) + '" ';
    xml += 'failures="' + (summary.failed || 0) + '" ';
    xml += 'time="' + ((summary.duration || 0) / 1000) + '">\n';

    if (testResults.tests) {
        for (var i = 0; i < testResults.tests.length; i++) {
            var test = testResults.tests[i];
            xml += '  <testcase name="' + test.testId + '" ';
            xml += 'classname="' + suiteName + '" ';
            xml += 'time="' + ((test.duration || 0) / 1000) + '"';

            if (test.status === 'FAIL') {
                xml += '>\n';
                xml += '    <failure message="' + (test.message || '').replace(/"/g, '&quot;') + '">';
                xml += (test.message || '').replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
                xml += '</failure>\n';
                xml += '  </testcase>\n';
            } else {
                xml += '/>\n';
            }
        }
    }

    xml += '</testsuite>\n';
    return xml;
}

/**
 * Print system information
 */
function printSystemInformation() {
    printToConsole("\n🖥️ SYSTEM INFORMATION:");
    printToConsole("-".repeat(30));

    // Device information
    var deviceInfo = getCurrentDeviceInfo();
    if (deviceInfo) {
        printToConsole("📱 Device: " + deviceInfo.uri);
        printToConsole("🔗 Connected: " + (deviceInfo.isConnected ? "Yes" : "No"));
        if (deviceInfo.connectionTime) {
            printToConsole("⏰ Connection Time: " + deviceInfo.connectionTime.toISOString());
        }
    } else {
        printToConsole("📱 Device: Not connected");
    }

    // Tool information
    var toolInfo = getCurrentToolInfo();
    if (toolInfo.currentTool) {
        printToConsole("🔧 Current Tool: " + toolInfo.currentTool);
        printToConsole("🔄 Total Switches: " + toolInfo.totalSwitches);
    }

    printToConsole("🕐 Report Generated: " + new Date().toISOString());
    printToConsole("-".repeat(30));
}

/**
 * Print complete test report
 */
function printCompleteReport(testResults, suiteName) {
    printSystemInformation();
    printTestSummary(testResults, suiteName);
    printDetailedResults(testResults);
    printFailedTestsSummary(testResults);

    // Print final status
    var summary = testResults.summary || testResults;
    var allPassed = summary.failed === 0;

    printToConsole("\n" + "=".repeat(60));
    if (allPassed) {
        printToConsole("🎉 ALL TESTS PASSED! 🎉");
    } else {
        printToConsole("❌ SOME TESTS FAILED - Review results above");
    }
    printToConsole("=".repeat(60));

    return allPassed;
}

/**
 * Create test report file name with timestamp
 */
function createReportFileName(prefix, extension) {
    prefix = prefix || "scopy_test_report";
    extension = extension || "txt";
    var timestamp = new Date().toISOString().replace(/[:.]/g, '-');
    return prefix + "_" + timestamp + "." + extension;
}

/**
 * Print progress indicator during test execution
 */
function printTestProgress(currentTest, totalTests, testName) {
    var progress = Math.round((currentTest / totalTests) * 100);
    var progressBar = "=".repeat(Math.floor(progress / 5)) + "-".repeat(20 - Math.floor(progress / 5));

    printToConsole("[" + progressBar + "] " + progress + "% - " + testName);
}

// Export for other modules
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        generateDetailedTestReport: generateDetailedTestReport,
        printTestSummary: printTestSummary,
        printDetailedResults: printDetailedResults,
        printFailedTestsSummary: printFailedTestsSummary,
        generateCSVReport: generateCSVReport,
        generateJUnitXMLReport: generateJUnitXMLReport,
        printSystemInformation: printSystemInformation,
        printCompleteReport: printCompleteReport,
        createReportFileName: createReportFileName,
        printTestProgress: printTestProgress
    };
}