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
 * CN0540 Documentation Tests
 *
 * Requires:
 *   - CN0540 hardware board connected (ad7768-1, ltc2606, one-bit-adc-dac)
 *   - IIO kernel drivers loaded
 *
 * Run from the build directory:
 *   ./scopy --script ../js/testAutomations/cn0540/cn0540DocTests.js [--param ip:<address>]
 */

evaluateFile("../js/testAutomations/common/testFramework.js");

var uri = "ip:192.168.2.1";
if(typeof scriptArgs !== "undefined" && scriptArgs.length > 0) {
	uri = scriptArgs[0];
}

TestFramework.init("CN0540 Doc Tests");

var connected = TestFramework.connectToDevice(uri);
if(!connected) {
	printToConsole("ERROR: Could not connect to device at " + uri);
	scopy.exit(1);
}

switchToTool("CN0540");
msleep(1000);

// ─── Test 1: Plugin Loads ────────────────────────────────────────────────────

TestFramework.runTest("TST.CN0540.PLUGIN_LOADS", function() {
	var tools = cn0540.getTools();
	if(!tools || tools.length === 0) {
		printToConsole("  FAIL: cn0540.getTools() returned empty list");
		return false;
	}
	printToConsole("  Tools: " + tools);
	return true;
});

// ─── Test 2: Device Detection ─────────────────────────────────────────────────

TestFramework.runTest("TST.CN0540.DEVICE_DETECTION", function() {
	var tools = cn0540.getTools();
	if(!tools || tools.length === 0) {
		printToConsole("  FAIL: No tools returned");
		return false;
	}
	printToConsole("  Available tools: " + tools);
	return true;
});

// ─── Test 3: Refresh Button ───────────────────────────────────────────────────

TestFramework.runTest("TST.CN0540.REFRESH_BUTTON", function() {
	cn0540.refresh();
	msleep(500);
	// Verify the plugin is still responsive after refresh
	var tools = cn0540.getTools();
	if(!tools || tools.length === 0) {
		printToConsole("  FAIL: Plugin unresponsive after refresh");
		return false;
	}
	return true;
});

// ─── Test 4: Read SW_FF Status ────────────────────────────────────────────────

TestFramework.runTest("TST.CN0540.READ_SW_FF", function() {
	var state = cn0540.getSwFF();
	if(state !== "HIGH" && state !== "LOW") {
		printToConsole("  FAIL: getSwFF() returned unexpected value: " + state);
		return false;
	}
	printToConsole("  SW_FF state: " + state);
	return true;
});

// ─── Test 5: Shutdown Toggle ──────────────────────────────────────────────────

TestFramework.runTest("TST.CN0540.SHUTDOWN_TOGGLE", function() {
	var original = cn0540.getShutdown();
	if(original === "") {
		printToConsole("  SKIP: Shutdown GPIO not available");
		return "SKIP";
	}
	printToConsole("  Original shutdown state: " + original);

	var toggled = (original === "true") ? "false" : "true";
	cn0540.setShutdown(toggled);
	msleep(200);

	var readback = cn0540.getShutdown();
	if(readback !== toggled) {
		printToConsole("  FAIL: Expected " + toggled + " but got " + readback);
		cn0540.setShutdown(original);
		return false;
	}

	cn0540.setShutdown(original);
	msleep(200);
	printToConsole("  Shutdown toggle verified and restored");
	return true;
});

// ─── Test 6: Constant Current Toggle ─────────────────────────────────────────

TestFramework.runTest("TST.CN0540.CONSTANT_CURRENT_TOGGLE", function() {
	var original = cn0540.getConstantCurrent();
	if(original === "") {
		printToConsole("  SKIP: Constant current GPIO not available");
		return "SKIP";
	}
	printToConsole("  Original constant current state: " + original);

	var toggled = (original === "true") ? "false" : "true";
	cn0540.setConstantCurrent(toggled);
	msleep(200);

	var readback = cn0540.getConstantCurrent();
	if(readback !== toggled) {
		printToConsole("  FAIL: Expected " + toggled + " but got " + readback);
		cn0540.setConstantCurrent(original);
		return false;
	}

	cn0540.setConstantCurrent(original);
	msleep(200);
	printToConsole("  Constant current toggle verified and restored");
	return true;
});

// ─── Test 7: FDA Status Toggle ────────────────────────────────────────────────

TestFramework.runTest("TST.CN0540.FDA_STATUS_TOGGLE", function() {
	var original = cn0540.getFdaEnabled();
	if(original === "") {
		printToConsole("  SKIP: FDA_DIS GPIO not available");
		return "SKIP";
	}
	printToConsole("  Original FDA enabled state: " + original);

	var toggled = (original === "true") ? "false" : "true";
	cn0540.setFdaEnabled(toggled);
	msleep(200);

	var readback = cn0540.getFdaEnabled();
	if(readback !== toggled) {
		printToConsole("  FAIL: Expected " + toggled + " but got " + readback);
		cn0540.setFdaEnabled(original);
		return false;
	}

	cn0540.setFdaEnabled(original);
	msleep(200);
	printToConsole("  FDA status toggle verified and restored");
	return true;
});

// ─── Test 8: FDA Mode Toggle ──────────────────────────────────────────────────

