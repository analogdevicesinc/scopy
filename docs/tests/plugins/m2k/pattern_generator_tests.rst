M2K Pattern Generator - Test Suite
====================================================================================================

Initial Setup
----------------------------------------------------------------------------------------------------

In order to proceed through the test case, first of all delete the Scopy \*.ini file (saves previous settings made in Scopy tool).

Test Case
----------------------------------------------------------------------------------------------------

Setup:
        - M2K.*

Test 1: Individual Channel Operation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.M2K.PG.INDIVIDUAL_CHANNEL_OPERATION

Description:
        - This test case verifies the functionality of the pattern generator in individual channel operation mode.

OS:
        - any

Steps:
        * Test 1: Checking Individual Channels: Use PP as output
        * Test 1.1: Connect DIO-0, CH0 to Scope CH1+, GND to Scope CH1-.
        * Test 1.2: Enable CH0. Double click on the DIO 0 indicator on the plot to open DIO 0 settings. Select pattern as Clock with a 5 KHz clock signal with duty cycle of 50%. Run instrument.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Test 1.3: Monitor CH0 through oscilloscope. Open built-in measurement feature for frequency, amplitude and duty cycle.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side). In the oscilloscope, Frequency: 5 KHz, Amplitude: 3.2V to 3.4 V, Duty+: 50 %, Duty-:50%
        * Test 1.4: Change frequency: 100 KHz, duty cycle: 30%.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side). In the oscilloscope, Frequency: 100 KHz, Amplitude: 3.2V to 3.4 V, Duty+: 30 %, Duty-: 70%
        * Test 1.5: Change frequency: 1 MHz, duty cycle: 60%.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side). In the oscilloscope, Frequency: 1 MHz, Amplitude: 3.2V to 3.4 V, Duty+: 60 %, Duty-: 40%
        * Test 1.6: Change frequency: 10 MHz, duty cycle: 70%.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side). In the oscilloscope, Frequency: 1 MHz, Amplitude: 3.2V to 3.4 V, Duty+: 70 %, Duty-: 30%
        * Test 1.7: Repeat steps 1.2 to 1.7 for DIO-1 to DIO-15.
                * Expected Result: Behavior of each channel should be the same as with DIO-0, CH0. As the parameters are changed, the trace displayed in oscilloscope should follow.
        * Test 2: Checking Phase
        * Test 2.1: Connect the following: DIO0 to ScopeCH1+, DIO1 to ScopeCH2+. GND to Scope CH1- and Scope CH2-.
        * Test 2.2: Enable DIO0 and DIO1. Set the following parameters: DIO0: Frequency: 5 KHz, Phase: 0°, Duty Cycle: 50%; DIO1: Frequency: 5 KHz, Phase: 45°, Duty Cycle: 50 %. Run instrument.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Test 2.3: Monitor pattern generator output through oscilloscope.
                * Expected Result: Use cursor feature of the oscilloscope. Move the vertical cursors as shown in steps resources. ΔT = 24us to 26us, corresponding to the 45° phase shift.
        * Test 2.4: Change DIO1 phase: 120°.
                * Expected Result: Use cursor feature of the oscilloscope. Move the vertical cursors as shown in steps resources. ΔT = 65us to 67us, corresponding to the 120° phase shift.
        * Test 2.5: Change DIO1 phase: 270°.
                * Expected Result: Use cursor feature of the oscilloscope. Move the vertical cursors as shown in steps resources. ΔT = 149us to 151us, corresponding to the 270° phase shift.
        * Test 2.6: Set DIO1 phase to 0°. Now repeat steps for DIO0.
                * Expected Result: Behavior of channel should be the same as with DIO1, CH1.
        * Test 2.7: Use other channels, DIO2 to DIO15, and repeat steps to verify each.
                * Expected Result: Behavior of each channel should be the same as with DIO1, CH1.


Test 2: Group Channel Operation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.M2K.PG.GROUP_CHANNEL_OPERATION

Description:
        - This test case verifies the functionality of the pattern generator in group channel operation mode.

OS:
        - any

