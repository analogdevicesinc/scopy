.. _ad936x_tests:

AD936x Test Suite
================================================================================

**Prerequisites:**
    - Scopy v2.1.0 or later with AD936x plugin installed
    - AD936x device connected
    - Device drivers and firmware are up to date
    - Reset .ini files to default by pressing **Reset** button in Preferences

Test 1: Plugin Loads
----------------------

**UID:** TST.AD936X.PLUGIN_LOADS

**RBP:** P0

**Description:** Verify that the AD936x plugin loads in Scopy.

**OS:** ANY

**Preconditions:**
    - Scopy is installed

**Steps:**
    1. Open Scopy application
        - **Expected result:** Scopy launches without errors
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Open AD936x plugin
        - **Expected result:** AD936x plugin loads and is accessible in the UI
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..

Test 2: Device Detection and Display
--------------------------------------

**UID:** TST.AD936X.DEVICE_DETECTION_AND_DISPLAY

**RBP:** P3

**Description:** Verify that the AD936x plugin detects and displays the connected AD936x device and its status panels.

**OS:** ANY

**Preconditions:**
    - AD936x device is connected

**Steps:**
    1. Open AD936x plugin
        - **Expected result:** The plugin detects and displays the AD936x device and its status panels
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..

Test 3: Change and Validate Device Global Settings
---------------------------------------------------

**UID:** TST.AD936X.CHANGE_VALIDATE_GLOBAL_SETTINGS

**RBP:** P3

**Description:** Change ENSM mode, calibration, and rate governor; verify changes are applied and reflected in hardware.

**OS:** ANY

**Preconditions:**
    - AD936x device is connected

**Steps:**
    1. Change ENSM mode (FDD/TDD).
        - **Expected result:** Mode changes and is reflected in the UI and hardware.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Change calibration mode and rate governor.
        - **Expected result:** New settings are applied and validated.
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..

Test 4: RX and TX Chain Configuration
---------------------------------------

**UID:** TST.AD936X.RX_TX_CHAIN_CONFIG

**RBP:** P3

**Description:** Change RX/TX bandwidth, sampling rate, LO frequency, and verify correct application.

**OS:** ANY

**Preconditions:**
    - AD936x device is connected

**Steps:**
    1. Change RX bandwidth and sampling rate.
        - **Expected result:** New values are applied and reflected in the UI and hardware.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Change TX bandwidth and LO frequency.
        - **Expected result:** New values are applied and reflected in the UI and hardware.
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


Test 5: Advanced Plugin Detection and Display
-----------------------------------------------

**UID:** TST.AD936X_ADVANCED.PLUGIN_DETECTION_AND_DISPLAY

**RBP:** P3

**Description:** Verify that the AD936x Advanced plugin detects and displays the connected AD936x device and its advanced configuration panels.

**OS:** ANY

**Preconditions:**
    - AD936x device is connected

**Steps:**
    1. Open AD936x Advanced plugin
        - **Expected result:** The plugin detects and displays the AD936x device and its advanced configuration panels
        - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..