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
       * - Ionut Muthi
         - 12/02/2025
         - v2.0.0
         - N/A
         - none

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

**Description:** This test case verifies the operation of the channel 1 of the signal generator.

**Preconditions:**
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-signal-generator>` setup.

**Steps:**
        1. Test the constant voltage generator
        2. Turn on channel 1 and view the configuration window by clicking the on/off button and menu button respectively. Choose Constant from the configuration menu.
        3. Checking increment/decrement value; ±1V
        4. Set the knob to ± 1V interval. No orange dot on the center.
        5. Change the voltage value using the up and down arrow
                    - **Expected Result:** The voltage value should change accordingly with an increment or decrement of ±1V from -5V to 5V. The graphical representation should follow accordingly
                    - **Actual Result:** The voltage value changes accordingly with an increment or decrement of ±1V from -5V to 5V. The graphical representation follows accordingly

..
  Actual test result goes here.
..

        6. Checking increment/decrement value; ±100mV
        7. Set the knob to ± 100mV interval. With orange dot on the center.
        8. Change the voltage value using the up and down arrow
                    - **Expected Result:** The voltage value should change accordingly with an increment or decrement of ±100mV from -5V to 5V. The graphical representation should follow accordingly
                    - **Actual Result:** The voltage value changes accordingly with an increment or decrement of ±100mV from -5V to 5V. The graphical representation follows accordingly

..
  Actual test result goes here.
..

        9. Connect AWG ch1 to scope ch1+ and scope ch1- to gnd
        10. Set the voltage value of the signal generator to 4.5V and set the Oscilloscope’s Volts/div from 1V/div to 5V/div and set the trigger mode to auto.
                    - **Expected Result:** The voltage reading on the oscilloscope should be from 4.4V to 4.6V using the cursor or from the measured data
                    - **Actual Result:** The voltage reading on the oscilloscope is from 4.4V to 4.6V using the cursor or from the measured data

..
  Actual test result goes here.
..

        11. Set the voltage value of the signal generator to -4.5V and set the Oscilloscope’s Volts/div from 1V/div to 5V/div and set the trigger mode to auto.
                    - **Expected Result:** The voltage reading on the oscilloscope should be from -4.4V to -4.6V using the cursor or from the measured data
                    - **Actual Result:** The voltage reading on the oscilloscope is from -4.4V to -4.6V using the cursor. 

..
  Actual test result goes here.
..

        1.  Testing different waveform types
        2.  Turn on channel 1 and view the configuration window by clicking the on/off button and menu button respectively. Choose Waveform from the configuration menu
        3.  Checking increment/decrement value of Amplitude and Frequency; Large increment
        4.  Set the knob to without the orange dot on the center.
        5.  Change the Amplitude or Frequency value using the up and down arrow
                    - **Expected Result:** The amplitude value should change accordingly with a high increment/decrement from 1uV to 10V. The frequency value should change accordingly with a high increment/decrement from 1mHz to 20MHz.The graphical representation should follow accordingly
                    - **Actual Result:** The amplitude value changes accordingly with a high increment/decrement from 1uV to 10V. The frequency value changes accordingly with a high increment/decrement from 1mHz to 20MHz.The graphical representation follows accordingly

..
  Actual test result goes here.
..

        17. Checking increment/decrement value of Amplitude and Frequency; ±1 unit on least significant digit
        18. Set the knob to with the orange dot on the center.
        19. Change the voltage or frequency value using the up and down arrow
                    - **Expected Result:** The Amplitude value should change accordingly with a ±1 unit on the least significant digit from 1uV to 10V. The frequency value should change accordingly with a ±1 unit on the least significant digit from 1mHz to 20MHz.The graphical representation should follow accordingly
                    - **Actual Result:** The Amplitude value changes accordingly with a ±1 unit on the least significant digit from 1uV to 10V. The frequency value changes accordingly with a ±1 unit on the least significant digit from 1mHz to 20MHz.The graphical representation follows accordingly 

..
  Actual test result goes here.
..

        20. Checking increment/decrement value of the Offset Voltage and Phase; ±1V and ±45°
        21. Set the knob without the orange dot on the center.
        22. Change the Offset voltage or Phase value using the up and down arrow
                    - **Expected Result:** The Offset voltage value should change accordingly with ±1 increment/decrement from -5V to 5V. The phase value should change accordingly with a ±45 increment/decrement from 0° to 360°.The graphical representation should follow accordingly
                    - **Actual Result:** The Offset voltage value changes accordingly with ±1 increment/decrement from -5V to 5V. The phase value changes accordingly with a ±45 increment/decrement from 0° to 360°.The graphical representation follows accordingly

..
  Actual test result goes here.
..

        23. Checking increment/decrement value of Offset voltage and Phase; ±100mV and ±1°
        24. Set the knob to with the orange dot on the center.
        25. Change the Offset voltage or Phase value using the up and down arrow
                    - **Expected Result:** The Offset voltage value should change accordingly with ±.1 increment/decrement from -5V to 5V. The phase value should change accordingly with ±1 increment/decrement from 0° to 360°.The graphical representation should follow accordingly
                    - **Actual Result:** The Offset voltage value changes accordingly with ±.1 increment/decrement from -5V to 5V. The phase value changes accordingly with ±1 increment/decrement from 0° to 360°.The graphical representation follows accordingly

..
  Actual test result goes here.
..

        26. Connect AWG ch1 to scope ch1+ and scope ch1- to gnd
        27. Testing Sinewave Waveform
        28. Set the signal generator’s waveform type: Sinewave, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 500mV/div, trigger mode: Auto and time base: 5ms.
                    - **Expected Result:** The measurement reading on Oscilloscope should be: Period: 5ms, Frequency: 200Hz, Peak-peak: 4.8V to 5.2V
                    - **Actual Result:** The measurement reading on Oscilloscope is: Period: 5ms, Frequency: 200Hz, Peak-peak: 4.8V to 5.2V

..
  Actual test result goes here.
..

        29. Set the signal generator’s waveform type: Sinewave, Amplitude 10V, Frequency: 500kHz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 2V/div, trigger mode: Auto and time base: 1us
                    - **Expected Result:** The measurement reading on Oscilloscope should be: Period: 2.000us, Frequency: 500 kHz, Peak-peak: 9.6V to 10.2V
                    - **Actual Result:** The measurement reading on Oscilloscope is: Period: 2.000us, Frequency: 500 kHz, Peak-peak: 9.6V to 10.2V

..
  Actual test result goes here.
..

        30. Set the signal generator’s waveform type: Sinewave, Amplitude 10V, Frequency: 5MHz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 200ns
                    - **Expected Result:** The measurement reading on Oscilloscope should be: Period: 200ns, Frequency: 5MHz, Peak-peak: 8.9V to 9.2V
                    - **Actual Result:** The measurement reading on Oscilloscope is: Period: 200ns, Frequency: 5MHz, Peak-peak: 8.9V to 9.2V

..
  Actual test result goes here.
..

        31. Testing Square Waveform
        32. Set the signal generator’s waveform type: Square wave, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 5ms
                    - **Expected Result:** Use the Oscilloscope’s cursor to check the peak to peak value of the Square wave generated, do not include the inherent overshoot of the signal. The measurement should be Period: 5.000ms, Frequency: 200Hz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.5V
                    - **Actual Result:** The measurement is: Period: 5.000ms, Frequency: 200Hz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.5V

..
  Actual test result goes here.
..

        33. Set the signal generator’s waveform type: Square wave, Amplitude 10V, Frequency: 5MHz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 2V/div, trigger mode: Auto and time base: 100ns
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 200ns, Frequency: 5MHz, peak to peak value: 9.8V to 10.2V and Min/Max: ±5V
                    - **Actual Result:** Oscilloscope’s measurement is: Period: 200ns, Frequency: 5MHz, peak to peak value: 9.8V to 10.2V and Min/Max: ±5V

..
  Actual test result goes here.
..

        34. Set the signal generator’s waveform type: Square wave, Amplitude 7V, Duty Cycle: 20%, Frequency: 100 kHz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 2us
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 200ns, Frequency: 5MHz, peak to peak value: 6.8V to 7.2V and Min/Max: ±3.5V, +Duty Cycle: 20%, -Duty Cycl: 80%
                    - **Actual Result:** Oscilloscope’s measurement is: Period: 200ns, Frequency: 5MHz, peak to peak value: 6.8V to 7.2V and Min/Max: ±3.5V, +Duty Cycle: 20%, -Duty Cycl: 80%

..
  Actual test result goes here.
..

        35. Repeat step 10.3 with varying duty cycle from 1% to 99%
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 200ns, Frequency: 5MHz, peak to peak value: 6.8V to 7.2V and Min/Max: ±5V and the varying ±Duty Cycle
                    - **Actual Result:** The value are displayed correctly

..
  Actual test result goes here.
..

        36. Testing Triangle Waveform
        37. Set the signal generator’s waveform type: Triangle, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 2ms
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 5.000ms, Frequency: 200Hz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.4V to ±2.6V
                    - **Actual Result:** The value are displayed correctly

..
  Actual test result goes here.
..

        38. Set the signal generator’s waveform type: Triangle, Amplitude 8V, Frequency: 2MHz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 100ns
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 500.000ns, Frequency: 2MHz, peak to peak value: 7.8V to 8.2V and Min/Max: +/- 3.9V
                    - **Actual Result:** The value are displayed correctly

..
  Actual test result goes here.
..

        39. Testing Rising Ramp Sawtooth Waveform
        40. Set the signal generator’s waveform type: Rising Ramp Sawtooth, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 2ms
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 5.000ms, Frequency: 200Hz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.4V to ±2.7V. Use the Oscilloscope’s cursor to disregard the overshoot of the signal
                    - **Actual Result:** The value are displayed correctly

..
  Actual test result goes here.
..

        41. Set the signal generator’s waveform type: Rising Ramp Sawtooth, Amplitude 8V, Frequency: 1MHz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 1us
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 1.000us, Frequency: 1MHz, peak to peak value: 7.8V to 8.2V and Min/Max: ±3.9V to ±4.1V. Use the Oscilloscope’s cursor to disregard the overshoot of the signal
                    - **Actual Result:** The value are displayed correctly

..
  Actual test result goes here.
..

        42. Testing Falling Ramp Sawtooth Waveform
        43. Set the signal generator’s waveform type: Falling Ramp Sawtooth, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 2ms
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 5.000ms, Frequency: 200Hz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.4V to ±2.6V
                    - **Actual Result:** The value are displayed correctly

..
  Actual test result goes here.
..

        44. Set the signal generator’s waveform type: Falling Ramp Sawtooth, Amplitude 8V, Frequency: 1MHz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 1us
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 1.000us, Frequency: 1MHz, peak to peak value: 7.8V to 8.2V and Min/Max: ±3.9V to ±4.1V. Use the Oscilloscope’s cursor to disregard the overshoot of the signal
                    - **Actual Result:** The value are displayed correctly

..
  Actual test result goes here.
..

        45. Testing Trapezoidal waveform
        46. Set the signal generator’s waveform type: Trapezoidal, Amplitude: 5V, Rise Time: 1us, Fall Time: 1us, Hold High Time: 1us, Hold Low time Time: 1us. Set the Oscilloscope’s Volt/div: 2V, Trigger Mode: Auto and Time Base: 1us
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 4.000us, Frequency: 250kHz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.4V to ±2.6.
                    - **Actual Result:** The value are displayed correctly

..
  Actual test result goes here.
..

        47. Set the signal generator’s waveform type: Trapezoidal, Amplitude: 10V, Rise Time: 1us, Fall Time: 1us, Hold High Time: 1us, Hold Low time Time: 1us. Set the Oscilloscope’s Volt/div: 2V, Trigger Mode: Auto and Time Base: 1us
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 4.000us, Frequency: 250kHz, peak to peak value: 9.6V to 10.4V and Min/Max: ±4.8V to ±5.2.
                    - **Actual Result:** The value are displayed correctly

..
  Actual test result goes here.
..

        48. Set the signal generator’s waveform type: Trapezoidal, Amplitude: 10V, Rise Time: 200ns, Fall Time: 200ns, Hold High Time: 200ns, Hold Low time: 200ns. Set the Oscilloscope’s Volt/div: 2V, Trigger Mode: Auto and Time Base: 200ns
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 800ns, Frequency: 1.250MHz, peak to peak value: 9.6V to 10.4V and Min/Max: ±4.8V to ±5.2.
                    - **Actual Result:** The value are displayed correctly

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:** none 

..
  Any comments about the test goes here.

**Result:** PASS

..
  The result of the test goes here (PASS/FAIL).


Test 2: Channel 2 Operation
-------------------------------------------------------

**UID:** TST.M2K.SG.CHANNEL_2_OPERATION

**Description:** This test case verifies the operation of the channel 2 of the signal generator.

**Preconditions:**
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-signal-generator>` setup.

