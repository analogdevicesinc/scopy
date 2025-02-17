.. _adc_tests:

ADC Plugin - Test Suite
=======================

.. note::

   User guide: :ref:`ADC user guide<adc>`

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

Setup environment
------------------

.. _adalm-pluto_device_usb:

**Pluto.usb:**
   - Connect a pluto device via usb to your PC
   - connect to the device in Scopy with ADCPlugin enabled

.. _adalm-pluto_device_emu:

**Pluto.emu:**
   - Press the add device button and go to emulator
   - Select pluto device
   - Enable, verify and select all plugins available
   - Connect to the device

.. _adalm-pluto_device_siggen:

**Pluto.signalGenerator:**
   - Requires DAC and Debugger tests to work
   - Go to Debugger, select context0/ad9361-phy/TX_LO/frequency and set it to 2400000000.
     Do the same for context0/ad9361-phy/RX_LO/frequency
   - Go to DAC plugin, mode: DDS, dds mode: independent I/Q control. 
     TX1_I_F1 scale: -40, TX1_I_F2: -40, TX1_Q_F1: -30, TX1_Q_F2: -30. set all frequency values to 1MHz.
   - Run DAC plugin.


.. _adc_time_tests:

ADC-Time
--------

Test 1 - X-AXIS
^^^^^^^^^^^^^^^

.. _TST.ADC_TIME.X_AXIS:

**UID:** TST.ADC_TIME.X_AXIS

**Description:** Verify X-axis behavior under different configurations

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Scroll down and turn on plot labels in the settings panel
   2. Set XMode to Samples on the X-axis
   3. Set buffer size to 100
   4. Press Single
       - **Expected result:** Plot labels should go up to 100, and channel curves 
         should span across the whole plot
       - **Actual result:**

..
  Actual test result goes here.
..

   5. Turn off Sync buffer-plot sizes
   6. Set plot size to 200
   7. Press Single
       - **Expected result:** Plot labels should go up to 200, and channel curves should span across the whole plot
       - **Actual result:**

..
  Actual test result goes here.
..

   8. Turn on Rolling mode
       - **Expected result:** Plot labels should invert, going from 200 to 0
       - **Actual result:**

..
  Actual test result goes here.
..

   9. Change XMin to 200 and XMax to -100
   10. Press Single
        - **Expected result:** Plot labels should range from 300 to -100, with channel curves centered and between samples 200 and 0
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


Test 2 - Y-AXIS
^^^^^^^^^^^^^^^
.. _TST.ADC_TIME.Y_AXIS:

**UID:** TST.ADC_TIME.Y_AXIS

**Description:** Verify Y-axis behavior under different configurations

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Scroll down and turn on plot labels in the settings panel
   2. Turn on Autoscale and press Run
       - **Expected result:** Y-axis should automatically scale according to channel curves
       - **Actual result:**

..
  Actual test result goes here.
..

   3. Press Stop
   4. Turn Autoscale off
   5. Set YMode to ADC Counts
   6. Set Min to -4000 and Max to 4000
   7. Press Single
       - **Expected result:** Y-axis labels should range from -4000 to 4000, and curves should vertically adjust accordingly
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



Test 3 - XY PLOT
^^^^^^^^^^^^^^^^

.. _TST.ADC_TIME.XY_PLOT:

**UID:** TST.ADC_TIME.XY_PLOT

**Description:** Verify XY plot functionality and visualization

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - :ref:`Pluto.signalGenerator<adalm-pluto_device_siggen>`
   - OS: ANY

**Steps:**
   1. Run Pluto.signalGenerator setup
   2. Turn on XY PLOT
       - **Expected result:** A new plot should appear on the right
       - **Actual result:**

..
  Actual test result goes here.
..

   3. Turn on Autoscale
       - **Expected result:** On the time plot, both channel curves should 
         look the same, but voltage0 should have a higher amplitude
       - **Actual result:**

