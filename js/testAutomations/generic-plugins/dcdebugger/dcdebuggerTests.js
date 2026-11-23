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
 */

// DC Debugger Plugin Automated Tests (DCDebugger_API, registered as "dcdebugger")
// Tests: TST.DCDBG.API, TST.DCDBG.TREE, TST.DCDBG.FILTER, TST.DCDBG.ATTR, TST.DCDBG.ACQ,
//        TST.DCDBG.GEN, TST.DCDBG.MONITOR, TST.DCDBG.REGS, TST.DCDBG.PING, TST.DCDBG.LOG,
//        TST.DCDBG.LAYOUT
// Written against iio-emu with packages/generic-plugins/emu-xml/pluto.xml:
//   python3 js/run_tests.py generic-plugins --emu-xml packages/generic-plugins/emu-xml/pluto.xml \
//       --js-file js/testAutomations/generic-plugins/dcdebugger/dcdebuggerTests.js

evaluateFile("js/testAutomations/common/testFramework.js");

TestFramework.init("DC Debugger Plugin Tests");

if (!TestFramework.connectToDevice("ip:127.0.0.1")) {
    printToConsole("ERROR: Cannot connect to the device");
    scopy.exit();
    exit(1);
}

var TOOL = "DC Debugger";
var BACKEND = "iio";

// First node whose path ends with `suffix` (optionally with "/" before it).
function findNode(suffix) {
    var nodes = dcdebugger.getNodes(BACKEND);
    for (var i = 0; i < nodes.length; i++) {
        if (nodes[i] === suffix || nodes[i].endsWith("/" + suffix)) {
            return nodes[i];
        }
    }
    return "";
}

// The device cannot stream (e.g. iio-emu without buffers): the stream tests are skipped then.
function noBuffers(status) {
    return status.indexOf("buffer open failed") !== -1;
}

// Selects the node and checks it shows `view`; "" when it does not.
function selectWithView(suffix, view) {
    var path = findNode(suffix);
    if (path === "" || !dcdebugger.selectNode(path, BACKEND)) {
        return "";
    }
    msleep(300);
    return dcdebugger.getViews().indexOf(view) !== -1 ? path : "";
}

// ============================================
// API registration and device
// ============================================
TestFramework.runTest("TST.DCDBG.API", function() {
    if (typeof dcdebugger === "undefined") {
        printToConsole("  FAIL: dcdebugger API not registered");
        return false;
    }
    var devices = dcdebugger.getDevices();
    printToConsole("  Devices: " + devices);
    if (devices.length === 0 || !dcdebugger.setDevice(devices[devices.length - 1])) {
        printToConsole("  FAIL: no device");
        return false;
    }
    if (dcdebugger.getTools().indexOf(TOOL) === -1) {
        printToConsole("  FAIL: tool list " + dcdebugger.getTools());
        return false;
    }
    if (dcdebugger.setDevice("ip:does-not-exist")) {
        printToConsole("  FAIL: unknown device accepted");
        return false;
    }
    if (!switchToTool(TOOL)) {
        printToConsole("  FAIL: cannot switch to " + TOOL);
        return false;
    }
    return dcdebugger.getBackends().indexOf(BACKEND) !== -1;
});

// ============================================
// Tree navigation
// ============================================
TestFramework.runTest("TST.DCDBG.TREE", function() {
    var nodes = dcdebugger.getNodes(BACKEND);
    printToConsole("  Nodes: " + nodes.length);
    if (nodes.length === 0) {
        return false;
    }
    var phy = findNode("ad9361-phy");
    if (phy === "" || !dcdebugger.selectNode(phy, BACKEND)) {
        printToConsole("  FAIL: cannot select ad9361-phy");
        return false;
    }
    if (!TestFramework.assertEqual(dcdebugger.getSelectedNode(), phy, "Selected node")) {
        return false;
    }
    if (dcdebugger.selectNode("no/such/node", BACKEND)) {
        printToConsole("  FAIL: unknown node selected");
        return false;
    }
    dcdebugger.refresh();
    return true;
});

TestFramework.runTest("TST.DCDBG.FILTER", function() {
    dcdebugger.setFilter("xadc");
    var ok = TestFramework.assertEqual(dcdebugger.getFilter(), "xadc", "Filter text");
    dcdebugger.setFilter("");
    return ok && dcdebugger.getFilter() === "";
});