**Steps:**
        1. Test the constant voltage generator
        2. Turn on channel 2 and view the configuration window by clicking the on/off button and menu button respectively. Choose Constant from the configuration menu.
        3. Checking increment/decrement value; ±1V
        4. Set the knob to ± 1V interval. No orange dot on the center.
        5. Change the voltage value using the up and down arrow
                    - **Expected Result:** The voltage value should change accordingly with an increment or decrement of ±1V from -5V to 5V. The graphical representation should follow accordingly
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        6. Checking increment/decrement value; ±100mV
        7. Set the knob to ± 100mV interval. With orange dot on the center.
        8. Change the voltage value using the up and down arrow
                    - **Expected Result:** The voltage value should change accordingly with an increment or decrement of ±100mV from -5V to 5V. The graphical representation should follow accordingly
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        9. Connect AWG ch2 to scope ch2+ and scope ch2- to gnd
        10. Set the voltage value of the signal generator to 4.5V and set the Oscilloscope’s Volts/div from 1V/div to 5V/div and set the trigger mode to auto.
                    - **Expected Result:** The voltage reading on the oscilloscope should be from 4.4V to 4.6V using the cursor or from the measured data
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        11. Set the voltage value of the signal generator to -4.5V and set the Oscilloscope’s Volts/div from 1V/div to 5V/div and set the trigger mode to auto.
                    - **Expected Result:** The voltage reading on the oscilloscope should be from -4.4V to -4.6V using the cursor or from the measured data
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        12. Testing different waveform types
        13. Turn on channel 2 and view the configuration window by clicking the on/off button and menu button respectively. Choose Waveform from the configuration menu
        14. Checking increment/decrement value of Amplitude and Frequency; Large increment
        15. Set the knob to without the orange dot on the center.
        16. Change the Amplitude or Frequency value using the up and down arrow
                    - **Expected Result:** The amplitude value should change accordingly with a high increment/decrement from 1uV to 10V. The frequency value should change accordingly with a high increment/decrement from 1mHz to 20MHz.The graphical representation should follow accordingly
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        17. Checking increment/decrement value of Amplitude and Frequency; ±1 unit on least significant digit
        18. Set the knob to with the orange dot on the center.
        19. Change the voltage or frequency value using the up and down arrow
                    - **Expected Result:** The Amplitude value should change accordingly with a ±1 unit on the least significant digit from 1uV to 10V. The frequency value should change accordingly with a ±1 unit on the least significant digit from 1mHz to 20MHz.The graphical representation should follow accordingly
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        20. Checking increment/decrement value of the Offset Voltage and Phase; ±1V and ±45°
        21. Set the knob without the orange dot on the center.
        22. Change the Offset voltage or Phase value using the up and down arrow
                    - **Expected Result:** The Offset voltage value should change accordingly with ±1 increment/decrement from -5V to 5V. The phase value should change accordingly with a ±45 increment/decrement from 0° to 360°.The graphical representation should follow accordingly
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        23. Checking increment/decrement value of Offset voltage and Phase; ±100mV and ±1°
        24. Set the knob to with the orange dot on the center.
        25. Change the Offset voltage or Phase value using the up and down arrow
                    - **Expected Result:** The Offset voltage value should change accordingly with ±.1 increment/decrement from -5V to 5V. The phase value should change accordingly with ±1 increment/decrement from 0° to 360°.The graphical representation should follow accordingly
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        26. Connect AWG ch2 to scope ch2+ and scope ch2- to gnd
        27. Testing Sinewave Waveform
        28. Set the signal generator’s waveform type: Sinewave, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 500mV/div, trigger mode: Auto and time base: 5ms.
                    - **Expected Result:** The measurement reading on Oscilloscope should be: Period: 5ms, Frequency: 200Hz, Peak-peak: 4.8V to 5.2V
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        29. Set the signal generator’s waveform type: Sinewave, Amplitude 10V, Frequency: 500kHz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 2V/div, trigger mode: Auto and time base: 1us
                    - **Expected Result:** The measurement reading on Oscilloscope should be: Period: 2.000us, Frequency: 500 kHz, Peak-peak: 9.6V to 10.2V
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        30. Set the signal generator’s waveform type: Sinewave, Amplitude 10V, Frequency: 5MHz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 200ns
                    - **Expected Result:** The measurement reading on Oscilloscope should be: Period: 200ns, Frequency: 5MHz, Peak-peak: 8.9V to 9.2V
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        31. Testing Square Waveform
        32. Set the signal generator’s waveform type: Square wave, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 5ms
                    - **Expected Result:** Use the Oscilloscope’s cursor to check the peak to peak value of the Square wave generated, do not include the inherent overshoot of the signal. The measurement should be Period: 5.000ms, Frequency: 200Hz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.5V
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        33. Set the signal generator’s waveform type: Square wave, Amplitude 10V, Frequency: 5MHz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 2V/div, trigger mode: Auto and time base: 100ns
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 200ns, Frequency: 5MHz, peak to peak value: 9.8V to 10.2V and Min/Max: ±5V
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        34. Set the signal generator’s waveform type: Square wave, Amplitude 7V, Duty Cycle: 20%, Frequency: 100 kHz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 2us
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 200ns, Frequency: 5MHz, peak to peak value: 6.8V to 7.2V and Min/Max: ±3.5V, +Duty Cycle: 20%, -Duty Cycl: 80%
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        35. Repeat step 10.3 with varying duty cycle from 1% to 99%
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 200ns, Frequency: 5MHz, peak to peak value: 6.8V to 7.2V and Min/Max: ±5V and the varying ±Duty Cycle
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        36. Testing Triangle Waveform
        37. Set the signal generator’s waveform type: Triangle, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 2ms
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 5.000ms, Frequency: 200Hz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.4V to ±2.6V
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        38. Set the signal generator’s waveform type: Triangle, Amplitude 8V, Frequency: 2MHz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 100ns
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 500.000ns, Frequency: 2MHz, peak to peak value: 7.8V to 8.2V and Min/Max: +/- 3.9V
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        39. Testing Rising Ramp Sawtooth Waveform
        40. Set the signal generator’s waveform type: Rising Ramp Sawtooth, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 2ms
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 5.000ms, Frequency: 200Hz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.4V to ±2.7V. Use the Oscilloscope’s cursor to disregard the overshoot of the signal
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        41. Set the signal generator’s waveform type: Rising Ramp Sawtooth, Amplitude 8V, Frequency: 1MHz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 1us
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 1.000us, Frequency: 1MHz, peak to peak value: 7.8V to 8.2V and Min/Max: ±3.9V to ±4.1V. Use the Oscilloscope’s cursor to disregard the overshoot of the signal
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        42. Testing Falling Ramp Sawtooth Waveform
        43. Set the signal generator’s waveform type: Falling Ramp Sawtooth, Amplitude 5V, Frequency: 200Hz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 2ms
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 5.000ms, Frequency: 200Hz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.4V to ±2.6V
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        44. Set the signal generator’s waveform type: Falling Ramp Sawtooth, Amplitude 8V, Frequency: 1MHz, offset: 0V and Phase: 0 degrees. Set the Oscilloscope’s Volts/div: 1V/div, trigger mode: Auto and time base: 1us
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 1.000us, Frequency: 1MHz, peak to peak value: 7.8V to 8.2V and Min/Max: ±3.9V to ±4.1V. Use the Oscilloscope’s cursor to disregard the overshoot of the signal
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        45. Testing Trapezoidal waveform
        46. Set the signal generator’s waveform type: Trapezoidal, Amplitude: 5V, Rise Time: 1us, Fall Time: 1us, Hold High Time: 1us, Hold Low time Time: 1us. Set the Oscilloscope’s Volt/div: 2V, Trigger Mode: Auto and Time Base: 1us
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 4.000us, Frequency: 250kHz, peak to peak value: 4.8V to 5.2V and Min/Max: ±2.4V to ±2.6.
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        47. Set the signal generator’s waveform type: Trapezoidal, Amplitude: 10V, Rise Time: 1us, Fall Time: 1us, Hold High Time: 1us, Hold Low time Time: 1us. Set the Oscilloscope’s Volt/div: 2V, Trigger Mode: Auto and Time Base: 1us
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 4.000us, Frequency: 250kHz, peak to peak value: 9.6V to 10.4V and Min/Max: ±4.8V to ±5.2.
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

        48. Set the signal generator’s waveform type: Trapezoidal, Amplitude: 10V, Rise Time: 200ns, Fall Time: 200ns, Hold High Time: 200ns, Hold Low time: 200ns. Set the Oscilloscope’s Volt/div: 2V, Trigger Mode: Auto and Time Base: 200ns
                    - **Expected Result:** Oscilloscope’s measurement should be Period: 800ns, Frequency: 1.250MHz, peak to peak value: 9.6V to 10.4V and Min/Max: ±4.8V to ±5.2.
                    - **Actual Result:** The values are displayed correctly

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:** none

