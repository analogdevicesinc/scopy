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

// ADALM2000 Digital I/O hardware loopback suite.
//
// Required wiring: DIO0-DIO8, DIO1-DIO9, ... DIO7-DIO15.
// dio::groupValue() is LSB-first within a group, so pin i -> pin i+8 means
// group 0's value equals group 1's value when one drives the other.
//
// Does not use TestFramework.connectToDevice(): it falls back to the emulator
// when the board does not answer, which would void every loopback assertion.

evaluateFile("js/testAutomations/common/testFramework.js");

var URI = (typeof scopyDeviceUri !== "undefined" && scopyDeviceUri) ? scopyDeviceUri : "ip:192.168.2.1";
var TOOL = "Digital I/O";

// Walking ones isolate a dead line or swapped jumper to one pin; 0x55/0xAA are
// complements, so every adjacent pair differs and shorts show up.
var VALUES = [0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x55, 0xaa, 0xff];

var preflightOk = false;

function hex(v) {
	if (v < 0) {
		return String(v);
	}
	var s = v.toString(16).toUpperCase();
	while (s.length < 4) {
		s = "0" + s;
	}
	return "0x" + s;
}

function waitFor(predicate, timeoutMs) {
	var waited = 0;
	while (waited < timeoutMs) {
		if (predicate()) {
			return true;
		}
		msleep(250);
		waited += 250;
	}
	return predicate();
}

// Device ids are dev_<cat>_<param>_<uuid>, so the uri appears delimited.
function findDevice(uri) {
	var ids = scopy.getDevicesName();
	for (var i = 0; i < ids.length; i++) {
		if (ids[i].indexOf("_" + uri + "_") !== -1) {
			return ids[i];
		}
	}
	return "";
}

function requireHardware(uri) {
	var id = findDevice(uri);
	if (id === "") {
		id = scopy.addDevice(uri, "iio");
	}
	if (id === "") {
		return "";
	}
	// connectDevice() returns once connectDev() has been requested, not once it
	// succeeded, and the connect runs from the event loop -- so the tool list is
	// the only real success signal.
	scopy.connectDevice(id);
	return id;
}

// The setGroupDirection(g,false) calls are load-bearing: a per-pin write leaves
// the group's cached direction stale, and entering group mode re-applies it to
// all eight pins -- a cache left at "out" makes both halves drive one jumper.
function dioReset() {
	dio.setRunning(false);
	dio.setDirectionWord(0x0000);
	dio.setOutputWord(0x0000);
	dio.setGroupDirection(0, false);
	dio.setGroupDirection(1, false);
	dio.setGroupMode(0, false);
	dio.setGroupMode(1, false);
}

// Run last -- writePinValue() is a no-op while output_enabled is false. Polling
// off -- its tick can fire inside QCoro::waitFor's loop and restore a stale word.
function arm() {
	if (!dio.setRunning(true)) {
		return false;
	}
	msleep(50);
	dio.setPolling(false);
	return true;
}

function disarm() {
	dio.setPolling(true);
	dio.setRunning(false);
	dioReset();
}

// Reads the input word once and reports both the value and whether a retry was
// needed. "Failed then passed on retry" is a timing root cause and "failed
// twice" is a logic one, and that distinction is what the findings report needs.
function readWordChecked(expected, mask) {
	var w = dio.readInputWord();
	if (w >= 0 && (w & mask) === (expected & mask)) {
		return { word: w, retried: false };
	}
	msleep(50);
	var w2 = dio.readInputWord();
	return { word: w2, retried: true, firstWord: w };
}

function reportMismatch(label, expected, got) {
	printToConsole("    " + label + ": expected " + hex(expected) + ", got " + hex(got));
	printToConsole("    diagnostics: dirWord=" + hex(dio.directionWord()) + " outWord=" +
		       hex(dio.outputWord()) + " shorted=" + hex(dio.shortedWord()) +
		       " groupOut=[" + dio.groupOutput(0) + "," + dio.groupOutput(1) + "]" +
		       " groupMode=[" + dio.isGroupMode(0) + "," + dio.isGroupMode(1) + "]");
}

