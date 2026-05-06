.. _fmcomms11_tests:

FMCOMMS11 Test Suite
================================================================================

**Prerequisites:**
    - Scopy v2.2.0 or later with FMCOMMS11 plugin installed
    - FMCOMMS11 evaluation board connected
    - Device drivers and firmware are up to date
    - Reset .ini files to default by pressing **Reset** button in Preferences


Setup Environment
-----------------

.. _fmcomms11_device_setup:

**FMCOMMS11.device:**
   - Connect an FMCOMMS11 evaluation board via Ethernet/USB to your PC
   - Ensure all four IIO devices are present: ``axi-ad9625-hpc``, ``axi-ad9162-hpc``, ``hmc1119``, ``adl5240``
   - Verify device drivers and firmware are up to date
   - Connect to the device in Scopy with the FMCOMMS11 plugin enabled


FMCOMMS11 Controls Tests
-------------------------

Test 1: Plugin Loads
~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.PLUGIN_LOADS

**RBP:** P0

**Description:** Verify that the FMCOMMS11 plugin loads in Scopy without errors.

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
    2. Open the FMCOMMS11 plugin
        - **Expected result:** FMCOMMS11 plugin loads and is accessible in the UI
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

Test 2: Device Detection
~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.DEVICE_DETECTION

**RBP:** P3

**Description:** Verify that the FMCOMMS11 plugin detects all required IIO devices and displays the plugin tool.

**OS:** ANY

**Preconditions:**
    - Use :ref:`FMCOMMS11.device <fmcomms11_device_setup>` setup

**Steps:**
    1. Connect to the FMCOMMS11 board in Scopy
        - **Expected result:** Plugin detects all four devices (axi-ad9625-hpc, axi-ad9162-hpc, hmc1119, adl5240) and the FMCOMMS11 tool entry appears in the device tool list
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Open the FMCOMMS11 tool
        - **Expected result:** The tool opens showing ADC, Input Attenuator, DDS, DAC, and Output VGA sections
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

Test 3: ADC Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.ADC_SECTION_VISIBLE

**RBP:** P1

**Description:** Verify that the ADC section is visible and contains the expected controls.

**OS:** ANY

**Preconditions:**
    - Use :ref:`FMCOMMS11.device <fmcomms11_device_setup>` setup

**Steps:**
    1. Open the FMCOMMS11 tool
        - **Expected result:** The ADC section is visible and contains the Sampling Frequency display, Test Mode combo box, and Scale combo box
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

Test 4: ADC Sampling Frequency Display
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.ADC_SAMPLING_FREQ_DISPLAY

**RBP:** P1

**Description:** Verify that the ADC sampling frequency is displayed as a read-only value in MHz.

**OS:** ANY

**Preconditions:**
    - Use :ref:`FMCOMMS11.device <fmcomms11_device_setup>` setup

**Steps:**
    1. Open the FMCOMMS11 tool and observe the ADC Sampling Frequency field
        - **Expected result:** The ADC Sampling Frequency field is read-only and displays a non-zero value in MHz (e.g., 2500.000000 MHz for AD9625 at 2.5 GSPS)
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Verify the field cannot be edited by the user
        - **Expected result:** The sampling frequency text field is disabled and does not accept user input
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

Test 5: ADC Test Mode Selection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.ADC_TEST_MODE

**RBP:** P1

**Description:** Verify that the ADC test mode combo box populates from the device and applies the selected mode.

**OS:** ANY

**Preconditions:**
    - Use :ref:`FMCOMMS11.device <fmcomms11_device_setup>` setup

**Steps:**
    1. Open the FMCOMMS11 tool and inspect the ADC Test Mode combo box
        - **Expected result:** The combo box is populated with options from the ``test_mode_available`` IIO attribute (e.g., off, midscale_short, fs, etc.)
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Select a different test mode from the combo box
        - **Expected result:** The selected test mode is applied to the ``axi-ad9625-hpc`` device ``voltage0`` channel ``test_mode`` attribute
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Select ``off`` to restore the default test mode
        - **Expected result:** Test mode returns to ``off`` on the device
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

