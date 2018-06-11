/* Generate using Power Supply and get data using Voltmeter */

/* Set Power Supply */
function set_power_supply(){
	
	/* Set Power Supplies as Independent */
	power.sync = false
	
	/* Set Positive Power Supply output value 5V */
	power.dac1_value = 5
	
	/* Set Negative Power Supply output value -3V */
	power.dac2_value = -3
	
	/* Enable Positive Power Supply */
	power.dac1_enabled = true
	
	/* Enable Negative Power Supply */
	power.dac2_enabled = true
		
}

/* Set Voltmeter */
function set_dmm(){
	
	/* Set Channel 1 to DC mode */
	dmm.mode_ac_low_ch1 = false
	dmm.mode_ac_high_ch1 = false
	
	/* Set Channel 2 to DC mode */
	dmm.mode_ac_low_ch2 = false
	dmm.mode_ac_high_ch2 = false
	
	/* Enable Channel 1 History plot. */
	dmm.histogram_ch1 = true
	
	/* Set Channel 1 History plot size to 10s */
	dmm.history_ch1_size_idx = 1
	

	/* Enable Channel 2 History plot. */
	dmm.histogram_ch2 = true
	
	/* Set Channel 2 History plot size to 10s */
	dmm.history_ch2_size_idx = 2
	
	/* Enable Peak Hold */
	dmm.peak_hold_en = true
	
	/* Run Voltmeter */
	dmm.running = true
}

/* main function */
function main(){
	
	set_power_supply()
	
	msleep(1000)
	
	set_dmm()
	
	msleep(1000)
	
	/* Read Channel 1 from Voltmeter */
	var ch1 = dmm.value_ch1
	
	/* Read Channel 2 from Voltmeter */
	var ch2 = dmm.value_ch2
	
	/* Print Channel 1 value to console */
	printToConsole(ch1)
	
	/* Print Channel 2 value to console */
	printToConsole(ch2)
	
}

main()