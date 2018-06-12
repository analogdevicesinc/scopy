/***************************************************************************//**
 *   @file   sar_adc.js
 *   @brief  SAR ADC script 
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

/* Comparator Input Voltage */
var input_voltage = 2

/* Positive Power Supply value */
var pos_supply = 5

/* Negative Power Supply value */
var neg_supply = -5

/* DAC Number of Bits */
var nr_bits = 4	

/* Maximum Decimal value for n bits */
var max_value

/* Comparator output value */
var comp_out

/* Signal Generator Setup Function */
function set_signal_generator() {
	
	/* Set Channel 1 to Constant Voltage Mode */
	siggen.mode[0] = 0
	
	/* Set the output voltage */
	siggen.constant_volts[0] = input_voltage
	
	/* Run Signal Generator */
	siggen.running = true
	
	/* Delay */
	msleep(300)
	
}

/* Power Supply Setup Function */
function set_power_supply() {
	
	/* Set Power Supplies as Independent */
	power.sync = false
	
	/* Set Positive Power Supply voltage */
	power.dac1_value = pos_supply
	
	/* Set Negative Power Supply voltage */
	power.dac2_value = neg_supply
	
	/* Enable Positive Power Supply */
	power.dac1_enabled = true
	
	/* Enable Negative Power Supply */
	power.dac2_enabled = true
	
	/* Delay */
	msleep(300)
	
}

/* Oscilloscope Setup Function */
function set_osc() {
	
	/* Run Oscilloscope */
	osc.running = true
	
	/* Delay */
	msleep(300)
	
}

/* Logic Analyzer Setup Function */
function set_la() {
	
	/* Run Logic Analyzer */
	logic.running = true
	
	/* Delay */	
	msleep(300)
	
}

/* DigitalIO Setup Function */
function set_dio(nb){
	
	/* Set first nb digital channels as output low*/
	for (i = 0; i < nb; i++){
		dio.dir[i]=true
		dio.out[i]=false
	}
	
	/* Set the rest of digital channels as input*/
	for (i = nb; i < 16; i++){
		dio.dir[i]=false
		dio.out[i]=false
	}		
	
	/* Run Digital IO*/
	dio.running = true
	
	/* Delay */	
	msleep(300)
	
}

/* Calculate maximum number of output codes for n bits */
function calcultate_max_value(nb){
	
	var max_value = 1
	
	for(i = 0; i < nb; i++)
		max_value *= 2
	
	return max_value-1
}

/* Set digital output based on the input value */
function set_dio_value(value){

	for(i = 0; i < nr_bits; i++){
		if((( value >> i ) % 2) == 1)
			dio.out[i]=true
		else
			dio.out[i]=false
	}

}

/* Perform Succesive Approximation using the maximum number of output codes */
function successive_approximation(max_val) {
	
	var comp_out, digital_output
	var min = 0
	var max = max_val
	
    while(min <= max){
		
		/* Get digital output value */
        digital_output = Math.ceil((min+max)/2)
		
		/* Set digital output value on DigitalIO  */
		set_dio_value(digital_output)
		
		/* Delay */
		msleep(2000)
		
		/* Get comparator output values */
		comp_out = osc.channels[0].mean
		
		if ( comp_out > 3 )
			min = digital_output + 1
		else if( comp_out < 1 )
			max = digital_output - 1
		else
			break
     }
}

/* Main function */
function main (){
	
	set_signal_generator()
	
	msleep(100)
	
	set_power_supply()
	
	msleep(100)
	
	set_osc()
	
	set_dio(nr_bits)
	
	set_la()
	
	max_value = calcultate_max_value(nr_bits)
	
	successive_approximation(max_value)
	
}

main()