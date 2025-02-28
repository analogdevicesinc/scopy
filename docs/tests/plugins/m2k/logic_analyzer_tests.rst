.. _logic_analyzer_tests:

Logic Analyzer - Test Suite
===========================

.. note::

   User guide: :ref:`Logic Analyzer user guide<logic_analyzer>`

The Logic Analyzer tests are a set of tests that are run to verify the 
Logic Analyzer instrument of the ADALM2000 plugin in Scopy.

The following apply for all the test cases in this suite.
If the test case has special requirements, they will be listed in the test case section.


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

.. _adalm2000_device_setup:

**Adalm2000.Device:**
    - Open Scopy.
    - Connect the ADALM2000 device to the system.
    - Connect the ADALM2000 device in Scopy using the USB/network backend.
    - Connect **W1** to **1+** and **GND** to **1-** using the pinout diagram. 

Depends on:
    - Test TST.PREFS.RESET

Prerequisites:
    - Scopy v2.0.0 or later with ADALM2000 plugin installed on the system.
    - Tests listed as dependencies are successfully completed.
    - Reset .ini files to default using the Preferences "Reset" button.

Test 1 - Channel Trigger Function
---------------------------------------------

.. _TST.LOGIC.CHN_TRIGGER:

**UID:** TST.LOGIC.CHN_TRIGGER

**Description:** This test verifies the basic trigger functionality on individual 
digital channels - rising edge, falling edge, high, low, any edge.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_setup>`
    - OS: ANY

**Prerequisites:**
    - :ref:`Pattern Generator<m2k_pattern_generator_tests>`
    - :ref:`Digital IO<digital_io_tests>`

**Steps:**
    1. Open the Logic Analyzer instrument.
    2. Enable DIO 0 and DIO 1 on the Logic Analyzer.
    3. Set the following parameters:
        - Sample rate to 50ksps.
        - Sample delay of -142 samples.
    4. Open the :ref:`Pattern Generator<pattern_generator>` instrument and enable DIO 1.
       Set the following parameters:
        
        - Pattern: clock.
        - Frequency: 100Hz.
        - Phase: 0 degrees.
        - Duty Cycle: 50%.
    5. Open the :ref:`Digital IO<digitalio>` instrument and set DIO 0 as output.
    6. In the Logic Analyzer set DIO0’s trigger to **rising edge** configuration.
    7. Run the Digital IO, Pattern Generator and Logic Analyzer instrument.
    8. In the Digital IO, change DIO0’s output from 0 to 1.
        - **Expected Result**: The logic analyzer initiates a capture.
        - **Actual Result:**

..
  Actual test result goes here.
..

    9. Stop the Logic Analyzer and set DIO0’s trigger to **rising edge** configuration.
    10. Run the Logic Analyzer instrument.
    11. In the Digital IO, change DIO0’s output from 1 to 0.
         - **Expected Result**: The logic analyzer initiates a capture.
         - **Actual Result:**

..
  Actual test result goes here.
..

    12. Stop the Logic Analyzer and set DIO0’s trigger to **any edge** configuration.
    13. Run the Logic Analyzer instrument.
    14. In the Digital IO, change DIO0’s output from 0 to 1 or 1 to 0.
         - **Expected Result**: The logic analyzer initiates a capture.
         - **Actual Result:**

..
  Actual test result goes here.
..

    15. Stop the Logic Analyzer and set DIO0’s trigger to **low** configuration.
    16. Run the Logic Analyzer instrument.
    17. In the Digital IO, set DIO0’s output to 0.
         - **Expected Result**: The logic analyzer continuously captures the signal.
         - **Actual Result:**

..
  Actual test result goes here.
..

    18. Stop the Logic Analyzer and set DIO0’s trigger to **high** configuration.
    19. Run the Logic Analyzer instrument.
    20. In the Digital IO, set DIO0’s output to 1.
         - **Expected Result**: The logic analyzer continuously captures the signal.
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


Test 2 - External Channel Trigger Function
---------------------------------------------

.. _TST.LOGIC.EXT_CHN_TRIGGER:

**UID:** TST.LOGIC.EXT_CHN_TRIGGER

**Description:** This test verifies the external trigger functionality.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_setup>`
    - OS: ANY

**Prerequisites:**
    - :ref:`Pattern Generator<m2k_pattern_generator_tests>`
    - :ref:`Digital IO<digital_io_tests>`

