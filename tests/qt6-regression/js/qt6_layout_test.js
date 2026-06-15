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

// Qt6 Regression: Layout Screenshot Test
// Captures screenshots of each loaded tool for visual inspection of
// setContentsMargins regressions after the Qt6 migration.
//
// Globals injected by qt6_layout_runner.py:
//   qt6ScreenshotDir  - output directory (with trailing slash)
//   qt6DeviceName     - device label for filename prefixing

var outDir = qt6ScreenshotDir;
var device = qt6DeviceName;

var deviceId = scopy.addDevice("ip:127.0.0.1");
scopy.connectDevice(deviceId);
msleep(2000);

var tools = scopy.getTools();
if (!tools || tools.length === 0) {
    printToConsole("WARNING: No tools loaded for " + device);
} else {
    printToConsole("Device: " + device + " — tools: " + tools.join(", "));
    for (var i = 0; i < tools.length; i++) {
        var toolName = tools[i];
        var safeName = toolName.replace(/[^a-zA-Z0-9_-]/g, "_");
        try {
            var switched = scopy.switchTool(toolName);
            msleep(500);
            if (switched) {
                scopy.screenshot(outDir + device + "_" + safeName + ".png");
                scopy.screenshotAllScrollAreas(outDir + device + "_" + safeName);
                printToConsole("  Captured: " + toolName);
            }
        } catch (e) {
            printToConsole("  ERROR capturing " + toolName + ": " + e);
        }
    }
}

scopy.disconnectDevice();
printToConsole("Done: " + device);
scopy.exit();
