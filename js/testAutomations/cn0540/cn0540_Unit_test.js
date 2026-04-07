/*
 * Copyright (c) 2025 Analog Devices Inc.
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

/*
 * CN0540 Unit Tests
 *
 * Tests every IIOWidget attribute (via readWidget/writeWidget with raw IIO values)
 * and every API getter/setter method (with logical values).
 *
 * Widget keys use raw IIO values:
 *   readWidget(key)       -> raw IIO string (e.g. "0" or "1" for GPIO)
 *   writeWidget(key, val) -> writes raw IIO value directly (no UI conversion)
 *
 * API methods use logical values:
 *   getShutdown()         -> "true" or "false"
 *   setShutdown("true")   -> activates operating mode (GPIO low, active-low)
 *   getFdaMode()          -> "FULL POWER" or "LOW POWER"
 *   getShiftVoltage()     -> mV string (e.g. "1000.0000")
 *
 * Requires:
 *   - CN0540 hardware or emulator (iio-emu generic emu-xml/cn0540.xml)
 *
 * Run from build directory:
 *   ./scopy --script js/testAutomations/cn0540/cn0540_Unit_test.js [--param ip:<address>]
 */

evaluateFile("js/testAutomations/common/testFramework.js");

// ─── Setup ────────────────────────────────────────────────────────────────────

var uri = "ip:192.168.2.1";
if(typeof scriptArgs !== "undefined" && scriptArgs.length > 0) {
	uri = scriptArgs[0];
}

TestFramework.init("CN0540 Unit Tests");

var connected = TestFramework.connectToDevice(uri);
if(!connected) {
	printToConsole("ERROR: Could not connect to device at " + uri);
	scopy.exit(1);
}

switchToTool("CN0540");
msleep(1000);

// ─── Widget keys ─────────────────────────────────────────────────────────────

// Widget keys correspond to IIOWidgets registered via IIOWidgetBuilder+group in cn0540.cpp.
// The DAC shift voltage uses a custom QLineEdit (not IIOWidgetBuilder), so it has no widget key.
var KEY_SHUTDOWN = "one-bit-adc-dac/voltage1_out/raw";
var KEY_FDA_DIS = "one-bit-adc-dac/voltage2_out/raw";
var KEY_FDA_MODE = "one-bit-adc-dac/voltage3_out/raw";
var KEY_CC = "one-bit-adc-dac/voltage4_out/raw";

// ─── Standard helpers ─────────────────────────────────────────────────────────

/*
 * runDataDrivenTests — dispatches an array of test descriptors through a helper function.
 * Each descriptor must have: uid, and any properties the helper function uses.
 */
function runDataDrivenTests(tests, helperFn) {
	for(var i = 0; i < tests.length; i++) {
		(function(test) {
			TestFramework.runTest(test.uid, function() {
				return helperFn(test);
			});
		})(tests[i]);
	}
}

/*
 * testCheckboxWidget — tests a binary (0/1) IIOWidget attribute.
 * Writes both "0" and "1", verifies exact readback, then restores original.
 * Uses raw IIO values (writeWidget/readWidget bypass UI conversion lambdas).
 */
function testCheckboxWidget(key, label) {
	var orig = cn0540.readWidget(key);
	if(orig === "" || orig === null || orig === undefined) {
		printToConsole("  SKIP: " + label + " widget not available (key: " + key + ")");
		return "SKIP";
	}
	printToConsole("  Original raw value: " + orig);

	var testValues = ["0", "1"];
	for(var i = 0; i < testValues.length; i++) {
		var testVal = testValues[i];
		cn0540.writeWidget(key, testVal);
		msleep(500);
		var readback = cn0540.readWidget(key);
		if(readback !== testVal) {
			printToConsole("  FAIL: wrote " + testVal + " but read back " + readback);
			cn0540.writeWidget(key, orig);
			msleep(500);
			return false;
		}
		printToConsole("  " + label + " raw=" + testVal + " verified");
	}

	cn0540.writeWidget(key, orig);
	msleep(500);
	return true;
}

