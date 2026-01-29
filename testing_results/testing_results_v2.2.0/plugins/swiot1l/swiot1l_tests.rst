.. _swiot1l_tests:

SWIOT Plugin - Test Suite
=========================

.. note::

    User guide :ref:`AD-SWIOT1L-SL user guide<swiot1l_index>`

The SWIOT plugin tests are a set of tests that are used to verify the functionality of the SWIOT plugin.
The tests are designed to be run in a specific order to ensure that the plugin is functioning correctly. 
The tests are divided into two main categories: **CONFIG** AND **RUNTIME**. 

The following apply for all the test cases in this suite.
If the test case has special requirements, they will be listed in the test case section.

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

.. _swiot1l_emu_config:

**SWIOT.Emu.Config:**
    - Open Scopy.
    - Start the IIO-EMU process.
    - Connect to the **swiot_config** device.

Depends on:
    - Test TST.EMU.CONNECT
    - Test TST.PREFS.RESET

.. _swiot1l_emu_runtime:

**SWIOT.Emu.Runtime:**
    - Open Scopy.
    - Start the IIO-EMU process.
    - Connect to the **swiot_runtime** device.

Depends on:
    - Test TST.EMU.CONNECT
    - Test TST.PREFS.RESET

.. _swiot1l_device:

**SWIOT.Device:**
    - Open Scopy.
    - Connect to AD-SWIOT1L-SL using the static ip:169.254.97.40
    - Connect **CH_1 GND** to **CH_2 GND** using loopback cables.
    - Connect **CH_1** to **CH_2** using loopback cables.
    - Connect **CH_3 GND** to **CH_4 GND** using loopback cables.
    - Connect **CH_3** to **CH_4** using loopback cables.

Depends on:
    - Test TST.PREFS.RESET

.. _swiot1l_device_runtime:

**SWIOT.Device.Runtime:**
    - Open Scopy.
    - Connect to AD-SWIOT1L-SL using the static ip:169.254.97.40
    - Connect **CH_1 GND** to **CH_2 GND** using loopback cables.
    - Connect **CH_1** to **CH_2** using loopback cables.
    - Connect **CH_3 GND** to **CH_4 GND** using loopback cables.
    - Connect **CH_3** to **CH_4** using loopback cables.
    - Configure SWIOT in the following way:
        - Channel 1: device **ad74413r** and function **voltage_out**
        - Channel 2: device **ad74413r** and function **voltage_in**
        - Channel 3: device **max14906** and function **input**
        - Channel 4: device **max14906** and function **output**
    - Click **Apply** to enter Runtime mode.

Depends on:
    - Test TST.PREFS.RESET

Prerequisites:
    - Scopy v2.0.0 or later with SWIOT plugin installed on the system.
    - Tests listed as dependencies are successfully completed.
    - Reset .ini files to default using the Preferences "Reset" button.

Test 1 - SWIOT compatibility
-----------------------------

.. _TST.SWIOT.COMPAT:

**UID**: TST.SWIOT.COMPAT

**RBP:** P0

**Description**: This test verifies that the SWIOT plugin is compatible 
with the selected device and that the plugin is able to correctly parse it. 

**Preconditions:**
    - :ref:`SWIOT.Emu.Config<swiot1l_emu_config>`
    - OS: ANY

**Steps:**
    1. Open the SWIOT plugin - Config Instrument.
        - **Expected Result**:
            - The plugin is able to connect to the device.
            - The Config instrument shows 4 disabled channels.
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



Test 2 - Mode switching
-----------------------------

.. _TST.SWIOT.MODES:

**UID**: TST.SWIOT.MODES

**RBP:** P0

**Description**: This test verifies that the SWIOT plugin is able 
to switch between Config and Runtime modes.

**Preconditions:**
    - :ref:`SWIOT.Device<swiot1l_device>`
    - OS: ALL

**Steps:**
    1. Open the Config instrument.
    2. Click the **Apply** button.
        - **Expected Result**:
            - The device disconnects for a moment and then reconnects
              automatically.
            - Upon reconnection, the Config instrument is replaced 
              by 3 instruments: AD74413R, MAX14906, and Faults.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Open the AD74413R instrument.
    4. Click the **Config** button.
        - **Expected Result**:
            - The device disconnects for a moment and then reconnects
              automatically.
            - The AD74413R, MAX14906 and Faults instruments are replaced 
              by the Config instrument.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Open the Config instrument.
    6. Click the **Apply** button.
        - **Expected Result**:
            - The device disconnects for a moment and then reconnects
              automatically.
            - Upon reconnection, the Config instrument is replaced 
              by 3 instruments: AD74413R, MAX14906, and Faults.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Open the MAX14906 instrument.    
    8. Click the **Config** button.
        - **Expected Result**:
            - The device disconnects for a moment and then reconnects
              automatically.
            - The AD74413R, MAX14906 and Faults instruments are replaced 
              by the Config instrument.
        - **Actual result:**

..
  Actual test result goes here.
..

    9. Open the Config instrument.
    10. Click the **Apply** button.
         - **Expected Result**:
            - The device disconnects for a moment and then reconnects
              automatically.
            - Upon reconnection, the Config instrument is replaced 
              by 3 instruments: AD74413R, MAX14906, and Faults.

         - **Actual result:**

..
  Actual test result goes here.
..

    11. Open the Faults instrument.    
    12. Click the **Config** button.
         - **Expected Result**:
            - The device disconnects for a moment and then reconnects automatically.
            - The AD74413R, MAX14906 and Faults instruments are replaced 
              by the Config instrument.

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


