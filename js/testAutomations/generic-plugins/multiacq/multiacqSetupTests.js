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

// MultiDevice Acq: configure through the `multiacq` API, save a setup, change everything,
// load it back and check it was restored; then load into an empty tool and check the load
// connects the boards itself.
//
// Needs the dummy boards of packages/generic-plugins/plugins/multiacq/test/emu/run_emu.sh:
//   ip:127.0.0.1:30431  Alif E8 no-OS
//   ip:127.0.0.1:30432  Alif E8 Linux
//
// A script that restores a session needs only (the load connects the saved devices):
//   scopy.loadSetup("my_session.ini", "/path/to/dir");
//   multiacq.run();

evaluateFile("js/testAutomations/common/testFramework.js");

var BOARD_A = "ip:127.0.0.1:30431";
var BOARD_B = "ip:127.0.0.1:30432";
var CHANNEL = "adxl345:accel_x";
var SETUP_DIR = "/tmp";
var SETUP_FILE = "multiacq_setup_test.ini";

function findBy(list, key, value) {
    for (var i = 0; i < list.length; i++) {
        if (list[i][key] === value) {
            return list[i];
        }
    }
    return null;
}

function channelOf(uri, id) { return findBy(multiacq.getChannels(uri), "id", id); }

function resetTool() {
    multiacq.stop();
    var plots = multiacq.getPlots();
    for (var i = 0; i < plots.length; i++) {
        multiacq.removePlot(plots[i].uuid);
    }
    var devices = multiacq.getDevices();
    for (var j = 0; j < devices.length; j++) {
        multiacq.removeDevice(devices[j].uri);
    }
}

TestFramework.init("MultiDevice Acq setups");
// No switchToTool(): the tool works unseen, and scopy.switchTool() wants a connected Scopy device.
resetTool();

TestFramework.runTest("TST.MULTIACQ.API_CONFIGURE", function () {
    var err = multiacq.addDevice(BOARD_A) + multiacq.addDevice(BOARD_B);
    if (err !== "") {
        return "Could not add the boards (is run_emu.sh running?): " + err;
    }
    if (!multiacq.setChannelMode(BOARD_A, CHANNEL, "buffered")) return "setChannelMode failed";
    if (!multiacq.setReadInterval(BOARD_B, 250)) return "setReadInterval failed";

    var uuid = multiacq.addPlot("Accel");
    if (uuid < 0) return "addPlot failed";
    if (!multiacq.addCurve(uuid, BOARD_A, CHANNEL)) return "addCurve A failed";
    if (!multiacq.addCurve(uuid, BOARD_B, CHANNEL)) return "addCurve B failed";
    multiacq.setCurveVisible(uuid, BOARD_B, CHANNEL, false);
    multiacq.setSpan(uuid, 30);
    multiacq.setTimeLabels(uuid, "timeofday");
    multiacq.setYRange(uuid, -2, 2);

    var p = multiacq.getPlots()[0];
    return TestFramework.assertEqual(channelOf(BOARD_A, CHANNEL).enabled, true, "addCurve enabled the channel") &&
        TestFramework.assertEqual(p.curves.length, 2, "two curves") &&
        TestFramework.assertEqual(p.autoscale, false, "setYRange turned autoscale off");
});

TestFramework.runTest("TST.MULTIACQ.SETUP_ROUNDTRIP", function () {
    if (!scopy.saveSetup(SETUP_FILE, SETUP_DIR)) return "saveSetup failed";

    // Change everything the setup holds.
    var plots = multiacq.getPlots();
    multiacq.removePlot(plots[0].uuid);
    multiacq.addPlot("Other");
    multiacq.setChannelMode(BOARD_A, CHANNEL, "polled");
    multiacq.setChannelEnabled(BOARD_B, CHANNEL, false);
    multiacq.setReadInterval(BOARD_B, 1000);

    if (!scopy.loadSetup(SETUP_FILE, SETUP_DIR)) return "loadSetup failed";

    plots = multiacq.getPlots();
    var ok = TestFramework.assertEqual(plots.length, 1, "the saved plots replace the current ones") &&
        TestFramework.assertEqual(plots[0].name, "Accel", "plot name") &&
        TestFramework.assertApproxEqual(plots[0].span, 30, 1e-6, "span") &&
        TestFramework.assertEqual(plots[0].timeLabels, "timeofday", "time labels") &&
        TestFramework.assertEqual(plots[0].autoscale, false, "autoscale") &&
        TestFramework.assertApproxEqual(plots[0].yMin, -2, 1e-6, "yMin") &&
        TestFramework.assertApproxEqual(plots[0].yMax, 2, 1e-6, "yMax") &&
        TestFramework.assertEqual(plots[0].curves.length, 2, "both curves") &&
        TestFramework.assertEqual(findBy(plots[0].curves, "uri", BOARD_B).visible, false, "hidden curve") &&
        TestFramework.assertEqual(channelOf(BOARD_A, CHANNEL).mode, "buffered", "channel mode") &&
        TestFramework.assertEqual(channelOf(BOARD_B, CHANNEL).enabled, true, "channel enabled") &&
        TestFramework.assertEqual(findBy(multiacq.getDevices(), "uri", BOARD_B).readIntervalMs, 250,
            "read interval") &&
        TestFramework.assertEqual(multiacq.getLoadReport().length, 0, "nothing skipped");
    return ok;
});

TestFramework.runTest("TST.MULTIACQ.SETUP_AUTOCONNECT", function () {
    // A saved device that is not added is connected by the load, at its saved URI.
    resetTool();
    if (!scopy.loadSetup(SETUP_FILE, SETUP_DIR)) return "loadSetup failed";

    var plots = multiacq.getPlots();
    printToConsole("  load report: " + JSON.stringify(multiacq.getLoadReport()));
    return TestFramework.assertEqual(multiacq.getDevices().length, 2, "both boards connected") &&
        TestFramework.assertEqual(multiacq.getLoadReport().length, 0, "nothing skipped") &&
        TestFramework.assertEqual(plots.length, 1, "the plot is restored") &&
        TestFramework.assertEqual(plots[0].curves.length, 2, "both curves") &&
        TestFramework.assertEqual(channelOf(BOARD_A, CHANNEL).mode, "buffered", "channel mode");
});

resetTool();
TestFramework.printSummary();
