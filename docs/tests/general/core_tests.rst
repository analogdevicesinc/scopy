.. _general_tests:

Core Tests
==========

**Homepage, scanned devices**
-----------------------------

**User guide:** :ref:`Scopy Overview <user_guide>`.

Test 1: The homepage displays correctly when running the application.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.HP.RUN

**Description:** This test checks if the homepage is displayed correctly.

**Preconditions:**
    - Scopy is installed on the system.
    - OS: any.

**Steps:**
    1. Open Scopy.
    2. Observe the application's behavior.
        - **Expected result:** The application launches without errors and the home page is displayed.

**Result:** PASS/FAIL

Test 2: Verifying the "Add device" (+) button.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.HP.ADD

**Description:** This test checks if the **Add device** button works correctly.

**Preconditions:**
    - Scopy is installed on the system.
    - OS: any.

**Steps:**
    1. Open Scopy.
    2. Navigate to the application's home page.
    3. Click on the **Add device** button.
    4. Observe the application's behavior.
        - **Expected result:** The page for adding a new device is displayed in the info page section.

**Result:** PASS/FAIL

.. _tst-hp-auto-scan-on:

Test 3: Verifying the auto-scan feature (ON).
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.HP.AUTO_SCAN_ON

**Description:** This test checks if the auto-scan feature detects and lists all available devices correctly.

**Preconditions:**
    - Scopy is installed on the system.
    - The "Regularly scan for new devices" preference is enabled in the preferences page.
    - OS: any.

**Steps:**
    1. Open Scopy.
    2. Ensure the **Scan** switch is enabled in the device browser section.
    3. Connect an **ADALM2000** device to the system by USB.
    4. Observe the application's behavior.
        - **Expected result:** After a short period, the device appears in the device browser section. 

**Result:** PASS/FAIL

.. _tst-hp-auto-scan-off:

Test 4: Verifying the auto-scan feature (OFF).
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.HP.AUTO_SCAN_OFF

**Description:** This test checks the behavior of the application when the auto-scan is turned off.

**Preconditions:**
    - Scopy is installed on the system.
    - The "Regularly scan for new devices" preference is enabled in the preferences page.
    - OS: any.

**Steps:**
    1. Open Scopy.
    2. Ensure the **Scan** switch is disabled in the device browser section.
    3. Connect an **ADALM2000** device to the system by USB.
    4. Observe the application's behavior.
        - **Expected result:** The device should not be visible in the device browser. 

**Result:** PASS/FAIL

.. _tst-hp-manual-scan:

Test 5: Verifying the manual scan feature.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.HP.MANUAL_SCAN

**Description:** This test checks if the manual scan feature detects and lists all available devices correctly.

**Preconditions:**
    - Scopy is installed on the system.
    - The "Regularly scan for new devices" preference is disabled in the preferences page.
    - OS: any.

**Steps:**
    1. Open Scopy.
    2. Connect an **ADALM2000** device to the system by USB.
    3. Wait a few seconds to ensure that the connection has been successfully established.
    4. Click on the **Scan** button.
    5. Observe the application's behavior.
        - **Expected result:** The device appears in the device browser section.

**Result:** PASS/FAIL

Test 6: Navigation using the buttons in the info page.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.HP.NAV

**Description:** This test checks if the navigation buttons work correctly.

**Test prerequisites:**
    - :ref:`TST.HP.AUTO_SCAN_ON <tst-hp-auto-scan-on>`.

**Preconditions:**
    - Scopy is installed on the system.
    - OS: any.

**Steps:**
    1. Open Scopy.
    2. Ensure the **Scan** switch is enabled in the device browser section.
    3. Connect an **ADALM2000** device to the system by USB.
    4. Wait until the device appears in device browser.
    5. Ensure that you are on "Welcome to Scopy" page.
    6. Click on the right arrow button.
        - **Expected result:** The info page changes from the 'Welcome to Scopy' page to the add page and in device browser the related button is highlighted.
    7. Click on the left arrow button.
        - **Expected result:** The "Welcome to Scopy" page is displayed and the related button is highlighted.
    8. Click on the left arrow button.
        - **Expected result:** The device's page is displayed and the device icon is highlighted. 

