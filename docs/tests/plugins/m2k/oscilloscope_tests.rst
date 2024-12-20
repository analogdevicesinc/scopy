.. _oscilloscope_tests:

Oscilloscope ~ Test cases
=========================
The following test cases are designed to verify the functionality of the Oscilloscope plugin.

Initial Setup
~~~~~~~~~~~~-

In order to proceed through the test case, first of all delete the Scopy *.ini file (saves previous settings made in Scopy tool) from the following path on Windows: ``C:\Users\your_username\AppData\Roaming\ADI``.

Open the Oscilloscope instrument. The interface must look like the picture below:

Press multiple times on the “Run” button to check if the instrument works.

Test 1 - Channel 1 operations
-----------------------------

Checking the Increment/Decrement Knobs: Time Base and Volts/div
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Open Channel 1’s settings. Set the knob to large increment (without an orange dot in the center). Change the Time base and Volts/div value using the ± button.
    - The time base value must change accordingly with a high increment/decrement from 2us/div to 5us/div. The volts/div value must change accordingly with a high increment/decrement from 2V/div to 5V/div, so must the graph follow, too.

2. Set the knob to ±1 unit interval (with an orange dot in the center) by clicking the dot in the middle of the knob. Change the Time base and Volts/div value using the ± button.
    - The time base and volts/div value must change accordingly.

Checking the Increment/Decrement Knobs: Position
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

3. Open Channel 1’s settings. Set the knob to large increment (without an orange dot in the center). Change the position value using the ± button.
    - The position value and graph must follow accordingly. The increment value is dependent to the value set in Time Base and Volts/div.

4. Open Channel 1’s settings. Set the knob to small increment (with an orange dot in the center) by clicking the dot in the middle of the knob. Change the position value using the ± button.
    - The position value and graph must follow accordingly. The increment value is dependent to the value set in Time Base and Volts/div.

Waveform Readout Verification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

5. Connect AWG CH1 to scope CH1+ and scope CH1- to GND.

Constant
~~~~~~~~

6. Signal Generator’s Configuration:
    - a. Constant, 3.3V
    - Oscilloscope’s Configuration:
        - a. Select Channel 1.
        - b. Trigger Settings: Trigger mode – Auto
        - c. Run the Signal Generator and the Oscilloscope.
    - The interface must look like the image in the “Step Resources” picture on the left side.

7. Enable “Measure”, open Measure Settings’ Menu, and turn on “Display All” to show all built~in signal measurements.
    - The interface must look like the image in the “Step Resources” picture on the left side. Within the list of readings, the RMS reading must be within 3.2V to 3.4V.

8. Change the value and monitor it on the oscilloscope.
    - The reading must be approximately close to the set value.

Sine Wave
~~~~~~~~~

9. Signal Generator’s Configuration:
    ~ a. Sine wave, 2Vpp, 200Hz, 0V Offset, 0deg Phase
    ~ Oscilloscope’s Configurations:
        ~ a. Select Channel 1.
        ~ b. Horizontal: Time Base – 500us/div, Position – 0ms
        ~ c. Vertical: Volts/Div – 500mV/div, Position – 0V
        ~ d. Trigger Settings: Trigger mode – Auto
        ~ e. Run the Signal Generator and the Oscilloscope.
    ~ The interface must look like the image in the “Step Resources” picture on the left side. The oscilloscope must display 1.5 cycle sine wave. Period: 5ms, Frequency: 200 Hz, Peak~peak: 1.9Vpp to 2.1Vpp, RMS: 0.6Vrms to 0.8Vrms

10. Change the amplitude to 5V and the frequency to 500Hz.
    ~ The oscilloscope must display 4 cycles of sine wave. Period: 2ms, Frequency: 500 Hz, Peak~peak: 4.9Vpp to 5.1Vpp, RMS: 1.74Vrms to 1.78Vrms

Square Wave
~~~~~~~~~~~

11. Signal Generator’s Configurations:
    ~ a. Square wave, 5Vpp, 500Hz, 0V Offset, 0deg Phase
    ~ Oscilloscope’s Configurations:
        ~ a. Select Channel 1.
        ~ b. Horizontal: Time Base – 500us/div, Position – 0ms
        ~ c. Vertical: Volts/Div – 1V/div, Position – 0V
        ~ d. Trigger Settings: Trigger mode – Auto
        ~ e. Run the Signal Generator and the Oscilloscope.
    ~ The interface must look like the image in the “Step Resources” picture on the left side. The oscilloscope must display 8 square waves. Period: 2ms, Frequency: 500 Hz, amplitude: 4.9Vpp to 5.1Vpp, RMS: 2.4Vrms to 2.6Vrms

