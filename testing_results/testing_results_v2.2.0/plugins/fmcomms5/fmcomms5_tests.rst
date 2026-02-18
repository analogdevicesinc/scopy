.. _fmcomms5_tests:

FMCOMMS5 Test Suite
================================================================================

**Prerequisites:**
		- Scopy v2.2.0 or later with FMCOMMS5 plugin installed
		- FMCOMMS5 device connected (with both AD9361-phy and AD9361-phy-B devices)
		- Device drivers and firmware are up to date
		- Reset .ini files to default by pressing **Reset** button in Preferences

Test 1: Plugin Loads
----------------------

**UID:** TST.FMCOMMS5.PLUGIN_LOADS

**RBP:** P3

**Description:** Verify that the FMCOMMS5 plugin loads in Scopy.

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
		2. Open FMCOMMS5 plugin
				- **Expected result:** FMCOMMS5 plugin loads and is accessible in the UI
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

**UID:** TST.FMCOMMS5.DEVICE_DETECTION_AND_DISPLAY

**RBP:** P3

**Description:** Verify that the FMCOMMS5 plugin detects and displays both AD9361-phy and AD9361-phy-B devices and their status panels.

**OS:** ANY

**Preconditions:**
		- FMCOMMS5 device is connected

**Steps:**
		1. Open FMCOMMS5 plugin
				- **Expected result:** The plugin detects and displays both devices and their status/configuration panels
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

Test 3: Change and Validate Global Settings
--------------------------------------------

**UID:** TST.FMCOMMS5.CHANGE_VALIDATE_GLOBAL_SETTINGS

**RBP:** P3

**Description:** Change global settings (e.g., ENSM mode, calibration, rate governor) and verify changes are applied and reflected in hardware for both devices.

**OS:** ANY

**Preconditions:**
		- FMCOMMS5 device is connected

**Steps:**
		1. Change ENSM mode (FDD/TDD).
				- **Expected result:** Mode changes and is reflected in the UI and hardware for both devices.
				- **Actual result:**

..
	Actual test result goes here.
..
		2. Change calibration mode and rate governor.
				- **Expected result:** New settings are applied and validated for both devices.
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

**UID:** TST.FMCOMMS5.RX_TX_CHAIN_CONFIG

**RBP:** P3

**Description:** Change RX/TX bandwidth, sampling rate, LO frequency, and verify correct application for all four channels (RX1-4, TX1-4).

**OS:** ANY

**Preconditions:**
		- FMCOMMS5 device is connected

**Steps:**
		1. Change RX bandwidth and sampling rate for all RX channels.
				- **Expected result:** New values are applied and reflected in the UI and hardware for RX1-4.
				- **Actual result:**

..
	Actual test result goes here.
..
		2. Change TX bandwidth and LO frequency for all TX channels.
				- **Expected result:** New values are applied and reflected in the UI and hardware for TX1-4.
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

**UID:** TST.FMCOMMS5_ADVANCED.PLUGIN_DETECTION_AND_DISPLAY

**RBP:** P3

**Description:** Verify that the FMCOMMS5 Advanced plugin detects and displays the connected FMCOMMS5 device and its advanced configuration panels.

**OS:** ANY

**Preconditions:**
		- FMCOMMS5 device is connected

**Steps:**
		1. Open FMCOMMS5 Advanced plugin
				- **Expected result:** The plugin detects and displays the FMCOMMS5 device and its advanced configuration panels
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

Test 6: FMCOMMS5 Calibration
---------------------------------------------------------

**UID:** TST.FMCOMMS5.CALIBRATION

**RBP:** P3

**Description:** Verify that calibration completes successfully and a sinusoidal signal appears in the ADC plugin after calibration.

**OS:** ANY

**Prerequisites:**
        - ADC plugin installed
        - Physical FMCOMMS5 device connected (uncalibrated)

**Steps:**
        1. Connect an uncalibrated FMCOMMS5 device.
        2. Go to FMCOMMS5 Advanced and open the FMCOMMS5 tab.
        3. Press **Calibrate**.
                - **Expected result:** Progression bar appears and reaches 100%.
                - **Actual result:**

..
    Actual test result goes here.
..
        4. Using the dropdown, select "TX1B_B->RX1C_A".
        5. Go to ADC - Time plugin.
        6. Press **Run**.
        7. Enable voltage0 and voltage1.
                - **Expected result:** A sinusoidal waveform appears.
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
