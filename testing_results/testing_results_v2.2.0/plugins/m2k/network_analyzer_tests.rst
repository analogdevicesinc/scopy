.. _network_analyzer_tests:

Network Analyzer - Test Suite
=============================

.. note::

    User guide: :ref:`Network Analyzer user guide<network_analyzer>`


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

.. _adalm2000_device_na_setup:

**Adalm2000.Device:**
    - Open Scopy.
    - Connect the ADALM2000 device to the system.
    - Connect the ADALM2000 device in Scopy using the USB/network backend.

Depends on:
    - Test TST.PREFS.RESET

Prerequisites:
    - Scopy v2.0.0 or later with ADALM2000 plugin installed on the system.
    - Tests listed as dependencies are successfully completed.
    - Reset .ini files to default using the Preferences "Reset" button.

Test 7 - Print Plot
-------------------

.. _TST.NA.PRINT_PLOT:

**UID:** TST.NA.PRINT_PLOT

**RBP:** P3

**Description:** Check the print plot feature of the Network Analyzer 
by exporting the low pass filter plot screenshot to a PDF file.

**Preconditions**:
    - :ref:`Adalm2000.Device<adalm2000_device_na_setup>`
    - :ref:`Low pass filter setup<TST.NA.LOWPASS_CH1_340HZ>`
    - OS: ANY

**Steps:**
    1. Setup the Network Analyzer as stated in the low pass filter test
       linked above.
    2. Click the Print plot button and choose a name and location for the file.
        - **Expected result:** The file is saved as a BMP in the selected
          location.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Open the exported file and verify it:
        - **Expected result:** The plot is correctly saved in the BMP file.
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


Test 8 - Buffer Previewer
----------------------------

.. _TST.NA.BUFFER_PREVIEWER:

**UID:** TST.NA.BUFFER_PREVIEWER

**RBP:** P3

**Description:** Check the buffer previewer feature of the Network Analyzer 
by viewing the acquired data in the Oscilloscope and analyzing measurements.

**Preconditions**:
    - :ref:`Adalm2000.Device<adalm2000_device_na_setup>`
    - Connect **1+** to **W1** and **2+** to **2-** using loopback cables.
    - OS: ANY

**Steps:**
    1. In the Network Analyzer set the following configuration:
        - Reference: Channel 1, 1V Amplitude, 0V Offset
        - Sweep: Linear, Start: 20Hz, Stop: 1MHz, Sample Count: 10
        - Display: Min. Magnitude: -50dB, Max. Magnitude: 5dB, Min. Phase: -180°, 
          Max. Phase: 180°
    2. Run a Single capture in the Network Analyzer.
    3. In the Sweep settings menu, enable the Buffer Previewer.
        - **Expected result:** A time domain plot appears above the Bode plot.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Slide the blue handle at the leftmost end of the plot.
        - **Expected result:** 
            - A sinewave is displayed on the time plot.
            - Below the time plot the Sample Count is 1/10, Current Frequency is 20Hz.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. In the Sweep settings menu click the **ViewInOsc** button.
        - **Expected result:**
            - The Oscilloscope instrument is opened.
            - The data is displayed as reference waveform in the Oscilloscope.
            - The measurements show a frequency of 20Hz.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Slide the blue handle at the rightmost end of the plot.
        - **Expected result:** 
            - A sinewave is displayed on the time plot.
            - Below the time plot the Sample Count is 10/10, Current Frequency is 1MHz.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. In the Sweep settings menu click the **ViewInOsc** button.
        - **Expected result:**
            - The Oscilloscope instrument is opened.
            - The data is displayed as reference waveform in the Oscilloscope.
            - The measurements show a frequency of 1MHz.
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

