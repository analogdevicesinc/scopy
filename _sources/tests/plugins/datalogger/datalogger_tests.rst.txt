.. _datalogger_tests:

Datalogger Test Suite
================================================================================

.. note::

  User guide :ref:`Data Logger user guide <datalogger>`

The following apply to all tests below, any special requirements will be mentioned.

**Prerequisites:**
    - Scopy v2.0.0 or later with Data Logger installed on the system
    - Tests listed as dependencies are successfully completed
    - Reset .ini files to default by pressing **Reset** button in Preferences

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

Setup Environment
------------------

.. _datalogger_adalm_pluto_emu_setup:

**AdalmPluto.Emu:**
    - Open Scopy
    - Start the iio-emu process using Pluto option from the dropdown
    - Connect to the Pluto Device

.. _datalogger_adalm_pluto_device_setup:

**AdalmPluto.Device:**
    - Open Scopy
    - Connect to the Pluto Device using IP or USB

.. _datalogger_m2k_emu_setup:

**M2k.Emu:**
    - Open Scopy
    - Start the iio-emu process using ADALM2000 option from the dropdown
    - Connect to the M2k Device

.. _TST_DATALOGGER_ENABLE_DISABLE_DATA_LOGGER_CHANNEL:

Test 1: Enable and Disable Data Logger Channel
------------------------------------------------

**UID:** TST.DATALOGGER.ENABLE_DISABLE_DATA_LOGGER_CHANNEL

**Description:** Enable the channel and check if the channel is enabled

**OS:** ANY

**Preconditions:**
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Steps:**
    1. Open Data Logger plugin
    2. Using channel menu enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Wait for some data to be collected
        - **Expected result:** Curve will be displayed on the plot
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Using channel menu disable channel **voltage0** of **xadc** device
        - **Expected result:** Curve will be removed from the plot
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


.. _TST_DATALOGGER_RUN_STOP_DATA_LOGGING:

Test 2: Run and Stop Data Logging
------------------------------------

**UID:** TST.DATALOGGER.RUN_STOP_DATA_LOGGING

**Description:** Start the Data Logger and check if the data is being collected

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Enable and Disable Data Logger Channel <TST_DATALOGGER_ENABLE_DISABLE_DATA_LOGGER_CHANNEL>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Using channel menu enable channel **voltage0** of **xadc** device
    3. Press **Run** button
        - **Expected result:** 
            - Data is being collected and displayed on the plot
            - **Run** button is changed to **Stop**
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Press **Stop** button
        - **Expected result:** 
            - Data is no longer being collected and displayed on the plot
            - **Stop** button is changed to **Run**
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


.. _TST_DATALOGGER_CLEAR_COLLECTED_DATA:

Test 3: Clear Collected Data
------------------------------

**UID:** TST.DATALOGGER.CLEAR_COLLECTED_DATA

**Description:** Clear the data and check if the data is removed from the plot

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Using channel menu enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Wait for some data to be collected
    5. Press **Stop** button
    6. Press **Clear** button
        - **Expected result:** Data is removed from the plot
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


.. _TST_DATALOGGER_CLEAR_DATA_WHILE_RUNNING:

Test 4: Clear data while running 
----------------------------------

**UID:** TST.DATALOGGER.CLEAR_DATA_WHILE_RUNNING

**Description:** Clear the data while running and check if the data is removed from the plot

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Using channel menu enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Wait for some data to be collected
    5. Press **Clear** button
        - **Expected result:** 
            - Data is removed from the plot
            - Buffer is cleared and the starting point is reset to "0"
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


.. _TST_DATALOGGER_CREATE_NEW_DATA_LOGGER_TOOL:

Test 5: Create a New Data Logger Tool
---------------------------------------

**UID:** TST.DATALOGGER.CREATE_NEW_DATA_LOGGER_TOOL

**Description:** Create a new tool and check if the new tool is created

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Steps:**
    1. Open Data Logger plugin
    2. Press **+** button
        - **Expected result:** 
            - New tool is created
            - We are moved to the new tool
            - The new tool has **x** button next to **+**

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


.. _TST_DATALOGGER_REMOVE_A_CREATED_TOOL:

Test 6: Remove a Created Tool
---------------------------------

**UID:** TST.DATALOGGER.REMOVE_A_CREATED_TOOL

**Description:** Remove the tool and check if the tool is removed

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Create a New Data Logger Tool <TST_DATALOGGER_CREATE_NEW_DATA_LOGGER_TOOL>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Press **+** button
    3. Press **x** button next to **+**
        - **Expected result:** Tool is removed
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


