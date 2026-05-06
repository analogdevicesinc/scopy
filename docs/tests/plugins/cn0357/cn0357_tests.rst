.. _cn0357_tests:

CN0357 Plugin - Test Suite
================================================================================

.. note::

   User guide: :ref:`CN0357 user guide<cn0357>`

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

**Prerequisites:**
    - Scopy v2.3.0 or later with CN0357 plugin installed
    - CN0357 evaluation board connected to a supported FPGA carrier
    - Device drivers and firmware are up to date
    - Reset .ini files to default by pressing **Reset** button in Preferences


Setup Environment
-----------------

.. _cn0357_device_setup:

**CN0357.device:**
   - Connect a CN0357 evaluation board to a supported FPGA carrier
   - Connect the carrier to the host system via USB or Ethernet
   - Verify the AD7790 ADC and AD5270 digital potentiometer are detected
   - Add and connect to the device in Scopy with the CN0357 plugin enabled


Test 1: Plugin Loads
~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0357.PLUGIN_LOADS

**RBP:** P0

**Description:** Verify that the CN0357 plugin loads and displays all three sections.

**OS:** ANY

**Preconditions:**
    - Use :ref:`CN0357.device <cn0357_device_setup>` setup

**Steps:**
    1. Open Scopy application
        - **Expected result:** Scopy launches without errors
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Add the CN0357 device and open the CN0357 plugin
        - **Expected result:** Plugin loads and displays ADC Settings, Feedback Settings, and System sections
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


Test 2: Change ADC Update Rate
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0357.ADC_UPDATE_RATE

**RBP:** P1

**Description:** Verify that the ADC Update Rate setting changes the sampling frequency.

**OS:** ANY

**Preconditions:**
    - Use :ref:`CN0357.device <cn0357_device_setup>` setup

**Steps:**
    1. Navigate to the ADC Settings section
        - **Expected result:** The Update Rate combo box is visible with a default value selected
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Change the Update Rate to each available option (120, 100, 60, 50, 25, 16 Hz)
        - **Expected result:** Each selection is accepted and the combo box displays the selected value
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Verify the ADC measurement updates reflect the selected sampling rate
        - **Expected result:** The Conversion (mV) reading in the System section updates at a rate consistent with the selected frequency
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


Test 3: Feedback Settings Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0357.FEEDBACK_SETTINGS

**RBP:** P1

**Description:** Verify feedback type switching, RDAC programming in Rheostat mode, and fixed resistor input.

**OS:** ANY

**Preconditions:**
    - Use :ref:`CN0357.device <cn0357_device_setup>` setup

**Steps:**
    1. Navigate to the Feedback Settings section and verify Rheostat mode is the default
        - **Expected result:** Feedback Type combo shows Rheostat, RDAC Value spinbox and Program Rheostat button are visible
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Set RDAC Value to 512 and press the Program Rheostat button
        - **Expected result:** The RDAC value is written to the AD5270 and the Feedback Resistance in the System section updates accordingly
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Set RDAC Value to 0 and press Program Rheostat, then set to 1023 and press Program Rheostat
        - **Expected result:** Both boundary values are accepted and written to hardware without errors
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Switch Feedback Type to Fixed Resistor
        - **Expected result:** The RDAC Value spinbox and Program Rheostat button are hidden, and the Fixed Resistor spinbox appears
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Enter a fixed resistor value (e.g. 10000 ohms)
        - **Expected result:** The Feedback Resistance in the System section updates to the entered value
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Switch Feedback Type back to Rheostat
        - **Expected result:** The Fixed Resistor spinbox is hidden and the RDAC Value spinbox and Program Rheostat button reappear
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


Test 4: System Measurements and Data
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0357.SYSTEM_MEASUREMENTS

**RBP:** P1

**Description:** Verify that the System section displays live measurements and computed calibration data.

**OS:** ANY

**Preconditions:**
    - Use :ref:`CN0357.device <cn0357_device_setup>` setup

**Steps:**
    1. Navigate to the System section
        - **Expected result:** The Measurements column shows Concentration (ppm), Conversion (mV), and Supply Voltage (V). The Data column shows Sensor Sensitivity, Feedback Resistance, ppm/mV, and mV/ppm
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Observe the Concentration (ppm) display
        - **Expected result:** A numeric ppm value is displayed and updates periodically
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Observe the Conversion (mV) display
        - **Expected result:** The ADC voltage reading is displayed in millivolts and updates periodically
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Observe the Supply Voltage (V) display
        - **Expected result:** The ADC supply voltage is displayed in volts
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Verify Feedback Resistance displays the computed resistance value
        - **Expected result:** The value matches the current feedback mode configuration (RDAC-derived or fixed)
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Verify ppm/mV and mV/ppm conversion coefficients are displayed
        - **Expected result:** Both coefficients show non-zero numeric values consistent with the current sensor sensitivity and feedback resistance
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


Test 5: Change Sensor Sensitivity
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0357.SENSOR_SENSITIVITY

**RBP:** P1

**Description:** Verify that changing the Sensor Sensitivity updates the computed concentration and conversion coefficients.

**OS:** ANY

**Preconditions:**
    - Use :ref:`CN0357.device <cn0357_device_setup>` setup

**Steps:**
    1. Navigate to the System section and note the current Sensor Sensitivity value
        - **Expected result:** Sensor Sensitivity spinbox shows the default value of 65 nA/ppm
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Change Sensor Sensitivity to 30 nA/ppm
        - **Expected result:** The ppm/mV and mV/ppm coefficients update to reflect the new sensitivity value
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Change Sensor Sensitivity to 1 nA/ppm (minimum)
        - **Expected result:** The value is accepted and coefficients update accordingly
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Change Sensor Sensitivity to 100 nA/ppm (maximum)
        - **Expected result:** The value is accepted and coefficients update accordingly
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


Test 6: Refresh Button Functionality
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0357.REFRESH_FUNCTION

**RBP:** P1

**Description:** Verify that the refresh button re-reads all values from the device.

**OS:** ANY

**Preconditions:**
    - Use :ref:`CN0357.device <cn0357_device_setup>` setup

**Steps:**
    1. Note the current measurement values in the System section
        - **Expected result:** All measurement and data fields display values
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Click the refresh button in the CN0357 plugin toolbar
        - **Expected result:** All measurement and data values are re-read from the device and the displays update
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..
