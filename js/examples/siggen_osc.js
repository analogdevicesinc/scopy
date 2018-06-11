/***************************************************************************//**
 *   @file   siggen_osc.js
 *   @brief  Generate and plot using Signal Generator and Oscilloscope 
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
