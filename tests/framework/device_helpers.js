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
 * Device Management Helpers for Scopy Test Automation
 *
 * Provides robust device connection, management, and validation utilities
 */

// Global device state tracking
var deviceState = {
    connectedDevices: [],
    currentDevice: null,
    connectionAttempts: 0,
    lastConnectionTime: null
};

/**
 * Device connection with enhanced error handling and validation
 */
function connectToDevice(deviceURI, calibrationWait) {
    deviceURI = deviceURI || getTestConfig("deviceIP") || "ip:192.168.2.1";
    calibrationWait = calibrationWait || getTestConfig("calibrationWait") || 3000;

    printToConsole("🔗 Attempting to connect to device: " + deviceURI);
    deviceState.connectionAttempts++;

    try {
        // Add device to Scopy
        var deviceId = scopy.addDevice(deviceURI);
        if (!deviceId || deviceId === "") {
            throw new Error("Failed to add device: " + deviceURI);
        }

        // Connect to the device
        var connected = scopy.connectDevice(deviceId);
        if (!connected) {
            throw new Error("Failed to connect to device: " + deviceURI);
        }

        // Wait for device calibration/initialization
        printToConsole("⏳ Waiting for device calibration...");
        safeWait(calibrationWait);

        // Verify connection by checking if tools are available
        // Note: scopy.isDeviceConnected() doesn't exist, so we use getTools() as verification
        var tools = scopy.getTools();
        if (!tools || tools.length === 0) {
            throw new Error("No tools available - device connection may have failed");
        }

        // Update state tracking
        deviceState.currentDevice = {
            id: deviceId,
            uri: deviceURI,
            connectionTime: new Date(),
            tools: tools || []
        };

        deviceState.connectedDevices.push(deviceState.currentDevice);
        deviceState.lastConnectionTime = new Date();

        printToConsole("✅ Successfully connected to device: " + deviceURI);
        printToConsole("📱 Device ID: " + deviceId);
        printToConsole("🔧 Available tools: " + tools.length);

        return deviceId;

    } catch (error) {
        printToConsole("❌ Device connection failed: " + error.toString());
        return null;
    }
}

/**
 * Safe device disconnection with cleanup
 */
function disconnectFromDevice(deviceId, removeDevice) {
    removeDevice = removeDevice !== false; // Default to true

    if (!deviceId && deviceState.currentDevice) {
        deviceId = deviceState.currentDevice.id;
    }

    if (!deviceId) {
        printToConsole("⚠️ No device to disconnect");
        return true;
    }

    printToConsole("🔌 Disconnecting from device: " + deviceId);

    try {
        // Stop any running tools first
        stopAllRunningTools(deviceId);

        // Disconnect the device
        var disconnected = scopy.disconnectDevice(deviceId);
        if (!disconnected) {
            printToConsole("⚠️ Device disconnection returned false");
        }

        // Remove device if requested (removeDevice takes URI, not deviceID)
        if (removeDevice) {
            // Find the device URI for removal
            var deviceURI = null;
            for (var i = 0; i < deviceState.connectedDevices.length; i++) {
                if (deviceState.connectedDevices[i].id === deviceId) {
                    deviceURI = deviceState.connectedDevices[i].uri;
                    break;
                }
            }

            if (deviceURI) {
                var removed = scopy.removeDevice(deviceURI);
                if (!removed) {
                    printToConsole("⚠️ Device removal returned false for URI: " + deviceURI);
                }
            } else {
                printToConsole("⚠️ Could not find device URI for removal");
            }
        }

        // Update state tracking
        if (deviceState.currentDevice && deviceState.currentDevice.id === deviceId) {
            deviceState.currentDevice = null;
        }

        // Remove from connected devices list
        deviceState.connectedDevices = deviceState.connectedDevices.filter(function(device) {
            return device.id !== deviceId;
        });

        printToConsole("✅ Device disconnected successfully");
        return true;

    } catch (error) {
        printToConsole("❌ Device disconnection error: " + error.toString());
        return false;
    }
}

/**
 * Stop all running tools for a device
 */
function stopAllRunningTools(deviceId) {
    if (!deviceId) return;

    try {
        var availableTools = scopy.getTools();
        for (var i = 0; i < availableTools.length; i++) {
            var toolName = availableTools[i];
            try {
                if (scopy.getToolBtnState && scopy.getToolBtnState(toolName)) {
                    scopy.runTool(toolName, false);
                }
            } catch (toolError) {
                // Continue with other tools
            }
        }
    } catch (error) {
        printToConsole("⚠️ Error stopping tools: " + error.toString());
    }
}

/**
 * Test device connection reliability
 */
