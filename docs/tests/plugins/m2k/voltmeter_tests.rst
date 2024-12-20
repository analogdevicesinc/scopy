M2K Voltmeter - Test Suite
===============================================================================

Initial Setup
-------------------------------------------------------------------------------
In order to proceed through the test case, first of all delete the Scopy *.ini file (saves previous settings made in Scopy tool) from the following path on Windows: C:\Users\your_username\AppData\Roaming\ADI .

Test Case
-------------------------------------------------------------------------------

Setup:
        - M2K.*

Test 1: Channel 1 Operation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.M2K.VOLTMETER.CHANNEL_1_OPERATION

Description:
        - This test case verifies the functionality of the M2K voltmeter channel 1 operation.

OS:
        - any

Steps:
        * Step 1: Checking the DC Mode of channel 1
        * Step 1.1: Set channel 1 in DC Mode
                * Expected Result: The interface should look like in the “Step Resources” picture (left side), the numerical value should indicate that it’s in VDC mode.
        * Step 1.2: Connect scope ch1+ to positive supply and ch1- to gnd
        * Step 1.3: Set the positive power supply voltage level to 3.3V
                * Expected Result: The voltage displayed in the voltmeter should be around 3.2V to 3.4V and the history graph should follow in 1s, 10s or 60s setting
        * Step 1.4: Connect scope ch1+ to negative supply and ch1- to gnd
        * Step 1.5: Set the negative power supply voltage level to -3.3V
                * Expected Result: The voltage displayed in voltmeter should be around -3.2V to -3.4V and the history graph should follow in 1s, 10s or 60s setting
        * Step 1.6: Connect scope ch1+ to positive power supply and scope ch1- to negative supply
        * Step 1.7: Set the positive power supply voltage level to 5V and negative power supply to -5V
                * Expected Result: The voltage displayed in the voltmeter should be around 9.9V to 10.1V and the history graph should follow
        * Step 1.8: In step 2 replace scope ch1+ with scope ch1- and scope ch1- with scope ch1+ then repeat step 1.3
                * Expected Result: The voltage displayed in voltmeter should be around -3.2V to -3.3V and the history graph should follow in 1s, 10s or 60s setting
        * Step 1.9: In step 4 replace scope ch1+ with scope ch1- and scope ch1- to scope ch1+ then repeat step 1.5
        * Step 1.10: In step 6 replace scope ch1+ with scope ch1- and scope ch1- with scope ch1+ then repeat step 1.7
                * Expected Result: The voltage displayed in voltmeter should be around -9.9V to -10.1V and the history graph should follow in 1s, 10s or 60s setting
        * Step 2: Checking the AC Mode of channel 1 for low frequencies (20Hz to 800Hz)
        * Step 2.1: Set channel 1 in AC Mode (20Hz to 800Hz)
                * Expected Result: The interface should look like in the “Step Resources” picture (left side), the numerical value should indicate that it’s in AC mode by showing that it’s reading the VRMS of the signal.
        * Step 2.2: Connect scope ch1+ to AWG Ch1 and scope ch1- to gnd
        * Step 2.3: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Sine Wave, Amplitude: 2.828V, Offset: 0V, Frequency: 20Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 2.4: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Sine Wave, Amplitude: 5V, Offset: 0V, Frequency: 800Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 1.66Vrms to 1.86Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 2.5: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Square Wave, Amplitude: 2.000V, Offset: 0V, Frequency: 20Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 2.6: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Square Wave, Amplitude: 5V, Offset: 0V, Frequency: 800Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 2.4Vrms to 2.6Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 2.7: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Triangle Wave, Amplitude: 3.464V, Offset: 0V, Frequency: 20Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 2.8: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Triangle Wave, Amplitude: 7V, Offset: 0V, Frequency: 800Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 1.9Vrms to 2.1Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 3: Checking the AC Mode of channel 1 for high frequencies (800Hz to 40kHz)
        * Step 3.1: Set channel 1 in AC Mode (800Hz to 40kHz)
                * Expected Result: The interface should look like in the “Step Resources” picture (left side), the numerical value should indicate that it’s in AC mode by showing that it’s reading the VRMS of the signal.
        * Step 3.2: Connect scope ch1+ to AWG Ch1 and scope ch1- to gnd
        * Step 3.3: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Sine Wave, Amplitude: 2.828V, Offset: 0V, Frequency: 800Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 3.4: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Sine Wave, Amplitude: 5V, Offset: 0V, Frequency: 40kHz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 1.66Vrms to 1.86Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 3.5: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Square Wave, Amplitude: 2.000V, Offset: 0V, Frequency: 800Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 3.6: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Square Wave, Amplitude: 5V, Offset: 0V, Frequency: 40kHz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 2.4Vrms to 2.6Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 3.7: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Triangle Wave, Amplitude: 3.464V, Offset: 0V, Frequency: 800Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 3.8: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Triangle Wave, Amplitude: 7V, Offset: 0V, Frequency: 40kHz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 1.9Vrms to 2.1Vrms and the history graph should follow in 1s, 10s or 60s setting


