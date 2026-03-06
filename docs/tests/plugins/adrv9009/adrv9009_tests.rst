.. _adrv9009_tests:

ADRV9009 Plugin - Test Suite
======================================

.. note::

   User guide: :ref:`ADRV9009 user guide<adrv9009>`

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
    - Scopy v2.2.0 or later with ADRV9009 plugin installed
    - ADRV9009 device connected
    - Device drivers and firmware are up to date
    - Reset .ini files to default by pressing **Reset** button in Preferences


Setup Environment
-----------------

.. _adrv9009_device_setup:

**ADRV9009.device:**
   - Connect an ADRV9009 evaluation board via Ethernet/USB to your PC
   - Ensure ADRV9009 IIO drivers are properly installed
   - Verify device firmware and driver compatibility
   - Connect to the device in Scopy with ADRV9009 plugin enabled

.. _adrv9009_device_emu:

**ADRV9009.emulator:**
   - Press the add device button and go to emulator
   - Select ADRV9009 device if available
   - Enable, verify and select all plugins available
   - Connect to the device

.. _adrv9009_device_default:

**ADRV9009.default:**
   - Use default device settings after connection
   - Load a known-good profile configuration
   - Ensure all channels are in a stable state


ADRV9009 Controls Tests
------------------------

Test 1: Plugin Loads and Displays ADRV9009 Device
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.CONTROLS.PLUGIN_LOADS

**RBP:** P0

**Description:** Verify that the ADRV9009 plugin loads successfully and the ADRV9009 tool appears in the tool list.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Open Scopy application
   2. Connect to ADRV9009 device
   3. Verify that the ADRV9009 tool appears in the tool list
      - **Expected result:** ADRV9009 tool is visible and accessible in the tool list
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.CONTROLS.DEVICE_DETECTION

**RBP:** P3

**Description:** Verify that the ADRV9009 tool displays all main sections: global settings, RX/TX/OBS chains and FPGA section.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Open the ADRV9009 tool
   2. Verify the Global Settings section is present
      - **Expected result:** Global Settings section is visible with ENSM mode and profile browser controls
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Verify RX1 and RX2 channel sections are present
      - **Expected result:** RX1 and RX2 controls are displayed
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Verify TX1 and TX2 channel sections are present
      - **Expected result:** TX1 and TX2 controls are displayed
      - **Actual result:**

..
  Actual test result goes here.
..

   5. Verify OBS RX channel sections are present
      - **Expected result:** OBS1 and OBS2 controls are displayed
      - **Actual result:**

..
  Actual test result goes here.
..

   6. Verify FPGA section is present
      - **Expected result:** FPGA phase rotation controls are displayed
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


Test 3: Global Settings — ENSM Mode
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.CONTROLS.GLOBAL_SETTINGS

**RBP:** P3

**Description:** Verify that the ENSM mode control in Global Settings changes the device state correctly.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Note the current ENSM mode value
   2. Change ENSM mode to a different value (e.g., alert)
      - **Expected result:** UI updates to reflect the new ENSM mode
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Read back the ENSM mode from hardware
      - **Expected result:** Hardware reflects the updated ENSM mode value
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


Test 4: Profile Configuration Load
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.CONTROLS.PROFILE_LOAD

**RBP:** P3

**Description:** Verify that a device profile can be loaded via the profile browser and channel rates update accordingly.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup
   - A valid ADRV9009 .txt profile file is available

**Steps:**
   1. Note the current channel sample rates in the UI
   2. Click the profile browser button in Global Settings
      - **Expected result:** File browser opens for selecting a profile
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Select a valid .txt profile file
      - **Expected result:** Profile is loaded and the device reconfigures
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Verify channel sample rates update after load
      - **Expected result:** RX and TX sample rates reflect the new profile values
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


Test 5: TRX LO Frequency Control
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.CONTROLS.TRX_LO_FREQUENCY

**RBP:** P3

