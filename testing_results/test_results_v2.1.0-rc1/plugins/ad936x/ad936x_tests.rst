.. _ad936x_tests:

AD936x Test Suite
================================================================================

**Prerequisites:**
    - Scopy v2.1.0 or later with AD936x plugin installed
    - AD936x device connected
    - Device drivers and firmware are up to date
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
       * - Muthi Ionut  
         - 30.07.2025
         - f76c8217  
         - 
         - 


Test 1: Plugin Loads
----------------------

**UID:** TST.AD936X.PLUGIN_LOADS

**Description:** Verify that the AD936x plugin loads in Scopy.

**OS:** ANY

**Preconditions:**
    - Scopy is installed

**Steps:**
    1. Open Scopy application
        - **Expected result:** Scopy launches without errors
        - **Actual result:**

..
  it behaves as expected.
..
    2. Open AD936x plugin
        - **Expected result:** AD936x plugin loads and is accessible in the UI
        - **Actual result:**

..
  it behaves as expected.
..

**Tested OS:**

..
  Windows 11
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS

..
  PASS
..

Test 2: Device Detection and Display
--------------------------------------

**UID:** TST.AD936X.DEVICE_DETECTION_AND_DISPLAY

**Description:** Verify that the AD936x plugin detects and displays the connected AD936x device and its status panels.

**OS:** ANY

**Preconditions:**
    - AD936x device is connected

**Steps:**
    1. Open AD936x plugin
        - **Expected result:** The plugin detects and displays the AD936x device and its status panels
        - **Actual result:**

..
  It behaves as expected.
..

**Tested OS:**

..
  Windows 11
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS

..
  PASS
..

Test 3: Change and Validate Device Global Settings
---------------------------------------------------

**UID:** TST.AD936X.CHANGE_VALIDATE_GLOBAL_SETTINGS

**Description:** Change ENSM mode, calibration, and rate governor; verify changes are applied and reflected in hardware.

**OS:** ANY

**Preconditions:**
    - AD936x device is connected

**Steps:**
    1. Change ENSM mode (FDD/TDD).
        - **Expected result:** Mode changes and is reflected in the UI and hardware.
        - **Actual result:**

..
  It behaves as expected.
..
    2. Change calibration mode and rate governor.
        - **Expected result:** New settings are applied and validated.
        - **Actual result:**

..
  It behaves as expected.
..

**Tested OS:**

..
  Windows 11
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS

..
  PASS
..

Test 4: RX and TX Chain Configuration
---------------------------------------

**UID:** TST.AD936X.RX_TX_CHAIN_CONFIG

**Description:** Change RX/TX bandwidth, sampling rate, LO frequency, and verify correct application.

**OS:** ANY

**Preconditions:**
    - AD936x device is connected

**Steps:**
    1. Change RX bandwidth and sampling rate.
        - **Expected result:** New values are applied and reflected in the UI and hardware.
        - **Actual result:**

..
  It behaves as expected.
..
    2. Change TX bandwidth and LO frequency.
        - **Expected result:** New values are applied and reflected in the UI and hardware.
        - **Actual result:**

..
  It behaves as expected.
..

**Tested OS:**

..
  Windows 11
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS

..
  PASS
..


Test 5: Advanced Plugin Detection and Display
-----------------------------------------------

**UID:** TST.AD936X_ADVANCED.PLUGIN_DETECTION_AND_DISPLAY

**Description:** Verify that the AD936x Advanced plugin detects and displays the connected AD936x device and its advanced configuration panels.

**OS:** ANY

**Preconditions:**
    - AD936x device is connected

**Steps:**
    1. Open AD936x Advanced plugin
        - **Expected result:** The plugin detects and displays the AD936x device and its advanced configuration panels
        - **Actual result:**

..
 It behaves as expected.
..

**Tested OS:**

..
 Windows 11
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
 PASS
..