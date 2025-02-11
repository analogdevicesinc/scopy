.. _registermap_tests:

Register Map - Test suite
================================================================================

.. note::

    :ref:`User guide<registermap>` 

The following apply for all tests bellow, any special requirements will be mentioned. 

Prerequisites: 
    - Scopy v2.0.0 or later with Register Map installed on the system
    - Tests listed as dependencies are successfully completed
    - Reset .ini files to default by pressing "Reset" button in Preferences

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

.. _regmap_adalm_pluto_emu_setup:

**AdalmPluto.Emu:**
    - Open Scopy
    - Start the iio-emu process using pluto option from the dropdown
    - Connect to the Pluto Device
  
.. _regmap_adalm_pluto_device_setup:

**AdalmPluto.Device:**
    - Open Scopy
    - Connect to the Pluto Device using Ip or USB 
  
.. _regmap_m2k_emu_setup:

**M2k.Emu:**
    - Open Scopy
    - Start the iio-emu process using adalm2000 option from the dropdown
    - Connect to the M2k Device
    

Test Cases with no XML available
---------------------------------

The following test cases work with or without XML file.

.. _TST_REGMAP_READ:

Test 1 : Read 
^^^^^^^^^^^^^^

**UID:** TST.REGMAP.READ

**Description:** Verify that the user can read the register value.

**OS:** ANY

**Precondition:**
    - :ref:`M2k.Emu <regmap_m2k_emu_setup>` is connected
  
**Steps:**
    1. Open Register Map plugin 
    2. Select a register address "0x2" using address picker
    3. Press **Read** button
        - **Expected result:** The value "0x02" is displayed in the **Value** field 
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


.. _TST_REGMAP_WRITE:

Test 2 : Write
^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.WRITE

**Description:** Verify that the user can write the register value.

**OS:** ANY

**Precondition:**    
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected

**Prerequisites:**
    - :ref:`Read <TST_REGMAP_READ>` is passed
  
**Steps:**
    1. Open Register Map plugin
    2. Select a register address "0x2" using address picker
    3. Read current value of the register
    4. Change the value to 0x4a
        - **Expected result:** The value "0x4a" is displayed in the **Value** field
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


.. _TST_REGMAP_WRITE_WRONG_VALUE:

Test 3 : Write wrong value
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.WRITE_WRONG_VALUE  

**Description:** Verify that the user can not write the wrong value to the register.

**OS:** ANY

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected

**Prerequisites:**
    - :ref:`Read <TST_REGMAP_READ>` is passed

**Steps:**
    1. Open Register Map plugin
    2. Select a register address "0x2" using address picker
    3. Read current value of the register
    4. Change the value to 0xtest
        - **Expected result:** The value "0x00" is displayed in the **Value** field 
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


.. _TST_REGMAP_CHANGE_DEVICE:

Test 4 : Change device
^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.CHANGE_DEVICE  

**Description:** Verify that the user can change the device.

**OS:** ANY

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` or 
      :ref:`AdalmPluto.Emu <regmap_adalm_pluto_emu_setup>` is connected

**Steps:**
    1. Open Register Map plugin

    2. Change device using the dropdown in the top right corner from 
       ad9361-phy to cf-ad9361-lpc

        - **Expected result:** The device is changed reflected by changing the register table 
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


Tests with XML available
--------------------------  

The following test cases require XML file of the device to be present in the system.

.. _TST_REGMAP_SHOW_TABLE:

Test 5 : Show register map table
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.SHOW_TABLE

**Description:** Verify that the user can see the register map table.

**OS:** ANY

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system

**Steps:**
    1. Open Register Map plugin
        - **Expected result:** The register map table is displayed in the plugin
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


.. _TST_REGMAP_SEARCH:

Test 6 : Search register
^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.SEARCH

**Description:** Verify that the user can search the for a register.

**OS:** ANY

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Show register map table <TST_REGMAP_SHOW_TABLE>` is passed

