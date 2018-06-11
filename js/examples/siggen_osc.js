/* Generate and plot using Signal Generator and Oscilloscope */

/*Setup Signal Generator*/
function set_signal_generator(){

	/* Enable Signal Generator Channel 1  */
	siggen.enabled[0] = true
	
	/* Set Channel 1 operation mode to Waveform */
	siggen.mode[0] = 1
	
	/* Set Channel 1 Waveform type to Sine */
	siggen.waveform_type[0] = 0
	
	/* Set Channel 1 frequency to 1kHz */
	siggen.waveform_frequency[0] = 1000
	
	/* Set Channel 1 amplitude to 3V */
	siggen.waveform_amplitude[0] = 3
	
	/* Set Channel 1 offset to 1V */
	siggen.waveform_offset[0] = 1
	
	/* Set Channel 1 phase to 0 degrees */
	siggen.waveform_phase[0] = 0
	
	/* Run Signal Generator */
	siggen.running = true
}

/*Setup Oscilloscope*/
function set_oscilloscope(){
	
	/* Enable Oscilloscope Channel 1  */
	osc.channels[0].enabled = true
	
	/* Disable Oscilloscope Channel 2  */
	osc.channels[0].enabled = true
	
	/* Set Volts/Div to 1V/div */
	osc.channels[0].volts_per_div = 1
	
	/* Set Time Base to 1 ms */
	osc.time_base = 0.001
	
	/* Set Time Position to 0s */
	osc.time_position = 0
	
	/* Run Oscilloscope */
	osc.running = true
}

/* Setup main function */
function main(){
	
	set_signal_generator()
	
	msleep(1000)
	
	set_oscilloscope()
	
	msleep(1000)
	
	/* Read Channel 1 Peak-to-Peak Value */
	var pp = osc.channels[0].peak_to_peak
	
	/* Print value to Console */
	printToConsole(pp)
}

main()
