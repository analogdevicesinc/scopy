/*
 * doc_screenshots.js
 *
 * Universal documentation screenshot script for Scopy.
 * Run via tools/doc_screenshots.py which injects the required globals:
 *   scopyPackage  - package name (e.g. "ad9371")
 *   scopyUri      - IIO device URI (e.g. "ip:127.0.0.1")
 *   scopyOutDir   - absolute path to output directory (trailing slash included)
 */

scopy.addDevice(scopyUri);
scopy.connectDevice(0);

var tools = scopyPlugin ? scopy.getToolsForPlugin(scopyPlugin) : scopy.getTools();

tools.forEach(function(tool) {
    scopy.switchTool(tool);
    msleep(500);
    scopy.screenshotFullContent(scopyOutDir + tool + ".png");

    var api = scopy.getCurrentToolApi();
    if(api && typeof api.switchTab === "function") {
        var tabs = api.getTabs();
        tabs.forEach(function(tab) {
            api.switchTab(tab);
            msleep(300);
            scopy.screenshotFullContent(scopyOutDir + tool + "_" + tab + ".png");
        });
    }
});

scopy.exit();
