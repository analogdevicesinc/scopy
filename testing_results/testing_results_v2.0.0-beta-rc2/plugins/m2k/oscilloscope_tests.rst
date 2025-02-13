.. _oscilloscope_tests:

Oscilloscope - Test Suite
=========================

.. note::

   User guide: :ref:`Oscilloscope user guide<oscilloscope>`

The following test cases are designed to verify the functionality of 
the Oscilloscope plugin.


.. note::
    .. list-table:: 
       :widths: 50 30 30 50 50
       :header-rows: 1

       * - Tester
         - Test Date
         - Scopy version
         - Plugin version (N/A if not applicable)
         - Comments
       * - 
         - 
         - 
         - 
         - 

Setup environment:
------------------

.. _adalm2000_osc_setup_ch1:

**Adalm2000.Osc.CH1:**
    - Open Scopy.
    - Connect the ADALM2000 device to the system.
    - Connect the ADALM2000 device in Scopy using the USB/network backend.
    - Open the Oscilloscope instrument and use only the CH1 settings and menus.

Depends on:
    - Test TST.PREFS.RESET

Prerequisites:
    - Scopy v2.0.0 or later with ADALM2000 plugin installed on the system.
    - Tests listed as dependencies are successfully completed.
    - Reset .ini files to default using the Preferences "Reset" button.
    - Connect **W1** to **1+** and **GND** to **1-** on the ADALM2000 device
      using loopback cables.
    - Connect **W2** to **2+** and **GND** to **2-** on the ADALM2000 device
      using loopback cables.

Test 1 - Time base and Volts/div Knobs
--------------------------------------

.. _TST.OSC.TIME_VOLTS_1:

**UID:** TST.OSC.TIME_VOLTS_1

**Description:** Check the increment/decrement operation of the time base and 
volts/div knobs.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the channel settings and set the knob to large increment 
       (NO orange dot in the center).
    2. Set the Time Base to 2us and the Volts/Div to 2 Volts.
    3. Use the - button 4 times and the + button 2 times on each knob:
        - **Expected result:**
            - The Time Base value changes to 1us, 500ns, 200ns, 100ns after the - button.
            - The Time Base value changes to 200ns and 500ns after the + button.
            - The Volts/Div value changes to 1V, 500, 200, 100 mV after the - button.
            - The Volts/Div value changes to 200mV and 500mV after the + button.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Set the knob to small increment (orange dot in the center).
       Set the Time Base to 2us and the Volts/Div to 2 Volts.
    5. Use the + and - button once on each knob:
        - **Expected result:**
            - The Time Base value changes to 3us and back to 2us.
            - The Volts/Div value changes to 3V and back to 2V.
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 2 - Position Knob
-----------------------

.. _TST.OSC.POSITION_1:

**UID:** TST.OSC.POSITION_1

**Description:** Check the increment/decrement operation of the position knob 
which depends on the value of the Time Base.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the channel settings and set the Position knob to large increment 
       (NO orange dot in the center).
    2. Set the Time Base to 2us and the horizontal position to 0.
    3. Use the + button 2 times and the - button 2 times on the horizontal Position knob:
        - **Expected result:**
            - The position changes to 200ns and 400ns after the + 
              and back to 200ns and 0 after the - button.
            - The plot handle position also changes accordingly.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Set the Time Base to 1us and the horizontal position to 0.
        - **Expected result:**
            - The position changes to 100ns and 200ns after the + button and 
              back to 100ns and 0 after the - button.
            - The plot handle position also changes accordingly.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Set the Position knob to small increment (orange dot in the center).
       Set the Time Base to 2us and the horizontal position to 0.
    6. Use the + button 2 times and the - button 2 times on the horizontal Position knob:
        - **Expected result:**
            - The position changes to 20ns and 40ns after the + button and 
              back to 20ns and 0 after the - button.
            - The plot handle position also changes accordingly.
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 3 - Constant Signal Check
-------------------------------

.. _TST.OSC.CONSTANT_SIGNAL_1:

**UID:** TST.OSC.CONSTANT_SIGNAL_1