Steps:
        * Step 1: Checking Group Channels and Patterns: Use PP as output. Binary Counter
        * Step 1.1: Create a 4-channel group. Enable channels DIO0 to DIO3. Then click “Group” and double click on the channel indicators on the plot, DIO 0 to DIO 3, then click “Done”. Change pattern to Binary Counter.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 1.2: Open logic analyzer. Make a group with channels DIO0 to DIO3. Once grouped, add parallel for the decoder. Make sure to select correct data lines in the parallel decoder settings.
                * Expected Result: The plot in the logic analyzer should resemble the plot seen in the pattern generator.
        * Step 1.3: Check the frequency of each channel through oscilloscope. Connect DIO0 to scopech1+. Enable built-in measurement for frequency.
                * Expected Result: Frequency shown should be 2.4 KHz to 2.6 KHz, corresponding to set clock frequency/2.
        * Step 1.4: Connect DIO1 to scopech1+. Enable built-in measurement for frequency.
                * Expected Result: Frequency shown should be 1.24 KHz to 1.27 KHz, corresponding to set clock frequency/4.
        * Step 1.5: Connect DIO2 to scopech1+. Enable built-in measurement for frequency.
                * Expected Result: Frequency shown should be 620 Hz to 630 Hz, corresponding to set clock frequency/8.
        * Step 1.6: Connect DIO3 to scopech1+. Enable built-in measurement for frequency.
                * Expected Result: Frequency shown should be 310 Hz to 315 Hz, corresponding to set clock frequency/16.
        * Step 2: Random
        * Step 2.1: Change pattern to Random. Frequency: 5KHz
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 2.2: Monitor through logic analyzer. Use parallel as decoder.
                * Expected Result: The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same hexadecimal equivalents should be seen in logic analyzer.
        * Step 2.3: Change frequency: 100 KHz
                * Expected Result: The interface should look like in the “Step Resources” picture (left side). There should be new set of data and hexadecimal equivalents.
        * Step 2.4: Monitor through logic analyzer. Use parallel as decoder.
                * Expected Result: The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same hexadecimal equivalents should be seen in logic analyzer.
        * Step 3: Number pattern
        * Step 3.1: Change pattern to Number pattern. Set number to 3. Enable DIO 4 and set to Clock pattern with 5kHz frequency. Do not add DIO 4 to group, keep it as individual channel.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 3.2: Monitor through logic analyzer. Enable DIO 4 as individual channel. Use parallel as decoder. Set data lines to DIO 0 to DIO 3 and set clock line to DIO 4.
                * Expected Result: The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same number is seen in logic analyzer. Number: 3
        * Step 3.3: Change number to 14. In the plot, it will show the hexadecimal equivalent which is E.
                * Expected Result: The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same hexadecimal equivalent is seen in logic analyzer. Hexadecimal equivalent: E
        * Step 3.4: Add channels DIO4 to DIO7 to the group. It will now be an 8-channel group. Change number to 254. The plot will show the hexadecimal equivalent which is FE.
                * Expected Result: The plot in the logic analyzer should resemble the plot seen in the pattern generator. The same hexadecimal equivalent is seen in logic analyzer. Hexadecimal equivalent: FE
        * Step 4: Gray Counter
        * Step 4.1: Change pattern to Gray Counter. Disable DIO 8.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 4.2: Monitor through logic analyzer. Choose parallel for the decoder. Set Clock line as X.
                * Expected Result: The plot in the logic analyzer should resemble the plot seen in the pattern generator. One bit change per clock cycle.
        * Step 5: UART
        * Step 5.1: Dissolve current group channel. Enable DIO 0 channel and double click on the channel indicator on the plot. Change channel pattern to UART. Set parameters: Baud: 9600, Stop bit: 1, no parity, Data to send: ‘HELLO’.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 5.2: Monitor the channel in the logic analyzer. Use UART as decoder. Set Baud: 9600, Data bits: 8, no parity.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 5.3: Change set parameters: Baud: 115200, Stop bit: 1, even parity, Data to send: ‘HI’.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 5.4: Monitor the channel in the logic analyzer. Use UART as decoder. Set Baud: 115200, Data bits: 8, even parity.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 5.5: Change set parameters: Baud: 115200, Stop bit: 1, odd parity, Data to send: ‘HI’.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 5.6: Monitor the channel in the logic analyzer. Use UART as decoder. Set Baud: 115200, Data bits: 8, odd parity.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 6: SPI
        * Step 6.1: Disable DIO 0. Enable and select DIO5 to DIO7 to create a 3-channel group. Change pattern to SPI. Set the following parameters: Bytes per frame: 2, inter frame space: 3, Data: ABCD1234.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 6.2: Monitor the channel through logic analyzer. Use SPI as decoder. Refer to steps resources picture for the configuration of logic analyzer.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 6.3: Change the following parameters: Bytes per frame: 1, inter frame space: 4, Data: ABCD1234.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 6.4: Monitor the channel through logic analyzer. Use SPI as decoder. Refer to steps resources picture for the configuration of logic analyzer.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 7: I2C
        * Step 7.1: Dissolve current group channel. Enable and select DIO0 and DIO1 to create a 2-channel group. Change pattern to I2C. Set the following parameters: Address: 72, Inter frame space: 3, Data: ABCD1234.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 7.2: Monitor the channel through logic analyzer. Use I2C as decoder. Refer to steps resources picture for the configuration of logic analyzer.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 8: Pulse Pattern
        * Step 8.1: Change pattern to Pulse Pattern. Set the following parameters: Low: 5, High: 1, Counter Init: 0, Delay: 10, Number of Pulses: 5.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 8.2: Monitor the channels through logic analyzer. Refer to steps resources picture for the configuration of logic analyzer.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).


Test 3: Simultaneous Group and Individual Channels Operation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.M2K.PG.SIMULTANEOUS_GROUP_AND_INDIVIDUAL_CHANNELS_OPERATION

Description:
        - This test case verifies the functionality of the pattern generator in simultaneous group and individual channels operation mode.

