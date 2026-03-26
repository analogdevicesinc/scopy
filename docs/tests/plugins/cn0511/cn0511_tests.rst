.. _cn0511_tests:

CN0511 Test Suite
================================================================================

**Prerequisites:**
    - Scopy v2.1.0 or later with CN0511 plugin installed
    - CN0511 board connected, or emulator running:
      ``iio-emu cn0511 <repo>/packages/cn0511/emu-xml/cn0511.xml`` (URI: ``ip:127.0.0.1``)
    - Device drivers and firmware are up to date

Test 1: Plugin Loads
----------------------

**UID:** TST.CN0511.PLUGIN_LOADS

**RBP:** P0

**Description:** Verify that the CN0511 plugin loads in Scopy and the tool is accessible.

**OS:** ANY

**Preconditions:**
    - Scopy is installed
    - CN0511 board is connected or the CN0511 emulator is running

**Steps:**
    1. Open Scopy and connect to the CN0511 device or emulator
        - **Expected result:** Scopy connects without errors and the CN0511 tool appears in the tool list
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Open the CN0511 tool
        - **Expected result:** The CN0511 tool loads and displays both the Single Tone Mode and DAC Amplifier sections
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

Test 2: Single Tone Mode Section Visible
-----------------------------------------

**UID:** TST.CN0511.SINGLE_TONE_SECTION_VISIBLE

**RBP:** P1

**Description:** Verify that the Single Tone Mode section displays the Frequency spinbox, Amplitude spinbox, and Enter button.

**OS:** ANY

**Preconditions:**
    - CN0511 tool is open

**Steps:**
    1. Observe the Single Tone Mode section
        - **Expected result:** The section shows a Frequency (MHz) spinbox with default value 4500, an Amplitude (dBm) spinbox with default value 0, and an Enter button
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

Test 3: Set Output Frequency
------------------------------

**UID:** TST.CN0511.SET_FREQUENCY

**RBP:** P1

**Description:** Verify that setting a new frequency value and clicking Enter writes the frequency to the AD9166 DAC without errors.

**OS:** ANY

**Preconditions:**
    - CN0511 tool is open

**Steps:**
    1. Set Frequency (MHz) to 2000
        - **Expected result:** The spinbox accepts the value 2000
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Click the Enter button
        - **Expected result:** The Enter button shows a loading animation while the calibration is applied, then returns to its normal state; no error is reported
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

Test 4: Set Output Amplitude
------------------------------

**UID:** TST.CN0511.SET_AMPLITUDE

**RBP:** P1

**Description:** Verify that setting a new amplitude value and clicking Enter writes the amplitude to the AD9166 DAC without errors.

**OS:** ANY

**Preconditions:**
    - CN0511 tool is open

**Steps:**
    1. Set Amplitude (dBm) to -20
        - **Expected result:** The spinbox accepts the value -20
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Click the Enter button
        - **Expected result:** The Enter button shows a loading animation while the calibration is applied, then returns to its normal state; no error is reported
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

Test 5: DAC Amplifier Enable
------------------------------

**UID:** TST.CN0511.DAC_AMP_ENABLE

**RBP:** P1

**Description:** Verify that the DAC Amplifier section is visible and that toggling the Enable checkbox writes the ``en`` attribute to the ``ad9166-amp`` device.

**OS:** ANY

**Preconditions:**
    - CN0511 tool is open

**Steps:**
    1. Observe the DAC Amplifier section
        - **Expected result:** The section displays an Enable checkbox
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Toggle the Enable checkbox to enabled
        - **Expected result:** The checkbox state changes and the write is applied to the device without error
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Toggle the Enable checkbox to disabled
        - **Expected result:** The checkbox state changes and the write is applied to the device without error
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

Test 6: Refresh Reads NCO Frequency
--------------------------------------

**UID:** TST.CN0511.REFRESH_READS_FREQUENCY

**RBP:** P1

**Description:** Verify that clicking the Refresh button reads the current NCO frequency from the device and updates the Frequency spinbox.

**OS:** ANY

**Preconditions:**
    - CN0511 tool is open

**Steps:**
    1. Set Frequency (MHz) to 3000 and click Enter
        - **Expected result:** Calibration is applied without error
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Manually change the Frequency (MHz) spinbox to a different value without clicking Enter
        - **Expected result:** The spinbox shows the new value but the device has not been updated
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Click the Refresh button (top-right corner)
        - **Expected result:** The Refresh button shows a loading animation, then the Frequency spinbox updates to reflect the value currently on the device (3000 MHz)
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

Test 7: Verify Output Frequency with AD936x Receiver
------------------------------------------------------

**UID:** TST.CN0511.VERIFY_FREQUENCY_WITH_AD936X

**RBP:** P2

**Description:** Verify that the CN0511 RF output is present at the configured frequency by receiving it with a connected AD936x-based device (e.g., PlutoSDR or AD-FMCOMMS) using the AD936x plugin.

**OS:** ANY

**Preconditions:**
    - CN0511 board is connected (emulator cannot be used for this test)
    - A PlutoSDR or AD-FMCOMMS board is connected to the same host
    - CN0511 RF SMA output is connected to the AD936x board RX SMA input via coaxial cable
    - Both devices are visible in Scopy

**Steps:**
    1. Open the CN0511 tool, set Frequency (MHz) to 1000, Amplitude (dBm) to -10, and click Enter
        - **Expected result:** Calibration is applied; CN0511 is transmitting at 1000 MHz
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Open the AD936x plugin and set the RX LO Frequency to 1000 MHz
        - **Expected result:** The AD936x receiver tunes to 1000 MHz
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Observe the AD936x plugin signal display or spectrum view
        - **Expected result:** A signal is visible at or near 1000 MHz on the AD936x receiver
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
