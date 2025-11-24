.. _connection_tests:

Connection flow
===============

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

.. _m2k-emu-conn:

**M2k.Emu:**
    - Open Scopy.
    - Start the iio-emu process using "adalm2000" in the dropdown and 
      "ip:127.0.0.1" as URI.
    - Add the device in device browser.

.. _m2k-usb-conn:

**M2k.Usb:**
    - Open Scopy.
    - Connect an **ADALM2000** device to the system by USB.
    - Add the device in device browser.

.. _tst-conn-succ:

Test 1: Verify the connection flow.
-----------------------------------

**UID:** TST.CONN.SUCC

**RBP:** P0

**Description:** This test checks if the **Connect** button works correctly.

**Test prerequisites:** 
    - :ref:`TST.ADD.PLUGINS_ADD_SINGLE <tst-add-plugins-add>`

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed in the system.
    - Use :ref:`M2k.Emu <m2k-emu-conn>` setup.
    - OS: ANY.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
        - **Expected result:** The **Disconnect** button appears, a status bar 
          is displayed under the icon in the device browser, and a progress bar 
          appears in the application's status bar. After a while, the connection 
          is established, the status bar turns green, and the tools in the menu 
          become accessible. 
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


Test 2: Unplug the device while connecting (AUTO-SCAN).
-------------------------------------------------------

**UID:** TST.CONN.UNPLUG_AUTO

**RBP:** P0

**Description:** This test verifies Scopy's behavior when the device is
unplugged during the connection process.

**Test prerequisites:** 
    - :ref:`TST.HP.AUTO_SCAN_ON <tst-hp-auto-scan-on>`.

**Preconditions:**
    - Scopy is installed on the system.
    - The "Regularly scan for new devices" preference is enabled in the 
      preferences page.
    - Use :ref:`M2k.Usb <m2k-usb-conn>` setup.
    - OS: ANY.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
    3. Unplug the device while connecting. 
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


Test 3: Unplug the device while connecting (MANUAL-SCAN).
---------------------------------------------------------

**UID:** TST.CONN.UNPLUG_AUTO

**RBP:** P3

**Description:** This test verifies Scopy's behavior when the device is 
unplugged during the connection process.

**Test prerequisites:**
    - :ref:`TST.HP.MANUAL_SCAN <tst-hp-manual-scan>`.

**Preconditions:**
    - Scopy is installed on the system.
    - The "Regularly scan for new devices" preference is disabled in the 
      preferences page.
    - Use :ref:`M2k.Usb <m2k-usb-conn>` setup.
    - OS: ANY.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
    3. Unplug the device while connecting. 
        - **Expected result:** A warning icon appears in the top-right corner 
          of the device in the device browser, and when hovered over, the 
          message "The device is not available! Verify the connection!" is 
          displayed.
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


Test 4: Unplug the device before connecting.
--------------------------------------------

**UID:** TST.CONN.UNPLUG_BEFORE

**RBP:** P0

**Description:** This test verifies Scopy's behavior when the device is
unplugged before the connection process.

**Test prerequisites:**
    - :ref:`TST.HP.MANUAL_SCAN <tst-hp-manual-scan>`.

**Preconditions:**
    - Scopy is installed on the system.
    - The "Regularly scan for new devices" preference is disabled in the 
      preferences page.
    - Use :ref:`M2k.Usb <m2k-usb-conn>` setup.
    - OS: ANY.

**Steps:**
    1. Click on the device icon from device browser.
    2. Unplug the device. 
    3. Click on **Connect** button.
        - **Expected result:** The connection process starts but does not 
          complete. A warning icon appears in the top-right corner.
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


Test 5: Disconnecting a device.
-------------------------------

**UID:** TST.CONN.DISCONN

**RBP:** P0

**Description:** This test checks if the **Disconnect** button works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed in the system.
    - Use :ref:`M2k.Emu <m2k-emu-conn>` setup.
    - OS: ANY.

**Steps:**
    1. Click on the device icon from device browser.
    2. Click on **Connect** button.
    3. Wait until the connection is done .
    4. Click on **Disconnect** button.
        - **Expected result:** The device is disconnected, the **Connect** 
          button appears, the device remains in the device browser, and the 
          tools in the menu are no longer accessible.
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