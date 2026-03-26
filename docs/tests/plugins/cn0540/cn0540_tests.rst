.. _cn0540_tests:

CN0540 Test Suite
================================================================================

**Prerequisites:**
    - Scopy v2.0 or later with CN0540 plugin installed
    - CN0540 hardware board connected
    - IIO kernel drivers loaded for ``ad7768-1``, ``ltc2606``, and ``one-bit-adc-dac``
    - Device drivers and firmware are up to date
    - Reset .ini files to default by pressing **Reset** button in Preferences

Test 1: Plugin Loads
~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0540.PLUGIN_LOADS

**RBP:** P0

**Description:** Verify that the CN0540 plugin loads in Scopy.

**OS:** ANY

**Preconditions:**
    - Scopy is installed

**Steps:**
    1. Open Scopy application
        - **Expected result:** Scopy launches without errors
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Open CN0540 plugin
        - **Expected result:** The CN0540 plugin loads and the tool is accessible in the UI
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

Test 2: Device Detection and Sections Display
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0540.DEVICE_DETECTION

**RBP:** P1

**Description:** Verify that the CN0540 plugin detects the required IIO devices and displays all four collapsible sections.

**OS:** ANY

**Preconditions:**
    - CN0540 hardware board is connected with ``ad7768-1``, ``ltc2606``, and ``one-bit-adc-dac`` devices present

**Steps:**
    1. Connect the CN0540 board and open the CN0540 plugin in Scopy
        - **Expected result:** The plugin loads and displays four collapsible sections: Power Control, ADC Driver Settings, Sensor Calibration, and Voltage Monitor
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Expand each section
        - **Expected result:** Each section expands to reveal its controls and labels without errors
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

Test 3: Refresh Button Re-reads Hardware
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0540.REFRESH_BUTTON

**RBP:** P1

**Description:** Verify that clicking the Refresh button re-reads all hardware values and updates the UI.

**OS:** ANY

**Preconditions:**
    - CN0540 hardware board is connected

**Steps:**
    1. Open the CN0540 plugin and note the displayed values in all sections
        - **Expected result:** All sections display current hardware values
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Click the **Refresh** button in the top bar
        - **Expected result:** All values are re-read from hardware and the displayed values are updated
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

Test 4: Power Control: Read SW_FF Status
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0540.READ_SW_FF

**RBP:** P1

**Description:** Verify that clicking Check Status reads the SW_FF fault flag GPIO and displays the result.

**OS:** ANY

**Preconditions:**
    - CN0540 hardware board is connected

**Steps:**
    1. Expand the **Power Control** section and click **Check Status** next to SW_FF
        - **Expected result:** The SW_FF status label is updated with the current fault flag value read from hardware
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

Test 5: Power Control: Shutdown Toggle
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0540.SHUTDOWN_TOGGLE

**RBP:** P1

**Description:** Verify that toggling the Shutdown checkbox writes to the shutdown GPIO and reads back the new state.

**OS:** ANY

**Preconditions:**
    - CN0540 hardware board is connected

**Steps:**
    1. Expand the **Power Control** section and note the current state of the **Shutdown** checkbox
        - **Expected result:** Checkbox reflects the current hardware shutdown state
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Toggle the **Shutdown** checkbox and observe the status label
        - **Expected result:** The shutdown GPIO is written with the new value and the status label reflects the updated state
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Toggle the **Shutdown** checkbox back to the original state
        - **Expected result:** The status label returns to the original state
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

Test 6: Power Control: Constant Current Toggle
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0540.CONSTANT_CURRENT_TOGGLE

**RBP:** P1

**Description:** Verify that toggling the Constant Current checkbox writes to the blue LED GPIO and reads back the new state.

**OS:** ANY

**Preconditions:**
    - CN0540 hardware board is connected

**Steps:**
    1. Expand the **Power Control** section and note the current state of the **Constant Current** checkbox
        - **Expected result:** Checkbox reflects the current hardware GPIO state
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Toggle the **Constant Current** checkbox and observe the status label
        - **Expected result:** The blue LED GPIO is written with the new value and the status label reflects the updated state
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Toggle the **Constant Current** checkbox back to the original state
        - **Expected result:** The status label returns to the original state
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

Test 7: ADC Driver Settings: FDA Status Toggle
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0540.FDA_STATUS_TOGGLE

**RBP:** P1

**Description:** Verify that toggling the FDA Status checkbox enables or disables the FDA amplifier and the status label updates accordingly.

**OS:** ANY

**Preconditions:**
    - CN0540 hardware board is connected

**Steps:**
    1. Expand the **ADC Driver Settings** section and note the current state of the **FDA Status** checkbox
        - **Expected result:** Checkbox reflects the current FDA enable state
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Toggle the **FDA Status** checkbox and observe the status label
        - **Expected result:** The FDA_DIS GPIO is written with the new value and the status label reflects the updated state
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Toggle the **FDA Status** checkbox back to the original state
        - **Expected result:** The status label returns to the original state
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

Test 8: ADC Driver Settings: FDA Mode Toggle
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0540.FDA_MODE_TOGGLE

**RBP:** P1