Test 6: ADC Scale Selection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.ADC_SCALE

**RBP:** P1

**Description:** Verify that the ADC scale combo box populates from the device and applies the selected scale.

**OS:** ANY

**Preconditions:**
    - Use :ref:`FMCOMMS11.device <fmcomms11_device_setup>` setup

**Steps:**
    1. Open the FMCOMMS11 tool and inspect the ADC Scale combo box
        - **Expected result:** The combo box is populated with options from the ``scale_available`` IIO attribute
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Select a different scale value from the combo box
        - **Expected result:** The selected scale is applied to the ``axi-ad9625-hpc`` device ``voltage0`` channel ``scale`` attribute
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

Test 7: Input Attenuator Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.INPUT_ATTN_SECTION_VISIBLE

**RBP:** P1

**Description:** Verify that the Input Attenuator section is visible and contains the HMC1119 gain control.

**OS:** ANY

**Preconditions:**
    - Use :ref:`FMCOMMS11.device <fmcomms11_device_setup>` setup

**Steps:**
    1. Open the FMCOMMS11 tool and locate the Input Attenuator section
        - **Expected result:** The Input Attenuator section is visible and contains a spin box for the HMC1119 hardwaregain control
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

Test 8: HMC1119 Attenuator Gain
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.HMC1119_GAIN

**RBP:** P1

**Description:** Verify that the HMC1119 attenuator gain spin box applies the value to the device within its valid range.

**OS:** ANY

**Preconditions:**
    - Use :ref:`FMCOMMS11.device <fmcomms11_device_setup>` setup

**Steps:**
    1. Open the FMCOMMS11 tool and locate the HMC1119 hardwaregain spin box in the Input Attenuator section
        - **Expected result:** Spin box is present with range -31.75 to 0 dB in 0.25 dB steps
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Set the attenuator gain to -15.75 dB
        - **Expected result:** The value -15.75 is applied to the ``hmc1119`` device ``voltage0`` output ``hardwaregain`` attribute
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Set the attenuator gain to 0 dB (maximum, minimum attenuation)
        - **Expected result:** The value 0 is applied and the device reflects the minimum attenuation
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

Test 9: DDS Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.DDS_SECTION_VISIBLE

**RBP:** P2

**Description:** Verify that the DDS section is visible and the DAC data manager widget is present.

**OS:** ANY

**Preconditions:**
    - Use :ref:`FMCOMMS11.device <fmcomms11_device_setup>` setup

**Steps:**
    1. Open the FMCOMMS11 tool and locate the DDS section
        - **Expected result:** The DDS section is visible and contains the DAC data manager widget with DDS tone controls or buffer mode selection
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

Test 10: DAC Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.DAC_SECTION_VISIBLE

**RBP:** P1

**Description:** Verify that the DAC section is visible and contains the expected controls.

**OS:** ANY

**Preconditions:**
    - Use :ref:`FMCOMMS11.device <fmcomms11_device_setup>` setup

**Steps:**
    1. Open the FMCOMMS11 tool and locate the DAC section
        - **Expected result:** The DAC section is visible and contains the Sampling Frequency display, NCO Frequency spin box, and FIR85 enable checkbox
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

Test 11: DAC Sampling Frequency Display
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.DAC_SAMPLING_FREQ_DISPLAY

**RBP:** P1

**Description:** Verify that the DAC sampling frequency is displayed as a read-only value in MHz.

**OS:** ANY

**Preconditions:**
    - Use :ref:`FMCOMMS11.device <fmcomms11_device_setup>` setup

**Steps:**
    1. Open the FMCOMMS11 tool and observe the DAC Sampling Frequency field in the DAC section
        - **Expected result:** The DAC Sampling Frequency field is read-only and displays a non-zero value in MHz
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Verify the field cannot be edited by the user
        - **Expected result:** The sampling frequency text field is disabled and does not accept user input
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