// ============================================
// Attributes
// ============================================
TestFramework.runTest("TST.DCDBG.ATTR", function() {
    var phy = findNode("ad9361-phy");
    var attrs = dcdebugger.getAttributes(phy, BACKEND);
    printToConsole("  ad9361-phy attributes: " + attrs.length);
    var ensm = "";
    for (var i = 0; i < attrs.length; i++) {
        if (attrs[i].endsWith("/ensm_mode")) {
            ensm = attrs[i];
        }
    }
    if (ensm === "") {
        printToConsole("  FAIL: no ensm_mode attribute");
        return false;
    }
    var original = dcdebugger.readAttribute(ensm, BACKEND);
    printToConsole("  " + ensm + " = " + original);
    if (original === "") {
        return false;
    }
    var options = dcdebugger.getAttributeOptions(ensm, BACKEND);
    printToConsole("  Options: " + options);
    var target = "";
    for (var j = 0; j < options.length; j++) {
        if (options[j] !== original) {
            target = options[j];
            break;
        }
    }
    if (target === "") {
        return "SKIP";
    }
    if (!dcdebugger.writeAttribute(ensm, target, BACKEND)) {
        printToConsole("  FAIL: write " + target);
        return false;
    }
    var readBack = dcdebugger.readAttribute(ensm, BACKEND);
    var ok = TestFramework.assertEqual(readBack, target, "Read back after write");
    ok = TestFramework.assertEqual(dcdebugger.getCachedValue(ensm, BACKEND), readBack, "Cached value") && ok;
    dcdebugger.writeAttribute(ensm, original, BACKEND);
    if (dcdebugger.readAttribute("no/such/attribute", BACKEND) !== "") {
        printToConsole("  FAIL: read of an unknown attribute returned a value");
        return false;
    }
    return ok;
});

// ============================================
// Acquisition (input stream)
// ============================================
TestFramework.runTest("TST.DCDBG.ACQ", function() {
    var stream = selectWithView("cf-ad9361-lpc/Stream", "Acquisition");
    if (stream === "") {
        printToConsole("  FAIL: no Acquisition view on cf-ad9361-lpc/Stream");
        return false;
    }
    var channels = dcdebugger.getAcquisitionChannels();
    printToConsole("  Channels: " + channels);
    if (channels.length === 0 || !dcdebugger.setAcquisitionChannelEnabled(channels[0], true)) {
        return false;
    }
    if (!dcdebugger.setAcquisitionSamples(1024) ||
        !TestFramework.assertEqual(dcdebugger.getAcquisitionSamples(), 1024, "Samples")) {
        return false;
    }
    if (!dcdebugger.setAcquisitionMode("Time") || dcdebugger.setAcquisitionMode("Bogus")) {
        return false;
    }
    if (!dcdebugger.singleAcquisition(10000)) {
        var status = dcdebugger.getAcquisitionStatus();
        if (noBuffers(status)) {
            printToConsole("  SKIP: the device cannot open buffers (" + status + ")");
            return "SKIP";
        }
        printToConsole("  FAIL: single acquisition: " + status);
        return false;
    }
    var rows = JSON.parse(dcdebugger.getMeasurements());
    printToConsole("  Measurements: " + JSON.stringify(rows));
    if (rows.length === 0) {
        return false;
    }
    if (!dcdebugger.exportAcquisitionCsv("/tmp/dcdebugger_acquisition.csv")) {
        printToConsole("  FAIL: CSV export");
        return false;
    }
    dcdebugger.setAcquisitionMode("Spectrum");
    var ok = TestFramework.assertEqual(dcdebugger.getAcquisitionMode(), "Spectrum", "Mode");
    dcdebugger.setAcquisitionMode("Time");

    if (!dcdebugger.startAcquisition()) {
        printToConsole("  FAIL: start");
        return false;
    }
    msleep(1000);
    ok = TestFramework.assertEqual(dcdebugger.isAcquisitionRunning(), true, "Running") && ok;
    dcdebugger.stopAcquisition();
    msleep(500);
    return TestFramework.assertEqual(dcdebugger.isAcquisitionRunning(), false, "Stopped") && ok;
});

// ============================================
// Generator (output stream)
// ============================================
TestFramework.runTest("TST.DCDBG.GEN", function() {
    var stream = selectWithView("cf-ad9361-dds-core-lpc/Stream", "Generator");
    if (stream === "") {
        printToConsole("  FAIL: no Generator view on cf-ad9361-dds-core-lpc/Stream");
        return false;
    }
    var channels = dcdebugger.getGeneratorChannels();
    printToConsole("  Channels: " + channels + " waveforms: " + dcdebugger.getWaveforms());
    if (channels.length === 0) {
        return false;
    }
    if (!dcdebugger.setGeneratorChannelEnabled(channels[0], true) ||
        !dcdebugger.setGeneratorTone(channels[0], "Sine", 1000, 0.5, 0, 0) ||
        dcdebugger.setGeneratorTone(channels[0], "Bogus", 1000, 0.5, 0, 0)) {
        return false;
    }
    if (!dcdebugger.startGenerator()) {
        printToConsole("  FAIL: start");
        return false;
    }
    msleep(500);
    var status = dcdebugger.getGeneratorStatus();
    printToConsole("  Status: " + status);
    if (!dcdebugger.isGeneratorRunning() && noBuffers(status)) {
        printToConsole("  SKIP: the device cannot open buffers");
        return "SKIP";
    }
    var ok = TestFramework.assertEqual(dcdebugger.isGeneratorRunning(), true, "Running");
    dcdebugger.stopGenerator();
    msleep(500);
    return TestFramework.assertEqual(dcdebugger.isGeneratorRunning(), false, "Stopped") && ok;
});