**Steps:**
    1. Open Register Map plugin
    2. Make sure device ad9361-phy is selected
    3. Input "test" in the search field
    4. Press **Search** button or enter

        - **Expected result:** Four registers are displayed in the 
          table each of them has "test" in the name or 
          description of the register or bitfields

        - **Actual result:**

..
  Actual test result goes here.
..

    5. Input "010" in the search field
        - **Expected result:** Three results are displayed in the 
          table one of them has "010" in the address and the 
          rest have "010" in the name or description of the 
          register or bitfields

        - **Actual result:**

..
  Actual test result goes here.
..

    6. Input "not in here" in the search field
        - **Expected result:** No register is displayed in the table
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Clear the search field
        - **Expected result:** All registers are displayed in the table.
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


.. _TST_REGMAP_SIMPLE_REGISTER_VALUE_UPDATED_ON_READ:

Test 7 : Simple Register value updated on read
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.SIMPLE_REGISTER_VALUE_UPDATED_ON_READ

**Description:** Verify that the user can see the register value in the table updated 
    on read.

**OS:** ANY

**Precondition:**  
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Show register map table <TST_REGMAP_SHOW_TABLE>` is passed

**Steps:**
    1. Open Register Map plugin
    2. Select a register address "0x2" using address picker
    3. Press **Read** button
        - **Expected result:** The value in the table is updated to the new value.
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


.. _TST_REGMAP_SIMPLE_REGISTER_VALUE_UPDATED_ON_WRITE:

Test 8 : Simple Register value updated on write
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.SIMPLE_REGISTER_VALUE_UPDATED_ON_WRITE

**Description:** Verify that the user can see the register value in the table updated 
    on write.

**OS:** ANY

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Simple Register value updated on read <TST_REGMAP_SIMPLE_REGISTER_VALUE_UPDATED_ON_READ>` is passed

**Steps:**
    1. Open Register Map plugin
    2. Select a register address "0x2" using address picker
    3. Read current value of the register
    4. Change the value to 0x4a
    5. Press **Write** button
        - **Expected result:** The value in the table is updated to the new value
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


.. _TST_REGMAP_DETAILED_REGISTER_VALUE_UPDATED_ON_READ:

Test 9 : Detailed Register value updated on read
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.DETAILED_REGISTER_VALUE_UPDATED_ON_READ

**Description:** Verify that the detailed register bitfields value is updated on read.

**OS:** ANY

**Precondition:** 
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system  

**Prerequisites:**
    - :ref:`Show register map table <TST_REGMAP_SHOW_TABLE>` is passed

**Steps:** 
    1. Open Register Map plugin
    2. Select a register address "0x2" using address picker
    3. Press **Read** button
        - **Expected result:** The detailed register bitfields value is updated to the new value
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


.. _TST_REGMAP_DETAILED_REGISTER_VALUE_UPDATED_ON_WRITE:

Test 10 : Detailed Register value updated on write
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.DETAILED_REGISTER_VALUE_UPDATED_ON_WRITE

**Description:** Verify that the detailed register bitfields value is updated on write.

**OS:** ANY

**Precondition:** 
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Detailed Register value updated on read <TST_REGMAP_DETAILED_REGISTER_VALUE_UPDATED_ON_READ>` is passed

**Steps:** 
    1. Open Register Map plugin
    2. Select a register address "0x2" using address picker
    3. Read current value of the register
    4. Change the value to 0x4a
    5. Press **Write** button
        - **Expected result:** The detailed register bitfields value is updated to the new value
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


.. _TST_REGMAP_CHANGE_BITFIELD_VALUE:

Test 11 : Change value of the bitfield inside the detailed register
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.CHANGE_BITFIELD_VALUE

**Description:** Verify that when value of the bitfield is changed the **Value** field is 
    updated. Changing the bitfield value should not change the value of the 
    register until **Write** button is pressed. 

**OS:** ANY

**Precondition:** 
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Show register map table <TST_REGMAP_SHOW_TABLE>` is passed

**Steps:** 
    1. Open Register Map plugin
    2. Select a register address "0x2" using address picker
    3. Change the value of "reg002_b3" bitfield from on to off
        - **Expected result:** The **Value** field is updated to the "0x44" value
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Press **Write** button
        - **Expected result:** The register value is updated to "0x44" 
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


