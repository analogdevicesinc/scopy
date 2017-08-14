#!/usr/bin/scopy -s

/* How to run this script:
 * scopy --script termal_calib.js
 */

 /* SCRIPT Global Parameters
  * NB_READINGS              - The number of readings the script should run. In a reading, one or more calibrations are ran (possibly at different temperatures, etc.).
  * NB_CALIBS_IN_A_READING   - The number of calibrations to be executed in a single reading.
  * SECONDS_BETWEEN_READINGS - The time (seconds) the script should wait between two consecutive readings.
  * SECONDS_BETWEEN_CALIBS   - The time (seconds) the script should wait between two consecutive calibrations withing a reading.
  * SHOW_TIMESTAMP           - Whether the timestamp of each calibration should be logged.
  * SHOW_START_END_TIME      - Whether the script should log the start/end date and time.
  * OUTPUT_FILENAME          - The path and name of the file where the script should log all results.
  */
  var NB_READINGS = 2
  var NB_CALIBS_IN_A_READING = 3
  var SECONDS_BETWEEN_READINGS = 0
  var SECONDS_BETWEEN_CALIBS = 0
  var SHOW_TIMESTAMP = true
  var SHOW_START_END_TIME = true
  var OUTPUT_FILENAME = "m2k_calibration_dava_vs_temp0.csv"
 
function run_one_calib() {
	var result = ''
	var ret;
	
	ret = calib.calibrateAll()
	if (ret == true) {
		var adc_offs = calib.adc_offsets
		var adc_gains = calib.adc_gains
		var dac_offs = calib.dac_offsets
		var dac_vlsb = calib.dac_gains
		
		result += adc_offs + ","
		result += adc_gains + ","
		result += dac_offs + ","
		result += dac_vlsb + ","
		result += calib.devTemp("ad9963") + ","
		result += calib.devTemp("xadc")
	} else {
		result += "calibration failed"
	}
	
	return result
}

function run_entire_calibration() {
	var calibData = ''
	var reading_iteration = 0;
	var calib_iteration = 0
	
	for (r = 0; r < NB_READINGS; r++) {
		for (c = 0; c < NB_CALIBS_IN_A_READING; c++) {
			printToConsole("Reading: " + reading_iteration + " Calibration: " + calib_iteration + " Calibrating..")
			date = new Date()
			var calib_data = run_one_calib()
			
			var time_stamp = date.toTimeString()
			
			calibData += reading_iteration + "," + ++calib_iteration + "," + calib_data + (SHOW_TIMESTAMP ? "," + time_stamp : '') + '\n'
			
			if (c + 1 < NB_CALIBS_IN_A_READING) {
				printToConsole("Waiting " + SECONDS_BETWEEN_CALIBS + "s until next calibration")
				msleep(SECONDS_BETWEEN_CALIBS * 1000)
			}
		}
		reading_iteration++;
		calib_iteration = 0;
		
		if (r + 1 < NB_READINGS) {
			printToConsole("Waiting " + SECONDS_BETWEEN_READINGS + "s until next reading")
			msleep(SECONDS_BETWEEN_READINGS * 1000)
		}
	}
	
	return calibData
}

function build_header() {
	var header = ''
	
	header += "Reading, Calibration, ADC Ch 1 Offset(RAW), ADC Ch 2 Offset(RAW), " + 
		"ADC Ch 1 Gain, ADC Ch 2 Gain, DAC Ch1 Offset(RAW), DAC Ch2 Offset(RAW)," +
		"DAC Ch 1VLsb (Volts), DAC Ch 2VLsb (Volts), Ad9963 Temp(Celsius), " + 
		"XAdc Temp(Celsius)" +  (SHOW_TIMESTAMP ? "," + "Time Stamp" : '') + '\n'
		
	return header
}

function connectToUSB(host) {
	printToConsole("Connecting to " + host + "...")

	var success = launcher.connect(host)

	if (success)
		printToConsole("Connected!")
	else
		printToConsole("Failed to connect to: " + host + "!")

	return success;
}

function connect() {
	var usb_devs = launcher.usb_uri_list() 
	
	var usb_uri = usb_devs[0]
	if (usb_uri) {	
		var connected = connectToUSB(usb_uri)
		if (!connected)
			return false;
	} else {
		printToConsole("No usb devices available")
		return false;
	}
	
	return true;
}
 
function main() {
	var script_output=''
	
	if (SHOW_START_END_TIME) {
		var date = new Date()
		script_output += "Script started on: " + date.toLocaleString() + '\n'
	}
	
	if (!connect())
		return Error()

	script_output += build_header()
	
	calib.setHardwareInCalibMode()
	
	script_output += run_entire_calibration();
	
	calib.restoreHardwareFromCalibMode()

	launcher.disconnect()
	
	if (SHOW_START_END_TIME) {
		date = new Date()
		script_output += "Script ended on: " + date.toLocaleString() + '\n'
	}
	
	fileIO.writeToFile(script_output, OUTPUT_FILENAME)
}

main()
