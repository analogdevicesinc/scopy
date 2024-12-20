.. _logic_analyzer_tests:

Logic Analyzer - Test Cases
===========================

In order to proceed through the test case, first of all delete the Scopy *.ini file (saves previous settings made in Scopy tool) from the following path on Windows: ``C:\Users\your_username\AppData\Roaming\ADI``.

Open the Logic Analyzer instrument. The interface should look like the picture below:

Press multiple times on the “Run” button to check if the instrument works.

Test Title
----------

A. Individual Pin Operation
----------------------------

Description
-----------

Checking Channel's Trigger Function
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Enable channel DIO0 and DIO1 on Logic analyzer, on the settings menu set sample rate to 50ksps and add a delay ex. -142 samples. On Pattern generator, enable DIO1, set the following parameter: Pattern: clock, Frequency: 100Hz, Phase: 0 degrees and Duty Cycle: 50%. On Digital IO instrument, set DIO0 as output.
    - The interface should look like in the “Step Resources” picture (left side). Additional signals from the pattern generator and Logic analyzer from DIO2 to DIO15 at different frequencies will be better.

Rising edge trigger
~~~~~~~~~~~~~~~~~~~

2. Set DIO0’s trigger to rising edge configuration, run Digital IO, pattern generator and logic analyzer instrument. Change DIO0’s output from 0 to 1 from the Digital IO’s instrument.
    - The logic analyzer should initiate a capture.

Falling edge trigger
~~~~~~~~~~~~~~~~~~~~~

3. Set DIO0’s trigger to falling edge configuration and run both pattern generator and logic analyzer. Change DIO0’s output from 1 to 0 from the Digital IO’s instrument.
    - The logic analyzer should initiate a capture.

Any edge trigger
~~~~~~~~~~~~~~~~

4. Set DIO0’s trigger to any edge configuration and run both pattern generator and logic analyzer. Change DIO0’s output either from 1 to 0 or 0 to 1 from the Digital IO’s instrument.
    - The logic analyzer should initiate a capture.

Low trigger
~~~~~~~~~~~

5. Set DIO0’s Low configuration and run both pattern generator and logic analyzer. Set DIO0’s output to 0.
    - The logic analyzer should continuously capture the signal.

High Trigger
~~~~~~~~~~~~

6. Set DIO0’s trigger to high configuration and run both pattern generator and logic analyzer. Set DIO0’s output to 1.
    - The logic analyzer should continuously capture the signal.

7. Repeat steps 1 to 6 using DIO1 to DIO15 as triggers in the logic analyzer and Digital IO instrument and the remaining DIO channel as the signal to be captured of using Pattern generator.
    - The results should all be the same.

Testing External Triggers
~~~~~~~~~~~~~~~~~~~~~~~~~

1. On Digital IO instrument set DIO0 as output. No changes on Pattern Generator. On Logic Analyzer’s trigger menu, turn on the External trigger. Select source as External Trigger In.
    - The interface should look like in the “Step Resources” pictures. Turning on the External Trigger should automatically turn off the triggers set on every DIO channels if there are any.

2. Connect Trigger in 1 to DIO0.

Rising Edge Trigger
~~~~~~~~~~~~~~~~~~~

3. Set External 1’s trigger to rising edge configuration, run Digital IO, pattern generator and logic analyzer instrument. Change DIO0’s output from 0 to 1 from the Digital IO instrument.
    - The logic analyzer should initiate a capture.

Falling Edge Trigger
~~~~~~~~~~~~~~~~~~~~~

4. Set External 1’s trigger to falling edge configuration, run Digital IO, pattern generator and logic analyzer instrument. Change DIO0’s output from 1 to 0 from the Digital IO instrument.
    - The logic analyzer should initiate a capture.

Any Edge Trigger
~~~~~~~~~~~~~~~~

5. Set External 1’s trigger to any edge configuration, run Digital IO, pattern generator and logic analyzer instrument. Change DIO0’s output from 1 to 0 or 0 to 1 from the Digital IO instrument.
    - The logic analyzer should initiate a capture.

Low Trigger
~~~~~~~~~~~

6. Set External 1’s trigger to low trigger configuration, run Digital IO, pattern generator and logic analyzer instrument. Set DIO0’s output to 0 from the Digital IO instrument.
    - The logic analyzer should continuously capture the signals.