TestFramework.init("ADALM2000 Digital I/O Loopback (hardware)");

var devId = requireHardware(URI);
if (devId === "") {
	printToConsole("ERROR: no device at " + URI + " -- refusing to fall back to an emulator");
	throw new Error("hardware required");
}

if (!waitFor(function () { return scopy.getTools().indexOf(TOOL) !== -1; }, 120000)) {
	printToConsole("ERROR: the " + TOOL + " tool entry never appeared");
	throw new Error("tool entry missing");
}

// onConnect() creates the tools disabled; calibrateAsync() enables them and sets
// the run button visible only once ADC calibration finishes, and getToolBtnState
// returns exactly that flag. Without this the suite races calibration.
if (!waitFor(function () { return scopy.getToolBtnState(TOOL); }, 120000)) {
	printToConsole("ERROR: calibration never completed (run button never became visible)");
	throw new Error("calibration timeout");
}

if (!scopy.switchTool(TOOL)) {
	printToConsole("ERROR: switchTool(" + TOOL + ") failed");
	throw new Error("switchTool failed");
}
msleep(1000);

// ============================================================================
TestFramework.runTest("TST.ADALM2000.DIO_WIRING_PREFLIGHT", function () {
	try {
		printToConsole("  uri: " + URI + "  device: " + devId);
		printToConsole("  devices: " + scopy.getDevicesName().join(", "));
		// getPreference() returns a QPair, which does not marshal into JS;
		// getPreferences() returns a QVariantMap, which becomes a plain object.
		printToConsole("  dc_cmd_execution_parallel: " +
			       scopy.getPreferences()["dc_cmd_execution_parallel"]);

		if (typeof dio === "undefined") {
			return "the dio JS global does not exist";
		}
		if (typeof dio.readInputWord !== "function") {
			return "the Q_INVOKABLE surface is missing -- stale build?";
		}
		if (!dio.isUsable()) {
			return "the controller reports the digital device as incomplete";
		}

		var allPass = true;
		var pattern = [0x00, 0xff, 0x55, 0xaa];
		var i, v, r;

		// Forward: 0-7 drive, 8-15 listen.
		dioReset();
		dio.setDirectionWord(0x00ff);
		if (!arm()) {
			return "setRunning(true) did not take effect";
		}
		for (i = 0; i < pattern.length; i++) {
			v = pattern[i];
			dio.setOutputWord(v);
			r = readWordChecked(v << 8, 0xff00);
			var hi = (r.word >= 0) ? ((r.word >> 8) & 0xff) : -1;
			if (hi !== v) {
				reportMismatch("forward " + hex(v), v, hi);
				allPass = false;
			}
		}
		disarm();

		// Reverse: 8-15 drive, 0-7 listen. Proves the jumpers conduct both ways
		// and that no pin is stuck.
		dio.setDirectionWord(0xff00);
		if (!arm()) {
			return "setRunning(true) did not take effect (reverse)";
		}
		for (i = 0; i < pattern.length; i++) {
			v = pattern[i];
			dio.setOutputWord(v << 8);
			r = readWordChecked(v, 0x00ff);
			var lo = (r.word >= 0) ? (r.word & 0xff) : -1;
			if (lo !== v) {
				reportMismatch("reverse " + hex(v), v, lo);
				allPass = false;
			}
		}
		disarm();

		if (!allPass) {
			printToConsole("  ABORT: pre-flight failed -- the loopback results that follow " +
				       "would be meaningless, so they are skipped");
			return "wiring pre-flight failed; check the DIO0-DIO8 .. DIO7-DIO15 jumpers";
		}
		preflightOk = true;
		printToConsole("  jumpers verified in both directions");
		return true;
	} catch (e) {
		printToConsole("  Error: " + e);
		return false;
	}
});