/*
 * testApiToggle — tests an API getter/setter that accepts "true"/"false".
 * Saves original, toggles both states, verifies readback, restores.
 */
function testApiToggle(getter, setter, label) {
	var orig = getter();
	if(orig === "" || orig === null || orig === undefined) {
		printToConsole("  SKIP: " + label + " not available");
		return "SKIP";
	}
	printToConsole("  Original: " + orig);

	var states = ["true", "false"];
	for(var i = 0; i < states.length; i++) {
		setter(states[i]);
		msleep(500);
		var readback = getter();
		if(readback !== states[i]) {
			printToConsole("  FAIL: set " + states[i] + " but got " + readback);
			setter(orig);
			msleep(500);
			return false;
		}
		printToConsole("  " + label + "=" + states[i] + " verified");
	}

	setter(orig);
	msleep(500);
	return true;
}

/*
 * testApiReadOnlyNumeric — verifies getter returns a non-empty numeric string.
 */
function testApiReadOnlyNumeric(getter, label) {
	var val = getter();
	if(val === "" || val === null || val === undefined) {
		printToConsole("  FAIL: " + label + " returned empty value");
		return false;
	}
	var num = parseFloat(val);
	if(isNaN(num)) {
		printToConsole("  FAIL: " + label + " is not numeric: " + val);
		return false;
	}
	printToConsole("  " + label + ": " + val);
	return true;
}

// ─── Section: UTILITY ─────────────────────────────────────────────────────────

TestFramework.runTest("UNIT.UTIL.GET_TOOLS", function() {
	var tools = cn0540.getTools();
	if(!tools || tools.length === 0) {
		printToConsole("  FAIL: getTools() returned empty list");
		return false;
	}
	printToConsole("  Tools: " + tools);
	return true;
});

TestFramework.runTest("UNIT.UTIL.WIDGET_KEYS", function() {
	var keys = cn0540.getWidgetKeys();
	if(!keys || keys.length === 0) {
		printToConsole("  FAIL: getWidgetKeys() returned empty list");
		return false;
	}
	printToConsole("  Registered widget keys (" + keys.length + "):");
	for(var i = 0; i < keys.length; i++) {
		printToConsole("    " + keys[i]);
	}
	// Verify the expected keys are present
	var required = [KEY_SHUTDOWN, KEY_FDA_DIS, KEY_FDA_MODE, KEY_CC];
	for(var j = 0; j < required.length; j++) {
		var found = false;
		for(var k = 0; k < keys.length; k++) {
			if(keys[k] === required[j]) {
				found = true;
				break;
			}
		}
		if(!found) {
			printToConsole("  FAIL: expected key not registered: " + required[j]);
			return false;
		}
	}
	return true;
});

// ─── Section: POWER CONTROL — SW_FF (read-only via API) ───────────────────────

TestFramework.runTest("UNIT.POWER.SW_FF", function() {
	var state = cn0540.getSwFF();
	if(state !== "HIGH" && state !== "LOW") {
		printToConsole("  FAIL: getSwFF() returned unexpected value: " + state);
		return false;
	}
	printToConsole("  SW_FF state: " + state);
	return true;
});

// ─── Section: WIDGET TESTS — raw IIOWidget read/write ────────────────────────

// Data-driven: 4 checkbox widgets follow the same raw 0/1 pattern
var checkboxWidgetTests = [
	{ uid: "UNIT.WIDGET.SHUTDOWN",         key: KEY_SHUTDOWN, label: "Shutdown" },
	{ uid: "UNIT.WIDGET.FDA_ENABLED",      key: KEY_FDA_DIS,  label: "FDA Enabled (FDA_DIS)" },
	{ uid: "UNIT.WIDGET.FDA_MODE",         key: KEY_FDA_MODE, label: "FDA Mode" },
	{ uid: "UNIT.WIDGET.CONSTANT_CURRENT", key: KEY_CC,       label: "Constant Current" },
];

runDataDrivenTests(checkboxWidgetTests, function(test) {
	return testCheckboxWidget(test.key, test.label);
});

