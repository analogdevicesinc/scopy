.. _jesd_status_tests:

JESD STATUS Test Suite
================================================================================

**Prerequisites:**
    - Scopy v2.1.0 or later with JESD STATUS plugin installed
    - AD9084 JESD204 device connected
    - Device drivers and firmware are up to date
    - Reset .ini files to default by pressing **Reset** button in Preferences

Test 1: Plugin Loads and Displays JESD Device
----------------------------------------------

**UID:** TST.JESD_STATUS.PLUGIN_LOADS_AND_DISPLAYS_DEVICE

**RBP:** P3

**Description:** Verify that the JESD STATUS tool loads and displays the connected JESD204 Link Layer Device and its status panels.

**OS:** ANY

**Preconditions:**
    - Connect AD9084 device

**Steps:**
    1. Open JESD STATUS tool
        - **Expected result:** The tool displays the JESD204 Link Layer Device (e.g., axi-jesd204-rx) and its status panels
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

Test 2: Link Status Display
-----------------------------

**UID:** TST.JESD_STATUS.LINK_STATUS_DISPLAY

**RBP:** P3

**Description:** Verify that the tool displays the correct link status fields and color coding.

**OS:** ANY

**Preconditions:**
    - AD9084 device is connected

**Steps:**
    1. Using the emulated device, open the Debugger Plugin and modify the actual link status string to set the link status to ON (enable the link on AD9084)
        - **Expected result:** Status changes to "enabled" and is shown in green
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Using the emulated device, open the Debugger Plugin and modify the actual link status string to set the link status to OFF (disable the link on AD9084)
        - **Expected result:** Status changes to "disabled" and is shown in red
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

Test 3: Lane Status Table
----------------------------

**UID:** TST.JESD_STATUS.LANE_STATUS_TABLE

**RBP:** P3

**Description:** Verify that the Lane Status table displays error count and extended multiblock alignment for all lanes (0-7).

**OS:** ANY

**Preconditions:**
    - AD9084 device is connected

**Steps:**
    1. Check the Lane Status table for all lanes (0-7)
        - **Expected result:** Each lane displays error count and extended multiblock alignment (e.g., "EMB_INIT")
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

Test 4: SYSREF and Alignment Error Reporting
-----------------------------------------------

**UID:** TST.JESD_STATUS.SYSREF_ALIGNMENT_ERROR_REPORTING

**RBP:** P3

**Description:** Verify that SYSREF captured and SYSREF alignment error fields are displayed and update according to device state.

**OS:** ANY

**Preconditions:**
    - AD9084 device is connected

**Steps:**
    1. Verify SYSREF captured and SYSREF alignment error fields
        - **Expected result:** Values are displayed and update according to device state
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

Test 5: Real-Time Updates
----------------------------

**UID:** TST.JESD_STATUS.REAL_TIME_UPDATES

**RBP:** P3

**Description:** Verify that the JESD STATUS panel updates in real time when device state changes (e.g., enable/disable link, induce errors).

**OS:** ANY

**Preconditions:**
    - AD9084 device is connected

**Steps:**
    1. Externally enable the link on AD9084
        - **Expected result:** JESD STATUS panel updates to show link enabled
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Externally disable the link on AD9084
        - **Expected result:** JESD STATUS panel updates to show link disabled
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Induce errors on the device (if possible)
        - **Expected result:** JESD STATUS panel updates in real time to reflect errors
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

Test 6: UI/UX and Color Coding
-------------------------------

**UID:** TST.JESD_STATUS.UI_UX_COLOR_CODING

**RBP:** P3

**Description:** Verify that status fields use correct color coding and the UI is clear and responsive.

**OS:** ANY

**Preconditions:**
    - AD9084 device is connected

**Steps:**
    1. Check that status fields use correct color coding (e.g., red for errors/disabled, green for OK/enabled)
        - **Expected result:** UI is clear, responsive, and color coding matches status
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
