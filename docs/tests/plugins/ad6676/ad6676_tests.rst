.. _ad6676_tests:

AD6676 Test Suite
================================================================================

**Prerequisites:**
    - Scopy v2.1.0 or later with AD6676 plugin installed
    - AD6676 device connected
    - Device drivers and firmware are up to date
    - Reset .ini files to default by pressing **Reset** button in Preferences

Test 1: Plugin Loads
----------------------

**UID:** TST.AD6676.PLUGIN_LOADS

**RBP:** P0

**Description:** Verify that the AD6676 plugin loads in Scopy.

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

    2. Open AD6676 plugin
        - **Expected result:** AD6676 plugin loads and is accessible in the UI
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

**UID:** TST.AD6676.DEVICE_DETECTION

**RBP:** P1

**Description:** Verify that the AD6676 plugin detects the connected device and displays all attribute sections.

**OS:** ANY

**Preconditions:**
    - AD6676 hardware is connected

**Steps:**
    1. Open AD6676 plugin
        - **Expected result:** The plugin detects the AD6676 device and displays all five sections: ADC Settings, Bandwidth Settings, Receive Settings, Shuffler Settings, and Test Settings
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Verify that attribute fields in each section are populated with values read from hardware
        - **Expected result:** All attribute fields show current hardware values
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

Test 3: ADC Frequency Configuration
-------------------------------------

**UID:** TST.AD6676.ADC_SETTINGS

**RBP:** P1

**Description:** Verify that the ADC frequency attribute can be configured within and clamped at the valid range.

**OS:** ANY

**Preconditions:**
    - AD6676 hardware is connected
    - AD6676 plugin is open

**Steps:**
    1. In the ADC Settings section, set ``adc_frequency`` to a value within the valid range (2925–3200 MHz)
        - **Expected result:** The value is written to hardware and read back correctly
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Set ``adc_frequency`` to a value outside the valid range (e.g. below 2925 MHz or above 3200 MHz)
        - **Expected result:** The value is clamped to the nearest valid boundary and applied
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

Test 4: Bandwidth Settings Configuration
-----------------------------------------

**UID:** TST.AD6676.BANDWIDTH_SETTINGS

**RBP:** P1

**Description:** Verify that bandwidth and bandwidth margin attributes can be configured.

**OS:** ANY

**Preconditions:**
    - AD6676 hardware is connected
    - AD6676 plugin is open

**Steps:**
    1. In the Bandwidth Settings section, set ``bandwidth`` to a valid value
        - **Expected result:** The value is written to hardware and read back correctly
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Set ``bw_margin_low`` to a value in the range 0–30
        - **Expected result:** The margin is applied and reflected in the UI
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Set ``bw_margin_high`` to a value in the range 0–30
        - **Expected result:** The margin is applied and reflected in the UI
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Set ``bw_margin_if`` to a value in the range -30–30
        - **Expected result:** The margin is applied and reflected in the UI
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

Test 5: Receive Settings Configuration
----------------------------------------

**UID:** TST.AD6676.RECEIVE_SETTINGS

**RBP:** P1

**Description:** Verify that receive chain attributes can be configured correctly.

**OS:** ANY

**Preconditions:**
    - AD6676 hardware is connected
    - AD6676 plugin is open

**Steps:**
    1. In the Receive Settings section, set ``intermediate_frequency`` to a value in the range 70–450 MHz
        - **Expected result:** The value is applied and reflected in the UI
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Attempt to edit the ``sampling_frequency`` field
        - **Expected result:** The field is read-only and cannot be modified by the user
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Set ``hardwaregain`` to a value in the range -27–0 dB
        - **Expected result:** The gain value is applied and reflected in the UI
        - **Actual result:**

..
  Actual test result goes here.
..
  
    4. Set ``scale`` to a value in the range 0–12.04 dBFS
        - **Expected result:** The value is applied and converted correctly to/from linear representation
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

Test 6: Shuffler Settings Configuration
-----------------------------------------

**UID:** TST.AD6676.SHUFFLER_SETTINGS

**RBP:** P2

**Description:** Verify that shuffler control and threshold attributes can be configured via dropdown.

**OS:** ANY

**Preconditions:**
    - AD6676 hardware is connected
    - AD6676 plugin is open

**Steps:**
    1. In the Shuffler Settings section, select an option from the ``shuffler_control`` dropdown
        - **Expected result:** The selected option is applied and reflected in the UI
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Select an option from the ``shuffler_thresh`` dropdown
        - **Expected result:** The selected threshold is applied and reflected in the UI
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

Test 7: Test Mode Configuration
---------------------------------

**UID:** TST.AD6676.TEST_MODE

**RBP:** P2

**Description:** Verify that test mode patterns can be selected and cleared via the Test Settings section.

**OS:** ANY

**Preconditions:**
    - AD6676 hardware is connected
    - AD6676 plugin is open

**Steps:**
    1. In the Test Settings section, select a test pattern from the ``test_mode`` dropdown
        - **Expected result:** The selected test pattern is applied to the hardware
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Set ``test_mode`` back to "off"
        - **Expected result:** Test mode is disabled and the hardware returns to normal operation
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

Test 8: ADC Frequency Updates Bandwidth Range
----------------------------------------------

**UID:** TST.AD6676.DYNAMIC_BANDWIDTH_RANGE

**RBP:** P2

**Description:** Verify that changing the ADC frequency dynamically updates the valid range for the bandwidth attribute.

**OS:** ANY

**Preconditions:**
    - AD6676 hardware is connected
    - AD6676 plugin is open

**Steps:**
    1. Note the current ``adc_frequency`` value and observe the ``bandwidth`` attribute range
        - **Expected result:** The bandwidth range is 0.5%–5% of the current ADC frequency
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Change ``adc_frequency`` to a different valid value
        - **Expected result:** The ``bandwidth`` attribute's min/max range updates to 0.5%–5% of the new ADC frequency
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

Test 9: Refresh Functionality
------------------------------

**UID:** TST.AD6676.REFRESH

**RBP:** P2

**Description:** Verify that the Refresh button re-reads all attribute values from hardware.

**OS:** ANY

**Preconditions:**
    - AD6676 hardware is connected
    - AD6676 plugin is open

**Steps:**
    1. Click the Refresh button in the AD6676 plugin toolbar
        - **Expected result:** All attribute values across all sections are re-read from hardware and the displayed values update accordingly
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