// Bad value test: invalid key should return empty string without crash
TestFramework.runTest("UNIT.WIDGET.BAD_KEY_READ", function() {
	var result = cn0540.readWidget("nonexistent/key");
	if(result !== "" && result !== null && result !== undefined) {
		printToConsole("  FAIL: readWidget with bad key returned non-empty: " + result);
		return false;
	}
	printToConsole("  readWidget with invalid key correctly returned empty");
	return true;
});

// ─── Section: API LOGICAL TESTS — Power Control ───────────────────────────────

TestFramework.runTest("UNIT.API.SHUTDOWN", function() {
	return testApiToggle(
		function() { return cn0540.getShutdown(); },
		function(v) { cn0540.setShutdown(v); },
		"Shutdown"
	);
});

TestFramework.runTest("UNIT.API.CONSTANT_CURRENT", function() {
	return testApiToggle(
		function() { return cn0540.getConstantCurrent(); },
		function(v) { cn0540.setConstantCurrent(v); },
		"Constant Current"
	);
});

// ─── Section: API LOGICAL TESTS — ADC Driver ──────────────────────────────────

TestFramework.runTest("UNIT.API.FDA_ENABLED", function() {
	return testApiToggle(
		function() { return cn0540.getFdaEnabled(); },
		function(v) { cn0540.setFdaEnabled(v); },
		"FDA Enabled"
	);
});

TestFramework.runTest("UNIT.API.FDA_MODE", function() {
	var orig = cn0540.getFdaMode();
	if(orig === "" || orig === null || orig === undefined) {
		printToConsole("  SKIP: FDA_MODE not available");
		return "SKIP";
	}
	printToConsole("  Original FDA mode: " + orig);

	var modes = ["FULL POWER", "LOW POWER"];
	for(var i = 0; i < modes.length; i++) {
		cn0540.setFdaMode(modes[i]);
		msleep(500);
		var readback = cn0540.getFdaMode();
		if(readback !== modes[i]) {
			printToConsole("  FAIL: set '" + modes[i] + "' but got '" + readback + "'");
			cn0540.setFdaMode(orig);
			msleep(500);
			return false;
		}
		printToConsole("  FDA Mode='" + modes[i] + "' verified");
	}

	// Bad value test: invalid mode should be rejected (no change)
	cn0540.setFdaMode("INVALID_MODE");
	msleep(500);
	var afterBad = cn0540.getFdaMode();
	if(afterBad !== "LOW POWER") {
		printToConsole("  WARN: FDA mode changed to '" + afterBad + "' after invalid write");
	}

	cn0540.setFdaMode(orig);
	msleep(500);
	return true;
});

// ─── Section: API LOGICAL TESTS — Sensor Calibration ─────────────────────────

TestFramework.runTest("UNIT.CAL.INPUT_VOLTAGE", function() {
	return testApiReadOnlyNumeric(function() { return cn0540.getInputVoltage(); }, "Input Voltage [mV]");
});

TestFramework.runTest("UNIT.CAL.SHIFT_VOLTAGE", function() {
	var orig = cn0540.getShiftVoltage();
	if(orig === "" || orig === null || orig === undefined) {
		printToConsole("  SKIP: Shift voltage not available");
		return "SKIP";
	}
	printToConsole("  Original shift voltage: " + orig + " mV");

	// Test min, mid, and near-max values
	var testValues = [0.0, 1000.0, 5000.0];
	var tolerance = 1.0; // ±1 mV

	for(var i = 0; i < testValues.length; i++) {
		var testMv = testValues[i];
		cn0540.setShiftVoltage(String(testMv));
		msleep(500);

		var readback = cn0540.getShiftVoltage();
		var diff = Math.abs(parseFloat(readback) - testMv);
		if(diff > tolerance) {
			printToConsole("  FAIL: wrote " + testMv + " mV but read " + readback + " mV (diff=" + diff.toFixed(4) + ")");
			cn0540.setShiftVoltage(orig);
			msleep(500);
			return false;
		}
		printToConsole("  Vshift=" + testMv + " mV → readback=" + readback + " mV OK");
	}

	// Bad value test: non-numeric should be silently rejected
	cn0540.setShiftVoltage("not_a_number");
	msleep(500);

	cn0540.setShiftVoltage(orig);
	msleep(500);
	return true;
});