12. Change the amplitude to 8V and the frequency to 2 kHz. On the oscilloscope change the time base to 200us/div.
    ~ The oscilloscope must display 6 cycles of square wave. Period: 500us, Frequency: 2 kHz, amplitude: 7.9Vpp to 8.1Vpp, RMS: 3.9Vrms to 4.1Vrms

Triangle Wave
~~~~~~~

13. Signal Generator’s Configurations:
    ~ a. Triangle wave, 4Vpp, 2kHz, 0V Offset, 0deg Phase
    ~ Oscilloscope’s Configurations:
        ~ a. Select Channel 1.
        ~ b. Horizontal: Time Base – 200us/div, Position – 0ms
        ~ c. Vertical: Volts/Div – 1V/div, Position – 0V
        ~ d. Trigger Settings: Trigger mode – Auto
        ~ e. Run the Signal Generator and the Oscilloscope.
    ~ The interface must look like the image in the “Step Resources” picture on the left side. The oscilloscope must display 6 triangle waves. Period: 2ms, Frequency: 2 kHz, Peak~peak: 3.9Vpp to 4.1Vpp, RMS: 1.0Vrms to 1.2Vrms

14. Change the amplitude to 5V and the frequency to 20kHz. On the oscilloscope, change the time base to 50us/div and the volts/div to 1V/div.
    ~ The oscilloscope must display 6 cycles of square wave. Period: 50us, Frequency: 20 kHz, amplitude: 4.9Vpp to 5.1Vpp, RMS: 1.3Vrms to 1.5Vrms

Rising/Falling Ramp Sawtooth Wave
~~~~~~~~~~~~~~~~~~~~~

15. Signal Generator’s Configurations:
    ~ a. Rising Ramp Sawtooth, 8Vpp, 20kHz, 0V Offset, 0deg Phase
    ~ Oscilloscope’s Configurations:
        ~ a. Select Channel 1.
        ~ b. Horizontal: Time Base – 10us/div, Position – 0ms
        ~ c. Vertical: Volts/Div – 2V/div, Position – 0V
        ~ d. Trigger Settings: Trigger mode – Auto
        ~ e. Run the Signal Generator and the Oscilloscope.
    ~ The interface must look like the image in the “Step Resources” picture on the left side. The oscilloscope must display 3 sawtooth waves. Period: 50us, Frequency: 20 Hz, Peak~peak: 7.9Vpp to 8.1Vpp, RMS: 2.2Vrms to 2.4Vrms

16. Change waveform to Falling Ramp Sawtooth.
    ~ The interface must look like the image in the “Step Resources” picture on the left side. The oscilloscope must display 3 sawtooth waves. Period: 50us, Frequency: 20 Hz, Peak~peak: 7.9Vpp to 8.1Vpp, RMS: 2.2Vrms to 2.4Vrms

Cursors’ Reading Verification
~~~~~~~~~~~~~~~~~

17. Signal Generator’s Configurations:
    ~ a. Sine wave, 2Vpp, 200Hz, 0V Offset, 0deg Phase
    ~ Oscilloscope’s Configurations:
        ~ a. Select Channel 1.
        ~ b. Horizontal: Time Base – 1ms/div, Position – 0ms
        ~ c. Vertical: Volts/Div – 500mV/div, Position – 0V
        ~ d. Trigger Settings: Trigger mode – Auto
        ~ e. Turn “Cursors” on. Turn “Measure” off.
        ~ f. Run the Signal Generator and the Oscilloscope.
    ~ The interface must look like the image in the “Step Resources” picture on the left side. The oscilloscope must display 3 sawtooth waves. Period: 50us, Frequency: 20 Hz, Peak~peak: 7.9Vpp to 8.1Vpp, RMS: 2.2Vrms to 2.4Vrms

18. Adjust the horizontal cursors such that the left cursor, CurT2, is on the positive~going zero crossing point and the right cursor, CurT1, is located on the adjacent positive~going zero crossing point. This corresponds to one period of the signal.
    ~ The interface must look like the image in the “Step Resources” picture on the left side. ΔT must be around 5ms. And 1/ΔT must correspond to the frequency around 200Hz.

19. Adjust the vertical cursors such that the upper cursor, CurV1, is on the crest of the sine wave and the lower cursor, CurV2, is on the trough. This corresponds to the peak~peak amplitude of the sine wave.
    ~ The interface must look like the image in the “Step Resources” picture on the left side. ΔV must be around 2V.

20. Turn off the horizontal and the vertical cursors on the Cursor’s settings.
    ~ The cursors seen on the plot as well as the readouts must disappear.

21. Turn off “Cursors.”
    ~ The interface must look like the image in the “Step Resources” picture on the left side.

