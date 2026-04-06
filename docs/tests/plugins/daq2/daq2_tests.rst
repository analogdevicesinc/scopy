.. _daq2_tests:

DAQ2 Test Suite
================================================================================

**Prerequisites:**
    - Scopy v2.1.0 or later with DAQ2 plugin installed
    - DAQ2 device connected, or emulator running: ``iio-emu generic packages/daq2/emu-xml/daq2.xml``
    - Device drivers and firmware are up to date
    - Reset .ini files to default by pressing **Reset** button in Preferences

Test 1: Plugin Loads
----------------------

**UID:** TST.DAQ2.PLUGIN_LOADS

**RBP:** P0

**Description:** Verify that the DAQ2 plugin loads in Scopy when a compatible device is connected.

**OS:** ANY

**Preconditions:**
    - Scopy is installed
    - DAQ2 device or emulator is available

**Steps:**
    1. Open Scopy application.
        - **Expected result:** Scopy launches without errors.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Connect to the DAQ2 device (or emulator at ``ip:127.0.0.1``).
        - **Expected result:** DAQ2 plugin is detected and the DAQ2 tool appears in the tool menu.
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Open the DAQ2 tool.
        - **Expected result:** DAQ2 tool loads and displays the ADC and DAC sections without errors.
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

Test 2: ADC Section Visible
-----------------------------

**UID:** TST.DAQ2.ADC_SECTION_VISIBLE

**RBP:** P1

**Description:** Verify that the ADC section is displayed in the DAQ2 tool with the expected controls.

**OS:** ANY

**Preconditions:**
    - DAQ2 plugin is loaded and the DAQ2 tool is open

**Steps:**
    1. Open the DAQ2 tool.
        - **Expected result:** The ADC section is visible with the title "ADC", a sampling frequency label, and two test mode combo boxes labeled "Ch0 Test Mode" and "Ch1 Test Mode".
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

Test 3: ADC Sampling Frequency Displayed
------------------------------------------

**UID:** TST.DAQ2.ADC_SAMPLING_FREQ_DISPLAYED

**RBP:** P1

**Description:** Verify that the ADC sampling frequency is read from the hardware and displayed in MHz.

**OS:** ANY

**Preconditions:**
    - DAQ2 plugin is loaded and the DAQ2 tool is open

**Steps:**
    1. Open the DAQ2 tool and observe the ADC section.
        - **Expected result:** The "Sampling frequency:" label shows a numeric value in MHz (e.g., "1000.000 MHz"), not "N/A".
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

Test 4: ADC Ch0 Test Mode Write-Readback
------------------------------------------

**UID:** TST.DAQ2.ADC_CH0_TEST_MODE_WRITE_READBACK

**RBP:** P1

**Description:** Verify that changing the ADC Ch0 test mode is applied to the hardware and read back correctly.

**OS:** ANY

**Preconditions:**
    - DAQ2 plugin is loaded and the DAQ2 tool is open

**Steps:**
    1. In the ADC section, note the current value of the "Ch0 Test Mode" combo box.
        - **Expected result:** The combo shows one of the valid modes (e.g., "off").
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Select a different test mode from the "Ch0 Test Mode" combo (e.g., "pn9").
        - **Expected result:** The combo updates to the selected value and the hardware attribute ``test_mode`` on ``axi-ad9680-hpc/voltage0`` reflects the new setting.
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Restore the original test mode value.
        - **Expected result:** The combo returns to the original value and the hardware reflects this.
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

Test 5: ADC Ch1 Test Mode Write-Readback
------------------------------------------

**UID:** TST.DAQ2.ADC_CH1_TEST_MODE_WRITE_READBACK

**RBP:** P1

**Description:** Verify that changing the ADC Ch1 test mode is applied to the hardware and read back correctly.

**OS:** ANY

**Preconditions:**
    - DAQ2 plugin is loaded and the DAQ2 tool is open

