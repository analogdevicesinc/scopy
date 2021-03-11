#!/usr/bin/scopy -s

/* How to run this script:
 * scopy --script read_dmm.js
 */

function run_dmm_constant_test(idx) {
	siggen.constant_volts = [-idx, idx]

	/* Wait for the DMM to settle */
	sleep(1)

	/* Read DMM values */
	var val1 = dmm.value_ch1
	var val2 = dmm.value_ch2

	/* Compute error value in mV */
	var err1 = Math.round((val1 + idx) * 1000)
	var err2 = Math.round((val2 - idx) * 1000)

	print("Value channel 1: " + val1 + ", error = " + err1 + " mV")
	print("Value channel 2: " + val2 + ", error = " + err2 + " mV")
}

function run_dmm_ac_test(freq) {
	/* Generate 1 kHz, 2.828V +1.5 VDC sine waves */

	/* XXX: why is this sleep necesary?
	 * If absent, Scopy errors when creating the buffer... */
	msleep(500)

	siggen.waveform_type = [0, 0]
	siggen.waveform_frequency = [freq, freq]
	siggen.waveform_offset = [1.5, 1.5]
	siggen.waveform_amplitude = [2.828, 2.828]
	siggen.waveform_phase = [0, 0]

	/* Switch to waveform generation */
	siggen.mode = [1, 1]

	/* Wait for the DMM to settle */
	sleep(2)

	/* Read DMM values */
	var val1 = dmm.value_ch1
	var val2 = dmm.value_ch2

	/* Compute error value in mV */
	var err1 = Math.round((val1 - 1) * 1000)
	var err2 = Math.round((val2 - 1) * 1000)

	print("DMM AC channel 1: " + val1 + ", error = " + err1 + " mV")
	print("DMM AC channel 2: " + val2 + ", error = " + err2 + " mV")
}

function run_tests() {
	/* Use DC mode */
	dmm.mode_ac_ch1 = false
	dmm.mode_ac_ch2 = false

	/* We want to generate constant values */
	siggen.constant_volts = [0, 0]
	siggen.mode = [0, 0]

	siggen.running = true
	dmm.running = true

	for (var i = 0; i < 5; i++) {
		run_dmm_constant_test(i)
	}

	/* Switch to AC mode */
	dmm.mode_ac_ch1 = true
	dmm.mode_ac_ch2 = true

	run_dmm_ac_test(200e3)

	dmm.running = false
	siggen.running = false
}

function connect(host) {
	print("Connecting to " + host + "...")

	var success = launcher.connect("ip:" + host)

	if (success)
		print("Connected!")
	else
		print("Failed!")

	return success;
}

function main() {
	/* hardcoded for now */
	var host = "192.168.2.1"

	var connected = connect(host)
	if (!connected)
		return Error()

	run_tests()

	launcher.disconnect()
}

main()