**Result:** PASS/FAIL 

Test 7: Auto-scan behavior when the device is unplugged.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.HP.UNPLUGGED

**Description:** This test checks if the auto-scan feature detects and lists all available devices correctly.

**Test prerequisites:**
    - :ref:`TST.HP.AUTO_SCAN_ON <tst-hp-auto-scan-on>`.

**Preconditions:**
    - Scopy is installed on the system.
    - OS: any.

**Steps:**
    1. Open Scopy.
    2. Ensure the **Scan** switch is enabled in the device browser section.
    3. Connect an **ADALM2000** device to the system by USB.
    4. Wait until the device appears in device browser.
    5. Unplug the device.
    6. Observe the application's behavior.
        - **Expected result:** The device is removed from device browser.

**Result:** PASS/FAIL

**Add device**
--------------

**User guide:** :ref:`Scopy Overview <user_guide>`.

**Setup:**
    - M2k.Emu:
        - Open Scopy.
        - Start the iio-emu process using "adalm2000" in the dropdown and "ip:127.0.0.1" as URI.

Test 1: Verifying the device scanning functionality.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.ADD.SCAN_ALL

**Description:** This test checks that the device scan correctly detects and lists available devices when initiated.

**Preconditions:**
    - Scopy is installed on the system.
    - OS: any.

**Steps:**
    1. Connect an **ADALM2000** device to the system by USB.
    2. Open Scopy.
    3. Click on **Add device** (+) button.
    4. Click on the refresh button on the right located in the **Add page** scan section.
    5. Wait until the refresh animation stops.
        - **Expected result:** In the "Context" dropdown, all available devices are listed, each item containing the device name/description and URI. The **ADALM2000** device [usb] must appear in the list. 

**Result:** PASS/FAIL

Test 2: Verifying the local device scanning functionality.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.ADD.SCAN_LOCAL

**Description:** This test checks that the device scan correctly detects and lists available local devices when initiated.

**Preconditions:**
    - Scopy is installed on the system.
    - There is at least one IIO device connected to the system that has local backend.
    - OS: any.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **local** checkbox located in the **Add page** scan section.
    4. Wait until the refresh animation stops.
        - **Expected result:** In the "Context" dropdown, all available local devices are listed, each item containing the device name/description and URI. If there is no available device the "No scanned devices available!" message is displayed.

**Result:** PASS/FAIL

Test 3: Verifying the ip device scanning functionality.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.ADD.SCAN_IP

**Description:** This test checks that the device scan correctly detects and lists available ip devices when initiated.

**Preconditions:**
    - Scopy is installed on the system.
    - There is at least one IIO device connected to the system that has ip backend.
    - OS: any.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **ip** checkbox located in the **Add page** scan section.
    4. Wait until the refresh animation stops.
        - **Expected result:** In the "Context" dropdown, all available ip devices are listed, each item containing the device name/description and URI. If there is no available device the "No scanned devices available!" message is displayed.

**Result:** PASS/FAIL

Test 4: Verifying the usb device scanning functionality.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.ADD.SCAN_USB

**Description:** This test checks that the device scan correctly detects and lists available usb devices when initiated.

**Preconditions:**
    - Scopy is installed on the system.
    - OS: any.

**Steps:**
    1. Connect an **ADALM2000** device to the system by USB.
    2. Open Scopy.
    3. Click on **Add device** (+) button.
    4. Click on **usb** checkbox located in the **Add page** scan section.
    5. Wait until the refresh animation stops.
        - **Expected result:** In the "Context" dropdown, all available usb devices are listed, each item containing the device name/description and URI. The **ADALM2000** device must appear in the list. 

**Result:** PASS/FAIL

Test 5: Verifying the serial device scanning functionality.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.ADD.SCAN_SERIAL

**Description:** This test checks that the device scan correctly detects and lists available serial devices when initiated.