.. _TST_REGMAP_CHANGE_VALUE_FIELD:

Test 12 : Change **Value** field updates detailed register bitfields
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.CHANGE_VALUE_FIELD

**Description:** Verify that when value of the **Value** field is changed the detailed 
    register bitfields are updated.

**OS:** ANY

**Precondition:** 
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Show register map table <TST_REGMAP_SHOW_TABLE>` is passed

**Steps:**
    1. Open Register Map plugin
    2. Select a register address "0x2" using address picker
    3. Change the value of the **Value** field to "0x44"
        - **Expected result:** The detailed bitfield "reg002_b3" value is changed from on to off
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


.. _TST_REGMAP_CHANGE_SELECTED_REGISTER:

Test 13 : Change selected register
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.CHANGE_SELECTED_REGISTER

**Description:** Verify that when the register is changed the detailed register bitfields 
    are updated.

**OS:** ANY

**Precondition:** 
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Show register map table <TST_REGMAP_SHOW_TABLE>` is passed

**Steps:**
    1. Open Register Map plugin
    2. Select a register address "0x20" using address picker
        - **Expected result:** The register "0x20" is displayed in the table and selected 
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Using + button from address picker change the register to "0x21"
        - **Expected result:** The register "0x21" is displayed in the table and selected
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Using - button from address picker change the register to "0x20"
        - **Expected result:** The register "0x20" is displayed in the table and selected
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Click on the register "0x21" in the table
        - **Expected result:** The register "0x21" in the table is selected value from address picker is updated to "0x21"
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


.. _TST_REGMAP_TOGGLE_DETAILED_REGISTER_VISIBLE:

Test 14 : Toggle detailed register visible 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.TOGGLE_DETAILED_REGISTER_VISIBLE

**Description:** Verify that the user can toggle the detailed register visibility.

**OS:** ANY

**Precondition:** 
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Show register map table <TST_REGMAP_SHOW_TABLE>` is passed

**Steps:** 
    1. Open Register Map plugin
    2. Press **Toggle detailed register** button
        - **Expected result:** The detailed register is hidden
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Press **Toggle detailed register** button
        - **Expected result:** The detailed register is visible
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


.. _TST_REGMAP_SETTINGS_AUTOREAD:

Test 15 : Settings autoread 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.SETTINGS_AUTOREAD

**Description:** Verify that the autoread setting is working.

**OS:** ANY

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Show register map table <TST_REGMAP_SHOW_TABLE>` is passed
    - :ref:`Read <TST_REGMAP_READ>` is passed

**Steps:**
    1. Open Register Map plugin
    2. Press **Settings** button
    3. Check **Autoread** checkbox
    4. Select a register address "0x02" using address picker
        - **Expected result:** The value is updated to the read value
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


.. _TST_REGMAP_SETTINGS_READ_INTERVAL:

Test 16 : Settings read interval
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.SETTINGS_READ_INTERVAL

**Description:** Verify that the read interval setting is working.

**OS:** ANY

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Show register map table <TST_REGMAP_SHOW_TABLE>` is passed
    - :ref:`Read <TST_REGMAP_READ>` is passed

**Steps:**
    1. Open Register Map plugin
    2. Press **Settings** button
    3. Input "2" in the "From register" field
    4. Input "4" in the "To register" field
    5. Press **Read interval** button
        - **Expected result:** registers "0x02", "0x03" and "0x04" now have their values read   
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


.. _TST_REGMAP_SETTINGS_READ_INTERVAL_WRONG_INTERVAL:

Test 17 : Settings read interval with wrong interval
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.SETTINGS_READ_INTERVAL_WRONG_INTERVAL

**Description:** Verify that the read interval setting is not working with wrong 
    interval.

**OS:** ANY

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Settings read interval <TST_REGMAP_SETTINGS_READ_INTERVAL>` is passed