Statistics
~~~~~~~~~~

22. Enable “Measure.” Go to its settings and turn “Statistics” on. Under the “Custom Selection”, click on the horizontal dropdown, select “Period” and “Frequency” under the “Stats” column. On the vertical dropdown, select peak~peak, RMS, and amplitude.
    ~ The interface must look like the image in the “Step Resources” picture on the left. The Min, Max, and Avg value of the measurement is shown in statistics feature.

23. Turn off the Measure feature.
    ~ The interface must look like the image in the “Step Resources” picture on the left.

Trigger Function Verification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

24. Trigger Settings’ Configuration:
    ~ a. Internal (Analog): Level – ±500mV, Hysteresis – ±50mV
    ~ b. Set the knob to large increment (without an orange dot in the center).
    ~ c. Change the values using the ± button. Its interval is dependent to the Volts/div value set. In order to have a ± 500mV and ±50mV intervals, set the volts/div to 500 Volts/div.
    ~ The level and hysteresis value and the graph must follow accordingly.

25. Trigger Settings’ Configuration:
    ~ a. Internal (Analog): Level – ±50mV, Hysteresis – ±5mV
    ~ b. Set the knob to small increment (with an orange dot in the center) by clicking the dot in the middle of the knob.
    ~ c. Change the values using the ± button.
    ~ The level and hysteresis value and the graph must follow accordingly.

26. Signal Generator’s Configurations:
    ~ a. Triangle wave, 5Vpp, 200Hz
    ~ Oscilloscope’s Configurations:
        ~ a. Select Channel 1.
        ~ b. Horizontal: Time Base – 1ms/div, Position – 0ms
        ~ c. Vertical: Volts/Div – 1V/div, Position – 0V
        ~ d. Trigger Settings: Trigger mode – Auto, Internal – ON, Source – Channel 1, Condition – Rising Edge, Level – 0V, Hysteresis – 50mV
        ~ e. Run the Signal Generator and the Oscilloscope.
    ~ The interface must look like the image in the “Step Resources” picture on the left. At the origin, the graph must start at the rising edge of the triangle wave. The signal must be static and must not be “dancing.”

27. Change condition to “Falling Edge.”
    ~ At the origin, the graph must start at the falling edge of the triangle wave. The signal must be static and must not be “dancing.”

28. Change Trigger Settings’ Configuration (Hysteresis’ Minimum Range):
    ~ a. Trigger Settings: Condition – Rising Edge, Hysteresis – 10mV (minimum)
    ~ b. Move the trigger level cursor until the trace is unstable.
    ~ As you move the trigger level cursor, the trace is triggered in all of the rising edge parts.

29. Change Trigger Settings’ Configuration (Hysteresis’ Middle Range):
    ~ a. Trigger Settings: Condition – Rising Edge, Hysteresis – 1.25V (midrange)
    ~ b. Move the trigger level cursor until the trace is unstable.
    ~ As you move the trigger level cursor, the trace must be triggered from ~1.3 V to +2.5V. Below ~1.3V, the trace must not be triggered.

30. Change Trigger Settings’ Configuration (Hysteresis’ Maximum Range):
    ~ a. Trigger Settings: Condition – Rising Edge, Hysteresis – 2.5V (maximum range)
    ~ b. Move the trigger level cursor until the trace is unstable.
    ~ As you move the trigger level cursor, the trace must be triggered from 0 V to +2.5V. The trace must not be triggered below and above the said range.


Test 2 - Channel 2 operations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Checking the Increment/Decrement Knobs: Time Base and Volts/div
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~-

1. Open Channel 2’s settings. Set the knob to large increment (without an orange dot in the center). Change the Time base and Volts/div value using the ± button.
    - The time base value must change accordingly with a high increment/decrement from 2us/div to 5us/div. The volts/div value must change accordingly with a high increment/decrement from 2V/div to 5V/div, so must the graph follow, too.

2. Set the knob to ±1 unit interval (with an orange dot in the center) by clicking the dot in the middle of the knob. Change the Time base and Volts/div value using the ± button.
    - The time base and volts/div value must change accordingly.

Checking the Increment/Decrement Knobs: Position
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

3. Open Channel 2’s settings. Set the knob to large increment (without an orange dot in the center). Change the position value using the ± button.
    - The position value and graph must follow accordingly. The increment value is dependent to the value set in Time Base and Volts/div.

4. Open Channel 2’s settings. Set the knob to small increment (with an orange dot in the center) by clicking the dot in the middle of the knob. Change the position value using the ± button.
    - The position value and graph must follow accordingly. The increment value is dependent to the value set in Time Base and Volts/div.

Waveform Readout Verification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

5. Connect AWG CH1 to scope CH2+ and scope CH2- to GND.

