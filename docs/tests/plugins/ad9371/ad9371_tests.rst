.. _ad9371_tests:

AD9371 Test Suite
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

.. _ad9371_device_setup:

**AD9371.device:**
   - Connect an AD9371 evaluation board via Ethernet/USB to your PC
   - Ensure AD9371 IIO drivers are properly installed
   - Verify device firmware and driver compatibility
   - Connect to the device in Scopy with AD9371 plugin enabled

.. _ad9371_device_emu:

**AD9371.emulator:**
   - Press the add device button and go to emulator
   - Select AD9371 device if available
   - Enable, verify and select all plugins available
   - Connect to the device


AD9371 Controls Tests
---------------------

Test 1: Plugin Loads
~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.UI_PLUGIN_LOADS

**RBP:** P0

**Description:** Verify that the AD9371 plugin loads in Scopy without errors.

**OS:** ANY

**Preconditions:**
    - Scopy is installed

**Steps:**
    1. Open Scopy application.
        - **Expected result:** Scopy launches without errors.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Connect to an AD9371 device and open the AD9371 plugin.
        - **Expected result:** AD9371 plugin loads and is accessible in the UI with no error dialogs.
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

Test 2: Device Detection and Display
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.DEVICE_DETECTION

**RBP:** P1

**Description:** Verify that the AD9371 plugin detects the connected device and displays the main tool UI.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. Open the AD9371 plugin.
        - **Expected result:** The plugin displays the Controls view with collapsible sections and a Refresh button in the top bar.
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

Test 3: Global Settings Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.GLOBAL_SETTINGS_VISIBLE

**RBP:** P1

**Description:** Verify that the AD9371 Global Settings section is visible and contains ENSM Mode, Calibrations, and Load Profile controls.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. Open the AD9371 plugin and expand the **AD9371 Global Settings** section.
        - **Expected result:** The section expands and shows the ENSM Mode combo box, Calibrations panel (with CAL RX QEC, CAL TX QEC, CAL TX LOL, CAL TX LOL Ext. switches and a CALIBRATE button), and Load Profile file browser.
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

Test 4: ENSM Mode Write-Readback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ENSM_MODE_WRITE

**RBP:** P1

**Description:** Verify that changing ENSM Mode is applied to the hardware and reflected in the UI.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. In the **AD9371 Global Settings** section, note the current ENSM Mode value.
        - **Expected result:** The ENSM Mode combo box shows the current hardware state (e.g., ``fdd`` or ``alert``).
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Select a different ENSM Mode from the combo box.
        - **Expected result:** The new mode is applied to the hardware. Clicking Refresh shows the updated value.
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

Test 5: Calibration Mask Write
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.CALIBRATION_MASK_WRITE

**RBP:** P1

**Description:** Verify that calibration enable switches are persisted and written to the hardware when Calibrate is triggered.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. In the **AD9371 Global Settings** Calibrations panel, toggle **CAL RX QEC** on or off.
        - **Expected result:** The switch changes state in the UI.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Click the **CALIBRATE** button.
        - **Expected result:** The calibration completes without error and the UI refreshes to reflect the updated calibration state.
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

Test 6: Calibrate Button Triggers Calibration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.UI_CALIBRATE_TRIGGER

**RBP:** P2

**Description:** Verify that pressing the CALIBRATE button writes all enabled calibration flags to hardware and triggers device calibration.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. Enable **CAL TX QEC** and **CAL TX LOL** switches in the Calibrations panel.
        - **Expected result:** Both switches show as enabled.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Click the **CALIBRATE** button.
        - **Expected result:** No error is shown; the tool emits a read-refresh after calibration completes.
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

Test 7: Load Profile from File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.LOAD_PROFILE

**RBP:** P2