**Steps:**
    1. Open the Logic Analyzer instrument.
    2. Enable DIO 0 and DIO 1 on the Logic Analyzer.
    3. Set the following parameters:
        - Sample rate to 50ksps.
        - Sample delay of -142 samples.
    4. Open the :ref:`Pattern Generator<pattern_generator>` instrument and enable DIO 1.
       Set the following parameters:
        
        - Pattern: clock.
        - Frequency: 100Hz.
        - Phase: 0 degrees.
        - Duty Cycle: 50%.
    5. Open the :ref:`Digital IO<digitalio>` instrument and set DIO 0 as output.
    6. Open the Logic Analyzer trigger menu and turn on the External trigger.
       Select the source as **External Trigger In**.
        
        - **Expected Result**: Triggers set on every DIO channels are automatically turned off.
        - **Actual Result:**

..
  Actual test result goes here.
..

    7. Connect Trigger in 1 to DIO0 using a loopback cable.
    8. In the Logic Analyzer Trigger settings menu set the External Trigger In condition
       to **rising edge**.
    9. Run the Digital IO, Pattern Generator and Logic Analyzer instrument.
    10. In the Digital IO, change DIO0’s output from 0 to 1.
         - **Expected Result**: The logic analyzer initiates a capture.
         - **Actual Result:**

..
  Actual test result goes here.
..

    11. Stop all instruments and set the External Trigger In condition to **falling edge**.
    12. Run the Digital IO, Pattern Generator and Logic Analyzer instrument.
    13. In the Digital IO, change DIO0’s output from 1 to 0.
         - **Expected Result**: The logic analyzer initiates a capture.
         - **Actual Result:**

..
  Actual test result goes here.
..

    14. Stop all instruments and set the External Trigger In condition to **any edge**.
    15. Run the Digital IO, Pattern Generator and Logic Analyzer instrument.
    16. In the Digital IO, change DIO0’s output from 0 to 1 or 1 to 0.
         - **Expected Result**: The logic analyzer initiates a capture.
         - **Actual Result:**

..
  Actual test result goes here.
..

    17. Stop all instruments and set the External Trigger In condition to **low**.
    18. Run the Digital IO, Pattern Generator and Logic Analyzer instrument.
    19. In the Digital IO, set DIO0’s output to 0.
         - **Expected Result**: The logic analyzer continuously captures the signal.
         - **Actual Result:**

..
  Actual test result goes here.
..

    20. Stop all instruments and set the External Trigger In condition to **high**.
    21. Run the Digital IO, Pattern Generator and Logic Analyzer instrument.
    22. In the Digital IO, set DIO0’s output to 1.
         - **Expected Result**: The logic analyzer continuously captures the signal.
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


Test 3 - Oscilloscope Source External Trigger
---------------------------------------------

.. _TST.LOGIC.OSC_EXT_TRIGGER:

**UID:** TST.LOGIC.OSC_EXT_TRIGGER