Constant
~~~~~~~~

6. Signal Generator’s Configuration:
    - a. Constant, 3.3V
    - Oscilloscope’s Configuration:
        - a. Select Channel 2.
        - b. Trigger Settings: Trigger mode – Auto
        - c. Run the Signal Generator and the Oscilloscope.
    - The interface must look like the image in the “Step Resources” picture on the left side.

7. Enable “Measure”, open Measure Settings’ Menu, and turn on “Display All” to show all built-in signal measurements.
    - The interface must look like the image in the “Step Resources” picture on the left side. Within the list of readings, the RMS reading must be within 3.2V to 3.4V.

8. Change the value and monitor it on the oscilloscope.
    - The reading must be approximately close to the set value.

Sine Wave
~~~~~~~~-

9. Signal Generator’s Configuration:
    - a. Sine wave, 2Vpp, 200Hz, 0V Offset, 0deg Phase
    - Oscilloscope’s Configurations:
        - a. Select Channel 2.
        - b. Horizontal: Time Base – 500us/div, Position – 0ms
        - c. Vertical: Volts/Div – 500mV/div, Position – 0V
        - d. Trigger Settings: Trigger mode – Auto
        - e. Run the Signal Generator and the Oscilloscope.
    - The interface must look like the image in the “Step Resources” picture on the left side. The oscilloscope must display 1.5 cycle sine wave. Period: 5ms, Frequency: 200 Hz, Peak-peak: 1.9Vpp to 2.1Vpp, RMS: 0.6Vrms to 0.8Vrms

10. Change the amplitude to 5V and the frequency to 500Hz.
    - The oscilloscope must display 4 cycles of sine wave. Period: 2ms, Frequency: 500 Hz, Peak-peak: 4.9Vpp to 5.1Vpp, RMS: 1.74Vrms to 1.78Vrms

Square Wave
~~~~~~~~~~-

11. Signal Generator’s Configurations:
    - a. Square wave, 5Vpp, 500Hz, 0V Offset, 0deg Phase
    - Oscilloscope’s Configurations:
        - a. Select Channel 2.
        - b. Horizontal: Time Base – 500us/div, Position – 0ms
        - c. Vertical: Volts/Div – 1V/div, Position – 0V
        - d. Trigger Settings: Trigger mode – Auto
        - e. Run the Signal Generator and the Oscilloscope.
    - The interface must look like the image in the “Step Resources” picture on the left side. The oscilloscope must display 8 square waves. Period: 2ms, Frequency: 500 Hz, amplitude: 4.9Vpp to 5.1Vpp, RMS: 2.4Vrms to 2.6Vrms

12. Change the amplitude to 8V and the frequency to 2 kHz. On the oscilloscope change the time base to 200us/div.
    - The oscilloscope must display 6 cycles of square wave. Period: 500us, Frequency: 2 kHz, amplitude: 7.9Vpp to 8.1Vpp, RMS: 3.9Vrms to 4.1Vrms

Triangle Wave
~~~~~~~~~~~~-

13. Signal Generator’s Configurations:
    - a. Triangle wave, 4Vpp, 2kHz, 0V Offset, 0deg Phase
    - Oscilloscope’s Configurations:
        - a. Select Channel 2.
        - b. Horizontal: Time Base – 200us/div, Position – 0ms
        - c. Vertical: Volts/Div – 1V/div, Position – 0V
        - d. Trigger Settings: Trigger mode – Auto
        - e. Run the Signal Generator and the Oscilloscope.
    - The interface must look like the image in the “Step Resources” picture on the left side. The oscilloscope must display 6 triangle waves. Period: 2ms, Frequency: 2 kHz, Peak-peak: 3.9Vpp to 4.1Vpp, RMS: 1.0Vrms to 1.2Vrms

14. Change the amplitude to 5V and the frequency to 20kHz. On the oscilloscope, change the time base to 50us/div and the volts/div to 1V/div.
    - The oscilloscope must display 6 cycles of square wave. Period: 50us, Frequency: 20 kHz, amplitude: 4.9Vpp to 5.1Vpp, RMS: 1.3Vrms to 1.5Vrms

Rising/Falling Ramp Sawtooth Wave
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~-

15. Signal Generator’s Configurations:
    - a. Rising Ramp Sawtooth, 8Vpp, 20kHz, 0V Offset, 0deg Phase
    - Oscilloscope’s Configurations:
        - a. Select Channel 2.
        - b. Horizontal: Time Base – 10us/div, Position – 0ms
        - c. Vertical: Volts/Div – 2V/div, Position – 0V
        - d. Trigger Settings: Trigger mode – Auto
        - e. Run the Signal Generator and the Oscilloscope.
    - The interface must look like the image in the “Step Resources” picture on the left side. The oscilloscope must display 3 sawtooth waves. Period: 50us, Frequency: 20 Hz, Peak-peak: 7.9Vpp to 8.1Vpp, RMS: 2.2Vrms to 2.4Vrms