High Trigger
~~~~~~~~~~~~

7. Set External 1’s trigger to high trigger configuration, run Digital IO, pattern generator and logic analyzer instrument. Set DIO0’s output to 1 from the Digital IO instrument.
    - The logic analyzer should continuously capture the signals.

Testing Oscilloscope Source External Trigger
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

8. In Logic analyzer enable External trigger and set Source to Oscilloscope.
    - The logic analyzer should initiate a capture when the Oscilloscope is triggered.

9. Connect W1 to 1+ and GND to 1- and generate a Sine Waveform with 2V peak-to-peak amplitude and 200Hz frequency. Set the Oscilloscope trigger to normal and Condition to Rising Edge.
    - The Oscilloscope should be triggered when the two blue Trigger cursors are intersected on the rising edge of the signal.

10. Run Signal Generator, Oscilloscope and Logic analyzer and verify if the Logic analyzer is triggered at the same time with the Oscilloscope.
    - If you drag the horizontal cursor in the Oscilloscope window above or below the signal, it should be in Waiting state, and Logic analyzer will be Waiting too.

11. Repeat steps 9-10 for each condition of the trigger available in Oscilloscope.
    - Logic Analyzer and Oscilloscope should capture signals simultaneously and be in Waiting state at the same time.

Testing the Trigger Modes
~~~~~~~~~~~~~~~~~~~~~~~~~

1. On Pattern Generator, enable DIO2 and then set it to clock pattern 5KHz frequency. On Logic Analyzer, enable DIO0, DIO1, and DIO2. Set DIO0 and DIO1 trigger to both HIGH. Disable External trigger.
    - The interface should look like in the “Step Resources” pictures. Turning on the External Trigger should automatically turn off the triggers set on every DIO channels if there are any.

2. On Digital IO instrument set DIO0 and DIO1 as output.

OR Mode
~~~~~~~

3. On Logic Analyzer’s trigger configuration, set the trigger logic to OR. On Digital IO instrument, Set DIO0’s output to 0 and DIO1’s output to 0. Run Digital IO, Pattern Generator and Logic Analyzer Instrument.
    - The logic analyzer should be on waiting mode and not capture any signal.

4. On Digital IO instrument, Set DIO0 or DIO1’s output to 1.
    - The logic analyzer should start capturing signal when either of the DIO0 or DIO1 is HIGH.

AND Mode
~~~~~~~~

5. On Logic Analyzer’s trigger configuration, set the trigger logic to AND. On Digital IO instrument, Set DIO0’s output to 0 and DIO1’s output to 1. Run Digital IO, Pattern Generator and Logic Analyzer Instrument.
    - The logic analyzer should be on waiting mode and not capture any signal.

6. On Digital IO instrument, Set DIO0’s and DIO1’s output to 1.
    - The logic analyzer should start capturing signal only when DIO0 and DIO1 are HIGH.

Checking Channel's Clock Signal Measurement Accuracy
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Enable channel DIO0, on the settings menu set sample rate to 50ksps, enable the cursor. On Pattern generator’s DIO0, set the following parameter: Pattern: clock, Frequency: 100Hz, Phase: 0 degrees and Duty Cycle: 50%.
    - Refer to the step resource picture.

2. Move the cursors to the consecutive rising edges or consecutive falling edges.
    - The data measured by the cursor should be close to ∆t: 10ms and 1/∆t: 100Hz.

3. Enable Lock cursor feature and measure the next edges.
    - The data measured by the cursor should be close to ∆t: 10ms and 1/∆t: 100Hz.

4. Set logic analyzer’s settings to sample rate: 100Msps, position: 0s. Set pattern generator DIO0’s parameters to: Pattern: clock, Frequency: 2.5MHz, Phase: 0 degrees and Duty Cycle: 50%.
    - Refer to the step resource picture.

5. Move the cursors to the consecutive rising edges or consecutive falling edges.
    - The data measured by the cursor should be close to ∆t: 400ns and 1/∆t: 2.5MHz.

6. Enable Lock cursor feature and measure the next edges.
    - The data measured by the cursor should be close to ∆t: 400ns and 1/∆t: 2.5MHz.