**Description:** Verify that selecting a profile file loads it onto the AD9371 device and triggers a UI refresh.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. In the **AD9371 Global Settings** section, click the **Load Profile** file browser and select a valid ``.txt`` profile file.
        - **Expected result:** The file path appears in the browser's text field and the profile is sent to the device without error.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Observe the Controls section values after loading.
        - **Expected result:** RF bandwidth and sampling rate values update to reflect the newly loaded profile.
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

Test 8: RX Chain Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.RX_CHAIN_VISIBLE

**RBP:** P1

**Description:** Verify that the AD9371 Receive Chain section is visible and displays RX attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. Expand the **AD9371 Receive Chain** section.
        - **Expected result:** The section shows RF Bandwidth (read-only), Gain Control Mode combo, Hardware Gain, Quadrature Tracking, RSSI (read-only), Sampling Frequency, and LO Frequency controls.
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

Test 9: RX Gain Control Mode Write-Readback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.RX_GAIN_CONTROL_MODE_WRITE

**RBP:** P1

**Description:** Verify that changing the RX Gain Control Mode is applied to the hardware and the UI reflects the new value.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. In the **AD9371 Receive Chain** section, note the current **Gain Control Mode** value.
        - **Expected result:** The combo box shows the current hardware value (e.g., ``slow_attack``).
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Select a different Gain Control Mode (e.g., ``manual``).
        - **Expected result:** The new mode is applied to the hardware. Clicking Refresh confirms the value persisted.
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

Test 10: RX LO Frequency Write-Readback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.RX_LO_FREQUENCY_WRITE

**RBP:** P1

**Description:** Verify that writing a new RX LO frequency updates the hardware and the UI reflects the new value.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. In the **AD9371 Receive Chain** section, note the current **LO Frequency** value.
        - **Expected result:** The widget displays the current hardware LO frequency in Hz.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Enter a new valid LO frequency value and confirm.
        - **Expected result:** The hardware is updated. Clicking Refresh shows the new LO frequency value.
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

Test 11: RX Hardware Gain Write-Readback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.RX_HARDWARE_GAIN_WRITE

**RBP:** P2

**Description:** Verify that writing RX Hardware Gain (in manual gain control mode) is applied and reflected in the UI.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup
    - RX Gain Control Mode set to ``manual``

**Steps:**
    1. In the **AD9371 Receive Chain** section, modify the **Hardware Gain** spinbox for channel 0.
        - **Expected result:** The new gain value is written to the hardware.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Click the Refresh button.
        - **Expected result:** The Hardware Gain widget displays the value that was written.
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

Test 12: TX Chain Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.TX_CHAIN_VISIBLE

**RBP:** P1

**Description:** Verify that the AD9371 Transmit Chain section is visible and displays TX attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. Expand the **AD9371 Transmit Chain** section.
        - **Expected result:** The section shows RF Bandwidth (read-only), TX Attenuation (channel 0), Quadrature Tracking, LO Leakage Tracking, Sampling Frequency, and LO Frequency controls.
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

Test 13: TX LO Frequency Write-Readback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.TX_LO_FREQUENCY_WRITE

**RBP:** P1

**Description:** Verify that writing a new TX LO frequency updates the hardware and the UI reflects the new value.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. In the **AD9371 Transmit Chain** section, note the current **LO Frequency** value.
        - **Expected result:** The widget displays the current hardware TX LO frequency in Hz.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Enter a new valid TX LO frequency value and confirm.
        - **Expected result:** The hardware is updated. Clicking Refresh shows the new TX LO frequency value.
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

Test 14: TX Attenuation Write-Readback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.TX_ATTENUATION_WRITE

**RBP:** P1

**Description:** Verify that writing TX Attenuation for channel 0 is applied and reflected in the UI.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. In the **AD9371 Transmit Chain** section, modify the **TX Attenuation** spinbox for channel 0.
        - **Expected result:** The new attenuation value is written to the hardware without error.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Click the Refresh button.
        - **Expected result:** The TX Attenuation widget shows the value that was written.
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