16. Change waveform to Falling Ramp Sawtooth.
    - The interface must look like the image in the “Step Resources” picture on the left side. The oscilloscope must display 3 sawtooth waves. Period: 50us, Frequency: 20 Hz, Peak-peak: 7.9Vpp to 8.1Vpp, RMS: 2.2Vrms to 2.4Vrms

Cursors’ Reading Verification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

17. Signal Generator’s Configurations:
    - a. Sine wave, 2Vpp, 200Hz, 0V Offset, 0deg Phase
    - Oscilloscope’s Configurations:
        - a. Select Channel 2.
        - b. Horizontal: Time Base – 1ms/div, Position – 0ms
        - c. Vertical: Volts/Div – 500mV/div, Position – 0V
        - d. Trigger Settings: Trigger mode – Auto
        - e. Turn “Cursors” on. Turn “Measure” off.
        - f. Run the Signal Generator and the Oscilloscope.
    - The interface must look like the image in the “Step Resources” picture on the left side. The oscilloscope must display 3 sawtooth waves. Period: 50us, Frequency: 20 Hz, Peak-peak: 7.9Vpp to 8.1Vpp, RMS: 2.2Vrms to 2.4Vrms

18. Adjust the horizontal cursors such that the left cursor, CurT2, is on the positive-going zero crossing point and the right cursor, CurT1, is located on the adjacent positive-going zero crossing point. This corresponds to one period of the signal.
    - The interface must look like the image in the “Step Resources” picture on the left side. ΔT must be around 5ms. And 1/ΔT must correspond to the frequency around 200Hz.

19. Adjust the vertical cursors such that the upper cursor, CurV1, is on the crest of the sine wave and the lower cursor, CurV2, is on the trough. This corresponds to the peak-peak amplitude of the sine wave.
    - The interface must look like the image in the “Step Resources” picture on the left side. ΔV must be around 2V.

20. Turn off the horizontal and the vertical cursors on the Cursor’s settings.
    - The cursors seen on the plot as well as the readouts must disappear.

21. Turn off “Cursors.”
    - The interface must look like the image in the “Step Resources” picture on the left side.

Statistics
~~~~~~~~~~

22. Enable “Measure.” Go to its settings and turn “Statistics” on. Under the “Custom Selection”, click on the horizontal dropdown, select “Period” and “Frequency” under the “Stats” column. On the vertical dropdown, select peak-peak, RMS, and amplitude.
    - The interface must look like the image in the “Step Resources” picture on the left. The Min, Max, and Avg value of the measurement is shown in statistics feature.

23. Turn off the Measure feature.
    - The interface must look like the image in the “Step Resources” picture on the left.

Trigger Function Verification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~-

24. Trigger Settings’ Configuration:
    - a. Internal (Analog): Level – ±500mV, Hysteresis – ±50mV
    - b. Set the knob to large increment (without an orange dot in the center).
    - c. Change the values using the ± button. Its interval is dependent to the Volts/div value set. In order to have a ± 500mV and ±50mV intervals, set the volts/div to 500 Volts/div.
    - The level and hysteresis value and the graph must follow accordingly.

25. Trigger Settings’ Configuration:
    - a. Internal (Analog): Level – ±50mV, Hysteresis – ±5mV
    - b. Set the knob to small increment (with an orange dot in the center) by clicking the dot in the middle of the knob.
    - c. Change the values using the ± button.
    - The level and hysteresis value and the graph must follow accordingly.

26. Signal Generator’s Configurations:
    - a. Triangle wave, 5Vpp, 200Hz
    - Oscilloscope’s Configurations:
        - a. Select Channel 1.
        - b. Horizontal: Time Base – 1ms/div, Position – 0ms
        - c. Vertical: Volts/Div – 1V/div, Position – 0V
        - d. Trigger Settings: Trigger mode – Auto, Internal – ON, Source – Channel 1, Condition – Rising Edge, Level – 0V, Hysteresis – 50mV
        - e. Run the Signal Generator and the Oscilloscope.
    - The interface must look like the image in the “Step Resources” picture on the left. At the origin, the graph must start at the rising edge of the triangle wave. The signal must be static and must not be “dancing.”

27. Change condition to “Falling Edge.”
    - At the origin, the graph must start at the falling edge of the triangle wave. The signal must be static and must not be “dancing.”

