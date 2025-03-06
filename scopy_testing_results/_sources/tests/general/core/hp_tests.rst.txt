.. _hp_tests:

Homepage
========

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

Test 1: The homepage displays correctly when running the application.
---------------------------------------------------------------------

**UID:** TST.HP.RUN

**Description:** This test checks if the homepage is displayed correctly.

**Preconditions:**
    - Scopy is installed on the system.
    - OS: ANY.

**Steps:**
    1. Open Scopy.
    2. Observe the application's behavior.
        - **Expected result:** The application launches without errors and the 
          home page is displayed.
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


Test 2: Verifying the "Add device" (+) button.
----------------------------------------------

**UID:** TST.HP.ADD

**Description:** This test checks if the **Add device** button works correctly.

**Preconditions:**
    - Scopy is installed on the system.
    - OS: ANY.

**Steps:**
    1. Open Scopy.
    2. Navigate to the application's home page.
    3. Click on the **Add device** button, having a **+** icon.
    4. Observe the application's behavior.
        - **Expected result:** The page for adding a new device is displayed in 
          the info page section.
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


.. _tst-hp-auto-scan-on:

Test 3: Verifying the auto-scan feature (ON).
---------------------------------------------

**UID:** TST.HP.AUTO_SCAN_ON

**Description:** This test checks if the auto-scan feature detects and lists all
 available devices correctly.

**Preconditions:**
    - Scopy is installed on the system.
    - ADALM2000 drivers are installed on the system.
    - The "Regularly scan for new devices" preference is enabled in the 
      preferences page.
    - OS: ANY.

**Steps:**
    1. Open Scopy.
    2. Ensure the **Scan** switch is enabled in the device browser section.
    3. Connect an **ADALM2000** device to the system by USB.
    4. Observe the application's behavior.
        - **Expected result:** After a short period, the device appears in the 
          device browser section. 
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


.. _tst-hp-auto-scan-off:

Test 4: Verifying the auto-scan feature (OFF).
----------------------------------------------

**UID:** TST.HP.AUTO_SCAN_OFF

**Description:** This test checks the behavior of the application when the auto-
scan is turned off.

**Preconditions:**
    - Scopy is installed on the system.
    - ADALM2000 drivers are installed on the system.
    - The "Regularly scan for new devices" preference is enabled in the 
      preferences page.
    - OS: ANY.

**Steps:**
    1. Open Scopy.
    2. Ensure the **Scan** switch is disabled in the device browser section.
    3. Connect an **ADALM2000** device to the system by USB.
    4. Observe the application's behavior.
        - **Expected result:** The device should not be visible in the device 
          browser. 
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


.. _tst-hp-manual-scan:

Test 5: Verifying the manual scan feature.
------------------------------------------

**UID:** TST.HP.MANUAL_SCAN

**Description:** This test checks if the manual scan feature detects and lists 
all available devices correctly.

**Preconditions:**
    - Scopy is installed on the system.
    - ADALM2000 drivers are installed on the system.
    - The "Regularly scan for new devices" preference is disabled in the 
      preferences page.
    - OS: ANY.

**Steps:**
    1. Open Scopy.
    2. Connect an **ADALM2000** device to the system by USB.
    3. Wait a few seconds to ensure that the connection has been successfully 
       established.
    4. Click on the **Scan** button.
    5. Observe the application's behavior.
        - **Expected result:** The device appears in the device browser section.
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


Test 6: Navigation using the buttons in the info page.
------------------------------------------------------

**UID:** TST.HP.NAV

**Description:** This test checks if the navigation buttons work correctly.

**Test prerequisites:**
    - :ref:`TST.HP.AUTO_SCAN_ON <tst-hp-auto-scan-on>`.

**Preconditions:**
    - Scopy is installed on the system.
    - ADALM2000 drivers are installed on the system.
    - OS: ANY.

**Steps:**
    1. Open Scopy.
    2. Ensure the **Scan** switch is enabled in the device browser section.
    3. Connect an **ADALM2000** device to the system by USB.
    4. Wait until the device appears in device browser.
    5. Ensure that you are on "Welcome to Scopy" page.
    6. Click on the right arrow button.
        - **Expected result:** The info page changes from the 'Welcome to Scopy' 
          page to the add page and in device browser the related button is 
          highlighted.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Click on the left arrow button.
        - **Expected result:** The "Welcome to Scopy" page is displayed and the 
          related button is highlighted.
        - **Actual result:**

..
  Actual test result goes here.
..

    8. Click on the left arrow button.
        - **Expected result:** The device's page is displayed and the device 
          icon is highlighted. 
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

Test 7: Auto-scan behavior when the device is unplugged.
--------------------------------------------------------

**UID:** TST.HP.UNPLUGGED

**Description:** This test checks if the auto-scan feature detects and lists all 
available devices correctly.

**Test prerequisites:**
    - :ref:`TST.HP.AUTO_SCAN_ON <tst-hp-auto-scan-on>`.

**Preconditions:**
    - Scopy is installed on the system.
    - ADALM2000 drivers are installed on the system.
    - The "Regularly scan for new devices" preference is enabled in the 
      preferences page.
    - OS: ANY.

**Steps:**
    1. Open Scopy.
    2. Ensure the **Scan** switch is enabled in the device browser section.
    3. Connect an **ADALM2000** device to the system by USB.
    4. Wait until the device appears in device browser.
    5. Unplug the device.
    6. Observe the application's behavior.
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