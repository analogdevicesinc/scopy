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
    - Check the Register Map plugin checkbox in the Add Device page.
    - Connect to the M2k Device
    

Test Cases with no XML available
---------------------------------

The following test cases work with or without XML file.

.. _TST_REGMAP_READ:

Test 1 : Read 
^^^^^^^^^^^^^^

**UID:** TST.REGMAP.READ

**RBP:** P2

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

**RBP:** P2

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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
    3. Set the value of the register to "0x4c" using the value picker.
    4. Change the value of "reg002_b3" bitfield from on to off
        - **Expected result:** The **Value** field is updated to the "0x44" value
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Press **Write** button
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

**RBP:** P3

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
    3. Set the value of the register to "0x4c" using the value picker.
    4. Change the value of the **Value** field to "0x44"
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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

Test 20 : Settings Dump register and write values disabled
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.SETTINGS_DUMP_REGIS_AND_WRITE_VALUES_DISABLED

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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

**RBP:** P3

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






Tests with the simulated dongleDevice
-------------------------------------

These tests use the simulated dongleDevice of the register map tool configuration POC. It needs the
``eprom_test`` package (build option ``-DENABLE_PACKAGE_EPROM_TEST=ON``). The device has no IIO context.
Its Register Map starts empty. The **EEPROM configuration** tool (``EpromTestPlugin``) reads the EEPROM of
the board behind the dongle on **Sync** and configures three register map devices on **Configure**:
``eprom1``, ``eprom2`` and ``eprom3``, one per mode. All read/write goes to the CSV files of the package.

.. _regmap_dongle_setup:

**Dongle.Sim:**
    - Install Scopy with the ``eprom_test`` package
    - Open Scopy
    - The simulated dongleDevice (``dongle:sim0``) is already on the Home page
    - Connect to it

.. _regmap_eeprom_sim_setup:

**Eeprom.Sim:**
    - :ref:`Dongle.Sim <regmap_dongle_setup>`
    - Open **EEPROM configuration**, press **Sync**, then **Configure**
    - Open the Register Map plugin

.. _TST_REGMAP_EEPROM_SIM_DEVICE:

Test 30 : Simulated EEPROM device
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.EEPROM_SIM_DEVICE

**RBP:** P1

**Description:** Verify that after Configure the register map of the dongleDevice shows the three
configured devices.

**OS:** ANY

**Precondition:**
    - :ref:`Eeprom.Sim <regmap_eeprom_sim_setup>`

**Steps:**
    1. Look at the Home page
        - **Expected result:** A device named ``dongleDevice`` with the URI ``dongle:sim0`` is on the Home page
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Look at the tool list of the connected device
        - **Expected result:** The tools are EEPROM configuration and Register Map
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Open Register Map and open the device list
        - **Expected result:** The list holds ``eprom1``, ``eprom2`` and ``eprom3``
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

.. _TST_REGMAP_EEPROM_CSV_READ:

Test 31 : Read from the CSV file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.EEPROM_CSV_READ

**RBP:** P1

**Description:** Verify that a read on ``eprom1`` returns the value from ``eprom1.csv``.

**OS:** ANY

**Precondition:**
    - :ref:`Eeprom.Sim <regmap_eeprom_sim_setup>`

**Steps:**
    1. Select device ``eprom1``
    2. Select register ``POC_REG`` (``0x004``) and press Read
        - **Expected result:** The value is ``0x9e``
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

.. _TST_REGMAP_EEPROM_CSV_WRITE:

Test 32 : Write to the CSV file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.EEPROM_CSV_WRITE

**RBP:** P1

**Description:** Verify that a write on ``eprom1`` changes the CSV file and the read-back shows the new
value.

**OS:** ANY

**Precondition:**
    - :ref:`Eeprom.Sim <regmap_eeprom_sim_setup>`

**Steps:**
    1. Select device ``eprom1``
    2. Write ``0x33`` to register ``0x001``
        - **Expected result:** The value shows ``0x33`` and the line in ``eprom1.csv`` becomes
          ``0x001,0x33``
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Write ``0x6b`` (the original value) to register ``0x001``
        - **Expected result:** The value shows ``0x6b``
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