..
  Actual test result goes here.
..

   4. Set X Axis source to voltage0
       - **Expected result:** A small circular curve should appear on the XY plot
       - **Actual result:**

..
  Actual test result goes here.
..

   5. Set X Axis source to voltage1
       - **Expected result:** A larger circular curve should appear on the XY plot
       - **Actual result:**

..
  Actual test result goes here.
..

   6. Turn Plot X source on and off
       - **Expected result:** A line cutting the circle should be visible in a different color
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



Test 4 - Settings
^^^^^^^^^^^^^^^^^

.. _TST.ADC_TIME.SETTINGS:

**UID:** TST.ADC_TIME.SETTINGS

**Description:** Change Plot Title and toggle various plot settings

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Change Plot Title to "test"
       - **Expected result:** text in top left should change to "test"
       - **Actual result:**

..
  Actual test result goes here.
..

   2. Turn on and off plot labels
       - **Expected result:** labels on x and y axis should appear
       - **Actual result:**

..
  Actual test result goes here.
..

   3. Change thickness and style to all available modes and observe the changes
       - **Expected result:** channel curves' appearance should behave as expected
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



Test 5 - Print
^^^^^^^^^^^^^^

.. _TST.ADC_TIME.PRINT:

**UID:** TST.ADC_TIME.PRINT

**Description:** Save the plot as a PDF file via the print functionality

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Run :ref:`Pluto.signalGenerator<adalm-pluto_device_siggen>` setup
   2. Press the single button 
   3. Press the print button
   4. Select a folder and save the file
       - **Expected result:** a new PDF file should appear within the folder containing 
         the last view of the plot
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



Test 6 - Info
^^^^^^^^^^^^^

.. _TST.ADC_TIME.INFO:

**UID:** TST.ADC_TIME.INFO

**Description:** Open the ADC plugin documentation via the info button

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Press the info button in the top left corner of the window
       - **Expected result:** this should open the ADC plugin documentation.
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



Test 7 - Plot Navigation
^^^^^^^^^^^^^^^^^^^^^^^^

.. _TST.PLOT_NAVIGATION:

**UID:** TST.PLOT_NAVIGATION

**Description:** Test plot zooming, panning, and undo features

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Scroll down and turn on plot labels in the settings panel
   2. Place mouse cursor within the plot and use the scroll wheel
       - **Expected result:** should zoom in/out on the area at the cursor, should not be able to zoom out more than the default view
       - **Actual result:**

..
  Actual test result goes here.
..

   3. Right-click the plot after zooming in
       - **Expected result:** should undo the zoom
       - **Actual result:**

..
  Actual test result goes here.
..

   4. Select an area on the plot using left-click
       - **Expected result:** should zoom in exactly on the selected area
       - **Actual result:**

..
  Actual test result goes here.
..

   5. Zoom in using the selection method, then hold left shift button and zoom using the scroll wheel
       - **Expected result:** should pan left and right
       - **Actual result:**

..
  Actual test result goes here.
..

   6. Zoom in using all methods described above and press right-click until zoom is back to default
       - **Expected result:** should undo each zoom operation one by one
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


Test 8 - ADC-Time Channel Settings
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _TST.ADC_TIME_CHANNEL_SETTINGS:

**UID:** TST.ADC_TIME_CHANNEL_SETTINGS

**Description:** Test Y-axis settings and adjustments for the ADC-Time tool

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Double-click on the voltage0 channel and go to the right menu which pops up
   2. Enable Y-AXIS and move up and down the axis handle
       - **Expected result:** this should move the channel curve within the Y-axis
       - **Actual result:**

..
  Actual test result goes here.
..

   3. Turn autoscale off. Set YMode to ADC Counts, Min to -4000 and Max to 4000, and press single
       - **Expected result:** only voltage0 curve should vertically adjust
       - **Actual result:**

..
  Actual test result goes here.