7. Set logic analyzer’s settings to sample rate: 20ksps. Set pattern generator DIO0’s parameters to: Pattern: clock, Frequency: 100Hz, Phase: 0 degrees and Duty Cycle: 70%.
    - Refer to the step resource.

8. Move the cursors to the rising and falling edge of the upper limit.
    - The data measured by the cursor should be close to ∆t: 7ms.

9. Enable Lock cursor feature and measure the next edges.
    - The data measured by the cursor should be close to ∆t: 7ms.

10. Move the cursors to the falling and rising edge of the lower limit.
    - The data measured by the cursor should be close to ∆t: 3ms.

11. Enable Lock cursor feature and measure the next edges.
    - The data measured by the cursor should be close to ∆t: 3ms.

12. Repeat steps 1 to 11 using DIO1 to DIO15 of both pattern generator and logic analyzer.
    - The results should all be the same.

B. Group Operation
-------------------

Description
-----------

Parallel Mode
~~~~~~~~~~~~~

1. In logic analyzer, add a parallel decoder. Set Clock line to DIO8 and set Data lines 0-7 to DIO0 to DIO7. In pattern generator, group DIO0 to DIO7 set as number pattern. Enable DIO8 and set to Clock with 500Hz frequency.
    - The interface should look like in the “Step Resources” picture (left side).

2. Run the Pattern Generator and Logic analyzer instrument. Set in the pattern generator’s instrument the desired decimal value.
    - On the logic analyzer plot, the reading is in hex format. For reference, 50 decimal = 32 hex, 100 decimal = 64 hex and 250 decimal = FA.

3. In logic analyzer, add another parallel decoder. Set Clock line to DIO8 and set Data lines 0-6 to DIO9 to DIO15. In pattern generator, group DIO9 to DIO15 set as number pattern.
    - The interface should look like in the “Step Resources” picture (left side).

4. Run the Pattern Generator and Logic analyzer instrument. Set in the pattern generator’s instrument the desired decimal value of both groups.
    - On the logic analyzer plot, the reading is in hex format. For reference, 50 decimal = 32 hex, 100 decimal = 64 hex and 250 decimal = FA.

SPI Mode
~~~~~~~~

5. In logic analyzer set sample rate: 50ksps, group DIO0 to DIO3. Add an SPI decoder, set DIO0: CLK, DIO1: MISO, DIO2: CS# and DIO3: MOSI, set DIO2’s trigger to falling edge. In the pattern generator group DIO0 to DIO2, set Pattern: SPI, Frequency: 5kHz, Bytes per frame: 2, Interframe space 4, Data: insert 4 bytes in hex form e.g: AB CD EF 15.
    - The interface should look like in the “Step Resources” picture (left side).

6. Connect DIO1 to DIO3.

7. Run Pattern Generator and Logic analyzer.
    - In the logic analyzer the plotted MISO and MOSI data should have a similar data and 2 bytes per frame.

8. Repeat step 5 to 7 using different groups in the logic analyzer, DIO4:DIO7, DIO8:DIO11 and DIO12:DIO15.
    - The results should be the same.

UART Mode
~~~~~~~~~

9. In logic analyzer set time base: 1ms, group DIO0 to DIO1, set DIO0: TX and DIO1: RX, set DIO0’s trigger to falling edge, set Baud rate: 9600, data bits: 8, parity type: none, stop bits: 1.0, bit order: lsb-first, data format: ASCII. In the pattern generator, group DIO0, set Pattern: UART, Baud: 9600, Stop bit: 1, Parity: none.
    - The interface should look like in the “Step Resources” picture (left side).

10. Connect DIO0 to DIO1.

11. Run the Pattern Generator, set in the pattern generator’s instrument the desired ASCII value.
    - On the logic analyzer plot, the TX and RX data should be the same.

12. In step 9, change the Baud rate to 115200 for both pattern generator and logic analyzer.
    - Refer to the step resource picture.

13. Repeat step 9 to 12 using different groups in the logic analyzer, DIO1:DIO2, DIO2:DIO3 . . . . . DIO14:DIO15.
    - The results should be the same.

PWM Mode
~~~~~~~~

14. In Logic Analyzer, Group DIO0, Set DIO0: Data, Polarity: active-high. In pattern generator, set pattern: Clock, Frequency: 10Hz.
    - Refer to the step resource image.

