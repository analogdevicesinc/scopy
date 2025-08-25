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


Test 3 - SWIOT config mode
--------------------------

.. _TST.SWIOT.CONFIG:

**UID**: TST.SWIOT.CONFIG

**Description:** This test verifies that the SWIOT plugin 
is able to configure the device correctly.

**Preconditions:**
    - :ref:`SWIOT.Device<swiot1l_device>`
    - OS: ANY

**Steps:**
    1. Open the Config instrument.
    2. Enable all 4 channels:
        - **Expected Result**:
            - Their respective dropdowns for device and function are 
              accessible and not greyed out anymore.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Configure SWIOT in the following way:
        - Channel 1: device **ad74413r** and function **voltage_out**
        - Channel 2: device **ad74413r** and function **voltage_in**
        - Channel 3: device **max14906** and function **input**
        - Channel 4: device **max14906** and function **output**
    4. Click the **Apply** button.
        - **Expected Result**:
            - The device disconnects for a moment and then reconnects
              automatically.
            - Upon reconnection, the Config instrument is replaced 
              by 3 instruments: AD74413R, MAX14906, and Faults.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Open the AD74413R instrument and check the channels configured 
       as AD74413R channels:
        
        - **Expected Result**:
            - There are 6 channels available in the channel manager:
              4 diagnostic channels, **voltage_out 1** and **voltage_in 2**.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Open the MAX14906 instrument and check the channels configured:
        - **Expected result:** There are 2 channels available in the 
          channel manager **voltage 2** as INPUT and **voltage 3** as 
          OUTPUT.
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


Test 4 - AD74413R plot operations
----------------------------------

.. _TST.AD74413R.PLOT:

**UID**: TST.AD74413R.PLOT

**Description:** This test verifies that the AD74413R instrument
plot changes such as labels and timestamp are correctly modified.

**Preconditions:**
    - :ref:`SWIOT.Device.Runtime<swiot1l_device_runtime>`
    - OS: ANY

**Steps:**
    1. Open the AD74413R instrument.
    2. In the General settings menu, enable Plot Labels and 
       set the Timespan to 4s.
        
        - **Expected Result**:
            - The labels are displayed on the right side of the plot.
            - The X axis of the plot shows a 4s timespan, 
              using the -4 to 0 range.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Enable channel **voltage_out 1** and run a **Single** capture:
        - **Expected Result**:
            - The voltage_out 1 data is displayed on the plot 
              from the right to the left side.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Click the **Measure** button to enable measurements:
        - **Expected Result**:
            - The measurements are displayed above the plot
              and the instant value for the first channel is
              around 0A.
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


Test 5 - AD74413R channel operations
-------------------------------------

.. _TST.AD74413R.CHANNEL:

**UID**: TST.AD74413R.CHANNEL

**Description:** This test verifies that AD74413R 
channels can correctly output and acquire accurate data as 
displayed on the plot and measured by the instrument.

**Preconditions:**
    - :ref:`SWIOT.Device.Runtime<swiot1l_device_runtime>`
    - OS: ANY    

**Steps:**
    1. Open the AD74413R instrument.
    2. In the General settings menu, enable Plot Labels and 
       set the Timespan to 2s.
    3. Enable **voltage_out 1 ** and **voltage_in 2** channels.
    4. Open the channel settings menu for the **voltage_out 1** channel
       and set the **sampling_frequency** to 1200.
        
        - **Expected result:** 
            - A green animation appears while underlining the field value.
            - On the top right on the plot the status message 
              displays: a number of samples at **1.2ksps**.
        
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Set YMin to -1A and YMax to 10A.
        - **Expected result:** 
            - The plot Y axis is scaled between -1A and 10A.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Open the channel settings menu for the **voltage_in 2** channel 
       and set YMin to 0V and YMax to 20V.
    7. Go back to the first channel menu and set the **RAW** output 
       value to **8192**:
        
        - **Expected result:** 
            - The value is automatically changed to 8191.
            - Below the field the value 10.9V is displayed.
        
        - **Actual result:**