**Preconditions:**
    - Scopy is installed on the system.
    - There is at least one IIO device connected to the system that has serial backend.
    - OS: any.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on the refresh button on the right located in the **Add page** serial port section.
    4. Wait until the refresh animation stops.
        - **Expected result:** In the "Name" dropdown, all available serial devices are listed, each item containing the device name/description and URI. If there is no available device the "No scanned devices available!" message is displayed.

**Result:** PASS/FAIL

Test 6: Scanning with no device.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.ADD.SCAN_EMPTY

**Description:** This test checks whether an appropriate message is displayed when no device is found.

**Preconditions:**
    - Scopy is installed on the system.
    - OS: any.

**Steps:**
    1. Run Scopy.
    2. Make sure that no devices using USB backends are connected to the system.
    3. Click on **Add device** (+) button.
    4. Click on **usb** checkbox located in the **Add page** scan section.
    5. Wait until the refresh animation stops.
        - **Expected result:** The "No scanned devices available!" message is displayed.

**Result:** PASS/FAIL

.. _tst-add-verify:

Test 7: Device validation.
^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.ADD.VERIFY

**Description:** This test verifies that the device validation works correctly.

**Test prerequisites:**
    - :ref:`TST.EMU.EN <tst-emu-en>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use "M2k.Emu" setup.
    - OS: any.

**Steps:**
    1. Click on **Add device** (+) button.
    2. Introduce the "ip:127.0.0.1" URI.
    3. Click on **Verify** button.
        - **Expected result:** A loading animation starts and after a short time a page with device details and compatible plugins is displayed. 

**Result:** PASS/FAIL

Test 8: Verify the "BACK" button from compatible plugin page.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.ADD.PLUGINS_BACK

**Description:** This test checks if the **BACK** button works correctly.

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use "M2k.Emu" setup.
    - OS: any.

**Steps:**
    1. Click on **Add device** (+) button.
    2. Introduce the "ip:127.0.0.1" URI.
    3. Click on **Verify** button.
    4. Click on **BACK** button.
        - **Expected result:** The verify page is displayed.

**Result:** PASS/FAIL

.. _tst-add-plugins-add:

Test 9: Verify the "ADD DEVICE" button from compatible plugin page (single plugin).
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.ADD.PLUGINS_ADD_SINGLE

**Description:** This test checks if the **ADD DEVICE** button works correctly. Only the main plugin of the device is used.

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use "M2k.Emu" setup.
    - OS: any.

**Steps:**
    1. Click on **Add device** (+) button.
    2. Introduce the "ip:127.0.0.1" URI.
    3. Click on **Verify** button.
        - **Expected result:** Only the M2kPlugin should be selected in the compatible plugins section.
    4. Click on **ADD DEVICE** button.
        - **Expected result:** The device is added to the device browser, the device's info page is displayed, and the m2k tools are added to the tool menu. 

**Result:** PASS/FAIL

Test 10: Verify the "ADD DEVICE" button from compatible plugin page (multiple plugins).
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.ADD.PLUGINS_ADD_MULTIPLE

**Description:** This test checks if the **ADD DEVICE** button works correctly. Multiple plugins are used.

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use "M2k.Emu" setup.
    - OS: any.

**Steps:**
    1. Click on **Add device** (+) button.
    2. Introduce the "ip:127.0.0.1" URI.
    3. Click on **Verify** button.
        - **Expected result:** Only the M2kPlugin should be selected in the compatible plugins section.
    4. Select the DebuggerPlugin.
    5. Click on **ADD DEVICE** button.
        - **Expected result:** The device is added to the device browser, the device's info page is displayed, and all tools, including the debugger, are added to the tool menu.

**Result:** PASS/FAIL

Test 11: Verify the "ADD DEVICE" button from compatible plugin page with no plugin selected.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.ADD.PLUGINS_EMPTY

**Description:** This test checks if the **ADD DEVICE** button works correctly.

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use "M2k.Emu" setup.
    - OS: any.

**Steps:**
    1. Click on **Add device** (+) button.
    2. Introduce the "ip:127.0.0.1" URI.
    3. Click on **Verify** button.
        - **Expected result:** Only the M2kPlugin should be selected in the compatible plugins section.
    4. Uncheck the M2kPlugin.
    5. Click on **ADD DEVICE** button.
        - **Expected result:** A NO_PLUGIN device is added in device browser and in tool menu.

**Result:** PASS/FAIL

**Connection flow**
-------------------

**User guide:** :ref:`Scopy Overview <user_guide>`.

**Setup:**
    - M2k.Emu:
        - Open Scopy.
        - Start the iio-emu process using "adalm2000" in the dropdown and "ip:127.0.0.1" as URI.
        - Add the device in device browser.
    - M2k.Usb:
        - Open Scopy.
        - Connect an **ADALM2000** device to the system by USB.
        - Add the device in device browser.

.. _tst-conn-succ:

Test 1: Verify the connection flow.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.CONN.SUCC

**Description:** This test checks if the **Connect** button works correctly.

**Test prerequisites:** 
    - :ref:`TST.ADD.PLUGINS_ADD_SINGLE <tst-add-plugins-add>` 

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed in the system.
    - Use "M2k.Emu" setup.
    - OS: any.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
        - **Expected result:** The **Disconnect** button appears, a status LED is displayed under the icon in the device browser, and a progress bar appears in the application's status bar. After a while, the connection is established, the status LED turns green, and the tools in the menu become accessible. 

**Result:** PASS/FAIL

Test 2: Unplug the device while connecting (AUTO-SCAN).
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.CONN.UNPLUG_AUTO

**Description:** This test verifies Scopy's behavior when the device is unplugged during the connection process.

**Test prerequisites:** 
    - :ref:`TST.HP.AUTO_SCAN_ON <tst-hp-auto-scan-on>`.

**Preconditions:**
    - Scopy is installed on the system.
    - The "Regularly scan for new devices" preference is enabled in the preferences page.
    - Use "M2k.Usb" setup.
    - OS: any.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
    3. Unplug the device while connecting. 
        - **Expected result:** The device is removed from device browser. 

**Result:** PASS/FAIL

Test 3: Unplug the device while connecting (MANUAL-SCAN).
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.CONN.UNPLUG_AUTO

**Description:** This test verifies Scopy's behavior when the device is unplugged during the connection process.

**Test prerequisites:**
    - :ref:`TST.HP.MANUAL_SCAN <tst-hp-manual-scan>`.

**Preconditions:**
    - Scopy is installed on the system.
    - The "Regularly scan for new devices" preference is disabled in the preferences page.
    - Use "M2k.Usb" setup.
    - OS: any.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
    3. Unplug the device while connecting. 
        - **Expected result:** A warning icon appears in the top-right corner of the device in the device browser, and when hovered over, the message "The device is not available! Verify the connection!" is displayed.

**Result:** PASS/FAIL

Test 4: Unplug the device before connecting.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.CONN.UNPLUG_BEFORE

**Description:** This test verifies Scopy's behavior when the device is unplugged before the connection process.

**Test prerequisites:**
    - :ref:`TST.HP.MANUAL_SCAN <tst-hp-manual-scan>`.

**Preconditions:**
    - Scopy is installed on the system.
    - The "Regularly scan for new devices" preference is disabled in the preferences page.
    - Use "M2k.Usb" setup.
    - OS: any.

**Steps:**
    1. Click on the device icon from device browser.
    2. Unplug the device. 
    3. Click on **Connect** button.
        - **Expected result:** The connection process starts but does not complete. A warning icon appears in the top-right corner.

**Result:** PASS/FAIL

Test 5: Disconnecting a device.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.CONN.DISCONN

**Description:** This test checks if the **Disconnect** button works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed in the system.
    - Use "M2k.Emu" setup.
    - OS: any.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
    3. Wait until the connection is done .
    4. Click on **Disconnect** button.
        - **Expected result:** The device is disconnected, the **Connect** button appears, the device remains in the device browser, and the tools in the menu are no longer accessible.

**Result:** PASS/FAIL

**EMU**
-------

**User guide:** :ref:`Scopy Overview <user_guide>`.

.. _tst-emu-en:

Test 1: Enable the emulator.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.EMU.EN

**Description:** This test verifies if starting the emulator in Scopy works correctly.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "adalm2000".
    5. Click on **Enable** button.
        - **Expected result:** The IIO device tab is selected, and the URI "ip:127.0.0.1" is entered in the corresponding section. 

**Result:** PASS/FAIL

Test 2: EMU is not installed.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.EMU.NOT

**Description:** This test verifies if Scopy handles the situation correctly when the IIO emulator is not found.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is not installed on the system.
    - Modify the "iio_emu_path" preference from "preferences.ini" file with an invalid path (you can access the file through the **Open** button in the general preferences).
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
        - **Expected result:** All the components are disabled and the message "Can't find iio-emu in the system!" is displayed. 

**Result:** PASS/FAIL

Test 3: Disable the emulator.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.EMU.DIS

**Description:** This test verifies if stopping the emulator in Scopy works correctly.

**Test prerequisites:**
    - :ref:`TST.EMU.EN <tst-emu-en>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "adalm2000".
    5. Click on **Enable** button.
    6. Click on **Emulator** tab.
    7. Click on **Disable** button.
        - **Expected result:** The fields in this tab become editable, and the **Enable** button appears. 

