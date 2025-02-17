.. _add_dev_tests:

Add device
==========

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

.. _m2k-emu-add:

**M2k.Emu:**
    - Open Scopy.
    - Start the iio-emu process using "adalm2000" in the dropdown and 
      "ip:127.0.0.1" as URI.

Test 1: Verifying the device scanning functionality.
----------------------------------------------------

**UID:** TST.ADD.SCAN_ALL

**Description:** This test checks that the device scan correctly detects and 
lists available devices when initiated.

**Preconditions:**
    - Scopy is installed on the system.
    - OS: ANY.

**Steps:**
    1. Connect an **ADALM2000** device to the system by USB.
    2. Open Scopy.
    3. Click on **Add device** (+) button.
    4. Click on the refresh button on the right located in the **Add page** scan 
       section.
    5. Wait until the refresh animation stops.
        - **Expected result:** In the "Context" dropdown, all available devices 
          are listed, each item containing the device name/description and URI. The 
          **ADALM2000** device [usb] must appear in the list.
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

Test 2: Verifying the local device scanning functionality.
----------------------------------------------------------

**UID:** TST.ADD.SCAN_LOCAL

**Description:** This test checks that the device scan correctly detects and 
lists available local devices when initiated.

**Preconditions:**
    - Scopy is installed on the system.
    - There is at least one IIO device connected to the system that has local 
      backend.
    - OS: ANY.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **local** checkbox located in the **Add page** scan section.
    4. Wait until the refresh animation stops.
        - **Expected result:** In the "Context" dropdown, all available local 
          devices are listed, each item containing the device name/description and URI. 
          If there is no available device the "No scanned devices available!" message 
          is displayed.
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

Test 3: Verifying the ip device scanning functionality.
-------------------------------------------------------

**UID:** TST.ADD.SCAN_IP

**Description:** This test checks that the device scan correctly detects and 
lists available ip devices when initiated.

**Preconditions:**
    - Scopy is installed on the system.
    - There is at least one IIO device connected to the system that has ip 
      backend.
    - OS: ANY.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **ip** checkbox located in the **Add page** scan section.
    4. Wait until the refresh animation stops.
        - **Expected result:** In the "Context" dropdown, all available ip 
          devices are listed, each item containing the device name/description and URI. 
          If there is no available device the "No scanned devices available!" message 
          is displayed.
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

Test 4: Verifying the usb device scanning functionality.
--------------------------------------------------------

**UID:** TST.ADD.SCAN_USB

**Description:** This test checks that the device scan correctly detects and 
lists available usb devices when initiated.

**Preconditions:**
    - Scopy is installed on the system.
    - OS: ANY.

**Steps:**
    1. Connect an **ADALM2000** device to the system by USB.
    2. Open Scopy.
    3. Click on **Add device** (+) button.
    4. Click on **usb** checkbox located in the **Add page** scan section.
    5. Wait until the refresh animation stops.
        - **Expected result:** In the "Context" dropdown, all available usb 
          devices are listed, each item containing the device name/description 
          and URI. The **ADALM2000** device must appear in the list. 
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

Test 5: Verifying the serial device scanning functionality.
-----------------------------------------------------------

**UID:** TST.ADD.SCAN_SERIAL

**Description:** This test checks that the device scan correctly detects and 
lists available serial devices when initiated.

**Preconditions:**
    - Scopy is installed on the system.
    - There is at least one IIO device connected to the system that has serial 
      backend.
    - OS: ANY.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on the refresh button on the right located in the **Add page** 
       serial port section.
    4. Wait until the refresh animation stops.
        - **Expected result:** In the "Name" dropdown, all available serial 
          devices are listed, each item containing the device name/description 
          and URI. If there is no available device the "No scanned devices 
          available!" message is displayed.
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

Test 6: Scanning with no device.
--------------------------------

**UID:** TST.ADD.SCAN_EMPTY

**Description:** This test checks whether an appropriate message is displayed 
when no device is found.

**Preconditions:**
    - Scopy is installed on the system.
    - OS: ANY.