**Description:** Check the constant signal from the signal generator on the 
oscilloscope using Channel 1.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator instrument and set the following config on CH1:
        - Constant signal, 3.3V
    2. In the Oscilloscope Trigger Settings, set the mode to **Auto**.
    3. In the Oscilloscope Measure Settings enable **Display All**.
    4. Run both instruments.
    5. Monitor the Oscilloscope RMS measurement:
        - **Expected result:**
            - The RMS reading is within 3.2V to 3.4V.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Change the value to 0V in the Signal Generator and monitor it on the Oscilloscope:
        - **Expected result:** 
            - The reading is within 4.9V to 5.1V.
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 4 - Sine Wave Signal Check
-------------------------------

.. _TST.OSC.SINE_WAVE_SIGNAL_1:

**UID:** TST.OSC.SINE_WAVE_SIGNAL_1

**Description:** Check the sine wave signal from the signal generator on the oscilloscope
using Channel 1.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator instrument and set the following config on CH1:
        - Sine wave, 2Vpp, 200Hz, 0V Offset, 0deg Phase
    2. In the Oscilloscope Trigger Settings, set the mode to **Auto** and 
       the following configuration in the general settings:
        
        - Horizontal: Time Base: 500us/div, Position: 0ms
        - Vertical: Volts/Div: 500mV/div, Position: 0V
    3. Run both instruments.
    4. Monitor the Oscilloscope measurements and plot:
        - **Expected result:**
            - The plot displays 1.5 periods.
            - The measurements display: Period: 5ms, Frequency: 200 Hz,
              Peak-peak: 1.9Vpp to 2.1Vpp, RMS: 0.6Vrms to 0.8Vrms.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Change the Signal Generator amplitude to 5V and the frequency to 500Hz:
        - **Expected result:**
            - The plot displays 4 periods.
            - The measurements display: Period: 2ms, Frequency: 500 Hz,
              Peak-peak: 4.9Vpp to 5.1Vpp, RMS: 1.74Vrms to 1.78Vrms.
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 5 - Square Wave Signal Check
---------------------------------

.. _TST.OSC.SQUARE_WAVE_SIGNAL_1:

**UID:** TST.OSC.SQUARE_WAVE_SIGNAL_1

**Description:** Check the square wave signal from the signal generator on 
the oscilloscope using Channel 1.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config on CH1:
        - Square wave, 5Vpp, 500Hz, 0V Offset, 0deg Phase
    2. In the Oscilloscope Trigger Settings, set the mode to **Auto** and
       the following configuration in the general settings:
        
        - Horizontal: Time Base: 500us/div, Position: 0ms
        - Vertical: Volts/Div: 1V/div, Position: 0V
    3. Run both instruments.
    4. Monitor the Oscilloscope measurements and plot:
        - **Expected result:**
            - The plot displays 4 square waves.
            - The measurements display: Period: 2ms, Frequency: 500 Hz,
              Amplitude: 4.9Vpp to 5.1Vpp, RMS: 2.4Vrms to 2.6Vrms.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Change the Signal Generator amplitude to 8V and the frequency to 2 kHz.
       Change the Oscilloscope Time Base to 200us/div:
        
        - **Expected result:**
            - The plot displays 6 square waves.
            - The measurements display: Period: 500us, Frequency: 2 kHz,
              Amplitude: 7.9Vpp to 8.1Vpp, RMS: 3.9Vrms to 4.1Vrms.
        
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 6 - Triangle Wave Signal Check
-----------------------------------

.. _TST.OSC.TRIANGLE_WAVE_SIGNAL_1:

**UID:** TST.OSC.TRIANGLE_WAVE_SIGNAL_1

**Description:** Check the triangle wave signal from the signal generator on 
the oscilloscope using Channel 1.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config on CH1:
        - Triangle wave, 4Vpp, 2kHz, 0V Offset, 0deg Phase
    2. In the Oscilloscope Trigger Settings, set the mode to **Auto** and
       the following configuration in the general settings:
        
        - Horizontal: Time Base: 200us/div, Position: 0ms
        - Vertical: Volts/Div: 1V/div, Position: 0V
    3. Run both instruments.
    4. Monitor the Oscilloscope measurements and plot:
        - **Expected result:**
            - The plot displays 6 triangle waves.
            - The measurements display: Period: 500us, Frequency: 2 kHz,
              Peak-peak: 3.9Vpp to 4.1Vpp, RMS: 1.0Vrms to 1.2Vrms.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Change the Signal Generator amplitude to 5V and the frequency to 20kHz.
       Change the Oscilloscope Time Base to 5us/dev.
        
        - **Expected result:**
            - The plot displays 6 triangle waves.
            - The measurements display: Period: 50us, Frequency: 20 kHz,
              Peak-peak: 4.9Vpp to 5.1Vpp, RMS: 1.3Vrms to 1.5Vrms.
        
        - **Actual result:**