// ============================================================================
// Test 1: both halves in Group mode. 8-15 output, 0-7 input.
TestFramework.runTest("TST.ADALM2000.DIO_GROUP_TO_GROUP", function () {
	if (!preflightOk) {
		return "SKIP";
	}
	try {
		var allPass = true;
		dioReset();
		// Mode before direction: entering group mode re-applies the cached
		// direction to all eight pins.
		dio.setGroupMode(0, true);
		dio.setGroupMode(1, true);
		dio.setGroupDirection(1, true);
		if (!arm()) {
			return "setRunning(true) did not take effect";
		}

		for (var i = 0; i < VALUES.length; i++) {
			var v = VALUES[i];
			if (!dio.setGroupValue(1, v)) {
				printToConsole("    setGroupValue(1, " + v + ") returned false");
				allPass = false;
				continue;
			}
			var r = readWordChecked(v, 0x00ff);
			var got = dio.groupInput(0);
			if (got !== v) {
				reportMismatch("group 1 drove " + hex(v) + ", group 0 read", v, got);
				allPass = false;
			} else if (r.retried) {
				printToConsole("    " + hex(v) + " needed a retry -- timing");
				allPass = false;
			}
		}
		disarm();
		return allPass;
	} catch (e) {
		printToConsole("  Error: " + e);
		return false;
	}
});

// ============================================================================
// Test 2: both halves Individual. 0-7 output, 8-15 input, one pair at a time.
TestFramework.runTest("TST.ADALM2000.DIO_INDIVIDUAL_TO_INDIVIDUAL", function () {
	if (!preflightOk) {
		return "SKIP";
	}
	try {
		var allPass = true;
		dioReset();
		dio.setGroupMode(0, false);
		dio.setGroupMode(1, false);
		dio.setDirectionWord(0x00ff);
		if (!arm()) {
			return "setRunning(true) did not take effect";
		}

		for (var p = 0; p < 8; p++) {
			var levels = [1, 0, 1];
			for (var k = 0; k < levels.length; k++) {
				var level = levels[k];
				if (!dio.setPinOutput(p, level === 1)) {
					printToConsole("    setPinOutput(" + p + ") returned false");
					allPass = false;
					continue;
				}
				var expectedHi = (level === 1) ? (1 << p) : 0x00;
				var r = readWordChecked(expectedHi << 8, 0xff00);
				var got = dio.pinInput(p + 8);
				if (got !== level) {
					reportMismatch("pin " + p + "=" + level + " -> pin " + (p + 8),
						       level, got);
					allPass = false;
				}
				// The whole-word check is what catches a swapped jumper or
				// crosstalk that a single-pin assertion passes.
				var hi = (r.word >= 0) ? ((r.word >> 8) & 0xff) : -1;
				if (hi !== expectedHi) {
					reportMismatch("pin " + p + "=" + level + " input half",
						       expectedHi, hi);
					allPass = false;
				}
			}
			dio.setPinOutput(p, false);
		}
		disarm();
		return allPass;
	} catch (e) {
		printToConsole("  Error: " + e);
		return false;
	}
});

// ============================================================================
// Test 3: 0-7 Individual + output drives the bits; 8-15 Group + input reads the
// composed value.
TestFramework.runTest("TST.ADALM2000.DIO_INDIVIDUAL_TO_GROUP", function () {
	if (!preflightOk) {
		return "SKIP";
	}
	try {
		var allPass = true;
		dioReset();
		dio.setGroupMode(0, false);
		dio.setGroupMode(1, true);
		// Per-pin, not setGroupDirection(0,true): this test is about individual
		// bits, and the group attribute would fan out instead.
		dio.setDirectionWord(0x00ff);
		if (!arm()) {
			return "setRunning(true) did not take effect";
		}

		for (var i = 0; i < VALUES.length; i++) {
			var v = VALUES[i];
			var wrote = true;
			for (var p = 0; p < 8; p++) {
				wrote = dio.setPinOutput(p, ((v >> p) & 1) === 1) && wrote;
			}
			if (!wrote) {
				printToConsole("    a per-pin write returned false for " + hex(v));
				allPass = false;
			}
			var r = readWordChecked(v << 8, 0xff00);
			var got = dio.groupInput(1);
			if (got !== v) {
				reportMismatch("pins 0-7 drove " + hex(v) + ", group 1 read", v, got);
				allPass = false;
			} else if (r.retried) {
				printToConsole("    " + hex(v) + " needed a retry -- timing");
				allPass = false;
			}
		}
		disarm();
		return allPass;
	} catch (e) {
		printToConsole("  Error: " + e);
		return false;
	}
});