..

   4. Turn off Y-AXIS

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).



Test 9 - Measure
^^^^^^^^^^^^^^^^

.. _TST.MEASURE:

**UID:** TST.MEASURE

**Description:** Test the Measure functionality, enabling panels 
and checking frequency and stats

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Double-click on the voltage0 channel and go to the right menu which pops up
   2. Open the Measure menu at the bottom of the window, enable Measure Panel and Stats panel
       - **Expected result:** check frequency measure and stats in the horizontal measurements table, 
         middle measure and stats in the vertical measurements table. Frequency should be around 1MHz in 
         both panels, middle should be hovering around the 0 value in both panels
       - **Actual result:**

..
  Actual test result goes here.
..

   3. Open the Measure menu and show all Stats and Measurements
       - **Expected result:** all checkboxes in the measurement tables should be checked and each 
         should have a corresponding label above and below the plot
       - **Actual result:**

..
  Actual test result goes here.
..

   4. Turn off measurements

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).



Test 10 - Cursors
^^^^^^^^^^^^^^^^^

.. _TST.CURSORS:

**UID:** TST.CURSORS

**Description:** Test cursor functionality including synchronization, 
tracking, and moving readouts

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Double-click on the voltage0 channel and go to the right menu which pops up
   2. Run :ref:`Pluto.signalGenerator<adalm-pluto_device_siggen>` setup
   3. In ADC, set X-AXIS buffer size to 200, XMode to Sample, and TMode to ADC Counts
   4. Open the Cursors menu at the bottom of the window. Enable X and Enable Y
   5. Move the X cursors on the plot via the axis handles and place one on voltage0 sine high peak and the other on the next low peak
       - **Expected result:** delta x should be around 15 samples between voltage0 high peak and low peak
       - **Actual result:**

..
  Actual test result goes here.
..

   6. Lock X cursors via the Cursor menu. Move one cursor to a voltage1 high peak
       - **Expected result:** the second cursor should move and maintain the same delta. High peak to low peak should match in both voltage0 and voltage1
       - **Actual result:**

..
  Actual test result goes here.
..

   7. Enable track and select voltage0
       - **Expected result:** markers should appear on the voltage0 curve synced with the X cursor. Values in cursor readouts should change accordingly
       - **Actual result:**

..
  Actual test result goes here.
..

   8. Select voltage1 channel
       - **Expected result:** markers and readouts should now be related to voltage1
       - **Actual result:**

..
  Actual test result goes here.
..

   9. Turn off track and measure voltage0 and voltage1 amplitudes
       - **Expected result:** for voltage0 delta y should be around 1.4kV and for voltage1 500V
       - **Actual result:**

..
  Actual test result goes here.
..

   10. Enable move readouts. Click and hold the readouts to move them
       - **Expected result:** should be able to move the readouts anywhere within the plot
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



.. _adc_frequency_tests:

ADC-Frequency
-------------

Test 1 - X-Axis settings
^^^^^^^^^^^^^^^^^^^^^^^^

.. _TST.ADC_FREQ.X_AXIS_SETTINGS:

**UID:** TST.ADC_FREQ.X_AXIS_SETTINGS

**Description:** Adjust X-axis settings for frequency and sample-based 
plotting in the ADC plugin

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Scroll down and turn on plot labels in the settings panel
   2. Set XMode to Samples, buffer size to 4000, and press single
       - **Expected result:** Plot labels should go up to 4000, and channel curves should span across the whole plot
       - **Actual result:**

..
  Actual test result goes here.
..

   3. Set XMode to Frequency, XMin to 0.5 MHz, XMax to 2 MHz, and press single
       - **Expected result:** Plot labels should be from 0.5 MHz to 2 MHz, and channel curves should be centered
       - **Actual result:**

..
  Actual test result goes here.
..

   4. Set frequency offset to 1 MHz and press single
       - **Expected result:** Curves should move 1 MHz to the left
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