..
  Actual test result goes here.
..

    8. Run a Single capture and check the measurements:
        - **Expected result:** 
            - The instant value of **voltage_in 2** shows 10V.
        - **Actual result:**

..
  Actual test result goes here.
..

    9. Run a Continuous capture and check the measurements:
        - **Expected result:** 
            - The instant value of **voltage_in 2** shows 10V.
        - **Actual result:**

..
  Actual test result goes here.
..

    10. While running change the RAW value to **4096**:
         - **Expected result:** 
            - Below the field the value 5.5V is displayed.
            - The signal on the plot for **voltage_in 2** is 
              dropping from the previous value to 5.5V.
            - The same value is displayed in the instant value 
              measurement.
         - **Actual result:**

..
  Actual test result goes here.
..

    11. While running change the RAW value to **-1**:
         - **Expected result:**
            - The value is automatically set to 0V.
            - Below the field the value 0V is displayed.
            - The signal on the plot for **voltage_in 2** is 
              dropping to 0V.
            - The same value is displayed in the instant value 
              measurement.
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


Test 6 - AD74413R diagnostic channels
--------------------------------------

.. _TST.AD74413R.DIAG:

**UID**: TST.AD74413R.DIAG

**Description:** This test verifies that the AD74413R
diagnostic channels can be correctly configured and 
displayed on the instrument.

**Preconditions:**
    - :ref:`SWIOT.Device.Runtime<swiot1l_device_runtime>`
    - OS: ANY

**Steps:**
    1. Open the AD74413R instrument.
    2. In the General settings menu, enable Plot Labels and 
       set the Timespan to 2s.
    3. Enable **voltage_out 1 **, **voltage_in 2**  and
       **diagnostic 5** channels.
    4. Open the channel settings for **diagnostic 5** and set 
       the **diag_function** to **sensel_b**.
    5. Open the channel settings for **voltage_out 1** 
       and set the **RAW** output value to **8192**.
    6. Run a Continuous capture and check the measurements.
        - **Expected result:** 
            - The instant value of **diagnostic 5** shows around 10V,
              the same as **voltage_in 2**.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. While running change the RAW value to **4096**:
        - **Expected result:** 
            - Both the **diagnostic 5** and **voltage_in 2** signals 
              are dropping from the previous value to 5.5V.
        - **Actual result:**

..
  Actual test result goes here.
..

    8. While running change the RAW value to **2000**:
        - **Expected result:** 
            - Both the **diagnostic 5** and **voltage_in 2** signals 
              are dropping from the previous value to around 2.7V.
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


Test 7 - AD74413R sampling frequency
-------------------------------------

.. _TST.AD74413R.SAMPLING:

**UID**: TST.AD74413R.SAMPLING

**Description:** This test verifies that the AD74413R
instrument can correctly compute the acquisition rate based on 
the number of enabled channels.

**Preconditions:**
    - :ref:`SWIOT.Device.Runtime<swiot1l_device_runtime>`
    - OS: ANY

**Steps:**
    1. Open the AD74413R instrument.
    2. Enable all the channels. Set the sampling frequency 
       for each channel to 4800.
        
        - **Expected result:** 
            - The status message on the top right of the plot 
              displays a number of samples at **800 sps**.
        
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Disable the last two diagnostic channels:
        - **Expected result:** 
            - The status message on the top right of the plot 
              displays a number of samples at **1.2 ksps**. 
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


Test 8 - AD74413R tutorial & docs
----------------------------------

.. _TST.AD74413R.TUTORIAL:

**UID**: TST.AD74413R.TUTORIAL

**Description:** This test verifies that the AD74413R
instrument tutorial can be correctly started, followed 
and the documentation is accessible.

**Preconditions:**
    - :ref:`SWIOT.Device.Runtime<swiot1l_device_runtime>`
    - OS: ANY

