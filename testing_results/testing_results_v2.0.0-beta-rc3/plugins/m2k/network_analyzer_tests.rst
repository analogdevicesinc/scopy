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

Test 1 - Low Pass Filter CH1 340Hz
---------------------------------------

.. _TST.NA.LOWPASS_CH1_340HZ:

**UID:** TST.NA.LOWPASS_CH1_340HZ

**Description:** This test verifies the functionality of the Network Analyzer
using a Low Pass Filter.

**Preconditions**:
    - :ref:`Adalm2000.Device<adalm2000_device_na_setup>`
    - Using a breadboard, create the following connections:
        - Connect the resistor and capacitor in series
          (resistor on the left side, capacitor on the right side).
        - Connect the right capacitor terminal to  **2-**, **1-** and **GND**.
        - Connect the left resistor terminal to **1+** and **W1**.
        - Probe the intersection node between the resistor and capacitor
          with **2+**.
    - OS: ANY

**Resources**:
    - Resistor: R = 470 Ohms
    - Capacitor: C = 1uF

**Steps:**
    1. In the Network Analyzer settings menu set the following:
        - Reference: Channel 1, 1V Amplitude, 0V Offset
        - Sweep: Logarithmic, Start: 10Hz, Stop: 500kHz, Sample Count: 100
        - Display: Min. Magnitude: -90dB, Max. Magnitude: 10dB, Min. Phase: -150°, 
          Max. Phase: 60°
    2. Run the Network Analyzer. Check the frequency response in the Bode plot.
        - **Expected result:** On the Magnitude Plot, after a flat section (passband),
          response, the trace drops at around 340Hz (cutoff frequency).
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Enable the “Cursor” and move it to find the -3dB point on the trace.
        - **Expected result:** The magnitude indicates -3dB at approximately 
          340 Hz. The phase corresponding to the same frequency is also displayed.

        - **Actual result:**

..
  Actual test result goes here.
..

    4. Disable the cursor by clicking the box again.
        - **Expected result:** The cursor controls disappear from the interface.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Open the General Settings and change the plot type to **Nyquist**:
        - **Expected result:** The Magnitude and Phase plot are replaced by 
          a Polar plot.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Change the plot type to **Nichols**.
        - **Expected result:** The polar plot is replaced by a single plot 
          with the magnitude on the Y Axis and phase on the X Axis.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Change the plot type to **Bode** and switch to **Linear** sweep type.
        - **Expected result:** The plot frequency scale changes to a linear one.
          The signal drops abruptly until it reaches -3dB at around 340Hz, then 
          the trace flattens.
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


Test 2 - Low Pass Filter CH2 340Hz
---------------------------------------

.. _TST.NA.LOWPASS_CH2_340HZ:

**UID:** TST.NA.LOWPASS_CH2_340HZ

**Description:** This test verifies the functionality of the Network Analyzer
using a Low Pass Filter.

**Preconditions**:
    - :ref:`Adalm2000.Device<adalm2000_device_na_setup>`
    - Using a breadboard, create the following connections:
        - Connect the resistor and capacitor in series
          (resistor on the left side, capacitor on the right side).
        - Connect the right capacitor terminal to  **2-**, **1-** and **GND**.
        - Connect the left resistor terminal to **2+** and **W1**.
        - Probe the intersection node between the resistor and capacitor
          with **1+**.
    - OS: ANY

**Resources**:
    - Resistor: R = 470 Ohms
    - Capacitor: C = 1uF

**Steps:**
    1. In the Network Analyzer settings menu set the following:
        - Reference: Channel 2, 1V Amplitude, 0V Offset
        - Sweep: Logarithmic, Start: 10Hz, Stop: 500kHz, Sample Count: 100
        - Display: Min. Magnitude: -90dB, Max. Magnitude: 10dB, Min. Phase: -150°, 
          Max. Phase: 60°
    2. Run the Network Analyzer. Check the frequency response in the Bode plot.
        - **Expected result:** On the Magnitude Plot, after a flat section (passband),
          response, the trace drops at around 340Hz (cutoff frequency).
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Enable the “Cursor” and move it to find the -3dB point on the trace.
        - **Expected result:** The magnitude indicates -3dB at approximately 
          340 Hz. The phase corresponding to the same frequency is also displayed.

        - **Actual result:**

..
  Actual test result goes here.
