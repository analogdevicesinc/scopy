.. _adrv9002_tests:

ADRV9002 (Jupiter) Plugin - Test Suite
======================================

.. note::

   User guide: :ref:`ADRV9002 user guide<adrv9002>`

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
    - Scopy v2.1.0 or later with ADRV9002 plugin installed
    - ADRV9002 device connected
    - Device drivers and firmware are up to date
    - Reset .ini files to default by pressing **Reset** button in Preferences


Setup Environment
-----------------

.. _adrv9002_device_setup:

**ADRV9002.device:**
   - Connect an ADRV9002 evaluation board via Ethernet/USB to your PC
   - Ensure ADRV9002 IIO drivers are properly installed
   - Verify device firmware and driver compatibility
   - Connect to the device in Scopy with ADRV9002 plugin enabled

.. _adrv9002_device_emu:

**ADRV9002.emulator:**
   - Press the add device button and go to emulator
   - Select ADRV9002 device if available
   - Enable, verify and select all plugins available
   - Connect to the device

.. _adrv9002_device_default:

**ADRV9002.default:**
   - Use default device settings after connection
   - Load a known-good profile configuration
   - Ensure all channels are in a stable state


ADRV9002 Controls Tests
-----------------------

Test 1: Plugin Loading and Device Detection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.CONTROLS.PLUGIN_LOADS

**RBP:** P0

**Description:** Verify that the ADRV9002 plugin loads and detects device with Controls tab accessible.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9002.device <adrv9002_device_setup>` setup

**Steps:**
   1. Open Scopy application
   2. Open ADRV9002 plugin and navigate to Controls tab
      - **Expected result:** Plugin loads and Controls tab shows device controls
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


Test 2: Device Driver API Display
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.CONTROLS.DEVICE_DRIVER_API

**RBP:** P3

**Description:** Verify Device Driver API section displays correct version information.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9002.device <adrv9002_device_setup>` setup

**Steps:**
   1. Navigate to Controls tab and observe Device Driver API section
   2. Verify version string format and validity
      - **Expected result:** Version string is displayed in correct format
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


Test 3: Global Settings Section
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.CONTROLS.GLOBAL_SETTINGS

**RBP:** P3

**Description:** Verify global settings controls: profile management, temperature monitoring, initial calibrations.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9002.device <adrv9002_device_setup>` setup

**Steps:**
   1. Expand ADRV9002 Global Settings section
   2. Check Profile Manager displays current configurations
      - **Expected result:** Profile Manager shows profile_config and stream_config status
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Verify Temperature widget shows current reading
      - **Expected result:** Temperature displays in Celsius with warning thresholds
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Check Initial Calibrations widget availability
      - **Expected result:** Widget enables/disables based on device support
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


Test 4: RX Channel Controls
~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.CONTROLS.RX_CHANNEL_CONFIG

**RBP:** P3

**Description:** Test RX1 and RX2 channel control functionality.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9002.device <adrv9002_device_setup>` setup

**Steps:**
   1. Open ADRV9002 tool
   2. Change RX1 Hardware Gain (0-36 dB)
      - **Expected result:** Gain control responds and updates device
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Change Gain Control Mode (manual/automatic)
      - **Expected result:** Mode changes and affects gain behavior
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Change ENSM Mode (radio enable state machine)
      - **Expected result:** ENSM mode updates correctly
      - **Actual result:**

..
  Actual test result goes here.
..

   5. Toggle Powerdown enable/disable
      - **Expected result:** Powerdown control functions (inverted logic)
      - **Actual result:**

..
  Actual test result goes here.
..

   6. Verify read-only values update: Decimated Power, Bandwidth, Sampling Rate
      - **Expected result:** Read-only widgets display current device values
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


Test 5: TX Channel Controls
~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.CONTROLS.TX_CHANNEL_CONFIG

**RBP:** P3

**Description:** Test TX1 and TX2 channel control functionality.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9002.device <adrv9002_device_setup>` setup

**Steps:**
   1. Open ADRV9002 tool
   2. Change TX1 Attenuation (-41.95 to 0 dB)
      - **Expected result:** Attenuation control responds and updates device
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Change Attenuation Control Mode
      - **Expected result:** Control mode changes correctly
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Toggle TX Powerdown enable/disable
      - **Expected result:** Powerdown control functions
      - **Actual result:**

..
  Actual test result goes here.
..

   5. Verify read-only values: Bandwidth, Sampling Rate
      - **Expected result:** Read-only widgets display current device values
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


Test 6: ORX Controls
~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.CONTROLS.ORX_CONFIG

**RBP:** P3

**Description:** Test ORX observation path controls (if available on device).

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9002.device <adrv9002_device_setup>` setup
   - Device supports ORX functionality

**Steps:**
   1. Check if ORX controls are visible
      - **Expected result:** ORX1 and/or ORX2 controls appear if supported by device
      - **Actual result:**

..
  Actual test result goes here.
..

   2. Change ORX Hardware Gain (4-36 dB)
      - **Expected result:** ORX gain control responds and updates device
      - **Actual result:**

..
  Actual test result goes here.
..

   3. Enable/disable BBDC Rejection
      - **Expected result:** BBDC Rejection toggle functions correctly
      - **Actual result:**

..
  Actual test result goes here.
..

   4. Toggle ORX Powerdown
      - **Expected result:** ORX enable/disable control functions
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


Test 7: Refresh Functionality
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.CONTROLS.REFRESH_FUNCTION

**RBP:** P3

**Description:** Test refresh button updates all control values from device.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9002.device <adrv9002_device_setup>` setup

**Steps:**
   1. Note current control values in UI
   2. Change device settings externally (using another tool or command line)
   3. Click refresh button in ADRV9002 plugin
      - **Expected result:** Refresh button triggers update of all controls
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
