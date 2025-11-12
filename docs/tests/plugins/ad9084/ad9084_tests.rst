
AD9084 Test Suite
================================================================================

**Prerequisites:**
    - Scopy v2.1.0 or later with AD9084 plugin installed
    - AD9084 device connected
    - Device drivers and firmware are up to date
    - Reset .ini files to default by pressing **Reset** button in Preferences

Test 1: Plugin Loads
----------------------

**UID:** TST.AD9084.PLUGIN_LOADS

**RBP:** P0

**Description:** Verify that the AD9084 plugin loads in Scopy and is accessible in the UI.

**OS:** ANY

**Preconditions:**
    - Scopy is installed

**Steps:**
    1. Open Scopy application
    2. Open AD9084 plugin
        - **Expected result:** AD9084 plugin loads and is accessible in the UI
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

**UID:** TST.AD9084.DEVICE_DETECTION_AND_DISPLAY

**RBP:** P3

**Description:** Verify that the plugin detects and displays the connected AD9084 device and its status panels.

**OS:** ANY

**Preconditions:**
    - AD9084 device is connected

**Steps:**
    1. Open AD9084 plugin
        - **Expected result:** The plugin detects and displays the AD9084 device and its status panels
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

Test 3: Channel Configuration
------------------------------

**UID:** TST.AD9084.CHANNEL_CONFIGURATION

**RBP:** P3

**Description:** Enable and disable individual channels; verify that channel status and data display update accordingly.

**OS:** ANY

**Preconditions:**
    - AD9084 device is connected

**Steps:**
    1. Enable a channel
        - **Expected result:** Channel is enabled 
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Disable the channel
        - **Expected result:** Channel is disabled 
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

Test 4: Disable and Enable RX/TX Tabs
--------------------------------------

**UID:** TST.AD9084.DISABLE_ENABLE_RX_TX_TABS

**RBP:** P3

**Description:** Disable then enable RX and TX tabs; verify that tab status and controls update accordingly.

**OS:** ANY

**Preconditions:**
    - AD9084 device is connected

**Steps:**
    1. Disable RX tab
        - **Expected result:** RX tab is disabled and controls are inaccessible
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Disable TX tab
        - **Expected result:** TX tab is disabled and controls are inaccessible
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Enable RX tab
        - **Expected result:** RX tab is enabled and controls are accessible
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Enable TX tab
        - **Expected result:** TX tab is enabled and controls are accessible
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
