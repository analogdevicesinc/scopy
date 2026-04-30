/*
 * Copyright (c) 2026 Analog Devices Inc.
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
 *
 */

/*
 * screenshots.js
 *
 * Universal documentation screenshot script for Scopy.
 * Run via tools/screenshots/screenshots.py which injects the required globals:
 *   scopyUri          - IIO device URI (e.g. "ip:127.0.0.1")
 *   scopyOutDir       - absolute path to output directory (trailing slash included)
 *   scopySkipPlugins  - JSON array of plugin names to skip (e.g. ["ADCPlugin", "DACPlugin"])
 */

var deviceId = scopy.addDevice(scopyUri);
scopy.connectDevice(deviceId);

var skipList = scopySkipPlugins || [];
var usePluginDirs = scopyUsePluginDirs || false;
var plugins = scopy.getPlugins(scopyUri);

function sanitize(name) {
    return name.replace(/\//g, "_");
}

plugins.forEach(function(plugin) {
    if(skipList.indexOf(plugin) !== -1) return;

    var tools = scopy.getToolsForPlugin(plugin);
    if(tools.length === 0) return;

    var baseDir = usePluginDirs ? scopyOutDir + plugin + "/" : scopyOutDir;
    tools.forEach(function(tool) {
        scopy.switchTool(tool);
        msleep(500);

        var safeTool = sanitize(tool);
        // Full window screenshot
        scopy.screenshot(baseDir + safeTool + ".png");

        // Capture all visible scroll areas (menus/panels with full content)
        scopy.screenshotAllScrollAreas(baseDir + safeTool);

        // Capture tabs if the tool has any
        var tabs = scopy.getTabs();
        if(tabs.length > 0) {
            tabs.forEach(function(tab) {
                if(!tab || tab.trim() === "") return;
                scopy.switchTab(tab);
                msleep(300);
                var safeTab = sanitize(tab);
                scopy.screenshot(baseDir + safeTool + "_" + safeTab + ".png");
                scopy.screenshotAllScrollAreas(baseDir + safeTool + "_" + safeTab);
            });
        }
    });
});

// Capture non-tool pages: home, about, preferences
var pages = ["home", "about", "preferences"];
pages.forEach(function(page) {
    scopy.showPage(page);
    msleep(500);

    scopy.screenshot(scopyOutDir + page + ".png");
    scopy.screenshotAllScrollAreas(scopyOutDir + page);

    var tabs = scopy.getTabs();
    if(tabs.length > 0) {
        tabs.forEach(function(tab) {
            if(!tab || tab.trim() === "") return;
            scopy.switchTab(tab);
            msleep(300);
            scopy.screenshot(scopyOutDir + page + "_" + tab + ".png");
            scopy.screenshotAllScrollAreas(scopyOutDir + page + "_" + tab);
        });
    }
});

scopy.disconnectDevice();