**Result:** PASS/FAIL

Test 4: Disable the emulator while the emulated device is connected.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.EMU.DIS_CONN

**Description:** The test verifies if Scopy handles the situation correctly when an emulated device is connected and the "iio-emu" process is stopped.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Open Scopy.
    2. Start the iio-emu process using "adalm2000" in the dropdown and "ip:127.0.0.1" as URI.
    3. Add the device in device browser.
    4. Connect to emu device.
    5. Click on **Add device** (+) button.
    6. Click on **Emulator** tab.
    7. Click on **Disable** button.
        - **Expected result:** The fields in this tab become editable, and the **Enable** button appears. After a few seconds, the device is disconnected.

**Result:** PASS/FAIL

Test 5: Verify adalm2000 emu device.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.EMU.EN_ADALM2000

**Description:** This test verifies if starting the emulator in Scopy works correctly (using adalm2000 device).

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "adalm2000".
    5. Click on **Enable** button.
    6. Click on **Verify** button.
        - **Expected result:** The compatible plugins page opens. The compatible plugins are: M2kPlugin (checked), ADCPlugin, DACPlugin, DataLoggerPlugin, DebuggerPlugin, RegmapPlugin.

**Result:** PASS/FAIL

Test 6: Verify pluto emu device.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.EMU.EN_PLUTO