..
  Any comments about the test goes here.

**Result:** PASS

..
  The result of the test goes here (PASS/FAIL).


Test 3: Channel 1 and Channel 2 Operation
-------------------------------------------------------

**UID:** TST.M2K.SG.CHANNEL_1_AND_CHANNEL_2_OPERATION

**Description:** This test case verifies the operation of the channel 1 and channel 2 of the signal generator.

**Preconditions:**
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-signal-generator>` setup.

**Steps:**
        1. Test constant voltage generator for both channels simultaneously
        2. Turn on channels 1 and 2 and view the configuration window by clicking the on/off button then the menu button. Choose Constant from the configuration menu for both channels
        3. Connect AWG ch1 to scope ch1+ and scope ch1- to gnd. Connect AWG ch2 to scope ch2+ and scope ch2- to gnd
        4. Set signal generator’s channel 1 to 4.5V and channel 2 to -4.0V
                    - **Expected Result:** Open voltmeter instrument in DC mode. Channel 1 should have a voltage of 4.4V to 4.6V and channel 2 should have a voltage of -4.1V to -3.9V
                    - **Actual Result:**

..
  Actual test result goes here.
..

        5. Set signal generator’s channel 1 to -4.5V and channel 2 to 4.0V
        6. Test different waveforms for both channels simultaneously
        7. Turn on channels 1 and 2 and view the configuration window by clicking the on/off button then the menu button. Choose waveform from the configuration menu for both channels
        8. Connect AWG ch1 to scope ch1+ and scope ch1- to gnd. Connect AWG ch2 to scope ch2+ and scope ch2- to gnd
        9. Test phase configuration
        10. Set signal generator channels 1 and 2 to either Sine or Triangle waveform type, they should be the same. For channel 1 set Amplitude: 5V, Frequency: 5kHz, offset: 0V and phase: 0°. Set signal generator’s channel 2 to Amplitude: 5V, Frequency: 5kHz, offset: 0V and phase: 180°. Set Oscilloscope’s both channel to Time Base: 200us, Volts/Div: 1V
        11. Run Oscilloscope, add channel with an input function: f(t) = sin(t1) + sin(t0).
                    - **Expected Result:** The new plot’s value should be very close to 0V ranging around -0.2V to 0.2V
                    - **Actual Result:**

..
  Actual test result goes here.
..

        12. Set signal generator channels 1 and 2 to either Sine or Triangle waveform type, they should be the same. For channel 1 set Amplitude: 5V, Frequency: 5kHz, offset: 0V and phase: 0°. Set signal generator’s channel 2 to Amplitude: 5V, Frequency: 5kHz, offset: 0V and phase: 360°. Set Oscilloscope’s both channel to Time Base: 200us, Volts/Div: 1V
        13. Run Oscilloscope, add channel with an input function: f(t) = sin(t1) - sin(t0).
                    - **Expected Result:** The new plot’s value should be very close to 0V ranging around -0.2V to 0.2V
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


Test 4: Additional Features
-------------------------------------------------------

**UID:** TST.M2K.SG.ADDITIONAL_FEATURES

**Description:** This test case verifies the additional features of the signal generator.

**Preconditions:**
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-signal-generator>` setup.

