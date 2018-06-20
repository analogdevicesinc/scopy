/***************************************************************************//**
 *   @file   power_dmm.js
 *   @brief  Generate using Power Supply and get data using Voltmeter
 *   @author Antoniu Miclaus (antoniu.miclaus@analog.com)
********************************************************************************
 * Copyright 2018(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/* Perform tool reset */
launcher.reset()

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