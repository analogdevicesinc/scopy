#!/usr/bin/scopy -s

/* Before running this example it is necessary to run: iio-emu adalm2000 */
var uri = "127.0.0.1"

function main() {

	var devId = scopy.addDevice(uri)
	var connected = scopy.connectDevice(devId)
	if (!connected)
		return Error()

	/* Run signal generator with values saved in .ini file*/
	siggen.running=true
	/* Run oscilloscope */
	osc.running=true
	/* Focus oscilloscope */
	scopy.switchTool("Oscilloscope")
}

/* To keep the application session after running a certain script */
/* use the command line options: -r or --keep-running. */
main()