..
  Actual test result goes here.
..

              
**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 7 - Rising/Falling Ramp Sawtooth Wave
------------------------------------------

.. _TST.OSC.RAMP_SAWTOOTH_WAVE_SIGNAL_1:

**UID:** TST.OSC.RAMP_SAWTOOTH_WAVE_SIGNAL_1

**Description:** Check the rising and falling ramp sawtooth wave signal from 
the signal generator on the oscilloscope using Channel 1.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config on CH1:
        - Rising Ramp Sawtooth, 8Vpp, 20kHz, 0V Offset, 0deg Phase.
    2. In the Oscilloscope Trigger Settings, set the mode to **Auto** and
       the following configuration in the general settings:
        
        - Horizontal: Time Base: 10us/div, Position: 0ms
        - Vertical: Volts/Div: 2V/div, Position: 0V
    3. Run both instruments.
    4. Monitor the Oscilloscope measurements and plot:
        - **Expected result:**
            - The plot displays 3 sawtooth waves.
            - The measurements display: Period: 50us, Frequency: 20 kHz,
              Peak-peak: 7.9Vpp to 8.1Vpp, RMS: 2.2Vrms to 2.4Vrms.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Change the Signal Generator configuration to Falling Ramp Sawtooth:
        - **Expected result:**
            - The plot displays 3 sawtooth waves.
            - The measurements display: Period: 50us, Frequency: 20 kHz,
              Peak-peak: 7.9Vpp to 8.1Vpp, RMS: 2.2Vrms to 2.4Vrms.
        - **Actual result:**

..
  Actual test result goes here.
..

              
**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 8 - Cursor Reading Check
------------------------------

.. _TST.OSC.CURSOR_READING_1:

**UID:** TST.OSC.CURSOR_READING_1

**Description:** Check the cursor reading value on the oscilloscope using 
Channel 1.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config on CH1:
        - Sine wave, 2Vpp, 200Hz, 0V Offset, 0deg Phase
    2. In the Oscilloscope Trigger Settings, set the mode to **Auto** and
       the following configuration in the general settings:
        
        - Horizontal: Time Base: 1ms/div, Position: 0ms
        - Vertical: Volts/Div: 500mV/div, Position: 0V
        - Enable **Cursors** and disable **Measure**.
    3. Run both instruments.
    4. Adjust the horizontal cursors to measure the period (place cursor
       T2 on the positive-going zero crossing point and T1 on the adjacent
       positive-going zero crossing point):
        
        - **Expected result:**
            - The frequency 1/ΔT is around 200Hz.
        
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Adjust the vertical cursors to measure the peak-peak amplitude:
       place cursor V1 on the crest and V2 on the bottom of the sine wave:
        
        - **Expected result:**
            - The peak-peak amplitude is around 2V.
        
        - **Actual result:**

..
  Actual test result goes here.
..

    6. In the Cursors Settings menu turn off the Horizontal cursors:
        - **Expected result:**
            - The horizontal cursors disappear from the plot as well as from the readouts.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. In the Cursors Settings menu turn off the Vertical cursors:
        - **Expected result:**
            - The vertical cursors disappear from the plot as well as from the readouts.
        - **Actual result:**

..
  Actual test result goes here.
..

              
**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).

    
Test 9 - Trigger Function Check
-------------------------------

.. _TST.OSC.TRIGGER_FUNCTION_1:

**UID:** TST.OSC.TRIGGER_FUNCTION_1