15. Run pattern generator and logic analyzer. Set the duty cycle of the clock signal to 5%, 30%, 50%, 75% and 95%.
    - The Data should follow the phase degrees set in the pattern generator.

16. In step 14 change the polarity of the PWM to active-low and repeat step 15.
    - The data should follow (100% - phase shift set).

17. Repeat step 14 to 16 for DIO1. . . .DIO15.
    - The result should be the same.

C. Additional Features
----------------------

Description
-----------

Customizing the channel’s visual representation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Enable channel DIO0, in the menu there are options for name and trace height.
    - Refer to the step resource picture.

2. Change the name using the text box in the setting.
    - The name should change accordingly. See step resource image for reference.

3. Change the trace height of each pin.
    - The height of the signal should change accordingly. See step resource image for reference.

4. Repeat steps 1 to 3 for DIO1 to DIO15.
    - The results should be the same.

Reset instrument
~~~~~~~~~~~~~~~~

5. In the Preferences menu click Reset Scopy.
    - All the changes made from name, thickness and color should return to default. See step resource image for reference.

Testing the Export Functionality
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. In the pattern generator, set two consecutive channel with similar configuration: DIO0 and DIO1: Pattern: Clock, Frequency: 100Hz, Phase: 0 and Duty Cycle: 50%. DIO2 and DIO3: Pattern: Clock, Frequency: 300Hz, Phase: 0 and Duty Cycle: 50%. DIO4 and DIO5: Pattern: Clock, Frequency: 500Hz, Phase: 0 and Duty Cycle: 50%. DIO6 and DIO7: Pattern: Clock, Frequency: 150Hz, Phase: 0 and Duty Cycle: 80%. DIO8 and DIO9: Pattern: Clock, Frequency: 200Hz, Phase: 0 and Duty Cycle: 20%. Group DIO10 to DIO15: Pattern: Number Pattern and Data: 50. Run Logic Analyzer and Pattern Generator.
    - Refer to the step resource picture.

2. On the settings menu, click export button.
    - A File menu box should appear allowing the user to choose the file destination.

3. On the settings menu, click export button.
    - A File menu box should appear allowing the user to choose the file destination.

4. Choose the desired file destination and save the file on all formats. Choose the desired configuration on different file formats.
    - The files should appear in the specified folder.

Print Plot
~~~~~~~~~~

1. In logic analyzer, group DIO0 to DIO7, set DIO0 : DIO0 . . . . DIO7:DIO7, set time base: 5ms. In pattern generator, group DIO0 to DIO7 set as binary counter. Run both instruments. In the Logic Analyzer, press “Print” button and save the file.
    - The file obtained should be similar to:

Decoder Table
~~~~~~~~~~~~~

1. In pattern generator add and set channel 0 to UART with Baud rate “9600” and Data to Send “123”. In logic analyzer add UART decoder with RX on channel 0, Baud rate “9600” and Data format “ascii”. In general settings set Sample Rate 1Msps and Nr of samples to 10k samples. Run pattern generator and logic analyzer.
    - Plot should look like the resource image.

2. Open decoder table, open settings. Set Group by “RX data”, Group size “3”, disable all rows except for “RX data” in Filter.
    - This should result in each table row having 3 annotations of RX data (only count the ones directly under the sample/time info box).

3. Set Group offset to each multiple of 3 (the input should automatically top out at the total number RX data annotations). For each offset, search for "^3$" (make sure to press enter in the search box after each offset change).
    - Every time there should be at least one “3” value annotation on each table row and look similar to the resource image.

4. Set the offset to “9”, click on each table row and double click annotations on plot. You can zoom out by right clicking the plot.
    - Clicking rows should zoom in the plot, centering it on the equivalent group. Double clicking on the plot should scroll and select the equivalent table row annotation (this will reset the Group by combo box).

5. Add a Gray code decoder and set Sample Rate to 1Msps and Nr of samples to 4M samples. Run pattern generator and logic analyzer and open decoder table settings.
    - The decoder table settings will be disabled and a “waiting for plot …” label should appear while the plot is loading.

6. In settings set Decoder to “Gray code”, Group by “Increments”, Group size “3”, Group offset “1”, Filter out all except for Phases and press Export. Perform 2 exports, using .csv and .txt formats.
    - “Logic Analyzer - Test Case