Test 15: Observation/Sniffer RX Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.OBS_CHAIN_VISIBLE

**RBP:** P1

**Description:** Verify that the AD9371 Observation/Sniffer Receive Chain section is visible and displays ORx attributes.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. Expand the **AD9371 Observation/Sniffer Receive Chain** section.
        - **Expected result:** The section shows RF Bandwidth (read-only), Gain Control Mode, Hardware Gain, Quadrature Tracking, RSSI (read-only), RF Port Select, Temp Comp Gain, and Sniffer LO Frequency controls.
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

Test 16: Obs RF Port Select Write-Readback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.OBS_RF_PORT_SELECT_WRITE

**RBP:** P1

**Description:** Verify that selecting a different Obs RF Port updates the hardware and is reflected in the UI.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. In the **Observation/Sniffer Receive Chain** section, note the current **RF Port Select** value.
        - **Expected result:** The combo box shows the current hardware port selection.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Select a different RF Port option.
        - **Expected result:** The new port is applied to the hardware. Clicking Refresh confirms the value persisted.
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

Test 17: Obs Gain Control Mode Write-Readback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.OBS_GAIN_CONTROL_MODE_WRITE

**RBP:** P2

**Description:** Verify that changing the Observation channel Gain Control Mode is applied and reflected in the UI.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. In the **Observation/Sniffer Receive Chain** section, change the **Gain Control Mode** to a different option.
        - **Expected result:** The new mode is applied to the hardware. Clicking Refresh shows the updated value.
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

Test 18: FPGA Settings Section Visible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.FPGA_SETTINGS_VISIBLE

**RBP:** P1

**Description:** Verify that the FPGA Settings section is visible and shows FPGA frequency and sampling rate controls.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. Expand the **FPGA Settings** section.
        - **Expected result:** The section displays FPGA TX Frequency, FPGA RX Frequency combo boxes, TX Sampling Rate and RX Sampling Rate read-only labels, and Phase Rotation spinboxes.
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

Test 19: FPGA TX Frequency Write-Readback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.FPGA_TX_FREQUENCY_WRITE

**RBP:** P1

**Description:** Verify that selecting an FPGA TX Frequency updates the hardware and the TX Sampling Rate label updates accordingly.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. In the **FPGA Settings** section, select a different option in the **FPGA TX Frequency** combo box.
        - **Expected result:** The new frequency is applied to the hardware and the TX Sampling Rate label updates to reflect the new value.
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

Test 20: FPGA RX Frequency Write-Readback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.FPGA_RX_FREQUENCY_WRITE

**RBP:** P1

**Description:** Verify that selecting an FPGA RX Frequency updates the hardware and the RX Sampling Rate label updates accordingly.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. In the **FPGA Settings** section, select a different option in the **FPGA RX Frequency** combo box.
        - **Expected result:** The new frequency is applied to the hardware and the RX Sampling Rate label updates to reflect the new value.
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

Test 21: Block Diagram View Displays
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.BLOCK_DIAGRAM_VIEW

**RBP:** P3

**Description:** Verify that the Block Diagram view displays the AD9371 block diagram image without errors.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. Click the **Block Diagram** button in the top bar of the AD9371 plugin.
        - **Expected result:** The view switches to show the AD9371 block diagram image centered in a scrollable area.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Click the **Controls** button to return to the controls view.
        - **Expected result:** The view switches back to the Controls layout with all sections intact.
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

Test 22: Refresh Button Updates Values
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.REFRESH_BUTTON

**RBP:** P1

**Description:** Verify that clicking the Refresh button reads all IIO widget values from hardware and updates the UI.

**OS:** ANY

**Preconditions:**
    - Use :ref:`AD9371.device <ad9371_device_setup>` setup

**Steps:**
    1. Click the **Refresh** button in the top bar.
        - **Expected result:** The button shows an animation while refreshing, then stops. All IIO widget values in the Controls view are updated from hardware.
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