**Description:** Check the trigger function on the oscilloscope using Channel 1
with different trigger configurations.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config on CH1:
        - Triangle wave, 5Vpp, 200Hz
    2. In the Oscilloscope set the following:
        - Time Base to 1ms/div, Position to 0ms
        - Volts/Div to 1V/div, Position to 0V
    3. Open the Oscilloscope Trigger Settings and set the following configuration:
        - Trigger mode: Auto
        - Internal: ON
        - Source: channel 1
        - Level: 0, Hysteresis: 50mV
        - Condition: Rising Edge
    4. Run both instruments.
    5. Check the Oscilloscope plot:
        - **Expected result:**
            - The plot time handle is centered at the rising edge of the triangle wave.
            - The signal is static (not moving around at each triggered sample).
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Change the Trigger Condition to Falling Edge:
        - **Expected result:**
            - The plot time handle is centered at the falling edge of the triangle wave.
            - The signal is static (not moving around at each triggered sample).
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Set the Hysteresis value to 1.25V and Level to -1.7V:
        - **Expected result:**
            - The signal on the plot is not triggered and unstable.
            - The plot level is outside the triggered range of ~1.3V to +2.5V.
        - **Actual result:**

..
  Actual test result goes here.
..

    8. Set the Hysteresis value to 1.25V and Level to -1.2V:
        - **Expected result:**
            - The signal on the plot is triggered and stable.
            - The plot level is in the triggered range of ~1.3V to +2.5V.
        - **Actual result:**

..
  Actual test result goes here.
..

    9. Set the Hysteresis value to 2.5V and Level to -2.5V:
        - **Expected result:**
            - The signal on the plot is not triggered and unstable.
            - The plot level is outside the triggered range of 0V to +2.5V.
        - **Actual result:**

..
  Actual test result goes here.
..

    10. Set the Hysteresis value to 2.5V and Level to 0.1V:
         - **Expected result:**
            - The signal on the plot is triggered and stable.
            - The plot level is in the triggered range of 0V to +2.5V.
         - **Actual result:**

..
  Actual test result goes here.
..

    11. Set the Hysteresis value to 2.5V and Level to 3V:
         - **Expected result:**
            - The signal on the plot is not triggered and unstable.
            - The plot level is outside the triggered range of 0V to +2.5V.
         - **Actual result:**

..
  Actual test result goes here.
..

              
**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 10 - Math Channel Operations
---------------------------------

.. _TST.OSC.MATH_CHANNEL:

**UID:** TST.OSC.MATH_CHANNEL

**Description:** Check the math channel operations on the oscilloscope using Channel 1.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config:
        - Channel 1: Sine wave, 5Vpp, 500Hz
        - Channel 2: Square wave, 2Vpp, 500Hz
    2. In the Oscilloscope set the following:
        - Time Base to 500us/div, Position to 0ms
        - Volts/Div to 1V/div, Position to 0V
        - Trigger mode: Auto
    3. Run both instrument.
    4. Add a Math Channel (using the + button beside Channel 2) with the following function:
        - *sqrt(t0*t0)*
        - **Expected result:**
            - The plot contains a new Channel having all the samples from Channel 1.
        - **Actual result:**

..
  Actual test result goes here.
..

    5.  Add a new Math Channel with the following function:
         - *2\*(t1+t1)*
         - **Expected result:**
            - The plot contains a new Channel having the amplitude of 
              Channel 2 increased 4 times.
         - **Actual result:**

..
  Actual test result goes here.
..

    6. Change the Signal Generator configuration to:
        - Channel 1: Square wave, 5Vpp, 200Hz
        - Channel 2: Sine wave, 3Vpp, 200Hz
    7. Add a new Math channel with the following function and verify the measurements:
        - *t0+t1*
        - **Expected result:**
            - The plot contains a new Channel having the sum of Channel 1 and Channel 2.
            - Math channel measurement: Vpp: 8V, Period: 5ms, Frequency: 200Hz.
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 11 - FFT Function
-----------------------

.. _TST.OSC.FFT_FUNCTION:

**UID:** TST.OSC.FFT_FUNCTION

**Description:** Check the FFT function on the oscilloscope.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config:
        - Channel 1: Square wave, 5Vpp, 1kHz
    2. In the Oscilloscope set the following:
        - Channel1 Horizontal: Time Base: 5ms/div, Position: 0ms
        - Channel1 Vertical: Volts/Div: 1V/div, Position: 0V
    3. Run the Oscilloscope and verify the plot:
        - **Expected result:** The resulting spectrum shows 
          a series of peaks at the fundamental frequency and its harmonics.
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 12 - XY Function
----------------------

