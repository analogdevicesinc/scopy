.. _dev_tests:

Device interaction
==================

.. note::

    User guide: :ref:`Scopy Overview <user_guide>`.

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

Setup environment:
------------------

.. _m2k-emu-dev:

**M2k.Emu:**
    - Open Scopy.
    - Start the iio-emu process using "adalm2000" in the dropdown and 
      "ip:127.0.0.1" as URI.
    - Add the device in device browser.

.. _m2k-usb-dev:

**M2k.Usb:**
    - Open Scopy.
    - Connect an **ADALM2000** device to the system by USB.
    - Add the device in device browser.

Test 1: Verifying the device button.
------------------------------------

**UID:** TST.HP.DEV_BTN

**RBP:** P0

**Description:** To verify that clicking on the device button triggers the
expected action: displaying device details and device tools.

**Test prerequisites:**
    - :ref:`TST.ADD.PLUGINS_ADD_SINGLE <tst-add-plugins-add>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`M2k.Emu <m2k-emu-dev>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the device icon.
    2. Observe the application's behavior.
        - **Expected result:** The device details page is displayed in the 
          info page section, and in the tool menu (on the left), the device 
          is added with a list of available tools (in this form, the tools 
          cannot be accessed). 
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


Test 2: Remove a device from device browser.
--------------------------------------------

**UID:** TST.DEV.FORGET

**RBP:** P0

**Description:** This test checks if the "Forget" (x) button work correctly.

**Test prerequisites:**
    - :ref:`TST.ADD.PLUGINS_ADD_SINGLE <tst-add-plugins-add>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`M2k.Emu <m2k-emu-dev>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click the **Forget** button located at the bottom-right of the device icon.
        - **Expected result:** The device is removed from device browser. 
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


Test 3: Remove a device from device browser while it is connected.
------------------------------------------------------------------

**UID:** TST.DEV.FORGET_CONN

**RBP:** P3

**Description:** This test verifies whether Scopy properly handles the removal 
of a connected device.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`M2k.Emu <m2k-emu-dev>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
    3. Click the **Forget** button located at the bottom-right of the device icon.
        - **Expected result:** Initially, the device is disconnected and then 
          successfully removed from the device browser.
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


Test 4: Edit the name of the device.
------------------------------------

**UID:** TST.DEV.EDIT_NAME

**RBP:** P3

**Description:** This test checks if the **Edit pen** button work correctly.

**Test prerequisites:**
    - :ref:`TST.ADD.PLUGINS_ADD_SINGLE <tst-add-plugins-add>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`M2k.Emu <m2k-emu-dev>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Place the cursor in the top-right corner of the device icon in the 
       device browser.
    2. After the pen icon appears, click on it.
    3. Delete "M2k".
    4. Write "M3k".
    5. Press enter. 
        - **Expected result:**  The device name changes both in the device 
          browser and in the tool menu.
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


Test 5: Select a tool. 
----------------------

**UID:** TST.DEV.SELECT_TOOL

**RBP:** P1

**Description:** This test checks if the tool infrastructure works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`M2k.Emu <m2k-emu-dev>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
    3. Click on "Oscilloscope" tool.
        - **Expected result:** The "Oscilloscope" tool is displayed and the tool menu item is highlighted.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Click on "Home" menu entry.
        - **Expected result:** The device page is displayed and the home menu 
          item is highlighted.
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


.. _tst-dev-detach-tool:

Test 6: Detach a tool.
----------------------

**UID:** TST.DEV.DETACH_TOOL

**RBP:** P3

**Description:** This test checks if the tool detachment mechanism is working 
correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`M2k.Emu <m2k-emu-dev>` setup or :ref:`M2k.Usb <m2k-usb-dev>` setup - same as in the previous test.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
    3. Double click on "Oscilloscope" tool.
        - **Expected result:** The "Oscilloscope" tool is integrated into a 
          separate window from the application. The window title should be: 
          "Scopy-M2kPlugin-Oscilloscope-"ip:127.0.0.1 or usb:x.y.z.

        - **Actual result:**

..
  Actual test result goes here.