OS:
        - any

Steps:
        * Step 1: Checking Group and Individual Channels Simultaneously: Use PP as output.
        * Step 1.1: Enable and select channels DIO0 to DIO3 to create 4-channel group. Change group pattern to Binary Counter with frequency set to 5 KHz. Enable DIO4 channel and set as clock with frequency of 5 KHz.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 1.2: Monitor DIO4 through oscilloscope. And at the same time monitor the group channel through logic analyzer.
                * Expected Result: On logic analyzer, the plot should resemble the plot seen in pattern generator, the group channel as well as the individual channel DIO4. On oscilloscope, frequency can be viewed by enabling measurement feature, frequency: 5KHz.
        * Step 1.3: Do not dissolve group channel. Add another group channel. Enable and select DIO5, create a 1-channel group for UART. Change pattern to UART. Baud: 2400, stop bit: 1, no parity, Data: ‘HI’. Also, individual DIO4 channel remains enabled.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 1.4: Monitor the 2 groups and DIO4 through logic analyzer.
                * Expected Result: On logic analyzer, the plot should resemble the plot seen in pattern generator.
        * Step 1.5: Do not dissolve group channels. Disable Group UART. Add another group channel. Enable and select DIO6 to DIO9, create a 4-channel group. Change pattern to Gray Counter. Frequency: 10 KHz. Name this group as Group GC. Also, individual DIO4 channel remains enabled.
                * Expected Result: The interface should look like in the “Step Resources” picture (left side).
        * Step 1.6: Monitor the 2 groups and DIO4 through logic analyzer.
                * Expected Result: On logic analyzer, the plot should resemble the plot seen in pattern generator.


Test 4: Other Features
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.M2K.PG.OTHER_FEATURES

Description:
        - This test case verifies the functionality of the pattern generator in other features.

OS:
        - any

Steps:
        * Step 1: Checking UI: Changing Channel Name
        * Step 1.1: Open individual channel DIO. On its channel manager, modify its name to ‘CH 0’.
                * Expected Result: The name should change as shown in steps resources picture.
        * Step 1.2: Enable DIO 1 and change its name to 'CH 1'. Create a group with 'CH 0' and 'CH 1'.
                * Expected Result: The list of names under the group should also correspond to the names of the channels as should change as shown in steps resources picture.
        * Step 2: Trace Height
        * Step 2.1: Open channel ‘CH 0’. On its channel manager, change trace height to 50.
                * Expected Result: The trace height should now be twice as shown in steps resources picture, compared to previous.
        * Step 2.2: Change height again to 10.
                * Expected Result: The height should now be lower as shown in steps resources picture
        * Step 3: Knobs
        * Step 3.1: Checking frequency knob. Set the knob to large increment. No orange dot on the center. Change frequency value using the ± button.
                * Expected Result: The frequency value should change accordingly with a high increment/decrement from 5 KHz to 10 KHz.
        * Step 3.2: Set the knob to ±1 unit interval. With orange dot on the center. Change frequency value using the ± button.
                * Expected Result: The frequency value should change accordingly with ±1 unit interval.
        * Step 4: Checking the output: PP mode
        * Step 4.1: Connect the DIO0 to oscilloscope ch1+, and oscilloscope ch1- to gnd. This is to monitor the output from the pattern generator.
        * Step 4.2: Enable DIO0 in pattern generator. Set pattern to clock with 5 kHz frequency. Set output as PP. Run instrument and monitor on Oscilloscope.
                * Expected Result: The oscilloscope should show clock pulses from logic 0 to 1. It should look like in steps resources picture.
        * Step 4.3: Try other patterns such as random pattern and monitor on oscilloscope.
                * Expected Result: The oscilloscope should show random pulses from logic 0 to 1. It should look like in steps resources picture.
        * Step 4.4: Repeat steps 4.2 and 4.3 for all channels
        * Step 5: OD mode
        * Step 5.1: Change output to OD. Monitor output in oscilloscope.
                * Expected Result: Oscilloscope should only show logic 0 since output is now in OD mode.
        * Step 5.2: Do 5.1 to other channels.
        * Step 5.3: To output two logic levels when operating in OD, a pull up resistor is needed. Connect the breadboard connection shown in steps resources.
        * Step 5.4: Set power supply to 5V. Run power supply, pattern generator and monitor in oscilloscope.
                * Expected Result: The trace should show two logic levels, with a few mV offset. When power supply is turned off, the oscilloscope should show only logic 0.
        * Step 5.5: Repeat step 5.3 and 5.4 for all channels.
        * Step 6: Print
        * Step 6.1: Click on Print button and save file as sample.pdf
                * Expected Result: Upon saving, the prompt window should look like the steps resources picture.
        * Step 6.2: Open the saved file.
                * Expected Result: The file should show the waveform that you have saved.
        * Step 7: See more info
        * Step 7.1: Click the 'See more info' icon on the upper left of the pattern generator window.
                * Expected Result: It should lead to the wiki page of pattern generator.

