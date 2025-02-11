.. _m2k_pattern_generator_tests:

Pattern Generator - Test Suite
====================================================================================================

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
----------------------------------------------------------------------------------------------------

.. _m2k-usb-pattern-generator:

**M2k.Usb:**
        - Open Scopy.
        - Connect an **ADALM2000** device to the system by USB.
        - Add the device in device browser.

Test 1: Individual Channel Operation
----------------------------------------------------------------------------------------------------

**UID:** TST.M2K.PG.INDIVIDUAL_CHANNEL_OPERATION

**Description:** This test case verifies the functionality of the pattern generator in individual channel operation mode.

**Preconditions:**
        - Scopy is installed on the system.
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-pattern-generator>` setup.

**Steps:**
        1. Checking Individual Channels: Use PP as output
        2. Connect DIO-0, CH0 to Scope CH1+, GND to Scope CH1-.
        3. Enable CH0. Double click on the DIO 0 indicator on the plot to open DIO 0 settings. Select pattern as Clock with a 5 KHz clock signal with duty cycle of 50%. Run instrument.
                - **Expected Result:** You should see a square wave with 5 KHz frequency, 50% duty cycle.
                - **Actual Result:**

..
  Actual test result goes here.
..

        4. Monitor CH0 through oscilloscope. Open built-in measurement feature for frequency, amplitude and duty cycle.
                - **Expected Result:** You should see a square wave with 5 KHz frequency, 50% duty cycle. In the oscilloscope, Frequency: 5 KHz, Amplitude: 3.2V to 3.4 V, Duty+: 50 %, Duty-:50%
                - **Actual Result:**

..
  Actual test result goes here.
..

        5. Change frequency: 100 KHz, duty cycle: 30%.
                - **Expected Result:** You should see a square wave with 100 KHz frequency, 30% duty cycle. In the oscilloscope, Frequency: 100 KHz, Amplitude: 3.2V to 3.4 V, Duty+: 30 %, Duty-: 70%
                - **Actual Result:**

..
  Actual test result goes here.
..

        6. Change frequency: 1 MHz, duty cycle: 60%.
                - **Expected Result:** You should see a square wave with 1 MHz frequency, 60% duty cycle. In the oscilloscope, Frequency: 1 MHz, Amplitude: 3.2V to 3.4 V, Duty+: 60 %, Duty-: 40%
                - **Actual Result:**

..
  Actual test result goes here.
..

        7. Change frequency: 10 MHz, duty cycle: 70%.
                - **Expected Result:** You should see a square wave with 10 MHz frequency, 70% duty cycle. In the oscilloscope, Frequency: 1 MHz, Amplitude: 3.2V to 3.4 V, Duty+: 70 %, Duty-: 30%
                - **Actual Result:**

..
  Actual test result goes here.
..

        8. Repeat steps 2. to 7. for DIO-1 to DIO-15.
                - **Expected Result:** Behavior of each channel should be the same as with DIO-0, CH0. As the parameters are changed, the trace displayed in oscilloscope should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        9. Checking Phase
        10. Connect the following: DIO0 to ScopeCH1+, DIO1 to ScopeCH2+. GND to Scope CH1- and Scope CH2-.
        11. Enable DIO0 and DIO1. Set the following parameters: DIO0: Frequency: 5 KHz, Phase: 0°, Duty Cycle: 50%; DIO1: Frequency: 5 KHz, Phase: 45°, Duty Cycle: 50 %. Run instrument.
                - **Expected Result:** You should see two square waves with 5 KHz frequency, 50% duty cycle. DIO0 should be at 0° phase and DIO1 should be at 45° phase.
                - **Actual Result:**

..
  Actual test result goes here.
..

        12. Monitor pattern generator output through oscilloscope.
                - **Expected Result:** Use cursor feature of the oscilloscope. Move the vertical cursors as shown in steps resources. ΔT = 24us to 26us, corresponding to the 45° phase shift.
                - **Actual Result:**

..
  Actual test result goes here.
..

        13. Change DIO1 phase: 120°.
                - **Expected Result:** Use cursor feature of the oscilloscope. Move the vertical cursors as shown in steps resources. ΔT = 65us to 67us, corresponding to the 120° phase shift.
                - **Actual Result:**

..
  Actual test result goes here.
..

        14. Change DIO1 phase: 270°.
                - **Expected Result:** Use cursor feature of the oscilloscope. Move the vertical cursors as shown in steps resources. ΔT = 149us to 151us, corresponding to the 270° phase shift.
                - **Actual Result:**

..
  Actual test result goes here.
..

        15. Set DIO1 phase to 0°. Now repeat steps for DIO0.
                - **Expected Result:** Behavior of channel should be the same as with DIO1, CH1.
                - **Actual Result:**

..
  Actual test result goes here.
..

        16. Use other channels, DIO2 to DIO15, and repeat steps to verify each.
                - **Expected Result:** Behavior of each channel should be the same as with DIO1, CH1.
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


Test 2: Group Channel Operation
----------------------------------------------------------------------------------------------------

**UID:** TST.M2K.PG.GROUP_CHANNEL_OPERATION

**Description:** This test case verifies the functionality of the pattern generator in group channel operation mode.

**Preconditions:**
        - Scopy is installed on the system.
        - Use :ref:`M2k.Usb <m2k-usb-pattern-generator>` setup.
        - OS: ANY

**Steps:**
        1. Checking Group Channels and Patterns: Use PP as output. Binary Counter
        2. Create a 4-channel group. Enable channels DIO0 to DIO3. Then click “Group” and double click on the channel indicators on the plot, DIO 0 to DIO 3, then click “Done”. Change pattern to Binary Counter.
                - **Expected Result:** The plot should show a binary counter from 1 to e. The frequency should be 5 KHz.
                - **Actual Result:**

..
  Actual test result goes here.
..

        3. Open logic analyzer. Make a group with channels DIO0 to DIO3. Once grouped, add parallel for the decoder. Make sure to select correct data lines in the parallel decoder settings.
                - **Expected Result:** The plot in the logic analyzer should resemble the plot seen in the pattern generator.
                - **Actual Result:**

..
  Actual test result goes here.
..

        4. Check the frequency of each channel through oscilloscope. Connect DIO0 to scopech1+. Enable built-in measurement for frequency.
                - **Expected Result:** Frequency shown should be 2.4 KHz to 2.6 KHz, corresponding to set clock frequency/2.
                - **Actual Result:**

..
  Actual test result goes here.
..

        5. Connect DIO1 to scopech1+. Enable built-in measurement for frequency.
                - **Expected Result:** Frequency shown should be 1.24 KHz to 1.27 KHz, corresponding to set clock frequency/4.
                - **Actual Result:**

..
  Actual test result goes here.
..

        6. Connect DIO2 to scopech1+. Enable built-in measurement for frequency.
                - **Expected Result:** Frequency shown should be 620 Hz to 630 Hz, corresponding to set clock frequency/8.
                - **Actual Result:**

..
  Actual test result goes here.
..

        7. Connect DIO3 to scopech1+. Enable built-in measurement for frequency.
                - **Expected Result:** Frequency shown should be 310 Hz to 315 Hz, corresponding to set clock frequency/16.
                - **Actual Result:**

..
  Actual test result goes here.
..

        8. Random
        9. Change pattern to Random. Frequency: 5KHz
                - **Expected Result:** The plot should show random data.
                - **Actual Result:**

..
  Actual test result goes here.
..

        10. Monitor through logic analyzer. Use parallel as decoder.
                - **Expected Result:** The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same hexadecimal equivalents should be seen in logic analyzer.
                - **Actual Result:**

..
  Actual test result goes here.
..

        11. Change frequency: 100 KHz
                - **Expected Result:** The frequency should now be 100 KHz. There should be new set of data and hexadecimal equivalents.
                - **Actual Result:**

..
  Actual test result goes here.
..

        12. Monitor through logic analyzer. Use parallel as decoder.
                - **Expected Result:** The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same hexadecimal equivalents should be seen in logic analyzer.
                - **Actual Result:**

..
  Actual test result goes here.
..

        13. Number pattern
        14. Change pattern to Number pattern. Set number to 3. Enable DIO 4 and set to Clock pattern with 5kHz frequency. Do not add DIO 4 to group, keep it as individual channel.
                - **Expected Result:** The plot should contain the group channel and individual channel. The group channel should show the number pattern and the individual channel should show the clock pattern.
                - **Actual Result:**

..
  Actual test result goes here.
..

        15. Monitor through logic analyzer. Enable DIO 4 as individual channel. Use parallel as decoder. Set data lines to DIO 0 to DIO 3 and set clock line to DIO 4.
                - **Expected Result:** The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same number is seen in logic analyzer. Number: 3
                - **Actual Result:**

..
  Actual test result goes here.
..

        16. Change number to 14. In the plot, it will show the hexadecimal equivalent which is E.
                - **Expected Result:** The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same hexadecimal equivalent is seen in logic analyzer. Hexadecimal equivalent: E
                - **Actual Result:**

..
  Actual test result goes here.
..

        17. Add channels DIO4 to DIO7 to the group. It will now be an 8-channel group. Change number to 254. The plot will show the hexadecimal equivalent which is FE.
                - **Expected Result:** The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same hexadecimal equivalent is seen in logic analyzer. Hexadecimal equivalent: FE
                - **Actual Result:**

..
  Actual test result goes here.
..

        18. Gray Counter
        19. Change pattern to Gray Counter. Disable DIO 8.
                - **Expected Result:** The plot should show a gray counter from 1 to 7. The frequency should be 5 KHz.
                - **Actual Result:**

..
  Actual test result goes here.
..

        20. Monitor through logic analyzer. Choose parallel for the decoder. Set Clock line as X.
                - **Expected Result:** The plot in the logic analyzer should resemble the plot seen in the pattern generator. One bit change per clock cycle.
                - **Actual Result:**

..
  Actual test result goes here.
..

        21. UART
        22. Dissolve current group channel. Enable DIO 0 channel and double click on the channel indicator on the plot. Change channel pattern to UART. Set parameters: Baud: 9600, Stop bit: 1, no parity, Data to send: ‘HELLO’.
                - **Expected Result:** The plot should show the data ‘HELLO’ in ASCII format. The frequency should be 9600 Hz.
                - **Actual Result:**

..
  Actual test result goes here.
..

        23. Monitor the channel in the logic analyzer. Use UART as decoder. Set Baud: 9600, Data bits: 8, no parity.
                - **Expected Result:** The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same ASCII data should be seen in logic analyzer.
                - **Actual Result:**

..
  Actual test result goes here.
..

        24. Change set parameters: Baud: 115200, Stop bit: 1, even parity, Data to send: ‘HI’.
                - **Expected Result:** The plot should show the data ‘HI’ in ASCII format. The frequency should be 115200 Hz.
                - **Actual Result:**

..
  Actual test result goes here.
..

        25. Monitor the channel in the logic analyzer. Use UART as decoder. Set Baud: 115200, Data bits: 8, even parity.
                - **Expected Result:** The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same ASCII data should be seen in logic analyzer.
                - **Actual Result:**

..
  Actual test result goes here.
..

        26. Change set parameters: Baud: 115200, Stop bit: 1, odd parity, Data to send: ‘HI’.
                - **Expected Result:** The plot should show the data ‘HI’ in ASCII format. The frequency should be 115200 Hz.
                - **Actual Result:**

..
  Actual test result goes here.
..

        27. Monitor the channel in the logic analyzer. Use UART as decoder. Set Baud: 115200, Data bits: 8, odd parity.
                - **Expected Result:** The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same ASCII data should be seen in logic analyzer.
                - **Actual Result:**

..
  Actual test result goes here.
..

        28. SPI
        29. Disable DIO 0. Enable and select DIO5 to DIO7 to create a 3-channel group. Change pattern to SPI. Set the following parameters: Bytes per frame: 2, inter frame space: 3, Data: ABCD1234.
                - **Expected Result:** The plot should show the data ‘ABCD1234’ in ASCII format. The frequency should be 5 KHz.
                - **Actual Result:**

..
  Actual test result goes here.
..

        30. Monitor the channel through logic analyzer. Use SPI as decoder. Refer to steps resources picture for the configuration of logic analyzer.
                - **Expected Result:** The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same ASCII data should be seen in logic analyzer.
                - **Actual Result:**

..
  Actual test result goes here.
..

        31. Change the following parameters: Bytes per frame: 1, inter frame space: 4, Data: ABCD1234.
                - **Expected Result:** The plot should show the data ‘ABCD1234’ in ASCII format. The frequency should be 5 KHz.
                - **Actual Result:**

..
  Actual test result goes here.
..

        32. Monitor the channel through logic analyzer. Use SPI as decoder. Refer to steps resources picture for the configuration of logic analyzer.
                - **Expected Result:** The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same ASCII data should be seen in logic analyzer.
                - **Actual Result:**

..
  Actual test result goes here.
..

        33. I2C
        34. Dissolve current group channel. Enable and select DIO0 and DIO1 to create a 2-channel group. Change pattern to I2C. Set the following parameters: Address: 72, Inter frame space: 3, Data: ABCD1234.
                - **Expected Result:** The plot should show the data ‘ABCD1234’ in ASCII format. The frequency should be 5 KHz.
                - **Actual Result:**

..
  Actual test result goes here.
..

        35. Monitor the channel through logic analyzer. Use I2C as decoder. Refer to steps resources picture for the configuration of logic analyzer.
                - **Expected Result:** The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same ASCII data should be seen in logic analyzer.
                - **Actual Result:**

..
  Actual test result goes here.
..

        36. Pulse Pattern
        37. Change pattern to Pulse Pattern. Set the following parameters: Low: 5, High: 1, Counter Init: 0, Delay: 10, Number of Pulses: 5.
                - **Expected Result:** The plot should show 5 pulses with 5 low and 1 high.
                - **Actual Result:**

..
  Actual test result goes here.
..

        38. Monitor the channels through logic analyzer. Refer to steps resources picture for the configuration of logic analyzer.
                - **Expected Result:** The plot in the logic analyzer should resemble the plot seen in the pattern generator.
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


Test 3: Simultaneous Group and Individual Channels Operation
----------------------------------------------------------------------------------------------------

**UID:** TST.M2K.PG.SIMULTANEOUS_GROUP_AND_INDIVIDUAL_CHANNELS_OPERATION

**Description:** This test case verifies the functionality of the pattern generator in simultaneous group and individual channels operation mode.

**Preconditions:**
        - Scopy is installed on the system.
        - Use :ref:`M2k.Usb <m2k-usb-pattern-generator>` setup.
        - OS: ANY

**Steps:**
        1. Checking Group and Individual Channels Simultaneously: Use PP as output.
        2. Enable and select channels DIO0 to DIO3 to create 4-channel group. Change group pattern to Binary Counter with frequency set to 5 KHz. Enable DIO4 channel and set as clock with frequency of 5 KHz.
        3. Monitor DIO4 through oscilloscope. And at the same time monitor the group channel through logic analyzer.
                - **Expected Result:** On logic analyzer, the plot should resemble the plot seen in pattern generator, the group channel as well as the individual channel DIO4. On oscilloscope, frequency can be viewed by enabling measurement feature, frequency: 5KHz.
                - **Actual Result:**

..
  Actual test result goes here.
..

        4. Do not dissolve group channel. Add another group channel. Enable and select DIO5, create a 1-channel group for UART. Change pattern to UART. Baud: 2400, stop bit: 1, no parity, Data: ‘HI’. Also, individual DIO4 channel remains enabled.
        5. Monitor the 2 groups and DIO4 through logic analyzer.
                - **Expected Result:** On logic analyzer, the plot should resemble the plot seen in pattern generator.
                - **Actual Result:**

..
  Actual test result goes here.
..

        6. Do not dissolve group channels. Disable Group UART. Add another group channel. Enable and select DIO6 to DIO9, create a 4-channel group. Change pattern to Gray Counter. Frequency: 10 KHz. Name this group as Group GC. Also, individual DIO4 channel remains enabled.
        7. Monitor the 2 groups and DIO4 through logic analyzer.
                - **Expected Result:** On logic analyzer, the plot should resemble the plot seen in pattern generator.
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


Test 4: Other Features
----------------------------------------------------------------------------------------------------

**UID:** TST.M2K.PG.OTHER_FEATURES

**Description:** This test case verifies the functionality of the pattern generator in other features.

**Preconditions:**
        - Scopy is installed on the system.
        - Use :ref:`M2k.Usb <m2k-usb-pattern-generator>` setup.
        - OS: ANY

**Steps:**
        1. Checking UI: Changing Channel Name
        2. Open individual channel DIO. On its channel manager, modify its name to ‘CH 0’.
                - **Expected Result:** The name should change as shown in steps resources picture.
                - **Actual Result:**

..
  Actual test result goes here.
..

        3. Enable DIO 1 and change its name to 'CH 1'. Create a group with 'CH 0' and 'CH 1'.
                - **Expected Result:** The list of names under the group should also correspond to the names of the channels as should change as shown in steps resources picture.
                - **Actual Result:**

..
  Actual test result goes here.
..

        4. Trace Height
        5. Open channel ‘CH 0’. On its channel manager, change trace height to 50.
                - **Expected Result:** The trace height should now be twice as shown in steps resources picture, compared to previous.
                - **Actual Result:**

..
  Actual test result goes here.
..

        6. Change height again to 10.
                - **Expected Result:** The height should now be lower as shown in steps resources picture
                - **Actual Result:**

..
  Actual test result goes here.
..

        7. Knobs
        8. Checking frequency knob. Set the knob to large increment. No orange dot on the center. Change frequency value using the ± button.
                - **Expected Result:** The frequency value should change accordingly with a high increment/decrement from 5 KHz to 10 KHz.
                - **Actual Result:**

..
  Actual test result goes here.
..

        9. Set the knob to ±1 unit interval. With orange dot on the center. Change frequency value using the ± button.
                - **Expected Result:** The frequency value should change accordingly with ±1 unit interval.
                - **Actual Result:**

..
  Actual test result goes here.
..

        10. Checking the output: PP mode
        11. Connect the DIO0 to oscilloscope ch1+, and oscilloscope ch1- to gnd. This is to monitor the output from the pattern generator.
        12. Enable DIO0 in pattern generator. Set pattern to clock with 5 kHz frequency. Set output as PP. Run instrument and monitor on Oscilloscope.
                - **Expected Result:** The oscilloscope should show clock pulses from logic 0 to 1. It should look like in steps resources picture.
                - **Actual Result:**

..
  Actual test result goes here.
..

        13. Try other patterns such as random pattern and monitor on oscilloscope.
                - **Expected Result:** The oscilloscope should show random pulses from logic 0 to 1. It should look like in steps resources picture.
                - **Actual Result:**

..
  Actual test result goes here.
..

        14. Repeat steps 10. and 13. for all channels
        15. OD mode
        16. Change output to OD. Monitor output in oscilloscope.
                - **Expected Result:** Oscilloscope should only show logic 0 since output is now in OD mode.
                - **Actual Result:**

..
  Actual test result goes here.
..

        17. Do 5.1 to other channels.
        18. To output two logic levels when operating in OD, a pull up resistor is needed. Connect the breadboard connection shown in steps resources.
        19. Set power supply to 5V. Run power supply, pattern generator and monitor in oscilloscope.
                - **Expected Result:** The trace should show two logic levels, with a few mV offset. When power supply is turned off, the oscilloscope should show only logic 0.
                - **Actual Result:**

..
  Actual test result goes here.
..

        20. Repeat step 5.3 and 5.4 for all channels.
        21. Print
        22. Click on Print button and save file as sample.pdf
                - **Expected Result:** Upon saving, the prompt window should look like the steps resources picture.
                - **Actual Result:**

..
  Actual test result goes here.
..

        23. Open the saved file.
                - **Expected Result:** The file should show the waveform that you have saved.
                - **Actual Result:**

..
  Actual test result goes here.
..

        24. See more info
        25. Click the 'See more info' icon on the upper left of the pattern generator window.
                - **Expected Result:** It should lead to the wiki page of pattern generator.
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