**Steps:** 
    1. Open Register Map plugin
    2. Press **Settings** button
    3. Input "4" in the "From register" field
    4. Input "2" in the "To register" field
    5. Press **Read interval** button
        - **Expected result:** Nothing happens
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


.. _TST_REGMAP_SETTINGS_REGISTER_DUMP:

Test 18 : Settings register dump 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.SETTINGS_REGISTER_DUMP

**Description:** Verify that the register dump is working.

**OS:** ANY

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Show register map table <TST_REGMAP_SHOW_TABLE>` is passed

**Steps:**
    1. Open Register Map plugin
    2. Read value from registers "0x02", "0x03" and "0x04"
    3. Press **Settings** button
    4. Press **Find path** button
    5. Select a ".csv" file to save the register dump
        - **Expected result:** file path is displayed in the **File path** field
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Press **Register dump** button
        - **Expected result:** The read register values will be saved 
          in the selected file with the structure of each row 
          containing register address and register value

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


.. _TST_REGMAP_SETTINGS_WRITE_VALUES:

Test 19 : Settings write values
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.SETTINGS_WRITE_VALUES

**Description:** Verify that the write values setting is working.

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Settings register dump <TST_REGMAP_SETTINGS_REGISTER_DUMP>` is passed

**Steps:**
    1. Open Register Map plugin
    2. Press **Settings** button
    3. Press **Find path** button
    4. Select the ".csv" file generated in Test 18
    5. Press **Write values** button
        - **Expected result:** The values from the selected file are written to the registers
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



.. _TST_REGMAP_SETTINGS_DUMP_REGIS_AND_WRITE_VALUES_DISABLED:

Test 20 : Settings Dump regisr and write values disabled
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.SETTINGS_DUMP_REGIS_AND_WRITE_VALUES_DISABLED

**Description:** Verify that the **Register dump** and **Write values** buttons are disabled 
    when the file path is not selected.

**OS:** ANY

**Precondition:**    
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` is connected
    - XML file of the device is present in the system   

**Steps:**
    1. Open Register Map plugin
    2. Press **Settings** button
    3. Press **Register dump** button
        - **Expected result:** The button is disabled
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Press **Write values** button
        - **Expected result:** The button is disabled
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Use **Find path** button to select a file
        - **Expected result:** The **Write values** and **Register dump** buttons are enabled
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


.. _TST_REGMAP_PREFERENCES_COLOR_CODING_BACKGROUND_COLOR:

Tests 21 : Preferences color coding background color
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.PREFERENCES_COLOR_CODING_BACKGROUND_COLOR

**Description:** Verify that the background color of the register in the table is 
    changing to reflect the value of the register.

**OS:** ANY

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` or 
      :ref:`AdalmPluto.Emu<regmap_adalm_pluto_emu_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Read <TST_REGMAP_READ>` is passed
    - :ref:`Show register map table <TST_REGMAP_SHOW_TABLE>` is passed

**Steps:**
    1. Open Preferences 
    2. Go to RegmapPlugin tab
    3. Using the dropdown select **Register background and bitfield 
       background**
    4. Open Register Map plugin
    5. Select a register with the address "0x2" 
    6. Press **Read** button
        - **Expected result:** 
            - The background color of the register is changed to reflect the value of the register 
            - The background color of the bitfields is changed to reflect the value of the bitfield
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


.. _TST_REGMAP_PREFERENCES_COLOR_CODING_TEXT_COLOR:

Test 22 : Preferences color coding text color
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.PREFERENCES_COLOR_CODING_TEXT_COLOR

**Description:** Verify that the text color of the register in the table is changing to 
    reflect the value of the register.