==========================

Initial Setup
-------------

In order to proceed through the test case, first of all delete the Scopy *.ini file (saves previous settings made in Scopy tool) from the following path on Windows: ``C:\Users\your_username\AppData\Roaming\ADI``.

Open the Logic Analyzer instrument. The interface should look like the picture below:

Press multiple times on the “Run” button to check if the instrument works.

Test Title
----------

A. Individual Pin Operation
----------------------------

Description
-----------

Checking Channel's Trigger Function
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Enable channel DIO0 and DIO1 on Logic analyzer, on the settings menu set sample rate to 50ksps and add a delay ex. -142 samples. On Pattern generator, enable DIO1, set the following parameter: Pattern: clock, Frequency: 100Hz, Phase: 0 degrees and Duty Cycle: 50%. On Digital IO instrument, set DIO0 as output.
    - The interface should look like in the “Step Resources” picture (left side). Additional signals from the pattern generator and Logic analyzer from DIO2 to DIO15 at different frequencies will be better.

Rising edge trigger
~~~~~~~~~~~~~~~~~~~

2. Set DIO0’s trigger to rising edge configuration, run Digital IO, pattern generator and logic analyzer instrument. Change DIO0’s output from 0 to 1 from the Digital IO’s instrument.
    - The logic analyzer should initiate a capture.

Falling edge trigger
~~~~~~~~~~~~~~~~~~~~~

3. Set DIO0’s trigger to falling edge configuration and run both pattern generator and logic analyzer. Change DIO0’s output from 1 to 0 from the Digital IO’s instrument.
    - The logic analyzer should initiate a capture.

Any edge trigger
~~~~~~~~~~~~~~~~

4. Set DIO0’s trigger to any edge configuration and run both pattern generator and logic analyzer. Change DIO0’s output either from 1 to 0 or 0 to 1 from the Digital IO’s instrument.
    - The logic analyzer should initiate a capture.

Low trigger
~~~~~~~~~~~

5. Set DIO0’s Low configuration and run both pattern generator and logic analyzer. Set DIO0’s output to 0.
    - The logic analyzer should continuously capture the signal.

High Trigger
~~~~~~~~~~~~

6. Set DIO0’s trigger to high configuration and run both pattern generator and logic analyzer. Set DIO0’s output to 1.
    - The logic analyzer should continuously capture the signal.

7. Repeat steps 1 to 6 using DIO1 to DIO15 as triggers in the logic analyzer and Digital IO instrument and the remaining DIO channel as the signal to be captured of using Pattern generator.
    - The results should all be the same.

Testing External Triggers
~~~~~~~~~~~~~~~~~~~~~~~~~

1. On Digital IO instrument set DIO0 as output. No changes on Pattern Generator. On Logic Analyzer’s trigger menu, turn on the External trigger. Select source as External Trigger In.
    - The interface should look like in the “Step Resources” pictures. Turning on the External Trigger should automatically turn off the triggers set on every DIO channels if there are any.

2. Connect Trigger in 1 to DIO0.

Rising Edge Trigger
~~~~~~~~~~~~~~~~~~~

3. Set External 1’s trigger to rising edge configuration, run Digital IO, pattern generator and logic analyzer instrument. Change DIO0’s output from 0 to 1 from the Digital IO instrument.
    - The logic analyzer should initiate a capture.

Falling Edge Trigger
~~~~~~~~~~~~~~~~~~~~~

4. Set External 1’s trigger to falling edge configuration, run Digital IO, pattern generator and logic analyzer instrument. Change DIO0’s output from 1 to 0 from the Digital IO instrument.
    - The logic analyzer should initiate a capture.

Any Edge Trigger
~~~~~~~~~~~~~~~~

5. Set External 1’s trigger to any edge configuration, run Digital IO, pattern generator and logic analyzer instrument. Change DIO0’s output from 1 to 0 or 0 to 1 from the Digital IO instrument.
    - The logic analyzer should initiate a capture.

Low Trigger
~~~~~~~~~~~

6. Set External 1’s trigger to low trigger configuration, run Digital IO, pattern generator and logic analyzer instrument. Set DIO0’s output to 0 from the Digital IO instrument.
    - The logic analyzer should continuously capture the signals.

