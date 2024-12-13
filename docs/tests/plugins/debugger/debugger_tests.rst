.. Debugger
This module contains tests for the debugger.
.. _debugger_tests:
Debugger Tests
--------------

Test 1: Loading the debugger
===========================
Description: This test checks if the debugger plugin is loaded when any device is connected to Scopy.
Preconditions:
    - Scopy is installed on the system
    - A device is connected to Scopy
    - The device is IIO compatible
    - The device is supported by the debugger plugin
    - The debugger plugin is installed on the system

Tested on:
        - Platform: Ubuntu 20.04
        - Scopy version: 2.0.0
        - Device: ADALM-PLUTO

Steps:
        Step 1: Connect a device to Scopy
        Step 2: Ensure that the Debugger plugin is appears in the plugin list (the debugger works with any IIO compatible device)
        Step 3: Connect to the device and see that the Debugger plugin appears on the left side of the screen (the tool menu)
        Step 4: Disconnect the device and see that the Debugger plugin disappears from the tool menu
        Step 5: Connect the device again and see that the Debugger plugin reappears in the tool menu
        Step 6: Disconnect the device and see that the Debugger plugin disappears from the tool menu

Test 2: The navigation tree from the debugger displays the correct information
========================================================================
Description: This test checks if the navigation tree from the debugger displays
the correct information.
Preconditions:
    - Scopy is installed on the system
    - A device is connected to Scopy
    - The device is IIO compatible
    - The device is supported by the debugger plugin
    - The debugger plugin is installed on the system

Tested on:
        - Platform: Ubuntu 20.04, Windows 10, MacOS 14.5
        - Scopy version: 2.0.0
        - Device: ADALM-PLUTO

Steps:
        Step 1: Connect a device to Scopy
        Step 2: Connect to the device and see that the Debugger plugin appears on the left side of the screen (the tool menu)
        Step 3: Ensure that on the left side of the Debugger tool there is an element under the filter bar that displays the devices from the connected IIO Context
        Step 4: Click on any element from the navigation tree and see that the information is displayed on the right side of the screen
        Step 5: Click on another element from the navigation tree and see that the information is displayed on the right side of the screen
        Step 6: Double click on an device name (should have an arrow on the left of the name). It should open a list below the device name with the channels from the device.
        Step 7: Click on a channel from the list and see that the information is displayed on the right side of the screen
        Step 8: Click on another channel from the list and see that the information is displayed on the right side of the screen
        Step 9: Double click on a channel name (should have an arrow on the left of the name). It should open a list below the channel name with the attributes from the channel.
        Step 10: Click on an attribute from the list and see that the information is displayed on the right side of the screen
        Step 11: Click on another attribute from the list and see that the information is displayed on the right side of the screen
        Step 12: Double click again on the channel name. The list with the attributes should disappear.
        Step 13: Double click again on the device name. The list with the channels should disappear.

Side effects:
        - Each selected element is highlighted
        - The arrow on the left of the element name changes its direction when the element is selected
        - The Navigation Tree will display a scroll bar if the elements do not fit in the window

Test 3: The Filter Bar from the debugger works correctly
========================================================
Description: This test checks if the Filter Bar from the debugger works correctly.
Preconditions:
    - Scopy is installed on the system
    - A device is connected to Scopy
    - The device is IIO compatible
    - The device is supported by the debugger plugin
    - The debugger plugin is installed on the system

Tested on:
        - Platform: Ubuntu 20.04, Windows 10, MacOS 14.5
        - Scopy version: 2.0.0
        - Device: ADALM-PLUTO

Steps:
        Step 1: Connect a device to Scopy
        Step 2: Connect to the device and see that the Debugger plugin appears on the left side of the screen (the tool menu)
        Step 3: Ensure that on the top left side of the Debugger tool there is a filter bar. The filter bar should have a placeholder text that says "Type to filter"
        Step 4: Click on the filter bar and type a string that is not present in the navigation tree. The navigation tree should be empty
        Step 5: Clear the filter bar and see that the navigation tree is populated again. No element should be missing and the context element should be the first one and the devices should be displayed below it (it is opened).
        Step 6: Click on the filter bar and type a string that is present in the navigation tree. The navigation tree should display only the elements that contain the string
        Step 7: Repeat step 6 with another string that is present in the navigation tree. The navigation tree should display only the elements that contain the new string
        Step 5: Repeat step 5. The navigation tree should be populated again.

Side effects:
        - Each time the user clicks on the filter bar, the placeholder text should disappear
        - Each time the user starts typing in the filter bar, a dropdown list should appear with the elements that contain the string. When the user clicks on an element from the dropdown list, the navigation tree should display only the selected element and its children.