Test 2: Channel 2 Operation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.M2K.VOLTMETER.CHANNEL_2_OPERATION

Description:
        - This test case verifies the functionality of the M2K voltmeter channel 2 operation.

OS:
        - any

Steps:
        * Step 1: Checking the DC Mode of channel 2
        * Step 1.1: Set channel 2 in DC Mode
                * Expected Result: The interface should look like in the “Step Resources” picture (left side), the numerical value should indicate that it’s in VDC mode.
        * Step 1.2: Connect scope ch2+ to positive supply and scope ch2- to gnd
        * Step 1.3: Set the positive power supply voltage level to 3.3V
                * Expected Result: The voltage displayed in the voltmeter should be around 3.2V to 3.4V and the history graph should follow in 1s, 10s or 60s setting
        * Step 1.4: Connect scope ch2+ to negative supply and scope ch2- to gnd
        * Step 1.5: Set the negative power supply voltage level to -3.3V
                * Expected Result: The voltage displayed in voltmeter should be around -3.2V to -3.4V and the history graph should follow in 1s, 10s or 60s setting
        * Step 1.6: Connect scope ch2+ to positive power supply and scope ch1- to negative supply
        * Step 1.7: Set the positive power supply voltage level to 5V and negative power supply to -5V
                * Expected Result: The voltage displayed in the voltmeter should be around 9.9V to 10.1V and the history graph should follow in 1s, 10s or 60s setting
        * Step 1.8: In step 2 replace scope ch2+ with scope ch2- and and scope ch2- with scope ch2+ then repeat step 1.3
                * Expected Result: The voltage displayed in voltmeter should be around -3.2V to -3.3V and the history graph should follow in 1s, 10s or 60s setting
        * Step 1.9: In step 4 replace scope ch2+ with scope ch2- and and scope ch2- with scope ch2+ then repeat step 1.5
                * Expected Result: The voltage displayed in voltmeter should be around 3.2V to 3.3V and the history graph should follow in 1s, 10s or 60s setting
        * Step 1.10: In step 6 replace scope ch2+ with scope ch2- and and scope ch2- with scope ch2+ then repeat step 1.7
                * Expected Result: The voltage displayed in voltmeter should be around -9.9V to -10.1V and the history graph should follow in 1s, 10s or 60s setting
        * Step 2: Checking the AC Mode of channel 2 for low frequencies (20Hz to 800Hz)
        * Step 2.1: Set channel 1 in AC Mode (20Hz to 800Hz)
                * Expected Result: The interface should look like in the “Step Resources” picture (left side), the numerical value should indicate that it’s in AC mode by showing that it’s reading the VRMS of the signal.
        * Step 2.2: Connect scope ch2+ to AWG ch1 and scope ch2- to gnd
        * Step 2.3: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Sine Wave, Amplitude: 2.828V, Offset: 0V, Frequency: 20Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 2.4: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Sine Wave, Amplitude: 5V, Offset: 0V, Frequency: 800Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 1.66Vrms to 1.86Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 2.5: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Square Wave, Amplitude: 2.000V, Offset: 0V, Frequency: 20Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 2.6: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Square Wave, Amplitude: 5V, Offset: 0V, Frequency: 800Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 2.4Vrms to 2.6Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 2.7: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Triangle Wave, Amplitude: 3.464V, Offset: 0V, Frequency: 20Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 2.8: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Triangle Wave, Amplitude: 7V, Offset: 0V, Frequency: 800Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 1.9Vrms to 2.1Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 3: Checking the AC Mode of channel 2 for high frequencies (800Hz to 40kHz)
        * Step 3.1: Set channel 1 in AC Mode (800Hz to 40kHz)
                * Expected Result: The interface should look like in the “Step Resources” picture (left side), the numerical value should indicate that it’s in AC mode by showing that it’s reading the VRMS of the signal.
        * Step 3.2: Connect scope ch2+ to AWG ch1 and scope ch2- to gnd
        * Step 3.3: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Sine Wave, Amplitude: 2.828V, Offset: 0V, Frequency: 800Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 3.4: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Sine Wave, Amplitude: 5V, Offset: 0V, Frequency: 40kHz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 1.66Vrms to 1.86Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 3.5: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Square Wave, Amplitude: 2.000V, Offset: 0V, Frequency: 800Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 3.6: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Square Wave, Amplitude: 5V, Offset: 0V, Frequency: 40kHz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 2.4Vrms to 2.6Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 3.7: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Triangle Wave, Amplitude: 3.464V, Offset: 0V, Frequency: 800Hz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms and the history graph should follow in 1s, 10s or 60s setting
        * Step 3.8: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Triangle Wave, Amplitude: 7V, Offset: 0V, Frequency: 40kHz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter should be around 1.9Vrms to 2.1Vrms and the history graph should follow in 1s, 10s or 60s setting