..

    4. Disable the cursor by clicking the box again.
        - **Expected result:** The cursor controls disappear from the interface.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Open the General Settings and change the plot type to **Nyquist**:
        - **Expected result:** The Magnitude and Phase plot are replaced by 
          a Polar plot.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Change the plot type to **Nichols**.
        - **Expected result:** The polar plot is replaced by a single plot 
          with the magnitude on the Y Axis and phase on the X Axis.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Change the plot type to **Bode** and switch to **Linear** sweep type.
        - **Expected result:** The plot frequency scale changes to a linear one.
          The signal drops abruptly until it reaches -3dB at around 340Hz, then 
          the trace flattens.
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


Test 3 - Low Pass Filter CH1 1.59MHz
-----------------------------------------

.. _TST.NA.LOWPASS_CH1_1_59MHZ:

**UID:** TST.NA.LOWPASS_CH1_1_59MHZ

**Description:** This test verifies the functionality of the Network Analyzer
using a Low Pass Filter.

**Preconditions**:
    - :ref:`Adalm2000.Device<adalm2000_device_na_setup>`
    - Using a breadboard, create the following connections:
        - Connect the resistor and capacitor in series
          (resistor on the left side, capacitor on the right side).
        - Connect the right capacitor terminal to  **2-**, **1-** and **GND**.
        - Connect the left resistor terminal to **1+** and **W1**.
        - Probe the intersection node between the resistor and capacitor
          with **2+**.
    - OS: ANY

**Resources**:
    - Resistor: R = 1 kOhms
    - Capacitor: C = 100 pF

**Steps:**
    1. In the Network Analyzer settings menu set the following:
        - Reference: Channel 1, 1V Amplitude, 0V Offset
        - Sweep: Logarithmic, Start: 50Hz, Stop: 30MHz, Sample Count: 100
        - Display: Min. Magnitude: -45dB, Max. Magnitude: 10dB, Min. Phase: -100°, 
          Max. Phase: 20°
    2. Run the Network Analyzer. Check the frequency response in the Bode plot.
        - **Expected result:** On the Magnitude Plot, after a flat section (passband),
          response, the trace drops at around 1.5 MHz (cutoff frequency).
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Enable the “Cursor” and move it to find the -3dB point on the trace.
        - **Expected result:** The magnitude indicates -3dB at approximately 
          1.5MHz. The phase corresponding to the same frequency is also displayed.

        - **Actual result:**

..
  Actual test result goes here.
..

    4. Disable the cursor by clicking the box again.
        - **Expected result:** The cursor controls disappear from the interface.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Switch to **Linear** sweep type.
        - **Expected result:** The plot frequency scale changes to a linear one.
          The signal gradually drops and reaches -3dB at around 1.59MHz.
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


Test 4 - High Pass Filter CH1 340Hz
----------------------------------------

.. _TST.NA.HIGHPASS_CH1_340HZ:

**UID:** TST.NA.HIGHPASS_CH1_340HZ

**Description:** This test verifies the functionality of the Network Analyzer
using a High Pass Filter.

**Preconditions**:
    - :ref:`Adalm2000.Device<adalm2000_device_na_setup>`
    - Using a breadboard, create the following connections:
        - Connect the capacitor and resistor in series
          (capacitor on the left side, resistor on the right side).
        - Connect the right resistor terminal to  **2-**, **1-** and **GND**.
        - Connect the left capacitor terminal to **1+** and **W1**.
        - Probe the intersection node between the resistor and capacitor
          with **2+**.
    - OS: ANY

**Resources**:
    - Resistor: R = 470 Ohms
    - Capacitor: C = 1uF

**Steps:**
    1. In the Network Analyzer settings menu set the following:
        - Reference: Channel 1, 1V Amplitude, 0V Offset
        - Sweep: Logarithmic, Start: 1Hz, Stop: 1MHz, Sample Count: 100
        - Display: Min. Magnitude: -90dB, Max. Magnitude: 10dB, Min. Phase: -180°, 
          Max. Phase: 180°
    2. Run the Network Analyzer. Check the frequency response in the Bode plot.
        - **Expected result:** On the Magnitude Plot, the first section is an 
          increasing ramp until the magnitude reaches -3dB at around 340Hz
          which is the cutoff frequency. After the cutoff frequency, the 
          magnitude plot flattens, indicating the high pass allows the frequencies
          to pass without attenuation.          
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Enable the “Cursor” and move it to find the -3dB point on the trace.
        - **Expected result:** The magnitude indicates -3dB at approximately 
          340 Hz. The phase corresponding to the same frequency is also displayed.

        - **Actual result:**

..
  Actual test result goes here.
..

    4. Disable the cursor by clicking the box again.
        - **Expected result:** The cursor controls disappear from the interface.
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


Test 5 - Band Pass Filter CH1
----------------------------------

.. _TST.NA.BANDPASS_CH1:

**UID:** TST.NA.BANDPASS_CH1