High Trigger
~~~~~~~~~~~~

7. Set External 1’s trigger to high trigger configuration, run Digital IO, pattern generator and logic analyzer instrument. Set DIO0’s output to 1 from the Digital IO instrument.
    - The logic analyzer should continuously capture the signals.

Testing Oscilloscope Source External Trigger
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

8. In Logic analyzer enable External trigger and set Source to Oscilloscope.
    - The logic analyzer should initiate a capture when the Oscilloscope is triggered.

9. Connect W1 to 1+ and GND to 1- and generate a Sine Waveform with 2V peak-to-peak amplitude and 200Hz frequency. Set the Oscilloscope trigger to normal and Condition to Rising Edge.
    - The Oscilloscope should be triggered when the two blue Trigger cursors are intersected on the rising edge of the signal.

10. Run Signal Generator, Oscilloscope and Logic analyzer and verify if the Logic analyzer is triggered at the same time with the Oscilloscope.
    - If you drag the horizontal cursor in the Oscilloscope window above or below the signal, it should be in Waiting state, and Logic analyzer will be Waiting too.

11. Repeat steps 9-10 for each condition of the trigger available in Oscilloscope.
    - Logic Analyzer and Oscilloscope should capture signals simultaneously and be in Waiting state at the same time.

Testing the Trigger Modes
~~~~~~~~~~~~~~~~~~~~~~~~~

1. On Pattern Generator, enable DIO2 and then set it to clock pattern 5KHz frequency. On Logic Analyzer, enable DIO0, DIO1, and DIO2. Set DIO0 and DIO1 trigger to both HIGH. Disable External trigger.
    - The interface should look like in the “Step Resources” pictures. Turning on the External Trigger should automatically turn off the triggers set on every DIO channels if there are any.

2. On Digital IO instrument set DIO0 and DIO1 as output.

OR Mode
~~~~~~~

3. On Logic Analyzer’s trigger configuration, set the trigger logic to OR. On Digital IO instrument, Set DIO0’s output to 0 and DIO1’s output to 0. Run Digital IO, Pattern Generator and Logic Analyzer Instrument.
    - The logic analyzer should be on waiting mode and not capture any signal.

4. On Digital IO instrument, Set DIO0 or DIO1’s output to 1.
    - The logic analyzer should start capturing signal when either of the DIO0 or DIO1 is HIGH.

AND Mode
~~~~~~~~

5. On Logic Analyzer’s trigger configuration, set the trigger logic to AND. On Digital IO instrument, Set DIO0’s output to 0 and DIO1’s output to 1. Run Digital IO, Pattern Generator and Logic Analyzer Instrument.
    - The logic analyzer should be on waiting mode and not capture any signal.

6. On Digital IO instrument, Set DIO0’s and DIO1’s output to 1.
    - The logic analyzer should start capturing signal only when DIO0 and DIO1 are HIGH.

Checking Channel's Clock Signal Measurement Accuracy
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Enable channel DIO0, on the settings menu set sample rate to 50ksps, enable the cursor. On Pattern generator’s DIO0, set the following parameter: Pattern: clock, Frequency: 100Hz, Phase: 0 degrees and Duty Cycle: 50%.
    - Refer to the step resource picture.

2. Move the cursors to the consecutive rising edges or consecutive falling edges.
    - The data measured by the cursor should be close to ∆t: 10ms and 1/∆t: 100Hz.

3. Enable Lock cursor feature and measure the next edges.
    - The data measured by the cursor should be close to ∆t: 10ms and 1/∆t: 100Hz.

4. Set logic analyzer’s settings to sample rate: 100Msps, position: 0s. Set pattern generator DIO0’s parameters to: Pattern: clock, Frequency: 2.5MHz, Phase: 0 degrees and Duty Cycle: 50%.
    - Refer to the step resource picture.

5. Move the cursors to the consecutive rising edges or consecutive falling edges.
    - The data measured by the cursor should be close to ∆t: 400ns and 1/∆t: 2.5MHz.

6. Enable Lock cursor feature and measure the next edges.
    - The data measured by the cursor should be close to ∆t: 400ns and 1/∆t: 2.5MHz.