**Description:** This test verifies the external trigger functionality using the 
Oscilloscope as the source.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_setup>`
    - OS: ANY

**Steps:**
    1. Open the Logic Analyzer instrument.
    2. Enable DIO 0 and DIO 1 on the Logic Analyzer.
    3. Set the following parameters:
        - Sample rate to 50ksps.
        - Sample delay of -142 samples.
    4. Open the Logic Analyzer trigger menu and turn on the External trigger.
       Select the source as **Oscilloscope**.
    5. Open the :ref:`Signal Generator<signal_generator>` instrument and generate a sinewave
       with the following parameters:
        
        - Peak-to-peak: 2V.
        - Frequency: 200Hz.
    6. Open the :ref:`Oscilloscope<oscilloscope>` instrument and set the trigger to **normal**
       and condition to **rising edge**.
    7. Run the Signal Generator, Oscilloscope and Logic Analyzer instrument and verify if the
       Logic Analyzer is triggered at the same time with the Oscilloscope.
        
        - **Expected Result**:
            - The Oscilloscope is triggered when the two blue Trigger cursors are intersected
              on the rising edge of the signal.
            - If you drag the horizontal cursor in the Oscilloscope window above or 
              below the signal, it should be in Waiting state, and Logic analyzer will
              be Waiting too.
        - **Actual Result:**

..
  Actual test result goes here.
..

    8. Open the Oscilloscope trigger menu and set the trigger condition to **falling edge**.
    9. Verify that the Logic Analyzer is triggered at the same time with the Oscilloscope.
        - **Expected Result**:
            - If you drag the horizontal cursor in the Oscilloscope window above or 
              below the signal, it should be in Waiting state, and Logic analyzer will
              be Waiting too.
        - **Actual Result:**

..
  Actual test result goes here.
..

    10. Open the Oscilloscope trigger menu and set the trigger condition to **low**.
    11. Verify that the Logic Analyzer is triggered at the same time with the Oscilloscope.
         - **Expected Result**:
             - If you drag the horizontal cursor in the Oscilloscope window above or 
               below the signal, it should be in Waiting state, and Logic analyzer will
               be Waiting too.
         - **Actual Result:**

..
  Actual test result goes here.
..

    12. Open the Oscilloscope trigger menu and set the trigger condition to **high**.
    13. Verify that the Logic Analyzer is triggered at the same time with the Oscilloscope.
         - **Expected Result**:
             - If you drag the horizontal cursor in the Oscilloscope window above or 
               below the signal, it should be in Waiting state, and Logic analyzer will
               be Waiting too.
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


Test 4 - Trigger Modes
---------------------------------------------

.. _TST.LOGIC.TRIGGER_MODES:

**UID:** TST.LOGIC.TRIGGER_MODES

**Description:** This test verifies the trigger modes (and/or) of the Logic Analyzer.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_setup>`
    - OS: ANY

**Prerequisites:**
    - :ref:`Pattern Generator<m2k_pattern_generator_tests>`
    - :ref:`Digital IO<digital_io_tests>`

**Steps:**
    1. Open the Logic Analyzer instrument.
    2. Enable DIO 0, DIO 1 and DIO 2 on the Logic Analyzer.
    3. Set the following parameters:
        - Sample rate to 50ksps.
        - Sample delay of -142 samples.
    4. Open the :ref:`Pattern Generator<pattern_generator>` instrument and enable DIO 2.
       Set the following parameters:
    
        - Pattern: clock.
        - Frequency: 5KHz.
    5. Open the :ref:`Digital IO<digitalio>` instrument and set DIO 0 and DIO 1 as output.
    6. In the Logic Analyzer trigger settings set DIO 0 and DIO 1 trigger conditions 
       to **HIGH** and disable the External trigger.
    7. In the Logic Analyzer trigger configuration, set the trigger logic to **OR**.
    8. Run the Digital IO, Pattern Generator and Logic Analyzer instrument.
    9. In the Digital IO instrument set DIO0’s output to 0 and DIO1’s output to 0.
        - **Expected Result**: The logic analyzer does not start capturing.
        - **Actual Result:**

..
  Actual test result goes here.
..

    10. In the Digital IO instrument set DIO0 or DIO1’s output to 1.
         - **Expected Result**: The logic analyzer starts capturing signal when either of the DIO0 OR DIO1 is HIGH.
         - **Actual Result:**

..
  Actual test result goes here.
..

    11. Stop all instruments and set the trigger logic to **AND**.
    12. Run the Digital IO, Pattern Generator and Logic Analyzer instrument.
    13. In the Digital IO instrument set DIO0’s output to 0 and DIO1’s output to 1.
         - **Expected Result**: The logic analyzer does not start capturing.
         - **Actual Result:**

..
  Actual test result goes here.
..

    14. In the Digital IO instrument set DIO0 and DIO1’s output to 1.
         - **Expected Result**: The logic analyzer starts capturing signal only when DIO0 AND DIO1 are HIGH.
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


Test 5 - Clock signal measurement accuracy
---------------------------------------------

.. _TST.LOGIC.CLOCK_SIGNAL:

**UID:** TST.LOGIC.CLOCK_SIGNAL

