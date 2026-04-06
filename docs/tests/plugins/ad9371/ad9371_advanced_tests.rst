.. _ad9371_advanced_tests:

AD9371 Advanced Test Suite
================================================================================

.. note::

   User guide: :ref:`AD9371 user guide<ad9371>`

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
    - Scopy v2.2.0 or later with AD9371 plugin installed
    - AD9371 device connected
    - Device drivers and firmware are up to date
    - Reset .ini files to default by pressing **Reset** button in Preferences


Setup Environment
-----------------

.. _ad9371_adv_device_setup:

**AD9371.device:**
   - Connect an AD9371 evaluation board via Ethernet/USB to your PC
   - Ensure AD9371 IIO drivers are properly installed
   - Verify device firmware and driver compatibility
   - Connect to the device in Scopy with AD9371 plugin enabled

.. _ad9371_adv_device_emu:

**AD9371.emulator:**
   - Press the add device button and go to emulator
   - Select AD9371 device if available
   - Enable, verify and select all plugins available
   - Connect to the device


AD9371 Advanced Controls Tests
-------------------------------

Test 1: Advanced Plugin Loads
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_PLUGIN_LOADS

**RBP:** P0

**Description:** Verify that the AD9371 Advanced plugin loads in Scopy without errors.

**OS:** ANY

**Preconditions:**
    - Scopy is installed

**Steps:**
    1. Open Scopy application and connect to an AD9371 device.
        - **Expected result:** Scopy launches without errors and the AD9371 package is available.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Open the AD9371 Advanced plugin.
        - **Expected result:** The Advanced plugin loads and displays the navigation buttons (Clock Settings, Calibrations, TX, RX, etc.) and a Refresh button in the top bar.
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

Test 2: Clock Settings Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_CLK_SETTINGS_VISIBLE

**RBP:** P1

**Description:** Verify that the Clock Settings section is accessible and displays clock-related IIO attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup

**Steps:**
    1. In the AD9371 Advanced plugin, click the **Clock Settings** navigation button.
        - **Expected result:** The Clock Settings panel is displayed in the central area with clock attribute widgets.
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

Test 3: Calibrations Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_CALIBRATIONS_VISIBLE

**RBP:** P1

**Description:** Verify that the Calibrations section is accessible and displays calibration enable switches.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup

**Steps:**
    1. In the AD9371 Advanced plugin, click the **Calibrations** navigation button.
        - **Expected result:** The Calibrations panel is displayed with TX QEC, TX LO Leakage, TX LO Leakage Ext., TX BB Filter, and (if DPD-capable) DPD, CLGC, VSWR calibration switches.
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

Test 4: Advanced Calibration Mask Write
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_CALIBRATIONS_MASK_WRITE

**RBP:** P2

**Description:** Verify that toggling a calibration enable switch in the Advanced Calibrations section is written to the hardware.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup

**Steps:**
    1. In the **Calibrations** section, toggle the **TX QEC Cal** switch on or off.
        - **Expected result:** The switch changes state in the UI and the new value is written to the hardware ``calibrate_tx_qec_en`` attribute.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Click the Refresh button and navigate back to the Calibrations section.
        - **Expected result:** The TX QEC Cal switch reflects the value that was written.
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

Test 5: TX Settings Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_TX_SETTINGS_VISIBLE

**RBP:** P1

**Description:** Verify that the TX Settings section is accessible and displays TX-specific IIO attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup

**Steps:**
    1. In the AD9371 Advanced plugin, click the **TX Settings** navigation button.
        - **Expected result:** The TX Settings panel is displayed in the central area with TX attribute widgets.
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

Test 6: RX Settings Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_RX_SETTINGS_VISIBLE

**RBP:** P1

**Description:** Verify that the RX Settings section is accessible and displays RX-specific IIO attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup

**Steps:**
    1. In the AD9371 Advanced plugin, click the **RX Settings** navigation button.
        - **Expected result:** The RX Settings panel is displayed in the central area with RX attribute widgets.
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

Test 7: Observation Settings Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_OBS_SETTINGS_VISIBLE

**RBP:** P1

**Description:** Verify that the Observation RX Settings section is accessible and displays ORx-specific attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup

**Steps:**
    1. In the AD9371 Advanced plugin, click the **Obs Settings** navigation button.
        - **Expected result:** The Observation Settings panel is displayed with ORx attribute widgets.
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

Test 8: Gain Setup Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_GAIN_SETUP_VISIBLE

**RBP:** P2

**Description:** Verify that the Gain Setup section is accessible and displays gain table configuration attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup

**Steps:**
    1. In the AD9371 Advanced plugin, click the **Gain Setup** navigation button.
        - **Expected result:** The Gain Setup panel is displayed in the central area with gain configuration widgets.
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

Test 9: AGC Setup Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_AGC_SETUP_VISIBLE