**Steps:**
    1. Open the AD74413R instrument.
    2. Click the top left info button.
        - **Expected result:** 
            - A pop up with 2 options (Tutorial and Documentation) 
              is displayed.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Click the **Documentation** button.
        - **Expected result:** 
            - The AD74413R documentation is opened in a browser.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Click the info button.
    5. Click the **Tutorial** button.
        - **Expected result:** 
            - The AD74413R tutorial starts, guiding the user 
              through the instrument's features and providing 
              a button to Exit the tutorial.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Click the **Continue** button.
        - **Expected result:** 
            - The tutorial continues with the next step, always
              greying out the background and highlighting only 
              the explained item.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Click the **Exit** button.
        - **Expected result:** 
            - The tutorial is closed and the user is returned 
              to the instrument.
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


Test 9 - MAX14906 plot operations
----------------------------------

.. _TST.MAX14906.PLOT:

**UID**: TST.MAX14906.PLOT

**Description:** This test verifies that the MAX14906 instrument
time span can be correctly modified.

**Preconditions:**
    - :ref:`SWIOT.Device.Runtime<swiot1l_device_runtime>`
    - OS: ANY

**Steps:**
    1. Open the MAX14906 instrument.
    2. In the General settings menu, set the Timespan to 10s.
        - **Expected result:** 
            - The X axis of the plots shows a 10s timespan, 
              using the 0 to 10 range.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Set the Timespan to 1s.
        - **Expected result:** 
            - The X axis of the plots shows a 1s timespan, 
              using the 0 to 1 range.
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


Test 10 - MAX14906 channel operations
-------------------------------------

.. _TST.MAX14906.CHANNEL:

**UID**: TST.MAX14906.CHANNEL

**Description:** This test verifies that MAX14906
channels can correctly output and acquire accurate data as
displayed on the plot and measured by the instrument.

**Preconditions:**
    - :ref:`SWIOT.Device.Runtime<swiot1l_device_runtime>`
    - OS: ANY

**Steps:**
    1. Open the MAX14906 instrument.
    2. In the General settings menu, set the Timespan to 1s.
    3. Run a continuous capture.
    4. Set the **Output** for **voltage3** to ON.
        - **Expected result:** 
            - The plot trace rises from 0 to 1 for 
              both channels.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Set the **Output** for **voltage3** to OFF.
        - **Expected result:** 
            - The plot trace drops from 1 to 0 for 
              both channels.
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


Test 11 - MAX14906 tutorial & docs
----------------------------------

.. _TST.MAX14906.TUTORIAL:

**UID**: TST.MAX14906.TUTORIAL

**Description:** This test verifies that the MAX14906
instrument tutorial can be correctly started, followed
and the documentation is accessible.

**Preconditions:**
    - :ref:`SWIOT.Device.Runtime<swiot1l_device_runtime>`
    - OS: ANY

**Steps:**
    1. Open the MAX14906 instrument.
    2. Click the top left info button.
        - **Expected result:** 
            - A pop up with 2 options (Tutorial and Documentation) 
              is displayed.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Click the **Documentation** button.
        - **Expected result:** 
            - The MAX14906 documentation is opened in a browser.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Click the info button.
    5. Click the **Tutorial** button.
        - **Expected result:** 
            - The MAX14906 tutorial starts, guiding the user 
              through the instrument's features and providing 
              a button to Exit the tutorial.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Click the **Continue** button.
        - **Expected result:** 
            - The tutorial continues with the next step, always
              greying out the background and highlighting only 
              the explained item.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Click the **Exit** button.
        - **Expected result:** 
            - The tutorial is closed and the user is returned 
              to the instrument.
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



Test 12 - Faults Run Single
---------------------------

.. _TST.FAULTS.RUN_SINGLE:

**UID**: TST.FAULTS.RUN_SINGLE

**Description:** This test verifies that the Faults instrument
can poll faults values for both devices.

**Preconditions:**
    - :ref:`SWIOT.Device.Runtime<swiot1l_device_runtime>`
    - OS: ANY