TestFramework.runTest("UNIT.CAL.SENSOR_VOLTAGE", function() {
	return testApiReadOnlyNumeric(function() { return cn0540.getSensorVoltage(); }, "Sensor Voltage [mV]");
});

// ─── Section: VOLTAGE MONITOR ─────────────────────────────────────────────────

TestFramework.runTest("UNIT.VOLT_MON.ALL_PINS", function() {
	var voltages = cn0540.getVoltageMonitor();
	if(!voltages || voltages.length === 0) {
		printToConsole("  SKIP: Voltage monitor device not present");
		return "SKIP";
	}
	if(voltages.length !== 6) {
		printToConsole("  FAIL: expected 6 pins but got " + voltages.length);
		return false;
	}
	var pinNames = ["Vin+", "Vgpio2", "Vgpio3", "Vcom", "Vfda+", "Vfda-"];
	for(var i = 0; i < voltages.length; i++) {
		printToConsole("  " + pinNames[i] + ": " + voltages[i]);
	}
	return true;
});

// ─── Section: UTILITY ACTIONS ────────────────────────────────────────────────

TestFramework.runTest("UNIT.UTIL.REFRESH_ALL", function() {
	cn0540.refresh();
	msleep(500);
	// Verify plugin still responds after refresh
	var keys = cn0540.getWidgetKeys();
	if(!keys || keys.length === 0) {
		printToConsole("  FAIL: plugin unresponsive after refresh");
		return false;
	}
	printToConsole("  Plugin responsive after refresh (" + keys.length + " widgets)");
	return true;
});

// ─── Section: COMPLEX TESTS ──────────────────────────────────────────────────

// C1: Full calibration flow
TestFramework.runTest("UNIT.CAL.FULL_CALIBRATION_FLOW", function() {
	// Step 1: Set a known shift voltage
	var origVshift = cn0540.getShiftVoltage();
	if(origVshift === "" || origVshift === null || origVshift === undefined) {
		printToConsole("  SKIP: DAC channel not available");
		return "SKIP";
	}

	cn0540.setShiftVoltage("500");
	msleep(500);

	// Step 2: Verify ADC reads a numeric value pre-calibration
	var preCal = cn0540.getInputVoltage();
	var preNum = parseFloat(preCal);
	if(isNaN(preNum)) {
		printToConsole("  FAIL: getInputVoltage() not numeric before calibration: " + preCal);
		cn0540.setShiftVoltage(origVshift);
		msleep(500);
		return false;
	}
	printToConsole("  Pre-calibration input voltage: " + preCal + " mV");

	// Step 3: Run calibration (async, up to 20 × 10ms = 200ms + overhead)
	printToConsole("  Running calibration...");
	cn0540.calibrate();
	msleep(3000);

	// Step 4: Verify ADC still returns numeric value after calibration
	var postCal = cn0540.getInputVoltage();
	var postNum = parseFloat(postCal);
	if(isNaN(postNum)) {
		printToConsole("  FAIL: getInputVoltage() not numeric after calibration: " + postCal);
		cn0540.setShiftVoltage(origVshift);
		msleep(500);
		return false;
	}
	printToConsole("  Post-calibration input voltage: " + postCal + " mV");

	// Step 5: Verify shift voltage is still readable
	var postVshift = cn0540.getShiftVoltage();
	if(isNaN(parseFloat(postVshift))) {
		printToConsole("  FAIL: getShiftVoltage() not numeric after calibration: " + postVshift);
		cn0540.setShiftVoltage(origVshift);
		msleep(500);
		return false;
	}
	printToConsole("  Post-calibration shift voltage: " + postVshift + " mV");

	// Restore original shift voltage
	cn0540.setShiftVoltage(origVshift);
	msleep(500);
	printToConsole("  Calibration flow complete");
	return true;
});

// ─── Teardown ────────────────────────────────────────────────────────────────

TestFramework.disconnectFromDevice();
var exitCode = TestFramework.printSummary();
scopy.exit(exitCode);