28. Change Trigger Settings’ Configuration (Hysteresis’ Minimum Range):
    - a. Trigger Settings: Condition – Rising Edge, Hysteresis – 10mV (minimum)
    - b. Move the trigger level cursor until the trace is unstable.
    - As you move the trigger level cursor, the trace is triggered in all of the rising edge parts.

29. Change Trigger Settings’ Configuration (Hysteresis’ Middle Range):
    - a. Trigger Settings: Condition – Rising Edge, Hysteresis – 1.25V (midrange)
    - b. Move the trigger level cursor until the trace is unstable.
    - As you move the trigger level cursor, the trace must be triggered from -1.3 V to +2.5V. Below -1.3V, the trace must not be triggered.

30. Change Trigger Settings’ Configuration (Hysteresis’ Maximum Range):
    - a. Trigger Settings: Condition – Rising Edge, Hysteresis – 2.5V (maximum range)
    - b. Move the trigger level cursor until the trace is unstable.
    - As you move the trigger level cursor, the trace must be triggered from 0 V to +2.5V. The trace must not be triggered below and above the said range.


Test 3 - Math channel operations
---------------------------------

C. Math Channel Operation
-------------------------

Description
-----------

1. Connect AWG CH1 to scope CH1+ and scope CH1- to GND. Connect AWG CH2 to scope CH2+ and scope CH2- to GND.

2. Signal Generator’s Configurations:
    - a. For Channel 1: Sine wave, 5Vpp, 500Hz
    - b. For Channel 2: Square wave, 2Vpp, 500Hz
    - Oscilloscope’s Configurations (For Channels 1 and 2):
        - a. Horizontal: Time Base – 500us/div, Position – 0ms
        - b. Vertical: Volts/Div – 1V/div, Position – 0V
        - c. Trigger Settings: Trigger mode – Auto

Adding Math Channel 1 using CH1
-------------------------------

3. Click on the plus sign beside Channel 2 to add the Math channel. In the box, input the following: `sqrt(t0*t0)`. Note that “t0” is the signal in CH1.
    - Oscilloscope must display the absolute values of Channel 1.

4. Turn on the “Display All” Measurement feature on for CH1 and Math CH1.
    - CH1 peak-peak: 4.9Vpp to 5.1 Vpp, Math CH1 peak-peak: 2.4Vpp to 2.6Vpp since there is no negative part.

Adding Math Channel 2 using CH2
-------------------------------

5. Click on the plus sign to add another Math channel. In the box, input the following: `2*(t1+t1)`. Note that “t1” is the signal in CH2.
    - The interface must look like the image in the “Step Resources” picture on the left. The amplitude of CH2 must increase 4 times based on the function.

6. Turn on the “Display All” Measurement feature on for CH2 and Math CH2.
    - CH2 peak-peak: 1.9Vpp to 2.1 Vpp, Math CH2 peak-peak: 7.9Vpp to 8.1Vpp. The amplitude of CH2 must increase 4 times based on the function.

Adding Math Channel 3 using both CH1 and CH2
--------------------------------------------

7. Click on the plus sign to add another channel. Set these Signal Generator’s Configurations:
    - a. For Channel 1: Sine wave, 5Vpp, 500Hz
    - b. For Channel 2: Sine wave, 2Vpp, 1kHz
    - Oscilloscope’s Configurations:
        - a. Select Channel 1.
        - b. Horizontal: Time Base – 500us/div, Position – 0ms
        - c. Vertical: Volts/Div – 1V/div, Position – 0V
        - d. Add Math CH3: `t0+t1`.

Adding Math Channel 4 using both CH1 and CH2
--------------------------------------------

8. Click on the plus sign to add another channel. Add math channel 4: `t0*t1`.
    - The interface must look like the image in the “Step Resources” picture on the left. The math pad panel will automatically close after adding the fourth math channel. This indicates that you have reached the number of math channels to be added. The plus sign on the bottom also disappears.


FFT Function
------------

1. Open the General Settings. Turn on FFT. Set the Signal Generator to the following:
    - a. For Channel 1: Square wave, 5Vpp, 1kHz
    - Oscilloscope’s Configurations:
        - a. Select Channel 1.
        - b. Horizontal: Time Base – 5ms/div, Position – 0ms
        - c. Vertical: Volts/Div – 1V/div, Position – 0V
    - The interface must look like the image in the “Step Resources” picture on the left.

X-Y Plot
--------

2. Plot the current vs voltage characteristics of a PN junction diode. Use 1k-ohm resistor and a 1N914 small signal diode. Refer to the Steps Resources picture for the circuit and breadboard connections.