**Description:** This test verifies the accuracy of the clock signal measurement.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_setup>`
    - OS: ANY

**Prerequisites:**
    - :ref:`Pattern Generator<m2k_pattern_generator_tests>`

**Steps:**
    1. Open the Logic Analyzer instrument.
    2. Enable DIO 0 on the Logic Analyzer.
    3. Set the following parameters:
        - Sample rate to 50ksps.
        - Enable the cursor.
    4. Open the :ref:`Pattern Generator<pattern_generator>` instrument and enable DIO 0.
       Set the following parameters:
          
          - Pattern: clock.
          - Frequency: 100Hz.
          - Phase: 0 degrees.
          - Duty Cycle: 50%.
    5. Run a single capture of the Logic Analyzer and move the cursor handles to the 
       consecutive rising edges or consecutive falling edges of the signal.
        
        - **Expected Result**: The data measured by the cursor is close to ∆t: 10ms and 1/∆t: 100Hz.
        - **Actual Result:**

..
  Actual test result goes here.
..

    6. In the Cursors settings menu enable **Cursors lock** and measure the next 
       set of edges.
        
        - **Expected Result**: The data measured by the cursor is close to ∆t: 10ms and 1/∆t: 100Hz.
        - **Actual Result:**

..
  Actual test result goes here.
..

    7. Set the Logic Analyzer sample rate to 100Msps and position to 0s.
    8. Set the Pattern Generator DIO 0 parameters to:
        - Pattern: clock.
        - Frequency: 2.5MHz.
    9. Run a single capture of the Logic Analyzer and move the cursor handles to the 
       consecutive rising edges or consecutive falling edges of the signal.
        
        - **Expected Result**: The data measured by the cursor is close to ∆t: 400ns and 1/∆t: 2.5MHz.
        - **Actual Result:**

..
  Actual test result goes here.
..

    10. In the Cursors settings menu enable **Cursors lock** and measure the next
        set of edges.
         
         - **Expected Result**: The data measured by the cursor is close to ∆t: 400ns and 1/∆t: 2.5MHz.
         - **Actual Result:**

..
  Actual test result goes here.
..

    11. Set the Logic Analyzer sample rate to 20ksps.
    12. Set the Pattern Generator DIO 0 parameters to:
         - Pattern: clock.
         - Frequency: 100Hz.
         - Duty Cycle: 70%.
    13. Run a single capture of the Logic Analyzer and move the cursor handles to the 
        rising and falling edge of the upper limit.
         
         - **Expected Result**: The data measured by the cursor is close to ∆t: 7ms.
         - **Actual Result:**

..
  Actual test result goes here.
..

    14. Move the cursors to the falling and rising edge of the lower limit.
         - **Expected Result**: The data measured by the cursor is close to ∆t: 3ms.
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


Test 6 - Parallel Decoder
---------------------------------------------

.. _TST.LOGIC.PARALLEL_DECODER:

**UID:** TST.LOGIC.PARALLEL_DECODER

**Description:** This test verifies the parallel decoder functionality.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_setup>`
    - OS: ANY

**Prerequisites:**
    - :ref:`Pattern Generator<m2k_pattern_generator_tests>`

**Steps:**
    1. Open the Logic Analyzer instrument.
    2. Add a parallel decoder from the main settings menu and set the following parameters:
        - Clock line to DIO8.
        - Data lines 0-7 to DIO0 to DIO7.
    3. Open the Pattern Generator instrument and group DIO0 to DIO7 as number pattern.
       Enable DIO8 and set it to Clock with 500Hz frequency.
       Set the number value to 50 (it is read as decimal).
    4. Run the Pattern Generator and Logic Analyzer instrument.
    5. Verify the Logic Analyze decoded value.
        - **Expected Result**: The reading is in hex format. For reference, 50 decimal = 32 hex.
        - **Actual Result:**

..
  Actual test result goes here.
..

    6. In the Pattern Generator set the number value to 250.
    7. Verify the Logic Analyzer decoded value.
        - **Expected Result**: The reading is in hex format. For reference, 250 decimal = FA.
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


Test 7 - SPI decoder
---------------------------------------------

.. _TST.LOGIC.SPI_DECODER:

**UID:** TST.LOGIC.SPI_DECODER