Test 4: The title bar updates when selecting different elements from the navigation tree
=======================================================================================
Description: This test checks if the title bar updates when selecting different elements from the navigation tree.
Preconditions:
    - Scopy is installed on the system
    - A device is connected to Scopy
    - The device is IIO compatible
    - The device is supported by the debugger plugin
    - The debugger plugin is installed on the system

Tested on:
        - Platform: Ubuntu 20.04, Windows 10, MacOS 14.5
        - Scopy version: 2.0.0
        - Device: ADALM-PLUTO

Steps:
        Step 1: Connect a device to Scopy
        Step 2: Connect to the device and see that the Debugger plugin appears on the left side of the screen (the tool menu)
        Step 3: Ensure that on the top right side of the Debugger tool there is a title bar. The title bar should have a square button with the name of the first element from the navigation tree. After this button, there should be a small green circle with a plus sign.
        Step 4: Click on any element from the navigation tree and see that the title bar updates with the name of the selected element. The name of the element should appear after the first element name and separated in a new button.
        Step 5: Click on another element from the navigation tree and see that the title bar updates with the name of the selected element. The name of the element should appear after the first element name and separated in a new button.
        Step 6: Open a device from the navigation panel, and then a channel from this device and select an attribute from the channel. The title bar should display the 4 names in order: The context name, the device name, the channel name and the attribute name. Each name should be separated a different button.
        Step 7: Click on any elemet from the title bar. The title bar should remove all button after it and the navigation tree should highlight the selected element.
        Step 8. Click the green circle with the plus sign. The circle should change its color to a red x. (The current element should be added to the watch list, but this behavior is tested in a different test).

Side effects:
        - The title bar should never be empty.

Test 5: The watch list from the debugger works correctly
========================================================
Description: This test checks if the watch list from the debugger works correctly.

Preconditions:
    - Scopy is installed on the system
    - A device is connected to Scopy
    - The device is IIO compatible
    - The device is supported by the debugger plugin
    - The debugger plugin is installed on the system

Tested on:
        - Platform: Ubuntu 20.04, Windows 10, MacOS 14.5
        - Scopy version: 2.0.0
        - Device: ADALM-PLUTO

Steps:
        Step 1: Connect a device to Scopy
        Step 2: Connect to the device and see that the Debugger plugin appears on the left side of the screen (the tool menu)
        Step 3: Ensure that at the bottom of the Debugger tool there is a watch list. The watch list should be a tabel with 4 columns: Name, Value, Type and Path. The table should have a header with the column names. There should also be another column with no name where the 'X' buttons for each row are displayed.
        Step 4: Click on an element from the navigation tree, the title bar should update with the name of the selected element and a green circle with a plus sign should exist on the right side of the title bar.
        Step 5: Click on the green circle with the plus sign. The circle should change its color to a red x and the selected element should be added to the watch list. The element should be added in the first row of the table and the columns should be filled with the information from the selected element.
        Step 6: Repeat step 4 and 5 with another element from the navigation tree. The new element should be added to the watch list in the second row of the table and the columns should be filled with the information from the selected element.
        Step 8: Modify the value of an element from the watch list. The value should be updated in the table and in the panel above it (the details view).
        Step 9: Click on the red x from the watch list. The selected element should be removed from the watch list.
        Step 10: Click on the x from the last column of the watch list. The selected element should be removed from the watch list.
        Step 11: Repeat steps 4 to 10 with other elements from the navigation tree. Nothing should crash and the watch list should be updated correctly.
        Step 12: Add a few elements in the wathch list. Click any other element from the watch list. The navigation bar, title bar and information from the details view should be updated with the information from the selected element.


Test 6: The details view from the debugger work correctly
========================================================
Description: This test checks if the details view from the debugger work correctly.

Preconditions:
    - Scopy is installed on the system
    - iio_info is installed on the system
    - A device is connected to Scopy
    - The device is IIO compatible
    - The device is supported by the debugger plugin
    - The debugger plugin is installed on the system

Tested on:
        - Platform: Ubuntu 20.04, Windows 10, MacOS 14.5
        - Scopy version: 2.0.0
        - Device: ADALM-PLUTO