3. Signal Generator’s Configuration:
    - a. For Channel 1: Triangle wave, 6Vpp, 100Hz, 0V Offset
    - Oscilloscope’s Configurations:
        - a. General Settings: XY (View) – ON (to plot the voltage across the diode (scope CH1) on the X axis against the current in the diode (scope CH2) on the Y axis)
    - The XY plot must look like the image in the Steps Resources on the left.

Export Feature
--------------

4. Remove the connections to the previous circuit. Set these configurations to the Signal Generator:
    - a. For Channel 1: Sine wave, 2Vpp, 200Hz
    - b. For Channel 2: Square wave, 5Vpp, 500Hz
    - c. Connect W1 to scope CH1 and W2 to scope CH2. Monitor this on the Oscilloscope.
    - Oscilloscope’s Configurations:
        - a. Select Channel 1.
        - b. Horizontal: Time Base – 1ms/div, Position – 0ms
        - c. Vertical: Volts/Div – 1V/div, Position – 0V
        - d. Run both instruments.
    - The interface must look like the image in the Steps Resources column.

Export Channel 1
----------------

5. Open General Settings and turn off “Export All.” Export only Channel 1 by selecting it in the channel choices. Click “Export.”
    - The interface must look like the image in the Steps Resources column.

6. A window will pop up for saving the file in .csv format. Set the file name as Channel1.csv. Save the file.
    - The interface must look like the image in the Steps Resources column.

7. Open the file.
    - The interface must look like the image in the Steps Resources column.

Export Channel 2
----------------

8. To export Channel 2, repeat the steps done in Channel 1 and save it as Channel2.csv.
    - The interface must look like the image in the Steps Resources column.

Exporting All Channels
----------------------

9. Add Math Channel 1, (2*t0). To export all channels including the Math channel, turn on the “Export All.” All the channels will be included in the file. Repeat the same steps of exporting a channel and save the file as allchannels.csv.
    - The interface must look like the image in the Steps Resources column.

Snapshot and Reference Waveform
-------------------------------

10. Add a reference waveform by clicking the ‘+’ button, just as you would add a Math channel. Click “Reference” and browse for Channel1.csv that was exported earlier. The channels available for import must only be Channel 1. Click import. Do the same to Channel 2.
    - The loaded waveform must be the same as what had been saved earlier. This is now a reference waveform.

11. Add another reference waveform; select allchannels.csv. The channels available for import must now be Channel 1, Channel 2, and Math Channel 1. Select only Math Channel 1. Click import.
    - The waveform must load the same as what has been saved earlier. This is now a reference waveform.

12. Add another reference waveform using Snapshot. Increase the amplitude of the waveform in Signal Generator’s CH1. Monitor CH1 of the oscilloscope, then click “Snapshot.”
    - REF 4 must be added. The waveform must correspond to the signal set in signal generator.

Preferences
-----------

13. Click “Preferences” at the bottom of the instrument list, go to the “Oscilloscope” section, and check the box for “Enable labels on the plot”.
    - The plot must look like the image in the Steps Resources on the left. Labels must be seen respectively in each channel and math channels. The labels must follow accordingly as the time base or volts/div is changed.

Software AC Coupling
--------------------

14. Set these configurations in the Signal Generator:
    - a. For Channel 1: Sine wave, 2Vpp, 1kHz, 3V Offset
    - Oscilloscope’s Configurations:
        - a. Select Channel 1.
        - b. Horizontal: Time Base – 200us/div, Position – 0ms
        - c. Vertical: Volts/Div – 1V/div, Position – 0V
        - d. Run both instruments.
    - The plot must look like the image in the Steps Resources on the left. It shows that there is a 3-volt offset.

15. Turn on “AC Software Coupling” on oscilloscope CH1.
    - The trace must slowly move towards 0V until it looks like the image in the Steps Resources on the left.

16. Repeat steps 14 and 15 for oscilloscope Channel 2.
    - The same result must show.

Probe Attenuation
-----------------

17. Turn on “Measurement” and turn on “Display All.” Change the probe attenuation to 0.1X, 10X, 100X, respectively.
    - As the probe attenuation is changed, the measurement must change respectively i.e. 2Vpp with 10X must be shown as 20Vpp. Labels on the plot as well as the Volts/div must also change.

External Trigger
----------------

18. Connect the pins as shown in the Steps Resources on the right.

19. Pattern Generator’s Configuration:
    - a. Enable DIO-0 and set as clock with 5kHz frequency.
    - Oscilloscope’s Configurations:
        - a. Select Channel 1.
        - b. Horizontal: Time Base – 200us/div, Position – 0ms
        - c. Vertical: Volts/Div – 1V/div, Position – 0V
        - d. Trigger Settings: Internal (Analog) – OFF, External Trigger – ON; Digital: Source – Channel 1, Condition – Rising Edge; Set to Auto (?)
    - The interface must look like the image in the “Step Resources” picture on the left. Trace is triggered at rising edge.

