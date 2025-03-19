Debugger - Test Suite
============================================================================================================================

.. note::

    User Guide: https://analogdevicesinc.github.io/scopy/plugins/debugger/index.html

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

Setup enviroment:
----------------------------------------------------------------------------------------------------------------------------

.. _pluto-usb-debugger:

**Pluto.Usb:**
        - Open the Scopy application
        - Connect the PlutoSDR to the computer via USB
        - Type the URI of the PlutoSDR in the URI field (if you don't know the URI, just type "ip:192.168.2.1")

.. _tst-dbg-explr-load:

Test 1: Loading the debugger
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.DBG.EXPLR.LOAD

**Description**: This test checks if the debugger plugin is loaded when any device is connected to Scopy.

**Preconditions**:
        - Disable the Scopy scan feature
        - Use :ref:`pluto-usb <pluto-usb>` setup.
        - OS: ANY

**Steps**:
        1. After adding the URI, click the "Verify" button
        2. Select the debugger plugin from the list of compatible plugins (the debugger works with any IIO compatible device)
                - **Expected Result:** The plugin list should contain the Debugger plugin
                - **Actual result:**

..
        Actual test result goes here.
..

        3. Connect to the device and see that the Debugger plugin appears on the left side of the screen (the tool menu)
                - **Expected Result:** The tool menu panel (left side of the screen) should contain the Debugger plugin
                - **Actual result:**

..
        Actual test result goes here.
..

        4. Disconnect the device and see that the Debugger plugin disappears from the tool menu
                - **Expected Result:** The tool menu panel (left side of the screen) should not contain the Debugger plugin
                - **Actual result:**

..
        Actual test result goes here.
..

        5. Connect the device again and see that the Debugger plugin reappears in the tool menu
                - **Expected Result:** The tool menu panel (left side of the screen) should contain the Debugger plugin
                - **Actual result:**

..
        Actual test result goes here.
..

        6. Disconnect the device and see that the Debugger plugin disappears from the tool menu
                - **Expected Result:** The tool menu panel (left side of the screen) should not contain the Debugger plugin
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


.. _tst-dbg-explr-nav:

Test 2: The navigation tree from the debugger displays the correct information
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.DBG.EXPLR.NAV

**Description**: This test checks if the navigation tree from the debugger displays the correct information.

**Test prerequisites**:
        - :ref:`TST.DBG.EXPLR.LOAD <tst-dbg-explr-load>`

**Preconditions**:
        - Scopy is installed on the system
        - Use :ref:`pluto-usb <pluto-usb>` setup.
        - OS: ANY

**Steps**:
        1. Connect a device to Scopy
        2. Connect to the device and see that the Debugger plugin appears on the left side of the screen (the tool menu)
        3. Click on the Debugger plugin
                - **Expected Result:** On the left side of the Debugger tool there is an element under the filter bar that displays the devices from the connected IIO Context
                - **Actual result:**

..
        Actual test result goes here.
..

        4. Click on any element from the navigation tree
                - **Expected Result:** The information about that element is displayed on the right side of the screen (Details View)
                - **Actual result:**

..
        Actual test result goes here.
..

        5. Click on another element from the navigation tree
                - **Expected Result:** The information about the new element is displayed on the right side of the screen
                - **Actual result:**

..
        Actual test result goes here.
..

        6. Double click on an device name, for example the ad9361-phy (should have an arrow on the left of the name).
                - **Expected Result:** The ad9361-phy has a small arrow in the left of it and the list with the channels should be displayed below the device name
                - **Actual result:**

..
        Actual test result goes here.
..

        7. Click on a channel from the list
                - **Expected Result:** The information about the channel is displayed on the right side of the screen
                - **Actual result:**

..
        Actual test result goes here.
..

        8. Click on another channel from the list
                - **Expected Result:** The information about the new channel is displayed on the right side of the screen
                - **Actual result:**

..
        Actual test result goes here.
..

        9. Double click on a channel name, for example voltage0 (should have an arrow on the left of the name).
                - **Expected Result:** The list with the attributes should be displayed below the channel name
                - **Actual result:**

..
        Actual test result goes here.
..

        10. Click on an attribute from the list
                - **Expected Result:** The information about the attribute is displayed on the right side of the screen
                - **Actual result:**

..
        Actual test result goes here.
..

        11. Click on another attribute from the list
                - **Expected Result:** The information about the new attribute is displayed on the right side of the screen
                - **Actual result:**

..
        Actual test result goes here.
..

        12. Double click again on the channel name (voltage0).
                - **Expected Result:** The list with the attributes should disappear
                - **Actual result:**

..
        Actual test result goes here.
..

        13. Double click again on the device name (ad9361-phy).
                - **Expected Result:** The list with the channels should disappear
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


.. _tst-dbg-explr-filter:

Test 3: The Filter Bar from the debugger works correctly
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.DBG.EXPLR.FILTER

**Description**: This test checks if the Filter Bar from the debugger works correctly.

**Test prerequisites**:
        - :ref:`TST.DBG.EXPLR.LOAD <tst-dbg-explr-load>`
        - :ref:`TST.DBG.EXPLR.NAV <tst-dbg-explr-nav>`

**Preconditions**:
        - Use :ref:`pluto-usb <pluto-usb>` setup.
        - OS: ANY

**Steps**:
        1. Connect a device to Scopy
        2. Connect to the device.
                - **Expected Result:** On the top left side of the Debugger tool there is a filter bar. The filter bar should have a placeholder text that says "Type to filter"
                - **Actual result:**

..
        Actual test result goes here.
..

        3. Click on the filter bar and type a string that is not present or contained in the navigation tree.
                - **Expected Result:** The navigation tree should be empty
                - **Actual result:**

..
        Actual test result goes here.
..

        4. Clear the filter bar
                - **Expected Result:** The navigation tree should be populated again. No element should be missing, the context element should be the first one and the devices should be displayed below it (it is opened).
                - **Actual result:**

..
        Actual test result goes here.
..

        5. Click on the filter bar and type a string that is present in the navigation tree.
                - **Expected Result:** The navigation tree should display only the elements that contain the string
                - **Actual result:**

..
        Actual test result goes here.
..

        6. Repeat step 4 and 5 with another string that is present in the navigation tree.
                - **Expected Result:** The steps should work as before.
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


.. _tst-dbg-explr-title:

Test 4: The title bar updates when selecting different elements from the navigation tree
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.DBG.EXPLR.TITLE

**Description**: This test checks if the title bar updates when selecting different elements from the navigation tree.

**Test prerequisites**:
        - :ref:`TST.DBG.EXPLR.LOAD <tst-dbg-explr-load>`
        - :ref:`TST.DBG.EXPLR.NAV <tst-dbg-explr-nav>`

**Preconditions**:
        - Use :ref:`pluto-usb <pluto-usb>` setup.
        - OS: ANY

**Steps**:
        1. Connect a device to Scopy
        2. Connect to the device.
                - **Expected Result:** On the top side of the Debugger tool there is a title bar. The title bar should have a square button with the name of the first element from the navigation tree. After this button, there should be a small green circle with a plus sign.
                - **Actual result:**

..
        Actual test result goes here.
..

        3. Click on any element from the navigation tree.
                - **Expected Result:** The title bar appends the name of the selected element. The name of the element should appear after the first element name and separated in a new button.
                - **Actual result:**

..
        Actual test result goes here.
..

        4. Click on another element from the navigation tree.
                - **Expected Result:** The title bar appends the name of the selected element. The name of the element should appear after the first element name and separated in a new button.
                - **Actual result:**

..
        Actual test result goes here.
..

        5. Open a device from the navigation panel, and then a channel from this device and select an attribute from the channel. 
                - **Expected Result:** The title bar should display the 4 names in order: The context name, the device name, the channel name and the attribute name. Each name should be separated a different button.
                - **Actual result:**

..
        Actual test result goes here.
..

        6. Click on any element from the title bar. 
                - **Expected Result:** The title bar should remove all button after it and the navigation tree should highlight the selected element.
                - **Actual result:**

..
        Actual test result goes here.
..

        7. Click the green circle with the plus sign. 
                - **Expected Result:** The circle should change to an x. (The current element should be added to the watch list, but this behavior is tested in a different test).
                  The watch list is located at the bottom of the instrument.
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


.. _tst-dbg-explr-watch:

Test 5: The watch list from the debugger works correctly
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.DBG.EXPLR.WATCH

**Description**: This test checks if the watch list from the debugger works correctly.

**Test prerequisites**:
        - :ref:`TST.DBG.EXPLR.LOAD <tst-dbg-explr-load>`
        - :ref:`TST.DBG.EXPLR.NAV <tst-dbg-explr-nav>`
        - :ref:`TST.DBG.EXPLR.TITLE <tst-dbg-explr-title>`

**Preconditions**:
        - Use :ref:`pluto-usb <pluto-usb>` setup.
        - OS: ANY

**Steps**:
        1. Connect a device to Scopy
        2. Connect to the device
                - **Expected Result:** At the bottom of the Debugger tool there is a watch list. The watch list should be a tabel with 4 columns: Name, Value, Type and Path. The table should have a header with the column names. There should also be another column with no name where the 'X' buttons for each row are displayed.
                - **Actual result:**

..
        Actual test result goes here.
..

        3. Click on an element from the navigation tree, the title bar should update with the name of the selected element and a green circle with a plus sign should exist on the right side of the title bar.
        4. Click on the green circle with the plus sign.
                - **Expected Result:** The circle should change to an x and the selected element should be added to the watch list. The element should be added in the first row of the table and the columns should be filled with the information from the selected element.
                - **Actual result:**

..
        Actual test result goes here.
..

        5. Repeat step 3 and 4 with another element from the navigation tree. 
                - **Expected Result:** The new element should be added to the watch list in the second row of the table and the columns should be filled with the information from the selected element.
                - **Actual result:**

..
        Actual test result goes here.
..

        6. Modify the value of an element from the watch list. 
                - **Expected Result:** The value should be updated in the table and in the panel above it (the details view).
                - **Actual result:**

..
        Actual test result goes here.
..

        7. Click on the red x from the watch list.
                - **Expected Result:** The selected element should be removed from the watch list.
                - **Actual result:**

..
        Actual test result goes here.
..

        8. Click on the x from the last column of the watch list.
                - **Expected Result:** The selected element should be removed from the watch list.
                - **Actual result:**

..
        Actual test result goes here.
..

        9. Repeat steps 3 to 8 with other elements from the navigation tree.
                - **Expected Result:** All elements are added/removed correctly and the watchlist is up-to-date.
                - **Actual result:**

..
        Actual test result goes here.
..

        10. Add a few elements in the watch list. Click any other element from the watch list.
                - **Expected Result:** The navigation bar, title bar and information from the details view should be updated with the information from the selected element.
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


.. _tst-dbg-explr-details:

Test 6: The details view from the debugger work correctly
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.DBG.EXPLR.DETAILS

**Description**: This test checks if the details view from the debugger work correctly.

**Test prerequisites**:
        - :ref:`TST.DBG.EXPLR.LOAD <tst-dbg-explr-load>`
        - :ref:`TST.DBG.EXPLR.NAV <tst-dbg-explr-nav>`

**Preconditions**: 
        - Have the iio_info command installed on the system
        - Use :ref:`pluto-usb <pluto-usb>` setup.
        - OS: ANY

**Steps**:
        1. Connect a device to Scopy
        2. Connect to the device
                - **Expected Result:** Ensure that on the right side of the Debugger tool there is a details view. The details view should have a title bar with the name of the selected element from the navigation tree. At first the details view should display the context attributes.
                - **Actual result:**

..
        Actual test result goes here.
..

        3. Test the GUI View.
        4. Click on any element from the navigation tree.
                - **Expected Result:** The details view should display the information from the selected element.
                - **Actual result:**

..
        Actual test result goes here.
..

        5. Click on any attribute from the details view. Change the data, press enter or click outside the attribute.
                - **Expected Result:** Under the attribute value, a progress bar should appear and start filling. After the progress bar is filled, the progress bar should turn green for a few seconds and the attribute value should be updated. To verify that the value was updated, run an external program like iio_info and check that the value is the same
                - **Actual result:**

..
        Actual test result goes here.
..

        6. Repeat step 5 with the following 3 types of attributes: a text box (a box where the user can type any string), a combo box (a box where the user can select from a list of options) and a range box (a box where the user can type a number or use the arrows (or the +/- sign) to increase or decrease the value and/or write the desired value).
        7. While selecting different elements from the navigation tree, lower part of the GUI View, the General Info section should update with the information from the selected element (information such as weather it is a hardware monitor, a trigger, has a trigger attached, if the channel is a scan element, input or output, enabled or disabled). All these informations should be present within the elements of the ADALM-PLUTO device.
                - **Expected Result:** The General Info section should update with the information from the selected element.
                - **Actual result:**

..
        Actual test result goes here.
..

        8. Test the IIO View.
        9. Click on the IIOView button, it should be next to the GUI View, under the title. 
                - **Expected Result:** The IIO View should display a snippet similar to the iio_info command. The snippet should contain the information from the selected element from the navigation tree. To check this, run the iio_info command in a terminal and compare the information from the terminal with the information from the IIO View.
                - **Actual result:**

..
        Actual test result goes here.
..

        10. Repeat step 9 with different elements from the navigation tree.
                - **Expected Result:** The IIO View should update with the information from the selected element.
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


.. _tst-dbg-explr-read_all:

Test 7: The read all button correctly reads all visible attributes
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.DBG.EXPLR.READ_ALL

**Description**: This test checks if the read all button correctly reads all visible attributes.

**Test prerequisites**:
        - :ref:`TST.DBG.EXPLR.LOAD <tst-dbg-explr-load>`
        - :ref:`TST.DBG.EXPLR.NAV <tst-dbg-explr-nav>`
        - :ref:`TST.DBG.EXPLR.DETAILS <tst-dbg-explr-details>`

**Preconditions**:
        - Have the iio_attr command installed on the system
        - Use :ref:`pluto-usb <pluto-usb>` setup.
        - OS: ANY

**Steps**:
        1. Connect a device to Scopy
        2. Connect to the device
                - **Expected Result:** The Debugger plugin appears on the left side of the screen (the tool menu)
                - **Actual result:**

..
        Actual test result goes here.
..

        3. Add a few elements in the watch list. The added elements should be device attributes or channel attributes.
        4. Select any element from the navigation tree (preferable one with a few attributes)
        5. Click the read all button.
                - **Expected Result:** Nothing should change as all the attributes are already read.
                - **Actual result:**

..
        Actual test result goes here.
..

        6. In a terminal, run the iio_attr command and change the value of one of the attributes from the selected element or the watch list.
        7. Click the read all button again. 
                - **Expected Result:** The value of the attribute should be updated in the details view and the watch list (optionally, based on weather the changed element is also in the watch list).
                - **Actual result:**

..
        Actual test result goes here.
..

        8. Also check the IIO View. 
                - **Expected Result:** The value of the attribute should be updated in the IIO View as well.
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


.. _tst-dbg-explr-log:

Test 8: The log window from the debugger correctly displays the operations
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.DBG.EXPLR.LOG

**Description**: This test checks if the log window from the debugger correctly displays the operations.

**Test prerequisites**:
        - :ref:`TST.DBG.EXPLR.LOAD <tst-dbg-explr-load>`
        - :ref:`TST.DBG.EXPLR.NAV <tst-dbg-explr-nav>`
        - :ref:`TST.DBG.EXPLR.DETAILS <tst-dbg-explr-details>`

**Preconditions**:
        - Use :ref:`pluto-usb <pluto-usb>` setup.
        - OS: ANY

**Steps**:
        1. Connect a device to Scopy
        2. Connect to the device and see that the Debugger plugin appears on the left side of the screen (the tool menu)
        3. At the bottom of the debugger instrument, next to the 'IIO Attributes' button, there should be a 'Log' button. Click on the 'Log' button.
                - **Expected Result:** The current window should change to the log window.
                - **Actual result:**

..
        Actual test result goes here.
..

        4. Select back the 'IIO Attributes' button.
                - **Expected Result:** The current window should change back to the details view.
                - **Actual result:**

..
        Actual test result goes here.
..

        5. (Correct attribute change) Modify an attribute from the detais view (e.g. on the ADALM-PLUTO device, select the ad9361-phy device and the voltage0 channel and modify the gain_control_mode attribute to hybrid).
                - **Expected Result:** The progress bar should appear and start filling. After the progress bar is filled, it should turn green and the value should be read again.
                - **Actual result:**

..
        Actual test result goes here.
..

        6. (Incorrect attribute change) Select the adm1177-iio device from the navigation tree, the voltage0 channel and modify the raw attribute (write whatever value you want, it should be read-only anyways). After the progress bar is filled, it should turn red and the value should be read again.
                - **Expected Result:** The progress bar should appear and start filling. After the progress bar is filled, it should turn red and the value should be read again.
                - **Actual result:**

..
        Actual test result goes here.
..

        7. Click on the 'Log' button. The log window should appear and display the operations from steps 6 and 7. The operations should be displayed in the following format: [timestamp] [operation] [status] [attribute path] [old value (if write operation)] [new value]. The timestamp should be the current time, the operation should be 'W' (write) or 'R' (read), the status should be SUCCESS or FAILURE and the error code, the attribute path should be the name of the modified attribute title, separated by slashes, the old value should be the value before the modification, the new value should be the value after the modification. If the operation is a read operation, the old value should be empty (not exist). Step 6 should be a successful write operation and step 7 should be a failed write operation. After each write operation, a read operation is automatically performed. The read operation should be displayed in the log window as well.
                - **Expected Result:** The log window should display the operations from steps 6 and 7 in the format described above.
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