**Steps:**
    1. In the ADC section, note the current value of the "Ch1 Test Mode" combo box.
        - **Expected result:** The combo shows one of the valid modes (e.g., "off").
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Select a different test mode from the "Ch1 Test Mode" combo (e.g., "pn23").
        - **Expected result:** The combo updates to the selected value and the hardware attribute ``test_mode`` on ``axi-ad9680-hpc/voltage1`` reflects the new setting.
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Restore the original test mode value.
        - **Expected result:** The combo returns to the original value and the hardware reflects this.
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

Test 6: DAC Section Visible
-----------------------------

**UID:** TST.DAQ2.DAC_SECTION_VISIBLE

**RBP:** P1

**Description:** Verify that the DAC section is displayed in the DAQ2 tool with the expected layout.

**OS:** ANY

**Preconditions:**
    - DAQ2 plugin is loaded and the DAQ2 tool is open

**Steps:**
    1. Open the DAQ2 tool and scroll to the DAC section.
        - **Expected result:** The DAC section is visible with the title "DAC" and a sampling frequency label.
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

Test 7: DAC Sampling Frequency Displayed
------------------------------------------

**UID:** TST.DAQ2.DAC_SAMPLING_FREQ_DISPLAYED

**RBP:** P1

**Description:** Verify that the DAC sampling frequency is read from the hardware and displayed in MHz.

**OS:** ANY

**Preconditions:**
    - DAQ2 plugin is loaded and the DAQ2 tool is open

**Steps:**
    1. Open the DAQ2 tool and observe the DAC section.
        - **Expected result:** The "Sampling frequency:" label shows a numeric value in MHz (e.g., "1000.000 MHz"), not "N/A".
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

Test 8: Refresh Button Updates Widgets
----------------------------------------

**UID:** TST.DAQ2.REFRESH_UPDATES_WIDGETS

**RBP:** P1

**Description:** Verify that clicking the refresh button re-reads all ADC widget values from the hardware.

**OS:** ANY

**Preconditions:**
    - DAQ2 plugin is loaded and the DAQ2 tool is open

**Steps:**
    1. Note the current values of "Ch0 Test Mode" and "Ch1 Test Mode" in the ADC section.
        - **Expected result:** Both combo boxes display valid test mode values.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Click the refresh button (top-right of the DAQ2 tool).
        - **Expected result:** The refresh button animates briefly, then stops. Both combo boxes still display valid test mode values consistent with the hardware state.
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

Test 9: ADC Ch0 Test Mode All Options Cycle
---------------------------------------------

**UID:** TST.DAQ2.ADC_CH0_TEST_MODE_ALL_OPTIONS

**RBP:** P2

**Description:** Verify that all nine test mode options for ADC Ch0 can be selected and read back correctly.

**OS:** ANY

**Preconditions:**
    - DAQ2 plugin is loaded and the DAQ2 tool is open

**Steps:**
    1. For each option in the "Ch0 Test Mode" combo — ``off``, ``midscale_short``, ``pos_fullscale``, ``neg_fullscale``, ``checkerboard``, ``pn9``, ``pn23``, ``one_zero_toggle``, ``ramp`` — select the option and observe the displayed value.
        - **Expected result:** Each option is applied immediately; the combo displays the newly selected value after each selection.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Restore the original test mode value.
        - **Expected result:** The combo returns to the value it held before this test.
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

Test 10: ADC Ch1 Test Mode All Options Cycle
----------------------------------------------

**UID:** TST.DAQ2.ADC_CH1_TEST_MODE_ALL_OPTIONS

**RBP:** P2

**Description:** Verify that all nine test mode options for ADC Ch1 can be selected and read back correctly.

**OS:** ANY

**Preconditions:**
    - DAQ2 plugin is loaded and the DAQ2 tool is open

**Steps:**
    1. For each option in the "Ch1 Test Mode" combo — ``off``, ``midscale_short``, ``pos_fullscale``, ``neg_fullscale``, ``checkerboard``, ``pn9``, ``pn23``, ``one_zero_toggle``, ``ramp`` — select the option and observe the displayed value.
        - **Expected result:** Each option is applied immediately; the combo displays the newly selected value after each selection.
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Restore the original test mode value.
        - **Expected result:** The combo returns to the value it held before this test.
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