.. _TST_REGMAP_EEPROM_TEMPLATE_ONLY:

Test 33 : Template only device
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.EEPROM_TEMPLATE_ONLY

**RBP:** P2

**Description:** Verify that a device with a template and no read/write shows the register table without
any read/write control.

**OS:** ANY

**Precondition:**
    - :ref:`Eeprom.Sim <regmap_eeprom_sim_setup>`

**Steps:**
    1. Select device ``eprom2``
        - **Expected result:** The POC register table is shown: 5 registers, the 5th (``0x004``) is ``POC_REG``
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Look under the table and at the top right
        - **Expected result:** There is no read/write section (no address, Read, value or Write) and no
          settings button. The bitfield details are shown under the table
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Search for ``0x0`` and click a few registers and bitfields
        - **Expected result:** The search filters the table, the details update and nothing crashes
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

.. _TST_REGMAP_EEPROM_READ_WRITE_ONLY:

Test 34 : Read/write only device
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.EEPROM_READ_WRITE_ONLY

**RBP:** P2

**Description:** Verify that a device with read/write and no template reads and writes its own CSV file.

**OS:** ANY

**Precondition:**
    - :ref:`Eeprom.Sim <regmap_eeprom_sim_setup>`

**Steps:**
    1. Select device ``eprom3``
        - **Expected result:** There is no register table and no search bar, only the address, Read and Write
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Read address ``0x0``
        - **Expected result:** The value is ``0x90062``
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Write ``0x12`` to address ``0x4``, then write ``0x0`` back
        - **Expected result:** The value shows ``0x12``, then ``0x0``
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

.. _TST_REGMAP_EEPROM_MISSING_ADDRESS:

Test 35 : Address missing from the CSV file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.EEPROM_MISSING_ADDRESS

**RBP:** P2

**Description:** Verify that reading an address that is not in the CSV file shows an error.

**OS:** ANY

**Precondition:**
    - :ref:`Eeprom.Sim <regmap_eeprom_sim_setup>`

**Steps:**
    1. Select device ``eprom1``
    2. Type ``0x8`` in the address field and press Read (``0x008`` is not in the template)
        - **Expected result:** The status bar shows
          ``Register map eprom1: no value for address 0x8 in the register values file``
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

.. _TST_REGMAP_DONGLE_EMPTY_TOOL:

Test 36 : Empty register map before Configure
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.DONGLE_EMPTY_TOOL

**RBP:** P1

**Description:** Verify that after connecting the dongleDevice the register map is an empty tool.

**OS:** ANY

**Precondition:**
    - :ref:`Dongle.Sim <regmap_dongle_setup>`

**Steps:**
    1. Open Register Map
        - **Expected result:** The device list is empty. There is no register table, no search bar, no
          read/write section and no settings button
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Open the info button and press the tutorial button
        - **Expected result:** Nothing happens and Scopy doesn't crash
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

.. _TST_REGMAP_DONGLE_SYNC:

Test 37 : Sync the board EEPROM
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.DONGLE_SYNC

**RBP:** P1

**Description:** Verify that Sync reads the board EEPROM and shows the configuration structure, without
configuring the register map.

**OS:** ANY

**Precondition:**
    - :ref:`Dongle.Sim <regmap_dongle_setup>`

**Steps:**
    1. Open EEPROM configuration
        - **Expected result:** The structure box is empty and Configure is disabled
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Press Sync
        - **Expected result:** The box shows a ``regmap`` list with ``eprom1``, ``eprom2`` and ``eprom3``,
          their template, access and file. The status says 3 devices were synced and Configure is enabled
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Try to type in the structure box
        - **Expected result:** The box is read-only
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Open Register Map
        - **Expected result:** The device list is still empty
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

.. _TST_REGMAP_DONGLE_CONFIGURE:

Test 38 : Configure the register map
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.DONGLE_CONFIGURE