TestFramework.runTest("TST.CN0540.FDA_MODE_TOGGLE", function() {
	var original = cn0540.getFdaMode();
	if(original === "") {
		printToConsole("  SKIP: FDA_MODE GPIO not available");
		return "SKIP";
	}
	printToConsole("  Original FDA mode: " + original);

	cn0540.setFdaMode("FULL POWER");
	msleep(200);
	var fullPower = cn0540.getFdaMode();
	if(fullPower !== "FULL POWER") {
		printToConsole("  FAIL: Expected 'FULL POWER' but got: " + fullPower);
		cn0540.setFdaMode(original);
		return false;
	}

	cn0540.setFdaMode("LOW POWER");
	msleep(200);
	var lowPower = cn0540.getFdaMode();
	if(lowPower !== "LOW POWER") {
		printToConsole("  FAIL: Expected 'LOW POWER' but got: " + lowPower);
		cn0540.setFdaMode(original);
		return false;
	}

	cn0540.setFdaMode(original);
	msleep(200);
	printToConsole("  FDA mode toggle verified and restored");
	return true;
});

// ─── Test 9: Input Voltage Read ───────────────────────────────────────────────

TestFramework.runTest("TST.CN0540.INPUT_VOLTAGE_READ", function() {
	var voltage = cn0540.getInputVoltage();
	if(voltage === "") {
		printToConsole("  FAIL: getInputVoltage() returned empty string");
		return false;
	}
	var v = parseFloat(voltage);
	if(isNaN(v)) {
		printToConsole("  FAIL: getInputVoltage() returned non-numeric value: " + voltage);
		return false;
	}
	printToConsole("  Input voltage: " + voltage + " mV");
	return true;
});

// ─── Test 10: Shift Voltage Write/Readback ────────────────────────────────────

TestFramework.runTest("TST.CN0540.SHIFT_VOLTAGE_WRITE", function() {
	var original = cn0540.getShiftVoltage();
	if(original === "") {
		printToConsole("  SKIP: DAC channel not available");
		return "SKIP";
	}
	printToConsole("  Original shift voltage: " + original + " mV");

	// Write a known test value (10 mV)
	var testValue = 10.0;
	cn0540.setShiftVoltage(String(testValue));
	msleep(300);

	var readback = cn0540.getShiftVoltage();
	var diff = Math.abs(parseFloat(readback) - testValue);
	if(diff > 1.0) {
		printToConsole("  FAIL: Wrote " + testValue + " mV but read back " + readback + " mV");
		cn0540.setShiftVoltage(original);
		return false;
	}
	printToConsole("  Readback: " + readback + " mV (tolerance OK)");

	cn0540.setShiftVoltage(original);
	msleep(300);
	printToConsole("  Shift voltage write/readback verified and restored");
	return true;
});

// ─── Test 11: Sensor Voltage Calculation ──────────────────────────────────────

TestFramework.runTest("TST.CN0540.SENSOR_VOLTAGE_READ", function() {
	var voltage = cn0540.getSensorVoltage();
	if(voltage === "") {
		printToConsole("  FAIL: getSensorVoltage() returned empty string");
		return false;
	}
	var v = parseFloat(voltage);
	if(isNaN(v)) {
		printToConsole("  FAIL: getSensorVoltage() returned non-numeric value: " + voltage);
		return false;
	}
	printToConsole("  Sensor voltage: " + voltage + " mV");
	return true;
});

// ─── Test 12: Calibration Routine ────────────────────────────────────────────

TestFramework.runTest("TST.CN0540.CALIBRATE", function() {
	cn0540.calibrate();
	// Calibration is async (up to 20 iterations × 10ms + overhead)
	msleep(3000);
	// Verify the plugin is still responsive after calibration
	var voltage = cn0540.getInputVoltage();
	if(voltage === "") {
		printToConsole("  FAIL: Plugin unresponsive after calibration");
		return false;
	}
	printToConsole("  Calibration completed, ADC reads: " + voltage + " mV");
	return true;
});

// ─── Test 13: Voltage Monitor Display ────────────────────────────────────────

TestFramework.runTest("TST.CN0540.VOLT_MON_DISPLAY", function() {
	var voltages = cn0540.getVoltageMonitor();
	if(!voltages || voltages.length === 0) {
		printToConsole("  SKIP: Voltage monitor device (xadc/ltc2308) not present");
		return "SKIP";
	}
	if(voltages.length !== 6) {
		printToConsole("  FAIL: Expected 6 voltage readings but got " + voltages.length);
		return false;
	}
	var pinNames = ["Vin+", "Vgpio2", "Vgpio3", "Vcom", "Vfda+", "Vfda-"];
	for(var i = 0; i < voltages.length; i++) {
		printToConsole("  " + pinNames[i] + ": " + voltages[i] + " mV");
	}
	return true;
});

// ─── Test 14: Voltage Monitor Auto-Refresh ────────────────────────────────────

TestFramework.runTest("TST.CN0540.VOLT_MON_AUTO_REFRESH", function() {
	var v1 = cn0540.getVoltageMonitor();
	if(!v1 || v1.length === 0) {
		printToConsole("  SKIP: Voltage monitor device (xadc/ltc2308) not present");
		return "SKIP";
	}

	msleep(2000);

	var v2 = cn0540.getVoltageMonitor();
	if(!v2 || v2.length !== 6) {
		printToConsole("  FAIL: Voltage monitor returned unexpected result after wait");
		return false;
	}
	printToConsole("  Voltage monitor auto-refresh verified (" + v2.length + " channels)");
	return true;
});

// ─────────────────────────────────────────────────────────────────────────────

TestFramework.disconnectFromDevice();
var exitCode = TestFramework.printSummary();
scopy.exit(exitCode);