**OS:** ANY

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` or 
      :ref:`AdalmPluto.Emu<regmap_adalm_pluto_emu_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Read <TST_REGMAP_READ>` is passed
    - :ref:`Show register map table <TST_REGMAP_SHOW_TABLE>` is passed
  
**Steps:** 
    1. Open Preferences 
    2. Go to RegmapPlugin tab
    3. Using the dropdown select **Register text and bitfield text**
    4. Open Register Map plugin
    5. Select a register with the address "0x2" 
    6. Press **Read** button
        - **Expected result:** 
            - The text color of the register is changed to reflect the value of the register 
            - The text color of the bitfields is changed to reflect the value of the bitfield
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


.. _TST_REGMAP_PREFERENCES_COLORS_VALUE_RANGE:

Test 23 : Preferences color for value
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.PREFERENCES_COLORS_VALUE_RANGE

**Description:** Verify that the colors used for the values are corect.

**OS:** ANY

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` or 
      :ref:`AdalmPluto.Emu<regmap_adalm_pluto_emu_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Read <TST_REGMAP_READ>` is passed
    - :ref:`Show register map table <TST_REGMAP_SHOW_TABLE>` is passed

**Steps:**
    1. Open Preferences 
    2. Go to RegmapPlugin tab
    3. Using the dropdown select **Register text and bitfield text**
    4. Open Register Map plugin
    5. Select a register with the address "0x2" 
    6. Write value "0x6f" to the register
    7. Press **Read** button
    8. Select a register with the address "0x3"
    9. Write value "0x6f" to the register
    10. Press **Read** button
        - **Expected result:** The bitfields that have the same value will have the text value same color 
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


.. _TST_REGMAP_PREFERENCES_COLOR_CODING_DYNAMIC_CHANGE:

Test 24 : Preferences color coding dynamic change
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.PREFERENCES_COLOR_CODING_DYNAMIC_CHANGE

**Description:** Verify that the color coding is changing dynamically.

**OS:** ANY

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` or 
      :ref:`AdalmPluto.Emu<regmap_adalm_pluto_emu_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Read <TST_REGMAP_READ>` is passed
    - :ref:`Show register map table <TST_REGMAP_SHOW_TABLE>` is passed

**Steps:**
    1. Open Register Map plugin
    2. Select a register with the address "0x2"
    3. Press **Read** button
    4. Open Preferences
    5. Go to RegmapPlugin tab
    6. Using the dropdown select **Register background and bitfield 
       background**
    7. Open Register Map plugin
        - **Expected result:** The background color of the register is changed to reflect the value of the register
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


.. _TST_REGMAP_PREFERENCES_SETTINGS_SAVE_ON_CLOSE:

Test 25 : Preference settings save on close
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.PREFERENCES_SETTINGS_SAVE_ON_CLOSE

**Description:** Verify that the preferences are saved on close.

**OS:** ANY

**Precondition:**   
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` or 
      :ref:`AdalmPluto.Emu<regmap_adalm_pluto_emu_setup>` is connected
    - XML file of the device is present in the system

**Prerequisites:**
    - :ref:`Read <TST_REGMAP_READ>` is passed
    - :ref:`Show register map table <TST_REGMAP_SHOW_TABLE>` is passed

**Steps:**
    1. Open Preferences
    2. Go to RegmapPlugin tab
    3. Using the dropdown select **Register background and bitfield 
       background**
    4. Close Scopy
    5. Open Scopy
    6. Connect to the device
    7. Open Register Map plugin 
    8. Select register with address "0x2"
    9. Press **Read** button
        - **Expected result:** The background color of the register is changed to reflect the value of the register
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



.. _TST_REGMAP_INFO_BUTTON_DOCUMENTATION:

Test 26 : Info button documentation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.INFO_BUTTON_DOCUMENTATION

**Description:** Verify that the user can see the documentation for the register.

