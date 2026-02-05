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
 * Core System Helpers for Scopy Test Automation
 *
 * Provides utilities for testing core Scopy functionality including
 * tool switching, configuration management, and system operations
 */


/**
 * Safe tool switching with validation
 */
function switchToTool(toolName, waitTime) {
    waitTime = waitTime || 1000;

    printToConsole("🔄 Switching to tool: " + toolName);

    try {
        // Attempt to switch to the tool
        var result = scopy.switchTool(toolName);
        if (!result) {
            throw new Error("Tool switch returned false");
        }

        // Wait for tool to initialize
        safeWait(waitTime);

        printToConsole("✅ Successfully switched to: " + toolName);
        return true;

    } catch (error) {
        printToConsole("❌ Tool switch failed: " + error.toString());
        return false;
    }
}


/**
 * Test tool running/stopping functionality
 */
function testToolRunning(toolName) {
    if (!switchToTool(toolName)) {
        return "Failed to switch to tool: " + toolName;
    }

    try {
        // Check if tool has run button
        if (!scopy.getToolBtnState(toolName)) {
            printToConsole("ℹ️ Tool " + toolName + " has no run button - skipping run test");
            return true;
        }

        // Start the tool
        printToConsole("▶️ Starting tool: " + toolName);
        var startResult = scopy.runTool(toolName, true);
        if (!startResult) {
            return "Failed to start tool: " + toolName;
        }

        safeWait(1000);

        // Stop the tool
        printToConsole("⏹️ Stopping tool: " + toolName);
        var stopResult = scopy.runTool(toolName, false);
        if (!stopResult) {
            return "Failed to stop tool: " + toolName;
        }

        printToConsole("✅ Tool run/stop test successful for: " + toolName);
        return true;

    } catch (error) {
        return "Tool run/stop error: " + error.toString();
    }
}

/**
 * Configuration file management helpers
 */
function testSaveConfiguration(fileName) {
    fileName = fileName || "test_config_" + new Date().getTime();

    printToConsole("💾 Testing configuration save: " + fileName);

    try {
        var result = scopy.saveSetup(fileName);
        if (!result) {
            return "Configuration save returned false";
        }

        printToConsole("✅ Configuration saved successfully");
        return fileName;

    } catch (error) {
        return "Configuration save error: " + error.toString();
    }
}

function testLoadConfiguration(fileName) {
    if (!fileName) {
        return "No filename provided for configuration load";
    }

    printToConsole("📂 Testing configuration load: " + fileName);

    try {
        var result = scopy.loadSetup(fileName);
        if (!result) {
            return "Configuration load returned false";
        }

        // Wait for configuration to apply
        safeWait(2000);

        printToConsole("✅ Configuration loaded successfully");
        return true;

    } catch (error) {
        return "Configuration load error: " + error.toString();
    }
}





/**
 * Validate system prerequisites
 */
function validateSystemPrerequisites() {
    printToConsole("✅ Validating system prerequisites");

    var checks = [];

    // Check if basic Scopy functions are available
    checks.push({
        name: "scopy.addDevice",
        result: typeof scopy.addDevice === 'function'
    });

    checks.push({
        name: "scopy.connectDevice",
        result: typeof scopy.connectDevice === 'function'
    });

    checks.push({
        name: "scopy.switchTool",
        result: typeof scopy.switchTool === 'function'
    });

    checks.push({
        name: "scopy.getTools",
        result: typeof scopy.getTools === 'function'
    });

    var failedChecks = checks.filter(function(check) {
        return !check.result;
    });

    if (failedChecks.length > 0) {
        var failedNames = failedChecks.map(function(check) {
            return check.name;
        });
        return "Missing required functions: " + failedNames.join(", ");
    }

    printToConsole("✅ All system prerequisites validated");
    return true;
}

// Export for other modules
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        switchToTool: switchToTool,
        testToolRunning: testToolRunning,
        testSaveConfiguration: testSaveConfiguration,
        testLoadConfiguration: testLoadConfiguration,
        validateSystemPrerequisites: validateSystemPrerequisites
    };
}