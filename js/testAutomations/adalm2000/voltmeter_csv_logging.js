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

// CSV data logging -- no legacy counterpart: DMM_API had data_logging_en but no
// filename, so logging was never scriptable. The caller greps for FAIL:.
var failures = 0;

function check(name, cond, detail) {
	if (cond) { printToConsole("PASS: " + name); }
	else { printToConsole("FAIL: " + name + " -- " + detail); failures++; }
}

function waitFor(predicate, timeoutMs) {
	var waited = 0;
	while (waited < timeoutMs) {
		if (predicate()) { return true; }
		msleep(250);
		waited += 250;
	}
	return predicate();
}

function hasTool(name) { return scopy.getTools().indexOf(name) !== -1; }

var LOG_PATH = fileIO.getTempPath() + "/adalm2000_voltmeter.csv";
var EXPECTED_HEADER = "Time,CH1 (V),CH2 (V)";

// Dev mode pre-adds the device and addDevice() then returns "". Ids are
// dev_<cat>_<param>_<uuid>; match the delimited uri.
function resolveDevice(uri) {
	var ids = scopy.getDevicesName();
	for (var i = 0; i < ids.length; i++) {
		if (ids[i].indexOf("_" + uri + "_") !== -1) {
			printToConsole("  reusing device already added by Scopy: " + ids[i]);
			// No tools means the connect is still in flight, and we run nested
			// inside it -- it cannot finish until we return, so start our own.
			if (scopy.getTools().length === 0) {
				printToConsole("  not connected yet; connecting from the script");
				scopy.connectDevice(ids[i]);
			}
			return ids[i];
		}
	}
	var id = scopy.addDevice(uri, "iio");
	if (id !== "") { scopy.connectDevice(id); }
	return id;
}

var URI = (typeof scopyDeviceUri !== "undefined" && scopyDeviceUri) ? scopyDeviceUri : "ip:127.0.0.1";
var devId = resolveDevice(URI);
check("device available", devId !== "", "no device could be reused or added for " + URI);

check("Voltmeter tool entry present", waitFor(function () { return hasTool("Voltmeter"); }, 120000),
      "the entry never appeared");
check("calibration released the run button",
      waitFor(function () { return scopy.getToolBtnState("Voltmeter"); }, 120000),
      "run button still hidden after 120s");
check("switched to Voltmeter", scopy.switchTool("Voltmeter"), "switchTool returned false");
msleep(1500);

// --- the empty-path guard -------------------------------------------------
// Enabling with no path must refuse, so this reads back false.
dmm.data_logging_path = "";
msleep(300);
dmm.data_logging_en = true;
msleep(500);
check("logging refuses to start without a path", dmm.data_logging_en == false,
      "data_logging_en read back " + dmm.data_logging_en + " with an empty path");

// --- arm the logger -------------------------------------------------------
// Path before data_logging_en: textChanged disarms logging by design.
printToConsole("  logging to " + LOG_PATH);
dmm.data_logging_path = LOG_PATH;
msleep(500);
check("data_logging_path round-trips", dmm.data_logging_path === LOG_PATH,
      "read back " + dmm.data_logging_path);

dmm.data_logging_en = true;
msleep(500);
check("logging arms once a path is set", dmm.data_logging_en == true,
      "data_logging_en read back " + dmm.data_logging_en);

// --- the disarm-on-path-change contract -----------------------------------
dmm.data_logging_path = fileIO.getTempPath() + "/adalm2000_voltmeter_other.csv";
msleep(500);
check("changing the path disarms logging", dmm.data_logging_en == false,
      "data_logging_en stayed " + dmm.data_logging_en + " after the path changed");

// Re-arm on the real path for the capture below.
dmm.data_logging_path = LOG_PATH;
msleep(500);
dmm.data_logging_en = true;
msleep(500);
check("logging re-arms on the original path", dmm.data_logging_en == true,
      "data_logging_en read back " + dmm.data_logging_en);

// --- capture --------------------------------------------------------------
dmm.mode_ac_ch1 = false;
dmm.mode_ac_ch2 = false;
dmm.gainModes = [1, 1];
msleep(300);

dmm.running = true;
check("acquisition started", waitFor(function () { return dmm.running == true; }, 5000),
      "running read back " + dmm.running);

// ~10 rows/s, so 3 s is ~30 rows.
msleep(3000);

// Stopping closes the logger and unchecks the switch by design, which is also
// what flushes the final row to disk.
dmm.running = false;
msleep(1000);
check("stopping disarms logging", dmm.data_logging_en == false,
      "data_logging_en stayed " + dmm.data_logging_en + " after the run stopped");

// --- verify the file ------------------------------------------------------
var raw = fileIO.readAll(LOG_PATH);
check("log file is not empty", raw.length > 0, "readAll returned nothing for " + LOG_PATH);

var lines = raw.split("\n");
// Drop a trailing empty element from the final newline.
while (lines.length > 0 && lines[lines.length - 1].trim() === "") { lines.pop(); }

check("header is exactly '" + EXPECTED_HEADER + "'", lines[0] === EXPECTED_HEADER,
      "got '" + lines[0] + "'");

var rows = lines.length - 1;
printToConsole("  captured " + rows + " data rows");
check("at least 5 data rows were written", rows >= 5, "only " + rows + " rows after a 3s run");

// Every row is "timestamp,ch1,ch2" with two parseable numbers.
var malformed = 0;
var firstBad = "";
for (var i = 1; i < lines.length; i++) {
	var parts = lines[i].split(",");
	var ok = parts.length === 3 &&
		 parts[0].trim().length > 0 &&
		 !isNaN(parseFloat(parts[1])) &&
		 !isNaN(parseFloat(parts[2]));
	if (!ok) {
		malformed++;
		if (firstBad === "") { firstBad = "line " + (i + 1) + ": '" + lines[i] + "'"; }
	}
}
check("every data row has a timestamp and two numeric channels", malformed === 0,
      malformed + " malformed row(s), first was " + firstBad);

// The timestamp format is dd/MM/yyyy hh:mm:ss.zzz.
var stamp = lines.length > 1 ? lines[1].split(",")[0] : "";
check("timestamp looks like dd/MM/yyyy hh:mm:ss.zzz",
      /^\d{2}\/\d{2}\/\d{4} \d{2}:\d{2}:\d{2}\.\d{3}$/.test(stamp),
      "got '" + stamp + "'");

// A fresh session truncates rather than appends, so re-arming on the same path
// must not leave the previous run's rows in place.
dmm.data_logging_path = LOG_PATH;
msleep(300);
dmm.data_logging_en = true;
msleep(500);
dmm.data_logging_en = false;
msleep(500);
var reopened = fileIO.readAll(LOG_PATH);
var reopenedLines = reopened.split("\n");
while (reopenedLines.length > 0 && reopenedLines[reopenedLines.length - 1].trim() === "") {
	reopenedLines.pop();
}
check("re-opening the same path truncates the previous rows", reopenedLines.length === 1,
      "file still has " + reopenedLines.length + " lines, expected just the header");

check("disconnected", scopy.disconnectDevice(devId), "disconnectDevice returned false");
printToConsole(failures === 0 ? "RESULT: ALL PASSED" : "RESULT: " + failures + " FAILED");
scopy.exit();