**Description:** This test verifies the functionality of the Network Analyzer
using a Band Pass Filter with a low cutoff frequency of 1.59kHz and a high cutoff
frequency of 15.9kHz.

**Preconditions**:
    - :ref:`Adalm2000.Device<adalm2000_device_na_setup>`
    - Using a breadboard, create the following connections:
        - Connect **C1** and **R1** in series
          (capacitor on the left side, resistor on the right side).
        - Connect the right R1 terminal to  **2+** and the left 
          terminal of **C2**.
        - Connect the left C1 terminal to **1+** and **W1**.
        - Connect the intersection node between R1 and C1
          to the left terminal of **R2**.
        - Connect the right terminal of **R2**, **2-**, **1-** and
          the right terminal of **C2** to **GND**.
    - OS: ANY

**Resources**:
    - Resistor: R1 = 100 Ohms
    - Resistor: R2 = 100 Ohms
    - Capacitor: C1 = 1uF
    - Capacitor: C2 = 0.1uF (Analog Devices part code 104)

**Steps:**
    1. In the Network Analyzer settings menu set the following:
        - Reference: Channel 1, 1V Amplitude, 0V Offset
        - Sweep: Logarithmic, Start: 50Hz, Stop: 5MHz, Sample Count: 100
        - Display: Min. Magnitude: -90dB, Max. Magnitude: 10dB, Min. Phase: -180°, 
          Max. Phase: 180°
    2. Run the Network Analyzer. Check the frequency response in the Bode plot.
        - **Expected result:** On the Magnitude Plot, the first section is an 
          increasing ramp until the magnitude reaches -3dB at around 1.59kHz
          which is the lower cutoff frequency. The trace section after the lower 
          cutoff frequency is a the passband. The trace drops from -3dB at around 
          15.9kHz which is the higher cutoff frequency.

        - **Actual result:**

..
  Actual test result goes here.
..

    3. Enable the “Cursor” and move it to find the -3dB points on the trace.
        - **Expected result:** The magnitude indicates -3dB at approximately 
          1.59 kHz and 15.9 kHz.

        - **Actual result:**

..
  Actual test result goes here.
..

    4. Disable the cursor by clicking the box again.
        - **Expected result:** The cursor controls disappear from the interface.
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


Test 6 - Band Stop Filter CH1
----------------------------------

.. _TST.NA.BANDSTOP_CH1:

**UID:** TST.NA.BANDSTOP_CH1

**Description:** This test verifies the functionality of the Network Analyzer
using a Band Stop notch Filter with a notch frequency of 795Hz.

**Preconditions**:
    - :ref:`Adalm2000.Device<adalm2000_device_na_setup>`
    - Using a breadboard, create the following connections:
        - Connect **C1** and **C2** in series.
        - Connect **C1** and **R1** in series 
          (capacitor on the left side, resistor on the right side).
        - Connect **R2** and **R3** in series.
        - Connect **C3** in series with **R1** on the left and 
          the intersection between **R2** and **R3** on the right.
        - Connect the right terminal of **R3** to the right terminal 
          of **C2** and to **2+**.
        - At the intersection between **R1** and **C3**, connect 
          **1-**, **2-** and **GND**.
        - Connect the left terminal of **C1** to the left terminal 
          of **R2** and to **1+** and **W1**.
    - OS: ANY

**Resources**:
    - Resistor: R1 = 100 Ohms
    - Resistor: R2 = 200 Ohms
    - Resistor: R3 = 200 Ohms
    - Capacitor: C1 = 1uF
    - Capacitor: C2 = 1uF
    - Capacitor: C3 = 2uF

**Steps:**
    1. In the Network Analyzer settings menu set the following:
        - Reference: Channel 1, 1V Amplitude, 0V Offset
        - Sweep: Logarithmic, Start: 20Hz, Stop: 10MHz, Sample Count: 100
        - Display: Min. Magnitude: -50dB, Max. Magnitude: 5dB, Min. Phase: -180°, 
          Max. Phase: 180°
    2. Run the Network Analyzer. Check the frequency response in the Bode plot.
        - **Expected result:** On the Magnitude Plot, the trace is around -3dB
          on the entire spectrum, except at around 795Hz where the trace drops 
          and then rises again to -3dB.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Enable the “Cursor” and move it to find the low point on the trace.
        - **Expected result:** The magnitude drops at around 795Hz.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Disable the cursor by clicking the box again.
        - **Expected result:** The cursor controls disappear from the interface.
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



Test 7 - Print Plot
-------------------

.. _TST.NA.PRINT_PLOT:

**UID**: TST.NA.PRINT_PLOT

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

**UID**: TST.NA.BUFFER_PREVIEWER

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