.. _TST.OSC.XY_FUNCTION:

**UID:** TST.OSC.XY_FUNCTION

**Description:** Check the XY function on the oscilloscope.
The plot displays the current vs voltage characteristics of a PN junction diode.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config:
        - Channel 1: Sine wave, 4Vpp, 100Hz, 3V Offset
    2. In the Oscilloscope set the following:
        - General Settings: XY (View): ON
        - Channel 1 on the X Axis and Channel 2 on the Y Axis.
    3. Run both instruments.
        - **Expected result:** The plot displays a horizontal line on level 
          0 of the Y-Axis and in range 1 to 5 on the X-Axis.

        - **Actual result:**

..
  Actual test result goes here.
..

    4. Change the Signal Generator offset to 2V:
        - **Expected result:** The plot displays a horizontal line on level 
          0 of the Y-Axis and in range 0 to 4 on the X-Axis.

        - **Actual result:**

..
  Actual test result goes here.
..

    5. Set the X-Y configuration to CH1 on both X-Axis and Y-Axis:
        - **Expected result:** The plot displays a diagonal line 
          in the range 0 to 4 on both axes.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Set the X-Y configuration to CH2 on X-Axis and CH1 on Y-Axis:
        - **Expected result:** The plot displays a vertical line 
          from 0 to 4 on the Y-Axis and 0 on the X-Axis.
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
        

Test 13 - Export feature
------------------------

.. _TST.OSC.EXPORT_FEATURE:

**UID:** TST.OSC.EXPORT_FEATURE

**Description:** Check the data export feature on the oscilloscope.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps**:
    1. Open the Signal Generator and set the following config:
        - Channel 1: Sine wave, 2Vpp, 200Hz
        - Channel 2: Square wave, 5Vpp, 500Hz
    2. In the Oscilloscope set the following:
        - Channel 1 Horizontal: Time Base: 1ms/div, Position: 0ms
        - Channel 1 Vertical: Volts/Div: 1V/div, Position: 0V
    3. Run both instruments.
    4. In the Oscilloscope, open the General Settings Menu:
        - Turn off "Export All".
        - From the dropdown list only choose Channel 1.
    5. Click **Export** and choose a name, the CSV format and location to save the file.
        - **Expected result:** The file is saved successfully at the 
          specified location.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Open the file and verify the data.
        - **Expected result:**
            - The first 7 lines of the CSV contain metadata such as: timestamp, 
              device name, number of samples, sample rate, instrument name.
            - The file contains 3 columns: sample number, time, voltage.
            - The data matches with that samples displayed on the plot.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. In the Export menu, turn on "Export All".
    8. Click **Export** and choose a name, the CSV format and location to save the file.
        - **Expected result:** The file is saved successfully at the 
          specified location.
        - **Actual result:**

..
  Actual test result goes here.
..

    9. Open the file and verify the data.
        - **Expected result:**
            - The first 7 lines of the CSV contain metadata such as: timestamp, 
              device name, number of samples, sample rate, instrument name.
            - The file contains 4 columns: sample number, time, voltage1, voltage2.
            - The data matches with that samples displayed on the plot.
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 14 - Software AC coupling
-----------------------------------------

.. _TST.OSC.SOFTWARE_AC_COUPLING:

**UID:** TST.OSC.SOFTWARE_AC_COUPLING

**Description:** Check the software AC coupling feature on the oscilloscope.
The Signal Generator will output a sine wave with a 3V DC offset and 
the Oscilloscope should be able to center the trace at 0V.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config:
        - Channel 1: Sine wave, 2Vpp, 1kHz, 3V Offset
    2. In the Oscilloscope set the following:
        - Channel 1 Horizontal: Time Base: 200us/div, Position: 0ms
        - Channel 1 Vertical: Volts/Div: 1V/div, Position: 0V
    3. Run both instruments.
    4. In the Oscilloscope Channel 1 Settings turn on Software AC Coupling.
        - **Expected result:**
            - The trace moves towards 0V until it is centered at level 0V.
            - The measurement Mean is around 0V.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Turn off Software AC Coupling.
        - **Expected result:**
            - The trace moves back to the original position.
            - The measurement Mean is around 0V.
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 15 - Probe Attenuation
---------------------------