Test 2 - Y-Axis settings
^^^^^^^^^^^^^^^^^^^^^^^^

.. _TST.ADC_FREQ.Y_AXIS_SETTINGS:

**UID:** TST.ADC_FREQ.Y_AXIS_SETTINGS

**Description:** Adjust Y-axis settings, including autoscaling and manual 
range settings

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Scroll down and turn on plot labels in the settings panel
   2. Turn on autoscale and press run
       - **Expected result:** Y-axis should automatically scale according to the channel curves
       - **Actual result:**

..
  Actual test result goes here.
..

   3. Press stop, turn autoscale off, set YMode to ADC Counts, Min to -140, Max to 20, and press single
       - **Expected result:** Y-axis labels should be from -140 to 20, and curves should vertically adjust
       - **Actual result:**

..
  Actual test result goes here.
..

   4. Set power offset to 20 dB and press single
       - **Expected result:** Curves should move 20 dB up
       - **Actual result:**

..
  Actual test result goes here.
..

   5. Press run and change through all the window options and window correction
       - **Expected result:** The curves should change slightly in real time
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



Test 3 - Settings adjustment
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _TST.ADC_FREQ.SETTINGS_ADJUSTMENT:

**UID:** TST.ADC_FREQ.SETTINGS_ADJUSTMENT

**Description:** Adjust plot settings such as title, labels, thickness, 
and style

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Change Plot Title to "test"
       - **Expected result:** Text in the top left should change to "test"
       - **Actual result:**

..
  Actual test result goes here.
..

   2. Turn on and off plot labels
       - **Expected result:** Labels on X and Y axis should appear
       - **Actual result:**

..
  Actual test result goes here.
..

   3. Change thickness and style to all available modes and observe the changes
       - **Expected result:** Channel curves should adjust appearance as expected
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


Test 4 - Print plot
^^^^^^^^^^^^^^^^^^^

.. _TST.ADC_FREQ.PRINT_PLOT:

**UID:** TST.ADC_FREQ.PRINT_PLOT

**Description:** Print the current plot to a PDF file

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Run :ref:`Pluto.signalGenerator<adalm-pluto_device_siggen>` setup
   2. Press the single button and then the print button
   3. Select a folder and save the file
       - **Expected result:** A new PDF file should appear in the folder containing the last view of the plot
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



Test 5 - View plugin documentation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _TST.ADC_FREQ.VIEW_PLUGIN_DOC:

**UID:** TST.ADC_FREQ.VIEW_PLUGIN_DOC

**Description:** Open the ADC plugin documentation

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Press the info button in the top left corner of the window
       - **Expected result:** The ADC plugin documentation should open
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



Test 6 - Y-Axis channel settings
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _TST.ADC_FREQ.Y_AXIS_CHANNEL_SETTINGS:

**UID:** TST.ADC_FREQ.Y_AXIS_CHANNEL_SETTINGS

**Description:** Adjust the Y-axis for individual channel curve manipulation

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Enable Y-Axis and move the axis handle up and down
       - **Expected result:** The channel curve should move within the Y-axis
       - **Actual result:**

..
  Actual test result goes here.
..

   2. Turn off autoscale, set YMode to ADC Counts, Min to -140, Max to 20, and press single
       - **Expected result:** Only the voltage0 curve should vertically adjust
       - **Actual result:**

..
  Actual test result goes here.
..

   3. Turn off Y-Axis

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).



Test 7 - Marker settings
^^^^^^^^^^^^^^^^^^^^^^^^

.. _TST.ADC_FREQ.MARKER_SETTINGS:

**UID:** TST.ADC_FREQ.MARKER_SETTINGS

**Description:** Use markers to identify peaks on channel curves

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Run :ref:`Pluto.signalGenerator<adalm-pluto_device_siggen>` setup
   2. Press run and enable the marker
   3. Set marker type to peak and count to 5
       - **Expected result:** 5 markers should appear on voltage0's curve at the highest peaks
       - **Actual result:**