**Description:** Verify that toggling the FDA Mode checkbox switches the FDA between FULL POWER and LOW POWER modes.

**OS:** ANY

**Preconditions:**
    - CN0540 hardware board is connected

**Steps:**
    1. Expand the **ADC Driver Settings** section and note the current state of the **FDA Mode** checkbox
        - **Expected result:** Checkbox reflects the current power mode (checked = FULL POWER, unchecked = LOW POWER)
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Toggle the **FDA Mode** checkbox and observe the status label
        - **Expected result:** The FDA_MODE GPIO is written and the status label shows either ``FULL POWER`` or ``LOW POWER`` matching the new checkbox state
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Toggle the **FDA Mode** checkbox back to the original state
        - **Expected result:** The status label returns to the original power mode text
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

Test 9: Sensor Calibration: Input Voltage Read
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0540.INPUT_VOLTAGE_READ

**RBP:** P1

**Description:** Verify that the Input Voltage field displays a valid ADC voltage reading from the ad7768-1 device.

**OS:** ANY

**Preconditions:**
    - CN0540 hardware board is connected

**Steps:**
    1. Expand the **Sensor Calibration** section and click **Refresh**
        - **Expected result:** The **Input Voltage [mV]** label displays a numeric voltage value read from the ``ad7768-1`` ``voltage0`` channel
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

Test 10: Sensor Calibration: Shift Voltage Write/Readback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0540.SHIFT_VOLTAGE_WRITE

**RBP:** P1

**Description:** Verify that entering a shift voltage value and clicking Write sends the value to the ltc2606 DAC, and clicking Read reflects the written value.

**OS:** ANY

**Preconditions:**
    - CN0540 hardware board is connected

**Steps:**
    1. Expand the **Sensor Calibration** section and click **Read** next to **Shift Voltage [mV]**
        - **Expected result:** The shift voltage field displays the current DAC voltage read from hardware
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Enter a new valid voltage value in the **Shift Voltage [mV]** field and click **Write**
        - **Expected result:** The value is written to the ``ltc2606`` ``voltage0`` channel without error
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Click **Read** next to **Shift Voltage [mV]**
        - **Expected result:** The displayed value matches the previously written voltage (accounting for DAC_BUF_GAIN = 1.22 scaling)
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

Test 11: Sensor Calibration: Sensor Voltage Calculation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0540.SENSOR_VOLTAGE_READ

**RBP:** P1

**Description:** Verify that clicking Read in the Sensor Voltage row calculates and displays a valid sensor voltage from the ADC, DAC, and FDA parameters.

**OS:** ANY

**Preconditions:**
    - CN0540 hardware board is connected

**Steps:**
    1. Expand the **Sensor Calibration** section and click **Read** next to **Sensor Voltage [mV]**
        - **Expected result:** The **Sensor Voltage [mV]** label updates with a calculated numeric voltage value derived from the ADC input, DAC shift voltage, FDA gain, and FDA output common mode
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

Test 12: Sensor Calibration: Calibration Routine
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0540.CALIBRATE

**RBP:** P2

**Description:** Verify that clicking Calibrate runs the iterative calibration routine and reports a completion status.

**OS:** ANY

**Preconditions:**
    - CN0540 hardware board is connected

**Steps:**
    1. Expand the **Sensor Calibration** section and click **Calibrate**
        - **Expected result:** The calibration routine starts; the status label updates while the routine runs (up to 20 iterations)
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Wait for the calibration to complete
        - **Expected result:** The calibration status label shows a completion message and the Shift Voltage field reflects the adjusted DAC value
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

Test 13: Voltage Monitor: Pin Voltages Display
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0540.VOLT_MON_DISPLAY

**RBP:** P2

**Description:** Verify that the Voltage Monitor section displays numeric voltage readings for all six analog pins when a supported auxiliary ADC is present.

**OS:** ANY

**Preconditions:**
    - CN0540 hardware board is connected
    - An ``xadc`` or ``ltc2308`` auxiliary ADC device is present

**Steps:**
    1. Open the CN0540 plugin and expand the **Voltage Monitor** section
        - **Expected result:** The section is visible and displays six labeled voltage fields: Vin+, Vgpio2, Vgpio3, Vcom, Vfda+, Vfda-
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Observe the voltage values displayed in the section
        - **Expected result:** All six voltage labels show numeric values in millivolts read from the auxiliary ADC channels
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

Test 14: Voltage Monitor: Automatic Refresh
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.CN0540.VOLT_MON_AUTO_REFRESH

**RBP:** P3

**Description:** Verify that the Voltage Monitor section automatically updates all six pin voltages every second without user interaction.

**OS:** ANY

**Preconditions:**
    - CN0540 hardware board is connected
    - An ``xadc`` or ``ltc2308`` auxiliary ADC device is present

**Steps:**
    1. Open the CN0540 plugin, expand the **Voltage Monitor** section, and note the displayed values
        - **Expected result:** Six voltage labels are displayed with current readings
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Wait at least 2 seconds without clicking anything and observe the voltage labels
        - **Expected result:** The displayed voltage values refresh automatically (the labels update without requiring a manual Refresh button click)
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