**Description:** Verify that the TRX LO frequency control operates within the valid range and updates the hardware.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Note the current TRX LO frequency value
   2. Change the TRX LO frequency to a new value within the valid range (70–6000 MHz)
      - **Expected result:** UI updates to reflect the new frequency
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Read back the frequency from hardware
      - **Expected result:** Hardware reflects the updated frequency value
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Enable frequency hopping mode (if supported)
      - **Expected result:** Frequency hopping mode activates without error
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


Test 6: Calibration Controls
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.CONTROLS.CALIBRATIONS

**RBP:** P3

**Description:** Verify that calibration checkboxes can be toggled and the CALIBRATE button triggers the calibration sequence.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Enable calibration checkboxes (e.g., RX QEC, TX QEC, TX LOL)
      - **Expected result:** Checkboxes toggle without error
      - **Actual result:**

..
  Actual test result goes here.
..

   2. Disable one or more calibration checkboxes
      - **Expected result:** Unchecked calibrations are deselected
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


Test 7: RX Channel Controls (RX1/RX2)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.CONTROLS.RX_CHANNEL_CONFIG

**RBP:** P3

**Description:** Verify RX1 and RX2 channel controls: gain control mode, hardware gain, powerdown and tracking toggles.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Change the RX section gain control mode to manual
      - **Expected result:** Manual gain widget becomes editable
      - **Actual result:**

..
  Actual test result goes here.
..

   2. Change the RX section gain control mode to automatic or hybrid
      - **Expected result:** Manual gain widget becomes read-only
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Set RX1 hardware gain to a new value in manual mode
      - **Expected result:** Gain readback matches the written value
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Toggle RX1 powerdown enable/disable
      - **Expected result:** Powerdown control functions correctly
      - **Actual result:**

..
  Actual test result goes here.
..

   5. Toggle RX1 Quadrature tracking enable/disable
      - **Expected result:** Quadrature tracking checkbox toggles without error
      - **Actual result:**

..
  Actual test result goes here.
..

   6. Toggle RX1 HD2 tracking enable/disable
      - **Expected result:** HD2 tracking checkbox toggles without error
      - **Actual result:**

..
  Actual test result goes here.
..

   7. Repeat steps 1–6 for RX2
      - **Expected result:** RX2 controls behave identically to RX1
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


Test 8: TX Channel Controls (TX1/TX2)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.CONTROLS.TX_CHANNEL_CONFIG

**RBP:** P3

**Description:** Verify TX1 and TX2 channel controls: attenuation, powerdown and tracking toggles.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Change TX1 attenuation to a new value (range 0–41.95 dB)
      - **Expected result:** Attenuation readback matches the written value
      - **Actual result:**

..
  Actual test result goes here.
..

   2. Toggle TX1 powerdown enable/disable
      - **Expected result:** Powerdown control functions correctly
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Toggle TX1 Quadrature tracking enable/disable
      - **Expected result:** Quadrature tracking checkbox toggles without error
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Toggle TX1 LO Leakage tracking enable/disable
      - **Expected result:** LO Leakage tracking checkbox toggles without error
      - **Actual result:**

..
  Actual test result goes here.
..

   5. Repeat steps 1–4 for TX2
      - **Expected result:** TX2 controls behave identically to TX1
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


Test 9: Observation RX Controls (OBS1/OBS2)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.CONTROLS.OBS_CHANNEL_CONFIG

**RBP:** P3

**Description:** Verify observation RX channel controls: hardware gain, LO source, AUX LO frequency and powerdown.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Change OBS1 hardware gain to a new value
      - **Expected result:** Gain readback matches the written value
      - **Actual result:**

..
  Actual test result goes here.
..

   2. Change the LO source for OBS1 (RFPLL/AUXPLL)
      - **Expected result:** LO source selection updates without error
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Change the AUX LO frequency
      - **Expected result:** AUX LO frequency updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Toggle OBS1 powerdown enable/disable
      - **Expected result:** Powerdown control functions correctly
      - **Actual result:**

..
  Actual test result goes here.
..

   5. Repeat steps 1–4 for OBS2
      - **Expected result:** OBS2 controls behave identically to OBS1
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


Test 10: FPGA Phase Rotation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.CONTROLS.FPGA_PHASE_ROTATION

**RBP:** P3