Test 3: Channel 1 and Channel 2 Operation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.M2K.VOLTMETER.CHANNEL_1_AND_CHANNEL_2_OPERATION

Description:
        - This test case verifies the functionality of the M2K voltmeter channel 1 and channel 2 operation.

OS:
        - any

Steps:
        * Step 1: Test both channels simultaneously in DC mode
        * Step 1.1: Set channel 1 and 2 in DC Mode
                * Expected Result: The interface should look like in the “Step Resources” picture (left side), the numerical value should indicate that it’s in VDC mode.
        * Step 1.2: Connect scope ch1+ to positive supply and scope ch1- to gnd. Connect scope ch2+ to negative supply and scope ch2- to gnd
        * Step 1.3: Set the positive power supply voltage level to 3.3V and negative power supply to -4.5V
                * Expected Result: The voltages shouldn’t interfere with each other. Voltage displayed in the voltmeter’s channel 1 should be around 3.2V to 3.4V and for voltmeter’s channel 2 should be around -4.6V to -4.4V. The history graph should follow in 1s, 10s or 60s setting
        * Step 1.4: Turn off the history graph of channel 1. Set the positive power supply voltage level to 3.3V and negative power supply to -4.5V
                * Expected Result: Turning off the history graph through the function shown on the picture shouldn’t reset or affect the voltage reading in the numerical display. Voltage displayed in the voltmeter’s channel 1 should be around 3.2V to 3.4V and for voltmeter’s channel 2 should be around -4.6V to -4.4V. The history graph of channel 2 should follow in 1s, 10s or 60s setting
        * Step 1.5: Turn off the history graph of channel 2. Set the positive power supply voltage level to 3.3V and negative power supply to -4.5V
                * Expected Result: Turning off the history graph through the function shown on the picture shouldn’t reset or affect the voltage reading in the numerical display. Voltage displayed in the voltmeter’s channel 1 should be around 3.2V to 3.4V and for voltmeter’s channel 2 should be around -4.6V to -4.4V. The history graph of channel 1 should follow in 1s, 10s or 60s setting
        * Step 1.6: Turn off the history graph of both channels. Set the positive power supply voltage level to 3.3V and negative power supply to -4.5V
                * Expected Result: Turning off the history graph through the function shown on the picture shouldn’t reset or affect the voltage reading in the numerical display. Voltage displayed in the voltmeter’s channel 1 should be around 3.2V to 3.4V and for voltmeter’s channel 2 should be around -4.6V to -4.4V.
        * Step 2: Test both channels simultaneously in AC mode
        * Step 2.1: Set channel 1 in low frequency AC mode and channel 2 in high frequency AC Mode
                * Expected Result: The interface should look like in the “Step Resources” picture (left side), the numerical value should indicate that it’s in AC mode by showing that it’s reading the VRMS of the signal.
        * Step 2.2: Connect scope ch1+ to AWG ch1 and scope ch1- to gnd. Connect scope ch2+ to AWG ch2 and scope ch2- to gnd
        * Step 2.3: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Sine Wave, Amplitude: 2.828V, Offset: 0V, Frequency: 200Hz and Phase: 0. Set the Signal generator’s channel 2 configuration to the following setting Waveform Type: Square Wave, Amplitude: 3, Offset: 0V, Frequency: 1kHz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter’s channel 1 should be around 0.9Vrms to 1.1Vrms and the voltage display for voltmeter’s channel 2 should be around 1.4Vrms to 1.6Vrms. The history graph should follow the voltage reading in 1s, 10s or 60s setting
        * Step 2.4: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Triangle Wave, Amplitude: 6.928V, Offset: 0V, Frequency: 200 Hz and Phase: 0. Set the Signal generator’s channel 2 configuration to the following setting Waveform Type: Sinewave, Amplitude: 2.828, Offset: 0V, Frequency: 1kHz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter’s channel 1 should be around 1.9Vrms to 2.1Vrms and the voltage display for voltmeter’s channel 2 should be around 0.9Vrms to 1.0Vrms. The history graph should follow the voltage reading in 1s, 10s or 60s setting
        * Step 3: Test one channel in DC mode and other channel in AC mode simultaneously
        * Step 3.1: Set channel 1 in DC Mode and channel 2 in AC Mode
                * Expected Result: The interface should look like in the “Step Resources” picture (left side), the numerical value should indicate that channel 1 is in VDC mode and channel 2 is in AC mode, channel 2 should measure the Vrms.
        * Step 3.2: Connect scope ch1+ to positive supply and scope ch1- to gnd. Connect scope ch2+ to AWG ch1 and scope ch2- to gnd
        * Step 3.3: Set the positive power supply voltage level to 3.3V. Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Sine Wave, Amplitude: 2.828V, Offset: 0V, Frequency: 10kHz and Phase: 0.
                * Expected Result: The voltage displayed in the voltmeter’s channel 1 should be around 3.2V to 3.4V and the voltage display for voltmeter’s channel 2 should be around 0.9Vrms to 1.1Vrms. The history graph should follow the voltage reading in 1s, 10s or 60s setting
        * Step 3.4: Set the positive power supply voltage level to 5V. Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Square Wave, Amplitude: 3, Offset: 0V, Frequency: 10kHz and Phase: 0.
                * Expected Result: The voltage displayed in the voltmeter’s channel 1 should be around 4.9V to 5.1V and the voltage display for voltmeter’s channel 2 should be around 1.4Vrms to 1.6Vrms. The history graph should follow the voltage reading in 1s, 10s or 60s setting
        * Step 3.5: Set channel 1 in AC Mode and channel 2 in DC Mode
                * Expected Result: The interface should look like in the “Step Resources” picture (left side), the numerical value should indicate that channel 1 is in AC mode and channel 2 is in DC mode, channel 1 should measure the Vrms.
        * Step 3.6: In step 3.2 replace scope ch1+ and scope ch1- with scope ch2+ and ch2- respectively and replace ch2+ and ch2- with ch1+ and ch1- respectively and repeat step 3.3
                * Expected Result: The voltage displayed in the voltmeter’s channel 2 should be around 3.2V to 3.4V and the voltage display for voltmeter’s channel 1 should be around 0.9Vrms to 1.1Vrms. The history graph should follow the voltage reading in 1s, 10s or 60s setting
        * Step 3.7: In step 3.2 replace scope ch1+ and scope ch1- with scope ch2+ and ch2- respectively and replace ch2+ and ch2- with ch1+ and ch1- respectively and repeat step 3.4
                * Expected Result: The voltage displayed in the voltmeter’s channel 2 should be around 4.9V to 5.1V and the voltage display for voltmeter’s channel 1 should be around 1.4Vrms to 1.6Vrms. The history graph should follow the voltage reading in 1s, 10s or 60s setting