.. _TST_DATALOGGER_VERIFY_REMOVAL_DISABLED_FOR_DEFAULT_TOOL:

Test 7: Verify Removal Disabled for Default Tool
--------------------------------------------------

**UID:** TST.DATALOGGER.VERIFY_REMOVAL_DISABLED_FOR_DEFAULT_TOOL

**Description:** Check if the remove tool is disabled for the first tool

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Steps:**
    1. Open Data Logger plugin
    2. Press **+** button
        - **Expected result:** A new tool where **x** button is available is created 
        - **Actual result:**

..
  Actual test result goes here.
..


    3. Open original Data Logger tool
        - **Expected result:** No **x** button is available
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


.. _TST_DATALOGGER_PRINT_COLLECTED_DATA:

Test 8: Print Collected Data
------------------------------

**UID:** TST.DATALOGGER.PRINT_COLLECTED_DATA

**Description:** Print the data and check if the data is printed

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Using channel menu enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Wait for some data to be collected
    5. Press **Print** button
    6. Choose directory where to save the file
        - **Expected result:** 
            - Data is saved to a pdf file in the chosen directory
            - The file name structure is "Scopy-Data-Logger-<date>-<time>.pdf"
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


.. _TST_DATALOGGER_INFO_BUTTON_DOCUMENTATION:

Test 9: Info button documentation
------------------------------------

**UID:** TST.DATALOGGER.INFO_BUTTON_DOCUMENTATION