**Steps:**
    1. Run Scopy.
    2. Make sure that no devices using USB backends are connected to the system.
    3. Click on **Add device** (+) button.
    4. Click on **usb** checkbox located in the **Add page** scan section.
    5. Wait until the refresh animation stops.
        - **Expected result:** The "No scanned devices available!" message is 
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

.. _tst-add-verify:

Test 7: Device validation.
--------------------------

**UID:** TST.ADD.VERIFY

**Description:** This test verifies that the device validation works correctly.

**Test prerequisites:**
    - :ref:`TST.EMU.EN <tst-emu-en>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`M2k.Emu <m2k-emu-add>` setup.
    - OS: ANY.

**Steps:**
    1. Click on **Add device** (+) button.
    2. Introduce the "ip:127.0.0.1" URI.
    3. Click on **Verify** button.
        - **Expected result:** A loading animation starts and after a short time 
          a page with device details and compatible plugins is displayed. 
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

Test 8: Verify the "BACK" button from compatible plugin page.
-------------------------------------------------------------

**UID:** TST.ADD.PLUGINS_BACK

**Description:** This test checks if the **BACK** button works correctly.

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`M2k.Emu <m2k-emu-add>` setup.
    - OS: ANY.

**Steps:**
    1. Click on **Add device** (+) button.
    2. Introduce the "ip:127.0.0.1" URI.
    3. Click on **Verify** button.
    4. Click on **BACK** button.
        - **Expected result:** The verify page is displayed.
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

.. _tst-add-plugins-add:

Test 9: Verify the "ADD DEVICE" button from compatible plugin page (single plugin).
-----------------------------------------------------------------------------------

**UID:** TST.ADD.PLUGINS_ADD_SINGLE

**Description:** This test checks if the **ADD DEVICE** button works correctly. 
Only the main plugin of the device is used.

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`M2k.Emu <m2k-emu-add>` setup.
    - OS: ANY.

**Steps:**
    1. Click on **Add device** (+) button.
    2. Introduce the "ip:127.0.0.1" URI.
    3. Click on **Verify** button.
        - **Expected result:** Only the M2kPlugin should be selected in the 
          compatible plugins section.
        - **Actual result:** 
        
..
  Actual test result goes here.
..

    4. Click on **ADD DEVICE** button.
        - **Expected result:** The device is added to the device browser, the 
          device's info page is displayed, and the m2k tools are added to the 
          tool menu. 
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

Test 10: Verify the "ADD DEVICE" button from compatible plugin page (multiple plugins).
---------------------------------------------------------------------------------------

**UID:** TST.ADD.PLUGINS_ADD_MULTIPLE

**Description:** This test checks if the **ADD DEVICE** button works correctly. 
Multiple plugins are used.

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`M2k.Emu <m2k-emu-add>` setup.
    - OS: ANY.

**Steps:**
    1. Click on **Add device** (+) button.
    2. Introduce the "ip:127.0.0.1" URI.
    3. Click on **Verify** button.
        - **Expected result:** Only the M2kPlugin should be selected in the 
          compatible plugins section.
        - **Actual result:** 
        
..
  Actual test result goes here.
..

    4. Select the DebuggerPlugin.
    5. Click on **ADD DEVICE** button.
        - **Expected result:** The device is added to the device browser, the 
          device's info page is displayed, and all tools, including the debugger, are added 
          to the tool menu.
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

Test 11: Verify the "ADD DEVICE" button from compatible plugin page with no plugin selected.
--------------------------------------------------------------------------------------------

**UID:** TST.ADD.PLUGINS_EMPTY

**Description:** This test checks if the **ADD DEVICE** button works correctly.

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`M2k.Emu <m2k-emu-add>` setup.
    - OS: ANY.

**Steps:**
    1. Click on **Add device** (+) button.
    2. Introduce the "ip:127.0.0.1" URI.
    3. Click on **Verify** button.
        - **Expected result:** Only the M2kPlugin should be selected in the 
          compatible plugins section.
        - **Actual result:** 

..
  Actual test result goes here.
..

    4. Uncheck the M2kPlugin.
    5. Click on **ADD DEVICE** button.
        - **Expected result:** A NO_PLUGIN device is added in device browser and 
          in tool menu.
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