.. _TST.OSC.PROBE_ATTENUATION:

**UID:** TST.OSC.PROBE_ATTENUATION

**Description:** Check the probe attenuation feature on the oscilloscope.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config:
        - Channel 1: Sine wave, 2Vpp, 1kHz
    2. In the Oscilloscope set the following:
        - Channel 1 Horizontal: Time Base: 200us/div, Position: 0ms
        - Channel 1 Vertical: Volts/Div: 1V/div, Position: 0V
    3. Enable the Measurement feature and turn on Display All.
    4. Run both instruments.
    5. In the Channel 1 settings of the Oscilloscope set Probe Attenuation to 0.1.
        - **Expected result:**
            - The measurement Vpp is 200mV.
            - The Volts/Div is 100mV.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Change the Probe Attenuation to 100.
        - **Expected result:**
            - The measurement Vpp is 200V.
            - The Volts/Div is 100V.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Change the Probe Attenuation to 1.
        - **Expected result:**
            - The measurement Vpp is 2V.
            - The Volts/Div is 1V.
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 16 - External Trigger
--------------------------

.. _TST.OSC.EXTERNAL_TRIGGER:

**UID:** TST.OSC.EXTERNAL_TRIGGER

**Description:** Check the external trigger feature on the oscilloscope 
using the TI pin and the digital DIO1 pin of the ADALM2000 device.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - Disconnect 1+ and 1- from any loopback cables.
    - Connect 2+ to W2 and 2- to GND using loopback cables.
    - Connect TI to DIO0 using loopback cables.
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config:
        - Channel 2: Sine wave, 5Vpp, 5kHz
    2. Open the Pattern Generator and set the following config:
        - DIO0: Clock, 5kHz
    3. In the Oscilloscope set the Channel 2 TimeBase to 500us.
    4. In the Oscilloscope Trigger settings set the following:
        - Mode: normal
        - Internal: OFF
        - Digital: ON
        - Source: External Trigger In
        - Condition: Rising Edge
    5. Run all instruments:
        - **Expected result:** The 5Vpp sinewave is displayed on CH2 
          of the Osc plot.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Stop the Pattern Generator:
        - **Expected result:** The plot stops updating and the status 
          displays "Waiting".
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Stop the Pattern Generator.
    8. Open the Logic Analyzer and set a Rising Edge trigger on DIO1.
    9. In the Oscilloscope Trigger settings set the following:
        - Mode: normal
        - Internal: OFF
        - Digital: ON
        - Source: Logic Analyzer
        - **Expected result:**
            - The plot is not trigger and the status displays "Waiting".
        - **Actual result:**

..
  Actual test result goes here.
..

    10. Open the DigitalIO, detach the instrument and run it.
    11. Manually toggle DIO1 from output to input and viceversa:
         - **Expected result:**
            - The Osc plot is triggered when toggling the digital pin.
         - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 17 - Autoset
-----------------

.. _TST.OSC.AUTOSET:

**UID:** TST.OSC.AUTOSET

**Description:** Check the Autoset feature on the oscilloscope.
A sinewave is fed by the Signal Generator, captured with the Oscilloscope 
using a configuration that displays too many periods on the plot.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config:
        - Channel 1: Sine wave, 5Vpp, 20kHz
    2. In the Oscilloscope set the following:
        - Channel 1 Horizontal: Time Base: 200us/div, Position: -200us
        - Channel 1 Vertical: Volts/Div: 500mV/div, Position: 2.5V
        - In the Trigger Settings set the analog level to 5V.
        - In the Trigger Settings set the trigger mode to Auto.
    3. Open the Channel 1 settings menu and click Autoset while running both instruments:
        - **Expected result:**
            - The plot displays less periods of the sinewave.
            - The vertical Volts/div are adjusted to 1V.
            - The horizontal Time Base is adjusted to 50us/div.
            - The horizontal and vertical positions are at 0.
            - The trigger level is at 0V.
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 18 - Print Plot
---------------------