7. Set logic analyzer’s settings to sample rate: 20ksps. Set pattern generator DIO0’s parameters to: Pattern: clock, Frequency: 100Hz, Phase: 0 degrees and Duty Cycle: 70%.
    - Refer to the step resource.

8. Move the cursors to the rising and falling edge of the upper limit.
    - The data measured by the cursor should be close to ∆t: 7ms.

9. Enable Lock cursor feature and measure the next edges.
    - The data measured by the cursor should be close to ∆t: 7ms.

10. Move the cursors to the falling and rising edge of the lower limit.
    - The data measured by the cursor should be close to ∆t: 3ms.

11. Enable Lock cursor feature and measure the next edges.
    - The data measured by the cursor should be close to ∆t: 3ms.

12. Repeat steps 1 to 11 using DIO1 to DIO15 of both pattern generator and logic analyzer.
    - The results should all be the same.

B. Group Operation
-------------------

Description
-----------

Parallel Mode
~~~~~~~~~~~~~

1. In logic analyzer, add a parallel decoder. Set Clock line to DIO8 and set Data lines 0-7 to DIO0 to DIO7. In pattern generator, group DIO0 to DIO7 set as number pattern. Enable DIO8 and set to Clock with 500Hz frequency.
    - The interface should look like in the “Step Resources” picture (left side).

2. Run the Pattern Generator and Logic analyzer instrument. Set in the pattern generator’s instrument the desired decimal value.
    - On the logic analyzer plot, the reading is in hex format. For reference, 50 decimal = 32 hex, 100 decimal = 64 hex and 250 decimal = FA.

3. In logic analyzer, add another parallel decoder. Set Clock line to DIO8 and set Data lines 0-6 to DIO9 to DIO15. In pattern generator, group DIO9 to DIO15 set as number pattern.
    - The interface should look like in the “Step Resources” picture (left side).

4. Run the Pattern Generator and Logic analyzer instrument. Set in the pattern generator’s instrument the desired decimal value of both groups.
    - On the logic analyzer plot, the reading is in hex format. For reference, 50 decimal = 32 hex, 100 decimal = 64 hex and 250 decimal = FA.

SPI Mode
~~~~~~~~

5. In logic analyzer set sample rate: 50ksps, group DIO0 to DIO3. Add an SPI decoder, set DIO0: CLK, DIO1: MISO, DIO2: CS# and DIO3: MOSI, set DIO2’s trigger to falling edge. In the pattern generator group DIO0 to DIO2, set Pattern: SPI, Frequency: 5kHz, Bytes per frame: 2, Interframe space 4, Data: insert 4 bytes in hex form e.g: AB CD EF 15.
    - The interface should look like in the “Step Resources” picture (left side).

6. Connect DIO1 to DIO3.

7. Run Pattern Generator and Logic analyzer.
    - In the logic analyzer the plotted MISO and MOSI data should have a similar data and 2 bytes per frame.

8. Repeat step 5 to 7 using different groups in the logic analyzer, DIO4:DIO7, DIO8:DIO11 and DIO12:DIO15.
    - The results should be the same.

UART Mode
~~~~~~~~~

9. In logic analyzer set time base: 1ms, group DIO0 to DIO1, set DIO0: TX and DIO1: RX, set DIO0’s trigger to falling edge, set Baud rate: 9600, data bits: 8, parity type: none, stop bits: 1.0, bit order: lsb-first, data format: ASCII. In the pattern generator, group DIO0, set Pattern: UART, Baud: 9600, Stop bit: 1, Parity: none.
    - The interface should look like in the “Step Resources” picture (left side).

10. Connect DIO0 to DIO1.

11. Run the Pattern Generator, set in the pattern generator’s instrument the desired ASCII value.
    - On the logic analyzer plot, the TX and RX data should be the same.

12. In step 9, change the Baud rate to 115200 for both pattern generator and logic analyzer.
    - Refer to the step resource picture.

13. Repeat step 9 to 12 using different groups in the logic analyzer, DIO1:DIO2, DIO2:DIO3 . . . . . DIO14:DIO15.
    - The results should be the same.

PWM Mode
~~~~~~~~

14. In Logic Analyzer, Group DIO0, Set DIO0: Data, Polarity: active-high. In pattern generator, set pattern: Clock, Frequency: 10Hz.
    - Refer to the step resource image.