// ============================================
// Monitor (device)
// ============================================
TestFramework.runTest("TST.DCDBG.MONITOR", function() {
    var dev = selectWithView("xadc", "Monitor");
    if (dev === "") {
        printToConsole("  FAIL: no Monitor view on xadc");
        return false;
    }
    var channels = dcdebugger.getMonitorChannels();
    printToConsole("  Channels: " + channels);
    if (channels.length === 0 || !dcdebugger.setMonitorChannelEnabled(channels[0], true)) {
        return false;
    }
    dcdebugger.setMonitorPeriod(0.1);
    if (!dcdebugger.startMonitor()) {
        printToConsole("  FAIL: start");
        return false;
    }
    msleep(1500);
    var values = JSON.parse(dcdebugger.getMonitorValues());
    printToConsole("  Values: " + JSON.stringify(values));
    dcdebugger.stopMonitor();
    var sampled = values.length > 0 && values[0].count > 0;
    var ok = TestFramework.assertEqual(sampled, true, "Monitor sampled");
    ok = TestFramework.assertEqual(dcdebugger.isMonitorRunning(), false, "Stopped") && ok;
    return dcdebugger.clearMonitor() && ok;
});

// ============================================
// Registers (device)
// ============================================
TestFramework.runTest("TST.DCDBG.REGS", function() {
    var dev = selectWithView("cf-ad9361-lpc", "Registers");
    if (dev === "") {
        printToConsole("  FAIL: no Registers view on cf-ad9361-lpc");
        return false;
    }
    var targets = dcdebugger.getRegisterTargets();
    printToConsole("  Targets: " + targets);
    if (targets.length === 0 || !dcdebugger.setRegisterTarget(0) || dcdebugger.setRegisterTarget(targets.length)) {
        return false;
    }
    var value = dcdebugger.readRegister("0x0");
    printToConsole("  0x0 = " + value);
    if (value === "") {
        printToConsole("  FAIL: register read");
        return false;
    }
    if (!dcdebugger.writeRegister("0x0", value)) {
        printToConsole("  FAIL: register write");
        return false;
    }
    return dcdebugger.readRegister("not-hex") === "";
});

// ============================================
// Ping
// ============================================
TestFramework.runTest("TST.DCDBG.PING", function() {
    // The connection's ping is no context's child: no backend.
    if (!dcdebugger.selectNode("Ping", "") || dcdebugger.getViews().indexOf("Ping") === -1) {
        printToConsole("  SKIP: no Ping node");
        return "SKIP";
    }
    var reachable = dcdebugger.ping();
    return TestFramework.assertEqual(reachable, true, "Device reachable");
});

// ============================================
// Command log and code generation
// ============================================
TestFramework.runTest("TST.DCDBG.LOG", function() {
    if (!dcdebugger.setLogEnabled(true) || !dcdebugger.isLogEnabled()) {
        printToConsole("  FAIL: cannot enable the log");
        return false;
    }
    dcdebugger.clearLog();
    var phy = findNode("ad9361-phy");
    var attrs = dcdebugger.getAttributes(phy, BACKEND);
    for (var i = 0; i < Math.min(3, attrs.length); i++) {
        dcdebugger.readAttribute(attrs[i], BACKEND);
    }
    msleep(300);
    var count = dcdebugger.getLogEntryCount();
    printToConsole("  Log entries: " + count);
    if (count === 0) {
        return false;
    }
    if (dcdebugger.getLogCsv().length === 0 || !dcdebugger.exportLogCsv("/tmp/dcdebugger_log.csv")) {
        return false;
    }
    dcdebugger.setLogFilter("no-such-component");
    dcdebugger.setLogFilter("");
    var languages = dcdebugger.getCodeLanguages();
    printToConsole("  Code languages: " + languages);
    var ok = true;
    if (languages.length > 0) {
        var code = dcdebugger.generateCode(languages[0]);
        printToConsole("  Generated " + code.length + " characters of " + languages[0]);
        ok = code.length > 0 && dcdebugger.saveCode("/tmp/dcdebugger_code.txt", languages[0]);
        ok = dcdebugger.generateCode("NoSuchLanguage") === "" && ok;
    }
    dcdebugger.clearLog();
    ok = TestFramework.assertEqual(dcdebugger.getLogEntryCount(), 0, "Cleared") && ok;
    dcdebugger.setLogEnabled(false);
    return TestFramework.assertEqual(dcdebugger.isLogEnabled(), false, "Log off") && ok;
});

// ============================================
// Layout (saved / loaded with the session)
// ============================================
TestFramework.runTest("TST.DCDBG.LAYOUT", function() {
    var layout = dcdebugger.layout;
    var keys = Object.keys(layout);
    printToConsole("  Layout keys: " + keys);
    if (keys.indexOf("tool/mainSplit") === -1) {
        printToConsole("  FAIL: the tool split is missing");
        return false;
    }
    dcdebugger.layout = layout;
    var again = Object.keys(dcdebugger.layout);
    var ok = TestFramework.assertEqual(again.indexOf("tool/mainSplit") !== -1, true, "Layout round trip");
    dcdebugger.resetLayout();
    dcdebugger.layout = layout;
    return ok;
});

TestFramework.disconnectFromDevice();

var exitCode = TestFramework.printSummary();
scopy.exit();
exit(exitCode);