**Description:** This test verifies the SPI decoder functionality.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_setup>`
    - OS: ANY

**Prerequisites:**
    - :ref:`Pattern Generator<m2k_pattern_generator_tests>`

**Steps:**
    1. Open the Logic Analyzer instrument.
    2. Add an SPI decoder from the main settings menu and set the following parameters:
        - Clock line to DIO0.
        - MISO line to DIO1.
        - MOSI line to DIO2.
        - CS# line to DIO3.
    3. Set DIO2’s trigger to falling edge.
    4. Open the Pattern Generator instrument and group DIO0 to DIO2 as SPI.
       Set the following parameters:
        
        - Frequency: 5kHz.
        - Bytes per frame: 2.
        - Interframe space: 4.
        - Data: insert 4 bytes in hex form e.g: AB CD EF 15.
    5. Run the Pattern Generator and Logic Analyzer instrument.
    6. Verify the Logic Analyzer plot for the decoder output:
        - **Expected Result**: The MISO data has 2 bytes per frame and the decoded data is AB CD EF 15.
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


Test 8 - UART decoder
---------------------------------------------

.. _TST.LOGIC.UART_DECODER:

**UID:** TST.LOGIC.UART_DECODER

**Description:** This test verifies the UART decoder functionality.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_setup>`
    - OS: ANY

**Prerequisites:**
    - :ref:`Pattern Generator<m2k_pattern_generator_tests>`

**Steps:**
    1. Open the Logic Analyzer instrument and set time base to 1 ms.
    2. Add a UART decoder from the main settings menu and set the following parameters:
        - TX line to DIO0.
        - RX line to DIO1.
        - Baud rate to 9600.
        - Data bits to 8.
        - Data format to ASCII.
        - Partity type to none.
        - Stop bits to 1.
        - Bit order to lsb-first.
    3. Set DIO0's trigger to falling edge.
    4. Open the Pattern Generator instrument and group DIO0 as UART.
       Set the following parameters:
        
        - Baud rate: 9600.
        - Stop bit to 1.
        - Parity to none.
        - Data to send: M2K.
    5. Connect DIO0 to DIO1 using a loopback cable.
    6. Run the Pattern Generator and Logic Analyzer instrument.
    7. Verify the Logic Analyzer plot for the decoder output:
        - **Expected Result**: The RX decoded data is "M2K".
        - **Actual Result:**

..
  Actual test result goes here.
..

    8. In the Pattern Generator change the baud rate to 115200.
    9. Verify the Logic Analyzer plot for the decoder output:
        - **Expected Result**: The RX decoded data is not "M2K".
        - **Actual Result:**

..
  Actual test result goes here.
..

    10. In the Logic Analyzer change the baud rate to 115200:
         - **Expected Result**: The RX decoded data is "M2K".
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


Test 9 - PWM Decoder
---------------------------------------------

.. _TST.LOGIC.PWM_DECODER:

**UID:** TST.LOGIC.PWM_DECODER

**Description:** This test verifies the PWM decoder functionality.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_setup>`
    - OS: ANY

**Prerequisites:**
    - :ref:`Pattern Generator<m2k_pattern_generator_tests>`

**Steps:**
    1. Open the Logic Analyzer instrument.
    2. Add a PWM decoder from the main settings menu and set the following parameters:
        - Data line to DIO0.
        - Polarity to active-high.
    3. In the Pattern Generator instrument, set DIO0 to Clock with 100 Hz frequency
       and set the duty cycle to 5%, 30%, 50%, 75% and 95% verifying the Logic Analyzer
       decoded data in between changes:
        
        - **Expected Result**: The data follows the duty cycle set in the pattern generator.
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


Test 10 - Channel visual representation
---------------------------------------------

.. _TST.LOGIC.CHN_VISUAL:

**UID:** TST.LOGIC.CHN_VISUAL

**Description:** This test verifies changing the channel name, 
trace height and plot color.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_setup>`
    - OS: ANY

**Steps:**
    1. Open the Logic Analyzer instrument.
    2. Enable DIO0 and open the channel settings by double clicking
       the channel handle.
    3. Change the channel name to "D0" and verify the channel handle:
        - **Expected Result**: The channel handle displays "D0".
        - **Actual Result:**

..
  Actual test result goes here.
..

    4. Change the trace height to 50:
        - **Expected Result**: The trace height on the plot is doubled.
        - **Actual Result:**

..
  Actual test result goes here.
..

    5. Change the color to **transparent** and verify the channel on plot:
        - **Expected Result**: The channel is no longer seen on the plot.
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


Test 11 - Export Functionality
---------------------------------------------

.. _TST.LOGIC.EXPORT:

**UID:** TST.LOGIC.EXPORT