**Description:** This test verifies if starting the emulator in Scopy works correctly (using pluto device).

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "pluto".
    5. Click on **Enable** button.
    6. Click on **Verify** button.
        - **Expected result:** The compatible plugins page opens. The compatible plugins are: ADCPlugin (checked), DACPlugin (checked), DataLoggerPlugin (checked), DebuggerPlugin (checked), RegmapPlugin (checked). 

**Result:** PASS/FAIL

Test 7: Verify pqm emu device.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.EMU.EN_PQM

**Description:** This test verifies if starting the emulator in Scopy works correctly (using pqm device).

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "pqm".
    5. Click on **Enable** button.
    6. Click on **Verify** button.
        - **Expected result:** The compatible plugins page opens. The compatible plugins are: PQMPlugin (checked), ADCPlugin, DACPlugin, DataLoggerPlugin, DebuggerPlugin.


**Result:** PASS/FAIL

Test 8: Verify swiot_config emu device.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.EMU.EN_SWIOT_CONFIG

**Description:** This test verifies if starting the emulator in Scopy works correctly (using swiot_config device).

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "swiot_config".
    5. Click on **Enable** button.
    6. Click on **Verify** button.
        - **Expected result:** The compatible plugins page opens. The compatible plugins are: SWIOTPlugin (checked), DataLoggerPlugin, DebuggerPlugin.