Steps:
        Step 1: Connect a device to Scopy
        Step 2: Connect to the device and see that the Debugger plugin appears on the left side of the screen (the tool menu)
        Step 3: Ensure that on the right side of the Debugger tool there is a details view. The details view should have a title bar with the name of the selected element from the navigation tree. At first the details view should display the context attributes.
        Step 4: Test the GUI View.
        Step 4.1: Click on any element from the navigation tree. The details view should display the information from the selected element.
        Step 4.2: Click on any attribute from the details view. Change the data, press enter or click outside the attribute. Under the attribute value, a progress bar should appear and start filling. After the progress bar is filled, the progress bar should turn green for a few seconds and the attribute value should be updated. To verify that the value was updated, run an external program like iio_info and check that the value is the same.
        Step 4.3: Repeat step 4.2 with the following 3 types of attributes: a text box (a box where the user can type any string), a combo box (a box where the user can select from a list of options) and a range box (a box where the user can type a number or use the arrows (or the +/- sign) to increase or decrease the value and/or write the desired value).
        Step 4.4: While selectig different elements from the navigation tree, lower part of the GUI View, the General Info section should update with the information from the selected element (information such as weather it is a hardware monitor, a trigger, has a trigger attached, if the channel is a scan element, input or output, enabled or disabled). All these informations should be present within the elements of the ADALM-PLUTO device.
        Step 5: Test the IIO View.
        Step 5.1: Click on the IIOView button, it should be next to the GUI View, under the title. The IIO View should display a snippet similar to the iio_info command. The snippet should contain the information from the selected element from the navigation tree. To check this, run the iio_info command in a terminal and compare the information from the terminal with the information from the IIO View.
        Step 5.2: Repeat step 5.1 with different elements from the navigation tree. The IIO View should update with the information from the selected element.

Test 7: The read all button correctly reads all visible attributes
==================================================================
Description: This test checks if the read all button correctly reads all visible attributes.

Preconditions:
    - Scopy is installed on the system
    - iio_info is installed on the system
    - iio_attr is installed on the system
    - A device is connected to Scopy
    - The device is IIO compatible
    - The device is supported by the debugger plugin
    - The debugger plugin is installed on the system

Tested on:
        - Platform: Ubuntu 20.04, Windows 10, MacOS 14.5
        - Scopy version: 2.0.0
        - Device: ADALM-PLUTO

Steps:
        Step 1: Connect a device to Scopy
        Step 2: Connect to the device and see that the Debugger plugin appears on the left side of the screen (the tool menu)
        Step 3: Add a few elements in the watch list. The added elements should be device attributes or channel attributes.
        Step 4: Select any element from the navigation tree (preferable one with a few attributes)
        Step 5: Click the read all button.
        Step 6: In a terminal, run the iio_attr command and change the value of one of the attributes from the selected element or the watch list.
        Step 7: Click the read all button again. The value of the attribute should be updated in the details view and the watch list (optionally, based on weather the changed element is also in the watch list).
        Step 8: Also check the IIO View. The value of the attribute should be updated in the IIO View as well.

Test 8: The log window from the debugger correctly displays the operations
=========================================================================
Description: This test checks if the log window from the debugger correctly displays the operations.

Preconditions:
    - Scopy is installed on the system
    - A device is connected to Scopy
    - The device is IIO compatible
    - The device is supported by the debugger plugin
    - The debugger plugin is installed on the system

Tested on:
        - Platform: Ubuntu 20.04, Windows 10, MacOS 14.5
        - Scopy version: 2.0.0
        - Device: ADALM-PLUTO

Steps:
        Step 1: Connect a device to Scopy
        Step 2: Connect to the device and see that the Debugger plugin appears on the left side of the screen (the tool menu)
        Step 3: At the bottom of the debugger instrument, next to the 'IIO Attributes' button, there should be a 'Log' button. Click on the 'Log' button.
        Step 4: The current window should change to the log window. It should have the elements that were currently read.
        Step 5: Select back the 'IIO Attributes' button. The log window should disappear.
        Step 6: (Correct attribute change) Modify an attribute from the detais view (e.g. on the ADALM-PLUTO device, select the ad9361-phy device and the voltage0 channel and modify the gain_control_mode attribute to hybrid).
        Step 7: (Incorrect attribute change) Select the adm1177-iio device from the navigation tree, the voltage0 channel and modify the raw attribute (write whatever value you want, it should be read-only anyways). After the progress bar is filled, it should turn red and the value should be read again.
        Step 8: Click on the 'Log' button. The log window should appear and display the operations from steps 6 and 7. The operations should be displayed in the following format: [timestamp] [operation] [status] [attribute path] [old value (if write operation)] [new value]. The timestamp should be the current time, the operation should be 'W' (write) or 'R' (read), the status should be SUCCESS or FAILURE and the error code, the attribute path should be the name of the modified attribute title, separated by slashes, the old value should be the value before the modification, the new value should be the value after the modification. If the operation is a read operation, the old value should be empty (not exist). Step 6 should be a successful write operation and step 7 should be a failed write operation. After each write operation, a read operation is automatically performed. The read operation should be displayed in the log window as well.