**Description:** Check if the info documentation option works

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`
    - An internet connection is available

**Steps:**
    1. Open Data Logger plugin
    2. Press **Info** button
    3. Press **Documentation** button
        - **Expected result:** A browser window is opened with the documentation page
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


.. _TST_DATALOGGER_INFO_BUTTON_TUTORIAL:

Test 10: Info button tutorial   
-----------------------------------

**UID:** TST.DATALOGGER.INFO_BUTTON_TUTORIAL

**Description:** Check if the info tutorial option works

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Steps:**
    1. Open Data Logger plugin
    2. Press **Info** button
    3. Press **Tutorial** button
        - **Expected result:** A tutorial explaining how to use the Data Logger is displayed
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


.. _TST_DATALOGGER_SETTINGS_CHANGE_DATA_LOGGER_TOOL_NAME:

Test 11: Settings Change Data Logger Tool Name
------------------------------------------------

**UID:** TST.DATALOGGER.SETTINGS_CHANGE_DATA_LOGGER_TOOL_NAME

**Description:** Change the tool name and check if the tool name is changed

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Steps:**
    1. Open Data Logger plugin
    2. Press **Settings** button
    3. Change the tool name at the top of the Settings menu from "Data Logger" to "Test Tool"
        - **Expected result:** Tool name is changed in the tools menu
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


.. _TST_DATALOGGER_SET_VALID_X_AXIS_DELTA_VALUE:

Test 12: Set Valid X-Axis Delta Value
-----------------------------------------

**UID:** TST.DATALOGGER.SET_VALID_X_AXIS_DELTA_VALUE

**Description:** Change the X-Axis Delta Value to a valid input and check if the X-Axis Delta Value is changed

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Wait for at least "20" seconds
    5. Press **Settings** button
    6. Change the X-Axis Delta Value from "10" to "20" then press enter
        - **Expected result:** X-Axis displays "20" seconds of data instead of "10" seconds
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


.. _TST_DATALOGGER_HANDLE_INVALID_X_AXIS_DELTA_VALUE_INPUT:

Test 13: Handle Invalid X-Axis Delta Value Input
----------------------------------------------------

**UID:** TST.DATALOGGER.HANDLE_INVALID_X_AXIS_DELTA_VALUE_INPUT

**Description:** Change the X-Axis Delta Value to an invalid input and check if the X-Axis Delta Value is not changed

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Wait for at least "20" seconds
    5. Press **Settings** button
    6. Change the X-Axis Delta Value from "10" to "test" then press enter
        - **Expected result:** X-Axis displays "10" seconds 
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


.. _TST_DATALOGGER_ADJUST_X_AXIS_DELTA_VALUE_USING_BUTTONS:

Test 14: Adjust X-Axis Delta Value Using Buttons
----------------------------------------------------

**UID:** TST.DATALOGGER.ADJUST_X_AXIS_DELTA_VALUE_USING_BUTTONS

**Description:** Change the X-Axis Delta Value using the + / - buttons and check if the X-Axis Delta Value is changed

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Press **Settings** button
    5. Press **+** button next to X-Axis Delta Value
        - **Expected result:** X-Axis displays "11" seconds of data instead of "10" seconds
        - **Actual result:**

..
  Actual test result goes here.
..


    6. Press **-** button next to X-Axis Delta Value
        - **Expected result:** X-Axis displays "10" seconds of data instead of "11" seconds
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


.. _TST_DATALOGGER_TOGGLE_X_AXIS_UTC_TIME_DISPLAY:

Test 15: Toggle X-Axis UTC Time Display
------------------------------------------

**UID:** TST.DATALOGGER.TOGGLE_X_AXIS_UTC_TIME_DISPLAY

**Description:** Enable the X-Axis UTC Time setting and check if the X-Axis displays UTC time instead of delta value.

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Press **Settings** button
    5. Toggle the **X-Axis UTC Time** on
        - **Expected result:** X-Axis displays UTC time instead of delta value
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Toggle the **X-Axis UTC Time** off
        - **Expected result:** X-Axis displays delta value instead of UTC time
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


.. _TST_DATALOGGER_TOGGLE_X_AXIS_LIVE_PLOTTING:

Test 16: Toggle X-Axis Live Plotting
---------------------------------------

**UID:** TST.DATALOGGER.TOGGLE_X_AXIS_LIVE_PLOTTING

**Description:** Toggle the X-Axis Live plotting off/on and check if the X-Axis displays live data

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
        - **Expected result:** X-Axis is updated so the last point added is always visible
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Press **Settings** button
    5. Toggle the **X-Axis Live plotting** off
        - **Expected result:** 
            - X-Axis displays data collected but the last point added is not always visible
            - Settings for picking date time value to show is now available
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


.. _TST_DATALOGGER_TOGGLE_Y_AXIS_AUTOSCALE:

Test 17: Toggle Y-Axis Autoscale
------------------------------------

**UID:** TST.DATALOGGER.TOGGLE_Y_AXIS_AUTOSCALE

**Description:** Toggle the Y-Axis autoscale off/on and check if the Y-Axis displays data with autoscale

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Press **Settings** button
    5. Toggle the **Y-Axis autoscale** off
        - **Expected result:** 
            - Y-Axis displays data without autoscale
            - Settings for picking min and max value are now enabled
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Toggle the **Y-Axis autoscale** on
        - **Expected result:** 
            - Y-Axis displays data with autoscale 
            - Settings for picking min and max value is now disabled
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


.. _TST_DATALOGGER_SET_Y_AXIS_MIN_MAX_VALUES:

Test 18: Set Y-Axis Minimum and Maximum Values
-------------------------------------------------

**UID:** TST.DATALOGGER.SET_Y_AXIS_MIN_MAX_VALUES

**Description:** Change the Y-Axis min and max value and check if the Y-Axis displays data with the new min and max value

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Press **Settings** button
    5. Toggle the **Y-Axis autoscale** off
    6. Change the Y-Axis min value to "0" then press enter
        - **Expected result:** Y-Axis displays data has now "0" as bottom value
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Change the Y-Axis max value to "2" then press enter
        - **Expected result:** Y-Axis displays data has now "2" as top value
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


.. _TST_DATALOGGER_ADJUST_CURVE_THICKNESS:

Test 19: Adjust Curve Thickness
-----------------------------------

**UID:** TST.DATALOGGER.ADJUST_CURVE_THICKNESS

**Description:** Change the curve thickness and check if the curve thickness is changed

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Press **Settings** button
    5. Change the curve thickness to "2" then press enter
        - **Expected result:** Curve thickness is changed to "2"
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


.. _TST_DATALOGGER_CHANGE_CURVE_STYLE:

Test 20: Change Curve Style
-------------------------------

**UID:** TST.DATALOGGER.CHANGE_CURVE_STYLE

**Description:** Change the curve style and check if the curve style is changed

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Press **Settings** button
    5. Change the curve style to **dots** then press enter
        - **Expected result:** Curve data is displayed as dots instead of lines
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


.. _TST_DATALOGGER_ADJUST_PLOT_DISPLAY_SETTINGS:

Test 21: Adjust Plot Display Settings 
------------------------------------------

**UID:** TST.DATALOGGER.ADJUST_PLOT_DISPLAY_SETTINGS

**Description:** Change the plot settings and check if the plot settings are changed

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Press **Settings** button
    3. Toggle **Buffer Preview** off
        - **Expected result:** Buffer preview is not displayed
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Toggle **Buffer Preview** on 
        - **Expected result:** Buffer preview is displayed 
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Toggle **X-AXIS label** off
        - **Expected result:** X-Axis labels are not displayed
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Toggle **X-AXIS label** on
        - **Expected result:** X-Axis labels are displayed
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Toggle **Y-AXIS label** off 
        - **Expected result:** Y-Axis labels are not displayed 
        - **Actual result:**

..
  Actual test result goes here.
..

    8. Toggle **Y-AXIS label** on
        - **Expected result:** Y-Axis labels are displayed
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


.. _TST_DATALOGGER_VERIFY_PLOT_DISPLAY_METHOD:

Test 22: Verify Plot Display Method  
-------------------------------------

**UID:** TST.DATALOGGER.VERIFY_PLOT_DISPLAY_METHOD

**Description:** Verify that the application displays data in the “Plot” mode correctly

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
        - **Expected result:** Data is displayed in the plot mode
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


.. _TST_DATALOGGER_VERIFY_PLAIN_TEXT_DISPLAY_METHOD:

Test 23: Verify Plain Text Display Method
-------------------------------------------

**UID:** TST.DATALOGGER.VERIFY_PLAIN_TEXT_DISPLAY_METHOD

**Description:** Verify that the application displays data in the “Plain Text” mode correctly

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Press **Text** button
        - **Expected result:** Data is displayed in the plain text mode
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


.. _TST_DATALOGGER_VERIFY_7_SEGMENT_DISPLAY_METHOD:

Test 24: Verify 7 Segment Display Method
------------------------------------------

**UID:** TST.DATALOGGER.VERIFY_7_SEGMENT_DISPLAY_METHOD

**Description:** Verify that the application displays data in the “7 Segment” mode correctly

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Press **7 Segment** button
        - **Expected result:** Data is displayed in the 7 segment mode
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


.. _TST_DATALOGGER_TOGGLE_BETWEEN_DISPLAY_METHODS:

Test 25: Toggle Between Display Methods
------------------------------------------

**UID:** TST.DATALOGGER.TOGGLE_BETWEEN_DISPLAY_METHODS

**Description:** Verify that the application toggles between the display methods correctly

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Press **Text** button
        - **Expected result:** Data is displayed in the plain text mode
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Press **7 Segment** button
        - **Expected result:** Data is displayed in the 7 segment mode
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Press **Plot** button
        - **Expected result:** Data is displayed in the plot mode
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


.. _TST_DATALOGGER_SET_7_SEGMENT_DISPLAY_PRECISION:

Test 26: Set 7 Segment Display Precision
------------------------------------------

**UID:** TST.DATALOGGER.SET_7_SEGMENT_DISPLAY_PRECISION

**Description:** Change the 7 Segment precision and check if the 7 Segment precision is changed

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Press **7 Segment** button
    5. Press **Settings** button
    6. Change the 7 Segment precision to "2" then press enter
        - **Expected result:** 7 Segment displays data with "2" decimal points
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


.. _TST_DATALOGGER_TOGGLE_7_SEGMENT_MIN_MAX_DISPLAY:

Test 27: Toggle 7 Segment Min/Max Display 
--------------------------------------------

**UID:** TST.DATALOGGER.TOGGLE_7_SEGMENT_MIN_MAX_DISPLAY

**Description:** Toggle 7 Segment min/max off/on and check if the 7 Segment displays data with min/max values

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Press **7 Segment** button
    5. Press **Settings** button
    6. Toggle the **7 Segment min/max** off
        - **Expected result:** 7 Segment displays data without min/max values
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Toggle the **7 Segment min/max** on
        - **Expected result:** 7 Segment displays data with min/max values
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


.. _TST_DATALOGGER_CHOOSE_FILE_FOR_DATA_LOGGING:

Test 28: Choose File for Data Logging
----------------------------------------

**UID:** TST.DATALOGGER.CHOOSE_FILE_FOR_DATA_LOGGING

**Description:** Verify that the application allows the user to choose a file to save data to

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Steps:**
    1. Open Data Logger plugin
    2. Open the settings menu
    3. Press **Browse** button
        - **Expected result:** A file explorer window is opened
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Choose a ".csv" file to save data to
        - **Expected result:** The file path is displayed in the settings menu
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


.. _TST_DATALOGGER_SAVE_COLLECTED_DATA_TO_FILE:

Test 29: Save Collected Data to File
--------------------------------------

**UID:** TST.DATALOGGER.SAVE_COLLECTED_DATA_TO_FILE

**Description:**  Verify that the application saves all 
collected data to a file when the “Save Data” function is triggered.

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed
    - :ref:`Choose File for Data Logging <TST_DATALOGGER_CHOOSE_FILE_FOR_DATA_LOGGING>` is completed successfully

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Wait for some data to be collected
    5. Open the settings menu
    6. Press **Save Data** button
        - **Expected result:** Data is saved to a file in the chosen file.
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


.. _TST_DATALOGGER_ENABLE_LIVE_DATA_LOGGING:

Test 30: Enable Live Data Logging
--------------------------------------

**UID:** TST.DATALOGGER.ENABLE_LIVE_DATA_LOGGING

**Description:** Verify that the application saves all collected data to a 
file when the “Live Data Logging” function is triggered.

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed
    - :ref:`Choose File for Data Logging <TST_DATALOGGER_CHOOSE_FILE_FOR_DATA_LOGGING>` is completed successfully

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Open the settings menu
    5. Toggle **Live Data Logging** on
        - **Expected result:** 
            - Data is saved to a file in the chosen file continuously
            - **Save Data** and **Import data** buttons are disabled
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


.. _TST_DATALOGGER_IMPORT_DATA_FROM_FILE:

Test 31: Import Data from File
-----------------------------------

**UID:** TST.DATALOGGER.IMPORT_DATA_FROM_FILE

**Description:** Verify that the application allows 
the user to import data from a file

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed
    - :ref:`Choose File for Data Logging <TST_DATALOGGER_CHOOSE_FILE_FOR_DATA_LOGGING>` is completed successfully
    - :ref:`Enable Live Data Logging <TST_DATALOGGER_ENABLE_LIVE_DATA_LOGGING>` is completed successfully

**Steps:**
    1. Open Data Logger plugin
    2. Open the settings menu
    3. Press **Import Data** button
            - **Expected result:** A file explorer window is opened
            - **Actual result:**

..
  Actual test result goes here.
..

    4. Choose a ".csv" file to import data from
        - **Expected result:** 
            - in the channel menu a new virtual device  called "Import: <file name>" is created
            - channel **xadc-voltage0** is available under the "Import: <file name>" device
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Enable channel **xadc-voltage0** of "Import: <file name>" device 
        - **Expected result:** Data from the file is displayed on the plot relative to the time and date it was recorded 
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


.. _TST_DATALOGGER_SET_MAXIMUM_CHANNEL_DATA_STORAGE:

Test 32: Set Maximum Channel Data Storage
---------------------------------------------

**UID:** TST.DATALOGGER.SET_MAXIMUM_CHANNEL_DATA_STORAGE

**Description:** Verify that the application allows the 
user to set the maximum channel data storage

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Steps:**
    1. Open Preferences
    2. Go to "DataLoggerPlugin" tab
    3. Change the "Maximum data stored for each monitor" from "10Kb" to "1Mb"
        - **Expected result:** The maximum channel data storage is set to "1Mb"
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


.. _TST_DATALOGGER_SET_DATA_LOGGER_READ_INTERVAL:

Test 33: Set Data Logger Read Interval
----------------------------------------

**UID:** TST.DATALOGGER.SET_DATA_LOGGER_READ_INTERVAL

**Description:** Verify that the application allows the 
user to set the read interval

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed  

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Open Preferences
    5. Go to "DataLoggerPlugin" tab
    6. Change the "Read interval" from "1" to "2"
    7. Open Data Logger plugin 
        - **Expected result:** Data is collected every "2" seconds instead of "1" second 
        - **Actual result:**

..
  Actual test result goes here.
..

    8. Change the **Read interval** to "test"
        - **Expected result:** Value is reset to the last default value.
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


.. _TST_DATALOGGER_SET_X_AXIS_DATE_TIME_FORMAT:

Test 34: Set X-Axis Date Time Format
----------------------------------------

**UID:** TST.DATALOGGER.SET_X_AXIS_DATE_TIME_FORMAT

**Description:** Verify that the application allows the user to set the 
date time format for the X Axis

**OS:** ANY

**Preconditions:** 
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Prerequisites:**
    - :ref:`Run and Stop Data Logging <TST_DATALOGGER_RUN_STOP_DATA_LOGGING>` is passed  

**Steps:**
    1. Open Data Logger plugin
    2. Enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Open Preferences
    5. Go to DataLoggerPlugin tab
    6. Change the **Date time format** from "hh:mm:ss" to "mm:ss"
    7. Open Data Logger plugin 
        - **Expected result:** X Axis displays date time in the format "mm:ss" 
        - **Actual result:**

..
  Actual test result goes here.
..

    8. Change the **Date time format** to "123"
        - **Expected result:** Value is reset to the last default value.
        - **Actual result:**

..
  Actual test result goes here.
..

    9. Change the **Date time format** to "test"
        - **Expected result:** Value is reset to the last default value.
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