**Result:** PASS/FAIL

Test 9: Verify swiot_runtime emu device.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.EMU.EN_SWIOT_RUNTIME

**Description:** This test verifies if starting the emulator in Scopy works correctly (using swiot_runtime device).

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "swiot_runtime".
    5. Click on **Enable** button.
    6. Click on **Verify** button.
        - **Expected result:** The compatible plugins page opens. The compatible plugins are: SWIOTPlugin (checked), ADCPlugin, DataLoggerPlugin, DebuggerPlugin, RegmapPlugin.

**Result:** PASS/FAIL

Test 10: Verify generic emu device.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.EMU.EN_GENERIC

**Description:** This test verifies if starting the emulator in Scopy works correctly (using generic device).

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "generic".
    5. Click on **Enable** button.
    6. Click on **Verify** button.
        - **Expected result:** The message **"ip:127.0.0.1" not a valid context!** is displayed. 

**Result:** PASS/FAIL

**Device interaction**
----------------------

**User guide:** :ref:`Scopy Overview <user_guide>`.

**Setup:**
    - M2k.Emu:
        - Open Scopy.
        - Start the iio-emu process using "adalm2000" in the dropdown and "ip:127.0.0.1" as URI.
        - Add the device in device browser.
    - M2k.Usb:
        - Open Scopy.
        - Connect an **ADALM2000** device to the system by USB.
        - Add the device in device browser.

Test 1: Verifying the device button.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.HP.DEV_BTN

**Description:** To verify that clicking on the device button triggers the expected action: displaying device details and device tools.

**Test prerequisites:**
    - :ref:`TST.ADD.PLUGINS_ADD_SINGLE <tst-add-plugins-add>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use "M2k.Emu" setup.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Click on the device icon.
    2. Observe the application's behavior.
        - **Expected result:** The device details page is displayed in the info page section, and in the tool menu (on the left), the device is added with a list of available tools (in this form, the tools cannot be accessed). 

**Result:** PASS/FAIL

Test 2: Remove a device from device browser.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.DEV.FORGET

**Description:** This test checks if the "Forget" (x) button work correctly.

**Test prerequisites:**
    - :ref:`TST.ADD.PLUGINS_ADD_SINGLE <tst-add-plugins-add>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use "M2k.Emu" setup.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Click the **Forget** button located at the bottom-right of the device icon.
        - **Expected result:** The device is removed from device browser. 

**Result:** PASS/FAIL

Test 3: Remove a device from device browser while it is connected.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.DEV.FORGET_CONN

**Description:** This test verifies whether Scopy properly handles the removal of a connected device.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use "M2k.Emu" setup.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
    3. Click the **Forget** button located at the bottom-right of the device icon.
        - **Expected result:** Initially, the device is disconnected and then successfully removed from the device browser.

**Result:** PASS/FAIL

Test 4: Edit the name of the device.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.DEV.EDIT_NAME

**Description:** This test checks if the **Edit pen** button work correctly.