**RBP:** P2

**Description:** Verify that the AGC Setup section is accessible and displays AGC configuration attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup

**Steps:**
    1. In the AD9371 Advanced plugin, click the **AGC Setup** navigation button.
        - **Expected result:** The AGC Setup panel is displayed in the central area with AGC attribute widgets.
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

Test 10: ARM GPIO Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_ARM_GPIO_VISIBLE

**RBP:** P2

**Description:** Verify that the ARM GPIO section is accessible and displays ARM GPIO pin configuration attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup

**Steps:**
    1. In the AD9371 Advanced plugin, click the **ARM GPIO** navigation button.
        - **Expected result:** The ARM GPIO panel is displayed in the central area with ARM GPIO attribute widgets.
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

Test 11: GPIO Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_GPIO_VISIBLE

**RBP:** P2

**Description:** Verify that the GPIO section is accessible and displays GPIO pin attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup

**Steps:**
    1. In the AD9371 Advanced plugin, click the **GPIO** navigation button.
        - **Expected result:** The GPIO panel is displayed in the central area with GPIO attribute widgets.
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

Test 12: AUX DAC Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_AUX_DAC_VISIBLE

**RBP:** P2

**Description:** Verify that the AUX DAC section is accessible and displays auxiliary DAC configuration attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup

**Steps:**
    1. In the AD9371 Advanced plugin, click the **AUX DAC** navigation button.
        - **Expected result:** The AUX DAC panel is displayed in the central area with DAC attribute widgets.
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

Test 13: JESD Framer Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_JESD_FRAMER_VISIBLE

**RBP:** P2

**Description:** Verify that the JESD Framer section is accessible and displays JESD framer configuration attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup

**Steps:**
    1. In the AD9371 Advanced plugin, click the **JESD Framer** navigation button.
        - **Expected result:** The JESD Framer panel is displayed in the central area with JESD framer attribute widgets.
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

Test 14: JESD Deframer Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_JESD_DEFRAMER_VISIBLE

**RBP:** P2

**Description:** Verify that the JESD Deframer section is accessible and displays JESD deframer configuration attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup

**Steps:**
    1. In the AD9371 Advanced plugin, click the **JESD Deframer** navigation button.
        - **Expected result:** The JESD Deframer panel is displayed in the central area with JESD deframer attribute widgets.
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

Test 15: BIST Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_BIST_VISIBLE

**RBP:** P2

**Description:** Verify that the BIST (Built-In Self Test) section is accessible and displays BIST controls.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup

**Steps:**
    1. In the AD9371 Advanced plugin, click the **BIST** navigation button.
        - **Expected result:** The BIST panel is displayed in the central area with BIST attribute widgets and controls.
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

Test 16: DPD Settings Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_DPD_SETTINGS_VISIBLE

**RBP:** P3

**Description:** Verify that the DPD Settings section is accessible on DPD-capable hardware and displays DPD attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup
    - AD9371 device must support DPD (``dpd_tracking_en`` attribute present on TX channel)

**Steps:**
    1. In the AD9371 Advanced plugin, click the **DPD Settings** navigation button.
        - **Expected result:** The DPD Settings panel is displayed in the central area with DPD tracking, actuator, and status attribute widgets.
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

Test 17: CLGC Settings Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_CLGC_SETTINGS_VISIBLE

**RBP:** P3

**Description:** Verify that the CLGC Settings section is accessible on DPD-capable hardware and displays CLGC attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup
    - AD9371 device must support DPD/CLGC

**Steps:**
    1. In the AD9371 Advanced plugin, click the **CLGC Settings** navigation button.
        - **Expected result:** The CLGC Settings panel is displayed in the central area with CLGC tracking, gain, and status attribute widgets.
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

Test 18: VSWR Settings Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_VSWR_SETTINGS_VISIBLE

**RBP:** P3

**Description:** Verify that the VSWR Settings section is accessible on DPD-capable hardware and displays VSWR attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup
    - AD9371 device must support DPD/VSWR

**Steps:**
    1. In the AD9371 Advanced plugin, click the **VSWR Settings** navigation button.
        - **Expected result:** The VSWR Settings panel is displayed in the central area with VSWR tracking, forward/reflected gain, and status attribute widgets.
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

Test 19: Advanced Refresh Button Updates Values
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_REFRESH_BUTTON

**RBP:** P1

**Description:** Verify that clicking the Refresh button in the Advanced plugin reads all widget values from hardware and updates the UI.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_adv_device_setup>` setup

**Steps:**
    1. Navigate to any section in the AD9371 Advanced plugin (e.g., Clock Settings).
        - **Expected result:** The section displays IIO attribute values read from hardware.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Click the **Refresh** button in the top bar.
        - **Expected result:** The button shows an animation while refreshing, then stops. All visible IIO widget values update from hardware.
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