**OS:** ANY

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` or 
      :ref:`AdalmPluto.Emu<regmap_adalm_pluto_emu_setup>` is connected
    - An internet connection is available

**Steps:**
    1. Open Register Map plugin
    2. Press **i** button
    3. Press **Documentation** button
        - **Expected result:** An internet browser is opened with the documentation for the register
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


.. _TST_REGMAP_INFO_BUTTON_TUTORIAL:

Test 27 : Info button tutorial
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.INFO_BUTTON_TUTORIAL

**Description:** Verify that the user can see the tutorial for the register.

**OS:** ANY

**Precondition:**
    - :ref:`AdalmPluto.Device <regmap_adalm_pluto_device_setup>` or 
      :ref:`AdalmPluto.Emu<regmap_adalm_pluto_emu_setup>` is connected
    - XML file of the device is present in the system

**Steps:**
    1. Open Register Map plugin
    2. Press **i** button
    3. Press **Tutorial** button
        - **Expected result:** A tutorial explaining how to use the plugin is displayed 
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


.. _TST_REGMAP_INFO_BUTTON_TUTORIAL_NO_XML:

Test 28 : Info button tutorial no XML
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.INFO_BUTTON_TUTORIAL_NO_XML

**Description:** Verify that the user can see the tutorial for the register when no XML 
    file is present.

**OS:** ANY

**Precondition:** 
    - :ref:`M2k<regmap_m2k_emu_setup>` is connected

**Steps:**
    1. Open Register Map plugin
    2. Press **i** button
    3. Press **Tutorial** button
        - **Expected result:** A tutorial explaining how to use the 
          plugin is displayed and does not include explanation of 
          the register map table

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


Test 29 : Custom XML file
^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.CUSTOM_XML_FILE

**Description:** Verify that the user can use a custom XML file.

**OS:** WINDOWS

**Precondition:**
    Create an XML file with the following structure:

    .. code-block:: xml

        <ad9963>
            <Register>
            <Name>reg000</Name>
            <Address>0x000</Address>
            <Description>Comm</Description>
            <Exists>True</Exists>
            <Width>8</Width>
            <Notes></Notes>
            <BitFields>
            <BitField>
                <Name>Reset</Name>
                <Access>R/W</Access>
                <DefaultValue>0</DefaultValue>
                <Description></Description>
                <Visibility>Public</Visibility>
                <Width>1</Width>
                <Notes>The device is placed in reset when this bit is written high and remains in reset until the bit is written low.</Notes>
                <BitOffset>0</BitOffset>
                <RegOffset>5</RegOffset>
                <SliceWidth>1</SliceWidth>
            </BitField>
            <BitField>
                <Name>LSB_FIRST</Name>
                <Access>R/W</Access>
                <DefaultValue>0</DefaultValue>
                <Description></Description>
                <Visibility>Public</Visibility>
                <Width>1</Width>
                <Notes>Serial port communication, LSB or MSB first.</Notes>
                <Options>
                <Option>
                    <Description>0 = MSB first</Description>
                    <Value>0</Value>
                </Option>
                <Option>
                    <Description>1 = LSB first</Description>
                    <Value>1</Value>
                </Options>
                <BitOffset>0</BitOffset>
                <RegOffset>6</RegOffset>
                <SliceWidth>1</SliceWidth>
            </BitField>
            <BitField>
                <Name>SDIO</Name>
                <Access>R/W</Access>
                <DefaultValue>0</DefaultValue>
                <Description></Description>
                <Visibility>Public</Visibility>
                <Width>1</Width>
                <Notes>SDIO pin operation.</Notes>
                <Options>
                <Option>
                    <Description>0 = SDIO - input only</Description>
                    <Value>0</Value>
                </Option>
                <Option>
                    <Description>1 = SDIO - bidirectional input/output</Description>
                    <Value>1</Value>
                </Options>
                <BitOffset>0</BitOffset>
                <RegOffset>7</RegOffset>
                <SliceWidth>1</SliceWidth>
            </BitField>
            </BitFields>
            </Register>
        </ad9963>


**Steps:**
    1. Open Scopy
    2. Enable the M2k.Emu with the "Register Map" plugin enabled 
    3. Connect to the M2k.Emu
    4. Open Register Map plugin
    5. Select device "ad9963"
        - **Expected result:** The register map has no table displayed
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Close Scopy
    7. Copy the XML file where you have installed Scopy in the 
       "plugins/xmls" folder
    8. Open Scopy   
    9. Enable the M2k.Emu with the "Register Map" plugin enabled
    10. Connect to the M2k.Emu
    11. Open Register Map plugin
    12. Select device "ad9963"
         - **Expected result:** 
            - The register map table is displayed and contains only the registers from the custom XML file
            - The detailed register is displayed and contains the data from the custom XML file

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