..

    4. Close the "Oscilloscope" window.
        - **Expected result:** The "Oscilloscope" tool is reintegrated into the main window. 
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Detach the oscilloscope tool again. 
    6. Close the  application.
        - **Expected result:** Scopy finished successfully. 
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


Test 7: Reopen the app with a detached tool.
--------------------------------------------

**UID:** TST.DEV.DETACH_TOOL_RE

**RBP:** P3

**Description:** This test checks whether detached tools retain their state upon 
reconnection.

**Test prerequisites:**
    - :ref:`TST.DEV.DETACH_TOOL <tst-dev-detach-tool>`.

**Preconditions:**
    - Scopy is installed on the system.
    - Use :ref:`M2k.Usb <m2k-usb-dev>` setup - same as in the previous test.
    - The application was previously closed with a detached tool (Oscilloscope).
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
        - **Expected result:** The oscilloscope tool is detached.
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


Test 8: Collapse device tools in the tool menu.
-----------------------------------------------

**UID:** TST.DEV.COLLAPSE

**RBP:** P1

**Description:** This test verifies if the collapse feature works properly in
the tool menu.

**Test prerequisites:**
    - :ref:`TST.ADD.PLUGINS_ADD_SINGLE <tst-add-plugins-add>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`M2k.Emu <m2k-emu-dev>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on the device header from tool menu.
        - **Expected result:** The tools collapse.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Repeat 2.
        - **Expected result:** The tools expand.
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


.. _tst-dev-multi-conn:

Test 9: Connecting to multiple devices.
---------------------------------------

**UID:** TST.DEV.MULTI_CONN

**RBP:** P1

**Description:** This test checks if connecting to multiple devices is working 
properly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - The "Connect to multiple devices" preference is enabled in the preferences 
      page.
    - Use :ref:`M2k.Emu <m2k-emu-dev>` and :ref:`M2k.Usb <m2k-usb-dev>` setups.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the emu device icon from device browser.
    2. Click on **Connect** button.
        - **Expected result:** The connection is established successfully.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Click on the usb device icon from device browser.
    4. Click on **Connect** button.
        - **Expected result:** The connection is established successfully, both 
          device icons have a green status bar, both devices are entered into the 
          tool menu (the device header contains the device name and uri), and the 
          tools of each device can be accessed. If the menu contains too many 
          entries, then the scroll bar will become visible.

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


Test 10: Resource manager with multiple devices.
------------------------------------------------

**UID:** TST.DEV.RM_MULTI

**RBP:** P1

**Description:** This test verifies that the resource manager is being used
properly.

**Test prerequisites:**
    - :ref:`TST.DEV.MULTI_CONN <tst-dev-multi-conn>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - The "Connect to multiple devices" preference is enabled in the 
      preferences page.
    - Use :ref:`M2k.Emu <m2k-emu-dev>` and :ref:`M2k.Usb <m2k-usb-dev>` setups.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the emu device icon from device browser.
    2. Click on **Connect** button.
    3. Click on the usb device icon from device browser.
    4. Click on **Connect** button.
    5. Click on the "Oscilloscope" run button of the emu device in the tool menu.
        - **Expected result:** The button changes from stop button to run button (green color).
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Click on the "Spectrum Analyzer" run button of the usb device in the tool menu.
        - **Expected result:** The button changes from stop button to run button, 
          and the oscilloscope button (of the emu device) remains the same.
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


Test 11: Devices order in device browser.
-----------------------------------------

**UID:** TST.DEV.ADD_ORDER

**RBP:** P0

**Description:** This test verifies that each newly added device is finally
entered into the device browser.

**Test prerequisites:**
    - :ref:`TST.ADD.PLUGINS_ADD_SINGLE <tst-add-plugins-add>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - The "Connect to multiple devices" preference is enabled in the 
      preferences page.
    - Use :ref:`M2k.Emu <m2k-emu-dev>` and :ref:`M2k.Usb <m2k-usb-dev>` setups.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Add the emu device.
    2. Add the usb device.
        - **Expected result:** The usb device is on the right side of emu 
          device. 
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