**Steps:**
    1. Run a continuous capture with all channels enabled 
       on the AD74413R instrument.
    2. Open the Faults instrument and run a **Single** capture.
        - **Expected result:** 
            - Bit 10 is enabled on the AD74413R device.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Click on Bit 10:
        - **Expected result:** 
            - The section below the leds only displays the 
              fault explanation for the selected bit.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Turn **Faults explanation** off:
        - **Expected result:** 
            - The section below the leds is hidden.
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


Test 13 - Faults clear and reset
--------------------------------

.. _TST.FAULTS.CLEAR_RESET:

**UID**: TST.FAULTS.CLEAR_RESET

**Description:** This test verifies that the Faults instrument
can reset the polled values after a capture.

**Preconditions:**
    - :ref:`SWIOT.Device.Runtime<swiot1l_device_runtime>`
    - OS: ANY

**Steps:**
    1. Run a continuous capture with all channels enabled 
       on the AD74413R instrument.
    2. Open the Faults instrument and run a **Single** 
       capture.
    3. Select Bit 10.   
    4. Click the **Clear selection** button.
        - **Expected result:** 
            - Bit 10 is not selected and the Faults explanation
              display all the text greyed out.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Run a **Reset stored** capture.
        - **Expected result:** 
            - Both the leds for Bit 10 are turned off.
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


Test 14 - Faults Run continuous
--------------------------------

.. _TST.FAULTS.RUN_CONTINUOUS:

**UID**: TST.FAULTS.RUN_CONTINUOUS

**Description:** This test verifies that the Faults instrument
can poll faults values for both devices in continuous mode.

**Preconditions:**
    - :ref:`SWIOT.Device.Runtime<swiot1l_device_runtime>`
    - OS: ANY

**Steps:**
    1. Run a continuous capture with all channels enabled 
       on the AD74413R instrument.
    2. Open the Faults instrument and run a **Continuous**  capture.
        - **Expected result:** 
            - Bit 10 is enabled on the AD74413R device.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. While running click the **Reset stored** button:
        - **Expected result:** 
            - The **STORED** LED is turned off until the next
              polled value turns it on again.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Stop the Faults instrument.

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 15 - Faults tutorial & docs
--------------------------------

.. _TST.FAULTS.TUTORIAL:

**UID**: TST.FAULTS.TUTORIAL

**Description:** This test verifies that the Faults
instrument tutorial can be correctly started, followed
and the documentation is accessible.

**Preconditions:**
    - :ref:`SWIOT.Device.Runtime<swiot1l_device_runtime>`
    - OS: ANY

**Steps:**
    1. Open the Faults instrument.
    2. Click the top left info button.
        - **Expected result:** 
            - A pop up with 2 options (Tutorial and Documentation) 
              is displayed.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Click the **Documentation** button.
        - **Expected result:** 
            - The Faults documentation is opened in a browser.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Click the info button.
    5. Click the **Tutorial** button.
        - **Expected result:** 
            - The Faults tutorial starts, guiding the user 
              through the instrument's features and providing 
              a button to Exit the tutorial.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Click the **Continue** button.
        - **Expected result:** 
            - The tutorial continues with the next step, always
              greying out the background and highlighting only 
              the explained item.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Click the **Exit** button.
        - **Expected result:** 
            - The tutorial is closed and the user is returned 
              to the instrument.
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



Test 16 - SWIOT external supply
-------------------------------

.. _TST.SWIOT.EXTERNAL_SUPPLY:

**UID**: TST.SWIOT.EXTERNAL_SUPPLY

**Description:** This test verifies that the SWIOT plugin
can correctly detect and notify the user that the external 
power supply switch is turned off.

**Preconditions:**
    - :ref:`SWIOT.Device.Runtime<swiot1l_device_runtime>`
    - OS: ANY

**Steps:**
    1. Open any instrument from the SWIOT plugin.
    2. Turn off the external power supply switch on the SWIOT device.
        - **Expected result:** 
            - A status bar is displayed at the bottom of the instrument
              with a warning message that the system is not running at 
              full capacity due to not being powered by the external supply.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Turn on the external power supply switch on the SWIOT device.
        - **Expected result:** 
            - The status bar is not displayed.
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