.. _TST.OSC.PRINT_PLOT:

**UID:** TST.OSC.PRINT_PLOT

**Description:** Check the Print feature on the oscilloscope.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config:
        - Channel 1: Sine wave, 5Vpp, 20kHz
    2. In the Oscilloscope set the following:
        - Channel 1 Horizontal: Time Base: 200us/div, Position: 0us
        - Channel 1 Vertical: Volts/Div: 500mV/div, Position: 0V
    3. Run both instruments.
    4. Click the Print button and choose a name and location for the PDF file:
        - **Expected result:**
            - The file is saved successfully at the specified location.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Open the file and verify the data.
        - **Expected result:**
            - The file contains a screenshot of the instrument with inverted colors.
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 19 - Curve style
----------------------

.. _TST.OSC.CURVE_STYLE:

**UID:** TST.OSC.CURVE_STYLE

**Description:** Check the curve style feature on the oscilloscope 
and plot the signal using lines, dots, sticks or steps.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config:
        - Channel 1: Sine wave, 5Vpp, 5kHz
    2. In the Oscilloscope set the following:
        - Channel 1 Horizontal: Time Base: 10us/div, Position: 0us
        - Channel 1 Vertical: Volts/Div: 1V/div, Position: 0V
    3. Run both instruments.
    4. Open the Channel 1 settings menu and set the curve style to Dots:
        - **Expected result:**
            - The plot is displayed using dots (when zoomed in to samples).
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Set the curve style to Sticks:
        - **Expected result:**
            - The plot is displayed using lines (the sinewave looks "full").
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Set the curve style to Smooth:
        - **Expected result:**
            - The plot is displayed using a smoother lines.
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 20 - Gating
-------------------------

.. _TST.OSC.GATING:

**UID:** TST.OSC.GATING

**Description:** Check the Gating feature on the oscilloscope.
Apply the measurements on a gated portion of the acquired signal.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config:
        - Channel 1: Sine wave, 5Vpp, 10kHz
    2. In the Oscilloscope set the following:
        - Channel 1 Horizontal: Time Base: 10us/div, Position: 0us
        - Channel 1 Vertical: Volts/Div: 1V/div, Position: 0V
    3. Enable Measurements, turon on Display All.
    4. Enable Gating and set the sliders to two consecutive zero-crossing points
       of the sine wave:
        
        - **Expected result:**
            - The measurement Vpp is half the set amplitude: 2.5V.
        
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Move the sliders to the left and right ends of the plot:
        - **Expected result:**
            - The measurement Vpp is the set amplitude: 5V.
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 21 - Histogram
--------------------

.. _TST.OSC.HISTOGRAM:

**UID:** TST.OSC.HISTOGRAM

**Description:** Check the Histogram feature on the oscilloscope.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the Signal Generator and set the following config:
        - Channel 1: Square wave, 5Vpp, 10kHz
    2. In the Oscilloscope set the following:
        - Channel 1 Horizontal: Time Base: 10us/div, Position: 0ms
        - Channel 1 Vertical: Volts/Div: 1V/div, Position: 0V
    3. Run both instruments.
    4. In the Oscilloscope General settings menu enable the Histogram:
        - **Expected result:**
            - A histogram is displayed above the time plot.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Change the Signal Generator to output a Square wave:
        - **Expected result:**
            - The histogram shows the min and max of the square wave.
        - **Actual result:**

..
  Actual test result goes here.
..


**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 22 - ADC Digital Filters
-------------------------------

.. _TST.OSC.ADC_DIGITAL_FILTERS:

**UID:** TST.OSC.ADC_DIGITAL_FILTERS

**Description:** Check the ADC digital Filter calibration.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Resources:**
    - `ADC Digital Filters documentation<https://wiki.analog.com/university/tools/m2k/scopy/adcdigitalfilters>``

**Steps:**
    1. Open the Signal Generator and set the following config on CH1:
        - Square wave, 2Vpp, 1kHz
    2. In the Oscilloscope set the following:
        - Channel 1 Horizontal: Time Base: 200ms/div, Position: 0ms
        - Channel 1 Vertical: Volts/Div: 500mV/div, Position: 0V
    3. TBD

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).