// ============================================================================
// Test 4: 8-15 Group + output writes the value; 0-7 Individual + input reads the
// bits.
TestFramework.runTest("TST.ADALM2000.DIO_GROUP_TO_INDIVIDUAL", function () {
	if (!preflightOk) {
		return "SKIP";
	}
	try {
		var allPass = true;
		dioReset();
		dio.setGroupMode(0, false);
		dio.setGroupMode(1, true);
		dio.setGroupDirection(1, true);
		if (!arm()) {
			return "setRunning(true) did not take effect";
		}

		for (var i = 0; i < VALUES.length; i++) {
			var v = VALUES[i];
			if (!dio.setGroupValue(1, v)) {
				printToConsole("    setGroupValue(1, " + v + ") returned false");
				allPass = false;
				continue;
			}
			var r = readWordChecked(v, 0x00ff);
			var bitsOk = true;
			for (var p = 0; p < 8; p++) {
				var want = (v >> p) & 1;
				var got = dio.pinInput(p);
				if (got !== want) {
					reportMismatch("group 1 = " + hex(v) + " -> pin " + p, want, got);
					bitsOk = false;
				}
			}
			if (!bitsOk) {
				allPass = false;
			} else if (r.retried) {
				printToConsole("    " + hex(v) + " needed a retry -- timing");
				allPass = false;
			}
		}
		disarm();
		return allPass;
	} catch (e) {
		printToConsole("  Error: " + e);
		return false;
	}
});

// ============================================================================
// Regression test for the group-vs-per-pin cache divergence and the whole-list
// property writes that fed on it. Runs last: it deliberately leaves odd state.
TestFramework.runTest("TST.ADALM2000.DIO_CACHE_COHERENCE", function () {
	if (!preflightOk) {
		return "SKIP";
	}
	try {
		var allPass = true;
		dioReset();
		dio.setGroupMode(1, true);
		dio.setGroupDirection(1, true);
		if (!arm()) {
			return "setRunning(true) did not take effect";
		}

		// A group write must leave the eight per-pin caches agreeing with it.
		dio.setGroupValue(1, 0xff);
		if (dio.outputWord() !== 0xff00) {
			reportMismatch("per-pin value cache after a group write", 0xff00, dio.outputWord());
			allPass = false;
		}
		if (dio.directionWord() !== 0xff00) {
			reportMismatch("per-pin direction cache after a group write", 0xff00,
				       dio.directionWord());
			allPass = false;
		}

		// A per-pin write must leave the group cache agreeing with it.
		dio.setPinOutput(8, false);
		if (dio.groupOutput(1) !== 0xfe) {
			reportMismatch("group value cache after a per-pin write", 0xfe, dio.groupOutput(1));
			allPass = false;
		}

		// The property path read-modify-writes the whole list through the setter.
		// With the caches coherent that is merely redundant; while they diverged it
		// pushed stale values onto the hardware and killed every driven line.
		dio.setGroupValue(1, 0xff);
		var before = dio.readInputWord();
		dio.out[8] = 0;
		var after = dio.readInputWord();
		printToConsole("    element assignment: input " + hex(before) + " -> " + hex(after));
		if (before !== 0xffff) {
			reportMismatch("group driving 0xFF", 0xffff, before);
			allPass = false;
		}
		// Only pin 8 may move: 0xFEFE, not 0x0000.
		if (after !== 0xfefe) {
			reportMismatch("dio.out[8]=0 changed more than pin 8", 0xfefe, after);
			allPass = false;
		}

		disarm();
		return allPass;
	} catch (e) {
		printToConsole("  Error: " + e);
		return false;
	}
});

printToConsole("Releasing the pins");
try {
	dioReset();
} catch (e) {
	printToConsole("Warning: reset at teardown failed: " + e);
}

scopy.disconnectDevice(devId);

var exitCode = TestFramework.printSummary();
printToConsole("exitCode " + exitCode);
scopy.exit();