**Description:** This test verifies the data export functionality 
of the Logic Analyzer.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_setup>`
    - OS: ANY

**Prerequisites:**
    - :ref:`Pattern Generator<m2k_pattern_generator_tests>`

**Steps:**
    1. Open the Pattern Generator and set two consecutive channels with similar
       configurations:
        
        - DIO0 and DIO1:
         
          - Pattern: Clock, 
          - Frequency: 100Hz, 
          - Phase: 0,
          - Duty Cycle: 50%.
         
        - DIO2 and DIO3: 

          - Pattern: Clock, 
          - Frequency: 300Hz, 
          - Phase: 0, 
          - Duty Cycle: 50%.
         
        - DIO4 and DIO5: 

          - Pattern: Clock, 
          - Frequency: 500Hz, 
          - Phase: 0, 
          - Duty Cycle: 50%.
         
        - DIO6 and DIO7: 

          - Pattern: Clock, 
          - Frequency: 150Hz, 
          - Phase: 0, 
          - Duty Cycle: 80%.
         
        - DIO8 and DIO9: 

          - Pattern: Clock, 
          - Frequency: 200Hz, 
          - Phase: 0, 
          - Duty Cycle: 20%.
         
        - Group DIO10 to DIO15: 

          - Pattern: Number Pattern, 
          - Data: 50.
        
    2. Run the Logic Analyzer and Pattern Generator.
    3. In the Logic Analyzer General Settings menu set the Export All to **On** and 
       click the Export button.
    4. Select the file name, location and choose .csv format.
        - **Expected Result**: The file is created in the specified location.
        - **Actual Result:**

..
  Actual test result goes here.
..

    5. Open the file and verify the data:
        - **Expected Result**: The exported data is in .csv format and correspods to the data on the plot.
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


Test 12 - Print Plot
---------------------------------------------

.. _TST.LOGIC.PRINT_PLOT:

**UID:** TST.LOGIC.PRINT_PLOT

**Description:** This test verifies the print plot functionality 
of the Logic Analyzer.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_setup>`
    - OS: ANY

**Prerequisites:**
    - :ref:`Pattern Generator<m2k_pattern_generator_tests>`

**Steps:**
    1. Open the Logic Analyzer and set the time base to 5 ms.
    2. Open the Pattern Generator and set two consecutive channels with similar configurations:
       - DIO0 and DIO1: 

         - Pattern: Clock
         - Frequency: 100Hz
         - Phase: 0
         - Duty Cycle: 50%
         
    3. Run the Logic Analyzer and Pattern Generator.
    4. Press the Print Plot button and choose a location for the exported file:
        - **Expected Result**: The file is created in the specified location.
        - **Actual Result:**

..
  Actual test result goes here.
..

    5. Open the .pdf exported file and visually compare it to the application plot.

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 13 - Decoder Table
---------------------------------------------

.. _TST.LOGIC.DECODER_TABLE:

**UID:** TST.LOGIC.DECODER_TABLE

**Description:** This test verifies the decoder table functionality of the Logic Analyzer.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_setup>`
    - OS: ANY

**Prerequisites:**
    - :ref:`Pattern Generator<m2k_pattern_generator_tests>`

**Steps:**
    1. Open the Pattern Generator and set the following parameters:

        - DIO 0: UART, 
        - Baud rate: 9600, 
        - Data to Send: 123.
    2. Open the Logic Analyzer, enable DIO 0 and add a UART decoder with the 
       following parameters:
        
        - RX on channel 0.
        - Baud rate: 9600.
        - Data format: ASCII.
    3. In the General Settings set the sample rate to 1Msps and nb of samples to 10k samples.
    4. Run the Pattern Generator and Logic Analyzer.
    5. In the Logic Analyzer open the Decoder Table using the right side Decode menu.
       Set the following parameters:
        
        - Group by: RX data.
        - Group size: 3.
        - Filter out all except for RX data.
    6. Check the decoder table:
        - **Expected Result**:
            - Each table row has RX data and time annotations.
            - The RX data corresponds to the data sent by the Pattern Generator.
        - **Actual Result:**

..
  Actual test result goes here.
..

    7. Write "^3$" in the Regex search box and press Enter.
        - **Expected Result**: Only the RX data equal to "3" is displayed in the table.
        - **Actual Result:**

..
  Actual test result goes here.
..

    8. Double click on the first RX data row of the decoder table:
        - **Expected Result**: The plot is zoomed in and centered on the corresponding data.
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