Test 12: DAC NCO Frequency
~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.DAC_NCO_FREQUENCY

**RBP:** P1

**Description:** Verify that the DAC NCO frequency spin box applies the value to the device in MHz.

**OS:** ANY

**Preconditions:**
    - Use :ref:`FMCOMMS11.device <fmcomms11_device_setup>` setup

**Steps:**
    1. Open the FMCOMMS11 tool and locate the NCO Frequency spin box in the DAC section
        - **Expected result:** Spin box is present with range 0 to 6000 MHz
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Set the NCO frequency to 100 MHz
        - **Expected result:** The value 100000000 Hz (100 MHz) is written to the ``axi-ad9162-hpc`` device ``altvoltage4`` or ``altvoltage2`` output channel ``frequency_nco`` attribute
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Set the NCO frequency to 0 MHz
        - **Expected result:** NCO frequency is set to 0 Hz on the device
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

Test 13: DAC FIR85 Filter Enable
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.DAC_FIR85_ENABLE

**RBP:** P1

**Description:** Verify that the FIR85 filter enable checkbox toggles the FIR85 filter on the DAC device.

**OS:** ANY

**Preconditions:**
    - Use :ref:`FMCOMMS11.device <fmcomms11_device_setup>` setup

**Steps:**
    1. Open the FMCOMMS11 tool and locate the FIR85 checkbox in the DAC section
        - **Expected result:** The FIR85 checkbox is present and reflects the current device state
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Toggle the FIR85 checkbox to enabled
        - **Expected result:** The ``fir85_enable`` attribute on the ``axi-ad9162-hpc`` device ``voltage0`` output channel is set to 1
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Toggle the FIR85 checkbox to disabled
        - **Expected result:** The ``fir85_enable`` attribute is set to 0
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

Test 14: Output VGA Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.OUTPUT_VGA_SECTION_VISIBLE

**RBP:** P1

**Description:** Verify that the Output VGA section is visible and contains the ADL5240 gain control.

**OS:** ANY

**Preconditions:**
    - Use :ref:`FMCOMMS11.device <fmcomms11_device_setup>` setup

**Steps:**
    1. Open the FMCOMMS11 tool and locate the Output VGA section
        - **Expected result:** The Output VGA section is visible and contains a spin box for the ADL5240 hardwaregain control
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

Test 15: ADL5240 VGA Gain
~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.ADL5240_GAIN

**RBP:** P1

**Description:** Verify that the ADL5240 VGA gain spin box applies the value to the device within its valid range.

**OS:** ANY

**Preconditions:**
    - Use :ref:`FMCOMMS11.device <fmcomms11_device_setup>` setup

**Steps:**
    1. Open the FMCOMMS11 tool and locate the ADL5240 hardwaregain spin box in the Output VGA section
        - **Expected result:** Spin box is present with range -11.5 to 20 dB in 0.5 dB steps
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Set the VGA gain to 10 dB
        - **Expected result:** The value 10 is applied to the ``adl5240`` device ``voltage0`` output ``hardwaregain`` attribute
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Set the VGA gain to -11.5 dB (minimum)
        - **Expected result:** The value -11.5 is applied and the device reflects the minimum gain
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

Test 16: Refresh Functionality
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.FMCOMMS11.REFRESH_FUNCTION

**RBP:** P1

**Description:** Verify that the refresh button reads back current attribute values from all devices and updates the UI.

**OS:** ANY

**Preconditions:**
    - Use :ref:`FMCOMMS11.device <fmcomms11_device_setup>` setup

**Steps:**
    1. Open the FMCOMMS11 tool and note the current displayed values
    2. Change a device attribute externally (e.g., via command line using ``iio_attr``)
    3. Click the refresh button in the FMCOMMS11 plugin
        - **Expected result:** All displayed values are updated to reflect the current device state, including ADC/DAC sampling frequencies, test mode, scale, NCO frequency, FIR85 state, and gain values
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