**Steps:**
        1. Test Noise
        2. Turn on Signal Generator’s channel 1 and set the following parameter, Waveform Type: Square Wave, Amplitude: 3V, Offset: 1.5V, Frequency: 1kHz, Phase: 0degrees and Duty Cycle: 50%
        3. Connect AWG ch1 to scope ch1+ and scope ch1- to gnd
                    - **Expected Result:** Check in the Oscilloscope if the Square Wave signal generated is from 0V to 3V.
                    - **Actual Result:**

..
  Actual test result goes here.
..

        4. Choose Uniform Noise Type in the dropdown menu and set it to 500mV
        5. Set the Oscilloscope’s setting to Time Base: 100us, Volts/Div: 500mV/Div; Using the cursors measure the noise generated in the square waveform
                    - **Expected Result:** The measured voltage should be close to 500mV. Check the step resource picture for reference.
                    - **Actual Result:**

..
  Actual test result goes here.
..

        6. Repeat steps 1.3 and 1.4 using different Noise Amplitude [1V, 1.5V, 2V and 2.5V]
                    - **Expected Result:** The measured voltage should be close to the desired noise voltage.
                    - **Actual Result:**

..
  Actual test result goes here.
..

        7. Test Buffer
        8. Download buffer test files (https://wiki.analog.com/_media/university/tools/m2k/scopy/test-cases/signal_generator_buffer_test.zip). Open Signal Generator Instrument and click the Buffer Tab
                    - **Expected Result:** Refer to the Step Resource Image for reference
                    - **Actual Result:**

..
  Actual test result goes here.
..

        9. Connect AWG ch1 to scope ch1+ and scope ch1- to gnd
        10. Test .csv file
        11. Load the .csv file from the downloaded .zip file
                    - **Expected Result:** The signal generated should be a stair step signal.
                    - **Actual Result:**

..
  Actual test result goes here.
..

        12. Test .mat file
        13. Load the .mat file from the downloaded .zip file. Set the frequency to 20kHz, and the time base of Oscilloscope to 10ms.
                    - **Expected Result:** The signal generated should be a sine wave signal.
                    - **Actual Result:**

..
  Actual test result goes here.
..

        14. Test Math
        15. Open Signal Generator Instrument and click the Math tab
                    - **Expected Result:** Refer to the Step Resource image for reference.
                    - **Actual Result:**

..
  Actual test result goes here.
..

        16. Connect AWG ch1 to scope ch1+ and scope ch1- to gnd
        17. Generate Sine waves
        18. In the Signal Generator Math Function tab, set frequency to 100Hz, and type in the function box 5*sin(t) and click apply. In the Oscilloscope instrument set Volts/div: 1V/div, Trigger: Auto, Time base: 2ms
                    - **Expected Result:** The generated sine wave signal should have the following parameters, peak to peak: 9.6Vpp to 10.4Vpp, frequency: 100Hz, and period: 10ms. Refer to the Step resource image for reference
                    - **Actual Result:**

..
  Actual test result goes here.
..

        19. In the Signal Generator Math Function tab, set frequency to 1kHz, and type in the function box 4*sin(10*t) and click apply. In the Oscilloscope instrument set Volts/div: 1V/div, Trigger: Auto, Time base: 20us
                    - **Expected Result:** The generated sine wave signal should have the following parameters, peak to peak: 7.6Vpp to 8.4Vpp, frequency: 10kHz, and period: 100us. Refer to the Step resource image for reference
                    - **Actual Result:**

..
  Actual test result goes here.
..

        20. In the Signal Generator Math Function tab, set frequency to 100kHz, and type in the function box 3*sin(50*t) and click apply. In the Oscilloscope instrument set Volts/div: 1V/div, Trigger: Auto, Time base: 100ns
                    - **Expected Result:** The generated sine wave signal should have the following parameters, peak to peak: 5.6Vpp to 6.4Vpp, frequency: 5MHz, and period: 200ns. Refer to the Step resource image for reference
                    - **Actual Result:**

..
  Actual test result goes here.
..

        21. Generate Square waves
        22. In the Signal Generator Math Function tab, set frequency to 500kHz, and type in the function box 4*sin(t) + 4*sin(3*t)/3 + 4*sin(5*t)/5 + 4*sin(7*t)/7 + 4*sin(9*t)/9 + 4*sin(11*t)/11 (you can copy and paste the text to Scopy) and click apply. In the Oscilloscope instrument set Volts/div: 1V/div, Trigger: Auto, Time base: 500ns
                    - **Expected Result:** The generated square wave signal should have the following parameters, peak to peak: 7Vpp to 7.4Vpp, frequency: 500kHz, and period: 2us. Refer to the Step resource image for reference
                    - **Actual Result:**

..
  Actual test result goes here.
..

        23. Waveform Phase – Seconds
        24. Open Waveform tab. Set frequency to 500Hz. Set Phase to 90 degrees. Then change phase unit to seconds.
                    - **Expected Result:** The value of Phase should automatically change to 500us that is 90 degrees in seconds for a frequency of 500Hz.
                    - **Actual Result:**

..
  Actual test result goes here.
..

        25. Increase and decrease the value of phase.
                    - **Expected Result:** The display should follow accordingly.
                    - **Actual Result:**

..
  Actual test result goes here.
..

        26. Increase phase value to 1.5 ms. Then change again the unit to degrees.
                    - **Expected Result:** The value should now be 270 degrees.
                    - **Actual Result:**

..
  Actual test result goes here.
..

        27. Change frequency to 1 MHz. Then set phase to 1us. This corresponds to a full period of a 1MHz frequency.
                    - **Expected Result:** The interface should look like in steps resources picture.
                    - **Actual Result:**

..
  Actual test result goes here.
..

        28. Change phase unit to degrees.
                    - **Expected Result:** The value should be 360 degrees.
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

