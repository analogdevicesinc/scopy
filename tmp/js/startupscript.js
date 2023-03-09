#!/usr/bin/scopy -s

/* This is the script that starts scopy up and connects to the emulator */
var host = "127.0.0.1"


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

	var connected = connect(host)
	if (!connected)
		return Error()

	/* Run signal generator with values saved in .ini file*/
	siggen.running=true
	/* Run oscilloscope */
	osc.running=true
	/* Focus oscilloscope */
	launcher.focused_instrument=0
	/* Resume control to the application*/
	returnToApplication();
}

main()