..
  Actual test result goes here.
..

   4. Repeat the process with a count of 7
       - **Expected result:** 7 markers should appear on voltage0's curve at the highest peaks
       - **Actual result:**

..
  Actual test result goes here.
..

   5. Check the table below the plot in peak mode
       - **Expected result:** The highest peak should be at 1 MHz
       - **Actual result:**

..
  Actual test result goes here.
..

   6. Set marker type to fixed and count to 5
       - **Expected result:** 5 movable cursors should appear
       - **Actual result:**

..
  Actual test result goes here.
..

   7. Repeat for a count of 7
       - **Expected result:** 7 movable cursors should appear
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



Test 8 - Cursor settings
^^^^^^^^^^^^^^^^^^^^^^^^

.. _TST.ADC_FREQ.CURSOR_SETTINGS:

**UID:** TST.ADC_FREQ.CURSOR_SETTINGS

**Description:** Use cursors to measure specific points on channel curves.

**Preconditions:**
   - :ref:`Pluto.usb<adalm-pluto_device_usb>`
   - OS: ANY

**Steps:**
   1. Run :ref:`Pluto.signalGenerator<adalm-pluto_device_siggen>` setup
   2. Press run and set X-Axis buffer size to 200, XMode to Sample, and TMode to ADC Counts
   3. Open the Cursors menu and enable X and Y cursors
   4. Move the X cursor on the plot and place one on voltage0's highest peak
       - **Expected result:** The cursor should measure 1 MHz in the readouts
       - **Actual result:**

..
  Actual test result goes here.
..

   5. Enable track and select voltage0
       - **Expected result:** Markers should appear on the voltage0 curve, synced with the X cursor
       - **Actual result:**

..
  Actual test result goes here.
..

   6. Select voltage1 channel
       - **Expected result:** Markers and readouts should now be related to voltage1
       - **Actual result:**

..
  Actual test result goes here.
..

   7. Turn off track and measure voltage0's highest peak using the Y cursor
       - **Expected result:** The cursor should measure about -13 dB in the readouts
       - **Actual result:**

..
  Actual test result goes here.
..

   8. Enable move readouts and click and hold the readouts to move them
       - **Expected result:** Readouts should be movable within the plot
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


.. _adc_preferences_tests:

Preferences
-----------

Test 1 - X Scale Position
^^^^^^^^^^^^^^^^^^^^^^^^^

.. _TST.PREF.X_SCALE_POS:

**UID:** TST.PREF.X_SCALE_POS

**Description:** Change the X-axis scale position and verify the label's position on the plot

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`Pluto.emu<adalm-pluto_device_emu>`
   - OS: ANY

**Steps:**
   1. Change Plot X-Axis scale position to Top, restart app, open ADC-Time and enable plot labels in SETTINGS panel
       - **Expected result:** Axis labels should appear on top
       - **Actual result:**

..
  Actual test result goes here.
..

   2. Do this again for Bottom option
       - **Expected result:** Axis labels should appear on the bottom
       - **Actual result:**

..
  Actual test result goes here.
..

   3. Repeat for ADC-Frequency
       - **Expected result:** Same result for ADC-Frequency
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


Test 2 - Y Scale Position
^^^^^^^^^^^^^^^^^^^^^^^^^

.. _TST.PREF.Y_SCALE_POS:

**UID:** TST.PREF.Y_SCALE_POS

**Description:** Change the Y-axis scale position and verify the label's 
position on the plot

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`Pluto.emu<adalm-pluto_device_emu>`
   - OS: ANY

**Steps:**
   1. Change Plot Y-Axis scale position to Right, restart app, open ADC-Time and enable plot labels in SETTINGS panel
       - **Expected result:** Axis labels should appear on the right
       - **Actual result:**

..
  Actual test result goes here.