**Description:** Verify that the FPGA phase rotation controls update correctly for RX1 and RX2.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Note the current RX1 phase rotation value in the FPGA section
   2. Change the RX1 phase rotation to a new value
      - **Expected result:** UI updates to reflect the new phase rotation value
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Change the RX2 phase rotation to a new value
      - **Expected result:** UI updates to reflect the new RX2 phase rotation value
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


Test 11: Refresh Functionality
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.CONTROLS.REFRESH_FUNCTION

**RBP:** P3

**Description:** Verify that the refresh button updates all controls with current values from the device.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Note current control values displayed in the UI
   2. Change device settings externally (using another tool or command line)
   3. Click the refresh button in the ADRV9009 tool
      - **Expected result:** All controls update to reflect the current device state
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


Test 12: Multi-Chip Sync (multi-device only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.CONTROLS.MCS_SYNC

**RBP:** P3

**Description:** Verify that the MCS Sync button is visible when multiple ADRV9009-PHY devices are connected and triggers synchronization.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup
   - Two or more ADRV9009-PHY devices must be connected

**Steps:**
   1. Connect two ADRV9009-PHY devices
   2. Open the ADRV9009 tool and verify the MCS Sync button is visible
      - **Expected result:** MCS Sync button is present in the UI
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Click the MCS Sync button
      - **Expected result:** Multi-chip synchronization sequence completes without error
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


ADRV9009 Advanced Tests
------------------------

Test 1: Advanced Tool Loads and Displays Tabs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.ADVANCED.PLUGIN_DETECTION

**RBP:** P3

**Description:** Verify that the ADRV9009 Advanced tool loads and displays all 15 configuration tabs.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Open the ADRV9009 Advanced tool
   2. Verify all 15 tabs are displayed: CLK Settings, Calibrations, TX Settings, RX Settings, ORX Settings, FHM Setup, PA Protection, GAIN Setup, AGC Setup, ARM GPIO, AUX DAC, JESD204 Settings, JESD Framer, JESD Deframer, BIST
      - **Expected result:** All 15 tabs are visible and accessible
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


Test 2: Clock Settings Tab
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.ADVANCED.CLK_SETTINGS

**RBP:** P3

**Description:** Verify that Clock Settings tab controls update device clock, VCO frequency and HS divider.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Navigate to the CLK Settings tab in the ADRV9009 Advanced tool
   2. Change the device clock frequency to a new value
      - **Expected result:** Device clock value updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Change the VCO frequency
      - **Expected result:** VCO frequency updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Change the HS (high-speed) divider setting
      - **Expected result:** HS divider value updates and readback matches
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


Test 3: RX Profile and Channel Settings
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.ADVANCED.RX_SETTINGS

**RBP:** P3

**Description:** Verify that RX Settings tab controls update FIR decimation, output rate, RF bandwidth and RX channel enable.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Navigate to the RX Settings tab
   2. Change the FIR decimation value
      - **Expected result:** FIR decimation updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Change the RX output rate
      - **Expected result:** Output rate updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Change the RX RF bandwidth
      - **Expected result:** RF bandwidth updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   5. Change the RX channels enable setting (OFF / RX1 / RX2 / RX1_and_RX2)
      - **Expected result:** Channel enable selection updates without error
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


Test 4: TX Profile and Channel Settings
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.ADVANCED.TX_SETTINGS

**RBP:** P3

**Description:** Verify that TX Settings tab controls update FIR interpolation, TX input rate, attenuation step size and TX1/TX2 attenuation.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Navigate to the TX Settings tab
   2. Change the FIR interpolation value
      - **Expected result:** FIR interpolation updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Change the TX input rate
      - **Expected result:** TX input rate updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Change the TX attenuation step size
      - **Expected result:** Attenuation step size updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   5. Change TX1 attenuation
      - **Expected result:** TX1 attenuation updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   6. Change TX2 attenuation
      - **Expected result:** TX2 attenuation updates and readback matches
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


Test 5: Observation RX Profile Settings
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.ADVANCED.ORX_SETTINGS

**RBP:** P3

**Description:** Verify that ORX Settings tab controls update ORX output rate, RF bandwidth and observation channels enable.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Navigate to the ORX Settings tab
   2. Change the ORX output rate
      - **Expected result:** ORX output rate updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Change the ORX RF bandwidth
      - **Expected result:** RF bandwidth updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Change the observation channels enable setting
      - **Expected result:** Channel enable selection updates without error
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


Test 6: JESD204 Physical Layer Settings
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.ADVANCED.JESD_SETTINGS

**RBP:** P3

**Description:** Verify that the JESD204 Settings tab controls update serializer amplitude, pre-emphasis, lane polarity, deserializer EQ and SYSREF LVDS mode.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Navigate to the JESD204 Settings tab
   2. Change the serializer amplitude (range 0–15)
      - **Expected result:** SER amplitude updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Change the serializer pre-emphasis (range 0–4)
      - **Expected result:** SER pre-emphasis updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Toggle individual serializer lane polarity switches (SERIALIZER INVERT POLARITY)
      - **Expected result:** Serializer lane polarity bitmask updates correctly
      - **Actual result:**

..
  Actual test result goes here.
..

   5. Toggle individual deserializer lane polarity switches (DESERIALIZER INVERT POLARITY)
      - **Expected result:** Deserializer lane polarity bitmask updates correctly
      - **Actual result:**

..
  Actual test result goes here.
..

   6. Change the deserializer EQ setting (range 0–4)
      - **Expected result:** DES EQ value updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   7. Toggle the SYSREF LVDS mode switch
      - **Expected result:** SYSREF LVDS mode updates and readback matches
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


Test 7: JESD204 Framer A/B Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.ADVANCED.JESD_FRAMER

**RBP:** P3

**Description:** Verify that the JESD Framer tab displays Framer A and Framer B columns and that all framer parameters can be configured.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Navigate to the JESD Framer tab
   2. Verify Framer A and Framer B columns are displayed
      - **Expected result:** Both framer columns are visible with their respective controls
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Change bank-id and device-id values for Framer A
      - **Expected result:** Values update and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Change M, K, F and NP (converter/octets per frame) values
      - **Expected result:** JESD204 link parameters update and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   5. Toggle individual lane enable switches
      - **Expected result:** Lane enable bitmask updates correctly
      - **Actual result:**

..
  Actual test result goes here.
..

   6. Enable/disable scramble
      - **Expected result:** Scramble setting updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   7. Repeat steps 3–6 for Framer B
      - **Expected result:** Framer B controls behave identically to Framer A
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


Test 8: JESD204 Deframer A/B Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.ADVANCED.JESD_DEFRAMER

**RBP:** P3

**Description:** Verify that the JESD Deframer tab displays Deframer A and Deframer B columns and that all deframer parameters can be configured.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Navigate to the JESD Deframer tab
   2. Verify Deframer A and Deframer B columns are displayed
      - **Expected result:** Both deframer columns are visible with their respective controls
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Change bank-id and device-id values for Deframer A
      - **Expected result:** Values update and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Change M and K (converters/frames per multiframe) values
      - **Expected result:** JESD204 link parameters update and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   5. Toggle individual deserializer lane enable switches
      - **Expected result:** Lane enable bitmask updates correctly
      - **Actual result:**

..
  Actual test result goes here.
..

   6. Change SYNCB out select setting
      - **Expected result:** SYNCB out select updates and readback matches
      - **Actual result:**

..
  Actual test result goes here.
..

   7. Repeat steps 3–6 for Deframer B
      - **Expected result:** Deframer B controls behave identically to Deframer A
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


Test 9: Advanced Tool Refresh Functionality
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9009.ADVANCED.REFRESH_FUNCTION

**RBP:** P3

**Description:** Verify that the refresh button in the Advanced tool updates all controls in all tabs with current values from the device.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9009.device <adrv9009_device_setup>` setup

**Steps:**
   1. Open the ADRV9009 Advanced tool and navigate to any tab
   2. Note current control values displayed in the tab
   3. Change device settings externally (using another tool or command line)
   4. Click the refresh button in the ADRV9009 Advanced tool
      - **Expected result:** All controls in all tabs update to reflect the current device state
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