15. Run pattern generator and logic analyzer. Set the duty cycle of the clock signal to 5%, 30%, 50%, 75% and 95%.
    - The Data should follow the phase degrees set in the pattern generator.

16. In step 14 change the polarity of the PWM to active-low and repeat step 15.
    - The data should follow (100% - phase shift set).

17. Repeat step 14 to 16 for DIO1. . . .DIO15.
    - The result should be the same.

C. Additional Features
----------------------

Description
-----------

Customizing the channel’s visual representation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Enable channel DIO0, in the menu there are options for name and trace height.
    - Refer to the step resource picture.

2. Change the name using the text box in the setting.
    - The name should change accordingly. See step resource image for reference.

3. Change the trace height of each pin.
    - The height of the signal should change accordingly. See step resource image for reference.

4. Repeat steps 1 to 3 for DIO1 to DIO15.
    - The results should be the same.

Reset instrument
~~~~~~~~~~~~~~~~

5. In the Preferences menu click Reset Scopy.
    - All the changes made from name, thickness and color should return to default. See step resource image for reference.

Testing the Export Functionality
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. In the pattern generator, set two consecutive channel with similar configuration: DIO0 and DIO1: Pattern: Clock, Frequency: 100Hz, Phase: 0 and Duty Cycle: 50%. DIO2 and DIO3: Pattern: Clock, Frequency: 300Hz, Phase: 0 and Duty Cycle: 50%. DIO4 and DIO5: Pattern: Clock, Frequency: 500Hz, Phase: 0 and Duty Cycle: 50%. DIO6 and DIO7: Pattern: Clock, Frequency: 150Hz, Phase: 0 and Duty Cycle: 80%. DIO8 and DIO9: Pattern: Clock, Frequency: 200Hz, Phase: 0 and Duty Cycle: 20%. Group DIO10 to DIO15: Pattern: Number Pattern and Data: 50. Run Logic Analyzer and Pattern Generator.
    - Refer to the step resource picture.

2. On the settings menu, click export button.
    - A File menu box should appear allowing the user to choose the file destination.

3. On the settings menu, click export button.
    - A File menu box should appear allowing the user to choose the file destination.

4. Choose the desired file destination and save the file on all formats. Choose the desired configuration on different file formats.
    - The files should appear in the specified folder.

Print Plot
~~~~~~~~~~

1. In logic analyzer, group DIO0 to DIO7, set DIO0 : DIO0 . . . . DIO7:DIO7, set time base: 5ms. In pattern generator, group DIO0 to DIO7 set as binary counter. Run both instruments. In the Logic Analyzer, press “Print” button and save the file.
    - The file obtained should be similar to:

Decoder Table
~~~~~~~~~~~~~

1. In pattern generator add and set channel 0 to UART with Baud rate “9600” and Data to Send “123”. In logic analyzer add UART decoder with RX on channel 0, Baud rate “9600” and Data format “ascii”. In general settings set Sample Rate 1Msps and Nr of samples to 10k samples. Run pattern generator and logic analyzer.
    - Plot should look like the resource image.

2. Open decoder table, open settings. Set Group by “RX data”, Group size “3”, disable all rows except for “RX data” in Filter.
    - This should result in each table row having 3 annotations of RX data (only count the ones directly under the sample/time info box).

3. Set Group offset to each multiple of 3 (the input should automatically top out at the total number RX data annotations). For each offset, search for "^3$" (make sure to press enter in the search box after each offset change).
    - Every time there should be at least one “3” value annotation on each table row and look similar to the resource image.

4. Set the offset to “9”, click on each table row and double click annotations on plot. You can zoom out by right clicking the plot.
    - Clicking rows should zoom in the plot, centering it on the equivalent group. Double clicking on the plot should scroll and select the equivalent table row annotation (this will reset the Group by combo box).

5. Add a Gray code decoder and set Sample Rate to 1Msps and Nr of samples to 4M samples. Run pattern generator and logic analyzer and open decoder table settings.
    - The decoder table settings will be disabled and a “waiting for plot …” label should appear while the plot is loading.

6. In settings set Decoder to “Gray code”, Group by “Increments”, Group size “3”, Group offset “1”, Filter out all except for Phases and press Export. Perform 2 exports, using .csv and .txt formats.
    - “