..

   2. Do this again for Left option
       - **Expected result:** Axis labels should appear on the left
       - **Actual result:**

..
  Actual test result goes here.
..

   3. Repeat for ADC-Frequency
       - **Expected result:** Same result for ADC-Frequency
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


Test 3 - Channel Handle Position
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _TST.PREF.CH_HANDLE_POS:

**UID:** TST.PREF.CH_HANDLE_POS

**Description:** Change the Y-handle position and verify the appearance of the handle

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`Pluto.emu<adalm-pluto_device_emu>`
   - OS: ANY
   
**Steps:**
   1. Change Plot channel Y-handle position to Right, restart app, open 
      ADC-Time, double click voltage0 to open channel settings and enable Y-AXIS
       
       - **Expected result:** Axis handle should appear on the right
       - **Actual result:**

..
  Actual test result goes here.
..

   2. Do this again for Left option
       - **Expected result:** Axis handle should appear on the left
       - **Actual result:**

..
  Actual test result goes here.
..

   3. Repeat for ADC-Frequency
       - **Expected result:** Same result for ADC-Frequency
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


Test 4 - X Cursor Position
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _TST.PREF.X_CURSOR_POS:

**UID:** TST.PREF.X_CURSOR_POS

**Description:** Change the X-cursor position and verify the cursor's appearance.

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`Pluto.emu<adalm-pluto_device_emu>`
   - OS: ANY

**Steps:**
   1. Change Plot X-Cursor position to Top, restart app, open ADC-Time and enable x cursors
       - **Expected result:** X cursor handle should appear on top
       - **Actual result:**

..
  Actual test result goes here.
..

   2. Do this again for Bottom option
       - **Expected result:** X cursor handle should appear on the bottom
       - **Actual result:**

..
  Actual test result goes here.
..

   3. Repeat for ADC-Frequency
       - **Expected result:** Same result for ADC-Frequency
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



Test 5 - Y Cursor Position
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _TST.PREF.Y_CURSOR_POS:

**UID:** TST.PREF.Y_CURSOR_POS

**Description:** Change the Y-cursor position and verify the cursor's appearance

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`Pluto.emu<adalm-pluto_device_emu>`
   - OS: ANY

**Steps:**
   1. Change Plot Y-Cursor position to Right, restart app, open ADC-Time and enable y cursors
       - **Expected result:** Y cursor handle should appear on the right
       - **Actual result:**

..
  Actual test result goes here.
..

   2. Do this again for Left option
       - **Expected result:** Y cursor handle should appear on the left
       - **Actual result:**

..
  Actual test result goes here.
..

   3. Repeat for ADC-Frequency
       - **Expected result:** Same result for ADC-Frequency
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



Test 6 - Show Buffer
^^^^^^^^^^^^^^^^^^^^

.. _TST.PREF.SHOW_BUFFER:

**UID:** TST.PREF.SHOW_BUFFER

**Description:** Enable/disable the buffer and verify its appearance on the plot

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`Pluto.emu<adalm-pluto_device_emu>`
   - OS: ANY

**Steps:**
   1. Set buffer to disabled, open ADC-Time
       - **Expected result:** There should be no buffer previewer above the plot
       - **Actual result:**

..
  Actual test result goes here.
..

   2. Do this again for the enabled option
       - **Expected result:** Buffer previewer should appear above the plot
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



Test 7 - Default YMode
^^^^^^^^^^^^^^^^^^^^^^

.. _TST.PREF.DEFAULT_YMODE:

**UID:** TST.PREF.DEFAULT_YMODE

**Description:** Set the default Y-mode and verify its behavior in ADC-Time

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`Pluto.emu<adalm-pluto_device_emu>`
   - OS: ANY

**Steps:**
   1. Set YMode to % Full scale, restart app, open ADC-Time
       - **Expected result:** In Y-AXIS, YMODE should be % Full scale
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