Test 4: Additional Features
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.M2K.VOLTMETER.ADDITIONAL_FEATURES

Description:
        - This test case verifies the functionality of the M2K voltmeter additional features.

OS:
        - any

Steps:
        * Step 1: Test Peak hold feature
        * Step 1.1: Set channel 1 and 2 in DC Mode
                * Expected Result: The interface should look like in the “Step Resources” picture (left side), the numerical value should indicate that it’s in VDC mode.
        * Step 1.2: Connect scope ch1+ to positive supply and scope ch1- to gnd. Connect scope ch2+ to negative supply and scope ch2- to gnd
        * Step 1.3: Turn on the Peak hold feature of the voltmeter
                * Expected Result: The voltmeter window should now show the min and max indicator for both channels. See image for reference.
        * Step 1.4: Set +power supply to 2.5V and –power supply to -3V then turn on the power supply first before the voltmeter
                * Expected Result: The voltage displayed in channel 1’s max voltage should be around 2.4V to 2.6V and the min should still be 0V. The voltage displayed on channel 2’s min voltage should be around -3.1V to -2.9V and the max voltage should be 0V
        * Step 1.5: Following step 4 Set +power supply to 5 V and –power supply to -5V
                * Expected Result: The voltage displayed in channel 1’s max voltage should be around 4.9V to 5.1V and the min should still be 0V. The voltage displayed on channel 2’s min voltage should be around -5.1V to -4.9V and the max voltage should be 0V
        * Step 1.6: Connect scope ch1+ to negative supply and scope ch1- to gnd. Connect scope ch2+ to positive supply and scope ch2- to gnd
        * Step 1.7: Set +power supply to 2.5V and –power supply to -3V then turn on the power supply first before the voltmeter
                * Expected Result: The voltage displayed in channel 2’s max voltage should be around 2.4V to 2.6V and the min should still be -5V. The voltage displayed on channel 1’s min voltage should be around -3.1V to -2.9V and the max voltage should be 5V
        * Step 1.8: Following step 7 Set +power supply to 5 V and –power supply to -5V
                * Expected Result: The voltage displayed in channel 2’s max voltage should be around 4.9V to 5.1V and the min should still be -5V. The voltage displayed on channel 1’s min voltage should be around -5.1V to -4.9V and the max voltage should be 5V
        * Step 2: Test the reset instrument feature
        * Step 2.1: Stop Voltmeter instrument then click the reset instrument button for the peak hold features
                * Expected Result: The max and min reading for both channels should return to 0V.
        * Step 3: Test Data logging feature
        * Step 3.1: Set channel 1 in low frequency AC mode and channel 2 in high frequency AC Mode
                * Expected Result: The interface should look like in the “Step Resources” picture (left side), the numerical value should indicate that it’s in AC mode by showing that it’s reading the VRMS of the signal.
        * Step 3.2: Connect scope ch1+ to AWG ch1 and scope ch1- to gnd. Connect scope ch2+ to AWG ch2 and scope ch2- to gnd
        * Step 4: Testing Append mode
        * Step 4.1: Turn on the Data logging feature and choose Append
                * Expected Result: Refer to the image for reference
        * Step 4.2: For the timer choose 5 seconds
                * Expected Result: Refer to the image for reference
        * Step 4.3: Open a .csv file where the data will be logged
                * Expected Result: Refer to the image for reference
        * Step 4.4: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Sine Wave, Amplitude: 2.828V, Offset: 0V, Frequency: 200Hz and Phase: 0. Set the Signal generator’s channel 2 configuration to the following setting Waveform Type: Square Wave, Amplitude: 3, Offset: 0V, Frequency: 1kHz and Phase: 0. Run both the Signal generator and voltmeter
                * Expected Result: Wait for about 1 minute to record at least 6 readings.
        * Step 4.5: Stop the voltmeter and open the .csv file using MS Excel.
                * Expected Result: The voltmeter reading should be recorded on the .csv file with 5 second interval.
        * Step 4.6: Change the timer for 20 seconds
                * Expected Result: Refer to step 4's resource image for reference
        * Step 4.7: Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Triangle Wave, Amplitude: 6.928V, Offset: 0V, Frequency: 200 Hz and Phase: 0. Set the Signal generator’s channel 2 configuration to the following setting Waveform Type: Sinewave, Amplitude: 2.828, Offset: 0V, Frequency: 1kHz and Phase: 0
                * Expected Result: The voltage displayed in the voltmeter’s channel 1 should be around 1.9Vrms to 2.1Vrms and the voltage display for voltmeter’s channel 2 should be around 0.9Vrms to 1.0Vrms. Wait for about 1 minute to record at least 3 readings
        * Step 4.8: Stop the voltmeter and open the .csv file using MS Excel.
                * Expected Result: The voltmeter reading should be recorded on the .csv file in continuation with the previous reading and should now record with 20 second interval.
        * Step 5: Testing overwrite mode
        * Step 5.1: Turn on the Data logging feature and choose Overwrite
                * Expected Result: Refer to the image for reference
        * Step 5.2: Repeat steps 4.2 to 4.8
                * Expected Result: The results should be the same but every run and stop of the voltmeter should replace the data on the .csv file chosen completely with the new readings.
        * Step 6: Test range feature
        * Step 6.1: Set channel 1 and 2 in DC Mode with range for both channels set to +-25V. Turn on the Peak hold feature of the voltmeter
                * Expected Result: The interface should look like in the “Step Resources” picture (left side), the numerical value should indicate that it’s in VDC mode.
        * Step 6.2: Connect scope ch1+ to positive supply and scope ch1- to gnd. Connect scope ch2+ to negative supply and scope ch2- to gnd
        * Step 6.3: Set the positive power supply to 3.3V and the negative supply to -3.3V.
                * Expected Result: The voltmeter readings should be around [3.2V, 3.4V] for channel 1 and [-3.4V, -3.2V] for channel 2.
        * Step 6.4: Without disabling the power supply, change the range for both voltmeter channels to +-2.5V instead of +-25V.
                * Expected Result: “Out of range” should be raised for both channels.
        * Step 6.5: Still with range set to +-2.5V for both channels, set the power supply to output +100mV and -100mV.
                * Expected Result: The voltmeter readings should be around [0.097V, 0.103V] for channel 1 and [-0.103V, -0.097V] for channel 2.
        * Step 6.6: Without disabling the power supply, change the range for both voltmeter channels to +-25V instead of +-2.5V.
                * Expected Result: “Out of range” should be raised for both channels.