20. Change the condition to “Falling Edge.”
    - Trace is triggered at falling edge. When changing the condition, the plot must follow accordingly.

21. Disconnect T1 and replace it with T0. Change channel source to Channel 2. Repeat steps 18 and 19.
    - The trace must still be triggered the same as with T1.

Autoset
-------

22. Disconnect the connections from the previous steps. Connect Scope CH1+ to W1 and Scope CH1- to GND. Set these on the Signal Generator:
    - a. For Channel 1: Sine wave, 2Vpp, 5MHz
    - b. Run Signal Generator and Oscilloscope. On the Oscilloscope, click “Autoset.”
    - After clicking, wait until the signal/waveform is properly displayed. Trigger must be automatically set to (max-min)/2 - at 50%. Vertical and Horizontal values must automatically be adjusted.

23. Change signal to a 5V, 500kHz sinewave. Click “Autoset” again.
    - After clicking, wait until the signal/waveform is properly displayed. Trigger must be automatically set to (max-min)/2 - at 50%. Vertical and Horizontal values must automatically be adjusted.

24. Change the signal to a 10V, 500 Hz sinewave. Click “Autoset” again.
    - After clicking, wait until the signal/waveform is properly displayed. Trigger must be automatically set to (max-min)/2 - at 50%. Vertical and Horizontal values must automatically be adjusted.

25. Repeat steps with Channel 2.

Print Plot
----------

26. Connect Oscilloscope’s CH1 to Signal Generator’s CH1+. Connect Oscilloscope’s CH2 to Signal Generator’s CH2. Set these on the Signal Generator:
    - a. For Channel 1: Sine wave, 10Vpp, 1kHz
    - b. For Channel 2: Square wave, 5Vpp, 1kHz
    - Oscilloscope’s Configurations (For Channels 1 and 2):
        - a. Horizontal: Time Base – 200us/div, Position – 0ms
        - b. Vertical: Volts/Div – 1V/div, Position – 0V
        - c. Press the “Print” button in the top left of the Oscilloscope instrument and save the file in pdf format.
    - The expected result must be a pdf file: plotscreenshot.pdf

Curve Style
-----------

27. On the Signal Generator, set Channel 1’s frequency to 2MHz and Channel 2’s to 5MHz. Monitor both channels in the Oscilloscope. Change curve styles of both channels.
    - The waveform must correspond to the changes set on the curve style.

Gating
------

28. Signal Generator’s Configurations:
    - a. For Channel 1: Sine wave, 10Vpp, 10kHz
    - b. For Channel 2: Sine wave, 5Vpp, 1Hz
    - Oscilloscope’s Configurations:
        - a. Enable the “Measure” feature and turn the “Gating Settings” on.
        - b. For Channel 1: Move the gating sliders to ~0s and ~50us.
        - c. For Channel 2: Move the gating sliders to ~0s and ~500us.
    - The peak-to-peak reading must be half the set amplitude. Channel 1 peak-peak = ~5V, channel 2 peak-peak = ~2.5V.

Histogram
---------

29. Open the General Settings and turn the “Histogram” on. Change the waveform of both channels to square.
    - A window must be displayed showing the histogram of the waveform. For this case, the histogram must show the high and low value of the square waves.

ADC Digital Filters
-------------------

30. Refer to the following document for ADC digital Filter calibration.
    - Compensation using the digital filters

31. Set these on the Signal Generator:
    - a. For Channel 1: Square wave, 2Vpp, 1kHz
    - Oscilloscope’s Configurations:
        - a. Select Channel 1.
        - b. Horizontal: Time Base – 200ms/div, Position – 0ms
        - c. Vertical: Volts/Div – 500mV/div, Position – 0V
        - d. Calibrate the signal with the hardware trimmers.
    - The interface must look like the image in the “Step Resources” picture on the left side.

32. Set the Oscilloscope’s Volt/Div to 1V/Div (high gain mode).
    - The waveform was adjusted in low gain mode and in high gain mode it must look similar to the step resources picture.

33. Enable the ADC Filters (Filter 1 Filter 2) from channel settings and set the parameters as follows: Filter 1: TC=60, gain=-0.025, Filter2: TC=9, gain=0.047.
    - The result must be like in the step resources picture where signals are as follows: Initial signal (green), Filter 1 signal (cyan), cascaded filters signal (orange).

34. Now if you switch between high gain (±2.5V) and low gain (±25V), you must see a Square waveform in both cases.
    - The filters must be active in the mode they were enabled.

35. Repeat steps 31-34 for Signal Generator and Oscilloscope’s Channel.