**RBP:** P1

**Description:** Verify that Configure fills the register map with the synced devices, and that a new
connection starts empty again.

**OS:** ANY

**Precondition:**
    - :ref:`Dongle.Sim <regmap_dongle_setup>`

**Steps:**
    1. Open EEPROM configuration, press Sync, then Configure
        - **Expected result:** The status says 3 devices were configured
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Open Register Map and open the device list
        - **Expected result:** The list holds ``eprom1``, ``eprom2`` and ``eprom3``
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Disconnect, connect again and open Register Map
        - **Expected result:** The device list is empty again
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

.. _TST_REGMAP_DONGLE_SWITCH_DEVICE:

Test 39 : Switch the board behind the dongle
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.DONGLE_SWITCH_DEVICE

**RBP:** P1

**Description:** Verify that after the board behind the dongle is replaced, Sync reads the new board and
Configure reconfigures the register map without disconnecting.

**OS:** ANY

**Precondition:**
    - :ref:`Eeprom.Sim <regmap_eeprom_sim_setup>`

**Steps:**
    1. Open EEPROM configuration and press **Switch device**
        - **Expected result:** The tool shows ``Connected board: board 2``, the structure box is empty,
          Configure is disabled and the status asks for Sync
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Open Register Map and open the device list
        - **Expected result:** The list still holds ``eprom1``, ``eprom2`` and ``eprom3``
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Go back to EEPROM configuration and press **Sync**
        - **Expected result:** The box shows ``"board": "board 2"`` and one ``regmap`` entry, ``eprom4``
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Press **Configure**, then open Register Map and open the device list
        - **Expected result:** The list holds only ``eprom4``. The device is still connected
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

.. _TST_REGMAP_DONGLE_EPROM4_READ_WRITE:

Test 40 : Read and write on the new board
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.DONGLE_EPROM4_READ_WRITE

**RBP:** P2

**Description:** Verify that ``eprom4`` uses its own template and reads and writes its own CSV file.

**OS:** ANY

**Precondition:**
    - :ref:`Eeprom.Sim <regmap_eeprom_sim_setup>`
    - Steps 1-4 of :ref:`Test 39 <TST_REGMAP_DONGLE_SWITCH_DEVICE>` are done (board 2 is configured)

**Steps:**
    1. Select device ``eprom4``
        - **Expected result:** 5 registers: BOARD_ID, BOARD_REV, GPIO_DIR, GPIO_OUT and POC_REG
          (``0x000``-``0x004``)
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Select register ``POC_REG`` (``0x004``) and press Read
        - **Expected result:** The value is ``0x4b``
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Write ``0x55`` to register ``0x001``, then write ``0x1c`` (the original value) back
        - **Expected result:** The value shows ``0x55``, then ``0x1c``, and ``eprom4.csv`` ends with ``0x001,0x1c``
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

.. _TST_REGMAP_DONGLE_SWITCH_BACK:

Test 41 : Switch back to the first board
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.REGMAP.DONGLE_SWITCH_BACK

**RBP:** P2

**Description:** Verify that switching back to board 1 and pressing Sync and Configure brings back its three
register map devices without disconnecting.

**OS:** ANY

**Precondition:**
    - :ref:`Eeprom.Sim <regmap_eeprom_sim_setup>`
    - Steps 1-4 of :ref:`Test 39 <TST_REGMAP_DONGLE_SWITCH_DEVICE>` are done (board 2 is configured)

**Steps:**
    1. Open EEPROM configuration, press **Switch device**, then **Sync**
        - **Expected result:** The tool shows ``Connected board: board 1`` and the box lists ``eprom1``,
          ``eprom2`` and ``eprom3``
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Press **Configure**, then open Register Map and open the device list
        - **Expected result:** The list holds ``eprom1``, ``eprom2`` and ``eprom3`` and no ``eprom4``
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Disconnect, connect again and open EEPROM configuration
        - **Expected result:** The tool shows ``Connected board: board 1`` and Register Map is empty
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