function testDeviceConnection(deviceURI, attempts) {
    attempts = attempts || 3;
    var successCount = 0;

    printToConsole("🔄 Testing device connection reliability (" + attempts + " attempts)");

    for (var i = 0; i < attempts; i++) {
        printToConsole("Attempt " + (i + 1) + "/" + attempts);

        var deviceId = connectToDevice(deviceURI, 1000); // Shorter calibration wait for reliability test
        if (deviceId) {
            // Test basic functionality
            var toolsAvailable = getAvailableTools(deviceId);
            if (toolsAvailable && toolsAvailable.length > 0) {
                successCount++;
                printToConsole("✅ Connection test " + (i + 1) + " successful");
            } else {
                printToConsole("❌ Connection test " + (i + 1) + " failed - no tools available");
            }

            // Disconnect and remove device to clean state for next attempt
            disconnectFromDevice(deviceId, true);
            safeWait(1000); // Wait for full disconnection
        } else {
            printToConsole("❌ Connection test " + (i + 1) + " failed - couldn't connect");
        }

        // Wait between attempts
        if (i < attempts - 1) {
            safeWait(2000); // Longer wait between attempts
        }
    }

    var successRate = (successCount / attempts) * 100;
    printToConsole("📊 Connection reliability: " + successCount + "/" + attempts + " (" + successRate.toFixed(1) + "%)");

    return successRate >= 80; // Consider 80%+ success rate as acceptable
}

/**
 * Get available tools for current device
 */
function getAvailableTools(deviceId) {
    try {
        // Note: We can't check device connection status, so just try to get tools
        var tools = scopy.getTools();
        if (tools && tools.length > 0) {
            printToConsole("🔧 Available tools: " + tools.join(", "));
            return tools;
        } else {
            printToConsole("⚠️ No tools available");
            return [];
        }
    } catch (error) {
        printToConsole("❌ Error getting available tools: " + error.toString());
        return [];
    }
}

/**
 * Get tools for specific plugin
 */
function getPluginTools(pluginName) {
    try {
        var tools = scopy.getToolsForPlugin(pluginName);
        if (tools && tools.length > 0) {
            printToConsole("🔧 Tools for " + pluginName + ": " + tools.join(", "));
            return tools;
        } else {
            printToConsole("⚠️ No tools available for plugin: " + pluginName);
            return [];
        }
    } catch (error) {
        printToConsole("❌ Error getting plugin tools: " + error.toString());
        return [];
    }
}

/**
 * Verify device connection by checking tool availability
 */
function verifyDeviceConnection(deviceId) {
    try {
        // Check if we can get tools (indicates device is connected and working)
        var tools = scopy.getTools();
        if (!tools || tools.length === 0) {
            return false;
        }

        printToConsole("✅ Device verification passed - " + tools.length + " tools available");
        return true;

    } catch (error) {
        printToConsole("❌ Device verification failed: " + error.toString());
        return false;
    }
}

/**
 * Get current device information
 */
function getCurrentDeviceInfo() {
    if (deviceState.currentDevice) {
        return {
            id: deviceState.currentDevice.id,
            uri: deviceState.currentDevice.uri,
            connectionTime: deviceState.currentDevice.connectionTime,
            isConnected: deviceState.currentDevice !== null // Simple check without API calls
        };
    }
    return null;
}

/**
 * Cleanup all devices and connections
 */
function cleanupAllConnections() {
    printToConsole("🧹 Cleaning up all device connections...");

    // Disconnect all tracked devices
    for (var i = 0; i < deviceState.connectedDevices.length; i++) {
        disconnectFromDevice(deviceState.connectedDevices[i].id, true);
    }

    // Reset state
    deviceState.connectedDevices = [];
    deviceState.currentDevice = null;

    printToConsole("✅ Connection cleanup completed");
}

// Connection state utilities
function isDeviceConnected() {
    return deviceState.currentDevice !== null;
}

function getConnectionAttempts() {
    return deviceState.connectionAttempts;
}

function getLastConnectionTime() {
    return deviceState.lastConnectionTime;
}

// Export for other modules
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        connectToDevice: connectToDevice,
        disconnectFromDevice: disconnectFromDevice,
        testDeviceConnection: testDeviceConnection,
        getAvailableTools: getAvailableTools,
        getPluginTools: getPluginTools,
        verifyDeviceConnection: verifyDeviceConnection,
        getCurrentDeviceInfo: getCurrentDeviceInfo,
        cleanupAllConnections: cleanupAllConnections,
        isDeviceConnected: isDeviceConnected,
        getConnectionAttempts: getConnectionAttempts,
        getLastConnectionTime: getLastConnectionTime
    };
}