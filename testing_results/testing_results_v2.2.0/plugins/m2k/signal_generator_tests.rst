.. _m2k_signal_generator_tests:

Signal Generator - Test Suite
=======================================================

.. note::

    User guide: :ref:`Scopy Overview <user_guide>`.


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
-------------------------------------------------------

.. _m2k-usb-signal-generator:

**M2k.Usb:**
        - Open Scopy.
        - Connect an **ADALM2000** device to the system by USB.
        - Add the device in device browser.

Test 1: Channel 1 Operation
-------------------------------------------------------

**UID:** TST.M2K.SG.CHANNEL_1_OPERATION

**RBP:** P0

**Description:** This test case verifies the operation of the channel 1 of the signal generator.

**Preconditions:**
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-signal-generator>` setup.

**Steps:**
    1. Test the constant voltage generator
    2. Turn on channel 1 and view the configuration window by clicking the on/off button and menu button respectively. Choose Constant from the configuration menu.
    3. Checking increment/decrement value; ±1V
    4. Set the knob to ± 1V interval. No orange dot on the center.
    5. Change the voltage value using the up and down arrow.
            - **Expected Result:**
                - The voltage value should change accordingly with an increment or decrement of ±1V from -5V to 5V.
                - The graphical representation should follow accordingly.
            - **Actual Result:**

..
  Actual test result goes here.
..

    6. Checking increment/decrement value; ±100mV
    7. Set the knob to ± 100mV interval. With orange dot on the center.
    8. Change the voltage value using the up and down arrow.
            - **Expected Result:**
                - The voltage value should change accordingly with an increment or decrement of ±100mV from -5V to 5V.
                - The graphical representation should follow accordingly.
            - **Actual Result:**

..
  Actual test result goes here.
..

    9. Connect AWG ch1 to scope ch1+ and scope ch1- to gnd
    10. Set the following:
         - The voltage value of the signal generator to 4.5V
         - The Oscilloscope’s Volts/div from 1V/div to 5V/div and set the trigger mode to auto.
            - **Expected Result:** The voltage reading on the oscilloscope should be from 4.4V to 4.6V using the cursor or from the measured data.
            - **Actual Result:**

..
  Actual test result goes here.
..

    11. Set the following:
         - The voltage value of the signal generator to -4.5V
         - The Oscilloscope’s Volts/div from 1V/div to 5V/div and set the trigger mode to auto.
            - **Expected Result:** The voltage reading on the oscilloscope should be from -4.4V to -4.6V using the cursor or from the measured data.
            - **Actual Result:**

..
  Actual test result goes here.
..

    12. Testing different waveform types
    13. Turn on channel 1 and view the configuration window by clicking the on/off button and menu button respectively. Choose Waveform from the configuration menu
    14. Checking increment/decrement value of Amplitude and Frequency; Large increment
    15. Set the knob to without the orange dot on the center.
    16. Change the Amplitude or Frequency value using the up and down arrow
            - **Expected Result:**
                - The amplitude value should change accordingly with a high increment/decrement from 1uV to 10V.
                - The frequency value should change accordingly with a high increment/decrement from 1mHz to 20MHz.
                - The graphical representation should follow accordingly.
            - **Actual Result:**

..
  Actual test result goes here.
..

    17. Checking increment/decrement value of Amplitude and Frequency; ±1 unit on least significant digit
    18. Set the knob to with the orange dot on the center.
    19. Change the voltage or frequency value using the up and down arrow.
            - **Expected Result:**
                - The Amplitude value should change accordingly with a ±1 unit on the least significant digit from 1uV to 10V.
                - The frequency value should change accordingly with a ±1 unit on the least significant digit from 1mHz to 20MHz.
                - The graphical representation should follow accordingly.
            - **Actual Result:**

..
  Actual test result goes here.
..

    20. Checking increment/decrement value of the Offset Voltage and Phase; ±1V and ±45°
    21. Set the knob without the orange dot on the center.
    22. Change the Offset voltage or Phase value using the up and down arrow.
            - **Expected Result:**
                - The Offset voltage value should change accordingly with ±1 increment/decrement from -5V to 5V.
                - The phase value should change accordingly with a ±45 increment/decrement from 0° to 360°.
                - The graphical representation should follow accordingly.
            - **Actual Result:**

..
  Actual test result goes here.
..

    23. Checking increment/decrement value of Offset voltage and Phase; ±100mV and ±1°
    24. Set the knob to with the orange dot on the center.
    25. Change the Offset voltage or Phase value using the up and down arrow.
            - **Expected Result:**
                - The Offset voltage value should change accordingly with ±.1 increment/decrement from -5V to 5V.
                - The phase value should change accordingly with ±1 increment/decrement from 0° to 360°.
                - The graphical representation should follow accordingly.
            - **Actual Result:**

..
  Actual test result goes here.
..

    26. Testing Sinewave Waveform
    27. Connect AWG ch1 to scope ch1+ and scope ch1- to gnd.Set the following:
    28. Set the following:
         - Signal generator’s waveform: Sinewave, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees.
         - Tveform: Sinewave, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees.
         - Oscilloscope: Volts/div: 500mV/div, trigger mode: Auto and time base: 5ms.
            - **Expected Result:** The measurement reading on Oscilloscope should be: Period: 5ms, Frequency: 200Hz, Peak-peak: 4.8V to 5.2V.
            - **Actual Result:**

..
  Actual test result goes here.
..

    29. Set the following:
         - Signal generator’s waveform: Sinewave, Amplitude 10V, Frequency: 500kHz, offset: 0V and Phase: 0 degrees.
         - Oscilloscope: Volts/div: 2V/div, trigger mode: Auto and time base: 1us.
            - **Expected Result:** The measurement reading on Oscilloscope should be: Period: 2.000us, Frequency: 500 kHz, Peak-peak: 9.6V to 10.2V.
            - **Actual Result:**

..
  Actual test result goes here.
..

    30. Set the following:
         - Signal generator’s waveform: Sinewave, Amplitude 10V, Frequency: 5MHz, offset: 0V and Phase: 0 degrees.
         - Oscilloscope: Volts/div: 1V/div, trigger mode: Auto and time base: 200ns.
            - **Expected Result:** The measurement reading on Oscilloscope should be: Period: 200ns, Frequency: 5MHz, Peak-peak: 8.9V to 9.2V.
            - **Actual Result:**

..
  Actual test result goes here.
..

    31. Testing Square Waveform
    32. Set the following:
         - Signal generator’s waveform: Square wave, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees.
         - Oscilloscope: Volts/div: 1V/div, trigger mode: Auto and time base: 5ms.
            - **Expected Result:**
                - Use the Oscilloscope’s cursor to check the peak to peak value of the Square wave generated, do not include the inherent overshoot of the signal.
                - The measurement should be Period: 5.000ms, Frequency: 200Hz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.5V.
            - **Actual Result:**

..
  Actual test result goes here.
..

    33. Set the following:
         - Signal generator’s waveform: Square wave, Amplitude 10V, Frequency: 5MHz, offset: 0V and Phase: 0 degrees.
         - Oscilloscope: Volts/div: 2V/div, trigger mode: Auto and time base: 100ns.
            - **Expected Result:** Oscilloscope’s measurement should be Period: 200ns, Frequency: 5MHz, peak to peak value: 9.8V to 10.2V and Min/Max: ±5V
            - **Actual Result:**

..
  Actual test result goes here.
..

    34. Set the following:
         - Signal generator’s waveform: Square wave, Amplitude 7V, Duty Cycle: 20%, Frequency: 100 kHz, offset: 0V and Phase: 0 degrees.
         - Oscilloscope: Volts/div: 1V/div, trigger mode: Auto and time base: 2us.
            - **Expected Result:** Oscilloscope’s measurement should be Period: 200ns, Frequency: 5MHz, peak to peak value: 6.8V to 7.2V and Min/Max: ±3.5V, +Duty Cycle: 20%, -Duty Cycl: 80%
            - **Actual Result:**

..
  Actual test result goes here.
..

    35. Repeat step 34 with varying duty cycle from 1% to 99%
            - **Expected Result:** Oscilloscope’s measurement should be Period: 200ns, Frequency: 5MHz, peak to peak value: 6.8V to 7.2V and Min/Max: ±5V and the varying ±Duty Cycle.
            - **Actual Result:**

..
  Actual test result goes here.
..

    36. Testing Triangle Waveform
    37. Set the following:
         - Signal generator’s waveform: Triangle, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees.
         - Oscilloscope: Volts/div: 1V/div, trigger mode: Auto and time base: 2ms
            - **Expected Result:** Oscilloscope’s measurement should be Period: 5.000ms, Frequency: 200Hz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.4V to ±2.6V.
            - **Actual Result:**

..
  Actual test result goes here.
..

    38. Set the following:
         - Signal generator’s waveform: Triangle, Amplitude 8V, Frequency: 2MHz, offset: 0V and Phase: 0 degrees.
         - Oscilloscope: Volts/div: 1V/div, trigger mode: Auto and time base: 100ns
            - **Expected Result:** Oscilloscope’s measurement should be Period: 500.000ns, Frequency: 2MHz, peak to peak value: 7.8V to 8.2V and Min/Max: +/- 3.9V.
            - **Actual Result:**

..
  Actual test result goes here.
..

    39. Testing Rising Ramp Sawtooth Waveform
    40. Set the following:
         - Signal generator’s waveform: Rising Ramp Sawtooth, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees.
         - Oscilloscope: Volts/div: 1V/div, trigger mode: Auto and time base: 2ms.
            - **Expected Result:**
                - Oscilloscope’s measurement should be Period: 5.000ms, Frequency: 200Hz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.4V to ±2.7V.
                - Use the Oscilloscope’s cursor to disregard the overshoot of the signal.
            - **Actual Result:**

..
  Actual test result goes here.
..

    41. Set the following:
         - Signal generator’s waveform: Rising Ramp Sawtooth, Amplitude 8V, Frequency: 1MHz, offset: 0V and Phase: 0 degrees.
         - Oscilloscope: Volts/div: 1V/div, trigger mode: Auto and time base: 1us.
            - **Expected Result:**
                - Oscilloscope’s measurement should be Period: 1.000us, Frequency: 1MHz, peak to peak value: 7.8V to 8.2V and Min/Max: ±3.9V to ±4.1V.
                - Use the Oscilloscope’s cursor to disregard the overshoot of the signal.
            - **Actual Result:**

..
  Actual test result goes here.
..

    42. Testing Falling Ramp Sawtooth Waveform
    43. Set the following:
         - Signal generator’s waveform: Falling Ramp Sawtooth, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees.
         - Oscilloscope: Volts/div: 1V/div, trigger mode: Auto and time base: 2ms.
            - **Expected Result:** Oscilloscope’s measurement should be Period: 5.000ms, Frequency: 200Hz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.4V to ±2.6V
            - **Actual Result:**

..
  Actual test result goes here.
..

    44. Set the following:
         - Signal generator’s waveform: Falling Ramp Sawtooth, Amplitude 8V, Frequency: 1MHz, offset: 0V and Phase: 0 degrees.
         - Oscilloscope: Volts/div: 1V/div, trigger mode: Auto and time base: 1us.
            - **Expected Result:**
                - Oscilloscope’s measurement should be Period: 1.000us, Frequency: 1MHz, peak to peak value: 7.8V to 8.2V and Min/Max: ±3.9V to ±4.1V.
                - Use the Oscilloscope’s cursor to disregard the overshoot of the signal.
            - **Actual Result:**

..
  Actual test result goes here.
..

    45. Testing Trapezoidal waveform
    46. Set the following:
         - Signal generator’s waveform: Trapezoidal, Amplitude: 5V, Rise Time: 1us, Fall Time: 1us, Hold High Time: 1us, Hold Low time Time: 1us.
         - Oscilloscope: Volt/div: 2V, Trigger Mode: Auto and Time Base: 1us.
            - **Expected Result:** Oscilloscope’s measurement should be Period: 4.000us, Frequency: 250kHz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.4V to ±2.6.
            - **Actual Result:**

..
  Actual test result goes here.
..

    47. Set the following:
         - Signal generator’s waveform: Trapezoidal, Amplitude: 10V, Rise Time: 1us, Fall Time: 1us, Hold High Time: 1us, Hold Low time Time: 1us.
         - Oscilloscope: Volt/div: 2V, Trigger Mode: Auto and Time Base: 1us.
            - **Expected Result:** Oscilloscope’s measurement should be Period: 4.000us, Frequency: 250kHz, peak to peak value: 9.6V to 10.4V and Min/Max: ±4.8V to ±5.2.
            - **Actual Result:**

..
  Actual test result goes here.
..

    48. Set the following:
         - Signal generator’s waveform type: Trapezoidal, Amplitude: 10V, Rise Time: 200ns, Fall Time: 200ns, Hold High Time: 200ns, Hold Low time: 200ns.
         - Oscilloscope: Volt/div: 2V, Trigger Mode: Auto and Time Base: 200ns
            - **Expected Result:** Oscilloscope’s measurement should be Period: 800ns, Frequency: 1.250MHz, peak to peak value: 9.6V to 10.4V and Min/Max: ±4.8V to ±5.2.
            - **Actual Result:**

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