**Test prerequisites:**
    - :ref:`TST.ADD.PLUGINS_ADD_SINGLE <tst-add-plugins-add>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use "M2k.Emu" setup.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Place the cursor in the top-right corner of the device icon in the device browser.
    2. After the pen icon appears, click on it.
    3. Delete "M2k".
    4. Write "M3k".
    5. Press enter. 

Expected result: The device name changes both in the device browser and in the tool menu.

**Result:** PASS/FAIL

Test 5: Select a tool. 
^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.DEV.SELECT_TOOL

**Description:** This test checks if the tool infrastructure works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use "M2k.Emu" setup.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
    3. Click on "Oscilloscope" tool.
        - **Expected result:** The "Oscilloscope" tool is displayed and the tool menu item is highlighted.
    4. Click on "Home" menu entry.
        - **Expected result:** The device page is displayed and the home menu item is highlighted.

**Result:** PASS/FAIL

.. _tst-dev-detach-tool:

Test 6: Detach a tool.
^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.DEV.DETACH_TOOL

**Description:** This test checks if the tool detachment mechanism is working correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use "M2k.Emu" setup.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
    3. Double click on "Oscilloscope" tool.
        - **Expected result:** The "Oscilloscope" tool is integrated into a separate window from the application. The window title should be: "Scopy-M2kPlugin-Oscilloscope-ip:127.0.0.1".
    4. Possible steps.
        1. Close the "Oscilloscope" window.
            - **Expected result:** The "Oscilloscope" tool is reintegrated into the main window. 
        2. Close the  application.
            - **Expected result:** Scopy finished successfully. 

**Result:** PASS/FAIL

Test 7: Reopen the app with a detached tool.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.DEV.DETACH_TOOL_RE

**Description:** This test checks whether detached tools retain their state upon reconnection.

**Test prerequisites:**
    - :ref:`TST.DEV.DETACH_TOOL <tst-dev-detach-tool>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use "M2k.Emu" setup.
    - The application was previously closed with a detached tool (Oscilloscope).
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
        - **Expected result:** The oscilloscope tool is detached.

**Result:** PASS/FAIL

Test 8: Collapse device tools in the tool menu.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.DEV.COLLAPSE

**Description:** This test verifies if the collapse feature works properly in the tool menu.

**Test prerequisites:**
    - :ref:`TST.ADD.PLUGINS_ADD_SINGLE <tst-add-plugins-add>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use "M2k.Emu" setup.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on the device header from tool menu.
        - **Expected result:** The tools collapse.
    3. Repeat 2.
        - **Expected result:** The tools expand.

**Result:** PASS/FAIL

.. _tst-dev-multi-conn:

Test 9: Connecting to multiple devices.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.DEV.MULTI_CONN

**Description:** This test checks if connecting to multiple devices is working properly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - The "Connect to multiple devices" preference is enabled in the preferences page.
    - Use "M2k.Emu" and "M2k.Usb" setups.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Click on the emu device icon from device browser.
    2. Click on **Connect** button.
        - **Expected result:** The connection is established successfully.
    3. Click on the usb device icon from device browser.
    4. Click on **Connect** button.
        - **Expected result:** The connection is established successfully, both device icons have a green status LED, both devices are entered into the tool menu (the device header contains the device name and uri), and the tools of each device can be accessed. If the menu contains too many entries, then the scroll bar will become visible.

**Result:** PASS/FAIL

Test 10: Resource manager with multiple devices.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.DEV.RM_MULTI

**Description:** This test verifies that the resource manager is being used properly.

**Test prerequisites:**
    - :ref:`TST.DEV.MULTI_CONN <tst-dev-multi-conn>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - The "Connect to multiple devices" preference is enabled in the preferences page.
    - Use "M2k.Emu" and "M2k.Usb" setups.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Click on the emu device icon from device browser.
    2. Click on **Connect** button.
    3. Click on the usb device icon from device browser.
    4. Click on **Connect** button.
    5. Click on the "Oscilloscope" run button of the emu device in the tool menu.
        - **Expected result:** The button changes from stop button to run button (green color).
    6. Click on the "Spectrum Analyzer" run button of the usb device in the tool menu.
        - **Expected result:** The button changes from stop button to run button, and the oscilloscope button (of the emu device) remains the same.

**Result:** PASS/FAIL

Test 11: Devices order in device browser.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.DEV.ADD_ORDER

**Description:** This test verifies that each newly added device is finally entered into the device browser.

**Test prerequisites:**
    - :ref:`TST.ADD.PLUGINS_ADD_SINGLE <tst-add-plugins-add>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - The "Connect to multiple devices" preference is enabled in the preferences page.
    - Use "M2k.Emu" and "M2k.Usb" setups.
    - OS: windows/linux_x86-64/linux_arm64/linux_arm32/macos.

**Steps:**
    1. Add the emu device.
    2. Add the usb device.
        - **Expected result:** The usb device is on the right side of emu device. 

**Result:** PASS/FAIL